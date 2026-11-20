# How GCC 2.7.2 compiles our C

This file explains which C forms make GCC 2.7.2 produce which machine code. Use it when an edit breaks the ROM
match, or when you want to know why a function is written the way it is. Each entry names a function where the
form is used, and that function is in `src/`.

The compiler is FSF GCC 2.7.2 for MIPS, with the flags in [building.md](building.md#compiler-and-flags). Its
source is in `tools/mips-gcc-2.7.2` (fetched by `make toolchain`). File and function names below, such as
`cse.c` or `find_best_addr`, refer to that source.

## Terms

- **RTL** is the compiler's internal form of the code, a list of **insns** (instructions) per function.
- A **pseudo** is a virtual register. Register allocation later gives each pseudo a hard register or a stack
  slot.
- A **basic block** is a run of insns with one entry and one exit. A label starts a block; a jump ends one.
- **CSE** (common subexpression elimination, `cse.c`) finds values computed twice and reuses the first one.
  It runs twice: CSE1 before the loop pass and CSE2 after it.
- **Hoisting** moves an insn out of a loop. It runs once (`loop.c`).
- **Cross-jumping** (`jump.c`) merges two identical code tails into one copy. The pass after register
  allocation is called jump2.
- A **delay slot** is the instruction after a branch, which runs before the branch takes effect. The
  `reorg.c` pass fills it. A **branch-likely** instruction (`beql`, `bnel`) cancels its delay slot when the
  branch is not taken.
- **Store-flag** code computes a comparison into a register (`slt`, `sltu`) instead of branching on it.

## Finding the cause of a difference

1. Compile the file with the per-pass dump flags (see [tools/session/README.md](../tools/session/README.md),
   "Compiler pass dumps") or run `explain.py FILE FN`, which ties each difference to an insn, a source line and
   an allocation or scheduling decision.
2. Find the earliest pass after which our code can no longer become the ROM's code. The pass order is:
   tree folding, RTL expansion, jump, CSE1, loop, CSE2, flow, combine, sched1, local-alloc, global-alloc,
   reload, sched2, jump2, reorg (delay slots).
3. Change the source where this pass makes its decision. Prefer a form a person would plausibly write.

Most differences come from register allocation (wrong register), CSE (wrong addressing), loop hoisting (an insn
inside or outside a loop), cross-jumping (shared or separate tails) or reorg (a different delay slot).

Two warnings:

- Renaming a variable, field or function never changes the output. Only types, offsets and the shape of the
  code do.
- `cf` hides relocations, constant values and branch targets. A score of 64 (the match baseline) can still
  hide a wrong field, a wrong constant or a wrong jump. Only `bld` proves a match.

## Tree folding (`fold-const.c`, `c-typeck.c`)

The front end rewrites expressions before any RTL exists. Several rewrites decide the final code.

- **Adjacent field tests merge.** `a.x == 0 && a.y == 0` on two adjacent small fields becomes one word load and
  a masked compare (`fold_truthop`). So do `(f & 2) && !(f & 0x80)` and `pad[4].pressed[10] ||
  pad[4].pressed[11]`. Write the two plain tests; a cast to a 32-bit word is not needed. Examples:
  NMEProcessMike, ProcessInputPersonal.
- **Conditionals become MIN and MAX.** `a < b ? a : b` folds into a MIN_EXPR, which loads `b` first and does
  not branch the way an if does. The macros in `include/macros.h` have this form.
  - When the ROM tests, branches and then recomputes the chosen operand, write the comparison the other way
    round: `b > a ? a : b`. Example: UpdateFade.
  - An integer literal as the other operand stops the fold. Example: MainGameLoop `-fovspeed * 3`.
  - A clamp is `MIN(MAX(lower, x), upper)`, with the lower bound first. This gives the ROM's test order.
  - `MAX(x - 1, 0)` on an int compiles to the mask `x & ~(x >> 31)`.
  - The macros evaluate an operand twice. A call or a memory read in an operand is repeated in the code.
    Example: ApplyAction `MIN(MAX(mass, 10.0f), 15.0)`.
  - `x *= MIN(a, b)` distributes the multiply into both arms, with `x` read once. `a + (c ? x : y)` becomes an
    add in each arm. Examples: UpdateActorPositionDependencies, UpdateLightDirections.
- **Sums are reassociated.** Fold turns `(a + c) - b` into `a - (b - c)`, and combine can rewrite it back. If the
  ROM computes `frameCount - 130` first, write `ckstart + 130 - frameCount`, not `ckstart - (frameCount -
  130)`. A constant can also move to the other operand: write `(frameCount * 3 - 990 + Random(56)) / 2` where
  the ROM subtracts 990 before the call result is added. Examples: NMEProcessEvilGlove, DoIntroEffects.
  `foldvar.py` tries these spellings.
- **Constants fold into products.** `row * 2 * spacing` folds the 2 into `spacing`; `row * (2 * spacing)`
  keeps the ROM's multiply (LevelSelectScreen, TimeTrialScreen). Two statements `x &= ~A; x &= ~B;` keep two `and` instructions; `x &= ~A & ~B`
  folds them.
- **Division moves into conditional arms.** `ABS(dot) / speed` puts the divide into both arms of the ABS
  conditional, and CSE then forgets earlier loads at the join. Write `a = ABS(dot); a /= speed;`. Example:
  UpdateActorPositionDependencies.
- **Pointer arithmetic is not array indexing** (`pointer_int_sum`).
  - `(p + n)->x` adds the scaled index first (`n * 12 + p`); `p[n].x` adds the base first. A vector macro given
    `&p[n]` expands to the first form. Use the form whose operand order the ROM shows. Examples:
    UpdateRopeBridges, CalculatePlaneInfo.
  - A constant in pointer arithmetic folds into the symbol: `SUB_VECTORS(&tongueOffsetPos[i + 1], ...)` gives
    `%lo(tongueOffsetPos + 12)` with no `addiu`; `tongueOffsetPos[i + 1].x` does not. Example: MaintainAction.
  - Indexing an array member gives `base + idx * 4` where pointer arithmetic on an `f32 *` cannot. puzzle.c's
    `Vec3Arr` (a struct with `f32 v[3]`) stands in for such a vector type. Example: PuzzleEvaluateCondition.
  - A face is `FaceIndices`, a struct with an `s16 v[3]` member, and `mesh->faces[i].v[j]` is the plain form.
    It is expanded through the struct and its array member, which puts the multiply first and emits the row sum
    before `j * 2`, as in the ROM. `mesh->faces[i][j]` on an `s16 (*)[3]` gives the same sum but swaps two
    registers. Examples: InitPlanes, DeformTextureCoords, CalculatePlatformInfo, RecalculatePlatformInfo.
  - A pointer local `item = &eolItems[i]` in a loop over a global array becomes the induction pointer the ROM
    uses. `eolItems[i].field` recomputes the address from the symbol at every access, and `eolItems[6].delay`
    forces the offset into a register. Example: EndOfLevelScreen.
- **Narrow comparisons.** Two `u8` operands are compared as unsigned bytes (`shorten_compare`). A MAX_EXPR with
  an int operand is not narrowed. `alpha = MAX(target, alpha - step)` is computed in int. Example:
  ProcessShadows.
- **Store-flag returns.** `if (c) return TRUE; return FALSE;` becomes a store-flag, as does `return a < b` on
  floats. Where the ROM branches, see "Cross-jumping" below.
- **Chained assignments store the rightmost target first.** `a = b = c = v` stores `c`, then `b`, then `a`.
  The ROM's store order shows the order of the chain. A chained float zero (`v.x = v.z = 0.0f`) uses one zero
  float register and `swc1`; two statements store `sw zero`. Examples: ReadController, EndOfWorldScreen,
  `ZERO_VECTOR` in include/macros.h.
- **Negated products.** `-(x * 0.2)` gives `mul.d` then `neg.d`; `x * -0.2` does not. Example: HiscoresRun.

## Types, prototypes and conversions

- **Prototypes must be in scope.** Without one, GCC passes float arguments as doubles and assumes an `int`
  return. A wrong `void` return makes the caller think `$v0` is unchanged. When a function passes a float pair in
  `$a2/$a3` or compares a call result oddly, check the prototype. Examples: CollideActorInsideSphere,
  HandCatchAllowed, DemoKeyRun. GCC 2.7 reports a call with no prototype only with `-Wimplicit`.
- **A prototype can change the caller's code.** With a prototype, a narrow parameter is truncated at the call
  (`SetActorTerrain` takes a `u8`, and an `s8` prototype changed one byte), and a call with more arguments than
  the definition takes no longer compiles. Where the ROM's callers pass an argument that the function never uses,
  the definition takes it as a trailing parameter: BulletAcquireTarget's `pos`, TimeTrialScr's `(act, me)`.
- **Duplicated tails merge.** Two arms that end in the same code become one copy by cross-jumping. Write the
  code in each arm; no `goto` is needed (NMEProcessFroggy's grow code).
- **A constant set on every path is scheduled early.** Setting `r = 255` in both arms of a test gives the load
  before the branch, as one assignment above the test does (MaintainBulletBomb).
- **Jump threading ignores user variables.** `thread_jumps` (jump.c) does not thread a test whose register is a
  named local. A local holding `&ball` blocks the threading that the ROM lacks (water.c, `b = &ball`).
- **Alias pointers by role.** Locals named for what they point at (`br` for the brain, `ins` for the instruction,
  `actor` for the actor) give the original allocation where copies of the enemy pointer were used before
  (NMEProcessLionFish).
- **Parameter widths come from the callers.** A prototype can declare a narrower type than the definition and
  still match, because only the caller's code depends on it. Examples: FlashScreen has `s16` colours in
  bullet.c and `u8` in overlay.c. `PlaySample` takes `s16` volume and pitch. Its callers need no casts.
- **Field and local types come from the loads.** `lb` means `s8`, `lbu` means `u8`, `lh` means `s16`, `lhu`
  means `u16`. `sltiu` compares and `multu` divides mean unsigned counters. A switch with cases 0 to 2 that
  lacks the `x > 1` test has an unsigned index. frontend2.c's copy of Screen declares `currentSpot` as `s32`, and
  WaitOKScr's switch has the test. A local the
  ROM loads with `lh` and never sign-extends again is an `s32` holding an `s16` field. Examples:
  CheckForInnerEdges, WaitOKScr, MaintainAction (`seg`).
- **Arguments come from the callee's use.** A register move before a call is not always an argument: PrintStats
  takes no parameter, although the ROM moves a value into `$a0` before calling it. DoneOKScr passes its own `act`
  and `me` on to WaitOKScr; they are still in `$a0` and `$a1`.
- **Conversions follow the casts.** `(f32)rv` converts to float and then to double, where `rv` alone converts
  straight to double. A double kept in a local (`d = 0.05`) is converted at run time instead of folded. A `u32`
  converted to float uses the unsigned sequence. Examples: AdjustColours, RestartPlayer, EndOfWorldScreen.
- **Literal precision.** `0.5` makes double maths and `0.5f` single. Write constants the way the ROM's
  precision shows. `PI` is `3.141592654`. `PI / 2` is that decimal over 2, not the exact double. Folded
  products only match in the ROM's form, for example `0.05 * 0.05` in GetHandPosAim.
- **Builtins.** `abs()` on an int is a builtin (`negu`); the `ABS` macro compares with `0.0f` and gives float
  code. `sqrtf` compiles to `sqrt.s` with a NaN check and a fallback call, and a `sqrtf` whose result is unused
  is still emitted. `sinf` and `cosf` are builtins (weak aliases of `__sinf` and `__cosf` in `libc.h`): GCC
  pre-evaluates calls on the right of an assignment to a complex left side, but skips builtins. With
  `sinf`, the target address is computed before the call, as in the ROM. Examples: HiscoresRun, MainOptionsWait,
  RotateVector2DXYZ.

## Memory, aliasing and addressing

GCC 2.7.2's scheduler and loop pass assume that a fixed scalar global and memory reached through a pointer or
inside an aggregate do not overlap. CSE turns addresses into base-plus-offset forms. Both decide whether the
ROM loads a global by absolute address (`lui` plus offset) or through a base register.

- **The ROM reloads a global after a store, but ours keeps it.** The global was probably an array element or a
  struct member in the original, not a scalar. GCC treats aggregate memory as possibly aliased and does not move
  loads across the stores. Examples: `fgSpin` (ActivateCollectable, DrawScreen), `backgroundLightList`
  (DoIntroEffects), `D_801F90D0[6].delay` (EndOfLevelScreen), `stats[2].scale` (NMEProcessHubChicken).
- **The ROM addresses neighbours absolutely, but ours uses one base register.** The neighbours were probably
  separate globals, not fields of one struct. Examples: the four `u8` menu globals in UpdatePause, the two stun
  counters in DoFortressBoss. The opposite also holds: replacing separate globals by a table can make GCC keep
  the table's address in a register (KillTitle `fadeNiceOL`). WaitOKScr reads `startMenuStrings[10]` and `[14]` as
  table rows; the frame is then 8 bytes larger and needs no pad. Two fields that the ROM reads as one
  32-bit word stay one symbol (ScreenFadedIn reads `gameInfo.fade` and `fade2` together through
  `D_801E753C`).
- **".x through a register, .y and .z absolute."** CSE1 puts each constant address in a pseudo and rewrites
  related ones as offsets from the first. CSE2 (`find_best_addr`) folds `(plus reg const)` back to an absolute
  address when it knows the register's value, but never folds a plain `(reg)`. So the field at offset 0 keeps
  the register and the others become absolute. `mips_address_cost` rates a register 4, a symbol 2 and a
  register plus small offset 1.
  - This pattern usually comes from a vector macro given the address of a global field, such as
    `SUB_VECTORS(&tempVect, &hand.pos, &garib->sprite.pos)` (see the next entry). Examples: UpdateGaribs,
    UpdatePickups, HandSnatchAllowed, MoveCam.
  - Where the ROM addresses all fields absolutely and ours uses a register, index the global with a constant
    (`enemies[0].actor`) or find the pointer local the original used (`en = enemies;` in DoFortressBoss). The PC
    port's local lists often name the pointer.
- **Taking an address is pointer arithmetic.** GCC 2.7's front end turns `&s.field` into `&s + offset` and
  `&a[i]` into `a + i`. So `(&hand.pos)->x` is not folded back to `hand.pos.x`, and `(&a[i])->x` is the same as
  `(a + i)->x`. The vector macros take pointers, so a macro use compiles differently from the same three
  statements written with `.x`, `.y` and `.z`. Forms like `(globalRot + i)->x`, `*(edgeLen + i)` and pointer
  locals such as `hp = &hand.pos; hp->x` turned out to be macro expansions: `ADD_TO_VECTOR(&p, &globalRot[i])`,
  `DOT_PRODUCT(&hand.pos, plane)`, `COPY_VECTOR(&hand.pos, &spheres[i].pos)`. Where the ROM needs such a form,
  try a macro from `include/macros.h` first. Elsewhere the macro and the written-out statements usually give
  the same code. Examples: UpdateActorCollisionAndGravity, CalculatePlaneInfo, ConditionsMet, UpdateRopeBridges,
  UpdateActorPositionDependencies (`SCALE_VECTOR_2D(&ball.vel, ...)`).
- **CSE records a branch's equality.** After `if (actor == &hand)`, CSE rewrites `actor->radius` as a load from
  `hand`. The ROM loads from `actor`. A flag set just before the test hides the equality:
  `isHand = actor == &hand; if (isHand && ...)`. Example: ProcessShadows. A test on a copy of the pointer hides
  it too: PuzzleExecuteAction copies `sound` to `tmp` and tests `if (tmp != NULL)`. A test on `sound` itself lets
  CSE delete the loop's own end test `while (sound != NULL)`. The same rewrite explains why UpdateActorCollisionAndGravity reads `ball.obj` in
  the test before its loop and `actor->obj` inside it.
- **`cf` does not catch a shorter function.** A variant that drops instructions can score 64, the matched
  baseline. Compare the whole object (`objdump -d -r`) or run `bld` before trusting such a score.
- **A pointer to a member stops CSE from rewriting later accesses.** An inline helper given `&ball` directly
  folds every access to an absolute address; pass a variable instead (`actor = &ball;
  PutActorOnPlatform(actor);`). Example: ProcessInputPersonal.
- **An unsized `extern s32 x[];`** stops loads and stores moving past each other and gives base-register
  addressing. Example: PrintSpritesOpaque.
- **A load is not moved above a store through an int pointer.** If the ROM loads a global first, read it into a
  local before the store. Example: SlotSelRun (`oldslot`).
- **Where an array sits decides the address arithmetic.** For `a[i].b.c[j]` GCC puts the outer field offset in
  the base register and the inner offset in the loop's induction variable; for `a[i].c[j]` the whole offset goes
  in the base. This showed that the enemy sample array is a field of `Enemy`, not of `NMEBrain`. Example:
  UpdateEnemies.
- **A struct is bigger than its definition** when the ROM reaches fields past its end through a pointer to it.
  `GameSprite` ends with a `SpriteAnimState`. Example: AnimateSprite.
- **Struct copies.** `*out = *in` on a Vec3 or Quat copies with `lw`/`sw`; field-by-field copies use float
  loads. Example: QuatSlerpAtFixedSpeed.

## Loops (`loop.c`)

- **Hoisting has a size limit.** An invariant is hoisted only when `threshold * savings * lifetime >=
  insn_count`. The threshold starts at about 61 and drops by 3 after each hoisted insn, and insns already moved
  out of an inner loop count double. One extra insn in the loop at loop time can stop a hoist. Examples:
  ProcessShadows (`(s16)` casts in FADE_SHADOW add sign extensions that combine removes later, which stops the
  hoist of 360000.0f), MainTitleWait (the `i * 45 + 53` rows use up the threshold).
- **Hoist order shows source order.** Invariants are hoisted in the order of their first use in the loop body.
  Example: KillEnemy.
- **No global constant propagation.** A constant assigned before a loop or at the top of a function stays in a
  register; GCC does not carry it across labels. Write `s32 bounce = TRUE;` and test `bounce` where the ROM
  tests a register that always holds 1. A pseudo set once from a constant has its live length doubled, which
  halves its allocation priority. Examples: CheckActorCollisions, PrintTimers, MoveHand.
- **Induction variables.** Strength reduction turns `i * 35 + 53` into a register that steps by 35. A variable
  set after `k++` in the for-step (`j = k * 2`) is not derived. Each use recomputes it, as in DrawBlurFX.
  `y = 80 + i * 20;` on an `s16` keeps `y << 16` stepping by `20 << 16` (HiscoresRun).
- **Counted loops run backwards.** GCC reverses a simple counted loop. A ROM loop from 3 down to 0 is
  `for (i = 0; i < 4; i++)`. Example: ActivateCheat.
- **Blocks that leave a loop.** `find_and_verify_loops` moves a block that ends in a jump out of the loop only
  when the insn before it skips just that block. An `||` puts a label there and keeps the block inside. Two
  separate ifs let both blocks move out. Examples: MoveHand, MaintainAction.
- **`do { } while (0)` is a loop.** It emits loop notes. Inside it, every register reference counts double
  (flow counts references per loop level), and the scheduler does not move insns across a loop note. A wrap
  can therefore change both register choice and instruction order. acosBetter's two nested wraps give the ROM's
  code in all three of its copies (it is inlined into QuatSlerp and GetRotationFromQuaternion). A
  multi-statement macro written as `do { ... } while (0)` is ordinary C (PrintStats `BUTTON_COLOUR`). A loop
  around a single statement is not plausible source on its own.

## Register allocation (`local-alloc.c`, `global.c`)

- **Priority.** Each pseudo is ranked by `floor_log2(refs) * refs * size / live_length`, with references inside
  loops weighted by depth. The highest goes first and takes the lowest free register. References that later
  passes delete still count. Two saved registers swapped usually means a near tie: add or remove one natural
  reference. Examples: NMEProcessInstruction (`needNewInstruction = TRUE` in both branches), NMEActionFacePlayer,
  MatrixToQuaternion.
- **Local allocation ties a result to an operand.** A result takes the register of the first operand that dies,
  if that operand is set once and used only in its block. A variable assigned two or more times goes to global
  allocation instead. Where the ROM keeps a result in a fresh register, build it in place on a variable
  assigned several times: `xh = tex->width - 1; xh *= 4096.0f; xh /= sx; xh += x;`. Examples: TileRectangle,
  ScreenShot (one `u8 byte` for three colour bytes), PrintOverlay, AdjustColours, StartEarthquakeAtPoint.
- **Block-local values are allocated first.** Constants and addresses used in one block get registers from
  local allocation before named variables that span blocks. A variable that lives in one block can lose the
  register it has in the ROM; making it live across blocks (a small inline function, a second assignment)
  changes that. Example: UpdateRestartGateShadows (`GateShadowScale`).
- **One C variable, one register.** Values that share a callee-saved register in the ROM are usually one C
  variable reused. Values in different registers at different sites were different variables. Examples:
  MaintainAction (all counted loops share `i`, one `dist` for four distances), UpdateWinds, QuatSlerp
  (`pdist`, `ndist` and four difference floats reused for both sums).
- **Preferences.** A temporary copied into a variable takes on that variable's register preference. In
  MaintainAction the `sqrtf` result gets `$f2` because `dist` prefers `$f2` (`expand_preferences`).
- **Names and declaration order.** Two names for one address can give two registers, with each access using
  the name it used in the original. Examples: DoCarnivalBoss (`e` and `tempObj`), NMEProcessCylon (`br` before
  `actor`), NMEConditionsMet (`actor = &enemy->actor`). Declaration order can decide saved registers.
- **Address-taken locals live on the stack.** A counter passed by address cannot use a register; a separate
  counter for the loop can.
- **Parameters.** A float parameter reused as a temporary keeps its register; new locals do not. Some functions
  copy a parameter and reuse it (`Object *obj = object; object = JallocAlloc(...); *object = *obj;` in
  MakeUniqueObject).

## Scheduling (`sched.c`)

- **sched1 reorders a basic block before local allocation.** Statement order inside a block of float code rarely
  matters. Order across calls and stores does. Examples: MoveCam (`atan2` before reading `camera.rot.y`),
  ProcessSFXScript (`id = e->id;` before the pitch).
- **Constant pool order shows source order.** Float constants are emitted in order of first use. Check the
  ROM's pool before keeping an odd statement order. Example: UpdateTip.
- **Priority boosts.** An insn whose operand is set once gets a launch boost. A variable set several times
  removes it. Example: UpdateActorTeleport (`r = Random(31) - 15;` then `+= r` for each axis).
- **Barriers.** sched1 never moves insns across a block boundary (a label, a branch or a loop note).
- **sched2 ties.** When all insns in a block have equal priority, RTL order decides. Example: MaintainAction
  (`ADD_VECTORS(&tongueBullet->actor.pos, &tongueOffsetPos[seg], &hand.pos)` puts the `la` before the index
  shifts).

## Cross-jumping, branches and delay slots (`jump.c`, `reorg.c`)

- **When tails merge.** jump2 merges two identical tails when at least two insns before their jumps match, or
  one when the insn before the block is a label or a conditional jump that skips just that block.
- **Shared returns.** Where the ROM repeats `j` to the epilogue with `move v0, zero` or `li v0, 1` in the slot,
  write each case as `if (cond) { return TRUE; } break;` with `return FALSE` after the switch, and write tests
  that lead to the same return as one `||` condition. Examples: ConditionsMet, NMEConditionsMet,
  PuzzleEvaluateCondition.
- **Write the code in both arms.** Where the ROM shares one tail, the source often had the same statement in
  each branch and jump2 merged them. Examples: PrintTimers, NMEProcessCannonball, TransformAndDrawObject,
  ActivateSpell.
- **A call in each arm.** A call whose argument is `c ? a : b` is two calls in an if/else in the ROM; each
  branch keeps its own argument set-up. Examples: CalculateShadows, MaintainAction.
- **break and continue.** A `break` and a `continue` leave by different jumps. Where the ROM leaves the loop,
  the source says `break` (MaintainAction). A `continue` that the ROM sends to the epilogue was `return result;`
  (CollideActorWithPlane). `cf` masks jump targets; use `jcheck.py` and `brcheck.py`.
- **Switches.** Case bodies appear in the ROM in source order. An empty `case N:` reproduces a comparison tree
  (HurtHand, DeactivateSpell, DoBoss); case ranges (`case 0 ... 5:`) give a tree where a jump table would
  otherwise be used. A missing `break` after the last case of an inner switch can be needed
  (PuzzleEvaluateCondition case 34), and the jump table's range check goes to `default`.
- **Range tests.** `case 5: case 6:` with a shared body, or `x == 5 || x == 6`, becomes one unsigned range test;
  separate bodies keep two tests. Constants that are not adjacent give an `or` of two set-on-equal results.
  Example: NMEAnalyseConditionalList.
- **Comparisons as values.** `xori`/`beqz` only appears when the comparison is first computed into a variable
  (`startRoll = prevAnim != 3;`). Example: MaintainAction.
- **Branch prediction.** With no hint, reorg predicts `==` tests not taken and `!=` tests taken, which decides
  whether the delay slot comes from the target or the fall-through.
- **Branch-likely.** A delay slot is annulled (giving `beql`) only if the moved register is live at the start of
  the fall-through. In CalculateStaticMatrices the first reorg pass scans forward from the `ptr->inst` test. If
  it reaches an unconditional `j`, it treats `$a0` as dead and the slot is not annulled (`beqz`). The form
  `if (type == LANDSCAPE_ACTOR) { free; alloc; } else { staticMtx = NULL; continue; }`, with the matrix code
  after the if/else, lets the static case fall straight into the matrix code with no `j`, and gives `beqzl`.
  Putting the matrix code inside the then-part, or an early `if (type != ...) { ...; continue; }`, adds the `j`.
- **Dead-looking code keeps a branch.** FindCheatButton's last `else if (pressed[13]) return 0;` keeps the
  previous test a branch instead of a store-flag.

## Inlining and macros

- **`inline` without `static`** emits a normal copy and inlines calls that come after the definition in the
  same file. Use it where the ROM inlines a function that also exists on its own. Examples: AddBullet,
  AddCollectable, StartEarthquake, acosBetter.
- **`static inline`** emits no copy unless one is needed. Use a separate `static inline` copy when the ROM calls
  a function in some callers and inlines it in others (FreeBackdropListInline in game.c,
  QueryAnimTimeInline).
- **A later caller that the ROM does not inline** marks a file boundary in the original. nmeprocess.c and
  nmeprocess2.c are split at such a boundary.
- **Inline functions and macros differ.** An inline function loads its argument once at the call; a macro
  re-reads it at each use (FADE_SHADOW in ProcessShadows). An inline call to a same-file function moves
  conversions to the call site (StartEarthquakeAtPoint). Each macro use emits its own rodata constants.
- **Repeated arithmetic** usually comes from a macro in the original. The vector macros in `include/macros.h`
  expand to `(v)->x` forms with pointer operands, which many functions need (see "Taking an address is pointer
  arithmetic" above).

## Stack frame

- **A bigger frame in the ROM means a missing local.** First look for an original local that fills it: a
  `CollFace plane` fills CreateWake's 120 bytes, `f32 inFloat[5]` fills ParseTokenPlatformPath's. Use
  `s32 pad[N]` with a comment only when no real local fits.
- **Slot order.** Address-taken arrays and structs get stack slots in declaration order; address-taken scalars
  get theirs at first use. An unused Vec3 declared last reserves 16 bytes; one declared first does not.
  Examples: ParseTokenBridge, CheckCameraForWater.
- **Adjacent stack vectors** at 12-byte steps are one array (`Vec3 v[6]` in NMEHandleDeathSpell).
- **Inlined functions** can add their locals' slots to the caller's frame.

## Constants and rodata

- **Strings.** Identical string literals are shared within a file, and the first function that uses one emits
  it. The ROM keeps one copy per original file. Rodata order shows which function used a string first. Write
  a shared string as a literal in each user. An `INCLUDE_RODATA` line that holds data before the string must
  come before the string's first user in the file (enemy.c `"UniqAct"`).
- **Float literals** are emitted where the function is compiled, in order of first use.
- **Linker padding** after a file's last constant is written as a `const u32` array at the end of the file. See
  [building.md](building.md#rodata-and-linker-padding).

## -O0 files

Nine files are compiled with `-O0` (listed in [building.md](building.md#compiler-and-flags)). At `-O0` every
local lives on the stack in declaration order and every literal gets its own rodata entry. Small differences in
the C show up directly:

- `x = *p; p++;` differs from `x = *p++;`.
- `if (a) { } else { ... }` gives "beqz; j" where `if (!a)` gives one branch.
- A trailing `return;` in a void function gives an extra jump to the epilogue.
- Narrow parameters (`s16`, `u8`) are copied to a stack slot. Do not add `register`.

`cf` and `cmpfunc.py` compile at `-O2` unless told otherwise. Score an `-O0` function with
`CFOPT=--opt=-O0 cf FILE FN`. `tryv.py` compiles at `-O2` only.

## Where these forms came from

Most entries were found while matching. Dr. Mario 64, built with the same
compiler family, marks similar constructs `//! FAKE` in its source: chained assignments that make a
temporary, a temporary inside a call argument, ordered pointer arithmetic, a `do { } while (0)` and redundant
casts. So these forms have precedent in real code from that compiler.
