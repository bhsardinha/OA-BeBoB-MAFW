#!/bin/bash
# Makes a Swift 4.x app self-contained on macOS older than 10.14.4 (no Swift ABI stability there, so the OS has no Swift runtime): copies the libswift*.dylib files
# the executable needs into Contents/Frameworks, points @rpath at them and DROPS the toolchain rpath, so the app cannot silently depend on Xcode being installed.
# Without this a clean 10.13.6 Mac crashes at launch: "Library not loaded: @rpath/libswiftAppKit.dylib" (found 2026-10-17 on a second Mac).
# Usage (build PC, Xcode 9.4.1):  scripts/bundle-swift-runtime.sh "<path to .app>" "<executable name>"
set -euo pipefail
APP=$1; EXE="$APP/Contents/MacOS/$2"
[ -x "$EXE" ] || { echo "no executable: $EXE"; exit 1; }
mkdir -p "$APP/Contents/Frameworks"
xcrun swift-stdlib-tool --copy --platform macosx --scan-executable "$EXE" --destination "$APP/Contents/Frameworks"
# keep only @executable_path/../Frameworks as an rpath
otool -l "$EXE" | awk '/cmd LC_RPATH/{f=1} f && /^ +path /{print $2; f=0}' | while read -r rp; do
  [ "$rp" = "@executable_path/../Frameworks" ] || install_name_tool -delete_rpath "$rp" "$EXE"
done
otool -l "$EXE" | grep -q "path @executable_path/../Frameworks" || install_name_tool -add_rpath @executable_path/../Frameworks "$EXE"
# every @rpath dependency (of the executable and of the bundled libraries) must now be inside the bundle
missing=0
for f in "$EXE" "$APP"/Contents/Frameworks/*.dylib; do
  for lib in $(otool -L "$f" | awk '/@rpath\//{n=$1; sub(".*@rpath/","",n); print n}'); do
    [ -f "$APP/Contents/Frameworks/$lib" ] || { echo "missing in bundle: $lib (needed by $f)"; missing=1; }
  done
done
[ $missing = 0 ] || exit 1
echo "swift runtime bundled: $(ls "$APP/Contents/Frameworks" | wc -l | tr -d ' ') libraries"
