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
#include "credits.h"
#include "decompress.h"
#include "frontend.h"
#include "object.h"
#include "present.h"

extern s32 space;
extern s32 aimBallX;
extern s32 aimBallY;
extern f32 aimBallScale;
extern f32 ballXOffset;
extern char *hsLevNames[][3];
extern BestTime bestTimes[];
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
    /* 0x1C */ u32 currentSpot;
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

typedef struct BestTimeRow {
    BestTime times[5];
} BestTimeRow;

typedef struct HiscoreTable {
    HiscoreEntry entries[6];
} HiscoreTable;

extern HiscoreEntry hiscoreTable[6];
extern HiscoreEntry hiscoreTableCopy[6];
extern BestTime bestTimesCopy[];
extern char *startMenuStrings[][3];
extern char *menuStrings[][3];
extern s32 questionMode;
extern s32 debounceHiscorePadX;
extern f32 hiscoresArrowRScaleAim;
extern f32 hiscoresArrowLScaleAim;
extern f32 aimEA;

void CreditsInit(void) {
    creditsPerson = 1;
    creditsFrame = 0;
    creditsTitle = 0;
    creditsPX = -300.0f;
    creditsPY = 300.0f;
}

void CreditsRun(void) {
    s32 alphaTemp1;
    s32 alphaTemp2;

    if (creditsPX > 500.0f) {
        creditsFrame = 0;
        creditsPerson++;
        creditsPX = -300.0f;
        creditsPY = 300.0f;
        if (!creditsText[creditsPerson].title[0]) {
            creditsTitle = creditsPerson;
            if (creditsText[creditsPerson].name[0]) {
                creditsPerson++;
            }
        }
    }
    if (creditsPX < -1.0f) {
        creditsPY += creditsPX / 3.0f;
        creditsPX -= creditsPX / 3.0f;
        if (creditsPX > -1.0f) {
            creditsPX = 0.0f;
        }
    } else if (++creditsFrame > 50) {
        creditsPY += (creditsPX / 2.0f < 1.0f) ? 1.0f : creditsPX / 3.0f;
        creditsPX += (creditsPX / 2.0f < 1.0f) ? 1.0f : creditsPX / 3.0f;
    }
    alphaTemp1 = MAX(0, 255.0f - ABS(creditsPX));
    alphaTemp2 = MAX(0, 255.0f - ABS(creditsPX * 2.0f));
    SetFont(1);
    if ((creditsTitle == creditsPerson - 1 && creditsFrame <= 50)
        || (!creditsText[creditsPerson + 1].title[0] && creditsFrame > 50)) {
        NEW_TEXT((s32)creditsPX + 160, 180, 255, 255, 0, alphaTemp1, 1.0f, 1.0f, 1, creditsText[creditsTitle].name, 1);
    } else {
        NEW_TEXT(160, 180, 255, 255, 0, 255, 1.0f, 1.0f, 1, creditsText[creditsTitle].name, 1);
    }
    NEW_TEXT(160, (s32)creditsPY / 3 + 200, 255, 255, 255, alphaTemp2, 1.0f, 1.0f, 1, creditsText[creditsPerson].title, 1);
    SetFont(0);
}

s32 CreditsInit2(void) {
    creditsScroll = 0;
    newDesiredFrameRate = 3;
    globalFontScale = 1.0f;
    SetFont(1);
    controllerActive = TRUE;
    FadeScreenInN();
    fadeOutVal = 255;
    return TRUE;
}

s32 CreditsRun2(s32 *act, Screen *me) {
    s32 i;
    s32 yPos;

    ballPicOL.x -= (ballPicOL.x + 32.0f) / 4.0f;
    creditsScroll++;
    for (i = 0; !(creditsText[i].name[0] == ' ' && creditsText[i].name[1] == ' ' && creditsText[i].name[2] == ' ');
         i++) {
        yPos = i * creditsLineHeight - creditsScroll + 260;
        if (yPos > -20 && yPos < 260) {
            NEW_TEXT(55, yPos, 255, 255, 255, fadeOutVal, 1.0f, 1.0f, 1, creditsText[i].title, 0);
            NEW_TEXT(36, yPos, 255, 155, 155, fadeOutVal, 1.0f, 1.0f, 1, creditsText[i].name, 0);
        } else if (yPos < -19 && i > 0 && creditsText[i + 1].name[0] == ' ' && creditsText[i + 1].name[1] == ' ' &&
                   creditsText[i + 1].name[2] == ' ') {
            nextScreen = 1;
            FadeScreenOutN();
            return TRUE;
        }
    }
    TitleScreenCameraSpin2();
    DefWait(act, me);
    if (*act == 4 || *act == 5) {
        if (aimArRScale == 1 && !aimBgScale) {
            return FALSE;
        }
        FadeScreenOutN();
        nextScreen = 3;
        return TRUE;
    }
    return FALSE;
}

