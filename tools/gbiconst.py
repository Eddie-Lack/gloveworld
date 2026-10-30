#!/usr/bin/env python3
"""Find gbi macro names whose value matches a display list word.

Usage: tools/gbiconst.py rm  W0 W1     (render mode: gDPSetRenderMode words)
       tools/gbiconst.py cc  W0 W1     (combine mode: gDPSetCombineMode words)
"""
import itertools, os, re, subprocess, sys, tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GBI = open(os.path.join(ROOT, "include/PR/gbi.h")).read()
GCC = os.path.join(ROOT, "tools/gcc-2.7.2")
CC = [os.path.join(GCC, "gcc"), "-S", "-o", "-", "-I" + os.path.join(ROOT, "include"), "-DF3DEX_GBI", "-D_LANGUAGE_C"]
ENV = dict(os.environ, COMPILER_PATH=GCC)
kind, w0, w1 = sys.argv[1], int(sys.argv[2], 16), int(sys.argv[3], 16)

def evaluate(exprs):
    src = '#include "ultra64.h"\n' + "".join(f"unsigned int v{i} = {e};\n" for i, e in enumerate(exprs))
    with tempfile.TemporaryDirectory() as d:
        c = os.path.join(d, "t.c")
        open(c, "w").write(src)
        out = subprocess.run(CC + [c], capture_output=True, text=True, env=ENV)
        vals = re.findall(r"\.word\s+(-?\d+)", out.stdout)
        return [int(v) & 0xFFFFFFFF for v in vals]

if kind == "rm":
    names = sorted(set(re.findall(r"#define\s+(G_RM_\w+)\s", GBI)))
    names = [n for n in names if not n.endswith("2")]
    vals = evaluate(names + [n + "2" for n in names if (n + "2") in GBI])
    one = dict(zip(names, vals[:len(names)]))
    two = dict(zip([n + "2" for n in names if (n + "2") in GBI], vals[len(names):]))
    for a, va in one.items():
        for b, vb in list(two.items()) + list(one.items()):
            if (va | vb) == w1:
                print(a, b)
else:
    names = sorted(set(n for n, v in re.findall(r"#define\s+(G_CC_\w+)\s+([^\n]*)", GBI) if v.count(",") == 7))
    ok = []
    for n in names:
        with tempfile.TemporaryDirectory() as d:
            c = os.path.join(d, "t.c")
            open(c, "w").write(f'#include "ultra64.h"\nGfx g[] = {{ gsDPSetCombineMode({n}, {n}) }};\n')
            if subprocess.run(CC + [c], capture_output=True, text=True, env=ENV).returncode == 0:
                ok.append(n)
    names = ok
    src_names = list(itertools.product(names, names))
    src = '#include "ultra64.h"\nGfx g[] = {\n' + "".join(f"gsDPSetCombineMode({a}, {b}),\n" for a, b in src_names) + "};\n"
    with tempfile.TemporaryDirectory() as d:
        c = os.path.join(d, "t.c")
        open(c, "w").write(src)
        out = subprocess.run(CC + [c], capture_output=True, text=True, env=ENV)
        vals = [int(v) & 0xFFFFFFFF for v in re.findall(r"\.word\s+(-?\d+)", out.stdout)]
    for i, (a, b) in enumerate(src_names):
        if vals[2 * i] == w0 and vals[2 * i + 1] == w1:
            print(a, b)
