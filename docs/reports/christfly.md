# Christ fly and its height limit

This report describes the code and the US ROM (NGVE) as of November 2026. It explains Christ fly, which this report
also calls the water launch: a trick where a ball that rises through water carries Glover up and out of it. It asks
how high the launch can go, and whether it can reach the floating platform in Pirates 1 (level 20, `PIRATES1`).

The results come from two sources:

- **The code.** References such as `UpdateActor` (src/actor.c:451-453) are functions, files and lines in
  gloveworld's `src/` and `include/`.
- **Runs.** The runs used instrumented copies of [weemitt](https://github.com/Eddie-Lack/weemitt) (main at
  b013617). weemitt is a native PC port built from gloveworld's C. The copies added print statements. Most runs
  also started from a state set by hand: Glover holding a chosen ball in the open sea. Only the route of section
  6 used ordinary input from the level start (see "Setup of the runs"). The runs were headless (no picture),
  64-bit builds, at the original timing, in Pirates 1.

Each answer says **measured** (seen in runs) or **from the code** (read in the code, not run). The runs were not
checked on an emulator or a console. The numbers should hold on the N64, but nobody has confirmed them there.

## Terms and units

| Term | Meaning |
| --- | --- |
| height | The hand's height above the Pirates 1 sea surface. The sea is the water actor `deepwat`, with its surface at y -571. |
| speed | Units per frame. Y is up. |
| gravity | 1.2 units per frame, every frame (`StartNewLevel`, src/game.c:1401). |
| floating ball | A ball whose actor flags hold 0x200000. It takes the flag from `defaultBehaviour[type].flags` (`ActivateCollectable`, src/collect.c:555-557; `DropBall`, src/ball.c:604). |
| `enteredWater` | A flag on the ball's state. The game sets it when a floating ball goes down through the water's surface. When a ball with the flag comes back up through the surface, the game slows it to 3 or less. This makes a floating ball bob at the surface. |
| ball-walk | `HAND_BALLWALK`: Glover stands on top of the ball. |
| joined | `HAND_JOINED`: Glover holds the ball. |
| aim pose | `HAND_THROWAIM`: Glover holds the ball ready to throw. |
| the cap | The limit of 15 on upward speed (section 3). |

## Summary

- The water launch cannot reach the floating platform in Pirates 1. (measured)
- That platform needs Glover's hand about **463 units above the water**.
- The best found with balls Glover can make in Pirates 1 is **240.5** in the open sea and **220.0** at the
  platform.
- The limit is a fixed cap on upward speed. Every object in the game is limited to 15 units per frame going up.

## 1. What happens

**Short answer:** the ball's buoyancy carries Glover up while he stands on it. This is both measured and from the
code.

The steps, from the code. The runs follow the same steps.

1. **Sink.** Bowling balls and ball bearings sink. A sinking ball does not set `enteredWater`. A dribble
   (`HAND_BOUNCE`) also clears it (`ApplyAction`, src/hand.c:931).
2. **Transform.** Press R while holding the ball. `MaintainAction` sets `transformState` to 30 and starts
   `HAND_SPIN` (src/hand.c:1462-1466). During the spin `UpdateActorPositionDependencies` pins the ball to Glover
   (`MoveBallToHand(); ball.vel = hand.vel;`, src/actor.c:1264-1266). Glover does not float and has gravity, so
   both keep sinking slowly. The ball changes type partway through the spin (`MoveBallToHand`,
   src/ball.c:866-868). It now floats, but the pin throws its buoyancy away each frame.
3. **B hold.** The spin ends with `hand.state->mode = HAND_JOINED` (`MaintainAction`, src/hand.c:2715-2716). In
   the same `MoveHand` call the table of allowed actions runs (src/hand.c:3160-3165). If B has been held for 5
   frames, the aim pose starts (`ConditionsMet`, src/hand.c:252-256). In that pose Glover has no gravity in the air
   (src/hand.c:1382-1385 and 892-893) and `vel.y` is 0 (src/hand.c:1709). He hovers at his current depth, and the
   ball stays pinned. Without the hold, step 5 starts at once.
4. **Cancel.** Pressing Z cancels the aim pose (`MaintainAction`, src/hand.c:1710-1712). `ApplyAction(HAND_JOINED)`
   then gives the ball its speed from before gravity, and gives Glover the ball's speed
   (`ball.vel = ball.prevVel` and `hand.vel = ball.vel`, src/hand.c:762-769). This includes one frame of lift,
   about +4.8.
