#!/usr/bin/env python3
"""rename.py: rename struct fields everywhere, using the typed accesses from xref.py.

    python3 tools/session/rename.py STRUCT.FIELD=NEWNAME [...]
    python3 tools/session/rename.py --file NAMES.tsv     lines: STRUCT<TAB>FIELD<TAB>NEWNAME[<TAB>...]

Only accesses that xref.py typed as STRUCT are renamed. Other structs' fields with the same name are left
alone. A rename that misses an access, or hits the wrong one, stops the build (the old or new name is not a
field of that struct). Run bld afterwards. Accesses that come from a macro body are listed for manual
work. Rebuilds the cross-reference first (fields renamed in an earlier call must be found under their new names).
"""
import collections, os, re, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
GW = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, HERE)
import xref  # noqa: E402


def main():
    pairs = []
    args = sys.argv[1:]
    if args[:1] == ['--file']:
        for line in open(args[1]):
            p = line.rstrip('\n').split('\t')
            if len(p) >= 3 and not line.startswith('#') and p[2].strip():
                pairs.append((p[0].strip(), p[1].strip(), p[2].strip()))
    else:
        for a in args:
            m = re.fullmatch(r'([\w.]+)\.(\w+)=(\w+)', a)
            pairs.append(m.groups())
    xref.build()
    d = xref.load()
    edits = collections.defaultdict(list)  # path -> [(line, col or None, old, new)]
    problems = []
    by_line = collections.defaultdict(list)  # (file, line, fieldname) -> accesses of any struct
    for a in d['accesses']:
        by_line[(a['file'], a['line'], a['field'])].append(a)
    for struct, old, new in pairs:
        s = d['structs'].get(struct)
        if not s or old not in s['fields']:
            problems.append(f'{struct}.{old}: no such field')
            continue
        if new in s['fields']:
            problems.append(f'{struct}.{old}: {new} already exists')
            continue
        f, line = s['fields'][old]['decl']
        edits[f].append((line, 'decl', old, new))
        for a in d['accesses']:
            if a['struct'] == struct and a['field'] == old:
                edits[a['file']].append((a['line'], a['col'], old, new, struct))
    for f, es in edits.items():
        path = os.path.join(GW, f)
        lines = open(path).read().split('\n')
        per_line = collections.defaultdict(list)
        for e in es:
            per_line[e[0]].append(e)
        for ln, items in per_line.items():
            text = lines[ln - 1]
            for item in items:
                if item[1] == 'decl':
                    _, _, old, new = item
                    new_text, n = re.subn(r'\b' + old + r'\b(?=\s*(\[[^\]]*\]\s*)*[;:,])', new, text, count=1)
                    if n != 1:
                        problems.append(f'{f}:{ln}: declaration of {old} not found: {text.strip()}')
                    text = new_text
            accs = [i for i in items if i[1] != 'decl']
            for old in {i[2] for i in accs}:
                new_at = {i[1]: i[3] for i in accs if i[2] == old}  # column -> new name (structs may differ)
                new = next(iter(new_at.values()))
                mine = sorted(new_at)
                everyone = sorted(by_line[(f, ln, old)], key=lambda a: a['col'])
                spots = [m.start(2) for m in re.finditer(r'(\.|->)\s*(' + old + r')\b', text)]
                if not spots and all(a['struct'] == accs[0][4] for a in everyone):
                    # the access is on a continuation line of a call that starts here, or in a macro body
                    near = [k for k in range(ln, min(ln + 6, len(lines)))
                            if re.search(r'(\.|->)\s*' + old + r'\b', lines[k])]
                    macro = [k for k, l in enumerate(lines) if re.search(r'(\.|->)\s*' + old + r'\b', l)
                             and (l.rstrip().endswith('\\') or (k and lines[k - 1].rstrip().endswith('\\')))]
                    for k in (near[:1] or macro):
                        lines[k] = re.sub(r'((\.|->)\s*)' + old + r'\b', r'\g<1>' + new, lines[k])
                    if near or macro:
                        continue
                if len(spots) == len(everyone):
                    keep = [sp for sp, a in zip(spots, everyone) if a['col'] in mine]
                elif all(a['struct'] == accs[0][4] for a in everyone) and spots:
                    keep = spots  # macro use on this line: every access here is ours
                else:
                    problems.append(f'{f}:{ln}: {old} has {len(spots)} uses in the text and {len(everyone)} '
                                    f'parsed accesses; edit by hand: {text.strip()[:120]}')
                    continue
                names = {sp: new_at.get(a['col'], new) for sp, a in zip(spots, everyone)} if len(spots) == len(everyone) else {}
                for sp in sorted(keep, reverse=True):
                    text = text[:sp] + names.get(sp, new) + text[sp + len(old):]
            lines[ln - 1] = text
        open(path, 'w').write('\n'.join(lines))
    print(f'renamed {len(pairs)} fields')
    for p in problems:
        print('PROBLEM', p)
    return 1 if problems else 0


if __name__ == '__main__':
    sys.exit(main())
