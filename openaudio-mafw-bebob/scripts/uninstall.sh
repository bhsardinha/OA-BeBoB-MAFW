#!/bin/bash
# Removes every file that any OpenAudio release has installed (this generation and the 0.1.0 "OpenAudio FW1814" generation). Run with sudo.
#   uninstall.sh            full uninstall: also the launch-at-login item; refreshes the kext caches
#   uninstall.sh --update   run by the package's preinstall before an install or update (as root): removes the old files of every release so the
#                           new package installs into a clean system, and keeps the user's settings (helper settings per unit, panel preferences,
#                           launch at login). Settings live in the user's preferences and are never touched by either mode.
# Rule (user, 2026-10-21, update method): this file is the one list of everything ever installed. When a release renames or stops shipping a
# path, the old path STAYS in the lists below, so any later package cleans any earlier install, however much the layout changed.
# The running kext is NOT unloaded (R117, capture 97: unloading with a unit attached can panic at the next bus reset): its files go, and it
# stays in memory until the Mac restarts (the package asks for a restart).
set -u
MODE=${1:-}
SUDO=; [ "$(id -u)" = 0 ] || SUDO=sudo

# --- everything any release has installed (keep old entries) ---
HELPER_LABELS=(com.openaudio.bebobmafw.helper        # 0.2.x
               com.openaudio.fw1814.helper)          # 0.1.0
KEXTS=(/Library/Extensions/OpenAudioBeBoBMAFW.kext   # 0.2.x
       /Library/Extensions/OpenAudioFW1814.kext)     # 0.1.0
APPS=("/Applications/OA BeBoB MAFW.app"                                                     # 0.2.2- (firmware check inside)
      "/Applications/OpenAudio BeBoB MAFW.app" "/Applications/OpenAudio Firmware Tool.app"    # 0.2.0, 0.2.1
      "/Applications/OpenAudio FW1814.app")                                                   # 0.1.0
OTHER_FILES=("/Library/Audio/MIDI Drivers/MAudioFireWireMIDI.plugin"                          # M-Audio plug-in (every release)
             "/Library/Application Support/OpenAudio")                                         # installed copy of this script
RECEIPTS=(com.openaudio.bebobmafw.pkg com.openaudio.fw1814.pkg)
ICON_DIR="/Library/Application Support/M-Audio/DeviceIcons"
ICONS=("FW 1814.png" "FW 410.png" "FW Audiophile.png" "Ozonic.tif" "ProFire Lightbridge.png" "ProjectMix.png")
LOGIN_AGENT=com.openaudio.bebobmafw.panel.plist      # panel's launch at login (plan v8 AP-5.5), in the console user's LaunchAgents

for l in "${HELPER_LABELS[@]}"; do
  $SUDO launchctl unload "/Library/LaunchDaemons/$l.plist" 2>/dev/null
  $SUDO rm -f "/Library/LaunchDaemons/$l.plist" "/Library/PrivilegedHelperTools/$l"
done
# quit the apps by their full executable path: a bare "OA BeBoB MAFW.app" would also match "Uninstall OA BeBoB MAFW.app", the app running this
for a in "${APPS[@]}"; do pkill -f "$a/Contents/MacOS/" 2>/dev/null; done
for k in "${KEXTS[@]}"; do $SUDO rm -rf "$k"; done
for a in "${APPS[@]}" "${OTHER_FILES[@]}"; do $SUDO rm -rf "$a"; done
for f in "${ICONS[@]}"; do $SUDO rm -f "$ICON_DIR/$f"; done                    # the icons are the reference files: removed by name
$SUDO rmdir "$ICON_DIR" 2>/dev/null; $SUDO rmdir "/Library/Application Support/M-Audio" 2>/dev/null
for r in "${RECEIPTS[@]}"; do $SUDO pkgutil --forget "$r" >/dev/null 2>&1; done
$SUDO killall MIDIServer 2>/dev/null

if [ "$MODE" = "--update" ]; then
  echo "old OpenAudio files removed; settings kept"
  exit 0                                    # the package installs the new files, refreshes the caches and asks for a restart
fi
CU=$(stat -f %Su /dev/console 2>/dev/null); [ -n "$CU" ] && $SUDO rm -f "/Users/$CU/Library/LaunchAgents/$LOGIN_AGENT"
if [ "$(sw_vers -productVersion | cut -d. -f1)" -ge 11 ]; then
  # macOS 11+ (Modern edition): the kext stays in the auxiliary kernel collection until it is rebuilt without it (man kmutil, "To uninstall a kext
  # from the Auxiliary kext collection": delete the bundle, kmutil rebuild, confirm, authorize, reboot). kmutil asks the confirmation on stdin.
  # **Unverified** (R128, 2026-10-21): that "y" is the answer it takes, and whether macOS also asks for an authorization in System Settings.
  printf 'y\n' | $SUDO /usr/bin/kmutil rebuild >/tmp/openaudio-kmutil-rebuild.log 2>&1
else
  $SUDO kextcache -system-caches
fi
echo "uninstalled; restart the Mac to remove the driver from memory"