5. **Get on the ball.** On the next frame, the `HAND_JOINED` case sees a floating ball in water, more than its
   radius + 11 above the floor. It calls `ReleaseBall` and puts Glover on top of the ball in ball-walk
   (`MaintainAction`, src/hand.c:1455-1457).
6. **Rise.** Under water a floating ball gains up to 6 per frame of lift (`WaterDoFloating`, src/water.c:161-163).
   It loses no upward speed to water drag. It reaches the cap of 15 in about 3 frames. Each frame
   `UpdateActorPositionDependencies` puts Glover on top of the ball. While the ball rises, Glover's speed is set to
   1.2 times the ball's, which is 18 (src/actor.c:1182-1192).
7. **Leave the water.** Glover keeps 18 while he stands on the ball. When he leaves it, his speed is cut back to
   the cap of 15. After that he slows by 1.2 each frame. Ball-walk has `actionLock = 15` on entry (`ApplyAction`,
   src/hand.c:1008-1010). Once the lock ends, if the ball is out of the water and rising faster than 3.6, Glover
   goes to `HAND_WALK` with `hand.vel.y = ball.vel.y` (`MaintainAction`, src/hand.c:1872-1879).

Why Glover does not stick to the surface:

- The ball under him never got `enteredWater`. It went under as a sinking ball, or a dribble cleared the flag.
- So the ball keeps its full speed and flies out with Glover, about 107 above the water.
- If the ball does have the flag, it is slowed to 3 at the surface. Glover is no longer held at the ball and keeps
  his own speed, so he flies on alone.

**Corrections to the trick as players describe it:**

- **The dribble is not needed for Glover's height. (measured)** Without the dribble, the ball stays at the surface
  and Glover flies on alone. He went about 10 units *higher* that way: 146 to 148, against 135 to 141 with the
  dribble. The dribble only decides whether the ball flies out too: about 103 above the water with the dribble,
  about 65 without.
- **The B hold mainly chooses when the launch starts. (measured)** It adds only about 3 to 9 units. While Glover
  hovers, the new ball finishes growing to full size. Without the hold the launch starts as soon as the spin ends,
  wherever Glover is at that moment.
- **Glover only stuck to the surface very near it. (measured)** In these tests he got stuck only when the launch
  started less than about 20 units under the surface.

### The order of one frame

`MainGameLoop` runs these steps in this order (src/main.c:1015-1063):

1. `UpdateWater` (src/water.c:258) puts buoyancy and water drag into `vel`.
2. `UpdateActor(&hand)` and `UpdateActor(&ball)` (src/main.c:1041-1042) move each actor by `vel`, then apply drag,
   collision, gravity and the cap (`UpdateActor`, src/actor.c:451-453).
3. `ball.prevVel` gets the ball's speed from before step 2 (src/main.c:1037 and 1043). It holds the buoyancy but
   not this frame's gravity.
4. `UpdateActorPositionDependencies` (src/actor.c:1144) couples the ball and the hand for each action.
5. `MoveBall` and `MoveHand` run (src/main.c:1062-1063).

The buoyancy is added before the move and the cap comes after it. So a ball deep in the water moves 15 + 6 = 21
per frame, and its stored speed reads 15.

### Water physics

All of this is in `WaterDoFloating` (src/water.c:65), called from `CheckActorForWater` (src/water.c:515).

- **Depth.** The depth reference is `actorHeight = pos.y - radius / phys->bottomDiv` (src/water.c:75-76).
  `radius` is the actor's current radius. It grows over a few frames after a transform.
- **Not floating** (bowling ball, ball bearing, Glover): the sideways speed is scaled by 0.8 under the surface,
  and `vel.y` by 0.85 each frame (src/water.c:108-115). Gravity is divided by 2.5 and the sink speed is limited
  to -3.75 (`UpdateActorCollisionAndGravity`, src/actor.c:588-590). The bowling ball sinks at about 3.2 per frame.
- **Floating, under the surface:** the sideways speed is scaled by 0.9, and there is no damping of the upward
  speed. The lift each frame is `(water height - actorHeight) / 5 + gravity / 3`, added as
  `MIN(lift, waterBouy)` (src/water.c:161-163). `waterBouy` is 6.0 (0x801F1B00). The net gain is up to
  6 - 1.2 = 4.8 per frame. The full 6 needs the ball's bottom about 28 below the surface.
