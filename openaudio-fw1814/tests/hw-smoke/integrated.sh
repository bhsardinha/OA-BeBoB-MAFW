#!/bin/bash
# IN.3 -- the single integrated hardware run (build PC, user present, no hot-plug, no flash). Airliner rule: every part exists before this runs;
# nothing is ever copied to /Library/Extensions (a reboot restores the installed M-FW kext). Each step prints PASS/FAIL against a criterion
# and waits for you; q aborts. Log: /tmp/oa-run-<time>.log (kernel lines are appended after every step).
# Usage (on the build PC, from ~/oa-build):  tests/hw-smoke/integrated.sh [--preflight] [--from N]
#   --preflight  run step 0 only (read-only checks: SIP, interface present, staged kext, built helper/panel)
#   --from N     start at step N (e.g. after you already loaded the kext)
# Steps: 0 preflight | 1 load from /tmp | 2 helper starts the firmware, device class publishes | 3 MA* properties | 4 panel connects |
#        5 meters + front-panel knob | 6 CoreAudio sees the device, tone out | 7 loopback capture | 8 rate sweep | 9 ADAT/S-PDIF switching |
#        10 MIDI | 11 sleep/wake | 12 unload
cd "$(dirname "$0")/../.." || exit 1
LOG=/tmp/oa-run-$(date +%Y%m%d-%H%M%S).log
KEXT=/tmp/OpenAudioFW1814.kext
BID=com.openaudio.OpenAudioFW1814
DEV=com_openaudio_FW1814Device
FROM=0; ONLY_PRE=0
while [ $# -gt 0 ]; do case "$1" in --preflight) ONLY_PRE=1;; --from) shift; FROM=$1;; *) echo "usage: $0 [--preflight] [--from N]"; exit 2;; esac; shift; done
say() { echo "$@" | tee -a "$LOG"; }
pass() { say "PASS  $*"; }
fail() { say "FAIL  $*"; FAILED=1; }
ask()  { read -r -p "$1 [enter = yes, n = no, q = abort] " a; [ "$a" = q ] && { say "aborted by user"; exit 1; }; [ "$a" != n ]; }
kernlog() { say "--- kernel lines ($1)"; log show --last 3m 2>/dev/null | grep -i "OpenAudioFW1814\|M-FW\|FW1814Device" | tail -25 | tee -a "$LOG"; }
prop() { ioreg -r -c $DEV -l 2>/dev/null | grep "\"$1\"" | head -1; }
waitfor() { local n=$1; shift; local i=0; while [ $i -lt "$n" ]; do "$@" && return 0; sleep 1; i=$((i + 1)); done; return 1; }
have_dev() { ioreg -r -c $DEV 2>/dev/null | grep -q "$DEV"; }
step() { N=$1; shift; [ "$N" -lt "$FROM" ] && return 1; say ""; say "=== STEP $N: $*"; return 0; }
FAILED=0

if step 0 "preflight (read-only)"; then
  csrutil status 2>&1 | tee -a "$LOG"
  if ioreg -p IOFireWire -l 2>/dev/null | grep -q "FW 1814"; then pass "FW1814 on the FireWire bus"; else fail "no FW 1814 on the FireWire bus (power the interface, never hot-plug)"; fi
  [ -d "$KEXT" ] && pass "staged kext $KEXT (re-stage with scripts/stage-kext.sh after every build)" || fail "run scripts/stage-kext.sh first"
  [ -x firmware-helper/build/oa-fwhelper ] && pass "helper built" || fail "helper not built (make -C firmware-helper)"
  [ -d "control-panel/build/OpenAudio FW1814.app" ] && pass "panel built" || fail "panel not built (make -C control-panel app)"
  kextstat 2>/dev/null | grep -i "avid\|m-audio\|maudio\|openaudio" | tee -a "$LOG" | sed 's/^/      loaded: /'
  say "      (Avid/M-Audio DICE kexts must not be loaded or installed: they panic on M-Audio FireWire devices)"
  [ "$ONLY_PRE" = 1 ] && { say "preflight done, failures=$FAILED"; exit $FAILED; }
  ask "Preflight looks right; continue?" || exit 1
fi

if step 1 "load the kext from /tmp (a reboot clears it)"; then
  sudo kextunload -b $BID >/dev/null 2>&1 && say "unloaded the previously installed copy"
  sudo kextutil "$KEXT" 2>&1 | tail -3 | tee -a "$LOG"
  kextstat 2>/dev/null | grep -q $BID && pass "kext loaded" || { fail "kext did not load"; kernlog step1; exit 1; }
  kernlog step1; ask "Kernel lines look sane (no panic, no errors)?" || exit 1
fi

if step 2 "helper writes MAFirmwareStart; $DEV publishes"; then
  firmware-helper/build/oa-fwhelper --once --no-gate 2>&1 | tail -5 | tee -a "$LOG"
  waitfor 30 have_dev && pass "$DEV is in the registry" || { fail "$DEV never appeared"; kernlog step2; ioreg -l -c com_openaudio_FWMetaNub 2>/dev/null | head -40 | tee -a "$LOG"; exit 1; }
  kernlog step2
fi

if step 3 "MA* properties"; then
  n=$(ioreg -r -c $DEV -l 2>/dev/null | grep -c '"MA')
  say "      $n MA* lines under the device"
  [ "$n" -ge 35 ] && pass "property surface published" || fail "only $n MA* properties (expected >= 35)"
  for k in MAClockSource MASampleRate MARateLocked MAFirmwareInfo MAInputLevels MAOutputLevels MARotaryControlV2 MAMomentarySwitchRoutings; do
    [ -n "$(prop $k)" ] && pass "$k" || fail "$k missing"
  done
  prop MAClockSource | tee -a "$LOG"; prop MASampleRate | tee -a "$LOG"; prop MARateLocked | tee -a "$LOG"
fi

if step 4 "panel connects"; then
  open -n "control-panel/build/OpenAudio FW1814.app"; sleep 3
  ask "Panel window shows the device (not the 'No FireWire 1814 found' window)?" && pass "panel found the device" || fail "panel did not find the device"
  ask "About tab: firmware date, hardware 'model 131 version 1' and GUID are right?" && pass "about tab" || fail "about tab"
  ask "Move a mixer fader: does the value stay and the property change (ioreg)?" && pass "property round trip" || fail "property round trip"
  kernlog step4
fi

if step 5 "meters and front-panel knob"; then
  ask "Feed a signal into input 1: do the input meters move with it?" && pass "input meters" || fail "input meters (HSCI poll / byte order)"
  ask "Turn the front Level knob: does the panel's output levels move?" && pass "front-panel knob" || fail "front-panel knob"
  ask "Press the front A/B switch: does the panel/LED follow?" && pass "A/B switch" || fail "A/B switch"
fi

if step 6 "CoreAudio sees the device; tone out"; then
  system_profiler SPAudioDataType 2>/dev/null | grep -i -B2 -A10 "1814" | tee -a "$LOG"
  ask "Audio MIDI Setup lists 'FireWire 1814 Multichannel' with the expected channels (18 in / 14 out at 48 kHz S/PDIF-ADAT per the panel)?" && pass "device visible" || fail "device not visible to CoreAudio"
  ask "Select it as output, route a mixer return to out 1/2, play a tone: clean, no clicks for 60 s?" && pass "tone out" || fail "tone out"
  kernlog step6
fi

if step 7 "loopback capture"; then
  ask "Cable out 1 to input 1 (or use the monitor mix). Record a tone in an app: clean, same level?" && pass "capture" || fail "capture"
fi

if step 8 "rate sweep (NO app may stream during a change)"; then
  for r in 44100 48000 88200 96000 176400 192000; do
    say "      set $r Hz in the panel (hardware tab stepper), wait for the lock icon"
    ask "Rate $r set and locked?" && { v=$(prop MASampleRate); say "      $v"; pass "rate $r"; } || fail "rate $r"
  done
  kernlog step8
fi

if step 9 "ADAT / S-PDIF switching"; then
  ask "Switch optical in/out ADAT <-> S/PDIF at 48 kHz in the panel: channel count changes, no panic?" && pass "optical switching" || fail "optical switching"
  ask "At 96 kHz with ADAT: ADAT 5-8 greyed, 4 ADAT channels in Audio MIDI Setup?" && pass "S/MUX layout" || fail "S/MUX layout"
  ask "At 192 kHz: 2 in / 4 out analog only?" && pass "192 kHz layout" || fail "192 kHz layout"
fi

if step 10 "MIDI"; then
  ask "Install the reference MIDI plug-in by hand (/Library/Audio/MIDI Drivers) and test: ports 'M-Audio FireWire 1814' appear, MIDI loopback works?" && pass "MIDI" || fail "MIDI"
fi

if step 11 "sleep / wake"; then
  ask "Run 'pmset sleepnow', wake the Mac: the interface is back and audio works?" && pass "sleep/wake" || fail "sleep/wake"
  kernlog step11
fi

if step 12 "unload"; then
  sudo kextunload -b $BID 2>&1 | tail -2 | tee -a "$LOG"
  kextstat 2>/dev/null | grep -q $BID && fail "kext still loaded" || pass "kext unloaded"
  kernlog step12
fi
say ""; say "DONE, failures=$FAILED  (log $LOG)"; exit $FAILED
