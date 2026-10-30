#include "common.h"
#include "ident.h"
#include "actor.h"
#include "ball.h"
#include "hand.h"
#include "camera.h"
#include "texture.h"
#include "libc.h"
#include "debris.h"
#include "sound.h"
#include "platform.h"
#include "maths.h"
#include "game.h"
#include "save.h"
#include "controller.h"
#include "collect.h"
#include "enemy.h"
#include "spell.h"
#include "nmeprocess2.h"

extern f32 levelHandScale;
extern u8 ActiveController;
extern f32 driftSpeed;
extern f32 driftSpeedAim;
extern s32 controlMethod;

extern Vec3 zero;
extern Texture *ballTextureList[];
extern ObjectInstance *crystalBall;

extern f32 handInertia;
extern Actor plaster[];
extern f32 plasterScale;
extern Quat vertQ;
extern s16 nextBallState[];
extern Quat zeroQuat;
u32 UpdateCRC(char *ptr);
void FindTexture(void *texPtr, u32 texID, s32 verbose);
extern s16 currentSaveSlot;
extern Actor extraBall[3];
extern Texture *lavaTexture;
extern void *snowBallBit;

extern void *crystalBallBit;
extern u8 **ballTextureData;
extern s32 **ballTexturePalette;
extern u8 **ballTextureData2;
extern s32 **ballTexturePalette2;
extern u8 **ballBitTextureData;
extern s32 **ballBitTexturePalette;

extern f32 plasterScaleAim;
extern s32 cheatActivated;
extern s32 frameCount;
extern s16 ballBasePitch[];
extern s16 ballBaseVol[];
extern f32 snowBallGrow;
extern f32 ballSnowSpeed;
extern f32 ballChangeTime;
extern f32 balltransformspeed;
extern f32 balltransformfreq[3];
extern s32 ballOrbiterFreq;
extern s32 ballWithHand;
extern s32 ballStatic;
extern Vec3 ballStart;
extern u8 ballEnemyType;
extern f32 gravity;
extern SpriteAnim garibAnimation[];
void SquashActor(Actor *actor, Vec3 *dir, f32 power, f32 maxSquash);
void InitSpriteAnimation(GameSprite *sprite, SpriteAnim *temp, s16 lifespan);
extern f32 spinPos;
extern f32 spinPosAim;
extern f32 handspiny;
extern f32 handspinyspeed;

void ResetBall(void) {
    char name[32];
    s32 i;
    Quat tempQ;

    if (saveSlot[currentSaveSlot].difficultyLevel == DIFFICULTY_EASY) {
        ballControl.maxPlasters = 4;
    } else if (saveSlot[currentSaveSlot].difficultyLevel == DIFFICULTY_MEDIUM) {
        ballControl.maxPlasters = 3;
    } else {
        ballControl.maxPlasters = 2;
    }
    for (i = 0; i < 4; i++) {
        SubActor(&plaster[i]);
    }
    ballControl.numPlasters = 0;
    plaster[0].spinAxis.x = 0.201828f;
    plaster[0].spinAxis.y = -0.235466f;
    plaster[0].spinAxis.z = -0.235466f;
    plaster[0].spinSpeed = 0.921074f;
    plaster[1].spinAxis.x = -0.409999f;
    plaster[1].spinAxis.y = 0.0585713f;
    plaster[1].spinAxis.z = -0.585713f;
    plaster[1].spinSpeed = 0.696714f;
    plaster[2].spinAxis.x = 0.412968f;
    plaster[2].spinAxis.y = 0.132924f;
    plaster[2].spinAxis.z = 0.825935f;
    plaster[2].spinSpeed = 0.360027f;
    plaster[3].spinAxis.x = -0.308204f;
    plaster[3].spinAxis.y = 0.0f;
    plaster[3].spinAxis.z = 0.308204f;
    plaster[3].spinSpeed = 0.900012f;
    for (i = 0; i < 4; i++) {
        GetRotationFromQuaternion(&tempQ, (Quat *)&plaster[i].spinAxis);
        MakeUnit((Vec3 *)&tempQ);
        GetQuaternionFromRotation((Quat *)&plaster[i].spinAxis, &tempQ);
    }
    sprintf(name, "newball2.bmp");
    FindTexture(&ballTextureList[0], UpdateCRC(name), 1);
    sprintf(name, "bowling.bmp");
    FindTexture(&ballTextureList[1], UpdateCRC(name), 1);
    sprintf(name, "power.bmp");
    FindTexture(&ballTextureList[2], UpdateCRC(name), 1);
    sprintf(name, "beach.bmp");
    FindTexture(&ballTextureList[4], UpdateCRC(name), 1);
    sprintf(name, "bearing.bmp");
    FindTexture(&ballTextureList[3], UpdateCRC(name), 1);
    sprintf(name, "snow2.bmp");
    FindTexture(&ballTextureList[5], UpdateCRC(name), 1);
    sprintf(name, "cball.bmp");
    FindTexture(&ballTextureList[6], UpdateCRC(name), 1);
    ball.phys = &defaultBehaviour[0];
    ball.friction = ball.phys->friction;
    ball.type = BALL;
    ball.pos.x = 10.0f;
    ball.flags = 0x60493D;
    ball.obj->spheres[0].radius = ball.obj->spheres[0].targetRadius = levelHandScale * 9.0 / 0.05;
    ball.obj->spheres[1].radius = ball.obj->spheres[1].targetRadius = 4.0f;
    ball.obj->spheres[1].pos.y = 7.5f;
    ball.obj->spheres[1].targetRadius = 5.6f;
    extraBall[0].vel = ball.obj->spheres[1].pos;
    ball.obj->spheres[2].radius = ball.obj->spheres[2].targetRadius = 4.0f;
    ball.obj->spheres[2].pos.x = 4.0f;
    ball.obj->spheres[2].pos.y = -2.0f;
    ball.obj->spheres[2].pos.z = 3.0f;
    ball.obj->spheres[2].targetRadius = 5.7f;
    extraBall[1].vel = ball.obj->spheres[2].pos;
    ball.obj->spheres[3].radius = ball.obj->spheres[3].targetRadius = 4.0f;
    ball.obj->spheres[3].pos.x = -2.0f;
    ball.obj->spheres[3].pos.y = 0.0f;
    ball.obj->spheres[3].pos.z = -5.5f;
    ball.obj->spheres[3].targetRadius = 6.5f;
    extraBall[2].vel = ball.obj->spheres[3].pos;
    ball.obj->numSpheres = 1;
    ball.shadow->shadowSize = ball.obj->spheres[0].radius * 0.85;
    ball.shadow->altitude = -1.0f;
    ball.active = 1;
    ballControl.state = BALL_NORMAL;
    ballControl.enemy = NULL;
    ballControl.snow = 0;
    ballControl.cancerTimer = 0;
    ballControl.transformState = 0;
    CalcMaxActorRadius(&ball);
    ball.alpha = 100;
    ZERO_VECTOR(&ball.vel);
    if (gameInfo.level >= INTRO) {
        ball.flags = 0;
    }
    ball.link = NULL;
}