s32 CreditsDone2(void) {
    s32 i;
    s32 yPos;

    creditsScroll++;
    if (!aimBgScale) {
        MoveGloveOL();
        for (i = 0; !(creditsText[i].name[0] == ' ' && creditsText[i].name[1] == ' ' && creditsText[i].name[2] == ' ');
             i++) {
            yPos = i * creditsLineHeight - creditsScroll + 260;
            if (yPos > -20 && yPos < 260) {
                NEW_TEXT(55, yPos, 255, 255, 255, fadeOutVal, 1.0f, 1.0f, 1, creditsText[i].title, 0);
                NEW_TEXT(36, yPos, 255, 155, 155, fadeOutVal, 1.0f, 1.0f, 1, creditsText[i].name, 0);
            }
        }
        return FALSE;
    }
    newDesiredFrameRate = 2;
    globalFontScale = 1.0f;
    return TRUE;
}

s32 HiscoresDone(s32 act, s32 me) {
    if (!aimBgScale) {
        spinOutFGObjects(2);
        HiscoresRun((s32 *)act, (struct Screen *)me);
        MoveGloveOL();
        return FALSE;
    }
    DestroyOverlay(&arrOL[0]);
    DestroyOverlay(&arrOL[1]);
    FreeFGObjects();
    return TRUE;
}

s32 HiscoresInit(s32 act, s32 me) {
    aA = 255;
    aB = 0;
    aaA = 255;
    aaB = 0;
    aimLA = 0;
    aimRA = 255;
    hTime = 0;
    Xos = 0;
    XosVel = 0;
    initFGObject("titleboxb.ndo", 0.0f, -48.0f, 400.0f, 0.1f, 0.0f, 3.14f, 1.6f, 3.5f, 1.0f, 255);
    initFGObject("titleboxbf.ndo", 0.0f, -48.0f, 400.0f, 0.1f, 0.0f, 3.14f, 1.6f, 3.5f, 1.0f, 100);
    FindSetupAndAddOverlay(&arrOL[0], "arrow.bmp", 205, 210, 4, 1);
    FindSetupAndAddOverlay(&arrOL[1], "arrowrev.bmp", 125, 210, 4, 1);
    arrOL[1].alpha = 0;
    cXLU = 0;
    currentHiscoresLevel = -1;
    nextHiscoresLevel = -1;
    hiscoresBoxScale = 1.0f;
    hiscoresBoxScaleAim = 1.0f;
    FadeScreenInN();
    initSpinFGObjects(2);
    exiting = 0;
    Xos1 = 250.0f;
    HiscoresRun((s32 *)act, (struct Screen *)me);
    return TRUE;
}

