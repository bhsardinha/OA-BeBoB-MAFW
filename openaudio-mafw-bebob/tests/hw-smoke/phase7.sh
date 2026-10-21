#!/bin/bash
# Plan v6 phase 7 -- the hardware run on the iMac (build PC), user present. Claude runs one step at a time over SSH and asks in chat between steps.
# Rules (CLAUDE.md, plan v6 §phase 7): nothing goes to /Library/Extensions (the test kext lives in /tmp; a reboot returns to the installed 0.1.0);
# no hot-plug (cabling only with the units off, power-cycle instead); no flash writes; rate/mode changes, loads and unloads only when the user
# said yes and nothing streams. Every step that changes state refuses to run without --yes.
#
# Usage (on the build PC, from ~/oa-build-mafw):  tests/hw-smoke/phase7.sh <step> [--yes]
#   steps, in session order:
#     pre        read-only: SIP, bus units (name, GUID, Model_ID), what is attached to them (incl. AppleFWAudio), loaded kexts, streaming state
#     stage      build kext/helper/panel/tool/bench tools, stage the kext in /tmp + kextutil -t -n (loads nothing)
#     watch      start the background recorders: FireWire bus nodes (bootloader -> runtime, Model_IDs) and MADebugTrace of every unit
#     load       [--yes] stop the 0.1.0 helper/panel, unload 0.1.0, load the /tmp kext                                   AP-7.1
#     helper     [--yes] start the 0.2.0 helper (normal, gated) in the background; its log goes to the run folder        AP-4.2
#     bringup    read-only: device classes, MAFirmwareInfo, plug-info lines (channel order), AppleFWAudio check     AP-3.10, AP-3.3
#     rates      [--yes] Audiophile through 44.1/48/88.2/96 and back to 48 with CoreAudio; channel counts per rate    AP-7.2, AP-7.5
#     ac3        read-only: 'cac3' formats on the Audiophile's output stream                                           AP-3.8
#     factory    [--yes] dump -> MAMetaResetToFactorySettings -> dump -> diff with capture 92 -> write the first dump back AP-3.6
#                (phones go to the factory 0 dB for about a second: headphones off the unit)
#     loop       [--yes] loopcheck + looplat on the Audiophile (cable Out 3 -> In 1, input set to line)               AP-7.4
#     replay     [--yes] persistence: set one level, wait for the save, unload/reload, check it came back, restore    AP-4.3
#     unload5    [--yes] kextunload + reload five times (helper stopped, panel closed)                                   AP-3.9
#     midi       read-only: CoreMIDI devices/ports (the unmodified plug-in finds the Audiophile)                        AP-6.1
#     fwtool     read-only: panel --firmware-list (both units "Current"; was the Firmware Tool's --list)                                                AP-7.9
#     soak       [--yes] duplex client for N seconds (default 300) on one unit                                          AP-7.8
#     sleep      [--yes] pmset sleepnow (the user wakes the Mac), then bringup again                                    AP-7.8
#     restore    [--yes] stop helper and recorders, unload the test kext, load the installed 0.1.0 + its helper
#     status     read-only: short state summary (kexts, units, engines, recorders)
# Manual checklists in chat (no script step): every panel control (AP-7.6), front panel (AP-7.7), device menu with one / both units (AP-5.3),
# bus reset by power-cycle (AP-7.8: run `watch`, the user power-cycles, then `bringup`).
# Environment: UNIT=FW1814 selects the 1814 for rates/loop/soak/replay (default: the Audiophile). RUN=<dir> (default ~/oa-runs/phase7).
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$(pwd)
RUN=${RUN:-$HOME/oa-runs/phase7}; mkdir -p "$RUN"
LOG="$RUN/session.log"
KEXT=/tmp/OpenAudioBeBoBMAFW.kext
BID=com.openaudio.OpenAudioBeBoBMAFW
OLD_BID=com.openaudio.OpenAudioFW1814; OLD_KEXT=/Library/Extensions/OpenAudioFW1814.kext
OLD_HELPER=com.openaudio.fw1814.helper
HELPER_BIN="$ROOT/firmware-helper/build/oa-fwhelper"
T="$ROOT/tools/build"                              # bench tools are built here (tools/ itself is rsync --delete'd)
STEP=${1:-help}; YES=0; [ "${2:-}" = --yes ] && YES=1
case "${UNIT:-Audiophile}" in FW1814|1814) CLS=com_openaudio_FW1814Device; PROD="FW 1814";; *) CLS=com_openaudio_FWAudiophileDevice; PROD="FW Audiophile";; esac

