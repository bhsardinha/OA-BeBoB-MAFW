#!/bin/bash
# Usage: scripts/decomp_by_name.sh '<awk-regex on qualified name>' [tag]
# Decompiles every function in ghidra-exports/functions.tsv whose name matches (excluding MetaClass/ctors/dtors) with
# Ghidra headless (needs ~/tools Ghidra + JDK, see memory reference_ghidra_headless_recipe) and writes one file per function
# to ghidra-exports/decomp/<Class__method>[_<addr> when the name repeats].c . Run from docs/system-map/A1-ghidra.
set -e
RE="$1"; TAG="${2:-bulk}"
T=~/tools; export JAVA_HOME=$T/jdk-21.0.12.1+1/Contents/Home PATH=$T/jdk-21.0.12.1+1/Contents/Home/bin:$PATH
W=$(mktemp -d); cp -R ghidra-project $W/p
ADDRS=$(awk -F'\t' -v re="$RE" '$3 ~ re && $3 !~ /MetaClass|~|::EXTERNAL|<EXTERNAL>/ {print $1}' ghidra-exports/functions.tsv | sort -u | tr '\n' ' ')
$T/ghidra_12.1.4_PUBLIC/support/analyzeHeadless $W/p fw1814-kext -process -readOnly -noanalysis -scriptPath scripts \
  -postScript DecompList.java $ADDRS > $W/out.txt 2>&1
python3 - "$W/out.txt" "$TAG" <<'PY'
import re,sys,os
t=open(sys.argv[1]).read(); tag=sys.argv[2]
parts=re.split(r'INFO  DecompList\.java> === (.+?) @ ([0-9a-f]+) === \(GhidraScript\)  \n',t)
seen={}
for i in range(1,len(parts),3):
    name,addr,body=parts[i],parts[i+1],parts[i+2]
    body=re.sub(r'^INFO  DecompList\.java> ','',body).split(' (GhidraScript)')[0]
    fn=name.replace('m_audio_b_','').replace('com_m_audio_','').replace('::','__').replace('<','').replace('>','')
    path=f'ghidra-exports/decomp/{fn}.c'
    if os.path.exists(path) and not open(path).read().startswith(f'// {name}\n// Entry: {addr}'): path=f'ghidra-exports/decomp/{fn}_{addr}.c'
    open(path,'w').write(f'// {name}\n// Entry: {addr}\n// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 ({tag})\n\n{body.strip()}\n'); seen[path]=1
print(len(seen),'functions written')
PY
