#!/usr/bin/env python3
# explain.py FILE FN [VARIANTS.py NAME]: link each difference against the ROM to the compiler decision behind it.
# Needs the traced compiler (tools/session/gtrace-build). Applies an optional tryv variant first.
import collections, difflib, os, re, runpy, subprocess, sys, tempfile

GW = os.environ.get('GW') or os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
TC = os.environ.get('GTRACE_TC') or os.path.join(GW, 'tools', 'gcc-trace')
f, fn = sys.argv[1:3]

GPR = 'zero at v0 v1 a0 a1 a2 a3 t0 t1 t2 t3 t4 t5 t6 t7 s0 s1 s2 s3 s4 s5 s6 s7 t8 t9 k0 k1 gp sp fp ra'.split()
def regname(n):
    return GPR[n] if n < 32 else f'f{n - 32}' if n < 64 else f'r{n}'
def regnum(name):
    return GPR.index(name) if name in GPR else 32 + int(name[1:]) if re.fullmatch(r'f\d+', name) else -1
def asmreg(t):  # '$2' -> 'v0', '$f6' -> 'f6', '$sp' -> 'sp'
    t = t.lstrip('$')
    return GPR[int(t)] if t.isdigit() else t

# 1. the source, with the variant applied
src = open(f'{GW}/src/{f}.c').read()
if len(sys.argv) > 4:
    reps = runpy.run_path(sys.argv[3])['V'][sys.argv[4]]
    for a, b in ([reps] if isinstance(reps, tuple) else reps):
        assert a in src, a[:60]
        src = src.replace(a, b, 1)
work = tempfile.mkdtemp(prefix='explain_')
open(f'{work}/{f}.c', 'w').write(src)

# 2. compile with traces, insn numbers in the asm and the allocation dumps
flags = (f'-G0 -mcpu=r4000 -mips2 -mmips-as -funsigned-char -O2 -I{GW}/include -I{GW}/include/PR -I{GW}/src -I{GW} '
         '-D_LANGUAGE_C -DF3DEX_GBI -D_FINALROM -DNDEBUG -DTARGET_N64 -g -dp -dl -dg -S').split()
r = subprocess.run([f'{TC}/gcc'] + flags + [f'{f}.c', '-o', f'{f}.s'], cwd=work, capture_output=True, text=True,
                   env=dict(os.environ, COMPILER_PATH=TC, GTRACE='1', GTRACE_FN=fn))
trace = [l for l in r.stderr.split('\n') if re.match(r'(LA|GA|SCHED[12]|PRI[12]|ADJ|PREF|DBR|LIVE|THREAD|XJUMP|EAGER) ', l)]

def section(path):
    t = open(path).read()
    i = t.find(f';; Function {fn}\n')
    j = t.find(';; Function ', i + 10)
    return t[i:j if j > 0 else None]
lreg, greg = section(f'{work}/{f}.c.lreg'), section(f'{work}/{f}.c.greg')
disp = {int(a): int(b) for a, b in re.findall(r'(\d+) in (\d+)', greg.split(';; Register dispositions:')[1].split(';;')[0])}
uid_pseudos = collections.defaultdict(set)
uid_line = {}

# 3. map each of our machine instructions (by index in the function) to its insn number: align the -dp
#    assembly with the assembler's listing, which keeps the line order and addresses but not comments
asm = open(f'{work}/{f}.s').read()
body = asm[asm.find(f'\n{fn}:'):asm.find(f'.end\t{fn}')]
dp = []  # (text, uid)
cur = None
for l in body.split('\n'):
    ln = re.search(rf'#\s+{re.escape(f)}\.c:(\d+)', l)
    if ln:
        cur = int(ln.group(1))
        continue
    m = re.match(r'\s+([a-z][\w.]*)(\s+[^#]*)?', l)
    u = re.findall(r'#\s+(\d+) [a-z_]', l)
    if m and u:
        dp.append((' '.join((m.group(1) + (m.group(2) or '')).split()), int(u[-1])))
        uid_line.setdefault(int(u[-1]), cur)
subprocess.run([f'{GW}/tools/gcc-2.7.2/as', '-EB', '-mips3', '-mabi=32', '-mgp32', '-march=vr4300', '-mfp32',
                '-mno-shared', '--no-pad-sections', '-Iinclude', f'-al={work}/lst.txt', f'{work}/{f}.s',
                '-o', f'{work}/{f}.o'], cwd=GW, capture_output=True)
