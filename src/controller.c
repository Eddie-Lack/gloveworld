#include "common.h"
#include "ident.h"
#include "controller.h"
#include "actor.h"
#include "camera.h"
#include "maths.h"
#include "game.h"
#include "jalloc.h"
#include "hand.h"
#include "libc.h"
#include "save.h"
#include "ball.h"
#include "eeprom.h"

extern OSMesgQueue serialMsgQ;
extern OSMesg serialMsg;
extern OSMesgQueue controllerMsgQ;
extern OSMesg controllerMsgBuf[8];
extern OSContStatus statusdata[4];
extern u8 controllerPresent;
extern s32 controlMethod;
extern f32 plasterScaleAim;
extern f32 camOrigRotY;
extern f32 cdist;
extern f32 cameraZoomVal[];
extern OSContPad controllerdata[];
extern u8 ActiveController;
extern u16 buttonFlag[14];
extern u16 *recordKeys;
extern u16 *rkp;
extern s16 nextKeyFrame;
extern s16 exitCount;
extern u16 playbackButtons;
extern s32 prevFrame2;
extern s32 autoPlayback;
extern s32 frameCount;
void ReadController(s32 cntlr);

// VEL in DealWithMaxSpeed indexes the velocity as an array. This changes the -O0 address arithmetic.
typedef struct VecArray {
    f32 v[3];
} VecArray;

extern u8 readingTip;
extern u8 pauseMode;
extern s32 cntrl;
extern u16 recordCode[2];
extern u16 sameCount;
extern s32 prevFrame;
extern s32 gameOverCount;
extern s16 currentSaveSlot;
extern u8 disablePause;
extern f32 camxrotaimval;
extern f32 cspeed;
extern s32 aimMode;
extern f32 aimMag;
extern f32 pwf;
extern f32 slamscalespeed;
extern f32 handSlideFactor;
extern f32 handhoppingfactor;
extern f32 handSlide2;
extern f32 handInertia;
extern f32 gravity;
void AccelerateActorAcrossPlane(Actor *actor, Plane *plane, f32 accel, f32 maxSpeed, f32 rot, s32 is3D);
void DealWithMaxSpeed(Actor *actor, f32 maxSpeed, f32 oldSpeed);

s32 initControllers(void) {
    s32 i;
    s32 j;
    u8 pattern;
    s32 cont = -1;
    s32 unused; // forces the original stack frame size

    osCreateMesgQueue(&serialMsgQ, &serialMsg, 1);
    osSetEventMesg(OS_EVENT_SI, &serialMsgQ, (OSMesg)1);
    osContInit(&serialMsgQ, &pattern, statusdata);
    osCreateMesgQueue(&controllerMsgQ, controllerMsgBuf, 8);
    osSetEventMesg(OS_EVENT_SI, &controllerMsgQ, (OSMesg)0);
    Wait(10000);
    for (i = 3; i >= 0; i--) {
        for (j = 0; j < 14; j++) {
            pad[i].sincePress[j] = 999999;
        }
        if (((pattern >> i) & 1) && !(statusdata[i].errno & CONT_NO_RESPONSE_ERROR)) {
            pad[i].present = TRUE;
            cont = i;
        }
    }
    if (cont != 0) {
        controllerPresent = FALSE;
        cont = 0;
    } else {
        controllerPresent = TRUE;
    }
    return cont;
}

