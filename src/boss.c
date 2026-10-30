#include "common.h"
#include "cheat.h"
#include "ident.h"
#include "platform.h"
#include "game.h"
#include "water.h"
#include "wind.h"
#include "save.h"
#include "libc.h"
#include "maths.h"
#include "ball.h"
#include "hand.h"
#include "cameo.h"
#include "camera.h"
#include "bullet.h"
#include "sound.h"
#include "quake.h"
#include "controller.h"
#include "texture.h"
#include "overlay.h"
#include "actor.h"
#include "banks.h"
#include "collect.h"
#include "debug.h"
#include "jalloc.h"
#include "boss.h"
#include "anim.h"
#include "bonus.h"
#include "credits.h"
#include "decompress.h"
#include "enemy.h"
#include "frontend.h"
#include "nmeprocess.h"
#include "object.h"
#include "present.h"
#include "nmeprocess2.h"

Platform *TagToFirstPlatform(s32 tag);
void SetFont(u8 type);
void StartNewLevel(s32 num);
f32 Altitude(Vec3 *pos, Vec3 **floor, s32 *plat, s32 coll);

extern s32 frameCount;
extern s16 currentSaveSlot;
extern s32 frameCount2;
extern u8 disablePause;
extern Quat piratesBossQ;
extern char D_80101F58[]; // "setflythr2" followed by linker junk

typedef struct CreditsLine {
    /* 0x00 */ char *title;
    /* 0x04 */ char *name;
} CreditsLine;
extern u8 cheatTempString[];
extern char cheatMsg[];
extern CreditsLine creditsText[];
extern s32 creditsPerson;
extern u8 controllerPresent;
extern u8 ActiveController;
extern OSContPad controllerdata[];
extern s32 cheatNum;
extern s32 flythru2Alpha;
extern f32 flythru2XPos;
extern s32 flythru2Num;
struct Text *NEW_TEXT(s16 x, s16 y, u8 r, u8 g, u8 b, u8 a, f32 sx, f32 sy, u8 oneShot, char *str, u8 centre);

extern u8 gamePaused;
extern s32 osTvType;
extern s32 autoPlayback;
extern s16 levelExitTimer;
extern s16 worldExitTimer;
extern s16 bonusTimer;
extern s16 timeupTimer;
extern s16 timer321;
extern s16 bonusExitTime;
extern f32 bonusTimeFlash;
extern s16 framesPerSec;
extern s16 beepfx;
extern s16 beepfx2;
extern s32 ckbScoreDisplayed;
extern s32 ckbNumShots;
extern char *goString[];
extern char *timeUpString[];
extern BestTime bestTimes[];
extern SaveSlot saveSlot[];
extern Overlay timePicOL;
void DisableControl(s16 time);
void PrepareSong(u8 song);

#define TIME_TRIAL() (saveSlot[currentSaveSlot].difficultyLevel == 2 && gameInfo.world != 0 && \
                      gameInfo.level < INTRO && gameInfo.level != WAYROOM && !autoPlayback)

struct Debris *AddDebris(s32 debrisType, Vec3 *position, Vec3 *velocity, struct GameSprite *after, struct Debris *newDeb);

extern Actor elecPlatform;
extern Actor ckWeapon;
extern Vec3 bossFocalPoint;
extern Vec3 oldHandPos;
extern Quat zeroQuat;
extern Vec3 zero;
extern u8 bombTimer;
extern u8 bombDelay;
extern u8 pianoFalling;
extern u8 noseLit;
u32 UpdateCRC(char *ptr);
void CreateStarCircle(Vec3 *pos, s32 numStars, s32 type);
void CreateDustCircleScaled(Vec3 *pos, s32 numStars, f32 verticalVelocity, f32 scale);
void HurtHand(s32 type, s32 damage);
void FlashScreen(s16 r, s16 g, s16 b, s16 fadeVal, s16 fadeStep);
Bullet *FireBulletFromActor(Actor *actor, s32 type, s32 flash);
void AnimateActor(Actor *actor, s32 animNum, s32 loop, s32 queue, f32 speed);
extern s16 elecTimer;
extern u8 handPain;
extern u8 ballPain;
extern u8 goForPlatform;
extern u8 frankieDead;
extern s32 lightningSfx;
extern s32 thunderClap;
extern u8 frStunCount;
extern Platform *nearPlat;
extern Vec3 frStunPos1;
extern Vec3 frStunPos2;
extern GameSprite *frankieSprite[2];
extern SpriteList spriteList;
void FindTexture(Texture **texPtr, u32 texID, s32 verbose);
void AddLightning(Vec3 *start, Vec3 *end, u8 r, u8 g, u8 b, u8 segments, s16 lifetime, s16 flags, f32 jagginess, f32 thickness);
void NMEGetNewInstruction(Enemy *enemy, s32 instructionNum, s32 overrideConditions);
void CreatePuffCircle(Vec3 *pos);
extern void *snowBallBit;
extern s32 controlMethod;
extern Actor *foregroundObjects[];
extern s32 numForegroundObjects;
extern Earthquake earthQuake;
extern s32 numBubbles;
extern s32 spaceBossShield;
extern s32 spbReloadCounter;
extern u8 spbalpha;
extern f32 spbcamfrac;
extern f32 spbcamfrac2;
extern f32 spbxspeed;
extern f32 spbyspeed;
extern f32 initrotval;
extern f32 panelfactor;
extern SpriteAnim bulletAnimation[];
extern Vec3 spaceBossTempVect2;
extern Vec3 spaceBossCamOffVect;
extern Vec3 spaceBossCamVect;
extern Vec3 spaceBossCamVect2;
extern Vec3 spaceBossRot;
extern s32 spaceBossReload;
extern GameSprite spbCrosshair;
extern u8 spbCrosshairAlpha;
extern s32 spaceBossPanelOn;
void InitSpriteAnimation(GameSprite *sprite, SpriteAnim *temp, s16 lifespan);

