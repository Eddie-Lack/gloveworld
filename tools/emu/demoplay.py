#!/usr/bin/env python3
"""demoplay: play the ROM's own demo recording of a level in the emulator and log placeholder fields.

    python3 tools/emu/demoplay.py LEVEL [LEVEL ...] [--gfx rice] [--max-frames N] [-j JOBS] [--eol] [--out DIR] [--probe ADDR] [--break ADDR[:REG],...]
    python3 tools/emu/demoplay.py all

Each level runs in its own emulator process, started from the title menu state `build/out/menu.st`
(`tools/emu/build/out/menu.st`). The script writes `autoPlayback = 2` and opens the level select
(`gameInfo.mode` 2) with `selectedLevel` set. A held A button starts the level. StartNewLevel then loads the level's
recording with LoadRecordKeyFile and ReadControllerFromData feeds the game from it. The recording ends with the
game going back to the menu (`gameInfo.mode` 1), or the frame limit stops the run.

The log goes to `OUT/demoNN.log` (default `tools/emu/build/out/`, untracked). It holds one line per game frame in
which any watched value changed, as `frame=N mode=M key=value ...`. The first line is the state at the start of the
level. Watched values: the placeholder fields (Backdrop.unk64, ObjectBank.unk00, GameSprite.unk1C,
CameraInfo.unk1A0, EolItem.unk0C, HandControl.unk14, PlatformCatapult.unk16), the global D_801F0232, and context
(hand mode, camera mode, the number of sprites, backdrops and catapult platforms).

The game reads the recording once per game frame. In the emulator a read is sometimes missed. The next key frame
is then in the past and the recording never ends. The script moves such a key to the next frame and counts them
(`lateKeys` in the last log line). `--no-catchup` turns this off.

`--eol` changes the exit that ends the demo from QUIT_EXIT to LEVEL_EXIT, so the end-of-level screen runs. The log
then has `eol=N` lines (N counts video frames) and ends with `EOL-END`. The A button is pulsed to leave the screen.

`--probe ADDR` overwrites the first instruction of the function at hex address ADDR with a branch to itself. If the
game ever calls that function, `frameCount` stops. The log then ends with `HANG`. A call during the end-of-level
screen (`--eol`) shows as `EOL-TIMEOUT`.

Levels without a recording show the game's own error screen (`gameInfo.mode` 13); the log then has no frames.

`--break` stops the CPU at each listed address (the core's debugger, which needs the interpreter, so a run is about
five times slower) and counts the values of one register there. TransformAndDrawObject's `arg3` is register 7 (a3).
The counts go to the end of the log as `BREAK` lines.

The module can also be given to emu.py as a script. Environment: DEMO_LEVEL, DEMO_LOG, DEMO_MAX, DEMO_PROBE, DEMO_EOL, DEMO_BREAK, DEMO_CATCHUP.
"""
import os, struct, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

GAMEINFO, AUTOPLAY, FRAMECOUNT = 0x801E7530, 0x801EC7F0, 0x802004E0
DEMOTIMER, DEMOLEVEL, NEXTKEYFRAME = 0x801E6628, 0x801E662C, 0x801EC772
SPRITES, BACKDROPS, OBJBANKS = 0x802892D8, 0x8028F350, 0x8028F828
CAMERA, HANDCONTROL, HAND, PLATFORMS, EOLITEMS = 0x8028F9C8, 0x8028FBD0, 0x802902D8, 0x802904D0, 0x801F90D0
FOOTPRINT_FMT = 0x801F0232