void GiveBallCancer(s32 time) {
    s32 i;

    for (i = 0; i < 3; i++) {
        AddActor(&extraBall[i]);
    }
    ballControl.cancerTimer = time;
    ballControl.cancerScale = 0;
    ball.obj->numSpheres = 4;
    *ballTextureData = lavaTexture->data;
    *ballTexturePalette = lavaTexture->palette;
    *ballTextureData2 = lavaTexture->data;
    *ballTexturePalette2 = lavaTexture->palette;
    *ballBitTextureData = lavaTexture->data;
    *ballBitTexturePalette = lavaTexture->palette;
}

void HurtBall(void) {
    if (ballControl.hurtTimer) {
        return;
    }
    if (ballControl.snow != 0.0f && ball.obj->spheres[0].radius > defaultBehaviour[ballControl.type].radius + 0.7) {
        BreakOffSnow(1.0f, 3.0f, 1.5f);
        return;
    }
    if (ballControl.type != BALL_MODE_BOWLING && ballControl.type != BALL_MODE_BEARING) {
        if (ballControl.numPlasters == ballControl.maxPlasters || ballControl.type == BALL_MODE_CRYSTAL) {
            BurstBall();
            return;
        }
        if (ballControl.snow != 0.0f) {
            BreakOffSnow(1.0f, 3.0f, 1.5f);
        }
        PlaySample(139, &ball.pos, 100, 128);
        AddActor(&plaster[ballControl.numPlasters++]);
        ballControl.hurtTimer = 60;
        if (IsHandHoldingBall()) {
            ApplyAction(HAND_RELEASE);
        }
        PlayContinuousSample(&ballControl.samples[1], 91, 100, &ball.pos, 160);
    }
}

