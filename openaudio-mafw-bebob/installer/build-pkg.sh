#!/bin/bash
# IN.5 - builds OA-BeBoB-MAFW-<version>-<edition>.pkg ON THE BUILD PC (10.13.6, Xcode 9.4.1: the only machine whose SDK builds the kext and the panel).
# Run via installer/make-pkg.sh from the dev Mac, or directly in ~/oa-build-mafw/installer once ~/oa-build-mafw/installer/assets holds the reference MIDI
# plug-in and the device icons (make-pkg.sh copies them from _extracted/).
# Payload (all root:wheel):
#   /Library/Extensions/OpenAudioBeBoBMAFW.kext                         kext (unsigned: IN.6 needs the user's Developer ID)
#   /Library/PrivilegedHelperTools/com.openaudio.bebobmafw.helper       helper daemon (firmware start, settings persistence)
#   /Library/LaunchDaemons/com.openaudio.bebobmafw.helper.plist
#   /Applications/OA BeBoB MAFW.app                                      control panel, with the read-only firmware check window (formerly the
#                                                                        separate Firmware Tool; ships no M-Audio image, plan v6)
#   /Library/Audio/MIDI Drivers/MAudioFireWireMIDI.plugin            the 2014 reference CoreMIDI plug-in, unmodified (M-H2; Q-MH plug-in decision)
#   /Library/Application Support/OpenAudio/uninstall.sh              removes all of the above
#   /Library/Application Support/M-Audio/DeviceIcons/*               device icons the plug-in looks up by MAIconName (A3-M2 section 3)
# Scripts: preinstall = this package's scripts/uninstall.sh --update (removes the files of every earlier release, keeps settings; never unloads a
# running driver, R117), postinstall (kextutil -t -n gate, caches, helper, MIDIServer). The package requires a restart: the new kext loads at boot.
# Editions (BROAD_COMPATIBILITY_PLAN_v2 §11a, R128): BAND=classic (default; 10.13 to 10.15, kext built here) or BAND=modern (macOS 11+, Intel: the
# kext is built on the dev Mac with the 15.x SDK and copied to kext/build/modern with imports.txt by make-pkg.sh; the panel is this machine's
# BAND=modern build; postinstall-modern links through kmutil). Each edition has its own allowed OS range, pages and postinstall.
set -euo pipefail
cd "$(dirname "$0")/.."
BAND=${BAND:-classic}
case $BAND in
  classic) EDITION=Classic; OSVERSIONS='<os-version min="10.13" before="11.0"/>'; KEXT_SRC=kext/build/OpenAudioBeBoBMAFW.kext; APPDIR=control-panel/build;
           POST=installer/scripts/postinstall; PAGES=installer/resources;;
  modern)  EDITION=Modern; OSVERSIONS='<os-version min="11.0" before="16.0"/>'; KEXT_SRC=kext/build/modern/OpenAudioBeBoBMAFW.kext; APPDIR=control-panel/build/modern
           POST=installer/scripts/postinstall-modern; PAGES=installer/resources/modern;;
  *) echo "BAND must be classic or modern"; exit 1;;
esac
# Versions (user, 2026-10-21): the package has its own number (VERSION, changes with every release; names the .pkg/.dmg and CHANGELOG.md entries);
# the driver (kext/Info.plist) and the panel (control-panel/Info.plist; includes the firmware check since 0.2.2) change only with their own code.
VERSION=${VERSION:-$(cat VERSION)}
PB=/usr/libexec/PlistBuddy
DRIVER_V=$($PB -c 'Print :CFBundleShortVersionString' kext/Info.plist)
PANEL_V=$($PB -c 'Print :CFBundleShortVersionString' control-panel/Info.plist)
OUT=installer/build
ROOT=$OUT/root
rm -rf "$OUT"; mkdir -p "$ROOT"

echo "== build kext, helper, panel ($EDITION)"
if [ "$BAND" = modern ]; then
  [ -d "$KEXT_SRC" ] && [ -s kext/build/modern/imports.txt ] || { echo "missing the Modern kext + imports.txt (run installer/make-pkg.sh modern on the dev Mac)"; exit 1; }
else
  make -C kext >/dev/null
fi
make -C firmware-helper >/dev/null
make -C control-panel app BAND=$BAND >/dev/null

echo "== stage payload"
install -d "$ROOT/Library/Extensions" "$ROOT/Library/PrivilegedHelperTools" "$ROOT/Library/LaunchDaemons" "$ROOT/Applications" \
           "$ROOT/Library/Audio/MIDI Drivers" "$ROOT/Library/Application Support/M-Audio"
