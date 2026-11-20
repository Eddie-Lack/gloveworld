#include "common.h"
#include "maths.h"
#include "macros.h"
#include "ident.h"
#include "overlay.h"
#include "save.h"
#include "game.h"
#include "jalloc.h"
#include "actor.h"
#include "banks.h"
#include "camera.h"
#include "light.h"
#include "controller.h"
#include "libc.h"
#include "ambient.h"
#include "debris.h"
#include "font.h"
#include "text.h"
#include "timer.h"
#include "decompress.h"
#include "frontend.h"
#include "object.h"
#include "present.h"

extern s32 introAnimTime;
extern s32 aimBallX;
extern s32 aimBallY;
extern f32 aimBallScale;
extern f32 ballXOffset;
extern char *hsLevNames[][3];
extern BestTime bestTimes[];
extern Overlay D_801F64B4;
extern u8 jallocHeapStart[];
void DestroyAndFreeActor(Actor *actor);
void PrepareSong(s32 song);
extern s16 waitTimer1;
extern s32 booting;
extern u8 ActiveController;
extern GameLight light;
void SetupLight(GameLight *light, u8 initR, u8 initG, u8 initB, u8 r, u8 g, u8 b, f32 x, f32 y, f32 speed);
void AddBackgroundLight(GameLight *light);
extern Vec3 zero;
void LoadObjectBank(s32 num);
extern s32 aA;
extern s32 aB;
extern s32 aaA;
extern s32 aaB;
extern s32 aimLA;
extern s32 aimRA;
extern s32 hTime;
extern s32 exiting;
extern s32 cXLU;
extern f32 Xos;
extern f32 XosVel;
extern s32 currentHiscoresLevel;
extern s32 nextLevel;
extern f32 bgScale;
extern f32 aimBgScale;
extern f32 Xos1;
extern Backdrop *pBackground;
extern u32 licenceFrame;
extern s16 D_801E64D8;
extern s16 farPlaneDist;
extern s32 osTvType;
void LoadTextureBank(s32 num);
void FindTexture(void *texPtr, u32 texID, s32 verbose);
u32 UpdateCRC(char *ptr);
extern Overlay fadeNiceOL[2];
extern s32 fadeAvailable;
extern s32 fadeDir;

void DestroyOverlay(Overlay *overlay);

extern s32 creditsPerson;
extern s32 creditsFrame;
extern s16 creditsTitle;
extern f32 creditsPX;
extern f32 creditsPY;
extern u8 onlyDrawBackdrops;
extern s32 credpos;
extern s32 credSpace;
extern Overlay ballPicOL;
struct Screen;
s32 DefWait(s32 *act, struct Screen *me);
extern u8 newDesiredFrameRate;
extern f32 globalFontScale;
extern u8 demoLevel;
extern s32 autoPlayback;
extern s32 cheatCount;
void ActivateCheat(s32 num, s32 show);
extern s32 controllerActive;
extern s32 fadeOutVal;
extern f32 fadeAdd;
extern struct Actor *foregroundObjects[];
extern s32 numForegroundObjects;
extern s32 animWaitTime;
extern s32 currentScreen;
extern s32 nextScreen;
extern s32 frameCount;
extern s32 fadeDone;
extern Overlay arrOL[2];
extern Overlay A1OL;
extern Overlay A2OL;
extern u32 cAAlpha;
extern u32 cAAlphaAim;
extern s32 currentAnim;

typedef struct ScreenSpot {
    /* 0x00 */ s32 type;
    /* 0x04 */ u32 x;
    /* 0x08 */ s32 y;
    /* 0x0C */ s32 (*func)(void);
    /* 0x10 */ s32 next;
} ScreenSpot; /* size = 0x14 */

typedef struct Screen {
    /* 0x00 */ s32 (*anims[7])(s32 *buttons, struct Screen *screen);
    /* 0x1C */ s32 currentSpot;
    /* 0x20 */ s32 numSpots;
    /* 0x24 */ ScreenSpot spots[10];
} Screen; /* size = 0xEC */

extern Screen screenList[];
typedef struct FGSpin {
    /* 0x00 */ s32 bnc[4];
    /* 0x10 */ s32 minSpot;
    /* 0x14 */ s32 maxSpot;
} FGSpin;

extern FGSpin fgSpin;
extern f32 animFloat;
extern s32 levelSelectCount;