say()  { echo "$@" | tee -a "$LOG"; }
hdr()  { say ""; say "=== $(date '+%H:%M:%S') $STEP: $*"; }
need_yes() { [ "$YES" = 1 ] || { echo "'$STEP' changes state: ask the user first, then run with --yes"; exit 2; }; }
klog() { say "--- kernel (last lines)"; sudo dmesg 2>/dev/null | grep -a -o -E "(OpenAudioBeBoBMAFW|M-FW|panic)[^\n]*" | sed -E 's/Sandbox:.*//' | tail -${1:-15} | tee -a "$LOG"; }   # dmesg glues sandbox lines onto ours
U="/usr/bin/python $ROOT/tests/hw-smoke/p7util.py"
units() { $U units; }                               # FireWire bus nodes: "<product> GUID=<hex> Model_ID=<hex>"
guid_of() { units | grep "^$1 GUID" | head -1 | sed 's/.*GUID=\([0-9a-f]*\).*/\1/'; }
engines() { ioreg -r -c com_openaudio_FWEngine2 -l -w0 2>/dev/null | grep -E '"IOAudioEngineGlobalUniqueID"|"IOAudioEngineState"|"IOAudioEngineNumActiveUserClients"' | sed 's/^ *| *//'; }
busy() { engines | grep -q '"IOAudioEngineState" = 1'; }
idle_or_die() { if busy; then say "an engine is running (a client streams): stop Studio One / reAMP / AMS I/O first"; engines | tee -a "$LOG"; exit 3; fi; }
dev_up() { ioreg -r -c "$1" -d 1 2>/dev/null | grep -q "<class "; }   # -c also matches subclasses (the printed class is the concrete one)
waitdev() { local i=0; while [ $i -lt "${2:-40}" ]; do dev_up "$1" && return 0; sleep 1; i=$((i + 1)); done; return 1; }
oadev() { local g; g=$(guid_of "$PROD"); [ -n "$g" ] && echo "$g"; }
stop_helper() { sudo pkill -f "$HELPER_BIN" 2>/dev/null; sudo launchctl unload /Library/LaunchDaemons/$OLD_HELPER.plist 2>/dev/null; true; }
close_panels() { pkill -f "OA BeBoB MAFW.app" 2>/dev/null; pkill -f "OpenAudio BeBoB MAFW.app" 2>/dev/null; pkill -f "OpenAudio FW1814.app" 2>/dev/null; true; }

case "$STEP" in
pre)
  hdr "read-only preflight"
  csrutil status 2>&1 | tee -a "$LOG"
  say "--- FireWire units (power state as found)"; units | tee -a "$LOG"
  say "--- drivers attached under each unit"
  ioreg -r -c IOFireWireDevice -w0 2>/dev/null | grep -E "<class (IOFireWireDevice|IOFireWireUnit|IOFireWireAVC|com_openaudio|com_m_audio|AppleFWAudio)" | sed -E 's/, id 0x[0-9a-f]+.*//' | tee -a "$LOG"
  if ioreg -r -c AppleFWAudioDevice 2>/dev/null | grep -q AppleFWAudioDevice; then say "NOTE: AppleFWAudioDevice is attached to a unit (no go-away personality covers it): report before loading"; fi
  say "--- loaded kexts"; kextstat 2>/dev/null | grep -i -E "openaudio|m-audio|maudio|avid|dice" | cut -c1-110 | tee -a "$LOG"
  say "--- engines"; engines | tee -a "$LOG"
  busy && say "STREAMING: an engine is running" || say "idle: no engine running"
  [ -d "$KEXT" ] && say "staged kext present: $KEXT" || say "no staged kext yet (run: stage)"
  say "run folder: $RUN" ;;
stage)
  hdr "build + stage (loads nothing)"
  make -C kext 2>&1 | grep -E "error|warning: .*(FW|OA|oa)" | tee -a "$LOG"
  make -C firmware-helper 2>&1 | grep -E "error|warning" | tee -a "$LOG"
  make -C control-panel app 2>&1 | grep -E "error:" | tee -a "$LOG"
  mkdir -p "$T"
  for t in maprop halinfo ratecycle loopcheck looplat aduplex midiprobe halwatch; do
    cc -w -mmacosx-version-min=10.13 -o "$T/$t" "tools/$t.c" -framework CoreAudio -framework CoreFoundation -framework IOKit -framework CoreMIDI 2>&1 | tee -a "$LOG"
  done
  ls "$T" | tr '\n' ' ' | tee -a "$LOG"; say ""
  scripts/stage-kext.sh 2>&1 | tail -3 | tee -a "$LOG"
  say "kext UUID: $(dwarfdump --uuid "$KEXT/Contents/MacOS/OpenAudioBeBoBMAFW" 2>/dev/null | awk '{print $2}')" ;;
