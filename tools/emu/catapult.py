#!/usr/bin/env python3
"""catapult: turn the platform under Glover into a catapult in a running level and log its whole cycle.

    python3 tools/emu/catapult.py [LEVEL] [--gfx rice] [--out DIR] [--max-frames N]

No shipped level has a catapult platform, so this builds one in memory the way the level parser does (the code
for token TOKEN_CATAPULT in `src/parse.c`). The level starts through the title's attract mode as in `demoplay.py`
(the ROM's demo recording drives the first frames). On the first game frame in which Glover stands on a platform
(`hand.obj->platform`) that has no motion and no catapult, the script:

  1. clears a 0x98 byte PlatformMotion in unused RAM (0x80700000) and sets active[0] = 1 and damping = 1.0f;
  2. fills a PlatformCatapult in unused RAM (0x80700100) with launchDelay 20, launchSpeed 6, launchVelY 8,
     armRotVel 0.1, active 1, and holdPos = the hand's contact point minus the platform position, so the hand
     is within 10 units of it at once (the platform has yaw 0, so no rotation applies);
  3. points platform->motion and platform->catapult at them;
  4. writes autoPlayback = 0 so that the demo recording stops driving the pad.

The game's own CheckHandCatapult then catches the hand, winds up and launches. The script logs every
PlatformCatapult field and the hand state each game frame in which any of them changed, to OUT/catapult.log
(default tools/emu/build/out/, untracked). The run ends 60 game frames after the launch timer returns to 0.
Environment: CAT_LEVEL, CAT_LOG, CAT_MAX.
"""
import os, struct, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import demoplay as D

MOT, CAT = 0x80700000, 0x80700100


def fields(emu, cat):
    return dict(holdPos='%g,%g,%g' % (emu.rf(cat), emu.rf(cat + 4), emu.rf(cat + 8)),
                launchSpeed=emu.rf(cat + 0xC), launchVelY=emu.rf(cat + 0x10),
                active=s16(emu.r16(cat + 0x14)), unk16=s16(emu.r16(cat + 0x16)),
                catchTimer=s16(emu.r16(cat + 0x18)), launchTimer=s16(emu.r16(cat + 0x1A)),
                armRotVel=emu.rf(cat + 0x1C), launchDelay=emu.r32(cat + 0x20))


def s16(v):
    return v - 0x10000 if v & 0x8000 else v


def run(emu):
    level = int(os.environ.get('CAT_LEVEL', '1'))
    log = open(os.environ.get('CAT_LOG', os.path.join(HERE, 'build', 'out', 'catapult.log')), 'w', buffering=1)
    maxf = int(os.environ.get('CAT_MAX', '3000'))
    emu.cheat('demolevel', [(0x8011468B, level)])
    yield 30
    emu.w8(D.DEMOLEVEL, 0)
    emu.w32(D.DEMOTIMER, 3)
    started, last, plat, prev, done_at, launched = False, -1, 0, None, None, False
    for it in range(100000):
        yield 1
        fc, mode = emu.r32(D.FRAMECOUNT), emu.read(D.GAMEINFO, 1)[0]
        if not started:
            started = mode in (4, 5) and emu.read(D.GAMEINFO + 1, 1)[0] == level and fc > 0
            if it > 3000:
                log.write('NOSTART\n'); break
            if not started:
                continue
        if fc == last:
            continue
        last = fc
        obj = emu.r32(D.HAND + 0xD8)
        state = emu.r32(D.HAND + 0xD4)
        if not plat:
            p = emu.r32(obj + 0x178) if 0x80000000 <= obj < 0x80800000 else 0
            if p and not emu.r32(p + 0x2C) and not emu.r32(p + 0x50) and emu.r32(p + 0x14) & 0x4 == 0:
                # contact point of the hand's first sphere, relative to the platform
                cx, cy, cz = (emu.rf(obj + 0x10 + 4 * i) for i in range(3))
                px, py, pz = (emu.rf(p + 0xB4 + 4 * i) for i in range(3))
                for i in range(0x98 // 4):
                    emu.w32(MOT + 4 * i, 0)
                emu.w8(MOT + 4, 1)
                emu.wf(MOT + 0x6C, 1.0)
                for i in range(0x24 // 4):
                    emu.w32(CAT + 4 * i, 0)
                for i, v in enumerate((cx - px, cy - py, cz - pz)):
                    emu.wf(CAT + 4 * i, v)
                emu.wf(CAT + 0xC, 6.0)
                emu.wf(CAT + 0x10, 8.0)
                emu.w16(CAT + 0x14, 1)
                emu.w16(CAT + 0x16, 0x7777)  # marker: any later change shows up in the log
                emu.wf(CAT + 0x1C, 0.1)
                emu.w32(CAT + 0x20, 20)
                emu.w32(p + 0x2C, MOT)
                emu.w32(p + 0x50, CAT)
                emu.w32(D.AUTOPLAY, 0)
                plat = p
                log.write(f'frame={fc} CONVERT platform={p:08X} contact=({cx:g},{cy:g},{cz:g}) platPos=({px:g},{py:g},{pz:g})\n')
            continue
        s = fields(emu, CAT)
        s['handmode'] = emu.r16(state + 4)
        s['handCatapult'] = '%08X' % emu.r32(D.HANDCONTROL + 0x4C0)
        s['noControl'] = emu.r16(D.HANDCONTROL + 0x8)
        s['motRot'] = '%g' % emu.rf(MOT + 0x38)
        s['motRotVel'] = '%g' % emu.rf(MOT + 0x50)
        if s != prev:
            log.write(f'frame={fc} ' + ' '.join(f'{k}={v}' for k, v in s.items() if prev is None or prev.get(k) != v) + '\n')
            prev = s
        if s['launchTimer'] < 0:
            launched = True
        if launched and s['launchTimer'] == 0 and done_at is None:
            done_at = fc
        if (done_at is not None and fc > done_at + 150) or fc > maxf:
            break
    log.write(f'END frame={last}\n')
    log.close()


def main():
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument('level', nargs='?', type=int, default=1)
    ap.add_argument('--gfx', default='rice')
    ap.add_argument('--max-frames', type=int, default=3000)
    ap.add_argument('--out', default=os.path.join(HERE, 'build', 'out'))
    a = ap.parse_args()
    os.makedirs(a.out, exist_ok=True)
    env = dict(os.environ, CAT_LEVEL=str(a.level), CAT_MAX=str(a.max_frames), CAT_LOG=os.path.join(a.out, 'catapult.log'))
    subprocess.run([sys.executable, os.path.join(HERE, 'emu.py'), os.path.abspath(__file__), '--state',
                    os.path.join(HERE, 'build', 'out', 'menu.st'), '--gfx', a.gfx, '--out', a.out], env=env, check=False,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(open(env['CAT_LOG']).read())


if __name__ == '__main__':
    main()
