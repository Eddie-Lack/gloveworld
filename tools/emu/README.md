# Scripted emulator

These tools run the game in mupen64plus under script control. Use them to find out what a variable or struct
field does at run time, which helps when naming it.

- `setup.sh` installs mupen64plus from Ubuntu's packages (the core, the rice and z64 plugins and the SDL audio
  plugin), builds angrylion's software video plugin and the `scriptinput` input plugin into `build/`, and starts
  a virtual display (`Xvfb :99`).
  After a restart, start the display again with `Xvfb :99 -screen 0 1024x768x24 &`.
- `emu.py SCRIPT.py [--state FILE] [--gfx angrylion|rice]` runs a script. The script holds buttons and moves the
  stick, reads and writes memory, takes screenshots and saves states. See the docstring at the top of `emu.py`.
  Output goes to `build/out/`.
- `demoplay.py LEVEL...` plays the ROM's own demo recording of each level, one emulator run per level, and logs the
  placeholder fields every game frame (see its docstring). `all` runs levels 0 to 43. It starts the level through the
  title's attract mode (`DemoKeyRun`), changes the level number in that function with a cheat, and sets
  `demoTimer` so that the attract mode starts at once. `--eol` ends the demo with a normal level exit and logs
  the end-of-level screen. `--probe ADDR` finds out whether a function is ever called, and `--break ADDR[:REG]`
  counts its calls and the values of a register with the core's debugger.
- `catapult.py [LEVEL]` builds a catapult platform in memory under Glover (no shipped level has one), lets the game's
  own `CheckHandCatapult` run it through a full cycle and logs every `PlatformCatapult` field each game frame to
  `build/out/catapult.log` (see its docstring).
- `structwatch.py` samples a struct every frame through named phases (for example idle, walk, jump) and lists
  which fields change in which phase. It reads field offsets and types from our headers.

`emu.py --debug` turns on the core's debugger (the interpreter instead of the recompiler, about five times slower).
A script can then call `emu.break_at(addr, fn)`, which runs `fn(emu, regs)` each time the CPU reaches `addr`, with the
32 general registers. The O32 arguments of a call are in registers 4 to 7 (a0 to a3).

A ROM that differs from the retail one (a patched or modded ROM) works with `--rom`. The core refuses a savestate
made with another ROM (it compares the MD5), and has no ini entry for the new ROM. `emu.py` therefore makes a copy
of the state that holds the new ROM's MD5, and a data folder with an ini entry that refers to the retail ROM
(`build/data-MD5`, `build/patched-*`; both untracked).

Savestates: mupen64plus `.st` files and Project64 `.pj` files both load. Keep them in `build/` (not tracked),
since they hold a copy of the game's memory. A script can also save and reload states while it runs
(`emu.save(name)`, `emu.load(path)`); both take effect at the next frame, so yield before loading a state that
was just saved.

States in `build/states/`: `levelselect48.st` is the level select with all 48 entries, and `Main_Menu.pj` is the
title menu. To load a level from `levelselect48.st`, write its number to `selectedLevel` (0x801E75A0), hold A for
a few frames and wait about 900 frames.

If a run with angrylion and a `.st` state crashes (a segmentation fault in the first seconds), use `--gfx rice`.
Rice draws less exactly but is fine for reading and writing memory.

Notes:

- angrylion needs the low-level `z64` RSP plugin; `emu.py` picks it automatically. `--gfx rice` is faster but
  less exact.
- The game reads the controller once per game frame. In mupen64plus a game frame is five video frames in the
  menus and in play: each frame's five RSP tasks take four video frames to retire, and the scheduler waits for
  them (`gfxTasks` in `src/sched.c`). Hold buttons for at least 6 frames, or hold them until `frameCount`
  (0x802004E0) changes for an input that must land on one game frame.
- `emu.py` attaches the SDL audio plugin with `SDL_AUDIODRIVER=dummy`. Without an audio plugin the game's main
  loop never runs: `frameCount` stays at 0 and no input is read.
- `pad[].button[]` and `pressed[]` indices (from `buttonFlag`): 0 A, 1 B, 2 Z, 3 Start, 4 L, 5 R, 6 left,
  7 right, 8 up, 9 down, 10 C-left, 11 C-right, 12 C-up, 13 C-down.
- `pad[4]` is a fifth, debug pad that the N64 code never polls. A value written there stays set.
- Level select: in `LevelSelectScreen`, Z previews the level's music and any other button starts it. The cursor
  is `selectedLevel` and the number of entries is `numLevels` (42). Setting it to 48 shows six more entries
  (WAYROOM, PRESENT, FLYTHRU, FLYTHRU2, INTRO, OUTRO); the GameShark code `8011E653 0030` patches the 42 in the
  code instead. To apply a code that writes to code, use `emu.cheat()`. A direct write misses the recompiler.
- The level select of `LevelSelectScreen` cannot be reached from `build/out/menu.st` by writing `gameInfo.mode` 2:
  the level then never finishes loading. `demoplay.py` goes through the attract mode instead.
- Demo playback: `autoPlayback` 2 makes `ReadControllerFromData` feed the recording, which `LoadRecordKeyFile(level)`
  loads in `StartNewLevel`. The ROM holds 44 recordings: 31 are one default recording (333 frames), levels 0 and 24
  have an empty one, and 11 differ (docs/game.md, "Debug code"). The game reads the recording once per game frame;
  in the emulator it sometimes misses a key frame and the demo then never ends, which `demoplay.py` repairs.
- GitHub must be reachable through git for `setup.sh` to fetch angrylion (plain HTTPS downloads from github.com
  may be blocked).