void DoBoss(void) {
    char buf[8];

    if (!gamePaused) {
        switch (gameInfo.level) {
            case SPACEBOSS_TWEEN:
                DoControlRoom();
                break;
            case CARNIVALBOSS:
                DoCarnivalBoss();
                break;
            case PIRATESBOSS:
                DoPiratesBoss();
                break;
            case FEARBOSS:
                DoFortressBoss();
                break;
            case PREHISTORICBOSS:
                DoPrehistBoss();
                break;
            case SPACEBOSS3:
                DoSpaceBoss3();
                break;
            case ATLANTISBONUS:
                DoAtlantisBonus();
                break;
            case CARNIVALBONUS:
                DoCarnivalBonus();
                break;
            case PREHISTORICBONUS:
                DoPrehistoricBonus();
                break;
            case FEARBONUS:
                DoFortressBonus();
                break;
            case PIRATESBONUS:
                DoPiratesBonus();
                break;
            case WAYROOM:
                DoWayRoom();
                break;
            case PRESENT:
                DoPresentScreen();
                break;
            case FLYTHRU:
                DoFlythruScreen();
                break;
            case FLYTHRU2:
                DoFlythru2Screen();
                break;
            case ATLANTISBOSS:
                break;
        }
    }
    SetFont(0);
    if (gameInfo.level == CARNIVALBONUS) {
        sprintf(buf, "%d/%d", ckbScoreDisplayed, ckbNumShots);
        NEW_TEXT(160, 210, 255, 255, 255, 255, 0.8f, 0.8f, 1, buf, 1);
    }
    if (frameCount == 0) {
        timeupTimer = 0;
        bonusTimer = levelInfo[gameInfo.level].bonusTime;
        if (osTvType == 0) {
            framesPerSec = 17;
        } else {
            framesPerSec = 20;
        }
        if (bonusTimer == -1 && TIME_TRIAL()) {
            bonusTimer = timer321 = 4;
        } else if (bonusTimer != -1) {
            timer321 = 0;
        }
    }
    if (frameCount == timer321 * framesPerSec) {
        bonusTimer = levelInfo[gameInfo.level].bonusTime;
        if (bonusTimer == -1 && TIME_TRIAL()) {
            bonusTimer = bestTimes[levelInfo[gameInfo.level].bit].time;
        }
        if (bonusTimer != -1) {
            FindSetupAndAddOverlay(&timePicOL, "clock.bmp", 160, 30, 2, 1);
        }
    }
    if (bonusTimer != -1 && !(gameInfo.exitFlags & 1)) {
        if (!gamePaused && frameCount % framesPerSec == 0 && frameCount != 0 && levelExitTimer == 0 &&
            worldExitTimer == 0 && bonusTimer > 0) {
            if (timer321 != 0 && frameCount <= timer321 * framesPerSec) {
                PlaySampleNot3D(beepfx, 200, 128, 128);
            } else if (frameCount % (framesPerSec * 2)) {
                PlaySampleNot3D(beepfx2, 140, 128, 130);
            } else {
                PlaySampleNot3D(beepfx2, 140, 128, 110);
            }
            bonusTimer--;
            bonusTimeFlash = 10.0f;
            if (bonusTimer == 0 && frameCount >= timer321 * framesPerSec) {
                gamePaused = 1;
                timeupTimer = bonusExitTime;
            }
        }
        if (bonusTimeFlash != 0.0f) {
            if (frameCount >= timer321 * framesPerSec - 1) {
                if (timer321 != 0 && frameCount <= timer321 * framesPerSec + 15) {
                    NEW_TEXT(160, 110, 255, 255, 255, 255.0f - (11.0f - bonusTimeFlash) * 25.0f,
                             ((10.0f - bonusTimeFlash) / 10.0f + 1.0f) * 2, ((10.0f - bonusTimeFlash) / 10.0f + 1.0f) * 2,
                             1, goString[gameInfo.language], 1);
                }
                sprintf(buf, "%d:%02d", (bonusTimer + 1) / 60, (bonusTimer + 1) % 60);
                NEW_TEXT(160, 50.0f - (10.0f - bonusTimeFlash) * 4.0f, 255, 255, 255,
                         255.0f - (11.0f - bonusTimeFlash) * 25.0f, (10.0f - bonusTimeFlash) / 10.0f + 0.8,
                         (10.0f - bonusTimeFlash) / 10.0f + 0.8, 1, buf, 1);
            } else {
                sprintf(buf, "%d", bonusTimer);
                NEW_TEXT(160, 110, 255, 255, 255, 255.0f - (11.0f - bonusTimeFlash) * 25.0f,
                         ((10.0f - bonusTimeFlash) / 10.0f + 1.0f) * 2, ((10.0f - bonusTimeFlash) / 10.0f + 1.0f) * 2,
                         1, buf, 1);
            }
            bonusTimeFlash -= 1.0f;
        }
        if (frameCount >= timer321 * framesPerSec) {
            sprintf(buf, "%d:%02d", bonusTimer / 60, bonusTimer % 60);
            NEW_TEXT(160, 50, 255, 255, 255, 255, 0.8f, 0.8f, 1, buf, 1);
        } else {
            DisableControl(2);
            if (bonusTimer == 1) {
                sprintf(buf, goString[gameInfo.language]);
            } else {
                sprintf(buf, "%d", bonusTimer - 1);
            }
            NEW_TEXT(160, 110, 255, 255, 255, 255, 2.0f, 2.0f, 1, buf, 1);
        }
        if (bonusTimer == 0 && frameCount >= timer321 * framesPerSec) {
            if (audioCtrl.current != gameInfo.world * 4 + 30) {
                PrepareSong(gameInfo.world * 4 + 30);
            }
            NEW_TEXT(160, 120, 255, 255, 255, 255, MIN((f32)(bonusExitTime + 1 - timeupTimer) / 10.0, 1.0),
                     MIN((f32)(bonusExitTime + 1 - timeupTimer) / 10.0, 1.0), 1, timeUpString[gameInfo.language], 1);
        }
        if (timeupTimer != 0) {
            if (--timeupTimer == 0) {
                if (saveSlot[currentSaveSlot].difficultyLevel == DIFFICULTY_HARD) {
                    ExitLevel(TIMETRIAL_EXIT, 0, 0);
                } else {
                    ExitLevel(BONUS_DONE_EXIT, 0, 0);
                }
            }
        }
    }
}

void DoPiratesBoss(void) {
    Platform *platform;

    if (LENGTH_2D(&hand.pos) > 1100.0f) {
        MakeUnit2D(&hand.pos);
        SCALE_VECTOR_2D(&hand.pos, 1100.0f);
    }
    if (LENGTH_2D(&ball.pos) > 1100.0f) {
        MakeUnit2D(&ball.pos);
        SCALE_VECTOR_2D(&ball.pos, 1100.0f);
    }
    if (enemies[0].actor.active != 1) {
        platform = TagToFirstPlatform(1);
        if (enemies[0].brain.scratch++ < 60) {
            MakeUnit((Vec3 *)&piratesBossQ);
            platform->motion->rotLimit = 3.1415927f;
            platform->actor.flags |= 0x4000;
            GetQuaternionFromRotation((Quat *)&platform->actor.spinAxis, &piratesBossQ);
        } else {
            platform->actor.spinSpeed = 0.0f;
        }
    }
}

inline void SetKlosetSpeed(Actor *actor, s32 level) {
    f32 accelSpeed;
    f32 animSpeed;

    switch (level) {
        case 3:
            accelSpeed = animSpeed = 1.0f;
            break;
        case 2:
            accelSpeed = 1.6f;
            animSpeed = 1.4f;
            break;
        case 1:
            accelSpeed = 2.3f;
            animSpeed = 2.0f;
            break;
    }
    actor->phys->moveSpeed = accelSpeed;
    actor->phys->accel = actor->phys->moveSpeed * 0.8;
    actor->anim->anims[15].speed = animSpeed;
}

