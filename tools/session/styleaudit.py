# usage: styleaudit.py IOBJ > report.md   (IOBJ is the path of the PC port's Glover.iobj; needs a built build/NGVE.elf)
# Groups the source files by the habits of their original authors, then ranks matched
# functions by matching tricks that are rare in their group. See docs/authors.md.
import collections, glob, json, math, os, re, struct, subprocess, sys

GW = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
if len(sys.argv) != 2:
    sys.exit(__doc__ or "usage: styleaudit.py IOBJ > report.md")
IOBJ = sys.argv[1]
K = 4

# 1. Original local names, from the CodeView records in the PC port's object file.
#    Locals of inlined callees are skipped.
def read_iobj(path):
    d = open(path, 'rb').read()
    _, nsec, _, _, _, opt, _ = struct.unpack_from('<HHIIIHH', d, 0)
    procs = {}
    for s in range(nsec):
        o = 20 + opt + 40 * s
        size, ptr = struct.unpack_from('<II', d, o + 16)
        if d[o:o + 8].rstrip(b'\0') != b'.debug$S' or not ptr:
            continue
        sec = d[ptr:ptr + size]
        if struct.unpack_from('<I', sec, 0)[0] != 4:
            continue
        p = 4
        while p + 8 <= len(sec):
            t, l = struct.unpack_from('<II', sec, p)
            p += 8
            if t == 0xF1:
                q, stack, cur = p, [], None
                while q + 4 <= p + l:
                    rl, k = struct.unpack_from('<HH', sec, q)
                    body = sec[q + 4:q + 2 + rl]
                    q += 2 + rl
                    if k in (0x1146, 0x1147, 0x110F, 0x1110):
                        nm = body[35:].split(b'\0')[0].decode('latin1')
                        cur = procs.setdefault(nm, {'locals': [], 'statics': []})
                        stack.append('P')
                    elif k in (0x114D, 0x1103):
                        stack.append('I' if k == 0x114D else 'B')
                    elif k in (0x114E, 0x114F, 0x0006):
                        if stack and stack.pop() == 'P':
                            cur = None
                    elif k == 0x113E and cur is not None and 'I' not in stack:
                        ty, fl = struct.unpack_from('<IH', body, 0)
                        cur['locals'].append((body[6:].split(b'\0')[0].decode('latin1'), ty, fl & 1))
                    elif k in (0x110C, 0x1153) and cur is not None:
                        cur['statics'].append(body[10:].split(b'\0')[0].decode('latin1'))
            p += (l + 3) & ~3
    return procs

procs = read_iobj(IOBJ)
glob_names = set(l.split('=')[0].strip() for l in open(os.path.join(GW, 'symbol_addrs.txt')) if '=' in l)

# 2. Functions per file from the map, and their machine code from the ELF.
fnfile = {}
cur = None
for l in open(os.path.join(GW, 'build', 'NGVE.map')):
    m = re.match(r'^ \.text +0x80[0-9a-f]+ +0x[0-9a-f]+ build/src/(\w+)\.o', l)
    if m:
        cur = m.group(1)
        continue
    if l.startswith(' .'):
        cur = None
    m = re.match(r'^ +0x80[0-9a-f]+ +([A-Za-z_]\w*)$', l)
    if cur and m:
        fnfile[m.group(1)] = cur
dis = subprocess.run(['mips-linux-gnu-objdump', '-d', '--no-show-raw-insn', '-M', 'no-aliases',
                      os.path.join(GW, 'build', 'NGVE.elf')], capture_output=True, text=True).stdout
code = collections.defaultdict(list)
cur = None
for l in dis.split('\n'):
    m = re.match(r'^[0-9a-f]{8} <(\w+)>:', l)
    if m:
        cur = m.group(1) if m.group(1) in fnfile else None
    elif cur and '\t' in l:
        parts = l.split('\t')
        code[cur].append((parts[1].strip(), parts[2] if len(parts) > 2 else ''))

