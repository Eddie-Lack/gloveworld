#include "common.h"
#include "ident.h"
#include "actor.h"
#include "hand.h"
#include "ball.h"
#include "game.h"
#include "platform.h"
#include "camera.h"
#include "controller.h"
#include "overlay.h"
#include "texture.h"
#include "jalloc.h"
#include "maths.h"
#include "footprint.h"
#include "debris.h"
#include "bullet.h"
#include "sound.h"
#include "collect.h"
#include "save.h"
#include "light.h"
#include "blur.h"
#include "banks.h"
#include "anim.h"
#include "collision.h"
#include "enemy.h"
#include "nmeprocess2.h"
#include "object.h"
#include "spell.h"
void FlashScreen(u8 r, u8 g, u8 b, s16 fadeVal, s16 fadeStep);

void StartEarthquakeAtPoint(Vec3 *pos, s16 duration, f32 maxStrength, f32 minStrength, s16 numCycles, s32 flags);
extern Texture *blastTex;
extern Vec3 sepVect;
extern s32 fakedies;
extern u8 hthrR;
extern u8 hthrG;
extern u8 hthrB;
extern u8 hwhkR;
extern u8 hwhkG;
extern u8 hwhkB;
void NMEGetNewInstruction(Enemy *enemy, s32 instructionNum, s32 overrideConditions);
void AnimateActor(Actor *actor, s32 animNum, s32 loop, s32 queue, f32 speed);

extern f32 handSlide2;

extern s32 cheatActivated;
extern f32 handInertia;
extern f32 bcfac;
extern s32 gameOverCount;
extern u8 gamePaused;
extern s32 controlMethod;
extern Overlay numLivesOL;
extern struct Bullet *tongueBullet;

u32 UpdateCRC(char *ptr);
void InitSpriteAnimation(GameSprite *sprite, SpriteAnim *temp, s16 lifespan);
void FindTexture(void *texPtr, u32 texID, s32 verbose);
extern ObjectInstance *frogObj;

extern s32 castSpellFX;
extern u16 tongueScale;
extern f32 levelHandScale;
extern Quat handQAim;
extern Quat vertQ;
extern s32 handOrbiterFreq;
extern s32 frameCount;
extern Quat zeroQuat;
extern Blur *blur1;
extern Blur *blur2;
extern Texture *handTextureList[2];
extern f32 D_801EEC3C;
extern u8 ActiveController;
extern s16 shrinkExitTimer;
void AddGlovePrint(s32 side);
extern Vec3 zero;

struct Bullet *FireBulletFromActor(Actor *actor, s32 type, s32 flash);

INCLUDE_ASM("asm/nonmatchings/hand", pad_80165338);

extern u8 handActionValidityTable[][45];

extern s32 ballStatic;
extern s32 earthQuake;
extern f32 bncealt;
extern f32 bncevel;
extern f32 frontprobey;
extern s32 facenum;
extern s8 specialActor;
extern s16 locateGaribsAllowed;
extern Vec3 globalPos[8];
extern Vec3 globalRot[8];
extern Vec3 tempGlobalPos[8];
extern Vec3 tempGlobalRot[8];
f32 Altitude(Vec3 *pos, Vec3 **floor, s32 *plat, s32 coll);
extern Vec3 camPos;
extern Vec3 camPos2;
extern f32 handcrawlinertia;
extern f32 handFrogVelY;
extern f32 handFrogVelXZ;
extern s32 ballWithHand;
extern s16 levelExitTimer;
extern s32 numGaribs;
extern Quat origQ;
extern SpriteAnim garibAnimation[];
extern s16 tTime1;
extern s16 tTime2;
extern Vec3 tongueOffsetPos[];
extern u16 tongueScale2;
extern f32 tongueTime;
extern s32 tongueStuck;
void SquashActor(Actor *actor, Quat *dir, f32 power, f32 maxSquash);
void SetupLight(void *light, u8 initR, u8 initG, u8 initB, u8 r, u8 g, u8 b, s32 x, s32 y, f32 speed);
void AddBackgroundLight(void *light);
void AddHandLocateSprite(SpriteAnim *anim);
void DisableControl(s16 time);
void UpdateActorPlatform(Vec3 *actorPos, Vec3 *actorActualOldpos, Vec3 *actorVel, Vec3 *actorRot, Vec3 *actorRotaim, Quat *actorQRot, s8 raimy, s32 flags, Platform *actorPlatform, Platform *actorOldPlatform);
extern f32 D_801ED3EC;
extern s32 crawlfx;
extern s32 joinVal;
extern f32 cartwheelAccel;
extern s32 handcrawlframe;
extern f32 gravity;
extern f32 handballwalkspeed;
extern f32 ballwalkfac;
extern f32 clingscale;
extern f32 clingy;
extern f32 numTongueSegs;
extern f32 tongueRad[];
extern s32 ballLeftBehind;
extern s16 nextBallState[];
extern f32 handlob;
extern s16 currentSaveSlot;
extern f32 swishMaxSpeed;
extern f32 swishMinSpeed;
extern f32 swishSpeed;
extern f32 handBank;
void HurtHand(s32 type, s16 damage);
void KillHand(s16 type);
void CalculateQuatForPlane(Actor *actor, Quat *qAim, Vec3 *plane);

s32 ActionAllowed(s32 handAction, s32 requestedAction) {
    return handActionValidityTable[handAction][requestedAction];
}

INCLUDE_RODATA("asm/nonmatchings/hand", D_80107734);

s32 ConditionsMet(s32 requestedAction) {
    Vec3 vel;
    Vec3 save;
    Vec3 tempVect;
    Vec3 w;
    CollFace *plane;
    CollFace *plane2;
    Platform *platform;
    f32 tempAlt;
    f32 temp;
    s32 j;

    plane = NULL;
    SUB_VECTORS(&vel, &hand.pos, &hand.prevPos);
    switch (requestedAction) {
        case HAND_IDLE:
            if ((hand.state->mode == HAND_NOTHING && hand.link != NULL)
                || (hand.state->mode == HAND_SLAM2ST && BallWalkAllowed())) {
                return FALSE;
            }
            if (hand.state->mode == HAND_CASTSPELL) {
                return TRUE;
            }
            if (pad[ActiveController].mag == 0.0f && controlMethod == CONTROL_HAND) {
                if (hand.state->waterState && !(hand.flags & 0x200000)
                    && (hand.obj->onGround || (hand.flags & 0x1000)) && LENGTH_2D(&vel) < 1.0f) {
                    return TRUE;
                }
                if (hand.state->mode == HAND_PUSH || hand.state->mode == HAND_SLAM2ST) {
                    return TRUE;
                }
                if (hand.state->mode == HAND_SLIDING && (hand.obj->onGround || (hand.flags & 0x1000))
                    && (LENGTH_2D(&vel) < 1.5 || earthQuake)) {
                    return TRUE;
                }
                if (!(LENGTH_2D(&vel) < 0.3) && !earthQuake) {
                    return FALSE;
                }
                if (!hand.obj->onGround && !(hand.flags & 0x1000)) {
                    return FALSE;
                }
                if (hand.state->mode == HAND_RELEASE) {
                    if (hand.state->prevMode != HAND_BALLWALK && hand.state->prevMode != HAND_HOPPING) {
                        return TRUE;
                    }
                    break;
                }
                return TRUE;
            } else {
                return FALSE;
            }
        case HAND_WALK:
            if (!(pad[ActiveController].mag > 5.0f)) {
                return FALSE;
            }
            if (controlMethod != CONTROL_HAND || (!(hand.flags & 0x1000) && !hand.obj->onGround)) {
                return FALSE;
            }
            if (hand.state->mode == HAND_NOTHING && hand.link != NULL) {
                break;
            }
            if (hand.state->mode == HAND_SLIDING && hand.obj->onGround && hand.speed < 0.25) {
                return TRUE;
            }
            if (hand.state->mode == HAND_PUSH) {
                vel = handControl.inputAccel;
                MakeUnit(&vel);
                for (j = 0; j < 4; j++) {
                    if (hand.obj->planes[j] != NULL && hand.obj->planes[j]->y < 0.5
                        && vel.x * hand.obj->planes[j]->x + vel.z * hand.obj->planes[j]->z < -0.7) {
                        return FALSE;
                    }
                }
            }
            return TRUE;
        case HAND_JUMP:
            if (hand.state->mode == HAND_SLAM && BallWalkAllowed()) {
                return TRUE;
            }
            if (pad[ActiveController].button[BUTTON_A] && controlMethod == CONTROL_HAND
                && (hand.obj->onGround || hand.state->mode == HAND_BALLWALK) && !(hand.flags & 0x8000)
                && hand.state->mode != HAND_SLAM) {
                return TRUE;
            }
            break;
        case HAND_JOINED:
            if (ballStatic || ballControl.loseTimer || (hand.state->mode == HAND_SLAM2ST && ConditionsMet(HAND_BALLWALK))
                || (hand.flags & 0x8000) || player.powerups[VANISHBALL] || ball.state->teleportTimer > 0
                || hand.state->teleportTimer > 0 || handControl.joinTimer) {
                return FALSE;
            }
            if (hand.state->mode == HAND_BOUNCE2CUP) {
                return TRUE;
            }
            if (ball.link != NULL) {
                return FALSE;
            }
            if (ABS(hand.pos.y - ball.pos.y) <= hand.radius + ball.radius) {
                if (handControl.touchedActor != 0) {
                    return TRUE;
                }
                break;
            }
            return FALSE;
        case HAND_THROWAIM:
            if (pad[ActiveController].button[BUTTON_B] && pad[ActiveController].holdTime[BUTTON_B] >= 5 && controlMethod == CONTROL_HAND
                && ABS(hand.pos.y - ball.pos.y) < ball.radius * 5.0f) {
                return TRUE;
            }
            break;
        case HAND_THROW2CUP:
            if (pad[ActiveController].pressed[BUTTON_Z] && controlMethod == CONTROL_HAND) {
                return TRUE;
            }
            break;
        case HAND_THROW:
            if (hand.state->prevMode != HAND_CATCH && !pad[ActiveController].button[BUTTON_B] && controlMethod == CONTROL_HAND) {
                return TRUE;
            }
            break;
        case HAND_SKIDDING:
            if (hand.flags & 0x8000) {
                return FALSE;
            }
            if (hand.state->mode == HAND_IDLE) {
                if (pad[ActiveController].mag < 5.0f && controlMethod == CONTROL_HAND) {
                    plane2 = (CollFace *)hand.obj->planes[0];
                    if (plane2 != NULL
                        && plane2->plane.y
                               <= 1.0f - (1.0f - hand.phys->slideLimit) / terrain[plane2->terrain].slideSlopeScale) {
                        if (LENGTH_2D(&hand.vel) > 0.6) {
                            return TRUE;
                        }
                        break;
                    }
                }
                return FALSE;
            }
            if (pad[ActiveController].mag < 5.0f && controlMethod == CONTROL_HAND
                && ((handControl.prevGait == HANDWALK_RUN && handControl.actionTime >= 11)
                    || (hand.obj->planes[0] != NULL
                        && terrain[((CollFace *)hand.obj->planes[0])->terrain].slip > 1.0f
                        && LENGTH_2D(&hand.vel) > 1.5))) {
                if (hand.obj->onGround != 0) {
                    return TRUE;
                }
                break;
            }
            if (!hand.obj->onGround) {
                return FALSE;
            }
            if (controlMethod == CONTROL_HAND && hand.state->mode == HAND_WALK && handControl.actionTime >= 3
                && FindShortestAngle(pad[ActiveController].angle[3], pad[ActiveController].angle[0]) > 2.5
                && LENGTH_2D(&hand.vel) > 3.0) {
                return TRUE;
            }
            break;
        case HAND_CRAWL:
            if (pad[ActiveController].mag != 0.0f && controlMethod == CONTROL_HAND && handControl.actionTime >= 5) {
                return TRUE;
            }
            break;
        case HAND_DUCK:
            if (hand.obj->onGround && pad[ActiveController].button[BUTTON_Z] && controlMethod == CONTROL_HAND
                && !(hand.flags & 0x8000) && pad[ActiveController].holdTime[BUTTON_Z] == 4) {
                if (hand.state->mode != HAND_CRAWL) {
                    return TRUE;
                }
            } else if (hand.state->mode != HAND_CRAWL) {
                return FALSE;
            }
            if (pad[ActiveController].mag == 0.0f && controlMethod == CONTROL_HAND) {
                return TRUE;
            }
            break;
        case HAND_GETUP:
            if (!pad[ActiveController].button[BUTTON_Z] && controlMethod == CONTROL_HAND && !handControl.noControlTimer) {
                return TRUE;
            }
            if (!hand.obj->onGround && hand.state->mode == HAND_DUCK) {
                return TRUE;
            }
            break;
        case HAND_PREWHACK:
            if (pad[ActiveController].button[BUTTON_A] && controlMethod == CONTROL_HAND && pad[ActiveController].holdTime[BUTTON_A] >= 3) {
                return TRUE;
            }
            break;
        case HAND_WHACK:
            if (pad[ActiveController].button[BUTTON_A] || controlMethod != CONTROL_HAND) {
                break;
            }
            return TRUE;
        case HAND_RELEASE:
            if (!pad[ActiveController].pressed[BUTTON_Z]) {
                return FALSE;
            }
            if (controlMethod != CONTROL_HAND) {
                return FALSE;
            }
            if ((player.powerups[POWERBALL] || player.powerups[BALLBEARING]) && !player.powerups[HERCULES]) {
                hand.pos.y = ball.pos.y;
            }
            PlaySample(168, &hand.pos, 100, 128);
            return TRUE;
        case HAND_FALLING:
            if (handControl.fallTime >= 3) {
                return TRUE;
            }
            break;
        case HAND_BOUNCE:
            if ((controlMethod == CONTROL_BALL && !pad[ActiveController].button[BUTTON_A] && pad[ActiveController].sincePress[BUTTON_A] < 6)
                || (!pad[ActiveController].button[BUTTON_B] && controlMethod != CONTROL_CAMERA
                    && pad[ActiveController].sincePress[BUTTON_B] < 6)) {
                handControl.bounceQueued = 1;
                return TRUE;
            }
            return FALSE;
        case HAND_BOUNCE2CUP:
            if (hand.state->mode == HAND_BOUNCE && handControl.actionTime < 2) {
                return FALSE;
            }
            temp = hand.pos.y - ball.pos.y;
            if (temp < ball.obj->spheres[0].radius - 9.0 + 5.0 && ball.shadow->altitude < ball.obj->spheres[0].radius - 9.0 + 20.0
                && ball.vel.y < 0.0f) {
                return TRUE;
            }
            if (temp < ball.obj->spheres[0].radius - 9.0 + 5.0 && ball.state->waterState && ball.vel.y < 0.0f) {
                return TRUE;
            }
            if (ball.shadow->altitude < bncealt && ball.vel.y > 0.0f && ball.vel.y < bncevel) {
                return TRUE;
            }
            break;
        case HAND_AIRBOURNEWHACK:
            if (pad[ActiveController].button[BUTTON_A] && controlMethod == CONTROL_HAND
                && hand.pos.y - ball.pos.y < ball.radius * 1.5) {
                return TRUE;
            }
            break;
        case HAND_THROW2LAND:
            if ((hand.obj->onGround == 1 || (hand.flags & 0x1000)) && handControl.actionTime >= 2
                && handControl.catapult == NULL) {
                return TRUE;
            }
            break;
        case HAND_SLAM:
            if (pad[ActiveController].pressed[BUTTON_Z] && controlMethod == CONTROL_HAND && hand.vel.y > -9.0f) {
                return TRUE;
            }
            break;
        case HAND_SLAM2ST:
            if (hand.obj->onGround == 1 || (hand.flags & 0x1000)) {
                return TRUE;
            }
            if (BallWalkAllowed()) {
                return TRUE;
            }
            break;
        case HAND_BALLWALK:
            if (BallWalkAllowed() != 0) {
                return TRUE;
            }
            break;
        case HAND_CARTWHEEL:
            if (hand.obj->onGround && !pad[ActiveController].button[BUTTON_Z] && controlMethod == CONTROL_HAND
                && pad[ActiveController].sincePress[BUTTON_Z] >= 1 && pad[ActiveController].sincePress[BUTTON_Z] <= 5
                && !(hand.flags & 0x8000)) {
                return TRUE;
            }
            break;
        case HAND_CLIMBDOWN:
            if (!hand.obj->onGround
                || (hand.obj->platform != NULL && (hand.obj->platform->flags & 0x200))) {
                return FALSE;
            }
            if ((hand.flags & 0x8000) || !(hand.speed > 0.0f)) {
                return FALSE;
            }
            if (!((hand.speed < 3.0 && pad[ActiveController].mag < 50.0f) || hand.state->mode == HAND_SKIDDING)) {
                return FALSE;
            }
            if (controlMethod != CONTROL_HAND) {
                return FALSE;
            }
            tempAlt = Altitude(&handControl.edgeProbe.pos, (Vec3 **)&plane, (s32 *)&platform, 0);
            if (hand.shadow->floor == NULL || !(hand.shadow->floor->y > 0.92)) {
                return FALSE;
            }
            if (tempAlt - hand.shadow->altitude > 30.0f || tempAlt < 0.0f) {
                if (!HandOverstepsEdge(&handControl.edgeProbe, (CollFace *)hand.shadow->floor)) {
                    return FALSE;
                }
            } else {
                return FALSE;
            }
            hand.rotAim.y = hand.rot.y = Aabs(hand.rot.y + 3.141592654);
            handControl.climbPlatform = hand.obj->platform;
            handControl.clingPlanes[0] = (struct Plane *)hand.shadow->floor;
            return TRUE;
        case HAND_CLING:
            if (hand.obj->onGround && !(hand.obj->contactFlags & 2)) {
                return FALSE;
            }
            if (hand.flags & 0x8000) {
                return FALSE;
            }
            if (!(hand.shadow->altitude > 45.0f || hand.shadow->altitude < 0.0f)) {
                return FALSE;
            }
            tempAlt = Altitude(&handControl.frontProbe.pos, (Vec3 **)&plane, (s32 *)&platform, 0);
            if (plane != NULL && !(platform != NULL && (platform->flags & 0x200)) && plane->plane.y > 0.92 && tempAlt < 12.0
                && tempAlt > 0.0f) {
                temp = hand.rot.y;
                handControl.edgeAltitude = tempAlt;
                if (!HandOverstepsEdge(&handControl.frontProbe, plane)) {
                    return FALSE;
                }
                tempVect.x = 0.0f;
                tempVect.z = 18.0f;
                tempVect.y = frontprobey;
                RotateVector2D(&tempVect, &tempVect, temp);
                ADD_VECTORS(&handControl.frontProbe.pos, &hand.pos, &tempVect);
                save = handControl.frontProbe.pos;
                handControl.frontProbe.prevPos = hand.pos;
                CalcCollisionSpheres(&handControl.frontProbe);
                handControl.savedFaceNum = facenum;
                for (j = 0; j < 8; j++) {
                    tempGlobalRot[j] = globalRot[j];
                    tempGlobalPos[j] = globalPos[j];
                }
                CheckCollisions(&handControl.frontProbe);
                specialActor = 0;
                for (j = 0; j < 8; j++) {
                    globalRot[j] = tempGlobalRot[j];
                    globalPos[j] = tempGlobalPos[j];
                }
                handControl.frontProbe.pos = save;
                facenum = handControl.savedFaceNum;
                for (j = 0; j < 4; j++) {
                    if (handControl.frontProbe.obj->planes[j] != NULL) {
                        if (j == 0) {
                            w = tempVect;
                            MakeUnit2D(&w);
                            SCALE_VECTOR_2D(&w, handControl.frontProbe.obj->spheres[0].radius);
                        }
                        plane = (CollFace *)handControl.frontProbe.obj->planes[j];
                        AddVector2D(&tempVect, &handControl.frontProbe.pos, &w);
                        tempVect.y = handControl.frontProbe.pos.y;
                        if ((DOT_PRODUCT(&tempVect, &plane->plane) + plane->plane.d) * (DOT_PRODUCT(&hand.pos, &plane->plane) + plane->plane.d) < 0.0f) {
                            hand.rot.y = hand.rotAim.y = temp;
                            return FALSE;
                        }
                    }
                }
                handControl.edgePlatform = platform;
                return TRUE;
            }
            handControl.edgeAltitude = -1.0f;
            return FALSE;
        case HAND_CLINGUP:
            if (pad[ActiveController].pressed[BUTTON_A] && controlMethod == CONTROL_HAND) {
                return TRUE;
            }
            if (pad[ActiveController].mag != 0.0f && controlMethod == CONTROL_HAND
                && FindShortestAngle(hand.rot.y, Aabs(-pad[ActiveController].angle[0] + 3.141592654 + camera.rot.y))
                       > 1.570796327) {
                return TRUE;
            }
            break;
        case HAND_CLIMBUP:
            if (pad[ActiveController].pressed[BUTTON_A] && controlMethod == CONTROL_HAND) {
                return TRUE;
            }
            if (pad[ActiveController].mag != 0.0f && controlMethod == CONTROL_HAND && hand.state->mode == HAND_CLIMBDOWN
                && FindShortestAngle(hand.rot.y, Aabs(-pad[ActiveController].angle[0] + camera.rot.y))
                       > 1.570796327) {
                return TRUE;
            }
            break;
        case HAND_DROPDOWN:
            if (pad[ActiveController].pressed[BUTTON_Z]) {
                return TRUE;
            }
            if (pad[ActiveController].mag != 0.0f && controlMethod == CONTROL_HAND) {
                if (hand.state->mode == HAND_CLIMBDOWN) {
                    if (FindShortestAngle(hand.rot.y, Aabs(-pad[ActiveController].angle[0] + camera.rot.y))
                        < 1.570796327) {
                        return TRUE;
                    }
                    break;
                }
                if (FindShortestAngle(hand.rot.y, Aabs(-pad[ActiveController].angle[0] + 3.141592654 + camera.rot.y))
                    < 1.570796327) {
                    return TRUE;
                }
            } else {
                return FALSE;
            }
            break;
        case HAND_PUSH:
            if (pad[ActiveController].mag != 0.0f && controlMethod == CONTROL_HAND && hand.obj->onGround && !(hand.flags & 0x8000)) {
                vel = handControl.inputAccel;
                MakeUnit(&vel);
                for (j = 0; j < 4; j++) {
                    if (hand.obj->planes[j] != NULL && hand.obj->planes[j]->y < 0.5
                        && vel.x * hand.obj->planes[j]->x + vel.z * hand.obj->planes[j]->z < -0.7) {
                        if (handControl.wallPushTime++ >= 3) {
                            hand.rotAim.x = atan2(hand.obj->planes[j]->y,
                                                        LENGTH_2D(hand.obj->planes[j]));
                            return TRUE;
                        }
                        break;
                    }
                }
                if (j >= 4) {
                    handControl.wallPushTime = 0;
                }
            }
            return FALSE;
        case HAND_LOB:
            if (!pad[ActiveController].button[BUTTON_A] && controlMethod == CONTROL_HAND && pad[ActiveController].sincePress[BUTTON_A] < 4) {
                return TRUE;
            }
            break;
        case HAND_CATCH:
            if (HandCatchAllowed() != 0) {
                return TRUE;
            }
            break;
        case HAND_CASTSPELL:
            if (handControl.spellDelay) {
                return FALSE;
            }
            if (!pad[ActiveController].button[BUTTON_SHOULDER_RIGHT]) {
                return FALSE;
            }
            if (controlMethod == CONTROL_HAND && (player.spell != 15 || (cheatActivated & 1) || locateGaribsAllowed)) {
                player.castSpellType = player.castSpell = player.spell;
                return TRUE;
            }
            return FALSE;
        case HAND_SNATCH:
            if ((pad[ActiveController].pressed[BUTTON_B]
                 || (pad[ActiveController].button[BUTTON_B] && hand.state->mode == HAND_LOCATE && !handControl.locateGaribs))
                && controlMethod == CONTROL_HAND && !(hand.flags & 0x8000)) {
                if (HandSnatchAllowed() != 0) {
                    return TRUE;
                }
                break;
            }
            return FALSE;
        case HAND_FROG_IDLE:
            if (!hand.obj->onGround) {
                if (hand.obj->planes[0] == NULL) {
                    return FALSE;
                }
                if (!(hand.obj->planes[0]->y > 0.0f)) {
                    return FALSE;
                }
            }
            if (hand.state->mode == HAND_FROG_WALK) {
                if (handControl.bounceCount >= 2) {
                    return TRUE;
                }
                break;
            }
            return TRUE;
        case HAND_FROG_JUMP:
            if (pad[ActiveController].button[BUTTON_A] && controlMethod == CONTROL_HAND && hand.obj->onGround
                && FindShortestAngle(hand.rot.y, hand.rotAim.y) < 0.1) {
                return TRUE;
            }
            break;
        case HAND_FROG_WALK:
            if (controlMethod == CONTROL_HAND && pad[ActiveController].button[BUTTON_B] && hand.obj->onGround
                && FindShortestAngle(hand.rot.y, hand.rotAim.y) < 0.1) {
                return TRUE;
            }
            break;
        case HAND_LOCATE:
            if (ballStatic != 2 || gameInfo.level == WAYROOM || gameInfo.level == CAVE || (u32)gameInfo.level < 7) {
                if (pad[ActiveController].button[BUTTON_B] && !HandSnatchAllowed() && hand.vel.y > -9.0f) {
                    handControl.locateGaribs = 0;
                    return TRUE;
                }
            }
            return FALSE;
        case HAND_FROG_TONGUE:
            if (pad[ActiveController].button[BUTTON_Z] != 0) {
                return TRUE;
            }
            break;
    }
    return FALSE;
}

