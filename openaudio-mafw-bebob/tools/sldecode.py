#!/usr/bin/env python3
# sldecode.py -- pair slsetprops.d records with the slpanel.py action log (bench tool, dev Mac).
# usage: sldecode.py SETPROPS.txt ACTIONS.txt   -> "wall ms | last action before the write | key value" per write
# Decoding as capture 92: 32-bit ints signed, 64-bit shown hex, arrays flattened, IDREF expanded.
import re, sys
import xml.etree.ElementTree as ET

def records(path):
    t = open(path, encoding='latin1').read()
    for blk in t.split('[wall=')[1:]:
        wall = int(blk.split()[0])
        rows = re.findall(r'^\s*[0-9a-f]+: ((?:[0-9a-f]{2} )+)', blk, re.M)
        b = bytes.fromhex(''.join(r.replace(' ', '') for r in rows))
        i = b.find(b'<dict'); j = b.rfind(b'</dict>')
        if i < 0: continue
        if j < i: yield wall, b[i:].split(b'\0')[0].decode('latin1')   # longer than the 768-byte tracemem: truncated
        else: yield wall, b[i:j + 7].decode('latin1')

def val(e, ids):
    if 'IDREF' in e.attrib: return ids.get(e.attrib['IDREF'], '?')
    if e.tag == 'integer':
        v = int(e.text, 0); size = int(e.attrib.get('size', '32'))
        if size == 64: r = hex(v)
        else: r = str(v - (1 << 32) if v >= 1 << 31 else v)
    elif e.tag == 'array': r = '[' + ','.join(val(c, ids) for c in e) + ']'
    elif e.tag == 'dict':
        ks = list(e); r = '{' + ','.join(ks[k].text + ':' + val(ks[k + 1], ids) for k in range(0, len(ks) - 1, 2)) + '}'
    else: r = e.text or e.tag
    if 'ID' in e.attrib: ids[e.attrib['ID']] = r
    return r

def actions(path):
    out = []
    for l in open(path):
        p = l.split(None, 1)
        if len(p) == 2 and p[0].isdigit() and not p[1].startswith(('sleep', 'shot')): out.append((int(p[0]), p[1].strip()))
    return out

acts = actions(sys.argv[2])
for wall, x in records(sys.argv[1]):
    last = '-'
    for w, a in acts:
        if w <= wall: last = a
    ids = {}
    try:
        d = list(ET.fromstring(x))
        print(wall, '|', last, '|', ' '.join(d[k].text + ' ' + val(d[k + 1], ids) for k in range(0, len(d) - 1, 2)))
    except ET.ParseError:
        # truncated record: key + the integers that fit (IDREF resolved), then '...'
        key = re.search(r'<key>([^<]+)</key>', x); ids2 = {}; vals = []
        for m in re.finditer(r'<integer(?: ID="(\d+)")?(?: IDREF="(\d+)")?[^>]*?(?:/>|>([^<]*)</integer>)', x):
            if m.group(2): vals.append(ids2.get(m.group(2), '?')); continue
            v = int(m.group(3), 0); v = str(v - (1 << 32) if v >= 1 << 31 else v)
            if m.group(1): ids2[m.group(1)] = v
            vals.append(v)
        print(wall, '|', last, '|', (key.group(1) if key else '?'), '[' + ','.join(vals) + ',...]')
