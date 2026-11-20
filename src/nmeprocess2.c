#include "common.h"
#include "ident.h"
#include "actor.h"
#include "game.h"
#include "sound.h"
#include "bullet.h"
#include "hand.h"
#include "maths.h"
#include "debris.h"
#include "macros.h"
#include "ball.h"
#include "quake.h"
#include "footprint.h"
#include "jalloc.h"
#include "camera.h"
#include "controller.h"
#include "cheat.h"
#include "cameo.h"
#include "banks.h"
#include "collect.h"
#include "nmeprocess2.h"
#include "anim.h"
#include "enemy.h"
#include "frontend.h"
#include "nmeprocess.h"
#include "object.h"

extern s32 frameCount;

extern f32 spbval;
extern s32 levelCamHeight;
extern f32 handInertia;
extern s32 numBubbles;
extern f32 swishSpeed;
void CreateWake(Vec3 *pos, Vec3 *vel, Platform *platform, f32 level, f32 size, Vec3 *normal);
extern s32 numSmallBubbles;
u32 UpdateCRC(char *ptr);
extern s32 hubChickenNum;
extern Object *hubChickenTempObj;
extern s32 downAgain;
extern s16 hcCluck[];
extern u8 ActiveController;
struct Bullet *FireBulletFromActor(Actor *actor, s32 type, s32 flash);
void HurtHand(s32 type, s32 damage);
extern f32 cannonballRot[];
extern f32 cannonballMinSpeed;
extern Quat vertQ;

void NMEGetNewInstruction(Enemy *enemy, s16 instructionNum, s32 overrideConditions);
void AnimateActor(Actor *actor, s32 animNum, s32 loop, s32 queue, f32 speed);
void SquashActor(Actor *actor, Vec3 *dir, f32 power, f32 maxSquash);
extern Texture *blastTex;
extern f32 gravity;
extern f32 sb3x;
extern Vec3 zero;
extern f32 sb3y;
void FlashScreen(u8 r, u8 g, u8 b, s16 fadeVal, s16 fadeStep);

extern s16 ethelAlpha;
extern s16 ethelAlphaPlus;
extern s16 ethelFreq;
extern u16 ethelLife;
extern f32 robesMoveSpeed;
extern f32 robotRadius;
extern s32 spaceBossExiting;
extern s32 waitForFootDown;
extern s16 levelExitTimer;
struct Platform *TagToFirstPlatform(s32 tag);
void KillHand(s32 type);
void DisableControl(s32 time);
extern f32 spballsp;
extern f32 spv1;
extern f32 spv2;
extern f32 spv3;
extern f32 spvely;
extern f32 spvelz;
extern f32 spankJumpVal;
extern s32 evilGloveLaugh;
extern s32 evilGloveCastSpell;
extern s32 evilGloveWait;
extern s32 evilGloveCount;
extern Actor *jackBox;
extern Vec3 evilGlovePos[2];
extern s32 evilGloveFootstepTimer;
extern s32 evilGloveFire;
extern s8 rotOrder[3];
extern f32 springfac;
extern f32 springfac2;
extern s32 ckstart;
extern f32 sb3loop;
extern Actor ckWeapon;
extern ActorList actorList;
void DestroyAndFreeActor(Actor *actor);
void KillPlatform(struct Platform *platform, s32 killKids);
void AddLightning(Vec3 *start, Vec3 *end, u8 r, u8 g, u8 b, u8 segments, s16 lifetime, s16 flags, f32 jagginess,
                  f32 thickness);
extern u8 portionHealth[3];
extern u8 immuneCount[3];
extern u8 portion;
extern s32 bobCounter;
extern s32 onCounter;
extern s32 lastFireFrame;
extern f32 robotDir;
extern u8 robPuffCol[5][3];
extern s32 spbflytime;
extern s32 bodyTexNum;
extern struct Texture *cyclingTextures[];

void NMEProcessSwish(Enemy *enemy) {
    Vec3 tempVect;
    Vec3 vel;

    enemy->actor.flags &= ~0x200;
    enemy->actor.obj->spheres[1].flags |= 4;
    switch (enemy->actor.anim->current) {
        case 9:
            if (TriggerAtFrame(52.0f, &enemy->actor)) {
                PlaySample(70, &enemy->actor.pos, 100, 128);
            } else if (TriggerAtFrame(59.0f, &enemy->actor)) {
                PlaySample(71, &enemy->actor.pos, 100, 128);
            }
            break;
        case 15:
            if (TriggerAtFrame(11.0f, &enemy->actor)) {
                PlaySample(70, &enemy->actor.pos, 100, 128);
            } else if (TriggerAtFrame(24.0f, &enemy->actor)) {
                PlaySample(71, &enemy->actor.pos, 100, 128);
            }
            break;
        case 0:
            enemy->actor.flags |= 0x200;
            enemy->actor.obj->spheres[1].flags &= ~4;
            if (TriggerAtFrame(144.0f, &enemy->actor) || TriggerAtFrame(156.0f, &enemy->actor)) {
                PlaySample(72, &enemy->actor.pos, 100, 128);
            }
            break;
    }
    if (enemy->actor.anim->current == 3 || enemy->actor.anim->queue[0] == 3) {
        enemy->actor.rotAim.y = Aabs(atan2(enemy->actor.vel.x, enemy->actor.vel.z));
        // Bitwise & keeps both compares; && would branch after the first.
        if (((enemy->actor.anim->time >= 75.0f) & (enemy->actor.anim->time <= 80.0f))
            || ((enemy->actor.anim->time >= 82.0f) & (enemy->actor.anim->time <= 85.0f))
            || ((enemy->actor.anim->time >= 88.0f) & (enemy->actor.anim->time <= 91.0f))) {
            SetVector2D(&tempVect, &enemy->actor.pos);
            AddToOneScaledVector2D(&tempVect, &enemy->actor.vel, 0.9f);
            tempVect.x += Random(5) - 2;
            tempVect.z += Random(5) - 2;
            tempVect.y = enemy->actor.pos.y - enemy->actor.obj->spheres[0].radius + 3.0f;
            vel = enemy->actor.vel;
            SCALE_VECTOR_2D(&vel, 0.8);
            tempVect.x += Random(5) - 2;
            tempVect.z += Random(5) - 2;
            tempVect.y += 4.0f;
            AddDebris(DUST, &tempVect, &enemy->actor.vel, NULL, NULL);
        }
        MakeUnit(&enemy->actor.vel);
        enemy->actor.vel.x *= LENGTH(&enemy->actor.prevVel);
        enemy->actor.vel.y *= LENGTH(&enemy->actor.prevVel);
        enemy->actor.vel.z *= LENGTH(&enemy->actor.prevVel);
    }
    if (enemy->actor.anim->current == 14 && enemy->actor.anim->time >= 104.0f) {
        SetVector2D(&enemy->actor.vel, &zero);
    }
    if (enemy->actor.anim->current == 9 || handControl.attachedTo == &enemy->actor) {
        enemy->actor.flags |= 0x1000000;
    } else {
        enemy->actor.flags &= ~0x1000000;
    }
    if (ballControl.state != BALL_BURST && enemy->brain.attackTime == 0 && !IsHandHoldingBall()
        && IsActorFacingActor(&hand, &enemy->actor, 1.2f) && !IsActorFacingActor(&enemy->actor, &hand, 1.2f)
        && hand.state->mode != HAND_HURT && hand.state->mode != HAND_RELEASE && handControl.attachedTo == NULL
        && DistanceBetweenPoints(&hand.pos, &enemy->actor.pos)
               <= hand.radius + enemy->actor.obj->spheres[0].radius + 4.0f) {
        handControl.status = HAND_HOLDING;
        hand.link = &enemy->actor;
        handControl.dropTimer = 0;
        handControl.releaseTimer = 0;
        handControl.attachedTo = &enemy->actor;
        handControl.attachTag = 2;
        AnimateActor(&enemy->actor, 15, 1, 0, enemy->actor.anim->anims[15].speed * swishSpeed);
        enemy->brain.delayTimer = 30000;
        enemy->brain.cooldownTimer = 30000;
        hand.flags &= ~1;
        ApplyAction(HAND_SWISH);
        cameraInfo.focus = &enemy->actor.pos;
        cameraInfo.focusVel = &enemy->actor.vel;
    }
}

void SpinCannonball(Actor *enemy, f32 rot) {
    enemy->spinAxis.x = 0.0f;
    enemy->spinAxis.y = 1.0f;
    enemy->spinAxis.z = 0.0f;
    if (frameCount & 1) {
        enemy->spinSpeed = rot;
    } else {
        enemy->spinSpeed = -rot;
    }
    enemy->flags &= ~0x20;
}

void NMEProcessCrumpet(Enemy *enemy) {
    Vec3 d;

    if (enemy->brain.instrTime == 1) {
        if (enemy->brain.instr.type == NME_MOVETO) {
            AnimateActor(&enemy->actor, 13, 0, 0, 0.0f);
            AnimateActor(&enemy->actor, 15, 1, 1, 0.0f);
        } else if (enemy->brain.instr.type == NME_FACETO) {
            AnimateActor(&enemy->actor, 5, 1, 0, 0.0f);
        } else if (enemy->brain.instr.type == NME_WAIT && Random(10) < 5) {
            PlaySample(37, &enemy->actor.pos, 100, 128);
        }
    }
    if (enemy->actor.anim->current == 13
        && enemy->actor.anim->time < (enemy->actor.anim->anims[13].start + enemy->actor.anim->anims[13].end) / 2) {
        ZERO_VECTOR(&enemy->actor.vel);
    }
    if (enemy->brain.attackTime == 2) {
        PlaySample(37, &enemy->actor.pos, 100, 128);
    }
    if (enemy->actor.anim->current == 12) {
        if (enemy->brain.instrTime == 1) {
            if (DistanceBetweenPointsSquared(&enemy->actor.pos, &hand.pos)
                < DistanceBetweenPointsSquared(&enemy->actor.pos, &ball.pos)) {
                enemy->brain.scratch = 1;
            } else {
                enemy->brain.scratch = 2;
            }
        }
        if (enemy->brain.scratch == 1) {
            SUB_VECTORS(&d, &enemy->actor.pos, &hand.pos);
        } else {
            SUB_VECTORS(&d, &enemy->actor.pos, &ball.pos);
        }
        enemy->actor.rotAim.y = Aabs(atan2(d.x, d.z) + 3.141592654);
        enemy->actor.rotAim.x = Aabs(atan2(d.y, LENGTH_2D(&d)) - 0.4);
        if (enemy->actor.anim->time > enemy->actor.anim->anims[12].start + 35) {
            if (enemy->samples[1].handle == 0) {
                PlayContinuousSample(&enemy->samples[1], 36, 80, &enemy->actor.pos, 128);
            }
            FireBulletFromActor(&enemy->actor, CRUMPET_BULLET, 0);
        }
    } else {
        if (enemy->samples[1].handle) {
            enemy->samples[1].volume -= 5;
            if (enemy->samples[1].volume <= 0) {
                StopContinuousSample(&enemy->samples[1]);
            }
        }
        enemy->actor.rotAim.x = 0.0f;
    }
}

void NMEProcessCannonball(Enemy *enemy) {
    Quat qAim;
    f32 rot;
    f32 speed = LENGTH_2D(&enemy->actor.vel);

    if (enemy->brain.flags & 8) {
        GiveActorRandomQRotVel(&enemy->actor);
        enemy->brain.spellTimer[0] = 500;
        enemy->actor.spinSpeed = 3.0f;
        enemy->actor.flags &= ~0x20;
    }
    if (enemy->actor.flags & 0x20) {
        if (enemy->brain.instr.type != NME_FOLLOWPLAYER) {
            enemy->actor.flags &= ~0x8000000;
            if (speed < cannonballMinSpeed) {
                vertQ.w = atan2(hand.pos.x - enemy->actor.pos.x, hand.pos.z - enemy->actor.pos.z);
                GetQuaternionFromRotation(&qAim, &vertQ);
                QuatSlerpAtFixedSpeedOld(&enemy->actor.rotQuat, &qAim, 0.8f, &enemy->actor.rotQuat);
                if (!Random(70)) {
                    enemy->actor.spinAxis.x = 0.0f;
                    enemy->actor.spinAxis.z = 0.0f;
                    enemy->actor.spinAxis.y = 1.0f;
                    rot = cannonballRot[0];
                    if (frameCount & 1) {
                        enemy->actor.spinSpeed = rot;
                    } else {
                        enemy->actor.spinSpeed = -rot;
                    }
                    enemy->actor.flags &= ~0x20;
                }
            }
        } else {
            enemy->actor.flags |= 0x8000000;
            if (enemy->brain.instrTime > 20 && !Random(70)) {
                enemy->actor.spinAxis.x = 0.0f;
                enemy->actor.spinAxis.z = 0.0f;
                enemy->actor.spinAxis.y = 1.0f;
                rot = cannonballRot[0];
                if (frameCount & 1) {
                    enemy->actor.spinSpeed = rot;
                } else {
                    enemy->actor.spinSpeed = -rot;
                }
                enemy->brain.spellTimer[0] = 50;
                enemy->actor.flags &= ~0x20;
                enemy->actor.vel.y += 9.0f;
            }
        }
    } else if (speed >= cannonballMinSpeed || ABS(enemy->actor.spinSpeed) <= 0.01) {
        enemy->actor.spinSpeed = 0.0f;
        enemy->actor.flags |= 0x20;
    } else {
        enemy->brain.delayTimer = 2;
    }
}

void NMEProcessSnail(Enemy *enemy) {
    switch (enemy->actor.anim->current) {
        case 9:
        case 15:
            if (DistanceBetweenPointsSquared(&enemy->actor.pos, &hand.pos) <= 3600.0f) {
                AnimateActor(&enemy->actor, 5, 0, 0, 1.3f);
            }
            break;
        case 5:
            enemy->actor.vel.x = enemy->actor.vel.z = 0.0f;
            enemy->actor.rotAim.y = enemy->actor.rot.y;
            if (enemy->actor.anim->finishedAnim && DistanceBetweenPointsSquared(&enemy->actor.pos, &hand.pos) > 3700.0f) {
                AnimateActor(&enemy->actor, 6, 0, 0, 0.0f);
                AnimateActor(&enemy->actor, 15, 1, 1, 0.0f);
            }
            break;
        case 6:
            enemy->actor.vel.x = enemy->actor.vel.z = 0.0f;
            enemy->actor.rotAim.y = enemy->actor.rot.y;
            break;
    }
}

void NMEProcessSamtex(Enemy *enemy) {
    if (enemy->brain.instr.type == NME_JUMP && enemy->brain.instrTime == 1 && enemy->actor.anim->current != 12) {
        PlaySample(104, &enemy->actor.pos, 100, 128);
    }
    if (enemy->brain.attackTime) {
        if (enemy->brain.attackTime == 2) {
            PlayContinuousSample(&enemy->samples[0], 105, 100, &enemy->actor.pos, 128);
        }
        enemy->actor.anim->speed = enemy->actor.anim->anims[enemy->actor.anim->current].speed * 2;
        if (enemy->brain.attackTime >= 100 && enemy->actor.anim->current != 12) {
            AnimateActor(&enemy->actor, 12, 0, 0, 0.0f);
            enemy->brain.delayTimer = 100;
        }
    }
    if (enemy->actor.active != 2 && enemy->actor.anim->current == 12 && enemy->actor.anim->finishedAnim) {
        FlashScreen(255, 200, 50, 255, -20);
        DoExplosion(&enemy->actor.pos, 60.0f, 180.0f, enemy, 30.0f, 0.5f);
        StartEarthquakeAtPoint(&enemy->actor.pos, 14, 2500.0f, 1250.0f, 1, 3);
        PlaySample(106, &enemy->actor.pos, 255, 128);
        KillEnemy(enemy);
    }
}

void NMEProcessSucker(Enemy *enemy) {
    Vec3 tempVect;
    Vec3 tempVect2;
    f32 dist;
    f32 tempFloat;

    if (Random(500) < 5) {
        PlaySample((Random(3) + 94), &enemy->actor.pos, 100, 128);
    }
    if (frameCount % 3 == 0) {
        tempVect.x = enemy->actor.obj->spheres[2].pos.x;
        tempVect.y = enemy->actor.obj->spheres[2].pos.y + 8.0f;
        tempVect.z = enemy->actor.obj->spheres[2].pos.z;
        tempVect2.x = tempVect2.z = 0.0f;
        tempVect2.y = 3.0f;
        AddToOneScaledVector(&tempVect2, &enemy->actor.vel, 0.8f);
        AddDebris(NEW_DUST, &tempVect, &tempVect2, NULL, NULL);
    }
    if (ballControl.enemy == &enemy->actor && enemy->actor.anim->current != 10 && enemy->actor.anim->current != 3) {
        AnimateActor(&enemy->actor, 9, 1, 0, 0);
        enemy->actor.flags |= 0x1000000;
        if (enemy->brain.flags & 8) {
            enemy->actor.rotAim.y = enemy->actor.rot.y;
            AnimateActor(&enemy->actor, 10, 0, 0, 0);
            enemy->brain.delayTimer = 30;
        }
    } else {
        enemy->actor.flags &= ~0x1000000;
        if (enemy->actor.anim->current == 10 && enemy->actor.anim->finishedAnim) {
            DropBall(5);
            tempVect.x = 0.0f;
            tempVect.y = 28.0f;
            tempVect.z = -2.0f;
            RotateVector2D(&ball.vel, &tempVect, enemy->actor.rot.y);
            ball.flags |= 0x1000018;
            AnimateActor(&enemy->actor, 11, 0, 0, 0);
            AnimateActor(&enemy->actor, 15, 1, 1, 0);
            enemy->brain.cooldownTimer = 150;
            PlaySample(93, &enemy->actor.pos, 200, 128);
        }
    }
    if (ballControl.enemy == &enemy->actor) {
        if (ballControl.type != BALL_MODE_CRYSTAL) {
            PlaySample(97, &enemy->actor.pos, 100, 128);
            ActivateCollectable(CRYSTALBALL, -1);
            ball.flags &= ~0x18;
        }
        player.powerups[CRYSTALBALL] = -1;
    }
    if (enemy->actor.anim->current == 12) {
        if (!enemy->samples[0].handle) {
            PlayContinuousSample(&enemy->samples[0], 91, 100, &enemy->actor.pos, 128);
        }
        SCALE_VECTOR_2D(&ball.vel, 0.5);
        SUB_VECTORS(&tempVect, &ball.pos, &enemy->actor.pos);
        enemy->actor.rotAim.y = Aabs(atan2(tempVect.x, tempVect.z));
        ActorHandleRotaim(&enemy->actor, 1);
        ActorHandleRotaim(&enemy->actor, 1);
        SUB_VECTORS(&tempVect, &ball.pos, &enemy->actor.obj->spheres[1].pos);
        dist = LENGTH(&tempVect);
        if (dist != 0.0f) {
            SCALE_VECTOR(&tempVect, 1.0f / dist);
        }
        tempFloat = 2.0 * (f32)enemy->brain.instrTime / 10.0 + 1.0;
        tempFloat = (tempFloat > 3.0f) ? 3.0f : tempFloat;
        SquashActor(&ball, &tempVect, -tempFloat, 1.0f - tempFloat / 6.0f);
        if (enemy->actor.anim->anims[12].start + 10 <= enemy->actor.anim->time) {
            ball.flags &= ~0x18;
            if (ball.link == &hand && !handControl.releaseTimer) {
                ApplyAction(HAND_RELEASE);
            }
            if (dist <= 12.0f) {
                AddToOneScaledVector(&ball.pos, &tempVect, -dist);
                NMEGetNewInstruction(enemy, enemy->brain.instrIndex + 1, 1);
                return;
            }
            AddToOneScaledVector(&ball.pos, &tempVect, -12.0f);
        }
    } else if (enemy->samples[0].handle) {
        enemy->samples[0].volume -= 10;
        if (enemy->samples[0].volume <= 0) {
            StopContinuousSample(&enemy->samples[0]);
        }
    }
}