- **Floating, going down through the surface:** sets `enteredWater` (src/water.c:137). A dribbled ball has its
  speed scaled by 0.4 or 0.8 (src/water.c:140-146). A fast ball that meets the surface at a low angle skips
  (src/water.c:147-153). Any other ball has its sideways speed scaled by 0.7 and its upward speed by 0.5
  (src/water.c:155-157).
- **Floating, going up through the surface** (src/water.c:165-182): only if `enteredWater` is set. The ball's
  `vel.y` is scaled by 0.8, another actor's by 0.6. The flag is cleared, and `vel.y` is capped at 3.0. If the ball
  is dribbled when it reaches the surface, `HAND_BOUNCE2CUP` is applied instead (src/water.c:170).
- **Who clears `enteredWater`:** `ApplyAction` for `HAND_WHACK`, `HAND_THROW`, `HAND_BOUNCE` and `HAND_LOB`
  (src/hand.c:801, 904, 931 and 1099), and the upward exit above.
- **Glover while joined** is not checked for water (`CheckActorForWater`, src/water.c:519-521).
- **Currents** add `currentX`, `currentY` and `currentZ` each frame to every actor in the water box
  (src/water.c:103-106). The Pirates 1 sea has only x -0.2.

## 2. The best ball

**Short answer:** in Pirates 1 the rubber ball and the crystal ball are equal. The ball bearing to crystal is the
quickest, at one R press. This is measured.

| ball after the transform | Glover's peak, plain launch | how it is made |
| --- | --- | --- |
| crystal | 134.6, or 143.4 with the B hold | R from the ball bearing |
| rubber | 135.0, or 143.8 with the B hold | one more R press |
| power | 131, or 135 with the B hold | potion only |
| beach | 221, or 241 with the B hold | potion only |

A crystal or rubber ball made by a potion, not by R, gave 134, or 137 with the B hold.

- **R order:** rubber, bowling, ball bearing, crystal, rubber. The table is `nextBallState`
  (`HandTransformBall`, src/ball.c:1224). Its values read at run time are 7, 9, -1, 12, -1, -1, -1. Power, beach
  and snow balls are not in the cycle. The one exception is cheat 4 in `cheatInfo` (bit 0x10 of
  `cheatActivated`). While it is on, `MoveBall` sets the crystal ball's entry to 8, and R turns the crystal ball
  into the power ball (src/ball.c:223-227).
- **Start ball:** whether you start from the bowling ball or the ball bearing makes no difference. It only decides
  how many R presses you need.
- **Mass:** a ball's mass does not change the launch. (measured, and from the code) It matters for the dribble
  bounce (`84 / clamp(mass, 10, 15)`, src/hand.c:927 and 1777), for throws (`HandThrowActor`, src/hand.c:3744),
  and for the sideways push given on mounting (`ApplyAction`, src/hand.c:1013).
- **Drag:** a ball's air drag does not matter. `UpdateActorPosAndRot` damps the speed (src/actor.c:710-711). Then
  `CheckCollisions` sets `vel = pos - prevPos` for actors with gravity (src/collision.c:636-640), which undoes the
  damping. Every ball and Glover lose exactly 1.2 per frame in the air. (measured)
- **Power ball:** it is a little worse. It is smaller, so Glover stands lower on it.
- **Beach ball:** it is much better, for three reasons:
  - its flags hold 0x40, which halves its gravity (`UpdateActorCollisionAndGravity`, src/actor.c:581-582);
  - it is larger (radius 25, `bottomDiv` 1.4), so it leaves the water about 12 units higher;
  - Glover stands higher on it: 35 above its centre, not 19.

  But R never makes a beach ball, and Pirates 1 has no potions. So you cannot use it there.
- **Snow ball:** its potion could not be applied to a sinking ball in these tests. The snow ball also grows
  (`MoveBall`, src/ball.c:338-341), so it was not studied. It only comes from snow terrain.

The ball types, from `defaultBehaviour` (0x801ED8F0), read at run time:

| # | ball | floats | mass | radius | `bottomDiv` | half gravity (flag 0x40) | squashes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 0 | rubber | yes | 10 | 9 | 1.5 | no | yes (0.4) |
| 1 | bowling | no | 14 | 14 | 1.0 | no | no |
| 2 | power | yes | 7 | 6 | 1.2 | no | yes |
| 3 | ball bearing | no | 10 | 6 | 1.0 | no | no |
| 4 | beach | yes | 3 | 25 | 1.4 | yes | yes |
| 5 | snow | yes | 12, grows | 9, grows | 1.0 | no | yes |
| 6 | crystal | yes | 9 | 9 | 1.5 | no | no |

