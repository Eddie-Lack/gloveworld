# The game code

This file is a map of the game's code: what each source file holds and the main data the files share. Use it
to find where something lives. The struct definitions in `include/` and the names in [naming.md](naming.md)
give the details.

## Main data

- **Actors.** Most things in a level are an `Actor` (actor.h): position, velocity, rotation, a physics block, an
  object (model and collision spheres) and a state block. The glove is the actor `hand`, the ball is `ball` and
  the camera is `camera`. Enemies (`Enemy`, 0x300 bytes) and platforms (`Platform`) embed an Actor.
- **Controllers.** `handControl`, `ballControl` and `cameraInfo` hold the extra state of the hand, the ball and
  the camera. `player` holds the score, lives and power-ups. `gameInfo` holds the level, world, exit and fade
  state. `saveSlot[]` holds the save files.
- **Lists.** Each kind of object sits in a doubly linked list with a head node (`actorList`, `platformList`,
  `bulletList`, `garibGroupList` and others), walked as `for (p = list.head.next; p != &list.head; p = next)`.
- **Memory.** `JallocAlloc(size, type, tag)` allocates from the game's heap. The tag string (`"Act_Shad"`,
  `"Plat_Rest"`) is good evidence for what a block holds.
- **Display lists.** Drawing code appends commands at `glistp` with the libultra gbi macros.
- **Input.** `pad[]` holds the four controllers and a fifth entry, `pad[4]`, that only the debug code reads (see
  [Debug code](#debug-code)). The button order
  of `pad[].button[]` is A, B, Z, Start, L, R, left, right, up, down, C-left, C-right, C-up, C-down.

## Files

### System

| File | Contents |
| --- | --- |
| main.c | boot, threads, the main loop, frame buffers and the graphics task |
| sched.c | the libultra-style scheduler (`nnSc...`) |
| dma.c | ROM to RAM copies and the music driver set-up |
| jalloc.c | the tagged heap allocator |
| banks.c | texture and object banks: loading, decompression into RAM, pointer fix-up |
| decompress.c, crc.c | data decompression and CRC checks |
| eeprom.c | save data, high scores and best times |
| controller.c | controller reads, demo input playback, camera control mode |
| text.c, font.c | text items and the two fonts |
| timer.c | debug timing bars |

### Game flow

| File | Contents |
| --- | --- |
| game.c | level loading and freeing, the per-frame game update, end-of-level and time trial screens |
| parse.c | the level file parser (one handler per token type) |
| frontend.c, frontend2.c | menu screens: save slots, name entry, continue and OK screens, screen fades |
| title.c, options.c, credits.c | the title screen, options, credits and high scores |
| present.c | the presentation screen and the level fly-through |
| cheat.c | cheat code entry and effects |
| personal.c | debug controls and screenshots |
| puzzle.c | level scripts: conditions and actions (see the enums in `include/enums.h`) |
| cameo.c | scripted cameo sequences |
| tip.c | the hint signs |
| bonus.c, boss.c | bonus levels and boss fights |

### Actors and objects

| File | Contents |
| --- | --- |
| actor.c | actor lists, movement, physics helpers |
| object.c | object set-up, matrices and drawing |
| anim.c | actor animation |
| collision.c | collision of actor spheres with planes and with each other |
| hand.c | the glove: allowed actions, ApplyAction, MaintainAction (the largest function), MoveHand |
| ball.c | the ball and its forms |
| camera.c | camera movement and collision |
| enemy.c | enemy loading and updates |
| nmeprocess.c, nmeprocess2.c | the enemy script interpreter, and the behaviour of each enemy type |
| bullet.c, spell.c | bullets and spells |
| platform.c | platforms: paths, spins, pushes, teleports, catapults |
| collect.c | garibs and other pick-ups |
| debris.c | debris particles |
| water.c, wind.c, vent.c, buzzer.c, spike.c | level features |

### Effects and drawing

| File | Contents |
| --- | --- |
| overlay.c | 2D overlays, backdrops, the HUD and screen effects |
| light.c | background and hand lights |
| shadow.c, footprint.c | shadows and footprints |
| blur.c | motion blur trails |
| weather.c | weather effects such as rain |
| quake.c | earthquakes and explosions |
| sound.c | samples, ambient sounds and songs |
| maths.c | vectors, quaternions, matrices, trigonometry tables, `Random` |

## Object banks

A model is an object in an object bank. LoadObjectBank, RestoreObjectPointers and RestoreDrawList (banks.c) read
a bank, and TransformAndDrawObject (object.c) draws its objects.

- **Place.** A bank lies from its `...ObjBankRomStart` up to the next bank in the ROM. If it starts with
  `FLA2`, DMAMemory unpacks it: a little-endian size, then DecompressBuffer's format. Each bit of a flag byte,
  highest first, picks a literal byte or a copy. A copy byte holds the length less 2 in its low 4 bits and the
  top of the distance in its high 4; the next byte is the rest of the distance. A copy byte of 0 ends the data.
- **Table.** The bank starts with pairs of a CRC of the file name (`UpdateCRC("goodglov.ndo")`) and an
  ObjectInstance, ended by a null instance. The instance points to the root Object (0x0C) and to an AnimState
  (0x18), whose table of AnimInfo (0x48) gives each animation's first and last key time and its speed.
- **Pointers.** Every pointer is an offset into the bank: Rom2Ram keeps its low 24 bits and adds the bank's
  address. The three functions above are the only code that converts them. Their fields are all the pointers
  there are: the instance's object and AnimState, the AnimState's table; an Object's mesh, display list, three
  key arrays, sprites, child and sibling; a Mesh's vertices (always), its faces, face colours, texture
  coordinates, face flags and texture table (only when it has faces) and its vertex colours; and the address of
  each `G_VTX` in a display list.
- **Pose.** Objects form a tree through `child` and `sibling`. Position and scale keys are blended linearly and
  rotation keys with QuatSlerp. Before the first key and after the last, the last key is used. A child's
  translation is scaled by its parent's own scale, but its rotation and geometry are not. Flag 0x40 turns the
  rotation keys with `frameCount` instead of the animation time.
- **Drawing.** A display list (F3DEX) is drawn when there is one, and the mesh only otherwise. RestoreDrawList
  steps through a list 8 bytes at a time up to `G_ENDDL` (0xB8). After `G_SETTIMG` it skips the 7 commands of
  gDPLoadTextureBlock when `G_SETTILE` follows, or the 6 of a palette load when `G_RDPTILESYNC` follows, and
  puts the texture's data or palette in place of the texture CRC that the command holds. For the objects named
  `GShinL` and `GShinR` it keeps the address of the last such word; the sticky-fingers power-up (collect.c)
  writes the `whitered.bmp` texture there. Meshes are drawn through `objectsVtx`, 800 vertices a frame for all
  of them. InitViewport turns on `G_CULL_BACK`.

## Debug code

The retail ROM still holds the team's debug code, but nothing turns it on. `pad` has room for five entries
(0x7D0 bytes before `rkp`). The game reads only the active controller into `pad[0]`. `pad[4]` stays zero.
The debug code reads `pad[4]`; the team probably filled it from a development PC. `tools/patches/debugpad2.py`
makes a ROM in which controller 2 drives it, and in which messages and recordings go to the IS-Viewer instead
of the PC (see [tools/patches/](../tools/patches/README.md)).

| Function | What it does | State in the retail ROM |
| --- | --- | --- |
| `ProcessInputPersonal` (personal.c) | debug controls, read from `pad[4]` every game frame | runs, never sees input |
| `MainTitleWait` (title.c) | `pad[4]` Z cycles `autoPlayback`: off, record input, play back the level's demo | runs, never sees input |
| `PrintTimers` (timer.c) | draws one bar per `StartTimer` slot (`CONTROL`, `GRAPHICS` and others) when `timerMode` is not 0 | runs; only the debug controls set `timerMode` |
| `ScreenShot` (personal.c) | writes the frame buffer to `c:\n64NNNN.bmp` on the development PC | never called |
| recording (controller.c, game.c, personal.c) | `autoPlayback` 1 stores button changes, then writes them to `c:\n64\oddballs\incbins\records\LEVEL.moo` | needs the development PC; without it, `PCcreat` (SN's libsn) waits forever for an answer |
| `debugPrintf` (main.c) | the target of `PRINTF`; `Crash` calls it too | empty. All messages are lost |
| `TitleScreen` (game.c) | an early title menu: continue, new game, and `LevelSelectScreen` (`gameInfo.mode` 2) | never called; no pointer to it exists |

`StartDrawing`, `StopDrawing` and the `StartTimer` names are labels for these messages and bars. A recording
has the layout of the demos in the ROM (`LoadRecordKeyFile`): a size word that counts itself, then pairs of a
frame number and the controller's button word, ended by -1. Only buttons are recorded, not the stick. The cheat
codes in cheat.c are a retail feature, not debug code.

Demo playback runs with `autoPlayback` 2, which `StartNewLevel` and `ReadControllerFromData` check. The attract
mode of the title screen (`DemoKeyRun`, after `demoTimer` runs out) starts five levels this way, then the
flythrough. The ROM holds 44 recordings, one per level number, found one after the other by their size words
(`LoadRecordKeyFile`). 31 of them are the same recording: 42 button changes over 333 game frames, which each level
plays out in its own way. Levels 10, 15, 16, 20, 25, 30, 35 and 39 have their own (208 to 448 bytes), levels 41 to
43 share a 96-byte one, and levels 0 and 24 have an empty one (16 bytes), so nothing is pressed in them and the
demo never ends by itself. When the last button change is read, `exitCount` runs 20 frames and then the game
returns to the menu with `ExitLevel(QUIT_EXIT)`. [tools/emu/demoplay.py](../tools/emu/demoplay.py) plays them in
the emulator.

The debug controls in `ProcessInputPersonal` are:

| Input | Effect |
| --- | --- |
| C-left, C-right | move Glover (and the ball, if he holds it) to the previous or next platform |
| L | mark every level and every garib as done in the current save slot |
| Z | cycle `timerMode` (0, 1, 2); 1 and 2 both show the timing bars |
| stick, B, A | move the `helper` box (a position and a size) on x and z, up and down |
| Z held with stick, B, A | change the size of the `helper` box |
| D-pad left, right | play the previous or next of the 61 songs |
| D-pad right held, down held | turn the background ambient light |
| D-pad up held | make a ring of puffs at Glover |
| R | toggle `inertia`: the hand's acceleration is divided by 3, or multiplied back |
| A held, then B | quit to the menu; in record mode it first writes the recording to the PC |
| C-up held, C-down held | push Glover (or the ball, when he holds it) up or down |
| Start | `HurtBall` |