void DoCarnivalBoss(void) {
    Vec3 tempVect;
    Vec3 tempVect2;
    Platform *plat;
    Bullet *pie;
    s16 doIt;
    Enemy *e;
    Actor *tempObj;
    NMEBrain *brain;

    doIt = FALSE;
    e = &enemies[0];
    brain = &e->brain;
    tempObj = &e->actor;
    if (frameCount == 0) {
        SetKlosetSpeed(tempObj, e->actor.state->hits);
        noseLit = pianoFalling = bombTimer = bombDelay = 0;
        elecPlatform.flags |= 4;
        InitActor(&elecPlatform, "klospie.ndo", 0.0f, 600.0f, 0.0f, 0x11);
        elecPlatform.scale.x = 0.05f;
        elecPlatform.scale.y = 0.05f;
        elecPlatform.scale.z = 0.05f;
        AddActor(&elecPlatform);
        InitActor(&ckWeapon, "spring.ndo", 0.0f, 0.0f, 0.0f, 0x17);
        ckWeapon.scale.x = 0.05f;
        ckWeapon.scale.y = 0.05f;
        ckWeapon.scale.z = 0.05f;
        ckWeapon.flags |= 4;
        AddActor(&ckWeapon);
        ckWeapon.phys->radius = 20.0f;
        ckWeapon.phys->friction = 0.9f;
        ckWeapon.phys->maxSpeed = 28.0f;
        ckWeapon.phys->moveSpeed = 28.0f;
        ckWeapon.obj->spheres[0].radius = 20.0f;
        ckWeapon.phys->bounce = 0.5f;
        ckWeapon.friction = 0.9f;
        ckWeapon.rot.x = 1.5707964f;
        PlayContinuousSample(&e->samples[0], 32, 100, &zero, 100);
    }
    UpdateActor(&ckWeapon);
    if (TriggerAtFrame(228.0f, &e->actor)) {
        PlaySample(219, &e->actor.pos, 100, 128);
    }
    if (!ball.obj->wasOnGround && ball.obj->contactPlatform != NULL && ball.obj->contactPlatform->tag == 10) {
        plat = FindTaggedPlatform(&platformList.head, 10);
        tempVect2.y = 0.0f;
        SetVector2D(&tempVect, &ball.pos);
        RotateVector2D(&tempVect, &tempVect, plat->motion->rotVel[1]);
        SubVector2D(&tempVect2, &tempVect, &ball.pos);
        SCALE_VECTOR_2D(&tempVect2, LENGTH_2D(&ball.pos) / 300.0f);
        AddConveyorSpeedToVel(&ball.vel, &tempVect2);
    }
    if (!cameoList.head.next->active) {
        bossFocalPoint = hand.pos;
        SCALE_VECTOR(&bossFocalPoint, 0.5);
        bossFocalPoint.y += LENGTH_2D(&hand.pos) / 6.0f;
        cameraInfo.focus = &bossFocalPoint;
        cameraInfo.targetHeight = 100.0f - LENGTH_2D(&hand.pos) / 3.0f;
        cameraInfo.targetDistance = LENGTH_2D(&hand.pos) * 0.9 + 100.0;
        cameraInfo.orbiting = 1;
        cameraInfo.orbitAngle = atan2(hand.pos.x, hand.pos.z);
        cameraInfo.orbitSpeed = 1.3f;
    }
    if (e->brain.instr.type == NME_SPECIAL && e->brain.instr.arg.i == 0
        && tempObj->anim->current == 15) {
        AccelerateActor2D(tempObj, tempObj->phys->accel, tempObj->phys->moveSpeed);
    }
    if (tempObj->anim->current == 0) {
        if (tempObj->anim->time >= 41.0f && tempObj->anim->time < 51.0f) {
            elecPlatform.pos = tempObj->obj->spheres[1].pos;
        } else {
            elecPlatform.pos.y = -200.0f;
        }
        if (tempObj->anim->time >= 51.0f
            && tempObj->anim->time < tempObj->anim->speed + 51.0f) {
            pie = FireBulletFromActor(&e->actor, PIE_BULLET, 0);
            pie->actor.spinAxis.y = 0.0f;
            pie->actor.spinAxis.z = 0.0f;
            pie->actor.spinAxis.x = 1.0f;
            pie->actor.spinSpeed = 0.3f;
            pie->actor.phys->rotDrag = 1.0f;
            tempVect.y = 0.0f;
            SubVector2D(&tempVect, &hand.pos, &pie->actor.pos);
            tempVect2 = tempVect;
            MakeUnit(&tempVect);
            SCALE_VECTOR(&tempVect, LENGTH_2D(&tempVect2) / 12.0f);
            tempVect.y = 8.0f;
            pie->actor.vel = tempVect;
            SUB_VECTORS(&tempVect2, &hand.pos, &oldHandPos);
            SCALE_VECTOR(&tempVect2, 1.5);
            ADD_TO_VECTOR(&pie->actor.vel, &tempVect2);
            PlaySample(167, &pie->actor.pos, 100, 100);
        }
    }
    if (ball.obj->contactPlatform != NULL && (!IsHandHoldingBall() || hand.state->mode == HAND_AIRBOURNEWHACK)
        && LENGTH_2D(&ball.vel) > 3.0f) {
        if (ball.obj->contactPlatform->tag == 1 && e->actor.state->hits >= 2) {
            plat = FindTaggedPlatform(&platformList.head, 4);
            if (plat->path->moving == -101) {
                plat->path->moving = 1;
                doIt = TRUE;
                FindTaggedPlatform(&platformList.head, 1)->path->moving = 1;
                FindTaggedPlatform(&platformList.head, 2)->path->moving = 1;
            }
        }
        if (ball.obj->contactPlatform->tag == 2 && e->actor.state->hits >= 2) {
            plat = FindTaggedPlatform(&platformList.head, 3);
            if (plat->path->moving == -101) {
                plat->path->moving = 1;
                doIt = TRUE;
                FindTaggedPlatform(&platformList.head, 1)->path->moving = 1;
                FindTaggedPlatform(&platformList.head, 2)->path->moving = 1;
            }
        }
        if (ball.obj->contactPlatform->tag == 6 && noseLit && e->actor.state->hits == 1) {
            ckWeapon.rotQuat = zeroQuat;
            pianoFalling = 1;
            plat = FindTaggedPlatform(&platformList.head, 20);
            plat->path->moving = 50;
            tempVect.y = 0.0f;
            SetVector2D(&tempVect, &tempObj->pos);
            MakeUnit(&tempVect);
            RotateVector2D(&tempVect, &tempVect, 3.1415927f);
            SCALE_VECTOR_2D(&tempVect, 100.0f);
            SetVector2D(&plat->path->points[0]->pos, &tempVect);
            SetVector2D(&plat->path->points[1]->pos, &tempVect);
            SetVector2D(&plat->path->points[2]->pos, &tempVect);
            PlaySample(93, &ball.pos, 150, 128);
            doIt = TRUE;
        }
        if (doIt) {
            switch (e->actor.state->hits) {
                case 3:
                    ckWeapon.pos = ball.obj->contactPlatform->actor.pos;
                    bombDelay = 20;
                    ckWeapon.pos.y += 70.0f;
                    SCALE_VECTOR_2D(&ckWeapon.pos, 1.2);
                    break;
                case 2:
                    FindObject(&ckWeapon.inst, UpdateCRC("boxingg.ndo"), NULL, 0);
                    InitActorAnim(&ckWeapon);
                    ckWeapon.obj->numSpheres = 2;
                    FindSubObjectFromObject(ckWeapon.inst->obj, UpdateCRC("bglvh"))->sphere = &ckWeapon.obj->spheres[1];
                    ckWeapon.obj->spheres[1].flags = 20;
                    ckWeapon.obj->spheres[1].targetRadius = ckWeapon.obj->spheres[1].radius = 20.0f;
                    ckWeapon.pos = ball.obj->contactPlatform->actor.pos;
                    SCALE_VECTOR(&ckWeapon.pos, 1.4);
                    ckWeapon.pos.y += 70.0f;
                    AnimateActor(&ckWeapon, 0, 0, 0, 0.6f);
                    AnimateActor(&ckWeapon, 1, 0, 1, 0.6f);
                    PlaySample(169, &ckWeapon.pos, 150, 128);
                    tempVect.x = ckWeapon.pos.x - tempObj->pos.x;
                    tempVect.z = ckWeapon.pos.z - tempObj->pos.z;
                    ckWeapon.rot.y = ckWeapon.rotAim.y = atan2(tempVect.x, tempVect.z);
                    ckWeapon.rot.x = 6.05f;
                    ckWeapon.flags &= ~1;
                    ckWeapon.phys->rotDrag = 0.0f;
                    break;
                case 1:
                    SetVector2D(&ckWeapon.pos, &tempObj->pos);
                    ckWeapon.scale.x = ckWeapon.scale.y = ckWeapon.scale.z = 0.0001f;
                    ckWeapon.pos.y = 500.0f;
                    ckWeapon.obj->spheres[0].radius = 90.0f;
                    ckWeapon.obj->numSpheres = 1;
                    ckWeapon.phys->radius = 90.0f;
                    ckWeapon.friction = 1.0f;
                    ckWeapon.phys->friction = 1.0f;
                    ckWeapon.flags |= 1;
                    break;
            }
        }
    }
    if (e->actor.state->hits == 2) {
        if (QueryAnimTime(&ckWeapon, 10.0f)) {
            PlaySample(19, &ckWeapon.pos, 170, 128);
        }
        if (DistanceBetweenPoints(&ckWeapon.obj->spheres[1].pos, &tempObj->pos) < 50.0f) {
            AnimateActor(tempObj, 4, 0, 0, 0.0f);
            CreateStarCircle(&tempObj->pos, 5, 19);
            e->brain.delayTimer = -1;
            e->brain.instrIndex = 13;
            e->brain.instrTimer = 1;
            e->actor.state->hits--;
            e->actor.state->hitFadeTimer = 20;
            SetKlosetSpeed(tempObj, e->actor.state->hits);
            PlaySample(33, &tempObj->pos, 160, 128);
            FindTaggedPlatform(&platformList.head, 1)->path->points[0]->pos.y -= 1000.0f;
            FindTaggedPlatform(&platformList.head, 2)->path->points[0]->pos.y -= 1000.0f;
        }
    }
    if (e->actor.state->hits == 1) {
        if (noseLit) {
            plat = FindTaggedPlatform(&platformList.head, 6);
            if (frameCount & 8) {
                plat->actor.pos.y = 151.0f;
            } else {
                plat->actor.pos.y = 800.0f;
            }
            plat = FindTaggedPlatform(plat, 6);
            if (frameCount & 8) {
                plat->actor.pos.y = 800.0f;
            } else {
                plat->actor.pos.y = 151.0f;
            }
        }
        plat = FindTaggedPlatform(&platformList.head, 5);
        if (pianoFalling) {
            SetVector2D(&ckWeapon.pos, &tempObj->pos);
            plat->actor.pos = ckWeapon.pos;
            plat->actor.rotQuat = ckWeapon.rotQuat;
        }
        if (ckWeapon.pos.y < 70.0f) {
            if (ckWeapon.vel.y == -15.0) {
                SetVector2D(&tempVect, &tempObj->pos);
                tempVect.y = 50.0f;
                CreateDustCircleScaled(&tempVect, 6, 0.0f, 8.0f);
                CreateDustCircleScaled(&tempVect, 4, 0.0f, 10.0f);
                StartEarthquakeAtPoint(&tempVect, 8, 2000.0f, 800.0f, 1, 2);
                tempObj->pos.y = -200.0f;
                e->actor.state->hits = 0;
                if (tempObj->active != 2) {
                    PlaySample(130, &ckWeapon.pos, 140, 128);
                }
                KillEnemy(e);
                PrepareSong(gameInfo.world * 4 + 32 + Random(2));
                FindTaggedPlatform(&platformList.head, 10)->motion->damping = 0.97f;
                TagToFirstPlatform(5)->flags &= ~4;
            } else {
                TagToFirstPlatform(5)->flags |= 4;
            }
            if (ckWeapon.vel.y < -5.0f) {
                ckWeapon.pos.y -= ckWeapon.vel.y;
                ckWeapon.vel.y = -ckWeapon.vel.y * 0.3;
            } else {
                ZERO_VECTOR(&ckWeapon.vel);
                ckWeapon.flags &= ~1;
                ckWeapon.pos.y = 70.0f;
            }
        } else {
            TagToFirstPlatform(5)->flags |= 4;
        }
        if (ckWeapon.phys->radius == 90.0f && ckWeapon.vel.y == -15.0) {
            ckWeapon.pos.y -= 3.0f;
        }
    }
    if (e->actor.state->hits == 3) {
        if (ckWeapon.obj->onGround) {
            ckWeapon.phys->rotDrag = 0.0f;
            if (!ckWeapon.obj->wasOnGround) {
                PlaySample(191, &ckWeapon.pos, 100, 128);
            }
        } else {
            ckWeapon.phys->rotDrag = 0.9f;
        }
    }
    if (bombDelay) {
        if (--bombDelay == 0) {
            SetVector2D(&ckWeapon.vel, &ckWeapon.pos);
            SCALE_VECTOR_2D(&ckWeapon.vel, 0.8);
            RotateVector2D(&ckWeapon.vel, &ckWeapon.vel, 3.1415927f);
            bombTimer = 80;
            ckWeapon.vel.y = 13.0f;
            ckWeapon.flags |= 0x19;
            ckWeapon.rotVel.x = 0.1f;
            ckWeapon.rotVel.y = 0.2f;
            PlaySample(169, &ckWeapon.pos, 150, 128);
        }
    }
    if (bombTimer) {
        bombTimer--;
        if (bombTimer < 20) {
            ckWeapon.pos.y += (frameCount & 1) * 5;
            ckWeapon.rotVel.x = 0.3f;
            ckWeapon.rotVel.y = 0.2f;
        }
        if (bombTimer < 4) {
            FlashScreen(250, 120, 80, 260, -30);
        }
        if (bombTimer == 0) {
            AddDebris(POPA, &ckWeapon.pos, &zero, NULL, NULL);
            AddDebris(POPB, &ckWeapon.pos, &zero, NULL, NULL);
            CreateDustCircleScaled(&ckWeapon.pos, 6, 0.0f, 8.0f);
            CreateDustCircleScaled(&ckWeapon.pos, 6, 8.0f, 2.0f);
            StartEarthquakeAtPoint(&ckWeapon.pos, 6, 1500.0f, 700.0f, 1, 2);
            ckWeapon.pos.y = -200.0f;
            if (DistanceBetweenPoints2D(&tempObj->pos, &ckWeapon.pos) < 100.0f) {
                AnimateActor(tempObj, 4, 0, 0, 0.0f);
                e->brain.delayTimer = -1;
                e->brain.instrIndex = 0;
                e->brain.instrTimer = 1;
                ckWeapon.flags &= ~0x19;
                ckWeapon.pos.y = 500.0f;
                e->actor.state->hits--;
                e->actor.state->hitFadeTimer = 20;
                SetKlosetSpeed(tempObj, e->actor.state->hits);
            }
            if (DistanceBetweenPoints2D(&hand.pos, &ckWeapon.pos) < 100.0f) {
                HurtHand(HURT_GO_BACKWARDS, 1);
            }
            if (DistanceBetweenPoints2D(&ball.pos, &ckWeapon.pos) < 100.0f) {
                BurstBall();
            }
            PlaySample(34, &tempObj->pos, 150, 128);
        }
    }
    plat = FindTaggedPlatform(&platformList.head, 10);
    e->samples[0].pitch = plat->motion->rotVel[1] * 1000.0f;
    e->samples[0].volume = MIN(2.0f * plat->motion->rotVel[1], 1.0f) * 1000.0f;
    if (tempObj->anim->current == 1) {
        if (tempObj->anim->time >= 94.0f && tempObj->anim->time < 102.0f) {
            plat = FindTaggedPlatform(&platformList.head, 10);
            if (e->actor.state->hits == 1) {
                plat->motion->rotVel[1] += 0.004;
                noseLit = 1;
                plat->motion->damping = 1.0f;
            } else {
                plat->motion->rotVel[1] += 0.008;
                plat->motion->damping = 0.99f;
            }
        }
    }
    if (tempObj->active == 1 && brain->instr.type == NME_SPECIAL && brain->instrTime == 1
        && brain->instr.arg.i == 5) {
        PlaySample(28, &tempObj->pos, 130, 128);
    }
    if (tempObj->anim->current == 15 && tempObj->active != 2) {
        if (QueryAnimTime(tempObj, 16.0f)) {
            PlaySample(29, &tempObj->pos, 120, 128);
        }
        if (QueryAnimTime(tempObj, 32.0f)) {
            PlaySample(30, &tempObj->pos, 120, 128);
        }
    }
}