## 3. Is there a hard limit?

**Short answer:** yes. Upward speed is capped at 15 per frame. This is from the code and was confirmed in every
run.

- **The cap:** at the end of each object's update, `UpdateActor` cuts any upward speed above 15 down to 15
  (src/actor.c:451-453).
  - In every run the launch reached exactly 15 and never more.
  - Glover shows 18 only while he stands on the rising ball.
- **The rise after leaving the ball:** from 15 per frame, with gravity 1.2, Glover can rise at most about 94 more
  units (15 × 15 / 2.4).
  - He leaves the ball about 20 to 60 units above the water.
  - So a plain launch tops out at about **160**.
  - The best plain launch found was **157.9**. In it, the ball enters the water already floating, so
    `enteredWater` is set. The stick is tapped about 28 frames after the transform, with B held from 8 frames
    before. The hand is then 86 deep. Z comes 6 frames later.
- **Depth:** depth beyond about 40 units adds nothing. Less than about 20 units deep, the ball cannot reach full
  speed: Glover gets only 27 to 50 when the launch starts near the surface. (measured)

| hand depth when the launch starts | 20 | 25 to 35 | 45 to 90 | 110 to 155 |
| --- | --- | --- | --- | --- |
| peak height | 107 to 129 | 123 to 152 | 127 to 158 | 119 to 143 |

- **Choosing the depth:** moving the stick ends the spin at once after the transform (`MaintainAction`,
  src/hand.c:2699-2702). This is the only way to choose a launch depth in mid-water. Otherwise the spin lasts
  about 63 frames after the last transform, and in water deeper than about 150, Glover reaches the floor first.
- **The cancel frame:** a Z press 0 to 45 frames after the spin gave 137 to 143. The later values are a little
  higher, because the ball has grown to full size by then.
- **Spread at one depth:** at a given depth, the peak varies by 10 to 30 units. The cause is timing. When Glover
  first gets on the ball, he is locked onto it for 15 frames (src/hand.c:1008-1010). The peak depends on when
  this lock ends compared with when he leaves the water.
- **The one way past the cap:** `UpdateActor` skips the cap for an object with flag 0x1000000. The same flag
  removes the falling limit (src/actor.c:586-587). Only catapults (`CheckHandCatapult`, src/hand.c:3276-3292)
  and bomb blasts (`MaintainBulletBomb`, src/bullet.c:913-916) set this flag, and it is cleared on landing
  (`CheckHandCatapult`, src/hand.c:3233-3238). This is from the code.

Other limits, from the code:

- The lift is at most 6 per frame (`waterBouy`, src/water.c:163).
- The exit is capped at 3 when `enteredWater` is set (src/water.c:179-181).
- Falling is limited to 15 per frame (src/actor.c:592).
- Glover's speed is *set* from the ball's at each hand-over, never added: `hand.vel.y = ball.vel.y`
  (src/hand.c:1487 and 1878), 1.2 times while he stands on it (src/actor.c:1188), 0 while joined
  (`MoveHandToBall`, src/ball.c:724), and `hand.vel = ball.vel` (src/hand.c:769 and 1480).

## 4. Does mashing L help?

**Short answer:** not in the water. In the air it helps a little, but only while the ball is also flying. This is
measured.

- **In water:** L in ball-walk is ignored while a floating ball is in the water (`MaintainAction`,
  src/hand.c:1864). In a launch with `enteredWater` set, the ball stays at the surface, and L presses at any frame
  gave the same 157.9 as no presses.
- **In the air, ball flying too:** an L press in ball-walk puts Glover back into holding the ball
  (`MaintainAction`, src/hand.c:1864-1865). That restores the ball's speed from before this frame's gravity
  (`ApplyAction`, src/hand.c:764). In effect, each return gives back 1.2 units per frame of speed.
  - It also restarts the 15-frame lock, which keeps Glover on the ball up to its peak. There he stands 19 above
    it.
  - L counts only as a new press, so at most every second frame. That gives one return to holding every 4
    frames, and the ball's gravity drops from 1.2 to about 0.9 on average.
  - Best found: L tapped every 2 frames, from 8 frames after the launch, for 16 frames. This raised 142.8 to
    **175.1**.
- **Risk:** in that run the crystal ball broke when it fell back into the sea, which costs a life. A ball that falls
  faster than 14.25 for 19 frames and then hits something breaks (`MaintainAction`, src/hand.c:2747-2756).