watch)
  hdr "background recorders"
  pkill -f "p7util.py (bus|trace)-watch" 2>/dev/null
  nohup $U bus-watch >> "$RUN/bus.log" 2>&1 &         # bus nodes, changes only, 0.5 s (the bootloader node lives ~3 s after power-on)
  nohup $U trace-watch >> "$RUN/trace.log" 2>&1 &     # every new MADebugTrace line (the ring holds 16; plug-info lines come at attach)
  sleep 2; pgrep -fl "p7util.py" | tee -a "$LOG"; say "logs: $RUN/bus.log, $RUN/trace.log" ;;
load)
  need_yes; hdr "load the test kext (AP-7.1)"; idle_or_die
  [ -d "$KEXT" ] || { say "no staged kext: run stage"; exit 1; }
  close_panels; stop_helper
  for i in 1 2 3 4 5 6; do sudo kextunload -b $OLD_BID 2>/dev/null; sudo kextunload -b $BID 2>/dev/null
    kextstat | grep -q -e $OLD_BID -e $BID || break; sleep 3; done
  kextstat | grep -q -e $OLD_BID -e $BID && { say "could not unload the running kext"; exit 1; }
  sudo kextutil "$KEXT" 2>&1 | tail -2 | tee -a "$LOG"
  kextstat | grep -i openaudio | cut -c1-110 | tee -a "$LOG"
  say "--- nubs"; ioreg -r -c com_openaudio_FWMetaNub -w0 | grep "<class" | tee -a "$LOG"
  klog ;;
helper)
  need_yes; hdr "0.2.0 helper in the background (AP-4.2)"
  sudo pkill -f "$HELPER_BIN" 2>/dev/null
  sudo nohup "$HELPER_BIN" >> "$RUN/helper.log" 2>&1 &
  sleep 6; tail -20 "$RUN/helper.log" | tee -a "$LOG"
  for c in com_openaudio_FW1814Device com_openaudio_FWAudiophileDevice; do waitdev $c 30 && say "$c up" || say "$c not present"; done ;;
bringup)
  hdr "bring-up readout (AP-3.10, AP-3.3)"
  units | tee -a "$LOG"
  for c in com_openaudio_FW1814Device com_openaudio_FWAudiophileDevice; do
    dev_up $c || { say "$c: not present"; continue; }
    say "--- $c"
    ioreg -r -c $c -l -w0 | grep -E '"(IOAudioDeviceName|MAActualProductName|GUID|MASampleRate|MARateLocked|MAClockSource|MAFirmwareInfo)"' | sed 's/^ *| *//' | tee -a "$LOG"
    say "    MADebugTrace now:"; ioreg -r -c $c -l -w0 | grep '"MADebugTrace"' | sed 's/^ *| *//' | tr ',' '\n' | sed 's/^/      /' | tee -a "$LOG"
  done
  say "--- plug-info lines recorded since load (channel order; all of them, from trace.log)"; grep "plug info" "$RUN/trace.log" 2>/dev/null | tee -a "$LOG"
  say "--- engines"; engines | tee -a "$LOG"
  ioreg -r -c AppleFWAudioDevice 2>/dev/null | grep -q AppleFWAudioDevice && say "WARNING: AppleFWAudioDevice attached" || say "AppleFWAudio: not attached"
  tail -8 "$RUN/helper.log" 2>/dev/null | tee -a "$LOG"; klog 20 ;;
rates)
  need_yes; hdr "rates on $PROD (AP-7.2/7.5)"; idle_or_die
  g=$(oadev) || { say "$PROD not on the bus"; exit 1; }
  if [ "$PROD" = "FW 1814" ]; then R="44100 48000 88200 96000 176400 192000 48000"; else R="44100 48000 88200 96000 48000"; fi
  OA_DEV=$g "$T/ratecycle" $R 2>&1 | tee -a "$LOG"
  OA_DEV=$g "$T/halinfo" 2>&1 | tee -a "$LOG"
  say "MARateLocked: $(MA_CLASS=$CLS "$T/maprop" get MARateLocked)"; klog ;;
