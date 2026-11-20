#!/usr/bin/env python3
# jcheck.py SRC FN: compare the targets of FN's `j` instructions with the ROM's. cmpfunc and cmpctx mask
# jump targets (they need a relocation in the object). A jump to the wrong label scores 0 there.
# Instructions are aligned by opcode. The check also works on a function that is a few instructions off.
import difflib, os, struct, sys, types
GW = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
code = open(GW + '/tools/session/cmpctx.py').read().rsplit('\nif __name__', 1)[0].rsplit('\nmain()', 1)[0]
cm = types.ModuleType('cm'); cm.__file__ = GW + '/tools/session/cmpctx.py'; exec(code, cm.__dict__)
src, fn = sys.argv[1:3]
addr = cm.load_syms()[fn]
rom = open(os.environ.get('BASEROM', os.path.join(GW, 'baserom.us.z64')), 'rb').read()
mine = cm.obj_func(cm.compile_c(src, '-O2', []), fn)
base = mine[0][0]
tgt = [struct.unpack('>I', rom[addr - cm.VRAM + cm.ROM_OFF + 4 * i:][:4])[0] for i in range(len(mine) + 64)]
ours_t = {}  # our index -> target index within the function
for i, (off, w, _, rel) in enumerate(mine):
    if w >> 26 == 2 and rel and rel[1].startswith('.text'):
        ours_t[i] = (((w & 0x3FFFFFF) << 2) - base) // 4
rom_t = {i: (((w & 0x3FFFFFF) << 2) - (addr & 0x0FFFFFFF)) // 4 for i, w in enumerate(tgt) if w >> 26 == 2}
sm = difflib.SequenceMatcher(None, [w >> 26 for _, w, _, _ in mine], [w >> 26 for w in tgt], autojunk=False)
amap = {}
for a, b, n in sm.get_matching_blocks():
    for k in range(n): amap[a + k] = b + k
bad = 0
for i, t in sorted(ours_t.items()):
    j = amap.get(i)
    if j is None or j not in rom_t: continue
    if amap.get(t) != rom_t[j]:
        bad += 1
        print(f'[{i}] j -> our [{t}] (ROM index {amap.get(t)}), ROM [{j}] j -> [{rom_t[j]}]')
print(f'{fn}: {len(ours_t)} jumps checked, {bad} with a different target')
