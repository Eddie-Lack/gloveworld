# Author styles

This note compares the habits of the people who wrote Glover's C. Code we write ourselves (names,
spellings the ROM does not fix) can follow the habits of the file's author.

## Evidence

Our C is our own writing. Its layout and most of its spellings say nothing about the original authors. Two
kinds of evidence do:

- **The PC port's local lists** (Glover.iobj, CodeView records). They give the original names of the locals and
  parameters of 678 functions, and basic types for ints and floats. They also list globals that the PC
  compiler kept in a register (for example `frameCount`, `camera`), which are not locals.
- **The ROM.** It fixes the code, and with it some habits: double or single precision in float maths,
  `__sinf` against `sinf`, 16-bit types, function size, and inline helpers.

`tools/session/styleaudit.py` groups the files by these habits (Ward's method on per-file rates). The percentage is how often a file stays in its group when one habit is
left out. The credits name three programmers and two additional programmers. Four groups is plausible, but
nothing names a person.

| Group | Files |
| --- | --- |
| 1, actors and enemies | actor, bullet, shadow, anim, puzzle, vent (77 to 81%), enemy, wind, nmeprocess, water, collect, debris |
| 2, menus and loading | options, personal, frontend2, title, collision, credits, parse, cameo, game (76 to 84%), banks, main, frontend |
| 3, effects and drawing | overlay, maths, footprint, blur, text, buzzer, light (68 to 77%), quake, spell, weather, font, sound |
| 4, gameplay physics | ball, bonus, camera, controller, boss, nmeprocess2, hand (88%), platform, tip, object |

## Naming habits (original locals)

The most common original names of each kind, per group, with counts:

| Kind | Group 1 | Group 2 | Group 3 | Group 4 |
| --- | --- | --- | --- | --- |
| int | x 23, i 14, j 11, y 5 | i 34, j 21, k 12, x 9 | x 7, i 5, j 3 | x 23, j 20, i 20, k 5 |
| float | angle 7, dist 7, temp 5, scale 5, val 4 | m 4, x 4, y 4, z 4, t 3 | temp 6, scale 3, dist 2 | angle 9, temp 8, tempFloat 7, val 6, dist 5 |
| Vec3 | tempVect 42, tempVect2 7, vect 4 | tempVect 7, rot 2, pos 2 | tempVect 2, pos 1 | tempVect 41, tempVect2 17, tempVect3 2 |
| other | ptr 10, bullet 9, platform 8, tempQ 6 | slot 15, platform 9, sval 8 | ptr 15, vtx 5, ptr2 3 | platform 12, plane 11, plat 11, tempQ 7 |

What this means for names we invent:

- Groups 1 and 4 use `x` as a loop counter as often as `i`, call scratch vectors `tempVect`, `tempVect2`,
  `tempVect3`, scratch quaternions `tempQ`, and scratch floats `temp`, `tempFloat` or `val`. Group 4 likes
  long `temp...` names; group 1 more often names floats by meaning (`angle`, `dist`, `scale`).
- Group 2 counts with `i`, `j`, `k` and uses short float names (`m`, `t`, `x`, `y`, `z`).
- Group 3 walks lists with `ptr` and `ptr2` and has few statics.

## Float constants

Where precision changes the code, the ROM shows whether the author wrote `0.5` (double maths) or `0.5f`. In
every group about one constant in five is double: group 1 23%, group 2 15%, group 3 18%, group 4 24%. So the
`f` suffix habit does not tell the authors apart, and constants that do not affect the code (a lone constant
assigned to a float) keep the spelling they have.

## Applied

- 148 locals and 206 parameters use their original names. Prototype parameter names match the definitions.
- MaintainAction (group 4) uses the PC port's `temp`, `scale` and `j` where the role matches, and group 4 style
  names (`tempFloat`, `val`) for the other scratch floats.
- No decompiler temporaries (`temp_f22`, `var_s0`, `sp28`) remain. 46 `argN` parameters remain, in functions that the PC
  port does not list or lists with a different number of parameters.

## Limits

The PC port is a later build compiled by another compiler, and its code can differ in shape (a loop where the
N64 code is unrolled, an extra temporary). A local the PC list lacks is evidence, not proof. The groups are a
working guess: the merge costs for three to seven groups are close.