cp -R "$KEXT_SRC" "$ROOT/Library/Extensions/"
cp firmware-helper/build/oa-fwhelper "$ROOT/Library/PrivilegedHelperTools/com.openaudio.bebobmafw.helper"
cp firmware-helper/com.openaudio.bebobmafw.helper.plist "$ROOT/Library/LaunchDaemons/"
cp -R "$APPDIR/OA BeBoB MAFW.app" "$ROOT/Applications/"
cp -R installer/assets/MAudioFireWireMIDI.plugin "$ROOT/Library/Audio/MIDI Drivers/"
install -d "$ROOT/Library/Application Support/OpenAudio"
install -m 755 scripts/uninstall.sh "$ROOT/Library/Application Support/OpenAudio/uninstall.sh"
cp -R installer/assets/DeviceIcons "$ROOT/Library/Application Support/M-Audio/DeviceIcons"
chmod 755 "$ROOT/Library/PrivilegedHelperTools/com.openaudio.bebobmafw.helper"
chmod 644 "$ROOT/Library/LaunchDaemons/com.openaudio.bebobmafw.helper.plist"
chmod -R go-w "$ROOT"
find "$ROOT" -name .DS_Store -delete
echo "== self-contained check + clean launch of the panel"
scripts/check-self-contained.sh "$ROOT"
# the staged panel must start with no Xcode rpath (its Swift runtime is inside the bundle); --demo --snapshot exits by itself
PANEL="$ROOT/Applications/OA BeBoB MAFW.app/Contents/MacOS/OA BeBoB MAFW"
SNAP=$(mktemp -d); "$PANEL" --demo --snapshot "$SNAP" >/dev/null 2>&1 || { echo "the staged panel does not launch"; exit 1; }
[ -f "$SNAP/48000-adat-mixer.png" ] || { echo "the staged panel did not render"; exit 1; }; rm -rf "$SNAP"
SCRIPTS=$OUT/scripts; mkdir -p "$SCRIPTS"
cp installer/scripts/preinstall "$SCRIPTS/"
cp "$POST" "$SCRIPTS/postinstall"
[ "$BAND" = modern ] && cp kext/build/modern/imports.txt "$SCRIPTS/"      # the Modern gate's list (postinstall-modern)
install -m 755 scripts/uninstall.sh "$SCRIPTS/uninstall.sh"      # the update step: the same list the uninstaller uses
chmod 755 "$SCRIPTS/preinstall" "$SCRIPTS/postinstall"

echo "== pkgbuild / productbuild: package $VERSION $EDITION (driver $DRIVER_V, panel $PANEL_V)"
# Bundles are relocatable by default: the Installer then updates any other copy of the same bundle id it finds (it replaced the build-tree panel app
# instead of installing /Applications/OpenAudio BeBoB MAFW.app, 2026-10-17). Every bundle in the payload is pinned to its path.
pkgbuild --analyze --root "$ROOT" "$OUT/components.plist" >/dev/null
# The default also version-checks ("upgrade" only replaces a bundle with a lower version): a rebuilt kext with the same CFBundleVersion would be skipped.
N=$(grep -c RootRelativeBundlePath "$OUT/components.plist")
for ((i = 0; i < N; i++)); do
  for kv in "BundleIsRelocatable bool false" "BundleIsVersionChecked bool false" "BundleOverwriteAction string overwrite"; do
    set -- $kv
    $PB -c "Delete :$i:$1" "$OUT/components.plist" 2>/dev/null || true
    $PB -c "Add :$i:$1 $2 $3" "$OUT/components.plist"
  done
done
pkgbuild --root "$ROOT" --component-plist "$OUT/components.plist" --identifier com.openaudio.bebobmafw.pkg --version "$VERSION" --install-location / \
         --ownership recommended --scripts "$SCRIPTS" "$OUT/component.pkg" >/dev/null
sed -e "s/@VERSION@/$VERSION/g" -e "s/@EDITION@/$EDITION/g" -e "s#@OSVERSIONS@#$OSVERSIONS#" installer/distribution.xml.in > "$OUT/distribution.xml"
# welcome / conclusion pages carry the package and component versions
mkdir -p "$OUT/resources"
for f in "$PAGES"/*.html; do
  sed -e "s/@VERSION@/$VERSION/g" -e "s/@DRIVER@/$DRIVER_V/g" -e "s/@PANEL@/$PANEL_V/g" "$f" > "$OUT/resources/$(basename "$f")"
done
PKG="$OUT/OA-BeBoB-MAFW-$VERSION-$BAND.pkg"
productbuild --distribution "$OUT/distribution.xml" --resources "$OUT/resources" --package-path "$OUT" "$PKG" >/dev/null
echo "built $PKG"
pkgutil --payload-files "$PKG" | grep -c . | sed 's/^/payload entries: /'
