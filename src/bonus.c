#include "common.h"
#include "ident.h"
#include "camera.h"
#include "ball.h"
#include "hand.h"
#include "platform.h"
#include "bullet.h"
#include "maths.h"
#include "game.h"
#include "controller.h"
#include "cameo.h"
#include "debris.h"
#include "sound.h"
#include "actor.h"
#include "banks.h"
#include "jalloc.h"
#include "bonus.h"
#include "object.h"
#include "parse.h"

extern s32 controlMethod;
extern Vec3 zero;
extern s32 fireTimer;
extern f32 tweenjoy;
extern f32 tweensep;
extern s32 frameCount;
Platform *TagToFirstPlatform(s32 tag);
Bullet *FireBulletFromActor(Actor *actor, s32 type, s32 flash);
void NMEGetNewInstruction(Enemy *enemy, s32 instructionNum, s32 overrideConditions);
void AnimateActor(Actor *actor, s32 animNum, s32 loop, s32 queue, f32 speed);

extern s32 atlantisInWater;
extern f32 atbonusval1;
extern f32 atbonusval2;
extern f32 atbonusmass1;
extern s32 maxCamZ;
extern s32 maxHandZ;
extern s32 maxCamTargetZ;
extern s32 minCamZ;
extern s32 minHandZ;
extern s32 minCamTargetZ;

extern f32 cannonxspeed;
extern f32 cannonyspeed;
extern f32 cannonrecoil;
extern f32 cannonrecoilfac;
extern f32 ckbcamfrac;
extern s32 reloadCounter;
extern s32 cannonrand;
extern f32 ckbmulfx;
extern s32 ckbfxnum;
extern char *nmeObjectName[];
extern Vec3 carnivalTempVect2;
extern Vec3 carnivalCamOffVect;
extern Vec3 carnivalCamVect;
extern Vec3 carnivalTempVect;
extern s32 carnivalReload;
extern s32 carnivalTimer;
extern f32 carnivalYPos;
extern s32 ckbScoreDisplayed;
extern s32 ckbNumShots;
extern u8 gamePaused;
extern u8 ActiveController;
extern Pad pad[4];
u32 UpdateCRC(char *ptr);

