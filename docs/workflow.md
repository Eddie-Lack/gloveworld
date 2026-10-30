# Workflow

This file explains how to change the code without breaking the match. Every tool named here is in
`tools/session/` and described in its [README](../tools/session/README.md).

## Every shell

```sh
. tools/session/env.sh
```

This sets `GW` (the repository), `S` (a scratch folder, `.session/`, ignored by git) and `B` (the ROM), and
puts `.venv/bin` and `tools/session` on `PATH`. Keep scratch files in `$S`. The commit script stages every
change in the tree.

## The basic loop

1. Make one change.
2. For a quick check of one function, run `cf FILE FN`. A matched function scores 64 (the baseline, not 0).
3. Run `bld`. It builds the full ROM and prints `ROM OK` or `ROM MISMATCH`. Only `bld` proves a match: `cf`
   hides relocations, constants and branch targets.
4. Commit with `commit "message"`. It runs `bld` first and refuses to commit a mismatch. It stages everything
   with `git add -A`.
5. Write down what you learned in the right document (see [CLAUDE.md](../CLAUDE.md#where-knowledge-goes)).

If the ROM does not match and you cannot see why within a fixed time, undo the change. A matching tree is
always worth more than a half-done rewrite.

## Renaming

| What | Command | Then |
| --- | --- | --- |
| a struct field | `rename.py STRUCT.FIELD=NEW` | `bld` |
| a global or a function | `gname.py OLD=NEW` | `make setup`, then `bld` |
| a local or a parameter | `lname.py FILE FUNCTION OLD=NEW` | `bld` |
| numbers that belong to an enum | `enumify.py [ENUM ...]` | `bld` |

- Each tool also takes a file of renames (`--file`), for batches.
- `rename.py` uses `xref.py`'s typed accesses. `unk1C` in one struct is not confused with `unk1C` in
  another.
- `lname.py` skips member names, strings and comments. A macro body that uses the local must be edited by
  hand.
- A wrong rename fails to compile or changes the code. `bld` catches it.
- Add each name and its evidence to [naming.md](naming.md).

## Finding out what something is

- `xref.py report STRUCT` summarises each field of a struct: reads, writes, values written, constants compared
  and the functions that use it. `xref.py field STRUCT FIELD` lists every access.
- The allocation tag in a `JallocAlloc` call often names a struct (`"Plat_Rest"`, `"Act_Shad"`).
- The PC port's local lists name locals, parameters and statics (see [sources.md](sources.md)).
- `tools/emu/` runs the game under a script. You can watch a field change while the hand walks, jumps or
  casts a spell.

## Rewriting a construct

Much of the matched C still contains constructs that exist only to steer the compiler. Each carries a comment
(see [conventions.md](conventions.md#comments)). To try a more natural form:

1. Find the construct by its comment.
2. Write the natural form as a variant and score it without editing the file: `tryv.py FILE FN VARIANTS.py`,
   where `VARIANTS.py` defines `V = {name: (old, new)}`.
3. For several sites, write the edits as JSON and run `tryrw.py SPECS.json`. It applies one site at a time,
   builds the full ROM after each, and keeps only the ones that still match.
4. If the natural form does not match, keep the current form and its comment. If you learned why, add it to
   [compiler.md](compiler.md).

Remove a local only if the match stays, and do not add original locals that would only be unused declarations.

## When the ROM no longer matches

1. `romdiff.py` prints the first differing ROM address and the object file it belongs to.
2. `cfx FILE FN` shows an aligned diff of the function against the ROM. `brcheck.py FILE FN` and
   `jcheck.py src/FILE.c FN` compare branch and jump targets, which `cf` and `cfx` hide.
3. `explain.py FILE FN` ties each difference to the compiler decision behind it (it needs `gtrace-build` once).
4. Look up the symptom in [compiler.md](compiler.md).

## Working with other agents

Agents are useful for wide, parallel work: reading every file for a pattern, proposing names, testing variants.

- Give each agent a brief with the rules it needs from [CLAUDE.md](../CLAUDE.md#rules), the paths it may write
  (a scratch folder, or its own git worktree), and the exact output format.
- Give agents extracted data, such as a JSON list of a function's original locals, not the PC port's files.
- Agents propose; the main session applies. Apply one change at a time with `bld`, or use `tryrw.py`.
- Have each result checked by a second agent for behaviour (does the code do the same thing?) and for
  plausibility (would a person have written it?).

## Commits and pull requests

- One topic per commit. The message says what changed and why, in plain language.
- The ROM must match at every commit, not only at the end of a branch.
- Write times in Pacific time.
- Never commit the ROM, a savestate or anything derived from the PC port beyond names.