void NMEProcessCaterpillar(Enemy *enemy) {
    if (enemy->actor.anim->current == 15 && enemy->actor.anim->time < 20.0f) {
        ZERO_VECTOR(&enemy->actor.vel);
    }
    if (enemy->actor.anim->current == 15 && enemy->actor.anim->time > 36.0f) {
        SCALE_VECTOR(&enemy->actor.vel, 0.7);
    }
}

void NMEProcessDroid(Enemy *enemy) {
    NMEBrain *br = &enemy->brain;
    Vec3 tempVect;
    Vec3 tempVect2;
    f32 dist;
    f32 ballDist;

    if ((enemy->actor.flags & 0x4000000) && (ball.link != &hand || handControl.releaseTimer)) {
        AnimateActor(&enemy->actor, 4, 0, 0, 0.0f);
        br->delayTimer = -1;
    }
    if (enemy->actor.anim->current == 5) {
        ballDist = DistanceBetweenPoints(&ball.pos, &enemy->actor.pos);
        dist = DistanceBetweenPoints(&hand.pos, &enemy->actor.pos);
        dist = MIN(ballDist, dist);
        if (dist <= 160.0f) {
            AnimateActor(&enemy->actor, 10, 0, 0, 0.0f);
        }
    } else if (enemy->actor.anim->current == 10 && enemy->actor.anim->finishedAnim) {
        AnimateActor(&enemy->actor, 6, 1, 0, 0.0f);
        NMEGetNewInstruction(enemy, 1, 1);
    }
    if (!(frameCount & 3)) {
        tempVect.x = enemy->actor.prevPos.x;
        tempVect.y = enemy->actor.prevPos.y - 17.0f;
        tempVect.z = enemy->actor.prevPos.z;
        tempVect.x += enemy->actor.vel.x;
        tempVect.y += enemy->actor.vel.y;
        tempVect.z += enemy->actor.vel.z;
        tempVect2.y = -1.0f;
        tempVect2.x = tempVect2.z = 0.0f;
        ADD_TO_VECTOR(&tempVect2, &enemy->actor.vel);
        AddDebris(NEW_DUST, &tempVect, &tempVect2, NULL, NULL);
    }
    if (enemy->actor.anim->current == 4) {
        if (enemy->actor.anim->finishedAnim) {
            AnimateActor(&enemy->actor, 6, 1, 0, 0.0f);
        }
        return;
    }
    dist = DistanceBetweenPoints2D(&hand.pos, &enemy->actor.pos);
    if (handControl.deathTimer == 0 && dist < 100.0f) {
        if (enemy->brain.bobPeriod) {
            enemy->actor.vel.y -= SineWave(enemy->brain.bobPeriod, 0.0f, frameCount) * enemy->brain.bobAmount * 0.5;
        }
        tempVect.y = hand.pos.y + 15.0f;
        if ((*(Actor **)enemy->brain.normalList)->flags & 8) {
            tempVect.x = hand.pos.x;
            tempVect.z = enemy->brain.target.z;
        } else {
            tempVect.z = hand.pos.z;
            tempVect.x = enemy->brain.target.x;
        }
        NMEActionMoveTo(enemy, &tempVect, 10.0f, 0x1006);
        tempVect.x = hand.pos.x - enemy->actor.pos.x;
        tempVect.y = hand.pos.y - enemy->actor.pos.y;
        tempVect.z = hand.pos.z - enemy->actor.pos.z;
        enemy->actor.rotAim.y = Aabs(atan2(tempVect.x, tempVect.z));
        if (dist < 65.0f) {
            if (enemy->actor.anim->current != 11 && enemy->actor.anim->current != 12) {
                AnimateActor(&enemy->actor, 12, 0, 0, 0.0f);
                AnimateActor(&enemy->actor, 11, 1, 1, 0.0f);
            }
        } else if (enemy->actor.anim->current == 11) {
            AnimateActor(&enemy->actor, 3, 0, 0, 0.0f);
            AnimateActor(&enemy->actor, 6, 1, 1, 0.0f);
            NMEGetNewInstruction(enemy, 1, 1);
        }
        if (enemy->actor.anim->current == 11) {
            if (!(frameCount & 3)) {
                PlaySample(87, &enemy->actor.pos, 100, 128);
            }
            FireBulletFromActor(&enemy->actor, DROID_BULLET, 0);
        }
        br->delayTimer = 1;
    } else {
        enemy->actor.rotAim.y = enemy->brain.startRot;
        if (enemy->actor.anim->current == 11) {
            AnimateActor(&enemy->actor, 3, 0, 0, 0.0f);
            AnimateActor(&enemy->actor, 6, 1, 1, 0.0f);
            NMEGetNewInstruction(enemy, 1, 1);
        }
    }
}

void NMEProcessRobes(Enemy *enemy) {
    Vec3 tempVect;
    Vec3 p;
    Bullet *bullet;
    Debris *d;
    f32 dist;
    f32 dot;
    s32 found;
    s32 mode;
    s32 x;
    s32 k;
    Actor *faceB;

    found = FALSE;
    enemy->actor.alpha = SineWave(8.0f, 0, frameCount) * 40.0f + 80.0f;
    if (frameCount == 0) {
        if (gameInfo.level == FEAR1A) {
            ethelLife = 40;
            ethelFreq = 3;
            ethelAlpha = 60;
            ethelAlphaPlus = 10;
        } else {
            ethelLife = 1;
            ethelFreq = 1;
            ethelAlpha = 120;
            ethelAlphaPlus = 30;
        }
    }
    if (frameCount % ethelFreq == 0) {
        d = AddDebris(SMOKETRAIL, &enemy->actor.pos, &zero, NULL, NULL);
        if (d != NULL) {
            d->sprite.r = 180;
            d->sprite.g = 255;
            d->sprite.b = 20;
            d->sprite.anim.def.startAlpha = ethelAlpha + SineWave(10.0f, 0, frameCount) * ethelAlphaPlus;
            d->sprite.anim.def.startScale *= 3;
            d->sprite.anim.def.endAlpha = 0;
            d->sprite.anim.def.frameDelay = 0;
            d->sprite.anim.def.endScale *= 3;
            d->sprite.scaleX *= 3;
            d->sprite.scaleY *= 3;
            d->sprite.anim.def.life = d->sprite.anim.lifeTimer = d->lifeTimer = ethelLife;
            d->sprite.alpha = d->sprite.anim.def.startAlpha;
        }
    }
    if (enemy->brain.instrTime == 1 && enemy->brain.instr.type == NME_WAIT && Random(10) < 5) {
        PlaySample(57, &enemy->actor.pos, 100, 128);
    }
    for (bullet = bulletList.head.next; bullet != &bulletList.head; bullet = bullet->next) {
        if (bullet->target == &enemy->actor && (dist = DistanceBetweenPoints(&bullet->actor.pos, &enemy->actor.pos)) <= 250.0f) {
            SUB_VECTORS(&tempVect, &bullet->actor.pos, &enemy->actor.pos);
            found = TRUE;
            mode = 2;
            dot = DOT_PRODUCT(&tempVect, &bullet->actor.vel);
            if (dist < 150.0f && dot < 0.0f) {
                if (enemy->actor.anim->current != 4) {
                    AnimateActor(&enemy->actor, 4, 0, 0, 0);
                    AnimateActor(&enemy->actor, 5, 1, 1, 0);
                }
            }
            if (enemy->actor.anim->current == 4 && enemy->actor.anim->time >= 345.0f) {
                if (dot > 0.0f) {
                    mode = 1;
                }
                if (enemy->actor.anim->time < 348.0f) {
                    if (dist <= 26.0f) {
                        mode = 0;
                        ZERO_VECTOR(&bullet->actor.vel);
                        bullet->age = -2;
                    } else {
                        enemy->actor.anim->speed = 0.0f;
                    }
                } else if (enemy->actor.anim->time < 351.0f) {
                    AccelerateActorFromQuaternion(&bullet->actor, 1.5f, 100.0f, &enemy->actor.rotQuat, 1);
                    mode = 1;
                } else {
                    AccelerateActorFromQuaternion(&bullet->actor, 8.0f, 100.0f, &enemy->actor.rotQuat, 1);
                    bullet->target = &hand;
                    bullet->actor.lifeTimer = 100;
                    bullet->age = 0;
                    bullet->actor.link = &enemy->actor;
                    if (enemy->brain.attackTime != 0) {
                        NMEEndAttack(enemy);
                        enemy->brain.cooldownTimer = 50;
                    }
                    NMEGetNewInstruction(enemy, 1, 1);
                    enemy->brain.instrTime = 2;
                }
            }
            if (mode == 2) {
                SUB_VECTORS(&tempVect, &bullet->actor.pos, &enemy->actor.pos);
                enemy->actor.rotAim.y = Aabs(atan2(tempVect.x, tempVect.z));
                enemy->actor.phys->turnSpeed *= 6.0f;
                ActorHandleRotaim(&enemy->actor, 1);
                enemy->actor.phys->turnSpeed /= 6.0f;
            } else {
                faceB = &hand;
                SUB_VECTORS(&tempVect, &faceB->pos, &enemy->actor.pos);
                enemy->actor.rotAim.y = Aabs(atan2(tempVect.x, tempVect.z));
                enemy->actor.phys->turnSpeed *= 6.0f;
                ActorHandleRotaim(&enemy->actor, 1);
                enemy->actor.phys->turnSpeed /= 6.0f;
                if (mode == 0) {
                    AddScaledVector2D(&bullet->actor.pos, &bullet->actor.pos, 0.5f, &enemy->actor.pos, 0.5f);
                    bullet->actor.pos.y += MIN(MAX(-6.0f, (enemy->actor.pos.y + 15.0f - bullet->actor.pos.y) / 2.0f), 6.0f);
                    if (IsActorFacingActor(&enemy->actor, &hand, 0.3f)) {
                        enemy->actor.anim->speed = enemy->actor.anim->anims[4].speed;
                    }
                }
            }
            break;
        }
    }
    if (!found && enemy->actor.anim->current == 4) {
        enemy->actor.anim->speed = enemy->actor.anim->anims[4].speed;
    }
    if (enemy->brain.instr.type == NME_WAIT) {
            if (enemy->brain.instrTime == 1) {
                AnimateActor(&enemy->actor, 14, 0, 1, 0);
                AnimateActor(&enemy->actor, 5, 1, 1, 0);
                enemy->actor.anim->loop = 0;
            }
    } else if (enemy->brain.instr.type == NME_RANDOMMOVE) {
            if (enemy->actor.anim->current != 15 && enemy->brain.instrTime == 1) {
                AnimateActor(&enemy->actor, 13, 0, 1, 0);
                AnimateActor(&enemy->actor, 15, 1, 1, 0);
                enemy->actor.anim->loop = 0;
            }
    } else if (enemy->brain.instr.type == NME_SPECIAL && enemy->brain.instr.arg.i == 0) {
                SCALE_VECTOR(&enemy->actor.vel, 0.5);
                if (ballControl.hurtTimer == 0 && ballControl.type != BALL_MODE_CRYSTAL && ballControl.cancerTimer == 0 &&
                    enemy->actor.anim->time >= 181.0f && enemy->actor.anim->time <= 186.0f) {
                    tempVect.x = 0.0f;
                    tempVect.y = 45.0f;
                    tempVect.z = -47.0f;
                    RotateVector2D(&tempVect, &tempVect, enemy->actor.rot.y);
                    p.x = enemy->actor.pos.x - tempVect.x;
                    p.y = enemy->actor.pos.y - tempVect.y;
                    p.z = enemy->actor.pos.z - tempVect.z;
                    if (DistanceBetweenPoints(&p, &ball.pos) <= 2.0f * ball.radius) {
                        for (x = 0; x < 10; x++) {
                            tempVect = ball.pos;
                            tempVect.x += (Random(11) - 5) * 3;
                            tempVect.y += (Random(11) - 5) * 3;
                            tempVect.z += (Random(11) - 5) * 3;
                            d = AddDebris(FAIRY_HALF, &tempVect, &zero, NULL, NULL);
                            if (d != NULL) {
                                k = x % 3 + 3;
                                d->sprite.scaleX *= k;
                                d->sprite.scaleY *= k;
                                d->sprite.anim.def.endScale *= k;
                                d->sprite.anim.def.startScale *= k;
                            }
                        }
                        PlaySample(57, &enemy->actor.pos, 100, 128);
                        PlaySample(59, &enemy->actor.pos, 100, 128);
                        if (gameInfo.level == FEARBONUS) {
                            HurtBall();
                        } else if (Random(100) < 50) {
                            ActivateCollectable(CRYSTALBALL, -1);
                            FlashScreen(0, 200, 0, 240, -30);
                        } else {
                            GiveBallCancer(500);
                            FlashScreen(200, 0, 0, 240, -30);
                        }
                    }
                }
                if (enemy->actor.anim->time <= 186.0f) {
                    SubVector2D(&tempVect, &enemy->actor.pos, &ball.pos);
                    MakeUnit2D(&tempVect);
                    SCALE_VECTOR_2D(&tempVect, 47.0f);
                    AddToVector2D(&tempVect, &ball.pos);
                    tempVect.y = ball.pos.y + 45.0f;
                    p.x = enemy->actor.pos.x - tempVect.x;
                    p.y = enemy->actor.pos.y - tempVect.y;
                    p.z = enemy->actor.pos.z - tempVect.z;
                    dist = LENGTH(&p);
                    if (dist > robesMoveSpeed) {
                        SCALE_VECTOR(&p, robesMoveSpeed / dist);
                        SUB_FROM_VECTOR(&enemy->actor.pos, &p);
                    } else {
                        enemy->actor.pos = tempVect;
                    }
                    SUB_VECTORS(&tempVect, &ball.pos, &enemy->actor.pos);
                    enemy->actor.rotAim.y = Aabs(atan2(tempVect.x, tempVect.z));
                }
    } else {
            if (enemy->actor.anim->current == 1) {
                if (enemy->brain.instrTime == 1) {
                    PlaySample(58, &enemy->actor.pos, 100, 128);
                }
                SUB_VECTORS(&tempVect, &hand.pos, &enemy->actor.pos);
                enemy->actor.rotAim.y = Aabs(atan2(tempVect.x, tempVect.z));
                if (enemy->brain.instrTime == 24) {
                    FireBulletFromActor(&enemy->actor, ETHEL_BULLET, 0);
                    PlaySample(59, &enemy->actor.pos, 100, 128);
                }
            }
    }
}

void NMEProcessRaptor(Enemy *enemy) {
    Vec3 tempVect;

    switch (enemy->actor.anim->current) {
        case 15:
            if (TriggerAtFrame(1.0f, &enemy->actor)) {
                PlaySample(46, &enemy->actor.pos, 10, 128);
            } else if (TriggerAtFrame(14.0f, &enemy->actor)) {
                PlaySample(47, &enemy->actor.pos, 10, 128);
            }
            break;
        case 9:
            if (TriggerAtFrame(28.0f, &enemy->actor) || TriggerAtFrame(36.0f, &enemy->actor)) {
                PlaySample(46, &enemy->actor.pos, 10, 128);
            } else if (TriggerAtFrame(32.0f, &enemy->actor) || TriggerAtFrame(40.0f, &enemy->actor)) {
                PlaySample(47, &enemy->actor.pos, 10, 128);
            }
            break;
        case 5:
            if (TriggerAtFrame(84.0f, &enemy->actor)) {
                PlaySample(44, &enemy->actor.pos, 255, 128);
            }
            break;
        case 12:
            if (TriggerAtFrame(56.0f, &enemy->actor)) {
                PlaySample(45, &enemy->actor.pos, 100, 128);
            }
            break;
    }
    if (enemy->actor.anim->current == 9) {
        enemy->actor.flags |= 0x1000000;
    } else {
        enemy->actor.flags &= ~0x1000000;
    }
    if (enemy->actor.anim->current == 12) {
        if (enemy->actor.anim->time > 48.0f) {
            tempVect.x = enemy->actor.pos.x + Random(5) - 2.0f;
            tempVect.y = enemy->actor.pos.y - enemy->actor.radius + 3.0f;
            tempVect.z = enemy->actor.pos.z + Random(5) - 2.0f;
            AddDebris(DUST, &tempVect, &zero, NULL, NULL);
        }
        enemy->actor.friction = 0.96f;
    } else {
        enemy->actor.friction = enemy->actor.phys->friction;
    }
    if (enemy->actor.anim->current == 5 && enemy->actor.anim->time > 83.0f && enemy->actor.anim->time < 90.0f
        && earthQuake.time == 0) {
        StartEarthquakeAtPoint(&enemy->actor.pos, 50, 700.0f, 350.0f, 1, 2);
    }
}