void StartNewLevel(s32 num);
void SetFont(u8 type);
void ReleaseActor(struct Actor *actor, s32 flags);
s32 HiscoresRun(s32 *act, struct Screen *me);
struct Text *NEW_TEXT(s16 x, s16 y, u8 r, u8 g, u8 b, u8 a, f32 sx, f32 sy, u8 oneShot, char *str, u8 centre);

typedef struct CreditsLine {
    /* 0x00 */ char *title;
    /* 0x04 */ char *name;
} CreditsLine;

extern CreditsLine creditsText[];
extern u8 currentName[3];
extern u8 enteredName[3];
extern s16 currentSaveSlot;
extern s32 toStartGame;
extern s32 curSoundFXVol;
extern s32 curSoundMUSVol;
extern Overlay titleSlide1OL[16];
extern Overlay titleSlide2OL[16];
s32 MainOptionsWait(s32 *act, struct Screen *me);
extern Overlay sfxOL;
extern Overlay musxOL;
extern s32 fgDirections[];
extern f32 D_NUM;
void AnimateActor(Actor *actor, s32 animNum, s32 loop, s32 queue, f32 speed);
s32 WaitOKScr();

void UpdateSaveSlot(s32 allGaribs);

extern s32 doFades;
extern s16 musVol;
void MusSetMasterVolume(s32 flags, s32 volume);
extern s32 activateOK;
extern s32 inputOK;
extern s32 cameFromTitle;

extern f32 fgXss[];
extern f32 fgYss[];
extern f32 fgRt[];
extern f32 spinInSpd[];
extern s32 spinInFrame;
extern s32 spinInCmpFrame[];
extern f32 bncAmt[];
extern f32 spinInCount[4];
extern u8 cfb_16_a[];
extern u8 cfb_16_b[];

s32 MainTitleWait();

extern s32 allAdder;
extern f32 jogAmt[];

extern Overlay gloverOL[6];
extern f32 aimScale[4];
extern f32 actScale[4];
extern s32 actTrans[4];
extern s32 aimTrans[4];
extern u8 tR;
extern u8 tG;
extern u8 tB;
extern s32 demoTimer;
extern s32 cheatActivated;
extern char *menuStrings[][3];

void SetupTitleWait(void) {
    SetFont(0);
    autoPlayback = FALSE;
    InitSpriteList();
    if (ballPicOL.bitmaps == NULL) {
        FindSetupAndAddOverlay(&ballPicOL, "glob01.bmp", 105, 60, 0, 1);
        ballPicOL.frames = (SpriteFrames *)&spriteFrameList[GLOVE_ANIM];
        ballPicOL.behind = 0;
    }
    inputOK = TRUE;
    globalFontScale = 1.0f;
}

