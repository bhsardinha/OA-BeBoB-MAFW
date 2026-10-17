#!/bin/bash
# IN.5 - builds OpenAudioFW1814-<version>.pkg ON THE BUILD PC (10.13.6, Xcode 9.4.1: the only machine whose SDK builds the kext and the panel).
# Run via installer/make-pkg.sh from the dev Mac, or directly in ~/oa-build/installer once ~/oa-build/installer/assets holds the reference MIDI
# plug-in and the device icons (make-pkg.sh copies them from _extracted/).
# Payload (all root:wheel):
#   /Library/Extensions/OpenAudioFW1814.kext                         kext (unsigned: IN.6 needs the user's Developer ID)
#   /Library/PrivilegedHelperTools/com.openaudio.fw1814.helper       helper daemon (firmware start, settings persistence)
#   /Library/LaunchDaemons/com.openaudio.fw1814.helper.plist
#   /Applications/OpenAudio FW1814.app                               control panel
#   /Library/Audio/MIDI Drivers/MAudioFireWireMIDI.plugin            the 2014 reference CoreMIDI plug-in, unmodified (M-H2; Q-MH plug-in decision)
#   /Library/Application Support/OpenAudio/uninstall.sh              removes all of the above
#   /Library/Application Support/M-Audio/DeviceIcons/*               device icons the plug-in looks up by MAIconName (A3-M2 section 3)
# Scripts: preinstall (stop helper, unload old kext), postinstall (kextutil -t -n gate, caches, helper, MIDIServer). Restart recommended.
set -euo pipefail
cd "$(dirname "$0")/.."
VERSION=${VERSION:-$(/usr/libexec/PlistBuddy -c 'Print :CFBundleShortVersionString' kext/Info.plist 2>/dev/null || echo 0.1.0)}
OUT=installer/build
ROOT=$OUT/root
rm -rf "$OUT"; mkdir -p "$ROOT"

echo "== build kext, helper, panel"
make -C kext >/dev/null
make -C firmware-helper >/dev/null
make -C control-panel app >/dev/null

echo "== stage payload"
install -d "$ROOT/Library/Extensions" "$ROOT/Library/PrivilegedHelperTools" "$ROOT/Library/LaunchDaemons" "$ROOT/Applications" \
           "$ROOT/Library/Audio/MIDI Drivers" "$ROOT/Library/Application Support/M-Audio"
cp -R kext/build/OpenAudioFW1814.kext "$ROOT/Library/Extensions/"
cp firmware-helper/build/oa-fwhelper "$ROOT/Library/PrivilegedHelperTools/com.openaudio.fw1814.helper"
cp firmware-helper/com.openaudio.fw1814.helper.plist "$ROOT/Library/LaunchDaemons/"
cp -R "control-panel/build/OpenAudio FW1814.app" "$ROOT/Applications/"
cp -R installer/assets/MAudioFireWireMIDI.plugin "$ROOT/Library/Audio/MIDI Drivers/"
install -d "$ROOT/Library/Application Support/OpenAudio"
install -m 755 scripts/uninstall.sh "$ROOT/Library/Application Support/OpenAudio/uninstall.sh"
cp -R installer/assets/DeviceIcons "$ROOT/Library/Application Support/M-Audio/DeviceIcons"
chmod 755 "$ROOT/Library/PrivilegedHelperTools/com.openaudio.fw1814.helper"
chmod 644 "$ROOT/Library/LaunchDaemons/com.openaudio.fw1814.helper.plist"
chmod -R go-w "$ROOT"
find "$ROOT" -name .DS_Store -delete
chmod 755 installer/scripts/preinstall installer/scripts/postinstall

echo "== pkgbuild / productbuild $VERSION"
# Bundles are relocatable by default: the Installer then updates any other copy of the same bundle id it finds (it replaced the build-tree panel app
# instead of installing /Applications/OpenAudio FW1814.app, 2026-10-17). Every bundle in the payload is pinned to its path.
pkgbuild --analyze --root "$ROOT" "$OUT/components.plist" >/dev/null
# The default also version-checks ("upgrade" only replaces a bundle with a lower version): a rebuilt kext with the same CFBundleVersion would be skipped.
N=$(grep -c RootRelativeBundlePath "$OUT/components.plist")
PB=/usr/libexec/PlistBuddy
for ((i = 0; i < N; i++)); do
  for kv in "BundleIsRelocatable bool false" "BundleIsVersionChecked bool false" "BundleOverwriteAction string overwrite"; do
    set -- $kv
    $PB -c "Delete :$i:$1" "$OUT/components.plist" 2>/dev/null || true
    $PB -c "Add :$i:$1 $2 $3" "$OUT/components.plist"
  done
done
pkgbuild --root "$ROOT" --component-plist "$OUT/components.plist" --identifier com.openaudio.fw1814.pkg --version "$VERSION" --install-location / \
         --ownership recommended --scripts installer/scripts "$OUT/component.pkg" >/dev/null
sed "s/@VERSION@/$VERSION/g" installer/distribution.xml.in > "$OUT/distribution.xml"
productbuild --distribution "$OUT/distribution.xml" --resources installer/resources --package-path "$OUT" "$OUT/OpenAudioFW1814-$VERSION.pkg" >/dev/null
echo "built $OUT/OpenAudioFW1814-$VERSION.pkg"
pkgutil --payload-files "$OUT/OpenAudioFW1814-$VERSION.pkg" | grep -c . | sed 's/^/payload entries: /'