void NMEProcessChuck(Enemy *enemy) {
    Vec3 dest;
    Vec3 d;
    f32 dist;
    s32 caught;

    if (ballControl.enemy == &enemy->actor) {
        if (!enemy->samples[0].handle) {
            PlayContinuousSample(&enemy->samples[0], 23, 100, &enemy->actor.pos, 128);
        }
        enemy->actor.flags |= 0x1000000;
        if (ballControl.type == BALL_MODE_BOWLING) {
            SCALE_VECTOR_2D(&enemy->actor.vel, 0.8);
        }
    } else {
        if (enemy->samples[0].handle) {
            enemy->samples[0].volume -= 5;
            if (enemy->samples[0].volume <= 0) {
                StopContinuousSample(&enemy->samples[0]);
            }
        }
        enemy->actor.flags &= ~0x1000000;
    }
    if (enemy->actor.anim->current == 7 && (enemy->brain.instr.flags & 0x80)) {
        if (!IsHandHoldingBall()) {
            SCALE_VECTOR_2D(&ball.vel, 0.7);
        }
        dest = ball.pos;
        dest.y += ball.obj->spheres[0].radius + enemy->actor.radius;
        SUB_VECTORS(&d, &dest, &enemy->actor.pos);
        caught = FALSE;
        dist = LENGTH(&d);
        if (dist <= 4.0f) {
            caught = TRUE;
        } else {
            AddToOneScaledVector(&enemy->actor.pos, &d, 4.0f / dist);
            SUB_VECTORS(&d, &dest, &enemy->actor.pos);
            if (LENGTH(&d) <= 4.0f) {
                caught = TRUE;
            }
        }
        if (caught) {
            enemy->actor.pos = dest;
            NMEActionHoldPlayer(&enemy->actor, (Vec3 *)&enemy->brain.instr.params, enemy->brain.instr.arg.i,
                                enemy->brain.instr.flags);
            ball.flags |= 1;
            AnimateActor(&enemy->actor, 9, 1, 1, 0);
            enemy->actor.vel = ball.vel;
            SUB_VECTORS(&ballControl.enemyOffset, &ball.pos, &enemy->actor.pos);
            NMEGetNewInstruction(enemy, enemy->brain.instrIndex + 1, 1);
        }
    } else if (enemy->brain.instr.type != NME_JUMP && enemy->brain.instr.type != NME_FOLLOWPLAYER && enemy->actor.anim->current != 10
               && enemy->actor.anim->current != 8 && enemy->brain.scratch == 0
               && IsActorFacingActor(&enemy->actor, &hand, 0.6f)
               && DistanceBetweenPoints2D(&enemy->actor.pos, &hand.pos) <= 150.0f) {
        PlaySample(21, &enemy->actor.pos, 100, 128);
        FireBulletFromActor(&enemy->actor, CHUCKIE_EGG, 0);
        enemy->brain.scratch = 50;
    } else if (enemy->brain.scratch) {
        enemy->brain.scratch--;
    }
    if (enemy->brain.instr.type == NME_RANDOMMOVE && ball.link == &enemy->actor) {
        enemy->actor.rotAim.y = Aabs(atan2(enemy->actor.vel.x, enemy->actor.vel.z) + PI);
    }
    if ((enemy->brain.flags & 8) || (ballControl.hitFlags & 8) || ball.shadow->altitude == -1.0f
        || ball.radius + 50.0f < ball.shadow->altitude) {
        if (ballControl.enemy == &enemy->actor) {
            PlaySample(124, &enemy->actor.pos, 100, 135);
            DropBall(5);
            NMEEndAttack(enemy);
            AnimateActor(&enemy->actor, 15, 1, 1, 0);
            NMEGetNewInstruction(enemy, enemy->brain.instrIndex, 0);
            enemy->actor.vel.y += 8.0f;
            AccelerateActor2D(&enemy->actor, -3.0f, 20.0f);
            enemy->brain.cooldownTimer = 70;
            enemy->brain.scratch = 70;
            return;
        }
        if (enemy->actor.obj->onGround && (enemy->brain.flags & 8)) {
            PlaySample(109, &enemy->actor.pos, 100, 135);
            enemy->actor.vel.y = 5.0f;
        }
    }
}

void NMEProcessFrog(Enemy *enemy) {
    Vec3 d;
    Vec3 centre;

    if (enemy->brain.instr.type == NME_JUMP && enemy->brain.instrTime == 2) {
        PlaySample(104, &enemy->actor.pos, 50, 140);
    }
    if (enemy->actor.obj->onGround) {
        if (enemy->actor.anim->current != 7) {
            if (TriggerAtFrame(35.0f, &enemy->actor)) {
                PlaySample(218, &enemy->actor.pos, 50, 128);
            }
            SCALE_VECTOR_2D(&enemy->actor.vel, 0.6);
        }
        if (!enemy->brain.cooldownTimer && !IsPointWithinBox2D(&enemy->brain.area, &enemy->actor.pos)) {
            centre.x = enemy->brain.area.pos.x + enemy->brain.area.size.x / 2.0f;
            centre.z = enemy->brain.area.pos.z + enemy->brain.area.size.z / 2.0f;
            SubVector2D(&d, &centre, &enemy->actor.pos);
            enemy->actor.rotAim.y = Aabs(atan2(d.x, d.z));
            NMEGetNewInstruction(enemy, 1, 0);
            enemy->brain.cooldownTimer = 80;
        }
    }
}

void NMEProcessBovva(Enemy *enemy) {
    Vec3 d;

    if (enemy->brain.scratch == 1 && !enemy->brain.delayTimer) {
        if (enemy->actor.flags & 0x4000000) {
            enemy->brain.scratch = 20;
            AddToOneScaledVector(&ball.vel, &enemy->actor.vel, 3.0f);
        }
        if (!enemy->brain.cooldownTimer) {
            enemy->brain.cooldownTimer = 100;
            enemy->brain.scratch = 0;
        }
        NMEActionMoveTo(enemy, &ball.pos, 0.0f, 0x2001);
    } else if (enemy->brain.scratch >= 2) {
        enemy->brain.scratch--;
    }
    if (!enemy->brain.attackTime && !enemy->brain.cooldownTimer &&
        DistanceBetweenPoints(&enemy->actor.pos, &ball.pos) <= 130.0f) {
        enemy->brain.cooldownTimer = 150;
        enemy->brain.scratch = 1;
    }
    if (enemy->actor.anim->current == 12) {
        SUB_VECTORS(&d, &enemy->actor.pos, &hand.pos);
        enemy->actor.rotAim.y = Aabs(atan2(d.x, d.z));
        enemy->actor.rotAim.x = Aabs(atan2(-d.y, LENGTH_2D(&d)));
        enemy->actor.rotAim.z = 0.0f;
        ActorHandleRotaim(&enemy->actor, 1);
        ActorHandleRotaim(&enemy->actor, 0);
    }
    if (enemy->actor.anim->current == 3 && enemy->brain.instrTime == 1) {
        PlaySample(102, &enemy->actor.pos, 100, 128);
        FireBulletFromActor(&enemy->actor, BOVVA_BULLET, 0);
        enemy->actor.rotAim.x = 0.0f;
    }
}

void NMEProcessTracey(Enemy *enemy) {
    Vec3 tempVect;
    Vec3 tempVel;

    if (enemy->actor.anim->current == 11) {
        NMEActionMoveTo2D(enemy, &hand.pos, 0.0f, 1);
    }
    if (enemy->actor.anim->current == 5) {
        if (enemy->samples[0].handle == 0) {
            PlayContinuousSample(&enemy->samples[0], 38, 100, &enemy->actor.pos, 128);
        }
        if (TriggerAtFrame(15.0f, &enemy->actor) || TriggerAtFrame(47.0f, &enemy->actor)) {
            PlaySample(40, &enemy->actor.pos, 50, 128);
        } else if (TriggerAtFrame(31.0f, &enemy->actor) || TriggerAtFrame(63.0f, &enemy->actor)) {
            PlaySample(41, &enemy->actor.pos, 50, 128);
        }
    } else if (enemy->samples[0].handle) {
        enemy->samples[0].volume -= 5;
        if (enemy->samples[0].volume <= 0) {
            StopContinuousSample(&enemy->samples[0]);
        }
    }
    if (enemy->actor.anim->current == 9) {
        if (TriggerAtFrame(122.0f, &enemy->actor)) {
            PlaySample(40, &enemy->actor.pos, 100, 128);
        } else if (TriggerAtFrame(128.0f, &enemy->actor)) {
            PlaySample(41, &enemy->actor.pos, 100, 128);
        }
    }
    if (enemy->actor.anim->current == 10) {
        if (TriggerAtFrame(94.0f, &enemy->actor) || TriggerAtFrame(105.0f, &enemy->actor)) {
            PlaySample(39, &enemy->actor.pos, 100, 128);
        }
        if (frameCount & 1) {
            SetVector2D(&tempVect, &enemy->actor.pos);
            tempVect.x += Random(11) - 5;
            tempVect.z += Random(11) - 5;
            tempVect.y = enemy->actor.pos.y + 25.0f;
            tempVel.x = Random(5) - 2;
            tempVel.z = Random(5) - 2;
            tempVel.y = 4.0f;
            AddDebris(HEART, &tempVect, &tempVel, NULL, NULL);
        }
    }
    if (enemy->brain.instr.type == NME_JUMP) {
        if (enemy->brain.instrTime == 2) {
            PlaySample(125, &enemy->actor.pos, 100, 128);
        }
        enemy->actor.flags |= 0x80080000;
        if (enemy->actor.obj->onGround && !enemy->actor.obj->wasOnGround) {
            PlaySample(42, &enemy->actor.pos, 200, 128);
            StartEarthquakeAtPoint(&enemy->actor.pos, 6, 2000.0f, 1000.0f, 1, 2);
            CreateBigDustCircle(&enemy->actor.pos, -20.0f);
            CreateEnemyBlastring(&enemy->actor.pos, 20, -28.5f, 0.3f);
        }
    } else {
        enemy->actor.flags &= ~0x80000;
    }
}

void NMEProcessCylon(Enemy *enemy) {
    NMEBrain *br = &enemy->brain;
    Actor *actor = &enemy->actor;
    NMEInstruction *ins = &br->instr;
    s32 radius;
    f64 angle;

    if (enemy->brain.instr.type != NME_FOLLOWPLAYER) {
        enemy->actor.rotAim.z = 0.0f;
    }
    if (enemy->brain.attackTime == 2) {
        PlaySample(88, &enemy->actor.pos, 100, 128);
    }
    if (enemy->actor.anim->current == 15 || enemy->actor.anim->current == 13 || enemy->actor.anim->current == 9) {
        radius = 120;
        if (enemy->actor.anim->current == 9) {
            radius = 150;
        }
        if (enemy->samples[0].handle == 0) {
            PlayContinuousSample(&enemy->samples[0], 89, 100, &enemy->actor.pos, radius);
            PlayContinuousSample(&enemy->samples[1], 90, 100, &enemy->actor.pos, radius);
        }
    } else {
        StopContinuousSample(&enemy->samples[0]);
        StopContinuousSample(&enemy->samples[1]);
    }
    if (ins->type == NME_FOLLOWPLAYER && br->instrTime == 1) {
        AnimateActor(actor, 9, 1, 0, 0);
    }
    if (ins->type == NME_FACETO) {
        if (br->instrTime == 1) {
            angle = FindShortestAngleSigned(actor->rotAim.y, actor->rot.y) - 1.570796327;
            if (ABS(angle) < 0.4) {
                AnimateActor(actor, 10, 0, 0, 0);
            }
        }
        if (actor->anim->current == 10) {
            if (!actor->anim->finishedAnim) {
                actor->rotAim.y = actor->rot.y;
            } else {
                actor->rot.y = actor->rotAim.y;
                GetQuaternionFromXYZRot(&actor->rotQuat, &actor->rot);
                AnimateActor(actor, 5, 1, 0, 0);
                UpdateAnimations(actor);
            }
        }
    }
    if ((enemy->actor.flags & 0x4000000) && enemy->brain.attackTime == 0) {
        enemy->brain.scratch = 70;
        enemy->brain.delayTimer = 70;
        enemy->actor.rotAim.x = 4.712389f;
        enemy->brain.flags |= 1;
        enemy->actor.anim->speed = 0.0f;
    }
    if (enemy->brain.scratch > 0) {
        enemy->brain.scratch--;
        if (enemy->brain.scratch == 10) {
            enemy->actor.rotAim.x = 0.0f;
            enemy->brain.flags &= ~1;
            enemy->actor.anim->speed = enemy->actor.anim->anims[enemy->actor.anim->current].speed;
        }
    }
}

void NMEProcessFumble(Enemy *enemy) {
    if (enemy->actor.anim->current == 6) {
        enemy->brain.flags |= 1;
    } else {
        enemy->brain.flags &= ~1;
    }
    switch (enemy->actor.anim->current) {
        case 15:
            if (TriggerAtFrame(69.0f, &enemy->actor) || TriggerAtFrame(84.0f, &enemy->actor)) {
                PlaySample(56, &enemy->actor.pos, 100, 128);
            }
            break;
        case 9:
            if (TriggerAtFrame(119.0f, &enemy->actor) || TriggerAtFrame(123.0f, &enemy->actor)) {
                PlaySample(56, &enemy->actor.pos, 100, 128);
            }
            break;
        case 10:
            if (TriggerAtFrame(87.0f, &enemy->actor)) {
                PlaySample(127, &enemy->actor.pos, 100, 128);
            }
            break;
        case 4:
            if (TriggerAtFrame(136.0f, &enemy->actor)) {
                PlaySample(128, &enemy->actor.pos, 100, 128);
            }
            break;
    }
    if (enemy->actor.anim->current == 5) {
        if (DistanceBetweenPoints(&hand.pos, &enemy->actor.pos) <= 100.0f
            || DistanceBetweenPoints(&ball.pos, &enemy->actor.pos) <= 100.0f) {
            enemy->brain.instrTimer = 1;
        }
    } else if (enemy->actor.anim->current == 15) {
        enemy->brain.scratch = 0;
        enemy->brain.cooldownTimer = 0;
    }
    if (enemy->actor.anim->current == 9) {
        enemy->actor.flags |= 0x1000000;
    } else {
        enemy->actor.flags &= ~0x1000000;
    }
    if ((enemy->brain.flags & 8) && enemy->brain.scratch == 0) {
        enemy->actor.rotAim.y = enemy->actor.rot.y;
        AccelerateActor(&enemy->actor, -3.0f, 20.0f);
        enemy->brain.scratch = 1;
        enemy->brain.cooldownTimer = 2000;
        if (enemy->brain.attackTime) {
            enemy->brain.attackTime = 0;
        }
        if (ballControl.enemy == &enemy->actor) {
            EnemyDropBall();
            ball.vel.y = 9.0f;
            AccelerateActorFromActor(&enemy->actor, &ball, 5.0f, 30.0f);
        }
        enemy->brain.curList = enemy->brain.normalList;
        NMEGetNewInstruction(enemy, 3, 1);
    }
}

