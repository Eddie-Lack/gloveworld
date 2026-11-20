# ROM patches

Scripts that make a modified copy of the ROM. They are not part of the build, and they never change the base
ROM. Never commit a ROM or an IPS file made from one.

## debugpad2.py: debug controls on controller 2, debug output to the IS-Viewer

```sh
python3 tools/patches/debugpad2.py baserom.us.z64 glover-debug.z64 [--ips glover-debug.ips]
```

The game's debug code reads a fifth controller entry, `pad[4]`, that the retail game never fills. Its messages
and input recordings were meant for the team's development PC (see [Debug code](../../docs/game.md#debug-code)).
This script makes controller 2 drive the debug code, and sends the messages and recordings to the IS-Viewer.
The IS-Viewer is a simple debug output channel: the game writes text at cartridge address 0x13FF0020 and its
length at 0x13FF0014. ares and mupen64plus show this text in their logs, and SummerCart64 sends it over USB.

The script needs Python 3 and nothing else. It checks the base ROM's SHA-1 against `NGVE.sha1` and each
known word it replaces before it writes anything. The base ROM may be in `.z64`, `.v64` or `.n64` byte
order; the output is `.z64`. With `--ips` it also writes an IPS patch from the base ROM in `.z64` order.

What it changes:

1. Every `pad[4]` access in `ProcessInputPersonal` and `MainTitleWait` reads `pad[1]` instead (30 words). Only
   the low half of each address changes.
2. The code from `debugio.c` goes over `ScreenShot`, which is never called, and over `PCcreat` and `PCclose`,
   the libsn functions it replaces.
3. In `ControllerProc`, the call to `EndTimer(2)` calls `DebugReadController2` instead. It reads controller 2
   and then calls `EndTimer(2)`. So controller 2 is read on every frame, just after controller 1.
4. `debugPrintf` and `PCwrite` jump to their replacements. `PRINTF` messages now reach the IS-Viewer.
   `PCcreat`, `PCwrite` and `PCclose` send a recording as text lines: `MOOFILE` with the game's file name,
   `MOODATA` with 32 bytes in hexadecimal, and `MOOEND`.
5. The two header checksum words are recomputed, so the ROM boots on hardware and in strict emulators.

Things to know:

- Plug controller 2 in before the console starts. Without it, `pad[1]` stays zero and nothing happens.
- Controller 2's stick and A and B move the `helper` box all the time; this is how the debug code works.
- L on controller 2 marks every level as done in the current save slot. The game may save this.
- On the title screen, Z on controller 2 cycles `autoPlayback`: off, record, play back.
- To record, press Z once on the title screen and start a level. The intro level always plays back instead.
  To save the recording, hold A and press B on controller 2. This also quits to the menu. Leaving a level by
  any other way discards the recording.
- Playback mode plays the input stored in the ROM for each level. A level without a recording stops with the
  message "LEVEL UNPLAYABLE - NO MOO FILE".
- An emulator or cartridge without IS-Viewer support ignores the output.

Tested in mupen64plus 2.5.9 with the Rice video plugin: the `PRINTF` messages appear in its log as
`IS64:` lines (shown at its verbose level), controller 2 drives the debug controls and the title screen's
Z, and a recording made in Atlantis 1 arrived as a 64-byte `.moo` file through `moolog.py`. The timing bars
were not checked, because Rice does not draw them.

## debugpatch.html: the same patch in a browser

Open `tools/patches/debugpatch.html` and drop the ROM on it. It makes the same debug ROM and IPS patch as
`debugpad2.py`, with the same checks, and needs no Python. The ROM never leaves the browser.

The page holds a copy of the patch list. After a change to `debugpad2.py` or `debugio_code.py`, run
`tools/patches/mkpatchpage.py` to write the new list into the page.

## moolog.py: recordings from the log

```sh
python3 tools/patches/moolog.py emulator.log [OUTDIR]
```

It finds the `MOOFILE`, `MOODATA` and `MOOEND` lines in a log and writes each recording as a `.moo` file,
named after the last part of the game's file name (for example `ATLANTIS_1.moo`). Text that the emulator adds
in front of a line does not matter.

## mooedit.html: view and edit recordings

Open `tools/patches/mooedit.html` in a browser. It needs no server and no install.

- **Open .moo** loads a recording, such as one from `moolog.py`.
- **Open ROM** lists the 44 demo recordings stored in the ROM, one for each level number up to 43.
- The timeline shows which buttons are held over time. The event list edits each event's frame and buttons.
- Play, Pause, Record and Stop run the recording at the game's speed: 20 game frames a second on NTSC, 16.7 on
  PAL. The controller drawing lights the buttons held at the playhead.
- Record replaces everything from the playhead on with keys held on the keyboard, sampled once per game frame:
  X is A, C is B, Z is Z, Enter is Start, Q and E are L and R, the arrow keys are the D-pad, and I, K, J and L
  are the C buttons. Space plays and pauses. Stop adds a last event so that playback lasts as long as the
  recording.
- Warnings name events that would stop playback: a frame before 9, or two events on the same frame.
- **Save .moo** writes the recording. **Save ROM** writes a copy of the ROM with the edited recordings put back
  into the table. That area is outside the header checksum, so no other change is needed.

Recordings hold buttons only. In playback the D-pad steers Glover, because the stick reads as zero.

The controller picture, `mooedit-pad.png`, is the one the game shows during its demos, with a transparent
background. The page loads it from its own folder. If the file is missing, the page shows its own drawing.

## Changing the code

`debugio.c` is the source of the new code. After a change, run `tools/patches/mkdebugio.sh`. It compiles the
file with the game's compiler (`make toolchain`), links each part at its place in the ROM, and writes
`debugio_code.py`, which `debugpad2.py` reads. The linker stops if a part does not fit its place: `ScreenShot`
has 0x278 bytes after alignment, `PCcreat` 0x90 and `PCclose` 0x68.
