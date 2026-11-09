#!/bin/bash
# OpenAudio Glue package. Runs on the build PC (10.13.6, Xcode 9.4.1) in ~/oa-glue with MAFW=~/oa-build-mafw (synced by
# make-pkg.sh). Builds the DSP, the plug-in and the app, stages them, checks the payload is self-contained and the app starts,
# then pkgbuild / productbuild with every bundle pinned to its path (as the main package does).
set -euo pipefail
cd "$(dirname "$0")/.."
MAFW=${MAFW:-$HOME/oa-build-mafw}
VERSION=${VERSION:-$(cat VERSION)}
BAND=${BAND:-classic}                 # edition: same binaries, each package accepts only its own macOS range (as the main project's editions)
case $BAND in
  classic) EDITION=Classic; OSVERSIONS='<os-version min="10.13" before="11.0"/>'; REQ="macOS 10.13 to 10.15";;
  modern)  EDITION=Modern;  OSVERSIONS='<os-version min="11.0" before="16.0"/>'; REQ="macOS 11 to 15, Intel only";;
  *) echo "BAND must be classic or modern"; exit 1;;
esac
OUT=installer/build; ROOT=$OUT/root; PB=/usr/libexec/PlistBuddy
rm -rf "$OUT"; mkdir -p "$ROOT"
echo "== build dsp, driver, app"
make -C dsp >/dev/null
make -C driver KEXT_INC="$MAFW/kext/src/base" >/dev/null
make -C panel MAFW="$MAFW" app >/dev/null
echo "== stage payload"
install -d "$ROOT/Library/Audio/Plug-Ins/HAL" "$ROOT/Applications" "$ROOT/Library/Application Support/OpenAudio Glue"
cp -R driver/build/OpenAudioGlue.driver "$ROOT/Library/Audio/Plug-Ins/HAL/"
cp -R "panel/build/OpenAudio Glue.app" "$ROOT/Applications/"
install -m 755 installer/glue-uninstall.sh "$ROOT/Library/Application Support/OpenAudio Glue/glue-uninstall.sh"
for plist in "$ROOT/Applications/OpenAudio Glue.app/Contents/Info.plist" "$ROOT/Library/Audio/Plug-Ins/HAL/OpenAudioGlue.driver/Contents/Info.plist"; do
  $PB -c "Set :CFBundleShortVersionString $VERSION" -c "Set :CFBundleVersion $VERSION" "$plist"      # every bundle says the package's number
done
# the Airwindows MIT notice travels with the binaries
install -d "$ROOT/Library/Audio/Plug-Ins/HAL/OpenAudioGlue.driver/Contents/Resources"
install -m 644 NOTICE.md "$ROOT/Library/Audio/Plug-Ins/HAL/OpenAudioGlue.driver/Contents/Resources/NOTICE.md"
install -m 644 NOTICE.md "$ROOT/Applications/OpenAudio Glue.app/Contents/Resources/NOTICE.md"
chmod -R go-w "$ROOT"; find "$ROOT" -name .DS_Store -delete
echo "== self-contained check + clean launch of the app"
"$MAFW/scripts/check-self-contained.sh" "$ROOT"
SNAP=$(mktemp -d); "$ROOT/Applications/OpenAudio Glue.app/Contents/MacOS/OpenAudio Glue" --demo --snapshot "$SNAP" >/dev/null 2>&1 || { echo "the staged app does not launch"; exit 1; }
[ -f "$SNAP/glue-unit-a.png" ] || { echo "the staged app did not render"; exit 1; }; rm -rf "$SNAP"
SCRIPTS=$OUT/scripts; mkdir -p "$SCRIPTS"
cp installer/scripts/preinstall installer/scripts/postinstall "$SCRIPTS/"
echo "== pkgbuild / productbuild: OpenAudio Glue $VERSION $EDITION"
pkgbuild --analyze --root "$ROOT" "$OUT/components.plist" >/dev/null
N=$(grep -c RootRelativeBundlePath "$OUT/components.plist")
for ((i = 0; i < N; i++)); do
  for kv in "BundleIsRelocatable bool false" "BundleIsVersionChecked bool false" "BundleOverwriteAction string overwrite"; do
    set -- $kv
    $PB -c "Delete :$i:$1" "$OUT/components.plist" 2>/dev/null || true
    $PB -c "Add :$i:$1 $2 $3" "$OUT/components.plist"
  done
done
pkgbuild --root "$ROOT" --component-plist "$OUT/components.plist" --identifier com.openaudio.glue.pkg --version "$VERSION" --install-location / \
         --ownership recommended --scripts "$SCRIPTS" "$OUT/component.pkg" >/dev/null
sed -e "s/@VERSION@/$VERSION/g" -e "s/@EDITION@/$EDITION/g" -e "s#@OSVERSIONS@#$OSVERSIONS#" installer/distribution.xml.in > "$OUT/distribution.xml"
mkdir -p "$OUT/resources"; sed -e "s/@VERSION@/$VERSION/g" -e "s/@EDITION@/$EDITION/g" -e "s/@REQ@/$REQ/g" installer/resources/welcome.html > "$OUT/resources/welcome.html"
PKG="$OUT/OpenAudio-Glue-$VERSION-$BAND.pkg"
productbuild --distribution "$OUT/distribution.xml" --resources "$OUT/resources" --package-path "$OUT" "$PKG" >/dev/null
echo "built $PKG"
pkgutil --payload-files "$PKG" | grep -c . | sed 's/^/payload entries: /'