void NMEProcessSelwynWhale(Enemy *enemy) {
    f32 step = PI * 2 / 5;
    Vec3 tempVect;
    Vec3 tempVel;
    Debris *debris;
    Bullet *bullet;
    f32 angle;
    s32 x;
    s8 hits;

    if (enemy->actor.state->hitFadeTimer != 0) {
        enemy->actor.state->hitFadeTimer--;
    }
    if (enemy->actor.obj->onGround && enemy->actor.shadow->altitude <= enemy->actor.radius + 4.0f) {
        enemy->actor.flags &= ~0x80000;
    } else {
        enemy->actor.flags |= 0x80000;
    }
    if (enemy->actor.anim->current == 15) {
        if (TriggerAtFrame(33.0f, &enemy->actor)) {
            PlaySample(14, &enemy->actor.pos, 100, 128);
        } else if (TriggerAtFrame(43.0f, &enemy->actor)) {
            PlaySample(15, &enemy->actor.pos, 100, 128);
        }
    }
    if (enemy->actor.state->waterState) {
        tempVect.x = 0.0f;
        tempVect.z = -55.0f;
        RotateVector2D(&tempVect, &tempVect, enemy->actor.rot.y);
        AddToVector2D(&tempVect, &enemy->actor.pos);
        tempVect.y = enemy->actor.pos.y;
        CreateWake(&tempVect, &enemy->actor.vel, NULL, -7.5f, 5.0f, (Vec3 *)&vertQ);
        if ((enemy->actor.rotVel.y != 0.0f && frameCount % 3 == 0) || frameCount % 10 == 0) {
            tempVel.x = Random(5) - 2;
            tempVel.y = Random(9) + 4;
            tempVel.z = Random(5) - 2;
            tempVect.y -= 15.0f;
            debris = AddDebris(SPLASH, &tempVect, &tempVel, NULL, NULL);
            if (debris != NULL && Random(10) < 5) {
                debris->sprite.flags |= 2;
            }
        }
    }
    if (enemy->brain.scratch != 2 && ABS(enemy->actor.rotVel.y) < 0.05) {
        enemy->brain.delayTimer = 0;
        enemy->actor.phys->rotAim[1] = 1;
        enemy->actor.rotVel.y = 0.0f;
    }
    if (enemy->actor.anim->current == 12) {
        if (enemy->actor.anim->time >= 135.0f && enemy->actor.anim->time <= 146.0f) {
            if (!enemy->samples[0].handle) {
                PlayContinuousSample(&enemy->samples[0], 12, 100, &enemy->actor.pos, 128);
            }
            bullet = FireBulletFromActor(&enemy->actor, WATER_BULLET, 0);
            if (bullet != NULL) {
                bullet->sprites[0].anim.lifeTimer = 11;
                bullet->sprites[0].anim.def.life = 11;
                bullet->actor.flags |= 8;
                InitActorStructures(&bullet->actor, 2);
                bullet->actor.obj->spheres[0].radius = bullet->actor.obj->spheres[0].targetRadius = bullet->actor.radius =
                    standardBullets[WATER_BULLET].radius;
            }
        }
    } else if (enemy->samples[0].handle) {
        enemy->samples[0].volume -= 5;
        if (enemy->samples[0].volume <= 0) {
            StopContinuousSample(&enemy->samples[0]);
        }
    }
    if (enemy->actor.anim->current == 8 && enemy->actor.obj->onGround && !enemy->actor.obj->wasOnGround) {
        PlaySample(13, &enemy->actor.pos, 255, 128);
        CreateEnemyBlastring(&enemy->actor.pos, 21, -enemy->actor.radius / 1.1, -0.7f);
        StartEarthquakeAtPoint(&enemy->actor.pos, 5, 5000.0f, 2500.0f, 1, 3);
        angle = Random(6);
        for (x = 0; x < 5; x++) {
            tempVect.x = 25.0f;
            tempVect.z = 0.0f;
            RotateVector2D(&tempVect, &tempVect, angle);
            tempVect.y = -20.0f;
            SetVector2D(&tempVel, &tempVect);
            tempVel.x *= 0.2;
            tempVel.y = 0.0f;
            tempVel.z *= 0.2;
            tempVect.x += enemy->actor.pos.x;
            tempVect.y += enemy->actor.pos.y;
            angle += step;
            tempVect.z += enemy->actor.pos.z;
            debris = AddDebris(DUST, &tempVect, &tempVel, NULL, NULL);
            if (debris != NULL) {
                if (x & 1) {
                    debris->sprite.flags |= 2;
                }
                debris->sprite.scaleX *= 2;
                debris->sprite.scaleY *= 2;
                debris->sprite.anim.def.endScale *= 2;
                debris->sprite.anim.def.startScale *= 2;
            }
        }
    }
    if ((enemy->actor.flags & 0x4000000) && enemy->brain.delayTimer == 0 && ball.obj->onGround == 0
        && handControl.deathTimer == 0 && (ball.link == NULL || handControl.releaseTimer)) {
        enemy->actor.state->hitFadeTimer = 20;
        PlaySample(117, &enemy->actor.pos, 100, 70);
        hits = --enemy->actor.state->hits;
        if (hits == 2) {
            enemy->actor.flags &= ~0x19;
            SubVector2D(&tempVect, &ball.pos, &enemy->actor.pos);
            MakeUnit2D(&tempVect);
            AddToOneScaledVector2D(&ball.pos, &tempVect, 2.0f);
            AddToOneScaledVector2D(&ball.vel, &tempVect, 2.0f);
            AddToOneScaledVector2D(&enemy->actor.vel, &tempVect, -15.0f);
            enemy->actor.flags &= ~1;
            enemy->actor.phys->friction = 1.0f;
            enemy->actor.friction = 1.0f;
            enemy->brain.scratch = hits;
            enemy->actor.phys->turnSpeed = 0.45f;
            enemy->brain.delayTimer = 10000;
            enemy->brain.cooldownTimer = 10000;
            NMEGetNewInstruction(&enemies[1], 1, 1);
            enemies[1].brain.scratch = 1;
            enemies[1].actor.flags |= 0x111;
            enemies[1].actor.state->flying = 0;
            NMEGetNewInstruction(&enemies[2], 1, 1);
            enemies[2].brain.scratch = 1;
            enemies[2].brain.cooldownTimer = 150;
            enemies[2].actor.flags |= 0x110;
        } else {
            enemy->brain.delayTimer = 10000;
            if (frameCount & 1) {
                enemy->actor.rotVel.y = 0.6f;
            } else {
                enemy->actor.rotVel.y = -0.6f;
            }
            enemy->actor.phys->rotAim[1] = 0;
        }
    }
    if (enemy->brain.scratch == 2) {
        enemy->actor.rotAim.y = enemy->actor.rot.y = Aabs(enemy->actor.rot.y + 0.45);
        enemy->actor.vel.y = 0.0f;
        if (LENGTH_2D(&enemy->actor.pos) > 470.0f) {
            PlaySample(13, &enemy->actor.pos, 255, 128);
            StartEarthquake(6, 1000, 500, 1, 2);
            enemy->actor.phys->friction = 0.9f;
            enemy->actor.phys->turnSpeed = 0.07f;
            KillEnemy(enemy);
        }
    }
}

void NMEProcessSelwynCrab(Enemy *enemy) {
    Vec3 v;
    Vec3 tempVect2;
    f32 dist;

    v.x = enemy->actor.pos.x;
    v.z = enemy->actor.pos.z;
    v.y = hand.pos.y;
    SUB_VECTORS(&tempVect2, &hand.pos, &v);
    MakeUnit(&tempVect2);
    SCALE_VECTOR_2D(&tempVect2, cameraInfo.targetDistance);
    AddVector2D(&v, &hand.pos, &tempVect2);
    v.y += 70.0f;
    if (LENGTH_2D(&v) < 500.0f) {
        cameraInfo.destActive = 1;
        cameraInfo.destPos = v;
    }
    if (enemy->brain.scratch == 0) {
        enemy->actor.anim->speed = enemies[0].actor.anim->speed;
        if (TriggerAtFrame(14.0f, &enemy->actor) || TriggerAtFrame(31.0f, &enemy->actor)
            || TriggerAtFrame(41.0f, &enemy->actor) || TriggerAtFrame(48.0f, &enemy->actor)
            || TriggerAtFrame(58.0f, &enemy->actor) || TriggerAtFrame(68.0f, &enemy->actor)
            || TriggerAtFrame(80.0f, &enemy->actor) || TriggerAtFrame(90.0f, &enemy->actor)
            || TriggerAtFrame(100.0f, &enemy->actor)) {
            PlaySample(16, &enemy->actor.pos, 100, 128);
        }
        enemy->actor.pos = enemies[0].actor.obj->spheres[1].pos;
        enemy->actor.rotAim.y = enemy->actor.rot.y = enemies[0].actor.rot.y;
        enemy->actor.anim->speed = 0.0f;
        enemy->actor.anim->time = enemies[0].actor.anim->time;
        return;
    }
    if (enemy->actor.anim->current == 15
        && (TriggerAtFrame(283.0f, &enemy->actor) || TriggerAtFrame(298.0f, &enemy->actor)
            || TriggerAtFrame(311.0f, &enemy->actor) || TriggerAtFrame(326.0f, &enemy->actor))) {
        PlaySample(16, &enemy->actor.pos, 100, 128);
    }
    if (enemy->brain.scratch >= 10) {
        enemy->actor.phys->turnSpeed *= 4.0f;
        ActorHandleRotaim(&enemy->actor, 2);
        enemy->actor.phys->turnSpeed /= 4.0f;
        enemy->brain.cooldownTimer = 2;
        if (--enemy->brain.scratch < 31) {
            enemy->actor.rotAim.z = 0.0f;
            enemy->actor.rotAim.x = 0.0f;
            if (enemy->brain.scratch == 30) {
                enemy->actor.vel.y = 10.0f;
                enemy->actor.obj->spheres[0].targetRadius -= 6.0f;
            }
            if (enemy->brain.scratch == 9) {
                enemy->brain.scratch = 1;
            }
        } else if ((enemy->brain.flags & 8)
                   || ((enemy->actor.flags & 0x4000000) && ballControl.type == BALL_MODE_BOWLING && ball.speed > 7.0f)) {
            KillEnemy(enemy);
            levelCamHeight = 300;
            NMEGetNewInstruction(&enemies[2], 0, 1);
            enemies[2].brain.scratch = 2;
            enemies[2].brain.attackTime = 0;
            enemies[2].brain.cooldownTimer = 0;
        }
    } else {
        enemy->actor.rot.x = 0.0f;
        if ((enemy->brain.flags & 8)
            || ((enemy->actor.flags & 0x4000000) && ballControl.type == BALL_MODE_BOWLING && ball.speed > 7.0f)) {
            if ((handControl.flags & 1) || enemy->actor.anim->current != 15) {
                ApplyAction(HAND_SLAM2ST);
            } else {
                enemy->brain.scratch = 250;
                ApplyAction(HAND_SLAM2ST);
                enemy->actor.rotAim.z = 3.1415927f;
                enemy->actor.rotAim.x = 0.4f;
                enemy->actor.vel.y = 10.0f;
                enemy->actor.obj->spheres[0].targetRadius += 6.0f;
                SCALE_VECTOR_2D(&hand.vel, 2.0f);
                hand.friction = handInertia;
                hand.flags |= 0x1002000;
            }
        } else {
            if (enemy->brain.attackTime == 0) {
                if (enemy->actor.obj->onGround && enemy->actor.anim->current == 15 && ballControl.state != BALL_BURST) {
                    if (DistanceBetweenPoints(&enemy->actor.pos, &hand.pos) < 100.0f
                        && IsActorFacingActor(&enemy->actor, &hand, 1.0f)) {
                        SUB_VECTORS(&v, &hand.pos, &enemy->actor.pos);
                        enemy->actor.rotAim.y = Aabs(atan2(v.x, v.z));
                    } else {
                        SUB_VECTORS(&v, &ball.pos, &enemy->actor.pos);
                        dist = DistanceBetweenPoints2D(&ball.pos, &enemy->actor.pos);
                        if (dist > 200.0f) {
                            enemy->actor.rotAim.y = Aabs(atan2(v.x, v.z) + 4.712388981);
                        } else if (dist < 100.0f) {
                            enemy->actor.rotAim.y = Aabs(atan2(v.x, v.z) + 1.570796327);
                        } else {
                            enemy->actor.rotAim.y = Aabs(atan2(v.x, v.z));
                        }
                    }
                    v.z = enemy->actor.phys->accel;
                    v.x = 0.0f;
                    RotateVector2D(&v, &v, enemy->actor.rot.y + 1.570796327);
                    AddToVector2D(&enemy->actor.vel, &v);
                }
            } else {
                SCALE_VECTOR_2D(&enemy->actor.vel, 0.7);
            }
            if (enemy->actor.anim->current == 7) {
                if (enemy->actor.anim->time < enemy->actor.anim->speed + 377.0f) {
                    SUB_VECTORS(&v, &enemy->actor.pos, &ball.pos);
                    enemy->actor.rot.x = Aabs(atan2(v.y, LENGTH_2D(&v)) - 0.12);
                    GetQuaternionFromXYZRot(&enemy->actor.rotQuat, &enemy->actor.rot);
                }
                if (enemy->actor.anim->time >= 358.0f && enemy->actor.anim->time <= 361.0f) {
                    enemy->actor.vel.y = 0.0f;
                }
                if (enemy->brain.cooldownTimer == 0 && enemy->actor.anim->time >= 358.0f
                    && enemy->actor.anim->time < enemy->actor.anim->speed + 358.0f) {
                    FireBulletFromActor(&enemy->actor, CLAW_BULLET, 0);
                    enemy->brain.cooldownTimer = 10;
                }
            }
        }
        ActorMaxOutVel2D(&enemy->actor, enemy->actor.phys->moveSpeed);
    }
}

void NMEProcessSelwynFish(Enemy *enemy) {
    Vec3 tempVect;

    if (enemy->brain.scratch == 0) {
        enemy->actor.pos = enemies[0].actor.obj->spheres[2].pos;
        enemy->actor.pos.y -= 8.0f;
        enemy->actor.rotAim.y = enemy->actor.rot.y = enemies[0].actor.rot.y;
        enemy->actor.anim->speed = enemies[0].actor.anim->speed;
        enemy->actor.anim->time = enemies[0].actor.anim->time;
        if (enemy->actor.anim->time >= 137.0f && enemy->actor.anim->time < enemies[0].actor.anim->speed + 137.0f) {
            FireBulletFromActor(&enemy->actor, SMALL_BUBBLE_BULLET, 0);
        }
        return;
    }
    if (LENGTH_2D(&enemy->actor.pos) > 450.0f) {
        MakeUnit2D(&enemy->actor.pos);
        SCALE_VECTOR_2D(&enemy->actor.pos, 450.0f);
    }
    if (enemy->actor.anim->current == 15 && TriggerAtFrame(179.0f, &enemy->actor)) {
        PlaySample(17, &enemy->actor.pos, 100, 128);
    }
    if (enemy->actor.anim->finishedAnim == 13) {
        if (numBubbles != 0 && (numBubbles >= 3 || Random(10) < 7)) {
            if (numSmallBubbles < 5) {
                FireBulletFromActor(&enemy->actor, SMALL_BUBBLE_BULLET, 0);
            }
        } else {
            FireBulletFromActor(&enemy->actor, BUBBLE_BULLET, 0);
        }
        AnimateActor(&enemy->actor, 15, 1, 1, 0);
    }
    if (enemy->brain.scratch == 1) {
        enemy->actor.phys->accel = 0.05f;
        enemy->actor.phys->turnSpeed = 0.05f;
        if (enemy->brain.cooldownTimer == 0
            && (enemy->brain.scratch == 0 || IsActorFacingActor(&enemy->actor, &hand, 0.4f))) {
            enemy->brain.delayTimer = 30;
            enemy->brain.cooldownTimer = 150;
            enemy->actor.anim->loop = 0;
            FlushQueue(&enemy->actor);
            AnimateActor(&enemy->actor, 12, 0, 1, 0);
            AnimateActor(&enemy->actor, 3, 0, 1, 0);
        }
    } else {
        enemy->actor.phys->accel = 1.0f;
        enemy->actor.phys->turnSpeed = 0.1f;
        if (enemy->brain.attackTime != 0) {
            if (enemy->actor.flags & 0x1000000) {
                if (enemy->actor.obj->planes[0] != NULL) {
                    PlaySample(13, &enemy->actor.pos, 255, 128);
                    AnimateActor(&enemy->actor, 4, 0, 0, 0);
                    CreateEnemyBlastring(&enemy->actor.pos, 30, -enemy->actor.radius / 1.1, -0.7f);
                    StartEarthquakeAtPoint(&enemy->actor.pos, 5, 5000.0f, 2500.0f, 1, 3);
                    enemy->brain.cooldownTimer = 2000;
                    enemy->brain.attackTime = 0;
                    enemy->brain.flags |= 1;
                    enemy->actor.flags &= ~0x10C0000;
                } else if (enemy->brain.cooldownTimer == 0) {
                    enemy->actor.anim->loop = 0;
                    AnimateActor(&enemy->actor, 0, 0, 1, 0);
                    if (enemy->actor.anim->current == 0) {
                        enemy->actor.vel.y -= 2.0f;
                    }
                    SCALE_VECTOR_2D(&enemy->actor.vel, 0.8);
                } else {
                    enemy->actor.rotAim.z = 0.0f;
                    SCALE_VECTOR(&enemy->actor.vel, 0.8);
                }
            } else {
                tempVect.x = 0.0f;
                tempVect.z = 70.0f;
                RotateVector2D(&tempVect, &tempVect, (f32)enemy->brain.attackTime / 15.0f);
                AddToVector2D(&tempVect, &hand.pos);
                NMEActionMoveTo2D(enemy, &tempVect, 0, 0x2001);
                enemy->actor.vel.y += 0.05;
                if (enemy->actor.pos.y >= hand.pos.y + 250.0f && enemy->actor.shadow->altitude > 100.0f) {
                    enemy->brain.cooldownTimer = 25;
                    enemy->actor.flags |= 0x10C0000;
                }
            }
        } else {
            if (enemy->actor.anim->current == 4) {
                ZERO_VECTOR(&enemy->actor.vel);
            } else if (enemy->actor.anim->current == 10) {
                enemy->actor.vel.y *= 0.7;
            }
            if (enemy->actor.anim->finishedAnim == 5) {
                AnimateActor(&enemy->actor, 10, 0, 0, 0);
                AnimateActor(&enemy->actor, 15, 1, 1, 0);
                enemy->actor.vel.y = 12.0f;
            }
            if (enemy->actor.anim->finishedAnim == 11) {
                enemy->brain.cooldownTimer = 0;
            }
            if (enemy->brain.cooldownTimer == 0) {
                enemy->brain.attackTime = 1;
                enemy->brain.flags &= ~1;
            }
        }
    }
}

void UpdateEnemyBlastring(Footprint *footprint) {
    Vec3 d;
    f32 dist;

    if (footprint->alpha > 20) {
        if (hand.obj->onGround && !(handControl.flags & 1) && !player.hurtTimer && !handControl.deathTimer) {
            SubVector2D(&d, &hand.pos, &footprint->pos);
            dist = LENGTH_2D(&d);
            if (dist < footprint->width * 10.0f && footprint->width * 7.5 < dist) {
                HurtHand(HURT_FALL, 1);
                AddToOneScaledVector2D(&hand.vel, &d, 8.0f / dist);
                hand.flags |= 0x2000;
                hand.vel.y = 6.0f;
            }
        }
        if (ball.obj->onGround) {
            SubVector2D(&d, &ball.pos, &footprint->pos);
            dist = LENGTH_2D(&d);
            if (dist < footprint->width * 10.0f && footprint->width * 7.0f < dist) {
                AddToOneScaledVector2D(&ball.vel, &d, 6.0f / dist);
                ball.vel.y = 6.0f;
            }
        }
    }
}

void CreateEnemyBlastring(Vec3 *pos, s32 time, f32 yAdd, f32 scaleAimSpeed) {
    Footprint *blastring = JallocAlloc(sizeof(Footprint), 1, "Footprnt");

    if (SetupFootprint(blastring, pos, NULL, time / 2 + 1, time, blastTex, 6, 0x800)) {
        blastring->pos.y += yAdd;
        if (gameInfo.level == ATLANTISBOSS && blastring->pos.y < 0.5) {
            blastring->pos.y = 0.5f;
        }
        blastring->alpha = 150;
        blastring->width = 0.0f;
        blastring->maxAlpha = 0;
        blastring->alphaStep = blastring->alpha / (f32)time;
        blastring->normal = *(Vec3 *)&vertQ;
        blastring->scaleSpeed = scaleAimSpeed;
        AddFootprint(blastring);
    }
}