void DoCarnivalBonus(void) {
    Platform *plat;
    Platform *platform;
    Bullet *bul;
    Debris *d;
    f32 ang;
    s32 numTargetsHit;
    s32 j;

    numTargetsHit = 0;
    plat = TagToFirstPlatform(1);
    if (frameCount == 0) {
        camera.flags &= ~0x10;
        ckbScoreDisplayed = 0;
        carnivalTimer = 0;
        ckbNumShots = 0;
        carnivalReload = 0;
        plat->motion->rot.x = 5.9f;
        hand.link = &ball;
        AnimateActor(&hand, HANDANIM_DENIS, 0, 0, 0);
        carnivalCamOffVect.x = 0.0f;
        carnivalYPos = 0.0f;
        carnivalCamOffVect.y = 40.0f;
        carnivalCamOffVect.z = -60.0f;
    }
    ZERO_VECTOR(&hand.vel);
    for (j = 70; j < 80; j++) {
        platform = TagToFirstPlatform(j);
        if (platform->path->target == 1 || (platform->path->current == 1 && platform->actor.speed == 0.0f)) {
            numTargetsHit++;
        }
    }
    for (j = 90; j < 100; j++) {
        platform = TagToFirstPlatform(j);
        if (platform->path->target == 1 || (platform->path->current == 1 && platform->actor.speed == 0.0f)) {
            numTargetsHit++;
        }
    }
    for (j = 110; j < 120; j++) {
        platform = TagToFirstPlatform(j);
        if (platform->path->target == 1 || (platform->path->current == 1 && platform->actor.speed == 0.0f)) {
            numTargetsHit++;
        }
    }
    if (ckbScoreDisplayed != numTargetsHit) {
        if (numTargetsHit >= ckbScoreDisplayed) {
            ckbScoreDisplayed++;
        } else {
            ckbScoreDisplayed--;
        }
    }
    if (ckbScoreDisplayed == 30) {
        if (carnivalTimer == 0) {
            carnivalTimer = 60;
        } else if (--carnivalTimer == 0) {
            ExitLevel(BONUS_DONE_EXIT, 1, 1);
        }
    } else {
        carnivalTimer = 0;
    }
    if (gameInfo.cameoActive != 0) {
        cameraInfo.followSpeed = 0;
        cameraInfo.followTimer = 5;
    }
    hand.flags = 0;
    hand.state->mode = HAND_NOTHING;
    plat->actor.flags |= 0x4000;
    plat->motion->oldRot.y = plat->motion->rot.y;
    plat->motion->oldRot.x = plat->motion->rot.x;
    if (!gamePaused && (cameoList.head.next == NULL || !cameoList.head.next->active)) {
        plat->motion->rot.y = Aabs(plat->motion->rot.y - pad[ActiveController].stickX / cannonxspeed);
        plat->motion->rot.x = MIN(MAX(5.4977871445, plat->motion->rot.x + pad[ActiveController].stickY / cannonyspeed), 6.282185308);
    }
    ang = FindShortestAngle(plat->motion->oldRot.y, plat->motion->rot.y);
    ang = MAX(ang, FindShortestAngle(plat->motion->oldRot.x, plat->motion->rot.x));
    if (ang != 0.0f) {
        if (handControl.samples[0].handle == 0) {
            PlayContinuousSample(&handControl.samples[0], ckbfxnum, 0, &hand.pos, 128);
        }
        handControl.samples[0].volume = MIN(ang * ckbmulfx, 100.0f);
        handControl.samples[0].pitch = ang * 1000.0f + 128.0f;
    } else if (handControl.samples[0].handle != 0) {
        handControl.samples[0].volume -= 5;
        if (handControl.samples[0].volume <= 0) {
            StopContinuousSample(&handControl.samples[0]);
        }
    }
    if (plat->motion->rot.x != plat->motion->oldRot.x || plat->motion->rot.y != plat->motion->oldRot.y) {
        carnivalYPos = ((f32)Random(cannonrand * 2 | 1) - cannonrand) / 10.0;
    } else {
        carnivalYPos = carnivalYPos / 2.0f;
    }
    platform = TagToFirstPlatform(2);
    platform->motion->rot.y = plat->motion->rot.y;
    carnivalTempVect.x = -12.0f;
    carnivalTempVect.y = -5.0f;
    carnivalTempVect.z = -2.0f;
    RotateVector2D(&platform->actor.pos, &carnivalTempVect, platform->motion->rot.y);
    platform = TagToFirstPlatform(3);
    platform->motion->rot.y = plat->motion->rot.y;
    carnivalTempVect.x = 12.0f;
    carnivalTempVect.y = -5.0f;
    carnivalTempVect.z = -2.0f;
    RotateVector2D(&platform->actor.pos, &carnivalTempVect, platform->motion->rot.y);
    GetQuaternionFromXYZRot(&plat->actor.rotQuat, &plat->motion->rot);
    hand.rotQuat = plat->actor.rotQuat;
    carnivalTempVect.x = 0.0f;
    carnivalTempVect.y = 13.0f;
    carnivalTempVect.z = 0.0f;
    RotateVectorByQuaternion(&carnivalTempVect, &carnivalTempVect, &hand.rotQuat);
    ADD_VECTORS(&hand.pos, &carnivalTempVect, &plat->actor.pos);
    if (!cameoList.head.next->active) {
        carnivalTempVect2.x = carnivalTempVect2.y = 0.0f;
        carnivalTempVect2.z = 40.0f;
        RotateVectorByQuaternion(&carnivalTempVect2, &carnivalTempVect2, &hand.rotQuat);
        RotateVectorByQuaternion(&carnivalCamVect, &carnivalCamOffVect, &hand.rotQuat);
        ADD_TO_VECTOR(&carnivalTempVect2, &hand.pos);
        ADD_TO_VECTOR(&carnivalCamVect, &hand.pos);
        AddScaledVector(&camera.pos, &camera.pos, ckbcamfrac, &carnivalCamVect, 1.0f - ckbcamfrac);
        SetCameraOverride(camera.pos.x, camera.pos.y, camera.pos.z, &carnivalTempVect2, &zero, 10, 0);
        if (!gamePaused && (cameoList.head.next == NULL || !cameoList.head.next->active)) {
            if (carnivalReload != 0) {
                carnivalReload--;
            }
            SCALE_VECTOR(&plat->actor.pos, cannonrecoilfac);
            if (bulletList.count < 8 && carnivalReload == 0 && pad[ActiveController].pressed[BUTTON_A]) {
                carnivalTempVect.x = carnivalTempVect.y = 0.0f;
                carnivalTempVect.z = cannonrecoil;
                RotateVectorByQuaternion(&carnivalTempVect, &carnivalTempVect, &hand.rotQuat);
                ADD_TO_VECTOR(&plat->actor.pos, &carnivalTempVect);
                ckbNumShots++;
                carnivalReload = reloadCounter;
                bul = FireBulletFromActor(&hand, BALL_BULLET, 0);
                if (bul != NULL) {
                    switch (Random(15)) {
                        case 0:
                            FindObject(&bul->actor.inst, UpdateCRC(nmeObjectName[28]), nmeObjectName[28], 1);
                            bul->actor.obj->spheres[0].radius = bul->actor.radius = defaultBehaviour[28].radius;
                            bul->actor.spinSpeed = 0.6f;
                            PlaySample(25, &bul->actor.pos, 80, (Random(24) + 116));
                            break;
                        case 1:
                            FindObject(&bul->actor.inst, UpdateCRC(nmeObjectName[29]), nmeObjectName[29], 1);
                            bul->actor.obj->spheres[0].radius = bul->actor.radius = defaultBehaviour[29].radius;
                            PlaySample(19, &bul->actor.pos, 80, (Random(24) + 116));
                            break;
                        case 2:
                            FindObject(&bul->actor.inst, UpdateCRC(nmeObjectName[30]), nmeObjectName[30], 1);
                            bul->actor.obj->spheres[0].radius = bul->actor.radius = defaultBehaviour[30].radius;
                            PlaySample(21, &bul->actor.pos, 80, (Random(24) + 116));
                            break;
                    }
                    PlaySample(27, &bul->actor.pos, 80, (Random(24) + 116));
                    AddDebris(POPB, &bul->actor.pos, &zero, NULL, NULL);
                    ang = Random(62) / 10;
                    for (j = 0; j < 5; j++) {
                        carnivalTempVect.x = carnivalTempVect.z = 0.0f;
                        carnivalTempVect.y = 8.0f;
                        RotateVector2DXYZ(&carnivalTempVect, &carnivalTempVect, ang + j * 2 * 3.141592654 / 5.0, 2);
                        RotateVectorByQuaternion(&carnivalTempVect, &carnivalTempVect, &hand.rotQuat);
                        ADD_VECTORS(&carnivalCamVect, &carnivalTempVect, &bul->actor.pos);
                        d = AddDebris(WIND_BIT, &carnivalCamVect, &carnivalTempVect, NULL, NULL);
                        if (d != NULL) {
                            d->lifeTimer = 5;
                            d->info->friction = 0.6f;
                            d->friction = 0.6f;
                        }
                    }
                    for (j = 0; j < 10; j++) {
                        carnivalTempVect = bul->actor.vel;
                        carnivalTempVect.x += Random(15) - 7;
                        carnivalTempVect.y += Random(15) - 7;
                        carnivalTempVect.z += Random(15) - 7;
                        if (j < 5) {
                            d = AddDebris(WIND_BIT, &bul->actor.pos, &carnivalTempVect, NULL, NULL);
                            if (d != NULL) {
                                d->lifeTimer = 8;
                                d->info->friction = 0.6f;
                                d->friction = 0.6f;
                            }
                        } else {
                            d = AddDebris(STAR, &bul->actor.pos, &carnivalTempVect, NULL, NULL);
                            if (d != NULL) {
                                d->flags &= ~0x800000;
                                d->info->friction = 0.6f;
                                d->friction = 0.6f;
                            }
                        }
                    }
                }
            }
            TagToFirstPlatform(60)->motion->speed =
                (frameCount / 30000.0f < 0.025) ? frameCount / 30000.0f * 0.55 + 0.015 : 0.02875f;
            TagToFirstPlatform(157)->motion->speed =
                (frameCount / 30000.0f < 0.025) ? -(frameCount / 30000.0f + 0.015) * 0.66 : -0.0264f;
            TagToFirstPlatform(129)->motion->speed =
                (frameCount / 30000.0f < 0.025) ? (frameCount / 30000.0f + 0.015) * 0.7 : 0.028f;
            plat->actor.pos.y += carnivalYPos;
            carnivalTempVect.x = 0.0f;
            carnivalTempVect.y = 13.0f;
            carnivalTempVect.z = 0.0f;
            RotateVectorByQuaternion(&carnivalTempVect, &carnivalTempVect, &hand.rotQuat);
            ADD_VECTORS(&hand.pos, &carnivalTempVect, &plat->actor.pos);
        }
    }
}