void printHiscores(char *name, s32 start) {
    char temp[56]; // larger than needed to match the original stack frame

    NEW_TEXT(40.0f - Xos1, 60, 255, 255, 128, 255, 0.7f, 0.7f, 1, name, 0);
    SetFont(1);
    sprintf(temp, "%6s", hsLevNames[6][gameInfo.language]);
    NEW_TEXT(Xos1 + 55.0f, 80, 255, 155, 155, 255, 1.0f, 1.0f, 1, temp, 0);
    sprintf(temp, "%3s", bestTimes[start].name);
    NEW_TEXT(Xos1 + 170.0f, 80, 255, 155, 155, 255, 1.0f, 1.0f, 1, temp, 0);
    sprintf(temp, "%i:%02i", bestTimes[start].time / 60, bestTimes[start].time % 60);
    NEW_TEXT(Xos1 + 235.0f, 80, 255, 155, 155, 255, 1.0f, 1.0f, 1, temp, 0);
    sprintf(temp, "%6s", hsLevNames[7][gameInfo.language]);
    NEW_TEXT(55.0f - Xos1, 100, 255, 155, 155, 255, 1.0f, 1.0f, 1, temp, 0);
    sprintf(temp, "%3s", bestTimes[start + 1].name);
    NEW_TEXT(170.0f - Xos1, 100, 255, 155, 155, 255, 1.0f, 1.0f, 1, temp, 0);
    sprintf(temp, "%i:%02i", bestTimes[start + 1].time / 60, bestTimes[start + 1].time % 60);
    NEW_TEXT(235.0f - Xos1, 100, 255, 155, 155, 255, 1.0f, 1.0f, 1, temp, 0);
    sprintf(temp, "%6s", hsLevNames[8][gameInfo.language]);
    NEW_TEXT(Xos1 + 55.0f, 120, 255, 155, 155, 255, 1.0f, 1.0f, 1, temp, 0);
    sprintf(temp, "%3s", bestTimes[start + 2].name);
    NEW_TEXT(Xos1 + 170.0f, 120, 255, 155, 155, 255, 1.0f, 1.0f, 1, temp, 0);
    sprintf(temp, "%i:%02i", bestTimes[start + 2].time / 60, bestTimes[start + 2].time % 60);
    NEW_TEXT(Xos1 + 235.0f, 120, 255, 155, 155, 255, 1.0f, 1.0f, 1, temp, 0);
    sprintf(temp, "%6s", hsLevNames[9][gameInfo.language]);
    NEW_TEXT(55.0f - Xos1, 140, 255, 155, 155, 255, 1.0f, 1.0f, 1, temp, 0);
    sprintf(temp, "%3s", bestTimes[start + 3].name);
    NEW_TEXT(170.0f - Xos1, 140, 255, 155, 155, 255, 1.0f, 1.0f, 1, temp, 0);
    sprintf(temp, "%i:%02i", bestTimes[start + 3].time / 60, bestTimes[start + 3].time % 60);
    NEW_TEXT(235.0f - Xos1, 140, 255, 155, 155, 255, 1.0f, 1.0f, 1, temp, 0);
    sprintf(temp, "%6s", hsLevNames[10][gameInfo.language]);
    NEW_TEXT(Xos1 + 55.0f, 160, 255, 155, 155, 255, 1.0f, 1.0f, 1, temp, 0);
    sprintf(temp, "%3s", bestTimes[start + 4].name);
    NEW_TEXT(Xos1 + 170.0f, 160, 255, 155, 155, 255, 1.0f, 1.0f, 1, temp, 0);
    sprintf(temp, "%i:%02i", bestTimes[start + 4].time / 60, bestTimes[start + 4].time % 60);
    NEW_TEXT(Xos1 + 235.0f, 160, 255, 155, 155, 255, 1.0f, 1.0f, 1, temp, 0);
}

