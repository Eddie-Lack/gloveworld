#!/usr/bin/env python3
"""poke: write marker values into the placeholder fields while a demo plays, and compare with an untouched run.

    python3 tools/emu/poke.py LEVEL [LEVEL ...] [--gfx rice] [--out DIR] [--max-frames N] [--only NAME,...]

Each level runs twice from the title menu state (`build/out/menu.st`), with the ROM's demo recording as input
(started as in `demoplay.py`, and ended with a normal level exit so that the end-of-level screen runs):

  base  nothing is written;
  poke  on every video frame the script writes a marker into each placeholder:
          sprite    GameSprite.unk1C = 1 in every sprite of spriteList
          backdrop  Backdrop.unk64 = 0x1234 in every backdrop of backdropList
          bank      ObjectBank.unk00 = 0x12345678 in all five objectBanks
          eol       EolItem.unk0C = 123.0 in all seven eolItems
          arg3      DrawActor's call passes 0x1234 as TransformAndDrawObject's arg3 (a code cheat on the
                    `move a3, zero` in the call's delay slot)
        `--only` limits the run to some of them.

On every game frame (each change of `frameCount`, and every fifth video frame of the end-of-level screen) both
runs take a CRC of the game's logic state (`LOGIC`: the hand, the ball, the camera, handControl, the lists' heads,
gameInfo and the random seeds) and a second CRC of every sprite and backdrop in the lists. The poke run first sets its markers back
to 0, and writes them again after the CRC. Both runs also save the RAM at a few game frames (`DUMP_FRAMES`) and a
screenshot with it. A CRC of all RAM is no use: two untouched runs differ in the audio and thread memory.

The result per level: the number of game frames compared, whether the logic state, the sprites and backdrops, and
the screenshots (compared byte for byte) are the same, and for each saved RAM pair the differing bytes grouped by
symbol (from `build/NGVE.map`), for a closer look. The CRC of the lists is kept apart from the CRC of the logic
state: AnimateSprite runs once per drawn picture, not once per game frame, so a sprite's animation step depends on
the emulator's timing.

Two runs of the emulator can differ by themselves in a frame or a screenshot (the picture of the end-of-level
screen often does, by a few pixel values). When the poke run differs from the base run, the script therefore
makes a second base run. A frame counts when the poke run differs from both in that frame and in the next one,
and a screenshot counts when the two base runs agree. When the two base runs differ in more than 20 frames, the
level gives no result: ATLANTIS3A (level 12) takes one of several courses from about frame 10 on, with or without
markers. `--base-twice` runs base against base. POKE_SHAM=1 writes zeros in place of the markers (the same work
for the emulator's host), and POKE_DUMP sets the game frames to save.

Logs, dumps and screenshots go to OUT (default `tools/emu/build/out/`, untracked): `pokeNN-base.*`, `pokeNN-poke.*`.
`--rule` is a separate run that watches what the game itself does with GameSprite.unk1C. Every 60 game frames it
sets the field to 1 in every sprite, once. On each game frame it compares the sprite list with the frame before and
counts (the `RULE` line of `pokeNN-rule.log`):

  added, added1        sprites new in the list, and how many of them have a value other than 0
  removed0, removed1   sprites that left the list, by their last value
  cleared              sprites that stayed in the list and went from 1 to 0, split into
    clearedByAdd         AddSprite's store ran for it (it left the list and came back in the same frame)
    clearedBySub         SubSprite's store ran for it (the sprite before it had 0 and was removed)
    clearedOther         anything else
  raised               sprites that stayed and went from 0 to another value
  sameTex, pairs       neighbours in the list with the same texture, and all neighbour pairs

The two stores are caught with debugger breakpoints, so this run uses the interpreter.

PlatformCatapult.unk16 has no instance in any level; `catapult.py --poke` tests it.
Environment when given to emu.py as a script: POKE_LEVEL, POKE_RUN (base or poke), POKE_PREFIX, POKE_MAX, POKE_ONLY.
"""
import bisect, ctypes, os, re, subprocess, sys, zlib

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import demoplay as D

ALL = ('sprite', 'backdrop', 'bank', 'eol', 'arg3')
DUMP_FRAMES = tuple(int(x) for x in os.environ.get('POKE_DUMP', '40,120,250').split(','))
DUMP_EOL = (300, 900)  # video frames into the end-of-level screen
ARG3_INSN = 0x80143A6C  # DrawActor: `move a3, zero` in the delay slot of the call
ADD_STORE = 0x8014ACA8  # AddSprite: `sb zero, 0x1C(a0)`
SUB_STORE = 0x8014ACE8  # SubSprite: `sb zero, 0x1C(v0)`, v0 = sprite->next; skipped when sprite->unk1C is not 0