void DoFortressBonus(void) {
    cameraInfo.sideOffsetEnabled = 0;
    SetControlToBall();
    if (ballControl.loseTimer && !cameraInfo.destActive && !cameraInfo.overrideTimer) {
        SetCameraOverride(camera.pos.x, camera.pos.y, camera.pos.z, &cameraInfo.actor.pos, &zero, 60, 0);
    }
}

void DoPrehistoricBonus(void) {
    if (ballControl.loseTimer && !cameraInfo.destActive && !cameraInfo.overrideTimer) {
        SetCameraOverride(camera.pos.x, camera.pos.y, camera.pos.z, &cameraInfo.actor.pos, &zero, 60, 0);
    }
    if (ballControl.loseTimer) {
        camera.pos.z += 5.8;
    }
    SetControlToBall();
    if (ball.pos.z > 3750.0f) {
        ExitLevel(BONUS_DONE_EXIT, 1, 1);
    }
}

void SetControlToBall(void) {
    hand.alpha = 0;
    ZERO_VECTOR(&hand.scale);
    if (controlMethod == CONTROL_HAND && !handControl.noControlTimer) {
        cameraInfo.focus = &ball.pos;
        cameraInfo.focusVel = &ball.vel;
        controlMethod = CONTROL_BALL;
    }
    if (ball.link != &hand) {
        ApplyAction(HAND_JOINED);
    }
}

