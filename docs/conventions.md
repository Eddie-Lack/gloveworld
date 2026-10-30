# Code conventions

These rules keep the C consistent and close to what the original team could have written. The ROM decides
what the code does; these rules decide how we write it.

## Layout

- Four spaces per indent, braces on the same line, a space after `if`, `for`, `while` and `switch`.
- Pointers are written `Type *name`.
- A struct definition gives each field's offset in a comment: `/* 0x1C */ s16 timer;`.
- Shared structs, globals and prototypes go in the subsystem's header in `include/`. A struct or global used by
  one file only is declared at the top of that file.
- A `src/X.c` whose functions other files call has a header `include/X.h` with their prototypes, and other
  files include it instead of repeating them. File-scope declarations go at the top of a file, never between functions.
- Every function a file calls needs a prototype in scope. Without one GCC passes floats as doubles (see
  [compiler.md](compiler.md#types-prototypes-and-conversions)).

## Names

Placeholders stay until the evidence supports a name:

| Placeholder | Meaning |
| --- | --- |
| `unkNN` | a struct field at offset 0xNN |
| `D_XXXXXXXX` | a global at that address |
| `func_XXXXXXXX` | a function at that address |
| `argN` | a parameter |

A placeholder is better than a guess. When you replace one, add a row to [naming.md](naming.md) with the
evidence and a confidence.

Where names come from, in order:

1. **The original name**, when a source gives it: the linker map for globals and functions, the PC port's
   `Glover.iobj` for locals, parameters, function statics and enum constants (see [sources.md](sources.md)).
   Keep the original spelling, even where it breaks the pattern (`font_xpos`, `doCopyMode`).
2. **A descriptive name** that says what the value is, based on how the code uses it, what it is allocated
   as (JallocAlloc tags such as `"Act_Shad"`) and what it does in the game (see the emulator in `tools/emu/`).

Rules:

- Functions and types use `CamelCase`. Fields, globals and locals use `camelCase`. Enum constants and macros
  use `UPPER_CASE`. These follow the original names.
- Never take a name from the PC port's map by address. Its layout differs from the N64 build. Check every name
  against the code that uses it.
- A function static in the original is a file-scope global in our code. When two functions' statics share a
  name, give one a short prefix.
- Use the PC port's name for a local or parameter only when it has the same type and role as ours. Where two
  locals share a type, the role decides (a search's bounds are `low` and `high`; the second of two vectors is
  `tempVect2`).
- Screen handlers use the PC port's parameter names, `(s32 *act, struct Screen *me)`, where the port has the
  handler.
- A new local name follows the habits of the file's author group: see [authors.md](authors.md). For example,
  groups 1 and 4 call scratch vectors `tempVect`, `tempVect2`, and group 2 counts with `i`, `j`, `k`.

## Types

- Use libultra's integer and float types (`s8` to `s64`, `u8` to `u64`, `f32`, `f64`) and the vector types in
  `include/types.h` (`Vec3`, `Quat`, `Box`).
- A field's type comes from the instruction the ROM uses to load or store it (`lb`, `lbu`, `lh`, `lhu`,
  `lwc1`). An intuitive type that changes the code is wrong.
- Do not keep stand-in structs that describe only the bytes one function uses. Find the real type (see "Stand-in
  types" in [naming.md](naming.md#stand-in-types)).
- A prototype may declare a narrower parameter type than the definition where the callers' code needs it (for
  example `FlashScreen` in bullet.c). Such a prototype stays in the calling file, not in the header. Add a
  comment when you write a new one.

## Enums and constants

- `include/enums.h` holds the game's enums. Their constants come from the PC port; the enum type names are ours.
  Use the constant wherever the context shows the enum. `tools/session/enumify.py` applies them.
- Write float literals in the precision the ROM shows (`0.5` for double maths, `0.5f` for float).
- `PI` is the macro for `3.141592654`; use it where the ROM's double is derived from that value.

## Macros and inline functions

- `include/macros.h` has `MIN`, `MAX`, `ABS` and the vector macros (`ADD_VECTORS`, `SUB_VECTORS`,
  `ADD_TO_VECTOR`, `SUB_FROM_VECTOR`, `SCALE_VECTOR`, `SCALE_VECTOR_2D`, `COPY_VECTOR`, `ZERO_VECTOR`,
  `DOT_PRODUCT`, `LENGTH`, `LENGTH_2D`). Use them where they give the ROM's code; they also read better than
  three field statements.
- Pointer arithmetic such as `(base + i)->x`, or a pointer local set to a global's field (`hp = &hand.pos;`),
  is usually a vector macro given an address. Try the macro first (see
  [compiler.md](compiler.md#memory-aliasing-and-addressing)).
- Do not change an existing macro's definition. Matched code depends on it. Where a function needs a different
  form (for example the reversed comparison of `MIN`), write the expression out in that function.
- A multi-statement macro is written `do { ... } while (0)`.
- Use `inline` where the ROM inlines a function that also exists out of line, and a separate `static inline`
  copy where only some callers inline it. See [compiler.md](compiler.md#inlining-and-macros).

## Comments

Comment only constructs that exist to make GCC produce the ROM's code. The comment is one plain sentence that
says why the construct is there:

```c
s32 pad[4]; // forces the original stack frame size

// a flag keeps CSE from replacing actor with &hand in this branch
isHand = actor == &hand;
```

- When the original form is unknown, say so: `// unknown construct; ...`.
- Do not comment ordinary code, and do not write history, scores or dates in comments.
- Every steering construct carries such a comment. This makes the remaining tricks easy to find and review.

## Steering constructs

Sometimes only an odd form gives the ROM's code. Prefer, in this order:

1. A form a person plausibly wrote: a vector macro, an inline helper in the same file, a reused local, a
   chained assignment, the same statement in both branches.
2. A form that is odd but harmless, with a comment: a flag local, an empty `case`, a `pad` array, a reversed
   comparison.
3. A construct whose original form is unknown, with an `// unknown construct` comment.

Never use a form that changes behaviour, reads a value before it is set, adds a dead store, or uses
`volatile` to force a reload. Check each rewrite with `cf` and `bld`.

## Stack pads and linker padding

- A frame that is bigger in the ROM than in our build needs a local. First look for an original local that
  fills it (the PC port's local list, an inlined helper's local). Only then use `s32 pad[N];` with a comment.
- Words the linker left after a file's last constant are written as `const u32 NAME[] = { ... };` at the end of
  the file. See [building.md](building.md#rodata-and-linker-padding).

## Rewrites that do not match

Every game function matches. A rewrite of matched C that does not match is not committed. Time-box each
attempt, and note a useful dead end in [status.md](status.md).