ac3)
  hdr "AC-3 formats on $PROD (AP-3.8)"
  n=$(ioreg -r -c $CLS -l -w0 | grep -o '"IOAudioStreamSampleFormat"=1667326771' | wc -l | tr -d ' ')
  say "'cac3' format entries under $CLS: $n (expected: 3 rates, 44.1/48/96, on the output stream)"
  ioreg -r -c $CLS -l -w0 | grep -o '{[^{}]*"IOAudioStreamSampleFormat"=1667326771[^{}]*}' | tee -a "$LOG" ;;
factory)
  need_yes; hdr "factory reset vs capture 92 (AP-3.6)"; idle_or_die
  [ "$PROD" = "FW Audiophile" ] || { say "capture 92 is the Audiophile's reference; run with the default UNIT"; exit 1; }
  REF="$ROOT/../docs/system-map/B5-sl-dtrace-truth/captures/92-sl-audiophile-controls-rates-reset/reset/device-settings-after-reset.plist"
  [ -f "$REF" ] || REF="$HOME/oa-ref/device-settings-after-reset.plist"
  [ -f "$REF" ] || { say "reference plist missing: copy capture 92's reset/device-settings-after-reset.plist to ~/oa-ref/"; exit 1; }
  MA_CLASS=$CLS "$T/maprop" allxml > "$RUN/factory-before.plist" || exit 1
  # the factory phones level is 0 dB (loud on 32-ohm headphones): dump at once, then the previous settings go back below
  MA_CLASS=$CLS "$T/maprop" set MAMetaResetToFactorySettings 1 | tee -a "$LOG"
  MA_CLASS=$CLS "$T/maprop" allxml > "$RUN/factory-after.plist"
  plutil -convert xml1 -o "$RUN/factory-ref.plist" "$REF"           # capture 92 is a binary plist; 10.13's Python 2.7 plistlib reads XML only
  python "$ROOT/tools/factory-diff.py" "$RUN/factory-after.plist" "$RUN/factory-ref.plist" D6C0300211D54 2>&1 | tee -a "$LOG"
  say "--- writing the unit's previous settings back"
  MA_CLASS=$CLS "$T/maprop" setxml "$RUN/factory-before.plist" | tee -a "$LOG"
  sleep 2; MA_CLASS=$CLS "$T/maprop" allxml > "$RUN/factory-restored.plist"
  cmp -s "$RUN/factory-before.plist" "$RUN/factory-restored.plist" && say "settings restored (identical dump)" || { say "restored dump differs:"; diff "$RUN/factory-before.plist" "$RUN/factory-restored.plist" | head -20 | tee -a "$LOG"; }
  klog ;;
loop)
  need_yes; hdr "loopback on $PROD (AP-7.4): Out 3 -> In 1, input on line"; idle_or_die
  g=$(oadev) || { say "$PROD not on the bus"; exit 1; }
  OA_DEV=$g "$T/loopcheck" 30 3 1 2>&1 | tail -15 | tee -a "$LOG"
  OA_DEV=$g "$T/looplat" 48000 64 8 2>&1 | tail -6 | tee -a "$LOG"
  OA_DEV=$g "$T/looplat" 48000 512 8 2>&1 | tail -6 | tee -a "$LOG"; klog ;;
replay)
  need_yes; hdr "per-GUID persistence on $PROD (AP-4.3)"; idle_or_die
  pgrep -f "$HELPER_BIN" >/dev/null || { say "helper not running: run the helper step"; exit 1; }
  MA_CLASS=$CLS "$T/maprop" allxml > "$RUN/replay-before.plist" || exit 1
  say "before: $($U getkey "$RUN/replay-before.plist" MAOutputLevels)"
  # one distinctive value: output pair 1 at -20 dB (levels are 0.5 dB steps); the rest of the array unchanged
  $U mkset "$RUN/replay-before.plist" MAOutputLevels "$RUN/replay-set.plist" 0=-40 1=-40 | tee -a "$LOG"
  MA_CLASS=$CLS "$T/maprop" setxml "$RUN/replay-set.plist" | tee -a "$LOG"
  sleep 6; say "saved for the console user ($(oadev)):"; defaults read com.m-audio.device-settings.firewire 2>/dev/null | grep -i -A8 "$(oadev)" | grep -A7 MAOutputLevels | head -8 | tee -a "$LOG"
  close_panels; sudo pkill -f "$HELPER_BIN"; sleep 1
  sudo kextunload -b $BID 2>&1 | tail -1 | tee -a "$LOG"; sleep 2; sudo kextutil "$KEXT" 2>&1 | tail -1 | tee -a "$LOG"
  sudo nohup "$HELPER_BIN" >> "$RUN/helper.log" 2>&1 &
  waitdev $CLS 40 || { say "$CLS did not come back"; klog; exit 1; }
  sleep 6; MA_CLASS=$CLS "$T/maprop" allxml > "$RUN/replay-after.plist"; say "after reload: $($U getkey "$RUN/replay-after.plist" MAOutputLevels)"
  tail -5 "$RUN/helper.log" | tee -a "$LOG"
  MA_CLASS=$CLS "$T/maprop" setxml "$RUN/replay-before.plist" | tee -a "$LOG"; sleep 6
  MA_CLASS=$CLS "$T/maprop" allxml > "$RUN/replay-restored.plist"; say "restored: $($U getkey "$RUN/replay-restored.plist" MAOutputLevels)"; klog ;;