def targets(emu, only):
    """(address, size, kind) of every placeholder instance alive now."""
    out, r32 = [], emu.r32
    ok = lambda p: 0x80000000 <= p < 0x80800000
    if 'sprite' in only:
        p, n = r32(D.SPRITES), 0
        while p != D.SPRITES and n < 400 and ok(p):
            out.append((p + 0x1C, 1, 'sprite'))
            p, n = r32(p), n + 1
    if 'backdrop' in only:
        p, n = r32(D.BACKDROPS), 0
        while p != D.BACKDROPS and n < 50 and ok(p):
            out.append((p + 0x64, 2, 'backdrop'))
            p, n = r32(p), n + 1
    if 'bank' in only:
        out += [(D.OBJBANKS + 0x10 * i, 4, 'bank') for i in range(5)]
    if 'eol' in only:
        out += [(D.EOLITEMS + 0x20 * i + 0xC, 4, 'eol') for i in range(7)]
    return out


# The game's logic state: one block of bss (the sprite list head, the backdrop list, the object banks, the camera,
# handControl, the hand and the platform list), the ball, the end-of-level items, gameInfo and the random seeds.
LOGIC = ((0x80289000, 0x80291000), (0x8029F978, 0x8029FB78), (D.EOLITEMS, D.EOLITEMS + 0xE0),
         (D.GAMEINFO, D.GAMEINFO + 0x40), (0x801ED3C0, 0x801ED3C4), (0x801ED3F0, 0x801ED3F4))


def logic_crc(emu, data):
    """Two CRCs: of the logic state, and of every sprite and backdrop in the lists. `data` is the RAM as the core
    stores it (host-endian words), which is the same in both runs."""
    crc = 0
    for a, b in LOGIC:
        crc = zlib.crc32(data[a & 0x7FFFFF:b & 0x7FFFFF], crc)
    lists = 0
    for head, size in ((D.SPRITES, 0x38), (D.BACKDROPS, 0x70)):
        p, n = emu.r32(head), 0
        while p != head and n < 400 and 0x80000000 <= p < 0x80800000:
            lists = zlib.crc32(data[p & 0x7FFFFF:(p & 0x7FFFFF) + size], lists)
            p, n = emu.r32(p), n + 1
    return crc, lists


MARK = {'sprite': b'\x01', 'backdrop': b'\x12\x34', 'bank': b'\x12\x34\x56\x78', 'eol': b'\x42\xf6\x00\x00'}