void MoveBall(void) {
    Vec3 tempVect;
    Vec3 tempVect2;
    Debris *d;
    f32 f2;
    f32 len;
    f32 lim;
    s32 i;
    s32 x;

    if (plasterScale != plasterScaleAim) {
        plasterScale += (plasterScaleAim - plasterScale) / 4.0f;
    }
    if (cheatActivated & 0x10) {
        nextBallState[6] = 8;
    } else {
        nextBallState[6] = -1;
    }
    if (ballControl.hurtTimer) {
        ballControl.hurtTimer--;
        if (!ballControl.loseTimer) {
            tempVect.x = 0.0f;
            tempVect.z = 0.0f;
            tempVect.y = -1.0f;
            x = MAX(ballControl.numPlasters - 1, 0);
            RotateVectorByQuaternion(&tempVect, &tempVect, &plaster[x].rotQuat);
            SCALE_VECTOR(&tempVect, 4.0f);
            tempVect.x += (f32)(Random(101) - 50) / 50.0;
            tempVect.y += (f32)(Random(101) - 50) / 50.0;
            tempVect.z += (f32)(Random(101) - 50) / 50.0;
            AddOneScaledVector(&tempVect2, &plaster[x].pos, &tempVect, ball.radius / LENGTH(&tempVect));
            d = AddDebris(WIND_BIT, &tempVect2, &tempVect, NULL, NULL);
            if (d != NULL) {
                d->sprite.alpha = d->sprite.anim.def.startAlpha = ballControl.hurtTimer * 3 + 60;
                d->sprite.anim.def.endAlpha = 0;
            }
            AddToOneScaledVector(&ball.vel, &tempVect, -0.05f);
            if (ball.phys->maxSquash != 0.0f && frameCount % 3 == 0 && Random(10) < 7) {
                MakeUnit(&tempVect);
                SquashActor(&ball, &tempVect, (f32)Random(7) * 0.01 + 0.07, ball.phys->maxSquash);
            }
            ballControl.samples[1].volume = ballControl.hurtTimer * 3;
            ballControl.samples[1].pitch = ballControl.hurtTimer + 160;
        } else {
            ballControl.samples[1].pitch = (ballControl.hurtTimer >> 1) + 200;
            ballControl.samples[1].volume = ballControl.hurtTimer * 1.6 + 80.0;
        }
        UpdateContinuousSample(&ballControl.samples[1]);
    } else if (ballControl.samples[1].handle) {
        StopContinuousSample(&ballControl.samples[1]);
    }
    if (ball.obj->onGround && ball.obj->resting[0]) {
        ballControl.samples[0].volume = MIN(MAX(0.0f, (ball.speed - gravity) * 5.0f), 100.0f);
        if (ballControl.samples[0].volume > 0) {
            ballControl.samples[0].volume = MIN(ballControl.samples[0].volume * ballBaseVol[ballControl.type] / 256, 100);
        }
        ballControl.samples[0].pitch = MIN(MAX(0.0f, ballBasePitch[ballControl.type] + ball.speed * 30.0f), 254.0f);
        if (!ballControl.samples[0].handle) {
            PlayContinuousSample(&ballControl.samples[0], 103, ballControl.samples[0].volume, &ball.pos,
                                 ballControl.samples[0].pitch);
        } else {
            UpdateContinuousSample(&ballControl.samples[0]);
        }
    } else if (ballControl.samples[0].handle) {
        if ((ballControl.samples[0].volume -= 30) <= 0) {
            StopContinuousSample(&ballControl.samples[0]);
        } else {
            UpdateContinuousSample(&ballControl.samples[0]);
        }
    }
    if (ballControl.transformState) {
        if (ballControl.transformState > 0) {
            tempVect.x = 0.0f;
            tempVect.y = SineWave(5.0f, 0.0f, frameCount) * 5.0f;
            tempVect.z = ball.radius + 4.0f;
            RotateVector2D(&tempVect, &tempVect, frameCount / balltransformspeed);
            tempVect2.x = 0.0f;
            tempVect2.y = 5.0f;
            tempVect2.z = 0.0f;
            ADD_TO_VECTOR(&tempVect, &ball.pos);
            d = AddDebris(PUFF, &tempVect, &tempVect2, NULL, NULL);
            if (d != NULL) {
                d->lifeTimer = d->life = d->sprite.anim.def.life = d->sprite.anim.lifeTimer = 20;
                d->sprite.r = SineWave(balltransformfreq[0], frameCount, 0) * 26.0f + 230.0f;
                d->sprite.g = SineWave(balltransformfreq[1], frameCount, 0) * 26.0f + 230.0f;
                d->sprite.b = SineWave(balltransformfreq[2], frameCount, 0) * 26.0f + 230.0f;
                d->flags &= ~0x800000;
                d->info->friction = 0.8f;
                d->friction = 0.8f;
                d->sprite.anim.def.endAlpha = 0;
                d->sprite.flags |= 1;
            }
        }
        if (ballControl.transformState < 0) {
            ballControl.transformState++;
        }
    }
    if (gameInfo.level == PREHISTORICBOSS) {
        snowBallGrow = 0.2f;
    } else {
        snowBallGrow = 0.05f;
    }
    if (ballControl.snow == 0.0f && ballControl.changeTime <= ballChangeTime) {
        BlendBallTexture(ballControl.type, ballControl.prevType, ballControl.changeTime / ballChangeTime);
    }
    ballControl.changeTime++;
    if (ballStatic) {
        ball.flags &= ~0x19;
        if (!ballWithHand) {
            ball.pos = ballStart;
            ball.state->squash = ball.state->squashTimer = 0;
            return;
        }
    }
    if (ballChangeTime < ballControl.changeTime && !(cheatActivated & 0x80) && !ballControl.transformState
        && ballControl.type != BALL_MODE_CRYSTAL) {
        if (ball.obj->planes[0] != NULL
            && (((CollFace *)ball.obj->planes[0])->terrain == 10 || ((CollFace *)ball.obj->planes[0])->terrain == 11)
            && LENGTH_2D(&ball.vel) > 3.0f) {
            if (ballControl.snow == 0.0f) {
                ActivateCollectable(SNOWBALL, 1);
            } else {
                ball.obj->spheres[0].targetRadius =
                    MIN(ball.obj->spheres[0].targetRadius + snowBallGrow, (gameInfo.level == PREHISTORICBOSS) ? 40.0f : 18.0f);
            }
        }
    }
    if (ballControl.snow != 0.0f) {
        ballControl.snow = MAX(0.01, (ball.obj->spheres[0].radius - defaultBehaviour[ballControl.type].radius) / ballSnowSpeed);
        BlendBallTexture(5, ballControl.type, ballControl.snow);
        ball.phys->mass = (ball.obj->spheres[0].radius - 9.0) * 10.0 / 18.0 + 10.0;
        ball.phys->accel = 4.5 / ball.obj->spheres[0].radius;
        if (gameInfo.level != PREHISTORICBOSS || (hand.pos.z < -42.0f && ball.pos.z < -42.0f)) {
            if ((ballControl.hitFlags & 8) || (ball.obj->planes[0] != NULL && LENGTH(&ball.prevVel) > 10.0f)) {
                BreakOffSnow(1.0f, 3.0f, 1.5f);
            }
        } else {
            if (ballControl.hitFlags & 8) {
                BreakOffSnow(1.0f, 3.0f, 1.5f);
            }
            if (ball.obj->planes[0] != NULL
                && LENGTH(&ball.prevVel) > 5.0f
                && ball.obj->planes[0]->y < 0.5) {
                PlaySample(198, &ball.pos, 100, 128);
                ShatterActor(&ball, snowBallBit, 8, 5.0f);
                DeactivateCollectable(SNOWBALL);
                ball.pos = hand.pos;
                ZERO_VECTOR(&ball.vel);
                ball.pos = hand.pos;
                ball.pos.y += 100.0f;
                CreatePuffCircle(&ball.pos);
            }
        }
    }
    if ((hand.state->mode == HAND_JOINED || hand.state->mode == HAND_BALLWALK) && pad[ActiveController].mag == 0.0f) {
        ball.friction = MAX(0.0f, 1.0f - (1.0f - ball.phys->friction) * 4.0f);
    } else {
        ball.friction = ball.phys->friction;
    }
    if (ballControl.releaseTimer) {
        if (--ballControl.releaseTimer == 0) {
            ball.link = NULL;
        }
    }
    if (ballControl.type == BALL_MODE_CRYSTAL && !ballControl.loseTimer) {
        ball.alpha = SineWave(5.0f, 0.0f, frameCount) * 30.0f + 110.0f;
        ball.shadow->shadowAlpha = ball.shadow->shadowTarget = (f32)ball.alpha * 1.8;
        if (!ballControl.loseTimer && frameCount % 3 == 0) {
            tempVect.x = ball.pos.x + Random(15) - 7.0f;
            tempVect.y = ball.pos.y + Random(15) - 7.0f;
            tempVect.z = ball.pos.z + Random(15) - 7.0f;
            SetVector2D(&tempVect2, &ball.vel);
            tempVect2.y = ball.vel.y + gravity;
            AddDebris(FAIRY, &tempVect, &tempVect2, NULL, NULL);
        }
    }
    if (ballControl.cancerTimer) {
        if (ballControl.cancerTimer < 40) {
            ballControl.cancerScale = (ballControl.cancerScale - 0.025 < 0.0) ? 0.0f : ballControl.cancerScale - 0.025;
        } else {
            ballControl.cancerScale = MIN(ballControl.cancerScale + 0.05, 1.0f);
        }
        for (i = 0; i < 3; i++) {
            ball.obj->spheres[i + 1].radius =
                (ball.obj->spheres[i + 1].targetRadius + SineWave(i / 2 + 3, i, frameCount) * 1.2) * ball.scale.x
                * ballControl.cancerScale / 0.05;
            COPY_VECTOR(&ball.obj->spheres[i + 1].pos, &extraBall[i].vel);
            SCALE_VECTOR(&ball.obj->spheres[i + 1].pos, ball.scale.x * ballControl.cancerScale * 1.2 / 0.05);
            RotateVectorByQuaternion(&tempVect, &ball.obj->spheres[i + 1].pos, &ball.rotQuat);
            ADD_VECTORS(&extraBall[i].pos, &ball.pos, &tempVect);
            extraBall[i].rotQuat = ball.rotQuat;
            extraBall[i].scale.x = extraBall[i].scale.y = extraBall[i].scale.z =
                ball.obj->spheres[i + 1].radius * 0.05 / 9.0;
        }
        if (--ballControl.cancerTimer == 0) {
            *ballTextureData = ballTextureList[ballControl.type]->data;
            *ballTexturePalette = ballTextureList[ballControl.type]->palette;
            *ballTextureData2 = ballTextureList[ballControl.type]->data;
            *ballTexturePalette2 = ballTextureList[ballControl.type]->palette;
            *ballBitTextureData = ballTextureList[ballControl.type]->data;
            *ballBitTexturePalette = ballTextureList[ballControl.type]->palette;
            ball.obj->numSpheres = 1;
            for (i = 0; i < 3; i++) {
                SubActor(&extraBall[i]);
            }
        }
    }
    if (ballControl.orbiter.inst != NULL) {
        if (ballControl.orbiter.type - 1000 == 14) {
            tempVect.x = hand.radius + 5.0f;
        } else {
            tempVect.x = ball.radius + 6.0f;
        }
        tempVect.y = SineWave(ballOrbiterFreq, 0.0f, frameCount) * 4.0f;
        tempVect.z = 0.0f;
        RotateVector2D(&tempVect, &tempVect, -ballControl.orbiter.rot.y);
        if (ballControl.orbiter.type - 1000 == 14 && ball.radius <= 0.1) {
            ADD_VECTORS(&ballControl.orbiter.pos, &hand.pos, &tempVect);
        } else {
            ADD_VECTORS(&ballControl.orbiter.pos, &ball.pos, &tempVect);
        }
        ballControl.orbiter.rot.y = Aabs(ballControl.orbiter.rot.y + 0.3);
        GetQuaternionFromXYZRot(&ballControl.orbiter.rotQuat, &ballControl.orbiter.rot);
        if (player.powerups[ballControl.orbiter.type - 1000] < 30) {
            ballControl.orbiter.scale.x -= 0.01;
            ballControl.orbiter.scale.y -= 0.01;
            ballControl.orbiter.scale.z -= 0.01;
        }
        if (player.powerups[ballControl.orbiter.type - 1000] >= 16 && !(frameCount & 1)) {
            tempVect.x = tempVect.z = 0.0f;
            tempVect.y = 5.0f;
            AddSpellDebris(6, &ballControl.orbiter.pos, &tempVect, ballControl.orbiter.type - 1000);
        }
    }
    if (ball.state->teleport == NULL) {
        if (cheatActivated & 0x80) {
            ball.scale.x = ball.scale.y = ball.scale.z = ball.obj->spheres[0].radius * enemyStats[ballEnemyType].scale
                                                        / defaultBehaviour[ballEnemyType].radius;
            if (handControl.checkpoint->restartGate->restartTimer <= 0) {
                ball.obj->spheres[0].targetRadius = defaultBehaviour[ballEnemyType].radius;
            }
        } else {
            if ((cheatActivated & 0x200) && handControl.checkpoint->restartGate->restartTimer <= 0 && !player.powerups[VANISHBALL]) {
                ball.obj->spheres[0].targetRadius = 20.0f;
            }
            ball.scale.x = ball.scale.y = ball.scale.z = ball.obj->spheres[0].radius * 0.05 / 9.0;
        }
    }
    ball.shadow->shadowSize = ball.obj->spheres[0].radius * 0.85;
    if (ballControl.returnTime > 0) {
        if (ballControl.returnTime >= 50) {
            EnemyDropBall();
            ball.flags &= ~1;
            SUB_VECTORS(&tempVect, &hand.pos, &ball.pos);
            MakeUnit(&tempVect);
            SCALE_VECTOR(&tempVect, 1.5);
            ADD_TO_VECTOR(&ball.vel, &tempVect);
            len = LENGTH(&ball.vel);
            lim = 0.6f;
            if (len > lim) {
                SCALE_VECTOR(&ball.vel, MAX(0.9, lim / len));
            }
            ball.speed = LENGTH(&ball.vel);
        } else {
            ballControl.returnTime++;
        }
    }
    if (ballControl.state == BALL_BURST) {
        if (!handControl.deathTimer && controlMethod != CONTROL_BALL) {
            if (hand.state->mode != HAND_LOCATE) {
                ApplyAction(HAND_LOCATE);
            } else {
                InitSpriteAnimation(handControl.locate[1], &garibAnimation[1], 0);
                handControl.locateGaribs = 0;
            }
        }
        hand.vel.y -= gravity;
        if (ballControl.loseTimer > 0) {
            ball.flags &= ~9;
            if (ball.phys->maxSquash != 0.0f) {
                d = AddDebris(WIND_BIT, &ball.pos, &ball.vel, NULL, NULL);
                if (d != NULL) {
                    d->sprite.anim.def.endAlpha = 0;
                    FlipVector(&d->vel);
                }
                if (!(ballControl.loseTimer & 3)) {
                    tempVect.x = Random(10) - 5;
                    tempVect.y = Random(10) - 5;
                    tempVect.z = Random(10) - 5;
                    MakeUnit(&tempVect);
                    SquashActor(&ball, &tempVect, (f32)(Random(4) + 4) / 10.0, 10.0f);
                    f2 = (80 - ballControl.loseTimer) * 10;
                    tempVect.x = ((f32)Random((u32)f2) - f2 / 2.0f) / 15.0f;
                    tempVect.z = ((f32)Random((u32)f2) - f2 / 2.0f) / 15.0f;
                    tempVect.y = (f32)Random((u32)f2) / 50.0f;
                    ball.vel = tempVect;
                }
            } else {
                ball.pos = ball.prevPos;
            }
        }
    }
    if (ballControl.loseTimer > 0) {
        if (--ballControl.loseTimer == 0) {
            LoseLife();
        }
    }
}