void DoFortressBoss(void) {
    Vec3 tempVect;
    Vec3 tempVect2;
    Texture *handTexture;
    GameSprite *sPtr;
    Platform *plat;
    Actor *e = &enemies[0].actor;
    Enemy *en = enemies;
    f32 dist2;
    f32 dist;
    s32 col;
    s32 r;
    s32 whichOne;
    u8 x;

    plat = hand.obj->platform;
    if (frameCount == 0) {
        frankieDead = 0;
        r = 0;
        FindTexture(&handTexture, UpdateCRC("frankhands.bmp"), 1);
        for (sPtr = spriteList.head.next; sPtr != &spriteList.head; sPtr = sPtr->next) {
            if (sPtr->tex == handTexture) {
                frankieSprite[r++] = sPtr;
            }
            if (r == 2) {
                break;
            }
        }
        AddAmbientSfx(63, 50, 128);
        elecPlatform.flags |= 4;
        InitActor(&elecPlatform, "elecplat.ndo", 0, -900.0f, 0, 0x15);
        AnimateActor(&elecPlatform, 0, 1, 0, 1.4f);
        elecTimer = 0;
        elecPlatform.scale.x = 0.13f;
        elecPlatform.scale.y = 0.24f;
        elecPlatform.scale.z = 0.13f;
        AddActor(&elecPlatform);
        NMEActionStartAttack(&enemies[0]);
        enemies[0].brain.instrIndex = 2;
    }
    if (cameoList.head.next->active == 0) {
        cameraInfo.focus = &bossFocalPoint;
        bossFocalPoint.y = hand.pos.y;
        bossFocalPoint.x = hand.pos.x / 5.0f;
        bossFocalPoint.z = hand.pos.z / 5.0f;
    }
    cameraInfo.targetHeight = 400.0f;
    cameraInfo.targetDistance = 100.0f;
    if (hand.pos.y > -100.0f) {
        bossFocalPoint.y += 2.0f * (hand.pos.y - -100.0f);
    }
    cameraInfo.orbiting = 1;
    cameraInfo.orbitAngle = atan2(hand.pos.x, hand.pos.z);
    cameraInfo.orbitSpeed = 0.7f;
    if (plat != NULL && hand.obj->wasOnGround == 0 && hand.state->mode == HAND_SLAM2ST && handControl.actionTime == 1 &&
        plat->path->current < 4 && plat->tag > 0) {
        if (plat->tag < 5) {
            plat->path->moving = 1;
            plat->path->dir = 1;
            frStunPos1 = plat->actor.pos;
            frStunPos1.y = -365.0f;
            plat = FindTaggedPlatform(&platformList.head, plat->tag + 4);
            plat->path->dir = -1;
            plat->path->moving = 1;
            frStunPos2 = plat->actor.pos;
            frStunPos2.y = -365.0f;
            AddLightning(&frStunPos2, &frStunPos1, 205, 205, 255, 10, 5, 6, 1.0f, 2.0f);
            PlaySample(62, &e->pos, 130, 128);
            frStunCount = 5;
        } else if (plat->tag < 9) {
            plat->path->moving = 1;
            plat->path->dir = 1;
            frStunPos1 = plat->actor.pos;
            frStunPos1.y = -365.0f;
            plat = FindTaggedPlatform(&platformList.head, plat->tag - 4);
            plat->path->dir = -1;
            plat->path->moving = 1;
            frStunPos2 = plat->actor.pos;
            frStunPos2.y = -365.0f;
            AddLightning(&frStunPos2, &frStunPos1, 205, 205, 255, 10, 5, 6, 1.0f, 2.0f);
            PlaySample(62, &e->pos, 130, 128);
            frStunCount = 5;
        }
    }
    if (frStunCount != 0) {
        frStunCount--;
        if (e->anim->current != 4 && e->anim->current != 0 && e->anim->current != 3 && e->anim->current != 9 &&
            e->anim->current != 12 && e->anim->current != 10 && e->anim->current != 11) {
            SubVector2D(&tempVect, &frStunPos1, &frStunPos2);
            SCALE_VECTOR_2D(&tempVect, 0.1);
            SetVector2D(&tempVect2, &frStunPos2);
            for (x = 0; x < 10; x++) {
                if (DistanceBetweenPoints2D(&tempVect2, &e->pos) < 2.0f * e->obj->spheres[0].radius) {
                    ZERO_VECTOR(&e->vel);
                    e->rotAim.y = e->rot.y;
                    NMEHitByBlastRing(&enemies[0]);
                    PlaySample(66, &e->pos, 110, 128);
                }
                AddToVector2D(&tempVect2, &tempVect);
            }
        }
    }
    if (enemies[0].brain.attackTime != 0) {
        enemies[0].brain.returnIndex = 4;
    }
    if (e->anim->current == 4 && enemies[0].brain.spellTimer[0] == 0) {
        if (lightningSfx == 0 && Random(20) == 0) {
            lightningSfx = PlaySample(62, &e->pos, 130, 128);
        }
        dist2 = 10000.0f;
        for (plat = platformList.head.next; plat != &platformList.head; plat = plat->next) {
            if (plat->tag >= 1 && plat->tag <= 8) {
                dist = DistanceBetweenPoints2D(&hand.pos, &plat->actor.pos);
                if (dist < dist2) {
                    dist2 = dist;
                    nearPlat = plat;
                }
            }
        }
        gameInfo.fade = 0;
        gameInfo.fade2 = 0;
        if (enemies[0].brain.instrTime >= 10) {
            gameInfo.fade = 125;
        } else {
            gameInfo.fade = 205;
        }
        gameInfo.fade *= 0.5;
        if (frameCount % 5 == 0) {
            if (thunderClap == 0) {
                thunderClap = 1;
                PlaySampleNot3D(248, Random(20) - 106, 128, Random(26) - 116);
            }
            gameInfo.fade = 205;
            tempVect.y = 400.0f;
            tempVect.x = tempVect.z = 0.0f;
            if (Random(10) & 1) {
                SetFadeColour(245, 245, 255);
            } else {
                SetFadeColour(0, 0, 0);
            }
            for (r = 0; r < 2; r++) {
                tempVect2 = e->pos;
                tempVect2.y += Random(50);
                AddLightning(&tempVect2, &tempVect, 205, 205, 255, 10, 1, 5, 1.0f, 2.0f);
            }
        }
    } else {
        lightningSfx = 0;
        thunderClap = 0;
    }
    if (enemies[0].brain.instr.type == NME_FOLLOWPLAYER) {
        if (enemies[0].brain.attackTime != 0) {
            enemies[0].brain.instr.type = NME_MOVETO2D;
            enemies[0].brain.instr.params.box.min = nearPlat->actor.pos;
        }
        if (enemies[0].brain.instr.type == NME_FOLLOWPLAYER && enemies[0].brain.attackTime == 0) {
            dist2 = 10000.0f;
            enemies[0].brain.instr.type = NME_MOVETO2D;
            for (plat = platformList.head.next; plat != &platformList.head; plat = plat->next) {
                if (plat->tag >= 1 && plat->tag <= 8) {
                    dist = DistanceBetweenPoints2D(&hand.pos, &plat->actor.pos);
                    if (dist < dist2) {
                        dist2 = dist;
                        nearPlat = plat;
                    }
                }
            }
            enemies[0].brain.instr.params.box.min = nearPlat->actor.pos;
        }
    }
    if (enemies[0].brain.instr.type == NME_MOVETO2D && enemies[0].brain.attackTime == 0 && e->obj->contactPlatform != NULL) {
        NMEGetNewInstruction(&enemies[0], 6, 0);
        NMEProcessInstruction(&enemies[0]);
    }
    if (elecTimer > 0) {
        if (--elecTimer == 0) {
            for (r = 1; r < 9; r++) {
                nearPlat = FindTaggedPlatform(&platformList.head, r);
                nearPlat->flags &= ~0x4000;
                elecPlatform.pos.x = 0;
                elecPlatform.pos.y = -900.0f;
                elecPlatform.pos.z = 0;
            }
            StopContinuousSample(&en->samples[0]);
        }
    }
    if (handControl.edgePlatform != NULL && (handControl.edgePlatform->flags & 0x4000)) {
        HurtHand(HURT_GO_TO_SAFE, 1);
    }
    if (enemies[0].brain.instr.type == NME_WAIT && enemies[0].brain.instrTime == 1) {
        PlaySample(66, &e->pos, 120, 128);
    }
    if (e->anim->current == 0 && en->brain.spellTimer[0] == 0) {
        AddLightning(&frankieSprite[0]->pos, &frankieSprite[1]->pos, 150, 180, 250, 8, 1, 2, 1.0f, 1.0f);
        if ((frameCount & 1) && elecTimer == 0 &&
            DistanceBetweenPoints2D(&nearPlat->actor.pos, &enemies[0].actor.pos) < 150.0f) {
            if (en->samples[0].handle == 0) {
                PlayContinuousSample(&en->samples[0], 60, 100, &nearPlat->actor.pos, 128);
            }
            elecPlatform.pos = nearPlat->actor.pos;
            elecPlatform.rotQuat = nearPlat->actor.rotQuat;
            elecTimer = 320;
            nearPlat->flags |= 0x4000;
        }
    }
    if (e->anim->current == 4 || e->anim->current == 9 || e->anim->current == 12 || e->anim->current == 0) {
        r = Random(60);
        if (r < 3 && (frameCount & 3)) {
            if (r == 0) {
                PlaySample(61, &e->pos, 60, 128);
            } else if (r == 1) {
                PlaySample(62, &e->pos, 60, 128);
            } else if (r == 2) {
                PlaySample(55, &e->pos, 60, 128);
            }
        }
        e->flags |= 0x100000;
        e->obj->spheres[2].targetRadius = 35.0f;
        e->obj->spheres[3].targetRadius = 35.0f;
        if (ball.obj->platform == NULL) {
            dist = 35.0f;
            if (DistanceBetweenPoints2D(&e->pos, &hand.pos) < dist) {
                handPain++;
                AddLightning(&frankieSprite[Random(10) % 2]->pos, &hand.pos, 180, 180, 250, 6, 1, 6, 1.0f, 1.0f);
                if (handPain == 8) {
                    HurtHand(HURT_GO_BACKWARDS, 1);
                    handPain = 0;
                }
            } else {
                handPain = 0;
            }
            dist = goForPlatform ? 70.0f : 111.0f;
            if (DistanceBetweenPoints2D(&e->pos, &ball.pos) < dist) {
                ballPain++;
                AddLightning(&frankieSprite[Random(10) % 2]->pos, &ball.pos, 180, 180, 250, 6, 1, 6, 1.0f, 1.0f);
                if (ballPain == 8) {
                    BurstBall();
                    ballPain = 0;
                }
            } else {
                ballPain = 0;
            }
        }
    } else {
        e->flags &= ~0x100000;
        e->obj->spheres[2].targetRadius = 17.0f;
        e->obj->spheres[3].targetRadius = 17.0f;
    }
    if (e->anim->current == 1 && TriggerAtFrame(229.0f, e)) {
        StartEarthquakeAtPoint(&e->pos, 15, 2000.0f, 1000.0f, 1, 3);
        PlaySample(54, &e->pos, 120, 128);
        if (hand.obj->onGround != 0) {
            hand.vel.y = 10.0f;
            hand.vel.x = Random(8) - 4;
            hand.vel.z = Random(8) - 4;
        }
        if (ball.obj->onGround != 0) {
            ball.vel.y = 10.0f;
            ball.vel.x = Random(8) - 4;
            ball.vel.z = Random(8) - 4;
        }
    }
    if (FindTaggedPlatform(&platformList.head, 20)->path->current == 1) {
        if (frankieDead == 0) {
            enemies[0].brain.delayTimer = 15000;
            AnimateActor(&enemies[0].actor, 10, 0, 0, 0);
            AnimateActor(&enemies[0].actor, 11, 1, 1, 0);
            frankieDead = 100;
            PlayContinuousSample(&en->samples[0], 60, 160, &hand.pos, 128);
        }
        if (frankieDead >= 31) {
            for (r = 1; r < 9; r++) {
                plat = FindTaggedPlatform(&platformList.head, r);
                tempVect = plat->actor.pos;
                tempVect.y += plat->extent.y;
                x = Random(12);
                if (x != 0) {
                    col = Random(35) - 36;
                    if (x == 1) {
                        AddLightning(&tempVect, &enemies[0].actor.obj->spheres[1].pos, 150, 150, col, 5, 1, 6, 1.4f, 2.0f);
                    }
                    if (x == 2) {
                        AddLightning(&tempVect, &frankieSprite[0]->pos, 150, 150, col, 5, 1, 6, 1.4f, 2.0f);
                    }
                    if (x == 3) {
                        AddLightning(&tempVect, &frankieSprite[1]->pos, 2, 150, col, 5, 1, 6, 1.4f, 2.0f);
                    }
                    if (frameCount & 1) {
                        FlashScreen(120, 120, 255, 170, -30);
                    } else {
                        FlashScreen(0, 0, 0, 120, -30);
                    }
                    if (Random(100) == 1) {
                        PlaySample(61, &tempVect, 120, 128);
                    }
                }
            }
        }
    }
    if (frankieDead >= 2) {
        frankieDead--;
        if (frankieDead == 20) {
            ZERO_VECTOR(&enemies[0].actor.vel);
            enemies[0].actor.anim->loop = 0;
            AnimateActor(&enemies[0].actor, 2, 0, 1, 0);
            enemies[0].actor.flags &= ~0x19;
            enemies[0].actor.obj->spheres[1].flags |= 4;
            enemies[0].actor.obj->spheres[2].flags |= 8;
            enemies[0].actor.obj->spheres[3].flags |= 8;
            PlaySample(67, &hand.pos, 200, 128);
            FindTaggedPlatform(&platformList.head, 30)->path->moving = 1;
        }
        if (frankieDead == 10) {
            PrepareSong(gameInfo.world * 4 + 32 + Random(2));
        }
        if (frankieDead >= 25 && frankieDead <= 27) {
            FlashScreen(0, 0, 0, 255, -30);
        }
        if (frankieDead >= 21 && frankieDead <= 24) {
            FlashScreen(180, 180, 255, 255, -30);
        }
        StopContinuousSample(&en->samples[0]);
    }
    plat = FindTaggedPlatform(&platformList.head, 21);
    if (plat->path->current == 1 && plat->path->moving == 2) {
        SetPlatformPoint(1, 0);
        SetPlatformPoint(2, 1);
        SetPlatformPoint(3, 2);
        SetPlatformPoint(4, 3);
        SetPlatformPoint(5, 4);
        SetPlatformPoint(6, 3);
        SetPlatformPoint(7, 2);
        SetPlatformPoint(8, 1);
    }
    for (whichOne = 1; whichOne < 9; whichOne++) {
        plat = FindTaggedPlatform(&platformList.head, whichOne);
        if (plat->actor.speed != 0.0f && !(frameCount & 3) && Random(10) < 5) {
            SetVector2D(&tempVect, &plat->actor.pos);
            tempVect.y = -390.0f;
            CreateDustCircleScaled(&tempVect, 2, 0, 4.0f);
        }
    }
    if (QueryAnimTime(e, 15.0f)) {
        PlaySample(64, &e->pos, 100, 128);
    }
    if (QueryAnimTime(e, 34.0f)) {
        PlaySample(64, &e->pos, 100, 128);
    }
    if (QueryAnimTime(e, 59.0f)) {
        PlaySample(64, &e->pos, 100, 128);
    }
    if (QueryAnimTime(e, 89.0f)) {
        PlaySample(64, &e->pos, 100, 128);
    }
    if (QueryAnimTime(e, 106.0f)) {
        PlaySample(65, &e->pos, 100, 128);
    }
}

