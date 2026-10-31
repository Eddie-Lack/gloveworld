# gloveworld

A still in progress decompilation of Glover for the Nintendo 64 (North America, NGVE)

Technically 100% matching! The C code compiles to the exact bytes of the game's ROM. 

Very much a work in progress though :)

You need your own copy of the ROM!! It isn't in this repository!

![Code](https://img.shields.io/badge/Code-100.00%25-2fcc3b?style=flat) ![Functions](https://img.shields.io/badge/Functions-100.00%25-2fcc3b?style=flat)

## Where it stands

- All 853 game functions are C, and each one matches the ROM
- The only assembly left in `src/` is 42 blocks of linker padding (they aren't functions)
- `make` compares the built ROM with the original's SHA-1 hash
- Most structs, globals and functions have names, with the evidence for each in _[docs/naming.md](docs/naming.md)_
- What's left is making the C read like the original team's code: clearer names, real types, natural constructs
- Every rewrite must still match the ROM
- _[docs/status.md](docs/status.md)_ lists what's done and what's left
- Only game code is in scope
- libultra (Nintendo's N64 library), libmus (the sound driver), SN's host I/O library, the C library and libgcc stay as assembly

## Thanks

- [naclomi](https://github.com/naclomi) the goat! Figured out pretty much everything - the true hero
- The developers of N64 decompilation tools: [splat](https://github.com/ethteck/splat), [spimdisasm](https://github.com/Decompollaborate/spimdisasm), [asm-differ](https://github.com/simonlindholm/asm-differ), [decomp-permuter](https://github.com/simonlindholm/decomp-permuter) and [m2c](https://github.com/matt-kempster/m2c)
- [decompals](https://github.com/decompals) for their builds of GCC 2.7.2: [mips-gcc-2.7.2](https://github.com/decompals/mips-gcc-2.7.2) and [SN64-gcc](https://github.com/decompals/SN64-gcc)
- [RocketRet](https://github.com/RocketRet) for [modern-asn64](https://github.com/RocketRet/modern-asn64), which `tools/asn64.py` is adapted from
- The [Dr. Mario 64](https://github.com/AngheloAlf/drmario64) decompilation
- Thanks to Piko Interactive for providing some of the files used, check out [bleem.net](https://bleem.net)

## Building

You need:

- Python 3
- the `mips-linux-gnu` binutils
- a host C compiler with 32-bit support (`gcc-multilib`)
- `bison` and `gperf`
- your own copy of the ROM

```sh
python3 -m venv .venv && . .venv/bin/activate
pip install -r requirements.txt
make toolchain                  # builds GCC 2.7.2 into tools/
cp /path/to/glover.z64 baserom.us.z64
make setup                      # splits the ROM into asm/ and bin/
make                            # builds build/NGVE.z64 and prints "build/NGVE.z64: OK"
```

The ROM's SHA-1 is `270be17b3c8da9b88a7b99c2a545b0bce16837f4`

Your ROM has another name? Pass `BASEROM=<path>` to `make`

How the build works is in _[docs/building.md](docs/building.md)_:

- GCC 2.7.2 at `-O2` (nine files use `-O0`)
- an emulation of SN's asn64 assembler in `tools/asn64.py`, adapted from [RocketRet/modern-asn64](https://github.com/RocketRet/modern-asn64)
- [splat](https://github.com/ethteck/splat), which splits the ROM into source files

## What's where

| Path | Contents |
| --- | --- |
| `src/` | The game's C, one file per original object file |
| `include/` | Headers: structs, globals, prototypes, `enums.h` and `macros.h` (`include/PR/` has the SDK's headers) |
| `asm/`, `bin/` | Made by `make setup`: the ROM's code and data, split out by splat (not committed) |
| `docs/` | The documentation, starting with its _[index](docs/README.md)_ |
| `docs/reports/` | One-off studies, like the _[stored crystal report](docs/reports/space-boss-stored-crystal.md)_ |
| `tools/session/` | Everyday tools: build check, renames, scoring, compiler tracing _(see [README](tools/session/README.md))_ |
| `tools/emu/` | Scripted emulator runs, to find out what a field does _(see [README](tools/emu/README.md))_ |
| `tools/patches/` | Scripts and pages that make modified ROMs _(see [README](tools/patches/README.md))_ |
| `tools/` | Other build tools: assembler emulation, the splat wrapper, function comparison |
| `THIRD_PARTY_NOTICES.md` | The licences of the code adapted from other projects |
| `glover.yaml`, `symbols.ld`, `symbol_addrs.txt` | The splat configuration, the symbol addresses for the linker, the symbol names |

## Contributing

Read _[CLAUDE.md](CLAUDE.md)_ first - it's the rules for people and for robots

Then read _[docs/workflow.md](docs/workflow.md)_ to see how to change code without breaking the match

The main rule is that the ROM must match at every commit

- `tools/session/bld` builds and prints `ROM OK` or `ROM MISMATCH`
- `tools/session/commit` builds first and refuses to commit a mismatch
