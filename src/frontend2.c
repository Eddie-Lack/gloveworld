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
#include "eeprom.h"
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
extern s32 Xos;
extern s32 XosVel;
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

s32 WaitNameScr(s32 *act, Screen *me);

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
s32 HiscoresRun(s32 act, s32 me);
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
s32 WaitOKScr(s32 *act, Screen *me);
s32 TimeTrialScr(s32 *act, Screen *me);
extern s32 bonusClosedAlpha;
extern char *difficultyStrings[][3];
extern char *startMenuStrings[][3];
extern char *menuStrings[][3];

void UpdateSaveSlot(s32 allGaribs);

extern s32 doFades;
extern s16 musVol;
void MusSetMasterVolume(s32 flags, s32 volume);
extern s32 activateOK;
extern s32 inputOK;
extern char letters[];
extern s32 currentLetter;
extern u32 lettersY;
extern s32 nameWait;
extern u32 lettersX;
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
extern f32 jogAmt[];

s32 MainTitleWait();

extern char bkspc[];
extern char space[];
extern char end[];
extern u32 centerX;
extern s32 centerY;
extern s32 centerDX;
extern u32 centerDY;
extern s32 debouncePadX;

s32 SetupNameScr(s32 *act, Screen *me) {
    inputOK = 1;
    FadeScreenInN();
    SetFont(0);
    lettersX = strlen(letters) / lettersY;
    currentLetter = 0;
    nameWait = 0;
    initFGObject("titleboxb.ndo", 0.0f, -38.0f, 400.0f, 0.1f, 0.0f, 3.14f, 2.0f, 3.8f, 1.0f, 255);
    initFGObject("titleboxbf.ndo", 0.0f, -38.0f, 400.0f, 0.1f, 0.0f, 3.14f, 2.0f, 3.8f, 1.0f, 100);
    initSpinFGObjects(2);
    memset(enteredName, 0, 4);
    StartDrawing("initnamescr");
    WaitNameScr(act, me);
    return TRUE;
}

s32 SetupOKScr(s32 *act, Screen *me) {
    controllerActive = FALSE;
    FadeScreenInN();
    SetFont(0);
    jogAmt[0] = 0.0f;
    jogAmt[1] = 0.0f;
    jogAmt[2] = 0.0f;
    jogAmt[3] = 0.0f;
    initFGObject("newwave.ndo", 0.0f, -30.0f, 400.0f, 0.1f, 0.0f, 3.14f, 1.0f, 1.0f, 1.0f, 255);
    InitActorStructures(foregroundObjects[0], 4);
    AnimateActor(foregroundObjects[0], 0, 1, 1, 1.0f);
    foregroundObjects[0]->inst->obj->flags |= 2;
    initFGObject("newwave.ndo", 0.0f, 25.0f, 400.0f, 0.1f, 0.0f, 3.14f, 1.0f, 1.0f, 1.0f, 255);
    InitActorStructures(foregroundObjects[1], 4);
    AnimateActor(foregroundObjects[1], 0, 1, 1, 0.9f);
    foregroundObjects[1]->inst->obj->flags |= 2;
    toStartGame = 0;
    me->currentSpot = 1;
    initSpinFGObjects(2);
    fgDirections[1] = 3;
    fgDirections[3] = 2;
    fgDirections[0] = 1;
    fgDirections[2] = 0;
    *act = 6;
    WaitOKScr(act, me);
    return TRUE;
}

s32 strncmp(char *a, char *b, s32 n) {
    s32 i;

    for (i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            return 1;
        }
        if (a[i] == '\0') {
            return 0;
        }
    }
    return 0;
}

inline void ActStartGame(void) {
    s32 slot;

    KillTitle();
    StopDrawing("actstng");
    StartNewGame();
    slot = currentSaveSlot;
    saveSlot[slot].name[0] = currentName[0] = enteredName[0];
    saveSlot[currentSaveSlot].name[1] = currentName[1] = enteredName[1];
    saveSlot[currentSaveSlot].name[2] = currentName[2] = enteredName[2];
    UpdateSaveSlot(0);
    StartNewLevel(INTRO);
}

