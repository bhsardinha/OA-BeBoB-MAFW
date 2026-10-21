#!/bin/bash
# Panel CPU bench (2026-10-21, R119). On the iMac: copy to the build PC, run with the panel built in ~/oa-build-mafw. The screen must be awake and
# unlocked (covered windows stop drawing; OA_BENCH_IGNORE_OCCLUSION=1 forces drawing, without WindowServer composition on a sleeping display).
# usage: panel-cpubench.sh <label> <args...>   runs a demo panel for 25 s, reports its CPU and WindowServer's CPU (top, 5 s samples, first skipped)
APP=~/oa-build-mafw/control-panel/build/"OA BeBoB MAFW.app"/Contents/MacOS/"OA BeBoB MAFW"
label=$1; shift
"$APP" --demo "$@" >/dev/null 2>&1 &
P=$!; sleep 4
WS=$(pgrep -x WindowServer)
top -l 5 -s 5 -stats pid,cpu -pid $P -pid $WS | awk -v p=$P -v w=$WS -v l="$label" '$1==p{a[++n]=$2} $1==w{b[++m]=$2} END{s=0;t=0;for(i=2;i<=n;i++)s+=a[i];for(i=2;i<=m;i++)t+=b[i];printf "%-28s panel %5.1f%%  WindowServer %5.1f%%\n", l, s/(n-1), t/(m-1)}'
kill $P