void ReadController(s32 cntlr) {
    s32 i;
    s32 stick;

    pad[cntlr].prevMag2 = pad[cntlr].prevMag;
    pad[cntlr].prevMag = pad[cntlr].mag;
    if (gameInfo.mode != 5 && !readingTip && !pauseMode && handControl.noControlTimer && cntlr == ActiveController
        && (gameInfo.cameoActive || (gameInfo.level != FLYTHRU && gameInfo.level != FLYTHRU2 && gameInfo.level != CARNIVALBONUS && gameInfo.level != SPACEBOSS3))) {
        pad[cntlr].stickX = 0;
        pad[cntlr].stickY = 0;
        pad[cntlr].mag = 0.0f;
        for (i = 0; i < 14; i++) {
            if (i != 3) {
                pad[cntlr].prevButton[i] = pad[cntlr].button[i];
                pad[cntlr].button[i] = pad[cntlr].pressed[i] = 0;
            } else {
                pad[cntlr].prevButton[i] = pad[cntlr].button[i];
                pad[cntlr].button[i] = pad[cntlr].pressed[i] = 0;
                if (controllerdata[cntlr].button & buttonFlag[i]) {
                    if (!pad[cntlr].latch[i]) {
                        pad[cntlr].button[i] = 1;
                    }
                } else {
                    pad[cntlr].latch[i] = 0;
                }
                if (pad[cntlr].button[i] && !pad[cntlr].prevButton[i]) {
                    pad[cntlr].pressed[i] = 1;
                }
            }
        }
        return;
    }
    stick = MAX(controllerdata[cntlr].stick_x, -79);
    stick = MIN(stick, 79);
    if (abs(stick) > 5) {
        if (stick > 0) {
            pad[cntlr].stickX = stick - 5;
        } else {
            pad[cntlr].stickX = stick + 5;
        }
    } else {
        pad[cntlr].stickX = 0;
    }
    stick = MAX(controllerdata[cntlr].stick_y, -79);
    stick = MIN(stick, 79);
    if (abs(stick) > 5) {
        if (stick > 0) {
            pad[cntlr].stickY = stick - 5;
        } else {
            pad[cntlr].stickY = stick + 5;
        }
    } else {
        pad[cntlr].stickY = 0;
    }
    pad[cntlr].angle[3] = pad[cntlr].angle[2];
    pad[cntlr].angle[2] = pad[cntlr].angle[1];
    pad[cntlr].angle[1] = pad[cntlr].angle[0];
    for (i = 0; i < 14; i++) {
        pad[cntlr].prevButton[i] = pad[cntlr].button[i];
        pad[cntlr].button[i] = pad[cntlr].pressed[i] = 0;
        if (controllerdata[cntlr].button & buttonFlag[i]) {
            if (!pad[cntlr].latch[i]) {
                pad[cntlr].button[i] = 1;
            }
        } else {
            pad[cntlr].latch[i] = 0;
        }
        if (pad[cntlr].button[i] && !pad[cntlr].prevButton[i]) {
            pad[cntlr].pressed[i] = 1;
        }
        if (pad[cntlr].prevButton[i] == pad[cntlr].button[i]) {
            pad[cntlr].holdTime[i]++;
        } else {
            pad[cntlr].lastHoldTime[i] = pad[cntlr].holdTime[i];
            pad[cntlr].holdTime[i] = 0;
        }
        if (pad[cntlr].pressed[i]) {
            pad[cntlr].sincePress[i] = 0;
        } else {
            pad[cntlr].sincePress[i]++;
        }
    }
    if (cntlr == ActiveController && pad[cntlr].stickX == 0 && pad[cntlr].stickY == 0) {
        if (pad[cntlr].button[BUTTON_DPAD_UP]) {
            pad[cntlr].stickY = 79;
        } else if (pad[cntlr].button[BUTTON_DPAD_DOWN]) {
            pad[cntlr].stickY = -79;
        }
        if (pad[cntlr].button[BUTTON_DPAD_LEFT]) {
            pad[cntlr].stickX = -79;
        } else if (pad[cntlr].button[BUTTON_DPAD_RIGHT]) {
            pad[cntlr].stickX = 79;
        }
    }
    pad[cntlr].mag = sqrtf(pad[cntlr].stickX * pad[cntlr].stickX + pad[cntlr].stickY * pad[cntlr].stickY);
    if (pad[cntlr].mag > 0.0f) {
        pad[cntlr].angle[0] = atan2(pad[cntlr].stickX, pad[cntlr].stickY);
        pad[cntlr].angle[0] = Aabs(pad[cntlr].angle[0]);
    }
    if (frameCount < 10) {
        sameCount = 0;
    }
    if (autoPlayback == 1 && cntrl == ActiveController && frameCount >= 10) {
        if (frameCount == prevFrame) {
            return;
        }
        prevFrame = frameCount;
        recordCode[1] = recordCode[0];
        recordCode[0] = 0;
        for (i = 0; i < 14; i++) {
            if (pad[cntrl].button[i] || pad[cntlr].pressed[i]) {
                recordCode[0] |= 1 << i;
            }
        }
        if (recordCode[1] != recordCode[0]) {
            if (recordKeys != NULL) {
                *rkp = frameCount;
                rkp++;
                *rkp = controllerdata[cntlr].button;
                rkp++;
            }
            sameCount = 0;
            return;
        }
        sameCount++;
    }
}

