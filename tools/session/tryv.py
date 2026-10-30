#!/usr/bin/env python3
# tryv.py FILE FN VARIANTS.py: score source variants of FN without touching src/FILE.c.
# VARIANTS.py defines V = {name: (old, new)} or {name: [(old, new), ...]}; each pair is a
# single-occurrence text replacement applied to src/FILE.c in a temporary copy.
import os, runpy, subprocess, sys
GW = os.environ.get('GW') or os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
f, fn, vp = sys.argv[1:4]
base = open(f"{GW}/src/{f}.c").read()
src = f"{GW}/src/_tryv_{os.getpid()}_{f}.c"
V = runpy.run_path(vp)["V"]
try:
    for k, reps in V.items():
        if isinstance(reps, tuple): reps = [reps]
        t = base
        ok = True
        for a, b in reps:
            if a not in t: print(k, "MISSING", a[:60]); ok = False; break
            t = t.replace(a, b, 1)
        if not ok: continue
        open(src, "w").write(t)
        r = subprocess.run(["python3", "tools/cmpfunc.py", src, fn, "--score"], capture_output=True, text=True, cwd=GW,
                           env=dict(os.environ, BASEROM=os.environ.get("B", "")))
        out = r.stdout + r.stderr
        sc = [l for l in out.split("\n") if "score" in l]
        print(k, sc[-1].split()[-1] if sc else out[-300:])
finally:
    if os.path.exists(src): os.remove(src)
