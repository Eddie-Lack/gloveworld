#!/usr/bin/env python3
"""Compile a C file and compare one function against the base ROM."""
import argparse, glob, os, re, struct, subprocess, sys, tempfile
GW = os.environ.get('GW') or os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))

ROOT = GW
ROM_OFF, VRAM = 0x1000, 0x80100000

def load_syms():
    syms = {}
    for f in ("symbol_addrs.txt", "undefined_funcs_auto.txt"):
        p = os.path.join(ROOT, f)
        if os.path.exists(p):
            for m in re.finditer(r"^(\w+)\s*=\s*0x([0-9A-Fa-f]+);", open(p).read(), re.M):
                syms[m.group(1)] = int(m.group(2), 16)
    return syms

def func_starts():
    starts = set()
    for root, _, files in os.walk(os.path.join(ROOT, "asm")):
        for fn in files:
            if fn.endswith(".s"):
                for m in re.finditer(r"^\s*/\* [0-9A-F]+ ([0-9A-F]{8}) [0-9A-F]{8} \*/", "", re.M):
                    pass
    return starts

def compile_c(src, opt, extra):
    out = tempfile.mktemp(suffix=".o")
    sn = os.environ.get("SN_DIR", os.path.join(ROOT, "tools/gcc-2.7.2"))
    cmd = [os.path.join(sn, "gcc"), "-c", "-G0", "-mcpu=r4000", "-mips2", "-mmips-as",
           "-funsigned-char", "-Wa,-EB,-mips3,-mabi=32,-mgp32,-march=vr4300,-mfp32,-mno-shared,--no-pad-sections,-I" + os.path.join(ROOT, "include"), opt,
           "-I" + os.path.join(ROOT, "include"), "-I" + os.path.join(ROOT, "include/PR"),
           "-I" + os.path.join(ROOT, "src"), "-D_LANGUAGE_C", "-DF3DEX_GBI", "-D_FINALROM",
           "-DNDEBUG", "-DTARGET_N64"] + extra + [src, "-o", out]
    env = dict(os.environ, COMPILER_PATH=sn)
    r = subprocess.run(cmd, env=env, capture_output=True, text=True, cwd=ROOT)
    if r.returncode:
        sys.stderr.write(r.stdout + r.stderr); sys.exit(1)
    return out

def obj_func(obj, name):
    d = subprocess.run(["mips-linux-gnu-objdump", "-d", "-r", "-z", "--section=.text", obj],
                       capture_output=True, text=True).stdout
    lines, cur, relocs = [], None, {}
    for l in d.splitlines():
        m = re.match(r"^[0-9a-f]+ <(\S+)>:", l)
        if m:
            cur = m.group(1); continue
        if cur != name:
            continue
        m = re.match(r"^\s+([0-9a-f]+):\s+([0-9a-f]{8})\s+(.*)$", l)
        if m:
            lines.append([int(m.group(1), 16), int(m.group(2), 16), m.group(3), None])
        m = re.match(r"^\s+([0-9a-f]+): (R_MIPS_\w+)\s+(\S+)", l)
        if m and lines:
            lines[-1][3] = (m.group(2), m.group(3))
    return lines

def mask(ins, reloc):
    op = ins >> 26
    if op in (2, 3):
        return ins & 0xFC000000
    if reloc:
        return ins & 0xFFFF0000
    return ins

REGS = {"s8": "fp"}

def norm(t):
    t = t.replace("$", "")
    t = re.sub(r"\s+", " ", t.strip())
    t = re.sub(r"func_[0-9A-F]+|\.L[0-9A-F]+|\. \+ 4 \+ \(-?0x[0-9A-F]+ << 2\)", "<sym>", t)
    t = re.sub(r"0x([0-9A-Fa-f]+)", lambda m: str(int(m.group(1), 16)), t)
    t = re.sub(r"(lui|addiu|lw|sw|lh|lhu|sh|lb|lbu|sb|lwc1|swc1) (\w+), (-?\d+)\((\w+)\)$", lambda m: f"{m.group(1)} {m.group(2)}, <off>({m.group(4)})" if m.group(4) in ("at",) or m.group(1) == "lui" else m.group(0), t)
    t = re.sub(r"^lui (\w+), \d+$", r"lui \1, <hi>", t)
    t = t.replace("addu ", "move ") if re.match(r"^addu \w+, \w+, zero$", t) else t
    t = re.sub(r"^move (\w+), (\w+), zero$", r"move \1, \2", t)
    t = re.sub(r"^addiu (\w+), \1, -?\d+$", r"addiu \1, \1, <lo>", t)
    return t

def relmask(lines):
    """Mask load/store offsets whose base register was last set by lui (relocated)."""
    hi = set()
    out = []
    for t in lines:
        m = re.match(r"^(\w[\w.]*) (\w+), (-?\d+)\((\w+)\)$", t)
        if m and m.group(4) in hi:
            t = f"{m.group(1)} {m.group(2)}, <off>({m.group(4)})"
        m = re.match(r"^lui (\w+), ", t)
        dst = re.match(r"^\S+ (\w+)", t)
        if m:
            hi.add(m.group(1))
        elif dst and not re.match(r"^(sw|sh|sb|swc1|sdc1|b\w*|j\w*)\b", t):
            hi.discard(dst.group(1))
        out.append(t)
    return out

