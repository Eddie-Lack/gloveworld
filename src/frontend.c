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
#include "frontend.h"
#include "decompress.h"
#include "eeprom.h"
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
extern Overlay D_801F64B4;
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

extern s32 copyMode;
extern s16 copyFromSlot;
extern s32 questionMode;
extern s32 aborting;
extern s32 yPsn;
extern Overlay slotsOL[6];
extern s32 selectingSlots;
extern s32 selectedSlot;
extern s32 debounceSlotPadX;
extern char *startMenuStrings[][3];
extern char *menuStrings[][3];
extern f32 jogAmt[];

void NoAction(void) {
}

s32 TimeTrialScr(s32 *act, Screen *me) {
    if (!fadeDone) {
        MoveGloveOL();
        return FALSE;
    }
    SetFadeColour(0, 0, 0);
    gameInfo.fade2 = gameInfo.fade = 300;
    UpdateFade();
    memset(cfb_16_a, 0, 0x25800);
    memset(cfb_16_b, 0, 0x25800);
    CheatStartGame(15);
    return TRUE;
}

s32 PracticeScr(void) {
    if (!fadeDone) {
        MoveGloveOL();
        return FALSE;
    }
    cameFromTitle = TRUE;
    SetFadeColour(0, 0, 0);
    gameInfo.fade2 = gameInfo.fade = 300;
    UpdateFade();
    StopDrawing("practice");
    memset(cfb_16_a, 0, 0x25800);
    memset(cfb_16_b, 0, 0x25800);
    KillTitle();
    currentSaveSlot = 6;
    memclear(&saveSlot[6], sizeof(SaveSlot));
    saveSlot[currentSaveSlot].difficultyLevel = DIFFICULTY_MEDIUM;
    StartNewGame();
    StartNewLevel(ACOURSE);
    nextScreen = 13;
    currentScreen = 1;
    currentAnim = 0;
    levelSelectCount = 0;
    introAnimTime = 31;
    return TRUE;
}

void DoneFadeOverlayN(void) {
    DestroyOverlay(&fadeNiceOL[0]);
    DestroyOverlay(&fadeNiceOL[1]);
    fadeAvailable = 0;
}

inline void InitFadeOverlayN(void) {
    FindSetupAndAddOverlay(&fadeNiceOL[0], "fade1.bmp", 160, 120, 0, 1);
    FindSetupAndAddOverlay(&fadeNiceOL[1], "fade2.bmp", 160, 120, 0, 1);
    fadeNiceOL[0].scaleY = fadeNiceOL[1].scaleY = 0x2454;
    fadeNiceOL[0].scaleX = fadeNiceOL[1].scaleX = 0x32C8;
    fadeNiceOL[0].r = fadeNiceOL[0].g = fadeNiceOL[0].b = fadeNiceOL[1].r = fadeNiceOL[1].g = fadeNiceOL[1].b = 0;
    fadeDir = fadeNiceOL[1].behind = fadeNiceOL[0].behind = 0;
    fadeAvailable = 1;
    fadeNiceOL[0].x = -250.0f;
    fadeNiceOL[1].x = 550.0f;
}

void FadeScreenOutN(void) {
    if (!fadeAvailable) {
        InitFadeOverlayN();
    }
    fadeDir = 1;
    fadeOutVal = 255;
    fadeNiceOL[0].x = -220.0f;
    fadeNiceOL[1].x = 520.0f;
    fadeAdd = 1.2f;
}

void MoveGloveOL(void) {
}

void FadeScreenInN(void) {
    if (!fadeAvailable) {
        InitFadeOverlayN();
    }
    fadeDir = -1;
    fadeNiceOL[0].x = 178.0f;
    fadeNiceOL[1].x = 122.0f;
    fadeAdd = 1.5f;
}

void RunFadeN(void) {
    if (fadeAvailable) {
        if (!doFades) {
            fadeNiceOL[1].y = fadeNiceOL[0].y = 520.0f;
        } else {
            fadeNiceOL[1].y = fadeNiceOL[0].y = 120.0f;
        }
        fadeDone = 1;
        if (fadeDir == -1) {
            if (fadeNiceOL[0].x > -250.0f) {
                fadeDone = 0;
            }
        } else {
            if (doFades) {
                if (musVol > 0) {
                    musVol -= 1000;
                }
                if (musVol < 0) {
                    musVol = 0;
                }
                MusSetMasterVolume(2, musVol);
            }
            if (fadeNiceOL[0].x < 160.0f) {
                if (fadeOutVal > 0) {
                    fadeOutVal -= 10;
                }
                if (fadeOutVal < 0) {
                    fadeOutVal = 0;
                }
                fadeDone = 0;
            }
        }
        if (!fadeDone) {
            fadeNiceOL[0].x += fadeAdd * fadeDir;
            fadeNiceOL[1].x -= fadeAdd * fadeDir;
            fadeAdd *= 1.16;
        }
    }
}

