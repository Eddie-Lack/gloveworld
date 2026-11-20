#!/usr/bin/env python3
"""lname.py: rename local variables inside one function.

    python3 tools/session/lname.py FILE FUNCTION OLD=NEW [...]
    python3 tools/session/lname.py --file NAMES.tsv     lines: FILE<TAB>FUNCTION<TAB>OLD<TAB>NEW[<TAB>...]

FILE is a name in src/ (for example hand.c). Every whole-word use of OLD in the function's body is renamed,
except member names after `.` or `->`, strings and comments. A rename is refused when NEW already appears in the function (as a
local, a parameter or a global it uses). Run bld afterwards.
"""
import collections, os, re, sys

GW = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))


def find_function(text, fn):
    m = re.search(r'^[A-Za-z][^\n;]*\b' + fn + r'\s*\([^;]*?\)\s*\{', text, re.M | re.S)
    if not m:
        return None
    depth, i = 0, m.end() - 1
    while i < len(text):
        if text[i] == '{':
            depth += 1
        elif text[i] == '}':
            depth -= 1
            if depth == 0:
                return m.start(), i + 1
        i += 1
    return None


def main():
    jobs = collections.defaultdict(list)  # (file, function) -> [(old, new)]
    args = sys.argv[1:]
    if args[:1] == ['--file']:
        for line in open(args[1]):
            p = line.rstrip('\n').split('\t')
            if len(p) >= 4 and not line.startswith('#'):
                jobs[(p[0].strip(), p[1].strip())].append((p[2].strip(), p[3].strip()))
    else:
        jobs[(args[0], args[1])] = [tuple(a.split('=', 1)) for a in args[2:]]
    problems = []
    texts = {}
    for (f, fn), pairs in jobs.items():
        path = os.path.join(GW, 'src', f)
        text = texts.setdefault(path, open(path).read())
        span = find_function(text, fn)
        if not span:
            problems.append(f'{f}:{fn}: function not found')
            continue
        body = text[span[0]:span[1]]
        for old, new in pairs:
            if re.search(r'(?<![.>])\b' + new + r'\b', body):
                problems.append(f'{f}:{fn}: {new} is already used in the function')
                continue
            parts = re.split(r'("(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|//[^\n]*|/\*.*?\*/)', body, flags=re.S)
            n = 0
            for k in range(0, len(parts), 2):  # odd parts are strings, characters and comments
                parts[k], c = re.subn(r'(?<!\.)(?<!->)\b' + old + r'\b', new, parts[k])
                n += c
            body = ''.join(parts)
            if n == 0:
                problems.append(f'{f}:{fn}: {old} not found')
            else:
                print(f'{f}:{fn}: {old} -> {new} ({n})')
        texts[path] = text[:span[0]] + body + text[span[1]:]
    for path, text in texts.items():
        open(path, 'w').write(text)
    for p in problems:
        print('PROBLEM', p)
    return 1 if problems else 0


if __name__ == '__main__':
    sys.exit(main())