void ApplyAction(s32 action) {
    Vec3 tempVect;
    Plane *plane;
    s32 j;
    s32 found;
    s32 lastActionTime;
    f32 speed;
    s32 lifetime;
    GameLight *light;

    lastActionTime = handControl.actionTime;
    if (hand.state->mode == HAND_ROTOR) {
        DeactivateCollectable(ROTORBLADES);
    }
    if (cameraInfo.focus == &camPos) {
        cameraInfo.focus = &hand.pos;
        cameraInfo.focusVel = &hand.vel;
    }
    if (handControl.actionLock && gameInfo.cameoActive) {
        return;
    }
    if (action == 28 && IsHandHoldingBall()) {
        ReleaseBall();
    }
    hand.state->prevMode = hand.state->mode;
    hand.state->mode = action;
    ball.phys->moveSpeed = 13.0f;
    handControl.actionTime = 0;
    handControl.lateBounceQueued = 0;
    handControl.frontProbe.flags &= ~0x10;
    CalcMaxActorRadius(&hand);
    if (action != 25 && action != 26) {
        handControl.edgePlatform = NULL;
    }
    switch (action) {
        case HAND_IDLE:
            if (hand.anim->current == HANDANIM_SKID) {
                hand.anim->speed = 0.666f;
                AnimateActor(&hand, HANDANIM_STAND, 1, 1, 0.0f);
            } else if (hand.state->prevMode == HAND_PUSH) {
                AnimateActor(&hand, HANDANIM_PUSH2STAND, 0, 0, 0.0f);
                AnimateActor(&hand, HANDANIM_STAND, 1, 1, 0.0f);
            } else if (hand.anim->queue[0] != 42) {
                if (hand.anim->current == HANDANIM_GETUP) {
                    FlushQueue(&hand);
                    AnimateActor(&hand, HANDANIM_STAND, 1, 1, 0.0f);
                } else {
                    AnimateActor(&hand, HANDANIM_STAND, 1, 0, 0.0f);
                }
            }
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 13.0f);
            break;
        case HAND_WALK:
            if (handControl.idleTime > 400) {
                AnimateActor(&hand, HANDANIM_GETUP, 0, 0, 1.0f);
                AnimateActor(&hand, HANDANIM_TIPTOE, 1, 1, 0.0f);
                handControl.gait = HANDWALK_TIPTOE;
            } else {
                if (hand.state->prevMode == HAND_PUSH) {
                    AnimateActor(&hand, HANDANIM_PUSH2STAND, 0, 0, 0.0f);
                    hand.anim->speed = 2.0f * hand.anim->speed;
                    AnimateActor(&hand, HANDANIM_TIPTOE, 1, 1, 0.0f);
                } else {
                    AnimateActor(&hand, HANDANIM_TIPTOE, 1, 0, 0.0f);
                }
                handControl.gait = -1;
            }
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 13.0f);
            break;
        case HAND_SKIDDING:
            hand.friction = 0.85f;
            DisableControl(8);
            handControl.actionLock = 8;
            AnimateActor(&hand, HANDANIM_SKID, 0, 0, 0.0f);
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 13.0f);
            hand.rotAim.y = hand.rot.y;
            PlaySample(150, &hand.pos, 50, 128);
            break;
        case HAND_JUMP:
            hand.phys->moveSpeed = 7.0f;
            if (hand.state->prevMode == HAND_BALLWALK) {
                handControl.spellDelay = 5;
                if (ball.state->waterState) {
                    ball.vel.y = -5.0f;
                }
                ReleaseBall();
            }
            handControl.doubleJumped = 0;
            FlushQueue(&hand);
            AnimateActor(&hand, HANDANIM_JUMP, 0, 0, 0.0f);
            if (ball.phys->maxSquash != 0.0f && hand.state->prevMode == HAND_SLAM && !hand.obj->onGround) {
                SquashActor(&ball, &vertQ, 10.0f, ball.phys->maxSquash);
                hand.vel.y = levelHandScale * 15.0 / 0.05;
                PlaySample(19, &ball.pos, 100, 128);
            } else {
                hand.vel.y = levelHandScale * 7.0 / 0.05;
            }
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 13.0f);
            PlaySample((Random(2) + 249), &hand.pos, 110, 130);
            handControl.actionLock = 1;
            hand.friction = handInertia;
            if (handControl.prevAction == HAND_BALLWALK && ball.phys->maxSquash != 0.0f) {
                hand.vel.x = hand.vel.z = 0.0f;
                hand.vel.y += (f32)handControl.bounceCount * 1.2;
                if (handControl.bounceCount < 3) {
                    handControl.bounceCount++;
                }
                PlaySample(19, &ball.pos, handControl.bounceCount * 20 + 50, handControl.bounceCount * 10 + 100);
            }
            if (hand.obj->platform != NULL && hand.obj->platform->bounce != 0.0f) {
                hand.vel.y += 2.0 * (f32)handControl.platformBounceCount;
                if (handControl.platformBounceCount < 4) {
                    handControl.platformBounceCount++;
                }
            }
            break;
        case HAND_JOINED:
            ballWithHand = TRUE;
            if (hand.state->prevMode != HAND_THROW2CUP && hand.state->prevMode != HAND_BOUNCE2CUP) {
                ball.vel = ball.prevVel;
                MoveHandToBall();
                if (player.powerups[BOOMERANGBALL] && ballControl.returnTime >= 50) {
                    SCALE_VECTOR(&ball.vel, 0.35);
                }
                hand.vel = ball.vel;
                if (hand.state->prevMode != HAND_SNATCH) {
                    AnimateActor(&hand, HANDANIM_ST2CUP, 0, 0, 0.0f);
                }
                handControl.actionLock = 4;
                ball.link = &hand;
                handControl.releaseTimer = 0;
                ballControl.releaseTimer = 0;
                hand.shadow->shadowTarget = 0;
                hand.shadow->shadowStep = 20;
                cameraInfo.followTimer = 1;
                cameraInfo.followSpeed = 0;
            }
            handControl.releaseTimer = 0;
            hand.obj->spheres[0].targetRadius = ball.obj->spheres[0].targetRadius + 0.05;
            ballControl.returnTime = 0;
            ball.flags |= 1;
            break;
        case HAND_PREWHACK:
            AnimateActor(&hand, HANDANIM_WHACK1, 0, 0, 1.0f);
            AnimateActor(&hand, HANDANIM_WHACKIDLE, 1, 1, 0.0f);
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 8.0f);
            break;
        case HAND_WHACK:
            AnimateActor(&hand, HANDANIM_WHACK2, 0, 0, 1.0f);
            ball.rot = hand.rot;
            handControl.actionLock = 3;
            handControl.actionDelay = 1;
            handControl.spellDelay = 10;
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 13.0f);
            hand.shadow->shadowTarget = 255;
            hand.shadow->shadowStep = 30;
            ball.state->enteredWater = 0;
            break;
        case HAND_RELEASE:
            hand.friction = 0.8f;
            AnimateActor(&hand, HANDANIM_CUP2ST, 0, 0, 0.6f);
            handControl.actionLock = 5;
            handControl.frontProbe.pos = hand.pos;
            handControl.frontProbe.prevPos = hand.pos;
            speed = handControl.frontProbe.obj->spheres[0].radius;
            handControl.frontProbe.radius = handControl.frontProbe.obj->spheres[0].targetRadius = handControl.frontProbe.obj->spheres[0].radius = 26.0f;
            CalcCollisionSpheres(&handControl.frontProbe);
            handControl.savedFaceNum = facenum;
            for (j = 0; j < 8; j++) {
                tempGlobalRot[j] = globalRot[j];
                tempGlobalPos[j] = globalPos[j];
            }
            CheckCollisions(&handControl.frontProbe);
            specialActor = 0;
            for (j = 0; j < 8; j++) {
                globalRot[j] = tempGlobalRot[j];
                globalPos[j] = tempGlobalPos[j];
            }
            found = FALSE;
            handControl.frontProbe.pos = hand.pos;
            facenum = handControl.savedFaceNum;
            handControl.frontProbe.radius = handControl.frontProbe.obj->spheres[0].targetRadius = handControl.frontProbe.obj->spheres[0].radius = speed;
            for (j = 0; j < 4; j++) {
                if (handControl.frontProbe.obj->planes[j] != NULL && handControl.frontProbe.obj->planes[j]->y < 0.7) {
                    tempVect.x = 0.0f;
                    tempVect.y = 0.0f;
                    tempVect.z = 1.0f;
                    RotateVector2D(&tempVect, &tempVect, hand.rot.y);
                    if (DOT_PRODUCT(&tempVect, handControl.frontProbe.obj->planes[j]) > 0.2) {
                        found = TRUE;
                        break;
                    }
                }
            }
            if (found) {
                AccelerateActorFromActor(&hand, &ball, 4.0f, ball.phys->maxSpeed);
            } else {
                AccelerateActor(&hand, -10.0f, hand.phys->maxSpeed);
                hand.vel.y += 1.0f;
            }
            ReleaseBall();
            handControl.spellDelay = 10;
            break;
        case HAND_DUCK:
            if (hand.state->prevMode != HAND_CRAWL) {
                AnimateActor(&hand, HANDANIM_ST2DUCK, 0, 0, 0.0f);
                SetHandRadius(&hand.obj->spheres[0].targetRadius, 4.0f);
                hand.obj->spheres[0].radius = hand.obj->spheres[0].targetRadius;
                handControl.actionLock = 1036;
                DisableControl(8);
                hand.phys->moveSpeed = 17.0f;
            } else {
                hand.anim->loop = 0;
            }
            break;
        case HAND_CRAWL:
            AnimateActor(&hand, HANDANIM_CRAWL, 1, 0, 0.0f);
            hand.friction = handcrawlinertia;
            hand.phys->friction = handcrawlinertia;
            break;
        case HAND_GETUP:
            if ((hand.state->prevMode == HAND_DUCK && lastActionTime < 4) == FALSE || hand.anim->current == HANDANIM_CRAWL) {
                AnimateActor(&hand, HANDANIM_DUCK2ST, 0, 0, 0.0f);
                AnimateActor(&hand, HANDANIM_STAND, 1, 1, 0.0f);
                handControl.actionLock = 6;
                hand.vel.y += 4.0f;
                SetHandRadius(&hand.obj->spheres[0].radius, 1.0f);
                SetHandRadius(&hand.obj->spheres[0].targetRadius, 13.0f);
            }
            hand.phys->moveSpeed = 7.0f;
            hand.phys->friction = 0.8f;
            hand.friction = handInertia;
            break;
        case HAND_THROW2CUP:
            hand.rot.x = hand.rotAim.x = 0.0f;
            AnimateActor(&hand, HANDANIM_THROW2CUP, 0, 0, 0.0f);
            handControl.actionLock = 5;
            hand.obj->spheres[0].targetRadius = ball.obj->spheres[0].targetRadius + 0.05;
            break;
        case HAND_THROWAIM:
            if (hand.state->prevMode == HAND_JOINED) {
                AnimateActor(&hand, HANDANIM_THROWAIM, 0, 0, 0.7f);
            } else {
                AnimateActor(&hand, HANDANIM_THROWAIM, 0, 1, 2.0f);
                hand.vel.y = 0.0f;
                SCALE_VECTOR_2D(&hand.vel, 0.3);
            }
            if (!hand.obj->onGround) {
                hand.flags &= ~1;
            }
            handControl.bounceQueued = 0;
            handControl.actionLock = 4;
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 19.0f);
            break;
        case HAND_THROW:
            handControl.actionLock = 10;
            handControl.actionDelay = 1;
            hand.shadow->shadowTarget = 255;
            hand.shadow->shadowStep = 30;
            ball.state->enteredWater = 0;
            break;
        case HAND_FALLING:
            AnimateActor(&hand, HANDANIM_JUMP, 0, 0, 0.0f);
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 13.0f);
            break;
        case HAND_BOUNCE2CUP:
            AnimateActor(&hand, HANDANIM_BOUNCE2CUP, 0, 0, 0.7f);
            handControl.actionLock = 1028;
            hand.obj->spheres[0].targetRadius = ball.obj->spheres[0].targetRadius + 0.05;
            break;
        case HAND_BOUNCE:
            ball.link = &hand;
            ballControl.releaseTimer = 0;
            handControl.releaseTimer = 0;
            hand.shadow->shadowTarget = 0;
            hand.shadow->shadowStep = 30;
            AnimateActor(&hand, HANDANIM_CUP2BOUNCE, 0, 0, 0.0f);
            hand.obj->platform = hand.obj->prevPlatform = hand.obj->contactPlatform = hand.obj->prevContactPlatform = NULL;
            hand.obj->onGround = 0;
            hand.obj->planes[0] = NULL;
            FindFlattestPlane(&ball, &plane, &j);
            if (plane != NULL && plane->y > 0.1) {
                ball.vel.y = levelHandScale * 84.0 / 0.05 / MIN(MAX(ball.phys->mass, 10.0f), 15.0);
            }
            ball.phys->moveSpeed = BallBounceMaxSpeed();
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 12.0f);
            ball.state->enteredWater = 0;
            break;
        case HAND_AIRBOURNEWHACK:
            AnimateActor(&hand, HANDANIM_WHACK2, 0, 0, 1.0f);
            ball.rot = hand.rot;
            ball.pos = hand.pos;
            handControl.actionLock = 9;
            handControl.actionDelay = 1;
            handControl.bounceQueued = 0;
            handControl.spellDelay = 10;
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 13.0f);
            hand.shadow->shadowTarget = 255;
            hand.shadow->shadowStep = 30;
            break;
        case HAND_THROW2LAND:
            if (HandOnTrampoline() || (HandOnTrampoline() && hand.prevVel.y < -12.0)) {
                ApplyAction(HAND_JUMP);
                break;
            }
            handControl.platformBounceCount = 0;
            pad[ActiveController].sincePress[BUTTON_Z] = 100;
            if (hand.state->prevMode == HAND_RELEASE) {
                PlaySample(154, &hand.pos, 90, 128);
            }
            AnimateActor(&hand, HANDANIM_THROW2LAND, 0, 0, 0.8f);
            AnimateActor(&hand, HANDANIM_STAND, 1, 1, 0.0f);
            if (!pad[ActiveController].button[BUTTON_A]) {
                if (pad[ActiveController].mag == 0.0f) {
                    SCALE_VECTOR_2D(&hand.vel, 0.7);
                }
                handControl.actionLock = 3;
            }
            if (hand.obj->onGround) {
                AddGlovePrint(4);
                AddGlovePrint(-4);
            }
            break;
        case HAND_SLAM:
            PlaySample(168, &hand.pos, 100, 128);
            AnimateActor(&hand, HANDANIM_SLAM, 0, 0, 2.0f);
            handControl.actionLock = 4;
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 16.0f);
            break;
        case HAND_SLAM2ST:
            if (hand.obj->onGround) {
                AddGlovePrint(0);
            }
            tempVect = hand.pos;
            handControl.actionLock = 3;
            tempVect.y -= 10.0f;
            if (handControl.flags & 1) {
                hand.vel.y = 0.0f;
                break;
            }
            if (hand.obj->contactFlags) {
                PlaySample(166, &hand.pos, 100, 128);
                CreateDustCircle(&tempVect, 8, 4.0f);
                CreateBlastCircle();
                if (HandOnTrampoline()) {
                    ApplyAction(HAND_JUMP);
                }
            }
            break;
        case HAND_BALLWALK:
            speed = ABS(hand.vel.y / 25.0f);
            if (ball.phys->maxSquash != 0.0f) {
                PlaySample(19, &ball.pos, (2.0f * -hand.prevVel.y < 50.0f) ? 100 : (s16)(2.0f * -hand.prevVel.y + 50.0f), 128);
                SquashActor(&ball, &vertQ, speed, ball.phys->maxSquash);
            }
            if (hand.state->prevMode != HAND_SLAM2ST) {
                AnimateActor(&hand, HANDANIM_THROW2LAND, 0, 0, 0.0f);
                AnimateActor(&hand, HANDANIM_STAND, 1, 1, 0.0f);
            }
            ball.link = &hand;
            ballControl.releaseTimer = 0;
            handControl.releaseTimer = 0;
            handControl.gait = -1;
            if (hand.state->prevMode != HAND_JUMP) {
                handControl.actionLock = 15;
            }
            hand.shadow->shadowTarget = 0;
            hand.shadow->shadowStep = 30;
            AddOneScaledVector2D(&ball.vel, &ball.vel, &hand.vel,
                                 hand.phys->mass / ((ball.phys->mass < 10.0f) ? 15.0 : ball.phys->mass * 1.5));
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 13.0f);
            hand.obj->spheres[0].radius = hand.obj->spheres[0].targetRadius;
            break;
        case HAND_CARTWHEEL:
            AnimateActor(&hand, HANDANIM_CARTWHEEL, 0, 0, 0.8f);
            handControl.actionLock = 1029;
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 17.0f);
            ZERO_VECTOR(&hand.vel);
            break;
        case HAND_CLIMBDOWN:
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 2.0f);
            hand.obj->spheres[0].radius = hand.obj->spheres[0].targetRadius;
            hand.vel = zero;
            handControl.actionLock = 20;
            AnimateActor(&hand, HANDANIM_SIT, 0, 0, 0.0f);
            AnimateActor(&hand, HANDANIM_SITIDLE, 0, 1, 0.0f);
            hand.shadow->shadowTarget = 0;
            hand.shadow->shadowStep = 30;
            break;
        case HAND_CLING:
            PlaySample((Random(4) + 257), &hand.pos, 100, 128);
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 2.0f);
            hand.obj->spheres[0].radius = hand.obj->spheres[0].targetRadius;
            handControl.frontProbe.flags |= 0x10;
            AnimateActor(&hand, HANDANIM_CLING, 0, 0, 0.0f);
            AnimateActor(&hand, HANDANIM_CLINGIDLE, 1, 1, 0.0f);
            handControl.actionLock = 20;
            handControl.frontProbe.rot.y = hand.rotAim.y;
            hand.vel = zero;
            AddScaledVector2D(&sepVect, &sepVect, 0.8f, &hand.pos, 0.2f);
            break;
        case HAND_CLINGUP:
            PlaySample((Random(4) + 257), &hand.pos, 100, 128);
            AnimateActor(&hand, HANDANIM_CLINGUP, 0, 0, 0.0f);
            AnimateActor(&hand, HANDANIM_STAND, 1, 1, 0.0f);
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 13.0f);
            sepVect = handControl.frontProbe.pos;
            handControl.actionLock = 10;
            sepVect.y += 16.0f;
            break;
        case HAND_CLIMBUP:
            PlaySample(152, &hand.pos, 120, 128);
            AnimateActor(&hand, HANDANIM_SITUP, 0, 0, 0.0f);
            hand.shadow->shadowTarget = 255;
            hand.shadow->shadowStep = 30;
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 13.0f);
            hand.obj->spheres[0].radius = hand.obj->spheres[0].targetRadius;
            handControl.actionLock = 20;
            hand.flags |= 1;
            DisableControl(20);
            break;
        case HAND_DROPDOWN:
            hand.shadow->shadowTarget = 255;
            hand.shadow->shadowStep = 30;
            handControl.edgePlatform = NULL;
            handControl.climbPlatform = NULL;
            if (hand.state->prevMode == HAND_CLIMBDOWN) {
                AccelerateActor2D(&hand, 4.0f, hand.phys->maxSpeed);
                AnimateActor(&hand, HANDANIM_SIT2DOWN, 0, 0, 0.0f);
            } else {
                AccelerateActor2D(&hand, -4.0f, hand.phys->maxSpeed);
                AnimateActor(&hand, HANDANIM_CLINGDOWN, 0, 0, 0.0f);
            }
            break;
        case HAND_HURT:
            if (ball.link == &hand) {
                ReleaseBall();
            }
            handControl.actionLock = 2003;
            AnimateActor(&hand, HANDANIM_HURT, 0, 0, 0.5f);
            hand.shadow->shadowTarget = 255;
            hand.shadow->shadowStep = 30;
            break;
        case HAND_PUSH:
            PlaySample((Random(4) + 257), &hand.pos, 100, 128);
            AnimateActor(&hand, HANDANIM_STAND2PUSH, 0, 0, 0.0f);
            AnimateActor(&hand, HANDANIM_PUSH, 1, 1, 0.0f);
            handControl.actionLock = 1036;
            break;
        case HAND_LOB:
            handControl.actionLock = 1046;
            cameraInfo.focus = &hand.pos;
            ball.vel = hand.vel;
            cameraInfo.focusVel = &hand.vel;
            ball.state->enteredWater = 0;
            AnimateActor(&hand, HANDANIM_LOB, 0, 0, 0.8f);
            break;
        case HAND_CATCH:
            AnimateActor(&hand, HANDANIM_THROWAIM, 0, 0, 2.0f);
            hand.rot.x = hand.rotAim.x = 0.5f;
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 19.0f);
            hand.obj->spheres[0].radius = hand.radius;
            ball.link = &hand;
            ballControl.releaseTimer = 0;
            handControl.releaseTimer = 0;
            break;
        case HAND_CASTSPELL:
            switch (player.castSpell) {
                case -1:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                    PlaySpellSample();
                    AnimateActor(&hand, HANDANIM_BALLSPELL, 0, 0, 1.0f);
                    lifetime = 1;
                    speed = 8.0f;
                    break;
                case 0:
                case 1:
                case 2:
                    PlaySpellSample();
                    AnimateActor(&hand, HANDANIM_NMESPELL, 0, 0, 0.0f);
                    lifetime = 10;
                    speed = 8.0f;
                    break;
                case 15:
                    if ((cheatActivated & 1) || locateGaribsAllowed) {
                        if (numGaribs > player.garibs) {
                            handControl.locateGaribs = 1;
                            ApplyAction(HAND_LOCATE);
                            return;
                        }
                        AnimateActor(&hand, HANDANIM_OKAY, 0, 0, 0.0f);
                        hand.rotAim.y = camera.rot.y;
                        hand.state->mode = HAND_NOTHING;
                        handControl.actionLock = 1013;
                        DisableControl(20);
                        SCALE_VECTOR(&hand.vel, 0.5);
                    }
                    return;
                default:
                    PlaySpellSample();
                    AnimateActor(&hand, HANDANIM_HANDSPELL, 0, 0, 0.6f);
                    lifetime = 15;
                    speed = 4.0f;
                    break;
            }
            handControl.actionLock = hand.anim->current + 1000;
            light = JallocAlloc(0x58, 1, "Light");
            SetupLight(light, 255, 50, 50, 255, 80, 80, 0, 0, speed);
            light->timer = lifetime;
            light->type = 2;
            light->spin[0] = 0.7f;
            AddBackgroundLight(light);
            light = JallocAlloc(0x58, 1, "Light");
            SetupLight(light, 50, 255, 50, 80, 255, 80, 0, 0, speed);
            light->timer = lifetime;
            light->type = 2;
            light->spin[1] = 1.0f;
            AddBackgroundLight(light);
            break;
        case HAND_ROTOR:
            AnimateActor(&hand, HANDANIM_THROWAIM, 0, 0, 2.0f);
            hand.obj->spheres[0].flags &= ~0x20;
            hand.obj->spheres[0].pos.y = -9.0f;
            hand.rot.x = hand.rotAim.x = 3.1415927f;
            hand.pos.y -= 9.0f;
            GetQuaternionFromXYZRot(&hand.rotQuat, &hand.rot);
            PlayContinuousSample(&handControl.samples[1], 312, 40, &hand.pos, 20);
            break;
        case HAND_SNATCH:
            AnimateActor(&hand, HANDANIM_SNATCH, 0, 0, 2.0f);
            PlaySample(167, &hand.pos, 100, 160);
            camPos = hand.pos;
            sepVect = hand.pos;
            ball.link = &hand;
            camPos2 = ball.pos;
            cameraInfo.focus = &camPos;
            ballControl.releaseTimer = 0;
            handControl.releaseTimer = 0;
            cameraInfo.focusVel = &zero;
            if (ball.radius > 9.0) {
                camPos2.y += 2.0 * (ball.radius - 9.0);
            }
            SUB_VECTORS(&tempVect, &ball.pos, &hand.pos);
            hand.rotAim.y = Aabs(atan2(tempVect.x, tempVect.z));
            pad[ActiveController].latch[BUTTON_B] = 1;
            handControl.climbPlatform = hand.obj->platform;
            break;
        case HAND_SLIDING:
            if (!handControl.deathTimer && !(hand.flags & 0x2000) && !handControl.steepSlide) {
                AnimateActor(&hand, HANDANIM_SKID, 0, 0, 0.0f);
            }
            handControl.actionLock = 10;
            break;
        case HAND_HOPPING:
            AnimateActor(&hand, HANDANIM_DENIS, 1, 0, 0.7719f);
            handControl.attachedTo = hand.link;
            hand.rotAim.y = hand.link->rotAim.y;
            cameraInfo.focus = &hand.link->pos;
            hand.flags &= ~0x100;
            break;
        case HAND_FROG_IDLE:
            if (ball.link == &hand) {
                ReleaseBall();
                AccelerateActor(&hand, -10.0f, hand.phys->maxSpeed);
                handControl.releaseTimer = 0;
                ball.link = NULL;
                hand.obj->spheres[0].radius = hand.obj->spheres[0].targetRadius = hand.phys->radius;
            }
            hand.friction = 0.9f;
            AnimateActor(&hand, HANDANIM_FALL, 1, 1, 0.0f);
            break;
        case HAND_FROG_WALK:
            hand.friction = 0.9f;
            AnimateActor(&hand, HANDANIM_DENIS, 0, 0, 0.9f);
            handControl.bounceCount = 0;
            break;
        case HAND_FROG_JUMP:
            hand.friction = 0.9f;
            hand.anim->current = HANDANIM_BALLWALK;
            AnimateActor(&hand, HANDANIM_GETUP, 0, 0, 0.0f);
            hand.vel.y = handFrogVelY;
            PlaySample(104, &hand.pos, 100, 118);
            AccelerateActor2D(&hand, handFrogVelXZ, hand.phys->moveSpeed);
            handControl.actionLock = 5;
            if (hand.obj->platform != NULL) {
                AddToVector2D(&hand.vel, &hand.obj->platform->actor.vel);
            }
            break;
        case HAND_SWISH:
            AnimateActor(&hand, HANDANIM_SWISH, 0, 0, 0.0f);
            break;
        case HAND_LOCATE:
            if (handControl.attachedTo != NULL) {
                EnemyDropHand();
            }
            AnimateActor(&hand, HANDANIM_POINT, 0, 0, 0.0f);
            AnimateActor(&hand, HANDANIM_POINTCYCLE, 1, 1, 0.0f);
            origQ = hand.rotQuat;
            if (!handControl.locateGaribs) {
                if (ballControl.loseTimer) {
                    AddHandLocateSprite(&garibAnimation[1]);
                } else {
                    AddHandLocateSprite(&garibAnimation[2]);
                }
            } else if (cheatActivated & 0x2000) {
                AddHandLocateSprite(&garibAnimation[3]);
            } else {
                AddHandLocateSprite(&garibAnimation[0]);
            }
            break;
        case HAND_SPIN:
            if (levelExitTimer || Random(50) < 20 || ballControl.transformState) {
                SetHandRadius(&hand.obj->spheres[0].targetRadius, 19.0f);
                if (ballControl.transformState) {
                    AnimateActor(&hand, HANDANIM_CUP2SPIN, 0, 0, 2.5f);
                    AnimateActor(&hand, HANDANIM_SPIN, 1, 1, 2.5f);
                } else {
                    AnimateActor(&hand, HANDANIM_CUP2SPIN, 0, 0, 0.0f);
                    AnimateActor(&hand, HANDANIM_SPIN, 1, 1, 0.0f);
                }
            } else {
                SetHandRadius(&hand.obj->spheres[0].targetRadius, 10.0f);
                AnimateActor(&hand, HANDANIM_CUP2PEDDLE, 0, 0, 0.0f);
                AnimateActor(&hand, HANDANIM_PEDDLE, 1, 1, 0.0f);
            }
            handControl.idleTime = 0;
            break;
        case HAND_FROG_TONGUE:
            PlaySample(8, &hand.pos, 100, 60);
            tongueBullet = FireBulletFromActor(&hand, TONGUE_BULLET, 0);
            if (tongueBullet->target != NULL) {
                tongueTime = tTime1;
            } else {
                tongueTime = tTime2;
            }
            tongueStuck = 0;
            hand.rotAim.y = hand.rot.y;
            if (tongueBullet != NULL) {
                tongueBullet->sprites[0].flags = 0;
                tongueBullet->sprites[0].scaleX = tongueScale2;
                tongueBullet->sprites[0].scaleY = tongueScale2;
                tongueBullet->sprites[0].anim.def.startScale = tongueScale2;
                tongueBullet->sprites[0].anim.def.endScale = tongueScale2;
                SubSprite(&tongueBullet->sprites[0]);
                AddSprite(&tongueBullet->sprites[0], NULL);
                SUB_VECTORS(&tongueOffsetPos[0], &tongueBullet->actor.pos, &hand.pos);
            }
            break;
    }
}