- **L, then jumps:** L, then a jump and a double jump, gave 234.5. That is still below the best run without L
  (240.5, section 5).
- **Search:** 14,645 combinations of L timing and jump timing were tried, for each kind of launch.

## 5. More height, and stacking

**Short answer:** a jump and a double jump at the right time add about 80 units. Stacking launches does not work.
This is measured.

**Jumps from the ball:**

- A jump from the ball sets 7 per frame (`ApplyAction`, src/hand.c:740). Holding A adds 1.2 per frame for a few
  frames (`MaintainAction`, src/hand.c:1838-1840). A double jump sets 8 (src/hand.c:1849). Squashing balls add
  `bounceCount` × 1.2 (src/hand.c:747-749).
- These are slower than 15, so they help only near the top of the rise.
- **Best found:**
  - **240.5** in the open sea. The launch has `enteredWater` set and uses no L. A is pressed 19 frames after the
    launch and held 5 frames. A is pressed again 11 frames later.
  - **220.0** at the Pirates 1 spot (section 6).

**Stacking:**

- **Deeper water:** no gain (section 3).
- **A second transform in one launch:** not possible. A transform needs a sinking ball under water and a spin of
  about 60 frames. During the spin the ball is pinned to Glover.
- **Speeds are copied, not added.** Whenever Glover gets on or holds the ball, his speed is *set* from the ball's
  (src/hand.c:769 and 1878; src/actor.c:1188). So no extra speed carries over.
- **Falling back onto the ball:** Glover can land on the floating ball, push it under and launch again at 15. Each
  new launch starts from zero and hits the same cap. The best second launch was 210.6 with a jump and a double
  jump, and about 158 without. No second launch beat the first. (8,000 random input sequences after the first
  launch were tried.)
- **Hover:** the aim pose in the air holds Glover's height but adds none. A throw from it adds
  `gravity * 0.7 * 10` = 8.4, times mass / 10 (`MaintainAction`, src/hand.c:1730-1731). It sent the crystal ball
  189 above the water, but not Glover.
- **Slam:** a slam (Z) onto a squashing ball in water or on the ground sets 15 (`ApplyAction`,
  src/hand.c:735-738; `BallWalkAllowed`, src/ball.c:693). This reaches no higher than the launch, and the crystal
  ball does not squash. (from the code)
- **Dribble in the air after the launch:** Glover gets half gravity (`MaintainAction`, src/hand.c:1398-1402), but
  his speed is pulled toward the ball's each frame (src/hand.c:1764-1766). It gave less height in a test (a peak
  near y -440).

Not tested:

- A repeated climb that throws the ball from the hover and catches it again in mid-air (`HandCatchAllowed`,
  src/hand.c:3596). The throw sends the ball forward about 5 per frame. It is likely hard to set up, and it is
  still under the same cap.

## 6. The Pirates 1 floating platform

**Answer: no.** The trick cannot reach it. This is measured.

**What is needed:**

- The platform is platform tag 114, object `islepla`, at (2554, -132, -311). Its collision box spans x 2498 to
  2608 and z -367 to -256. Its top is flat at y -121. A hand standing on it would be at about y -108, which is
  **463 above the water**.
- The ground under the platform is the island's sand (y -548.5 at its centre), not water. Open water is only on
  the platform's -z side, toward the ship's plank (z below about -375).
- Rising straight up under it is not possible: the sand is there, and its underside (y -143) would block. So
  Glover must launch from the water past the -z edge, then move sideways onto the platform while above y -121.
  From z -450 (50 deep) that is about 83 units. From z -550 (103 deep) it is about 183 units.

The sea floor just past the platform's -z edge (x 2554):

| z | -380 | -400 | -450 | -500 | -550 | -600 | -640 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| floor y | -577 | -590 | -620 | -650 | -674 | -698 | -717 |
| depth | 6 | 19 | 50 | 79 | 103 | 127 | 146 |

**Best at the platform:** with a real route there, the best was **220.0**, at hand y -351. That is about 243
units short.

- The route starts at the level start and uses ordinary input. Glover walks off the roof of the start hut and picks
  up the ball (frame 168). He taps R twice (frames 176 and 196), which makes the rubber ball a ball bearing. He
  pushes it along the lagoon floor, over the jetty, through the sea and over the island to the water past the
  platform's -z edge. At frame 892 he holds the ball bearing on the sea floor at (2541, -636, -484), 65 under the
  surface and about 117 units in -z from the platform's edge.
