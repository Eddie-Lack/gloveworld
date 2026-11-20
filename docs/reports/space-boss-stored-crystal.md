# The stored crystal and the Space boss's second phase

This report describes the code and the US ROM (NGVE) as of October 2026. It explains why the second phase of the
Out of This World boss starts already damaged when a crystal is "stored", and whether the trick works anywhere
else. Addresses are N64 RAM addresses. For code in the main segment, the ROM offset is the address less
0x800FF000.

## Summary

- A **stored crystal** is a crystal the game still owes to the cave, while the ball is out of play. Players store
  one by slapping the ball into a hub portal after beating a world.
- While a crystal is owed, the level loader ignores ball start markers of type 2. Type 2 means "this level has no
  ball".
- Phase 2 of the Space boss (level 40, `SPACEBOSS3`, the shooting stage) has a type 2 marker. With a crystal owed,
  the real ball stays in the level as a live object at (10, 0, 0).
- On the first frame, the robot's arm platforms are still at the origin. The ball touches both gun platforms, and
  each arm loses 1 of its 3 points.
- The robot keeps all 3 hits and the health icons still show 3. Each arm needs 2 shots instead of 3, so the fight
  needs 7 shots on the robot's parts instead of 9.
- One point per arm is the most the trick can give. No other boss level has a type 2 marker.

## Terms

| Term | Meaning |
| --- | --- |
| `hubTarget`, `hubLevel` | Save slot fields. `hubTarget` counts crystals earned, `hubLevel` crystals placed in the cave. A crystal is owed while `hubTarget > hubLevel`. |
| ball start marker | The `TOKEN_BALL_START` record of a level file: a type, then the ball's start position. |
| `ballStatic` | 0: the ball is live. 2: the ball is parked at its start position, hidden and without collision. |
| portion | One of the Phase 2 robot's three parts: left arm (0), right arm (1), face (2). Each has 3 points in `portionHealth`. |
| hit | One of the robot's 3 hits, `enemies[0].actor.state->hits`. An arm's last point or the face's last point takes one. |

## Step 1: a crystal becomes owed

Leaving a world through its final exit (`WORLD_EXIT`) adds 1 to `hubTarget` the first time
(`CheckForExits`, src/game.c:2216-2219).

| What | Address |
| --- | --- |
| `CheckForExits` | 0x80123514 |
| `hubTarget++` (load, add, store) | 0x80123D00, 0x80123D04, 0x80123D10 |

The owed crystal is part of the save slot, so it survives saving and loading.

## Step 2: storing the crystal

In the hub, an owed crystal locks the portals of unfinished worlds. The lock only applies while `ballStatic` is 0
(`CheckForExit`, src/game.c:2443-2447). When the ball itself touches a world portal, the game hides it, sets
`ballStatic` to 2 and records the portal in `ballLeftBehind` (src/game.c:2516-2527). The lock no longer applies,
and the crystal stays owed.

| What | Address |
| --- | --- |
| `CheckForExit` | 0x801246AC |
| lock: load `ballStatic` | 0x80124808 |
| lock: compare `hubTarget` with `hubLevel` | 0x8012485C, 0x80124868, 0x8012486C |
| ball into portal: `ballStatic = 2` | 0x80124EE0 |
| ball into portal: `ballLeftBehind = exitType + 36` | 0x80124EE8, 0x80124EF4 |

Entering a world's levels from its Wayroom sets `ballLeftBehind` back to -1 (src/game.c:2172-2178). Only the owed
crystal is left.

## Step 3: the loader keeps the ball live

`ParseTokenGeneric` reads the ball start marker (src/parse.c:574-590). It skips a type 2 marker when a crystal is
owed. The rule serves the hub: there you must carry the owed crystal to the cave. The test does not check the
level, so it applies everywhere.

| What | Address |
| --- | --- |
| `ParseTokenGeneric` | 0x801835F4 |
| `type != 2` test | 0x80183AF8 |
| load `hubTarget`, `hubLevel` | 0x80183B1C, 0x80183B28 |
| `hubTarget != hubLevel`: skip the marker | 0x80183B2C |

When the marker is skipped, `ballStatic` keeps the 0 that `ParseLandscapeFile` (0x80182090) set, and the ball is
live. On every level load the ball's position is (0, 0, 0) when `ResetBall` (0x801AE510) runs, and `ResetBall`
sets x to 10 (src/ball.c:130). The ball starts at (10, 0, 0), whatever happened in the level before.

`RestartPlayer` (0x8017E49C) does not move it either. For level 40 it returns before the code that places the ball
(src/platform.c:1368-1382).

## Step 4: the ball hits the robot's guns

The Phase 2 robot is enemy type `EVIL_ROBOT`. `InitEnemy` (0x801919F4) gives it 3 hits on frame 0
(src/enemy.c:548-551; the store is at 0x80192BC8). `NMEProcessEvilRobot` (0x801A44F8) gives each portion 3 points
on frame 1 (src/nmeprocess2.c:2546).

