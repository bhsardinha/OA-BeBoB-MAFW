#!/bin/bash
# Panel <-> kext <-> plist conformance check (Build row IN.7); static, no hardware. See scripts/conformance_check.py and docs/CONFORMANCE.md.
exec python3 "$(dirname "$0")/conformance_check.py" "$@"