s32 ATeamInit(void) {
    s32 i;

    if (licenceFrame == 0) {
        LoadTextureBank(TITLE_TEX_BANK);
        farPlaneDist = 30000;
    }
    if (licenceFrame == 1) {
        pBackground = JallocAlloc(sizeof(Backdrop), 1, "Backdrop");
        pBackground->r = pBackground->g = pBackground->b = 255;
        pBackground->alpha = 0;
        pBackground->x = 10;
        AddBackdrop(pBackground);
        FindTexture(&pBackground->tex, UpdateCRC(osTvType == 0 ? "n64.bmp" : "licence.bmp"), 1);
        pBackground->scaleX = 0x400;
        pBackground->scaleY = 0x400;
        fog.r = 0;
        fog.g = 0;
        fog.b = 0;
    }
    if (licenceFrame >= 2) { // a separate test keeps GCC from folding the range check
        if (licenceFrame < 70 && pBackground->alpha < 255) {
            pBackground->alpha = (pBackground->alpha + 10 > 255) ? 255 : pBackground->alpha + 10;
        }
    }
    licenceFrame++;
    if (licenceFrame == 125 || licenceFrame == 250) {
        FadeScreenOut();
    }
    if (licenceFrame > 125 && ScreenFadedOut() && D_801E64D8 == 0) {
        if (osTvType == 0) {
            D_801E64D8 = 1;
            FindTexture(&pBackground->tex, UpdateCRC("licence.bmp"), 1);
            FadeScreenIn();
        } else {
            goto done;
        }
    }
    if (licenceFrame > 250 && ScreenFadedOut() && D_801E64D8 == 1) {
    done:
        for (i = 0; i < 5; i++) {
            JallocFree(&textureBanks[i].data);
        }
        for (i = 0; i < NUM_SPRITE_ANIMS; i++) {
            JallocFree(&spriteFrameList[i].textures);
        }
        FreeBackdropList();
        return TRUE;
    }
    return FALSE;
}

s32 ATeamRun(void) {
    A2OL.g = A1OL.g = 255;
    A2OL.r = A1OL.b = A1OL.r = 0;
    A2OL.b = 230;
    A2OL.alpha = ~cAAlpha;
    A1OL.alpha = cAAlpha;
    if (cAAlphaAim < cAAlpha) {
        cAAlpha -= 6;
    }
    if (cAAlpha < cAAlphaAim) {
        cAAlpha += 5;
    }
    if (abs(cAAlpha - cAAlphaAim) < 6) {
        cAAlphaAim = Random(255);
    }
    return 0;
}

void KillTitle(void) {
    Overlay *overlay;
    Overlay *overlay2;
    GameSprite *sprite;
    GameSprite *sprite2;
    s32 i;
    s32 j;

    newDesiredFrameRate = 3;
    globalFontScale = 1.0f;
    StopDrawing("killtitle");
    ClearText();
    DestroyOverlay(&fadeNiceOL[0]);
    DestroyOverlay(&D_801F64B4); // fadeNiceOL[1]: the ROM loads this address again instead of adding 0x74
    fadeAvailable = 0;
    DestroyOverlay(&ballPicOL);
    JallocFree(&pBackground);
    SetFadeColour(0, 0, 0);
    gameInfo.fade = 300;
    gameInfo.fade2 = 300;
    UpdateFade();
    FreeFGObjects();
    gameInfo.mode = 4;
    currentScreen = 1;
    nextScreen = 1;
    currentAnim = 7;
    levelSelectCount = 0;
    introAnimTime = 31;
    cameraInfo.overrideTimer = 0;
    frameCount = 0;
    ShowJalloc();
    InitOverlayList();
    InitSpriteList();
    InitActorList();
    InitBackdropList();
    InitBackgroundLightList();
    InitAmbientSoundList();
    for (i = 0; i < 5; i++) {
        for (j = 0; j < objectBanks[i].count; j++) {
            FreeOFTC(objectBanks[i].entries[j].inst->obj);
        }
        JallocFree(&objectBanks[i].data);
        objectBanks[i].count = 0;
    }
    for (i = 0; i < 5; i++) {
        JallocFree(&textureBanks[i].data);
    }
    for (i = 0; i < NUM_SPRITE_ANIMS; i++) {
        JallocFree(&spriteFrameList[i].textures);
    }
    for (overlay = overlayList.head.next; overlay != &overlayList.head; overlay = overlay2) {
        overlay2 = overlay->next;
        DestroyOverlay(overlay);
        overlay->next = NULL;
        JallocFree(&overlay);
    }
    InitOverlayList();
    for (sprite = spriteList.head.next; sprite != &spriteList.head; sprite = sprite2) {
        sprite2 = sprite->next;
        JallocFree(&sprite);
    }
    FreeBackdropList();
    InitLinkedLists();
    ShowJalloc();
    LoadFont();
    DestroyAndFreeActor(&camera);
    DestroyAndFreeActor(&cameraInfo.actor);
    JallocInit((u32)jallocHeapStart, 0x80400000 - (u32)jallocHeapStart);
    LoadObjectBank(0);
    ResetCamera();
}

void initFGObject(char *NameString, f32 XPos, f32 YPos, f32 ZPos, f32 XRot, f32 YRot, f32 ZRot, f32 XSize, f32 YSize, f32 ZSize, s32 TL) {
    s32 i;

    foregroundObjects[numForegroundObjects] = JallocAlloc(sizeof(Actor), 1, "FGObj");
    InitActor(foregroundObjects[numForegroundObjects], NameString, XPos, YPos, ZPos, 0);
    foregroundObjects[numForegroundObjects]->scale.x = XSize;
    foregroundObjects[numForegroundObjects]->scale.y = YSize;
    foregroundObjects[numForegroundObjects]->scale.z = ZSize;
    foregroundObjects[numForegroundObjects]->rot.x = XRot;
    foregroundObjects[numForegroundObjects]->rot.y = YRot;
    foregroundObjects[numForegroundObjects]->rot.z = ZRot;
    if (TL != 255) {
        foregroundObjects[numForegroundObjects]->inst->obj->flags |= 2;
    }
    foregroundObjects[numForegroundObjects++]->alpha = TL;
    for (i = 0; i < numForegroundObjects; i++) {
        GetQuaternionFromXYZRot(&foregroundObjects[i]->rotQuat, &foregroundObjects[i]->rot);
    }
}

