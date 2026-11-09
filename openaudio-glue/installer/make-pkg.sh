#!/bin/bash
# On the dev Mac: sync the main tree (for GlueTap.h, the style sources and the scripts) and openaudio-glue to the build PC, build the
# Glue package there, fetch it to openaudio-glue/dist/. Usage: installer/make-pkg.sh [classic|modern]   (edition, default classic; same binaries,
# each package accepts only its own macOS range, like the main project's editions; VERSION=... overrides openaudio-glue/VERSION)
set -euo pipefail
cd "$(dirname "$0")/.."
HOST=${BUILDPC:-fw1814-build}
BAND=${1:-classic}
case $BAND in classic|modern) ;; *) echo "usage: installer/make-pkg.sh [classic|modern]"; exit 1;; esac
(cd ../openaudio-mafw-bebob && BUILDPC=$HOST scripts/sync-to-buildpc.sh >/dev/null)
rsync -a --delete --exclude build --exclude dist --exclude art --exclude .DS_Store --exclude 'tools/*/glue-*' ./ "$HOST":oa-glue/      # keeps the tool binaries built there
ssh "$HOST" "cd ~/oa-glue && ${VERSION:+VERSION=$VERSION} BAND=$BAND installer/build-pkg.sh"
mkdir -p dist
V=${VERSION:-$(cat VERSION)}
scp -q "$HOST:oa-glue/installer/build/OpenAudio-Glue-$V-$BAND.pkg" dist/
ls -la "dist/OpenAudio-Glue-$V-$BAND.pkg"