# 3. Pool the habits of each file. Names come from the iobj; the rest are facts of the ROM.
pool = collections.defaultdict(collections.Counter)
for fn, f in fnfile.items():
    c = pool[f]
    c['funcs'] += 1
    for op, args in code[fn]:
        c['n'] += 1
        c['dbl'] += op.endswith('.d') or op in ('ldc1', 'sdc1')
        c['sgl'] += op.endswith('.s')
        c['sinlib'] += op == 'jal' and bool(re.search(r'<__(sin|cos)f>', args))
        c['sinbi'] += op == 'jal' and bool(re.search(r'<(sin|cos)f>', args))
        c['half'] += op in ('lh', 'lhu', 'sh') or (op == 'sra' and args.endswith(',0x10'))
        c['word'] += op in ('lw', 'sw')
        c['byte'] += op in ('lb', 'lbu', 'sb')
    cv = procs.get(fn)
    if not cv:
        continue
    c['cvf'] += 1
    c['stat'] += len(cv['statics'])
    seen = set()
    for nm, ty, par in cv['locals']:
        if nm in glob_names or nm in cv['statics'] or nm.startswith('$') or nm in seen:
            continue
        seen.add(nm)
        c['names'] += 1
        c['camel'] += bool(re.match(r'^[a-z]+[A-Z]', nm))
        c['under'] += '_' in nm
        c['cap'] += nm[0].isupper()
        c['dbl2'] += bool(re.match(r'^([a-z])\1$', nm))
        c['temp'] += nm.lower().startswith(('temp', 'tmp'))
        c['ptr'] += 'ptr' in nm.lower()
        c['num'] += bool(re.search(r'[a-zA-Z]\d$', nm))
        c['one'] += len(nm) == 1
        c['len'] += len(nm)

def habits(c):
    k = max(1, c['names'])
    fl, sn = c['dbl'] + c['sgl'], c['sinlib'] + c['sinbi']
    return {'camelCase names': c['camel'] / k, 'snake_case names': c['under'] / k,
            'Capitalised names': c['cap'] / k, 'doubled-letter names (ww, xx)': c['dbl2'] / k,
            'temp... names': c['temp'] / k, '...ptr names': c['ptr'] / k, 'numbered names (temp2)': c['num'] / k,
            'one-letter names': c['one'] / k, 'name length': c['len'] / k,
            'static locals': c['stat'] / max(1, c['cvf']),
            'double maths': c['dbl'] / fl if fl >= 20 else None,
            '__sinf over sinf': c['sinlib'] / sn if sn else None,
            '16-bit values': c['half'] / max(1, c['word'] + c['half'] + c['byte']),
            'function size': math.log(c['n'] / c['funcs'])}

files = sorted(f for f in pool if pool[f]['names'] >= 8)
H = {f: habits(pool[f]) for f in files}
keys = list(H[files[0]])
Z = {f: [] for f in files}
for k in keys:  # rank-normalise each habit to 0..1; unknown is 0.5
    vals = sorted(H[f][k] for f in files if H[f][k] is not None)
    for f in files:
        v = H[f][k]
        Z[f].append(0.5 if v is None else (sum(x < v for x in vals) + 0.5 * sum(x == v for x in vals)) / len(vals))

def ward(Z, n):
    dim = len(next(iter(Z.values())))
    def sse(fs):
        c = [sum(Z[f][i] for f in fs) / len(fs) for i in range(dim)]
        return sum((Z[f][i] - c[i]) ** 2 for f in fs for i in range(dim))
    C = [[f] for f in files]
    while len(C) > n:
        _, i, j = min((sse(C[i] + C[j]) - sse(C[i]) - sse(C[j]), i, j)
                      for i in range(len(C)) for j in range(i + 1, len(C)))
        C[i] += C.pop(j)
    return C

groups = sorted(ward(Z, K), key=len, reverse=True)
gof = {f: g for g, fs in enumerate(groups) for f in fs}
lab = gof
stab = collections.Counter()
for drop in range(len(keys)):  # how often a file stays with its group when one habit is left out
    l = {f: g for g, fs in enumerate(ward({f: [v for i, v in enumerate(Z[f]) if i != drop] for f in files}, K)) for f in fs}
    for f in files:
        same = [x for x in groups[gof[f]] if x != f]
        stab[f] += sum(l[x] == l[f] for x in same) / max(1, len(same)) / len(keys)

# 4. Matching tricks in our C, per function.
TYPES = (r'(?:const\s+|static\s+|register\s+|volatile\s+|unsigned\s+|struct\s+)*'
         r'(?:s8|u8|s16|u16|s32|u32|s64|u64|f32|f64|int|char|short|long|float|double|[A-Z]\w*)(?:\s*\*+\s*|\s+)')