def sample(emu):
    """The watched values as an ordered dict of name -> value."""
    r32, r16 = emu.r32, emu.r16
    s = {}
    s['level'] = emu.read(GAMEINFO + 1, 1)[0]
    s['gmode'] = emu.read(GAMEINFO, 1)[0]
    state = r32(HAND + 0xD4)
    s['handmode'] = r16(state + 4) if 0x80000000 <= state < 0x80800000 else -1
    s['camMode'] = r32(CAMERA + 0x1A4)
    s['camUnk1A0'] = r32(CAMERA + 0x1A0)
    s['handUnk14'] = r32(HANDCONTROL + 0x14)
    s['fmt'] = r16(FOOTPRINT_FMT)
    s['bank00'] = ','.join(str(r32(OBJBANKS + 0x10 * i)) for i in range(5))
    # sprites: a linked list; head.next at +0, count at +0x38
    n = r32(SPRITES + 0x38)
    vals, p = [], r32(SPRITES)
    while p != SPRITES and len(vals) < 300 and 0x80000000 <= p < 0x80800000:
        vals.append(emu.read(p + 0x1C, 1)[0])
        p = r32(p)
    s['nSprites'] = n
    s['spriteUnk1C'] = ','.join(map(str, sorted(set(vals)))) or '-'
    # backdrops: linked list; head is the first member, count at +0x70
    vals, p = [], r32(BACKDROPS)
    while p != BACKDROPS and len(vals) < 50 and 0x80000000 <= p < 0x80800000:
        v = r16(p + 0x64)
        vals.append(v - 0x10000 if v & 0x8000 else v)
        p = r32(p)
    s['nBackdrops'] = r32(BACKDROPS + 0x70)
    s['backdropUnk64'] = ','.join(map(str, vals)) or '-'
    # platforms: count at +0, head at +4 (next at +0 of the head)
    cats, p, k = [], r32(PLATFORMS + 4), 0
    while p != PLATFORMS + 4 and k < 400 and 0x80000000 <= p < 0x80800000:
        c = r32(p + 0x50)
        if c:
            cats.append(r16(c + 0x16))
        p, k = r32(p), k + 1
    s['nCatapults'] = len(cats)
    s['catapultUnk16'] = ','.join(map(str, cats)) or '-'
    s['eolUnk0C'] = ','.join('%g' % emu.rf(EOLITEMS + 0x20 * i + 0xC) for i in range(7))
    return s