static inline void AddVectorInline(Vec3 *out, Vec3 *a, Vec3 *b) {
    ADD_VECTORS(out, a, b);
}

void MaintainAction(void) {
    Actor *actor;
    s32 prevAnim;
    s32 startRoll;
    Vec3 tempVect;
    Vec3 tempVect2;
    Quat q;
    Quat qAim;
    s32 unused[6]; // forces the original stack frame size
    Actor *base;
    Actor *holder;
    Bullet *bullet;
    Debris *debris;
    GameSprite *sprite;
    GameSprite *locSprite;
    Garib *garib2;
    Garib *garib;
    GaribGroup *garibGroup;
    Plane *plane;
    Platform *platform;
    f32 garibDist;
    f32 dist;
    f32 temp;
    f32 tempFloat;
    f32 scale;
    f32 ratio;
    f32 animSpeed;
    f32 distSq;
    f32 val;
    f32 maxDownSpeed;
    f32 maxUpSpeed;
    s32 y;
    s32 j;
    s32 x;

    if (player.powerups[SPEEDUP] >= 40) {
        tempVect.y = 0.0f;
        tempVect.z = 0.0f;
        tempVect.x = 8.0f;
        RotateVectorByQuaternion(&tempVect2, &tempVect, &hand.rotQuat);
        ADD_VECTORS(&tempVect, &hand.pos, &tempVect2);
        SUB_VECTORS(&tempVect2, &hand.pos, &tempVect2);
        temp = Random(5) - 2;
        tempVect.y += temp;
        tempVect2.y -= temp;
        if (blur1 == NULL) {
            blur1 = JallocAlloc(sizeof(Blur), 1, "Blur");
            AddBlur(blur1);
            StartBlurFX(blur1, &tempVect, &tempVect2, 255, 255, 30, 30);
        } else {
            IncrementBlurFX(blur1, &tempVect, &tempVect2);
        }
        if (hand.anim->current == HANDANIM_RUN || hand.anim->current == HANDANIM_BALLWALK2) {
            handControl.samples[1].volume = 100;
            if (handControl.samples[1].handle == 0) {
                PlayContinuousSample(&handControl.samples[1], 129, 100, &hand.pos, 180);
            } else if (frameCount % 10 == 0) {
                handControl.samples[1].pitch = MIN(MAX(160, handControl.samples[1].pitch + Random(13) - 6), 200);
            }
        } else if (handControl.samples[1].handle != 0) {
            handControl.samples[1].volume -= 10;
            if (handControl.samples[1].volume <= 0) {
                StopContinuousSample(&handControl.samples[1]);
            }
        }
    } else if ((player.powerups[SPEEDUP] == 1 || frameCount == 1
                || (handControl.checkpoint != NULL && handControl.checkpoint->restartGate->restartTimer != 0))
               && blur1 != NULL) {
        SubBlur(blur1);
        JallocFree(&blur1);
    }
    if (frameCount == 1 && blur2 != NULL) {
        SubBlur(blur2);
        JallocFree(&blur2);
    }
    if ((hand.state->mode == HAND_NOTHING && levelExitTimer == 0)
        || ((hand.state->mode == HAND_THROWAIM || (hand.state->mode == HAND_LOCATE && hand.anim->current != HANDANIM_POINT2IDLE))
            && !hand.obj->onGround)) {
        hand.flags &= ~1;
    } else {
        hand.flags |= 1;
    }
    if (hand.state->mode != HAND_PREWHACK && hand.state->mode != HAND_THROWAIM) {
        if (gameInfo.level == SPACEBOSS3) {
            hand.phys->turnSpeed = 0.5f;
        } else if (hand.state->mode == HAND_ROTOR) {
            hand.phys->turnSpeed = 0.2f;
        } else {
            hand.phys->turnSpeed = 0.6f;
        }
    }
    if (hand.state->mode == HAND_BOUNCE || hand.state->mode == HAND_ROTOR) {
        hand.flags |= 0x40;
    } else {
        hand.flags &= ~0x40;
    }
    if (hand.state->mode == HAND_SLAM) {
        hand.flags |= 0x200;
    } else {
        hand.flags &= ~0x200;
    }
    if (hand.state->mode != HAND_BOUNCE) {
        handControl.bounceQueued = 0;
    }
    if (handControl.spellDelay > 0) {
        handControl.spellDelay -= 1;
    }
    if (hand.state->mode == HAND_JOINED || hand.state->mode == HAND_PREWHACK || hand.state->mode == HAND_BOUNCE) {
        hand.flags &= ~0x18;
        hand.obj->platform = ball.obj->platform;
        hand.obj->prevPlatform = ball.obj->prevPlatform;
        hand.obj->contactPlatform = ball.obj->contactPlatform;
        hand.obj->prevContactPlatform = ball.obj->prevContactPlatform;
        hand.obj->onGround = ball.obj->onGround;
    } else if (!handControl.deathTimer && hand.state->mode != HAND_NOTHING) {
        hand.flags |= 0x18;
    }
    if (hand.state->mode == HAND_JOINED) {
        if (!(hand.flags & 0x2000)) {
            ball.flags &= 0xFEFFFFFF;
        }
        base = &ball;
    } else {
        base = &hand;
    }
    if (handControl.joinTimer != 0) {
        handControl.joinTimer -= 1;
    }
    if (IsHandHoldingBall() == 0 && hand.state->mode != HAND_ROTOR && pad[ActiveController].button[BUTTON_B]) {
        handControl.joinTimer++;
    }
    if (hand.obj->onGround == 1 && hand.state->mode != HAND_FROG_WALK) {
        handControl.bounceCount = 0;
        handControl.doubleJumped = 0;
    }
    if (handControl.releaseTimer && ball.link == &hand) {
        if (--handControl.releaseTimer == 0) {
            ball.link = NULL;
        }
    } else if (handControl.releaseTimer && hand.link) {
        if (--handControl.releaseTimer == 0) {
            hand.link = NULL;
        }
    } else {
        handControl.releaseTimer = 0;
    }
    switch (hand.state->mode) {
    case HAND_JOINED:
        if (ball.state->waterState && (ball.flags & 0x200000) && ball.shadow->altitude > ball.obj->spheres[0].radius + 11.0) {
            ReleaseBall();
            ApplyAction(HAND_BALLWALK);
        } else if (pad[ActiveController].pressed[BUTTON_SHOULDER_LEFT] && controlMethod == CONTROL_HAND) {
            ApplyAction(HAND_BALLWALK);
        } else if (controlMethod == CONTROL_HAND && handControl.idleTime == 400) {
            ApplyAction(HAND_SPIN);
        } else if (!(cheatActivated & 0x80) && controlMethod == CONTROL_HAND && !ballControl.cancerTimer && ballControl.snow <= 0.9
                   && pad[ActiveController].button[BUTTON_SHOULDER_RIGHT] && !ballControl.transformState) {
            ballControl.transformState = 30;
            PlaySpellSample();
            ApplyAction(HAND_SPIN);
        } else {
            if (hand.anim->finishedAnim && Random(500) < 10) {
                AnimateActor(&hand, HANDANIM_CUPIDLE, 0, 0, 0.0f);
            }
            ball.phys->moveSpeed = BallJoinedMaxSpeed();
            hand.obj->spheres[0].targetRadius = ball.obj->spheres[0].targetRadius + 0.05;
            if (ballControl.transformState == 0 && handControl.actionTime >= 11 && player.powerups[VANISHBALL] == 0
                && (handControl.checkpoint == NULL || handControl.checkpoint->restartGate->restartTimer == 0)) {
                val = ball.obj->spheres[0].radius - 9.0;
                if (val > 0.0f) {
                    val += MIN((ball.obj->spheres[0].radius - 9.0) / 2.0, 0.5);
                }
                if (!shrinkExitTimer && DistanceBetweenPoints2D(&hand.pos, &ball.pos) > val + 13.0 && !player.powerups[SPEEDUP]) {
                    hand.vel = ball.vel;
                    ApplyAction(HAND_SKIDDING);
                    handControl.actionLock = 10;
                    ReleaseBall();
                }
                if (ball.obj->spheres[0].radius + 5.0 + MAX(0.0, 9.0 - ball.radius) < hand.pos.y - ball.pos.y) {
                    ApplyAction(HAND_BOUNCE);
                    hand.vel.y = ball.vel.y;
                }
            }
        }
        break;
    case HAND_IDLE:
        if (handControl.idleTime >= 401 && controlMethod == CONTROL_HAND) {
            if (hand.anim->current == HANDANIM_STAND) {
                if (Random(50) < 25 || player.powerups[SUCTION] != 0) {
                    AnimateActor(&hand, HANDANIM_GETDOWN, 0, 0, 1.0f);
                    AnimateActor(&hand, HANDANIM_IDLE1, 0, 1, 0.0f);
                } else {
                    AnimateActor(&hand, HANDANIM_IDLE2WAVE, 0, 0, 0.0f);
                    AnimateActor(&hand, HANDANIM_WAVE, 0, 1, 0.0f);
                    AnimateActor(&hand, HANDANIM_IDLE2WAVE, 0, 1, -0.66f);
                    AnimateActor(&hand, HANDANIM_STAND, 1, 1, 0.0f);
                    hand.rotAim.y = Aabs(camera.rot.y + PI);
                    handControl.idleTime = 0;
                }
            } else {
                if (handControl.idleTime > 600.0) {
                    AnimateActor(&hand, HANDANIM_GETUP, 0, 0, 0.0f);
                    AnimateActor(&hand, HANDANIM_STAND, 1, 1, 0.0f);
                    handControl.idleTime = 0;
                }
                if (hand.anim->current == HANDANIM_IDLE1 && TriggerAtFrame(328.0f, &hand)) {
                    PlaySample(164, &hand.pos, 90, Random(16) + 120);
                }
            }
            if (frameCount % 30 == 0 && Random(10) < 4) {
                AnimateActor(&hand, HANDANIM_IDLE1, 0, 1, 0.0f);
                AnimateActor(&hand, HANDANIM_IDLE1, 0, 1, 0.0f);
                if (Random(10) < 5) {
                    AnimateActor(&hand, HANDANIM_IDLE1, 0, 1, 0.0f);
                }
            }
        }
        break;
    case HAND_CRAWL:
        if (TriggerAtFrame(handcrawlframe, &hand)) {
            PlaySample(crawlfx, &hand.pos, 80, Random(56) + 100);
        }
        break;
    case HAND_WALK:
    case HAND_PUSH:
        if (hand.state->prevMode == HAND_SKIDDING && handControl.actionTime < 3) {
            hand.rot.y = hand.rotAim.y;
        }
        if (hand.anim->queue[0] == -1 || hand.state->mode != HAND_WALK) {
            prevAnim = handControl.gait;
            if (hand.anim->current == HANDANIM_STUN || hand.anim->current == HANDANIM_PUSH2STAND) {
                break;
            }
            SUB_VECTORS(&tempVect, &hand.pos, &hand.prevPos);
            if (hand.flags & 0x8000) {
                hand.phys->moveSpeed = 2.3333333f;
                distSq = DOT_PRODUCT(&tempVect, &tempVect);
            } else if (!hand.obj->onGround && (hand.flags & 0x1000)) {
                hand.phys->moveSpeed = 1.4f;
                distSq = tempVect.x * tempVect.x + tempVect.z * tempVect.z;
            } else {
                hand.phys->moveSpeed = 7.0f;
                distSq = DOT_PRODUCT(&tempVect, &tempVect);
            }
            temp = sqrtf(distSq);
            if (player.powerups[SPEEDUP]) {
                temp *= 1.5;
            }
            if (handControl.flags & 1) {
                temp = 1.0f;
                handControl.gait = HANDWALK_RUN;
            } else if (temp == 0.0f) {
                handControl.gait = -1;
            } else if (temp < 1.6 || (hand.flags & 0x8000)) {
                handControl.gait = HANDWALK_TIPTOE;
                handControl.walkTime++;
                temp = MIN(temp * 4.0f, 2.8);
            } else if (temp < 3.5 && hand.state->waterState != INWATER_SWIMMING) {
                handControl.gait = HANDWALK_WALK;
                handControl.walkTime++;
                temp = MIN((temp - 0.7) * 0.8, 1.4);
            } else {
                temp = 1.0f;
                handControl.walkTime = 0;
                handControl.gait = HANDWALK_RUN;
            }
            if (hand.state->mode == HAND_PUSH) {
                y = -1;
                temp = 2.0f;
                for (x = 0; x < 4; x++) {
                    if (hand.obj->planes[x] != NULL && ABS(hand.obj->planes[x]->y) < temp) {
                        y = x;
                        temp = ABS(hand.obj->planes[x]->y);
                    }
                }
                if (y != -1) {
                    temp = handControl.inputAccel.x * hand.obj->planes[y]->x
                               + handControl.inputAccel.z * hand.obj->planes[y]->z;
                    if (ABS(temp) > 0.01) {
                        SetVector2D(&hand.vel, (Vec3 *)base->obj->planes[y]);
                        SCALE_VECTOR_2D(&hand.vel, temp);
                        hand.rotAim.y = Aabs(atan2(hand.vel.x, hand.vel.z));
                        hand.rotAim.x =
                            atan2(hand.obj->planes[y]->y,
                                  LENGTH_2D(hand.obj->planes[y]));
                    }
                }
            } else {
                if (prevAnim != handControl.gait && hand.anim->queue[0] == -1) {
                    switch (handControl.gait) {
                        case HANDWALK_TIPTOE:
                            AnimateActor(&hand, HANDANIM_TIPTOE, 1, 0, 0.0f);
                            break;
                        case HANDWALK_WALK:
                            AnimateActor(&hand, HANDANIM_WALK, 1, 0, 0.0f);
                            break;
                        case HANDWALK_RUN:
                            AnimateActor(&hand, HANDANIM_RUN, 1, 0, 0.0f);
                            break;
                    }
                }
                if (player.powerups[SPEEDUP]) {
                    temp *= 1.5;
                }
                SetAnimationSpeed(&hand, temp);
            }
            CheckForFootsteps();
        }
        break;
    case HAND_HURT:
        if (hand.anim->finishedAnim == 42) {
            hand.state->mode = HAND_JUMP;
        }
        break;
    case HAND_DUCK:
        SCALE_VECTOR(&hand.vel, 0.8);
        break;
    case HAND_RELEASE:
        hand.friction = 0.8f;
        if (hand.state->prevMode == HAND_BOUNCE) {
            hand.vel.x *= 1.05;
            hand.vel.z *= 1.05;
        }
        if (hand.state->waterState == INWATER_WADING || hand.state->waterState == INWATER_SWIMMING) {
            SCALE_VECTOR_2D(&hand.vel, 1.4);
        }
        if (player.powerups[HERCULES] || ball.radius > 9.0) {
            hand.vel.x *= 1.1;
            hand.vel.z *= 1.1;
        }
        if (player.powerups[HERCULES] && player.powerups[BEACHBALL]) {
            hand.vel.x *= 1.05;
            hand.vel.z *= 1.05;
        }
        if (player.powerups[BEACHBALL]) {
            hand.vel.x *= 1.1;
            hand.vel.z *= 1.1;
        }
        if (hand.state->prevMode == HAND_BALLWALK && ball.radius > 9.0) {
            hand.vel.x *= 1.1;
            hand.vel.z *= 1.1;
        }
        if (((hand.flags & 0x1000) || hand.state->waterState) && !handControl.actionLock) {
            hand.state->mode = HAND_JUMP;
        }
        break;
    case HAND_WHACK:
    case HAND_AIRBOURNEWHACK:
        if (handControl.actionDelay) {
            handControl.actionDelay--;
            ZERO_VECTOR(&hand.vel);
            ball.vel = hand.vel;
            if (!handControl.actionDelay) {
                HandWhackActor(&ball);
                PlaySample(171, &hand.pos, 100, 128);
                ReleaseBall();
                handControl.actionLock = 7;
                ActorStartRoll(&ball, ball.vel.x, ball.vel.z);
                ballControl.prevGround[1] = 1;
                if (player.powerups[BOOMERANGBALL]) {
                    ballControl.returnTime = 1;
                }
            }
        } else if (hand.anim->finishedAnim == 72) {
            ApplyActionIdle();
        }
        break;
    case HAND_PREWHACK:
        if (pad[ActiveController].button[BUTTON_Z] && controlMethod == CONTROL_HAND) {
            pad[ActiveController].latch[BUTTON_A] = 1;
            ApplyAction(HAND_JOINED);
            AnimateActor(&hand, HANDANIM_WHACK1, 0, 0, -1.0f);
        } else {
            MoveHandToBallStatic();
            hand.vel = zero;
            if (handControl.actionTime > 20 && handControl.fakeball == NULL && !handControl.fakeballDelay) {
                AddFakeball();
                if (handControl.fakeball != NULL) {
                    HandWhackActor((Actor *)handControl.fakeball);
                }
            }
        }
        break;
    case HAND_THROWAIM:
        SCALE_VECTOR(&hand.vel, 0.5);
        if (hand.state->prevMode == HAND_CATCH && handControl.actionTime >= 4) {
            ApplyAction(HAND_JOINED);
            AnimateActor(&hand, HANDANIM_THROWAIM, 0, 0, -1.0f);
            break;
        }
        if (!hand.obj->onGround && !(frameCount & 1)) {
            tempVect.x = Random(11) - 5;
            tempVect.y = -22.0f;
            tempVect.z = Random(11) - 5;
            ADD_TO_VECTOR(&tempVect, &hand.pos);
            debris = AddDebris(WISP, &tempVect, &hand.vel, NULL, NULL);
            if (debris != NULL) {
                debris->flags |= 0x41;
                debris->info->friction = 0.6f;
                debris->friction = 0.6f;
            }
        }
        hand.vel.y = 0.0f;
        if (pad[ActiveController].button[BUTTON_Z] && controlMethod == CONTROL_HAND) {
            pad[ActiveController].latch[BUTTON_B] = 1;
            ApplyAction(HAND_JOINED);
            AnimateActor(&hand, HANDANIM_THROWAIM, 0, 0, -1.0f);
            break;
        }
        if (handControl.actionTime > 20 && handControl.fakeball == NULL && !handControl.fakeballDelay) {
            AddFakeball();
            if (handControl.fakeball != NULL) {
                HandThrowActor((Actor *)handControl.fakeball);
            }
        }
        break;
    case HAND_THROW:
        if (handControl.actionDelay && !--handControl.actionDelay) {
            AnimateActor(&hand, HANDANIM_THROW, 0, 0, 0.5f);
            AnimateActor(&hand, HANDANIM_STAND, 1, 1, 0.0f);
            handControl.actionLock = 12;
            HandThrowActor(&ball);
            ActorStartRoll(&ball, ball.vel.x, ball.vel.z);
            hand.vel.y += gravity * 0.7 * 10.0;
            hand.vel.y *= hand.phys->mass / 10.0f;
            ReleaseBall();
            PlaySample(167, &hand.pos, 100, 128);
            if (ballControl.type == BALL_MODE_BOWLING) {
                PlaySample(260, &hand.pos, 100, 128);
            }
            if (player.powerups[BOOMERANGBALL]) {
                ballControl.returnTime = 1;
            }
        }
        break;
    case HAND_SKIDDING:
        hand.friction = 0.85f;
        tempVect = hand.pos;
        tempVect.y -= 10.0f;
        if (!(handControl.flags & 1) && handControl.actionTime < 4) {
            CreateDustCircle(&tempVect, 2, 0.0f);
        }
        if (ConditionsMet(HAND_JOINED)) {
            ApplyAction(HAND_JOINED);
        } else if (ConditionsMet(HAND_JUMP)) {
            ApplyAction(HAND_JUMP);
        } else if (hand.anim->finishedAnim == 62) {
            ApplyActionIdle();
        } else if (ConditionsMet(HAND_CLIMBDOWN)) {
            ApplyAction(HAND_CLIMBDOWN);
        }
        break;
    case HAND_BOUNCE:
        if (ball.state->waterState && (ball.flags & 0x200000) && ball.shadow->altitude > ball.obj->spheres[0].radius + 11.0) {
            SCALE_VECTOR(&ball.vel, 0.4);
            ApplyAction(HAND_BALLWALK);
        }
        if (hand.pos.y - ball.pos.y > 14.0) {
            hand.vel.y = hand.vel.y * 0.85 + ball.vel.y * 0.15;
        }
        dist = DistanceBetweenPoints(&hand.pos, &ball.pos);
        if (((controlMethod == CONTROL_BALL && handControl.bounceQueued)
             || (handControl.bounceQueued && !pad[ActiveController].button[BUTTON_B] && controlMethod != CONTROL_CAMERA))
            && !pad[ActiveController].button[BUTTON_A]) {
            if (dist < ball.obj->spheres[0].radius + levelHandScale * 7.0 / 0.05
                && ball.vel.y < levelHandScale / 0.05) {
                if (controlMethod != CONTROL_BALL) {
                    PlaySample(172, &hand.pos, 100, 128);
                }
                if (ball.obj->contactFlags & 2) {
                    ball.vel.y = levelHandScale * 84.0 / 0.05 / MIN(MAX(ball.phys->mass, 10.0f), 15.0);
                } else {
                    ball.vel.y = levelHandScale * -15.0f / 0.05;
                }
                handControl.bounceQueued = 0;
                ball.prevVel.y = ball.vel.y;
                if (handControl.prevAction == HAND_BOUNCE) {
                    if (ball.shadow->altitude > 30.0f) {
                        AnimateActor(&hand, HANDANIM_BOUNCE2, 0, 0, 1.2f);
                    } else {
                        AnimateActor(&hand, HANDANIM_BOUNCE2, 0, 0, 2.2f);
                    }
                }
            }
        }
        if ((pad[ActiveController].pressed[BUTTON_A] || pad[ActiveController].pressed[BUTTON_B]) && controlMethod != CONTROL_CAMERA) {
            handControl.bounceQueued = 1;
            if (hand.anim->finishedAnim && !handControl.lateBounceQueued) {
                handControl.lateBounceQueued = 1;
            }
        }
        break;
    case HAND_SLAM:
        if (handControl.actionTime < 3) {
            hand.vel.y = 0.0f;
        } else {
            hand.vel.y = -14.0f;
        }
        if (!(hand.flags & 0x2000)) {
            hand.vel.x /= 2.0f;
            hand.vel.z /= 2.0f;
        }
        break;
    case HAND_SLAM2ST:
        if (handControl.actionTime == 3) {
            AnimateActor(&hand, HANDANIM_SLAM2ST, 0, 0, 1.0f);
            AnimateActor(&hand, HANDANIM_STAND, 1, 1, 0.0f);
            handControl.actionLock = 6;
        }
        if (BallWalkAllowed()) {
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 11.0f);
        } else {
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 13.0f);
        }
        break;
    case HAND_JUMP:
        if (ball.link == NULL && hand.vel.y < 0.0f) {
            if (hand.pos.y - ball.pos.y < ball.obj->spheres[0].radius + hand.obj->spheres[0].radius * 3.0f
                && ball.obj->spheres[0].radius + hand.obj->spheres[0].radius < hand.pos.y - ball.pos.y) {
                if (hand.state->prevMode != HAND_BALLWALK
                    && DistanceBetweenPoints2D(&hand.pos, &ball.pos) < ball.obj->spheres[0].radius + 10.0f) {
                    SubVector2D(&tempVect, &hand.pos, &ball.pos);
                    SCALE_VECTOR_2D(&tempVect, 0.15);
                    SubVector2D(&hand.vel, &hand.vel, &tempVect);
                }
            }
        }
        if (handControl.actionTime >= 2 && ball.link == &hand) {
            ball.link = NULL;
        }
        if (handControl.actionTime > 0 && handControl.actionTime < 5 && hand.state->prevMode != HAND_SLAM
            && pad[ActiveController].button[BUTTON_A] && controlMethod == CONTROL_HAND) {
            hand.vel.y += levelHandScale * 1.2 / 0.05;
        }
        if (pad[ActiveController].pressed[BUTTON_A] && !hand.obj->onGround && handControl.actionTime >= 2) {
            if (controlMethod == CONTROL_HAND && !handControl.doubleJumped && hand.vel.y >= -13.0f) {
                PlaySample((Random(4) + 251), &hand.pos, 100, 128);
                AnimateActor(&hand, HANDANIM_LONGJUMP, 0, 0, 1.4f);
                tempFloat = LENGTH_2D(&hand.vel) / 4.0f;
                hand.vel.x = hand.vel.z = 0.0f;
                AccelerateActor2D(&hand, -tempFloat, hand.phys->maxSpeed);
                handControl.doubleJumped = 1;
                hand.vel.y = levelHandScale * 8.0 / 0.05;
            }
        }
        break;
    case HAND_BALLWALK:
        if (TriggerAtFrame(934.1f, &hand) || TriggerAtFrame(942.0f, &hand) || TriggerAtFrame(950.0f, &hand)
            || TriggerAtFrame(1079.1f, &hand) || TriggerAtFrame(1084.0f, &hand)
            || TriggerAtFrame(1088.0f, &hand)) {
            PlaySample(148, &hand.pos, 120, (Random(40) + 110));
        } else if (TriggerAtFrame(937.0f, &hand) || TriggerAtFrame(946.0f, &hand) || TriggerAtFrame(954.0f, &hand)
            || TriggerAtFrame(1082.0f, &hand) || TriggerAtFrame(1086.0f, &hand)
            || TriggerAtFrame(1090.0f, &hand)) {
            PlaySample(149, &hand.pos, 120, (Random(40) + 110));
        }
        ball.phys->moveSpeed = hand.phys->moveSpeed;
        if ((!(ball.flags & 0x200000) || !ball.state->waterState) && pad[ActiveController].pressed[BUTTON_SHOULDER_LEFT] && controlMethod == CONTROL_HAND) {
            ApplyAction(HAND_JOINED);
        } else if (pad[ActiveController].button[BUTTON_Z] && controlMethod == CONTROL_HAND) {
            DismountBall();
        } else {
            if (!pad[ActiveController].button[BUTTON_A] && controlMethod == CONTROL_HAND) {
                handControl.bounceCount = 0;
            }
            if (!handControl.actionLock && (!(ball.flags & 0x200000) || !ball.state->waterState) && handControl.actionTime >= 2
                && (hand.pos.y - ball.pos.y > ball.radius + hand.obj->spheres[0].radius + 1.0f
                    || gravity * 3.0f < ball.vel.y)) {
                ApplyAction(HAND_WALK);
                ReleaseBall();
                ball.link = NULL;
                hand.vel.y = ball.vel.y;
            }
            ActorStartRoll(&ball, ball.vel.x, ball.vel.z);
            if (LENGTH_2D(&ball.vel) > 0.1) {
                hand.rotAim.y = Aabs(atan2(ball.vel.x, ball.vel.z) + 3.141592654);
            }
            hand.obj->onGround = 1;
            dist = DistanceBetweenPoints2D(&ball.pos, &ball.prevPos);
            prevAnim = handControl.gait;
            if (dist < 0.1) {
                if (prevAnim != -1) {
                    AnimateActor(&hand, HANDANIM_BALLWALK2IDLE, 0, 0, 0.0f);
                    AnimateActor(&hand, HANDANIM_STAND, 1, 1, 0.0f);
                }
                handControl.gait = -1;
            } else {
                startRoll = prevAnim != 3;
                handControl.gait = HANDWALK_ONBALL;
                if (startRoll) {
                    if (hand.anim->current == HANDANIM_STAND) {
                        AnimateActor(&hand, HANDANIM_BALLWALK2IDLE, 0, 0, -0.6f);
                        AnimateActor(&hand, HANDANIM_BALLWALK, 1, 1, 0.0f);
                    } else {
                        AnimateActor(&hand, HANDANIM_BALLWALK, 1, 0, 0.0f);
                    }
                }
                if (hand.anim->current != HANDANIM_BALLWALK2IDLE) {
                    if (dist > handballwalkspeed && hand.anim->current != HANDANIM_BALLWALK2) {
                        AnimateActor(&hand, HANDANIM_BALLWALK2, 1, 0, 0.0f);
                    } else if (dist <= handballwalkspeed && hand.anim->current != HANDANIM_BALLWALK) {
                        AnimateActor(&hand, HANDANIM_BALLWALK, 1, 0, 0.0f);
                    }
                    if (hand.anim->current == HANDANIM_BALLWALK) {
                        SetAnimationSpeed(&hand, dist / ballwalkfac);
                    } else {
                        SetAnimationSpeed(&hand, dist / (ballwalkfac * 3.0f));
                    }
                }
            }
        }
        break;
    case HAND_CARTWHEEL:
        if (handControl.actionTime == 3) {
            cartwheelAccel = 3.5f;
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 6.5f);
        }
        if (handControl.actionTime >= 4 && handControl.actionTime < 22) {
            hand.phys->maxSpeed = 14.0f;
            AccelerateActor2D(&hand, cartwheelAccel, hand.phys->maxSpeed);
            hand.phys->maxSpeed = 35.0f;
            cartwheelAccel *= 0.95;
        }
        if (handControl.actionTime == 22) {
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 13.0f);
        }
        if (handControl.actionTime == 4) {
            hand.vel.y += 6.0;
            PlaySample((Random(2) + 255), &hand.pos, 100, 138);
        }
        if (hand.anim->finishedAnim && hand.obj->onGround) {
            hand.rot.y = hand.rotAim.y = hand.rot.y;
        }
        break;
    case HAND_CLIMBUP:
        if (handControl.actionTime == 10) {
            hand.rotAim.y = Aabs(hand.rot.y + 3.141592654);
        }
        if (handControl.actionTime < 11) {
            AccelerateActor2D(&hand, -0.2f, hand.phys->maxSpeed);
        } else {
            AccelerateActor2D(&hand, 0.2f, hand.phys->maxSpeed);
        }
        if (hand.anim->finishedAnim) {
            ApplyActionIdle();
        }
        break;
    case HAND_CLIMBDOWN:
        hand.vel = zero;
        ZERO_VECTOR(&tempVect);
        UpdateActorPlatform(&sepVect, &tempVect, &zero, NULL, NULL, NULL, 0, 32, handControl.climbPlatform, handControl.climbPlatform);
        AddScaledVector2D(&hand.pos, &hand.pos, 0.5f, &sepVect, 0.5f);
        if (handControl.actionTime > 20 && hand.anim->finishedAnim == 19 && Random(200) < 10) {
            AnimateActor(&hand, HANDANIM_SITIDLE, 0, 0, 0.0f);
        }
        if (handControl.clingPlanes[0]->y <= 0.92) {
            ApplyAction(HAND_DROPDOWN);
        }
        hand.obj->onGround = 1;
        break;
    case HAND_CLINGUP:
        if (TriggerAtFrame(825.0f, &hand)) {
            PlaySample(154, &hand.pos, 90, 128);
        }
        ZERO_VECTOR(&tempVect);
        UpdateActorPlatform(&sepVect, &tempVect, &zero, NULL, NULL, NULL, 0, 32, handControl.edgePlatform, handControl.edgePlatform);
        SUB_VECTORS(&tempVect, &sepVect, &hand.pos);
        AddToOneScaledVector(&hand.pos, &tempVect, 0.4f);
        if (hand.anim->finishedAnim == 35) {
            ApplyActionIdle();
        }
        break;
    case HAND_CLING:
        ZERO_VECTOR(&tempVect);
        UpdateActorPlatform(&sepVect, &tempVect, &zero, NULL, NULL, NULL, 0, 32, handControl.edgePlatform, handControl.edgePlatform);
        hand.vel.y = 0.0f;
        handControl.frontProbe.pos.y -= handControl.edgeAltitude;
        if (handControl.actionTime < 11) {
            SubVector2D(&tempVect, &sepVect, &hand.pos);
            SCALE_VECTOR_2D(&tempVect, clingscale);
            tempVect.y = sepVect.y + clingy - hand.pos.y;
            AddToOneScaledVector(&hand.pos, &tempVect, 0.4f);
        }
        break;
    case HAND_THROW2LAND:
        if (hand.anim->finishedAnim) {
            ApplyActionIdle();
        }
        break;
    case HAND_DROPDOWN:
        if (handControl.actionTime >= 4) {
            if (hand.obj->spheres[0].targetRadius != hand.obj->spheres[0].radius) {
                hand.obj->spheres[0].radius +=
                    MIN(MAX(-hand.phys->radiusRate,
                            (hand.obj->spheres[0].targetRadius - hand.obj->spheres[0].radius) / 4.0f),
                        hand.phys->radiusRate);
            }
            if (hand.state->prevMode == HAND_CLIMBDOWN && handControl.actionTime < 6) {
                AccelerateActor2D(&hand, 1.0f, hand.phys->maxSpeed);
            }
        }
        if (handControl.actionTime == 3) {
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 13.0f);
            hand.flags |= 1;
            if (hand.state->prevMode == HAND_CLIMBDOWN) {
                AccelerateActor2D(&hand, 4.0f, hand.phys->maxSpeed);
            } else {
                AccelerateActor2D(&hand, -4.0f, hand.phys->maxSpeed);
            }
        }
        break;
    case HAND_LOB:
        if (handControl.actionTime < 5) {
            ball.pos.y += handlob;
            ZERO_VECTOR(&ball.vel);
            ball.flags &= ~1;
            SCALE_VECTOR(&hand.vel, 0.9);
        } else if (handControl.actionTime == 5) {
            ball.flags |= 1;
            PlaySample(168, &hand.pos, 100, 128);
            ball.rot.y = ball.rotAim.y = hand.rot.y;
            ReleaseBall();
            ball.vel.y = hand.phys->mass * 13.5 / MAX(ball.phys->mass, 5.0f);
            ball.vel.x = ball.vel.z = 0.0f;
            ball.obj->planes[0] = NULL;
            AccelerateActor2D(&ball, hand.phys->mass * 3.0 / ball.phys->mass, ball.phys->maxSpeed);
            ActorStartRoll(&ball, ball.vel.x, ball.vel.z);
            hand.vel = zero;
            pad[ActiveController].button[BUTTON_Z] = 0;
            pad[ActiveController].sincePress[BUTTON_Z] = 1000;
            handControl.joinTimer = 10;
            if (player.powerups[BOOMERANGBALL]) {
                ballControl.returnTime = 1;
            }
        } else if (ConditionsMet(HAND_WALK)) {
            ApplyAction(HAND_WALK);
        }
        break;
    case HAND_CATCH:
        if (player.powerups[VANISHBALL] || (player.powerups[BOOMERANGBALL] && ballControl.returnTime >= 50)) {
            SubVector2D(&tempVect, &ball.pos, &hand.pos);
            if (LENGTH_2D(&tempVect) > 10.0f) {
                hand.rotAim.y = Aabs(atan2(tempVect.x, tempVect.z));
            }
        }
        if (hand.rot.x > 5.9 || hand.rot.x < 4.0f) {
            hand.rot.x = hand.rotAim.x = Aabs(hand.rot.x - 0.24);
        }
        if (ABS(ball.pos.y - hand.pos.y) < 15.0f
            && DistanceBetweenPoints2D(&ball.pos, &hand.pos)
                   < ball.obj->spheres[0].radius + hand.obj->spheres[0].radius - 18.0f) {
            PlaySample(171, &hand.pos, 70, 138);
            hand.state->mode = HAND_THROWAIM;
            hand.state->prevMode = HAND_CATCH;
            handControl.actionTime = 0;
            SCALE_VECTOR(&ball.vel, 0.0f);
        } else if (!HandCatchAllowed()) {
            ball.link = NULL;
            ApplyActionIdle();
        } else {
            temp = ball.pos.y - hand.pos.y;
            if (temp < 60.0f) {
                SUB_VECTORS(&tempVect, &hand.pos, &ball.pos);
                if (tempVect.x * tempVect.x + tempVect.z * tempVect.z < 250.0f) {
                    scale = (60.0f - temp) / 120.0f;
                    SCALE_VECTOR(&tempVect, scale);
                    tempVect.y *= 2.0f;
                    ADD_TO_VECTOR(&ball.pos, &tempVect);
                    tempVect.y = 0.0f;
                    SUB_FROM_VECTOR(&hand.pos, &tempVect);
                    if (handControl.checkpoint->actor.scale.x <= 0.001) {
                        cameraInfo.followTimer = 2;
                        cameraInfo.followSpeed = tempVect.y;
                    }
                    tempVect.z = hand.pos.z - ball.pos.z;
                    tempVect.x = hand.pos.x - ball.pos.x;
                    tempVect.y = hand.pos.y - ball.pos.y;
                    dist = LENGTH(&tempVect);
                    if (dist < ball.speed) {
                        ratio = dist / ball.speed;
                        SCALE_VECTOR(&ball.vel, ratio);
                    }
                }
            }
        }
        break;
    case HAND_CASTSPELL:
        if (player.castSpell != 15) {
            SCALE_VECTOR(&hand.vel, 0.6);
        }
        if (hand.state->prevMode == HAND_JUMP && handControl.actionTime < 30) {
            hand.vel.y = 0.0f;
        }
        switch (player.castSpell) {
        case -1:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
            if (handControl.actionTime == 8) {
                PlaySample(176, &hand.pos, 100, 128);
                FireBulletFromActor(&hand, BALL_SPELL_BULLET, 1);
                hand.rotAim.y = hand.rot.y;
            }
            break;
        case 0:
        case 1:
        case 2:
            if (handControl.actionTime == 10) {
                PlaySample(176, &hand.pos, 100, 128);
                FireBulletFromActor(&hand, NME_SPELL_BULLET, 1);
                hand.rotAim.y = hand.rot.y;
            }
            break;
        case 3:
        case 4:
        case 5:
        case 6:
            if (TriggerAtFrame(561.0f, &hand)) {
                PlaySample(176, &hand.pos, 100, 128);
                FireBulletFromActor(&hand, HAND_SPELL_BULLET, 1);
                hand.rotAim.y = hand.rot.y;
            }
            break;
        }
        break;
    case HAND_ROTOR:
        handControl.samples[1].volume = MIN(handControl.rotorSpeed * 60.0f + 60.0f, 220);
        handControl.samples[1].pitch = MIN(MAX(10, handControl.rotorSpeed * 200.0f - 50.0f), 200);
        handControl.rotorSpeed *= 0.95;
        if (pad[ActiveController].button[BUTTON_A] && controlMethod == CONTROL_HAND) {
            handControl.rotorSpeed = MIN(gravity * 1.001, handControl.rotorSpeed + gravity * 0.06);
        }
        handControl.rotorSpeed = MAX(gravity * 0.25, handControl.rotorSpeed);
        if (controlMethod == CONTROL_HAND) {
            if (pad[ActiveController].button[BUTTON_B]) {
                hand.rotAim.x = Aabs(MIN(pad[ActiveController].mag / 300.0f, 0.6) + 3.141592654);
                SCALE_VECTOR_2D(&hand.vel, 0.85);
                hand.phys->moveSpeed = 2.3333333f;
                hand.flags &= ~1;
                hand.vel.y *= 0.98;
                handControl.rotorSpeed = handControl.rotorSpeed * 0.8 + gravity * 0.8 * 0.2;
            } else {
                hand.rotAim.x = Aabs(MIN(pad[ActiveController].mag / 90.0f, 0.7) + 3.141592654);
                hand.flags |= 1;
                if (!pad[ActiveController].prevButton[BUTTON_B]) {
                    hand.vel.y += handControl.rotorSpeed * (1.0 - Aabs(hand.rot.x - 3.141592654) / 1.8);
                }
            }
        } else {
            hand.rotAim.x = 3.1415927f;
        }
        if (!ballControl.releaseTimer && ball.link == NULL && (ball.flags & 0x40000000)) {
            ball.link = &hand;
            handControl.releaseTimer = 0;
            handControl.joinTimer = joinVal - 1;
        }
        holder = ball.link;
        if (holder == &hand && !ballControl.releaseTimer) {
            RotateBallWithHand();
            maxUpSpeed = gravity * 5.0f * (hand.phys->mass / (hand.phys->mass + ball.phys->mass * 3.0f / 4.0f));
            hand.phys->moveSpeed = maxUpSpeed;
            maxDownSpeed = -gravity * 6.0f * ((hand.phys->mass + ball.phys->mass / 2.0f) / hand.phys->mass);
            tempVect.x = 0.0f;
            tempVect.z = 0.0f;
            if (ball.obj->contactFlags) {
                ball.flags |= 1;
                ball.vel = hand.vel;
                tempVect.y = ball.obj->spheres[0].radius + -9.0f;
                maxDownSpeed = -gravity;
                RotateVector2DXYZ(&tempVect, &tempVect, Aabs(hand.rot.x - 3.141592654), 0);
                RotateVector2D(&tempVect, &tempVect, hand.rot.y);
                ADD_VECTORS(&tempVect2, &ball.pos, &tempVect);
                AddScaledVector(&holder->pos, &holder->pos, (f32) handControl.joinTimer / joinVal, &tempVect2, 1.0f - (f32) handControl.joinTimer / joinVal);
            } else {
                ball.flags &= ~1;
                ball.vel = hand.vel;
                tempVect.y = -(ball.obj->spheres[0].radius + -9.0f);
                RotateVector2DXYZ(&tempVect, &tempVect, Aabs(hand.rot.x - 3.141592654), 0);
                RotateVector2D(&tempVect, &tempVect, hand.rot.y);
                ADD_VECTORS(&tempVect2, &hand.pos, &tempVect);
                AddScaledVector(&ball.pos, &ball.pos, (f32) handControl.joinTimer / joinVal, &tempVect2, 1.0f - (f32) handControl.joinTimer / joinVal);
            }
            if (pad[ActiveController].button[BUTTON_Z] && !ball.obj->contactFlags) {
                ballControl.releaseTimer = 6;
                cameraInfo.focus = &hand.pos;
                cameraInfo.focusVel = &hand.vel;
                ball.flags |= 1;
            }
        } else {
            maxUpSpeed = gravity * 6.0f;
            maxDownSpeed = -gravity * 5.0f;
        }
        if (hand.vel.y < maxDownSpeed) {
            hand.vel.y = maxDownSpeed;
        } else if (hand.vel.y > maxUpSpeed) {
            hand.vel.y = maxUpSpeed;
        }
        break;
    case HAND_SNATCH:
        for (x = 0; x < 4; x++) {
            plane = hand.obj->planes[x];
            if (plane != NULL && (DOT_PRODUCT(&hand.pos, plane) + plane->d) * (DOT_PRODUCT(&ball.pos, plane) + plane->d) < 0.0f) {
                ApplyActionIdle();
                if (ball.link == &hand) {
                    ball.link = NULL;
                }
                break;
            }
        }
        if (hand.state->mode == HAND_SNATCH) {
            ActorHandleRotaim(&hand, 1);
            hand.obj->spheres[0].targetRadius = ball.obj->spheres[0].targetRadius + 0.05;
            UpdateActorPlatform(&sepVect, &tempVect, &zero, NULL, NULL, NULL, 0, 32, handControl.climbPlatform, handControl.climbPlatform);
            if (hand.anim->finishedAnim) {
                ball.pos = sepVect;
                ball.pos.y += ball.radius - hand.scale.x * 13.0 / 0.05;
                ApplyAction(HAND_JOINED);
            } else {
                SUB_VECTORS(&tempVect, &camPos2, &sepVect);
                if (hand.anim->anims[HANDANIM_SNATCH].start + (hand.anim->anims[HANDANIM_SNATCH].end - hand.anim->anims[HANDANIM_SNATCH].start) / 2
                    < hand.anim->time) {
                    AddToOneScaledVector(&hand.pos, &tempVect, -0.15f);
                    hand.pos.y = (ball.pos.y + hand.pos.y) / 2.0f;
                    if (ball.radius > 9.0) {
                        hand.pos.y += ball.radius - 9.0;
                    }
                    SetVector2D(&ball.pos, &hand.pos);
                } else {
                    AddToOneScaledVector(&hand.pos, &tempVect, 0.4 / handControl.actionTime);
                }
                camPos.y = hand.pos.y;
            }
        }
        break;
    case HAND_SLIDING:
        if (ConditionsMet(HAND_CLING)) {
            ApplyAction(HAND_CLING);
        } else if (handControl.steepSlide != 1) {
            if (handControl.actionTime == 4) {
                SetAnimationSpeed(&hand, 0.0f);
            }
            if (LENGTH_2D(&hand.vel) > 0.1) {
                if (hand.obj->planes[0] != NULL && !hand.obj->onGround) {
                    hand.rotAim.y = Aabs(atan2(hand.obj->planes[0]->x, hand.obj->planes[0]->z));
                    if (hand.speed != 0.0f && hand.speed < 5.0f && FindShortestAngle(hand.rot.y, hand.rotAim.y) < 0.1) {
                        actor = &hand;
                        SCALE_VECTOR(&actor->vel, (actor->speed + 1.5) / actor->speed);
                    }
                } else {
                    hand.rotAim.y = Aabs(atan2(hand.vel.x, hand.vel.z));
                }
                if (hand.obj->planes[0] != NULL) {
                    AddOneScaledVector(&tempVect, &hand.pos, (Vec3 *) hand.obj->planes[0], -hand.radius * 0.88);
                    tempVect.x += Random(7) - 3;
                    tempVect.z += Random(7) - 3;
                    tempVect2.x = hand.vel.x / 1.4;
                    tempVect2.y = 2.0f;
                    tempVect2.z = hand.vel.z / 1.4;
                    AddDebris(DUST, &tempVect, &tempVect2, NULL, NULL);
                }
            }
        }
        break;
    case HAND_HOPPING:
        hand.link->rotAim = hand.rotAim;
        ActorHandleRotaim(hand.link, 1);
        enemies[hand.link->state->index].brain.delayTimer = 10;
        if (!pad[ActiveController].button[BUTTON_A] && controlMethod == CONTROL_HAND) {
            handControl.bounceCount = 0;
        }
        SetHandHopPosition();
        if (pad[ActiveController].button[BUTTON_A] && controlMethod == CONTROL_HAND && hand.link->obj->onGround && !handControl.actionDelay) {
            if (handControl.actionTime >= 6) {
                if (!handControl.bounceCount) {
                    AnimateActor(hand.link, 10, 0, 0, 0.0f);
                    AnimateActor(hand.link, 7, 0, 1, 0.0f);
                    handControl.actionDelay = 5;
                } else {
                    handControl.actionDelay = 1;
                    AnimateActor(hand.link, 7, 0, 0, 0.0f);
                    PlaySample(19, &hand.pos, 100, 128);
                }
            }
        }
        if (handControl.actionDelay) {
            if (!--handControl.actionDelay) {
                PlaySample(19, &hand.pos, 100, 128);
                if (handControl.bounceCount < 15) {
                    handControl.bounceCount += 4;
                }
                ZERO_VECTOR(&hand.link->vel);
                hand.link->vel.y = handControl.bounceCount + 8;
            }
        }
        if (pad[ActiveController].pressed[BUTTON_Z] && controlMethod == CONTROL_HAND) {
            DismountEnemy();
        }
        break;
    case HAND_FROG_TONGUE:
        if (handControl.actionTime <= tongueTime) {
            tongueOffsetPos[handControl.actionTime].x = tongueBullet->actor.pos.x - hand.pos.x;
            tongueOffsetPos[handControl.actionTime].y = tongueBullet->actor.pos.y - hand.pos.y;
            tongueOffsetPos[handControl.actionTime].z = tongueBullet->actor.pos.z - hand.pos.z;
            SUB_VECTORS(&tongueOffsetPos[handControl.actionTime + 1], &tongueBullet->actor.pos, &hand.pos);
            y = handControl.actionTime;
        } else if (tongueBullet != NULL) {
            tongueBullet->target = NULL;
            ZERO_VECTOR(&tongueBullet->actor.vel);
            y = 2.0f * tongueTime - handControl.actionTime + 1.0f;
            if (y >= 0) {
                ADD_VECTORS(&tongueBullet->actor.pos, &tongueOffsetPos[y], &hand.pos);
                tongueBullet->sprites[0].pos = tongueBullet->actor.pos;
            }
        }
        for (x = 0; x <= y; x++) {
            if (x < y) {
                for (j = 0; j < numTongueSegs; j++) {
                    AddScaledVector(&tempVect, &tongueOffsetPos[x], j / numTongueSegs, &tongueOffsetPos[x + 1], 1.0f - j / numTongueSegs);
                    AddVectorInline(&tempVect, &tempVect, &hand.pos);
                    AddTongueBit(&tempVect);
                }
            } else {
                AddVectorInline(&tempVect, &tongueOffsetPos[x], &hand.pos);
                AddTongueBit(&tempVect);
            }
        }
        if (tongueBullet != NULL && tongueBullet->next != NULL) {
            for (x = 0; x < numberOfEnemies; x++) {
                if (!tongueStuck && enemies[x].actor.active == 1 && enemies[x].actor.radius <= tongueRad[0]
                    && DistanceBetweenPointsSquared(&enemies[x].actor.pos, &tongueBullet->actor.pos)
                    < (enemies[x].actor.radius + 10.0f) * (enemies[x].actor.radius + 10.0f)) {
                    enemies[x].actor.active = 10;
                    enemies[x].brain.flags |= 0x400;
                    tongueBullet->target = NULL;
                    if (handControl.actionTime <= tongueTime) {
                        tongueTime = handControl.actionTime;
                    }
                    tongueStuck = 1;
                }
                if (enemies[x].brain.flags & 0x400) {
                    AddScaledVector(&enemies[x].actor.pos, &tongueBullet->actor.pos, handControl.actionTime / (2.0f * tongueTime),
                                    &enemies[x].actor.pos, 1.0f - handControl.actionTime / (2.0f * tongueTime));
                }
            }
            for (garibGroup = garibGroupList.head.prev; garibGroup != &garibGroupList.head; garibGroup = garibGroup->prev) {
                for (garib = garibGroup->garibs.head.prev; garib != &garibGroup->garibs.head; garib = garib->prev) {
                    if (!tongueStuck && garib->active && garib->mode != 10) {
                        if (DistanceBetweenPointsSquared(&garib->sprite.pos, &tongueBullet->actor.pos) < 256.0f) {
                            garib->mode = 10;
                            if (handControl.actionTime <= tongueTime) {
                                tongueTime = handControl.actionTime;
                            }
                            tongueStuck = 1;
                        }
                    }
                    if (garib->mode == 10) {
                        AddScaledVector(&garib->sprite.pos, &tongueBullet->actor.pos, handControl.actionTime / (2.0f * tongueTime),
                                        &garib->sprite.pos, 1.0f - handControl.actionTime / (2.0f * tongueTime));
                        ZERO_VECTOR(&garibGroup->pos);
                        for (garib2 = garibGroup->garibs.head.prev; garib2 != &garibGroup->garibs.head; garib2 = garib2->prev) {
                            ADD_TO_VECTOR(&garibGroup->pos, &garib2->sprite.pos);
                        }
                        SCALE_VECTOR(&garibGroup->pos, 1.0f / garibGroup->garibs.count);
                        for (garib2 = garibGroup->garibs.head.prev; garib2 != &garibGroup->garibs.head; garib2 = garib2->prev) {
                            garibDist = DistanceBetweenPoints(&garibGroup->pos, &garib2->sprite.pos);
                            if (garibGroup->radius < garibDist) {
                                garibGroup->radius = garibDist;
                            }
                        }
                        garibGroup->radius += 30.0f;
                    }
                }
            }
            if (!tongueStuck && tongueBullet->actor.obj != NULL && tongueBullet->actor.obj->contactFlags) {
                tongueBullet->target = NULL;
                ZERO_VECTOR(&tongueBullet->actor.vel);
                if (handControl.actionTime <= tongueTime) {
                    tongueTime = handControl.actionTime;
                }
                tongueStuck = 1;
            }
        }
        if (2.0f * tongueTime + 1.0f <= handControl.actionTime) {
            for (x = 0; x < numberOfEnemies; x++) {
                if (enemies[x].brain.flags & 0x400) {
                    KillEnemy(&enemies[x]);
                    GainExtraLife();
                    PlaySample(111, &hand.pos, 100, 160);
                    SCALE_VECTOR(&hand.scale, 1.4);
                }
            }
        }
        if (2.0f * tongueTime + 2.0f <= handControl.actionTime) {
            bullet = tongueBullet;
            tongueBullet = NULL;
            bullet->actor.lifeTimer = 1;
            ApplyAction(HAND_FROG_IDLE);
        }
        break;
    case HAND_FROG_IDLE:
        if (TriggerAtFrame(35.0f, &hand) && Random(50) < 5) {
            PlaySample(218, &hand.pos, 100, 118);
        }
        if (hand.obj->onGround && !hand.obj->wasOnGround) {
            SCALE_VECTOR_2D(&hand.vel, 0.25);
        }
        break;
    case HAND_FROG_JUMP:
        hand.friction = 0.9f;
        if ((hand.obj->onGround || (hand.anim->finishedAnim == 8 && hand.obj->planes[0] != NULL && hand.obj->planes[0]->y > 0.0f))
            && hand.anim->queue[0] == -1 && hand.anim->current == HANDANIM_GETUP
            && hand.anim->anims[HANDANIM_GETUP].start + 5 < hand.anim->time) {
            PlaySample(218, &hand.pos, 100, 118);
            AnimateActor(&hand, HANDANIM_SNATCH, 0, 1, 0.0f);
        }
        break;
    case HAND_FROG_WALK:
        hand.friction = 0.9f;
        if (handControl.bounceCount) {
            handControl.bounceCount++;
        }
        if (hand.obj->onGround) {
            if (!hand.obj->wasOnGround) {
                SCALE_VECTOR_2D(&hand.vel, 0.25);
            }
            if (TriggerAtFrame(5.0f, &hand)) {
                PlaySample(218, &hand.pos, 50, 118);
                hand.vel.y = 3.0f;
                AccelerateActor(&hand, 4.0f, hand.phys->moveSpeed);
                handControl.bounceCount = 1;
            }
        }
        if (hand.anim->finishedAnim) {
            AnimateActor(&hand, HANDANIM_DENIS, 0, 0, -hand.anim->speed);
        }
        break;
    case HAND_SWISH:
        HandSwish(handControl.attachedTo->inst->obj->child, handControl.attachedTo->anim->time,
                  handControl.attachedTo->anim->anims[handControl.attachedTo->anim->current].start,
                  handControl.attachedTo->anim->anims[handControl.attachedTo->anim->current].end, handControl.attachedTo->anim->speed);
        tempVect.x = 0.0f;
        tempVect.y = 9.0f;
        tempVect.z = LENGTH_2D(&handControl.attachedTo->vel) - 8.0f;
        GetRotationFromQuaternion(&q, &hand.rotQuat);
        RotateVectorByRotation(&handControl.attachOffset, &tempVect, &q);
        COPY_VECTOR(&hand.pos, &handControl.attachedTo->obj->spheres[handControl.attachTag].pos);
        RotateVectorByQuaternion(&tempVect, &handControl.attachOffset, &handControl.attachedTo->rotQuat);
        ADD_TO_VECTOR(&hand.pos, &tempVect);
        handControl.attachedTo->rotAim.y = hand.rotAim.y;
        hand.rotAim.y = hand.rot.y = handControl.attachedTo->rot.y;
        swishSpeed = MIN(MAX(pad[ActiveController].mag / 24.0f, 0.0f), swishMaxSpeed - swishMinSpeed) + swishMinSpeed;
        if (handControl.attachedTo->anim->current == 15) {
            handControl.attachedTo->anim->speed = handControl.attachedTo->anim->anims[15].speed * swishSpeed;
            handControl.attachedTo->flags &= ~0x01000000;
            AccelerateActor2D(handControl.attachedTo, handControl.attachedTo->phys->accel, handControl.attachedTo->phys->moveSpeed * swishSpeed);
            handControl.attachedTo->flags |= 0x01000000;
        } else {
            hand.flags &= ~8;
            if (handControl.attachedTo->anim->time >= 131.0f && handControl.attachedTo->anim->time <= 172.0f) {
                SUB_VECTORS(&tempVect, &handControl.attachedTo->obj->spheres[1].pos, &handControl.attachedTo->pos);
                MakeUnit(&tempVect);
                AddOneScaledVector(&tempVect2, &handControl.attachedTo->pos, &tempVect, 13.0f);
                AddOneScaledVector(&tempVect, &handControl.attachedTo->obj->spheres[1].pos, &tempVect, 13.0f);
                if (handControl.attachedTo->anim->time < handControl.attachedTo->anim->anims[handControl.attachedTo->anim->current].speed + 131.0f) {
                    if (blur2 != NULL) {
                        if (blur2->next != NULL) {
                            SubBlur(blur2);
                        }
                    } else {
                        blur2 = JallocAlloc(sizeof(Blur), 1, "Blur");
                    }
                    AddBlur(blur2);
                    StartBlurFX(blur2, &tempVect, &tempVect2, 255, 0, 30, 30);
                }
                IncrementBlurFX(blur2, &tempVect, &tempVect2);
            }
            for (x = 0; x < numberOfEnemies; x++) {
                if (enemies[x].actor.active == 1 && &enemies[x].actor != handControl.attachedTo
                    && DistanceBetweenPoints(&enemies[x].actor.pos, &handControl.attachedTo->obj->spheres[1].pos)
                    <= enemies[x].actor.radius + handControl.attachedTo->obj->spheres[1].radius + 5.0f) {
                    KillEnemy(&enemies[x]);
                }
            }
        }
        vertQ.w = hand.rot.y;
        GetQuaternionFromRotation(&q, &vertQ);
        QuaternionMultiply(&hand.rotQuat, &q, &hand.rotQuat);
        if (pad[ActiveController].pressed[BUTTON_A] && handControl.attachedTo->obj->onGround) {
            AnimateActor(handControl.attachedTo, 0, 0, 0, 0.0f);
            AnimateActor(handControl.attachedTo, 15, 1, 1, handControl.attachedTo->anim->anims[15].speed * swishSpeed);
        }
        if (pad[ActiveController].pressed[BUTTON_Z] && controlMethod == CONTROL_HAND) {
            DismountEnemy();
        }
        break;
    case HAND_NOTHING:
        CheckForFootsteps();
        if (hand.anim->current == HANDANIM_OKAY || hand.anim->current == HANDANIM_THUMBSUP) {
            ZERO_VECTOR(&hand.vel);
            if (hand.anim->time > hand.anim->anims[hand.anim->current].start
                + (hand.anim->anims[hand.anim->current].end - hand.anim->anims[hand.anim->current].start) * 0.6) {
                AnimateActor(&hand, HANDANIM_STAND, 1, 1, 0.0f);
                hand.state->mode = HAND_IDLE;
            }
        }
        break;
    case HAND_LOCATE:
        tempVect.x = -12.0f;
        tempVect.z = 0.0f;
        RotateVector2D(&tempVect, &tempVect, camera.rot.y);
        for (x = 0; x < 2; x++) {
            if (handControl.locate[x] != NULL) {
                AddVector2D(&handControl.locate[x]->pos, &hand.pos, &tempVect);
                handControl.locate[x]->pos.y = hand.pos.y + 20.0f;
            }
        }
        if (hand.anim->current == HANDANIM_POINT || hand.anim->current == HANDANIM_POINTCYCLE) {
            for (x = 0; x < 2; x++) {
                sprite = handControl.locate[x];
                if (sprite != NULL) {
                    sprite->scaleX = sprite->scaleY = sprite->anim.def.startScale = sprite->anim.def.endScale =
                        sprite->anim.def.endScale + ((1 - x) * 40 + 48 - sprite->scaleX) / 2;
                }
            }
            if (!handControl.locateGaribs && ballControl.loseTimer) {
                handControl.locate[1]->scaleY = handControl.locate[1]->scaleX =
                    MAX(0, handControl.locate[1]->scaleX + SineWave(1.0f, frameCount, 0) * 16.0f);
            }
            if (TriggerAtFrame(776.0f, &hand)) {
                PlaySample(178 - handControl.locateGaribs, &hand.pos, 100, 128);
            }
            if (hand.state->prevMode == HAND_JUMP || hand.state->prevMode == HAND_CASTSPELL) {
                hand.vel.y *= 0.5;
            }
            if ((!handControl.locateGaribs && !pad[ActiveController].button[BUTTON_B] && ballControl.state != BALL_BURST)
                || (handControl.locateGaribs == 1 && !pad[ActiveController].button[BUTTON_SHOULDER_RIGHT])) {
                if (hand.anim->current == HANDANIM_POINTCYCLE) {
                    hand.anim->loop = 0;
                    AnimateActor(&hand, HANDANIM_POINT2IDLE, 0, 1, 1.0f);
                    AnimateActor(&hand, HANDANIM_STAND, 1, 1, 0.0f);
                }
            }
            if (handControl.locateGaribs == 1) {
                dist = 1e7f;
                for (garibGroup = garibGroupList.head.prev; garibGroup != &garibGroupList.head; garibGroup = garibGroup->prev) {
                    for (garib = garibGroup->garibs.head.prev; garib != &garibGroup->garibs.head; garib = garib->prev) {
                        if (garib->active && garib->type != EXTRA_LIFE) {
                            val = DistanceBetweenPoints(&garib->sprite.pos, &hand.pos);
                            if (val < dist) {
                                tempVect = garib->sprite.pos;
                                dist = val;
                            }
                        }
                    }
                }
                for (x = 0; x < numberOfEnemies; x++) {
                    if ((enemies[x].brain.flags & 0x40) && !(enemies[x].brain.flags & 0x80)) {
                        val = DistanceBetweenPoints(&enemies[x].actor.pos, &hand.pos);
                        if (val < dist) {
                            tempVect = enemies[x].actor.pos;
                            dist = val;
                        }
                    }
                }
                if (dist < 1e7f) {
                    SUB_FROM_VECTOR(&tempVect, &hand.pos);
                } else {
                    tempVect.x = Random(20000) - 10000;
                    tempVect.y = Random(20000) - 10000;
                    tempVect.z = Random(20000) - 10000;
                }
            } else if (ballStatic < 2) {
                SUB_VECTORS(&tempVect, &ball.pos, &hand.pos);
                if (!(cheatActivated & 0x80) && pad[ActiveController].pressed[BUTTON_SHOULDER_RIGHT] && ballControl.snow <= 0.9
                    && (ballControl.enemy == NULL || ballControl.enemy->type == BUGLE || ballControl.enemy->type == CHUCK)) {
                    player.castSpell = nextBallState[ballControl.type];
                    player.castSpellType = nextBallState[ballControl.type];
                    handControl.castSpellTime = -1;
                    ApplyAction(HAND_CASTSPELL);
                    break;
                }
            } else {
                for (platform = platformList.head.next; platform != &platformList.head; platform = platform->next) {
                    if (ballLeftBehind == 8 && platform->exitType == 12) {
                        SUB_VECTORS(&tempVect, &platform->actor.pos, &hand.pos);
                        break;
                    }
                    if (ballLeftBehind == platform->exitType + 36) {
                        SUB_VECTORS(&tempVect, &platform->actor.pos, &hand.pos);
                        break;
                    }
                    if (gameInfo.level == CAVE) {
                        if (platform->exitType != 0) {
                            SUB_VECTORS(&tempVect, &platform->actor.pos, &hand.pos);
                        }
                    } else if (gameInfo.level == WAYROOM) {
                        if (ballLeftBehind >= 1 && ballLeftBehind <= 6 && platform->exitType == 13) {
                            SUB_VECTORS(&tempVect, &platform->actor.pos, &hand.pos);
                            break;
                        }
                        if (!((saveSlot[currentSaveSlot].levels >> ((gameInfo.world - 1) * 5)) & 1)) {
                            if (platform->exitType == 16) {
                                SUB_VECTORS(&tempVect, &platform->actor.pos, &hand.pos);
                            }
                        } else if (!((saveSlot[currentSaveSlot].levels >> ((gameInfo.world - 1) * 5 + 1)) & 1)) {
                            if (platform->exitType == 17) {
                                SUB_VECTORS(&tempVect, &platform->actor.pos, &hand.pos);
                            }
                        } else if (!((saveSlot[currentSaveSlot].levels >> ((gameInfo.world - 1) * 5 + 2)) & 1)) {
                            if (platform->exitType == 18) {
                                SUB_VECTORS(&tempVect, &platform->actor.pos, &hand.pos);
                            }
                        } else if (!((saveSlot[currentSaveSlot].levels >> ((gameInfo.world - 1) * 5 + 3)) & 1)) {
                            if (platform->exitType == 19) {
                                SUB_VECTORS(&tempVect, &platform->actor.pos, &hand.pos);
                            }
                        } else if (platform->exitType == 20) {
                            SUB_VECTORS(&tempVect, &platform->actor.pos, &hand.pos);
                        }
                    } else if (platform->exitType == 6 && gameInfo.level == HUB2) {
                        SUB_VECTORS(&tempVect, &platform->actor.pos, &hand.pos);
                    } else if (gameInfo.level == HUB3 || gameInfo.level == HUB4) {
                        if ((platform->exitType == 8 && (gameInfo.worldComplete[2] || (!gameInfo.worldComplete[3] && handControl.actionTime % 60 >= 30)))
                            || (platform->exitType == 7 && (gameInfo.worldComplete[3] || (!gameInfo.worldComplete[2] && handControl.actionTime % 60 < 30)))) {
                            SUB_VECTORS(&tempVect, &platform->actor.pos, &hand.pos);
                            break;
                        }
                    } else if (gameInfo.level == HUB5 || gameInfo.level == HUB6) {
                        if (platform->exitType == 10 && (gameInfo.worldComplete[4] || (!gameInfo.worldComplete[5] && handControl.actionTime % 60 >= 30))) {
                            SUB_VECTORS(&tempVect, &platform->actor.pos, &hand.pos);
                            break;
                        } else if (platform->exitType == 9 && (gameInfo.worldComplete[5] || (!gameInfo.worldComplete[4] && handControl.actionTime % 60 < 30))) {
                            SUB_VECTORS(&tempVect, &platform->actor.pos, &hand.pos);
                            break;
                        }
                    } else if (platform->exitType == 11 && gameInfo.level == HUB7) {
                        SUB_VECTORS(&tempVect, &platform->actor.pos, &hand.pos);
                    }
                }
            }
            tempFloat = LENGTH_2D(&tempVect);
            if (hand.anim->current == HANDANIM_POINTCYCLE) {
                if ((handControl.locateGaribs == 1 && pad[ActiveController].button[BUTTON_SHOULDER_RIGHT])
                    || ((pad[ActiveController].button[BUTTON_B] || ballControl.state == BALL_BURST) && !handControl.locateGaribs)) {
                    animSpeed = 300.0f / LENGTH(&tempVect);
                    hand.anim->speed = MIN(MAX(0.2, animSpeed), 3.0);
                } else {
                    hand.anim->speed = MAX(hand.anim->speed, hand.anim->anims[HANDANIM_POINTCYCLE].speed);
                }
            }
            if (tempFloat != 0.0f) {
                tempVect2.y = hand.rot.y = hand.rotAim.y = atan2(tempVect.x / tempFloat, tempVect.z / tempFloat);
                tempVect2.x = -atan2(tempVect.y, tempFloat);
                tempVect2.z = 0.0f;
                GetQuaternionFromXYZRot(&qAim, &tempVect2);
            } else {
                q.x = 1.0f;
                q.y = 0.0f;
                q.z = 0.0f;
                if (tempVect.y > 0.0f) {
                    q.w = -1.5707964f;
                } else {
                    q.w = 1.5707964f;
                }
                GetQuaternionFromRotation(&qAim, &q);
            }
        } else {
            for (x = 0; x < 2; x++) {
                locSprite = handControl.locate[x];
                if (locSprite != NULL) {
                    locSprite->scaleX = locSprite->scaleY = locSprite->anim.def.startScale = locSprite->anim.def.endScale = locSprite->anim.def.endScale >> 1;
                }
            }
            vertQ.w = hand.rot.y;
            GetQuaternionFromRotation(&qAim, &vertQ);
        }
        QuatSlerpAtFixedSpeed(&hand.rotQuat, &qAim, 0.15f, &hand.rotQuat);
        if (hand.anim->finishedAnim == 51) {
            ApplyActionIdle();
        }
        break;
    case HAND_SPIN:
        if ((!levelExitTimer && hand.anim->current == HANDANIM_SPIN && ballControl.transformState >= -15 && pad[ActiveController].pressed[BUTTON_SHOULDER_RIGHT])
            || (!ballControl.transformState && pad[ActiveController].button[BUTTON_SHOULDER_RIGHT])) {
            HandTransformBall();
            handControl.actionTime = 0;
        }
        if (hand.anim->loop && (hand.anim->current == HANDANIM_SPIN || hand.anim->current == HANDANIM_PEDDLE)
            && (handControl.actionTime >= 63
                || (pad[ActiveController].mag != 0.0f && !pad[ActiveController].button[BUTTON_SHOULDER_RIGHT] && ballControl.transformState >= -17)
                || (levelExitTimer && handControl.actionTime >= 26))) {
            hand.anim->loop = 0;
            if (levelExitTimer) {
                AnimateActor(&hand, HANDANIM_LEVELDONE, 0, 1, 1.2f);
            } else if (hand.anim->current == HANDANIM_SPIN) {
                AnimateActor(&hand, HANDANIM_SPIN2CUP, 0, 0, 0.0f);
            } else {
                AnimateActor(&hand, HANDANIM_CUP2PEDDLE, 0, 0, -0.66f);
            }
        }
        if (hand.anim->current == HANDANIM_SPIN2CUP && !levelExitTimer) {
            SetHandRadius(&hand.obj->spheres[0].targetRadius, ball.obj->spheres[0].targetRadius + 0.05);
        }
        if (hand.anim->finishedAnim == 66 || (hand.anim->finishedAnim == 17 && hand.anim->current != HANDANIM_PEDDLE)) {
            hand.state->mode = HAND_JOINED;
            handControl.actionTime = 0;
        }
        if (TriggerAtFrame(1289.0f, &hand)) {
            PlaySample(148, &hand.pos, 120, Random(56) + 100);
        }
        if (TriggerAtFrame(1293.0f, &hand)) {
            PlaySample(149, &hand.pos, 120, Random(56) + 100);
        }
        if (hand.anim->current == HANDANIM_SPIN2CUP && pad[ActiveController].mag > 5.0f) {
            hand.state->mode = HAND_JOINED;
            handControl.actionTime = 0;
        }
        break;
    }
    if (handControl.status == HAND_TELEPORTING) {
        tempVect = hand.pos;
        temp = Random(30) - 15;
        temp /= 10.0f;
        tempVect.x += temp;
        temp = Random(30) - 15;
        temp /= 10.0f;
        tempVect.y += temp;
        temp = Random(30) - 15;
        temp /= 10.0f;
        tempVect.z += temp;
        AddDebris(SPARKLE, &tempVect, &zero, NULL, NULL);
        if (!--hand.lifeTimer) {
            handControl.status = HAND_NORMAL;
        }
    }
    if (!ballControl.loseTimer) {
        if (ballControl.fallTime >= 19 && ball.obj->contactFlags && ball.obj->contactFlags != 8 && ball.prevVel.y < -14.25) {
            ballControl.loseTimer = 30;
            ballControl.state = BALL_BURST;
            PlayContinuousSample(&handControl.samples[0], 276, 100, &hand.pos, 140);
        }
        if (ballControl.fallTime >= 41) {
            ballControl.loseTimer = 30;
            ballControl.state = BALL_BURST;
            PlayContinuousSample(&handControl.samples[0], 276, 100, &hand.pos, 140);
        }
    }
    if (hand.obj->onGround == 1 && !handControl.prevOnGround && !IsHandHoldingBall() && !handControl.frog) {
        if (hand.prevVel.y < -7.0) {
            if (hand.vel.y == -15.0) {
                AnimateActor(&hand, HANDANIM_STUN, 0, 0, 0.0f);
                AnimateActor(&hand, HANDANIM_STAND, 1, 1, 0.0f);
                handControl.actionLock = 1080;
            }
            tempVect = hand.pos;
            tempVect.y -= 10.0f;
            if (!(handControl.flags & 1)) {
                CreateDustCircle(&tempVect, 4, 0.0f);
            }
        }
        PlaySample(154, &hand.pos, MIN(MAX(0, -hand.prevVel.y * 15.0f), 128), 128);
    }
    if (ball.obj->onGround == 1 && !ballControl.prevGround[0]) {
        if (ball.obj->planes[0] != NULL && ((CollFace *) ball.obj->planes[0])->terrain != 5) {
            tempVect = ball.pos;
            tempVect.y -= ball.obj->spheres[0].radius * 0.8;
            if (ball.prevVel.y < -10.0f) {
                AddDebris(DUST, &tempVect, &zero, NULL, NULL);
            }
        }
    }
}