s32 InitTitleWait(s32 *act, Screen *me) {
    FadeScreenInN();
    inputOK = TRUE;
    me->currentSpot = 1;
    fgSpin.minSpot = 1;
    fgDirections[0] = 1;
    fgDirections[1] = 2;
    fgDirections[2] = 0;
    fgDirections[3] = 3;
    jogAmt[0] = 0.0f;
    jogAmt[1] = 0.0f;
    jogAmt[2] = 0.0f;
    jogAmt[3] = 0.0f;

    foregroundObjects[0] = JallocAlloc(sizeof(Actor), 1, "TitA1");
    InitActor(foregroundObjects[0], "newwave.ndo", -15.0f, -30.0f, 400.0f, 0);
    foregroundObjects[0]->rot.z = 3.14f;
    foregroundObjects[0]->rot.y = 0.0f;
    foregroundObjects[0]->rot.x = -0.2f;
    foregroundObjects[0]->alpha = 255;
    foregroundObjects[0]->inst->obj->flags |= 2;
    InitActorStructures(foregroundObjects[0], 4);
    AnimateActor(foregroundObjects[0], 0, 1, 1, 1.0f);

    foregroundObjects[1] = JallocAlloc(sizeof(Actor), 1, "TitA1");
    InitActor(foregroundObjects[1], "newwave.ndo", -15.0f, 35.0f, 400.0f, 0);
    foregroundObjects[1]->rot.z = 3.14f;
    foregroundObjects[1]->rot.y = 0.0f;
    foregroundObjects[1]->rot.x = -0.2f;
    foregroundObjects[1]->scale.y = 0.6f;
    foregroundObjects[1]->alpha = 255;
    foregroundObjects[1]->inst->obj->flags |= 2;
    InitActorStructures(foregroundObjects[1], 4);
    AnimateActor(foregroundObjects[1], 0, 1, 1, 0.8f);

    foregroundObjects[2] = JallocAlloc(sizeof(Actor), 1, "TitA1");
    InitActor(foregroundObjects[2], "newwave.ndo", -15.0f, 95.0f, 400.0f, 0);
    foregroundObjects[2]->rot.z = 3.14f;
    foregroundObjects[2]->rot.y = 0.0f;
    foregroundObjects[2]->alpha = 255;
    foregroundObjects[2]->inst->obj->flags |= 2;
    InitActorStructures(foregroundObjects[2], 4);
    AnimateActor(foregroundObjects[2], 0, 1, 1, 1.1f);

    foregroundObjects[3] = JallocAlloc(sizeof(Actor), 1, "TitA1");
    InitActor(foregroundObjects[3], "newwave.ndo", -15.0f, -90.0f, 400.0f, 0);
    foregroundObjects[3]->rot.z = 3.14f;
    foregroundObjects[3]->rot.y = 0.0f;
    foregroundObjects[3]->alpha = 255;
    foregroundObjects[3]->inst->obj->flags |= 2;
    InitActorStructures(foregroundObjects[3], 4);
    AnimateActor(foregroundObjects[3], 0, 1, 1, 1.1f);

    numForegroundObjects = 4;
    initFGObject("newglove.ndo", 325.0f, 396.0f, 550.0f, 0.0f, 3.5f, 0.0f, 1.0f, 1.0f, 1.0f, 255);
    InitActorStructures(foregroundObjects[4], 4);
    AnimateActor(foregroundObjects[4], 0, 0, 0, 1.0f);
    AnimateActor(foregroundObjects[4], 3, 0, 1, 1.0f);
    AnimateActor(foregroundObjects[4], 2, 1, 1, 1.0f);
    foregroundObjects[0]->pos.x += allAdder;
    foregroundObjects[1]->pos.x += allAdder;
    foregroundObjects[2]->pos.x += allAdder;
    foregroundObjects[3]->pos.x += allAdder;
    initSpinFGObjects(4);
    ballXOffset = 0.0f;
    MainTitleWait(act, me);
    FadeScreenInN();
    StartDrawing("inittitlewait");
    return TRUE;
}

s32 DoneTitleWait(s32 act, Screen *me) {
    if (!fadeDone) {
        spinOutFGObjects(4);
        if (nextScreen == 9 || nextScreen == 12) {
            doFades = TRUE;
        }
        MoveGloveOL();
        MainTitleWait(act, me);
        return FALSE;
    }
    if (nextScreen != 3) {
        if (nextScreen != 4) {
            if (nextScreen != 2) {
                StopDrawing("donttitleweair");
                memset(cfb_16_a, 0, 0x25800);
                memset(cfb_16_b, 0, 0x25800);
            }
        }
    }
    FreeFGObjects();
    inputOK = TRUE;
    if (me->spots[me->currentSpot].func && activateOK) {
        me->spots[me->currentSpot].func();
        return FALSE;
    }
    return TRUE;
}

s32 IntroTitleWait(void) {
    MainTitleWait();
    introAnimTime = 0;
    return TRUE;
}

#define TITLE_FADE                                                                               \
    ((fgSpin.minSpot < 2) ? (((111.0f - spinInCount[0]) * 255.0f / 110.0f > 255.0f) ? 255                \
                      : ((110.0f - spinInCount[0]) * 255.0f / 110.0f < 0.0f) ? 0                   \
                      : (110.0f - spinInCount[0]) * 255.0f / 110.0f) : 255)
#define TITLE_PULSE (sinf(animFloat * 0.65) * 90.0f + 128.0f + 38.0f)