s32 HiscoresRun(s32 *act, Screen *me) {
    char buf[56];
    s32 i;
    Actor *obj;
    s32 j;
    f32 temp;

    SetFont(1);
    NEW_TEXT(260, spinInCount[0] / 4.0f + 227.0f, 255, 255, 255, 255, 1.0f, 1.0f, 1,
             startMenuStrings[10][gameInfo.language], 1);
    SetFont(0);
    arrOL[0].scaleX = 0x400;
    arrOL[0].scaleY = 0x400;
    arrOL[1].scaleX = 0x400;
    arrOL[1].scaleY = 0x400;
    if (currentHiscoresLevel == -1) {
        aimLA = 0;
    } else {
        aimLA = 255;
    }
    if (currentHiscoresLevel == 5) {
        aimRA = 0;
    } else {
        aimRA = 255;
    }
    aimEA = (fgSpin.minSpot < 2) ? (((111.0f - spinInCount[0]) * 255.0f / 110.0f > 255.0f) ? 255.0f : ((110.0f - spinInCount[0]) * 255.0f / 110.0f < 0.0f) ? 0.0f : (110.0f - spinInCount[0]) * 255.0f / 110.0f) : 255.0f;
    if (questionMode) {
        aimLA = 0;
        aimRA = 0;
        aimEA = 0.0f;
    }
    hTime++;
    hiscoresArrowRScaleAim = 1024.0f;
    hiscoresArrowLScaleAim = 1024.0f;
    arrOL[1].alpha -= (arrOL[1].alpha - aimLA) / 6;
    arrOL[0].alpha -= (arrOL[0].alpha - aimRA) / 6;
    spinInFGObjects(2);
    foregroundObjects[0]->pos.x = 0.0f;
    foregroundObjects[1]->pos.x = 0.0f;
    foregroundObjects[0]->pos.y = -48.0f;
    foregroundObjects[1]->pos.y = -48.0f;
    foregroundObjects[0]->rot.y = 0.0f;
    foregroundObjects[1]->rot.y = 0.0f;
    foregroundObjects[0]->rot.x = 0.0f;
    foregroundObjects[1]->rot.x = 0.0f;
    obj = foregroundObjects[0];
    obj->rot.z = __sinf(animFloat / 7.0f) * 0.05 + 3.14;
    obj = foregroundObjects[1];
    obj->rot.z = __sinf(animFloat / 7.0f) * 0.05 + 3.14;
    foregroundObjects[0]->alpha = (fgSpin.minSpot < 2) ? (((111.0f - spinInCount[0]) * 255.0f / 110.0f > 255.0f) ? 255 : ((110.0f - spinInCount[0]) * 255.0f / 110.0f < 0.0f) ? 0 : (110.0f - spinInCount[0]) * 255.0f / 110.0f) : 255;
    foregroundObjects[1]->alpha = (fgSpin.minSpot < 2) ? (((111.0f - spinInCount[0]) * 255.0f / 110.0f > 255.0f) ? 127 : ((110.0f - spinInCount[0]) * 255.0f / 110.0f < 0.0f) ? 0 : (110.0f - spinInCount[0]) * 255.0f / 110.0f / 2.0f) : 127;
    for (j = 0; j < numForegroundObjects; j++) {
        GetQuaternionFromXYZRot(&foregroundObjects[j]->rotQuat, &foregroundObjects[j]->rot);
    }
    hiscoresBoxScale += (hiscoresBoxScaleAim - hiscoresBoxScale) / 3.0f;
    foregroundObjects[0]->scale.y = hiscoresBoxScale * 3.5;
    foregroundObjects[0]->scale.x = hiscoresBoxScale * 1.6;
    foregroundObjects[1]->scale.y = hiscoresBoxScale * 3.5;
    foregroundObjects[1]->scale.x = hiscoresBoxScale * 1.6;
    if (controllerActive && !questionMode) {
        if (abs(pad[ActiveController].stickX) > 20) {
            if (pad[ActiveController].stickX > 0) {
                if (currentHiscoresLevel < 5) {
                    ballPicOL.x = 210.0f;
                    hiscoresArrowRScaleAim = 600.0f;
                    if (!debounceHiscorePadX) {
                        nextHiscoresLevel = currentHiscoresLevel + 1;
                        hiscoresBoxScaleAim = 1.2f;
                        PlaySampleNot3D(212, 100, 128, 128);
                        debounceHiscorePadX = 40;
                        Xos = 0.0f;
                        cXLU = 0;
                        XosVel = 53.0f;
                    }
                }
            } else if (currentHiscoresLevel >= 0) {
                ballPicOL.x = 110.0f;
                hiscoresArrowLScaleAim = 600.0f;
                if (!debounceHiscorePadX) {
                    nextHiscoresLevel = currentHiscoresLevel - 1;
                    hiscoresBoxScaleAim = 1.2f;
                    PlaySampleNot3D(212, 100, 128, 128);
                    debounceHiscorePadX = 40;
                    Xos = 0.0f;
                    cXLU = 0;
                    XosVel = 53.0f;
                }
            }
        }
    }
    if (hiscoresBoxScale > 1.14) {
        hiscoresBoxScaleAim = 1.0f;
    }
    Xos1 += XosVel;
    if (Xos1 < 0.0f) {
        XosVel = -(XosVel * 0.2);
        Xos += 1.0f;
    }
    if (Xos > 2.0f) {
        XosVel = 0.0f;
        Xos1 = 0.0f;
    }
    if (!exiting && Xos1 > 250.0f) {
        currentHiscoresLevel = nextHiscoresLevel;
        XosVel = -(XosVel * 0.4);
    }
    if (Xos1 < 0.0f) {
        Xos1 = 0.0f;
    }
    XosVel -= 1.0f;
    temp = (arrOL[0].scaleY - hiscoresArrowRScaleAim) / 3.0f;
    arrOL[0].x = spinInCount[0] + 205.0f;
    arrOL[1].x = 125.0f - spinInCount[0];
    arrOL[0].scaleX = arrOL[0].scaleY = arrOL[0].scaleY - temp;
    arrOL[1].scaleX = arrOL[1].scaleY = arrOL[1].scaleY - (arrOL[1].scaleY - hiscoresArrowLScaleAim) / 3.0f;
    if (cXLU < 255) {
        cXLU += (255 - cXLU) / 3;
    }
    if (cXLU > 255) {
        cXLU = 255;
    }
    if (debounceHiscorePadX) {
        debounceHiscorePadX--;
    }
    for (j = 0; j < numForegroundObjects; j++) {
        GetQuaternionFromXYZRot(&foregroundObjects[j]->rotQuat, &foregroundObjects[j]->rot);
    }
    SetFont(0);
    if (currentHiscoresLevel == -1) {
        aaA = 255;
        aaB = 0;
    } else {
        aaB = 255;
        aaA = 0;
    }
    aA -= (aA - aaA) / 5;
    aB -= (aB - aaB) / 5;
    NEW_TEXT(160, 30.0f - spinInCount[0] / 4.0f, 255, 255, 255, aA, 1.0f, 1.0f, 1, menuStrings[9][gameInfo.language], 1);
    NEW_TEXT(160, 30.0f - spinInCount[0] / 4.0f, 255, 255, 255, aB, 1.0f, 1.0f, 1, menuStrings[8][gameInfo.language], 1);
    switch (currentHiscoresLevel) {
        case -1:
            SetFont(1);
            for (i = 0; i < 6; i++) {
                hiscoreTable[i].name[3] = 0;
                NEW_TEXT(120.0f - Xos1, 80 + i * 20, 255, 155, 155, 255, 1.0f, 1.0f, 1, hiscoreTable[i].name, 1);
                sprintf(buf, "%lu", hiscoreTable[i].score);
                NEW_TEXT(Xos1 + 200.0f, 80 + i * 20, 255, 155, 155, 255, 1.0f, 1.0f, 1, buf, 1);
            }
            break;
        case 0:
            printHiscores(hsLevNames[0][gameInfo.language], 0);
            break;
        case 1:
            printHiscores(hsLevNames[1][gameInfo.language], 5);
            break;
        case 2:
            printHiscores(hsLevNames[2][gameInfo.language], 10);
            break;
        case 3:
            printHiscores(hsLevNames[3][gameInfo.language], 15);
            break;
        case 4:
            printHiscores(hsLevNames[4][gameInfo.language], 20);
            break;
        case 5:
            printHiscores(hsLevNames[5][gameInfo.language], 25);
            break;
    }
    DefWait(act, me);
    if (questionMode) {
        ballPicOL.x = 560.0f;
        if (*act == 4) {
            switch (currentHiscoresLevel) {
                case -1:
                    *(HiscoreTable *)hiscoreTable = *(HiscoreTable *)hiscoreTableCopy;
                    break;
                case 0:
                    *(BestTimeRow *)&bestTimes[0] = *(BestTimeRow *)&bestTimesCopy[0];
                    break;
                case 1:
                    *(BestTimeRow *)&bestTimes[5] = *(BestTimeRow *)&bestTimesCopy[5];
                    break;
                case 2:
                    *(BestTimeRow *)&bestTimes[10] = *(BestTimeRow *)&bestTimesCopy[10];
                    break;
                case 3:
                    *(BestTimeRow *)&bestTimes[15] = *(BestTimeRow *)&bestTimesCopy[15];
                    break;
                case 4:
                    *(BestTimeRow *)&bestTimes[20] = *(BestTimeRow *)&bestTimesCopy[20];
                    break;
                case 5:
                    *(BestTimeRow *)&bestTimes[25] = *(BestTimeRow *)&bestTimesCopy[25];
                    break;
            }
            *act = 6;
            questionMode = 0;
        }
        if (*act == 5) {
            questionMode = 0;
            *act = 6;
        }
    }
    if (*act == 5) {
        if (aimArRScale == 1 && !aimBgScale) {
            return FALSE;
        }
        FadeScreenOutN();
        Xos = 0.0f;
        cXLU = 0;
        XosVel = 53.0f;
        initSpinOutFGObjects(2);
        exiting = 1;
        nextScreen = 3;
        return TRUE;
    }
    return FALSE;
}

void KanafyCredits(void) {
    s16 i;

    for (i = 0; !(creditsText[i].name[0] == ' ' && creditsText[i].name[1] == ' ' && creditsText[i].name[2] == ' ');
         i++) {
        KanafyLetterString(creditsText[i].title);
        KanafyLetterString(creditsText[i].name);
    }
}

INCLUDE_ASM("asm/nonmatchings/credits", pad_80117018);

// linker junk that follows the 53.0f constant emitted by HiscoresRun
const u32 D_80100838[] = { 0x63686172, 0x5F747261 };
