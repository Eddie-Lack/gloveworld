#!/usr/bin/env python3
# tryrw.py SPECS.json...: try source rewrites one site at a time, each checked with a full ROM build.
# Each entry is {"site": NAME, "alt": N, "edits": [{"file": "src/x.c", "old": TEXT, "new": TEXT}, ...]}.
# The alternatives of a site are tried in order of "alt"; the first that keeps the ROM matching stays in the
# tree and the others are not tried. Each "old" must occur exactly once in its file. Prints KEEP, fail (the
# ROM no longer matches) or nomatch (an "old" text was not found exactly once) for each alternative.
import collections, json, os, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
GW = os.environ.get('GW') or os.path.dirname(os.path.dirname(HERE))

def build():
    r = subprocess.run([os.path.join(HERE, 'bld')], capture_output=True, text=True)
    return 'ROM OK' in r.stdout

bysite = collections.OrderedDict()
for p in sys.argv[1:]:
    for e in json.load(open(p)):
        bysite.setdefault(e['site'], []).append(e)

for site, alts in bysite.items():
    alts.sort(key=lambda e: e.get('alt', 1))
    kept = False
    for e in alts:
        saved = {}
        ok = True
        for ed in e['edits']:
            path = os.path.join(GW, ed['file'])
            cur = open(path).read()
            saved.setdefault(path, cur)
            if cur.count(ed['old']) != 1:
                ok = False
                break
            open(path, 'w').write(cur.replace(ed['old'], ed['new']))
        if ok and build():
            print('KEEP', site, 'alt', e.get('alt', 1), flush=True)
            kept = True
            break
        for path, text in saved.items():
            open(path, 'w').write(text)
        print('fail' if ok else 'nomatch', site, 'alt', e.get('alt', 1), flush=True)
    if not kept:
        print('----', site, 'none kept', flush=True)

print('final', 'ROM OK' if build() else 'ROM MISMATCH')
