⏺ Consulting proposal — optimal clone topology for 10.13.6                                                  
                                               
  Posture: keep the reference's shape where 10.13.6 either forbids modernization (kext DCL, IOAudioFamily,
  CoreMIDI CFPlugIn) or where the reference's choice is simply correct. Modernize the glue layers (IPC,
  launchd model, UI toolkit) because that's where 2007-era conventions actively cost us. Below, each
  component gets a verdict: keep, modernize, or drop.                                                       
                                          
  ---                                                                                                       
  1. Non-negotiable facts that constrain the design                                                         
                                                                                                            
  - Kext stays IOAudioFamily. AudioServerPlugIn (userland CoreAudio) doesn't own FireWire isoch DMA cleanly 
  on 10.13.6; the kernel-side DMA mapping is what makes low-latency work.                                   
  - Classic DCL only (not NuDCL). createLocalIsochPort returns kIOReturnNoResources from kernel space on    
  NuDCL in 10.13.6 — the reference 2014 kext knew this; our clone kext learned the hard way
  (kext/README.md).                                                                                         
  - CoreMIDI driver model is CFPlugIn. DriverKit CoreMIDI arrives post-10.13. Stuck here.
  - Xcode 9.4.1 → Swift 4.1. No SwiftUI, no async/await, no @MainActor. Swift is still fine for userland    
  components; just know what you're missing.                      
  - Carbon is dead on x86_64. Rewriting the panel in AppKit isn't a modernization choice — it's forced.     
                                                                                                            
  Everything else is up for discussion.                                                                     
                                                                                                            
  ---                                                                                                       
  2. Component-by-component verdict                                                                         
                                                                                                            
  Kext — keep shape, tighten internals                                                                      
                                                                  
  The reference's FWMetaNub + per-product *Device subclass + MakeAppleFWAudioGoAway AV/C blocker is a clean 
  three-layer design. Keep it exactly:    
                                                                                                            
  - com_openaudio_FW1814Nub matches on vendor+model (both 0x10070 bootloader and 0x10071 runtime via two    
  personalities).                                                                                           
  - com_openaudio_FW1814Device is the audio engine host.                                                    
  - com_openaudio_MakeAppleFWAudioGoAway keeps AppleFWAudio off the AV/C subunits.                          
                                              
  Internal tightening:                                                                                      
  - Split protocol logic (AV/C envelope, HSCI vendor-dependent framing, mixer model, BCD header parse) into 
  a core/ C++17 library compiled twice: once with -mkernel -fno-exceptions -fno-rtti for the kext, once with
   libc++ for userland. The reference bakes everything into the kext monolithically; splitting it is nearly 
  free and pays off the day you need to port to AudioServerPlugIn or build a userland test harness.         
  - Use os_log + os_signpost throughout the kext. os_signpost landed in 10.13 specifically — perfect for    
  proving your round-trip latency budget matches or beats the 2014 driver.                              
  - Model the user-client as a typed selector table with explicit input/output struct layouts, not the flat 
  uint32_t selector + opaque payload idiom. The reference's selector surface is implicit — we have one     
  chance to make ours self-documenting.                                                               
                                                                                                            
  Firmware helper — modernize substantially
                                                                                                            
  The reference daemon is where 2007 conventions sting hardest. Four worthwhile changes, in order of value:
                                                                                                            
  - XPC instead of NSConnection Distributed Objects. DO is deprecated and effectively a dead ABI on anything
   newer than 10.13. XPC gets you typed messages via NSXPCConnection + NSXPCInterface, codable payloads, and
   automatic lifecycle. The reference's "panel talks to daemon via DO over named Mach port" is a straight   
  line translation to one XPC service.                                                                      
  - Drop the StartupItem bundle entirely. Install directly under  
  /Library/LaunchDaemons/com.openaudio.firewirehelper.plist. The reference ships both for some pre-10.4     
  compat ghost. We don't owe anyone that.                         
  - Split daemon ↔ user UI via LaunchAgent, not sudo -u shell exec. The reference literally does [NSString 
  stringWithFormat:@"sudo -u %@ %@ launch %@ %@ %@", user, path, icon, name, guid] + system(3). In 2026     
  that's a code review failure. The clean pattern is: daemon runs as root in /Library/LaunchDaemons/; status
   UI (if we ship one) runs as a LaunchAgent in /Library/LaunchAgents/; they wire up via a shared XPC       
  service name. No shell execs, no setuid ceremony.                                                         
  - Add SIGTERM handling + KeepAlive={SuccessfulExit=false}. Reference has neither. These together make the
  daemon survive crashes and shut down cleanly on logout — both free wins.                                  
                                                                  
  Language choice: Swift 4.1 for the daemon. IOKit from Swift is awkward but manageable with a thin Obj-C
  bridging header for IOServiceAddMatchingNotification callbacks. The alternative (Go, as the current       
  scaffold uses) is fine but gives up Codable + os_log + the natural XPC story.
                                                                                                            
  Status UI — drop as mandatory, keep as optional                 
                                                                                                            
  A4-H5 showed the reference's status UI is a LSUIElement=1 background Cocoa app with a progress bar,
  invoked via that sudo -u trick. It's genuinely optional. Firmware upload is a CFProperty write on the kext
   — it works at the login window with no UI present.
                                                                                                            
  - Simplest path: no status UI. Progress is a /var/log/system.log line via os_log. Users who want to know
  can log stream --predicate 'subsystem == "com.openaudio.fwhelper"'.                                       
  - If you want parity: build it as a LaunchAgent + AppKit progress window + XPC progress stream from the
  daemon. 300 lines of Swift. Only trigger it when a user is logged in; otherwise log-only.                 
                                          
  I lean toward no status UI for the clone unless you specifically miss it. The reference's UI is a 2007 UX 
  relic.                                                                                                    
   
  Control panel — AppKit + Swift, with a clear data model                                                   
                                                                  
  Carbon is non-negotiable here (dead on x86_64). The real design question is how the panel talks to the    
  kext:                                                                                                     
   
  - Direct user-client, not through the helper. The reference's helper is firmware-only; the panel opens its
   own user-client against the runtime kext. Keep that split — it means a panel crash can't poison firmware
  upload, and vice versa.                                                                                   
  - One typed user-client wrapper, exposed to the Swift panel via Obj-C. Methods look like                  
  -(IOReturn)setClockSource:(ClockSource)src error:(NSError**)err, not IOConnectCallScalarMethod(conn, 42, 
  …). All selector numbers hide inside one .mm file.                                                        
  - State model: single source of truth is the device. The panel reads the current state from the kext
  (which read it from the device via AV/C descriptor queries, per A5), reflects it to the UI, and writes    
  back on user action. Don't cache state in the panel. The reference does cache, and it's where UI/device
  desync bugs come from.                                                                                    
  - Settings persistence: per-device NSUserDefaults suite keyed by device GUID — exactly what the reference 
  does, just with a com.openaudio.panel.<GUID> suite name. Reference stores per-user prefs; preserve that
  model.                                                                                                    
                                                                  
  CoreMIDI plug-in — keep verbatim                                                                          
                                          
  A3 showed the MIDI plugin is an in-process CFPlugIn with a well-understood endpoint model. Modernization  
  options are nil (DriverKit CoreMIDI is post-10.13), and the reference's shape is already correct. Reuse   
  the core/ AV/C envelope for the wire format and otherwise mirror the reference.
                                                                                                            
  Prefpane — drop                                                 
                                                                                                            
  A6 was unambiguous: it's a trampoline. Modern macOS discourages .prefPane bundles. Install the panel as
  /Applications/FW1814 Panel.app, done. Zero functional loss.                                               
   
  Firmware payload handling — mirror exactly                                                                
                                                                  
  A5 proved both CRCs line up and the payload is opaque ARM code + a BridgeCo attribute namespace the device
   self-reports back over AV/C. Our clone:                                                                  
  - mmap the .bcd.
  - Verify payload CRC (h[0x3c]) — reference does this.                                                     
  - Also verify appendix CRC (h[0x5c]) — reference skips this; we don't have to. Free integrity win.
  - Hand the opaque NSData to the kext via setProperty:@"OpenAudioFirmwareImage".                           
                                                                                                            
  Don't parse the attribute namespace in the driver. If we ever want to use its PLL / mixer declarations,
  read them from the device via AV/C descriptors (per A5-mentioned FFADO pattern), not from the file. Keeps 
  us honest about reference-parity.                                                                         
                                                                                                            
  ---                                                                                                       
  3. Cross-cutting choices                                                                                  
                                                                                                            
  IPC topology (the biggest architectural win)                    
                                              
                             ┌─────────────────────────────┐                                                
                             │  OpenAudioFW1814.kext       │
                             │  (IOAudioFamily + user      │                                                
                             │   client — typed methods)   │      
                             └──────▲──────────────▲───────┘                                                
                                    │              │                                                        
                   IOServiceOpen +  │              │  IOServiceOpen +                                       
                   typed selectors  │              │  CFProperty writes                                     
                                    │              │                                                        
                     ┌──────────────┴─┐    ┌───────┴──────────────┐                                         
                     │  Panel.app     │    │  openaudio-fwhelper  │                                         
                     │  (AppKit/Swift)│    │  (LaunchDaemon       │                                         
                     │                │    │   /Library/          │                                         
                     │                │    │   LaunchDaemons)     │                                         
                     └──────▲─────────┘    └───────▲──────────────┘                                         
                            │                      │                                                        
                            │   XPC (optional)     │                                                        
                            └──────────────────────┘                                                        
                                   (status/control events,                                                  
                                    not required for firmware upload)                                       
                                                                                                            
  Key simplification over the reference: the panel talks to the kext directly for everything control-plane. 
  It only needs XPC to the helper for "trigger upload" or "subscribe to firmware status," which are rare    
  events. The reference tangled control-plane and firmware-plane through the daemon unnecessarily.          
                                                                                                            
  Logging + tracing                                                                                         
                                                                                                            
  - os_log with a com.openaudio.fw1814 subsystem, categories per component (kext, helper, panel, midi).
  - os_signpost for every DMA callback, every AV/C round-trip, every rate change. This is how we prove C6   
  (latency budget) in Part D numerically rather than by hand-waving.
                                                                                                            
  Config files + sentinels                                        
                                                                                                            
  - Drop the ad-hoc /Library/Preferences/com.m-audio.firewire.firmware.autoupdate.{not,force} sentinel
  files. Replace with a single JSON /Library/Preferences/com.openaudio.fwhelper.json with keys              
  {"autoupdateInhibited": bool, "forceNextUpdate": bool}. One file to live-edit, one Codable struct to
  parse.                                                                                                    
  - Per-user panel settings stay in NSUserDefaults suites.        
                                                                                                            
  Build layout                                                    

  openaudio-fw1814/                                                                                         
  ├── core/              # transport-agnostic C++17 — AV/C, HSCI, BCD, mixer model
  ├── kext/              # IOKit shim + classic DCL — links core/ with -mkernel                             
  ├── fwhelper/          # LaunchDaemon in Swift 4.1 — links core/ via Obj-C bridge                         
  ├── panel/             # AppKit app in Swift 4.1 — links core/ for format strings                         
  ├── midi-plugin/       # CFPlugIn — C++ linking core/                                                     
  └── _extracted/        # untouched reference (read-only)                                                  
                                                                                                            
  The scaffold already has kext/, firmware-helper/, and (empty) control-panel/. The step is to introduce    
  core/ and move the protocol code into it before you have too much duplication.                            
                                                                                                            
  ---                                                                                                       
  4. What I'd not modernize                                                                                 
                                                                                                            
  A few places where the reference is right and we'd regret deviating:
                                              
  - AV/C descriptor discovery at probe time. Hard-coded would break on firmware revisions. Reference does it
   right; mirror it.                      
  - IOCommandGate for all state transitions in the kext. Not glamorous but single-serialization-point is the
   thing that makes isoch reliable. Don't try to parallelize probe/start/stop.                              
  - IOAudioFamily buffer-size ladder (64…4096, default 512). Already in memory as                           
  project_fw1814_buffer_sizes.md. Don't invent values.                                                      
  - AV/C stream-kick after CMP connect (project_fw1814_stream_kick.md). Preserve exactly.                   
  - Classic DCL ring shape per reference_dcl_structure_authoritative.md. Preserve exactly.
                                                                                                            
  ---                                                             
  5. Net recommendation                                                                                     
                                                                                                            
  Keep the kext's class hierarchy, DCL topology, and firmware-upload wire protocol identical to the         
  reference. Modernize everything around it: XPC replaces DO, LaunchDaemon-only install replaces            
  StartupItem, AppKit+Swift replace Carbon, direct kext↔panel user-client replaces the daemon-as-middleman  
  tangle, structured logging + signposts replace NSLog + "feels about right" latency claims. Factor the
  protocol layer into core/ so a later retarget doesn't require re-reverse-engineering.                     
                                                                  
  Rough effort delta vs. "mirror the reference verbatim": maybe +15% on the glue layers, minus whatever time
   the reference's quirks (DO, StartupItem, sudo-shell-exec) would cost to debug. Net likely a wash in time,
   substantial win in debuggability.
                                                                                                            
  One thing I'd flag before Part B: if B5 (live DTrace against the SL Mac Pro) reveals that the panel-side
  IPC is heavier than I think — e.g. the panel streams meter data at 60 Hz through the daemon rather than   
  directly from the kext — the "panel talks to kext directly" simplification above weakens. Worth confirming
   in B5 whether the meter path involves the daemon at all; I suspect not, but it's cheap to check.

  ---
  6. Mandatory design invariants discovered via B5 live DTrace (2026-10-09)

  These are non-negotiable deltas the clone MUST adopt over the reference. Each cites the ground-truth
  evidence that forces it.

  **A. Rate-change must gate on engine-busy** — `kIOReturnBusy` if any client has the IOAudioEngine open.

   - Evidence: SL v1.9 reference kernel-panicked on 2026-10-09 when the Panel's sample-rate dropdown fired
     a `MASampleRate` setProperties write while iTunes/coreaudiod was holding the engine open. Stack:
     `_is_io_registry_entry_set_properties → IOAudioFamily::setFormat gated action → m-audio.driver.firewire
     + 0x11005 (engine setFormat callback) → IOAudioFamily helper → buffer realloc → _blkclr page fault on
     unmapped page`. CR2 = `0x00e3c000`. See memory `project_v19_rate_change_while_streaming_panics.md`
     and `docs/system-map/B5-sl-dtrace-truth/README.md §Session 2026-10-09 PM`.
   - Root cause: the reference's engine `::performFormatChange` hands IOAudioFamily a stale/misaligned
     buffer pointer when the DMA ring is still in-flight. It skips the pause-reconfigure-resume dance that
     IOAudioFamily expects.
   - Clone implementation (locked):
     - In `com_openaudio_FW1814Engine::performFormatChange()` (or wherever the `MASampleRate` property
       dispatcher lands), return `kIOReturnBusy` if `getNumClients() > 0`. Panel should then surface
       a user-facing "stop playback to change rate" message rather than attempting a hot swap.
     - Do NOT attempt the full pause-swap-resume dance as an alternative. It's higher-risk and the
       reference 2014 kext never did it either — accepting a client-busy error is the correct behavior
       for a format change that invalidates the DMA ring.
     - The guard belongs on the KEXT side, not Panel-side. Even if a well-behaved Panel refuses to send
       the write while clients are active, a third-party caller (another AU, a scripted ioreg write,
       the open-source Rust crate) must not be able to crash the kernel.
   - Open question for Part D re-check: does the v1.11.2 `_extracted/kext/` have the same bug? If
     already fixed in 1.11.2, we still want the explicit guard for defense-in-depth. Re-decompile
     `_extracted/kext/`'s `MASampleRate` dispatch when Part D code work resumes.

  **B. `MAClockSource` is a composite 32-bit word; dispatch per field, then let the engine format
  re-resolve absorb bit 4 / bit 5 side-effects.** The reference kext does NOT have a dedicated
  bit-mask dispatcher — refined 2026-10-12 via Ghidra decompile of
  `SwitchDigitalSignals @ 0xe5dc` and `InitializeUserSettableProperties @ 0xe6ca`
  (`docs/system-map/A1-ghidra/ghidra-exports/decomp/dispatcher-mute.log`).

   - Evidence: `docs/system-map/B5-sl-dtrace-truth/captures/pulled/66-dtrace-smux-toggle.txt` + memory
     `project_maclocksource_is_composite_bitfield.md` + `A1-ghidra/Q5-clock-source.md §Dispatcher + mute-bit decode`.
   - Bit layout (**fully locked 2026-10-12 PM via caps 66+68+69**):
     | Bit(s) | Field | Encoding |
     |---|---|---|
     | bit 30 (`0x40000000`) | required gate | reject write if clear |
     | bits [1:0] | **Sync Source radio** (4-radio UI per manual §2.3) | `00`→Internal-unmuted, `01`→External-digital, `10`→Word-Clock, `11`→Internal-muted |
     | bit 2 (`0x4`) | **S/PDIF input port** | 0=optical, 1=coaxial |
     | bit 3 (`0x8`) | **Optical-IN mode** | 0=S/PDIF, 1=ADAT |
     | bit 4 (`0x10`) | **Optical-OUT mode** | 0=S/PDIF, 1=ADAT |
     | bit 5 (`0x20`) | **S/MUX / ADAT 4-channel** | 0=normal, 1=S/MUX active |
     | bits 6..29 | always 0 in all captures | — |
   - Clone implementation (locked):
     ```
     if ((value & 0x40000000) == 0) return kIOReturnBadArgument;
     fMode = value;                                                     // store full composite
     SetClockSourceInternal(value);                                     // bits[1:0] → HSCI wire byte
     if (changed_bits & (0x10 | 0x20)) EngineFormatResolve();           // triggers SwitchDigitalSignals
     ```
     where `changed_bits = value ^ previous_value`. `EngineFormatResolve` re-runs the engine's
     `SetIOFormat` path which calls `SwitchDigitalSignals @ 0xe5dc` — that function reads the current
     channel count from `[+0x2e0]` and selects the peak-meter dispatch (6=S/PDIF, 8=ADAT8,
     12=ADAT+S/MUX, else=fallback). We do NOT need separate `SetOpticalOutMode(bit4)` /
     `SetADAT4Channel(bit5)` entry points — the engine format reshape IS the dispatch.
   - This also means NO separate `MASyncSource` / `MASPDIFInputPort` / `MADigitalInMode` /
     `MADigitalOutMode` / `MAADAT4Channel` property keys are published — all six logical fields
     multiplex through this one composite key.

  **C. `IOAudioStreamAvailableFormats` is dynamic — rebuild on (digital-mode × active-rate × S/MUX)
  change.**

   - Evidence: `captures/pulled/67a..c-ioreg-channels-*.txt` show the advertised format list differs
     across the three snapshots (same hardware, different mode/rate/S/MUX combos).
   - Clone implementation: after any `MAClockSource` write that touches bits 4 or 5, OR any `MASampleRate`
     write that lands, the engine must call `IOAudioStream::clearAvailableFormats()` + repopulate per the
     hardware topology:
     - 44.1/48k: S/PDIF→10ch (8 analog + 2 S/PDIF), ADAT→16ch (8 + 8)
     - 88.2/96k: S/PDIF→10ch, ADAT→12ch (8 + 4 S/MUX)
     - 176.4/192k: S/PDIF unsupported, ADAT→10ch (8 + 2 S/MUX ×4)
   - Bracket the rebuild with `beginConfigurationChange()` / `completeConfigurationChange()` so CoreAudio
     subscribers see a clean atomic transition.

  **D. Physical front-panel A/B button does NOT round-trip through setProperties.**

   - Evidence: `captures/pulled/64-dtrace-momentary-switch.txt` + `64b-dtrace-momentary-nopidfilter.txt`
     — zero setProperties writes on any process while the hardware button fired.
   - Clone implementation: the physical-button state change propagates kext→Panel via the Mach-port
     notification channel (`FWKIPCUserClient` in A1-Q14), NOT via a property write. The Panel UI reads
     the state from a notification body; it does not write anything back when the hardware fires.
   - Corollary: there is no `MADirectMonitoringActive` user-writable key. The DM state IS the physical
     button state when the button is in DM mode.