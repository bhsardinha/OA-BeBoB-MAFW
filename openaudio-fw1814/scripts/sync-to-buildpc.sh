#!/bin/bash
# The dev Mac is the single source of truth; the build PC (10.13.6, Xcode 9.4.1) only receives copies.
# One-way rsync of the buildable tree to ~/oa-build, so uncommitted work builds there too.
# Usage: scripts/sync-to-buildpc.sh [--build]     (--build also runs make for kext + helper there)
set -euo pipefail
HOST=${BUILDPC:-bernardosardinha@192.168.100.20}
cd "$(dirname "$0")/.."
rsync -a --delete --exclude build --exclude .DS_Store kext scripts tools firmware-helper control-panel tests "$HOST":oa-build/
if [ "${1:-}" = "--build" ]; then
  ssh "$HOST" 'cd ~/oa-build && make -C kext 2>&1 | grep -E "error|warning: .*(FW|OA|oa)" ; make -C firmware-helper 2>&1 | grep -E "error|warning"; ls kext/build/*.kext/Contents/MacOS firmware-helper/build'
fi
