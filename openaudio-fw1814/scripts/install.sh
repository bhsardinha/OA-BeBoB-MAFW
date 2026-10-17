#!/bin/bash
# Install kext + helper daemon on the build PC. ONLY after scripts/stage-kext.sh succeeded (kextutil -t -n on a
# /tmp copy): a bad kext in /Library/Extensions causes an autoload boot loop (feedback_stage_kext_before_autoload).
# Usage: scripts/install.sh   (run from the repo copy that holds kext/build and firmware-helper/build)
set -euo pipefail
cd "$(dirname "$0")/.."
KEXT=kext/build/OpenAudioFW1814.kext
HELPER=firmware-helper/build/oa-fwhelper
PLIST=firmware-helper/com.openaudio.fw1814.helper.plist
[ -d "$KEXT" ] && [ -x "$HELPER" ] || { echo "build first"; exit 1; }
[ -d /tmp/OpenAudioFW1814.kext ] || { echo "run scripts/stage-kext.sh first"; exit 1; }
# Stop the helper FIRST (2026-10-17): it holds a user client on the device until it sees the device terminate, and kextunload refuses
# (exit 3, kext in use) while that reference exists. The retry covers the short window in which another client is still releasing.
sudo launchctl unload /Library/LaunchDaemons/com.openaudio.fw1814.helper.plist 2>/dev/null || true
for try in 1 2 3 4; do sudo kextunload -b com.openaudio.OpenAudioFW1814 2>/dev/null && break || sleep 3; done
sudo rm -rf /Library/Extensions/OpenAudioFW1814.kext
sudo cp -R "$KEXT" /Library/Extensions/OpenAudioFW1814.kext
sudo chown -R root:wheel /Library/Extensions/OpenAudioFW1814.kext
sudo chmod -R go-w /Library/Extensions/OpenAudioFW1814.kext
sudo mkdir -p /Library/PrivilegedHelperTools
sudo cp "$HELPER" /Library/PrivilegedHelperTools/com.openaudio.fw1814.helper
sudo chown root:wheel /Library/PrivilegedHelperTools/com.openaudio.fw1814.helper
sudo chmod 755 /Library/PrivilegedHelperTools/com.openaudio.fw1814.helper
sudo cp "$PLIST" /Library/LaunchDaemons/com.openaudio.fw1814.helper.plist
sudo chown root:wheel /Library/LaunchDaemons/com.openaudio.fw1814.helper.plist
sudo chmod 644 /Library/LaunchDaemons/com.openaudio.fw1814.helper.plist
sudo kextcache -system-caches
echo "installed. Reboot to test autoload (kext loads on first matching device; the helper starts at boot)."
