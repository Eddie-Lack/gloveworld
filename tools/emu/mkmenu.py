#!/usr/bin/env python3
"""mkmenu: boot the game, skip the logos and save the title menu state that demoplay.py, catapult.py and poke.py start from.

    python3 tools/emu/emu.py tools/emu/mkmenu.py --gfx rice

It pulses Start every 160 video frames until the title runs (`gameInfo.mode` 1 with `demoTimer` counting down), then
saves `menu.st` in the output folder (default `tools/emu/build/out/`, untracked).
"""
GAMEINFO, DEMOTIMER, FRAMECOUNT = 0x801E7530, 0x801E6628, 0x802004E0


def run(emu):
    yield 750
    for i in range(40):
        yield 155
        if emu.read(GAMEINFO, 1)[0] == 1 and 0 < emu.r32(DEMOTIMER) < 600 and emu.r32(FRAMECOUNT) > 0:
            emu.save('menu')
            yield 10
            print('saved menu.st at video frame', emu.frame)
            return
        emu.hold('START')
        yield 8
        emu.release()
    print('the title did not appear')