void initSpinFGObjects(s32 numFgObjects) {
    s32 i;

    for (i = 0; i < numFgObjects; i++) {
        fgXss[i] = foregroundObjects[i]->pos.x;
        fgYss[i] = foregroundObjects[i]->pos.y;
        fgRt[i] = foregroundObjects[i]->rot.y;
    }
    fgSpin.bnc[3] = fgSpin.bnc[2] = fgSpin.bnc[1] = fgSpin.bnc[0] = 1;
    spinInFrame = 0;
    spinInCount[3] = spinInCount[2] = spinInCount[1] = spinInCount[0] = 300.0f;
    spinInSpd[3] = spinInSpd[2] = spinInSpd[1] = spinInSpd[0] = 4.0f;
}

void initSpinOutFGObjects(s32 numFgObjects) {
    s32 i;

    for (i = 0; i < numFgObjects; i++) {
        fgXss[i] = foregroundObjects[i]->pos.x;
        fgYss[i] = foregroundObjects[i]->pos.y;
        fgRt[i] = foregroundObjects[i]->rot.y;
        spinInSpd[i] = 3.0f;
    }
}

void spinOutFGObjects(s32 numFgObjects) {
    s32 i;
    Actor *obj;
    s16 alpha;

    controllerActive = 0;
    for (i = 0; i < numFgObjects; i++) {
        obj = foregroundObjects[i];
        if (fgSpin.minSpot < 2) {
            if ((111.0f - spinInCount[0]) * 255.0f / 110.0f > 255.0f) {
                alpha = 255;
            } else if ((110.0f - spinInCount[0]) * 255.0f / 110.0f < 0.0f) {
                alpha = 0;
            } else {
                alpha = (110.0f - spinInCount[0]) * 255.0f / 110.0f;
            }
        } else {
            alpha = 255;
        }
        obj->alpha = alpha;
        switch (fgDirections[i]) {
            case 0:
                foregroundObjects[i]->pos.y = fgYss[i] + spinInCount[i];
                foregroundObjects[i]->rot.x = fgRt[i] + spinInCount[i] / D_NUM;
                break;
            case 1:
                foregroundObjects[i]->pos.y = fgYss[i] - spinInCount[i];
                foregroundObjects[i]->rot.x = fgRt[i] + spinInCount[i] / D_NUM;
                break;
            case 2:
                foregroundObjects[i]->pos.x = fgXss[i] - spinInCount[i] * 1.6;
                foregroundObjects[i]->rot.x = fgRt[i] + spinInCount[i] / D_NUM;
                break;
            case 3:
                foregroundObjects[i]->pos.x = fgXss[i] + spinInCount[i] * 1.6;
                foregroundObjects[i]->rot.x = fgRt[i] + spinInCount[i] / D_NUM;
                break;
        }
        if (spinInCount[i] < 500.0f) {
            spinInCount[i] += spinInSpd[i];
            spinInSpd[i] += 0.8;
        }
    }
    for (i = 0; i < numForegroundObjects; i++) {
        GetQuaternionFromXYZRot(&foregroundObjects[i]->rotQuat, &foregroundObjects[i]->rot);
    }
}

void spinInFGObjects(s32 numFgObjects) {
    s32 i;
    s32 j;
    Actor *obj;
    s16 alpha;

    if (fgSpin.bnc[numFgObjects - 1] >= 3) {
        controllerActive = 1;
    } else {
        controllerActive = 0;
    }
    for (i = 0; i < numFgObjects; i++) {
        if (fgSpin.bnc[i] >= 4) {
            continue;
        }
        obj = foregroundObjects[i];
        if (fgSpin.minSpot < 2) {
            if ((111.0f - spinInCount[0]) * 255.0f / 110.0f > 255.0f) {
                alpha = 255;
            } else if ((110.0f - spinInCount[0]) * 255.0f / 110.0f < 0.0f) {
                alpha = 0;
            } else {
                alpha = (110.0f - spinInCount[0]) * 255.0f / 110.0f;
            }
        } else {
            alpha = 255;
        }
        obj->alpha = alpha;
        switch (fgDirections[i]) {
            case 0:
                foregroundObjects[i]->pos.y = fgYss[i] + spinInCount[i];
                foregroundObjects[i]->rot.x = fgRt[i] + spinInCount[i] / D_NUM;
                break;
            case 1:
                foregroundObjects[i]->pos.y = fgYss[i] - spinInCount[i];
                foregroundObjects[i]->rot.x = fgRt[i] + spinInCount[i] / D_NUM;
                break;
            case 2:
                foregroundObjects[i]->pos.x = fgXss[i] - spinInCount[i] * 1.2;
                foregroundObjects[i]->rot.x = fgRt[i] + spinInCount[i] / D_NUM;
                break;
            case 3:
                foregroundObjects[i]->pos.x = fgXss[i] + spinInCount[i] * 1.2;
                foregroundObjects[i]->rot.x = fgRt[i] + spinInCount[i] / D_NUM;
                break;
        }
        if (spinInFrame > spinInCmpFrame[i]) {
            spinInCount[i] -= spinInSpd[i];
            if (spinInCount[i] < 0.0f) {
                spinInCount[i] = 0.0f;
            }
            spinInSpd[i] += 0.8;
            if (spinInCount[i] < 1.0f) {
                spinInSpd[i] *= -bncAmt[i];
                fgSpin.bnc[i]++;
            }
            if (fgSpin.bnc[i] >= 3) {
                for (j = 0; j < numFgObjects; j++) {
                    foregroundObjects[i]->pos.x = fgXss[i];
                    foregroundObjects[i]->pos.y = fgYss[i];
                    foregroundObjects[i]->rot.x = fgRt[i];
                    spinInCount[i] = 0.0f;
                }
            }
        }
    }
    spinInFrame++;
    for (i = 0; i < numForegroundObjects; i++) {
        GetQuaternionFromXYZRot(&foregroundObjects[i]->rotQuat, &foregroundObjects[i]->rot);
    }
}