void BurstBall(void) {
    if (ballControl.state == BALL_BURST) {
        return;
    }
    if (ballControl.snow != 0.0f) {
        if (ball.obj->spheres[0].radius > defaultBehaviour[ballControl.type].radius + 0.7) {
            ShatterActor(&ball, snowBallBit, 8, 1.0f);
        }
        DeactivateCollectable(SNOWBALL);
    }
    if (ballControl.type == BALL_MODE_CRYSTAL || ballControl.type == BALL_MODE_BOWLING || ballControl.type == BALL_MODE_BEARING) {
        ShatterBall();
        return;
    }
    ballControl.state = BALL_BURST;
    ball.flags &= ~8;
    ball.vel.y += 0.5;
    PlaySample(139, &ball.pos, 100, 128);
    PlayContinuousSample(&handControl.samples[0], 276, 100, &hand.pos, 140);
    LoseBall(60);
    handControl.locateGaribs = 0;
    if (!handControl.deathTimer && controlMethod != CONTROL_BALL) {
        ApplyAction(HAND_LOCATE);
    }
    SCALE_VECTOR(&hand.vel, 0.4);
    ballControl.hurtTimer = 60;
    PlayContinuousSample(&ballControl.samples[1], 91, 100, &ball.pos, 160);
}

inline void LoseBall(s32 time) {
    handControl.joinTimer = time;
    ballControl.loseTimer = time;
    if (IsHandHoldingBall() == TRUE) {
        handControl.actionLock = 10;
        ApplyAction(HAND_RELEASE);
        hand.flags &= ~0x40;
    }
    ball.link = NULL;
    if (ballControl.enemy) {
        EnemyDropBall();
    }
    if (handControl.attachedTo) {
        EnemyDropHand();
    }
}

