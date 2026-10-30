# apply.py FILE NAME [VARIANTS.py]: apply variant NAME (default variants file: $S/v.py) to src/FILE.c.
import runpy,sys
import os
GW = os.environ.get('GW') or os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
V=runpy.run_path(sys.argv[3] if len(sys.argv)>3 else os.environ['S'] + '/v.py')["V"]
p=GW + "/src/"+sys.argv[1]+".c";s=open(p).read()
r=V[sys.argv[2]]
if isinstance(r,tuple): r=[r]
for o,n in r:
    assert o in s,o
    s=s.replace(o,n,1)
open(p,"w").write(s)
