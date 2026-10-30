#!/usr/bin/env python3
# romdiff.py: after a failed build, print the number of differing ROM bytes, the first one and its object file.
import sys,re
import os
GW = os.environ.get('GW') or os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
a=open(os.environ['B'],'rb').read()
b=open(GW + '/build/NGVE.z64','rb').read()
d=[i for i in range(min(len(a),len(b))) if a[i]!=b[i]]
print(len(d),"diffs", len(a), len(b))
if d:
    i=d[0]; print("first", hex(i), "vram", hex(i-0x1000+0x80100000))
    # find object from map
    best=None
    for line in open(GW + '/build/NGVE.map'):
        m=re.match(r'\s+\.(\w+)\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(\S+\.o)',line)
        if m:
            v=int(m.group(2),16); s=int(m.group(3),16)
            if v<=i-0x1000+0x80100000<v+s: print(line.strip())