void SetControlToHand(void) {
    hand.scale.x = hand.scale.y = hand.scale.z = 0.05f;
    hand.alpha = 255;
    cameraInfo.focus = &hand.pos;
    cameraInfo.focusVel = &hand.vel;
    if (!controlMethod) {
        DeactivateCameraControlMode();
    }
    controlMethod = CONTROL_HAND;
}

void DoAtlantisBonus(void) {
    Vec3 tempVect;
    Platform *plat;
    Platform *platform;

    if (hand.state->waterState == INWATER_SWIMMING && !(gameInfo.exitFlags & 1)) {
        if (++atlantisInWater > 20) {
            ExitLevel(BONUS_DONE_EXIT, 0, 0);
        }
    }
    if (camera.pos.z >= maxCamZ) {
        camera.pos.z = maxCamZ;
    } else if (camera.pos.z <= minCamZ) {
        camera.pos.z = minCamZ;
    }
    if (cameraInfo.actor.pos.z >= maxCamTargetZ) {
        cameraInfo.actor.pos.z = maxCamTargetZ;
        cameraInfo.followSpeed = 0;
        cameraInfo.followTimer = 2;
    }
    if (cameraInfo.actor.pos.z <= minCamTargetZ) {
        cameraInfo.actor.pos.z = minCamTargetZ;
        cameraInfo.followSpeed = 0;
        cameraInfo.followTimer = 2;
    }
    if (hand.pos.z > maxHandZ || hand.pos.z < minHandZ) {
        ExitLevel(BONUS_DONE_EXIT, 0, 0);
    }
    if (handControl.frog) {
        handControl.spellTimer[2]++;
    }
    cameraInfo.sideOffsetEnabled = 0;
    if (hand.obj->planes[0] != NULL && hand.vel.y < 0.0f) {
        SCALE_VECTOR_2D(&hand.vel, 0.6);
    }
    if (frameCount == 0) {
        atlantisInWater = 0;
        StopDrawing("atbonus");
        for (plat = platformList.head.next; plat != &platformList.head; plat = plat->next) {
            if (plat->tag == 1 || plat->tag == 2) {
                plat->flags &= ~0x20000;
                tempVect = plat->actor.pos;
                if (plat->tag == 1) {
                    plat->push = JallocAlloc(sizeof(PlatformPush), 1, "Plat_Push");
                    plat->push->axis[1] = 1;
                    SetPlatformPos(plat, &tempVect, 0.0f);
                    plat->maxSpeed = 1000.0f;
                } else if (plat->tag == 2) {
                    platform = JallocAlloc(sizeof(Platform), 1, "Plat");
                    platform->actor.type = PLAT_ACTOR;
                    platform->flags |= 0x80;
                    InitActorStructures(&platform->actor, 0);
                    platform->actor.friction = 1.0f;
                    AddPlatform(platform);
                    platform->tag = 3;
                    platform->maxSpeed = 1000.0f;
                    PlatformAddPoint(platform, tempVect.x, tempVect.y, 600.0f, 0);
                    PlatformAddPoint(platform, tempVect.x, tempVect.y, -4900.0f, 0);
                    platform->path->accel = 0.001f;
                    plat->parent = platform;
                    PlatformAddPoint(plat, 0.0f, 0.0f, 0.0f, -100);
                    PlatformAddPoint(plat, 0.0f, -30.0f, 0.0f, 65);
                    SetPlatformPos(platform, &tempVect, 0.0f);
                    SetPlatformPos(plat, &zero, 0.0f);
                    plat->tag = 0;
                    plat->path->accel = 0.015f;
                    plat->maxSpeed = 3.0f;
                }
            }
        }
        StartDrawing("atbonus");
    }
    if (hand.obj->platform != NULL && hand.obj->platform->parent != NULL
        && hand.obj->platform->parent->tag == 3 && hand.obj->platform->path->moving < -99) {
        hand.obj->platform->path->moving = 25;
    }
    for (plat = platformList.head.next; plat != &platformList.head; plat = plat->next) {
        if (plat->tag == 1) {
            plat->actor.vel.z = 1.0f / atbonusval1;
            if (plat->actor.pos.z > 200.0f) {
                plat->actor.pos.z = -4500.0f;
            }
            plat->push->mass = atbonusmass1;
            plat->actor.friction = atbonusval1;
        }
        if (plat->tag == 3) {
            plat->path->points[0]->pos.y = plat->actor.pos.y;
            plat->path->points[1]->pos.y = plat->actor.pos.y;
            plat->actor.vel.z = -2.0f / atbonusval2;
            if (plat->actor.pos.z < -4500.0f) {
                plat->actor.pos.z = 200.0f;
            }
            plat->actor.friction = atbonusval2;
        }
    }
}

