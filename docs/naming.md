# Names

This file records how the names in the code were chosen, and the evidence for each name that replaced a
placeholder (`unkNN`, `D_XXXXXXXX`, `func_XXXXXXXX`, `argN`). Add to it whenever you give such a name. The
rules for choosing names are in [conventions.md](conventions.md#names), and the evidence sources are described
in [sources.md](sources.md).

Confidence is **high** when the code and the game's behaviour make the meaning clear, and **medium** when the
evidence is strong but not complete. Names without evidence are not given: a placeholder is better than a
guess.

## Struct fields

One table per struct. "Old" is the placeholder the field had. Fields not listed kept their name (they were
already named, or nothing shows what they do).

### Actor

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk1C | staticMtx | high | matrix allocated as "statMtx" in CalculateStaticMatrices for static objects, filled by guMtxF2L and used by DrawActor |
| unk22 | active | medium | 1 in the actor lists, 2 waiting to be removed (UpdateActor calls SubActor and sets 0), 0 not active |
| unk23 | sparkleRate | medium | set by TOKEN_SPARKLY; platforms with it are recalculated, and water faces sparkle every unk23 frames |
| unk24 | lifeTimer | medium | bullets and debris: life counted down to removal; the hand uses it as the length of its sparkle effect |
| unk26 | alpha | medium | translucency passed to SetAIRenderMode: 100 normal, 0 hidden, 200-255 for flashes and fades |
| unk2C | link | medium | actor this one is joined to: the hand for a held ball, an enemy or the ball the hand stands on |
| unk40 | prevPos | high | pos at the start of the update (actor->pos copied before moving); differences give the frame's movement |
| unk4C | platPrevPos | medium | passed as prevPos to UpdateActorPlatform, which carries the actor with its platform |
| unk64 | prevVel | high | UpdateActor copies vel into it before changing vel; fall-speed tests use its y |
| unk70 | friction | high | set from ActorPhysics.friction; the velocity is multiplied by it every frame |
| unk9C | rotQuat | high | orientation as a quaternion (GetQuaternionFromXYZRot, QuatSlerp, zeroQuat at init) |
| unkCC | shadow | high | allocated as "Act_Shad"; holds the altitude, the floor normal and the shadow size and fade |

### ActorObject

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk010 | contactPos | high | func_80158F84 stores q (the sphere centre projected onto the touched plane) in unk010[facenum]; Altitude uses unk010[0].y as the ground height; UpdateActorCollisionAndGravity subtracts it. |
| unk040 | contactSphere | high | func_80158F84 stores the sphere index i in unk040[facenum]; used as index into globalPos/globalRot/spheres[] for plane j. |
| unk044 | contactCount | medium | CheckCollisions increments unk044[j] while plane j is the same as last frame, resets to 0 on a new plane (index 0 counts airborne frames); MoveCam and MoveHand test >= 2. |
| unk054 | resting | medium | Set to 1 in func_80158F84 when the touched plane faces up (normal y > 0) and cleared when the contact bounces; gravity code needs unk054[j]==1 for support, ball roll sound needs unk054[0], bubble bounce sound needs !unk054[0]. |
| unk180 | contactPlatform | high | Cleared each frame, set to the platform in func_80158F84 for any platform touched (platform is only set for standing-on); read by puzzle conditions and bosses as the platform under the actor. |
| unk184 | prevContactPlatform | high | UpdateActorCollisionAndGravity copies unk180 into it before clearing unk180; PuzzleEvaluateCondition compares unk180 vs unk184 to detect stepping on/off a platform. |
| unk188 | onGround | high | UpdateActor sets it to 1 when a contact plane has normal y >= 0.75 (or a slide terrain), else 0; nearly all hand/ball/enemy code tests it for 'standing'; CollideActorInsideSphere also sets it. |
| unk189 | wasOnGround | high | UpdateActor copies unk188 into unk189 before recomputing it; used as 'unk188 && !unk189' = just landed in many bullet/enemy/hand functions. |
| unk18A | contactFlags | high | Cleared each frame; func_80158F84 /= 1 face contact, /= 2 near an edge with upward normal, /= 4 near other edge; CheckActorCollisions /= 8 actor-actor contact. |

### ActorPhysics

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk04 | accel | high | Passed as the acceleration argument of AccelerateActor2D(actor, phys->unk04, phys->unk0C) in enemy movement and scaled by mass in ProcessInput; ball value is 4.5/radius. |
| unk08 | maxSpeed | high | UpdateActor clamps /vel/ to it every frame; passed as max speed to AccelerateActor/AccelerateActor2D/ActorMaxOutVel in hand.c; table value is 2x unk0C (26 vs 13, 14 vs 7). |
| unk0C | moveSpeed | medium | Max speed of controlled/AI movement: ProcessInput uses unk0C * terrain factor as maxSpeed and passes it as max to AccelerateActor2D; hand 7.0, ball 13.0, enemies 2-8. |
| unk10 | minSpeed | medium | CheckCollisions zeroes vel.x/z when horizontal movement < unk10; UpdateActor/UpdateActorCollisionAndGravity compare speed with it to decide the actor has come to rest on a slope. |
| unk14 | slideLimit | medium | Plane normal y <= 1-(1-unk14)/terrain.unk0C means gravity pulls the actor into/along the slope (ball 0.99 slides on any slope, enemies 0 only on walls); ConditionsMet case 7 (slide action) uses the same test. |
| unk18 | friction | medium | Actor.unk70 is set from it (ResetBall, CheckCollisions); func_8015EE6C multiplies vel by it (air, quartered) or by terrain-scaled 1-(1-unk70) on ground; hand 0.8-0.92, ball 0.94, boss weapon 1.0 = none. |
| unk1C | rotDrag | high | func_8015EE6C multiplies rotVel[i] and spinSpeed by it every frame. |
| unk20 | bounce | high | CheckCollisions multiplies the bounce velocity by speed * unk20 and func_80158F84 skips bouncing when <= 0, negates platform spin by it; table values 0.8 (ball) / 0.1 / -1 (no bounce). |
| unk28 | turnSpeed | high | ActorHandleRotaim steps rot toward rotAim by at most unk28 per frame; hand set to mag/80*0.6, camera 0.05-0.5, enemy values 0.02-0.9. |
| unk2C | bottomDiv | medium | Only use: WaterDoFloating computes the actor's bottom as pos.y - radius/unk2C (default 2.5, hand 1.0, bullets 1.5). |
| unk30 | maxSquash | high | Passed as the maxSquash argument of SquashActor (10 callers); nonzero tests enable squashing of the ball. |
| unk38 | squashMin | medium | UpdateActor ends the squash (squash = 0, scales 1.0) once squashTimer >= 5 and squash < unk38 (0.01). |
| unk3C | squashPeriod | medium | UpdateActor uses it as the SineWave period of the squash wobble (sign picks the phase); SquashActor only applies the width stretch when it is > 0. |
| unk40 | squashDecay | high | UpdateActor does state->squash *= phys->unk40 every frame (0.95 or 1.0). |
| unk44 | flags | high | Default Actor.flags: InitEnemy does actor->flags = phys->unk44 & ~0x100, ball.flags /= / &= ~phys->unk44, tested with 0x200000 and 0x100. |
| unk48 | radius | high | Default sphere radius: InitEnemy and ApplyAction do spheres[0].radius = spheres[0].unk1C = phys->unk48; ball entry is 9, ball snow growth is measured against it. |
| unk4C | radiusRate | high | UpdateActor/MaintainAction move each sphere radius toward its unk1C by clamp((unk1C-radius)/4, -unk4C, unk4C) per frame. |

### ActorShadow

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk00 | altitude | high | Set from Altitude(pos, &floor, ...) (height above ground), -1.0f means no ground; passed as the altitude argument of CastShadow (the Garib struct calls the same value altitude). |
| unk04 | floor | high | Vec3 * out-param of Altitude (named floor there) = plane normal of the ground under the actor; passed as the plane argument of CastShadow; MoveHand tests ->y > 0. |
| unk08 | shadowSize | high | Passed as the size argument of CastShadow; set to about radius*0.8 for actors. |
| unk0C | shadowAlpha | high | Current shadow alpha: ProcessShadows fades it toward unk0D by unk0E per frame (FADE_SHADOW) and passes it as alpha to CastShadow (Garib has the same pair shadowAlpha/shadowTarget). |
| unk0D | shadowTarget | high | Target alpha for the shadow fade, set to 255 or 0 by ApplyAction, UpdatePickups, ProcessShadows (distance cull). |
| unk0E | shadowStep | high | Alpha step per frame in FADE_SHADOW(unk0C, unk0D, unk0E); set to 10/20/30/40. |

### ActorState

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk01 | damage | medium | Only read: CheckActorCollisions does actPtr->state->hits -= actor->state->unk01 when two actors collide (never written; always 0 from the zeroed alloc). |
| unk03 | teleportTimer | high | Set from tp->unk10 when a teleporter is entered, counts down to 0 (shrink), then set to -tp->unk12 and counts up to 0 (grow) in func_8015F098, which scales the actor by it. |
| unk06 | prevMode | high | ApplyAction does hand.state->unk06 = hand.state->mode before mode = action; MaintainAction/ConditionsMet compare it with earlier mode numbers (e.g. unk06 == 29). |
| unk08 | flying | high | InitEnemy sets 1 when !(flags & 1) (no gravity); NMEActionRandomMove/FollowPlayer/etc choose 3D NMEActionMoveTo instead of 2D when it is 1; CheckCollisions keeps speed in 3D. |
| unk09 | waterState | high | WaterDoFloating sets 4/1/3/2 by how deep the actor is below the water surface, UpdateWater clears it to 0; read as 'in water' by hand/ball/enemy code, camera copies it for underwater reverb. |
| unk0A | enteredWater | medium | WaterDoFloating sets 1 when a floating actor first crosses the surface downwards, and on leaving the water clears it (damping vel.y); ApplyAction clears it for ball jumps. |
| unk0B | onHurtPlatform | medium | Cleared each frame in UpdateActorCollisionAndGravity, set in func_80158F84 when touching a platform with flags & 8 (parse token that also sets 0x4000 hurt); UpdateActor only records the hand's safe platform position (used by HurtHand) while it is 0. |
| unk0C | teleport | high | Set to the PlatformTeleport tp in func_8016225C when an actor steps on a teleporter, cleared to NULL when the teleport finishes; UpdateActor calls func_8015F098 while it is set. |
| unk10 | squashXZ | high | Scale applied to X and Z in DrawActor guScaleF(unk10, unk14, unk10); set to 1+power / maxSquash in SquashActor, eased back to 1.0 in UpdateActor. |
| unk14 | squashY | high | Y scale in DrawActor guScaleF(unk10, unk14, unk10); set to 1-power in SquashActor, eased back to 1.0 in UpdateActor. |
| unk30 | hitFadeTimer | high | Set to 20 when a boss loses a hit and decremented each frame; PrintStats uses it to fade/scale the lost boss health marker (alpha = unk30*255/20). |

### AmbientSound

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk46 | duration | medium | AddAmbientSfxAtPoint arg6: non-zero means the looping sample plays for this many frames (timer reloaded with it) before UpdateAmbientSounds fades it out. |
| unk48 | volume | high | Volume argument stored in AddAmbientSfxAtPoint and used to restart the sample with PlayContinuousSample. |

### AnimState

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk04 | finishedAnim | high | UpdateAnimations sets it to current+1 on the frame a non-looping animation reaches its end (0 otherwise); many callers compare it with an animation number+1 (== 13, == 11, != 0). |
| unk08 | loop | high | AnimateActor stores its loop argument in it (or queueLoop[0]); UpdateAnimations only stops at the end when it is 0, else wraps. |

### Backdrop

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk0C | offsetX | medium | Parsed from the level file (buf[0]); child backdrops place x = parent->unk68 + unk0C * 4 in UpdateBackSprites. |
| unk10 | offsetY | medium | Parsed from buf[1]; child y = parent->y + unk10 * 4. |
| unk48 | tex | high | The Texture whose data, palette, width, height, fmt, siz feed guSprite2DInit in PrintBackdrop; looked up by FindTexture in ParseTokenGeneric. |
| unk4C | x | high | Screen x passed to gSPSprite2DDraw (set to (s32)x - width in UpdateBackSprites); y is the paired field. |
| unk60 | visible | high | Set to 1/0 in UpdateBackSprites depending on whether it is on screen; PrintBackdrops skips it when 0. |
| unk66 | yBias | medium | Read from the level file; added to the camera-pitch term in the vertical centring of wrap backdrops (centreY = (tilt.x + unk66) / 2 + 136). |
| unk68 | scrollX | medium | Written x + width * wrap for the wrapping backdrop; its children take x = parent->unk68 + offsetX * 4. |
| unk5C+flipX, unk5E+flipY | flipX, flipY (s16) | high | ParseTokenGeneric copies two 16-bit level-file values into 0x5C and 0x5E, and the draw code passes them to gSPSprite2DScaleFlip, which keeps the low byte; as `s16` fields the ROM's byte loads come out the same. 0 in every backdrop of the 43 levels |

### BallControl

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk00 | loseTimer | high | set by LoseBall(arg0)/BurstBall (60), KillHand (81) and MaintainAction (30 when falling too far); counts down in MoveBall and calls func_8017479C (lose a life) at 0; nonzero blocks garib pickup, hand catch and enemy attacks |
| unk08 | enemyOffset | high | set to the offset in NMEActionHoldPlayer or to ball.pos - enemy.pos in NMEProcessChuck, rotated by the enemy rotation and added to the enemy position to place the held ball (beside enemyRot) |
| unk14 | enemySphere | high | set to the sphere tag in NMEActionHoldPlayer (or -1) and used as the index into ballControl.enemy->obj->spheres to place the held ball |
| unk16 | releaseTimer | medium | set to 5 or 1 by DropBall, 6 when the hand lets go with Z, 0 by every pickup action; counts down in MoveBall and clears ball.unk2C (holder) at 0; hand cannot re-grab while nonzero |
| unk18 | fallTime | high | ++ each MoveHand frame while the ball is airborne with vel.y < -14.25 and no plane, else 0; compared with 19 and 41 in MaintainAction to start the ball-lost sequence (unk00 = 30) |
| unk1A | returnTime | medium | set to 1 when the ball is whacked with powerup 13 and ++ in MoveBall up to 50, or set to 50 on pickup of 13/14; at >= 50 the ball is pulled to the hand and HandCatchAllowed uses a wider range |
| unk1C | prevGround | medium | unk1C[0] = ball.obj->unk188 (on-ground flag) copied each MoveHand and tested with !unk1C[0] in MaintainAction to detect the landing frame; unk1C[1] is only ever written (1) |
| unk1E | state | high | 0 free (ResetBall/DropBall), 1 held by an enemy (NMEActionHoldPlayer), 2 bursting/lost (BurstBall, ShatterBall), 5 swallowed inside enemy type 28; tested in BallWalkAllowed, HandCatchAllowed, NMEActionFollowPlayer |
| unk104 | cancerTimer | high | set by GiveBallCancer(arg0), counts down in MoveBall and restores the ball texture and numSpheres at 0 |
| unk108 | cancerScale | high | 0 in GiveBallCancer, eased up to 1.0 (down while unk104 < 40) and used as the scale of the three extra balls (spheres 1-3) in MoveBall |
| unk10C | hitFlags | medium | bit 8 set in CheckActorCollisions when the hand in ground-slam mode 18 touches the ball, cleared every frame in MainGameLoop, tested to break off snow and to drop the ball from Chuck |
| unk11C | changeTime | high | zeroed in ChangeBallInto, ++ each MoveBall, compared with ballChangeTime and divided by it as the blend factor for BlendBallTexture |
| unk160 | transformState | medium | set to 30 when R starts the transform spell (MaintainAction), HandTransformBall sets -20 and MoveBall counts it up to 0; positive makes sparkle debris, zeroed on release/shatter |

### Blur

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk570 | buffer | high | Index of the double-buffered vtx[2][30] being filled in DrawBlurFX, flipped with 1 - unk570 after each draw. |
| unk576 | oldest | high | Start index of the point ring buffer in DrawBlurFX (index is the write position); advanced past fully faded points. |
| unk577 | idleCount | high | Incremented each frame DrawBlurFX has fewer than 4 vertices, UpdateBlurs removes the blur when it is 11..254; 255 (platform blurs) means permanent. |

### Bullet

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk15C | age | high | set to 0 in CreateNewBullet, ++ each frame in UpdateBullets, compared with BulletInfo.unk9E and 20 for ground contact; negative values (-1, -2) hold a bullet back. |
| unk15E | spellTime | medium | copied from handControl.unk55E (the collectable's time value) in FireBulletFromActor and passed as the timeVal argument of ActivateCollectable(bullet->spell, ...) when the bullet hits. |
| unk160 | targetFlags | medium | copied from BulletInfo.unkA0; BulletAcquireTarget tests 1 ball, 2 hand, 4 nearest enemy/garib, UpdateBullets 8 re-acquire each frame and 0x10 stuns the hand. |
| unk163 | spellType | medium | copied from player.unk68 (spell type picked up) and used to index spellColour[unk163 + 1] for the sprite colour and passed to AddSpellDebris. |

### BulletInfo

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk9C | life | high | copied to actor.unk24 in CreateNewBullet, which UpdateBullets counts down and kills the bullet at 0; func_801ABD38 uses (life - unk24) as elapsed time; ROM table has 5000, 400, 100, 6... |
| unk9E | targetDelay | medium | UpdateBullets lets a bullet hit its target only when age > unk9E and steer only when age >= -unk9E (ROM values 0, 40, -50, 500). It delays hitting (positive) or homing (negative). |
| unkA0 | targetFlags | medium | copied to Bullet.unk160 in CreateNewBullet (ROM values 1, 2, 4, 0x10). |
| unkA8 | maxSpeed | high | copied to actor.phys->unk08 which actor.c uses as the speed clamp (vel scaled by unk08/speed when faster); ROM values 5-90. |
| unkAC | drag | medium | copied to actor.unk70 and phys->unk18, the per-frame velocity multiplier in func_8015EE6C (1.0 none, 0.9-0.98 slows); ROM values 0.9-1.0. |
| drag | friction | high | copied to actor.unk70 and phys->friction in CreateNewBullet |

### Buzzer

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk31 | buzzing | high | 1 while the electric arc is on: UpdateBuzzer toggles it as the timer expires, only draws the lightning and hurts when set, starts/stops the sample, ParseTokenPlatformFeatures sets it 1 (permanent when offTime is -1). |
| unk50 | jitter | high | passed as the jitter (f32) argument of AddLightning and loaded from the landscape token 0xB5 right before unk54. |
| unk54 | width | high | passed as the width (f32) argument of AddLightning, loaded from the landscape token 0xB5. |

### CameoItem

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk02 | time | medium | 0 when idle, set 1 at start, ++ each frame in ProcessCameoInstruction until it equals unk04, then reset to 0; == 1 triggers the one-off start actions |
| unk04 | duration | medium | parsed from the file; instruction ends when unk02 == unk04; 0 is a one-frame instruction, -1 means until the animation, rotation or move finishes, < -1 means when another item finishes |
| unk06 | trigger | medium | parsed from the file; -1 starts with the cameo (StartCameo), >= 0 starts when that item is done, < -1 starts when the cameo frame count equals -unk06 (UpdateCameo) |
| unk08 | done | high | cleared each frame in UpdateCameo and set to 1 when the instruction has finished; other items and NMEProcessEvilGlove test it |
| unk0A | target | high | actor selector parsed from the file: 0 is the hand, n > 0 is enemies[n - 1], -1 and -2 are the camera and camera target |
| unk18 | speed | high | animation speed for type 0 (AnimateActor last argument) and per-frame move distance for type 2 |
| u.h.unk0C, u.h.unk0E | u.args[2] | high | two s16 arguments read by ParseTokenCameo; their meaning depends on the instruction: animation number and loop flag (CAMEO_INSTRUCTION_ANIMATION), actor and sphere index (TIE_ACTOR_TO_SPHERE), base delay and random range (LIGHTNING) |

### CameraInfo

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk000 | distance | high | MoveCam eases it toward unk15C (the wanted distance) by 5 per frame and compares it with the camera's horizontal distance from the target; puzzle action 20 sets it with levelCamDistance |
| unk0E8 | targetOffsetY | high | unk000 / 11 in UpdateActorPositionDependencies, added to focus->y to give unk0EC, the height of the camera target |
| unk0EC | targetY | high | wanted y of cameraInfo.actor (the camera target): focus->y + unk0E8, and cameraInfo.actor.pos.y is eased toward it |
| unk0F0 | prevTargetY | high | SetCameraOverride saves unk0EC in it, MoveCam puts it back in actor.pos.y when the override ends |
| unk0F8 | heightOffset | medium | MoveCam sets it from the slope term plus unk160 and then pulls camera y toward target y + unk000/3 + unk0F8 |
| unk108 | focusVel | high | points at the velocity of whatever focus tracks (&hand.vel with &hand.pos, &ball.vel with &ball.pos), used to lead the target |
| unk10C | prevFocusVel | high | SetCameraOverride copies unk108 into it, MoveCam restores unk108 from it, as prevFocus does for focus |
| unk110 | prevTargetPos | high | SetCameraOverride saves cameraInfo.actor.pos in it and MoveCam restores it when the override ends |
| unk11C | prevDestActive | high | SetCameraOverride saves unk130 in it and MoveCam restores unk130 from it |
| unk120 | prevDestPos | medium | MoveCam copies it to unk134 when unk130 == 1 after an override, pairing with unk11C; nothing in the C code ever writes it |
| unk12C | overrideTimer | high | SetCameraOverride stores its time argument, MoveCam counts it down and restores the saved focus/camera at 0 |
| unk130 | destActive | high | set to 1 by puzzle actions 28/32, bosses, bonus levels; MoveCam then glides the camera toward unk134 and clears it each frame |
| unk134 | destPos | high | position the camera glides to when unk130 == 1 (quarter of the gap per frame, max 20); set from puzzle actions, bosses and camera.pos |
| unk140 | prevCamPos | high | SetCameraOverride saves camera.pos in it and MoveCam restores camera.pos from it |
| unk14C | orbiting | high | set to 1 by bosses and puzzle action 29, cleared by action 27 and every frame; MoveCam then swings the camera around the hand toward unk150 |
| unk150 | orbitAngle | high | angle the camera is swung to when orbiting: atan2(hand.x, hand.z) in the bosses, cam->unk00.x in puzzle action 29; copied to unk158 |
| unk154 | orbitSpeed | high | cap on the orbit push in MoveCam (1.3/0.7 in the bosses, cam->unk0C in puzzle action 29) |
| unk158 | focusAngle | medium | MoveCam stores Aabs(atan2) of the camera-to-focus direction in it; PlayerSpinCamera steers the camera toward it while unk1AC is set |
| unk15C | targetDistance | high | wanted value of unk000: levelCamDistance * cameraZoomVal[cameraZoom] each frame, set by puzzle action 26 and by the bosses |
| unk160 | targetHeight | high | levelCamHeight each frame, set by puzzle action 25 and by the bosses; added to unk0F8 in MoveCam |
| unk164 | alignBehind | medium | MoveCam: when set, the camera is not blocked and the stick is idle, it is pushed round until camera yaw matches hand.rot.y (behind Glover); never written in C |
| unk168 | slopeAdjust | medium | set to 1 each MoveCam, cleared by puzzle action 31; when set MoveCam derives camVal from the ground normal under the hand to raise or lower the camera |
| unk16C | turnTimer | medium | MoveCam counts it down while pushing camera yaw toward unk170 and stops early within 0.05 rad; puzzle camera actions are skipped while it runs; never set in C |
| unk170 | turnAngle | medium | angle MoveCam steers camera.rot.y toward while unk16C is non-zero (FindShortestAngleSigned(unk170, camera.rot.y)) |
| unk174 | followSpeed | high | max distance cameraInfo.actor moves toward the focus per frame; ramps up to 20 once unk178 is 0, reset to 0/1/2/5 on events |
| unk178 | followTimer | high | counts down in UpdateActorPositionDependencies holding unk174 from ramping; set to 1/2/5 together with unk174 = 0 |
| unk17C | sideOffset | medium | current sideways shift of the camera target, eased by unk184 toward unk180 and rotated by camera yaw into the target offset |
| unk180 | sideOffsetAim | medium | wanted sideways shift: 0, +min(unk000/10,40) or -min(unk000/10,40) depending on which way Glover faces relative to the camera |
| unk184 | sideOffsetVel | medium | (unk184 + a) * 0.6 where a is a clamped step toward unk180; added to unk17C each frame |
| unk198 | squeezeDistAvg | medium | DoCameraCollision moves it 1/8 of the way toward unk19C every frame; read nowhere else |
| unk19C | squeezeDist | medium | DoCameraCollision: when the camera is jammed against a wall, min(unk000 - horizontal dist to hand, 120) clamped to >= 0, else 0 |
| unk1A0 | noCameraAction | medium | MainGameLoop sets 1 every frame while a level is played; puzzle.c sets 0 when a camera puzzle action starts (IsCameraAction, only when turnTimer is 0). Nothing reads it. In the demo recordings it was 1 at the end of every game frame except frames in which such an action ran: 0 at the first frame of levels with a camera action at the start, and 0 for the whole time a camera zone's action fires every frame (level 24 from frame 7 to 3602). |
| unk1A4 | mode | high | 0 while camera control mode is active (Activate/DeactivateCameraControlMode), 1 normal follow, 2 when a cameo moves the camera; MoveCam only runs for 1 |
| unk1A8 | blocked | medium | result of DoCameraCollision (camera jammed against a wall); stops the slope and follow-behind adjustments in MoveCam |
| unk1AC | angleLock | high | set by puzzle action 27, cleared every MoveCam; while set the camera keeps its heading to the focus (unk158) instead of auto-turning |
| unk1B0 | prevAngleLock | high | MoveCam copies unk1AC into it before clearing; puzzle action 27 uses it (!= 1) to take a fresh unk158 on the first frame |
| unk1B4 | sideOffsetEnabled | medium | gates the sideways target shift (unk17C etc.) in UpdateActorPositionDependencies; set to 1 each frame there, cleared by cameos, bonus rooms, present screen |
| unk1B8 | targetFrozen | medium | when non-zero UpdateActorPositionDependencies skips focus switching and target tracking; zeroed in RestartCamera, never set otherwise |
| unk1BC | prevRotX | high | func_80165230 saves camera.rot.x in it before recomputing; MainGameLoop compares the change with 0.5 to call SetupViewing |
| unk1C0 | prevRotY | high | saves camera.rot.y before it is recomputed in MoveCam/func_80165230; also used for the snow drift in overlay.c and the SetupViewing test |
| unk1C8 | fallTime | medium | set to 1 when Glover falls to his death (KillHand type 2, long fall in hand.c) and ++ in MoveCam while it raises and pulls in the camera; PlayerSpinCamera ignores C-left/right while set |
| unk1CC | overrideFlags | high | SetCameraOverride arg6 (puzzle action flags); &1 zeroes hand/ball vel in PlayerSpinCamera, &2 freezes bullets and vents, &4 freezes enemies in MainGameLoop |
| unk1D0 | exitingControlMode | medium | set when a button is pressed in camera control mode (rotAim[1] == 2); camera then returns to camOrigRotY and DeactivateCameraControlMode runs, clearing it |

### Collectable

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk08 | radiusSq | medium | set to 400.0 in CreateNewPickup and compared with the squared hand distance in UpdatePickups to collect |
| unk0D | active | high | 1 when created and when the respawn countdown ends, 0 when collected (timer = 10); gates scale easing and the pickup test in UpdatePickups |
| unk0E | duration | high | CreateNewPickup parameter named duration, passed as the second argument of ActivateCollectable on pickup |
| unk12 | respawnTime | high | CreateNewPickup parameter named time; when the collect animation ends the pickup is removed if 0, else timer = -unk12 (respawn countdown) |
| unkF8 | scaleTarget | high | 0.4 in CreateNewPickup; actor.scale eases toward it each frame in UpdatePickups |
| unkFC | glowTarget | medium | 101 in CreateNewPickup; Actor.unk26 eases toward it by up to 15 per frame and sparkle debris is spawned while > 0 (actor.unk26 meaning not confirmed) |
| unkFE | tag | high | set from the puzzle action in PuzzleExecuteAction and compared with the condition value in PuzzleEvaluateCondition to find a pickup by tag |
| glowTarget | alphaTarget | medium | Actor.alpha eases toward it in UpdatePickups |

### CollSphere

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk0C | prevPos | high | TransformAndDrawObject does obj->sphere->unk0C = obj->sphere->pos before storing the new world pos; DoOutroEffects uses unk0C - pos as the movement vector. |
| unk1C | targetRadius | high | UpdateActor eases radius toward unk1C each frame (step limited by phys->unk4C); code sets radius = unk1C = value for instant change or only unk1C to grow/shrink. |
| unk20 | platTag | medium | u8[2] of platform tags: TransformAndDrawObject moves the platform with tag [0] to the sphere's world pos and uses the platform with tag [1] for rotation; set to 1/2/4/5 for the Robot arms/guns, cleared when they are shot off. |

### ContinuousSample

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk08 | outVolume | medium | UpdateContinuousSample sets it to volume * distance falloff and applies it with MusHandleSetVolume; StopContinuousSample saves volume into it. |
| unk0C | baseVolume | medium | AddAmbientSfx sets it with unk08 to vol; the non-positional sample 223 scales its volume by it in UpdateAmbientSounds. |
| unk18 | range | high | Passed as the radius argument of PlaySampleRadius (distance subtracted before attenuation); AddAmbientSfxAtPoint arg named range. |
| radius | pitch | high | PlaySampleRadius takes it as the pitch argument; UpdateContinuousSample uses (value - 128) / 10 as the frequency offset (128 is normal) |

### Debris

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk020 | actorType | high | Aliases Actor.type at 0x20, set to 3 for every debris in InitDebris; func_8016225C treats types 3 and 4 as non-ball actors. |
| unk024 | lifeTimer | high | Set to the life in AddDebris, decremented each frame in UpdateDebris, KillDebris when it reaches 0. |
| unk040 | prevPos | high | Aliases Actor.unk40 which func_8015EE6C sets to pos before moving; UpdateWater compares pos and unk040 to detect crossing the water surface. |
| unk070 | inertia | medium | Per-frame velocity retention (0.6-1.0) loaded from DebrisDetails.unk0C along with info->unk18; Actor.unk70 is set from the global handcrawlinertia and multiplies vel in func_8015EE6C. |
| unk078 | rot | high | Aliases Actor.rot at 0x78; zeroed in AddDebris. |
| unk09C | rotQuat | medium | Aliases Actor.unk9C, the quaternion built from rot by GetQuaternionFromXYZRot; set to zeroQuat in InitDebris. |
| unkE4 | active | high | Slot is free when 0 (AddDebris searches for it, InitDebris/KillDebris clear it, UpdateDebris/UpdateWater skip when 0), set to 1 in AddDebris. |
| unk0E6 | life | high | Written together with unk024 from DebrisDetails.life + Random(lifeRange) and passed as the lifetime to InitSpriteAnimation; never decremented. |
| inertia | friction | high | aliases Actor.unk70, the per-frame velocity retention set from ActorPhysics.friction |
| unk022 | actorActive | high | aliases Actor.active at 0x22 (Debris is cast to Actor); AddDebris sets it to 1 next to Debris.active |
| unkCC | shadow | high | aliases Actor.shadow at 0xCC; ProcessShadows and CalculateShadows read the fake ball's altitude and floor through it |

### DebrisDetails

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk0C | inertia | medium | Copied into Debris.unk070 and info->unk18 in AddDebris (the velocity retention factor). |
| inertia | friction | high | copied into Debris.friction in AddDebris |

### DifficultyStats

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk01 | garibsPerLife | high | GainGarib gives an extra life when player.unk0C (garibs collected) % unk01 == 0; ROM data 40, 50, 60 |

### Dynamic

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk4080 | cameraView | high | guLookAt(Hilite) from the camera position in SetupViewing, multiplied into the projection (also used by the sprite and backdrop code); viewing is the origin one used by ClearViewing. |

### Footprint

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk34 | scaleSpeed | high | 0.2 in SetupFootprint or the scaleAimSpeed argument of CreateEnemyBlastring; DrawFootprints moves width toward length by this rate |
| unk88 | texScale | high | set from the texScale argument of SetupFootprint and used as the texture coordinate extent in DrawFootprint |
| unk8A | alphaStep | high | 5 in SetupFootprint (or alpha / time in the effect creators); added to or subtracted from alpha each frame until it reaches maxAlpha |

### GameInfo

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk02 | prevLevel | high | set to gameInfo.level before every level change in func_80123514; indexes levelInfo[].bit for best times and completion, copied to ballLeftBehind, tested == 8 or 9 in StartCameo |
| unk04 | prevWorld | high | set to gameInfo.world next to unk02 in func_80123514; selects the backdrop and time-up string of the finished world in EndOfWorldScreen |
| unk05 | worldComplete | high | 7 flags indexed by world: ContinueGame sets them from saveSlot.levels bit (world * 5 - 2), func_80123514 sets 1 on world exit, MaintainAction and func_801246AC read them for portals |
| unk11 | exitType | high | ExitLevel stores its type argument here; func_80123514 switches on it and ExitLevel(gameInfo.unk11) is re-issued after the exit timers, also indexes worldInfo bytes (16-20 pick the level) |
| unk12 | cameoActive | high | set TRUE/FALSE every frame in UpdateCameos depending on whether any cameo is playing; read by ReadController, UpdatePause, hand and enemy code |
| unk54 | exitFlags | high | bit 1 set by ExitLevel (leaving level, cleared in FreeLevel2/StartNewLevel), bit 2 set when arg2 is true (level completed: records time and saveSlot.levels) |
| unk55 | caveStage | medium | only used in DoCaveStuff: 0 idle, 1 ball lifted toward the cave platform, 2 placed; reset to 0 on frame 1 |
| unk5C | starZ | medium | current z of starOverlay in PrintFadeOverlay (clamped to 150 when fieldOfView > 70), eased toward unk5E in UpdateFade, set by FadeScreenIn/Out |
| unk5E | starZTarget | medium | target for unk5C: 600 or 0 set by FadeScreenOut/FadeScreenIn depending on the game mode |

### GameLight

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk38 | colourSpeed | high | SetupLight arg z (1.0); UpdateLightColours moves col[0..2] toward col[3..5] by this much per frame. |
| unk4C | origX | medium | Set to the initial angle x in SetupLight; copied back into target[0] when leaving the water in CheckCameraForWater. |
| unk50 | origY | medium | Initial angle y, restored into target[1] the same way. |
| unk54 | turnSpeed | high | Max angle step per frame toward target in UpdateLightDirections; set by ChangeLightDir (arg4) and 0.2 on parse. |
| unk9 | type | medium | 0 for level lights (SetupLight, ParseTokenGeneric), 2 for the two spell-flash lights ApplyAction adds; type 2 lights are counted in numSkipped and not loaded for actors with flag 0x10000 |

### GameSprite

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk18 | scaleX | high | PrintSprite multiplies it by 33 for the horizontal size; interpolated from SpriteAnim start/end scale. |
| unk1A | scaleY | high | PrintSprite vertical size, same as unk18. |
| unk20 | alpha | high | Passed as alpha to gDPSetPrimColor with r, g, b in PrintSprite. |
| unk21 | flags | high | Bit 1 translucent (draw in the second pass, list order), 2 flip x, 4 flip y, 0x10 animation running backwards (TileRectangle, AddSprite, AnimateSprite). |

### Garib

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk08 | velY | high | vertical velocity: unk08 = max(unk08 - gravity, -15) added to sprite.pos.y, bounced with * -0.7, 12.0 when dropped by KillEnemy |
| unk0F | mode | medium | read from the level file; 0 = altitude integrated from velocity in UpdateGaribs, 1 = altitude re-queried each frame in CalculateShadows, 10 = being pulled by the frog tongue in MaintainAction |
| unk10 | scale | high | eased toward unk14 by half each frame in UpdateGaribs and used as sprite size (* 64) and shadow size |
| unk14 | scaleTarget | high | target the scale unk10 eases toward (1.0 everywhere it is set) |

### GaribGroup

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unkD0 | suspended | medium | set to 1 at parse time and the garibs only fall (gravity and bounce in UpdateGaribs) when 0; DropGaribs sets it to 0 |
| unkD1 | startActive | high | read from the level file and copied to each new garib's active in ParseTokenCollectable (KillEnemy sets 1) |

### HandControl

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk14 | lateBounceQueued | medium | Only HAND_BOUNCE sets it: A or B pressed (outside camera control) while the bounce animation has finished, together with bounceQueued. ApplyAction clears it with every action change. Nothing reads it. In the demo recordings (all 44 levels, every frame) it was 1 in 14 levels, always in HAND_BOUNCE, and 0 again at the next action change (to HAND_AIRBOURNEWHACK, HAND_RELEASE, HAND_JUMP, HAND_BOUNCE2CUP or HAND_LOCATE). |
| unk1A | actionTime | high | Frames since the last action change: ApplyAction clears it (its saved copy there is the PC port's lastActionTime); watched in the emulator. |
| unk18 | prevAction | high | The action of the previous update, copied from hand.state->mode at the end of each update; watched in the emulator. |
| unk28 | idleTime | high | Counts frames without input; watched in the emulator. |
| unk00 | savedFaceNum | high | facenum (global) saved before CheckCollisions on the probe and restored after |
| unk08 | actionLock | high | no new action starts while nonzero; counts down, 1000+N waits for animation N, 2000 waits for landing |
| unk0A | actionDelay | medium | set to 1 or 5 by an action; the action's effect (whack, throw, hop) happens when it counts down to 0 |
| unk1C | gait | high | walk animation band: -1 still, 0 slow (anim 24), 1 walk (anim 12), 2 run (anim 58), 3 rolling the ball |
| unk1D | prevGait | high | copy of gait from the previous update |
| unk1E | attachMode | high | 0 free, 3 held by an enemy or snowball (control disabled), 4 riding an enemy, 5 level 40 |
| unk24 | noControlTimer | high | DisableControl(x) raises it to x; counts down; ReadController clears input while nonzero |
| unk26 | deathTimer | high | KillHand sets 80; counts down; at 0 the player respawns |
| unk2A | bounceCount | medium | jumps off the ball in a row; raises the jump and the pitch of sample 19 |
| unk2B | releaseTimer | high | ReleaseBall sets 7; while nonzero IsHandHoldingBall is false |
| unk2C | platformBounceCount | high | bounces on a bouncy platform in a row (up to 4), adds to the jump |
| unk2E | walkTime | medium | updates spent in the slow gaits; reset when running |
| unk30 | fallTime | high | updates spent falling fast; 9 and 17 give fall damage, 16 starts the falling animation |
| unk34 | doubleJumped | high | set when the mid-air second jump is used; cleared on landing |
| unk38 | edgeProbe | high | actor kept 5 units in front of the hand; its altitude detects a drop ahead (edge action) |
| unk11C | frontProbe | high | actor placed in front of the hand (local frontprobey) and run through CheckCollisions |
| unk4AC | edgeAltitude | medium | altitude found under edgeProbe (local tempAlt) |
| unk4BC | wallPushTime | medium | updates pushing into a steep plane before action 29 starts |
| unk4C8 | lastGroundPos | high | position on the last ground (relative to its platform if any); HurtHand knocks back from it |
| unk4D4 | lastGroundPlatform | high | platform under lastGroundPos |
| unk4D8 | edgePlatform | medium | platform found by the edge probe; kept through actions 25 and 26 |
| unk4EC | suctionPlane | high | plane the sticky power-up holds on to (local oldSuctionPlane); gravity points along it |
| unk4F0 | suctionTime | high | updates on the same suction plane; drives the slerp to the plane |
| unk4F4 | suctionLostTime | medium | updates without a plane while sticky; at 6 the suction plane is dropped |
| unk4F8 | attachedTo | high | actor the hand is attached to (enemy holding or carrying it, snowball) |
| unk4FC | attachOffset | high | offset from attachedTo, rotated with it |
| unk508 | attachTag | medium | -1 uses attachOffset; otherwise the tag of the attach point |
| unk50A | dropTimer | high | DropHand sets it; at 0 hand.unk2C is cleared |
| unk510 | checkpoint | medium | platform RestartPlayer returns the player to |
| unk518 | morphStep | medium | step through the tongueRad table that scales the hand when a power-up starts or ends |
| unk51C | morphReverse | medium | 0 shrinks, 1 grows back (DeactivateCollectable) |
| unk53A | touchedActor | medium | set by CheckActorCollisions, cleared each MainGameLoop |
| unk564 | flags | medium | bit 1: the hand is inside a bubble (MaintainBulletBubble) |
| unk57A | glovePrintTimer | high | set to 100 in MoveHand, counts down; AddGlovePrint fades with it |
| unk20 | prevOnGround | high | copy of hand.obj->onGround made at the end of MoveHand |
| unk534 | groundStateTime | medium | updates since hand.obj->onGround last changed (compared with prevOnGround in MoveHand) |
| unk0C | spellDelay | medium | set to 5 or 10 frames when the hand jumps off, whacks or releases the ball, counted down in MaintainAction; ConditionsMet(HAND_CASTSPELL) fails while it is nonzero |
| unk10 | bounceQueued | medium | set when A or B is tapped for HAND_BOUNCE; MaintainAction bounces the ball when the button is released near it and clears the flag |
| unk4B0 | inputAccel | high | ProcessInput stores the velocity change the stick made this frame (vel after minus vel before); zero when there is no input |
| unk4C4 | climbPlatform | medium | the platform the hand stood on when it started to climb down or snatch; UpdateActorCollisionAndGravity keeps hand.obj->platform on it, and MoveHand clears it outside HAND_SNATCH, HAND_CLIMBDOWN and HAND_CLIMBUP |
| unk4DC | clingPlanes | medium | the hand's four contact planes copied during the first 12 frames of HAND_CLING; the hand drops when they change; HAND_CLIMBDOWN drops when clingPlanes[0] is too steep |
| unk50C | joinTimer | medium | blocks ConditionsMet(HAND_JOINED) while nonzero (set after a teleport, LoseBall and a bubble bullet) and blends the ball into the hand over joinVal frames |
| unk514 | rotorSpeed | high | HAND_ROTOR: A raises it towards gravity, it decays by 0.95 each frame, lifts hand.vel.y, sets the rotor sample's volume and pitch and spins orbiter3 |
| unk524 | fakeballDelay | medium | counted down in MoveHand; AddFakeball waits while it is nonzero. Nothing in the code sets it |
| unk538 | steepSlide | medium | CheckPlaneForSliding sets it when the slope's normal y is at most handSlide2 (0.5, a 60 degree slope); the skid animation is skipped while it is set |
| unk55E | castSpellTime | high | the duration of the pick-up whose spell is being cast; FireBulletFromActor copies it into the spell bullet's spellTime |
| unk568 | smokeTimer | high | set to 40 when certain bullets hit the hand; while it counts down, MaintainAction fires SMOKE_BULLETs from the hand |
| unk56C | locateGaribs | high | 1 when HAND_LOCATE points at the nearest garib (the locate cheat, R button), 0 when it points at the ball (B button) |
| unk578 | gentleSlide | medium | set by CheckPlaneForSliding when the slope's normal y is above handSlide2 divided by the terrain's handSlideScale |

### LightList

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk5A | numSkipped | medium | Counts lights with unk9 == 2 in UpdateLights and is subtracted from count in gSPNumLights for actors with flags & 0x10000 (those lights are skipped). |

### LightSet

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk00 | ambient | medium | LightSet is BackgroundLighting: RotateLight callers pass &backgroundLightList.ambient, and the 8 bytes are the Ambient light before the LightList. |

### Mesh

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk0C | faceColours | high | WriteObjectDisplayListFlat reads MeshColour *colours = mesh->unk0C and indexes it by face (colours[face]); the Gouraud path uses mesh->colours by vertex. |
| unk1C | faceFlags (u8 *) | medium | one byte per face (RestoreObjectPointers relocates it; the emulator shows numFaces bytes). Only meshes with moving textures have it: water, mist, mud, lava, goo floors, conveyors, a monitor and a window. Values 0, 1, 3, 6, 7; they do not follow the mesh border or the UV span, and no N64 code reads them |

### NMEBrain

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk48 | inConditional | high | set to TRUE in NMEAnalyseConditionalList when a conditional instruction interrupts the normal list (saving the normal index in unk171), cleared in NMEProcessInstruction when returning to the normal list; cleared by NMEActionStartAttack and RestartEnemy. |
| unk4C | normalList | high | filled by NMEAddNormalInstruction (landscape token 0x9A); unk16C points at it as the default instruction list in RestartEnemy and NMEEndAttack. |
| unkC4 | conditionalList | high | filled by NMEAddConditionalInstruction (token 0x9B) and scanned with NMEConditionsMet in NMEAnalyseConditionalList. |
| unkEC | attackList | high | filled by NMEAddAttackInstruction (token 0x9C); NMEActionStartAttack switches unk16C to it. |
| unk164 | cooldownTimer | medium | set by instruction 14 (params.flags) and NMEEndAttack, counts toward 0 each frame in NMEProcessInstruction; NMEConditionsMet refuses a start-attack instruction (15) while >0, conditionals are skipped while <0; bosses set 30000 to block attacks. |
| unk166 | normalCount | high | ++ in NMEAddNormalInstruction after storing into unk4C[unk166], error 'too many instructions' at >30; NMEProcessActorScript returns if 0. |
| unk168 | conditionalCount | high | ++ in NMEAddConditionalInstruction after storing into unkC4, limit 10; loop bound in NMEAnalyseConditionalList. |
| unk16A | attackCount | high | ++ in NMEAddAttackInstruction after storing into unkEC, limit 30; NMEActionStartAttack prints 'no attack instructions present' if <=0. |
| unk16C | curList | high | points at unk4C, unkC4 or unkEC (assigned in RestartEnemy, NMEEndAttack, NMEActionStartAttack, NMEAnalyseConditionalList) and is indexed by unk170 in NMEGetNewInstruction. |
| unk170 | instrIndex | high | index into curList of the running instruction, set in NMEGetNewInstruction, incremented (+1) to advance, passed back into NMEGetNewInstruction. |
| unk171 | returnIndex | high | index in the normal list to resume at: set to unk170 (or unk170+1 for an attack) when a conditional/attack interrupts, copied back to unk170 in NMEProcessInstruction and NMEEndAttack. |
| unk173 | loopCount | high | instruction 21 sets it from params.flags, instruction 20 decrements it and jumps to the target index while non-zero (a repeat-N-times loop). |
| unk174 | attackTime | high | 0 when not attacking, set to 1 in NMEActionStartAttack, ++ every frame in NMEProcessInstruction, cleared in NMEEndAttack; bosses compare it with 2 and 100 and NMEProcessActorScript only checks conditionals when 0. |
| unk178 | instrTime | high | zeroed in NMEGetNewInstruction, ++ every frame in NMEProcessInstruction; instruction handlers test ==0 to run their start-up code and compare with frame limits. |
| unk188 | scratch | medium | per-enemy-type general purpose int: attack/hit counter, timer or phase in Chuck, Bovva, Cylon, robot, Selwyn bosses, HubChicken and others (set to state->index in NMEProcessFroggy). |
| unk18C | scratchF | medium | per-enemy-type general purpose float: countdown in Robot/EvilRobot/func_801A1558, cheat-code letter index in HubChicken; only NMEActionCircle uses it as the orbit angle. |
| unk190 | tiltAngle | medium | only used in NMEActionCircle: always increases with the orbit, used as rotAim.x pitch angle (a + PI) when the circle runs in the opposite direction (flag 0x400). |
| unk194 | delayTimer | high | NMEProcessActorScript skips the script while >0 (decrements it) and while <0 until the animation ends (anim->unk04); set to 30 after NMEOverstepEdge, 40 in EnemyDropBall, 30000 to freeze. |
| unk196 | instrTimer | high | set from instr.unk02 (the instruction's time) in NMEGetNewInstruction, counts down in NMEProcessInstruction and ends the instruction at 0; -1 means wait for the animation to finish. |
| unk198 | flags | high | bit set initialised from enemyStats[type][0] in InitEnemy, bits tested/set across enemy.c, collision.c and hand.c (8 hit this frame and cleared each frame, 0x10 frogged, 0x100 disabled, 0x400 held, 4 no edge check from token 0xBA). |
| unk19C | tag | high | written from the tag argument in RestartEnemy and compared with the puzzle action/condition tag in PuzzleExecuteAction and PuzzleEvaluateCondition. |
| unk19E | bobPeriod | high | set per enemy type in InitEnemy (5,6,10,20; EvilRobot sets 8) and used as the period argument of SineWave(period, 0, frameCount) added to vel.y in NMEProcessInstruction and NMEProcessDroid. |
| unk1A0 | bobAmount | medium | InitEnemy sets 0.05-0.5 alongside unk19E; multiplies the SineWave result added to vel.y (hover bob amplitude). |
| unk1A4 | fireState | medium | u8 array read by FireBulletFromActor as 'mode' [0] (0 single gun, 1 cycle guns, 2 fire all shots) and 'gun' [1] (current gun index, wrapped at numShots); [2] and [3] unused. |
| unk1AC | area | medium | Box loaded from landscape token 0xA1 (size.x=-1 means none) and used as the enemy's allowed area in NMEConditionsMet (is player/enemy inside), NMEActionFollowPlayer and NMEProcessFrog (return to its centre). |

### NMEInstruction

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk02 | time | high | NMEInitInstruction reads it into a local named time; copied to brain.unk196 as the instruction's duration in frames. |
| unk2C | condition | high | switch value in NMEConditionsMet (0-22: ball/hand distance, facing, frame modulus, holding ball, random chance, inside area...). |
| unk30 | condMin | medium | used as the lower bound of the distance/rotation range in NMEConditionsMet cases 0-5 and 10, and as the single threshold angle/modulus/chance in cases 6-9 and 22. |
| unk3C | condMax | medium | upper bound (exclusive) of the distance range in NMEConditionsMet cases 0-5 and of rot.x in case 10. |

### Object

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk0D | alpha | high | Declared u8[1], used as obj->unk0D[0]: multiplied by xluOverride/100 to give xluFact, passed as alpha in gDPSetPrimColor, scaled by Actor.unk26/100 for sprite alpha. |
| unk18 | displayList | high | TransformAndDrawObject does gSPDisplayList(glistp++, obj->unk18) when non-NULL (instead of building a mesh list); RestoreObjectPointers relocates it with RestoreDrawList. |

### Overlay

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk18 | scaleX | high | PrintOverlay: x scale = unk18 / 1024.0f passed to spScale; 0x400 is 1.0. |
| unk1C | scaleY | high | PrintOverlay: y scale = unk1C / 1024.0f. |

### PathPoint

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk10 | sampleVolume | high | Token 0xC2 value 2; passed as volume to PlaySample(unk16, pos, unk10, unk14) in MovePlatform when the point is reached. |
| unk14 | samplePitch | high | Token 0xC2 value 3; 4th argument (pitch) of PlaySample in MovePlatform. |
| unk16 | sampleId | high | Token 0xC2 value 1; played on arrival at the point if nonzero. |

### Platform

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk18 | exitType | high | Token 0xA8 value; func_801246AC passes it to ExitLevel as the exit type and indexes gameInfo.unk05 with it. |
| unk54 | restartGate | high | Token 0x63 allocates it ('Plat_Rest'), locals are named restartGate in CheckRestartGate and RestartPlayer. |
| unk58 | blur | high | Allocated as PlatformBlur ('Blur') by token 0x70, passed to AddBlur/StartBlurFX/IncrementBlurFX, cast to PlatformBlur *; declared void *. |
| unk5C | conveyorVel | medium | Tokens 0x68/0x9E set flags /= 0x200 and read it; AddConveyorSpeed uses it as the conveyor velocity (or target offset when unk68 != 0); texture slides by it. |
| unk68 | pullSpeed | medium | Token 0x9E extra float; if nonzero AddConveyorSpeed pulls actors toward platform pos + unk5C at up to this speed instead of using unk5C as velocity. |
| unk74 | bounceAxis | high | Token 0x6E bits 1/2/4 set unk74[0..2]; func_80158F84 multiplies bounce velocity component j by platform->bounce when unk74[j] is set. |
| unk78 | pathStopFlags | medium | Puzzle actions set 0/1/2 from flags; MovePlatform sets 0x800 (halted) on arrival if bit 2 or (point 0 and bit 1); CheckHandPlatform sets bit 2. |
| unk7A | spinStopFlags | medium | Same pattern as unk78 for spin: SpinPlatform stops (unk00 = 0) at a pause stop if bit 2 or (pauseIndex 0 and bit 1); set by puzzle action types 1/4. |
| unk7C | orbitStopFlags | medium | Same pattern for orbits: OrbitPlatform stops (speed = 0) at a pause stop; set by puzzle action type 2. |

### PlatformBlur

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk578 | innerScale | medium | Token 0x70 value; RotatePlatform scales the blade-tip vector by it to get the blur's second point (p2) on each frame. |

### PlatformCatapult

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk00 | holdPos | medium | Vec3 rotated by platform yaw plus platform pos; the hand/ball is pulled to it (0.2 lerp) and the catapult triggers when within 10 units. |
| unk0C | launchSpeed | high | Passed as speed to AccelerateActor2D / AccelerateActorFromActor at launch (max 100000). |
| unk10 | launchVelY | high | Assigned to hand.vel.y / ball.vel.y at launch in func_801735E8. |
| unk14 | active | high | Set to 1 at parse, set by puzzle action (flag 0x20), must be nonzero for the catapult to catch the hand. |
| unk18 | catchTimer | medium | Set to 10 on activation, counts down while the hand is pulled to holdPos; at 0 the arm starts moving. |
| unk1A | launchTimer | medium | Set to unk20 when the arm starts, counts down to 0 then launch; -1 after launch, counts further down for the reset; actor.c tests > 0 for 'held'. |
| unk1C | armRotVel | medium | Becomes motion->unk50[0] = -unk1C (initial arm rotation speed) when the arm starts. |
| unk16 | (unnamed) | none | Only written, to 0 when the catapult catches the hand (`CheckHandCatapult`). In a catapult built in memory (`tools/emu/catapult.py`) it stayed 0 through two full cycles. Nothing reads it. |
| unk20 | launchDelay | medium | Token 0x69 first value; copied into unk1A as the wind-up count and used as unk20+11 frames of disabled control. |

### PlatformDestruct

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk00 | numDebris | high | UpdatePlatforms loops x < unk00 calling AddDebris for each piece; token 0x65 reads it first. |
| unk08 | sampleVolume | high | Token 0xC8 value 2; volume of PlaySample(unk0E, pos, unk08, unk0C) when destroyed. |
| unk0C | samplePitch | high | Token 0xC8 value 3; pitch argument of the same PlaySample. |
| unk0E | sampleId | high | Token 0xC8 value 1; checked != 0 then played in UpdatePlatforms. |

### PlatformLink

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk10 | spinToMoveScale | high | RotatePlatform: tagged platforms get vel = (link->vel + their unk34) * rotVel * unk10; token 0x58 reads tag then this scale. |
| unk14 | spinToMoveTag | high | Tag passed to FindTaggedPlatform together with unk10 in RotatePlatform; token 0x58. |
| unk18 | spinToSpinScale | high | RotatePlatform: tagged platform's motion->unk00 (spin speed) = this platform's rotVel * unk18; token 0x5A. |
| unk1C | spinToSpinTag | high | Tag passed to FindTaggedPlatform together with unk18 in RotatePlatform; token 0x5A. |
| unk20 | moveToSpinScale | high | MovePlatform: tagged platform's motion->unk00 = (vel . unk28) * unk20; token 0x59. |
| unk24 | moveToSpinTag | high | Tag passed to FindTaggedPlatform in MovePlatform together with unk20; token 0x59. |
| unk28 | moveToSpinDir | high | Vec3 dotted with the platform's vel in MovePlatform to get the spin speed given to linked platforms; token 0x59 reads 12 bytes. |
| unk34 | velOffset | medium | Token 0x74; added to the linked platform's actor.vel after it is set from the driver's link->vel (RotatePlatform, CheckHandPlatform). |

### PlatformMotion

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk00 | speed | high | Spin speed: SpinPlatform copies it into unk50[i] each frame, set by SPIN tokens (0x7F), puzzle action 1 and DoCarnivalBonus; same slot as PlatformOrbit.speed. |
| unk08 | snapCount | medium | Token 0x78 sets it; RotatePlatform rounds the angle to the nearest multiple of TWOPI/unk08 (needs >= 2) while seeking the target. |
| unk0D | pauseIndex | high | Index of the next pause stop: set by SPINPAUSE (0x7C), advanced (unk0D+1)%pauseType in SpinPlatform, stop angle = unk0D*TWOPI/pauseType+unk30, set by puzzle action 4. |
| unk10 | limitFlags | medium | Bit 1/2 choose whether the positive/negative swing limit is the hard limit unk28 (else 4/5 or 3/5 of unk24) in RotatePlatform; /= 3 for ropes and springs. |
| unk18 | pendulumLength | medium | RotatePlatform: unk50[i] -= angle/unk18 (restoring pull to zero); set by token 0x70, which also builds the swing blur. |
| unk1C | accel | medium | Called acc in RotatePlatform: acceleration toward the target angle unk5C (look-at/spring tokens 0x5E 0x60 0x61 0x80 0x81), spring strength in the 0x9F quaternion mode. |
| unk20 | idleCount | medium | Reset to 0 in func_80158F84 when an actor pushes the platform, ++ per axis in RotatePlatform, UpdateRopeBridges skips segments with unk20 < 2. |
| unk24 | tipAngle | medium | Token 0x81 value; if distance to target exceeds it the platform is flung (vel*1.3, accel cleared) and travel limits are 4/5 and 3/5 of it until the unk28 limit is reached. |
| unk28 | rotLimit | medium | Maximum swing around unk30: RotatePlatform clamps rot to unk30 +/- unk28; rope bridge sets it from rope[3], DoPiratesBoss writes PI; divisor in the 0x9F mode. |
| unk2C | maxRotVel | high | RotatePlatform clamps unk50[i] to +/-unk2C when flags & 0x10; set from rope[5] and token 0x80 value d. |
| unk30 | baseRot | high | Set equal to the initial rot[axis] by every spin token; SpinPlatform pause/flip angles are measured from it and RotatePlatform swing limits are centred on it. |
| unk34 | bankScale | high | GOFORWARDS (0x5F): target roll unk5C[2] = yaw change * unk34 in RotatePlatform, roll acceleration unk34*0.002; unk68[2] only set if nonzero. |
| unk50 | rotVel | high | Per-axis angular velocity: added to rot[i] every frame in RotatePlatform, set to unk00 by SpinPlatform, passed as 'spin' in collision. |
| unk5C | targetRot | high | Per-axis target angle that rot[i] is driven toward when unk68[i]==1; look-at tokens write atan2 results into unk5C[1]. |
| unk68 | seekTarget | high | Per-axis flag set to 1 by look-at/spring/GOFORWARDS tokens; RotatePlatform only accelerates rot[i] toward unk5C[i] when it is 1. |
| unk6C | damping | high | Per-frame velocity multiplier: unk50[i] *= unk6C in RotatePlatform, spinSpeed *= unk6C in SpinPlatform; init 1.0 (none), 0.97/0.99 set by DoCarnivalBoss. |
| unk70 | inertia | medium | Divisor of the torque an actor applies in func_80158F84 (mass*...)/(unk70*100000) and of the 0x9F spin kick; set from tokens (sval[1]). |
| unk90 | stopSampleVolume | high | Token 0xC5 value 2; passed as volume to PlaySample(unk96, pos, unk90, unk94) when the spin stops or reaches a limit. |
| unk94 | stopSamplePitch | high | Token 0xC5 value 3; passed as pitch (4th arg) to PlaySample with unk96 and unk90. |
| unk96 | stopSampleId | high | Token 0xC5 value 1; sample id played in RotatePlatform/SpinPlatform when the spin stops (checked != 0 before PlaySample). |

### PlatformOrbit

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk0D | pauseIndex | high | Same use as PlatformMotion.unk0D: set by ORBITPAUSE token 0x76, advanced (unk0D+1)%pauseType, stop angle unk0D*TWOPI/pauseType. |
| unk44 | stopSampleVolume | high | Token 0xC6 value 2; volume argument of PlaySample(unk4A, pos, unk44, unk48) when the orbit stops. |
| unk48 | stopSamplePitch | high | Token 0xC6 value 3; pitch argument of the same PlaySample. |
| unk4A | stopSampleId | high | Token 0xC6 value 1; played in OrbitPlatform when the continuous sample fades out. |

### PlatformPath

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk58 | accel | high | MovePlatform adds unk58 along the direction to the next point and uses it as deceleration in the braking distance; token 0x6D, DoAtlantisBonus sets 0.001/0.015. |

### PlatformPush

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk04 | mass | high | func_80158F84 scales actor push by actor->phys->mass / push->unk04; DoAtlantisBonus writes atbonusmass1. |

### PlatformRestartGate

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk02 | restartTimer | high | Set to 65 on gate init and player death, RestartPlayer counts it down to 0 with stages at 35/36/10/11; ball.c checks <= 0. |
| unk04 | cameraPos | high | camera.pos is saved here when the gate is touched (CheckRestartGate) and restored in RestartPlayer; level start computes it from gate pos. |
| unk10 | shadowAlt | medium | UpdateRestartGateShadows uses it as the altitude for the shadow width when unk17 is 0 (never written; always 0). |
| unk14 | untouched | high | Set to 1 at init (0 for the first gate), cleared in CheckRestartGate when the ball reaches the gate; gate-select menu in game.c lists gates with unk14 == 0. |
| unk17 | groundShadow | medium | Token 0x63 value; if set UpdateRestartGateShadows places the shadow from Altitude() under the gate. |
| unk18 | targetScale | high | CheckRestartGate eases actor.scale toward unk18 (1.0 untouched, 0 after touch, hand scale when respawning). |
| unk1C | yaw | high | Token 0x63 float (hardcoded angles for hub gates); RestartPlayer sets hand.rot.y to it and RotateVector2D uses it for the camera offset. |

### PlatformTeleport

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk0C | flags | high | Token 0x89 value; bit tests in func_8016225C (1 hand, 2 ball, 4 others, 8 hand mode 3) and func_8015F098 (0x10 clear vel, 0x20 land on target, 0x40 swap hand and ball). |
| unk0E | active | high | Set to 1 at parse, checked == 1 in func_8016225C, set by puzzle action (flag 0x10). |
| unk10 | shrinkTime | high | state->unk03 starts at unk10 and counts down while scale = unk03*0.05/unk10 (shrinking) until the teleport happens. |
| unk12 | growTime | high | After teleporting state->unk03 = -unk12 and scale = (unk12+unk03)*0.05/unk12 grows back to 0.05. |

### Player

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk08 | score | high | passed to UpdateHiscoreTable and AddToScore; saved as saveSlot.score |
| unk0C | garibs | high | ++ in GainGarib; compared with the level's garib total in CheckForLevelCompleted |
| unk14 | health | high | 3 at the start, -= damage in HurtHand, death at 0; GainGarib gives back hearts |
| unk18 | levelFrames | high | ++ every update, 0 in StartNewLevel; divided by 16.67 or 20 for the time in seconds (the global `levelTime`) |
| unk5C | hurtTimer | high | HurtHand sets 5 and ignores hits while it is nonzero; MoveHand counts it down, bit 2 flashes the hand |
| unk60 | spell | high | pick-up spell held (a Pickup); NUMPICKUPS (15) means none; needed to cast (HAND_CASTSPELL) |
| unk64 | castSpell | medium | spell being cast: copied from spell when casting starts, switched on in ApplyAction, reset to 15 |
| unk68 | castSpellType | medium | copied with castSpell and passed to the bullet as its spellType (the sprite colour) |

### PuzzleAction

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk0E | tag | high | passed to FindTaggedVent/Buzzer/Platform/Wind/Water/AmbientSound, DropGaribs, enemy brain.unk19C test, AddAmbientSfxAtPoint tag arg; in actions 11-13 a puzzleVal index, 14/36 a pickup/cameo type |
| unk10 | value | high | the main float argument: puzzleVal source, speed, active flag, score, gravity, camera distance/height, SetCameraOverride time, DropGaribs count |

### PuzzleCamAction

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk00 | pos | medium | position for actions 28/31/32 (camera destination), ambient sound pos (39), texture crc u32 (40); actions 25/26/29 only use .x as a scalar |
| unk0C | speed | medium | only read by action 29, where it becomes cameraInfo.unk154 (orbit speed), negated and with unk1B4 cleared when negative |

### PuzzleCamCond

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk00 | pos | high | cast to Box (pos, size) for hand/ball/camera-in-box tests, sphere centre for distance tests, cylinder centre (x, z) for case 32 |
| unk0C | size | medium | Box size when the cam is cast to Box (cond 26/28/30); in cond 32 .x/.y are the upper and lower y limits |
| unk18 | radius | high | distance squared is compared with unk18 * unk18 in sphere conds 27/29/31; parsed as a lone f32 after the centre for ids 36/38/40 |

### PuzzleCond

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk9 | tag | high | platform tag via FindTaggedPlatform in the default conds, enemy brain.unk19C tag, garib group/pickup tag; for conds 18-21 and 35 it is a puzzleVal/powerup index |
| unkA | value | medium | second parameter: literal or puzzleVal index compared in conds 18-21, otherwise on/off flag, mode (cond 34), path point (-1 = any) or axis index (22/23) |

### PuzzleCondList

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk14 | requiredCount | high | token AND sets 0 (all must hold), OR sets 1, ANY n sets n; PuzzleEvaluateConditionList returns TRUE once that many conds are true |
| unk16 | timesLeft | high | -1 from InitConditionList (unlimited), token NUMTIMES overwrites it, CheckPuzzles only fires while it is non-zero and decrements it on each firing |

### RopeBridge

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk08 | stiffness | medium | rope[0] from token 0x8D; UpdateRopeBridges scales neighbouring segments' angle differences by it, 0 ends the bridge loop. |
| unk0C | dampEnds | medium | ParseTokenBridge sets it when the level gives no rotation-limit growth (inFloat[6] == 0); UpdateRopeBridges then scales the end segments' rotation velocity by 0.075 |

### SaveSlot

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk08 | allGaribs | high | bit per level (levelInfo[].bit) set in CheckForLevelCompleted when player.unk0C >= numGaribs (local named allGaribs); read by BonusOpen, DoWayRoom and the slot select percentage |
| unk10 | lives | high | stores player.lives in UpdateSaveSlot and restores it in ContinueGame (minimum 3); StartNewGame sets difficultyStats[].lives |
| unk11 | difficultyLevel | high | 0 EASY and 1 NORMAL chosen on the SELECT DIFFICULTY screen, 2 for time trial; indexes difficultyStats[], sets maxPlasters 4/3/2 in ResetBall, TIME_TRIAL macro tests == 2 |
| unk12 | hubTarget | medium | ++ when a world is first completed (exit type 3), set to 1 on first cave visit; DoCaveStuff walks the ball to platform unk12 while unk13 < unk12 |
| unk13 | hubLevel | high | gameInfo.level = saveSlot.unk13 (HUB 1-8 = 0-7) in ContinueGame and on return from a world; ++ in DoCaveStuff, == 7 is the last hub |

### ScreenDebris

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk18 | scaleX | high | Overlay layout: PrintOverlay divides by 1024 for spScale x; set to 0x400 (1.0) and to unk84 each frame in UpdateScreenDebris. |
| unk1C | scaleY | high | Same as unk18, spScale y (/1024). |
| unk24 | bitmaps | high | Aliases Overlay.bitmaps, allocated by SetupOverlay((Overlay *)debris) and freed in DestroyScreenDebris. |
| unk80 | curAlpha | medium | alpha = unk80 each frame; unk80 steps by 1 toward unk82 after the fade delay. |
| unk82 | targetAlpha | medium | unk80 moves toward it (star 0, tick 255, rain/snow weather colour); never written during update except for snow. |
| unk84 | curScale | high | unk18 = unk1C = unk84 each frame; eases toward unk86 by up to 100 per frame. |
| unk86 | targetScale | high | unk84 eases toward it; set to 0x400/0x2000/0x300/0x200 at creation and by ProcessRain. |
| unk8A | lifeTimer | medium | Set to 20/60/2 at creation, decremented each frame unless flags & 8, debris deactivated at 0; snow also uses it as sway phase. |
| unk8C | age | medium | Zeroed at creation, ++ each frame in UpdateScreenDebris, never read. |
| unk8D | flags | high | Bit tests in UpdateScreenDebris: 1 friction on vel, 2 gravity, 4 pulsing scale, 8 no life countdown, 0x10 friction also when moving down. |
| unk8E | fadeTimer | medium | 15 at creation, counts down to 0 before the alpha fade (unk80 toward unk82) starts. |
| unk8F | phase | high | Random(50) at creation, ++ each frame, fed to __sinf for the pulsing scale (flags & 4). |

### ScreenSpot

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk0C | func | high | the function SlotSelDone and DoneTitleWait call for the selected spot (`me->spots[me->currentSpot].func()`) |

### SpriteAnim

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk04 | frameDelay | medium | AnimateSprite adds it to anim.unk13 after each frame advance and skips 32 per game frame until unk13 <= 0 (0 = every frame, 32 = every second frame). |
| unk06 | startAlpha | high | InitSpriteAnimation copies it to GameSprite.unk20 (the prim alpha); AnimateSprite interpolates unk20 from unk06 to unk07 over the life. |
| unk07 | endAlpha | high | Target of the alpha interpolation in AnimateSprite. |
| unk08 | startScale | high | InitSpriteAnimation sets GameSprite.unk18/unk1A (scale) to it; AnimateSprite interpolates the scale from unk08 to unk0A. |
| unk0A | endScale | high | Scale target in AnimateSprite (type 6 reverse animations start at it). |
| unk0C | flags | high | InitSpriteAnimation copies it to GameSprite.unk21, the sprite flag byte. |
| unk0E | life | high | Frame count before the sprite disappears (0 = never): AnimateSprite ends it when anim.unk10 counts down to 0; set to the Debris life. |

### SpriteAnimState

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk10 | lifeTimer | high | Loaded from def.unk0E and decremented each AnimateSprite; the sprite is removed at 0. |
| unk13 | delayTimer | medium | Counted down by 32 per frame while > 0, reloaded with def.unk04 when the frame advances. |

### Terrain

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk00 | slip | medium | Values >1 (ice-like 3/6/7) divide the ground friction in func_8015EE6C, scale ProcessInput max speed, and allow the hand slide action (ConditionsMet case 7, slip > 1); <1 sticks. |
| unk04 | bounce | high | func_80158F84 multiplies the bounce velocity (and the bounce accumulation) by terrain[face->terrain].unk04; values 0.5-1.0. |
| unk10 | gravityScale | medium | UpdateActorCollisionAndGravity multiplies actorGravity by it when pulling the actor onto the plane; < 1.0 marks slide terrain (CheckPlaneForSliding returns early, onGround stays set on steep planes); table has 0.2 and 0.9. |
| unk14 | shatterSpeed | medium | func_80158F84 shatters the crystal ball (ballControl.type == 6) when its impact speed along the plane exceeds terrain.unk14 (10-17). |
| unk08 | slideSpeedScale | medium | UpdateActor divides minSpeed by it to get the speed below which a resting actor slides down a slope; equals slip on the icy terrains (3, 6, 7), else about 1 |
| unk0C | slideSlopeScale | medium | divides (1 - slideLimit) in the steep-slope tests of UpdateActor, UpdateActorCollisionAndGravity and ConditionsMet; it is slip squared for most terrains (0.25 for slip 0.5, 36 for slip 6) |
| unk18 | handSlideScale | medium | CheckPlaneForSliding divides the hand's slide limits (0.25 and handSlide2) by it; 1.5, 3 and 4 on the icy terrains |

### Text

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk5C | oneShot | medium | NEW_TEXT arg8 (1 for transient text); ShowText draws it once then sets 2, ClearText frees entries with 2. |

### Texture

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk10 | maskS | high | Passed in the masks slot of gDPLoadTextureBlock (log2 width, 3 for the 8x8 whiteTex) in LoadTexture and ProcessShadows. |
| unk12 | maskT | high | Passed in the maskt slot of gDPLoadTextureBlock (log2 height). |

### TextureBank

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk00 | textures | high | Points at the first Texture after the count word; FindTexture walks it for unk08 entries. |
| unk04 | data | high | The JallocAlloc'd bank block; non-zero marks the slot in use and it is freed with JallocFree(&unk04). |
| unk08 | count | high | Number of textures in the bank, read from the first word of the bank data. |

### Tip

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk15 | read | medium | 0 until the player reads the tip, then set to 1 in UpdateTips; read tips use the dimmer texture and shadow alpha 140 |
| unk16 | animTime | medium | frame counter of the open/read/close sequence in UpdateTip: zeroed on open, < 6 and < 12 are pop-in steps, 100 waits for A, >= 1000 closes, 1012 ends |
| unk20 | altitude | high | Altitude() result stored at parse time and passed as the altitude argument of CastShadow in ProcessShadows |
| unk24 | normal | high | ground normal filled by Altitude() at parse time and passed as the plane argument of CastShadow |
| unk2C | shadowAlpha | high | 255 at parse, 140 once read; passed as the alpha argument of CastShadow |
| unk2D | shadowTarget | medium | written 140 together with unk2C in UpdateTips; sits where Garib.shadowTarget sits after shadowAlpha (never read) |

### Water

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk24 | currentX | high | read from the level file; WaterDoFloating adds unk24/unk28/unk2C to the actor velocity when doCurrent is set |
| unk28 | currentY | high | y part of the water current added to actor velocity in WaterDoFloating (never loaded from the file, stays 0) |
| unk2C | currentZ | high | read from the level file; z part of the water current added to actor velocity |
| unk30 | height | high | surface height read from the level file, compared with actor heights everywhere and passed to WaterDoSplashes, CreateWaterRipple and CreateWake |
| unk34 | targetHeight | high | set equal to unk30 at parse, set by puzzle action 33, and UpdateWater eases unk30 toward it by a quarter per frame |

### WorldInfo

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| unk03 | secondLevel | medium | ROM data gives the second level of each world (11 for Atlantis, 36 for Space); read through the byte cast worldInfo[world][gameInfo.unk11 - 14] for exit type 17 |
| unk05 | extraLevels | medium | ROM data: [0] boss level, [1] bonus level (read as unk05[1] for exit type 4, via cast for 19 and 20), [2] repeats lastLevel |

## Struct renames

- `PlatformUnk54` is `PlatformRestartGate` (allocated with the tag "Plat_Rest"; the locals that hold it are
  named `restartGate`).
- `ActorExtra` is `ActorShadow` (allocated with the tag "Act_Shad").

## Stand-in types

Local structs only described the few bytes that one function used. They are replaced by the real types:

- `HandLight` (local to hand.c) was `GameLight`: ApplyAction allocates 0x58 bytes with the tag "Light" and
  passes it to SetupLight. The code now uses GameLight's fields (timer, unk9, spin[0], spin[1]).
- `RobotPanel` in nmeprocess2.c: `D_802997C4` is `enemies[0].actor.anim` (0x802996E8 + 0xDC). NMEProcessEvilRobot
  now writes `enemies[0].actor.anim->speed`, limited by `anims[5].speed`, as nearby code already does.
- `OFTC` and `OFTCTex` in actor.c: FreeOFTC takes an `Object`. It frees `mesh->origUV`, the copy of the UVs that
  RestoreObjectPointers allocates with the tag "OFTC%d".
- `ScreenDebris` keeps its own copy of the `Overlay` fields (it is cast to `Overlay *`, as `Debris` is cast to
  `Actor *`), but the copy now has Overlay's types: `frame`, `Bitmap *bitmaps`, `Sprite sprite` and `frames`.
  DestroyScreenDebris frees `sprite.bitmap`.
- `Platform.blur` is a `PlatformBlur *`. The casts in parse.c and platform.c are gone.
- `Menu` (in five front-end files) was `Screen`: SlotSelRun, SlotSelDone and DoneTitleWait cast their
  `Screen *` to it. `selected` is `currentSpot`, `unk28` is `spots[0].x`, and `items[i].func` is
  `spots[i].func`. SlotSelRun and WaitOKScr compare `currentSpot` as signed.

## Enums

`include/enums.h` holds the game's original enums. Glover.iobj has a CodeView `S_CONSTANT` record for each
enum constant (name, value and enum type index); the Windows, SDL and Steam ones are left out, and so are
enums that differ on the N64 (the game modes, which use values the PC enum lacks, the menu screens and the
cheats). The enum type names are new (the originals are not in the file). Each enum was checked against the
N64 code before use: the hand actions against the emulator (HAND_JOINED 3, HAND_DUCK 8, HAND_CARTWHEEL 22,
HAND_LOB 30, HAND_LOCATE 42), the animations against the actions that play them, the levels against
`levelInfo`, the tracks against each level's song, the exits, puzzle actions and conditions, enemy
instructions and debris types against how the code uses them. `HandControl.attachMode` is now `status`
(HAND_NORMAL, HAND_TELEPORTING, HAND_HELD, HAND_HOLDING, HAND_ROBOT).

Also in enums.h: the garib types (`NORMAL_GARIB`, `EXTRA_LIFE`; the PC port has no names for types 1 and 3) for
`Garib.type`, which picks GainExtraLife; and the screen debris types (`SDT_STAR` to `SDT_STAR2`) for
AddScreenDebris and AddStarBurst, whose seven values match the textures AddScreenDebris picks (sspark1,
numlives01, raindrop, ai_snow, yes, no, sspark1). `D_801EFD74`, `D_801EFD84` and `D_801EFD94` were entries 1 to 3
of `garibAnimation`.

The texture banks (`TITLE_TEX_BANK` to `CKBONUS_TEX_BANK`) are the order of LoadTextureBank's cases; the N64
adds banks 16 to 18 (CKCHEAT1, CKCHEAT2, RCDEMO), which have no PC names. The bullet types were checked against
`standardBullets`: entries 9 to 28 hold the matching functions (DestroyBulletEgg for CHUCKIE_EGG,
MaintainBulletBovva for BOVVA_BULLET, ... DestroyBulletBomb2 for BOMB2_BULLET). The vent types are used for
`Vent.type` (`BUBBLE_VENT` makes bubble debris instead of bullets). The case labels of LoadLevel's switch on the
level number are levels.

## Globals

Names of a function's statics come from Glover.iobj, which lists them per function; in our code they are
file-scope globals. A name used by statics of two functions gets a short prefix.

| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| D_801F0232 | footprintTexFmt | medium | DrawFootprint stores the footprint texture's format (`ptr->texture->fmt`) in it and nothing reads it. It was 3 at the title screen and in every frame of every demo recording. |
| D_801E65A4 | questionMode | medium | map name at the -0x12A0 front-end alignment; SlotSelRun's confirm sub-menu: 1 when copying onto another slot (doCopyMode), 2 when starting a new game on a used slot, 3 once that is confirmed; while nonzero the yes/no rows and the red warning text are shown |
| D_801E65A8 | selectingSlots | medium | map name at the -0x12A0 front-end alignment; set when a main menu row (load, new game, copy) is chosen. The cursor moves to the slot row; B clears it and restores the row |
| D_801E65AC | aborting | medium | map name at the -0x12A0 front-end alignment; set when the save-slot screen is left with Back; SlotSelDone then skips the selected spot's func |
| D_801E65B0 | inputOK | medium | map name at the -0x12A0 front-end alignment; TRUE while the save-slot screen takes input (set by the title and name screens, cleared whenever SlotSelRun starts to leave); gates DefWait and the stick |
| D_801EC772 | nextKeyFrame | high | ReadControllerFromData: the frame number of the next recorded button change in demo playback, read from the record stream; -1 ends the demo |
| D_801FA9E4 | playbackButtons | high | ReadControllerFromData: the button word read from the record stream and copied into controllerdata[].button each frame |
| D_8029F110 | evilGloveLaugh | medium | static `laugh` of NMEProcessEvilGlove: set when sample 176 plays at the start of animation 6 (the one that ends in KillEnemy). It plays once |
| D_8029F114 | evilGloveCastSpell | medium | static `castSpell` of NMEProcessEvilGlove: set when the spell animation (4) starts its looping sample, which is then faded out |
| D_8029F118 | evilGloveWait | medium | static `wait` of NMEProcessEvilGlove: in SPACEBOSS3 the glove holds still while it is set (cameo items 0 and 4 clear and set it); at CARNIVALBOSS it stops the rising sound repeating |
| D_8029F13C | evilGloveFootstepTimer | medium | static `footstepTimer` of NMEProcessEvilGlove: counts down from 78 in SPACEBOSS3's cameo while samples 148 and 149 (the hand's footstep sounds) alternate |
| D_801F0208 | evilGloveFire | medium | static `fire` of NMEProcessEvilGlove (the last of its PC statics): toggles between 0 and 1 to pick the two evilGlovePos entries whose difference aims AddEvilGloveFire |
| D_802B0D40 | staticMemory | medium | the 1 MB that ClearStaticMemory zeroes, starting 0x30 bytes after jallocHeapStart |
| D_801E6750 | ckbfxnum | medium | map name at the -0x12A0 front-end alignment; sample 304, played by DoCarnivalBonus while the cannon turns (its volume uses ckbmulfx); the code passed its low half as `D_801E6752` |
| D_801E6608 | introAnimTime | medium | map name at the -0x12A0 front-end alignment; nonzero stops the menus' idle wobble of the foreground objects (MainTitleWait, MainOptionsWait); the Intro screens clear it and leaving a menu sets it to 31. The emulator shows the title box rocking at 0 and frozen at 31 or 1 |
| D_801E660C | allAdder | medium | map name at the -0x12A0 front-end alignment; -40, added to the x of the title menu's first four foreground objects in InitTitleWait |
| aimArLScale | cAAlphaAim | high | map name at the -0x12A0 front-end alignment; the target that cAAlpha steps toward, reset to Random(255) when reached (the old name came from the map at the wrong offset) |
| aimArRScale | fadeDir | high | map name at the -0x12A0 front-end alignment; 1 or -1, multiplies the slide of the two fadeNiceOL halves |
| bgScale | fadeAdd | high | map name at the -0x12A0 front-end alignment; 1.2 or 1.5, then multiplied by 1.16 each frame; the distance the fadeNiceOL halves move |
| aimBgScale | fadeDone | high | map name at the -0x12A0 front-end alignment; set to 1 when the fadeNiceOL slide finishes; menus wait for it |
| areYouSure | fadeAvailable | medium | map name at the -0x12A0 front-end alignment; 1 while the two fadeNiceOL overlays exist (set by InitFadeOverlayN, cleared by DoneFadeOverlayN) |
| nextLevel | licenceFrame | high | map name at the -0x12A0 front-end alignment; frame counter of the licence screen (n64.bmp or licence.bmp); text changes at 125 and 250 |
| temptext | D_801E64D8 | low | the map has no global here (a static in its build); the old name belongs to D_801E664C |
| D_801E64D0 | zoomBodgeAmount | medium | map name at the -0x12A0 front-end alignment; unused in C |
| fgSpinRot | jogAmt | medium | map name at the -0x12A0 front-end alignment; 4 floats at 0x801E6614; the code wrote fgSpinRot[1] to [4] from 0x801E6610. Set to 6.28 when a menu row is chosen, then decays by 0.9 per frame |
| D_801E6610 | allVAddr | medium | map name at the -0x12A0 front-end alignment; unused in C |
| credpos, spacing, credyscale, credSpace | fgDirections | high | map name at the -0x12A0 front-end alignment; s32 array at 0x801E650C; the spin-in switch reads fgDirections[i] (0 to 3: up, down, left, right). The four old scalar names belong to 0x801E6680 to 0x801E668C |
| slotCopyMode | copyMode | medium | map name at the -0x12A0 front-end alignment; toggled by the copy row of the save-slot screen; while set, the next slot chosen is the copy target |
| sfxVolume | curSoundFXVol | high | map name at the -0x12A0 front-end alignment; sound effect volume of the options screen |
| musicVolume | curSoundMUSVol | high | map name at the -0x12A0 front-end alignment; music volume of the options screen |
| DiffDelay | aimTrans | high | map name at the -0x12A0 front-end alignment; array of targets that actTrans[i] steps toward (pairs with actTrans, as aimScale with actScale). DiffDelay is the map name of 0x801E66B8 |
| D_801E664C | temptext | medium | map name at the -0x12A0 front-end alignment; unused in C |
| D_801E6684 | spacing | medium | map name at the -0x12A0 front-end alignment; unused in C |
| D_801E6688 | credyscale | medium | map name at the -0x12A0 front-end alignment; unused in C |
| D_801E66B0 | difficultySetting | medium | map name at the -0x12A0 front-end alignment; unused in C |
| D_801E66B8 | DiffDelay | medium | map name at the -0x12A0 front-end alignment; unused in C |
| D_801E6728 | sYaddr | medium | map name at the -0x12A0 front-end alignment; unused in C |
| actorDistList | distList | high | map name at -0x13C0; AddActorToDistList adds to it |
| handShinTextureData | handTextureData | high | map name at -0x13C0, next to handTextureList |
| handShinTextures | handTextureList | high | map name at -0x13C0 |
| handShinTexturePalette | handTexturePalette | high | map name 8 bytes before hand (-0x13C8). The old symbol_addrs entry handTexturePalette = 0x802902C8 was 8 bytes out and unused |
| bgLighting | backgroundLightList | high | map name at -0x13D0, between lightningList and windList |
| D_801F40C0 | jumptable | high | map name at -0x13B0, directly after aspMainDataEnd; 16 code addresses read by the music player at 0x801C15F0 |
| D_801E7584 | gameInfo.exitFlags | high | gameInfo + 0x54; RunGame tests bit 0 before letting a button skip a cameo |
| D_8028FAC8 | cameraInfo.focus | high | cameraInfo + 0x100; StartNewLevel and the title set it to &hand.pos and &zero, as other code does |
| D_801FB108 | pad[4].pressed[BUTTON_Z] | high | pad + 0x678: Z on the fifth (debug) pad, which the N64 code never polls; MainTitleWait then cycles autoPlayback (0 off, 1 record, 2 play back). In the emulator, setting it advanced autoPlayback and stayed set |
| D_801E75A0 | selectedLevel | high | static of LevelSelectScreen: the menu cursor |
| D_801F7DC8 | numLevels | high | static of LevelSelectScreen: 42 entries (48 with the cheat) |
| D_801E75A4 | oldSelectedLevel | high | static of LevelSelectScreen: the cursor of the last redraw |
| D_801E75A8 | levelSelectWait | high | static `wait` of LevelSelectScreen: the stick repeat delay (UpdatePause also has a `wait`) |
| D_801F7DCC | textItem | high | static of LevelSelectScreen: the text items of the menu |
| D_80000430 | zbuffer | high | gDPSetDepthImage target in main.c; D_430 (its physical alias) is filled with 0xFFFCFFFC as the depth clear; 320x240x2 bytes end at 0x80025C30, just below D_80025C60 |
| D_80025C60 | rdp_output | high | RDP output buffer: task output_buff, output_buff_size = &D_80025C60[rdp_output_size] (0x1000 u64); sits beside rdp_output_size/rdp_output_len |
| D_8002DC90 | audio_memory | high | linker map name (same fixed address in every build); musicPlayer.heap with heap_length 0x493E0, which ends exactly at D_80077070 |
| D_80077070 | pbankBuffer | medium | audioCtrl.base: pbank1 is DMAed here and pbank2 follows it (audioCtrl.ptr = base + pbank1 size) |
| D_800DD030 | sysfont | high | linker map name (same fixed address in every build); fixed RAM buffer LoadTextureBank(10) DMAs FONT_TEX_BANK into instead of JallocAlloc; follows the second framebuffer (D_800B7830+0x25800) |
| D_8011C720 | rspbootTextStart | high | ucode_boot of the gfx task; the splat textbin segment of that name at ROM 0x1D720 (libultra ucode.h name) |
| D_801BC710 | gspF3DEX_fifoTextStart | high | ucode text of UseUCode 4, paired with gspF3DEX_fifoDataStart; splat textbin segment of that name at ROM 0xBD710 |
| D_801BDB40 | gspSprite2DTextStart | high | ucode text swapped in when flag is set, paired with D_801F2E40; splat textbin segment at ROM 0xBEB40 |
| D_801E76C0 | decompOut | medium | DecompressBuffer output write pointer (set to dst, advanced by func_8012C32C) |
| D_801E76C4 | decompSize | medium | count of bytes written, returned by DecompressBuffer as the decompressed size |
| D_801EC7F6 | controllerBusy | medium | ControllerProc skips polling while it is nonzero and sets it to 1 around the EEPROM operations (saves and restores the old value) |
| D_801EC908 | letters_index | high | letters_font.index in the data (character list of the big font, strlen used by LoadFont) |
| D_801EC940 | help_letters_index | high | help_letters_font.index in the data (character list of the help font) |
| D_801ECAA4 | letters_bitmaps | high | letters_font.bitmaps (Bitmap array whose .buf LoadFont fills from letters_offset) |
| D_801ECE04 | help_letters_bitmaps | high | help_letters_font.bitmaps (Bitmap array whose .buf LoadFont fills from help_letters_offset) |
| D_801ED394 | font_xpos | high | font_set_pos x, passed to spMove; iobj global font_xpos |
| D_801ED398 | font_ypos | high | font_set_pos y, passed to spMove; iobj global font_ypos |
| D_801ED39C | font_win_w | medium | font_set_win width (init 40), passed to text_sprite xlen and sprite width |
| D_801ED3A0 | font_win_h | medium | font_set_win height (init 1), passed to text_sprite ylen and sprite height |
| D_801ED3A8 | font_xscale | high | font_set_scale x (f64, init 1.0), passed to spScale; iobj global font_xscale |
| D_801ED3B0 | font_yscale | high | font_set_scale y (f64, init 1.0), passed to spScale; iobj global font_yscale |
| D_801EFC98 | schedRetraceCounts | medium | s16[2] in nnScEventHandler: [0] counts retraces up to desiredFrameRate (game frame msg), [1] up to 3 (retraceMsg) |
| D_801EFC9C | timerRetraceCounts | medium | s16[2] in TimerProc, the same two counters as schedRetraceCounts but driven by the 19.667ms timer |
| D_801F2E40 | gspSprite2DDataStart | high | ucode_data swapped in with gspSprite2DTextStart; splat databin segment at ROM 0xF3E40 |
| D_801F9690 | decompWindow | medium | 0x1000-byte LZ sliding window (ring buffer) cleared at start and read back by offset |
| D_801FA690 | decompWindowPos | medium | write position in the window, advanced and masked with 0xFFF in func_8012C32C |
| D_801FB2A0 | CRCtable | high | u32[256] filled by InitCRCTable (poly 0x04C11DB7) and used by UpdateCRC/UpdateCRCData; iobj global CRCtable |
| D_801FB6A0 | idleThread | high | boot() creates it with thread id 1 and idlePriority and it starts func_80139ED4 (symbol also covers its 0x400-byte stack after the 0x1B0 OSThread) |
| D_801FBC50 | piMessageBuf | high | osCreatePiManager message buffer of 12 entries (0x30 bytes), next to piMessageQ; sits at the top of the idle thread stack |
| D_801FBC80 | piMessageQ | high | OSMesgQueue passed to osCreatePiManager (OS_PRIORITY_PIMGR), 0x18 bytes |
| D_802AE990 | piMgrThread | high | osCreatePiManager passes it to osCreateThread and osStartThread for __osDevMgrMain and stores it in __osPiDevMgr; a 0x1B0 OSThread |
| (none) | piMgrStack | high | 0x802AEB40, the 4096 bytes (OS_PIM_STACKSIZE) between piMgrThread and piMgrEventQueue; osCreatePiManager gives piMgrEventQueue's address as the thread's stack top. Replaces the map label `_objBank9_data_objend` (0x802AF201), which lay inside it |
| D_802AFB40 | piMgrEventQueue | high | osCreatePiManager makes it with osCreateMesgQueue (one message, piMgrEventBuf) and gives it to osSetEventMesg for the PI event |
| D_802AFB58 | piMgrEventBuf | high | the one-message buffer of piMgrEventQueue |
| D_8028DA38 | overlay_dl_next | high | running write pointer into overlay_dl/overlay_dl2 (set per draw_buffer in game.c and DrawGraphics, fed to sp->rsp_dl_next and stored back in font_show_string and PrintOverlay) |

The 80 ROM addresses (`D_B0...`) in banks.c are named after the message string of the bank that starts there:
`titleTexBankRomStart`, `atlantisL1ObjBankRomStart` and so on. Each one is also the end of the bank before it
in ROM order. `landscapeFilesRomStart`/`RomEnd` and `recordKeyFilesRomStart`/`RomEnd` bound the level files and
the demo key files.

Some symbols were addresses inside other variables. They are now written as accesses to those variables:
`template_sprite.rsp_dl` and `.rsp_dl_next`, `fadeOverlay.alpha`, `audioCtrl.handle`, `sched.audioThread`, and
`&overlay_dl[NUM_DL(180 - fontmagicnum)]` (with the libultra macro from sp.h) for the two display list limits in
font_show_string. The four colour bytes are one array, `font_col[4]`, as the PC port's global `font_col`
suggests. The microcode symbols use the libultra declarations in ucode.h; `rspbootTextEnd` (in symbols.ld) is
the same address as the first function of game.c.
| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| D_801E7590 | carnivalBankToggle | medium | flips 0/1 each LoadLevel of CARNIVALBONUS while cheat 21 (SECRET CHEAT) is on and picks texture bank 16+n. |
| D_801E7594 | titleMenuOldSelected | medium | TitleScreen: starts at 999 and holds the previous cursor. The menu texts are rebuilt on change (same role as oldSelectedLevel). |
| D_801E7598 | titleMenuSelected | medium | TitleScreen: u8 cursor 0..2 moved by stickY, switch on it picks Continue/New game/Options. |
| D_801E7599 | titleMenuWait | medium | TitleScreen: stick repeat delay (10) and 200 while fading out, counts down like levelSelectWait. |
| D_801F7D90 | titleMenuText | medium | TitleScreen: Text*[14] filled with NEW_TEXT and freed with SubText, same layout as textItem. |
| D_801E75AC | timeTrialSelectedLevel | high | iobj static selectedLevel of TimeTrialScreen (func_8011EAB8): cursor into the level array; prefixed; selectedLevel also exists. |
| D_801E75B0 | timeTrialOldSelectedLevel | medium | starts at 999, set to the cursor each frame, texts rebuilt when different (oldSelectedLevel role); no listed static for it. |
| D_801E75B4 | timeTrialWait | high | iobj static wait of TimeTrialScreen: stick repeat delay 3, 200/-200 while fading out; static, prefixed. |
| D_801F7E04 | timeTrialSaveRequired | high | iobj static saveRequired of TimeTrialScreen: set when a best time is reset, then SaveBestTimes and SaveCRC run. |
| D_801F7E08 | timeTrialCancelMode | high | iobj static cancelMode of TimeTrialScreen: Z enters the reset-time prompt, B cancels it, Z again resets. |
| D_801F7E0C | timeTrialTextItem | high | iobj static textItem of TimeTrialScreen: Text*[14][3] (name, time, holder); static, prefixed. |
| D_801F7EB4 | timeTrialLevelArray | high | iobj static levelArray of TimeTrialScreen: s32[48] of the playable levels, indexed by the cursor. |
| D_801F7F74 | timeTrialNumLevels | high | iobj static numLevels of TimeTrialScreen: count of entries stored in the level array; static, prefixed. |
| D_801F7F78 | timeTrialFirstLevel | medium | only written, with j when the level array is still empty. It is the first listed level; no listed static. |
| D_801E75B8 | timePicXpos | high | iobj static of EndOfLevelScreen: target x of item 0, the eolcl01 clock picture (D_801F92F8). |
| D_801E75BA | timeBoxXpos | high | iobj static of EndOfLevelScreen: target x of item 1, the box that shows this level's time. |
| D_801E75BC | bestPicXpos | high | iobj static of EndOfLevelScreen: target x of item 2, the trop0001 trophy picture. |
| D_801E75BE | bestBoxXpos | high | iobj static of EndOfLevelScreen: target x of item 3, the box that shows the best time. |
| D_801E75C0 | garibPicXpos | high | iobj static of EndOfLevelScreen: target x of item 4, the eolgb01 garib picture. |
| D_801E75C2 | garibBoxXpos | high | iobj static of EndOfLevelScreen: target x of item 5, the garib count box. |
| D_801E75C4 | endOfLevelWait | high | iobj static wait of EndOfLevelScreen: set to 200 when A or the timer ends the screen, cleared after the fade; static, prefixed. |
| D_801F7F7C | quitTimer | high | iobj static quitTimer of EndOfLevelScreen: -1, set to 60 once the garibs are counted, at 0 the screen quits. |
| D_801F7F80 | conquered | high | iobj static conquered of EndOfLevelScreen: result of CheckForLevelCompleted (all garibs), makes the garib count flash. |
| D_801F7F84 | oldBestTime | high | iobj static oldBestTime of EndOfLevelScreen: bestTimes entry saved at start, compared with the new time for the star burst. |
| D_801F90D0 | eolItems | high | iobj global eolItems: EolItem[7] sliding the time, best time, garib and score boxes in EndOfLevelScreen. |
| D_801F92F8 | timePicOL | high | iobj global timePicOL: overlay set up with eolcl01.bmp in EndOfLevelScreen and clock.bmp in boss.c (present.c reuses the slot for glover.bmp); stale symbol_addrs timePicOL=0x801F92F0. |
| D_801F9280 | bestPicOL | high | iobj global bestPicOL: overlay set up with trop0001.bmp, the best-time picture of EndOfLevelScreen |
| D_801F94D0 | garibPicOL | high | iobj global garibPicOL: overlay set up with eolgb01.bmp in EndOfLevelScreen; stale symbol_addrs garibPicOL=0x801F94C8. |
| D_801F9548 | scoreBoxOL | high | iobj global scoreBoxOL: eolItems[6] overlay whose alpha and scale grow the score box; stale symbol_addrs scoreBoxOL=0x801F9540. |
| D_801F94CC | levelTime | high | iobj global levelTime: f32 seconds from player.levelTime, shown on the end screen and stored as the best time (Player.levelTime is only a struct field); stale symbol_addrs levelTime=0x801F94C4. |
| D_801E75F4 | eowWait | high | iobj static wait of EndOfWorldScreen: 200 once the screen is left (A or 500 frames); static, prefixed. |
| D_801E75F8 | wellDoneX | high | iobj static wellDoneX of EndOfWorldScreen: x offset of the two world-complete texts, starts 200 and bounces to 0. |
| D_801E75FC | wellDoneV | high | iobj static wellDoneV of EndOfWorldScreen: velocity of wellDoneX (-1.3 per frame, -0.5 on a bounce). |
| D_801E7600 | wellDoneB | high | iobj static wellDoneB of EndOfWorldScreen: bounce count of wellDoneX, stops bouncing at 2. |
| D_801E7604 | wellDoneBX | high | iobj static wellDoneBX of EndOfWorldScreen: height of the newwave object (pos.y and rot.x), bounces like wellDoneX. |
| D_801E7608 | wellDoneBV | high | iobj static wellDoneBV of EndOfWorldScreen: velocity of wellDoneBX. |
| D_801E760C | wellDoneBB | high | iobj static wellDoneBB of EndOfWorldScreen: bounce count of wellDoneBX. |
| D_801F7F88 | eowLettersX | high | iobj static lettersX of EndOfWorldScreen: f32[100] x offsets of the time-up letters (and 15 and 19 for text lines); prefixed, lettersX is a global. |
| D_801F8118 | eowLettersV | high | iobj static lettersV of EndOfWorldScreen: f32[100] velocity of each lettersX entry. |
| D_801F8310 | eowLettersB | high | iobj static lettersB of EndOfWorldScreen: s16[100] bounce count of each letter. |
| D_801F82A8 | eowLetters | high | iobj static letters of EndOfWorldScreen: char[50][2], the time-up string split into one-letter strings. |
| D_801F830C | eowLettersN | high | iobj static lettersN of EndOfWorldScreen: strlen of the time-up string, the letter count. |
| D_801F83D8 | eowBackdrop | high | iobj static backdrop of EndOfWorldScreen: the Backdrop allocated and added there; prefixed. |
| D_801F91B0 | eowLight | high | iobj global eowLight: the GameLight set up with SetupLight in EndOfWorldScreen; stale symbol_addrs eowLight=0x801F91A8. |
| D_801E760E | errorWait | high | iobj static wait of ErrorScreen: 200 after A is pressed, cleared on fade out. |
| D_801E7618 | gameWonWait | high | iobj static wait of GameWonScreen (func_801230A8): 200 after A or frame 200. |
| D_801F83E0 | gameWonXPos | high | iobj static xPos of GameWonScreen: starts 320, eases to 1, x offset of the two text lines. |
| D_801E7628 | exitDir | high | iobj static dir of CheckForExit (func_801246AC): angle rotating the puff vector, += exitspinspeed. |
| D_801E762C | titleSpinCamRot | high | iobj static camRot of TitleScreenCameraSpin: angle += 0.005 per call. |
| D_801F83E4 | titleSpinVect | high | iobj static vect of TitleScreenCameraSpin: camera position passed to SetCameraOverride. |
| D_801F83F0 | titleSpinVect2 | high | iobj static vect2 of TitleScreenCameraSpin: camera target Vec3. |
| D_801E7648 | titleSpin2CamRot | high | iobj static camRot of TitleScreenCameraSpin2: angle += 0.005 per call. |
| D_801F83FC | titleSpin2Vect | high | iobj static vect of TitleScreenCameraSpin2: rotated vectC, the camera position. |
| D_801F8408 | titleSpin2Vect2 | high | iobj static vect2 of TitleScreenCameraSpin2: rotated vectC2, the camera target. |
| D_801E764C | gameOverWait | high | iobj static wait of GameOverScreen: 200 after A or frame 100, cleared on fade out. |
| D_801F841C | gameOverVel | high | iobj static vel of GameOverScreen: f32 speed of the sliding GAME and OVER texts (+1.5 per frame, bounces); prefixed. |
| D_801F8428 | numBalls | high | iobj static numBalls of DoCaveStuff: hub crystal balls placed, loop bound for the platform glow and ++ per new ball. |
| D_801F842C | caveSfx | medium | iobj global caveSfx: the ContinuousSample DoCaveStuff plays (sample 60) for the lightning; stale symbol_addrs caveSfx=0x801F8BB8 is not it. |
| D_801E765C | pauseCount | high | iobj static pauseCount of UpdatePause: frame counter of the pause menu open/close animation (1000+ closing). |
| D_801E7664 | pauseFontAlpha | high | iobj static fontAlpha of UpdatePause: menu alpha raised by 42 and passed to NEW_TEXT; also a static of UpdateTip. It is prefixed. |
| D_801E7665 | pauseSelection | medium | by elimination the iobj static mode of UpdatePause: selected row, indexes menuList (original name mode clashes with pauseMode). |
| D_801E7666 | pausePadWait | high | iobj static padWait of UpdatePause: set while the stick is held. The row moves once. It starts at 1. |
| D_801E7667 | pauseQuestion | high | iobj static question of UpdatePause: 0 none, 1 yes, 2 no on the are-you-sure tick and cross. |
| D_801F8448 | pauseHoop | high | iobj static hoop of UpdatePause: index of the chosen restart gate among the touched ones. |
| D_801F844C | pauseNumHoops | high | iobj static numHoops of UpdatePause: count of platforms with a touched restartGate. |
| D_801F8450 | pauseWait | high | iobj static wait of UpdatePause: 5 frame stickX delay when changing the restart gate; static, prefixed. |
| D_801F8454 | pauseRestart | high | iobj static restart of UpdatePause: Platform* chosen as the checkpoint, copied to handControl.checkpoint. |
| D_801F8458 | pauseItemActive | high | iobj static itemActive of UpdatePause: u8[2] flags for the restart and exit-to-hub entries. |
| D_801F845C | pauseMenuList | high | iobj static menuList of UpdatePause: u8[4] item ids (1, 2, 3) of the rows shown. |
| D_801F8460 | pauseNumItems | high | iobj static numItems of UpdatePause: number of menu rows, selects the box height and baseY. |
| D_801F8464 | pauseBaseY | high | iobj static baseY of UpdatePause: y of the PAUSED heading, 90, 80 or 70 by row count. |
| D_801E7674 | languageWait | high | iobj static wait of SelectLanguage (func_8012860C): stick repeat delay 5; static, prefixed. |
| D_801E7678 | languageCount | high | iobj static count of SelectLanguage: frame count that holds, 1000 on confirm, ends at 1010; static, prefixed. |
| D_801F9370 | languageFlagOL | medium | Overlay[3] only SubOverlay'd in SelectLanguage, one per flag; matches the iobj global flag (flag is too generic as a name). |
| D_801E767C | outroFireTime | medium | s16 456 compared with the animation time to fire the red bullet in DoOutroEffects. |
| D_801E7680 | outroSpinDiv | medium | 1.5 divisor: q.w = frameCount / it is the spin angle of the outro bullet around its path. |
| D_801F8468 | outroBullet | high | iobj static bul of DoOutroEffects: Bullet* created at the fire time; prefixed, bul also in DoIntroEffects. |
| D_801F846C | outroBulletPos | high | iobj static bulPos of DoOutroEffects: Vec3 path position of the bullet. |
| D_801F847C | introText | high | iobj static mess of DoIntroEffects: Text* of the intro caption, faded then freed with SubText. |
| D_801F8480 | introBullets | high | iobj static bul of DoIntroEffects: Bullet*[3] spell bullets, freed at frame 876; prefixed. |
| D_801F848C | introBulletNum | high | iobj static bulNum of DoIntroEffects: index of the next bullet in introBullets. |
| D_801E7694 | introDimEndFrame | medium | DoIntroEffects dims the lights from frame outroposplus (530) to this 551 and sets fixed colours on the last frame. |
| D_801F8BD0 | controllerOL | high | iobj global controllerOL: Overlay[11] pad, buttons and stick drawn when printController is set; stale symbol_addrs controllerOL=0x801F8BD8. |
| D_80297904 | bubbleTexture | high | Texture* loaded from bubble.bmp in InitMorePersonal and animated with updateLava on ATLANTISBOSS in MainGameLoop. |
| D_802B0D10 | jallocHeapStart | high | address passed to JallocInit as the heap start (heap runs to 0x80400000) in main, frontend and game.c. |
| D_801F9208 | ballPicOL | high | iobj global ballPicOL: the glob01.bmp overlay of the title and front end; title.c sets its frames to spriteFrameList[GLOVE_ANIM] |

`spriteFrameList` now has one declaration, `SpriteFrameList spriteFrameList[NUM_SPRITE_ANIMS]` in overlay.h. Its index is the PC port's sprite animation enum (`enum SpriteFrameId`,
DUST_ANIM to TRAJ_ANIM). All 38 entries were checked against the frame names in the table's data (smoke, smk,
plat, ai_spl, ... glob, eolgb, eolcl, trop00, puff, cross, horn00, note00, ohno, rgarib, traj); entry 13
(ballg2) has no PC name.
| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| D_801E6730 | cannonxspeed | high | iobj global; carnival cannon aim: stickX / 2000 turns the platform (DoCarnivalBonus) |
| D_801E6734 | cannonyspeed | high | iobj global; stickY / 2000 tilts the cannon platform, twin of D_801E6730 |
| D_801E6738 | cannonrecoil | high | iobj global; -4 pushed along z on firing (platform recoil) |
| D_801E673C | cannonrecoilfac | high | iobj global; 0.8 multiplied into the platform position every frame (recoil decay) |
| D_801E6740 | ckbcamfrac | high | iobj global; 0.9 camera blend fraction, same use as spbcamfrac in DoSpaceBoss3 |
| D_801E6744 | reloadCounter | medium | iobj global; 5, copied into the shot cooldown (carnivalReload) after firing, like spbReloadCounter |
| D_801E6748 | cannonrand | medium | iobj global; 4, range of the random y jitter added when the cannon moves |
| D_801E674C | ckbmulfx | medium | iobj global; 5000, multiplies the turn amount to give the aim sound volume |
| D_801F7D40 | carnivalTempVect2 | medium | iobj static tempVect2 of DoCarnivalBonus: the (0,0,40) look-at point (same role as D_801FA6C0 in DoSpaceBoss3) |
| D_801F7D4C | carnivalCamOffVect | high | iobj static camOffVect of DoCarnivalBonus: constant camera offset (0,40,-60) |
| D_801F7D58 | carnivalCamVect | medium | iobj static camVect of DoCarnivalBonus: rotated offset + hand.pos, the camera target position |
| D_801F7D64 | carnivalTempVect | high | iobj static tempVect of DoCarnivalBonus: scratch vector used all through the function |
| D_801F7D70 | carnivalReload | high | iobj static reload of DoCarnivalBonus: shot cooldown, set to reloadCounter and counted down |
| D_801F7D74 | carnivalTimer | high | iobj static timer of DoCarnivalBonus: 60 frame wait once all 30 targets are hit |
| D_801F7D78 | carnivalYPos | high | iobj static yPos of DoCarnivalBonus: random y jitter added to the platform |
| D_801F7D7C | atlantisInWater | high | iobj static inWater of DoAtlantisBonus: frames the hand has been swimming, exits after 20 |
| D_801F7D80 | fireTimer | high | iobj static fireTimer of DoControlRoom: counts frames, fires at 30 and 60 |
| D_801FA6A0 | framesPerSec | high | iobj static framesPerSec of DoBoss: 17 on PAL, 20 otherwise (frames per timer second) |
| D_801E76E0 | piratesBossQ | high | iobj static q of DoPiratesBoss (the only static there); Quat used with MakeUnit/GetQuaternionFromRotation |
| D_801E76FA | handPain | high | iobj static handPain of DoFortressBoss: counts frames Frankie is near the hand, HurtHand at 8 |
| D_801E76FB | ballPain | high | iobj static ballPain of DoFortressBoss: same for the ball, BurstBall at 8 |
| D_801E76FC | goForPlatform | medium | iobj static of DoFortressBoss left over by elimination: u8 flag only read, picks the 70/111 ball radius |
| D_801E76FD | frankieDead | medium | iobj static of DoFortressBoss: set to 100 when platform 20 fires and runs down through the death sequence |
| D_801E7700 | lightningSfx | medium | iobj static lightningSfx of DoFortressBoss: handle of PlaySample(62) lightning, 0 when not playing |
| D_801E7704 | thunderClap | medium | iobj static thunderClap of DoFortressBoss: one-shot flag around PlaySampleNot3D(248) |
| D_801E7708 | frStunCount | medium | iobj static frStunCount of DoFortressBoss: 5 frames the stun bolt between the two platforms can hit Frankie |
| D_801FA6A4 | nearPlat | high | iobj static nearPlat of DoFortressBoss: platform 1-8 nearest the hand |
| D_801FA6A8 | frStunPos1 | medium | iobj static frStunPos1 of DoFortressBoss: first stun bolt end (stomped platform) |
| D_801FA6B4 | frStunPos2 | medium | iobj static frStunPos2 of DoFortressBoss: second stun bolt end (partner platform) |
| D_801FA6C0 | spaceBossTempVect2 | medium | iobj static tempVect2 of DoSpaceBoss3: (0,0,40) look-at point |
| D_801FA6CC | spaceBossCamOffVect | high | iobj static camOffVect of DoSpaceBoss3: constant camera offset (0,150,-50) |
| D_801FA6D8 | spaceBossCamVect | medium | iobj static camVect of DoSpaceBoss3: rotated offset + hand.pos |
| D_801FA6E4 | spaceBossCamVect2 | medium | iobj static camVect2 of DoSpaceBoss3: copy of the look-at point passed to SetCameraOverride |
| D_801FA6F0 | spaceBossRot | high | iobj static rot of DoSpaceBoss3: Vec3 euler angles of the aim, clamped and turned into hand.rotQuat |
| D_801FA6FC | spaceBossReload | high | iobj static reload of DoSpaceBoss3: shot cooldown set from spbReloadCounter |
| D_801FA73C | spaceBossPanelOn | high | iobj static panelOn of DoSpaceBoss3: set once the foreground panel reaches z 299 |
| D_801FA700 | spbCrosshair | medium | GameSprite centred (offset -w/2,-h/2) at the look-at point with bulletAnimation[10]; spb prefix as spbxspeed etc |
| D_801FA738 | spbCrosshairAlpha | medium | target alpha of the crosshair: 0 while Z (shield) is held else spbalpha |
| D_801FA740 | cheatNum | high | iobj static cheatNum of DoFlythru2Screen: index into cheatInfo[], (+1) % 26 each time |
| D_801FA744 | flythru2Alpha | high | iobj static alpha of DoFlythru2Screen: text alpha 255 fading by 15 |
| D_801FA748 | flythru2XPos | high | iobj static xPos of DoFlythru2Screen: text slide offset 320 -> 1 |
| D_801FA74C | flythru2Num | medium | iobj static num of DoFlythru2Screen by elimination: s32 flag that gates the cheat text (1 = skip) |
| D_801EC770 | sameCount | high | iobj static sameCount of ReadControllerFromData: frames since the recorded buttons last changed |
| D_801EC774 | exitCount | high | iobj static exitCount of ReadControllerFromData: 20 frame countdown to leave the demo |
| D_801EC76C | recordCode | medium | iobj static recordCode of ReadControllerFromData by elimination: u16[2] button masks (now, previous) compared when recording |
| D_801FA9E0 | prevFrame | medium | iobj global; frameCount guard in the recording half of ReadController (returns if frame already handled) |
| D_801FA9E8 | prevFrame2 | medium | iobj global; same guard in ReadControllerFromData, twin of prevFrame |
| D_801EEBC8 | icl | high | iobj static icl of PlayerSpinCamera: u8 counter (20) that holds the C-left latch |
| D_801EEBC9 | icr | high | iobj static icr of PlayerSpinCamera: same for C-right |
| D_801EF45C | joinVal | high | iobj static joinVal of MaintainAction: 20, join animation length (unk50C / joinVal) |
| D_801EF460 | cartwheelAccel | high | iobj static cartwheelAccel of MaintainAction: set 3.5, *0.95 per frame in the HAND_CARTWHEEL case |
| D_801EF464 | handOrbiterFreq | high | iobj static freq of HandleSpellOrbiters (4): SineWave period of the orbiters; prefixed as MoveBall also has freq |
| D_801F1AE4 | ballOrbiterFreq | high | iobj static freq of MoveBall (3): SineWave period of the ball orbiter bob; prefixed as HandleSpellOrbiters has freq |
| D_801F1AE8 | driftSpeed | high | iobj static driftSpeed of MoveHandToBall: eases toward driftSpeedAim by 1/4 per frame |
| D_801F1AEC | driftSpeedAim | high | iobj static driftSpeedAim of MoveHandToBall: target of driftSpeed |
| D_8029F430 | spinPos | high | iobj static spinPos of MoveBallToHand: eases toward spinPosAim |
| D_8029F434 | spinPosAim | high | iobj static spinPosAim of MoveBallToHand: target set with SetHandRadius |
| D_801F01B4 | cannonballMinSpeed | high | iobj static minSpeed of NMEProcessCannonball: speed threshold 0.1 |
| D_801F01B0 | cannonballRot | high | iobj static rot of NMEProcessCannonball: 0.96 spin speed given to the cannonball |
| D_801F01C0 | robesMoveSpeed | high | iobj static moveSpeed of NMEProcessRobes: 3, max step per frame toward the ball |
| D_801F01CC | waitForFootDown | medium | iobj static waitForFootDown of NMEProcessRobot: latch set when the ball is under the foot platform |
| D_8029F160 | downAgain | high | iobj static downAgain of NMEProcessHubChicken: frames until the chicken head goes back down (hcCluck[type]) |
| D_8029F158 | hubChickenNum | high | iobj static num of NMEProcessHubChicken: index of the cheat code letter (scratch / 15) |
| D_8029F15C | hubChickenTempObj | high | iobj static tempObj of NMEProcessHubChicken: Object* found with FindSubObjectFromObject |
| D_8029F11C | evilGloveCount | high | iobj static count of NMEProcessEvilGlove: ++ per frame while firing, platform shrinks after 11 |
| D_8029F120 | jackBox | high | iobj static jackBox of NMEProcessEvilGlove: the CAMEO_ACTOR found at CARNIVALBOSS, destroyed at the end |
| D_8029F124 | evilGlovePos | high | iobj static pos of NMEProcessEvilGlove: Vec3[2] previous positions indexed by D_801F0208 |
| D_802994A0 | puzzleCamVect | high | iobj static camVect of PuzzleExecuteAction: Vec3 camera position for the cam action |
| D_802994AC | puzzleFrameNum | high | iobj static frameNum of PuzzleExecuteAction: frame the cam override started |
| D_80297C00 | parsePuzzle | high | iobj static puzzle of ParseTokenPuzzle: Puzzle* being built |
| D_80297C04 | parseAction | high | iobj static action of ParseTokenPuzzle: PuzzleAction* being built |
| D_80297C08 | parseCondition | high | iobj static condition of ParseTokenPuzzle: PuzzleCond* being built |
| D_80297C0C | parseGaribGroup | high | iobj static garibGroup of ParseTokenCollectable: GaribGroup* being built |
| D_801F95C0 | cameoCamPos | high | iobj static camPos of ProcessCameoInstruction: saved cameraInfo.focus pointer |
| D_801F95C4 | cameoCamVel | high | iobj static camVel of ProcessCameoInstruction: saved cameraInfo.focusVel pointer |
| D_801EFCB4 | personalPlatform | medium | Platform* the debug personal mode puts the hand/ball on; C-left/right steps it round platformList |
| D_801EFCBC | screenshotCount | high | u32 counter used in the c:\n64%04d.bmp screenshot file name, post-incremented |
| D_801EFC84 | confineV | medium | iobj static v of ConfinePlatform (the only one): f32 1.0 added to extent.y for the top test |
| func_80112808 | IsSaveSlotEmpty | high | returns saveSlot[slot].name[0] == 13, the blank-name test used to show the empty slot text and by the copy screen |
| func_80118B70 | SetupNameScr | high | iobj SetupNameScr: initnamescr setup of the name entry screen, ends with WaitNameScr (twin of SetupOKScr) |
| func_8011A6EC | DoneNameScr | high | iobj DoneNameScr: spins the screen out, stores the 3 letter name into bestTimes, calls TimeTrialScr (twin of DoneOKScr) |
| func_8011EAB8 | TimeTrialScreen | high | iobj TimeTrialScreen: 'timetrial' screen, sets up slot 6 on HARD, builds the level list with IsTimeTrialLevel/IsLevelAvailable; mode 15 handler |
| func_80122D68 | NewWorldScreen | high | linker map: NewWorldScreen is 0x28 bytes and sits between EndOfWorldScreen and ErrorScreen, as this function does. Mode 10 handler: just StartNewLevel(gameInfo.level) (D_801E7531 is gameInfo.level); mode 10 is set when exiting to the way room. Named StartCurrentLevel before |
| NewWorldScreen | ErrorScreen | high | linker map: ErrorScreen is 0x20C bytes and follows NewWorldScreen, as this function does. It draws the "errorscn" screen with gameInfo.message and "PRESS A TO CONTINUE"; Glover.iobj has the static errorWait in ErrorScreen |
| func_801230A8 | GameWonScreen | high | iobj GameWonScreen: gameWonCount, hiscore table update, fade out after 200 frames |
| func_80123514 | CheckForExits | medium | iobj CheckForExits: per frame exit timers (levelExitTimer, worldExitTimer, newLevExitTimer) and the exit type dispatch once the screen has faded; per platform part is CheckForExit |
| func_801246AC | CheckForExit | high | iobj CheckForExit: per exit platform, spinning puffs and test of hand/ball against the exit radius; has the static dir (D_801E7628) |
| func_8012860C | SelectLanguage | high | iobj SelectLanguage: 'sellang' screen with the three flags, mode 14 handler |
| func_8012A850 | SetSfxVols | high | iobj SetSfxVols: applies sfxVol to every playing sample (buzzers, platforms, enemies, vents, bullets, hand, ball) |
| func_8012ABFC | IsTimeTrialLevel | medium | iobj IsTimeTrialLevel: level lies inside the first..last range of one of worlds 1-6 |
| func_8012AC54 | IsLevelAvailable | medium | iobj IsLevelAvailable: some save slot has the level bit set (or level 10); filter used with IsTimeTrialLevel for the level list |
| func_8012C32C | DecompressWriteByte | high | DecompressBuffer helper: stores one byte in the output and in the 4K window |
| func_80132FF4 | EepromSaveHiscoreTable | high | eeprom message 1 handler, posted by SaveHiscoreTable: osEepromLongWrite of hiscoreTable |
| func_8013305C | EepromLoadHiscoreTable | high | eeprom message 2 handler, posted by LoadHiscoreTable: osEepromLongRead of hiscoreTable |
| func_801330C4 | EepromSaveBestTimes | high | eeprom message 4 handler, posted by SaveBestTimes: write of bestTimes |
| func_8013312C | EepromLoadBestTimes | high | eeprom message 5 handler, posted by LoadBestTimes: read of bestTimes |
| func_80133194 | EepromSaveSlots | high | eeprom message 6 handler, posted by SaveSlots: write of saveSlot |
| func_801331FC | EepromLoadSlots | high | eeprom message 7 handler, posted by LoadSlots: read of saveSlot |
| func_80139ED4 | IdleProc | high | idle thread entry: creates the PI manager, starts the main thread, drops to priority 0 and spins |
| func_8013C9BC | RenderObjects | medium | called in DrawGraphics before the overlays: resets lightOffset, times 'NEAR', CreateObjects(), animateActors = FALSE |
| func_8013CA1C | InitViewport | medium | sets the 640x480 viewport and the geometry mode (called from InitDisplayLists) |
| func_8013CBBC | InitRdpState | high | sets RDP defaults (scissor, texture modes, dither, depth image) at the start of InitDisplayLists |
| func_8013D24C | ClearFrameBuffer | high | fill-mode rectangle over the colour image using the fog colour (black when not drawing) |
| func_8013D4DC | ClearZBuffer | high | fill-mode rectangle with 0xFFFCFFFC over the depth buffer |
| func_8013D990 | RunGame | medium | starts the controller and graphics threads, loads eeprom data, then the per frame game mode loop |
| func_8013E90C | MeasureFrameTicks | high | waits 13 retrace messages and stores TicksPerFrame / TicksPerSec |
| func_8013EB18 | MainProc | medium | main thread entry: scheduler, task lists, message queues, music driver, then RunGame |
| func_8014F400 | InitTextureBanks | high | iobj InitTextureBanks: clears textureBanks[5] and cyclingTextures[20] |
| func_80158F84 | CollideActorWithPlane | high | iobj CollideActorWithPlane: one CollFace against all actor spheres, bounce and edge handling |
| func_8015EE6C | UpdateActorPosAndRot | high | iobj UpdateActorPosAndRot: pos += vel with friction, then rot += rotVel and rotAim |
| func_8015F098 | UpdateActorTeleport | high | iobj UpdateActorTeleport: shrink, move and grow steps of a teleporting actor; TRUE while shrinking |
| func_8016225C | CheckActorForTeleport | high | iobj CheckActorForTeleport: starts a teleport (state->teleport, shrinkTime) when the actor stands on an active teleport platform |
| func_80165230 | CalcCameraRot | high | iobj CalcCameraRot: camera rot.x/rot.y from camera.pos to cameraInfo.actor.pos |
| func_801735E8 | CheckHandCatapult | high | iobj CheckHandCatapult: catapult hold position, catchTimer and launch for the hand (also clears the launched flag 0x2000) |
| func_80173D28 | HandOverstepsEdge | high | iobj HandOverstepsEdge: tests a face edge against hand and probe, returns the point on the edge (D_80290200) |
| func_8017479C | LoseLife | high | iobj LoseLife: lives--, gameOverCount at -1, restores health, restart gate timer |
| func_80182018 | SetActorXlu | high | SubActor, set the xlu flag on the object and its children (SetObjectXlu), AddActor |
| func_8019A668 | NMEProcessBigRaptor | high | enemy type 34 handler (the prehistoric boss: stomp earthquake, dust, footsteps); iobj NMEProcessBigRaptor is the only unmatched NMEProcess left for it |
| func_8019A990 | NMEProcessSwish | high | enemy type 16 = SWISH, grabs the hand with ApplyAction(HAND_SWISH) and swishSpeed |
| func_801A1558 | NMEProcessSpank | high | enemy type 40 = SPANK, uses enemyStats[40] and spankJumpVal |
| func_801A21B8 | NMEProcessBabySpank | high | enemy type 41 = BABY_SPANK, paired with the other baby via 3 - state->index |
| func_801AB88C | MaintainBulletBomb2 | high | maintainFunc of standardBullets[28] (BOMB2_BULLET): spider walker that spits bubbles and hurts the hand |
| func_801ABD38 | MaintainBulletBomb | high | maintainFunc of standardBullets[15] (BOMB_BULLET): bombLife / bombScaleRate explosion that kills hand and ball |
| func_801BC5E4 | CheatAllowed | high | iobj CheatAllowed: per cheat level/state test used by ActivateCheat |
| D_8028F8B0 | totalVel | high | iobj global totalVel: Vec3 sum of the bounce velocities, zeroed at the start of CheckCollisions and assigned to actor->vel |
| D_8028F8BC | edgePlane | medium | iobj global edgePlane: the CollFace of the nearest edge, stored with its distance by CollideActorWithPlane |
| D_8028F8C0 | edge | medium | iobj global edge: the distance to that edge returned by NearEdges; 0 means no edge |
| D_8028FBC0 | camPos | high | iobj global camPos: Vec3 = hand.pos at HAND_SNATCH; cameraInfo.focus points at it |
| D_8028FBCC | handInertia | medium | iobj global handInertia: f32 copied into hand.friction everywhere (0.6 normal, 0.92 robot) |
| D_802901F0 | camPos2 | medium | iobj global camPos2: Vec3 = ball.pos at HAND_SNATCH, raised by the ball's extra radius |
| D_802901FC | tongueStuck | high | iobj global tongueStuck: 0 when the tongue is fired, 1 once it catches an enemy, garib or wall |
| D_80290200 | sepVect | medium | iobj global sepVect: Vec3 between hand and ball while snatching, kept on its platform |
| D_8029020C | tongueBullet | high | iobj global tongueBullet: the tongue's Bullet, from FireBulletFromActor(&hand, TONGUE_BULLET) |
| D_80290210 | tempGlobalPos | high | iobj global tempGlobalPos: Vec3[8] copy of globalPos saved and restored around the edge test |
| D_80290270 | tempGlobalRot | high | iobj global tempGlobalRot: Vec3[8] copy of globalRot, saved with tempGlobalPos |

Other `D_` symbols were members of named objects and are now written that way: `gameInfo.fade`,
`cameraInfo.targetHeight`, `handControl.deathTimer`, `handControl.frontProbe`, `handControl.fakeball`,
`player.powerups[SUCTION]`, `ballControl.state`, `cameraZoomVal[2]`, `nextBallState[6]`,
`standardBullets[WATER_BULLET].radius`, `standardBullets[TONGUE_BULLET].maxSpeed`, `bulletAnimation[4]`,
`defaultBehaviour[ROBOT]`, `defaultBehaviour[NME_FROG]` and `defaultBehaviour[BUTTERFLY].accel`/`.moveSpeed`.
`enemyStats` is an array of `EnemyStat` (flags, scale, score) indexed by actor type, and `stats` (the
HUD counters, `Stat` in overlay.h) is indexed by the PC port's `GARIB_STATS` to `LIVES_STATS`. Not changed:
ScreenFadedIn reads `gameInfo.fade` and `fade2` as one 32-bit word. It keeps `D_801E753C`. Where the code read only the low half of an `s32` (`D_801E6716`, `D_801EEBF2`, `D_801F19B6`, `D_801E6752`), it now passes the variable itself (`centerDX`, `crawlfx`, `spellDuration`, `ckbfxnum`) to a 16-bit parameter, and GCC loads only the low half. object.c keeps
`D_80284D74` (`matrixStack.result`). Its matrix prototypes use `f32[4][4]`, not maths.h's `Matrix`.
| Old | New | Confidence | Evidence |
| --- | --- | --- | --- |
| D_80092030 | cfb_16_a | high | linker map name (same fixed address in every build); assigned to cfb_ptrs[0] in main.c and cleared with memset 0x25800, one 320x240 16 bit buffer |
| D_800B7830 | cfb_16_b | high | linker map name (same fixed address in every build); assigned to cfb_ptrs[1] in main.c, directly after cfb_16_a |
| D_801E6630 | aimArLScale | medium | map name at the -0x12A0 front-end alignment; target scale of arrOL[0] (right arrow) in HiscoresRun, 1024 or 600 when pressed |
| D_801E6634 | aimArRScale | medium | map name at the -0x12A0 front-end alignment; target scale of arrOL[1] (left arrow) in HiscoresRun |
| D_801E6638 | bgScale | medium | map name at the -0x12A0 front-end alignment; current scale of the two title box foreground objects in HiscoresRun, eases toward the aim |
| D_801E663C | aimBgScale | medium | map name at the -0x12A0 front-end alignment; target of bgScale, 1.2 on a page change then back to 1.0 |
| D_801E6640 | areYouSure | medium | map name at the -0x12A0 front-end alignment; hides the arrows and makes A restore the default table and B cancel in HiscoresRun |
| D_801E6644 | currentHiscoresLevel | high | original global currentHiscoresLevel; page index in HiscoresRun, -1 is the score table and 0 to 5 the worlds |
| D_801E6648 | nextLevel | medium | map name at the -0x12A0 front-end alignment; page chosen by the stick, copied to currentHiscoresLevel when the slide reaches the edge |
| D_801E668C | credSpace | medium | map name at the -0x12A0 front-end alignment; 20 pixel line spacing used as i * credSpace in CreditsRun2 and CreditsDone2 |
| D_801E6720 | toStartGame | medium | original global toStartGame; gates ActStartGame and the fade in DoneOKScr, never set in C |
| D_801E6724 | nameWait | medium | map name at the -0x12A0 front-end alignment; set when the end key is chosen. A is ignored afterwards in WaitNameScr |
| D_801E672C | bonusClosedAlpha | medium | alpha of the BONUS LEVELS CLOSED text, ramped by 30 while Normal is selected in WaitOKScr |
| D_801EB538 | tipTextEnglish | high | original global tipTextEnglish; tipText for language 0 in InitTipList |
| D_801EBB30 | tipTextFrench | high | original global tipTextFrench; tipText for language 1 |
| D_801EC130 | tipTextGerman | high | original global tipTextGerman; tipText for language 2 |
| D_801ED628 | overlayBitCount | medium | counter used to name overlays bit-%d in SetupOverlay |
| D_801ED630 | noControllerTextFrench | medium | Text for D_801EAB40 (French line) made in SetupFlythruScreen and blinked in PrintOverlays |
| D_801ED634 | noControllerTextGerman | medium | Text for D_801EAB44 (German line) made in SetupFlythruScreen and blinked in PrintOverlays |
| D_801F1B14 | cameraInWater | high | iobj static inWater of CheckCameraForWater; prefixed. DoAtlantisBonus also has inWater |
| D_801F2104 | tipSentence | high | iobj static sentence of UpdateTip; index into tipText |
| D_801F2108 | tipFontScale | high | iobj static fontScale of UpdateTip |
| D_801F210C | tipFontAlpha | high | iobj static fontAlpha of UpdateTip; u8 fading in by 42 a frame |
| D_801F2180 | rainLayer | high | iobj static layer of ProcessRain; counts 0 to 2 then wraps |
| D_801F2638 | cheatTextPause | high | iobj static pause of UpdateCheatText (only static); 20, cheatCount counts past it before the text slides |
| D_80289314 | spriteXluMode | medium | flag set once the translucent render mode has been selected in PrintSprite |
| D_80289318 | lastSpriteTex | high | texture of the last sprite drawn, compared in TileRectangle to skip reloading |
| D_80297C30 | lastLitActorFlags | medium | written with actor->flags at the end of ProcessBackgroundLights and never read |
| D_80298928 | lightningVect | high | original global lightningVect; Vec3 array of the lightning points in DrawLightning |
| D_8029FA70 | cameraWaterFrame | high | iobj static frame of CheckCameraForWater; set to -frameCount when entering water |
| D_802AA700 | lastTipTag | medium | tag of the tip just read in UpdateTips, written and never read |
| D_801E6680 | credpos | medium | map name at the -0x12A0 front-end alignment; frame counter in CreditsInit2 that scrolls the credits text up |
| D_801E6690 | cXLU | medium | PC global cXLU: the alpha ramp to 255 restarted on each hiscores page change |
| D_801E6694 | Xos | medium | PC global Xos: bounces of the hiscores text slide, which stops after 2 |
| D_801E6698 | Xos1 | medium | PC global Xos1: x offset of the hiscores text sliding in from 250 |
| D_801E669C | XosVel | medium | PC global XosVel: velocity of Xos1 |
| D_801E66A0 | debounceHiscorePadX | high | PC global: the 40 frame stick delay after a hiscores page change |
| D_801E66B4 | SfxDelay | high | PC global: the 5 frame delay between samples while the options sfx slider moves |
| D_801E66C0 | letters | high | PC global: the key grid of the name entry screen |
| D_801E66F4 | bkspc | high | PC global: the backspace glyph string of the name entry screen |
| D_801E66F8 | space | high | PC global: the space glyph string |
| D_801E66FC | end | high | PC global: the end glyph string |
| D_801E6700 | enteredName | high | PC global: the name being typed |
| D_801E6704 | currentLetter | high | PC global: the highlighted key |
| D_801E6708 | lettersY | high | PC global: the rows of the key grid |
| D_801E670C | centerX | high | PC global: x of the first key column (70) |
| D_801E6710 | centerY | high | PC global: y of the first key row (80) |
| D_801E6714 | centerDX | medium | PC global: x of the typed name (140); WaitNameScr passes it to NEW_TEXT's s16 x. GCC loads its low half |
| D_801E6718 | centerDY | medium | PC global: y of the typed name (180) |
| D_801E671C | debouncePadX | high | PC global: the 7 frame stick repeat delay of the name entry screen |

The 60 ROM addresses in sound.c are the song boundaries of PrepareSong, named `<track>RomStart` after the
Track enum (`trackNN` where the enum has no name). Aliases are written as members:
`noControllerString[1]` and `[2]`, rows of `menuStrings` and `difficultyStrings`, `cheatInfo[i].code`,
`spinInCount[1]`, `arrOL[1].alpha`, `tipTexture[1]`, `ballControl.transformState` and
`&overlay_dl[NUM_DL(178)]`. One place keeps a separate symbol, as the code differs otherwise: KillTitle's `D_801F64B4`
(`fadeNiceOL[1]`). With the table form, GCC keeps the table's address in a register. WaitOKScr's
`startMenuStrings[10]` and `[14]` use the table form too, which matches since WaitOKScr has a
prototype and no stack pad.

## Parameters

Most parameters have the PC port's names (see [sources.md](sources.md#the-pc-ports-gloveriobj)); those are not
listed here. These replaced `argN` placeholders where the PC port has no counterpart:

| Function | Old | New | Confidence | Evidence |
| --- | --- | --- | --- | --- |
| AccelerateActorAcrossPlane2 | arg5 | is3D | high | passed on as the `is3D` parameter of AccelerateActorFromQuaternion; AccelerateActorAcrossPlane has the same last parameter in the PC port |
| GiveBallCancer | arg0 | time | high | stored in `ballControl.cancerTimer`; the one caller passes 500 frames |
| NEW_TEXT | arg8 | oneShot | high | stored in `Text.oneShot`; the PC port's version has `bye` at this position |
| NEW_TEXT | arg10 | centre | high | passed to AddText's `centre`, which centres the text on x and y; the PC port splits it into `alignx` and `aligny` |
| UpdateSaveSlot | arg0 | allGaribs | medium | never read; the caller in game.c passes its `allGaribs` flag, the others pass 0 |
| ChangeLightDir | arg4 | turnSpeed | high | stored in `GameLight.turnSpeed` next to the new target angles |
| TransformAndDrawObject | arg2 | doKeys | medium | never read except to pass on to the recursive calls, and the top-level call passes 0; the PC port's third parameter is `doKeys` |
| FindAndSetupOverlay, FindSetupAndAddOverlay | arg5 | attr | high | stored in `sprite.attr`; callers pass 1 (`SP_TRANSPARENT`) |
| CreateWake | arg2 | platform | high | never read; callers pass the platform the actor touches (`aPtr->obj->platform`) or none. It is now a `Platform *` |

TransformAndDrawObject's fourth parameter stays `arg3`: it is only passed on, and nothing names it.