void FreeFGObjects(void) {
    s32 i;

    for (i = 0; i < numForegroundObjects; i++) {
        ReleaseActor(foregroundObjects[i], -1);
        JallocFree(&foregroundObjects[i]);
    }
    numForegroundObjects = 0;
}

void DrawScreen(s32 *cond) {
    fgSpin.maxSpot = screenList[currentScreen].numSpots;
    fgSpin.minSpot = 0;
    if (currentAnim >= 7) {
        currentAnim = 0;
    }
    while (!screenList[currentScreen].anims[currentAnim]) {
        addAnim(&currentAnim);
    }
    if (screenList[currentScreen].anims[currentAnim](cond, &screenList[currentScreen])) {
        addAnim(&currentAnim);
    }
    animFloat += 1.0f;
}

void ActContinueGame(void) {
    onlyDrawBackdrops = FALSE;
    KillTitle();
    StopDrawing("actcontgame");
    ContinueGame();
    StartNewLevel(gameInfo.level);
}

void CheatStartGame(s8 mode) {
    StopDrawing("cheatstart");
    KillTitle();
    StopDrawing("cheatstart");
    gameInfo.mode = mode;
    LoadLevel(1);
    currentScreen = 1;
    nextScreen = 13;
    currentAnim = 0;
    levelSelectCount = 0;
    introAnimTime = 31;
}

s32 addAnim(s32 *anim) {
    s32 t = *anim;

    if (++t >= 7) {
        currentScreen = nextScreen;
        nextScreen = 13;
        t = 0;
        if (frameCount > 10) {
            if (currentScreen != 10) {
                if (currentScreen != 6) {
                    animWaitTime = 20;
                }
            }
        }
    }
    *anim = t;
    return t;
}

void DoMenu(void) {
    s32 cond = 6;

    if (levelSelectCount == 0) {
        StopDrawing("domenu");
        animWaitTime = 0;
        newDesiredFrameRate = 2;
        InitLinkedLists();
        cameraInfo.focus = &zero;
        fog.on = 0;
        fog.r = 0;
        fog.g = 0;
        fog.b = 0;
        memset(cfb_16_a, 0, 0x25800);
        memset(cfb_16_b, 0, 0x25800);
        FadeScreenIn();
        SetupLight(&light, 255, 200, 255, 255, 200, 255, 4.0f, 1.0f, 0.0f);
        light.next = NULL;
        AddBackgroundLight(&light);
        SetupAmbientLight(&backgroundLightList.ambient, 100, 80, 80);
        SetFadeColour(0, 0, 0);
        gameInfo.fade = 255;
        gameInfo.fade2 = 300;
        if (currentScreen != 0) {
            if (!booting) {
                LoadTextureBank(TITLE_TEX_BANK);
                LoadObjectBank(70);
            }
            booting = 0;
            InitDebris();
            PrepareSong(TITLE2_TRACK);
            FreeBackdropList();
            pBackground = JallocAlloc(sizeof(Backdrop), 1, "Backdrop");
            pBackground->r = pBackground->g = pBackground->b = pBackground->alpha = 255;
            AddBackdrop(pBackground);
            FindTexture(&pBackground->tex, UpdateCRC("backgr.bmp"), 1);
            pBackground->scaleX = 0x400;
            pBackground->scaleY = 0x400;
            fog.r = 0;
            fog.g = 0;
            fog.b = 0;
        } else {
            fog.r = fog.g = fog.b = 0;
        }
        fog.on = 1;
        fog.min = 990;
        fog.max = 1000;
        farPlaneDist = 2500;
        gameInfo.fade = 0;
        gameInfo.fade2 = 0;
        SetFont(1);
        StartDrawing("domenu");
    }
    cond = 6;
    levelSelectCount++;
    if (controllerActive) {
        if (pad[ActiveController].pressed[BUTTON_A] || pad[ActiveController].pressed[BUTTON_START]) {
            cond = 4;
        }
        if (controllerActive && pad[ActiveController].pressed[BUTTON_B] && currentScreen != 1) {
            cond = 5;
        }
    }
    if (waitTimer1 == 0) {
        if (pad[ActiveController].stickY != 0) {
            if (abs(pad[ActiveController].stickY) > 40) {
                waitTimer1 = 6;
            }
            if (pad[ActiveController].stickY > 40) {
                cond = 1;
            } else if (pad[ActiveController].stickY < -40) {
                cond = 0;
            }
        }
    } else {
        waitTimer1--;
    }
    TitleScreenCameraSpin();
    DrawScreen(&cond);
    RunFadeN();
}