lst, sect, prev, inside, start = [], 'text', 'text', False, None  # lst: (text, [addresses])
for l in open(f'{work}/lst.txt', errors='replace'):
    m = re.match(r'\s*\d+ ([0-9a-f]{4}) ([0-9A-F]{8})\s*\t?(.*)$', l.rstrip('\n'))
    c = re.match(r'\s*\d*\s+([0-9a-f]{4}) ([0-9A-F]{8})\s*$', l.rstrip('\n'))
    t = l.split('\t', 1)[1].strip() if '\t' in l else ''
    if t == f'{fn}:': inside = True
    if inside and t.startswith(f'.end {fn}'): break
    if t.startswith('.section') or t in ('.text', '.data', '.rdata'):
        prev, sect = sect, 'text' if '.text' in t else 'other'
    elif t.startswith('.previous'):
        prev, sect = sect, prev
    if not inside or sect != 'text': continue
    if m and m.group(3).strip():
        lst.append((' '.join(m.group(3).split()), [int(m.group(1), 16)]))
    elif c and lst:
        lst[-1][1].append(int(c.group(1), 16))
index_uid = {}
if lst:
    start = lst[0][1][0]
    sm = difflib.SequenceMatcher(None, [x.split()[0] for x, _ in dp], [x.split()[0] for x, _ in lst], autojunk=False)
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        for k in range(j2 - j1):
            u = dp[min(i1 + k, i2 - 1)][1] if i2 > i1 else (dp[i1 - 1][1] if i1 else None)
            for addr in lst[j1 + k][1]:
                index_uid[(addr - start) // 4] = u

# 4. the aligned diff against the ROM: for each changed line, our index (if ours) and its text
diff = subprocess.run(['python3', f'{GW}/tools/session/cmpctx.py', f'{work}/{f}.c', fn, '--diff'], cwd=GW,
                      capture_output=True, text=True, env=dict(os.environ, CTX='0')).stdout
pairs, moved = [], []  # pairs: (our index, our text, ROM text); moved: (our index or None, text)
blk = None
def flush():
    if not blk: return
    k, i0, ms, rs = blk
    if k == 'delete':
        moved.extend((i0 + n, x) for n, x in enumerate(ms))
    elif k == 'insert':
        moved.extend((None, x) for x in rs)
    else:
        sm = difflib.SequenceMatcher(None, [x.split()[0] for x in ms], [x.split()[0] for x in rs], autojunk=False)
        for tag, i1, i2, j1, j2 in sm.get_opcodes():
            if tag == 'equal':
                pairs.extend((i0 + i1 + n, ms[i1 + n], rs[j1 + n]) for n in range(i2 - i1))
            else:
                moved.extend([(i0 + n, ms[n]) for n in range(i1, i2)] + [(None, x) for x in rs[j1:j2]])
for l in diff.split('\n'):
    if l.startswith('---'):
        flush()
        m = re.search(r'mine\[(\d+):', l)
        blk = (l.split()[1], int(m.group(1)) if m else 0, [], [])
    elif l.startswith('  - ') and blk:
        blk[2].append(l[4:].strip())
    elif l.startswith('  + ') and blk:
        blk[3].append(l[4:].strip())
flush()

def tr(pseudo):
    out = [l for l in trace if re.search(rf'(regs( \d+)* {pseudo}( |$)|reg {pseudo} )', l)]
    return out or [f'(pseudo {pseudo}: no allocation line)']
def block_of(u):
    for l in trace:
        if l.startswith('SCHED1') and re.search(rf' {u}( |$)', l.split(':', 1)[1] + ' '):
            return int(l.split(' block ')[1].split(':')[0])

print(f'== {f} {fn}: {len(pairs)} changed lines with the same opcode, {len(moved)} moved or extra\n')
for i, a, b in pairs:
    ta, tb = a.replace(',', ' ').split(), b.replace(',', ' ').split()
    diffs = [(x, y) for x, y in zip(ta[1:], tb[1:]) if x != y and re.fullmatch(r'[a-z]+\d*|f\d+', x)]
    if not diffs: continue
    u = index_uid.get(i)
    print(f'[{i}] insn {u} (line {uid_line.get(u)}): "{a}"  ROM "{b}"')
    for x, y in diffs:
        for p in sorted(uid_pseudos.get(u, ())):
            if regname(disp.get(p, -1)) == x:
                print(f'  {x} (ROM {y}) holds pseudo {p}:')
                for l in tr(p): print('    ' + l)
        b0 = block_of(u)
        same = [l for l in trace if l.startswith('LA ') and b0 is not None
                and int(l.split(' block ')[1].split()[0]) == b0 and l.endswith(f'-> {regnum(y)}')]
        for l in same: print(f'  also in {y} in this block: {l}')
    print()

shown = set()
for i, t in moved:
    u = index_uid.get(i) if i is not None else None
    if u is None or u in shown: continue
    shown.add(u)
    print(f'[{i}] insn {u} (line {uid_line.get(u)}) moved or extra: "{t}"')
    for l in trace:
        if l.startswith(('PRI', 'SCHED', 'ADJ')) and re.search(rf' {u}(/| |$)', l.split(':', 1)[-1] + ' '):
            print('    ' + l)
print(f'\nFull trace: {len(trace)} lines; work folder {work}')
