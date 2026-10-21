#!/bin/bash
# Run WITH sudo on a test Mac (the MacBook Air): swaps the installed kext for a test build staged next to this script, or restores the installed one.
#   sudo ~/fw1814-captures/load-test-kext.sh            unload the running kext, load ~/fw1814-captures/kext-test/OpenAudioBeBoBMAFW.kext from /tmp
#   sudo ~/fw1814-captures/load-test-kext.sh restore    unload it and load the installed /Library/Extensions copy
# The test copy lives in /tmp, so a reboot also returns to the installed kext. Quit audio apps first; the panel is closed by the script.
# Both generations are handled: whichever of 0.2.0 (OpenAudio BeBoB MAFW) or 0.1.0 (OpenAudio FW1814) is running is stopped,
# and "restore" brings back whichever generation is installed in /Library/Extensions.
set -u
HERE=$(cd "$(dirname "$0")" && pwd)
# 0.1.0 generation (OpenAudio FW1814): same com_openaudio_* C++ classes, so it can never be loaded together with this driver.
OLD_KEXT_ID=com.openaudio.OpenAudioFW1814; OLD_KEXT=/Library/Extensions/OpenAudioFW1814.kext
OLD_HELPER=com.openaudio.fw1814.helper; OLD_APP="/Applications/OpenAudio FW1814.app"
NEW_HELPER=com.openaudio.bebobmafw.helper
pkill -f "OA BeBoB MAFW.app" 2>/dev/null; pkill -f "OpenAudio BeBoB MAFW.app" 2>/dev/null; pkill -f "$OLD_APP" 2>/dev/null
launchctl unload /Library/LaunchDaemons/$NEW_HELPER.plist 2>/dev/null; launchctl unload /Library/LaunchDaemons/$OLD_HELPER.plist 2>/dev/null
for t in 1 2 3 4 5 6; do kextunload -b com.openaudio.OpenAudioBeBoBMAFW 2>/dev/null; kextunload -b $OLD_KEXT_ID 2>/dev/null
  kextstat | grep -q -e com.openaudio.OpenAudioBeBoBMAFW -e $OLD_KEXT_ID || break; sleep 3; done
restore_installed() {
  if [ -d /Library/Extensions/OpenAudioBeBoBMAFW.kext ]; then kextload /Library/Extensions/OpenAudioBeBoBMAFW.kext && echo "installed 0.2.0 kext loaded"; RESTORE_HELPER=$NEW_HELPER
  elif [ -d $OLD_KEXT ]; then kextload $OLD_KEXT && echo "installed 0.1.0 kext loaded"; RESTORE_HELPER=$OLD_HELPER
  else echo "no installed kext"; RESTORE_HELPER=; fi
}
if [ "${1:-}" = restore ]; then
  restore_installed
else
  rm -rf /tmp/OpenAudioBeBoBMAFW.kext; cp -R "$HERE/kext-test/OpenAudioBeBoBMAFW.kext" /tmp/
  chown -R root:wheel /tmp/OpenAudioBeBoBMAFW.kext; chmod -R go-w /tmp/OpenAudioBeBoBMAFW.kext
  kextutil -t -n /tmp/OpenAudioBeBoBMAFW.kext >/dev/null 2>&1 || { echo "static check failed; nothing loaded"; restore_installed; [ -n "$RESTORE_HELPER" ] && launchctl load /Library/LaunchDaemons/$RESTORE_HELPER.plist; exit 1; }
  kextutil /tmp/OpenAudioBeBoBMAFW.kext 2>&1 | tail -1
  RESTORE_HELPER=$NEW_HELPER
  [ -f /Library/LaunchDaemons/$NEW_HELPER.plist ] || RESTORE_HELPER=   # the 0.2.0 helper is not installed yet: run it by hand if needed
fi
sleep 8; [ -n "$RESTORE_HELPER" ] && launchctl load /Library/LaunchDaemons/$RESTORE_HELPER.plist
sleep 25; kextstat | grep -i openaudio | cut -c1-110
