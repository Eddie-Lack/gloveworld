# Session tools

Working tools for changing the code and checking the match. They are not part of the build. Each script starts
with a one-line usage comment. [docs/workflow.md](../../docs/workflow.md) shows how they fit together.

## Setup

Build the repository first (see the main [README](../../README.md#building)). Then source the environment in each
shell:

```sh
. tools/session/env.sh
```

It sets these variables. Each can be set beforehand to override the default.

| Variable | Meaning | Default |
| --- | --- | --- |
| `GW` | repository root | two levels above this folder |
| `S` | scratch folder for logs and variants | `$GW/.session` (ignored by git) |
| `B` | the base ROM (also exported as `BASEROM`) | `$BASEROM`, else `$GW/baserom.us.z64` |
| `JOBS` | worker processes for `make` | all cores |

It also puts `.venv/bin` (if present) and this folder on `PATH`.

`FILE` below is always the bare source name, for example `nmeprocess2` for `src/nmeprocess2.c`.

## Building and committing

- `bld`: build the full ROM and check its checksum. Prints `ROM OK` or `ROM MISMATCH`; the log is in
  `$S/build.log`.
- `commit "message"`: run `bld`, then commit every change in the tree (`git add -A`). Set `COMMIT_TRAILERS` to
  append trailer lines after a blank line.
- `romdiff.py`: after a failed build, print the number of differing bytes, the first one and its object file.

## Scoring one function

- `cf FILE FN...`: score functions with `tools/cmpfunc.py`. A matched function scores 64: with no asm file
  for the function, cmpfunc compares against 64 extra ROM words. Lower is closer. Pass `CFOPT=--opt=-O0` for
  the `-O0` files.
- `cfx FILE FN [CTX]`: aligned assembly diff of FN against the ROM (`cmpctx.py`).
- `brcheck.py FILE FN`: compare branch targets, which cmpfunc masks.
- `jcheck.py src/FILE.c FN`: compare the targets of `j` instructions, which cmpfunc and cfx mask.

cmpfunc masks relocations, constants and branch targets, so a good score does not prove a match. A wrong
store order, a wrong constant (for example a double that should be the `PI` macro) or a jump to the wrong label
only shows up in `bld`.

## Names

- `xref.py`: find every access to every struct field, typed by struct (fields that share a name such as `unk1C`
  are kept apart). Run it with no arguments to build `$GW/.session/xref.json`. `xref.py report STRUCT`
  summarises each field (reads, writes, values written, constants compared, functions); `xref.py field STRUCT
  FIELD` lists every access with its line.
- `rename.py STRUCT.FIELD=NEW ...` (or `--file NAMES.tsv`): rename fields in their declaration and in every
  access typed as that struct. Run `bld` afterwards; a missed or wrong rename stops the build.
- `gname.py OLD=NEW ...` (or `--file NAMES.tsv`): rename a global (data or function) in `symbol_addrs.txt`,
  `src/` and `include/`. Run `make setup` afterwards (splat writes the new labels into `asm/`), then `bld`.
- `lname.py FILE FUNCTION OLD=NEW ...` (or `--file TSV`: FILE, FUNCTION, OLD, NEW): rename locals and parameters
  inside one function. It skips member names, strings and comments, and refuses a name the function already
  uses. A macro body that uses the local must be edited by hand.
- `enumify.py [--dry] [ENUM ...]`: replace numeric constants with the names in `include/enums.h` where the
  context shows the enum (a comparison with `hand.state->mode`, the first argument of `ApplyAction`, an index
  into `pad[].button`, a case of a switch on a puzzle action's type, and so on). The rules are at the top of the
  file.
- `styleaudit.py IOBJ > report.md`: group the source files by their original authors' habits and list
  functions whose tricks are rare in their group, with our locals next to the original ones. It reads the PC
  port's `Glover.iobj` (the path is an argument) and `build/NGVE.elf`, so build first. The findings
  are in [docs/authors.md](../../docs/authors.md).

## Trying rewrites

- `tryv.py FILE FN VARIANTS.py`: score variants without changing `src/FILE.c`. `VARIANTS.py` defines
  `V = {name: (old, new)}` or `{name: [(old, new), ...]}`. Each pair is a single-occurrence text replacement.
  The variant is written to a temporary file in `src/` and removed afterwards. It compiles at `-O2` only.
- `apply.py FILE NAME [VARIANTS.py]`: apply one variant to `src/FILE.c` (default variants file: `$S/v.py`).
- `tryrw.py SPECS.json ...`: apply rewrites one site at a time, build the full ROM after each, and keep only
  those that still match. Each entry is `{"site": NAME, "alt": N, "edits": [{"file": "src/x.c", "old": ...,
  "new": ...}]}`; the alternatives of a site are tried in order. Run it on a clean tree, so that `git diff`
  shows exactly what was kept.

## Compiler pass dumps

When a register or an instruction is in the wrong place, look at what the compiler did, not at the source.
Compile a copy of the file with the dump flags, then cut out your function:

```sh
cp src/FILE.c $S/work/ && cd $S/work
COMPILER_PATH=$GW/tools/gcc-2.7.2 $GW/tools/gcc-2.7.2/gcc -c -G0 -mcpu=r4000 -mips2 -mmips-as \
  -funsigned-char -O2 -I$GW/include -I$GW/include/PR -I$GW/src -I$GW -D_LANGUAGE_C -DF3DEX_GBI \
  -D_FINALROM -DNDEBUG -DTARGET_N64 \
  -dr -dj -ds -dL -dt -df -dc -dS -dl -dg -dR -dJ -dd -S FILE.c -o FILE.s
awk '/^;; Function FN$/,/^;; Function [^F]/' FILE.c.lreg
```

This writes one file per pass (`.rtl`, `.jump`, `.cse`, `.loop`, `.cse2`, `.flow`, `.combine`, `.sched`, `.lreg`,
`.greg`, `.sched2`, `.jump2`, `.dbr`). The `.lreg` file lists each value's reference count, its live length and
whether it crosses a call ("Register N used R times across L insns"), and the hard register it was given. Work
out the first pass where the output can no longer become the ROM's code, then look for a plausible spelling that
changes that pass. [docs/compiler.md](../../docs/compiler.md) describes the passes and the C forms that steer
them. The compiler source is in `tools/mips-gcc-2.7.2` (fetched by `make toolchain`).

## Explaining compiler decisions

`gtrace-build` copies `tools/mips-gcc-2.7.2` to `tools/gcc-trace-src`, applies `gtrace.patch` and builds a cc1 into
`tools/gcc-trace`. Without `GTRACE` set, its output is identical to the shipped cc1 (checked on all `-O2` files).
With `GTRACE=1` (and `GTRACE_FN=name` to limit it to one function) it prints to stderr:

| Line | Meaning |
| --- | --- |
| `PRI1`, `PRI2` | each insn's scheduler priority in a basic block, before sched1 (before register allocation) and sched2 (after) |
| `SCHED1`, `SCHED2` | the order each scheduling pass chose |
| `ADJ` | each priority adjustment in sched1: insn, deaths, whether it makes a register live, and the destination's set count |
| `PREF` | each register pair global allocation looks at for preferences (`set_preference`): destination, source (as hard registers when already allocated) and whether it is a plain copy |
| `DBR` | each delay slot filled by reorg: the branch, whether it is annulled, the insns in its slots, and the pass |
| `LIVE` | `mark_target_live_regs`: the registers live at a branch target, after the block scan, the forward scan (with the first unconditional jump it followed) and the intersection with that jump's target |
| `THREAD` | each `fill_slots_from_thread` attempt: branch, thread, opposite thread, direction, whether the thread is owned, and what the opposite thread needs |
| `EAGER` | each branch `fill_eager_delay_slots` looks at: pass, branch, target label, `mostly_true_jump` prediction (2 means very likely), and whether it owns the target and fall-through threads |
| `XJUMP` | each cross-jump in the jump passes: the jump, the insn whose copy is kept, and the start of the copy that is dropped |
| `LA` | each local-allocation quantity: its pseudos, references, size, birth, death, priority and hard register |
| `GA` | each global-allocation pseudo in allocation order: references, live length, size, priority and hard register |

Two switches change reorg's decisions, for experiments only (both need `GTRACE=1`): `GTRACE_PRED=UID:N` sets
`mostly_true_jump`'s prediction for branch UID to N, and `GTRACE_SKIP=UID` leaves branch UID unfilled in reorg's
first pass, so the second pass fills it.

Hard registers are numbered 2 and 3 for v0 and v1, 4 to 7 for a0 to a3, 16 to 23 for s0 to s7, and 32 and up for
$f0, $f1 and so on.

`explain.py FILE FN [VARIANTS.py NAME]` compiles the function (with a tryv variant applied, if given) and goes
through its diff against the ROM. Each of our instructions is tied to its exact insn and source line: the compile
uses `-dp` (insn numbers in the assembly) and `-g` (line numbers; the code is the same), and the assembler's
listing gives each line's address. For each register that differs, it names the pseudo in that register, prints
the pseudo's allocation line, and lists the values that got the ROM's register in the same block. For each
instruction in the wrong place, it prints the priorities and the order of its block in both scheduling passes.
Allocation priority is `floor_log2(refs) * refs * size / live length`: higher goes first and takes the lowest
free register.