def run(emu):
    level = int(os.environ['POKE_LEVEL'])
    poke = os.environ['POKE_RUN'] == 'poke'
    prefix = os.environ['POKE_PREFIX']
    maxf = int(os.environ.get('POKE_MAX', '6000'))
    only = os.environ.get('POKE_ONLY', ','.join(ALL)).split(',')
    log = open(prefix + '.log', 'w', buffering=1)
    if os.environ.get('POKE_SHAM') == '1':
        # the same work on the host, but the value written is the 0 that is already there
        for k in MARK:
            MARK[k] = bytes(len(MARK[k]))
    emu.cheat('eolexit', [(0x8013587F, 1)])
    emu.cheat('demolevel', [(0x8011468B, level)])
    if poke and 'arg3' in only:
        # addiu a3, zero, 0x1234
        emu.cheat('arg3', [(0x81000000 | (ARG3_INSN & 0xFFFFFF), 0x2407), (0x81000000 | ((ARG3_INSN + 2) & 0xFFFFFF), 0x1234)])
    yield 30
    emu.w8(D.DEMOLEVEL, 0)
    emu.w32(D.DEMOTIMER, 3)
    ram = lambda: ctypes.string_at(ctypes.addressof(emu._ram), 0x800000)
    seen = {k: 0 for k in MARK}
    started, last, in7, nframes, dumped, lastfc = False, -1, 0, 0, set(), 0

    def record(tag, dump):
        nonlocal nframes
        t = targets(emu, only) if poke else []
        for a, n, k in t:
            emu._put(a, bytes(n))
        data = ram()
        log.write(f'{tag} vi={emu.frame} crc=%08x lists=%08x\n' % logic_crc(emu, data))
        nframes += 1
        if dump and tag not in dumped:
            dumped.add(tag)
            open(f'{prefix}.{tag}.ram', 'wb').write(data)
            emu.shot(os.path.basename(prefix) + '.' + tag)
        for a, n, k in t:
            emu._put(a, MARK[k])
            seen[k] += 1

    rule = os.environ['POKE_RUN'] == 'rule'
    poke = poke and not rule
    prev = None
    st = dict(set1=0, added=0, added1=0, removed0=0, removed1=0, cleared=0, clearedByAdd=0, clearedBySub=0, clearedOther=0,
              raised=0, sameTex=0, pairs=0)
    readded, passed = set(), set()
    if rule:
        emu.break_at(ADD_STORE, lambda e, r: readded.add(r[4]))
        emu.break_at(SUB_STORE, lambda e, r: passed.add(r[2]))

    def watch(fc):
        """SubSprite's rule: follow GameSprite.unk1C of every sprite from one game frame to the next."""
        nonlocal prev
        cur, p = [], emu.r32(D.SPRITES)
        while p != D.SPRITES and len(cur) < 400 and 0x80000000 <= p < 0x80800000:
            cur.append((p, emu.r8(p + 0x1C), emu.r32(p + 8)))
            p = emu.r32(p)
        for (_, _, t0), (_, _, t1) in zip(cur, cur[1:]):
            st['pairs'] += 1
            st['sameTex'] += t0 == t1 and t0 != 0
        if prev is not None:
            now, before = {q: v for q, v, _ in cur}, {q: v for q, v, _ in prev}
            for q, v in before.items():
                if q not in now:
                    st['removed1' if v else 'removed0'] += 1
                elif now[q] and not v:
                    st['raised'] += 1
                elif v and not now[q]:
                    st['cleared'] += 1
                    st['clearedByAdd' if q in readded else 'clearedBySub' if q in passed else 'clearedOther'] += 1
            for q, v in now.items():
                if q not in before:
                    st['added'] += 1
                    st['added1'] += v != 0
        if fc % 60 == 30:
            # mark every sprite once; the game alone changes the values until the next marking
            for q, v, _ in cur:
                emu.w8(q + 0x1C, 1)
            st['set1'] += len(cur)
            cur = [(q, 1, t) for q, _, t in cur]
        prev = cur
        readded.clear()
        passed.clear()

    for it in range(200000):
        yield 1
        fc, mode = emu.r32(D.FRAMECOUNT), emu.read(D.GAMEINFO, 1)[0]
        if not started:
            started = mode in (4, 5) and emu.read(D.GAMEINFO + 1, 1)[0] == level and fc > 0
            if it > 3000:
                log.write('NOSTART\n')
                break
            if not started:
                continue
        if poke:
            for a, n, k in targets(emu, only):
                emu._put(a, MARK[k])
        if mode == 7 and rule:
            break
        if mode == 7:
            in7 += 1
            if in7 % 5 == 0:
                record(f'eol{in7}', in7 in DUMP_EOL)
            emu.hold('A') if in7 > 1000 and (in7 // 10) % 2 == 0 else emu.release()
            if in7 > 6000:
                log.write('EOL-TIMEOUT\n')
                break
            continue
        if in7:
            emu.release()
            break
        if mode in (4, 5) and fc >= 12:
            nk = emu.r16(D.NEXTKEYFRAME)
            nk = nk - 0x10000 if nk & 0x8000 else nk
            if 0 <= nk < fc:
                emu.w16(D.NEXTKEYFRAME, fc + 1)
        if fc != last:
            last = fc
            lastfc = max(lastfc, fc)
            if rule:
                watch(fc)
                if mode == 1 or fc > maxf:
                    break
                continue
            record(f'f{fc}', fc in DUMP_FRAMES)
        if mode == 1 or fc > maxf:
            break
    if rule:
        log.write('RULE ' + ' '.join(f'{k}={v}' for k, v in st.items()) + '\n')
    log.write(f'END frames={nframes} lastFrame={lastfc} eolVideoFrames={in7} pokes=' + ','.join(f'{k}:{v}' for k, v in seen.items()) + '\n')
    log.close()


def symbols():
    syms = []
    for line in open(os.path.join(D.HERE, '..', '..', 'build', 'NGVE.map')):
        m = re.match(r'\s+0x0*([0-9a-f]{8})\s+(\w+)$', line)
        if m and m.group(1).startswith('80'):
            syms.append((int(m.group(1), 16), m.group(2)))
    return sorted(set(syms))


def crcs(prefix):
    lines = [l.split() for l in open(prefix + '.log').read().splitlines()]
    return [(l[0], l[2], l[3]) for l in lines if len(l) == 4 and l[2].startswith('crc=')], ' '.join(lines[-1])


def compare(prefix_base, prefix_poke, syms, rerun):
    """Print the result for one level. rerun() makes a second base run and returns its prefix; it is called only when
    the poke run differs from the first base run, to tell the emulator's own run-to-run differences from an effect."""
    (fa, _), (fb, end) = crcs(prefix_base), crcs(prefix_poke)
    tags = [t for t in [f'f{n}' for n in DUMP_FRAMES] + [f'eol{n}' for n in DUMP_EOL]
            if all(os.path.exists(f'{p}.{t}.ram') for p in (prefix_base, prefix_poke))]
    png = lambda p, t: open(f'{p}.{t}.png', 'rb').read()
    diff = [x[0] for x, y in zip(fa, fb) if x != y]
    shots = [t for t in tags if png(prefix_base, t) != png(prefix_poke, t)]
    logic, lists, note = [], [], ''
    if diff or shots or len(fa) != len(fb):
        base2 = rerun()
        fc, _ = crcs(base2)

        def lasting(k):
            """Frames in which field k of the poke run differs from both base runs, and in the next frame too. One
            sample can fall inside an update, also between two base runs."""
            bad = [y[k] != x[k] and y[k] != z[k] for x, y, z in zip(fa, fb, fc)]
            return [fa[i][0] for i in range(len(bad) - 1) if bad[i] and bad[i + 1]]
        logic, lists = lasting(1), lasting(2)
        # a screenshot counts only where the two base runs give the same picture
        steady = [t for t in shots if os.path.exists(f'{base2}.{t}.png') and png(base2, t) == png(prefix_base, t)]
        note = (f'; the two base runs differ from each other in {sum(x[1] != y[1] for x, y in zip(fa, fc))} frames (logic), '
                f'{sum(x[2] != y[2] for x, y in zip(fa, fc))} frames (lists) and {len(shots) - len(steady)} of {len(shots)} screenshots')
        shots = steady
        if sum(x[1] != y[1] for x, y in zip(fa, fc)) > 20:
            note += '; THE BASE RUNS DO NOT REPEAT, so this level gives no result'
            logic, lists = [], []
    print(f'  game frames compared: {min(len(fa), len(fb))} (base {len(fa)}, poke {len(fb)}); '
          f'logic state: {"DIFFERS in %d frames, first %s" % (len(logic), logic[0]) if logic else "same"}; '
          f'sprites and backdrops: {"differ in %d frames, first %s" % (len(lists), lists[0]) if lists else "same"}; '
          f'screenshots: {"DIFFER " + str(shots) if shots else "same"}{note}')
    print('  ' + end)
    addrs = [s[0] for s in syms]
    for tag in tags:
        da, db = (open(f'{p}.{tag}.ram', 'rb').read() for p in (prefix_base, prefix_poke))
        groups = {}
        for i in range(0, 0x800000, 4096):
            if da[i:i + 4096] != db[i:i + 4096]:
                for j in range(i, i + 4096):
                    if da[j] != db[j]:
                        addr = 0x80000000 + (j ^ 3)  # RDRAM is stored as host-endian words
                        k = bisect.bisect_right(addrs, addr) - 1
                        name = syms[k][1] if k >= 0 else 'below the code'
                        groups[name] = groups.get(name, 0) + 1
        print(f'  {tag}: RAM bytes that differ: {sum(groups.values())}'
              + (' (' + ', '.join(f'{k} {v}' for k, v in sorted(groups.items())) + ')' if groups else ''))


def main():
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument('levels', nargs='+', type=int)
    ap.add_argument('--gfx', default='rice')
    ap.add_argument('--max-frames', type=int, default=6000)
    ap.add_argument('--only', default=','.join(ALL))
    ap.add_argument('--out', default=os.path.join(HERE, 'build', 'out'))
    ap.add_argument('--state', default=os.path.join(HERE, 'build', 'out', 'menu.st'))
    ap.add_argument('--base-twice', action='store_true', help='run base against base, to check that runs repeat')
    ap.add_argument('--rule', action='store_true', help="watch SubSprite's rule for GameSprite.unk1C instead")
    a = ap.parse_args()
    os.makedirs(a.out, exist_ok=True)
    syms = symbols()
    for lv in a.levels:
        def one(kind, name):
            prefix = os.path.join(a.out, f'poke{lv:02d}-{name}')
            env = dict(os.environ, POKE_LEVEL=str(lv), POKE_RUN=kind, POKE_PREFIX=prefix, POKE_MAX=str(a.max_frames), POKE_ONLY=a.only)
            subprocess.run([sys.executable, os.path.join(HERE, 'emu.py'), os.path.abspath(__file__), '--state', a.state,
                            '--gfx', a.gfx, '--out', a.out] + (['--debug'] if a.rule else []), env=env, check=False,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            return prefix
        if a.rule:
            line = open(one('rule', 'rule') + '.log').read().splitlines()[-2]
            print(f'level {lv}:\n  {line}', flush=True)
        else:
            base, other = one('base', 'base'), one('base', 'base2') if a.base_twice else one('poke', 'poke')
            print(f'level {lv}:', flush=True)
            compare(base, other, syms, lambda: one('base', 'base2'))


if __name__ == '__main__':
    main()