void SetPlatformPoint(s16 num, s16 point) {
    SetPlatPoint(FindTaggedPlatform(&platformList.head, num), point);
}

// point is an int compared as an s16. GCC sign-extends it into a new register.
void SetPlatPoint(Platform *plat, s32 point) {
    if (plat->path->current != (s16)point) {
        plat->path->moving = TRUE;
        if (plat->path->current < (s16)point) {
            plat->path->target = point - 1;
            plat->path->dir = 1;
        } else {
            plat->path->target = point + 1;
            plat->path->dir = -1;
        }
    }
}

void DoPiratesBonus(void) {
    Platform *p2;
    s32 plat;

    if (frameCount == 0) {
        for (p2 = platformList.head.next; p2 != &platformList.head; p2 = p2->next) {
            InitActorStructures(&p2->actor, 8);
            p2->actor.shadow->altitude = Altitude(&p2->actor.pos, &p2->actor.shadow->floor, &plat, 0);
        }
    }
    cameraInfo.targetHeight = 200.0f;
    windList.head.next->size[1] = FindTaggedWater(159)->height - windList.head.next->pos[1];
}

void DoWayRoom(void) {
    s16 j;
    Platform *a;
    Platform *platform2;
    Vec3 tempVect;

    if (frameCount == 0) {
        for (j = 0; j < 5; j++) {
            if (j == 3) {
                if (saveSlot[currentSaveSlot].levels >> ((gameInfo.world - 1) * 5 + 3) & 1) {
                    a = TagToFirstPlatform(14);
                    platform2 = TagToFirstPlatform(24);
                    tempVect = a->actor.pos;
                    a->actor.pos = platform2->actor.pos;
                    platform2->actor.pos = tempVect;
                }
            } else if (saveSlot[currentSaveSlot].allGaribs >> ((gameInfo.world - 1) * 5 + j) & 1) {
                a = TagToFirstPlatform(j + 11);
                platform2 = TagToFirstPlatform(j + 21);
                tempVect = a->actor.pos;
                a->actor.pos = platform2->actor.pos;
                platform2->actor.pos = tempVect;
            }
        }
    }
}