s32 DefWait(s32 *act, Screen *me) {
    aimBallX = me->spots[me->currentSpot].x + ballXOffset;
    aimBallY = me->spots[me->currentSpot].y;
    ballPicOL.x -= (ballPicOL.x - aimBallX) / 3.0f;
    ballPicOL.y -= (ballPicOL.y - aimBallY) / 3.0f;
    if (ballPicOL.scaleX < aimBallScale) {
        ballPicOL.scaleX += 120;
        ballPicOL.scaleY += 120;
    }
    if (ballPicOL.scaleX > aimBallScale) {
        ballPicOL.scaleX -= 120;
        ballPicOL.scaleY -= 120;
    }
    if (ballPicOL.scaleX < 350) {
        aimBallScale = 1024.0f;
    }
    if (ballPicOL.scaleX >= 910) {
        ballPicOL.scaleX = 0x400;
        ballPicOL.scaleY = 0x400;
    }
    switch (*act) {
        case 0:
            if (me->currentSpot < fgSpin.maxSpot - 1) {
                PlaySampleNot3D(212, 100, 128, 128);
            }
            me->currentSpot++;
            while (me->spots[me->currentSpot].type == 3) {
                me->currentSpot++;
            }
            if (me->spots[me->currentSpot].type == 4) {
                me->currentSpot--;
            }
            return FALSE;
        case 1:
            if (me->currentSpot > fgSpin.minSpot) {
                PlaySampleNot3D(212, 100, 128, 128);
            }
            if (me->currentSpot == 0) {
                me->currentSpot = 1;
            }
            me->currentSpot--;
            while (me->spots[me->currentSpot].type == 3) {
                me->currentSpot--;
            }
            return FALSE;
        case 2:
            if (me->spots[me->currentSpot + 1].type > 0) {
                if (me->spots[me->currentSpot + 1].type < 4) {
                    me->currentSpot++;
                    if (me->spots[me->currentSpot].type == 4) {
                        me->currentSpot = 0;
                    }
                }
            }
            return TRUE;
        case 3:
            if (me->currentSpot == 0) {
                if (me->spots[me->numSpots - 1].type > 0) {
                    if (me->spots[me->numSpots - 1].type < 4) {
                        me->currentSpot = me->numSpots;
                    }
                }
            } else if (me->spots[me->currentSpot - 1].type > 0) {
                if (me->spots[me->currentSpot - 1].type < 4) {
                    me->currentSpot--;
                }
            }
            return TRUE;
        case 4:
            if (me->spots[me->currentSpot].next != 13) {
                nextScreen = me->spots[me->currentSpot].next;
                introAnimTime = 31;
            }
            PlaySampleNot3D(216, 100, 128, 128);
            ballPicOL.scaleX = 700;
            ballPicOL.scaleY = 700;
            aimBallScale = 300.0f;
            return TRUE;
        case 5:
            nextScreen = 1;
            PlaySampleNot3D(217, 100, 128, 128);
            return TRUE;
        case 6:
            break;
    }
    return FALSE;
}

INCLUDE_ASM("asm/nonmatchings/frontend", pad_80110518);

inline void doCopyMode(void) {
    if (!copyMode) {
        copyFromSlot = currentSaveSlot;
    } else if (currentSaveSlot != copyFromSlot) {
        questionMode = 1;
    }
    copyMode = !copyMode;
    currentAnim--;
}

INCLUDE_RODATA("asm/nonmatchings/frontend", ident_80100220);

