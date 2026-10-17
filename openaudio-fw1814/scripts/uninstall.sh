#!/bin/bash
# Removes everything the installer (installer/build-pkg.sh) or scripts/install.sh put on the machine. Run with sudo.
# The device icons are removed by name (they are the reference files); the folder only if it is then empty.
set -u
sudo launchctl unload /Library/LaunchDaemons/com.openaudio.fw1814.helper.plist 2>/dev/null
pkill -f "OpenAudio FW1814.app" 2>/dev/null
# the helper is already stopped; retry because kextunload fails (exit 3) while any client still releases the device
for try in 1 2 3 4; do sudo kextunload -b com.openaudio.OpenAudioFW1814 2>/dev/null && break || sleep 3; done
sudo rm -f /Library/LaunchDaemons/com.openaudio.fw1814.helper.plist /Library/PrivilegedHelperTools/com.openaudio.fw1814.helper
sudo rm -rf /Library/Extensions/OpenAudioFW1814.kext "/Applications/OpenAudio FW1814.app" "/Library/Audio/MIDI Drivers/MAudioFireWireMIDI.plugin"
D="/Library/Application Support/M-Audio/DeviceIcons"
for f in "FW 1814.png" "FW 410.png" "FW Audiophile.png" "Ozonic.tif" "ProFire Lightbridge.png" "ProjectMix.png"; do sudo rm -f "$D/$f"; done
sudo rmdir "$D" 2>/dev/null; sudo rmdir "/Library/Application Support/M-Audio" 2>/dev/null
sudo rm -rf "/Library/Application Support/OpenAudio"      # this script's installed copy (the uninstaller app runs its own bundled copy)
sudo killall MIDIServer 2>/dev/null
sudo kextcache -system-caches
sudo pkgutil --forget com.openaudio.fw1814.pkg >/dev/null 2>&1
echo uninstalled