def norm_objdump(t):
    t = re.sub(r"\s+", " ", t.strip()).replace(",", ", ").replace(",  ", ", ")
    t = re.sub(r"^(beq|bne)zl (\S+), ", r"\1l \2, zero, ", t)
    t = t.replace("$", "")
    for k, v in REGS.items():
        t = re.sub(r"\b" + k + r"\b", v, t)
    t = re.sub(r"[0-9a-f]+ <[^>]+>", "<sym>", t)
    t = re.sub(r"^li (\w+), (-?\d+)$", r"addiu \1, zero, \2", t)
    t = re.sub(r"^(lui) (\w+), (0x)?[0-9a-f]+$", r"lui \2, <hi>", t)
    t = re.sub(r"^(lw|sw|lh|lhu|sh|lb|lbu|sb|lwc1|swc1|addiu) (\w+), (-?\d+)\((at)\)$", r"\1 \2, <off>(\4)", t)
    t = re.sub(r"0x([0-9a-f]+)", lambda m: str(int(m.group(1), 16)), t)
    t = re.sub(r"^addiu (\w+), \1, -?\d+$", r"addiu \1, \1, <lo>", t)
    return t

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("src"); ap.add_argument("func")
    ap.add_argument("--opt", default="-O2")
    ap.add_argument("--flags", default="")
    ap.add_argument("-v", action="store_true")
    ap.add_argument("--diff", action="store_true", help="aligned diff of mnemonics/operands")
    ap.add_argument("--score", action="store_true", help="print only the number of differing lines")
    ap.add_argument("--configs", action="store_true",
                    help="compare under GCC 2.7.2 and the SN64 2.7.2-970404 build")
    a = ap.parse_args()
    if a.configs:
        configs = [("gcc2.7.2", {}), ("sn64", {"SN_DIR": os.path.join(ROOT, "tools/sn64")})]
        out = []
        for name, env in configs:
            args = [sys.executable, __file__, a.src, a.func, "--opt=" + a.opt, "--flags=" + a.flags]
            r = subprocess.run(args, env=dict(os.environ, **env), capture_output=True, text=True)
            last = (r.stdout.strip().splitlines() or ["?"])[-1]
            out.append(f"{name}: {last.split(': ', 1)[-1]}")
        print(f"{a.func}: " + " | ".join(out))
        return
    syms = load_syms()
    addr = syms.get(a.func)
    if addr is None:
        m = re.match(r"func_([0-9A-F]{8})$", a.func)
        addr = int(m.group(1), 16)
    rom = open(os.environ.get("BASEROM", os.path.join(ROOT, "baserom.us.z64")), "rb").read()
    obj = compile_c(a.src, a.opt, a.flags.split())
    mine = obj_func(obj, a.func)
    os.unlink(obj)
    if not mine:
        print("function not found in object"); sys.exit(1)
    bad = 0
    import rabbitizer
    if a.diff or a.score:
        import difflib
        n = len(mine) + 64
        tgt = []
        for i in range(n):
            o = addr - VRAM + ROM_OFF + 4 * i
            w = struct.unpack(">I", rom[o:o + 4])[0]
            tgt.append(norm(rabbitizer.Instruction(w, addr + 4 * i).disassemble()))
        size = None
        for path in glob.glob(os.path.join(ROOT, "asm", "nonmatchings", "*", a.func + ".s")):
            m = re.search(r"^nonmatching " + re.escape(a.func) + r", (0x[0-9A-Fa-f]+)", open(path).read(), re.M)
            if m:
                size = int(m.group(1), 16) // 4
        if size:
            tgt = tgt[:size]
        mn = relmask([norm_objdump(t) for _, _, t, _ in mine])
        tgt = relmask(tgt)
        sm = difflib.SequenceMatcher(None, mn, tgt, autojunk=False)
        score = sum(max(i2 - i1, j2 - j1) for tag, i1, i2, j1, j2 in sm.get_opcodes() if tag != "equal")
        if a.score:
            print(f"{a.func}: score {score}")
            return
        C = int(os.environ.get('CTX', '4'))
        for tag, i1, i2, j1, j2 in sm.get_opcodes():
            if tag == "equal":
                continue
            print(f"--- {tag} mine[{i1}:{i2}] target[{j1}:{j2}]")
            for k in range(max(0, i1 - C), i1):
                print(f"    {mn[k]}")
            for k in range(i1, i2):
                print(f"  - {mn[k]}")
            for k in range(j1, j2):
                print(f"  + {tgt[k]}")
            for k in range(i2, min(len(mn), i2 + C)):
                print(f"    {mn[k]}")
        return
    for i, (off, ins, txt, rel) in enumerate(mine):
        o = addr - VRAM + ROM_OFF + 4 * i
        tgt = struct.unpack(">I", rom[o:o + 4])[0]
        ok = mask(ins, rel) == mask(tgt, rel or ((tgt >> 26) in (2, 3)))
        if not ok:
            bad += 1
        if a.v or not ok:
            t = rabbitizer.Instruction(tgt, addr + 4 * i).disassemble()
            print(f"{'  ' if ok else '!!'} {addr+4*i:08X} {txt:40s} | {t}")
    size = None
    for path in glob.glob(os.path.join(ROOT, "asm", "nonmatchings", "*", a.func + ".s")):
        m = re.search(r"^nonmatching " + re.escape(a.func) + r", (0x[0-9A-Fa-f]+)", open(path).read(), re.M)
        if m:
            size = int(m.group(1), 16) // 4
    if size is not None and size != len(mine):
        bad += 1
        print(f"size mismatch: target {size} insns")
    print(f"{a.func}: {len(mine)} insns, {bad} mismatches")
    sys.exit(1 if bad else 0)

main()