s32 MainTitleWait(s32 *act, Screen *me) {
    s32 i;
    s32 j;

    cameFromTitle = FALSE;
    UpdatePause();
    if (*act != 6) {
        demoTimer = 600;
    }
    for (i = 0; i < 6; i++) {
        if (gloverOL[i].x < 44.0f) {
            gloverOL[i].x += 2.0f;
        }
    }
    for (i = 0; i < 4; i++) {
        if (me->currentSpot == i + 1) {
            aimTrans[i] = 255;
            aimScale[i] = 0.25f;
        } else {
            aimTrans[i] = 180;
            aimScale[i] = 0.1f;
        }
        if (aimScale[i] < actScale[i]) {
            if (ABS(aimScale[i] - actScale[i]) > 0.04) {
                actScale[i] -= 0.01;
            } else {
                actScale[i] = aimScale[i];
            }
        }
        if (aimScale[i] > actScale[i]) {
            if (ABS(aimScale[i] - actScale[i]) > 0.04) {
                actScale[i] += 0.01;
            } else {
                actScale[i] = aimScale[i];
            }
        }
        if (aimTrans[i] < actTrans[i]) {
            actTrans[i] -= 2;
        } else {
            actTrans[i] = aimTrans[i];
        }
        if (aimTrans[i] > actTrans[i]) {
            actTrans[i] += 2;
        } else {
            actTrans[i] = aimTrans[i];
        }
        if (me->currentSpot != i + 1) {
            if (i == 0) {
                NEW_TEXT(spinInCount[1] + 199.0f, i * 45 + 53, tR, tG, tB, TITLE_FADE, 0.8f, 0.8f, 1,
                         menuStrings[i][gameInfo.language], 1);
            }
            if (i == 1) {
                NEW_TEXT(199.0f - spinInCount[0], i * 45 + 53, tR, tG, tB, TITLE_FADE, 0.8f, 0.8f, 1,
                         menuStrings[i][gameInfo.language], 1);
            }
            if (i == 2) {
                NEW_TEXT(spinInCount[2] + 199.0f, i * 45 + 53, tR, tG, tB, TITLE_FADE, 0.8f, 0.8f, 1,
                         menuStrings[i][gameInfo.language], 1);
            }
            if (i == 3) {
                NEW_TEXT(spinInCount[2] + 199.0f, i * 45 + 53, tR, tG, tB, TITLE_FADE, 0.8f, 0.8f, 1,
                         menuStrings[14][gameInfo.language], 1);
            }
        } else {
            if (i == 0) {
                NEW_TEXT(spinInCount[1] + 199.0f, i * 35 + 53, tR, tG, tB,
                         TITLE_FADE * TITLE_PULSE * (actTrans[i] / 255) / 255.0f, 1.0f, 1.0f, 1,
                         menuStrings[i][gameInfo.language], 1);
            }
            if (i == 1) {
                NEW_TEXT(199.0f - spinInCount[0], i * 35 + 63, tR, tG, tB,
                         TITLE_FADE * TITLE_PULSE * (actTrans[i] / 255) / 255.0f, 1.0f, 1.0f, 1,
                         menuStrings[i][gameInfo.language], 1);
            }
            if (i == 2) {
                NEW_TEXT(spinInCount[2] + 199.0f, i * 35 + 73, tR, tG, tB,
                         TITLE_FADE * TITLE_PULSE * (actTrans[i] / 255) / 255.0f, 1.0f, 1.0f, 1,
                         menuStrings[i][gameInfo.language], 1);
            }
            if (i == 3) {
                NEW_TEXT(spinInCount[2] + 199.0f, i * 35 + 83, tR, tG, tB,
                         TITLE_FADE * TITLE_PULSE * (actTrans[i] / 255) / 255.0f, 1.0f, 1.0f, 1,
                         menuStrings[14][gameInfo.language], 1);
            }
        }
        if (!introAnimTime) {
            foregroundObjects[0]->rot.x = jogAmt[2];
            foregroundObjects[0]->rot.y = sinf(animFloat * 0.2) * 0.1 + 3.14;
            foregroundObjects[0]->rot.z = sinf(animFloat * 0.15) * 0.1;
            foregroundObjects[1]->rot.x = jogAmt[1];
            foregroundObjects[1]->rot.y = sinf(animFloat * 0.3) * 0.1 + 3.14;
            foregroundObjects[1]->rot.z = sinf(animFloat * 0.15) * 0.1;
            foregroundObjects[2]->rot.x = jogAmt[0];
            foregroundObjects[2]->rot.y = sinf(animFloat * 0.1) * 0.1 + 3.14;
            foregroundObjects[2]->rot.z = sinf(animFloat * 0.15) * 0.1;
            foregroundObjects[3]->rot.x = jogAmt[3];
            foregroundObjects[3]->rot.y = sinf(animFloat * 0.18) * 0.1 + 3.14;
            foregroundObjects[3]->rot.z = sinf(animFloat * 0.15) * 0.1;
        }
        for (j = 0; j < numForegroundObjects; j++) {
            GetQuaternionFromXYZRot(&foregroundObjects[j]->rotQuat, &foregroundObjects[j]->rot);
        }
    }
    activateOK = TRUE;
    TitleScreenCameraSpin2();
    if (Random(200) == 15) {
        if (foregroundObjects[4]->anim->current == 2 && foregroundObjects[4]->anim->loop) {
            foregroundObjects[4]->anim->loop = 0;
            AnimateActor(foregroundObjects[4], 3, 0, 1, 1.0f);
            AnimateActor(foregroundObjects[4], 2, 1, 1, 1.0f);
        }
    }
    if (--demoTimer == 0) {
        nextScreen = 7;
        introAnimTime = 31;
        demoTimer = 600;
        currentSaveSlot = 6;
        activateOK = FALSE;
        doFades = TRUE;
        gameInfo.exitFlags |= 1;
        FadeScreenOutN();
        return TRUE;
    }
    if (pad[4].pressed[BUTTON_Z]) {
        if (++autoPlayback >= 3) {
            autoPlayback = 0;
        }
    }
    fgSpin.minSpot = 1;
    if (fadeDone) {
        DefWait(act, me);
    } else {
        aimBallX = me->spots[me->currentSpot].x + ballXOffset;
        aimBallY = me->spots[me->currentSpot].y;
        ballPicOL.x -= (ballPicOL.x - aimBallX) / 3.0f;
        ballPicOL.y -= (ballPicOL.y - aimBallY) / 3.0f;
    }
    if (me->currentSpot == 0 && !(cheatActivated & 0x800000)) {
        me->currentSpot = 1;
    }
    if (*act == 0 || *act == 1) {
        if (me->currentSpot > 0) {
            jogAmt[me->currentSpot - 1] = 6.28f;
        }
    }
    jogAmt[0] *= 0.9;
    jogAmt[1] *= 0.9;
    jogAmt[2] *= 0.9;
    jogAmt[3] *= 0.9;
    if (*act == 4 || *act == 5) {
        if (fadeDir == 1 && !fadeDone) {
            return FALSE;
        }
        if (nextScreen == 9 || nextScreen == 12) {
            doFades = TRUE;
            gameInfo.exitFlags |= 1;
        }
        doFades = FALSE;
        if (me->currentSpot == 2) {
            doFades = TRUE;
        }
        FadeScreenOutN();
        initSpinOutFGObjects(4);
        foregroundObjects[4]->anim->loop = 0;
        AnimateActor(foregroundObjects[4], 1, 0, 0, 0.7f);
        return TRUE;
    }
    PrintTimers();
    spinInFGObjects(4);
    return FALSE;
}

s32 DemoKeyRun(void) {
    StopDrawing("dkr");
    memset(cfb_16_a, 0, 0x25800);
    memset(cfb_16_b, 0, 0x25800);
    KillTitle();
    globalFontScale = 1.0f;
    StopDrawing("dkr");
    doFades = 1;
    currentSaveSlot = 6;
    saveSlot[currentSaveSlot].difficultyLevel = DIFFICULTY_MEDIUM;
    ActivateCheat(22, 0);
    cheatCount = 1;
    StartNewGame();
    autoPlayback = 2;
    switch (demoLevel) {
        case 0:
            StartNewLevel(ATLANTIS1);
            break;
        case 1:
            StartNewLevel(CARNIVAL2A);
            break;
        case 2:
            StartNewLevel(PIRATES1);
            break;
        case 3:
            StartNewLevel(PREHISTORIC1A);
            break;
        case 4:
            StartNewLevel(FEAR1A);
            break;
        case 5:
            autoPlayback = 0;
            SetupFlythruScreen();
            break;
    }
    demoLevel = (demoLevel + 1) % 6;
    return FALSE;
}

INCLUDE_ASM("asm/nonmatchings/title", pad_80114738);

INCLUDE_RODATA("asm/nonmatchings/title", D_801005F8);