void SquashActor(Actor *actor, Vec3 *dir, f32 power, f32 maxSquash) {
    Quat tempQ;

    if (power >= 0.0f) {
        power *= actor->phys->squashScale;
        if ((actor->state->squash == 0.0f || actor->state->squashTimer >= 4) && ball.link == actor
            && actor->type == BUGLE && ballControl.type != BALL_MODE_BOWLING) {
            ball.vel.x += power * dir->x * 8.0f;
            ball.vel.y += (ABS(power * dir->y * 8.0f) < 2.0f) ? 2.0f : ABS(power * dir->y * 8.0f);
            ball.vel.z += power * dir->z * 8.0f;
        }
        power = MIN(power, maxSquash);
        if (power < ABS(actor->state->squash)) {
            return;
        }
        if (actor->phys->squashPeriod > 0.0f) {
            actor->state->squashXZ = power + 1.0f;
            actor->state->squashY = 1.0f - power;
        }
        actor->state->squashTimer = 0;
    } else {
        actor->state->squashXZ = maxSquash;
        power = -power;
        actor->state->squashY = power;
        actor->state->squashTimer = -1;
    }
    CrossProductVertical((Vec3 *)&tempQ, dir);
    tempQ.w = acos(dir->y);
    MakeUnit((Vec3 *)&tempQ);
    GetQuaternionFromRotation(&actor->state->squashQ, &tempQ);
    actor->state->squash = power;
}

void DropBall(s16 timer) {
    ball.vel = zero;
    ballControl.state = BALL_NORMAL;
    ballControl.enemy = NULL;
    ballControl.enemyOffset = zero;
    ballControl.releaseTimer = timer;
    ball.flags |= ball.phys->flags;
}

void ReleaseBall(void) {
    hand.shadow->shadowTarget = 255;
    hand.shadow->shadowStep = 30;
    ball.shadow->shadowTarget = 255;
    ball.shadow->shadowStep = 30;
    SetHandRadius(&hand.obj->spheres[0].targetRadius, 13.0f);
    handControl.releaseTimer = 7;
    if (cameraInfo.focus == &ball.pos) {
        cameraInfo.focus = &hand.pos;
        cameraInfo.focusVel = &hand.vel;
    }
    hand.friction = handInertia;
    if (ballControl.transformState > 0) {
        ballControl.transformState = 0;
    }
}

f32 BallBounceMaxSpeed(void) {
    switch (ballControl.type) {
        case BALL_MODE_BOWLING:
        case BALL_MODE_BEARING:
            return 7.0f;
        case BALL_MODE_BEACH:
            return 6.0f;
        case BALL_MODE_CRYSTAL:
            return 5.0f;
        case BALL_MODE_NORMAL:
        case BALL_MODE_POWER:
        case BALL_MODE_SNOW:
            return 8.0f;
    }
    return 0.0f;
}

f32 BallJoinedMaxSpeed(void) {
    if (player.powerups[SPEEDUP]) {
        return 14.0f;
    }
    switch (ballControl.type) {
        case BALL_MODE_NORMAL:
        case BALL_MODE_BOWLING:
        case BALL_MODE_POWER:
        case BALL_MODE_BEARING:
        case BALL_MODE_BEACH:
        case BALL_MODE_SNOW:
            return 7.0f;
        case BALL_MODE_CRYSTAL:
            return 8.4f;
    }
    return 0.0f;
}

s32 BallWalkAllowed(void) {
    f32 temp1;
    f32 temp2;

    if (ballControl.snow != 0.0f && hand.state->mode == HAND_SLAM) {
        return FALSE;
    }
    if (ballStatic) {
        return FALSE;
    }
    if (!(ball.flags & 8)) {
        return FALSE;
    }
    if (hand.pos.y < ball.pos.y + ball.obj->spheres[0].radius + hand.vel.y) {
        return FALSE;
    }
    if (player.powerups[VANISHBALL]) {
        return FALSE;
    }
    if (handControl.actionTime < 2) {
        return FALSE;
    }
    if (ball.state->teleport) {
        return FALSE;
    }
    if (hand.state->teleport) {
        return FALSE;
    }
    if (ballControl.state == BALL_HELD || ballControl.state == BALL_HELD_SPECIAL) {
        return FALSE;
    }
    if (hand.vel.y > 0.0f && hand.state->mode != HAND_SLAM2ST && !handControl.touchedActor) {
        return FALSE;
    }
    if (!ball.obj->onGround && !ball.state->waterState) {
        return FALSE;
    }
    if (!(DistanceBetweenPoints2D(&hand.pos, &ball.pos) < ball.obj->spheres[0].radius * 0.8)) {
        return FALSE;
    }
    if (handControl.touchedActor && ball.pos.y < hand.pos.y - hand.radius - hand.vel.y) {
        return TRUE;
    }
    temp1 = hand.pos.y - ball.pos.y;
    temp2 = hand.radius + ball.obj->spheres[0].radius + 5.0;
    if (ball.state->waterState) {
        temp2 += 3.0;
    }
    if (ABS(temp1) < temp2) {
        return TRUE;
    }
    return FALSE;
}

void MoveHandToBall(void) {
    Vec3 v;
    Vec3 bestPos;
    Vec3 lagPos;
    Vec3 d;
    s32 behind = FALSE;
    s32 facingSameWay;
    f32 scale;
    f32 dot; // assigned in the facing tests but never read; the assignment gives the ROM's float registers

    facingSameWay = GetHandPosAim(&bestPos, &lagPos);
    hand.vel.y = 0.0f;
    if ((hand.flags & 0x2000) || handControl.catapult) {
        hand.pos = bestPos;
        return;
    }
    SUB_VECTORS(&d, &lagPos, &hand.pos);
    if (facingSameWay) {
        if ((dot = d.x * ball.vel.x + d.z * ball.vel.z) < 0.0f) {
            behind = TRUE;
            driftSpeedAim = 1.0f;
        }
    } else if (0.0f < (dot = d.x * ball.vel.x + d.z * ball.vel.z)) {
        behind = TRUE;
        driftSpeedAim = 1.0f;
    }
    if (!behind) {
        driftSpeedAim = MIN(pad[ActiveController].mag / 100.0f / 2.0f, 1.0f);
    }
    driftSpeed += (driftSpeedAim - driftSpeed) / 4.0f;
    scale = (driftSpeed < 0.3) ? 0.3f : driftSpeed;
    if (pad[ActiveController].mag != 0.0f) {
        driftSpeedAim = scale = driftSpeed = 1.0f;
    }
    SUB_VECTORS(&v, &bestPos, &hand.pos);
    SCALE_VECTOR(&v, scale);
    ADD_TO_VECTOR(&hand.pos, &v);
    if (hand.state->mode == HAND_JOINED && !ball.obj->onGround && hand.pos.y < ball.pos.y && ballControl.type != BALL_MODE_BEACH) {
        hand.pos.y = ball.pos.y;
    }
}

