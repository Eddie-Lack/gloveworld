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
#include "ambient.h"
#include "debris.h"
#include "font.h"
#include "text.h"
#include "decompress.h"
#include "frontend.h"
#include "object.h"
#include "present.h"

extern s32 fgStill;
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
extern s32 nextHiscoresLevel;
extern f32 hiscoresBoxScale;
extern f32 hiscoresBoxScaleAim;
extern f32 Xos1;
extern Backdrop *pBackground;
extern u32 nextLevel;
extern s16 temptext;
extern s16 farPlaneDist;
extern s32 osTvType;
void LoadTextureBank(s32 num);
void FindTexture(void *texPtr, u32 texID, s32 verbose);
u32 UpdateCRC(char *ptr);
extern Overlay fadeNiceOL[2];
extern s32 areYouSure;
extern s32 aimArRScale;

void DestroyOverlay(Overlay *overlay);

extern s32 creditsPerson;
extern s32 creditsFrame;
extern s16 creditsTitle;
extern f32 creditsPX;
extern f32 creditsPY;
extern u8 onlyDrawBackdrops;
extern s32 creditsScroll;
extern s32 creditsLineHeight;
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
extern f32 bgScale;
extern struct Actor *foregroundObjects[];
extern s32 numForegroundObjects;
extern s32 animWaitTime;
extern s32 currentScreen;
extern s32 nextScreen;
extern s32 frameCount;
extern s32 aimBgScale;
extern Overlay arrOL[2];
extern Overlay A1OL;
extern Overlay A2OL;
extern u32 cAAlpha;
extern u32 aimArLScale;
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
extern s32 sfxVolume;
extern s32 musicVolume;
extern Overlay titleSlide1OL[16];
extern Overlay titleSlide2OL[16];
s32 MainOptionsWait(s32 *act, struct Screen *me);
extern Overlay sfxOL;
extern Overlay musxOL;
extern s32 spacing;
extern s32 credSpace;
extern s32 credpos;
extern f32 D_NUM;
extern s32 credyscale;
void AnimateActor(Actor *actor, s32 animNum, s32 loop, s32 queue, f32 speed);
s32 WaitOKScr();

void UpdateSaveSlot(s32 allGaribs);

extern s32 doFades;
extern s16 musVol;
void MusSetMasterVolume(s32 flags, s32 volume);
extern s32 activateOK;
extern s32 slotSelActive;
extern s32 cameFromTitle;

extern f32 fgXss[];
extern f32 fgYss[];
extern f32 fgRt[];
extern f32 spinInSpd[];
extern s32 spinInFrame;
extern s32 spinInCmpFrame[];
extern f32 bncAmt[];
extern f32 spinInCount[4];
extern u8 cfb0[];
extern u8 cfb1[];

s32 MainTitleWait();

extern char *menuStrings[][3];
extern char *startMenuStrings[][3];
extern char *optionMenuString[3];
extern s32 SfxDelay;
extern f32 fgSpinRot[];
s32 MusStartEffect2(s32 number, s32 volume, s32 pan, s32 restartflag, s32 priority);

s32 DoneOptionsWait(s32 *act, Screen *me) {
    s32 i;

    if (!aimBgScale) {
        MainOptionsWait(act, me);
        spinOutFGObjects(4);
        MoveGloveOL();
        return FALSE;
    }
    FreeFGObjects();
    for (i = 0; i < 16; i++) {
        DestroyOverlay(&titleSlide1OL[i]);
        DestroyOverlay(&titleSlide2OL[i]);
    }
    DestroyOverlay(&musxOL);
    DestroyOverlay(&sfxOL);
    return TRUE;
}

#define OPT_FADE                                                                                 \
    ((fgSpin.minSpot < 2) ? (((111.0f - spinInCount[0]) * 255.0f / 110.0f > 255.0f) ? 255                \
                      : ((110.0f - spinInCount[0]) * 255.0f / 110.0f < 0.0f) ? 0                   \
                      : (110.0f - spinInCount[0]) * 255.0f / 110.0f) : 255)
#define OPT_FADE_HALF                                                                            \
    ((fgSpin.minSpot < 2) ? (((111.0f - spinInCount[0]) * 255.0f / 110.0f > 255.0f) ? 127                \
                      : ((110.0f - spinInCount[0]) * 255.0f / 110.0f < 0.0f) ? 0                   \
                      : (110.0f - spinInCount[0]) * 255.0f / 110.0f / 2.0f) : 127)
#define OPT_PULSE (sinf(animFloat * 0.65) * 90.0f + 128.0f + 38.0f)

