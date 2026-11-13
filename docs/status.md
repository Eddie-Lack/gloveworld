# Status

This file says what is done, what is left and what is still unknown. Update it when you finish or start a piece
of work. Counts can be recomputed with the commands given.

## Done

- **Matching.** All 853 game functions are C, and the ROM matches byte for byte at every commit.
- **Names.** About 429 struct fields, 531 globals and 46 functions have names, with the evidence in
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

What is left has no evidence for a name. The code never uses these values:

- **3 struct fields** named `unkNN`:
  - `Backdrop.unk64`: read from the level file, never used, and 0 in every backdrop (27 levels have one) and
    on the end-of-level screen;
  - `ObjectBank.unk00`: FindObject copies it into a local that it never uses. It is never written, and 0 in all
    four banks that hold data, in every level;
  - `PlatformCatapult.unk16`: only cleared, when the catapult catches the hand. No level has a catapult platform.
    A catapult built in memory (`tools/emu/catapult.py`) ran the game's own `CheckHandCatapult` through two full
    cycles (catch, wind-up, launch, reset). `unk16` was 0 in every game frame, and a marker value written
    before the catch was cleared by the catch and never changed.

  Recount with `xref.py` (it writes `.session/xref.json`).
- **1 `argN` parameter:** TransformAndDrawObject's `arg3` is only passed on to the recursive calls, and the
  one outside call passes the constant 0. A debugger breakpoint on the function saw `arg3` = 0 in all 65,524,
  87,624 and 26,884 calls of three level runs.
- **No data global** is left. Two `D_` names are deliberate (see [naming.md](naming.md#globals)), and the other 37 are
  strings, constants and linker padding, which need no other name.
- **No `func_` name** is left.

Two more fields have names at low confidence, from the code around them: `GameSprite.sameSpriteTex` and
`EolItem.velY` ([naming.md](naming.md)).

What was run in the emulator for these six values:

- **Watching.** The ROM's own demo recordings, played in every level that has one (`tools/emu/demoplay.py`, with
  the values logged every game frame), show only the constant values above. Four levels have no end-of-level
  screen in those runs: HUB1 (its recording is empty, so the demo never ends), and SPACEBOSS_TWEEN, WAYROOM and
  PRESENT (the demo ends in the menu).
- **Poking.** `tools/emu/poke.py` plays a level's demo twice: once untouched, and once with a marker written into
  every instance of the five values that levels have, on every video frame (`arg3` through a code cheat).
  Over all 44 levels that is 40,691 game frames and about 2.6 million marker writes. In 43 levels the game's
  logic state (hand, ball, camera, handControl, random seeds), the sprites and backdrops and the pictures were the
  same. Three of them had one or two single frames that differed, as two untouched runs of the same level also
  have. ATLANTIS3A is the exception: its untouched runs take one of two courses from
  about frame 10 on, and runs with the sprite marker took other courses. A read watch on the sprite byte in that
  level, over 26 game frames, saw no reader but SubSprite, which skips one store when the value is not 0. With
  that branch replaced by a `nop` (`POKE_NOBRANCH=1`), three runs with the sprite marker were the same as an
  untouched run in all 603 game frames. The marker changes only how long SubSprite takes.
  `catapult.py --poke` does the same for `unk16` through a full catapult cycle: every other catapult field and the
  hand's contact point and state were the same in all 325 logged game frames.
- **SubSprite's rule.** `poke.py --rule` sets `sameSpriteTex` to 1 in every sprite once every 60 game frames and
  follows each sprite with breakpoints on the two stores.
  Over all 44 levels the game never gave a sprite a value other than 0. 62,958 sprites entered the list, all with
  0. Of the 2,050 sprites that lost their 1 while in the list, 2,048 had passed through AddSprite again and 2 were
  cleared by SubSprite's rule. 39% of neighbours in the sprite list had the same texture.

Other sources checked, with no result:

- The PAL ROM (NGVP) has the same instructions as the USA ROM in AddSprite, SubSprite, FindObject,
  EndOfLevelScreen, ParseTokenGeneric and TransformAndDrawObject, and around the `unk16` store in
  CheckHandCatapult.
- The PC port's lists give TransformAndDrawObject three parameters and FindObject no locals. They name the
  struct types but not their members. Its code analysis files name a few members, none of these.

Find the placeholders with `grep -nE '\barg[0-9]+\b|\bD_[0-9A-F]{8}\b|\bfunc_[0-9A-F]{8}\b' src/*.c`.

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
- **Map names not taken.** The map has `spinBnc`, `minSpot` and `maxSpot` where we have the struct `fgSpin`.
  Three separate globals make the code 16 bytes longer, so the struct stays. The map's `bounceVel` sits at our
  `conveyorVel`, but the sizes differ (0x10 against 0x18) and every use is a conveyor speed.
- **The author groups** are a working guess. Nothing in the data names a person, and three to seven groups fit
  almost equally well.
- **Unexplained stack slots.** RotateVector2D, CalcMaxActorRadius, NearEdges and CheckPlaneForSliding have a
  Vec3 slot that no listed local explains; perhaps an inlined helper or a nested block.
- **Debris loops.** Eight functions have our `Debris *d` where the PC port has an `int` (`x` or `y`). The PC
  port's AddDebris returns an index, but the N64 one returns a pointer. The PC locals do not carry over.