s32 GetHandPosAim(Vec3 *bestPos, Vec3 *lagPos) {
    Vec3 tempVect;
    Vec3 w;
    f32 dX;
    f32 dZ;
    f32 angle;

    tempVect.x = 0.0f;
    tempVect.y = 0.0f;
    tempVect.z = (ball.obj->spheres[0].radius - 9.0 < -5.0 ? -5.0 : ball.obj->spheres[0].radius - 9.0) / (0.05 * 0.05) * levelHandScale
          * (hand.scale.x > 0.05 ? 0.05 : hand.scale.x);
    if (player.powerups[HERCULES]) {
        if (ball.obj->spheres[0].radius < 9.0) {
            tempVect.y = 9.0 - ball.obj->spheres[0].radius;
        }
        tempVect.z -= 2.0f;
    }
    RotateVector2DXYZ(&tempVect, &tempVect, -hand.rot.x, 0);
    w = tempVect;
    RotateVector2D(&tempVect, &tempVect, Aabs(hand.rot.y + 3.141592654));
    ADD_VECTORS(bestPos, &ball.pos, &tempVect);
    dX = ball.pos.x - ball.prevPos.x;
    dZ = ball.pos.z - ball.prevPos.z;
    angle = Aabs(atan2(dX, dZ));
    RotateVector2D(&w, &w, angle);
    ADD_VECTORS(lagPos, &ball.pos, &w);
    if ((dX == 0.0f && dZ == 0.0f) || FindShortestAngle(angle, hand.rot.y) < 1.0f) {
        return TRUE;
    }
    return FALSE;
}

void MoveHandToBallStatic(void) {
    Vec3 temp;

    GetHandPosAim(&hand.pos, &temp);
}

#define SPIN_OFFSET() spinPos += MIN(MAX(-handspinyspeed, (spinPosAim - spinPos) / 2.0f), handspinyspeed)

void MoveBallToHand(void) {
    Vec3 tempVect;
    f32 tempY;
    f32 tempY2;
    s32 noXZ = FALSE;

    tempVect.x = 0.0f;
    if (player.powerups[HERCULES]) {
        handspiny = 14.5f;
    } else {
        handspiny = 16.5f;
    }
    switch (hand.anim->current) {
        case HANDANIM_CUP2PEDDLE:
            ball.spinSpeed *= 0.8;
            if (hand.anim->speed > 0.0f && TriggerAtFrame(1285.0f, &hand)) {
                spinPos = 0.0f;
                SetHandRadius(&spinPosAim,
                              handspiny - 3.0f + (ball.obj->spheres[0].radius - levelHandScale * 9.0 / 0.05) * 0.9);
            } else if (hand.anim->speed < 0.0f && hand.anim->time <= 1286.0f &&
                       hand.anim->speed + 1286.0f < hand.anim->time) {
                spinPosAim = 0.0f;
                SetHandRadius(&hand.obj->spheres[0].targetRadius, ball.obj->spheres[0].targetRadius + 0.05);
            }
            if (ABS(ball.radius - 9.0) > 0.2) {
                tempY = hand.pos.y;
                tempY2 = hand.vel.y;
                noXZ = TRUE;
                MoveHandToBall();
                hand.pos.y = tempY;
                hand.vel.y = tempY2;
            }
            SPIN_OFFSET();
            break;
        case HANDANIM_PEDDLE:
            tempVect.y = 0.0f;
            tempVect.z = 0.0f;
            tempVect.x = 1.0f;
            RotateVector2D(&ball.spinAxis, &tempVect, hand.rot.y);
            ball.spinSpeed = -4.05 / ball.radius;
            SPIN_OFFSET();
            break;
        case HANDANIM_SPIN2CUP:
            spinPosAim = 0.0f;
            SetHandRadius(&hand.obj->spheres[0].targetRadius, ball.obj->spheres[0].targetRadius + 0.05);
            if (spinPos != spinPosAim) {
                SPIN_OFFSET();
            }
            if (ABS(ball.radius - 9.0) > 0.2) {
                tempY = hand.pos.y;
                tempY2 = hand.vel.y;
                noXZ = TRUE;
                MoveHandToBall();
                hand.pos.y = tempY;
                hand.vel.y = tempY2;
            }
            break;
        case HANDANIM_CUP2SPIN:
            if (hand.anim->time < 413.0f) {
                spinPos = 0.0f;
                SetHandRadius(&spinPosAim,
                              handspiny + (ball.obj->spheres[0].radius - levelHandScale * 9.0 / 0.05) * 0.85);
            } else if (spinPos != spinPosAim) {
                SPIN_OFFSET();
                ball.spinAxis = *(Vec3 *)&vertQ;
                if (ABS(ball.spinSpeed) <= 0.01) {
                    PlaySample(168, &ball.pos, 100, 128);
                }
                ball.spinSpeed = 2.0f;
            }
            break;
        case HANDANIM_SPIN:
            if (ballControl.transformState > 0) {
                HandTransformBall();
            }
            SetHandRadius(&spinPosAim,
                          handspiny + (ball.obj->spheres[0].radius - levelHandScale * 9.0 / 0.05) * 0.85);
            if (spinPos != spinPosAim) {
                SPIN_OFFSET();
            }
            break;
        case HANDANIM_LEVELDONE:
            spinPos -= 0.5;
            break;
        default:
            spinPos = 0.0f;
            break;
    }
    tempVect.y = 0.0f;
    if (spinPos != 0.0f) {
        tempVect.z = 0.0f;
    } else {
        tempVect.z = ball.obj->spheres[0].radius - levelHandScale * 9.0 / 0.05;
        if (player.powerups[HERCULES]) {
            tempVect.z -= 2.0f;
        }
        RotateVector2DXYZ(&tempVect, &tempVect, 1.0f - hand.rot.x, 0);
        RotateVector2D(&tempVect, &tempVect, Aabs(hand.rot.y + 3.141592654));
    }
    tempVect.y -= spinPos;
    if (noXZ) {
        ball.pos.y = hand.pos.y - tempVect.y;
    } else {
        SUB_VECTORS(&ball.pos, &hand.pos, &tempVect);
    }
    RotateBallWithHand();
    if (hand.state->mode != HAND_SPIN) {
        ball.spinSpeed = 0.0f;
    }
}

