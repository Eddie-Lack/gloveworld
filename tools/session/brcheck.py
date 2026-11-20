#!/usr/bin/env python3
# brcheck.py FILE FN: compare relative branch targets (masked by cmpfunc) between the draft and the ROM,
# over the instructions that cmpfunc's alignment pairs as equal. Prints mismatching branches.
import sys, os, re, struct, difflib, importlib.util
GW = os.environ.get('GW') or os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
G = GW
import types
cf = types.ModuleType('cf'); _src = open(G + '/tools/cmpfunc.py').read().rsplit('\nmain()', 1)[0]; cf.__dict__['__file__'] = G + '/tools/cmpfunc.py'; exec(compile(_src, 'cmpfunc', 'exec'), cf.__dict__)
import rabbitizer
f, fn = sys.argv[1], sys.argv[2]
src = f if f.endswith('.c') else '%s/src/%s.c' % (G, f)
syms = cf.load_syms(); addr = syms.get(fn) or (int(fn[5:], 16) if fn.startswith('func_') else None)
rom = open(os.environ['B'], 'rb').read()
obj = cf.compile_c(src, '-O2', [])
mine = cf.obj_func(obj, fn); os.unlink(obj)
base = mine[0][0]
size = None
import glob
for path in glob.glob(G + '/asm/nonmatchings/*/%s.s' % fn):
    m = re.search(r'^nonmatching %s, (0x[0-9A-Fa-f]+)' % re.escape(fn), open(path).read(), re.M)
    if m: size = int(m.group(1), 16) // 4
n = size or len(mine)
tw = [struct.unpack('>I', rom[addr - cf.VRAM + cf.ROM_OFF + 4*i: addr - cf.VRAM + cf.ROM_OFF + 4*i + 4])[0] for i in range(n)]
tt = cf.relmask([cf.norm(rabbitizer.Instruction(w, addr + 4*i).disassemble()) for i, w in enumerate(tw)])
mt = cf.relmask([cf.norm_objdump(t) for _, _, t, _ in mine])
def btarget(word, pc):
    op = word >> 26
    rt = (word >> 16) & 31
    if op in (4, 5, 6, 7, 20, 21, 22, 23) or (op == 1) or (op == 17 and ((word >> 21) & 31) == 8):
        off = word & 0xFFFF
        if off & 0x8000: off -= 0x10000
        return pc + 4 + off * 4
    return None
sm = difflib.SequenceMatcher(None, mt, tt, autojunk=False)
# map mine index -> target index for equal runs
mp = {}
for tag, i1, i2, j1, j2 in sm.get_opcodes():
    if tag == 'equal':
        for k in range(i2 - i1): mp[i1 + k] = j1 + k
bad = 0
for i, (off, w, txt, rel) in enumerate(mine):
    if i not in mp: continue
    j = mp[i]
    a = btarget(w, off - base)
    b = btarget(tw[j], 4 * j)
    if a is None or b is None: continue
    # compare targets through the mapping: mine target index -> target index
    ai, bi = a // 4, b // 4
    if mp.get(ai) != bi:
        R = lambda x: re.sub(r'\b(s\d|t\d|a\d|v\d|fp|f\d+)\b', 'R', x)
        if 0 <= ai < len(mt) and 0 <= bi < len(tt) and [R(x) for x in mt[ai:ai+3]] == [R(x) for x in tt[bi:bi+3]]:
            continue
        bad += 1
        if len(sys.argv) > 3:
            ms = ' | '.join(mt[ai:ai+3]) if 0 <= ai < len(mt) else '?'
            ts = ' | '.join(tt[bi:bi+3]) if 0 <= bi < len(tt) else '?'
            dm = mt[i+1] if i+1 < len(mt) else ''
            dt = tt[j+1] if j+1 < len(tt) else ''
            print('mine[%d] %s [%s] -> %s\n   tgt[%d] %s [%s] -> %s' % (i, mt[i], dm, ms, j, tt[j], dt, ts))
print('%s: %d branch target mismatches' % (fn, bad))
