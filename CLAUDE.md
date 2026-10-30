# Guide for agents

This repository is a complete, matching decompilation of the N64 game Glover (USA, NGVE). Every game function
is C that GCC 2.7.2 compiles to the original ROM, byte for byte. The work now is to make that C read like the
original team's code: names, types and natural constructs. Read this file first, then the parts of
[docs/](docs/README.md) that your task needs.

## Rules

- **The ROM must match at every commit.** Build with `tools/session/bld`; it must print `ROM OK`. Commit with
  `tools/session/commit "message"`, which builds first and refuses a mismatch.
- **Never commit the ROM**, copy it into a tracked folder, or change it. Do not commit savestates either: they
  hold a copy of the game's memory.
- **Never edit generated files to force a match:** `asm/`, `bin/`, the linker scripts, `NGVE.sha1`.
- **Never use leaked original source code**, in any form. The PC port's `Glover.iobj` may be used for its
  local, parameter, static and enum names, and for nothing else.
- **Code adapted from another project needs its licence.** Name the source and its licence in a comment at the top
  of the file, and add the licence text to [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). The project's own
  licence is not chosen yet.
- **Name only what the evidence supports.** A placeholder (`unkNN`, `D_80123456`, `argN`) is better than a
  guess. Never name a global by its address in the PC port's map; check every name against its uses. Record
  each new name and its evidence in [docs/naming.md](docs/naming.md).
- **Write code a person would plausibly have written.** Check each rewrite with `cf` and `bld`. Never use a form that changes behaviour or adds a dead store.
- **Comment only constructs that exist to make GCC produce the ROM's code**, in one plain sentence that says
  why. Do not comment ordinary code. Follow the existing code style ([docs/conventions.md](docs/conventions.md)).
- **Time-box each attempt.** A rewrite of matched C that does not match is not committed.
- **Write all prose (docs, comments, commits, PR text) in plain language** (ISO 24495-1): short sentences, one
  idea each, terms defined on first use. No em dashes. No AI model names. Times in Pacific time.
- **Do not end sentences with a purpose clause** that explains why the preceding detail matters, such as "so
  that X instead of Y". State the fact and stop.

## Everyday work

```sh
. tools/session/env.sh        # sets GW, S (scratch, .session/) and B (the ROM); puts .venv/bin on PATH
tools/session/bld             # full build and checksum: ROM OK or ROM MISMATCH
```

| Task | Tool (in `tools/session/`) | Afterwards |
| --- | --- | --- |
| Rename a struct field | `rename.py STRUCT.FIELD=NEW` | `bld` |
| Rename a global or function | `gname.py OLD=NEW` | `make setup`, then `bld` |
| Rename a local or parameter | `lname.py FILE FUNCTION OLD=NEW` | `bld` |
| Put enum names for constants | `enumify.py` | `bld` |
| Find every use of a field | `xref.py field STRUCT FIELD` | |
| Score one function | `cf FILE FN` | confirm with `bld` |
| Try rewrites, each with a full build | `tryrw.py SPECS.json` | review `git diff` |

- `make setup` re-splits the ROM into `asm/`. It needs the virtual environment (`env.sh` puts it on `PATH`).
- `cf` scores a function against the ROM. A matched function scores 64 (the baseline), not 0. It masks
  relocations, constants and jump targets. Only `bld` proves a match.
- `commit` stages every change in the tree (`git add -A`). Keep scratch files in `$S`.
- When a rewrite no longer matches, look at what the compiler did, not at the source text.
  [docs/compiler.md](docs/compiler.md) explains which C forms give which code.

Full details: [docs/workflow.md](docs/workflow.md) (how to change code safely) and
[tools/session/README.md](tools/session/README.md) (every tool).

## Where knowledge goes

Put what you learn where the next reader will look for it:

| What you learned | Where it goes |
| --- | --- |
| A name and its evidence | [docs/naming.md](docs/naming.md) |
| How GCC 2.7.2 behaves; a C form that gives the ROM's code | [docs/compiler.md](docs/compiler.md) |
| Build, toolchain or splat facts | [docs/building.md](docs/building.md) |
| A rule for writing the C | [docs/conventions.md](docs/conventions.md) |
| How the game's code is organised | [docs/game.md](docs/game.md) |
| What a source of evidence can and cannot tell | [docs/sources.md](docs/sources.md) |
| Work done, work left, open questions | [docs/status.md](docs/status.md) |
| A large one-off study (a survey of one game mechanic, say) | a new file in [docs/reports/](docs/reports/) |
| How to use a tool | the tool's usage comment and [tools/session/README.md](tools/session/README.md) |

Update the topic document in place: correct or remove what is no longer true, rather than adding a newer note
below an old one. Session logs, timings and scores of past attempts do not go in the repository; the commit
messages and PR descriptions record what changed and why. The rules for the docs are in
[docs/README.md](docs/README.md#keeping-the-documentation-useful).