void RotateBallWithHand(void) {
    Quat tempQ;
    Quat q;

    vertQ.w = hand.rot.y - handControl.prevRot.y;
    GetQuaternionFromRotation(&q, &vertQ);
    QuaternionMultiply(&ball.rotQuat, &q, &ball.rotQuat);
    tempQ.z = -__sinf(hand.rot.y);
    tempQ.x = __cosf(hand.rot.y);
    tempQ.y = 0.0f;
    tempQ.w = hand.rot.x - handControl.prevRot.x;
    GetQuaternionFromRotation(&q, &tempQ);
    QuaternionMultiply(&ball.rotQuat, &q, &ball.rotQuat);
}

s32 TrackBall(void) {
    s32 track = FALSE;
    s16 mode;

    if (!ballStatic) {
        mode = hand.state->mode;
        if (mode == 3 || mode == 5 || mode == 14 || mode == 15) {
            track = TRUE;
        }
    }
    return track;
}

void ActorStartRoll(Actor *actor, f32 x, f32 z) {
    Vec3 dir;
    Vec3 tempVect2;
    Quat tempQ;
    Quat tempQ2;
    Quat tempQ3;
    Plane *plane = NULL;
    s32 j;
    Platform *platform;
    f32 dot;
    f32 speed;
    f32 factor;
    f32 slip;

    if (actor->radius == 0.0f) {
        return;
    }
    if (actor->flags & 0x4000) {
        if (actor->obj->planes[0] != NULL) {
            FindFlattestPlane(actor, &plane, &j);
            // the repeated dot test gives the ROM's second branch on the same comparison
            if (!(plane->y <= 0.0f
                  && (dot = DOT_PRODUCT(&actor->vel, plane)) >= 0.0f)
                || !(dot >= 0.0f)) {
                if (actor->obj->resting[j] == 0) {
                    platform = actor->obj->contactPlatform;
                } else {
                    platform = actor->obj->platform;
                }
                if (x == 0.0f && z == 0.0f) {
                    if (actor->obj != NULL && actor->obj->prevPlatform != NULL && (actor->obj->prevPlatform->flags & 0x200)) {
                        SUB_VECTORS(&dir, &actor->pos, &actor->platPrevPos);
                    } else {
                        SUB_VECTORS(&dir, &actor->pos, &actor->prevPos);
                    }
                } else {
                    dir.x = x;
                    dir.y = 0.0f;
                    dir.z = z;
                }
                if (platform != NULL && (platform->flags & 0x200) && platform->pullSpeed == 0.0f) {
                    if (platform->motion != NULL) {
                        RotateVectorByQuaternion(&tempVect2, &platform->conveyorVel, &platform->actor.rotQuat);
                        SUB_FROM_VECTOR(&dir, &tempVect2);
                    } else {
                        SUB_FROM_VECTOR(&dir, &platform->conveyorVel);
                    }
                }
                dot = DOT_PRODUCT(&dir, plane);
                speed = sqrtf(MAX(0.0f, DOT_PRODUCT(&dir, &dir) - dot * dot));
                if (speed != 0.0f) {
                    SCALE_VECTOR(&dir, 1.0f / speed);
                    dot = terrain[((CollFace *)plane)->terrain].slip / 3.0f;
                    factor = 1.0f;
                    if (!(dot < 1.0f)) {
                        factor = 1.0f / dot;
                    }
                    speed *= factor;
                    CrossProduct((Vec3 *)&tempQ, (Vec3 *)plane, &dir);
                    tempQ.w = speed / actor->radius;
                } else {
                    tempQ.w = tempQ.x = tempQ.y = tempQ.z = 0.0f;
                }
                factor = 1.0f;
                slip = (terrain[((CollFace *)plane)->terrain].slip - 1.0f) * 3.0f;
                if (!(slip < 1.0f)) {
                    factor = 1.0f / slip;
                }
                if (factor == 1.0f) {
                    *(Quat *)&actor->spinAxis = tempQ;
                    return;
                }
                GetQuaternionFromRotation(&tempQ3, &tempQ);
                GetQuaternionFromRotation(&tempQ2, (Quat *)&actor->spinAxis);
                QuatSlerp(&tempQ2, &tempQ3, factor, &tempQ);
                if (speed == 0.0f && tempQ.w >= 0.999) {
                    tempQ = zeroQuat;
                }
                GetRotationFromQuaternion((Quat *)&actor->spinAxis, &tempQ);
            }
        } else {
            speed = sqrtf(x * x + z * z);
            actor->spinSpeed = speed / actor->radius;
            if (speed != 0.0f) {
                actor->spinAxis.y = 0;
                actor->spinAxis.x = z / speed;
                actor->spinAxis.z = -x / speed;
            }
        }
    } else {
        speed = sqrtf(x * x + z * z);
        actor->rot.x = Aabs(actor->rot.x + speed / actor->radius);
        if (speed > 0.1 || (actor == &ball && hand.state->mode == HAND_BALLWALK && speed != 0.0f)) {
            actor->rotAim.y = Aabs(atan2(z, -x) + 3 * PI / 2);
        }
    }
}

void RollActor(Actor *actor) {
    Platform *platform = actor->obj->platform;
    f32 angle;
    f32 x;
    f32 z;
    f32 a;
    f32 b;
    f32 slip;

    if ((actor->flags & 0x4000) && actor->obj->planes[0]) {
        ActorStartRoll(actor, 0.0f, 0.0f);
        return;
    }
    if (platform && (platform->flags & 0x200) && platform->pullSpeed == 0.0f) {
        if (platform->motion) {
            angle = -platform->motion->rot.y;
            x = -platform->conveyorVel.x;
            z = -platform->conveyorVel.z;
            a = __cosf(angle) * x;
            a -= __sinf(angle) * z;
            b = __cosf(angle) * z;
            b += __sinf(angle) * x;
        } else {
            a = -platform->conveyorVel.x;
            b = -platform->conveyorVel.z;
        }
        a -= actor->prevPos.x - actor->pos.x;
        b -= actor->prevPos.z - actor->pos.z;
    } else {
        a = actor->pos.x - actor->prevPos.x;
        b = actor->pos.z - actor->prevPos.z;
    }
    if (actor->obj->planes[0] && terrain[((CollFace *)actor->obj->planes[0])->terrain].slip > 1.0f) {
        slip = terrain[((CollFace *)actor->obj->planes[0])->terrain].slip / 2.0f;
        if (!(slip < 1.0f)) {
            a /= slip;
        }
        if (!(slip < 1.0f)) {
            b /= slip;
        }
    }
    ActorStartRoll(actor, a, b);
}

void FindFlattestPlane(Actor *actor, Plane **plane, s32 *planeNum) {
    s32 i;

    *plane = actor->obj->planes[0];
    *planeNum = 0;
    for (i = 1; i < 4; i++) {
        if (actor->obj->planes[i] && actor->obj->planes[i]->y > (*plane)->y) {
            *plane = actor->obj->planes[i];
            *planeNum = i;
        }
    }
}

