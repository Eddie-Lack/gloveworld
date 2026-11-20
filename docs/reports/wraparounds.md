# Counters that wrap: underflows and overflows

This report describes the code and the US ROM (NGVE) as of November 2026. It lists the counters in Glover that can
pass their lowest or highest value, what each wrap does, and whether a player can reach it.

The results come from two sources:

- **The code.** References such as `LoseLife` (src/hand.c:3537-3558) are functions, files and lines in
  gloveworld's `src/` and `include/`.
- **Runs.** The runs used copies of [weemitt](https://github.com/Eddie-Lack/weemitt) (main at b013617). weemitt is
  a native PC port built from gloveworld's C. The copies ran headless (no picture), as 64-bit builds, at the
  original timing. Many runs are "instrumented": the copy prints values, or sets one value by hand in place of a
  hard step. A step that is "forced" is one that the copy made happen by hand, such as a call or a touch. Each
  finding says which steps were real inputs.

No run was checked on an emulator or a console. Separately, the Fear boss trick is known from play on the N64.

## Terms

A **wrap** is when a counter passes its lowest or highest value and jumps to the other end. For example, an 8-bit
unsigned counter (u8, range 0 to 255) goes from 0 to 255. A signed 8-bit counter (s8, range -128 to 127) goes from
-128 to 127. s16 is 16 bits (-32768 to 32767) and s32 is 32 bits.

The game logic runs at 20 frames a second in levels. So 32768 frames is about 27 minutes.

Each finding has a grade:

- **Definitive:** shown with real inputs, or players already do it on the N64, or every step is plain code with no
  condition left open.
- **Possible:** the code allows it and a route is plausible, but nobody has shown it with real inputs.
- **Theoretical:** the code allows it, but it needs months or years of play, or a state no known route gives.
- **Not possible:** no route exists.

## Summary

| Goal | Result | Grade |
| --- | --- | --- |
| Boss health to 0 by wrapping | No boss. Every boss hit counter is guarded, set exactly, or lowered only from boss code that cannot run twice. The robot arm wrap gives 255 but never a second boss hit. | Not possible |
| Kloset: skip the boxing glove with two bombs | The code would allow 3 to 2 to 1 if Kloset stood within 100 units of a box spot. A second box trigger moves the first bomb there. Kloset's largest radius was 210 (it needs about 227). 2005 forced tests never reached hits 1 before the glove. | Not possible in practice |
| Whale (Atlantis boss) skipping its phase or going below 0 | The order is forced: three whale hits, then crab, then fish. No kill source exists early. | Not possible |
| 255 lives | Pause on the GAME OVER frame, Return to Castle, Continue. | Definitive |
| 255 lives with no timing | Game over in a hub room entered by `HUB_EXIT`. Needs a hub death. | Possible |
| Losing the 255 | One more life before a save gives 3 after the next load. | Definitive |
| Huge extra lives in one session | The Carnival 1 bubble killed every frame on a spike out of camera range. | Possible, no route found |
| Sequence break to the ending | Hub game overs after the first boss add crystal balls. | Possible |
| Rearranging the Fear staircase | A slam during the 3-to-4 descent. It may also leave the end-of-fight reset wrong. | Definitive (the reset part from the code only) |
| Extra height | The frog counter carried into a ball jump: about 209 units at count 30, up to about 480. | Possible |
| Dead puzzles (cameras, vents) | After 55 minutes of the condition holding. No useful case found. | Definitive |
| A destroyed robot arm firing again | Shoot its debris after turning the robot. | Definitive |
| Riding the ball off ledges | After 27 minutes in ball-walk with no input. A handicap. | Definitive |

## 1. The Fear boss platforms

**Grade: definitive.** A player already does this on the N64.

It is not an integer underflow. It is a wrap of a path index.

- The eight floor platforms of the Fear boss (level 33, `FEARBOSS`; tags 1 to 8) each have a path of 5 points.
  Point 0 is the top (y -290). Point 4 is the bottom (y -470). Points 1 to 3 are at y -335, -380 and -425.
- Each path has a `target` (the point it is going to) and a `current` (the last point it reached)
  (`PlatformPath`, include/platform.h:61-70). `current` only changes when the platform arrives
  (`MovePlatform`, src/platform.c:393: `current = target`).
- `MovePlatform` steps the target with `target = (target + numPoints + dir) % numPoints`
  (src/platform.c:366-369). This is modulo 5. So 4 + 1 becomes 0, and 0 - 1 becomes 4.
- `DoFortressBoss` accepts a fist slam (src/boss.c:740-768) when the hand lands in `HAND_SLAM2ST` with
  `actionTime == 1`, was in the air, the platform tag is 1 to 8, and `plat->path->current < 4`. The slam gives the
  platform `dir = +1` (one level down) and its partner (tag + 4 or - 4) `dir = -1` (one level up).
- The bug: the guard tests `current`, not `target`. It also does not check that the platform is at rest. While a
  platform travels from point 3 to point 4, `current` is still 3. A slam then passes the guard. The new target is
  (4 + 5 + 1) % 5 = 0, the top.

The partner wraps too, in the same frame. The pairs are (1, 5), (2, 6), (3, 7) and (4, 8). They start at points
(0, 4), (1, 3), (2, 2) and (3, 1), so the targets of a pair always add up to 4. When the slammed platform's target
is 4, its partner's target is 0. The partner gets (0 + 5 - 1) % 5 = 4 and goes from the top to the bottom.

The window is the whole 3-to-4 descent, about 26 frames. Two quick slams from rest at point 3 do it. A slam during
the 0-to-1 or 2-to-3 descent does not wrap: the platform just skips a stop.

**Effect.** The slammed platform rises about 180 units in one move, and its partner drops 180. More slams before it
arrives keep stepping the target, so it can be sent to any point. No value leaves 0 to 4. Boss health, the stun
lightning (drawn at a fixed y -365) and every counter are untouched.

One effect follows from the code but was not run. At the end of the fight, `DoFortressBoss` resets the staircase
through `SetPlatformPoint` (src/boss.c:1009-1018). `SetPlatPoint` (src/boss.c:1050-1061) compares `current`, not
`target`. A platform in flight whose stale `current` already equals its reset point is skipped. It keeps going to
its wrapped target, so the staircase can be left wrong.

**Only Fear has this.** Every other writer of `moving = 1` checks that the platform is waiting: the Carnival boss
(`moving == -101`), ball contact (`== -3`), hand slams in `CheckHandPlatform` (`-1` or `-2`), and the puzzle action
`ACTION_START_MOVE` (`!= 0`). `ACTION_SET_NEXT_POINT` and `SetPlatPoint` set the target outright.

**Evidence.** The code above. Instrumented runs in level 33 made the slam branch's writes on chosen frames. Trace:
a slam at frame 300, and a second slam at frame 315 with tag 4 at `current` 3, `target` 4, y -451. At frame 316
tag 4 has `target` 0 and tag 8 has `target` 4. Tag 4 rests at point 0 (y -290) and tag 8 at point 4 (y -470) by
frames 406 to 410.

## 2. The lives counter

### How the 0 to 255 trick works (definitive)

- `player.lives` is an s32 in RAM (include/hand.h:90). The save file's lives field is a u8 (include/save.h:12).
- `LoseLife` (src/hand.c:3537-3558) does `lives--`. A game over starts only when lives is exactly -1. It sets
  `gameOverCount = 1` and pauses the game.
- `ProcessInput` runs before `MoveHand` in the same frame (`MainGameLoop`, src/main.c:1058-1066). The pause check
  (`ProcessInput`, src/controller.c:373) needs `gameOverCount == 0`, which still holds before `LoseLife`. So Start
  pressed on the death frame opens the pause menu in the same frame that lives become -1. Pausing one frame
  earlier freezes the death timer. One frame later is refused. The window is exactly one frame, and it can be
  retried.
- Return to Castle is offered when `!cameFromTitle` and the level is 8 or higher (`UpdatePause`,
  src/game.c:2899). It calls `ExitLevel(HUB_EXIT)` and sets health to 3 (src/game.c:3112-3116).
- When the fade ends, `CheckForExits` runs its `HUB_EXIT` case. That calls `CheckForLevelCompleted`, then
  `UpdateSaveSlot`, which does `saveSlot.lives = player.lives` (src/eeprom.c:222). -1 becomes 255 and is written to
  the EEPROM.
- In the same frame, `GameOverScreen` sends the game to the title. **Play never goes on with -1 lives.**
- Continue loads `lives = saved < 3 ? 3 : saved` (`ContinueGame`, src/game.c:1306), so 255 comes back as 255. The
  HUD shows it.

Timing of the A presses: `GameOverScreen` keeps running under the pause. At `gameOverCount` 20 it starts taking A
itself (src/game.c:2655-2659). Once it has set its exit flag, the pause menu's `ExitLevel` is blocked. One run
showed both A presses must land before `gameOverCount` 20, about 6 frames after the menu accepts input. Another
run confirmed the final A as late as frame 260 (about 30 frames after GAME OVER appeared). The safe rule: open the
Yes/No question fast.

Evidence: two runs. Only the last death was set up by hand (lives 0, then `KillHand`). The pause and the menu were
real inputs. Trace: "frame 309 mode 1 level 0 lives -1 slotlives 255". A normal game over (the control run) keeps
the saved 5.

### Lives above 255 lose everything on save (definitive, a trap)

`GainExtraLife` (src/collect.c:818-822) does `lives++` with no cap. RAM can go past 255, and the HUD prints 3
digits. But the save keeps only the low byte. 256 is saved as 0, 257 as 1 and 258 as 2, and all of these load as
3. 259 or more loads as lives minus 256. So after loading 255 lives, one more life followed by a level exit or
Return to Castle throws the stack away at the next load. **Keep lives at 255 or below whenever the game saves.**
Not run in weemitt. Every step is a plain assignment.

### A timing-free route to 255 (possible)

`GameOverScreen` sets the exit flag but never writes `gameInfo.exitType`. A level load clears the exit flags but
not the exit type. So a normal game over runs the `CheckForExits` case of **the exit that brought you into the
current room** (`CheckForExits`, src/game.c:2149; `GameOverScreen`, src/game.c:2655-2667; `RunGame`,
src/main.c:665-668).

The castle hub is entered with `HUB_EXIT` from the cave, from the Wayroom's castle door, and by Return to Castle.
A game over in a hub room entered that way saves 255 lives with no pause timing at all.

- Instrumented run (level 0, exit type 13 (`HUB_EXIT`) set by hand, lives 0, `KillHand`): the slot ends with 255.
  With exit type 0 it stays 5.
- What is missing: a real death in the hub. The hubs have no spikes, no harmful enemies and no water in HUB1 to
  HUB6. Falls are the only source: 1 damage needs a drop of about 214 units, death about 334. A normal jump does
  not reach that from any surface checked. Lifting Glover 400 units in HUB1 did give a fall death. The
  [practice ROM project](https://github.com/Eddie-Lack/glover-practice-rom) lists 70 holes in the hub's collision
  mesh in its `docs/game.md`, so a route may exist.

### Two lives lost in one death (possible)

`LoseLife` can run twice in one frame: once from the ball's `loseTimer` (`MoveBall`) and once from the hand's
`deathTimer` (`MoveHand`). This needs the ball to burst or shatter after Glover has died, on exactly the frame
where both timers end together. `BurstBall` sets 60 frames and `ShatterBall` 30, so the ball must break exactly 20
or 50 frames after the death. The window is one frame.

- With 2 or more lives, one death costs two.
- With 1 life, it is an instant game over.
- With 0 lives it gives -2, but the game still goes to the title. Combined with the pause trick it would save 254,
  which is worse than 255.

The burst from a fall during the death animation cannot cause this, because `MoveHand` returns early while Glover
is dead (src/hand.c:2979-2981). Enemies, spikes, lava and bombs can. Instrumented run (level 10, `ATLANTIS1`; a hit
at frame 150, `BurstBall` forced at frame 170): lives went from 5 to 3 in one frame. Bursts at frames 169 and 171
lose one life.

### What is not possible with lives

- **Unlimited deaths at -1 or below while the HUD shows 0.** The code would allow it (a game over starts only at
  exactly -1), and a test that set lives to -1 by hand did give endless deaths and a save of 254. But no real route
  leaves Glover alive at -1: every path to -1 goes through the game over screen to the title. Continue, New Game,
  Level Select and Time Trial all reset lives.
- Health cannot wrap. It stays 0 to 3 (`HurtHand`, src/hand.c:3456-3500).

## 3. Every other wrap, by grade

### Definitive

**Space boss phase 2 (the evil robot) arm health, 0 to 255.**

- What it does: `portionHealth[3]` is a u8 (src/nmeprocess2.c:97). An arm loses a point whenever one of its
  platforms (tags 1 and 4 for arm 0, 2 and 5 for arm 1) has `path->moving > 0` and its 20-frame immunity is over
  (`NMEProcessEvilRobot`, src/nmeprocess2.c:2655-2716). Nothing checks that the arm is already destroyed. Any
  `BALL` actor touching a `moving == -3` platform sets `moving = 1` (`CollideActorWithPlane`,
  src/collision.c:901-912 and 1109-1120), and the player's space-ball shots are `BALL` actors. A destroyed arm's
  two platforms fly off in a straight line at about 50 units a frame, forever, with `moving` still -3.
- How to reach it: hit arm 1 just before the shot that destroys arm 0. The hit turns the robot (`rot.y` ± 0.35,
  src/nmeprocess2.c:2667-2673), so arm 0's debris flies toward the player's ship. About one second later, shoot the
  piece that flies toward the screen. Without the turn the debris flies sideways and is out of reach (the best
  misses were 470 to 2300 units).
- Effect: the destroyed arm counts as alive. Its smoke stops. It fires missiles again (animation 0, and the two-arm
  animation 10). The boss's hit count does not change, and the fight goes on normally. Reaching 0 a second time
  needs 255 more hits 20 frames apart, on debris that leaves range in about 40 frames. So **boss health cannot be
  skipped this way.**
- Evidence: two input sequences in level 40 (`SPACEBOSS3`), each made of real inputs only. An unmodified weemitt
  build replays both.
  - In the first, arm 0 is destroyed by a real shot at frame 1127 (`hits` 3 to 2), and a shot hits its debris at
    frame 1152, giving `portionHealth[0]` 255. The unmodified build gave the same traces and memory hashes as the
    instrumented one. 10 of 12 follow-up shot timings also wrapped.
  - The second keeps the robot on screen. The destroying shot lands at frame 1210, a shot hits the debris at frame
    1241 giving 255, and the arm fires again at frame 1301.

**Puzzle trigger count: "unlimited" puzzles die after 65535 firings.**

- What it does: `timesLeft` is an s16 (include/puzzle.h:32). -1 means unlimited. Each firing does `timesLeft--`,
  and a puzzle fires only while it is not 0 (`CheckPuzzles`, src/puzzle.c:122-123). Conditions are states, so a
  true condition fires every frame. -1 counts down to -32768, wraps to 32767, and reaches 0 after 65535 firings. The
  puzzle then never fires again until the level reloads.
- How to reach it: keep the condition true for 65535 frames (about 55 minutes). Puzzles with no conditions just
  need you to wait. Examples that fire from the level start: Atlantis 1 #4, Carnival 1 #22, Carnival 3A #8 and
  #11, Pirates 1 #20, Prehistoric boss #1, Space 2A #0 and #9, the training course #5, and several camera puzzles.
- Effect: that puzzle is dead for the visit. A camera zone stops, a vent stays as it was, and a platform is no
  longer kept moving, shown or hidden. None of the ones found guards progress, so no skip or softlock is known.
- Evidence: an instrumented print only, level 10, no input. Puzzle 4 reads -30001 at frame 30000, +30535 at frame
  35000, 535 at frame 65000, and 0 from frame 65535.

**Hand action timer (`handControl.actionTime`, s16).**

- What it does: it counts up every frame in one hand mode (`MoveHand`, src/hand.c:3184) and resets only when the
  action changes. After 32768 frames (27 minutes) it goes negative. Every `actionTime >= N` check is then false for
  another 27 minutes.
- How to reach it: give no input at all (`HAND_IDLE`), or push a wall. Or, in Atlantis 2, Atlantis 3, Pirates 2 or
  Fear 2, tap L once at the start and then do nothing (Glover stays in ball-walk, `HAND_BALLWALK`).
- Effect: in ball-walk, Glover can no longer drop off the ball (`MaintainAction`, src/hand.c:1872). When the ball
  rolls off a ledge he rides it down instead of grabbing the ledge. In Carnival 2 this made him fall into the pit.
  In idle nothing visible changes. This is a handicap, not an exploit. The tongue-mode array index
  (`tongueOffsetPos[handControl.actionTime]`, src/hand.c:2312) could go out of bounds with a negative value, but
  that mode ends on its own long before.
- Evidence: a print-only copy, with three input sequences of real inputs: one with no input, and two with an L tap
  that leaves Glover in ball-walk. "HT 32820 mode 0 act 32767", then negative.

**Hand idle timer (`handControl.idleTime`, s16).**

- What it does: it counts frames on the ground with no stick, A, B, Z, R or L (`MoveHand`, src/hand.c:3168-3174).
  It ignores the C buttons. The resets need normal hand control.
- How to reach it: stand on the ground, press C-Up once (camera look mode) and touch nothing for 32768 frames.
- Effect: cosmetic. Idle animations are delayed by up to 27 minutes after leaving camera mode.
- Evidence: a run with real inputs (one C-Up press, then nothing). "f=32816 t=32767", "f=32817 t=-32768".

**End-of-level time shown through an s16.**

- What it does: the end-of-level screen prints `(s16)levelTime` (`EndOfLevelScreen`, src/game.c:1780). After 32768
  seconds the time shows negative ("-546:-8" at exactly 32768 s, "-425:-36" at 40000 s). It reads "0:00" again at
  65536 s.
- How to reach it: stay in one level, unpaused, for 9 h 6 min, then finish it.
- Effect: display only. The best time cannot change, because the compare is done in floating point against an s16
  best of at most 32767 (`CheckForExits`, src/game.c:2380; `CheckForLevelCompleted`, src/game.c:3257).
- Evidence: an idle run reached 655400 frames and printed "shown=-546:-6". The touch of the goal was forced.

**Lives above 255 on save.** See section 2.

### Possible

**The popped Carnival 1 bubble (a Bugle) killed again every frame: unlimited score and lives.**

- What it does: `UpdateSpikes` (src/spike.c:33-34) kills any Bugle touching a spike, every frame, and never checks
  whether it is already dead. `KillEnemy` has no such check either. Each kill adds 500 to the score. A dead enemy
  leaves the actor list only when its own update runs (`UpdateActor`, src/actor.c:330-333). Enemies more than
  about 922 units from the camera are not updated (`UpdateEnemies`, src/enemy.c:1244-1248). So a bubble that pops
  on a spike while far from the camera is killed again every frame until the camera comes back.
- Effect if reached: the score climbs 500 per spike in range per frame. A life comes every 10000 points (medium
  difficulty), with no cap. In the test, lives went from 5 to 53 in 320 frames. The bubble's own s8 `hits` wraps
  too, with no effect. (The save keeps lives modulo 256, section 2.)
- How to reach it: unknown. Carnival 1 has one Bugle (`enemies[0]`) and 6 spikes. The bubble needs to touch a spike
  (within 51 units) while the camera is over 922 units away. Ideas: outrun the bubble while it carries the ball
  along a path past a spike, or pop it during the 65-frame camera cut to (490, 300, 650). Chase tests got it only
  to 95 to 185 units from a spike. Level 16 (`CARNIVAL2A`) has a Bugle but no spikes.
- Evidence: the bubble was moved by hand onto a cluster of spikes (in place of the pop). "fc=380 lives=53
  score=480000".

**A game over in the hub right after a boss adds unearned crystal balls (`hubTarget`).**

- What it does: this is the same stale exit type as the timing-free 255 route. After a boss, the exit type stays
  `WORLD_EXIT` in the hub. A game over there replays `WORLD_EXIT`, which does `hubTarget++` (`CheckForExits`,
  src/game.c:2214-2217). The `++` is guarded by bit 30 of the save slot's `levels`. That bit is set the first time
  you take a completed exit out of the cave, the Wayroom or the assault course. Return to Castle does not set it.
- How to reach it: on a new file, keep bit 30 clear. Go from the intro to the hub, through the Atlantis door, and
  through the Atlantis levels by the Wayroom's level exits, then to the boss. Then take a game over in the hub.
  Continue keeps the stale exit type, so each further game over adds one more.
- Effect: while `hubTarget` is above `hubLevel`, the cave places one crystal ball per visit. Placing the 7th plays
  the ending. In theory: beat Atlantis only, take 6 game overs in the hub, visit the cave 7 times, and the game
  ends. Going past 7 never crashes, because the ending starts first.
- Missing: the route to the first boss that skips the cave, and a real hub death (section 2).
- Evidence: the state was set by hand. "fr 263 mode 1 hubT 2", then "hubT 3" after Continue. In the cave at
  `hubLevel` 6 and `hubTarget` 7, the ending started at frame 930.

**The frog bounce counter leaks into a big ball jump (`handControl.bounceCount`, u8).**

- What it does: as a frog in `HAND_FROG_WALK`, `bounceCount` rises by 1 every frame after a hop, on the ground too
  (`MaintainAction`, src/hand.c:2430-2432). Each hop sets it back to 1 (src/hand.c:2441). The reset to 0 on the ground
  skips frog walk (`MaintainAction`, src/hand.c:1438-1439). When the frog spell ends, `DefrogHand` does not clear it.
  If Glover lands on a squashy ball (rubber, power, beach, snow) in ball-walk with A held, the jump gets
  `vel.y = 7 + bounceCount * 1.2` (`ApplyAction`, src/hand.c:740 and 748).
- Effect: upward speed is capped at 15 after the first frame (`UpdateActor`, src/actor.c:451-453), so the bonus
  acts on one frame only. With a count of 30, the jump peaked 209 units above the ball. A normal ball bounce peaks
  near 100. Every later bounce repeats it while A stays held and Glover touches only the ball. The ceiling is about
  480 units at count 255. Past 255 the count wraps to 0 and the gain is lost.
- How to reach it: get the frog spell, hop off a high ledge, let the spell run out in the air, and land on a rubber
  ball with A held. Cheat 20 in `cheatInfo` gives the frog spell outside bosses. It sets the spell timer
  (`handControl.spellTimer[2]`) to 1000 frames. Entering the cheat again while the timer is above 50 sets it to 50
  (`ActivateCheat`, src/cheat.c:293-301). Sinking in water also counts frames (count 72 at the Pirates 1 seabed).
- Missing: a real route that puts the ball under the fall. The ball was placed by hand.
- Evidence: a run in level 20 (`PIRATES1`) from an input sequence, with the ball placed by hand. It printed
  "209: m2 p20 bc30 hv.y 43.000", with the peak at frame 227.
- This height is relevant to the Pirates 1 floating platform, which needs the hand 463 above the water. See the
  [Christ fly report](christ-fly.md#6-the-pirates-1-floating-platform).

**Two lives lost for one death.** See section 2.

**Timing-free 255 lives from a hub game over.** See section 2.

**Score past 2,147,483,647.** The s32 score has no cap (`AddToScore`, src/collect.c:850) and carries over through
the save. Replaying levels on one file would get there after hundreds to thousands of hours. The score would then
show negative, and one extra-life step would be lost. No gain.

### Theoretical

- **The Pirates boss counter after its death** (`enemies[0].brain.scratch`, s32, `DoPiratesBoss`,
  src/boss.c:318). It counts up every frame after Spank dies. After about 2.3 years it wraps, and the boss platform
  starts spinning again. Cosmetic.
- **The hand fall timer** (`handControl.fallTime`, s16, `MoveHand`, src/hand.c:2960). A wrap needs 32768 frames of
  fast falling (about 470,000 units). In a test with the counter set to 32760, Glover landed with no fall damage. No
  frozen fall state is known. In all 48 levels the longest count was 24.
- **The frog bounce counter wrap itself** (255 to 0). It needs 255 airborne frog frames, a drop of about 3800 units,
  and any landing after 17 fast-falling frames kills a frog. It was shown only with a teleport. The wrap only
  removes the useful large value.
- **Bugle hits in Carnival 1 and 2A.** One kill gives the dead Bugle's own `hits` -1. A wrap needs the repeat-kill
  loop above. Nothing reads the value.
- **s32 frame counters:** platform `idleCount` (it rises 3 a frame, wraps after 4.5 to 9 months, and then stops
  the rope bridge's sway), the robot's `bobCounter`, the pad hold and press timers, the enemy brain timers, and
  `levelFrames`. All need months or years.

## 4. Checked and found safe

These have no reachable wrap. Each is guarded (it counts only while nonzero or above 0), reset at a fixed value, or
taken modulo.

- **Bosses.** The Fear boss's `handPain`, `ballPain` (reset at 8), `frStunCount`, `frankieDead` and `elecTimer`.
  Spank's hits (`KillEnemy` at 0). The Prehistoric boss's hits (inside `if (hits)`). The Space boss phase 1 robot
  (never lowered). The robot's face portion (only while `hits == 1`). Kloset's `bombDelay` and `bombTimer`. The
  whale's hits. The boss time trial's `bonusTimer`. The boss health bar has 5 entries, and no boss goes above 5.
- **Collision damage** (`CheckActorCollisions`, src/collision.c:171, `hits -= damage`). `damage` is never written,
  so it always subtracts 0.
- **Player.** Health (0 to 3). Ball plasters (the ball bursts at the limit). Ball timers and the ball type index
  (always 0 to 6; -1 in `nextBallState` means "no next ball"). Power-up timers (stacking only lengthens them; Fear 1
  can stack Rotor Blades and Boomerang Ball without limit until the first hit cuts it to 40). Garibs and the garib
  combo. The push timer. The release and drop timers.
- **Spell timers.** Enemy frog timers can wrap after about 55 stacked frog hits on one enemy, but the enemy then
  turns back about 15 frames later. Stun and death cannot be stacked enough. Glover's own frog timer cannot get many
  hits (one Evil Glove shot in the Atlantis bonus).
- **Level data.** No retail level has a catapult, so the catapult timer wrap cannot run. All 17 teleports have
  times of 20 or less, far from the s8 limit.
- **Menus.** The save slot, the volumes, the pause text fade, the pause cursor and the name entry are clamped. The
  off-by-one menu index in `DefWait` (src/frontend.c:834) is in code that never runs.
- **Difficulty table.** It has three valid entries. A difficulty of 3 cannot be loaded, because the save's CRC
  rejects it.