void MoveHand(void) {
    Quat tempQ;
    Quat qAim;
    Vec3 v;
    struct Plane *plane;
    struct Plane *oldSuctionPlane;
    struct Plane *p;
    Actor *actor;
    Bullet *bul;
    f32 rot;
    f32 max;
    f32 dot;
    s32 j;
    s32 contactTime;
    s32 action;
    s32 found;
    s32 step;
    s32 angle;
    s32 sectors = 8; // a variable, not a constant: the ROM converts it to double at run time

    UpdateBlastCircle();
    if (hand.state->mode == HAND_WALK && hand.speed > 2.5) {
        rot = FindShortestAngleSigned(hand.rotAim.y, hand.rot.y) * handBank;
        if (ABS(rot) > 0.08) {
            hand.rotAim.z = Aabs(rot);
        } else {
            hand.rotAim.z = 0.0f;
        }
        hand.phys->rotAim[2] = 1;
        ActorHandleRotaim(&hand, 2);
        hand.phys->rotAim[2] = 0;
    } else if (handControl.checkpoint->restartGate->restartTimer < 11) {
        hand.rotVel.z /= 2.0f;
        hand.rotAim.z = 0.0f;
        hand.phys->rotAim[2] = 1;
        ActorHandleRotaim(&hand, 2);
        hand.phys->rotAim[2] = 0;
    }
    if (((hand.flags & 0x8000) && hand.prevVel.y < -10.5) || hand.prevVel.y < -14.25) {
        if (hand.obj->contactFlags != 0 && hand.obj->contactFlags != 8) {
            if (handControl.fallTime >= 17) {
                if (!HandOnTrampoline()) {
                    HurtHand(HURT_FALL, -1);
                }
            } else if (handControl.fallTime >= 9 && !HandOnTrampoline()) {
                HurtHand(HURT_FALL, 1);
            }
        }
    }
    if (handControl.fallTime >= 17
        && (hand.shadow->altitude == -1.0f
            || (hand.link != NULL && hand.link->shadow->altitude == -1.0f))) {
        KillHand(HURT_FALL);
    }
    for (j = 0; j < 2; j++) {
        if (handControl.samples[j].handle) {
            UpdateContinuousSample(&handControl.samples[j]);
            if (j == 0) {
                if (handControl.samples[0].id == 160) {
                    if (!handControl.deathTimer && (hand.state->waterState || hand.obj->onGround)) {
                        StopContinuousSample(&handControl.samples[0]);
                    }
                } else if (handControl.samples[0].volume) {
                    handControl.samples[0].volume--;
                } else {
                    StopContinuousSample(&handControl.samples[0]);
                }
            } else if (hand.state->mode != HAND_ROTOR && !player.powerups[SPEEDUP]) {
                StopContinuousSample(&handControl.samples[1]);
            }
        }
    }
    if (gameInfo.level == SPACEBOSS3 || hand.state->mode == HAND_CLIMBDOWN || hand.state->mode == HAND_CLING) {
        hand.phys->mass = 999999.0f;
        hand.flags |= 0x10000000;
    } else {
        hand.flags &= ~0x10000000;
        if (player.powerups[HERCULES]) {
            hand.phys->mass = 18.0f;
        } else {
            hand.phys->mass = 10.0f;
        }
    }
    if (hand.state->waterState
        || (hand.obj->planes[0] != NULL && ((CollFace *)hand.obj->planes[0])->terrain == 5)) {
        handControl.glovePrintTimer = 100;
    } else if (handControl.glovePrintTimer) {
        handControl.glovePrintTimer--;
    }
    if (hand.state->mode != HAND_LOCATE || handControl.deathTimer) {
        for (j = 0; j < 2; j++) {
            if (handControl.locate[j] != NULL) {
                SubSprite(handControl.locate[j]);
                JallocFree(&handControl.locate[j]);
            }
        }
    }
    if (handControl.smokeTimer) {
        handControl.smokeTimer--;
        if (Random(35) < 11) {
            bul = FireBulletFromActor(&hand, SMOKE_BULLET, 0);
            if (bul != NULL) {
                bul->actor.vel.y += 2.0f;
                v.x = 0.0f;
                v.z = -4.0f;
                RotateVector2D(&v, &v, hand.rot.y);
                AddToVector2D(&bul->actor.pos, &v);
            }
        }
    }
    if (!hand.obj->onGround && !(hand.flags & 0x2000) && hand.state->mode != HAND_SLIDING && !handControl.frog
        && LENGTH(&handControl.inputAccel) == 0.0f) {
        SCALE_VECTOR_2D(&hand.vel, 0.7);
    }
    if (handControl.dropTimer) {
        if (--handControl.dropTimer == 0) {
            hand.link = NULL;
        }
    }
    if (handControl.fakeballDelay) {
        handControl.fakeballDelay--;
    }
    if (handControl.morphStep) {
        if (!handControl.morphReverse) {
            hand.scale.x = hand.scale.y = hand.scale.z = tongueRad[handControl.morphStep++];
        } else {
            hand.scale.x = hand.scale.y = hand.scale.z = 0.12 - tongueRad[handControl.morphStep++];
        }
        if (handControl.morphStep >= 11) {
            handControl.morphStep = 0;
        }
        SetHandRadius(&hand.obj->spheres[0].targetRadius, 13.0f);
        hand.obj->spheres[0].radius = hand.obj->spheres[0].targetRadius;
        hand.shadow->shadowSize = hand.obj->spheres[0].radius * 0.6;
    }
    if (handControl.deathTimer) {
        if (((hand.obj->contactFlags & 1) || (hand.obj->contactFlags & 2)) && hand.anim->current == HANDANIM_FALL) {
            AnimateActor(&hand, HANDANIM_FALLDEATH, 0, 0, 0.0f);
        }
        player.hurtTimer = 1;
        handControl.fallTime = 0;
        if (--handControl.deathTimer == 0) {
            LoseLife();
        }
        HandleSpellOrbiters();
        if (hand.obj->planes[0] != NULL && handControl.deathTimer < 70) {
            FindFlattestPlane(&hand, (Plane **)&plane, &j);
            hand.flags |= 0x4000;
            CalculateQuatForPlane(&hand, &tempQ, (Vec3 *)plane);
            QuatSlerp(&hand.rotQuat, &tempQ, 0.1f, &hand.rotQuat);
        }
    }
    if (player.hurtTimer) {
        player.hurtTimer--;
        if (gameInfo.level != SPACEBOSS3) {
            if (player.hurtTimer & 2) {
                hand.alpha = 0;
            } else {
                hand.alpha = 100;
            }
        }
    }
    if (handControl.noControlTimer) {
        handControl.noControlTimer--;
    }
    if (IsHandHoldingBall()) {
        actor = &ball;
    } else if (hand.link != NULL) {
        actor = hand.link;
    } else {
        actor = &hand;
    }
    if ((hand.state->mode != HAND_SLIDING || hand.shadow->altitude == -1.0f) && hand.state->mode != HAND_ROTOR
        && !actor->obj->onGround
        && (((hand.flags & 0x8000) && actor->vel.y < -10.5) || actor->vel.y < -14.25)) {
        handControl.fallTime++;
        if (actor == &hand && handControl.fallTime >= 16) {
            if (!handControl.samples[0].handle) {
                PlayContinuousSample(&handControl.samples[0], 160, 100, &actor->pos, 128);
            }
            if (!handControl.frog) {
                AnimateActor(actor, 5, 1, 0, 0.0f);
                handControl.doubleJumped = 1;
                hand.state->mode = HAND_JUMP;
                handControl.gait = -1;
            }
            if (!cameraInfo.fallTime) {
                cameraInfo.fallTime = 1;
            }
        }
    } else if (!handControl.deathTimer) {
        handControl.fallTime = 0;
        cameraInfo.fallTime = 0;
    }
    if (handControl.deathTimer) {
        return;
    }
    for (j = 0; j < 3; j++) {
        if (handControl.spellTimer[j]) {
            if (--handControl.spellTimer[j] == 0) {
                DeactivateSpellHand(j);
            }
        }
    }
    if (handControl.spellTimer[2]) {
        HandProcessFroggy();
    }
    if (hand.state->mode != HAND_SNATCH && hand.state->mode != HAND_CLIMBDOWN && hand.state->mode != HAND_CLIMBUP) {
        handControl.climbPlatform = NULL;
    }
    CheckHandCatapult();
    CheckHandPlatform();
    MaintainAction();
    HandleSpellOrbiters();
    if (player.powerups[VANISHBALL]) {
        ball.link = &hand;
        ballControl.releaseTimer = 0;
        handControl.releaseTimer = 0;
        if (!ballControl.returnTime && IsHandHoldingBall()) {
            ball.obj->spheres[0].targetRadius = 0.0f;
            ReleaseBall();
            ApplyActionIdle();
        }
        if (ball.radius <= 0.1) {
            ball.pos.z = ball.pos.y = ball.pos.x = -10000.0f;
        } else if (ball.obj->spheres[0].targetRadius == 0.0f) {
            ball.pos = hand.pos;
        }
    }
    if (handControl.actionLock >= 1000 && handControl.actionLock < 2000) {
        if (hand.anim->current != handControl.actionLock - 1000) {
            handControl.actionLock = 0;
        } else if (hand.anim->finishedAnim) {
            handControl.actionLock = 0;
        }
    } else if (handControl.actionLock > 2000) {
        handControl.actionLock--;
    } else if (handControl.actionLock == 2000) {
        if (hand.obj->onGround == 1) {
            handControl.actionLock = 0;
        } else if (hand.flags & 0x1000) {
            handControl.actionLock = 0;
        }
    } else if (handControl.actionLock) {
        handControl.actionLock--;
    }
    if (hand.state->mode == HAND_DUCK || hand.state->mode == HAND_CRAWL || hand.state->mode == HAND_SWISH
        || hand.state->mode == HAND_NOTHING || hand.state->mode == HAND_JOINED || hand.state->mode == HAND_PREWHACK
        || (hand.flags & 0x8000) || hand.state->mode == HAND_LOCATE) {
        hand.flags |= 0x4000;
        ActorHandleRotaim(&hand, 1);
    } else {
        hand.flags &= ~0x4000;
    }
    if (hand.state->mode == HAND_DUCK || hand.state->mode == HAND_CRAWL || hand.state->mode == HAND_JOINED
        || hand.state->mode == HAND_PREWHACK || (hand.link == NULL && hand.state->mode == HAND_NOTHING)) {
        if (hand.obj->platform != NULL && hand.obj->platform->motion != NULL) {
            hand.rot.y = Aabs(hand.rot.y + hand.obj->platform->motion->rot.y
                                    - hand.obj->platform->motion->oldRot.y);
            hand.rotAim.y = Aabs(hand.rotAim.y + hand.obj->platform->motion->rot.y
                                       - hand.obj->platform->motion->oldRot.y);
        }
        if (hand.state->mode == HAND_DUCK || hand.state->mode == HAND_CRAWL) {
            FindFlattestPlane(&hand, (Plane **)&plane, &j);
            contactTime = hand.obj->contactCount[j];
        } else {
            FindFlattestPlane(&ball, (Plane **)&plane, &j);
            contactTime = ball.obj->contactCount[j];
        }
        if (plane != NULL && plane->y > 0.0f) {
            v.x = 0.0f;
            v.y = 0.0f;
            v.z = 1.0f;
            RotateVector2D(&v, &v, hand.rot.y);
            if (contactTime >= 2) {
                hand.rotAim.x = Aabs(1.570796327 - acos(v.x * plane->x + v.z * plane->z));
                ActorHandleRotaim(&hand, 0);
                CalculateQuatForPlane(&hand, &handQAim, (Vec3 *)plane);
            }
        } else {
            vertQ.w = hand.rot.y;
            if (contactTime >= 2) {
                GetQuaternionFromRotation(&handQAim, &vertQ);
                hand.rotAim.x = 0.0f;
                ActorHandleRotaim(&hand, 0);
            }
        }
        if (hand.obj->platform != NULL && hand.obj->platform->motion != NULL) {
            vertQ.w = hand.obj->platform->motion->rot.y - hand.obj->platform->motion->oldRot.y;
            GetQuaternionFromRotation(&qAim, &vertQ);
            QuaternionMultiply(&handQAim, &qAim, &handQAim);
        }
        QuatSlerpAtFixedSpeedOld(&hand.rotQuat, &handQAim, hand.phys->turnSpeed * 1.5, &hand.rotQuat);
    } else {
        if (hand.state->mode != HAND_PUSH && hand.state->mode != HAND_ROTOR && hand.state->mode != HAND_THROWAIM
            && hand.state->mode != HAND_CATCH
            && ((!(hand.flags & 0x2000) && !handControl.catapult) || hand.state->mode == HAND_JOINED)) {
            if (player.powerups[SPEEDUP] && hand.anim->current == HANDANIM_RUN) {
                hand.rotAim.x = 5.5f;
            } else {
                hand.rotAim.x = 0.0f;
            }
        }
        handQAim = hand.rotQuat;
    }
    if ((hand.flags & 0x8000) && (hand.state->mode != HAND_LOCATE || hand.anim->current == HANDANIM_POINT2IDLE)) {
        max = 200000.0f;
        ActorHandleRotaim(&hand, 1);
        oldSuctionPlane = (struct Plane *)handControl.suctionPlane;
        for (j = 0; j < 4; j++) {
            p = hand.obj->planes[j];
            if (p != NULL) {
                dot = DOT_PRODUCT(&handControl.inputAccel, p);
                if (dot <= max) {
                    handControl.suctionPlane = (Vec3 *)p;
                    max = dot;
                }
            }
        }
        if (oldSuctionPlane == (struct Plane *)handControl.suctionPlane) {
            handControl.suctionTime++;
        } else {
            handControl.suctionTime = 0;
        }
        if (handControl.suctionLostTime == 6) {
            handControl.suctionPlane = NULL;
            handControl.suctionTime = 0;
        }
        if (handControl.suctionPlane != NULL) {
            CalculateQuatForPlane(&hand, &tempQ, handControl.suctionPlane);
        } else {
            vertQ.w = hand.rot.y;
            GetQuaternionFromRotation(&tempQ, &vertQ);
        }
        QuatSlerp(&hand.rotQuat, &tempQ, MIN(handControl.suctionTime * 0.16, 1.0f), &hand.rotQuat);
        if (pad[ActiveController].pressed[BUTTON_Z]) {
            DeactivateCollectable(SUCTION);
            pad[ActiveController].sincePress[BUTTON_Z] = 6;
            pad[ActiveController].latch[BUTTON_Z] = 1;
        }
    } else if (hand.state->mode != HAND_ROTOR && !handControl.frog && !hand.state->waterState && !IsHandHoldingBall()
               && !ConditionsMet(HAND_JOINED) && (ball.link != &hand || handControl.releaseTimer)
               && hand.obj->planes[0] != NULL) {
        if (hand.shadow->floor != NULL && hand.shadow->floor->y > 0.0f) {
            found = FALSE;
            plane = (struct Plane *)hand.shadow->floor;
            for (j = 0; j < 4; j++) {
                if (hand.obj->planes[j] == plane
                    || (hand.obj->planes[j] != NULL
                        && DOT_PRODUCT(hand.shadow->floor, hand.obj->planes[j]) > 0.95)) {
                    found = TRUE;
                    break;
                }
            }
            if (found) {
                CheckPlaneForSliding((CollFace *)plane);
            }
        }
        if (hand.state->mode == HAND_JUMP && !hand.obj->onGround && handControl.groundStateTime >= 3) {
            for (j = 0; j < 4; j++) {
                plane = hand.obj->planes[j];
                if (plane != NULL && plane->y > 0.0f
                    && DOT_PRODUCT(&hand.vel, plane) < 0.0f) {
                    CheckPlaneForSliding((CollFace *)plane);
                }
            }
        }
    }
    if (gameInfo.level == ATLANTISBONUS) {
        angle = hand.rotAim.y * 1000.0f;
        step = 6283.185308 / sectors;
        angle += step / 2;
        angle = angle / step * step;
        hand.rotAim.y = Aabs((f32)angle / 1000.0);
    }
    if (handControl.actionLock == 0) {
        for (action = 0; action < 45; action++) {
            if (handActionValidityTable[hand.state->mode][action] && ConditionsMet(action) && !levelExitTimer) {
                ApplyAction(action);
                break;
            }
        }
    }
    if (hand.obj->onGround && pad[ActiveController].mag == 0.0f && !pad[ActiveController].button[BUTTON_B]
        && !pad[ActiveController].button[BUTTON_A] && !pad[ActiveController].button[BUTTON_Z] && !pad[ActiveController].button[BUTTON_SHOULDER_RIGHT]
        && !pad[ActiveController].button[BUTTON_SHOULDER_LEFT] && !gameInfo.cameoActive) {
        handControl.idleTime++;
    } else {
        handControl.idleTime = 0;
    }
    handControl.prevGait = handControl.gait;
    handControl.prevAction = hand.state->mode;
    if (handControl.prevOnGround == hand.obj->onGround) {
        handControl.groundStateTime++;
    } else {
        handControl.groundStateTime = 0;
    }
    handControl.prevOnGround = hand.obj->onGround;
    ballControl.prevGround[0] = ball.obj->onGround;
    handControl.actionTime++;
    if (hand.flags & 0x8000) {
        if (hand.obj->planes[0] != NULL) {
            handControl.suctionLostTime = 0;
        } else {
            handControl.suctionLostTime++;
        }
    }
    if (ball.link != &hand) {
        if (!ball.obj->onGround && ball.vel.y < -14.25 && ball.obj->planes[0] == NULL) {
            ballControl.fallTime++;
        } else {
            ballControl.fallTime = 0;
        }
    }
    hand.phys->accel = 5.0f;
    hand.phys->moveSpeed = 7.0f;
    if (player.powerups[HERCULES]) {
        hand.phys->accel *= 2.8;
        hand.phys->moveSpeed = 10.5f;
    }
    if (player.powerups[SPEEDUP]) {
        hand.phys->moveSpeed = 10.0f;
        hand.phys->accel *= 2.5;
    }
    if (player.powerups[SUCTION]) {
        hand.phys->moveSpeed /= 3.0f;
    }
    if (hand.state->mode == HAND_SLIDING) {
        hand.phys->moveSpeed *= 3.0f;
    }
    if (handControl.flags & 1) {
        hand.phys->accel *= 2.5;
    }
    if (hand.state->mode == HAND_CRAWL) {
        if (hand.anim->time >= 270.0f && hand.anim->time <= 280.0f) {
            hand.phys->moveSpeed = 1.0f;
            return;
        }
        hand.phys->moveSpeed = hand.phys->accel = 0.0f;
        SCALE_VECTOR_2D(&hand.vel, 0.5);
    }
}