- The launch: R at frame 900 (ball bearing to crystal), B from 955, Z at 999. This alone gave 137.4 above the
  water. A at frame 1017 for 5 frames and A again at 1028 gave the 220.0.
- 1,224 combinations of cancel, L and jump timing were tried, with and without the stick held.
- A sinking ball keeps Glover on course on the way. With the rubber ball in water he got onto the ball, and the
  reversed ball controls of medium difficulty took him off course.

**Best anywhere:** the best anywhere with a ball Glover can make there is 240.5, which is 222 short.

**Beach ball:** even the beach ball, which Pirates 1 cannot make, reached only 393.7, with L, a jump and a double
jump. That is 69 short.

**Sideways movement:** in the best runs Glover rises almost straight up. He moves 0 to 45 units sideways with the
stick held.

**The island pool** (`islewat`, surface y -535.5) is 36 units higher than the sea, but only 25 units deep. That is
too shallow for full lift. It also lies under the other platform (tag 94), not this one. (from the code and the
level data, not launched from)

The level's other waters are the start lagoon (`pi3watb`, surface y -575, current x +0.4) and the sea (`deepwat`,
surface y -571, current x -0.2).

What could still work, as far as the code shows:

- **No-cap flag:** an object with flag 0x1000000 from a catapult or a bomb blast has no cap. No catapult or bomb
  is known in Pirates 1.
- **Moving platform:** platform tag 94, in the island pool, is the normal way up. A ball button on the sea floor
  (tag 117, under the sand hill tag 158) starts it. A second ball button (tag 18) moves the sand hill and uncovers
  button 117. The water spout lifts platform 94 about 420 units, and it bobs between tops of y -101 and -202. It is
  14 units from platform 114 in z. When the hand or the ball first lands on platform 114, it moves to the edge of
  the high ground near the ship's plank.

## 7. Similar tricks out of the water

Everything in this section is from the code. None of it was run.

- **Standing on any rising ball:** this gives Glover 1.2 times the ball's upward speed
  (`UpdateActorPositionDependencies`, src/actor.c:1188). He keeps the ball's speed when he steps off
  (`MaintainAction`, src/hand.c:1878).
  - Anything that pushes the ball up works the same way out of water.
  - Examples: a dribble bounce (8.4), a slam onto a squashy ball (15), trampolines, bouncy ground, wind and vents,
    and moving platforms.
  - All of these are under the same cap of 15.
- **L in the air:** the speed given back by L (section 4) works with any ball in the air (src/hand.c:764).
- **Copied speed:** when Glover goes from holding the ball to a dribble, he gets the ball's upward speed
  (`MaintainAction`, src/hand.c:1485-1488). When he slides off the ball, he gets all of its speed
  (src/hand.c:1479-1484).
- **Catapults and bomb blasts:** these are the only sources with no cap until landing
  (`CheckHandCatapult`, src/hand.c:3276-3292; `MaintainBulletBomb`, src/bullet.c:913-916). A bomb blast gives at
  least 5 upward and a push of 14. These are the best places to look for height beyond the cap.
- **Platform bounce jumps:** each bounce adds 2, up to 8 (`ApplyAction`, src/hand.c:754-758).
- **Water currents:** a current with an upward part would add speed every frame, up to the cap
  (`WaterDoFloating`, src/water.c:105). The Pirates 1 sea has only a sideways current.
- **The frog bounce counter** can give one large upward speed on a squashy ball. The
  [wraparounds report](wraparounds.md#possible) describes it.

## Setup of the runs

- **Level start.** The runs started Pirates 1 the way weemitt's `--level 20` does: as the level select does, in
  save slot 6, at medium difficulty.
- **Real route.** The route of section 6 used only ordinary input from the level start.
- **Teleport.** Most sweeps used a hook that started Glover holding a chosen ball at a chosen point in the open sea,
  at (2000, y, 0), where the floor is at about y -745. The hook set the ball with `ActivateCollectable`, the
  function the game calls for a potion and for the R transform. A bowling ball or a ball bearing then became
  crystal or rubber with R, as in the game. The power and beach balls were given by a potion at a chosen frame.
- **Camera.** An actor's animation advances only while it is within the camera's far plane (`DrawActor` calls
  `IsActorVisible`, src/object.c:392). After a teleport the camera was far away, and the spin animation froze.
  The hook moved the camera with Glover. With it, the transform comes 8 frames after R, as on land and in the real
  route.
- **Level data.** The positions of the waters, platforms and floor were read from the game's own data at run
  time, at frame 150 and later.
