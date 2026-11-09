#!/bin/bash
# The Glue disk image: Install OpenAudio Glue.pkg + Uninstall OpenAudio Glue.app on a styled background. Run on the dev Mac. The package and the uninstaller
# are built on the build PC (10.13 toolchain); art and dmgbuild come from the main project's venv.
# Usage: BAND=classic|modern installer/make-dmg.sh [--skip-pkg]       output: dist/OpenAudio-Glue-<version>-<edition>.dmg
set -euo pipefail
cd "$(dirname "$0")/.."
ROOT=$PWD
HOST=${BUILDPC:-fw1814-build}
VENV=../openaudio-mafw-bebob/installer/.venv/bin
VERSION=$(cat VERSION)
BAND=${BAND:-classic}
case $BAND in classic) EDITION=Classic;; modern) EDITION=Modern;; *) echo "BAND must be classic or modern"; exit 1;; esac
STAGE=$ROOT/installer/build/dmg
[ "${1:-}" = "--skip-pkg" ] || installer/make-pkg.sh "$BAND"
"$VENV/python" installer/make-art.py "$BAND"

echo "== uninstaller app (built on the build PC)"
A="Uninstall OpenAudio Glue.app"
rsync -a installer/uninstaller installer/art/UninstallIcon.icns installer/glue-uninstall.sh "$HOST":oa-glue/installer/
ssh "$HOST" 'cd ~/oa-glue/installer && rm -rf uapp && mkdir -p "uapp/'"$A"'/Contents/MacOS" "uapp/'"$A"'/Contents/Resources" \
  && SDK=$(xcrun --sdk macosx --show-sdk-path) \
  && xcrun swiftc -sdk "$SDK" -target x86_64-apple-macosx10.13 -O -o "uapp/'"$A"'/Contents/MacOS/Uninstall OpenAudio Glue" uninstaller/Uninstaller.swift \
  && ~/oa-build-mafw/scripts/bundle-swift-runtime.sh "uapp/'"$A"'" "Uninstall OpenAudio Glue" \
  && cp uninstaller/Info.plist "uapp/'"$A"'/Contents/Info.plist" \
  && /usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString '"$VERSION"'" "uapp/'"$A"'/Contents/Info.plist" \
  && cp UninstallIcon.icns "uapp/'"$A"'/Contents/Resources/UninstallIcon.icns" \
  && cp glue-uninstall.sh "uapp/'"$A"'/Contents/Resources/glue-uninstall.sh" && chmod 755 "uapp/'"$A"'/Contents/Resources/glue-uninstall.sh" \
  && ~/oa-build-mafw/scripts/check-self-contained.sh uapp \
  && tar -C uapp -czf uapp.tgz "'"$A"'"'
rm -rf "$STAGE"; mkdir -p "$STAGE"
scp -q "$HOST":oa-glue/installer/uapp.tgz "$STAGE/"
tar -C "$STAGE" -xzf "$STAGE/uapp.tgz"

DMG=dist/OpenAudio-Glue-$VERSION-$BAND.dmg
rm -f "$DMG"
"$VENV/dmgbuild" -s installer/dmg-settings.py \
  -D pkg="$ROOT/dist/OpenAudio-Glue-$VERSION-$BAND.pkg" -D app="$STAGE/$A" -D art="$ROOT/installer/art" \
  -D background="$ROOT/installer/art/dmg-background-$BAND.tiff" "OpenAudio Glue $VERSION $EDITION" "$DMG"
echo "built $DMG"; ls -l "$DMG"; shasum -a 256 "$DMG"