TRICKS = {  # name: (weight, pattern or None)
    'steering comment': (3, r'^\s*//|/\*'),
    'do/while (0)': (3, None),
    'flag local': (2, r'^\s*\w+\s*=\s*[^;=]*[=!<>]=[^;]*;'),
    'step-by-step updates': (2, None),
    'register': (2, r'\bregister\b'),
    'goto': (1, r'\bgoto\b'),
    '(&x)->y': (1, r'\(&[\w.\[\]]+\)->'),
    'initialised local': (1, r'^\s*' + TYPES + r'\w+\s*=\s*(?:NULL|0|0\.0f?)\s*;'),
    'cast': (0.5, r'\((?:f32|f64|s16|u16|s8|u8|s32|u32)\)'),
    'more locals than the original': (1, None),
}
found = []
for path in sorted(glob.glob(os.path.join(GW, 'src', '*.c'))):
    file = os.path.basename(path)[:-2]
    txt = open(path).read()
    dw = set(re.findall(r'#define\s+(\w+)\([^)]*\)[^\n]*(?:\\\n[^\n]*)*?while \(0\)', txt))
    lines = txt.split('\n')
    i = 0
    while i < len(lines):
        m = re.match(r'^(?!static\s+inline|extern)[\w\s*]+?\b(\w+)\([^;]*\)\s*\{\s*$', lines[i])
        if not m or m.group(1) not in fnfile:
            i += 1
            continue
        j = i + 1
        while j < len(lines) and lines[j] != '}':
            j += 1
        fn, body = m.group(1), lines[i + 1:j]
        b = '\n'.join(body)
        hits = {t: len(re.findall(p, b, re.M)) for t, (_, p) in TRICKS.items() if p}
        hits['do/while (0)'] = len(re.findall(r'\}\s*while\s*\(0\)', b)) + sum(len(re.findall(r'\b%s\(' % d, b)) for d in dw)
        run = steps = 0
        st = [re.match(r'^\s*(\w+)\s*[-+*/|&]?=', l) for l in body] + [None]
        for a, c in zip(st, st[1:]):
            if a and c and a.group(1) == c.group(1):
                run += 1
            else:
                steps += run >= 2
                run = 0
        hits['step-by-step updates'] = steps
        decl = set(re.findall(r'^\s+' + TYPES + r'([A-Za-z_]\w*)\s*(?:\[[^\]]*\])?\s*(?:=[^;]*)?;', b, re.M))
        cv = procs.get(fn)
        orig = set(n for n, t, p in cv['locals'] if not p and n not in glob_names and not n.startswith('$')) - set(cv['statics']) if cv else set()
        hits['more locals than the original'] = max(0, len(decl) - len(orig)) if orig else 0
        found.append((fn, file, hits, sorted(decl - orig) if orig else []))
        i = j

freq = collections.Counter()
size = collections.Counter()
for fn, f, hits, _ in found:
    g = gof.get(f)
    size[g] += 1
    for t, n in hits.items():
        freq[g, t] += n > 0
scored = []
for fn, f, hits, extra in found:
    g = gof.get(f)
    s = sum(TRICKS[t][0] * min(n, 3) * (1 - freq[g, t] / size[g]) for t, n in hits.items())
    scored.append((round(s, 1), fn, f, hits, extra))
scored.sort(key=lambda x: -x[0])

# 5. Report.
print('# Style audit\n')
for g, fs in enumerate(groups):
    cen = [sum(Z[f][i] for f in fs) / len(fs) for i in range(len(keys))]
    marks = sorted(range(len(keys)), key=lambda i: -abs(cen[i] - 0.5))[:4]
    print(f"## Group {g + 1}: {len(fs)} files\n")
    print(', '.join(f"{f} ({stab[f]:.0%})" for f in sorted(fs, key=lambda f: -stab[f])) + '\n')
    print('Most typical: ' + '; '.join(f"{'more' if cen[i] > 0.5 else 'fewer'} {keys[i]}" for i in marks) + '\n')
print('## Functions to review (rarest tricks first)\n')
print('| Score | Function | File | Group | Tricks | Locals not in the iobj |')
print('| --- | --- | --- | --- | --- | --- |')
for s, fn, f, hits, extra in scored[:40]:
    t = ', '.join(f"{k}{' x%d' % n if n > 1 else ''}" for k, n in hits.items() if n)
    print(f"| {s} | {fn} | {f} | {gof[f] + 1 if f in gof else '-'} | {t} | {' '.join(extra[:8])} |")