s32 WaitOKScr(s32 *act, Screen *me) {
    SetFont(1);
    NEW_TEXT(260, spinInCount[0] / 4.0f + 227.0f, 255, 255, 255, 255, 1.0f, 1.0f, 1, startMenuStrings[10][gameInfo.language], 1);
    NEW_TEXT(spinInCount[0] + 160.0f, 190, 255, 255, 255, bonusClosedAlpha, 1.0f, 1.0f, 1, startMenuStrings[14][gameInfo.language], 1);
    if (me->currentSpot == 0) {
        bonusClosedAlpha = MIN(bonusClosedAlpha + 30, 255);
    } else {
        bonusClosedAlpha = MAX(bonusClosedAlpha - 30, 0);
    }
    SetFont(0);
    NEW_TEXT(160, 30.0f - spinInCount[0], 255, 255, 55, 255, 0.8f, 0.8f, 1, difficultyStrings[4][gameInfo.language], 1);
    NEW_TEXT(spinInCount[1] + 160.0f, 100, 255, 255, 255,
             me->currentSpot == 0 ? (u8)(sinf(animFloat * 0.65) * 90.0f + 128.0f + 38.0f) : 255,
             me->currentSpot != 0 ? 0.8f : 1.0f, me->currentSpot != 0 ? 0.8f : 1.0f, 1,
             difficultyStrings[0][gameInfo.language], 1);
    NEW_TEXT(160.0f - spinInCount[0], 139, 255, 255, 255,
             me->currentSpot == 1 ? (u8)(sinf(animFloat * 0.65) * 90.0f + 128.0f + 38.0f) : 255,
             me->currentSpot != 1 ? 0.8f : 1.0f, me->currentSpot != 1 ? 0.8f : 1.0f, 1,
             difficultyStrings[1][gameInfo.language], 1);
    if (fadeDone) {
        DefWait(act, me);
    }
    foregroundObjects[0]->rot.x = jogAmt[1];
    foregroundObjects[0]->rot.y = sinf(animFloat * 0.2) * 0.1 + 3.14;
    foregroundObjects[0]->rot.z = sinf(animFloat * 0.15) * 0.1;
    foregroundObjects[1]->rot.x = jogAmt[0];
    foregroundObjects[1]->rot.y = sinf(animFloat * 0.3) * 0.1 + 3.14;
    foregroundObjects[1]->rot.z = sinf(animFloat * 0.15) * 0.1;
    if (*act == 0 || *act == 1) {
        jogAmt[me->currentSpot] = 6.28f;
    }
    jogAmt[0] *= 0.9;
    jogAmt[1] *= 0.9;
    jogAmt[2] *= 0.9;
    jogAmt[3] *= 0.9;
    if (*act == 5) {
        nextScreen = *act;
        toStartGame = 0;
        FadeScreenOutN();
        initSpinOutFGObjects(2);
        return TRUE;
    }
    if (*act == 4) {
        if (fadeDir == 1 && !fadeDone) {
            return FALSE;
        }
        switch (me->currentSpot) {            case 0:
                memclear(&saveSlot[currentSaveSlot], sizeof(SaveSlot));
                saveSlot[currentSaveSlot].difficultyLevel = DIFFICULTY_EASY;
                toStartGame = 1;
                doFades = 1;
                break;
            case 1:
                memclear(&saveSlot[currentSaveSlot], sizeof(SaveSlot));
                saveSlot[currentSaveSlot].difficultyLevel = DIFFICULTY_MEDIUM;
                toStartGame = 1;
                doFades = 1;
                break;
            case 2:
                break;
        }
        initSpinOutFGObjects(2);
        FadeScreenOutN();
        return TRUE;
    }
    spinInFGObjects(2);
    return FALSE;
}