s32 SlotSelRun(s32 *act, Screen *me) {
    char temp[256];
    s32 x;
    s32 j;
    s32 i;
    u32 pcCDone;
    s32 k;
    f32 pulse;
    s32 oldCurspot;
    s32 oldslot;
    s32 world;

    SetFont(1);
    NEW_TEXT(260, spinInCount[0] / 4.0f + 227.0f, 255, 255, 255, 255, 1.0f, 1.0f, 1,
             startMenuStrings[10][gameInfo.language], 1);
    SetFont(0);
    slotsOL[0].x = 35.0f - spinInCount[2];
    slotsOL[1].x = 85.0f - spinInCount[1];
    slotsOL[2].x = 135.0f - spinInCount[0];
    slotsOL[3].x = spinInCount[0] + 185.0f;
    slotsOL[4].x = spinInCount[1] + 235.0f;
    slotsOL[5].x = spinInCount[2] + 285.0f;
    foregroundObjects[0]->scale.x = 1.5f;
    foregroundObjects[1]->scale.x = 1.5f;
    foregroundObjects[2]->scale.x = 1.5f;
    foregroundObjects[2]->scale.y = 1.0f;
    pcCDone = 0;
    fgSpin.minSpot = 1;
    foregroundObjects[2]->scale.z = 1.0f;
    if (selectingSlots) {
        for (world = 0; world < 6; world++) {
            if ((saveSlot[currentSaveSlot].levels >> (world * 5)) & 1) {
                pcCDone += 2;
            }
            if ((saveSlot[currentSaveSlot].levels >> (world * 5 + 1)) & 1) {
                pcCDone += 2;
            }
            if ((saveSlot[currentSaveSlot].levels >> (world * 5 + 2)) & 1) {
                pcCDone += 2;
            }
            if ((saveSlot[currentSaveSlot].levels >> (world * 5 + 3)) & 1) {
                pcCDone += 3;
            }
            if ((saveSlot[currentSaveSlot].levels >> (world * 5 + 4)) & 1) {
                pcCDone += 2;
            }
            pcCDone += (saveSlot[currentSaveSlot].allGaribs >> (world * 5)) & 1;
            pcCDone += (saveSlot[currentSaveSlot].allGaribs >> (world * 5 + 1)) & 1;
            pcCDone += (saveSlot[currentSaveSlot].allGaribs >> (world * 5 + 2)) & 1;
            pcCDone += (saveSlot[currentSaveSlot].allGaribs >> (world * 5 + 4)) & 1;
        }
        pcCDone = pcCDone * 100 / 90;
        SetFont(1);
        sprintf(temp, startMenuStrings[12][gameInfo.language], saveSlot[currentSaveSlot].score,
                saveSlot[currentSaveSlot].lives < 3 ? 3 : saveSlot[currentSaveSlot].lives);
        NEW_TEXT(160, 80, 255, 255, 0, 255, 0.8f, 0.8f, 1, temp, 1);
        sprintf(temp, startMenuStrings[11][gameInfo.language], pcCDone);
        NEW_TEXT(160, 90, 255, 255, 0, 255, 0.8f, 0.8f, 1, temp, 1);
        SetFont(0);
    }
    if (questionMode) {
        foregroundObjects[0]->scale.x = 0.9f;
        foregroundObjects[1]->scale.x = 0.9f;
        foregroundObjects[2]->scale.x = 0.0f;
        foregroundObjects[2]->scale.y = 0.0f;
        fgSpin.minSpot = 2;
        fgSpin.maxSpot = 3;
        foregroundObjects[2]->scale.z = 0.0f;
    }
    for (i = 0; i < 6; i++) {
        if (i == currentSaveSlot && (selectingSlots || copyMode)) {
            slotsOL[i].r = 255;
            slotsOL[i].g = 255;
            slotsOL[i].b = 255;
            slotsOL[i].alpha = !me->currentSpot ? sinf(animFloat * 0.65) * 90.0f + 128.0f + 38.0f : 255;
            slotsOL[i].scaleX = 1500;
            slotsOL[i].scaleY = 1500;
        } else if (copyMode && i == copyFromSlot) {
            slotsOL[i].r = 255;
            slotsOL[i].g = 100;
            slotsOL[i].b = 100;
            slotsOL[i].alpha = 255;
            slotsOL[i].scaleX = 1400;
            slotsOL[i].scaleY = 1400;
        } else {
            slotsOL[i].r = 155;
            slotsOL[i].g = 155;
            slotsOL[i].b = 155;
            slotsOL[i].alpha = 255;
            slotsOL[i].scaleX = 1024;
            slotsOL[i].scaleY = 1024;
        }
    }
    if (!me->currentSpot || copyMode) {
        if (inputOK && !questionMode && fadeDone
            && abs(pad[ActiveController].stickX) > 20 && !debounceSlotPadX) {
            if (pad[ActiveController].stickX > 0) {
                if (currentSaveSlot < 5) {
                    currentSaveSlot++;
                    PlaySampleNot3D(212, 100, 128, 128);
                }
            } else if (currentSaveSlot > 0) {
                PlaySampleNot3D(212, 100, 128, 128);
                currentSaveSlot--;
            }
            debounceSlotPadX = 8;
        }
        if (debounceSlotPadX) {
            debounceSlotPadX--;
        }
    } else {
        debounceSlotPadX = 8;
    }
    SetFont(1);
    for (i = 0; i < 6; i++) {
        switch (i) {
            case 0:
                x = -spinInCount[2];
                break;
            case 1:
                x = -spinInCount[1];
                break;
            case 2:
                x = -spinInCount[0];
                break;
            case 3:
                x = spinInCount[0];
                break;
            case 4:
                x = spinInCount[1];
                break;
            case 5:
                x = spinInCount[2];
                break;
        }
        SetFont(1);
        if (IsSaveSlotEmpty(i)) {
            NEW_TEXT(i * 50 + 35 + x, yPsn + 30, 255, 255, 0, 255, 0.8f, 0.8f, 1,
                     menuStrings[5][gameInfo.language], 1);
        } else {
            NEW_TEXT(i * 50 + 35 + x, yPsn + 30, 255, 255, 255, 255, 0.8f, 0.8f, 1, saveSlot[i].name, 1);
        }
        SetFont(0);
    }
    SetFont(0);
    TitleScreenCameraSpin2();
    pulse = sinf(animFloat * 0.65) * 90.0f + 128.0f + 38.0f;
    foregroundObjects[0]->rot.x = jogAmt[2];
    foregroundObjects[0]->rot.y = sinf(animFloat * 0.2) * 0.1 + 3.14;
    foregroundObjects[0]->rot.z = sinf(animFloat * 0.15) * 0.1;
    foregroundObjects[1]->rot.x = jogAmt[1];
    foregroundObjects[1]->rot.y = sinf(animFloat * 0.3) * 0.1 + 3.14;
    foregroundObjects[1]->rot.z = sinf(animFloat * 0.15) * 0.1;
    foregroundObjects[2]->rot.x = jogAmt[0];
    foregroundObjects[2]->rot.y = sinf(animFloat * 0.1) * 0.1 + 3.14;
    foregroundObjects[2]->rot.z = sinf(animFloat * 0.15) * 0.1;
    for (k = 0; k < numForegroundObjects; k++) {
        GetQuaternionFromXYZRot(&foregroundObjects[k]->rotQuat, &foregroundObjects[k]->rot);
    }
    if (!copyMode) {
        if (!me->currentSpot) {
            me->spots[0].x = currentSaveSlot * 50 + 35;
        }
        if (inputOK) {
            DefWait(act, me);
        }
        // SlotSelRun compares currentSpot as signed; the field is a u32 for DefWait's unsigned tests
        if ((s32)me->currentSpot <= 0 && !selectingSlots) {
            me->currentSpot = 1;
        }
        if ((s32)me->currentSpot > 0 && selectingSlots) {
            me->currentSpot = 0;
        }
    } else {
        ballPicOL.x -= (ballPicOL.x - (currentSaveSlot * 50 + 35)) / 3.0f;
        ballPicOL.y -= (ballPicOL.y - 60.0f) / 3.0f;
    }
    if (!questionMode) {
        for (j = 0; j < numForegroundObjects; j++) {
            foregroundObjects[j]->alpha = 255;
        }
        oldCurspot = me->currentSpot;
        if (selectingSlots) {
            me->currentSpot = selectedSlot;
        }
        NEW_TEXT(spinInCount[0] + 160.0f, yPsn + 95, 255, 255, 255, (me->currentSpot == 1 ? (copyMode ? pulse / 2.0f : pulse) : (copyMode ? 128 : 255)), me->currentSpot == 1 ? 1.0f : 0.8f, me->currentSpot == 1 ? 1.0f : 0.8f, 1,
                 startMenuStrings[0][gameInfo.language], 1);
        NEW_TEXT(160.0f - spinInCount[1], yPsn + 126, 255, 255, 255, (me->currentSpot == 2 ? (copyMode ? pulse / 2.0f : pulse) : (copyMode ? 128 : 255)), me->currentSpot == 2 ? 1.0f : 0.8f, me->currentSpot == 2 ? 1.0f : 0.8f, 1,
                 startMenuStrings[1][gameInfo.language], 1);
        NEW_TEXT(spinInCount[2] + 160.0f, yPsn + 159, 255, 255, 255, me->currentSpot == 3 ? pulse : 255, me->currentSpot == 3 ? 1.0f : 0.8f,
                 me->currentSpot == 3 ? 1.0f : 0.8f, 1,
                 startMenuStrings[copyMode + 2 + (copyMode ? currentSaveSlot == copyFromSlot : 0)][gameInfo.language],
                 1);
        me->currentSpot = oldCurspot;
    } else {
        foregroundObjects[2]->alpha = 0;
        foregroundObjects[2]->alpha = 0;
        NEW_TEXT(spinInCount[0] + 160.0f, yPsn + 124, 255, 255, 255, (me->currentSpot == 2 ? (copyMode ? pulse / 2.0f : pulse) : (copyMode ? 128 : 255)), me->currentSpot == 2 ? 1.0f : 0.8f, me->currentSpot == 2 ? 1.0f : 0.8f, 1,
                 startMenuStrings[5][gameInfo.language], 1);
        NEW_TEXT(160.0f - spinInCount[0], yPsn + 159, 255, 255, 255, (me->currentSpot == 3 ? (copyMode ? pulse / 2.0f : pulse) : (copyMode ? 128 : 255)), me->currentSpot == 3 ? 1.0f : 0.8f, me->currentSpot == 3 ? 1.0f : 0.8f, 1,
                 startMenuStrings[6][gameInfo.language], 1);
        SetFont(1);
        NEW_TEXT(spinInCount[0] + 160.0f, yPsn + 78, 255, 50, 50, 255, 1.0f, 1.0f, 1,
                 startMenuStrings[7][gameInfo.language], 1);
        if (gameInfo.language == 2) {
            NEW_TEXT(160.0f - spinInCount[0], yPsn + 93, 255, 50, 50, 255, 0.8f, 1.0f, 1,
                     startMenuStrings[8][gameInfo.language], 1);
        } else {
            NEW_TEXT(160.0f - spinInCount[0], yPsn + 93, 255, 50, 50, 255, 1.0f, 1.0f, 1,
                     startMenuStrings[8][gameInfo.language], 1);
        }
        SetFont(0);
        if ((s32)me->currentSpot < 2) {
            me->currentSpot = 2;
        }
        if ((s32)me->currentSpot >= 4) {
            me->currentSpot = 3;
        }
        if (*act == 4) {
            *act = 6;
            if (me->currentSpot == 2) {
                if (questionMode == 2) {
                    *act = 4;
                    inputOK = FALSE;
                    me->currentSpot = 1;
                    questionMode = 3;
                } else {
                    saveSlot[currentSaveSlot] = saveSlot[copyFromSlot];
                    SaveSlots();
                    SaveCRC();
                    questionMode = 0;
                    me->currentSpot = 3;
                }
            } else {
                if (questionMode == 2) {
                    me->currentSpot = questionMode;
                }
                questionMode = 0;
            }
        }
    }
    if (pad[ActiveController].pressed[BUTTON_B]) {
        if (copyMode) {
            currentSaveSlot = copyFromSlot;
        } else if (selectingSlots) {
            oldslot = selectedSlot;
            *act = 6;
            selectingSlots = 0;
            me->currentSpot = oldslot;
        }
    }
    if (selectingSlots) {
        fgSpin.maxSpot = 0;
    }
    if ((*act == 0 || *act == 1) && (s32)me->currentSpot > 0) {
        jogAmt[me->currentSpot - 1] = 6.28f;
    }
    jogAmt[0] *= 0.9;
    jogAmt[1] *= 0.9;
    jogAmt[2] *= 0.9;
    if (*act == 5) {
        if (!questionMode) {
            nextScreen = 1;
            selectingSlots = 0;
            FadeScreenOutN();
            copyMode = 0;
            questionMode = 0;
            initSpinOutFGObjects(3);
            aborting = 1;
            inputOK = FALSE;
            return TRUE;
        }
        selectingSlots = 0;
        questionMode = 0;
    }
    if (questionMode) {
        ballXOffset = 40.0f;
    } else {
        ballXOffset = 0.0f;
    }
    if (*act == 4) {
        if (copyMode) {
            PlaySampleNot3D(216, 100, 128, 128);
        }
        switch (me->currentSpot) {
            case 0:
                switch (selectedSlot) {
                    case 1:
                        if (saveSlot[currentSaveSlot].name[0] == 13) {
                            me->currentSpot = 2;
                            nextScreen = 5;
                            selectingSlots = 0;
                            FadeScreenOutN();
                            inputOK = FALSE;
                            initSpinOutFGObjects(3);
                            return TRUE;
                        }
                        inputOK = FALSE;
                        doFades = 1;
                        FadeScreenOutN();
                        selectingSlots = 0;
                        initSpinOutFGObjects(3);
                        return TRUE;
                    case 2:
                        if (saveSlot[currentSaveSlot].name[0] == 13) {
                            me->currentSpot = 2;
                            selectingSlots = 0;
                            FadeScreenOutN();
                            initSpinOutFGObjects(3);
                            inputOK = FALSE;
                            return TRUE;
                        }
                        questionMode = 2;
                        selectingSlots = 0;
                        me->currentSpot = 2;
                        break;
                    case 3:
                        if (IsSaveSlotEmpty(currentSaveSlot) && copyMode) {
                            saveSlot[currentSaveSlot] = saveSlot[copyFromSlot];
                            SaveSlots();
                            SaveCRC();
                            questionMode = 0;
                            selectingSlots = 0;
                            copyMode = 0;
                            me->currentSpot = 3;
                        } else {
                            doCopyMode();
                            if (!copyMode) {
                                selectingSlots = 0;
                                me->currentSpot = 3;
                            }
                        }
                        break;
                }
                break;
            case 1:
                if (questionMode == 3) {
                    FadeScreenOutN();
                    selectingSlots = 0;
                    me->currentSpot = 2;
                    initSpinOutFGObjects(3);
                    inputOK = FALSE;
                    return TRUE;
                }
                currentSaveSlot = 0;
                for (i = 0; i < 6; i++) {
                    if (saveSlot[i].name[0] != 13) {
                        currentSaveSlot = i;
                    }
                }
                me->currentSpot = 0;
                selectedSlot = 1;
                selectingSlots = 1;
                break;
            case 2:
                for (i = 6; i > 0; i--) {
                    if (saveSlot[i - 1].name[0] == 13) {
                        currentSaveSlot = i - 1;
                    }
                }
                me->currentSpot = 0;
                selectedSlot = 2;
                selectingSlots = 1;
                break;
            case 3:
                currentSaveSlot = 0;
                for (i = 0; i < 6; i++) {
                    if (saveSlot[i].name[0] != 13) {
                        currentSaveSlot = i;
                    }
                }
                me->currentSpot = 0;
                selectedSlot = 3;
                selectingSlots = 1;
                break;
            case 4:
                FadeScreenOutN();
                initSpinOutFGObjects(3);
                inputOK = FALSE;
                return TRUE;
        }
    }
    spinInFGObjects(3);
    return FALSE;
}

