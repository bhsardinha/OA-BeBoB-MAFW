# A2-P2 — Notification receive path (closes A1-Q14a)

**Scope:** How the panel receives the Mach-port async notifications the kext's `MAServiceClient` broadcasts via `IOConnectSetNotificationPort`.

**Decompile log:** `ghidra-exports/decomp/a2-batch1.log` — `CXDevice::RegisterForNotifications` @ 0006cfd6, `CXDevice::StaticHandleMachMessage` @ 00070eda, `CXDevice::HandleNotification` @ 00070f18.

## Setup (`CXDevice::RegisterForNotifications`)

```
IOMasterPort(bootstrap_port, &master)
notifyPort = IONotificationPortCreate(master)
machPort   = IONotificationPortGetMachPort(notifyPort)

ctx.version         = 1
ctx.info            = this
ctx.retain          = ctx.release = NULL
ctx.copyDescription = NULL

cfPort = CFMachPortCreateWithPort(NULL, machPort,
                                  CXDevice::StaticHandleMachMessage,
                                  &ctx, NULL)
src    = CFMachPortCreateRunLoopSource(NULL, cfPort, 0)
CFRunLoopAddSource(CFRunLoopGetCurrent(), src, kCFRunLoopDefaultMode)

IOConnectSetNotificationPort(this->conn,
                             0,         /* notificationType = 0 (any) */
                             machPort,
                             0)         /* refCon */
```

**Mach-port wiring confirms the kext-side model from A1-Q14 verbatim:**

- The kext calls `IOUserClient::setNotificationPort(port, type=0, refCon=0)` and stores the Mach port + a 4-byte `msgh_id = 0` tag.
- From then on, every call to `MAServiceClient::sendAsyncNotification` wraps a `MAudioSettingsMessage` (header `mach_msg_header_t` + 32-byte body) into a `mach_msg_send`.
- The panel's run loop pulls it off `machPort`; `CFMachPortInvalidationCallback` → `StaticHandleMachMessage(cfPort, msg, size, info)`.

## Dispatch (`StaticHandleMachMessage` → `HandleNotification`)

```
CXDevice::StaticHandleMachMessage(cfPort, msg, size, info):
    [[NSAutoreleasePool alloc] init]
    CXDevice *dev = (CXDevice*)info
    CXDevice::HandleNotification(dev, msg, size)
    [pool release]

CXDevice::HandleNotification(this, msg, size):
    uint32_t bodyType = *(uint32_t*)(msg + 0x18)       /* offset 0x18 from msg start */
    if (bodyType == 0)
        this->ReceivedPropertiesChangedMessage(msg)
    /* no other bodyTypes handled — anything else is dropped silently */
```

**Correction from P9 (2026-10-06):** earlier draft of this doc listed three bodyType cases (0/1/2) — only `bodyType == 0` is actually handled in the shipping binary. There is no `ReceivedPeakMeterUpdate` or `ReceivedRateLockedChange` function. Peak meters and sample-rate state are **pulled** via a ~30 Hz `LPeriodical` tick that calls `GetHWSetting(cookie)` and broadcasts the result over `LBroadcaster` message `0x3f2` — see `P9-peak-meters.md`. The Mach-port notification is used **only** for property-dict change events.

The `+0x18` offset into the Mach message corresponds to **the first byte after the standard 24-byte `mach_msg_header_t`** — i.e. the first word of the `MAudioSettingsMessage` body is the discriminator. This is exactly the 44-byte message layout A1-Q14 predicted:

| offset | field                          |
|-------:|--------------------------------|
| 0x00   | `mach_msg_header_t` (24 bytes) |
| 0x18   | `uint32_t bodyType`            |
| 0x1c   | `uint32_t changedKeyID` (unused for bodyType 0 — the panel re-fetches the whole dict and diffs) |
| 0x20   | 20 bytes of body payload (ignored for bodyType 0) |

The body-type field is effectively a single value in practice (`0 = property-dict changed`). Any kext reserving codes 1/2 for peak/rate is wasted work on this panel — those paths are polled.

## `ReceivedPropertiesChangedMessage` (summary, full write-up with P3 integration)

- Reads the current full property dictionary via `IORegistryEntryCreateCFProperties` on `this->conn`.
- Diffs it against `this->cachedPropertyDict` (`this[+0x6c]`).
- For every changed key: looks up the handler C function in `this->propHandlerDict` (`this[+0x74]`), calls it with `(this, newNSNumberOrNSArray)` — these are the `ReceiveSWReturnLevelChange` / `ReceiveClockSource` / etc. functions enumerated in P3.
- Replaces `this->cachedPropertyDict` with the new one.

**Clone implication:** our user client broadcast only needs to send `bodyType == 0` when the property dict changes. The panel re-fetches the full dict via `IORegistryEntryCreateCFProperties` on each notification and diffs — it does not read `keyOrFrame` or the payload tail at all for bodyType 0. For peaks, rate-lock, and anything else time-varying, the panel runs its own 30 Hz pull — no kext push needed.

## Clone implication for `MAServiceClient::sendAsyncNotification`

The kext's `sendAsyncNotification()` needs this wire format (minimum viable — only bodyType 0 is consumed):

```c
struct __attribute__((packed)) MAudioSettingsMessage {
    mach_msg_header_t hdr;        /* 24 B — msgh_remote_port = fPort,
                                            msgh_bits = MACH_MSGH_BITS(COPY_SEND, 0),
                                            msgh_size = 44, msgh_id = 0 */
    uint32_t bodyType;            /* always 0 on this panel */
    uint32_t keyOrFrame;          /* unread by panel */
    uint8_t  payload[16];         /* unread by panel */
};
```

Panel is agnostic to the sender address — it only validates `bodyType == 0` and triggers a full property-dict re-fetch; the 16-byte tail is never dereferenced.

**Rate-change path** does *not* travel through this notification. See `P8-rate-and-buffer.md` and `P9-peak-meters.md` — the panel polls the sample rate via cookie `0x601` as part of the same `LPeriodical` tick that services peak meters.

## Status

Closes **A1-Q14a**. The `bodyType == 1 / == 2` lines in the earlier draft of this doc were speculative hopes; P9 confirmed those branches do not exist. See `P9-peak-meters.md` for the real meter-dispatch path (shared-memory map + 30 Hz GetHWSetting pump + LBroadcaster `0x3f2`).
