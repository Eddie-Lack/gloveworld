# Documentation

Start with [CLAUDE.md](../CLAUDE.md) (the rules and the everyday commands), then read what your task needs.

| File | Read it when you want to know |
| --- | --- |
| [workflow.md](workflow.md) | how to change the code without breaking the match |
| [conventions.md](conventions.md) | how to write the C: layout, names, types, comments, steering constructs |
| [compiler.md](compiler.md) | why GCC 2.7.2 produces a certain instruction, and which C form gives the ROM's code |
| [building.md](building.md) | how the build works: compiler, assembler, splat, file layout, linker padding |
| [game.md](game.md) | where things are in the game code |
| [naming.md](naming.md) | where each name came from, with its evidence |
| [sources.md](sources.md) | what each source of evidence (ROM, linker map, PC port) can and cannot tell |
| [authors.md](authors.md) | the habits of the original authors, for names we invent |
| [status.md](status.md) | what is done, what is left, and the open questions |
| [reports/](reports/) | one-off studies, such as the [stored crystal report](reports/space-boss-stored-crystal.md) |

The tools have their own READMEs: [tools/session/](../tools/session/README.md) for everyday work,
[tools/emu/](../tools/emu/README.md) for running the game under a script.

## Keeping the documentation useful

- **One topic per file, one place per fact.** Put a new fact in the file whose topic it belongs to (the table
  in [CLAUDE.md](../CLAUDE.md#where-knowledge-goes) says which). Link to it from elsewhere instead of repeating
  it.
- **Update in place.** When something changes, correct or remove the old text. Do not add a newer note below
  an old one, and do not keep notes that are no longer true.
- **No logs.** Session notes, timings, scores of past attempts and to-do lists of finished work do not go in
  the docs. Commit messages and pull requests record what changed and why. [status.md](status.md) holds what
  is left.
- **Reports are snapshots.** A file in `reports/` describes the code at one date and says so at the top. Do
  not update it; write a new report, and move lasting findings into the topic files.
- **Examples name a function.** A rule about the compiler or the code gives a function where it applies. The
  reader can check it in `src/`. When you rename a function or local that a document cites, update the
  document.
- **Plain language** (ISO 24495-1): short sentences, one idea each, terms defined on first use. No em dashes.
  No AI model names. Times in Pacific time, dates as 2026-10-07.