void DoPrehistBoss(void) {
    Enemy *e = enemies;
    Box box;
    Vec3 d;
    Bullet *bullet;
    Platform *plat;
    s32 i;

    if (cameoList.head.next->active) {
        return;
    }
    cameraInfo.destActive = 1;
    cameraInfo.destPos.x = hand.pos.x;
    cameraInfo.destPos.y = 250.0f;
    cameraInfo.destPos.z = 830.0f;
    box.pos.x = -316.0f;
    box.pos.y = -370.0f;
    box.pos.z = -611.0f;
    box.size.x = 690.0f;
    box.size.y = 480.0f;
    box.size.z = 920.0f;
    if (IsPointWithinBox(&box, &hand.pos) && controlMethod != CONTROL_CAMERA) {
        camera.pos.x = -250.0f;
        camera.pos.y = 260.0f;
        camera.pos.z = -309.0f;
    }
    box.pos.x = -320.0f;
    box.pos.y = -250.0f;
    box.pos.z = 200.0f;
    box.size.x = 630.0f;
    box.size.y = 370.0f;
    box.size.z = 300.0f;
    if (IsPointWithinBox(&box, &hand.pos) && (IsHandHoldingBall() == 1 || hand.state->mode == HAND_BALLWALK)) {
        ApplyAction(HAND_RELEASE);
    }
    if (e->actor.state->hitFadeTimer) {
        e->actor.state->hitFadeTimer--;
    }
    if (e->actor.state->hits) {
        if (hand.pos.z < 55.0f && e->brain.attackTime == 0) {
            if (handControl.deathTimer == 0 && e->brain.cooldownTimer == 0) {
                NMEActionStartAttack(e);
                NMEGetNewInstruction(e, 0, 0);
            }
        }
        if (e->brain.attackTime != 0) {
            if (handControl.deathTimer != 0 || player.hurtTimer != 0) {
                NMEEndAttack(e);
                e->brain.cooldownTimer = 400;
                NMEGetNewInstruction(e, e->brain.instrIndex, 0);
            }
        }
        if (e->actor.flags & 0x04000000) {
            if (e->actor.anim->current != 12 && e->actor.anim->current != 3
                && LENGTH_2D(&ball.vel) > 3.0f) {
                if (ball.obj->spheres[0].radius > 9.0) {
                    PlaySample(117, &e->actor.pos, 100, 70);
                    e->brain.instr.type = NME_WAIT;
                    AnimateActor(&e->actor, 12, 0, 0, 0.0f);
                    AnimateActor(&e->actor, 3, 0, 1, 0.0f);
                    AnimateActor(&e->actor, 5, 0, 1, 0.0f);
                    e->brain.instrTimer = 120;
                    e->actor.state->hits--;
                    e->actor.state->hitFadeTimer = 20;
                    if (e->actor.state->hits == 0) {
                        PlaySample(55, &e->actor.pos, 100, 128);
                        AnimateActor(&e->actor, 12, 0, 0, 0.0f);
                        AnimateActor(&e->actor, 2, 0, 1, 0.0f);
                        e->brain.delayTimer = 200;
                    }
                }
            }
            PlaySample(198, &ball.pos, 100, 128);
            ShatterActor(&ball, snowBallBit, 8, 5.0f);
            DeactivateCollectable(SNOWBALL);
            ball.pos = hand.pos;
            ZERO_VECTOR(&ball.vel);
            ball.pos.y += 100.0f;
            CreatePuffCircle(&ball.pos);
        }
        if (hand.pos.z > 400.0f && e->actor.anim->current == 5) {
            SUB_VECTORS(&d, &e->actor.pos, &hand.pos);
            e->actor.rotAim.y = atan2(d.x, d.z) + 3.141592654;
            if (e->actor.anim->time > 83.0f && e->actor.anim->time < 90.0f && frameCount % 5 == 0) {
                bullet = FireBulletFromActor(&e->actor, FIREBALL_BULLET, 0);
                bullet->actor.scale.x = 0.5f;
                bullet->actor.scale.y = 0.5f;
                bullet->actor.scale.z = 0.5f;
            }
        }
    } else {
        if (ball.pos.z > 480.0f && hand.pos.z <= 0.0f && ball.vel.z > -2.0f) {
            ball.vel.z -= 0.5;
        }
        if (e->actor.state->hits == 0 && e->actor.anim->finishedAnim != 0) {
            e->brain.delayTimer = 200;
            e->actor.obj->spheres[0].flags |= 3;
            for (i = 0; i < 2; i++) {
                if (Random(2)) {
                    bullet = CreateNewBullet(SMOKE_BULLET);
                    bullet->actor.pos = (&e->actor.obj->spheres[1])[i].pos;
                    bullet->actor.pos.y += 10.0f;
                    bullet->actor.vel.x = Random(4) - 2;
                    bullet->actor.vel.z = Random(4) - 2;
                    bullet->actor.vel.y = 6.0f;
                    InitBulletSpritesOrMesh(bullet, NULL, 0);
                    AddBullet(bullet);
                    bullet->sprites[0].scaleX = bullet->sprites[0].scaleY = Random(56) + 156;
                }
            }
        }
    }
    if ((e->actor.state->hits == 2 && (frameCount & 1)) || e->actor.state->hits == 1) {
        for (i = 0; i < 2; i++) {
            if (i != 1 || e->actor.state->hits != 2) {
                bullet = CreateNewBullet(SMOKE_BULLET);
                bullet->actor.pos = e->actor.obj->spheres[3].pos;
                bullet->actor.pos.y += 10.0f;
                bullet->actor.pos.x += i * 10;
                bullet->actor.vel.x = Random(4) - 2;
                bullet->actor.vel.z = Random(4) - 2;
                bullet->actor.vel.y = 6.0f;
                InitBulletSpritesOrMesh(bullet, NULL, 0);
                AddBullet(bullet);
                bullet->sprites[0].scaleX = bullet->sprites[0].scaleY = Random(56) + 156;
            }
        }
    }
    if (e->actor.state->hits == 0 && e->actor.anim->finishedAnim == 0) {
        if (e->samples[0].handle == 0 && e->actor.anim->time < 207.0f) {
            PlayContinuousSample(&e->samples[0], 36, 70, &zero, 70);
        }
        bullet = CreateNewBullet(FLAME_BULLET);
        bullet->actor.pos = e->actor.obj->spheres[3].pos;
        bullet->actor.pos.y += 10.0f;
        bullet->actor.pos.x += i * 10;
        bullet->actor.vel.x = Random(8) - 2;
        bullet->actor.vel.z = Random(8) - 2;
        bullet->actor.vel.y = Random(6) + 4;
        InitBulletSpritesOrMesh(bullet, NULL, 0);
        AddBullet(bullet);
        bullet->sprites[0].scaleX = bullet->sprites[0].scaleY = Random(56) + 256;
        bullet->sprites[0].anim.def.startScale = Random(56) + 256;
        bullet->sprites[0].anim.def.endScale = Random(56) + 256;
        if (QueryAnimTime(&e->actor, 207.0f)) {
            StopContinuousSample(&e->samples[0]);
            PlaySample(106, &e->actor.pos, 130, 128);
            PrepareSong(gameInfo.world * 4 + 32 + Random(2));
            FlashScreen(200, 50, 50, 200, -30);
            plat = FindTaggedPlatform(&platformList.head, 20);
            plat->path->points[0]->pos.x = 0.0f;
            plat->actor.pos.x = 0.0f;
            plat->path->moving = 5;
            for (i = 0; i < 20; i++) {
                bullet = CreateNewBullet(FLAME_BULLET);
                bullet->actor.pos = e->actor.obj->spheres[3].pos;
                bullet->actor.vel.x = Random(40) - 20;
                bullet->actor.vel.y = Random(40) - 20;
                bullet->actor.vel.z = Random(40) - 20;
                InitBulletSpritesOrMesh(bullet, NULL, 0);
                AddBullet(bullet);
                bullet->sprites[0].anim.def.startScale = Random(256) + 212;
                bullet->sprites[0].anim.def.endScale = Random(56) + 256;
                bullet->sprites[1].anim.def.startScale = Random(256) + 212;
                bullet->sprites[1].anim.def.endScale = Random(56) + 256;
            }
        }
    }
}

