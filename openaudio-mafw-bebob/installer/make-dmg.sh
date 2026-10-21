#!/bin/bash
# IN.5 - the distributable disk image: Install OA BeBoB MAFW.pkg + Uninstall OA BeBoB MAFW.app on a styled background.
# Run on the dev Mac. Steps: (1) installer/make-pkg.sh builds the .pkg on the build PC (10.13 toolchain); (2) the uninstaller app is built on the build PC
# too and fetched; (3) installer/art/ (icons, background) comes from make-art.py (Pillow, in installer/.venv); (4) dmgbuild lays the image out.
# Usage: installer/make-dmg.sh [--skip-pkg]       output: dist/OpenAudioBeBoBMAFW-<version>.dmg
set -euo pipefail
HOST=${BUILDPC:-bernardosardinha@192.168.100.20}
cd "$(dirname "$0")/.."
ROOT=$PWD
VERSION=$(cat VERSION)                            # the package version (user, 2026-10-21); components keep their own numbers
STAGE=$ROOT/installer/build/dmg
[ -x installer/.venv/bin/python ] || { python3 -m venv installer/.venv && installer/.venv/bin/pip install -q -r installer/requirements.txt; }

[ "${1:-}" = "--skip-pkg" ] || installer/make-pkg.sh
installer/.venv/bin/python installer/make-art.py

echo "== uninstaller app (built on the build PC)"
rsync -a installer/uninstaller installer/art/UninstallIcon.icns scripts/uninstall.sh "$HOST":oa-build-mafw/installer/
rsync -a scripts/bundle-swift-runtime.sh "$HOST":oa-build-mafw/scripts/
ssh "$HOST" 'cd ~/oa-build-mafw/installer && rm -rf uapp && mkdir -p "uapp/Uninstall OA BeBoB MAFW.app/Contents/MacOS" "uapp/Uninstall OA BeBoB MAFW.app/Contents/Resources" \
  && SDK=$(xcrun --sdk macosx --show-sdk-path) \
  && xcrun swiftc -sdk "$SDK" -target x86_64-apple-macosx10.13 -O -o "uapp/Uninstall OA BeBoB MAFW.app/Contents/MacOS/Uninstall OA BeBoB MAFW" uninstaller/Uninstaller.swift \
  && ../scripts/bundle-swift-runtime.sh "uapp/Uninstall OA BeBoB MAFW.app" "Uninstall OA BeBoB MAFW" \
  && cp uninstaller/Info.plist "uapp/Uninstall OA BeBoB MAFW.app/Contents/Info.plist" \
  && /usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString '"$VERSION"'" "uapp/Uninstall OA BeBoB MAFW.app/Contents/Info.plist" \
  && cp UninstallIcon.icns "uapp/Uninstall OA BeBoB MAFW.app/Contents/Resources/UninstallIcon.icns" \
  && cp uninstall.sh "uapp/Uninstall OA BeBoB MAFW.app/Contents/Resources/uninstall.sh" && chmod 755 "uapp/Uninstall OA BeBoB MAFW.app/Contents/Resources/uninstall.sh" \
  && tar -C uapp -czf uapp.tgz "Uninstall OA BeBoB MAFW.app"'
rm -rf "$STAGE"; mkdir -p "$STAGE"
scp -q "$HOST":oa-build-mafw/installer/uapp.tgz "$STAGE/"
tar -C "$STAGE" -xzf "$STAGE/uapp.tgz"
echo "== self-contained check of the uninstaller app"
rsync -a scripts/check-self-contained.sh "$HOST":oa-build-mafw/scripts/
ssh "$HOST" 'cd ~/oa-build-mafw/installer && ../scripts/check-self-contained.sh uapp'
# note: a missing Swift runtime shows up as a launch crash only on a Mac without Xcode; this check is the machine-independent proof

DMG=dist/OA-BeBoB-MAFW-$VERSION.dmg
rm -f "$DMG"
installer/.venv/bin/dmgbuild -s installer/dmg-settings.py \
  -D pkg="$ROOT/dist/OA-BeBoB-MAFW-$VERSION.pkg" -D app="$STAGE/Uninstall OA BeBoB MAFW.app" -D art="$ROOT/installer/art" \
  "OA BeBoB MAFW $VERSION" "$DMG"
echo "built $DMG"; ls -l "$DMG"; shasum -a 256 "$DMG"
