#!/bin/bash
# Builds and runs every host test (kernel-free parts of the kext: frames, tables, decoders, converters). Run from anywhere; binaries go to
# openaudio-mafw-bebob/build/ (git-ignored). Exit status 1 if any test fails.
set -u
cd "$(dirname "$0")/../.."
mkdir -p build
B=kext/src/base; M=kext/src/models
fail=0
run() {   # name, then compiler arguments
    local name=$1; shift
    if ! c++ -std=c++11 -Wall "$@" -o "build/$name" 2>"build/$name.log"; then echo "BUILD FAIL $name (build/$name.log)"; fail=1; return; fi
    if "build/$name" > "build/$name.out"; then echo "PASS $name ($(grep -c '^OK' "build/$name.out") checks)"; else echo "FAIL $name"; grep -v '^OK' "build/$name.out"; fail=1; fi
}
run avc_frames        -I $B tests/host/avc_frames_test.cpp
run samples           -I $B -I $M/FW1814 tests/host/samples_test.cpp $B/FWSamples.cpp $M/FW1814/FW1814Peaks.cpp
run audiophile_status -I $M/FWAudiophile tests/host/audiophile_status_test.cpp
run audiophile_mixer  -I $B -I $M/FWAudiophile tests/host/audiophile_mixer_test.cpp
run frontpanel        -I $B tests/host/frontpanel_test.cpp
run fw410             -I $B -I $M/FW410 tests/host/fw410_test.cpp $B/FWSamples.cpp
exit $fail