void NMEProcessRobot(Enemy *enemy) {
    Vec3 tempVect;
    Platform *platform;
    f32 val;
    f64 d;
    s32 i;
    s32 j;
    s32 vol;
    s32 footDown = FALSE;

    if (frameCount == 0) {
        waitForFootDown = 0;
    }
    if (waitForFootDown || (ball.link == &hand && TagToFirstPlatform(2)->actor.pos.y < TagToFirstPlatform(2)->extent.y + 10.0f)) {
        platform = TagToFirstPlatform(3);
        SUB_VECTORS(&tempVect, &platform->actor.pos, &ball.pos);
        val = LENGTH(&tempVect);
        if (waitForFootDown) {
            TagToFirstPlatform(1)->actor.vel.x = TagToFirstPlatform(1)->actor.vel.y = TagToFirstPlatform(1)->actor.vel.z = 0.0f;
            TagToFirstPlatform(2)->actor.vel.x = TagToFirstPlatform(2)->actor.vel.y = TagToFirstPlatform(2)->actor.vel.z = 0.0f;
            TagToFirstPlatform(3)->actor.vel.x = TagToFirstPlatform(3)->actor.vel.y = TagToFirstPlatform(3)->actor.vel.z = 0.0f;
            ZERO_VECTOR(&enemy->actor.vel);
            MakeUnit(&tempVect);
            if (val < 5.0f || levelExitTimer != 0) {
                if (levelExitTimer == 0) {
                    levelExitTimer = 10000;
                    PlaySample((Random(4) + 269), &hand.pos, 100, 128);
                    gameInfo.exitPlatform = platform;
                    DisableControl(10000);
                    handControl.actionLock = 10000;
                    ApplyAction(HAND_SPIN);
                    hand.rotAim.y = Aabs(camera.rot.y + PI);
                    gameInfo.exitType = SUB_LEVEL_EXIT;
                    ball.flags &= ~0x18;
                }
            } else {
                AddToOneScaledVector(&ball.pos, &tempVect, 5.0f);
            }
            return;
        }
        if (val < 50.0f) {
            waitForFootDown = 1;
            enemy->brain.delayTimer = 30000;
            enemy->brain.cooldownTimer = 30000;
            enemy->brain.scratch = 1000;
            enemy->actor.anim->speed = 0.0f;
            TagToFirstPlatform(1)->actor.vel.x = TagToFirstPlatform(1)->actor.vel.y = TagToFirstPlatform(1)->actor.vel.z = 0.0f;
            TagToFirstPlatform(2)->actor.vel.x = TagToFirstPlatform(2)->actor.vel.y = TagToFirstPlatform(2)->actor.vel.z = 0.0f;
            TagToFirstPlatform(3)->actor.vel.x = TagToFirstPlatform(3)->actor.vel.y = TagToFirstPlatform(3)->actor.vel.z = 0.0f;
            ZERO_VECTOR(&enemy->actor.vel);
            DisableControl(900);
            handControl.actionLock = 900;
        }
    }
    d = enemy->actor.scale.x * 100.0 / enemyStats[ROBOT].scale;
    vol = d < 70.0 ? 70 : (s32)d;
    enemy->samples[0].volume = vol;
    enemy->samples[0].pitch = (enemyStats[ROBOT].scale - enemy->actor.scale.x) * 200.0 + 128.0;

    platform = TagToFirstPlatform(1);
    SUB_VECTORS(&tempVect, &enemy->actor.obj->spheres[1].pos, &enemy->actor.pos);
    RotateVector2D(&tempVect, &tempVect, -enemy->actor.rot.y);
    tempVect.x += enemy->actor.scale.x * 170.0f / enemyStats[ROBOT].scale;
    tempVect.z += enemy->actor.scale.x * -50.0f / enemyStats[ROBOT].scale;
    RotateVector2D(&tempVect, &tempVect, enemy->actor.rot.y);
    platform->actor.pos.x = enemy->actor.pos.x + tempVect.x;
    platform->actor.pos.y = enemy->actor.pos.y + tempVect.y;
    platform->actor.pos.z = enemy->actor.pos.z + tempVect.z;
    platform->motion->rot = enemy->actor.rot;
    platform->actor.vel.x = platform->actor.pos.x - platform->actor.prevPos.x;
    platform->actor.vel.y = platform->actor.pos.y - platform->actor.prevPos.y;
    platform->actor.vel.z = platform->actor.pos.z - platform->actor.prevPos.z;
    if (frameCount >= 11 && enemy->actor.anim->speed != 0.0f) {
        if (platform->actor.pos.y - platform->extent.y <= 0.0f && platform->actor.prevPos.y - platform->extent.y > 0.0f) {
            PlaySample(98, &platform->actor.pos, 255, 128);
            StartEarthquakeAtPoint(&platform->actor.pos, 10, 1000.0f, 500.0f, 1, 14);
            footDown = TRUE;
        } else if (platform->actor.pos.y - platform->extent.y >= 0.0f && platform->actor.prevPos.y - platform->extent.y < 0.0f) {
            PlaySample(121, &platform->actor.pos, 255, 60);
        }
    }
    platform->actor.scale = enemy->actor.scale;

    platform = TagToFirstPlatform(2);
    tempVect.x = enemy->actor.obj->spheres[2].pos.x - enemy->actor.pos.x;
    tempVect.y = enemy->actor.obj->spheres[2].pos.y - enemy->actor.pos.y;
    tempVect.z = enemy->actor.obj->spheres[2].pos.z - enemy->actor.pos.z;
    RotateVector2D(&tempVect, &tempVect, -enemy->actor.rot.y);
    tempVect.x -= enemy->actor.scale.x * 170.0f / enemyStats[ROBOT].scale;
    tempVect.z -= enemy->actor.scale.x * -50.0f / enemyStats[ROBOT].scale;
    RotateVector2D(&tempVect, &tempVect, enemy->actor.rot.y);
    platform->actor.pos.x = enemy->actor.pos.x + tempVect.x;
    platform->actor.pos.y = enemy->actor.pos.y + tempVect.y;
    platform->actor.pos.z = enemy->actor.pos.z + tempVect.z;
    platform->motion->rot = enemy->actor.rot;
    platform->actor.vel.x = platform->actor.pos.x - platform->actor.prevPos.x;
    platform->actor.vel.y = platform->actor.pos.y - platform->actor.prevPos.y;
    platform->actor.vel.z = platform->actor.pos.z - platform->actor.prevPos.z;
    if (frameCount >= 11 && enemy->actor.anim->speed != 0.0f) {
        if (platform->actor.pos.y - platform->extent.y <= 0.0f && platform->actor.prevPos.y - platform->extent.y > 0.0f) {
            PlaySample(98, &platform->actor.pos, 255, 128);
            StartEarthquakeAtPoint(&platform->actor.pos, 10, 1000.0f, 500.0f, 1, 14);
            footDown = TRUE;
        } else if (platform->actor.pos.y - platform->extent.y >= 0.0f && platform->actor.prevPos.y - platform->extent.y < 0.0f) {
            PlaySample(121, &platform->actor.pos, 255, 60);
        }
    }
    platform->actor.scale = enemy->actor.scale;

    if (frameCount == 0) {
        enemy->actor.obj->spheres[0].radius = enemy->actor.obj->spheres[0].targetRadius = enemy->actor.phys->radius / 20.0f;
        waitForFootDown = 0;
        spaceBossExiting = 0;
        camera.flags = 0;
        camera.obj->planes[0] = NULL;
        enemy->brain.scratchF = 3.0f;
        enemy->brain.scratch = 0;
    }
    enemy->actor.scale.y = enemy->actor.scale.z = enemy->actor.scale.x =
        enemy->actor.obj->spheres[0].radius / enemy->actor.phys->radius * enemyStats[ROBOT].scale;
    enemy->actor.shadow->shadowSize = enemy->actor.obj->spheres[0].radius * 0.8;
    enemy->actor.phys->accel = enemy->actor.scale.x * 3.4 / enemyStats[ROBOT].scale;
    enemy->actor.phys->moveSpeed = enemy->actor.scale.x * 3.4 / enemyStats[ROBOT].scale;
    enemy->actor.pos.y = enemy->actor.scale.x * 250.0f / enemyStats[ROBOT].scale;
    enemy->actor.vel.y = 0.0f;
    enemy->actor.state->flying = 0;
    if (hand.obj->onGround && hand.obj->contactPlatform != NULL && hand.obj->contactPlatform->actor.vel.y < 0.0f) {
        for (i = 0; i < 4; i++) {
            if (hand.obj->planes[i] != NULL && hand.obj->planes[i]->y < 0.0f) {
                KillHand(HURT_SQUASH);
                hand.flags &= ~0x11;
                for (j = 0; j < 4; j++) {
                    hand.obj->planes[j] = NULL;
                }
                ZERO_VECTOR(&hand.vel);
            }
        }
    }
    if (handControl.deathTimer != 0) {
        hand.pos.y = MAX(5.0f, hand.pos.y);
        hand.vel.x = hand.vel.y = hand.vel.z = 0.0f;
        hand.flags &= ~0x11;
    }
    if (ball.obj->onGround && ball.obj->contactPlatform != NULL && ball.obj->contactPlatform->actor.vel.y < 0.0f) {
        for (i = 0; i < 4; i++) {
            if (ball.obj->planes[i] != NULL && ball.obj->planes[i]->y < 0.0f) {
                BurstBall();
            }
        }
    }
    if (DistanceBetweenPoints2D(&enemy->actor.pos, &hand.pos) > 200.0f) {
        if (!bulletList.count) {
            CamCirclePoint(&enemy->actor.pos);
        } else {
            CamCirclePoint(&bulletList.head.next->actor.pos);
        }
        levelCamHeight = 20;
    } else {
        levelCamHeight = 0;
    }

    platform = TagToFirstPlatform(3);
    tempVect.x = enemy->actor.obj->spheres[2].pos.x - enemy->actor.pos.x;
    tempVect.y = enemy->actor.obj->spheres[2].pos.y - enemy->actor.pos.y;
    tempVect.z = enemy->actor.obj->spheres[2].pos.z - enemy->actor.pos.z;
    RotateVector2D(&tempVect, &tempVect, -enemy->actor.rot.y);
    platform->actor.alpha = 0;
    tempVect.x -= 310.0f;
    tempVect.y -= 180.0f;
    RotateVector2D(&tempVect, &tempVect, enemy->actor.rot.y);
    platform->actor.pos.x = enemy->actor.pos.x + tempVect.x;
    platform->actor.pos.y = enemy->actor.pos.y + tempVect.y;
    platform->actor.pos.z = enemy->actor.pos.z + tempVect.z;

    if (enemy->brain.instrIndex == 0) {
        if (enemy->actor.anim->finishedAnim == 0) {
            f32 t;

            tempVect.y = 0.0f;
            tempVect.x = 2000.0f;
            tempVect.z = 2600.0f;
            t = (f32)(100 - frameCount) / 25.0;
            enemy->actor.anim->speed = MAX(0.0f, enemy->actor.anim->anims[15].speed * t);
            enemy->actor.phys->accel *= t;
            enemy->actor.phys->moveSpeed *= t;
            if (enemy->actor.phys->accel > 0.0f) {
                NMEActionMoveTo2D(enemy, &tempVect, 0.0f, 2);
            }
        }
        if (enemy->brain.instrIndex == 0) {
            return;
        }
    }
    if (enemy->brain.scratch == 0) {
        enemy->actor.anim->speed = enemy->actor.anim->anims[15].speed;
        enemy->actor.flags &= ~8;
        if (frameCount % 800 == 550) {
            enemy->actor.anim->loop = 0;
            AnimateActor(&enemy->actor, 10, 0, 1, 0);
        }
        tempVect.x = 0.0f;
        tempVect.y = 0.0f;
        tempVect.z = robotRadius;
        RotateVector2D(&tempVect, &tempVect, enemy->brain.scratchF * enemy->actor.phys->moveSpeed / robotRadius);
        tempVect.z += 1600.0f;
        enemy->brain.scratchF += 1.0f;
        if (enemy->actor.anim->current == 15) {
            NMEActionMoveTo2D(enemy, &tempVect, 0.0f, 1);
        }
        if (TriggerAtFrame(43.0f, &enemy->actor)) {
            PlaySample(119, &enemy->actor.pos, 255, 0);
        }
        if (enemy->actor.anim->finishedAnim == 11) {
            AnimateActor(&enemy->actor, 11, 0, 0, 0);
            FireBulletFromActor(&enemy->actor, BOMB_BULLET, 0);
            enemy->brain.target = enemy->actor.pos;
            PlaySample(119, &enemy->actor.pos, 255, 0);
        } else if (enemy->actor.anim->finishedAnim == 12) {
            enemy->brain.scratch = 75;
            AnimateActor(&enemy->actor, 15, 1, 0, 0);
            enemy->actor.anim->speed = 2.0f * -enemy->actor.anim->speed;
        }
    } else if (enemy->brain.scratch > 0) {
        if (spaceBossExiting) {
            if (footDown) {
                enemy->actor.anim->speed = 0.0f;
            }
            if (enemy->actor.anim->speed == 0.0f) {
                if (--enemy->brain.scratch == 0) {
                    spaceBossExiting = 0;
                    enemy->actor.anim->speed = enemy->actor.anim->anims[15].speed;
                    enemy->brain.scratch = -1;
                }
                return;
            }
        }
        tempVect.x = 0.0f;
        tempVect.y = 0.0f;
        tempVect.z = -robotRadius;
        RotateVector2D(&tempVect, &tempVect, enemy->actor.rot.y);
        tempVect.x += enemy->actor.pos.x;
        tempVect.y += enemy->actor.pos.y;
        tempVect.z += enemy->actor.pos.z;
        NMEActionMoveTo2D(enemy, &tempVect, 0.0f, 0x1002);
        SCALE_VECTOR(&enemy->actor.vel, 2.0f);
        if (--enemy->brain.scratch == 0) {
            spaceBossExiting = 1;
            enemy->brain.scratch = 180;
        }
    } else if (NMEActionMoveTo2D(enemy, &enemy->brain.target, 10.0f, 1)) {
        enemy->brain.scratch = 0;
    }
}

void NMEProcessSpank(Enemy *enemy) {
    Vec3 tempVect;
    f32 m;

    if (enemy->actor.obj->spheres[7].targetRadius != enemy->actor.obj->spheres[7].radius) {
        enemy->actor.obj->spheres[7].radius +=
            MIN(MAX(-1.0f, (enemy->actor.obj->spheres[7].targetRadius - enemy->actor.obj->spheres[7].radius) / 4.0f), 1.0f);
    }
    if (frameCount == 1) {
        enemy->actor.obj->spheres[7].radius = enemy->actor.obj->spheres[7].targetRadius = enemy->actor.phys->radius / 2.0f;
    }
    enemy->brain.target.y = (enemyStats[SPANK].scale - enemy->actor.scale.y) * 35.0f / 0.04;
    enemy->actor.scale.y = enemy->actor.scale.z = enemy->actor.scale.x =
        enemy->actor.obj->spheres[7].radius / enemy->actor.phys->radius * enemyStats[SPANK].scale;
    enemy->actor.shadow->shadowSize = enemy->actor.obj->spheres[7].radius * 0.8;
    enemy->actor.obj->spheres[1].targetRadius = enemy->actor.obj->spheres[7].targetRadius * 20.0f / enemy->actor.phys->radius;
    if (enemy->actor.state->hitFadeTimer != 0) {
        enemy->actor.state->hitFadeTimer--;
    }
    if (frameCount % 50 == 0 && Random(20) < 5) {
        PlaySample((Random(2) + 115), &enemy->actor.pos, 100, 128);
    }
    enemy->actor.flags &= ~0x01000000;
    enemy->actor.flags &= ~0x180000;
    if (enemy->actor.anim->current == 8 && TriggerAtFrame(172.0f, &enemy->actor)) {
        PlaySample(82, &enemy->actor.pos, 150, 128);
    }
    if (enemy->actor.anim->current == 10) {
            ZERO_VECTOR(&enemy->actor.vel);
            AccelerateActor2D(&enemy->actor, 2.0f, 20.0f);
    } else if (enemy->actor.anim->current == 5) {
            enemy->actor.pos.y = (enemyStats[SPANK].scale - enemy->actor.scale.y) * 35.0f / 0.04;
            if (TriggerAtFrame(67.0f, &enemy->actor) || TriggerAtFrame(79.0f, &enemy->actor)) {
                PlaySample(80, &enemy->actor.pos, 50, 128);
            }
            enemy->actor.vel.y = 0.0f;
            enemy->actor.flags &= ~0x11;
            enemy->actor.obj->onGround = 0;
            enemy->actor.obj->planes[0] = NULL;
            if (enemy->brain.scratchF == 0.0f && (enemy->actor.flags & 0x04000000) && LENGTH(&ball.vel) > spballsp
                && (ball.link == NULL || handControl.releaseTimer != 0)) {
                PlaySample(117, &enemy->actor.pos, 100, 128);
                enemy->brain.cooldownTimer = -300;
                enemy->actor.anim->loop = 0;
                enemy->brain.scratchF = spv3;
                AnimateActor(&enemy->actor, 11, 0, 0, 0);
                enemy->actor.anim->speed = 0.0f;
                enemy->actor.anim->time = 103.0f;
            }
    } else if (enemy->actor.anim->current == 11) {
            if (enemy->brain.scratchF <= 15.0f) {
                enemy->actor.flags |= 0x01000000;
                if (enemy->brain.scratchF == 15.0f) {
                    enemy->actor.vel.y = spvely;
                    enemy->actor.vel.z = spvelz;
                    enemy->actor.anim->time = 113.2f;
                    enemy->brain.scratchF = 1.0f;
                    enemy->actor.flags |= 1;
                    enemy->actor.obj->spheres[0].targetRadius =
                        enemy->actor.obj->spheres[7].targetRadius * 25.0f / enemy->actor.phys->radius;
                }
                if (enemy->actor.vel.y < 0.0f) {
                    enemy->actor.flags |= 0x10;
                }
                if (TriggerAtFrame(116.0f, &enemy->actor)) {
                    PlaySample(82, &enemy->actor.pos, 255, 128);
                    StartEarthquakeAtPoint(&enemy->actor.pos, 10, 2000.0f, 1000.0f, 1, 2);
                    CreateBigDustCircle(&enemy->actor.pos, -25.0f);
                }
                if (enemy->actor.obj->onGround && (enemy->actor.flags & 0x10)) {
                    enemy->actor.anim->speed = enemy->actor.anim->anims[11].speed;
                    AnimateActor(&enemy->actor, 6, 1, 1, 0);
                    enemy->brain.scratchF = 0.0f;
                }
            } else {
                m = MIN(enemy->brain.scratchF / spv2, spv1);
                enemy->brain.scratchF -= 1.0f;
                enemy->actor.anim->time += m;
                if (enemy->actor.anim->time > 106.0f) {
                    PlaySample(80, &enemy->actor.pos, 50, 128);
                    enemy->actor.anim->time -= 3.0f;
                }
            }
    } else {
            enemy->actor.flags |= 0x11;
            if (enemy->actor.anim->current == 1) {
                    enemy->actor.obj->spheres[0].targetRadius =
                        enemy->actor.obj->spheres[7].targetRadius * 32.0f / enemy->actor.phys->radius;
            } else if (enemy->actor.anim->current == 12) {
                    enemy->actor.flags |= 0x01000000;
                    enemy->actor.flags |= 0x180000;
                    SubVector2D(&tempVect, &hand.pos, &enemy->actor.obj->spheres[1].pos);
                    m = Aabs(atan2(tempVect.x, tempVect.z));
                    if (FindShortestAngle(enemy->actor.rot.y, m) < 1.570796327) {
                        MakeUnit2D(&tempVect);
                        enemy->actor.rotAim.y = m;
                        AddToOneScaledVector2D(&enemy->actor.vel, &tempVect, 2.0f);
                        MakeUnit(&enemy->actor.vel);
                        SCALE_VECTOR(&enemy->actor.vel, enemy->actor.speed + enemy->actor.phys->accel * 1.1);
                    } else {
                        AccelerateActor2D(&enemy->actor, enemy->actor.phys->accel, 30.0f);
                    }
                    if (enemy->actor.anim->time >= 185.0f && enemy->actor.anim->time < enemy->actor.anim->speed + 185.0f) {
                        PlaySample(82, &enemy->actor.pos, 255, 128);
                        StartEarthquakeAtPoint(&enemy->actor.pos, 10, 2000.0f, 1000.0f, 1, 2);
                        CreateBigDustCircle(&enemy->actor.pos, -25.0f);
                    }
            } else if (enemy->actor.anim->current == 6) {
                    if (++enemy->brain.scratch == 80) {
                        enemy->brain.scratch = 0;
                        FlushQueue(&enemy->actor);
                        enemy->brain.curList = enemy->brain.normalList;
                        NMEGetNewInstruction(enemy, 1, 1);
                        StartBabiesAttacking();
                    }
                    if (enemy->brain.flags & 8) {
                        PlaySample(117, &enemy->actor.pos, 100, 128);
                        enemy->actor.state->hits--;
                        enemy->actor.obj->spheres[7].targetRadius -= enemy->actor.phys->radius / 4.0f;
                        enemy->actor.state->hitFadeTimer = 20;
                        if (enemy->actor.state->hits == 0) {
                            KillEnemy(enemy);
                            enemy->brain.scratch = 1;
                            return;
                        }
                        enemy->brain.scratch = 0;
                        enemy->brain.curList = enemy->brain.normalList;
                        AnimateActor(&enemy->actor, 4, 0, 0, 0);
                        NMEGetNewInstruction(enemy, 1, 1);
                        hand.vel.y = 7.0f;
                        hand.vel.z = 6.0f;
                        hand.flags |= 0x01002000;
                        hand.friction = handInertia;
                        StartBabiesAttacking();
                    }
            } else if (enemy->actor.anim->current == 7) {
                    enemy->actor.flags &= ~0x10;
                    enemy->actor.obj->onGround = 0;
                    enemy->actor.obj->planes[0] = NULL;
                    enemy->actor.rotAim.y = 0.0f;
                    SUB_VECTORS(&tempVect, &enemy->brain.target, &enemy->actor.pos);
                    m = LENGTH(&tempVect);
                    spankJumpVal = (enemyStats[SPANK].scale - enemy->actor.scale.x) * 30.0f + 4.0f;
                    if (m <= spankJumpVal) {
                        ZERO_VECTOR(&enemy->actor.vel);
                        enemy->actor.pos = enemy->brain.target;
                        if (enemy->actor.anim->finishedAnim != 0) {
                            enemy->brain.instrTimer = 1;
                        }
                    } else {
                        ZERO_VECTOR(&enemy->actor.vel);
                        AddToOneScaledVector(&enemy->actor.pos, &tempVect, spankJumpVal / m);
                    }
            } else if (enemy->actor.anim->current == 9) {
                    if (TriggerAtFrame(32.0f, &enemy->actor)) {
                        PlaySample(82, &enemy->actor.pos, 50, 128);
                    }
                    enemy->actor.flags |= 0x01000000;
                    if (enemy->brain.instr.type != NME_FOLLOWPLAYER) {
                        NMEActionMoveTo2D(enemy, &hand.pos, 0, 1);
                    }
                    tempVect.x = 0.0f;
                    tempVect.z = 140.0f;
                    if (hand.pos.z < enemy->brain.target.z - 10.0f ||
                        (enemy->brain.instrTime >= 31 && enemy->actor.pos.z > -100.0f && enemy->brain.instrIndex != 6 &&
                         DistanceBetweenPoints2D(&tempVect, &enemy->actor.pos) > 180.0f)) {
                        enemy->actor.anim->loop = 0;
                        FlushQueue(&enemy->actor);
                        AnimateActor(&enemy->actor, 13, 0, 1, 0);
                        AnimateActor(&enemy->actor, 15, 1, 1, 0);
                        NMEGetNewInstruction(enemy, 6, 1);
                    }
            }
    }
}

