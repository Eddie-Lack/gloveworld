#!/usr/bin/env python3
"""gname.py: rename global symbols (data or functions) everywhere.

    python3 tools/session/gname.py OLD=NEW [...]
    python3 tools/session/gname.py --file NAMES.tsv        lines: OLD<TAB>NEW[<TAB>...]

Renames the symbol in symbol_addrs.txt (adding an entry for an auto-named D_/func_ symbol, whose address is
in its name) and every whole-word use in src/ and include/. Then run `make setup` (splat regenerates asm/
with the new labels) and bld.
"""
import glob, os, re, sys

GW = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))


def main():
    pairs = []
    args = sys.argv[1:]
    if args[:1] == ['--file']:
        for line in open(args[1]):
            p = line.rstrip('\n').split('\t')
            if len(p) >= 2 and not line.startswith('#') and p[1].strip():
                pairs.append((p[0].strip(), p[1].strip()))
    else:
        pairs = [tuple(a.split('=', 1)) for a in args]
    files = glob.glob(os.path.join(GW, 'src', '*.c')) + glob.glob(os.path.join(GW, 'include', '*.h'))
    texts = {f: open(f).read() for f in files}
    symp = os.path.join(GW, 'symbol_addrs.txt')
    sym = open(symp).read()
    problems = []
    for old, new in pairs:
        if re.search(r'\b' + new + r'\b', sym) or any(re.search(r'\b' + new + r'\b', t) for t in texts.values()):
            problems.append(f'{old}: {new} is already used')
            continue
        m = re.search(r'^' + old + r' = (0x[0-9A-Fa-f]+);(.*)$', sym, re.M)
        if m:
            sym = sym[:m.start()] + f'{new} = {m.group(1)};{m.group(2)}' + sym[m.end():]
        else:
            a = re.fullmatch(r'(?:D|func)_([0-9A-F]{8})', old)
            if not a:
                problems.append(f'{old}: not in symbol_addrs.txt and not an auto name')
                continue
            sym = sym.rstrip('\n') + f'\n{new} = 0x{a.group(1)};' + (' // type:func' if old.startswith('func_') else '') + '\n'
        n = 0
        for f in files:
            texts[f], k = re.subn(r'\b' + old + r'\b', new, texts[f])
            n += k
        print(f'{old} -> {new}: {n} uses')
    open(symp, 'w').write(sym)
    for f, t in texts.items():
        if t != open(f).read():
            open(f, 'w').write(t)
    for p in problems:
        print('PROBLEM', p)
    return 1 if problems else 0


if __name__ == '__main__':
    sys.exit(main())