void DoSpaceBoss3(void) {
    s32 i;
    MeshColour *c;

    if (frameCount == 0) {
        spaceBossCamOffVect.x = 0.0f;
        spaceBossPanelOn = 0;
        spaceBossReload = 0;
        camera.flags &= ~0x10;
        spaceBossCamOffVect.y = 150.0f;
        spaceBossCamOffVect.z = -50.0f;
        spaceBossRot.x = spaceBossRot.y = spaceBossRot.z = spaceBossCamOffVect.x;
        spaceBossRot.x = initrotval;
        spaceBossRot.y = 6.2831855f;
        memclear(&spbCrosshair, sizeof(GameSprite));
        spbCrosshairAlpha = 0;
        for (i = 1; i < 6; i++) {
            if (i != 3) {
                TagToFirstPlatform(i)->actor.flags |= 0x4000;
            }
        }
        initFGObject("panelback.ndo", 0, 0, 300.0f, 0, 3.1415927f, 0, 1.0f, 1.0f, 1.0f, 0);
    }
    if (frameCount == 300) {
        hand.alpha = 0;
    }
    if (foregroundObjects[1] != NULL && spaceBossPanelOn == 0) {
        foregroundObjects[1]->pos.z = MIN((hand.pos.z - camera.pos.z) * panelfactor, 300.0f);
        if (foregroundObjects[1]->pos.z >= 299.0f) {
            spaceBossPanelOn = 1;
        }
    }
    hand.anim->speed = 0;
    hand.anim->time = 0;
    for (i = 0; i < numForegroundObjects; i++) {
        GetQuaternionFromXYZRot(&foregroundObjects[i]->rotQuat, &foregroundObjects[i]->rot);
    }
    ZERO_VECTOR(&hand.vel);
    hand.pos = handControl.checkpoint->actor.pos;
    TagToFirstPlatform(4)->actor.rotQuat = TagToFirstPlatform(1)->actor.rotQuat;
    TagToFirstPlatform(5)->actor.rotQuat = TagToFirstPlatform(2)->actor.rotQuat;
    if (gameInfo.cameoActive == 0 && frameCount > 10) {
        if (spbCrosshair.next == NULL) {
            InitSpriteAnimation(&spbCrosshair, &bulletAnimation[10], 0);
            spbCrosshair.r = 255;
            spbCrosshair.g = 255;
            spbCrosshair.b = 255;
            spbCrosshair.alpha = 0;
            spbCrosshair.offsetX = -spbCrosshair.tex->width / 2;
            spbCrosshair.offsetY = -spbCrosshair.tex->height / 2;
            spbCrosshairAlpha = spbalpha;
            AddSprite(&spbCrosshair, NULL);
        }
        spbCrosshair.alpha += MIN(MAX(-20.0, (((f32)spbCrosshairAlpha - (f32)spbCrosshair.alpha) / 4.0)), 20.0);
        if (spaceBossReload != 0) {
            spaceBossReload--;
        }
        hand.state->mode = HAND_NOTHING;
        if (!gamePaused) {
            spaceBossRot.y -= pad[ActiveController].stickX / spbxspeed;
            spaceBossRot.y = MIN(MAX(4.712388981, spaceBossRot.y), 7.853981635);
            spaceBossRot.x += pad[ActiveController].stickY / spbyspeed;
            spaceBossRot.x = MIN(MAX(5.89048622625, spaceBossRot.x), 7.0685834715);
            GetQuaternionFromXYZRot(&hand.rotQuat, &spaceBossRot);
        }
        spaceBossTempVect2.x = spaceBossTempVect2.y = 0.0f;
        spaceBossTempVect2.z = 40.0f;
        RotateVectorByQuaternion(&spaceBossTempVect2, &spaceBossTempVect2, &hand.rotQuat);
        RotateVectorByQuaternion(&spaceBossCamVect, &spaceBossCamOffVect, &hand.rotQuat);
        ADD_TO_VECTOR(&spaceBossCamVect, &hand.pos);
        AddScaledVector(&camera.pos, &camera.pos, spbcamfrac, &spaceBossCamVect, 1.0f - spbcamfrac);
        ADD_TO_VECTOR(&spaceBossTempVect2, &camera.pos);
        spbCrosshair.pos = spaceBossTempVect2;
        if (earthQuake.time < 3) {
            AddScaledVector(&spaceBossTempVect2, cameraInfo.focus, spbcamfrac2, &spaceBossTempVect2, 1.0f - spbcamfrac2);
            spaceBossCamVect2 = spaceBossTempVect2;
            SetCameraOverride(camera.pos.x, camera.pos.y, camera.pos.z, &spaceBossCamVect2, &zero, 10, 0);
            CalcCameraRot();
        }
        if (pad[ActiveController].button[BUTTON_Z]) {
            spaceBossShield = 1;
            spbCrosshairAlpha = 0;
        } else {
            spaceBossShield = 0;
            spbCrosshairAlpha = spbalpha;
        }
        if (foregroundObjects[0] != NULL) {
            if (spaceBossShield) {
                foregroundObjects[0]->alpha += MIN(MAX(-20, (100 - foregroundObjects[0]->alpha) / 4), 20);
                c = (MeshColour *)foregroundObjects[0]->inst->obj->mesh->colours;
                c->r = (s32)(SineWave(5.0f, frameCount, 0) * 63.0f + 192.0f);
                c->g = 0;
                c->b = 0;
                c++;
                c->r = (s32)(SineWave(5.0f, frameCount, 0) * 63.0f + 192.0f);
                c->g = 0;
                c->b = (s32)(192.0f - SineWave(6.0f, frameCount, 0) * 63.0f);
                c++;
                c->r = (s32)(SineWave(5.0f, frameCount, 60) * 63.0f + 192.0f);
                c->g = (s32)(192.0f - SineWave(6.0f, frameCount, 80) * 63.0f);
                c->b = 0;
                c++;
                c->r = 0;
                c->g = (s32)(SineWave(5.0f, frameCount, 20) * 63.0f + 192.0f);
                c->b = 0;
                c++;
                c->r = 0;
                c->g = 0;
                c->b = (s32)(SineWave(5.0f, frameCount, 40) * 63.0f + 192.0f);
                c++;
                c->r = (s32)(SineWave(5.0f, frameCount, 60) * 63.0f + 192.0f);
                c->g = (s32)(SineWave(5.0f, frameCount, 80) * 63.0f + 192.0f);
                c->b = 0;
                c++;
                c->r = 0;
                c->g = (s32)(SineWave(5.0f, frameCount, 80) * 63.0f + 192.0f);
                c->b = (s32)(192.0f - SineWave(6.0f, frameCount, 60) * 63.0f);
            } else {
                foregroundObjects[0]->alpha = foregroundObjects[0]->alpha * 5 / 6;
            }
        }
        if (!spaceBossShield && numBubbles < 8 && spaceBossReload == 0 && pad[ActiveController].pressed[BUTTON_A]) {
            spaceBossReload = spbReloadCounter;
            FireBulletFromActor(&hand, SPACEBALL_BULLET, 0);
        }
    } else {
        player.hurtTimer = 3;
    }
}