void ShatterBall(void) {
    s32 i;

    ball.obj->spheres[0].radius = ball.obj->spheres[0].targetRadius = 0.0f;
    ball.alpha = 0;
    ZERO_VECTOR(&ball.scale);
    switch (ballControl.type) {
        case BALL_MODE_CRYSTAL:
            PlaySample(138, &ball.pos, 100, 128);
            ShatterActor(&ball, crystalBallBit, 10, 1.0f);
            break;
        case BALL_MODE_BOWLING:
            PlaySample(314, &ball.pos, 100, 128);
            ShatterActor(&ball, crystalBallBit, 10, 1.2f);
            break;
        case BALL_MODE_BEARING:
            PlaySample(138, &ball.pos, 100, 150);
            ShatterActor(&ball, crystalBallBit, 8, 0.8f);
            break;
    }
    LoseBall(30);
    ball.flags &= ~0x10;
    for (i = 0; i < 4; i++) {
        ball.obj->planes[i] = NULL;
    }
    ball.obj->onGround = 0;
    ballControl.state = BALL_BURST;
    ballControl.transformState = 0;
    PlayContinuousSample(&handControl.samples[0], 276, 100, &hand.pos, 140);
}

void ShatterActor(Actor *actor, void *bit, s32 numBits, f32 scaleMult) {
    Vec3 tempVect2;
    Vec3 tempVect;
    f32 angle = 3.1415927f;
    f32 angleStep = 6.283185308 / numBits;
    s32 x;
    Debris *d;

    for (x = 0; x < numBits; x++) {
        tempVect = actor->pos;
        tempVect2.x = 0.0f;
        tempVect2.y = Random(8) + 10;
        tempVect2.z = Random(6) + 5;
        SCALE_VECTOR(&tempVect2, 0.7);
        RotateVector2D(&tempVect2, &tempVect2, angle + (f32)(Random(11) - 5) / 20.0f);
        angle += angleStep;
        d = AddDebris(DESTRUCTIBLE_BIT, &tempVect, &tempVect2, NULL, NULL);
        if (d) {
            d->inst = bit;
            d->lifeTimer = d->life -= 4;
            d->friction = 0.92f;
            d->flags = (d->flags | 0x12000) & ~0x10;
            SCALE_VECTOR(&d->scale, (f32)(Random(6) + 6) / 35.0f * scaleMult);
            if (actor != &ball || ballControl.type == BALL_MODE_CRYSTAL) {
                d->alpha = 100;
            } else {
                d->alpha = 255;
            }
            d->rotVel.x = (f32)Random(10) / 5.0f;
            d->rotVel.y = (f32)Random(10) / 5.0f;
        }
    }
}

void CalcBallTexture(s32 index) {
    u8 *p;
    s32 i;
    s32 j;
    s32 v;

    for (i = 0; i < 16; i++) {
        for (j = 0; j < 16; j++) {
            ballTextureList[index]->data[j * 16 + i] &= 0xF;
            v = (ballTextureList[ballControl.type]->data[j * 16 + i] & 0xF) + 1;
            p = &ballTextureList[index]->data[j * 16 + i];
            if (v > 15) {
                v = 15;
            }
            *p += v << 4;
        }
    }
}

void BlendBallTexture(s32 mode1, s32 mode2, f32 val) {
    s32 i;
    s32 j;
    u16 ca;
    u16 cb;

    for (i = 0; i < 16; i++) {
        if (val >= 0.99) {
            if (val <= 2.0f) {
                for (j = 1; j < 16; j++) {
                    ((u16 *)ballTextureList[mode1]->palette)[j * 16 + i] = ((u16 *)ballTextureList[mode1]->palette)[i];
                }
            }
        } else {
            for (j = 1; j < 16; j++) {
                ca = ((u16 *)ballTextureList[mode1]->palette)[i];
                cb = ((u16 *)ballTextureList[mode2]->palette)[j - 1];
                ca = ((s32)((ca >> 11) * val + (cb >> 11) * (1.0f - val)) << 11) +
                    ((s32)(((ca & 0x7C0) >> 6) * val + ((cb & 0x7C0) >> 6) * (1.0f - val)) << 6) +
                    ((s32)(((ca & 0x3E) >> 1) * val + ((cb & 0x3E) >> 1) * (1.0f - val)) << 1) | 1;
                ((u16 *)ballTextureList[mode1]->palette)[j * 16 + i] = ca;
            }
        }
    }
}

void ChangeBallInto(s32 type) {
    ObjectInstance *tempObj;
    s32 i;

    if ((type == 6 && ballControl.type != type) || (ballControl.type == BALL_MODE_CRYSTAL && type != 6)) {
        tempObj = crystalBall;
        crystalBall = ball.inst;
        ball.inst = tempObj;
        SubActor(&ball);
        AddActor(&ball);
    }
    for (i = 1; i < 7; i++) {
        if (i != type) {
            player.powerups[i + 6] = 0;
        }
    }
    ballControl.changeTime = 0;
    ballControl.prevType = ballControl.type;
    ballControl.type = type;
    BlendBallTexture(ballControl.type, ballControl.prevType, 0.0f);
}

void HandTransformBall(void) {
    PlaySample(176, &ball.pos, 100, 128);
    ballControl.snow = 0.0f;
    ActivateCollectable(nextBallState[ballControl.type], -1);
    ball.spinAxis = *(Vec3 *)&vertQ;
    ballControl.transformState = -20;
    ball.spinSpeed = 2.0f;
    CreateStarCircle(&ball.pos, 5, 19);
}

void UpdatePlasters(void) {
    s32 i;

    for (i = 0; i < ballControl.numPlasters; i++) {
        plaster[i].pos = ball.pos;
        plaster[i].scale = ball.scale;
        plaster[i].scale.x += plasterScale;
        plaster[i].scale.y += plasterScale;
        plaster[i].scale.z += plasterScale;
        QuaternionMultiply(&plaster[i].rotQuat, &ball.rotQuat, (Quat *)&plaster[i].spinAxis);
        *plaster[i].state = *ball.state;
    }
}

void BreakOffSnow(f32 scale, f32 radAim, f32 rad) {
    if (ball.obj->spheres[0].radius > defaultBehaviour[ballControl.type].radius + 0.7) {
        ShatterActor(&ball, snowBallBit, 8, scale);
    }
    ball.obj->spheres[0].targetRadius -= radAim;
    ball.obj->spheres[0].radius -= rad;
    if (ball.obj->spheres[0].targetRadius <= defaultBehaviour[ballControl.type].radius) {
        DeactivateCollectable(SNOWBALL);
    }
}

INCLUDE_RODATA("asm/nonmatchings/ball", pad_8010BAB4);