s32 MainOptionsWait(s32 *act, Screen *me) {
    s32 i;
    s32 j;

    SetFont(1);
    NEW_TEXT(260, spinInCount[0] / 4.0f + 227.0f, 255, 255, 255, 255, 1.0f, 1.0f, 1,
             startMenuStrings[10][gameInfo.language], 1);
    SetFont(0);
    if (!fgStill) {
        foregroundObjects[0]->rot.x = fgSpinRot[2];
        foregroundObjects[0]->rot.y = sinf(animFloat * 0.2) * 0.1 + 3.14;
        foregroundObjects[0]->rot.z = sinf(animFloat * 0.15) * 0.1;
        foregroundObjects[1]->rot.x = fgSpinRot[1];
        foregroundObjects[1]->rot.y = sinf(animFloat * 0.3) * 0.1 + 3.14;
        foregroundObjects[1]->rot.z = sinf(animFloat * 0.15) * 0.1;
        foregroundObjects[2]->rot.x = fgSpinRot[4];
        foregroundObjects[2]->rot.y = sinf(animFloat * 0.15) * 0.1 + 3.14;
        foregroundObjects[2]->rot.z = sinf(animFloat * 0.2) * 0.1;
        foregroundObjects[3]->rot.x = fgSpinRot[3];
        foregroundObjects[3]->rot.y = sinf(animFloat * 0.19) * 0.1 + 3.14;
        foregroundObjects[3]->rot.z = sinf(animFloat * 0.2) * 0.1;
    }
    switch (me->currentSpot) {
        case 0:
            musicVolume += pad[ActiveController].stickX / 30;
            break;
        case 1:
            sfxVolume += pad[ActiveController].stickX / 30;
            if (pad[ActiveController].stickX) {
                if (!SfxDelay) {
                    MusStartEffect2(131, 100, 128, 0, -1);
                    SfxDelay = 5;
                } else {
                    SfxDelay--;
                }
            }
            break;
    }
    musxOL.x = 2.0f * spinInCount[2] + 260.0f;
    sfxOL.x = 270.0f - spinInCount[1];
    if (sfxVolume < 0) {
        sfxVolume = 0;
    }
    if (sfxVolume > 160) {
        sfxVolume = 160;
    }
    if (musicVolume < 0) {
        musicVolume = 0;
    }
    if (musicVolume > 160) {
        musicVolume = 160;
    }
    for (i = 0; i < 16; i++) {
        titleSlide1OL[i].x = 135 + i * 6 + spinInCount[2];
        titleSlide1OL[i].alpha = OPT_FADE;
        titleSlide2OL[i].x = 135 + i * 6 - 2.0f * spinInCount[1];
        titleSlide2OL[i].alpha = OPT_FADE;
        if (i > (f32)(musicVolume - 5) / 10.0) {
            titleSlide1OL[i].alpha = OPT_FADE_HALF;
        }
        if (i > (f32)(sfxVolume - 5) / 10.0) {
            titleSlide2OL[i].alpha = OPT_FADE_HALF;
        }
    }
    UpdatePause();
    for (j = 0; j < numForegroundObjects; j++) {
        GetQuaternionFromXYZRot(&foregroundObjects[j]->rotQuat, &foregroundObjects[j]->rot);
    }
    if (me->currentSpot == 1) {
        sfxOL.alpha = OPT_FADE * OPT_PULSE / 255.0f;
    } else {
        sfxOL.alpha = OPT_FADE;
    }
    if (me->currentSpot == 0) {
        musxOL.alpha = OPT_FADE * OPT_PULSE / 255.0f;
    } else {
        musxOL.alpha = OPT_FADE;
    }
    if (me->currentSpot == 2) {
        NEW_TEXT(spinInCount[0] + 173.0f, 161, 255, 255, 255, OPT_FADE * OPT_PULSE / 255.0f, 1.0f, 1.0f, 1,
                 optionMenuString[gameInfo.language], 1);
    } else {
        NEW_TEXT(spinInCount[0] + 173.0f, 161, 255, 255, 255, OPT_FADE, 0.8f, 0.8f, 1,
                 optionMenuString[gameInfo.language], 1);
    }
    if (me->currentSpot == 3) {
        NEW_TEXT(175, spinInCount[1] + 193.0f, 255, 255, 255, OPT_FADE * OPT_PULSE / 255.0f, 1.0f, 1.0f, 1,
                 menuStrings[9][gameInfo.language], 1);
    } else {
        NEW_TEXT(175, spinInCount[1] + 193.0f, 255, 255, 255, OPT_FADE, 0.8f, 0.8f, 1,
                 menuStrings[9][gameInfo.language], 1);
    }
    NEW_TEXT(160, 30, 255, 255, 255, OPT_FADE, 0.8f, 0.8f, 1, menuStrings[2][gameInfo.language], 1);
    TitleScreenCameraSpin2();
    DefWait(act, me);
    if (*act == 0 || *act == 1) {
        fgSpinRot[me->currentSpot + 1] = 6.28f;
    }
    fgSpinRot[1] *= 0.9;
    fgSpinRot[2] *= 0.9;
    fgSpinRot[3] *= 0.9;
    fgSpinRot[4] *= 0.9;
    if ((*act == 4 || *act == 5) && (me->currentSpot >= 2 || *act == 5)) {
        if (aimArRScale == 1 && !aimBgScale) {
            return FALSE;
        }
        FadeScreenOutN();
        initSpinOutFGObjects(4);
        foregroundObjects[4]->anim->loop = 0;
        AnimateActor(foregroundObjects[4], 1, 0, 0, 0.7f);
        return TRUE;
    }
    spinInFGObjects(4);
    return FALSE;
}