s32 WaitNameScr(s32 *act, Screen *me) {
    s32 i = 0;
    s32 k;
    u32 row;
    u32 col;
    f32 curScale;

    spinInFGObjects(2);
    SetFont(1);
    if (saveSlot[currentSaveSlot].difficultyLevel != DIFFICULTY_HARD) {
        NEW_TEXT(260, spinInCount[0] / 4.0f + 227.0f, 255, 255, 255, 255, 1.0f, 1.0f, 1, startMenuStrings[10][gameInfo.language], 1);
    }
    SetFont(0);
    UpdatePause();
    foregroundObjects[0]->pos.x = 0.0f;
    foregroundObjects[1]->pos.x = 0.0f;
    foregroundObjects[0]->pos.y = -38.0f;
    foregroundObjects[1]->pos.y = -38.0f;
    foregroundObjects[0]->rot.x = 0.0f;
    foregroundObjects[1]->rot.x = 0.0f;
    foregroundObjects[0]->rot.y = 0.0f;
    foregroundObjects[1]->rot.y = 0.0f;
    foregroundObjects[0]->rot.z = sinf(animFloat / 7.0f) * 0.05 + 3.14;
    foregroundObjects[1]->rot.z = sinf(animFloat / 7.0f) * 0.05 + 3.14;
    foregroundObjects[0]->alpha = (fgSpin.minSpot < 2) ? (((111.0f - spinInCount[0]) * 255.0f / 110.0f > 255.0f) ? 255
                                                   : ((110.0f - spinInCount[0]) * 255.0f / 110.0f < 0.0f)
                                                       ? 0
                                                       : (s32)((110.0f - spinInCount[0]) * 255.0f / 110.0f))
                                                : 255;
    foregroundObjects[1]->alpha = (fgSpin.minSpot < 2) ? (((111.0f - spinInCount[0]) * 255.0f / 110.0f > 255.0f) ? 127
                                                   : ((110.0f - spinInCount[0]) * 255.0f / 110.0f < 0.0f)
                                                       ? 0
                                                       : (s32)((110.0f - spinInCount[0]) * 255.0f / 110.0f / 2.0f))
                                                : 127;
    for (k = 0; k < numForegroundObjects; k++) {
        GetQuaternionFromXYZRot(&foregroundObjects[k]->rotQuat, &foregroundObjects[k]->rot);
    }
    for (row = 0; row < lettersY; row++) {
        for (col = 0; col < lettersX; col++) {
            char tempStr[20] = "M";

            tempStr[0] = letters[i];
            curScale = 0.8f;
            switch (letters[i]) {
                case '$':
                    memcpy(tempStr, bkspc, strlen(bkspc));
                    break;
                case 0xA3:
                    memcpy(tempStr, end, strlen(end));
                    break;
                case ' ':
                    memcpy(tempStr, space, strlen(space));
                    break;
                case '@':
                    memcpy(tempStr, " ", strlen(" "));
                    break;
            }
            if (i == currentLetter) {
                curScale *= 1.5;
            }
            if (row & 1) {
                NEW_TEXT((f32)(centerX + col * 20) - spinInCount[1], centerY + row * 20, 255, 255, 255, 255, curScale, curScale, 1, tempStr, 1);
            } else {
                NEW_TEXT((f32)(centerX + col * 20) + spinInCount[1], centerY + row * 20, 255, 255, 255, 255, curScale, curScale, 1, tempStr, 1);
            }
            if (i == currentLetter) {
                ballPicOL.x -= (ballPicOL.x - (f32)(centerX + col * 20 - 10)) / 4.0f;
                ballPicOL.y -= (ballPicOL.y - (f32)(centerY + row * 20 - 10)) / 4.0f;
            }
            i++;
        }
    }
    NEW_TEXT(centerDX, (f32)centerDY + spinInCount[0], 255, 255, 255, 255, 1.0f, 1.0f, 1, enteredName, 0);
    if (saveSlot[currentSaveSlot].difficultyLevel == DIFFICULTY_HARD) {
        NEW_TEXT(160, 30.0f - spinInCount[1] / 2.0f, 255, 255, 55, 255, 0.8f, 0.8f, 1, menuStrings[10][gameInfo.language], 1);
    } else {
        NEW_TEXT(160, 30.0f - spinInCount[1] / 2.0f, 255, 255, 55, 255, 0.8f, 0.8f, 1, menuStrings[3][gameInfo.language], 1);
    }
    if (abs(pad[ActiveController].stickX) > 20 && !debouncePadX) {
        PlaySampleNot3D(212, 100, 128, 128);
        if (pad[ActiveController].stickX > 0) {
            if (currentLetter % lettersX == lettersX - 1) {
                currentLetter -= 9;
            } else {
                currentLetter++;
            }
            while (letters[currentLetter] == '@') {
                currentLetter++;
            }
        } else {
            if (currentLetter % lettersX == 0) {
                currentLetter += 9;
            } else {
                currentLetter--;
            }
        }
        while (letters[currentLetter] == '@') {
            currentLetter--;
        }
        debouncePadX = 7;
    }
    if (abs(pad[ActiveController].stickY) > 20 && !debouncePadX) {
        PlaySampleNot3D(212, 100, 128, 128);
        if (pad[ActiveController].stickY > 0) {
            if (currentLetter < lettersX) {
                currentLetter += 40;
            } else {
                currentLetter -= lettersX;
            }
            while (letters[currentLetter] == '@') {
                currentLetter -= lettersX;
            }
        } else {
            if (currentLetter < strlen(letters) - lettersX) {
                currentLetter += lettersX;
            } else {
                currentLetter -= 40;
            }
        }
        while (letters[currentLetter] == '@') {
            currentLetter++;
        }
        debouncePadX = 7;
    }
    if ((fadeDone || fadeDir != 1) && controllerActive && pad[ActiveController].pressed[BUTTON_Z] && strlen(enteredName)) {
        enteredName[strlen(enteredName) - 1] = '\0';
        PlaySampleNot3D(217, 100, 128, 128);
    }
    if (controllerActive && pad[ActiveController].pressed[BUTTON_A] && !nameWait) {
        PlaySampleNot3D(216, 100, 128, 128);
        if (letters[currentLetter] == '$') {
            if (strlen(enteredName)) {
                enteredName[strlen(enteredName) - 1] = '\0';
            }
        } else if (letters[currentLetter] == 0xA3) {
            FadeScreenOutN();
            nextScreen = 10;
            initSpinOutFGObjects(2);
            nameWait = 1;
            return TRUE;
        } else if (strlen(enteredName) < 3) {
            enteredName[strlen(enteredName)] = letters[currentLetter];
            enteredName[strlen(enteredName) + 1] = '\0';
            if (strlen(enteredName) == 3) {
                currentLetter = strlen(letters) - 1;
            }
        } else {
            currentLetter = strlen(letters) - 1;
        }
    }
    if (debouncePadX) {
        debouncePadX--;
    }
    if (*act == 5) {
        if (nextScreen != 2) {
            PlaySampleNot3D(217, 100, 128, 128);
            nextScreen = 2;
            FadeScreenOutN();
            initSpinOutFGObjects(2);
            return TRUE;
        }
    }
    return FALSE;
}