void CheckHandCatapult(void) {
    Vec3 tempVect;
    Platform *platform;

    if (hand.flags & 0x2000) {
        if ((hand.obj->onGround && hand.state->mode != HAND_JOINED)
            || (hand.state->mode == HAND_JOINED && ball.obj->onGround)) {
            hand.flags &= ~0x01002000;
            if (hand.state->mode == HAND_JOINED) {
                ball.flags &= ~0x01000000;
            }
        } else {
            hand.flags |= 0x10;
            hand.friction = handInertia;
            DisableControl(2);
        }
    }
    if (handControl.catapult != NULL) {
        platform = handControl.catapult;
        hand.friction = handInertia;
        if (platform->catapult->catchTimer > 0) {
            tempVect = platform->catapult->holdPos;
            if (platform->motion != NULL) {
                RotateVector2D(&tempVect, &tempVect, platform->motion->rot.y);
            }
            ADD_VECTORS(&tempVect, &platform->actor.pos, &tempVect);
            if (hand.state->mode == HAND_JOINED) {
                AddScaledVector2D(&ball.pos, &ball.pos, 0.8f, &tempVect, 0.2f);
            } else {
                AddScaledVector2D(&hand.pos, &hand.pos, 0.8f, &tempVect, 0.2f);
            }
            platform->catapult->catchTimer--;
            if (platform->catapult->catchTimer == 0 && hand.obj->platform == platform && platform->catapult->launchTimer == 0) {
                if (hand.state->mode != HAND_JOINED) {
                    hand.rotAim.x = 1.5707964f;
                }
                hand.obj->platform->motion->rotVel[0] = -platform->catapult->armRotVel;
                platform->catapult->launchTimer = platform->catapult->launchDelay;
            }
            if (hand.state->mode == HAND_JOINED) {
                SetVector2D(&ball.vel, &zero);
                MoveHandToBall();
            } else {
                SetVector2D(&hand.vel, &zero);
            }
        }
        if (platform->catapult->launchTimer > 0) {
            if (--platform->catapult->launchTimer == 0) {
                platform->motion->rotVel[0] = -platform->motion->rotVel[0] / 4.0f;
                platform->catapult->launchTimer = -1;
                hand.flags = (hand.flags | 0x2000) & ~0x10;
                if (hand.state->mode == HAND_JOINED) {
                    ball.vel = zero;
                    AccelerateActorFromActor(&hand, &ball, platform->catapult->launchSpeed, 100000.0f);
                    ball.vel.y = platform->catapult->launchVelY;
                    ball.flags |= 0x01000000;
                } else {
                    hand.vel = zero;
                    ApplyAction(HAND_JUMP);
                    AccelerateActor2D(&hand, handControl.catapult->catapult->launchSpeed, 100000.0f);
                    hand.vel.y = platform->catapult->launchVelY;
                    hand.friction = handInertia;
                    hand.flags |= 0x01000000;
                }
            }
        } else if (platform->catapult->launchTimer != 0) {
            if (--platform->catapult->launchTimer == (1 - platform->catapult->launchDelay) * 4) {
                handControl.catapult = NULL;
                platform->motion->rotVel[0] = 0.0f;
                platform->motion->rot.x = 0.0f;
                platform->catapult->launchTimer = 0;
            }
        }
    }
    if (hand.obj->platform != NULL) {
        if (hand.obj->platform->catapult != NULL && hand.obj->platform->catapult->active != 0
            && hand.obj->platform->catapult->catchTimer == 0 && hand.obj->platform->catapult->launchTimer == 0) {
            tempVect = hand.obj->platform->catapult->holdPos;
            if (hand.obj->platform->motion != NULL) {
                RotateVector2D(&tempVect, &tempVect, hand.obj->platform->motion->rot.y);
            }
            ADD_VECTORS(&tempVect, &hand.obj->platform->actor.pos, &tempVect);
            if ((hand.state->mode == HAND_JOINED && DistanceBetweenPointsSquared(&ball.obj->contactPos[0], &tempVect) < 100.0f)
                || (hand.state->mode != HAND_JOINED && DistanceBetweenPoints(&hand.obj->contactPos[0], &tempVect) < 10.0f)) {
                if (hand.state->mode == HAND_JOINED) {
                    SetVector2D(&ball.vel, &zero);
                    MoveHandToBallStatic();
                    ball.spinSpeed = 0.0f;
                } else {
                    ApplyActionIdle();
                    SetVector2D(&hand.vel, &zero);
                }
                PRINTF("catapult activated\n");
                handControl.catapult = hand.obj->platform;
                handControl.catapult->catapult->catchTimer = 10;
                handControl.catapult->catapult->unk16 = 0;
                handControl.actionLock = handControl.catapult->catapult->launchDelay + 11;
                DisableControl(handControl.actionLock);
                hand.rotAim.y = Aabs(hand.obj->platform->motion->rot.y + 3.141592654);
            }
        }
    }
}