void ReadControllerFromData(s32 cntlr) {
    s32 i;

    if (!GameBeingPlayed()) {
        ReadController(cntlr);
        return;
    }
    if (frameCount < 9) {
        if (recordKeys != NULL) {
            rkp = recordKeys;
            nextKeyFrame = *rkp;
            rkp++;
        }
        return;
    }
    if (frameCount == prevFrame2) {
        return;
    }
    prevFrame2 = frameCount;
    if (controllerPresent && gameInfo.level != SPACEBOSS_TWEEN && gameInfo.fade == 0 && gameInfo.starZ == 0) {
        if ((controllerdata[ActiveController].button & buttonFlag[0])
            || (controllerdata[ActiveController].button & buttonFlag[1])
            || (controllerdata[ActiveController].button & buttonFlag[3])) {
            ExitLevel(QUIT_EXIT, 1, 0);
            JallocFree(&recordKeys);
            rkp = NULL;
            autoPlayback = 0;
            nextKeyFrame = -1;
            playbackButtons = 0;
        }
    }
    if (nextKeyFrame == frameCount) {
        playbackButtons = *rkp;
        rkp++;
        nextKeyFrame = *rkp;
        rkp++;
        if (nextKeyFrame == -1) {
            exitCount = 20;
        }
    }
    if (exitCount) {
        exitCount--;
        if (exitCount == 0) {
            if (gameInfo.level == SPACEBOSS_TWEEN) {
                ExitLevel(SUB_LEVEL_EXIT, 1, 0);
            } else {
                ExitLevel(QUIT_EXIT, 1, 0);
            }
            JallocFree(&recordKeys);
            rkp = NULL;
            autoPlayback = 0;
            playbackButtons = 0;
            return;
        }
    }
    controllerdata[cntlr].button = playbackButtons;
    pad[cntlr].stickX = 0;
    pad[cntlr].stickY = 0;
    pad[cntlr].angle[3] = pad[cntlr].angle[2];
    pad[cntlr].angle[2] = pad[cntlr].angle[1];
    pad[cntlr].angle[1] = pad[cntlr].angle[0];
    for (i = 0; i < 14; i++) {
        pad[cntlr].prevButton[i] = pad[cntlr].button[i];
        pad[cntlr].button[i] = pad[cntlr].pressed[i] = 0;
        if (controllerdata[cntlr].button & buttonFlag[i]) {
            if (!pad[cntlr].latch[i]) {
                pad[cntlr].button[i] = 1;
            }
        } else {
            pad[cntlr].latch[i] = 0;
        }
        if (pad[cntlr].button[i] && !pad[cntlr].prevButton[i]) {
            pad[cntlr].pressed[i] = 1;
        }
        if (pad[cntlr].prevButton[i] == pad[cntlr].button[i]) {
            pad[cntlr].holdTime[i]++;
        } else {
            pad[cntlr].lastHoldTime[i] = pad[cntlr].holdTime[i];
            pad[cntlr].holdTime[i] = 0;
        }
        if (pad[cntlr].pressed[i]) {
            pad[cntlr].sincePress[i] = 0;
        } else {
            pad[cntlr].sincePress[i]++;
        }
    }
    if (pad[cntlr].stickX == 0 && pad[cntlr].stickY == 0) {
        if (pad[cntlr].button[BUTTON_DPAD_UP]) {
            pad[cntlr].stickY = 79;
        } else if (pad[cntlr].button[BUTTON_DPAD_DOWN]) {
            pad[cntlr].stickY = -79;
        }
        if (pad[cntlr].button[BUTTON_DPAD_LEFT]) {
            pad[cntlr].stickX = -79;
        } else if (pad[cntlr].button[BUTTON_DPAD_RIGHT]) {
            pad[cntlr].stickX = 79;
        }
    }
    pad[cntlr].mag = sqrtf(pad[cntlr].stickX * pad[cntlr].stickX + pad[cntlr].stickY * pad[cntlr].stickY);
    if (pad[cntlr].mag > 0.0f) {
        pad[cntlr].angle[0] = atan2(pad[cntlr].stickX, pad[cntlr].stickY);
        pad[cntlr].angle[0] = Aabs(pad[cntlr].angle[0]);
    }
}

void ActivateCameraControlMode(void) {
    if (hand.state->mode == HAND_WALK) {
        ApplyAction(HAND_IDLE);
    }
    controlMethod = CONTROL_CAMERA;
    cameraInfo.exitingControlMode = 0;
    cameraInfo.mode = 0;
    camera.rotAim.y = camera.rot.y;
    camera.phys->rotAim[1] = TRUE;
    plasterScaleAim = 0.002f;
    camera.obj->spheres[0].radius = camera.obj->spheres[0].targetRadius = 5.0f;
    camOrigRotY = camera.rot.y;
}

