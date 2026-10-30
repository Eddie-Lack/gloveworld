# Building

This file explains how the ROM is rebuilt from the source, and why the build is set up the way it is. The quick
steps are in the [README](../README.md#building).

## What the build does

1. `make toolchain` runs `tools/setup_toolchain.sh`. It clones the compiler source into
   `tools/` at a fixed commit and builds the compilers.
2. `make setup` runs splat (through `tools/splat_run.py`) on the ROM. splat writes the assembly into `asm/`,
   the binary blobs into `bin/`, the linker script `NGVE.ld` and the `undefined_*_auto.txt` files. It first
   deletes `asm/` and `bin/`. It needs the virtual environment, and stops with a message if splat is missing.
3. `make` compiles each `src/*.c` file, assembles each `asm/*.s` file, links them with `NGVE.ld`, `symbols.ld`
   and the `undefined_*_auto.txt` files, copies the result into an 8 MiB ROM and checks it against `NGVE.sha1`.
   It prints `build/NGVE.z64: OK` on a match.

`asm/`, `bin/`, `build/`, `NGVE.ld` and `undefined_*_auto.txt` are generated and not tracked. Never edit them,
the linker scripts or `NGVE.sha1` to force a match.

Run `make setup` again after any change to `symbol_addrs.txt` or `glover.yaml`. Renaming a global changes
the labels splat writes, and the build reads the asm files that the C includes.

## Compiler and flags

The game was built with GCC 2.7.2 for MIPS (KMC's compiler is based on it). `make toolchain` builds FSF GCC
2.7.2 from [decompals/mips-gcc-2.7.2](https://github.com/decompals/mips-gcc-2.7.2) into `tools/gcc-2.7.2`.
It has no patches.

Flags (see the `Makefile`):

```
-G0 -mcpu=r4000 -mips2 -mmips-as -funsigned-char -O2
```

- `-mcpu=r4000` gives the ROM's instruction scheduling (`vr4300` does not).
- `-mmips-as` makes GCC write unsplit macros such as `lw $3,sym`. The ROM's assembler expected these. The default
  (`-mgas`) splits addresses into `%hi`/`%lo` itself and gives different code.
- Nine files are compiled with `-O0` (frame-pointer code): `decompress eeprom controller main jalloc text font
  banks sched` (`O0_FILES` in the `Makefile`). All others use `-O2`.

The SN64 build of GCC (`cygnus-2.7.2-970404`, from
[decompals/SN64-gcc](https://github.com/decompals/SN64-gcc)) is also built, into `tools/sn64`, for comparison
only. Its later CSE and scheduler changes do not reproduce the ROM (list helpers, AddVent, AddHandLight and
RemoveLight differ). `cmpfunc.py --configs` compiles a function with both.

`tools/mips-gcc-2.7.2` holds the compiler source. A cc1 rebuilt from it is byte-identical to the one in
`tools/gcc-2.7.2`. `tools/session/gtrace-build` adds trace output to a pass (see
[tools/session/README.md](../tools/session/README.md)).

## Assembler

The original assembler was SN's asn64. `tools/asn64.py` emulates it in front of GNU `as`. It is based on
[RocketRet/modern-asn64](https://github.com/RocketRet/modern-asn64) (Unlicense; the licence is in
[THIRD_PARTY_NOTICES.md](../THIRD_PARTY_NOTICES.md)). The compiler calls it through the `as`
wrapper that `make toolchain` writes into `tools/gcc-2.7.2`.

What it reproduces:

- Macro expansion: `lX reg, sym(base)` builds the address in `$at`; `l.d`, `s.d` and `li.d` become two
  `lwc1`/`swc1` (odd register first) or two `.word`s; `remu` expands as asn64 did.
- Hazard `nop`s:
  - a `nop` after a float compare before its branch, also when the branch opens a `noreorder` block;
  - a `nop` between a float multiply (`mul.s`, `mul.d`) and a following multiply when the hardware needs it.
    An integer `mult` does not set this state. When a multiply moves out of a delay slot, the check looks at
    the instruction before the branch;
  - a load or store from a bare symbol counts as two instructions for the `mflo`/`mfhi` hazard.
- `.rdata` maps to `.section .rodata`. C rodata and asm rodata interleave.
- `.ident` strings are written into `.rodata`; GCC's own `GCC: ...` ident is dropped.
- Doubles are only word-aligned.

If the only differences left in a function are missing or extra `nop`s, look at `asn64.py`, not at the C.

## ROM layout

- ROM: Glover (USA), NGVE, 8 MiB, SHA-1 `270be17b3c8da9b88a7b99c2a545b0bce16837f4`.
- Code starts at ROM 0x1000, loaded at 0x80100000. Section order: rodata, then text, data and bss
  (`glover.yaml`).
- Game code: 0x8010E0D0 to 0x8011C720 and 0x8011C7F0 to 0x801BC710, 853 functions. Everything else stays as
  assembly: libultra 2.0H, F3DEX 1.21 and other microcode, libmus, SN's host I/O library (libsn), the C library
  and libgcc.

## Source files

The game code is split into 56 files under `src/`, one per original object file as far as the evidence
shows. The original linker packed objects with no padding. File boundaries were inferred:

- Most original objects start their rodata with the pair `$Revision: 1.17 $` and `$Revision: 1.49 $` on a
  16-byte boundary. Each file whose first object has the pair includes `include/ident.h`, which writes the pair
  with `#ident`. Pairs of later objects inside a combined file stay as `ident_XXXXXXXX` INCLUDE_RODATA lines.
- Code compiled at `-O0` and `-O2` must be in separate files.
- A function the ROM calls in one place and inlines in another means a file boundary before the caller that
  does not inline it (nmeprocess.c and nmeprocess2.c).
- No rodata item may be used by two files. Overlay and screen effects are one file, `overlay.c`.
- Junk blocks between objects mark object ends (next section).

## Rodata and linker padding

- **Junk between objects.** The original linker padded each object's `.text` to 16 bytes with leftover bytes
  (strings, PE section names such as `.idata$4`, nops) that C cannot produce. They are `pad_XXXXXXXX` symbols
  in `symbol_addrs.txt`, kept as `INCLUDE_ASM` lines. A source file ends at each pad block's 16-aligned end.
- **Padding after rodata.** Words the linker left after a file's last constant (zeros or leftover bytes) are
  written as a
  `const u32 NAME[]` array at the end of the C file (for example `D_8010B1F4` in shadow.c, `D_8010877C` in
  sound.c). Where that padding is more than this build adds, the constant stays an `INCLUDE_RODATA` line and C
  reads it through an `extern` (`D_8010894C` in platform.c).
- **Double alignment.** The original linker aligned doubles by absolute address; GNU as aligns them relative to
  the section start. So a file with doubles starts its rodata at the 8-aligned address at or below its first
  constant, which can move up to 4 bytes of the previous file's rodata into it. `tools/splat_run.py` makes
  spimdisasm write doubles as `.word` pairs. GNU as never pads them.
- **INCLUDE_RODATA lines** stay where rodata cannot yet come from C: a string followed by junk in the same word,
  or a constant shared with an asm function. A function's own constants come from its C literals.

## Library headers

`include/PR/` and `include/ultra64.h` are the headers of Nintendo's N64 SDK (libultra, the graphics macros and the
sprite library). They keep their SGI and Nintendo notices. Other public N64 decompilations keep the same headers in
the same way, for example decompals/ultralib, n64decomp/sm64 and pmret/papermario. The game's code compiles against
them, and the display list macros must expand to the same code as in the original build.

## Symbols

- `symbol_addrs.txt` holds every name splat uses, with `type:` and `size:` where needed. Rename a global or a
  function with `tools/session/gname.py`, then run `make setup`.
- `symbols.ld` defines absolute symbols that splat cannot express: two odd instructions (`j 0x880332A0` at
  0x8011282C and a junk `bltz $zero` at 0x80132934) and symbols that share an address with another name
  (splat allows one name per address, but InitMusicDriver passes the end of one bank and the start of the next
  as two symbols).
- The first names came from a linker map of another build; see [sources.md](sources.md#the-linker-map).

## splat settings

- `generate_asm_macros_files: False`: splat's `macro.inc` defines COP0 register names such as `Random`, which
  clash with the game's `Random`. `include/macro.inc` is used instead.
- `migrate_rodata_to_functions: True`: a function's rodata goes into its own `.s` file. A C version of the
  function can emit it.
- splat writes a per-function `.s` file only for names that a C file references with `INCLUDE_ASM`.
- splat can cut a jump table short. A rodata split change needs the owner's approval.

## Tools in `tools/`

| Tool | Use |
| --- | --- |
| `setup_toolchain.sh` | build the compilers (run by `make toolchain`) |
| `asn64.py` | the assembler emulation (above) |
| `splat_run.py` | run splat with doubles written as word pairs (run by `make setup`) |
| `cmpfunc.py FILE FN` | compile one file and compare one function with the ROM (`tools/session/cf` wraps it) |
| `gbiconst.py rm\|cc W0 W1` | name the gbi macro for a render mode or combine mode word |

`tools/session/` holds the tools for everyday work; `tools/emu/` runs the game under script control;
`tools/patches/` makes modified ROMs. Each has its own README.

## Setting up a machine

The README's steps work on Debian and Ubuntu with:

```sh
sudo apt install git python3 python3-venv build-essential gcc-multilib \
    binutils-mips-linux-gnu bison gperf
```

`make toolchain` clones repositories from GitHub. `setup_toolchain.sh` backdates the compiler's `.y` and
`.gperf` files. make does not regenerate the parsers.
