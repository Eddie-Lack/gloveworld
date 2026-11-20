#include "common.h"
#include "game.h"
#include "camera.h"
#include "sound.h"
#include "hand.h"
#include "ball.h"
#include "maths.h"
#include "debris.h"
#include "overlay.h"
#include "banks.h"
#include "collect.h"
#include "controller.h"
#include "present.h"

extern s32 frameCount;
extern s32 frameCount2;
extern s16 currentSaveSlot;
extern s32 autoPlayback;
extern u8 disablePause;

void SetFont(u8 type);
void StartNewLevel(s32 num);
void PrepareSong(s32 song);

extern Enemy enemies[];
extern u8 controllerPresent;
extern u8 ActiveController;
extern Overlay timePicOL;
extern struct Text *startText;
extern char *startString[];
extern char *noControllerString[3];
extern struct Text *noControllerTextFrench;
extern struct Text *noControllerTextGerman;
extern s32 osTvType;
struct Text *NEW_TEXT(s16 x, s16 y, u8 r, u8 g, u8 b, u8 a, f32 sx, f32 sy, u8 oneShot, char *str, u8 centre);

s32 SetupPresentScreen(void) {
    StopDrawing("setpresent");
    SetFont(0);
    gameInfo.exitFlags = 0;
    frameCount = frameCount2 = 0;
    InitLinkedLists();
    currentSaveSlot = 6;
    StartNewGame();
    autoPlayback = 2;
    StartNewLevel(PRESENT);
    PrepareSong(HASBRO_TRACK);
    FadeScreenIn();
    disablePause = TRUE;
    cameraInfo.sideOffsetEnabled = 0;
    return TRUE;
}

void DoPresentScreen(void) {
    cameraInfo.sideOffsetEnabled = 0;
    if (hand.state) {
        if (hand.state->mode == HAND_SLAM2ST && handControl.actionTime == 8) {
            ballStatic = 0;
            ball.pos.x = hand.pos.x;
            ball.pos.y = hand.pos.y - 0.5;
            ball.pos.z = hand.pos.z;
            ball.flags |= 0x19;
            ApplyAction(HAND_BALLWALK);
            CreateStarCircle(&hand.pos, 6, 19);
            CreateStarCircle(&ball.pos, 8, 21);
        } else if (hand.state->mode == HAND_BALLWALK && handControl.actionTime == 10) {
            ActivateCollectable(-1, -1);
            ball.obj->spheres[0].targetRadius = 15.0f;
        }
    }
    if (frameCount == 199) {
        SetVector2D(&hand.pos, &camera.pos);
    } else if (frameCount == 100) {
        ActivateCollectable(BOWLINGBALL, -1);
        ball.obj->spheres[0].targetRadius = 15.0f;
    }
}

void SetupFlythruScreen(void) {
    StopDrawing("setflythru");
    SetFont(0);
    gameInfo.exitFlags = 0;
    frameCount = frameCount2 = 0;
    InitLinkedLists();
    currentSaveSlot = 6;
    defaultBehaviour[BUTTERFLY].accel *= 2.0f;
    defaultBehaviour[BUTTERFLY].moveSpeed *= 2.0f;
    StartNewLevel(FLYTHRU);
    FindSetupAndAddOverlay(&timePicOL, "glover.bmp", 165, 40, 2, 0);
    if (controllerPresent == TRUE) {
        startText = NEW_TEXT(160, 200, 255, 255, 255, 255, 1.0f, 1.0f, 0, startString[gameInfo.language], 1);
    } else {
        startText = NEW_TEXT(160, 150, 255, 255, 255, 255, 0.6f, 1.0f, 0, noControllerString[0], 1);
        if (osTvType == 0) {
            noControllerTextFrench = NEW_TEXT(160, 180, 255, 255, 255, 255, 0.6f, 1.0f, 0, noControllerString[1], 1);
            noControllerTextGerman = NEW_TEXT(160, 210, 255, 255, 255, 255, 0.6f, 1.0f, 0, noControllerString[2], 1);
        }
    }
    FadeScreenIn();
    disablePause = TRUE;
}

void DoFlythruScreen(void) {
    cameraInfo.focus = &enemies[0].actor.pos;
    if (enemies[0].actor.shadow->altitude < 20.0f && enemies[0].actor.shadow->altitude != -1.0f) {
        enemies[0].actor.vel.y += 0.75;
    }
    if (enemies[0].actor.shadow->altitude > 40.0f) {
        enemies[0].actor.vel.y -= 0.2;
    }
    if (controllerPresent) {
        if ((AnyDebouncePressed(ActiveController) && !gameInfo.starZ && !gameInfo.fade) || frameCount == 2345) {
            PlaySampleNot3D(59, 100, 128, 190);
            defaultBehaviour[BUTTERFLY].accel /= 2.0f;
            defaultBehaviour[BUTTERFLY].moveSpeed /= 2.0f;
            ExitLevel(QUIT_EXIT, 1, 1);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/present", pad_8012C2A8);

INCLUDE_RODATA("asm/nonmatchings/present", pad_80101998);