void NMEProcessBabySpank(Enemy *enemy) {
    Vec3 tempVect;
    s32 j;

    if (frameCount == 1) {
        enemy->brain.spellTimer[2] = 10000;
        enemy->brain.scratch = enemy->actor.state->index;
    }
    if (enemy->brain.spellTimer[2] != 0) {
        return;
    }
    if (frameCount < 500 && enemy->brain.instr.type == NME_FOLLOWPLAYER) {
        enemy->brain.instrTimer = 1;
    }
    if (frameCount % 50 == 0 && Random(20) < 5) {
        PlaySample((Random(2) + 78), &enemy->actor.pos, 100, 128);
    }
    if (enemy->brain.attackTime && enemy->brain.instr.type == NME_SPECIAL && enemy->brain.instrTimer == -1
        && enemy->actor.anim->loop) {
        enemy->actor.anim->loop = 0;
        if (enemy->actor.anim->queueLen == 0) {
            AnimateActor(&enemy->actor, 9, 1, 1, 0);
        }
    }
    if (enemy->brain.cooldownTimer < 2 && IsHandHoldingBall()) {
        enemy->brain.cooldownTimer = 2;
    }
    if (enemy->actor.anim->current == 10) {
        if (ball.link != &enemy->actor && enemy->actor.anim->time <= enemy->actor.anim->speed + 75.0f) {
            j = 3 - enemy->actor.state->index;
            enemies[j].brain.scratch = 1;
            enemies[j].brain.cooldownTimer = 70;
            if (&enemies[j].actor == ball.link) {
                enemies[j].brain.curList = enemies[j].brain.normalList;
                NMEGetNewInstruction(&enemies[j], 4, 1);
            }
            NMEActionHoldPlayer(&enemy->actor, &zero, 2, 0x20080);
        }
        AccelerateActor2D(&enemy->actor, enemy->actor.phys->accel, enemy->actor.phys->moveSpeed);
        enemy->actor.obj->spheres[1].targetRadius = 0.0f;
    } else {
        enemy->actor.obj->spheres[1].targetRadius = 15.0f;
    }
    if (ball.link == &enemy->actor) {
        if (enemy->brain.cooldownTimer == 16) {
            enemy->brain.curList = enemy->brain.normalList;
            NMEGetNewInstruction(enemy, 1, 1);
        } else if (enemy->brain.instrTime >= spbval && Random(5) == 0) {
            enemy->actor.anim->loop = 0;
            FlushQueue(&enemy->actor);
            enemy->brain.delayTimer = 80;
            AnimateActor(&enemy->actor, 12, 0, 1, 0);
            AnimateActor(&enemy->actor, 0, 0, 1, 0);
            AnimateActor(&enemy->actor, 9, 1, 1, 0);
            j = 3 - enemy->actor.state->index;
            enemy->brain.scratch = 0;
            enemies[j].brain.scratch = 0;
            enemy->brain.curList = enemy->brain.normalList;
            NMEGetNewInstruction(enemy, 1, 1);
            enemy->brain.cooldownTimer = 70;
            enemy->brain.delayTimer = 2;
        }
    } else if (enemy->brain.scratch == 1) {
        if (enemy->brain.delayTimer == 0) {
            NMEActionMoveTo2D(enemy, &ball.pos, 0, 1);
        }
    } else if (enemy->brain.instr.type == NME_WAIT) {
        enemy->brain.curList = enemy->brain.normalList;
        NMEGetNewInstruction(enemy, 1, 1);
    }
    if ((enemy->brain.flags & 8) && enemy->actor.anim->current != 6 && enemy->actor.anim->current != 4) {
        if (ball.link == &enemy->actor) {
            j = 3 - enemy->actor.state->index;
            enemy->brain.scratch = 0;
            enemies[j].brain.scratch = 0;
            EnemyDropBall();
            if (enemies[j].brain.attackTime == 0) {
                NMEGetNewInstruction(&enemies[j], 1, 1);
            }
            ball.vel.y = 7.0f;
            AccelerateActorFromActor(&enemy->actor, &ball, -3.0f, 30.0f);
        } else {
            enemy->brain.cooldownTimer = 170;
            FlushQueue(&enemy->actor);
            enemy->actor.anim->loop = 0;
            AnimateActor(&enemy->actor, 11, 0, 0, 0);
            AnimateActor(&enemy->actor, 6, 1, 1, 0);
            AnimateActor(&enemy->actor, 1, 0, 1, 0);
            enemy->brain.scratchF = 100.0f;
        }
    }
    if (enemy->actor.anim->current == 11 || enemy->actor.anim->current == 1 || enemy->actor.anim->current == 6) {
        if (enemy->actor.anim->finishedAnim == 2) {
            AnimateActor(&enemy->actor, 9, 1, 0, 0);
        }
        enemy->brain.delayTimer = 2;
    } else if (enemy->actor.anim->current == 12 && enemy->actor.anim->time > 88.0f && ballControl.state != BALL_BURST) {
        ball.link = NULL;
        ballControl.enemy = NULL;
        PlaySample(81, &enemy->actor.pos, 150, 128);
        BurstBall();
    }
    if (enemy->actor.anim->current == 6) {
        tempVect.x = 0.0f;
        tempVect.z = -12.0f;
        RotateVector2D(&tempVect, &tempVect, enemy->actor.rot.y);
        AddToVector2D(&tempVect, &enemy->actor.pos);
        tempVect.y = enemy->actor.pos.y + 5.0f;
        DoDizzyStars(&tempVect);
        if (enemy->brain.scratchF != 0.0f && --enemy->brain.scratchF == 0.0f) {
            enemy->actor.anim->loop = 0;
        }
        if (enemy->brain.flags & 8) {
            if (enemy->brain.attackTime) {
                NMEEndAttack(enemy);
                NMEGetNewInstruction(enemy, enemy->brain.instrIndex, 0);
            }
            AnimateActor(&enemy->actor, 4, 0, 0, 0);
            AnimateActor(&enemy->actor, 6, 1, 1, 0);
            AnimateActor(&enemy->actor, 1, 0, 1, 0);
            enemy->brain.scratchF = 200.0f;
        }
    }
}

void StartBabiesAttacking(void) {
    s32 i;

    for (i = 1; i < 3; i++) {
        if (!enemies[i].brain.attackTime) {
            NMEActionStartAttack(&enemies[i]);
            NMEGetNewInstruction(&enemies[i], 2, 1);
            enemies[i].brain.instrTimer = 300;
        }
    }
}

s32 TriggerAtFrame(f32 frame, Actor *actor) {
    if (frame <= actor->anim->time && actor->anim->time < frame + actor->anim->speed) {
        return TRUE;
    }
    return FALSE;
}

void NMEProcessReggie(Enemy *enemy) {
    if (enemy->actor.anim->current == 15 && frameCount % 10 == 0) {
        PlaySample(68, &enemy->actor.pos, 50, 128);
    }
}

static inline s32 QueryAnimTimeInline(Actor *actor, f32 t) {
    if (t <= actor->anim->time && actor->anim->time < actor->anim->speed + t) {
        return TRUE;
    }
    return FALSE;
}