def run(emu):
    level = int(os.environ['DEMO_LEVEL'])
    log = open(os.environ['DEMO_LOG'], 'w', buffering=1)
    maxf = int(os.environ.get('DEMO_MAX', '6000'))
    probe = os.environ.get('DEMO_PROBE', '0') not in ('', '0')
    eol = os.environ.get('DEMO_EOL') == '1'
    if eol:
        # ReadControllerFromData ends the demo with ExitLevel(QUIT_EXIT, 1, 0). Make it ExitLevel(LEVEL_EXIT, 1, 0).
        emu.cheat('eolexit', [(0x8013587F, 1)])
    if probe:
        addr = int(os.environ['DEMO_PROBE'], 16)
        # GameShark 0x81 codes write 16 bits: the MIPS instruction `b .` (0x1000FFFF)
        emu.cheat('probe', [(0x81000000 | (addr & 0xFFFFFF), 0x1000), (0x81000000 | ((addr + 2) & 0xFFFFFF), 0xFFFF)])
    # The title's attract mode runs DemoKeyRun, which sets up a game and starts a level with autoPlayback 2.
    # Its first case calls StartNewLevel(10). A cheat changes that level number, and demoTimer is set low so that
    # the attract mode starts at once.
    emu.cheat('demolevel', [(0x8011468B, level)])
    yield 30
    emu.w8(DEMOLEVEL, 0)
    emu.w32(DEMOTIMER, 3)
    catchup = os.environ.get('DEMO_CATCHUP', '1') == '1'
    skipped = 0
    hits = {}
    for spec in filter(None, os.environ.get('DEMO_BREAK', '').split(',')):
        addr, _, reg = spec.partition(':')
        hits[(int(addr, 16), int(reg or 7))] = {}
        emu.break_at(int(addr, 16), lambda e, r, a=int(addr, 16), n=int(reg or 7): hits[(a, n)].__setitem__(r[n], hits[(a, n)].get(r[n], 0) + 1))
    started, last, prev, stalled, maxsprites, lastfc, in7 = False, -1, None, 0, 0, 0, 0
    for it in range(200000):
        yield 1
        if not started and it % 100 == 0:
            log.write(f'wait it={it} mode={emu.read(GAMEINFO,1)[0]} fc={emu.r32(FRAMECOUNT)} play={emu.r32(AUTOPLAY)} lvl={emu.read(GAMEINFO+1,1)[0]}\n')
        fc = emu.r32(FRAMECOUNT)
        mode = emu.read(GAMEINFO, 1)[0]
        if not started:
            if mode in (4, 5) and emu.read(GAMEINFO + 1, 1)[0] == level and fc > 0:
                started = True
            elif it > 3000:
                log.write('NOSTART' + (' (a probed function was called?)' if probe else '') + '\n')
                break
            else:
                continue
        if eol and mode == 7:
            # The end-of-level screen does not advance frameCount, so sample on every fifth video frame.
            in7 += 1
            if in7 % 5 == 0:
                s = sample(emu)
                s['nSprites'] = 0
                if s != prev:
                    diff = ' '.join(f'{k}={v}' for k, v in s.items() if prev is None or prev.get(k) != v)
                    log.write(f'eol={in7} {diff}\n')
                    prev = s
            if in7 > 1000 and (in7 // 10) % 2 == 0:
                emu.hold('A')
            else:
                emu.release()
            if in7 > 6000:
                log.write('EOL-TIMEOUT\n')
                break
            continue
        if eol and in7:
            emu.release()
            log.write(f'EOL-END demoFrames={lastfc} mode={mode} level={emu.read(GAMEINFO + 1, 1)[0]} eolVideoFrames={in7} lateKeys={skipped}\n')
            break
        if catchup and mode in (4, 5) and fc >= 12:
            # The game reads the recording once per game frame. In the emulator a read is sometimes missed, and
            # then a key frame is never equal to frameCount again and the recording never ends.
            nk = struct.unpack('>h', emu.read(NEXTKEYFRAME, 2))[0]
            if 0 <= nk < fc:
                emu.w16(NEXTKEYFRAME, fc + 1)
                skipped += 1
        if fc != last:
            last, stalled = fc, 0
            if fc > 0 and mode != 1:
                lastfc = fc
            if fc % 500 == 0:
                log.write(f'tick frame={fc}\n')
            s = sample(emu)
            maxsprites = max(maxsprites, s['nSprites'])
            s['nSprites'] = 0
            if s != prev:
                diff = ' '.join(f'{k}={v}' for k, v in s.items() if prev is None or prev.get(k) != v)
                log.write(f'frame={fc} {diff}\n')
                prev = s
        else:
            stalled += 1
            if stalled > (600 if probe else 3000):
                log.write(('HANG' if probe else 'STALL') + f' at frame={fc} mode={mode}\n')
                break
        if mode == 1 or fc > maxf:
            log.write(f'END frame={lastfc} mode={mode} autoPlayback={emu.r32(AUTOPLAY)} maxSprites={maxsprites} lateKeys={skipped}\n')
            break
    for (addr, reg), counts in hits.items():
        log.write(f'BREAK {addr:08X} r{reg}: ' + (' '.join(f'{v:#x} x{n}' for v, n in sorted(counts.items())) or 'never reached') + '\n')
    log.close()


def main():
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument('levels', nargs='+', help='level numbers, or "all"')
    ap.add_argument('--gfx', default='rice')
    ap.add_argument('--max-frames', type=int, default=6000)
    ap.add_argument('--out', default=os.path.join(HERE, 'build', 'out'))
    ap.add_argument('--state', default=os.path.join(HERE, 'build', 'out', 'menu.st'))
    ap.add_argument('--eol', action='store_true', help='end each demo with a normal level exit and log the end-of-level screen')
    ap.add_argument('--break', dest='brk', metavar='ADDR[:REG],...',
                    help='count the calls of the functions at these hex addresses, with the values of register REG (default 7, a3); runs the interpreter')
    ap.add_argument('--no-catchup', action='store_true', help='do not repair a missed key frame of the recording')
    ap.add_argument('-j', '--jobs', type=int, default=1, help='emulators to run at once')
    ap.add_argument('--probe', metavar='ADDR', help='hex address of a function to test for calls')
    a = ap.parse_args()
    levels = range(44) if a.levels == ['all'] else [int(x) for x in a.levels]
    os.makedirs(a.out, exist_ok=True)
    def one(lv):
        log = os.path.join(a.out, f'{"eol" if a.eol else "demo"}{lv:02d}.log')
        env = dict(os.environ, DEMO_LEVEL=str(lv), DEMO_LOG=log, DEMO_MAX=str(a.max_frames),
                   DEMO_PROBE=a.probe or '0', DEMO_BREAK=a.brk or '', DEMO_CATCHUP='0' if a.no_catchup else '1', DEMO_EOL='1' if a.eol else '0')
        subprocess.run([sys.executable, os.path.join(HERE, 'emu.py'), os.path.abspath(__file__),
                        '--state', a.state, '--gfx', a.gfx, '--out', a.out] + (['--debug'] if a.brk else []), env=env, check=False,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        last = open(log).read().splitlines()[-1] if os.path.exists(log) else 'no log'
        print(f'level {lv}: {last}', flush=True)

    from multiprocessing.pool import ThreadPool
    list(ThreadPool(a.jobs).imap_unordered(one, levels))


if __name__ == '__main__':
    main()