void DoControlRoom(void) {
    Platform *joy = TagToFirstPlatform(4);
    Platform *platform2 = TagToFirstPlatform(5);
    Bullet *bullet;
    Vec3 tempVect;

    if (frameCount < 130) {
        cameraInfo.actor.pos.x = -8.1f;
        cameraInfo.actor.pos.y = 22.7f;
        cameraInfo.actor.pos.z = 268.0f;
        if (frameCount == 1) {
            camera.pos = cameraInfo.destPos;
        }
    }
    camera.flags &= ~0x10;
    if (hand.state->mode == HAND_NOTHING) {
        fireTimer++;
        if (joy->actor.pos.z > -100.0f) {
            joy->actor.pos.z -= (joy->actor.pos.z + 100.0f) / 4.0f;
        }
        if (fireTimer == 30 || fireTimer == 60) {
            bullet = FireBulletFromActor(&hand, SPACEBALL_BULLET, 0);
            joy->actor.pos.z += tweenjoy;
            if (fireTimer == 30) {
                enemies[1].actor.vel.x += 15.0f;
            } else {
                enemies[1].actor.vel.x -= 15.0f;
            }
            if (bullet) {
                bullet->actor.pos.z -= 50.0f;
                bullet->actor.vel.y += 5.0f;
            }
        }
        if (fireTimer >= 70) {
            if (fireTimer == 70) {
                NMEGetNewInstruction(&enemies[1], 4, 1);
                cameraInfo.focus = &enemies[1].actor.pos;
                cameraInfo.focusVel = &enemies[1].actor.vel;
                cameraInfo.destPos = camera.pos;
            }
            cameraInfo.destActive = 1;
        }
        hand.rotAim.y = 3.1415927f;
        tempVect = joy->actor.pos;
        tempVect.y += 17.0f;
        tempVect.z += 4.0f;
        if (fireTimer >= 30) {
            hand.pos.z = joy->actor.pos.z + 4.0f;
        } else {
            AddScaledVector(&hand.pos, &hand.pos, 0.7f, &tempVect, 0.3f);
        }
    } else if (DistanceBetweenPointsSquared(&hand.pos, &joy->actor.pos) < tweensep) {
        fireTimer = 0;
        ZERO_VECTOR(&hand.vel);
        hand.state->mode = HAND_NOTHING;
        AnimateActor(&hand, HANDANIM_SLAM, 0, 0, 0.0f);
        hand.flags &= ~0x11;
    }
    platform2->actor.pos.z = joy->actor.pos.z;
}