void DeactivateCameraControlMode(void) {
    Vec3 tempVect;

    tempVect.z = cdist * ((cameraZoomVal[2] < 0.5 ? 0.5 : cameraZoomVal[2]) < 1.0 ? (cameraZoomVal[2] < 0.5 ? 0.5 : cameraZoomVal[2]) : 1.0);
    tempVect.x = tempVect.y = 0.0f;
    RotateVector2DXYZ(&tempVect, &tempVect, -camera.rot.x, 0);
    RotateVector2D(&tempVect, &tempVect, camera.rot.y);
    ADD_VECTORS(&cameraInfo.actor.pos, &camera.pos, &tempVect);
    controlMethod = CONTROL_HAND;
    cameraInfo.mode = 1;
    camera.phys->rotAim[1] = FALSE;
    camera.phys->turnSpeed = 0.05f;
    cameraInfo.sideOffset = 0;
    cameraInfo.followSpeed = 1.0f;
    plasterScaleAim = 0.003f;
    camera.obj->spheres[0].radius = camera.obj->spheres[0].targetRadius = 40.0f;
}

void ProcessInput(void) {
    f32 a = 0.0f;
    f32 maxSpeed;
    f32 oldSpeed;
    f32 dist;
    Vec3 tempVect;
    Vec3 vect;
    Plane *plane;
    s32 index;

    if (pad[ActiveController].pressed[BUTTON_START] && !pauseMode && !gameInfo.fade && !gameOverCount && !disablePause) {
        PlaySampleNot3D(179, 120, 128, 128);
        pauseMode = TRUE;
    }
    if (handControl.noControlTimer) {
        return;
    }
    if (hand.state->mode != HAND_SWISH && (handControl.suctionPlane == NULL || handControl.suctionPlane->y > 0.0f)
        && gameInfo.level != SPACEBOSS3
        && (hand.obj->onGround || hand.state->mode == HAND_CLIMBDOWN || hand.state->mode == HAND_CLING
            || (ball.link == &hand
                && (ball.obj->onGround || ((ball.flags & 0x200000) && ball.state->waterState))))
        && (controlMethod == CONTROL_HAND || controlMethod == CONTROL_BALL)) {
        if (pad[ActiveController].pressed[BUTTON_C_UP]) {
            ActivateCameraControlMode();
        }
    } else if (camera.phys->rotAim[1] == 2) {
        if (pad[ActiveController].pressed[BUTTON_C_DOWN] || pad[ActiveController].pressed[BUTTON_C_UP] || pad[ActiveController].pressed[BUTTON_A]
            || pad[ActiveController].pressed[BUTTON_B] || pad[ActiveController].pressed[BUTTON_Z]) {
            cameraInfo.exitingControlMode = 1;
            camera.phys->turnSpeed = 0.5f;
            if (pad[ActiveController].pressed[BUTTON_C_DOWN]) {
                pad[ActiveController].latch[BUTTON_C_DOWN] = 1;
                pad[ActiveController].button[BUTTON_C_DOWN] = 0;
                pad[ActiveController].pressed[BUTTON_C_DOWN] = 0;
                pad[ActiveController].sincePress[BUTTON_C_DOWN] = 999999;
            }
            if (pad[ActiveController].pressed[BUTTON_A]) {
                pad[ActiveController].latch[BUTTON_A] = 1;
                pad[ActiveController].button[BUTTON_A] = 0;
                pad[ActiveController].pressed[BUTTON_A] = 0;
                pad[ActiveController].sincePress[BUTTON_A] = 999999;
            }
            if (pad[ActiveController].pressed[BUTTON_B]) {
                pad[ActiveController].latch[BUTTON_B] = 1;
                pad[ActiveController].button[BUTTON_B] = 0;
                pad[ActiveController].pressed[BUTTON_B] = 0;
                pad[ActiveController].sincePress[BUTTON_B] = 999999;
            }
            if (pad[ActiveController].pressed[BUTTON_Z]) {
                pad[ActiveController].latch[BUTTON_Z] = 1;
                pad[ActiveController].button[BUTTON_Z] = 0;
                pad[ActiveController].pressed[BUTTON_Z] = 0;
                pad[ActiveController].sincePress[BUTTON_Z] = 999999;
            }
        }
    }
    if (controlMethod == CONTROL_CAMERA) {
        ZERO_VECTOR(&handControl.inputAccel);
        vect.z = -20.0f;
        vect.x = 0.0f;
        vect.y = 15.0f;
        RotateVector2DXYZ(&tempVect, &vect, -camera.rot.x, 0);
        RotateVector2D(&vect, &tempVect, camera.rot.y);
        vect.x += hand.pos.x;
        vect.y += hand.pos.y;
        vect.z += hand.pos.z;
        tempVect.x = vect.x - camera.pos.x;
        tempVect.y = vect.y - camera.pos.y;
        tempVect.z = vect.z - camera.pos.z;
        dist = LENGTH(&tempVect);
        if (camera.phys->rotAim[1] == 2) {
            if (cameraInfo.exitingControlMode) {
                camera.rotAim.x = camxrotaimval;
                camera.rotAim.y = camOrigRotY;
                if (ABS(camera.rot.x - camera.rotAim.x) < 0.1 && ABS(camera.rot.y - camera.rotAim.y) < 0.1) {
                    DeactivateCameraControlMode();
                    cameraInfo.exitingControlMode = 0;
                }
            } else {
                oldSpeed = FindShortestAngleSigned(camera.rotAim.y, hand.rot.y);
                camera.rotAim.y -= pad[ActiveController].stickX / 800.0f;
                oldSpeed = camera.rotAim.x;
                camera.rotAim.x = Aabs(camera.rotAim.x - pad[ActiveController].stickY / 800.0f);
                if (camera.rotAim.x > 0.6 && camera.rotAim.x < 4.962388981) {
                    if (oldSpeed <= 2.8559933218181817) {
                        camera.rotAim.x = 0.6f;
                    } else {
                        camera.rotAim.x = 4.962389f;
                    }
                }
                if (!cameraInfo.exitingControlMode) {
                    s32 j;

                    vect.z = -20.0f;
                    vect.x = 0.0f;
                    vect.y = 15.0f;
                    RotateVector2DXYZ(&tempVect, &vect, -camera.rotAim.x, 0);
                    RotateVector2D(&vect, &tempVect, camera.rotAim.y);
                    ADD_TO_VECTOR(&vect, &hand.pos);
                    SUB_VECTORS(&tempVect, &vect, &camera.pos);
                    dist = LENGTH(&tempVect);
                    for (j = 0; j < 4; j++) {
                        if (camera.obj->planes[j] != NULL
                            && DOT_PRODUCT(&tempVect, camera.obj->planes[j]) < -0.1) {
                            camera.rotAim.x = camera.rot.x;
                            camera.rotAim.y = camera.rot.y;
                            return;
                        }
                    }
                }
            }
        } else if (dist > 20.0f) {
            camera.rotAim.y = hand.rot.y;
            camera.rotAim.x = 6.07f;
            camera.phys->turnSpeed = MIN(20.0f / dist, 0.2);
        } else {
            camera.phys->turnSpeed = 0.1f;
            camera.phys->rotAim[1] = 2;
        }
        vect.z = -20.0f;
        vect.x = 0.0f;
        vect.y = 15.0f;
        RotateVector2DXYZ(&tempVect, &vect, -camera.rot.x, 0);
        RotateVector2D(&vect, &tempVect, camera.rot.y);
        vect.x += hand.pos.x;
        vect.y += hand.pos.y;
        vect.z += hand.pos.z;
        tempVect.x = vect.x - camera.pos.x;
        tempVect.y = vect.y - camera.pos.y;
        tempVect.z = vect.z - camera.pos.z;
        dist = LENGTH(&tempVect);
        if (dist > cspeed) {
            AddToOneScaledVector(&camera.pos, &tempVect, cspeed / dist);
        } else {
            camera.pos = vect;
        }
    }
    if (controlMethod == CONTROL_HAND || controlMethod == CONTROL_BALL) {
        if (pad[ActiveController].mag >= 5.0f) {
            if (handControl.suctionPlane != NULL && handControl.suctionPlane->y == -1.0f) {
                pad[ActiveController].stickX = -pad[ActiveController].stickX;
                pad[ActiveController].angle[0] = atan2(pad[ActiveController].stickX, pad[ActiveController].stickY);
                pad[ActiveController].angle[0] = Aabs(pad[ActiveController].angle[0]);
            }
            if (aimMode == 1 && (hand.state->mode == HAND_PREWHACK || hand.state->mode == HAND_THROWAIM)) {
                hand.rotAim.y = Aabs(pad[ActiveController].stickX / pwf + hand.rotAim.y);
            } else if ((hand.state->mode != HAND_THROWAIM && hand.state->mode != HAND_PREWHACK)
                       || (pad[ActiveController].prevMag2 > aimMag && pad[ActiveController].mag > aimMag)) {
                if (hand.state->mode != HAND_NOTHING && hand.state->mode != HAND_HURT
                    && !(hand.state->mode == HAND_FROG_JUMP && !hand.obj->onGround)
                    && hand.state->mode != HAND_DUCK && hand.state->mode != HAND_CLIMBDOWN && hand.state->mode != HAND_CLING
                    && hand.state->mode != HAND_SKIDDING && hand.state->mode != HAND_SLIDING && hand.state->mode != HAND_FROG_TONGUE
                    && !(hand.state->mode == HAND_CASTSPELL && player.castSpell == 15)) {
                    hand.rotAim.y = Aabs(-pad[ActiveController].angle[0] + camera.rot.y);
                }
            }
            if (hand.state->mode == HAND_PREWHACK || hand.state->mode == HAND_THROWAIM) {
                hand.phys->turnSpeed = pad[ActiveController].mag / 80.0f * 0.6;
            }
            hand.friction = handInertia;
        } else if (hand.state->mode == HAND_THROWAIM || hand.state->mode == HAND_PREWHACK) {
            hand.rotAim.y = hand.rot.y;
        }
        if (hand.state->mode == HAND_CATCH || hand.state->mode == HAND_ROTOR || hand.state->mode == HAND_WALK
            || hand.state->mode == HAND_JUMP || hand.state->mode == HAND_PUSH || hand.state->mode == HAND_SLIDING
            || hand.state->mode == HAND_CRAWL || hand.state->mode == HAND_HOPPING || hand.state->mode == HAND_SLAM) {
            tempVect.x = 0.0f;
            tempVect.y = 0.0f;
            tempVect.z = hand.phys->accel * 10.0f / hand.phys->mass * pad[ActiveController].mag / 180.0f;
            a = Aabs(camera.rot.y - pad[ActiveController].angle[0]);
            RotateVector2D(&tempVect, &tempVect, a);
            if (hand.state->mode == HAND_CATCH) {
                SCALE_VECTOR_2D(&tempVect, 0.8);
            } else if (hand.state->mode == HAND_SLAM) {
                SCALE_VECTOR_2D(&tempVect, slamscalespeed);
            } else if (hand.state->mode == HAND_SLIDING) {
                if ((handControl.steepSlide || handControl.gentleSlide) && hand.obj->planes[0] != NULL) {
                    SetVector2D(&vect, (Vec3 *)hand.obj->planes[0]);
                } else {
                    SetVector2D(&vect, &hand.vel);
                }
                MakeUnit2D(&vect);
                dist = tempVect.x * vect.x + tempVect.z * vect.z;
                if (dist > 0.0f) {
                    SCALE_VECTOR_2D(&tempVect, handSlideFactor);
                    SCALE_VECTOR_2D(&vect, handSlideFactor);
                } else {
                    SCALE_VECTOR_2D(&tempVect, 1.0f - (1.0f - handSlideFactor) / 2.0f);
                    SCALE_VECTOR_2D(&vect, 1.0f - (1.0f - handSlideFactor) / 2.0f);
                }
                SCALE_VECTOR_2D(&vect, dist);
                SubFromVector2D(&tempVect, &vect);
            }
            if (hand.state->mode == HAND_ROTOR) {
                SCALE_VECTOR_2D(&tempVect, handControl.rotorSpeed / gravity);
                if (ball.link == &hand) {
                    SCALE_VECTOR_2D(&tempVect, hand.phys->mass / (hand.phys->mass + ball.phys->mass / 2.0f));
                }
            }
            if (hand.obj->planes[0] != NULL) {
                if (terrain[((CollFace *)hand.obj->planes[0])->terrain].slip > 1.0f) {
                    tempVect.x /= terrain[((CollFace *)hand.obj->planes[0])->terrain].slip / 2.0f;
                    tempVect.z /= terrain[((CollFace *)hand.obj->planes[0])->terrain].slip / 2.0f;
                    maxSpeed = hand.phys->moveSpeed * terrain[((CollFace *)hand.obj->planes[0])->terrain].slip / 3.0f;
                } else {
                    maxSpeed = hand.phys->moveSpeed * terrain[((CollFace *)hand.obj->planes[0])->terrain].slip;
                }
            } else if (hand.state->mode == HAND_HOPPING) {
                if (!handControl.attachedTo->obj->onGround
                    && (handControl.attachedTo->shadow->altitude == -1.0f
                        || handControl.attachedTo->shadow->altitude >= handControl.attachedTo->radius + 8.0f)) {
                    SCALE_VECTOR(&tempVect, handhoppingfactor);
                    AccelerateActor2D(handControl.attachedTo, LENGTH_2D(&tempVect), handControl.attachedTo->phys->moveSpeed);
                }
                return;
            } else {
                maxSpeed = hand.phys->moveSpeed;
            }
            oldSpeed = LENGTH_2D(&hand.vel);
            if (handControl.suctionPlane != NULL) {
                plane = (Plane *)handControl.suctionPlane;
            } else if (hand.obj->planes[0] != NULL) {
                FindFlattestPlane(&hand, &plane, &index);
                if (plane != NULL && plane->y <= handSlide2) {
                    plane = NULL;
                }
            } else {
                plane = NULL;
            }
            if (plane != NULL) {
                a = LENGTH_2D(&tempVect);
                if (a != 0.0f) {
                    vect = hand.vel;
                    if (hand.flags & 0x8000) {
                        AccelerateActorAcrossPlane(&hand, plane, a, hand.phys->moveSpeed, hand.rotAim.y, 1);
                    } else if (terrain[((CollFace *)plane)->terrain].gravityScale == 1.0f) {
                        AccelerateActorAcrossPlane(&hand, plane, ((tempVect.x * plane->x + tempVect.z * plane->z) / (a * 3.0f) + 1.0f) * a,
                                                   hand.phys->moveSpeed, Aabs(-pad[ActiveController].angle[0] + camera.rot.y), 0);
                    } else {
                        AccelerateActorAcrossPlane(&hand, plane, ((tempVect.x * plane->x + tempVect.z * plane->z) / (2.0f * a) + 1.0f) * a,
                                                   hand.phys->moveSpeed, Aabs(-pad[ActiveController].angle[0] + camera.rot.y), 0);
                        AddToOneScaledVector(&hand.vel, (Vec3 *)plane, -gravity);
                    }
                    SUB_VECTORS(&handControl.inputAccel, &hand.vel, &vect);
                } else {
                    ZERO_VECTOR(&handControl.inputAccel);
                }
            } else {
                ADD_TO_VECTOR(&hand.vel, &tempVect);
                handControl.inputAccel = tempVect;
            }
            if (LENGTH_2D(&tempVect) != 0.0f) {
                hand.friction = handInertia;
            }
            if (hand.state->mode != HAND_SLIDING && oldSpeed != 0.0f) {
                DealWithMaxSpeed(&hand, maxSpeed, oldSpeed);
            }
        } else if (hand.state->mode == HAND_JOINED || hand.state->mode == HAND_BOUNCE) {
            tempVect.y = 0.0f;
            tempVect.x = hand.phys->accel / 0.5 * ball.phys->accel * 10.0 / ball.phys->mass
                  * (sinf(camera.rot.y) * pad[ActiveController].stickY / 120.0f
                     - sinf(camera.rot.y + PI / 2) * pad[ActiveController].stickX / 120.0f);
            tempVect.z = hand.phys->accel / 0.5 * ball.phys->accel * 10.0 / ball.phys->mass
                  * (cosf(camera.rot.y) * pad[ActiveController].stickY / 120.0f
                     - cosf(camera.rot.y + PI / 2) * pad[ActiveController].stickX / 120.0f);
            tempVect.x *= 0.32;
            tempVect.z *= 0.32;
            oldSpeed = LENGTH_2D(&ball.vel);
            if (oldSpeed <= 2.0f) {
                SCALE_VECTOR_2D(&tempVect, 1.25);
            }
            if (ball.obj->planes[0] != NULL) {
                if (terrain[((CollFace *)ball.obj->planes[0])->terrain].slip > 1.0f) {
                    tempVect.x /= terrain[((CollFace *)ball.obj->planes[0])->terrain].slip / 2.0f;
                    tempVect.z /= terrain[((CollFace *)ball.obj->planes[0])->terrain].slip / 2.0f;
                    maxSpeed = ball.phys->moveSpeed * terrain[((CollFace *)ball.obj->planes[0])->terrain].slip / 3.0f;
                } else {
                    maxSpeed = ball.phys->moveSpeed * terrain[((CollFace *)ball.obj->planes[0])->terrain].slip;
                }
            } else {
                maxSpeed = ball.phys->moveSpeed;
            }
            if (hand.state->mode == HAND_JOINED && ball.obj->planes[0] != NULL) {
                a = LENGTH_2D(&tempVect);
                if (a != 0.0f) {
                    FindFlattestPlane(&ball, &plane, &index);
                    if (plane != NULL && (plane->y > 0.3 || terrain[((CollFace *)plane)->terrain].gravityScale < 1.0f)) {
                        ball.rotAim.y = hand.rotAim.y;
                        vect = ball.vel;
                        if (terrain[((CollFace *)plane)->terrain].gravityScale == 1.0f) {
                            AccelerateActorAcrossPlane(&ball, plane, ((tempVect.x * plane->x + tempVect.z * plane->z) / (a * 3.0f) + 1.0f) * a,
                                                       ball.phys->moveSpeed * 10.0f, hand.rotAim.y, 0);
                        } else {
                            AccelerateActorAcrossPlane(&ball, plane, ((tempVect.x * plane->x + tempVect.z * plane->z) / (2.0f * a) + 1.0f) * a,
                                                       ball.phys->moveSpeed * 10.0f, hand.rotAim.y, 0);
                            AddToOneScaledVector(&ball.vel, (Vec3 *)plane, -gravity);
                        }
                        SUB_VECTORS(&handControl.inputAccel, &ball.vel, &vect);
                    } else {
                        ADD_TO_VECTOR(&ball.vel, &tempVect);
                        handControl.inputAccel = tempVect;
                    }
                } else {
                    ZERO_VECTOR(&handControl.inputAccel);
                }
            } else {
                ball.vel.x += tempVect.x;
                ball.vel.y += tempVect.y;
                ball.vel.z += tempVect.z;
                handControl.inputAccel = tempVect;
            }
            if (LENGTH_2D(&tempVect) != 0.0f) {
                hand.friction = handInertia;
            }
            if (oldSpeed != 0.0f) {
                DealWithMaxSpeed(&ball, maxSpeed, oldSpeed);
            }
        } else if (hand.state->mode == HAND_BALLWALK) {
            tempVect.y = 0.0f;
            tempVect.x = hand.phys->accel / 0.5 * ball.phys->accel * 10.0 / ball.phys->mass
                  * (sinf(camera.rot.y) * pad[ActiveController].stickY / 180.0f
                     - sinf(camera.rot.y + PI / 2) * pad[ActiveController].stickX / 180.0f);
            tempVect.z = hand.phys->accel / 0.5 * ball.phys->accel * 10.0 / ball.phys->mass
                  * (cosf(camera.rot.y) * pad[ActiveController].stickY / 180.0f
                     - cosf(camera.rot.y + PI / 2) * pad[ActiveController].stickX / 180.0f);
            tempVect.x *= 0.32;
            tempVect.z *= 0.32;
            FlipVector(&tempVect);
            oldSpeed = LENGTH_2D(&ball.vel);
            if (saveSlot[currentSaveSlot].difficultyLevel == DIFFICULTY_EASY) {
                SUB_FROM_VECTOR(&ball.vel, &tempVect);
            } else {
                ball.vel.x += tempVect.x;
                ball.vel.y += tempVect.y;
                ball.vel.z += tempVect.z;
            }
            if (oldSpeed != 0.0f) {
                DealWithMaxSpeed(&ball, ball.phys->maxSpeed, oldSpeed);
            }
            if (oldSpeed <= ball.phys->moveSpeed) {
                f32 len = LENGTH_2D(&ball.vel);

                if (len > ball.phys->moveSpeed) {
                    SCALE_VECTOR_2D(&ball.vel, ball.phys->moveSpeed / len);
                }
            }
        } else if (!(hand.flags & 0x1000000)) {
            DealWithMaxSpeed(&hand, hand.phys->moveSpeed, 10000.0f);
            ZERO_VECTOR(&handControl.inputAccel);
        }
    }
}