inline void DisableControl(s16 time) {
    if (gameInfo.mode == 4 || gameInfo.mode == 5) {
        if (handControl.noControlTimer < time) {
            handControl.noControlTimer = time;
        }
    }
}

s32 HandOverstepsEdge(Actor *probe, CollFace *plane) {
    Vec3 dir;
    Vec3 d;
    f32 val1;
    f32 b;
    s32 x;
    s32 j;

    for (x = 0; x < 3; x++) {
        if (plane->edgeLen[x] == -1.0f) {
            continue;
        }
        j = (x + 1) % 3;
        dir.x = -plane->edgeStart[x].z + plane->edgeStart[j].z;
        dir.z = plane->edgeStart[x].x - plane->edgeStart[j].x;
        d.x = hand.pos.x - plane->edgeStart[j].x;
        d.z = hand.pos.z - plane->edgeStart[j].z;
        val1 = d.x * dir.x + d.z * dir.z;
        d.x = probe->pos.x - plane->edgeStart[j].x;
        d.z = probe->pos.z - plane->edgeStart[j].z;
        b = d.x * dir.x + d.z * dir.z;
        if (val1 * b < 0.0f) {
            hand.rot.y = hand.rotAim.y = Aabs(atan2(dir.x, dir.z));
            SubVector2D(&dir, &hand.pos, &plane->edgeStart[x]);
            SubVector2D(&d, &hand.pos, &plane->edgeStart[j]);
            val1 = LENGTH_2D(&dir);
            b = LENGTH_2D(&d);
            AddScaledVector(&sepVect, &plane->edgeStart[x], b / (val1 + b), &plane->edgeStart[j], val1 / (val1 + b));
            return TRUE;
        }
    }
    return FALSE;
}