s32 SlotSelInit(s32 *act, Screen *me) {
    FadeScreenInN();
    currentSaveSlot = 0;
    me->currentSpot = 1;
    initFGObject("newwave.ndo", 0.0f, -125.0f, 500.0f, 3.14f, 0.0f, 0.0f, 3.3f, 1.2f, 1.0f, 255);
    InitActorStructures(foregroundObjects[0], 4);
    AnimateActor(foregroundObjects[0], 0, 1, 1, 0.8f);
    foregroundObjects[0]->scale.x = 1.5f;
    initFGObject("newwave.ndo", 0.0f, -70.0f, 500.0f, 3.14f, 0.0f, 3.14f, 3.3f, 1.2f, 1.0f, 255);
    InitActorStructures(foregroundObjects[1], 4);
    AnimateActor(foregroundObjects[1], 0, 1, 1, 1.1f);
    foregroundObjects[1]->scale.x = 1.5f;
    initFGObject("newwave.ndo", 0.0f, -15.0f, 500.0f, 3.14f, 0.0f, 0.0f, 3.6f, 1.2f, 1.0f, 255);
    InitActorStructures(foregroundObjects[2], 4);
    AnimateActor(foregroundObjects[2], 0, 1, 1, 1.0f);
    foregroundObjects[2]->scale.x = 1.5f;
    FindSetupAndAddOverlay(&slotsOL[0], "saveslot.bmp", 60, yPsn + 5, 0, 1);
    FindSetupAndAddOverlay(&slotsOL[1], "saveslot.bmp", 100, yPsn + 5, 0, 1);
    FindSetupAndAddOverlay(&slotsOL[2], "saveslot.bmp", 140, yPsn + 5, 0, 1);
    FindSetupAndAddOverlay(&slotsOL[3], "saveslot.bmp", 180, yPsn + 5, 0, 1);
    FindSetupAndAddOverlay(&slotsOL[4], "saveslot.bmp", 220, yPsn + 5, 0, 1);
    FindSetupAndAddOverlay(&slotsOL[5], "saveslot.bmp", 260, yPsn + 5, 0, 1);
    initSpinFGObjects(3);
    fgDirections[1] = 2;
    fgDirections[0] = 1;
    fgDirections[2] = 3;
    fgDirections[3] = 0;
    SlotSelRun(act, me);
    aborting = 0;
    return TRUE;
}

s32 SlotSelDone(s32 *act, Screen *me) {
    if (!fadeDone) {
        SlotSelRun(act, me);
        spinOutFGObjects(3);
        MoveGloveOL();
        return FALSE;
    }
    if (nextScreen != 8) {
        onlyDrawBackdrops = TRUE;
    }
    StopDrawing("slotseldone");
    DestroyOverlay(&slotsOL[0]);
    DestroyOverlay(&slotsOL[1]);
    DestroyOverlay(&slotsOL[2]);
    DestroyOverlay(&slotsOL[3]);
    DestroyOverlay(&slotsOL[4]);
    DestroyOverlay(&slotsOL[5]);
    questionMode = 0;
    FreeFGObjects();
    if (!aborting) {
        if (me->spots[me->currentSpot].func) {
            activateOK = TRUE;
            me->spots[me->currentSpot].func();
            return FALSE;
        }
    } else {
        aborting = 0;
    }
    inputOK = FALSE;
    return TRUE;
}

INCLUDE_RODATA("asm/nonmatchings/frontend", pad_801003FC);

s32 IsSaveSlotEmpty(s32 slot) {
    return saveSlot[slot].name[0] == 13;
}

INCLUDE_ASM("asm/nonmatchings/frontend", pad_8011282C);