s32 IntroOptionsWait(void) {
    fgStill = 0;
    return TRUE;
}

s32 InitOptionsWait(s32 *act, Screen *me) {
    s32 i;

    FadeScreenInN();
    fgSpinRot[1] = 0.0f;
    fgSpinRot[2] = 0.0f;
    fgSpinRot[3] = 0.0f;
    fgSpinRot[4] = 0.0f;
    SetFont(0);
    for (i = 0; i < 16; i++) {
        FindSetupAndAddOverlay(&titleSlide1OL[i], "sliderbit.bmp", 155 + i * 6, 80, 0, 1);
        titleSlide1OL[i].alpha = 255;
        titleSlide1OL[i].r = 255;
        titleSlide1OL[i].g = 80;
        titleSlide1OL[i].b = 255;
        titleSlide1OL[i].scaleX = 990;
        titleSlide1OL[i].scaleY = 1450 - (16 - i) * 60;
        if (i > (f32)(musicVolume - 5) / 10.0) {
            titleSlide1OL[i].alpha = 90;
        }
        FindSetupAndAddOverlay(&titleSlide2OL[i], "sliderbit.bmp", 155 + i * 6, 110, 0, 1);
        titleSlide2OL[i].alpha = 255;
        titleSlide2OL[i].r = 80;
        titleSlide2OL[i].g = 255;
        titleSlide2OL[i].b = 80;
        titleSlide2OL[i].scaleX = 990;
        titleSlide2OL[i].scaleY = 1450 - (16 - i) * 60;
        if (i > (f32)(sfxVolume - 5) / 10.0) {
            titleSlide2OL[i].alpha = 90;
        }
    }
    initFGObject("newwave.ndo", -25.0f, 18.0f, 500.0f, 0.1f, 0.0f, 3.14f, 1.2f, 0.7f, 1.0f, 255);
    InitActorStructures(foregroundObjects[0], 4);
    AnimateActor(foregroundObjects[0], 0, 1, 1, 1.0f);
    foregroundObjects[0]->scale.x = 1.3f;
    initFGObject("newwave.ndo", -25.0f, 70.0f, 500.0f, 0.1f, 0.0f, 3.14f, 1.0f, 0.7f, 1.0f, 255);
    InitActorStructures(foregroundObjects[1], 4);
    AnimateActor(foregroundObjects[1], 0, 1, 1, 1.1f);
    foregroundObjects[1]->scale.x = 1.3f;
    initFGObject("newwave.ndo", -25.0f, -125.0f, 500.0f, 0.1f, 0.0f, 3.14f, 0.9f, 0.9f, 1.0f, 255);
    InitActorStructures(foregroundObjects[2], 4);
    AnimateActor(foregroundObjects[2], 0, 1, 1, 0.9f);
    foregroundObjects[2]->scale.x = 1.3f;
    initFGObject("newwave.ndo", -25.0f, -70.0f, 500.0f, 0.1f, 0.0f, 3.14f, 0.9f, 0.9f, 1.0f, 255);
    InitActorStructures(foregroundObjects[3], 4);
    AnimateActor(foregroundObjects[3], 0, 1, 1, 0.9f);
    foregroundObjects[3]->scale.x = 1.3f;
    initFGObject("newglove.ndo", 360.0f, 396.0f, 550.0f, 0.0f, 3.5f, 0.0f, 1.0f, 1.0f, 1.0f, 255);
    InitActorStructures(foregroundObjects[4], 4);
    AnimateActor(foregroundObjects[4], 0, 0, 0, 1.0f);
    AnimateActor(foregroundObjects[4], 2, 1, 1, 1.0f);
    FindSetupAndAddOverlay(&sfxOL, "horn0001.bmp", 270, 112, 0, 1);
    sfxOL.frames = (SpriteFrames *)&spriteFrameList[HORN_ANIM];
    sfxOL.behind = 0;
    FindSetupAndAddOverlay(&musxOL, "note0001.bmp", 260, 70, 0, 1);
    musxOL.frames = (SpriteFrames *)&spriteFrameList[NOTE_ANIM];
    musxOL.behind = 0;
    initSpinFGObjects(4);
    credpos = 3;
    spacing = 0;
    credyscale = 1;
    credSpace = 2;
    MainOptionsWait(act, me);
    return TRUE;
}

INCLUDE_ASM("asm/nonmatchings/options", pad_80118B68);

INCLUDE_RODATA("asm/nonmatchings/options", pad_80100A6C);