void DropHand(s16 timer) {
    hand.vel = zero;
    handControl.status = HAND_NORMAL;
    handControl.attachedTo = NULL;
    handControl.attachOffset = zero;
    ApplyAction(HAND_JUMP);
    DisableControl(20);
    handControl.dropTimer = timer;
    if (!cameraInfo.overrideTimer) {
        cameraInfo.focus = &hand.pos;
        cameraInfo.focusVel = &hand.vel;
    }
}

void KillHand(s16 type) {
    s32 i;

    player.health = 0;
    DeactivateCollectable(ROTORBLADES);
    if (handControl.attachedTo) {
        EnemyDropHand();
    }
    if (controlMethod == CONTROL_CAMERA) {
        DeactivateCameraControlMode();
    }
    hand.flags = (hand.flags & ~8) | 0x11;
    hand.state->mode = HAND_HURT;
    for (i = 0; i < 15; i++) {
        if (player.powerups[i] > 40) {
            player.powerups[i] = 40;
        }
    }
    if (handControl.deathTimer) {
        return;
    }
    if (gameInfo.level == SPACEBOSS3) {
        LoseLife();
        return;
    }
    ApplyAction(HAND_HURT);
    SetHandRadius(&hand.obj->spheres[0].targetRadius, 13.0f);
    DisableControl(60);
    switch (type) {
        case 2:
            if (hand.obj->contactFlags) {
                if (handControl.samples[0].handle) {
                    StopContinuousSample(&handControl.samples[0]);
                }
                PlaySample((Random(3) + 273), &hand.pos, 100, 128);
                if (!handControl.frog) {
                    AnimateActor(&hand, HANDANIM_FALLDEATH, 0, 0, 0.5f);
                }
            } else {
                if (!handControl.samples[0].handle) {
                    PlayContinuousSample(&handControl.samples[0], 160, 100, &hand.pos, 128);
                }
                AnimateActor(&hand, HANDANIM_FALL, 1, 0, 0.0f);
                hand.state->mode = HAND_JUMP;
                handControl.doubleJumped = 1;
                handControl.gait = -1;
                if (!cameraInfo.fallTime) {
                    cameraInfo.fallTime = 1;
                }
            }
            break;
        case 3:
            PlaySample((Random(3) + 273), &hand.pos, 100, 128);
            AnimateActor(&hand, HANDANIM_SQUASH, 0, 0, 0.0f);
            hand.flags &= ~0x100;
            break;
        default:
            PlaySample((Random(3) + 273), &hand.pos, 100, 128);
            AnimateActor(&hand, HANDANIM_CARDIACDEATH, 0, 0, 0.6f);
            break;
    }
    handControl.deathTimer = 80;
    if (ballControl.loseTimer) {
        ballControl.loseTimer = 81;
    }
    handControl.actionLock = 80;
}

void HurtHand(s32 type, s16 damage) {
    Vec3 tempVect;
    f32 speed = 13.0f;

    DeactivateCollectable(ROTORBLADES);
    if (controlMethod == CONTROL_BALL || handControl.deathTimer || player.hurtTimer) {
        return;
    }
    if (damage == 0) {
        damage = 1;
    }
    if (handControl.frog) {
        DefrogHand();
        KillHand(type);
        handControl.deathTimer = 1;
        ballControl.loseTimer = 0;
        return;
    }
    SetHandRadius(&hand.obj->spheres[0].targetRadius, speed);
    hand.friction = handInertia;
    if (damage >= 0) {
        if (!(cheatActivated & 0x40)) {
            player.health -= damage;
        }
    } else {
        player.health = 0;
    }
    DisableControl(20);
    if (IsHandHoldingBall() == 1) {
        ApplyAction(HAND_RELEASE);
        hand.vel = zero;
        hand.flags |= 0x18;
    }
    if (hand.state->mode == HAND_HOPPING || hand.state->mode == HAND_SWISH) {
        DismountEnemy();
    }
    if (hand.state->mode == HAND_BALLWALK) {
        DismountBall();
    }
    if (player.health <= 0) {
        if (gameInfo.level == SPACEBOSS3) {
            player.hurtTimer = 5;
        }
        KillHand(type);
        return;
    }
    stats[HEALTH_STATS].scale = 0x800;
    PlaySample((Random(4) + 261), &hand.pos, 140, 128);
    if (gameInfo.level == SPACEBOSS3) {
        player.hurtTimer = 5;
        return;
    }
    player.hurtTimer = 60;
    ApplyAction(HAND_HURT);
    hand.vel.y = 9.0f;
    switch (type) {
        case HURT_GO_TO_SAFE:
            if (handControl.lastGroundPlatform == NULL) {
                SubVector2D(&tempVect, &hand.pos, &handControl.lastGroundPos);
            } else {
                SubVector2D(&tempVect, &hand.pos, &handControl.lastGroundPlatform->actor.pos);
                AddVector2D(&tempVect, &tempVect, &handControl.lastGroundPos);
            }
            MakeUnit2D(&tempVect);
            speed = -speed;
            SCALE_VECTOR_2D(&tempVect, speed);
            if (LENGTH_2D(&tempVect) != 0.0f) {
                SubVector2D(&hand.vel, &hand.vel, &tempVect);
                break;
            }
            AccelerateActor2D(&hand, speed, hand.phys->maxSpeed);
            break;
        case HURT_GO_BACKWARDS:
            speed = -speed;
            AccelerateActor2D(&hand, speed, hand.phys->maxSpeed);
            break;
        case HURT_FALL:
            break;
    }
}

void LoseLife(void) {
    if (IsBonusLevel()) {
        player.health = 3;
        ExitLevel(BONUS_DONE_EXIT, 1, 0);
        return;
    }
    if (!(cheatActivated & 8)) {
        AddScreenDebris(numLivesOL.x, numLivesOL.y, -4.0f, 12.0f, SDT_LIFE, 1);
        player.lives--;
        if (player.lives == -1) {
            gameOverCount = 1;
            gamePaused = TRUE;
        }
    }
    if (player.lives != -1) {
        player.health = 3;
    }
    if (gameInfo.level != SPACEBOSS3 && handControl.checkpoint && handControl.checkpoint->restartGate->restartTimer == 0) {
        handControl.checkpoint->restartGate->restartTimer = 65;
    }
    if (!controlMethod) {
        DeactivateCameraControlMode();
    }
}

void CheckHandPlatform(void) {
    Platform *platPtr;

    if (hand.obj->platform && hand.obj->platform->path &&
        (hand.obj->platform->path->moving == -1 || hand.obj->platform->path->moving == -2)) {
        if (hand.obj->platform->link && &hand.obj->platform->link->tag) {
            for (platPtr = FindTaggedPlatform(&platformList.head, hand.obj->platform->link->tag); platPtr;
                 platPtr = FindTaggedPlatform(platPtr, hand.obj->platform->link->tag)) {
                platPtr->actor.vel = hand.obj->platform->link->vel;
                if (platPtr->link) {
                    ADD_TO_VECTOR(&platPtr->actor.vel, &platPtr->link->velOffset);
                }
            }
        } else if (hand.state->mode == HAND_SLAM) {
            if (hand.obj->platform->path->moving == -1) {
                hand.obj->platform->pathStopFlags |= 2;
                hand.obj->platform->pathStopFlags &= ~0x800;
            }
            hand.obj->platform->path->moving = 1;
        }
        return;
    }
    if (hand.obj->prevPlatform && hand.obj->prevPlatform->path && hand.obj->prevPlatform->path->moving == -1 &&
        hand.obj->prevPlatform->link && &hand.obj->prevPlatform->link->tag) {
        for (platPtr = FindTaggedPlatform(&platformList.head, hand.obj->prevPlatform->link->tag); platPtr;
             platPtr = FindTaggedPlatform(platPtr, hand.obj->prevPlatform->link->tag)) {
            platPtr->actor.vel = hand.obj->prevPlatform->link->vel;
            if (platPtr->link) {
                ADD_TO_VECTOR(&platPtr->actor.vel, &platPtr->link->velOffset);
            }
        }
    }
}

s32 HandCatchAllowed(void) {
    s32 range;
    s32 height;

    if (ball.link && ball.link != &hand) {
        return FALSE;
    }
    if (ballControl.loseTimer) {
        return FALSE;
    }
    if (ballControl.state == BALL_HELD || ballControl.state == BALL_HELD_SPECIAL) {
        return FALSE;
    }
    if (hand.flags & 0x8000) {
        return FALSE;
    }
    if (ball.flags & 0x200000) {
        if (ball.state->waterState) {
            return FALSE;
        }
        if (hand.state->waterState == INWATER_SWIMMING) {
            return FALSE;
        }
    }
    if (hand.state->mode == HAND_CATCH) {
        range = 30;
        height = 0;
    } else {
        range = 20;
        height = 20;
    }
    if (ballControl.returnTime >= 50 &&
        DistanceBetweenPoints(&hand.pos, &ball.pos) < ball.radius + hand.radius + range * 2.5) {
        return TRUE;
    }
    if (ball.obj->onGround) {
        return FALSE;
    }
    if (!(ball.vel.y < 0.0f)) {
        return FALSE;
    }
    if (ball.pos.y - hand.pos.y > height && DistanceBetweenPoints2D(&hand.pos, &ball.pos) < range) {
        return TRUE;
    }
    return FALSE;
}

void HandleSpellOrbiters(void) {
    Quat quat;
    Quat q1;
    Quat q2;
    Vec3 tempVect;
    Actor *actor;

    if (handControl.orbiter.inst) {
        tempVect.x = 20.0f;
        tempVect.y = SineWave(handOrbiterFreq, 0, frameCount) * 5.0f + 3.0f;
        tempVect.z = 0.0f;
        RotateVector2D(&tempVect, &tempVect, -handControl.orbiter.rot.y);
        ADD_VECTORS(&handControl.orbiter.pos, &hand.pos, &tempVect);
        handControl.orbiter.rot.y = Aabs(handControl.orbiter.rot.y + 0.3);
        GetQuaternionFromXYZRot(&handControl.orbiter.rotQuat, &handControl.orbiter.rot);
        if (player.powerups[handControl.orbiter.type - 1000] < 25) {
            handControl.orbiter.scale.x = (handControl.orbiter.scale.x - 0.008 < 0.0) ? 0.0f : handControl.orbiter.scale.x - 0.008;
            handControl.orbiter.scale.y = (handControl.orbiter.scale.y - 0.008 < 0.0) ? 0.0f : handControl.orbiter.scale.y - 0.008;
            handControl.orbiter.scale.z = (handControl.orbiter.scale.z - 0.008 < 0.0) ? 0.0f : handControl.orbiter.scale.z - 0.008;
        }
        if (player.powerups[handControl.orbiter.type - 1000] >= 16 && !(frameCount & 1)) {
            tempVect.x = tempVect.z = 0.0f;
            tempVect.y = 5.0f;
            AddSpellDebris(6, &handControl.orbiter.pos, &tempVect, handControl.orbiter.type - 1000);
        }
    }
    if (handControl.orbiter2.inst) {
        tempVect.x = 20.0f;
        tempVect.y = SineWave(handOrbiterFreq, 0, frameCount) * 5.0f + 3.0f;
        tempVect.z = 0.0f;
        RotateVector2D(&tempVect, &tempVect, -handControl.orbiter2.rot.y);
        ADD_VECTORS(&handControl.orbiter2.pos, &hand.pos, &tempVect);
        handControl.orbiter2.rot.y = Aabs(handControl.orbiter2.rot.y - 0.3);
        GetQuaternionFromXYZRot(&handControl.orbiter2.rotQuat, &handControl.orbiter2.rot);
        if (player.powerups[handControl.orbiter2.type - 1000] < 30) {
            handControl.orbiter2.scale.x -= 0.01;
            handControl.orbiter2.scale.y -= 0.01;
            handControl.orbiter2.scale.z -= 0.01;
        }
        if (player.powerups[handControl.orbiter2.type - 1000] >= 16 && !(frameCount & 1)) {
            tempVect.x = tempVect.z = 0.0f;
            tempVect.y = 5.0f;
            AddSpellDebris(6, &handControl.orbiter2.pos, &tempVect, handControl.orbiter2.type - 1000);
        }
    }
    if (handControl.orbiter3.inst) {
        actor = handControl.orbiter3.link;
        tempVect.x = 0.0f;
        if (actor == &hand) {
            tempVect.y = 1.0f;
            tempVect.z = -0.13636364f;
        } else {
            tempVect.y = -0.9f;
            tempVect.z = -0.45f;
        }
        RotateVector2DXYZ(&tempVect, &tempVect, actor->rot.x, 0);
        RotateVector2D(&tempVect, &tempVect, actor->rot.y);
        AddOneScaledVector(&handControl.orbiter3.pos, &actor->pos, &tempVect, -22.0f);
        handControl.orbiter3.rot.y = Aabs(handControl.orbiter3.rot.y - handControl.rotorSpeed);
        quat.x = 1.0f;
        quat.y = 0.0f;
        quat.z = 0.0f;
        if (actor == &hand) {
            quat.w = hand.rot.x + PI;
        } else {
            quat.w = actor->rot.x;
        }
        GetQuaternionFromRotation(&q1, &quat);
        vertQ.w = actor->rot.y;
        GetQuaternionFromRotation(&quat, &vertQ);
        QuaternionMultiply(&q2, &quat, &q1);
        tempVect.x = 0.0f;
        tempVect.z = 0.0f;
        tempVect.y = 1.0f;
        RotateVector2DXYZ(&tempVect, &tempVect, actor->rot.x, 0);
        RotateVector2D(&tempVect, &tempVect, actor->rot.y);
        quat.x = tempVect.x;
        quat.y = tempVect.y;
        quat.z = tempVect.z;
        quat.w = handControl.orbiter3.rot.y - actor->rot.y;
        GetQuaternionFromRotation(&q1, &quat);
        QuaternionMultiply(&handControl.orbiter3.rotQuat, &q1, &q2);
    }
}

void SetHandRadius(f32 *radius, f32 value) {
    f32 r;
    f32 scale;

    if (gameInfo.level != SPACEBOSS3) {
        *radius = r = value * levelHandScale / 0.05;
        if (player.powerups[HERCULES]) {
            scale = hand.scale.x;
            if (scale == 0.0f) {
                scale = D_801EEC3C;
            }
            *radius = r * (scale / 0.05);
        }
    }
}

void HandThrowActor(Actor *actor) {
    f32 temp;

    actor->vel = hand.vel;
    temp = 5.25f;
    temp *= hand.phys->mass / actor->phys->mass;
    AccelerateActorFromActor(&hand, actor, temp, actor->phys->maxSpeed);
    actor->vel.y = temp * 2.0f;
    ActorMaxOutVel(actor, actor->phys->maxSpeed);
}

