#!/bin/bash
# IN.5 - the distributable disk image: Install OpenAudio FW1814.pkg + Uninstall OpenAudio FW1814.app on a styled background.
# Run on the dev Mac. Steps: (1) installer/make-pkg.sh builds the .pkg on the build PC (10.13 toolchain); (2) the uninstaller app is built on the build PC
# too and fetched; (3) installer/art/ (icons, background) comes from make-art.py (Pillow, in installer/.venv); (4) dmgbuild lays the image out.
# Usage: installer/make-dmg.sh [--skip-pkg]       output: dist/OpenAudioFW1814-<version>.dmg
set -euo pipefail
HOST=${BUILDPC:-bernardosardinha@192.168.100.20}
cd "$(dirname "$0")/.."
ROOT=$PWD
VERSION=$(/usr/libexec/PlistBuddy -c 'Print :CFBundleShortVersionString' kext/Info.plist)
STAGE=$ROOT/installer/build/dmg
[ -x installer/.venv/bin/python ] || { python3 -m venv installer/.venv && installer/.venv/bin/pip install -q -r installer/requirements.txt; }

[ "${1:-}" = "--skip-pkg" ] || installer/make-pkg.sh
installer/.venv/bin/python installer/make-art.py

echo "== uninstaller app (built on the build PC)"
rsync -a installer/uninstaller installer/art/UninstallIcon.icns scripts/uninstall.sh "$HOST":oa-build/installer/
rsync -a scripts/bundle-swift-runtime.sh "$HOST":oa-build/scripts/
ssh "$HOST" 'cd ~/oa-build/installer && rm -rf uapp && mkdir -p "uapp/Uninstall OpenAudio FW1814.app/Contents/MacOS" "uapp/Uninstall OpenAudio FW1814.app/Contents/Resources" \
  && SDK=$(xcrun --sdk macosx --show-sdk-path) \
  && xcrun swiftc -sdk "$SDK" -target x86_64-apple-macosx10.13 -O -o "uapp/Uninstall OpenAudio FW1814.app/Contents/MacOS/Uninstall OpenAudio FW1814" uninstaller/Uninstaller.swift \
  && ../scripts/bundle-swift-runtime.sh "uapp/Uninstall OpenAudio FW1814.app" "Uninstall OpenAudio FW1814" \
  && cp uninstaller/Info.plist "uapp/Uninstall OpenAudio FW1814.app/Contents/Info.plist" \
  && cp UninstallIcon.icns "uapp/Uninstall OpenAudio FW1814.app/Contents/Resources/UninstallIcon.icns" \
  && cp uninstall.sh "uapp/Uninstall OpenAudio FW1814.app/Contents/Resources/uninstall.sh" && chmod 755 "uapp/Uninstall OpenAudio FW1814.app/Contents/Resources/uninstall.sh" \
  && tar -C uapp -czf uapp.tgz "Uninstall OpenAudio FW1814.app"'
rm -rf "$STAGE"; mkdir -p "$STAGE"
scp -q "$HOST":oa-build/installer/uapp.tgz "$STAGE/"
tar -C "$STAGE" -xzf "$STAGE/uapp.tgz"
echo "== self-contained check of the uninstaller app"
rsync -a scripts/check-self-contained.sh "$HOST":oa-build/scripts/
ssh "$HOST" 'cd ~/oa-build/installer && ../scripts/check-self-contained.sh uapp'
# note: a missing Swift runtime shows up as a launch crash only on a Mac without Xcode; this check is the machine-independent proof

DMG=dist/OpenAudioFW1814-$VERSION.dmg
rm -f "$DMG"
installer/.venv/bin/dmgbuild -s installer/dmg-settings.py \
  -D pkg="$ROOT/dist/OpenAudioFW1814-$VERSION.pkg" -D app="$STAGE/Uninstall OpenAudio FW1814.app" -D art="$ROOT/installer/art" \
  "OpenAudio FW1814 $VERSION" "$DMG"
echo "built $DMG"; ls -l "$DMG"; shasum -a 256 "$DMG"