s32 AnyButtonPressed(u8 padNum) {
    if (pad[padNum].button[BUTTON_A] != 0 || pad[padNum].button[BUTTON_B] != 0 || pad[padNum].button[BUTTON_START] != 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

s32 AnyDebouncePressed(u8 padNum) {
    if (pad[padNum].pressed[BUTTON_A] != 0 || pad[padNum].pressed[BUTTON_B] != 0 || pad[padNum].pressed[BUTTON_START] != 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

#define VEL(actor) (((VecArray *)&(actor)->vel)->v)

void DealWithMaxSpeed(Actor *actor, f32 maxSpeed, f32 oldSpeed) {
    f32 speed;

    speed = sqrtf(VEL(actor)[0] * VEL(actor)[0] + VEL(actor)[2] * VEL(actor)[2]);
    if (speed > maxSpeed) {
        if (oldSpeed <= maxSpeed) {
            VEL(actor)[0] *= maxSpeed / speed;
            VEL(actor)[2] *= maxSpeed / speed;
        } else {
            VEL(actor)[0] *= 0.9;
            VEL(actor)[2] *= 0.9;
        }
    }
}

// linker junk that follows the 0.9 constant emitted by DealWithMaxSpeed
const u32 D_80105B9C[] = { 0x04007004 };