unload5)
  need_yes; hdr "kextunload x5 (AP-3.9)"; idle_or_die
  close_panels; sudo pkill -f "$HELPER_BIN" 2>/dev/null
  for i in 1 2 3 4 5; do
    sudo kextunload -b $BID 2>&1 | tail -1 | tee -a "$LOG"
    kextstat | grep -q $BID && { say "round $i: still loaded"; klog; exit 1; }
    sleep 2; sudo kextutil "$KEXT" 2>&1 | tail -1 | tee -a "$LOG"
    sudo "$HELPER_BIN" --once >> "$RUN/helper.log" 2>&1
    waitdev com_openaudio_FWAVCAudioDevice 40 && say "round $i: unload + reload OK" || { say "round $i: device did not come back"; klog; exit 1; }
    sleep 5
  done
  sudo nohup "$HELPER_BIN" >> "$RUN/helper.log" 2>&1 &
  klog 25 ;;
midi)
  hdr "MIDI (AP-6.1)"
  "$T/midiprobe" list 2>&1 | tee -a "$LOG"
  ls "/Library/Audio/MIDI Drivers" 2>/dev/null | tee -a "$LOG"
  for c in com_openaudio_FW1814Device com_openaudio_FWAudiophileDevice; do dev_up $c && ioreg -r -c $c -l -w0 | grep -E '"MAMIDI' | sed "s/^ *| */$c /" | tee -a "$LOG"; done ;;
fwtool)
  hdr "Firmware Tool (AP-7.9)"
  "$ROOT/control-panel/build/OA BeBoB MAFW.app/Contents/MacOS/OA BeBoB MAFW" --firmware-list 2>&1 | tee -a "$LOG" ;;
soak)
  need_yes; hdr "soak on $PROD, ${SECS:-300} s (AP-7.8)"; idle_or_die
  g=$(oadev) || { say "$PROD not on the bus"; exit 1; }
  OA_DEV=$g "$T/aduplex" "${SECS:-300}" > "$RUN/soak-$(date +%H%M%S).log" 2>&1; tail -5 "$RUN"/soak-*.log | tee -a "$LOG"; klog ;;
sleep)
  need_yes; hdr "sleep now; wake the Mac with a key, then run bringup"; idle_or_die
  sudo pmset sleepnow | tee -a "$LOG" ;;
restore)
  need_yes; hdr "back to the installed 0.1.0"
  close_panels; sudo pkill -f "$HELPER_BIN" 2>/dev/null; pkill -f "p7util.py" 2>/dev/null
  for i in 1 2 3 4 5 6; do sudo kextunload -b $BID 2>/dev/null; kextstat | grep -q $BID || break; sleep 3; done
  kextstat | grep -q $BID && { say "test kext still loaded"; exit 1; }
  [ -d $OLD_KEXT ] && sudo kextload $OLD_KEXT 2>&1 | tail -1 | tee -a "$LOG"
  sleep 8; sudo launchctl load /Library/LaunchDaemons/$OLD_HELPER.plist 2>/dev/null
  sleep 15; kextstat | grep -i openaudio | cut -c1-110 | tee -a "$LOG"; units | tee -a "$LOG"; klog ;;
status)
  hdr "status"
  kextstat | grep -i openaudio | cut -c1-110; units; engines
  pgrep -fl "p7util.py|oa-fwhelper" ;;
*) sed -n '2,32p' "$0"; exit 2 ;;
esac
