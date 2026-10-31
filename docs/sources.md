# Sources of evidence

This file lists what we may use as evidence for names, types and code shapes, what each source can tell, and
its limits. The rules for using them are in [CLAUDE.md](../CLAUDE.md#rules).

## The ROM

The ROM is the only source of truth for the code. It fixes:

- every instruction, and with it the types of fields and locals (from the load and store instructions), the
  store order, the precision of each constant, and which functions are inlined;
- the order of rodata, which shows the order in which functions first used shared strings and constants;
- the file boundaries (`$Revision` idents and linker junk, see [building.md](building.md#source-files)).

The game's own strings are evidence too: JallocAlloc tags (`"Act_Shad"`, `"Plat_Rest"`, `"OFTC%d"`), `Crash`
messages (`"ACOSBETTER"`), file names in level data, and the credits text.

## The linker map

`block.map` is a linker map in SN psylink format, kept outside this repository. It lists 4,902 global symbols.
Static functions and locals are not in it. It comes from a **different build** of the game: its text is 0xFB0
bytes larger, and its offset from the ROM drifts from -0x2F0 to -0x12A4.

The map is from a PAL build. The final European ROM (NGVP, English, French and German) is the nearest known
build. Its code is only 0x2C0 bytes smaller than the map's, and the offset between them changes at about 13
places. The map has three functions that the USA ROM lacks (`FreeTitleScreen`, `CheckFrontProbe`, `ZCarBodge`).
The front end's `NewWorldScreen` (0x28 bytes) and `ErrorScreen` (0x20C bytes) are this ROM's `StartCurrentLevel`
and `NewWorldScreen`.

The first names in `symbol_addrs.txt` were placed by aligning the map piece by piece:

- A function name was kept where the map's size for it matched the ROM's function size.
- A data name was kept only where a placed neighbour had the same offset. A second pass added names whose two
  placed neighbours agreed.

Several data names turned out to be placed at the wrong address (blocks in collision.c and hand.c were 8 bytes
out). The front-end data (0x801E64B8 to 0x801E6790) lines up with the map at exactly -0x12A0, which the PC
port's globals in that block confirm. Names there that had been placed 0xF0 or 0x174 out were moved to that
alignment. So check every map-derived data name against its uses before relying on it.
Corrections are logged in [naming.md](naming.md#globals).

## The PC port's Glover.iobj

`Glover.iobj` is an object file from the Steam release of the PC port, built with MSVC for 64-bit Windows. It is
kept outside this repository. Its CodeView debug records give:

- the original **local and parameter names** of 678 of 895 functions, with basic types;
- the names of **function statics** (`?name@?1??Function@@9@9` in its strings);
- `S_CONSTANT` records for 160 **enum** types, of which about 30 are the game's own;
- a type list with `VECTOR`, `BYTEVECTOR` and `SHORTVECTOR`.

Limits:

- The PC port is a later build compiled by another compiler. Some functions differ from the N64 ones (the bank
  loaders, DrawGraphics, the game modes and menu screens).
- MSVC can drop or add locals. A local missing from a list is evidence that the original had none, not proof.
- The lists do not keep declaration order.
- Some lists include globals that MSVC kept in a register (`frameCount`, `camera`). These are not locals.
- Its addresses do not map to the N64 build by any offset. Never name a global by its address in the PC map.

`tools/session/styleaudit.py` reads the local lists. Useful commands:

```sh
strings -n 3 Glover.iobj | grep -n -A40 "^FunctionName$"   # a function's locals
strings -n 6 Glover.iobj | grep "@?[0-9]??"                  # function statics
```

The names may be used. Nothing else from the PC port is used.

## Other decompilations

Public N64 decompilations may be read as references for the compiler's behaviour, with the owner's approval. No
game code is copied. Dr. Mario 64 is the closest: KMC GCC 2.7.2 at `-O2`, fully matched. Projects built with
SN64 or IDO are less useful.

## The running game

`tools/emu/` runs the game in mupen64plus under script control. Use it to see what a field does at run time
before naming it (`structwatch.py` lists which fields change in which phase of play).

## Not allowed

Leaked original source code, in any form, for any purpose.
