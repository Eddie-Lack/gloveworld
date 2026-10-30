# Status

This file says what is done, what is left and what is still unknown. Update it when you finish or start a piece
of work. Counts can be recomputed with the commands given.

## Done

- **Matching.** All 853 game functions are C, and the ROM matches byte for byte at every commit.
- **Names.** About 427 struct fields, 531 globals and 45 functions have names, with the evidence in
  [naming.md](naming.md). The original enums from the PC port are in `include/enums.h` and used in the code.
- **Types.** Stand-in structs that described only a few bytes were replaced by the real types.
- **Locals and parameters.** 600 matched functions have a local list in the PC port. In 325 of them, every
  local and parameter of ours has a name from that list. Overall, 1,169 locals and 894 of 934 parameters carry
  the original names. No decompiler temporaries (`temp_f22`, `var_s0`, `sp28`) are left.
- **Author styles.** The files were grouped by the habits of their original authors ([authors.md](authors.md)).
- **Vector macros.** The pointer-sum forms and pointer locals that steered the compiler were vector macros given
  an address. All of them that matched now use the macros. Plain vector code that compiles the same way uses
  them too: x, y, z statement groups, dot products, lengths and chained zeroing, about 830 uses in all
  ([compiler.md](compiler.md#memory-aliasing-and-addressing)).
- **Headers.** Each source file whose functions other files call has a header in `include/` that declares them
  (all but frontend2.c, main.c, options.c, sched.c and title.c), and declarations do not sit between
  functions. About 300 local prototypes remain where a caller needs different parameter
  types from the definition, or where the header cannot see a type.
- **Decompiler leftovers.** MaintainAction no longer has decompiler-style hex counts, casts or parentheses.
  Sample numbers are decimal everywhere, and enum names replace raw hand modes, levels and actor types.

## Left to do

### Placeholders

What is left has no evidence for a name. The code does not use these values. The ROM's own demo recordings,
played in the emulator in every level that has one (`tools/emu/demoplay.py`, with the values logged every game
frame), show only constant values for most of them:

- **5 struct fields** named `unkNN`:
  - `Backdrop.unk64`: read from the level file, never used, and 0 in every backdrop (27 levels have one) and
    on the end-of-level screen;
  - `ObjectBank.unk00`: read in FindObject, never written, and 0 in all four banks that hold data, in every level;
  - `GameSprite.unk1C`: cleared when a sprite is added, and 0 in every sprite of every frame (up to 200 sprites at
    once);
  - `EolItem.unk0C`: only cleared, and 0 in all seven items throughout the end-of-level screen of 40 levels;
  - `PlatformCatapult.unk16`: only cleared, when the catapult catches the hand. No level has a catapult platform.
    A catapult built in memory (`tools/emu/catapult.py`) ran the game's own `CheckHandCatapult` through two full
    cycles (catch, wind-up, launch, reset). `unk16` was 0 in every game frame, and a marker value written
    before the catch was cleared by the catch and never changed.

  Recount with `xref.py` (it writes `.session/xref.json`).
- **No data global** is left. Two `D_` names are deliberate (see [naming.md](naming.md#globals)), and the other 37 are
  strings, constants and linker padding, which need no other name.
- **`func_8011C7F0`** only calls FreeBackdropList and has no callers. Its address is also the end of the
  `rspboot` microcode, which the audio library reads as `rspbootTextEnd`. The PC port has no counterpart. With
  the first instruction replaced by a branch to itself, three levels with their end-of-level screens and the
  present screen never hung. A debugger breakpoint on it was never reached either (title screen, three levels).
- **1 `argN` parameter:** TransformAndDrawObject's `arg3` is only passed on to the recursive calls, and the
  one outside call passes the constant 0. A debugger breakpoint on the function saw `arg3` = 0 in all 65,524,
  87,624 and 26,884 calls of three level runs.

Four levels have no end-of-level screen in the runs: HUB1 (its recording is empty, so the demo
never ends), and SPACEBOSS_TWEEN, WAYROOM and PRESENT (the demo ends in the menu).

Find them with `grep -nE '\barg[0-9]+\b|\bD_[0-9A-F]{8}\b|\bfunc_[0-9A-F]{8}\b' src/*.c`.

### Steering constructs

About 80 comment lines in `src/` mark constructs that exist for the compiler, or note linker padding. Most are
register allocation or scheduling tricks. Each natural form that matched has been applied. A new idea for one
of the rest is worth testing; compare the whole object (`objdump -d -r`), since `cf` can score a shorter
function as matched.

Constructs whose original form is unknown:

- acosBetter's two nested loops around the table lookup. Both levels are needed; `for (;;)` also works as the
  outer one, and no single loop matches. The ROM has no loop: the loop depth only raises the weight of the
  register references (flow counts a reference 3 times at depth 2), which changes the float register
  assignment. A grid of 729 placements showed that the lookup, the division and the test must be at depth 2.
  The original was probably a macro holding `return a`, or another construct whose loop notes are all that is
  left.
- The flag locals `isHand` (ProcessShadows) and `bounce` (CheckActorCollisions). Neither is in the PC port's
  local lists. `isHand` also matches as a call to a `static inline` predicate, which is no clearer. `bounce` is
  always 1, and the ROM loads and tests the constant: the original probably had a variable whose other
  assignments are gone.
- MoveCam's dead initializer `Vec3 *plane = NULL;`. Without it CSE folds the copy `plane = floor` away and
  `plane` gets `$s2` where the ROM has `$s0`. A second set of `plane` is needed, and MoveCam has no other use
  of it. Assigning `plane` in the floor test, moving the declaration or dropping `plane` all differ.
- DrawBlurFX's strip loop. The PC port has two `quad` locals and no `j` or `k`. The ROM's counter is a vertex
  index stepping by 2, not a quad counter, and the second batch's bound needs its own variable (written inline,
  loop.c hoists it into a new register). One `quad` may be that bound and the other the counter.

Stack pads with no known original local: IsActorVisible (64 bytes), DoEarthquake,
ComputeResultMatrix, initControllers, LoadRecordKeyFile and MaintainAction (`unused[6]`). LoadLandscapeFile's
slot is the PC port's `fn`, which our code does not use.

### Locals

In October 2026 we compared every function's locals and parameters with the PC port's lists. 125 functions
had names on both sides with no partner. The renames that were clear by type and role are done. Then the
cases where one of our locals stands for two original ones, or two for one, were tried; each kept change gives
the same object file. Done: ApplyAction, MoveHand, OrbitPlatform and NMEProcessRobes lost a local, and
QuatSlerpAtFixedSpeed and its Old version split `angle` into `tempFloat` and `tempFloat2`. UpdateWinds uses a
`Vec3 tempVect` where it had `f32 pos[3]`.

A second pass looked at 134 functions where we have locals that the original lists do not. Locals that only
held a copy of a plain value could go: 32 locals in 17 functions (for example UpdateActor's `period`,
FreeLevel2's `i` and `m`, the eight differences and sums in each slerp function). Removing a pointer alias (`actor`, `ins`,
`br`) or joining two float scratch locals almost always changed the register choice, and joining two vectors
changed the stack frame.

Tried and left, because every form changed the code:

- One of ours for two originals: RotateVectorByRotation (`t` for `cosTheta`, `m`).
- Two of ours for one original: HandWhackActor (`handMass`, `mass`), ConditionsMet (`save`, `w`),
  MaintainAction's floats, WaitNameScr (`row`, `col` for `j`), SelectLanguage (`k` into `i`), UpdateActor
  (`period` into `val`), MoveBall (`f2`, `len`, `lim` for `val`), NMEActionMoveTo (`dist`, `scale` into
  `temp`), NMEProcessRobot (`dist`, `d` for `val`), CheckCollisions (`e` into `tempPoint`), OrbitPlatform
  (`diff` into `ang2`), UpdateWinds (`actor` into `actPtr`), LoadTextureBank (`cur` into `tex`).
- Locals that the original did not have: NMEProcessSelwynWhale's `step`, DrawGraphics' `moved`, HiscoresRun's
  f32 `temp`, ParseTokenGeneric's loop counter `crc`.
- Live at the same time, so no join is possible: DoCameraCollision (`n`, `sum`), NMEProcessSwish (`vel`,
  `tempVect`), SpinPlatform (`d`, `target` with `ang`), UpdateGaribs (`handDist`, `ballDistance`).
- Unclear roles: MaintainBulletBomb (our red `r` against the original `a`), HandSwish (three matrices for
  `rotmat`, `scalemat`), RotateVector2DXYZ, DrawLightning.

Some PC signatures differ from ours and were left: NEW_TEXT, font_show_string, PlayContinuousSample,
SetKlosetSpeed, StartEarthquakeAtPoint, AddSpellDebris. `styleaudit.py` lists our locals next to the
original ones.

Checked and left as they are:

- The PC port has a `SaveSlot *slot` in 13 functions. The ROM recomputes `saveSlot + index` at every access.
  The N64 code indexed `saveSlot[currentSaveSlot]` directly. A pointer would sit in a saved register.
- HandWhackActor's PC local `temp` does not fit: no form with one float matches.

## Open questions

- **The PC port's enums.** `include/enums.h` uses the enum constant names from Glover.iobj's `S_CONSTANT`
  records. The project rules allow the iobj's local names; whether they also cover its enum names was raised
  with the owner and not objected to.
- **The author groups** are a working guess. Nothing in the data names a person, and three to seven groups fit
  almost equally well.
- **Unexplained stack slots.** RotateVector2D, CalcMaxActorRadius, NearEdges and CheckPlaneForSliding have a
  Vec3 slot that no listed local explains; perhaps an inlined helper or a nested block.
- **Debris loops.** Eight functions have our `Debris *d` where the PC port has an `int` (`x` or `y`). The PC
  port's AddDebris returns an index, but the N64 one returns a pointer. The PC locals do not carry over.