s32 DoneNameScr(s32 *act, Screen *me) {
    if (!fadeDone) {
        spinOutFGObjects(2);
        WaitNameScr(act, me);
        MoveGloveOL();
        return FALSE;
    }
    FreeFGObjects();
    if (saveSlot[currentSaveSlot].difficultyLevel == DIFFICULTY_HARD) {
        bestTimes[levelInfo[gameInfo.prevLevel].bit].name[0] = enteredName[0];
        bestTimes[levelInfo[gameInfo.prevLevel].bit].name[1] = enteredName[1];
        bestTimes[levelInfo[gameInfo.prevLevel].bit].name[2] = enteredName[2];
        SaveBestTimes();
        SaveCRC();
        TimeTrialScr(act, me);
    }
    return TRUE;
}

s32 WaitContinueScr(void) {
    if (!fadeDone) {
        MoveGloveOL();
        return FALSE;
    }
    onlyDrawBackdrops = FALSE;
    SetFadeColour(0, 0, 0);
    gameInfo.fade2 = gameInfo.fade = 300;
    UpdateFade();
    memset(cfb_16_a, 0, 0x25800);
    memset(cfb_16_b, 0, 0x25800);
    ActContinueGame();
    return TRUE;
}

s32 WaitCheatScr(void) {
    if (!fadeDone) {
        MoveGloveOL();
        return FALSE;
    }
    SetFadeColour(0, 0, 0);
    gameInfo.fade2 = gameInfo.fade = 300;
    UpdateFade();
    memset(cfb_16_a, 0, 0x25800);
    memset(cfb_16_b, 0, 0x25800);
    CheatStartGame(2);
    return TRUE;
}

s32 DoneOKScr(s32 *act, Screen *me) {
    if (!fadeDone) {
        WaitOKScr(act, me);
        spinOutFGObjects(2);
        if (toStartGame) {
            doFades = 1;
        }
        return FALSE;
    }
    FreeFGObjects();
    if (toStartGame) {
        SetFadeColour(0, 0, 0);
        gameInfo.fade2 = gameInfo.fade = 300;
        UpdateFade();
        memset(cfb_16_a, 0, 0x25800);
        memset(cfb_16_b, 0, 0x25800);
        ActStartGame();
    }
    return TRUE;
}

INCLUDE_ASM("asm/nonmatchings/frontend2", pad_8011AB24);

INCLUDE_RODATA("asm/nonmatchings/frontend2", D_80100C24);
