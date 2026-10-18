#!/bin/bash
# Run WITH sudo on a test Mac (the MacBook Air): swaps the installed kext for a test build staged next to this script, or restores the installed one.
#   sudo ~/fw1814-captures/load-test-kext.sh            unload the running kext, load ~/fw1814-captures/kext-test/OpenAudioFW1814.kext from /tmp
#   sudo ~/fw1814-captures/load-test-kext.sh restore    unload it and load the installed /Library/Extensions copy
# The test copy lives in /tmp, so a reboot also returns to the installed kext. Quit audio apps first; the panel is closed by the script.
set -u
HERE=$(cd "$(dirname "$0")" && pwd)
pkill -f "OpenAudio FW1814.app" 2>/dev/null
launchctl unload /Library/LaunchDaemons/com.openaudio.fw1814.helper.plist 2>/dev/null
for t in 1 2 3 4 5 6; do kextunload -b com.openaudio.OpenAudioFW1814 2>/dev/null && break || sleep 3; done
if [ "${1:-}" = restore ]; then
  kextload /Library/Extensions/OpenAudioFW1814.kext && echo "installed kext loaded"
else
  rm -rf /tmp/OpenAudioFW1814.kext; cp -R "$HERE/kext-test/OpenAudioFW1814.kext" /tmp/
  chown -R root:wheel /tmp/OpenAudioFW1814.kext; chmod -R go-w /tmp/OpenAudioFW1814.kext
  kextutil -t -n /tmp/OpenAudioFW1814.kext >/dev/null 2>&1 || { echo "static check failed; nothing loaded"; launchctl load /Library/LaunchDaemons/com.openaudio.fw1814.helper.plist; kextload /Library/Extensions/OpenAudioFW1814.kext; exit 1; }
  kextutil /tmp/OpenAudioFW1814.kext 2>&1 | tail -1
fi
sleep 8; launchctl load /Library/LaunchDaemons/com.openaudio.fw1814.helper.plist
sleep 25; kextstat | grep openaudio | cut -c1-100
