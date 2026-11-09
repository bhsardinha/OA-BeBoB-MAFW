#!/bin/sh
# Downloads the four Airwindows originals Glue ports, at the pinned commit, into ref/ (git-ignored).
# Needs curl. Source: github.com/airwindows/airwindows (MIT).
set -e
COMMIT=dfa54018e788fd67b8e57ae6c8012a05f1f086a6
cd "$(dirname "$0")"
for P in Interstage Tube2 Channel9 ToTape9; do
    mkdir -p "ref/$P"
    for F in "$P.h" "$P.cpp" "${P}Proc.cpp"; do
        curl -fsSL "https://raw.githubusercontent.com/airwindows/airwindows/$COMMIT/plugins/MacVST/$P/source/$F" -o "ref/$P/$F"
    done
done
echo "ref/ ready ($COMMIT)"