void NMEProcessEvilGlove(Enemy *enemy) {
    Vec3 tempVect;
    Vec3 tempVect2;
    Vec3 tempVect3;
    Bullet *bul;
    struct Platform *plat;
    Debris *debris;
    f32 temp;
    s32 x;
    s32 t;

    if (frameCount == 0) {
        evilGloveWait = 1;
        evilGloveCount = 0;
        evilGloveCastSpell = 0;
        if (gameInfo.level == CARNIVALBOSS) {
            for (jackBox = actorList.head.next; jackBox != &actorList.head; jackBox = jackBox->next) {
                if (jackBox->type == CAMEO_ACTOR) {
                    break;
                }
            }
        }
        if (gameInfo.level == SPACEBOSS1) {
            enemy->brain.bobPeriod = 5;
            enemy->brain.bobAmount = 0.2f;
            CreateOrbiter(&handControl.orbiter3, 0, 0.2f);
            ZERO_VECTOR(&handControl.orbiter3.pos);
            PlayContinuousSample(&enemy->samples[1], 312, 60, &handControl.orbiter3.pos, 205);
            handControl.orbiter3.link = &enemy->actor;
            handControl.orbiter3.scale.x = 0.15f;
            handControl.orbiter3.scale.y = 0.05f;
            handControl.orbiter3.scale.z = 0.15f;
            handControl.rotorSpeed = gravity;
        }
        if (gameInfo.level == SPACEBOSS3) {
            evilGloveFootstepTimer = 0;
            enemy->actor.rot.x = enemy->actor.rotAim.x = 1.5707964f;
            PlayContinuousSample(&enemy->samples[0], 342, 40, &enemy->actor.pos, 80);
        } else if (gameInfo.level == SPACEBOSS_TWEEN) {
            PlayContinuousSample(&enemy->samples[1], 342, 40, &enemy->actor.pos, 80);
        }
    }
    if (gameInfo.level == SPACEBOSS_TWEEN) {
        tempVect.x = 0.0f;
        tempVect.z = 0.0f;
        tempVect.y = -7.0f;
        tempVect.x += enemy->actor.vel.x;
        tempVect.y += enemy->actor.vel.y;
        tempVect.z += enemy->actor.vel.z;
        AddEvilGloveSmoke(&enemy->actor, &tempVect);
        if (enemy->actor.anim->current != 4 && enemy->brain.instrIndex == 5) {
            AnimateActor(&enemy->actor, 4, 0, 0, 1.5f);
        }
    }
    if (gameInfo.level == SPACEBOSS3) {
        evilGlovePos[evilGloveFire] = enemy->actor.pos;
        cameraInfo.sideOffsetEnabled = 0;
        evilGloveFire = 1 - evilGloveFire;
        if (cameoList.head.next->items[0]->done) {
            evilGloveWait = 0;
        }
        if (cameoList.head.next->items[4]->time > 0) {
            evilGloveWait = 1;
        }
        if (!evilGloveWait) {
            rotOrder[0] = 2;
            rotOrder[1] = 0;
            rotOrder[2] = 1;
            enemy->actor.rot.x = Aabs(enemy->actor.rot.x - sb3loop - 1.570796327);
            temp = enemy->actor.rot.y;
            ZERO_VECTOR(&enemy->actor.vel);
            enemy->actor.rot.y = 0.0f;
            enemy->actor.rotAim.y = 0.0f;
            enemy->actor.flags &= ~0x4000;
            AccelerateActor(&enemy->actor, 15.0f, 15.0f);
            enemy->actor.flags |= 0x4000;
            enemy->actor.rot.x = Aabs(enemy->actor.rot.x + 1.570796327);
            enemy->actor.rotAim.y = enemy->actor.rot.y = Aabs(temp + 2.0f * sb3loop);
            GetQuaternionFromXYZRot(&enemy->actor.rotQuat, &enemy->actor.rot);
            rotOrder[0] = 1;
            rotOrder[1] = 2;
            rotOrder[2] = 0;
        } else if (enemies[0].actor.state->hits == 0) {
            enemy->actor.flags |= 0x4000;
        } else {
            enemy->actor.flags &= ~0x4000;
        }
        if (gameInfo.cameoActive) {
            if (cameoList.head.next->items[9]->time && enemy->actor.pos.y > 270.0f) {
                if (evilGloveFootstepTimer == 0) {
                    StopContinuousSample(&enemy->samples[0]);
                    evilGloveFootstepTimer = 78;
                }
            }
            if (evilGloveFootstepTimer) {
                t = evilGloveFootstepTimer--;
                if (evilGloveFootstepTimer > 50) {
                    if (!(frameCount & 3)) {
                        PlaySample(148, &enemies[0].actor.pos, 200, (220 - (t - 58) * 5));
                    } else if (frameCount % 4 == 2) {
                        PlaySample(149, &enemies[0].actor.pos, 200, (220 - (t - 58) * 5));
                    }
                } else {
                    if (t < 12 || t >= 42 || (t >= 22 && t < 32)) {
                        enemies[0].actor.pos.x += Random(21) - 10;
                        enemies[0].actor.pos.y += Random(21) - 10;
                        enemies[0].actor.pos.z += Random(21) - 10;
                    }
                    if (evilGloveFootstepTimer == 50 || evilGloveFootstepTimer == 30) {
                        PlaySample(338, &enemies[0].actor.pos, 200, 80);
                    } else if (evilGloveFootstepTimer == 10) {
                        PlayContinuousSample(&enemies[0].samples[0], 339, 160, &enemies[0].actor.pos, 80);
                    }
                }
            }
            if (enemies[0].actor.state->hits == 0) {
                if (!enemy->samples[0].handle && enemy->actor.pos.y > 4000.0f) {
                    PlayContinuousSample(&enemy->samples[0], 327, 100, &enemy->actor.pos, 100);
                }
                if (enemy->actor.pos.y > -5000.0f) {
                    SUB_VECTORS(&tempVect, &evilGlovePos[1 - evilGloveFire], &evilGlovePos[evilGloveFire]);
                    AddEvilGloveFire(&enemy->actor.pos, &tempVect, 6, 3.0f, 2.0f);
                } else {
                    GiveActorRandomQRotVel(&enemy->actor);
                    GetQuaternionFromRotation(&enemy->actor.rotQuat, (Quat *)&enemy->actor.spinAxis);
                    GiveActorRandomQRotVel(&enemy->actor);
                    enemy->actor.spinSpeed = 1.2f;
                    enemy->actor.pos = enemies[0].actor.pos;
                    enemy->actor.pos.y += 100.0f;
                    hand.alpha = 100;
                }
            } else if (enemy->actor.pos.y > -5000.0f) {
                AddEvilGloveSmoke(&enemy->actor, &zero);
            }
        }
    }
    if (enemy->actor.anim->current == 4 && !enemy->samples[0].handle) {
        PlayContinuousSample(&enemy->samples[0], 28, 100, &enemy->actor.pos, 20);
        evilGloveCastSpell = 1;
    }
    if (evilGloveCastSpell && enemy->samples[0].volume
        && (enemy->actor.anim->time > 150.0f || enemy->actor.anim->current != 4)) {
        enemy->samples[0].volume -= 2;
    }
    if (enemy->actor.anim->current == 6) {
        if (!evilGloveLaugh) {
            PlaySample(176, &enemy->actor.pos, 100, 128);
            evilGloveLaugh = 1;
        }
        if (QueryAnimTimeInline(&enemy->actor, 218.0f)) {
            PlaySample(165, &enemy->actor.pos, 100, 140);
        }
        if (enemy->actor.anim->finishedAnim == 7) {
            FlashScreen(255, 0, 0, 300, 0);
            KillEnemy(enemy);
            if (gameInfo.level == CARNIVALBOSS) {
                InitActor(&ckWeapon, "bomb.ndo", 0.0f, 300.0f, 0.0f, 0);
                CreateStarCircle(&jackBox->pos, 5, 19);
                AddDebris(POPB, &jackBox->pos, &zero, NULL, NULL);
                DestroyAndFreeActor(jackBox);
                ckWeapon.flags = (ckWeapon.flags | 4) & ~1;
                ckWeapon.scale.x = 0.05f;
                ckWeapon.scale.y = 0.05f;
                ckWeapon.scale.z = 0.05f;
                AddActor(&ckWeapon);
                ckWeapon.phys->radius = 20.0f;
                ckWeapon.phys->friction = 0.9f;
                ckWeapon.phys->maxSpeed = 28.0f;
                ckWeapon.phys->moveSpeed = 28.0f;
                ckWeapon.obj->spheres[0].radius = 20.0f;
                ckWeapon.phys->bounce = 0.5f;
                ckWeapon.friction = 0.9f;
                ckWeapon.pos.y = -5000.0f;
            } else if (gameInfo.level == SPACEBOSS1) {
                StopContinuousSample(&enemy->samples[1]);
                handControl.orbiter3.inst = NULL;
                SubActor(&handControl.orbiter3);
            }
            return;
        }
        tempVect.x = Random(25) - 12;
        tempVect.y = Random(25) - 12;
        tempVect.z = Random(25) - 12;
        tempVect.x += enemy->actor.pos.x;
        tempVect.y += enemy->actor.pos.y;
        tempVect.z += enemy->actor.pos.z;
        AddDebris(SPARKLE, &tempVect, &zero, NULL, NULL);
    } else {
        evilGloveLaugh = 0;
    }
    if (gameInfo.level == CARNIVALBOSS) {
        jackBox->anim->speed = 0.87f;
        ckWeapon.pos.y = 39.0f;
        ckWeapon.flags = 4;
        enemy->actor.flags &= ~0x100;
        if (frameCount >= ckstart) {
            if (frameCount == ckstart) {
                PlaySample(27, &zero, 100, 120);
                PlaySample(104, &zero, 100, 120);
                CreateStarCircle(&enemy->actor.pos, 5, 19);
                AddDebris(POPB, &enemy->actor.pos, &zero, NULL, NULL);
                for (x = 0; x < 10; x++) {
                    tempVect.x = Random(11) - 5;
                    tempVect.z = Random(11) - 5;
                    tempVect.y = Random(6) + 3;
                    debris = AddDebris(WIND_BIT, &enemy->actor.pos, &tempVect, NULL, NULL);
                    if (debris != NULL) {
                        debris->lifeTimer = 5;
                        debris->info->friction = 0.6f;
                        debris->friction = 0.6f;
                    }
                }
            }
            enemy->actor.scale.y = enemy->actor.scale.z = enemy->actor.scale.x =
                enemyStats[EVIL_GLOVE].scale * 0.1 + enemy->actor.scale.x * 0.9;
            ckWeapon.scale.y = enemy->actor.scale.x * 0.05 / enemyStats[EVIL_GLOVE].scale;
            ckWeapon.scale.x = ckWeapon.scale.y * 0.6;
            ckWeapon.scale.z = ABS(SineWave(6.0f, frameCount, -ckstart) * MAX(ckstart + 130 - frameCount, 15) * springfac2) + 0.035;
            tempVect.x = tempVect.z = 0.0f;
            tempVect.y = springfac * ckWeapon.scale.z;
            ckWeapon.rot.z = SineWave(8.0f, frameCount, -ckstart) * 0.3;
            ckWeapon.rot.y = 0.0f;
            RotateVector2DXYZ(&tempVect, &tempVect, ckWeapon.rot.z, 2);
            SetVector2D(&enemy->actor.pos, &tempVect);
            enemy->actor.pos.y = tempVect.y + ckWeapon.pos.y;
            enemy->actor.rot.z = ckWeapon.rot.z;
            if (enemy->actor.pos.y > enemy->actor.prevPos.y) {
                if (!evilGloveWait) {
                    PlaySample(104, &zero, MIN(((220 - frameCount) < 50) ? 50 : (220 - frameCount), 100), (Random(31) + 105));
                    evilGloveWait = 1;
                }
            } else {
                evilGloveWait = 0;
            }
        } else {
            ZERO_VECTOR(&enemy->actor.scale);
            ckWeapon.scale.z = 0.0f;
        }
    }
    if (enemy->actor.anim->current == 2) {
        if (!enemy->samples[1].handle) {
            PlayContinuousSample(&enemy->samples[1], 36, 100, &enemy->actor.pos, 128);
        }
        bul = CreateNewBullet(CRUMPET_BULLET);
        bul->actor.pos = enemy->actor.pos;
        bul->actor.pos.z -= 8.0f;
        bul->actor.vel.x = Random(9) - 4;
        bul->actor.vel.y = Random(5) - 2;
        bul->actor.vel.z = -8 - Random(3);
        InitBulletSpritesOrMesh(bul, NULL, 0);
        bul->sprites[0].anim.def.startScale *= 3;
        bul->sprites[1].anim.def.startScale *= 3;
        bul->sprites[0].anim.def.endScale *= 3;
        bul->sprites[1].anim.def.endScale *= 3;
        AddBullet(bul);
        if (evilGloveCount++ >= 11) {
            plat = TagToFirstPlatform(1);
            plat->actor.scale.y -= 0.15714285714285714;
        }
    } else if (gameInfo.level == PREHISTORICBOSS && enemy->samples[1].handle) {
        plat = TagToFirstPlatform(1);
        if (plat != NULL) {
            KillPlatform(plat, 0);
        }
        if ((enemy->samples[1].volume -= 5) <= 0) {
            StopContinuousSample(&enemy->samples[1]);
        }
    }
    if (enemy->actor.anim->current == 0) {
        switch (gameInfo.level) {
            case ATLANTISBONUS:
            case CARNIVALBOSS:
            case PIRATESBOSS:
            case SPACEBOSS1:
                if (QueryAnimTimeInline(&enemy->actor, 86.0f)) {
                    PlaySample(175, &enemy->actor.pos, 100, 128);
                }
                if (QueryAnimTimeInline(&enemy->actor, 89.0f)) {
                    bul = FireBulletFromActor(&enemy->actor, NME_SPELL_BULLET, 1);
                    bul->spellType = 2;
                    bul->actor.flags &= ~0x10;
                    bul->spell = 2;
                    switch (gameInfo.level) {
                        case PIRATESBOSS:
                            bul->actor.flags &= ~8;
                            if (enemies[1].brain.spellTimer[2] > 50) {
                                bul->target = &enemies[1].actor;
                            } else if (enemies[2].brain.spellTimer[2] > 50) {
                                bul->target = &enemies[2].actor;
                            } else {
                                bul->target = &enemies[0].actor;
                            }
                            return;
                        case ATLANTISBONUS:
                            bul->target = &hand;
                            return;
                        case CARNIVALBOSS:
                        case SPACEBOSS1:
                            bul->target = &enemies[0].actor;
                            return;
                    }
                }
                break;
            case ATLANTISBOSS:
            case FEARBOSS:
                if (enemy->actor.anim->time >= 89.0f && enemy->actor.anim->time <= 100.0f) {
                    if (!enemy->samples[0].handle) {
                        PlayContinuousSample(&enemy->samples[0], 62, 100, &enemy->actor.pos, 128);
                    }
                    if (gameInfo.level == ATLANTISBOSS) {
                        tempVect.x = tempVect.z = 0.0f;
                        if (enemies[0].actor.pos.y > 500.0f) {
                            tempVect.y = 36.0f;
                        } else if (enemies[1].actor.prevPos.y > 500.0f) {
                            tempVect.y = 84.0f;
                        } else {
                            tempVect.y = 120.0f;
                        }
                        if (frameCount % 3 == 0) {
                            CreateStarCircle(&tempVect, 5, 19);
                        }
                    } else if (gameInfo.level == FEARBOSS) {
                        tempVect.x = tempVect.z = 0.0f;
                        tempVect.y = 400.0f;
                    }
                    tempVect2.x = 8.0f;
                    tempVect2.y = 10.0f;
                    tempVect2.z = 10.0f;
                    RotateVector2D(&tempVect2, &tempVect2, enemy->actor.rot.y);
                    tempVect3.y = 10.0f;
                    tempVect3.z = 10.0f;
                    tempVect3.x = -8.0f;
                    RotateVector2D(&tempVect3, &tempVect3, enemy->actor.rot.y);
                    ADD_TO_VECTOR(&tempVect2, &enemy->actor.pos);
                    ADD_TO_VECTOR(&tempVect3, &enemy->actor.pos);
                    if (Random(10) < 4) {
                        AddLightning(&tempVect2, &tempVect, 0, 0, 0, 10, 1, 6, 1.0f, 1.0f);
                        AddLightning(&tempVect3, &tempVect, 0, 0, 0, 10, 1, 6, 1.0f, 1.0f);
                        if (frameCount & 1) {
                            SetFadeColour(255, 255, 255);
                            gameInfo.fade = 235;
                        }
                    } else {
                        AddLightning(&tempVect2, &tempVect, 255, 255, 255, 10, 1, 6, 1.0f, 1.0f);
                        AddLightning(&tempVect3, &tempVect, 255, 255, 255, 10, 1, 6, 1.0f, 1.0f);
                        if (frameCount & 1) {
                            SetFadeColour(0, 0, 0);
                            gameInfo.fade = 235;
                        }
                    }
                } else if (enemy->samples[0].handle) {
                    StopContinuousSample(&enemy->samples[0]);
                }
                break;
        }
    }
}

void NMEProcessKloset(Enemy *enemy) {
    if (enemy->actor.state->hitFadeTimer) {
        enemy->actor.state->hitFadeTimer--;
    }
    if (frameCount == 1) {
        enemy->actor.obj->spheres[0].radius = enemy->actor.obj->spheres[0].targetRadius = enemy->actor.phys->radius / 2.0f;
        enemy->actor.anim->time = 1.0f;
    }
    enemy->actor.scale.y = enemy->actor.scale.z = enemy->actor.scale.x =
        enemy->actor.obj->spheres[0].radius / enemy->actor.phys->radius * enemyStats[SPANK].scale;
    enemy->actor.shadow->shadowSize = enemy->actor.obj->spheres[0].radius * 0.8;
}

void AddEvilGloveFire(Vec3 *pos, Vec3 *vel, s16 type, f32 scale1, f32 scale2) {
    Bullet *bul = CreateNewBullet(type);

    bul->actor.pos = *pos;
    bul->actor.pos.x += Random(21) - 10;
    bul->actor.pos.z += Random(21) - 10;
    bul->actor.pos.y += Random(21) - 10;
    bul->actor.vel = *vel;
    InitBulletSpritesOrMesh(bul, 0, 0);
    bul->sprites[0].anim.def.startScale *= scale1;
    bul->sprites[1].anim.def.startScale *= scale1;
    bul->sprites[0].anim.def.endScale *= scale2;
    bul->sprites[1].anim.def.endScale *= scale2;
    bul->sprites[0].scaleX *= scale1;
    bul->sprites[0].scaleY *= scale1;
    bul->sprites[1].scaleX *= scale1;
    bul->sprites[1].scaleY *= scale1;
    AddBullet(bul);
}

void NMEProcessBat(Enemy *enemy) {
    if (frameCount % 20 == enemy->actor.state->index && Random(100) < 20) {
        PlaySample(235, &enemy->actor.pos, 60, 128);
    }
}