Each arm is two platforms: a shoulder (tag 1 or 2) and a gun (tag 4 or 5). An armed platform has
`path->moving == -3`. When an actor of type `BALL` touches an armed platform, `CollideActorWithPlane` sets
`moving` to 1 (src/collision.c:905-917 and 1113-1124). The player's shots are `BALL` actors
(`InitBulletSpritesOrMesh`, src/bullet.c:303). Each shot is changed to type `BULLET` after it scores, so it scores
once. The code skips that change for the real ball (`if (actor != &ball)`), so the real ball can score repeatedly.

| What | Address |
| --- | --- |
| `CollideActorWithPlane` | 0x80158F84 |
| first site: `moving == -3` test | 0x80159524, 0x8015952C |
| first site: `actor == &ball` test | 0x8015958C |
| first site: `moving = 1` | 0x801595A0 |
| second site: `moving == -3` test | 0x8015A768, 0x8015A76C |
| second site: `actor == &ball` test | 0x8015A7D0 |
| second site: `moving = 1` | 0x8015A7E4 |

`NMEProcessEvilRobot` takes a point from an arm when either of its platforms has `moving > 0`
(src/nmeprocess2.c:2662-2719). The arm is then immune for 20 frames. On its last point the arm flies off and
`hits` drops by 1.

| What | Address |
| --- | --- |
| `moving > 0` tests | 0x801A4EF8, 0x801A4F08 |
| `immuneCount[portion] = 20` | 0x801A4F30 |
| `--portionHealth[portion]` | 0x801A4FA8 to 0x801A4FB8 |

Before frame 2, all four arm platforms are still at the origin, and all four are armed. The robot is at
(0, 350, 4000). A run with an owed crystal gives:

| Frame | Event |
| --- | --- |
| 0 | ball at (10, 0, 0); it touches gun platforms 4 and 5; both get `moving = 1` |
| 1 | each arm loses a point: `portionHealth` 2, 2, 3; both arms immune |
| 2 | the arm platforms move to the robot, about 4,000 units away |
| 2 onward | the ball comes to rest at about (18.6, 9.0, -0.6) and does not move again |

Without an owed crystal, the marker parks the ball at (0, -1000, -5000) and nothing touches the arms.

The arm damage happens during the intro cutscene. The health icons count `hits` (`PrintStats`, 0x8014D66C,
src/overlay.c:962-967), so they still show 3. What the player sees is smoke from both arms
(src/nmeprocess2.c:2634-2644), and each arm breaking after 2 shots.

## Can it be taken further?

Not in Phase 2. One point per arm is the limit:

- The ball's start position is fixed at (10, 0, 0). No earlier action changes it.
- The arms are within reach only on frames 0 and 1. Each arm is immune for 20 frames after its first point.
- The player cannot move the ball. `DoSpaceBoss3` (0x80131424) pins the glove to its checkpoint every frame
  (src/boss.c:1313) and puts it in `HAND_NOTHING` mode (src/boss.c:1332). The robot fights 2,000 to 4,000 units
  from where the ball rests.
- The face cannot be hit early. It only takes points once `hits` is 1 (src/nmeprocess2.c:2551-2570).

## Can it be applied elsewhere?

The owed crystal affects level loading only through the type 2 marker. The other code that reads `hubTarget` and
`hubLevel` sets them, chooses the hub level, or runs in the hub, the cave or on a hub exit (src/game.c:427,
1392, 2443-2447, 2492, 2672-2850). This
table lists the ball start marker of every level.

| Marker | Levels |
| --- | --- |
| type 0 (live ball at a position) | 0 (hub 1), 9 (assault course), 10, 15, 20, 30 (first levels of Atlantis, Carnival, Pirates and Fear) |
| type 1 | 25 (first Prehistoric level) |
| type 2 (no ball, unless a crystal is owed) | 1 to 6 (hubs 2 to 7), 8 (cave), 40 (Space boss phase 2), 42 (Wayroom) |
| type 3 (no ball, always) | 7 (hub 8), 14, 19 (bonus levels), 43, 44, 45 (presentation and fly-throughs) |
| no marker | all other levels, among them every boss level and Space boss phase 1 (38) |

Phase 2 is the only boss level with a type 2 marker. The other bosses, and Space boss phase 1, have no marker, so an
owed crystal changes nothing there. The Wayroom is the only other level outside the hub and cave that it affects:
with a crystal owed, the ball is live there. No gain from that is known.

## How this was checked

Temporary print statements reported each ball start marker, each arm platform trigger and each change in the
robot's health. The owed crystal was made by setting `hubTarget` to `hubLevel + 1` before the level loaded.
`--level 40` and `--level 39` (the control-room cutscene, then Phase 2) gave the same result.