void HandWhackActor(Actor *actor) {
    f32 handMass;
    f32 mass;

    actor->vel = hand.vel;
    handMass = hand.phys->mass;
    mass = actor->phys->mass;
    actor->vel.y = handMass * 5.25 / mass;
    AccelerateActorFromActor(&hand, actor, handMass * 14.0 / mass, actor->phys->maxSpeed);
}

void AddFakeball(void) {
    handControl.fakeball = AddDebris(FAKEBALL, &ball.pos, &zero, NULL, NULL);
    if (handControl.fakeball) {
        *handControl.fakeball->info = *ball.phys;
        if (fakedies) {
            handControl.fakeball->flags = ball.flags | 0x80;
        } else {
            handControl.fakeball->flags = ball.flags;
        }
        handControl.fakeball->flags &= ~8;
        handControl.fakeball->flags |= 0x40000000;
        handControl.fakeball->owner = &hand;
        handControl.fakeball->obj->spheres[0].radius = ball.obj->spheres[0].radius;
        handControl.fakeball->obj->spheres[0].targetRadius = ball.obj->spheres[0].targetRadius;
        handControl.fakeball->friction = ball.friction;
        if (hand.state->mode == HAND_THROWAIM) {
            handControl.fakeball->sprite.r = hthrR;
            handControl.fakeball->sprite.g = hthrG;
            handControl.fakeball->sprite.b = hthrB;
        } else {
            handControl.fakeball->sprite.r = hwhkR;
            handControl.fakeball->sprite.g = hwhkG;
            handControl.fakeball->sprite.b = hwhkB;
        }
    }
}

void CheckPlaneForSliding(CollFace *plane) {
    Vec3 d;

    if (terrain[plane->terrain].gravityScale < 1.0f) {
        return;
    }
    SUB_VECTORS(&d, &hand.pos, &hand.prevPos);
    if (plane->plane.y <= (f32)(1.0 - 0.25 / terrain[plane->terrain].handSlideScale)) {
        if (hand.state->mode == HAND_SLIDING) {
            handControl.actionLock = 1;
            return;
        }
        if (plane->plane.y <= handSlide2 / terrain[plane->terrain].handSlideScale) {
            handControl.gentleSlide = 0;
        } else {
            handControl.gentleSlide = 1;
        }
        if (plane->plane.y > handSlide2) {
            handControl.steepSlide = 0;
        } else {
            handControl.steepSlide = 1;
        }
        ApplyAction(HAND_SLIDING);
    }
}

void ResetHand(s32 first) {
    char name[16];
    Object *tempObj;

    handQAim = zeroQuat;
    hand.type = HAND;
    blur1 = 0;
    blur2 = 0;
    sprintf(name, "white.bmp");
    FindTexture(&handTextureList[0], UpdateCRC(name), 1);
    sprintf(name, "whitered.bmp");
    FindTexture(&handTextureList[1], UpdateCRC(name), 1);
    memclear(hand.phys, sizeof(ActorPhysics));
    hand.friction = 0.6f;
    hand.phys->bottomDiv = 1.0f;
    hand.phys->rotDrag = 0.8f;
    hand.phys->radiusRate = 10.0f;
    handInertia = 0.6f;
    hand.phys->rotAim[1] = 1;
    hand.phys->rotAim[0] = 1;
    hand.flags = 0x40091D;
    hand.rot.x = hand.rotAim.x = hand.rot.z = hand.rotAim.z = 0.0f;
    if (gameInfo.level == CARNIVALBONUS) {
        levelHandScale = 0.025f;
    } else {
        levelHandScale = 0.05f;
    }
    hand.obj->spheres[0].radius = hand.obj->spheres[0].targetRadius = levelHandScale * 13.0 / 0.05;
    hand.shadow->shadowSize = hand.obj->spheres[0].radius * 0.65;
    hand.shadow->altitude = -1.0f;
    hand.phys->turnSpeed = 0.6f;
    hand.phys->maxSpeed = 14.0f;
    hand.phys->moveSpeed = 7.0f;
    hand.phys->friction = 0.8f;
    hand.phys->minSpeed = 0.1f;
    hand.phys->slideLimit = 0.5f;
    hand.active = 1;
    hand.phys->mass = 10.0f;
    if (first) {
        handControl.locate[0] = NULL;
        handControl.locate[1] = NULL;
        hand.state->mode = HAND_IDLE;
        hand.state->prevMode = HAND_IDLE;
        memclear(&handControl.actionLock, sizeof(HandControl) - 8);
        handControl.status = HAND_NORMAL;
        memclear(&handControl.edgeProbe, sizeof(Actor));
        InitActorStructures(&handControl.edgeProbe, 1);
        memclear(&handControl.frontProbe, sizeof(Actor));
        InitActorStructures(&handControl.frontProbe, 3);
        handControl.frontProbe.radius = handControl.frontProbe.obj->spheres[0].targetRadius = handControl.frontProbe.obj->spheres[0].radius = 10.0f;
        if (gameInfo.level == SPACEBOSS3) {
            ApplyAction(HAND_NOTHING);
            *hand.phys = defaultBehaviour[ROBOT];
            hand.obj->spheres[0].radius = hand.obj->spheres[0].targetRadius = hand.phys->radius = 180.0f;
            hand.phys->mass = 999999.0f;
            handControl.status = HAND_ROBOT;
            hand.friction = handInertia = 0.92f;
            hand.phys->friction = 0.92f;
            FindSubObjectFromObject(hand.inst->obj, UpdateCRC("bot_foot_R"))->sphere = &hand.obj->spheres[1];
            hand.obj->spheres[1].flags = 0x1F;
            FindSubObjectFromObject(hand.inst->obj, UpdateCRC("bot_foot_L"))->sphere = &hand.obj->spheres[2];
            hand.obj->spheres[2].flags = 0x1F;
            FindSubObjectFromObject(hand.inst->obj, UpdateCRC("bot_sac01"))->sphere = &hand.obj->spheres[3];
            hand.obj->spheres[3].flags = 0x1F;
            FindSubObjectFromObject(hand.inst->obj, UpdateCRC("bot_hed_UFO"))->sphere = &hand.obj->spheres[4];
            hand.obj->spheres[4].flags = 0x1F;
            tempObj = FindSubObjectFromObject(hand.inst->obj, UpdateCRC("bot_hed_light"));
            tempObj->flags |= 0x40;
            hand.flags = 0x4000;
            ZERO_VECTOR(&hand.vel);
            ZERO_VECTOR(&hand.rot);
            hand.rotQuat = zeroQuat;
        }
    }
    player.castSpellType = 15;
    player.castSpell = 15;
    GetQuaternionFromXYZRot(&hand.rotQuat, &hand.rot);
}

void SetHandHopPosition(void) {
    Vec3 tempVect;
    Vec3 d;
    Quat tempQuat;
    Object *obj;
    f32 time;
    f32 tempFloat;
    f32 YTranslateAmount;
    f32 YScaleAmount;
    s32 i;
    s32 toKey;
    s32 fromKey;
    f32 dx;
    f32 dy;
    f32 dz;
    s16 lo;
    s16 high;

    obj = hand.link->inst->obj;
    fromKey = -1;
    toKey = -1;
    time = hand.link->anim->time + hand.link->anim->speed;
    if (obj->numPosKeys >= 2) {
        for (i = 0; i < obj->numPosKeys; i++) {
            if (time < obj->posKeys[i].time) {
                toKey = i;
                fromKey = i - 1;
                break;
            }
        }
        if (fromKey != -1) {
            tempFloat = abs(obj->posKeys[toKey].time - obj->posKeys[fromKey].time);
            tempFloat = (time - obj->posKeys[fromKey].time) / tempFloat;
        } else {
            tempFloat = 1.0f;
            toKey = fromKey = obj->numPosKeys - 1;
        }
        YTranslateAmount = obj->posKeys[fromKey].pos.y + (obj->posKeys[toKey].pos.y - obj->posKeys[fromKey].pos.y) * tempFloat;
    } else {
        YTranslateAmount = obj->posKeys[0].pos.y;
    }
    if (obj->numScaleKeys >= 2) {
        toKey = -1;
        fromKey = -1;
        for (i = 0; i < obj->numScaleKeys; i++) {
            if (time < obj->scaleKeys[i].time) {
                toKey = i;
                fromKey = i - 1;
                break;
            }
        }
        if (fromKey != -1) {
            tempFloat = abs(obj->scaleKeys[toKey].time - obj->scaleKeys[fromKey].time);
            tempFloat = (time - obj->scaleKeys[fromKey].time) / tempFloat;
        } else {
            tempFloat = 1.0f;
            toKey = fromKey = obj->numScaleKeys - 1;
        }
        YScaleAmount = obj->scaleKeys[fromKey].pos.y + (obj->scaleKeys[toKey].pos.y - obj->scaleKeys[fromKey].pos.y) * tempFloat;
    } else {
        YScaleAmount = obj->scaleKeys[0].pos.y;
    }
    if (obj->numRotKeys >= 2) {
        toKey = -1;
        fromKey = -1;
        lo = 0;
        high = obj->numRotKeys - 1;
        while (lo <= high) {
            i = (lo + high) >> 1;
            if (obj->rotKeys[i].time == time) {
                break;
            }
            if (time < obj->rotKeys[i].time) {
                high = i - 1;
            } else {
                lo = i + 1;
            }
        }
        if (lo > 0) {
            lo--;
        }
        for (i = lo; i < obj->numRotKeys; i++) {
            if (time < obj->rotKeys[i].time) {
                toKey = i;
                fromKey = i - 1;
                break;
            }
        }
        if (fromKey != -1) {
            tempFloat = abs(obj->rotKeys[toKey].time - obj->rotKeys[fromKey].time);
            tempFloat = (time - obj->rotKeys[fromKey].time) / tempFloat;
        } else {
            tempFloat = 1.0f;
            toKey = fromKey = obj->numRotKeys - 1;
        }
        QuatSlerp(&obj->rotKeys[fromKey].q, &obj->rotKeys[toKey].q, tempFloat, &tempQuat);
    }
    if (enemies[hand.link->state->index].brain.flags & 0x10) {
        tempVect.y = YScaleAmount * 4.0f + YTranslateAmount * 0.05;
    } else {
        tempVect.y = YScaleAmount * 21.0f + YTranslateAmount * 0.05;
    }
    tempVect.x = 0.0f;
    tempVect.z = -5.0f;
    RotateVectorByQuaternion(&tempVect, &tempVect, &tempQuat);
    RotateVector2D(&tempVect, &tempVect, hand.rot.y);
    ADD_TO_VECTOR(&tempVect, &hand.link->pos);
    dx = tempVect.x - hand.pos.x;
    dy = tempVect.y - hand.pos.y;
    dz = tempVect.z - hand.pos.z;
    d.x = dx;
    d.y = dy;
    d.z = dz;
    if (handControl.actionTime < 5) {
        d.x = dx * 0.5;
        d.y = dy * 0.5;
        d.z = dz * 0.5;
    } else {
        d.x = dx;
        d.y = dy;
        d.z = dz;
    }
    ADD_TO_VECTOR(&hand.pos, &d);
    hand.vel = hand.link->vel;
}

void DismountEnemy(void) {
    AnimState *anim;

    if (enemies[handControl.attachedTo->state->index].actor.type == DENIS) {
        NMEGetNewInstruction(&enemies[handControl.attachedTo->state->index], 7, 0);
    } else {
        NMEGetNewInstruction(&enemies[handControl.attachedTo->state->index], 1, 0);
    }
    enemies[handControl.attachedTo->state->index].brain.delayTimer = 0;
    enemies[handControl.attachedTo->state->index].brain.cooldownTimer = 50;
    if (handControl.attachedTo->type == SWISH) {
        anim = handControl.attachedTo->anim;
        if (anim->current == 15) {
            anim->speed = anim->anims[15].speed;
        } else {
            anim->queueSpeed[0] = anim->anims[15].speed;
        }
    } else if (handControl.attachedTo->type == DENIS) {
        AnimateActor(handControl.attachedTo, 5, 1, 0, 0.0f);
    }
    handControl.releaseTimer = 5;
    handControl.attachedTo = NULL;
    handControl.status = HAND_NORMAL;
    ApplyAction(HAND_JUMP);
    hand.state->mode = HAND_RELEASE;
    hand.vel.y *= 0.5;
    AccelerateActor2D(&hand, -10.0f, hand.phys->maxSpeed);
    pad[ActiveController].pressed[BUTTON_Z] = 0;
    cameraInfo.focus = &hand.pos;
    cameraInfo.focusVel = &hand.vel;
    hand.vel.y = 5.0f;
    hand.flags |= 0x100;
}

void DismountBall(void) {
    ApplyAction(HAND_JUMP);
    hand.state->mode = HAND_RELEASE;
    hand.vel.y *= 0.5;
    AccelerateActor2D(&hand, -10.0f, hand.phys->maxSpeed);
    pad[ActiveController].pressed[BUTTON_Z] = 0;
}

void HandProcessFroggy(void) {
    if (handControl.frog) {
        if (handControl.spellTimer[2] < 50) {
            SCALE_VECTOR(&hand.scale, 0.5);
            if (hand.scale.x <= 0.001) {
                DefrogHand();
            }
        } else if (hand.scale.x < enemyStats[NME_FROG].scale - 0.001) {
            hand.scale.x += (enemyStats[NME_FROG].scale - hand.scale.x) / 2.0f;
            hand.scale.y = hand.scale.x;
            hand.scale.z = hand.scale.x;
        } else {
            hand.scale.x = hand.scale.y = hand.scale.z = enemyStats[NME_FROG].scale;
        }
    } else if (handControl.spellTimer[2] < 50) {
        if (hand.scale.x < 0.049) {
            hand.scale.x += (0.05 - hand.scale.x) / 2.0;
            hand.scale.y = hand.scale.x;
            hand.scale.z = hand.scale.x;
        } else {
            handControl.spellTimer[2] = 1;
            hand.scale.x = hand.scale.y = hand.scale.z = 0.05f;
        }
    } else {
        SCALE_VECTOR(&hand.scale, 0.5);
        if (hand.scale.x <= 0.001) {
            SubActor(&hand);
            handControl.frog = 1;
            hand.inst = frogObj;
            FlashScreen(0, 200, 0, 250, -30);
            *hand.phys = defaultBehaviour[NME_FROG];
            hand.friction = handInertia;
            hand.flags = hand.phys->flags;
            hand.obj->spheres[0].radius = hand.obj->spheres[0].targetRadius = hand.phys->radius;
            MakeUniqueActor(&hand, 58);
            InitActorAnim(&hand);
            AddActor(&hand);
            ZERO_VECTOR(&hand.scale);
            hand.rot.x = hand.rotAim.x = hand.rot.z = hand.rotAim.z = 0.0f;
            hand.state->squash = 0.0f;
            handControl.actionLock = 0;
            ApplyAction(HAND_FROG_IDLE);
            hand.shadow->shadowSize = 8.0f;
            handControl.actionLock = 0;
        }
    }
}

void DefrogHand(void) {
    s32 i;

    cheatActivated &= ~0x100000;
    SubActor(&hand);
    RemoveUniqueActor(&hand, NME_FROG);
    FindObject(&hand.inst, UpdateCRC("goodglov.ndo"), "goodglov.ndo", 1);
    handControl.frog = 0;
    ResetHand(0);
    InitActorAnim(&hand);
    AddActor(&hand);
    ZERO_VECTOR(&hand.scale);
    ApplyActionIdle();
    for (i = 0; i < numberOfEnemies; i++) {
        enemies[i].brain.flags &= ~0x400;
        if (enemies[i].actor.active == 10) {
            enemies[i].actor.active = 1;
        }
    }
    if (tongueBullet) {
        tongueBullet->actor.lifeTimer = 1;
        tongueBullet = NULL;
    }
}

s32 HandOnTrampoline(void) {
    if (hand.obj->platform != NULL && hand.obj->platform->bounce != 0.0f) {
        return TRUE;
    }
    return FALSE;
}

void CreateBlastCircle(void) {
    Plane *plane;
    s32 index;
    Footprint *footprint;
    s32 blastTime;

    FindFlattestPlane(&hand, &plane, &index);
    footprint = JallocAlloc(sizeof(Footprint), 1, "Footprnt");
    footprint->parent = hand.obj->platform;
    blastTime = player.powerups[HERCULES] ? 15 : 10;
    if (SetupFootprint(footprint, &hand.pos, (Vec3 *)plane, blastTime / 2 + 1, blastTime, blastTex, 6, 0x800)) {
        footprint->pos.y -= hand.radius / 1.1;
        footprint->alpha = 150;
        footprint->maxAlpha = 0;
        footprint->width = blastTime / 10;
        footprint->alphaStep = footprint->alpha / (f32)blastTime;
        footprint->b = footprint->g = footprint->r = 255;
        AddFootprint(footprint);
        handControl.footprint = footprint;
        handControl.blastPos = footprint->pos;
        handControl.blastSize = blastTime;
        StartEarthquakeAtPoint(&hand.pos, 10, blastTime * 100, blastTime * 25, 1, 14);
    }
}

void UpdateBlastCircle(void) {
    Vec3 tempVect;
    f32 range;
    f32 blastRadius;
    s32 j;

    if (handControl.blastSize <= 0 || handControl.footprint == NULL) {
        return;
    }
    if (handControl.footprint->parent != NULL) {
        ADD_VECTORS(&tempVect, &handControl.blastPos, &handControl.footprint->parent->actor.pos);
    } else {
        tempVect = handControl.blastPos;
    }
    range = handControl.footprint->length * bcfac;
    blastRadius = handControl.footprint->width * bcfac;
    handControl.blastSize--;
    if (ball.obj->onGround && ball.obj->planes[0] && ball.obj->resting[0]
        && ABS(ball.pos.y - tempVect.y - ball.shadow->altitude) <= 10.0f
        && DistanceBetweenPoints2D(&ball.pos, &tempVect) <= blastRadius) {
        ball.vel.y = ((range - blastRadius) / 8.0f + 1.0f) * 10.0f / 10.0f;
    }
    for (j = 0; j < numberOfEnemies; j++) {
        if (!(enemies[j].brain.flags & 0x20) && enemies[j].actor.active == 1) {
            if ((!(enemies[j].actor.flags & 0x100) && enemies[j].actor.pos.y > tempVect.y
                 && ABS(enemies[j].actor.pos.y - tempVect.y - enemies[j].actor.radius) <= 25.0f)
                || ABS(enemies[j].actor.pos.y - tempVect.y - enemies[j].actor.shadow->altitude) <= 10.0f) {
                if (DistanceBetweenPoints2D(&enemies[j].actor.pos, &tempVect) <= blastRadius) {
                    NMEHitByBlastRing(&enemies[j]);
                }
            }
        }
    }
}

s32 HandSnatchAllowed(void) {
    s32 j;
    f32 temp;

    if (ball.link) {
        return FALSE;
    }
    if (ballControl.state == BALL_BURST) {
        return FALSE;
    }
    if (!ball.obj->onGround) {
        return FALSE;
    }
    if (hand.obj->onGround) {
        for (j = 0; j < 4; j++) {
            if (hand.obj->planes[j] != NULL) {
                temp = DOT_PRODUCT(&hand.pos, hand.obj->planes[j]) + hand.obj->planes[j]->d;
                if (temp * (DOT_PRODUCT(&ball.pos, hand.obj->planes[j]) + hand.obj->planes[j]->d) < 0.0f) {
                    return FALSE;
                }
            }
        }
        if (DistanceBetweenPointsSquared(&hand.pos, &ball.pos) < 1600.0f) {
            return TRUE;
        }
    }
    return FALSE;
}

void AddHandLocateSprite(SpriteAnim *anim) {
    s32 j;

    for (j = 0; j < 2; j++) {
        if (handControl.locate[j]) {
            SubSprite(handControl.locate[j]);
            JallocFree(&handControl.locate[j]);
        }
        handControl.locate[j] = JallocAlloc(sizeof(GameSprite), 1, "Spr");
        if (j == 1) {
            InitSpriteAnimation(handControl.locate[j], anim, 0);
            handControl.locate[j]->offsetX = -14;
            handControl.locate[j]->offsetY = -21;
        } else {
            FindTexture(&handControl.locate[j]->tex, UpdateCRC("thought.bmp"), 1);
            handControl.locate[j]->offsetX = -handControl.locate[j]->tex->width / 2;
            handControl.locate[j]->offsetY = -handControl.locate[j]->tex->height / 2;
        }
        handControl.locate[j]->alpha = 255;
        handControl.locate[j]->flags = 1;
        handControl.locate[j]->r = 255;
        handControl.locate[j]->g = 255;
        handControl.locate[j]->b = 255;
        handControl.locate[j]->scaleX = 0;
        handControl.locate[j]->scaleY = 0;
        handControl.locate[j]->anim.def.startScale = 0;
        handControl.locate[j]->anim.def.endScale = 0;
    }
    AddSprite(handControl.locate[0], NULL);
    AddSprite(handControl.locate[1], handControl.locate[0]);
}

void CheckForFootsteps(void) {
    if (!hand.obj->onGround && !shrinkExitTimer) {
        return;
    }
    if (QueryAnimTime(&hand, 27.0f) || QueryAnimTime(&hand, 69.0f) || QueryAnimTime(&hand, 85.0f)) {
        if (hand.state->waterState
            || (hand.obj->planes[0] != NULL && ((CollFace *)hand.obj->planes[0])->terrain == 5)) {
            PlaySample(3, &hand.pos, 50, (Random(20) + 140));
        } else if (hand.flags & 0x8000) {
            PlaySample(313, &hand.pos, 130, 128);
        } else if (shrinkExitTimer) {
            PlaySample(148, &hand.pos, (shrinkExitTimer * 3 + 40), (Random(36) + 110));
        } else {
            PlaySample(148, &hand.pos, 130, (Random(56) + 100));
        }
    }
    if (QueryAnimTime(&hand, 58.0f) || QueryAnimTime(&hand, 76.0f) || QueryAnimTime(&hand, 90.0f)) {
        if (hand.state->waterState
            || (hand.obj->planes[0] != NULL && ((CollFace *)hand.obj->planes[0])->terrain == 5)) {
            PlaySample(3, &hand.pos, 50, (Random(20) + 140));
        } else if (hand.flags & 0x8000) {
            PlaySample(313, &hand.pos, 130, 128);
        } else if (shrinkExitTimer) {
            PlaySample(149, &hand.pos, (shrinkExitTimer * 3 + 40), (Random(36) + 110));
        } else {
            PlaySample(149, &hand.pos, 130, (Random(56) + 100));
        }
    }
    if (QueryAnimTime(&hand, 77.0f) || QueryAnimTime(&hand, 15.0f) || QueryAnimTime(&hand, 86.0f)) {
        AddGlovePrint(-4);
    }
    if (QueryAnimTime(&hand, 69.0f) || QueryAnimTime(&hand, 44.0f) || QueryAnimTime(&hand, 91.0f)) {
        AddGlovePrint(4);
    }
}

void AddTongueBit(Vec3 *pos) {
    Debris *d = AddDebris(TRAJ, pos, &zero, 0, 0);

    if (d) {
        d->sprite.g = 0;
        d->sprite.b = 0;
        d->sprite.flags = 0;
        d->lifeTimer = 1;
        d->sprite.anim.def.endScale = d->sprite.anim.def.startScale = d->sprite.scaleY = d->sprite.scaleX = tongueScale;
        SubSprite(&d->sprite);
        AddSprite(&d->sprite, 0);
    }
}

void PlaySpellSample(void) {
    PlaySample(castSpellFX + 265, &hand.pos, 100, 138);
    castSpellFX = (castSpellFX + 1) % 4;
}

void ApplyActionIdle(void) {
    ApplyAction(HAND_IDLE);
    if (!hand.obj->onGround) {
        hand.state->mode = HAND_JUMP;
    }
}

INCLUDE_ASM("asm/nonmatchings/hand", pad_801779BC);

INCLUDE_RODATA("asm/nonmatchings/hand", pad_80108568);