void NMEProcessEvilRobot(Enemy *enemy) {
    Vec3 tempVect;
    Vec3 tempVect2;
    Bullet *bullet;
    struct Platform *platform;
    struct Platform *platform2;
    Debris *debris;
    s32 x;
    s32 j;
    u8 c;

    hand.obj->spheres[0].radius = hand.obj->spheres[0].targetRadius = hand.phys->radius = 180.0f;
    bobCounter++;
    if (enemy->actor.state->hitFadeTimer != 0) {
        enemy->actor.state->hitFadeTimer--;
    }
    if (frameCount == 1) {
        portionHealth[0] = portionHealth[1] = portionHealth[2] = 3;
        if (cyclingTextures[bodyTexNum] != NULL) {
            cyclingTextures[bodyTexNum]->cycleSpeed = 8;
        }
    }
    if (enemy->actor.state->hits == 1) {
        if (cyclingTextures[bodyTexNum] != NULL) {
            cyclingTextures[bodyTexNum]->cycleSpeed = 8;
        }
        if ((enemy->actor.flags & 0x04000000) && immuneCount[2] == 0) {
            PlaySample(121, &enemy->actor.pos, 220, 60);
            immuneCount[2] = 20;
            if (enemy->brain.scratch < spbflytime / 2) {
                enemy->brain.scratchF = 0.0f;
            }
            if (--portionHealth[2] == 0) {
                enemy->actor.state->hits = 0;
                enemy->actor.state->hitFadeTimer = 20;
                SCALE_VECTOR_2D(&enemy->actor.vel, 0.6);
                if (enemy->actor.vel.y < 0.0f) {
                    enemy->actor.vel.y = 0.0f;
                }
                return;
            }
        }
    }
    if (enemy->actor.state->hits == 0) {
        bullet = CreateNewBullet(SMOKE_BULLET);
        bullet->actor.pos = enemy->actor.obj->spheres[3].pos;
        bullet->actor.pos.y += 90.0f;
        bullet->actor.vel.x = enemy->actor.vel.x + (Random(4) - 2) * 2;
        bullet->actor.vel.z = enemy->actor.vel.z + (Random(4) - 4);
        bullet->actor.vel.y = enemy->actor.vel.y + 12.0f;
        InitBulletSpritesOrMesh(bullet, NULL, 0);
        bullet->sprites[0].scaleX = bullet->sprites[0].scaleY = bullet->sprites[0].anim.def.startScale =
            bullet->sprites[0].anim.def.endScale = (Random(56) + 156) * 3;
        AddBullet(bullet);
        if (!(frameCount & 7) && Random(10) < 6) {
            PlaySample(106, &enemy->actor.pos, 150, (Random(60) + 80));
            tempVect.x = enemy->actor.radius / 3.0f + Random((u32)(enemy->actor.radius / 2.0f));
            if (Random(2)) {
                tempVect.x = -tempVect.x;
            }
            tempVect.y = enemy->actor.radius / 3.0f + Random((u32)(enemy->actor.radius / 2.0f));
            if (Random(2)) {
                tempVect.y = -tempVect.y;
            }
            tempVect.z = enemy->actor.radius / 3.0f + Random((u32)(enemy->actor.radius / 2.0f));
            if (Random(2)) {
                tempVect.z = -tempVect.z;
            }
            ADD_VECTORS(&tempVect2, &enemy->actor.pos, &tempVect);
            for (j = 0; j < 6; j++) {
                tempVect.x = Random(21) - 10;
                tempVect.y = Random(21) - 10;
                tempVect.z = Random(21) - 10;
                bullet = CreateNewBullet(FLAME_BULLET);
                bullet->actor.pos = tempVect2;
                bullet->actor.vel = tempVect;
                InitBulletSpritesOrMesh(bullet, NULL, 0);
                AddBullet(bullet);
                bullet->sprites[0].anim.def.startScale = Random(256) + 312;
                bullet->sprites[0].anim.def.endScale = Random(56) + 356;
                bullet->sprites[1].anim.def.startScale = Random(256) + 312;
                bullet->sprites[1].anim.def.endScale = Random(56) + 356;
                bullet->actor.flags &= ~0x20000;
            }
        }
        enemy->actor.rot.z += 0.01;
        enemy->brain.delayTimer = 5;
        SCALE_VECTOR_2D(&enemy->actor.vel, 0.6);
        enemy->actor.vel.y -= gravity / 10.0f;
        if (enemy->samples[0].handle) {
            if (Random(40) < 4) {
                StopContinuousSample(&enemy->samples[0]);
            }
        } else if (Random(40) < 4) {
            PlayContinuousSample(&enemy->samples[0], 339, 160, &enemy->actor.pos, 80);
        }
        if (enemy->actor.pos.y <= enemy->actor.radius * 3.0f / 4.0f) {
            KillEnemy(enemy);
        }
        return;
    }
    for (x = 0; x < 3; x++) {
        if (immuneCount[x]) {
            immuneCount[x]--;
        }
        if (portionHealth[x] != 0 && portionHealth[x] < 3 && (portionHealth[x] == 1 || Random(20) < 7)) {
            bullet = CreateNewBullet(SMOKE_BULLET);
            bullet->actor.pos = (&enemy->actor.obj->spheres[1])[x].pos;
            bullet->actor.pos.y += 90.0f;
            bullet->actor.vel.x = enemy->actor.vel.x + (Random(4) - 2) * 2;
            bullet->actor.vel.z = enemy->actor.vel.z + (Random(4) - 4);
            bullet->actor.vel.y = enemy->actor.vel.y + 12.0f;
            InitBulletSpritesOrMesh(bullet, NULL, 0);
            bullet->sprites[0].scaleX = bullet->sprites[0].scaleY = bullet->sprites[0].anim.def.startScale =
                bullet->sprites[0].anim.def.endScale = (Random(56) + 156) * 3;
            AddBullet(bullet);
        }
    }
    if (enemy->brain.bobPeriod) {
        enemy->actor.vel.y += SineWave(enemy->brain.bobPeriod, 0.0f, bobCounter);
    }
    if (enemy->brain.instrIndex == 1) {
        if (enemy->brain.bobPeriod == 0) {
            initFGObject("panel.ndo", 0.0f, 0.0f, -300.0f, 0.0f, 3.1415927f, 0.0f, 1.0f, 1.0f, 1.0f, 255);
            enemy->brain.bobPeriod = 8;
            bobCounter = 0;
            onCounter = 0;
            lastFireFrame = 0;
            enemy->brain.scratchF = 5.0f;
        }
        onCounter++;
        enemies[0].actor.anim->speed = MIN(onCounter / 40.0f, enemies[0].actor.anim->anims[5].speed);
    }
    for (portion = 0; portion < 2; portion++) {
        if (immuneCount[portion]) {
            TagToFirstPlatform(portion + 1)->path->moving = -3;
            TagToFirstPlatform(portion + 4)->path->moving = -3;
            continue;
        }
        platform = TagToFirstPlatform(portion + 1);
        platform2 = TagToFirstPlatform(portion + 4);
        if (platform->path->moving > 0 || platform2->path->moving > 0) {
            platform->path->moving = -3;
            platform2->path->moving = -3;
            immuneCount[portion] = 20;
            if (portion == 0) {
                enemy->actor.rotVel.y = 0.3f;
                enemy->actor.rot.y = Aabs(enemy->actor.rot.y + 0.35);
            } else {
                enemy->actor.rotVel.y = -0.3f;
                enemy->actor.rot.y = Aabs(enemy->actor.rot.y - 0.35);
            }
            if (--portionHealth[portion] == 0) {
                if ((enemy->actor.anim->current == 5 || enemy->actor.anim->current == 6) && enemy->actor.anim->loop == 0) {
                    enemy->actor.anim->loop = 1;
                }
                for (x = 0; x < 10; x++) {
                    bullet = CreateNewBullet(FLAME_BULLET);
                    bullet->actor.pos = platform->actor.pos;
                    bullet->actor.vel.x = (platform->actor.pos.x - enemy->actor.pos.x) / 8.0f + Random(31) - 15.0f;
                    bullet->actor.vel.y = Random(31) - 15;
                    bullet->actor.vel.z = Random(31) - 15;
                    InitBulletSpritesOrMesh(bullet, NULL, 0);
                    AddBullet(bullet);
                    bullet->sprites[0].anim.def.startScale = Random(256) + 312;
                    bullet->sprites[0].anim.def.endScale = Random(56) + 356;
                    bullet->sprites[1].anim.def.startScale = Random(256) + 312;
                    bullet->sprites[1].anim.def.endScale = Random(56) + 356;
                    bullet->actor.flags &= ~0x20000;
                }
                PlaySample(106, &platform->actor.pos, 100, 128);
                enemy->actor.obj->spheres[portion + 1].platTag[0] = enemy->actor.obj->spheres[portion + 1].platTag[1] = 0;
                enemy->actor.obj->spheres[portion + 4].platTag[0] = enemy->actor.obj->spheres[portion + 4].platTag[1] = 0;
                SUB_VECTORS(&platform->actor.vel, &platform->actor.pos, &enemy->actor.pos);
                platform->actor.vel.y *= 0.2;
                platform->actor.vel.x *= 0.2;
                platform->actor.vel.z *= 0.2;
                platform->actor.vel.y += 10.0f;
                GiveActorRandomQRotVel(&platform->actor);
                platform->actor.spinSpeed = 0.4f;
                platform->motion->damping = 1.0f;
                SUB_VECTORS(&platform2->actor.vel, &platform2->actor.pos, &enemy->actor.pos);
                platform2->actor.vel.y *= 0.18;
                platform2->actor.vel.x *= 0.18;
                platform2->actor.vel.z *= 0.18;
                platform2->actor.vel.y += 6.0f;
                GiveActorRandomQRotVel(&platform2->actor);
                platform2->actor.spinSpeed = 0.4f;
                platform2->motion->damping = 1.0f;
                enemy->actor.state->hitFadeTimer = 20;
                enemy->actor.state->hits--;
                PlaySample(99, &platform->actor.pos, 180, 200);
            } else {
                PlaySample(121, &platform->actor.pos, 220, 60);
            }
        }
    }
    if (enemy->brain.bobPeriod && (frameCount & 1)) {
        tempVect.x = 80.0f;
        tempVect.y = -100.0f;
        tempVect.z = 0.0f;
        RotateVector2D(&tempVect, &tempVect, robotDir);
        tempVect2.x = 0.0f;
        tempVect2.y = -20.0f;
        tempVect2.z = 0.0f;
        tempVect.x += enemy->actor.pos.x;
        tempVect.y += enemy->actor.pos.y;
        tempVect.z += enemy->actor.pos.z;
        tempVect2.x += enemy->actor.vel.x;
        tempVect2.y += enemy->actor.vel.y;
        robotDir += 0.4;
        tempVect2.z += enemy->actor.vel.z;
        debris = AddDebris(PUFF, &tempVect, &tempVect2, NULL, NULL);
        if (debris != NULL) {
            c = Random(5);
            debris->sprite.r = robPuffCol[c][0];
            debris->sprite.g = robPuffCol[c][1];
            debris->sprite.b = robPuffCol[c][2];
            debris->sprite.anim.def.startScale *= 5;
            debris->sprite.anim.def.endScale *= 5;
            debris->sprite.scaleX *= 5;
            debris->sprite.scaleY *= 5;
        }
        tempVect.x = -80.0f;
        tempVect.y = -100.0f;
        tempVect.z = 0.0f;
        RotateVector2D(&tempVect, &tempVect, robotDir);
        tempVect.x += enemy->actor.pos.x;
        tempVect.y += enemy->actor.pos.y;
        tempVect.z += enemy->actor.pos.z;
        debris = AddDebris(PUFF, &tempVect, &tempVect2, NULL, NULL);
        if (debris != NULL) {
            c = Random(5);
            debris->sprite.r = robPuffCol[c][0];
            debris->sprite.g = robPuffCol[c][1];
            debris->sprite.b = robPuffCol[c][2];
            debris->sprite.anim.def.startScale *= 5;
            debris->sprite.anim.def.endScale *= 5;
            debris->sprite.scaleX *= 5;
            debris->sprite.scaleY *= 5;
        }
    }
    if (enemy->brain.scratch) {
        enemy->brain.delayTimer = 3;
        if (enemy->brain.scratch <= spbflytime) {
            if (enemy->brain.scratch > 140) {
                enemy->brain.scratchF = 5.0f;
                enemy->samples[0].volume = (enemy->samples[0].volume - 2 < 20) ? 20 : enemy->samples[0].volume - 2;
                enemy->actor.flags |= 0x01000000;
                tempVect.x = enemy->actor.pos.x;
                tempVect.y = 2000.0f;
                tempVect.z = enemy->actor.pos.z;
                NMEActionMoveTo(enemy, &tempVect, 100.0f, 2);
            } else {
                enemy->samples[0].volume = MIN(enemy->samples[0].volume + 2, 160);
                enemy->actor.phys->accel = 10.0f;
                enemy->actor.flags &= ~0x01000000;
                tempVect.x = enemy->actor.pos.x;
                tempVect.y = 350.0f;
                tempVect.z = enemy->actor.pos.z;
                if (NMEActionMoveTo(enemy, &tempVect, 100.0f, 2)) {
                    enemy->brain.scratch = 1;
                }
                bobCounter = 0;
            }
        }
        if (--enemy->brain.scratch == 0) {
            enemy->actor.phys->accel = 5.0f;
        }
    }
    if (enemy->brain.scratch >= 60) {
        return;
    }
    if (enemy->brain.delayTimer) {
        tempVect.x = hand.pos.x - enemy->actor.pos.x;
        tempVect.y = hand.pos.y - enemy->actor.pos.y;
        tempVect.z = hand.pos.z - enemy->actor.pos.z;
        enemy->actor.rotAim.y = Aabs(atan2(tempVect.x, tempVect.z));
    }
    if (gameInfo.cameoActive) {
        return;
    }
    switch (enemy->actor.anim->current) {
        case 5:
        case 6:
            if (enemy->actor.anim->loop == 1
                && ((enemy->actor.state->hits == 1 && !(frameCount & 1)) || enemy->actor.anim->speed == 0.0f
                    || (frameCount % 20 == 0 && Random(50) < 40))) {
                enemy->actor.anim->loop = 0;
                if (enemy->actor.state->hits == 1 || Random(10) < 5) {
                    enemy->brain.fireState[1] = 0;
                    if (enemy->actor.state->hits == 1 && enemy->brain.scratchF == 0.0f) {
                        AnimateActor(&enemy->actor, 11, 0, enemy->actor.anim->speed > 0.0f, 0);
                        AnimateActor(&enemy->actor, Random(2) + 5, 1, 1, 0);
                        break;
                    }
                    if (enemy->brain.scratchF != 0.0f) {
                        enemy->brain.scratchF -= 1.0f;
                    }
                    AnimateActor(&enemy->actor, 12, 0, enemy->actor.anim->speed > 0.0f, 0);
                    if (enemy->actor.state->hits >= 2 && Random(10) < 6) {
                        AnimateActor(&enemy->actor, Random(2) + 5, 1, 1, 0);
                    }
                } else if (Random(10) < 5 && portionHealth[0] && portionHealth[1]) {
                    AnimateActor(&enemy->actor, 10, 0, enemy->actor.anim->speed > 0.0f, 0);
                    AnimateActor(&enemy->actor, Random(2) + 5, 1, 1, 0);
                } else {
                    if (portionHealth[0]) {
                        AnimateActor(&enemy->actor, 0, 0, enemy->actor.anim->speed > 0.0f, 0);
                    }
                    if (portionHealth[1]) {
                        AnimateActor(&enemy->actor, 1, 0, enemy->actor.anim->speed > 0.0f || portionHealth[0], 0);
                    }
                    AnimateActor(&enemy->actor, Random(2) + 5, 1, 1, 0);
                }
            }
            break;
        case 11:
            if (QueryAnimTimeInline(&enemy->actor, 178.0f)) {
                PlaySample(120, &enemy->actor.pos, 140, 50);
            }
            if (QueryAnimTimeInline(&enemy->actor, 210.0f)) {
                PlaySample(120, &enemy->actor.pos, 140, 50);
                enemy->brain.scratch = spbflytime + 1;
            }
            if (QueryAnimTimeInline(&enemy->actor, 198.0f) && lastFireFrame + 5 < frameCount) {
                lastFireFrame = frameCount;
                enemy->brain.fireState[0] = 2;
                FireBulletFromActor(&enemy->actor, BOMB2_BULLET, 0);
                PlaySample(93, &enemy->actor.pos, 140, 50);
            }
            break;
        case 12:
            if (QueryAnimTimeInline(&enemy->actor, 156.0f) && lastFireFrame + 5 < frameCount) {
                lastFireFrame = frameCount;
                enemy->brain.fireState[0] = 1;
                FireBulletFromActor(&enemy->actor, LASERBOLT_BULLET, 0);
                PlaySample(84, &enemy->actor.pos, 140, 50);
            }
            if (enemy->actor.anim->queue[0] == -1
                && (enemy->actor.anim->finishedAnim == 13 || enemy->actor.anim->time >= 175.5)) {
                enemy->actor.anim->current = 5;
                enemy->actor.anim->speed = 0.0f;
                enemy->actor.anim->loop = 1;
            }
            break;
        case 10:
            if (QueryAnimTimeInline(&enemy->actor, 65.0f) && lastFireFrame + 5 < frameCount) {
                lastFireFrame = frameCount;
                if (portionHealth[0] && portionHealth[1]) {
                    enemy->brain.fireState[0] = 2;
                    enemy->brain.fireState[1] = 0;
                } else {
                    enemy->brain.fireState[0] = 1;
                    if (portionHealth[0]) {
                        enemy->brain.fireState[1] = 0;
                    } else {
                        enemy->brain.fireState[1] = 1;
                    }
                }
                FireBulletFromActor(&enemy->actor, MISSILE_BULLET, 0);
                PlaySample(106, &enemy->actor.pos, 250, 80);
            }
            break;
        case 0:
            if (QueryAnimTimeInline(&enemy->actor, 93.0f) && lastFireFrame + 5 < frameCount) {
                lastFireFrame = frameCount;
                if (portionHealth[0]) {
                    enemy->brain.fireState[0] = 1;
                    enemy->brain.fireState[1] = 0;
                    FireBulletFromActor(&enemy->actor, MISSILE_BULLET, 0);
                    PlaySample(106, &enemy->actor.pos, 140, 80);
                }
            }
            break;
        case 1:
            if (QueryAnimTimeInline(&enemy->actor, 121.0f) && lastFireFrame + 5 < frameCount) {
                lastFireFrame = frameCount;
                if (portionHealth[1]) {
                    enemy->brain.fireState[0] = 1;
                    enemy->brain.fireState[1] = 1;
                    FireBulletFromActor(&enemy->actor, MISSILE_BULLET, 0);
                    PlaySample(106, &enemy->actor.pos, 140, 80);
                }
            }
            break;
    }
}

void AddEvilGloveSmoke(Actor *enemy, Vec3 *vel) {
    Vec3 tempVect;
    Debris *d;
    f32 temp;

    tempVect.x = sb3x;
    tempVect.y = sb3y;
    tempVect.z = 4.0f;
    RotateVectorByQuaternion(&tempVect, &tempVect, &enemy->rotQuat);
    tempVect.x += enemy->pos.x;
    tempVect.y += enemy->pos.y;
    tempVect.z += enemy->pos.z;
    d = AddDebris(SMOKETRAIL, &tempVect, vel, NULL, NULL);
    if (d) {
        d->flags &= ~0x800000;
        temp = Random(110) - 55;
        d->sprite.scaleX += temp;
        d->sprite.scaleY += temp;
        d->sprite.anim.def.startScale += temp;
    }
    tempVect.x = -sb3x;
    tempVect.y = sb3y;
    tempVect.z = 4.0f;
    RotateVectorByQuaternion(&tempVect, &tempVect, &enemy->rotQuat);
    tempVect.x += enemy->pos.x;
    tempVect.y += enemy->pos.y;
    tempVect.z += enemy->pos.z;
    d = AddDebris(SMOKETRAIL, &tempVect, vel, NULL, NULL);
    if (d) {
        d->flags &= ~0x800000;
        temp = Random(110) - 55;
        d->sprite.scaleX += temp;
        d->sprite.scaleY += temp;
        d->sprite.anim.def.startScale += temp;
    }
}

void NMEProcessHubChicken(Enemy *enemy) {
    char name[12];
    s32 type;
    s32 j;

    if (frameCount == 0) {
        downAgain = 0;
        enemy->brain.scratch = 0;
        sprintf(name, "HubChick%d", HubChickenType() + 1);
        hubChickenTempObj = FindSubObjectFromObject(enemy->actor.inst->obj, UpdateCRC(name));
    }
    type = HubChickenType();
    if (downAgain != 0 && --downAgain == 0) {
        for (j = 0; j < hubChickenTempObj->numPosKeys; j++) {
            hubChickenTempObj->posKeys[j].pos.y -= 20.0f;
        }
    }
    if (enemy->brain.scratch == 0) {
        if (frameCount % 20 == enemy->actor.state->index && Random(100) < 5) {
            PlaySample((type + 107), &enemy->actor.pos, 60, 128);
            downAgain = hcCluck[type];
            for (j = 0; j < hubChickenTempObj->numPosKeys; j++) {
                hubChickenTempObj->posKeys[j].pos.y += 20.0f;
            }
        }
        if (enemy->brain.scratch == 0) {
            if (pad[ActiveController].pressed[BUTTON_B]
                && DistanceBetweenPointsSquared(&hand.pos, &enemy->actor.obj->spheres[0].pos) < 6400.0f) {
                enemy->brain.scratch = 1;
                switch (gameInfo.level) {
                    case HUB1:
                        enemy->brain.scratchF = 7.0f;
                        break;
                    case HUB2:
                        enemy->brain.scratchF = 10.0f;
                        break;
                    case HUB3:
                        enemy->brain.scratchF = 11.0f;
                        break;
                    case HUB4:
                        enemy->brain.scratchF = 13.0f;
                        break;
                    case HUB5:
                        enemy->brain.scratchF = 20.0f;
                        break;
                    case HUB6:
                        enemy->brain.scratchF = 21.0f;
                        break;
                    case HUB7:
                        enemy->brain.scratchF = 5.0f;
                        break;
                    case HUB8:
                        do {
                            enemy->brain.scratchF = Random(26);
                        } while (enemy->brain.scratchF == 7.0f || enemy->brain.scratchF == 10.0f
                                 || enemy->brain.scratchF == 11.0f || enemy->brain.scratchF == 13.0f
                                 || enemy->brain.scratchF == 20.0f || enemy->brain.scratchF == 21.0f
                                 || enemy->brain.scratchF == 5.0f);
                        break;
                }
                pad[ActiveController].pressed[BUTTON_B] = 0;
                pad[ActiveController].button[BUTTON_B] = 0;
                pad[ActiveController].latch[BUTTON_B] = 1;
                pad[ActiveController].sincePress[BUTTON_B] = 999999;
                player.health = 3;
                stats[HEALTH_STATS].scale = 0x800;
            }
            if (enemy->brain.scratch == 0) {
                return;
            }
        }
    }
    if (++enemy->brain.scratch % 15 == 2) {
        for (j = 0; j < hubChickenTempObj->numPosKeys; j++) {
            hubChickenTempObj->posKeys[j].pos.y += 20.0f;
        }
    }
    if (enemy->brain.scratch % 15 == 3) {
        hubChickenNum = enemy->brain.scratch / 15;
        type = cheatInfo[(s32)enemy->brain.scratchF].code[hubChickenNum];
        PlayContinuousSample(&enemy->samples[0], cheatSampleData[type].id, 100, &enemy->actor.pos,
                             cheatSampleData[type].pitch);
        for (j = 0; j < hubChickenTempObj->numPosKeys; j++) {
            hubChickenTempObj->posKeys[j].pos.y -= 20.0f;
        }
    }
    if (enemy->brain.scratch % 15 == 14) {
        if (hubChickenNum == 7) {
            enemy->brain.scratch = 0;
        }
        StopContinuousSample(&enemy->samples[0]);
    }
}

s32 HubChickenType(void) {
    if (gameInfo.level >= 0) {
        if (gameInfo.level < 3) {
            return 2;
        }
        if (gameInfo.level < 5) {
            return 1;
        }
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/nmeprocess2", pad_801A6814);

INCLUDE_RODATA("asm/nonmatchings/nmeprocess2", pad_8010B0DC);