void DoFlythru2Screen(void) {
    s32 i;
    s32 t;

    if (frameCount < 10) {
        cheatNum = 0;
        for (i = 0; i < 6; i++) {
            flythru2Num = 0;
            if (!(saveSlot[currentSaveSlot].allGaribs & (1 << (i * 5 + 4)))) {
                flythru2Num = 1;
            }
        }
    }
    if (flythru2Num == 0) {
        t = frameCount % 100;
        if (t == 0 && frameCount >= 1301) {
            flythru2Alpha = 255;
            flythru2XPos = 320.0f;
            cheatNum = (cheatNum + 1) % 26;
        }
        if (t > 80) {
            flythru2Alpha -= 15;
            if (flythru2Alpha < 0) {
                flythru2Alpha = 0;
            }
        }
        if (frameCount >= 1300) {
            flythru2XPos += MIN(MAX(-50.0f, -flythru2XPos / 4.0f), 50.0f);
            if (flythru2XPos < 1.0f) {
                flythru2XPos = 1.0f;
            }
            for (i = 0; i < 8; i++) {
                cheatTempString[i] = cheatInfo[cheatNum].code[i];
            }
            PrintCheatString();
            memset(cheatTempString, 0xFF, 8);
            SetFont(1);
            NEW_TEXT(flythru2XPos + 160.0f, 110, 255, 255, 255, flythru2Alpha, 1.0f, 1.0f, 1, cheatMsg, 1);
            SetFont(0);
            NEW_TEXT(160.0f - flythru2XPos, 135, 255, 255, 255, flythru2Alpha, 1.0f, 1.0f, 1, cheatInfo[cheatNum].msg, 1);
        }
    }
    if (enemies[0].actor.shadow->altitude < 20.0f && enemies[0].actor.shadow->altitude != -1.0f) {
        enemies[0].actor.vel.y += 0.75;
    }
    if (enemies[0].actor.shadow->altitude > 40.0f) {
        enemies[0].actor.vel.y -= 0.2;
    }
    if (creditsText[creditsPerson].title[0] == ' '
        || (controllerPresent && !(gameInfo.exitFlags & 1) && gameInfo.starZ == 0 && gameInfo.fade == 0
            && (AnyDebouncePressed(ActiveController) || (controllerdata[ActiveController].button & 0x8000)
                || (controllerdata[ActiveController].button & 0x4000)))) {
        defaultBehaviour[BUTTERFLY].accel /= 2.0f;
        defaultBehaviour[BUTTERFLY].moveSpeed /= 2.0f;
        ExitLevel(QUIT_EXIT, 1, 1);
    }
    if (frameCount > 20) {
        CreditsRun();
    }
}

void SetupFlythru2Screen(void) {
    StopDrawing(D_80101F58);
    SetFont(0);
    gameInfo.exitFlags = 0;
    frameCount = frameCount2 = 0;
    InitLinkedLists();
    defaultBehaviour[BUTTERFLY].accel *= 2.0f;
    defaultBehaviour[BUTTERFLY].moveSpeed *= 2.0f;
    StartNewLevel(FLYTHRU2);
    CreditsInit();
    FadeScreenIn();
    disablePause = TRUE;
}

INCLUDE_ASM("asm/nonmatchings/boss", pad_801326E4);

INCLUDE_RODATA("asm/nonmatchings/boss", D_80101F58);
