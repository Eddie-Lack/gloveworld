#include "common.h"
#include "macros.h"
#include "ball.h"
#include "texture.h"
#include "platform.h"
#include "controller.h"
#include "sound.h"
#include "bullet.h"
#include "text.h"
#include "vent.h"
#include "buzzer.h"
#include "banks.h"
#include "ident.h"
#include "game.h"
#include "overlay.h"
#include "jalloc.h"
#include "save.h"
#include "hand.h"
#include "debug.h"
#include "libc.h"
#include "maths.h"
#include "camera.h"
#include "debris.h"
#include "light.h"
#include "weather.h"
#include "blur.h"
#include "tip.h"
#include "cameo.h"
#include "spike.h"
#include "wind.h"
#include "puzzle.h"
#include "footprint.h"
#include "ambient.h"
#include "water.h"
#include "collect.h"
#include "controller.h"
#include "actor.h"
#include "cheat.h"
#include "boss.h"
#include "collision.h"
#include "decompress.h"
#include "eeprom.h"
#include "enemy.h"
#include "frontend.h"
#include "nmeprocess2.h"
#include "object.h"
#include "parse.h"
#include "personal.h"
#include "present.h"
#include "shadow.h"
#include "spell.h"
void FlashScreen(u8 r, u8 g, u8 b, s16 fadeVal, s16 fadeStep);

extern s32 cheatActivated;
extern s16 currentSaveSlot;
extern char currentName[];

extern HiscoreEntry hiscoreTable[6];

extern s32 fadeSpeed2;
extern s32 fadeSpeed3;
extern s32 endOfLevelCount;
extern s32 osTvType;
extern f32 levelTime;
extern u8 gamePaused;

Platform *TagToFirstPlatform(s32 tag);
void FindTexture(Texture **texPtr, u32 texID, s32 verbose);
u32 UpdateCRC(char *ptr);
void AnimateActor(Actor *actor, s32 animNum, s32 loop, s32 queue, f32 speed);
extern Texture *lavaTexture;
extern s16 farPlaneDist;
extern Texture *bubbleTexture;
extern Texture *lightningTexture;
void SetFont(u8 type);
struct Text *NEW_TEXT(s16 x, s16 y, u8 r, u8 g, u8 b, u8 a, f32 sx, f32 sy, u8 oneShot, char *str, u8 centre);
extern s32 errorCount;
extern u8 errorWait;
extern s32 gameOverCount;
extern f32 bombLife;
extern Quat zeroQuat;
extern char *gameString[];
extern char *overString[];
extern Text *gameOverText[2];
extern f32 gameOverVel;
extern f32 gameOverY[2];
extern u8 gameOverWait;
void PrepareSong(u8 song);
extern s16 sfxVol;
void MusHandleSetVolume(s32 handle, s32 volume);

#define SET_SAMPLE_VOLUME(cs)                                             \
    if ((cs).handle && (cs).outVolume > 0) {                              \
        MusHandleSetVolume((cs).handle, (cs).outVolume * sfxVol / 32768); \
    }

extern s16 gameWonCount;
extern f32 gameWonXPos;
extern s16 gameWonWait;
extern char *gameWonString[3][3];
extern u8 ActiveController;
extern Bullet *outroBullet;
typedef struct IntroPause {
    /* 0x00 */ s16 frame;
    /* 0x02 */ s16 time;
} IntroPause;
extern IntroPause introPause[];
extern char *introString[][3];
extern s16 crysOff;
extern Text *introText;
extern Bullet *introBullets[3];
extern s32 introBulletNum;
extern s32 outroposplus;
extern s32 introDimEndFrame;
extern f32 startDarkFrame;
extern u8 darkMul[3];
extern u8 originalAmbientLight[3];
extern u8 spellColour[][2][4];
extern Vec3 outroBulletPos;
extern s32 outrospeed;
extern f32 outrofireframe;
extern f32 spiral;
extern s16 outroFireTime;
extern f32 outroSpinDiv;
extern Backdrop *titleSlideOL;
extern Fog originalFog;
extern CollSphere *cameoExtraSphere;
extern u8 doneStaticMatrices;
extern u32 *ballBitTextureData;
extern u32 *ballBitTexturePalette;
extern char *nmeObjectName[];
extern u8 newDesiredFrameRate;
extern s16 musVol;
extern s16 cameraZoom;
extern u8 printController;
extern s32 autoPlayback;
extern Overlay controllerOL[];
extern Texture *shadTex;
extern Texture *rippleTex;
extern Texture *blastTex;
extern Texture *wakeTex;
extern Texture *tipTexture[2];
extern Texture *printTex;
extern Overlay tipOverlay;
extern Overlay tipOverlay2;
extern s32 carnivalBankToggle;
extern s32 dispFrameCount;
extern f32 DETAIL_RANGE;
extern u8 introSfxScript[];
extern u8 entroSfxScript[];
extern s32 languageCount;
extern Backdrop *pBackground;
extern Actor *foregroundObjects[];
extern s32 numForegroundObjects;
extern u8 languageWait;
extern u8 controllerPresent;
extern f32 flagFreq;
extern f32 flagVal;
extern Overlay languageFlagOL[];

typedef struct WorldInfo {
    /* 0x00 */ s8 objBank;
    /* 0x01 */ s8 texBank;
    /* 0x02 */ s8 firstLevel;
    /* 0x03 */ u8 secondLevel;
    /* 0x04 */ s8 lastLevel;
    /* 0x05 */ u8 extraLevels[0x3];
    /* 0x08 */ char backdrop[0x10];
} WorldInfo; /* size = 0x18 */

extern WorldInfo worldInfo[];
extern Vec3 zero;
extern u8 numtasks;
extern s32 frameCount2;
extern s32 frameCount;
extern u8 disableGraphics;
extern s16 fieldOfView;
extern s16 fieldOfViewAim;
extern s16 OLDfieldOfView;
extern s16 waitTimer;
extern s32 levelSelectCount;
extern s32 numLevels;
extern struct Text *textItem[14];
extern s32 selectedLevel;
extern char message[];
extern s32 oldSelectedLevel;
extern s32 levelSelectWait;
void SubText(struct Text *text);
void MusSetMasterVolume(s32 flags, s32 volume);
void LoadTextureBank(s32 num);

extern BestTime bestTimes[];
extern s32 oldNumGaribs;
void UpdateSaveSlot(s32 allGaribs);

void MusStop(s32 flags, s32 speed);
s32 MusAsk(s32 flags);

extern Actor fadeOverlay;
extern Actor starOverlay;
extern Texture whiteTex;
extern u8 fadeTextureData[];

extern s32 curSoundFXVol;
extern s32 curSoundMUSVol;
extern s32 ballLeftBehind;
extern s32 numGaribs;
extern Vec3 ballEnd;
extern Vec3 vectC;
extern Vec3 vectC2;
extern f32 titleSpin2CamRot;
extern Vec3 titleSpin2Vect;
extern f32 titleSpinCamRot;
extern Vec3 titleSpinVect;
extern Vec3 titleSpinVect2;
extern Vec3 titleSpin2Vect2;

void DestroyAndFreeActor(Actor *actor);
void LoadObjectBank(s32 num);
extern u8 jallocHeapStart[];
void StartNewLevel();

void DisableControl(s16 time);

typedef struct EolItem {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 vel;
    /* 0x0C */ f32 unk0C;
    /* 0x10 */ f32 target;
    /* 0x14 */ Overlay *ol;
    /* 0x18 */ s16 delay;
    /* 0x1A */ s16 active;
    /* 0x1C */ s32 dir;
} EolItem; /* size = 0x20 */

extern EolItem eolItems[7];

extern s32 numoftc;
extern s16 locateGaribsAllowed;
extern s32 SPIKE_RADIUS;
extern s32 numCyclingTextures;
extern s16 worldExitTimer;
extern s16 levelExitTimer;
extern s32 MAX_SFX_DIST;
extern u16 *recordKeys;
extern u16 *rkp;
extern s32 reverbOn;
extern Overlay health1OL;
extern Overlay health2OL;
extern Overlay bossHealthOL[5];
extern Overlay pickupTimerOL[4];
extern Overlay garibsGotOL;
extern Overlay numLivesOL;
extern f32 gravity;
extern Weather rain;
u16 *LoadRecordKeyFile(s32 num);
extern Actor plaster[4];
extern Vec3 cameraUpVectAim;
extern Vec3 cameraUpVect;
extern u8 draw_buffer;
extern Gfx overlay_dl[];
extern Gfx overlay_dl2[];
extern Gfx *overlay_dl_next;
extern Overlay controllerOL[11];
extern Overlay tipOverlay3;
extern Overlay timePicOL;
extern s32 numberOfEnemies;
extern Debris debris[120];
void DestroyOverlay(Overlay *overlay);
void ReleaseActor(Actor *actor, s32 flags);
extern u8 titleMenuSelected;
extern u8 titleMenuWait;
extern s32 titleMenuOldSelected;
extern struct Text *titleMenuText[14];
extern char *versionString;
extern char *menuStrings[][3];
extern char *difficultyStrings[][3];
extern char *startMenuStrings[][3];
extern char *shortWorldNames[][3];
extern BestTime bestTimesCopy[];
extern s32 currentScreen;
extern s32 currentAnim;
extern s32 timeTrialSelectedLevel;
extern s32 timeTrialOldSelectedLevel;
extern s32 timeTrialWait;
extern s32 timeTrialSaveRequired;
extern s32 timeTrialCancelMode;
extern struct Text *timeTrialTextItem[14][3];
extern s32 timeTrialLevelArray[48];
extern s32 timeTrialNumLevels;
extern s32 timeTrialFirstLevel;
extern s16 timePicXpos;
extern s16 timeBoxXpos;
extern s16 bestPicXpos;
extern s16 bestBoxXpos;
extern s16 garibPicXpos;
extern s16 garibBoxXpos;
extern u8 endOfLevelWait;
extern s16 quitTimer;
extern s32 conquered;
extern s32 oldBestTime;
extern Overlay bestPicOL;
extern Overlay garibPicOL;
extern Overlay scoreBoxOL;
extern Player player;
extern s32 newWorldCount;
extern s32 eowx1[];
extern s32 eowx2[];
extern s16 eowWait;
extern f32 wellDoneX;
extern f32 wellDoneV;
extern s16 wellDoneB;
extern f32 wellDoneBX;
extern f32 wellDoneBV;
extern s16 wellDoneBB;
extern char *timeUpString[][3];
extern char *worldCompleteString[][3];
extern f32 eowLettersX[100];
extern f32 eowLettersV[100];
extern s16 eowLettersB[100];
extern char eowLetters[50][2];
extern u8 eowLettersN;
extern Backdrop *eowBackdrop;
extern GameLight eowLight;
extern f32 D1;
extern f32 D2;
extern f32 D3;
void SetupLight(GameLight *light, u8 initR, u8 initG, u8 initB, u8 r, u8 g, u8 b, f32 x, f32 y, f32 speed);
void AddBackgroundLight(GameLight *light);
extern s16 newLevExitTimer;
extern s16 timer321;
extern s32 cameFromTitle;
extern s32 booting;
extern u8 disablePause;
extern s32 pcFile;
extern OSContPad controllerdata[];
s32 PCclose(s32 fd);
extern f32 exitspinspeed;
extern f32 exitDir;
extern s32 levelCamDistance;
extern f32 ballFlySpeed;
extern Vec3 ballStart;
extern s32 ballWithHand;
extern ContinuousSample caveSfx;
extern s32 numBalls;
extern Texture *ballTextureList[];
extern u32 *ballTextureData2;
extern u32 *ballTexturePalette2;
extern s32 pauseCount;
extern f32 D_801E7660;
extern u8 pauseFontAlpha;
extern u8 pauseSelection;
extern u8 pausePadWait;
extern u8 pauseQuestion;
extern s32 pauseHoop;
extern s32 pauseNumHoops;
extern s32 pauseWait;
extern Platform *pauseRestart;
extern u8 pauseItemActive[2];
extern u8 pauseMenuList[4];
extern u8 pauseNumItems;
extern u8 pauseBaseY;
extern ScreenDebris *tickGfx;
extern ScreenDebris *crossGfx;
extern s16 timeupTimer;
extern s16 bonusTimer;
extern u8 readingTip;
extern u8 pauseMode;
extern u8 cheatMsgCount;
extern u8 cheatTempString[];
extern s32 ballStatic;
extern char *pauseString[];
extern char *pauseMenuString[][3];

// FreeScreen, Update3DTitleGraphics and DoOutroEffects have the loop inline;
// other callers call FreeBackdropList.
static inline void FreeBackdropListInline(void) {
    Backdrop *b;
    Backdrop *next;

    for (b = backdropList.head.next; b != &backdropList.head; b = next) {
        next = b->next;
        JallocFree(&b);
    }
    InitBackdropList();
}

void func_8011C7F0(void) {
    FreeBackdropList();
}

void FreeBackdropList(void) {
    Backdrop *backdrop;
    Backdrop *backdrop2;

    for (backdrop = backdropList.head.next; backdrop != &backdropList.head; backdrop = backdrop2) {
        backdrop2 = backdrop->next;
        JallocFree(&backdrop);
    }
    InitBackdropList();
}

inline void InitPersonal(void) {
    helper.pos = zero;
    numtasks = 2;
    helper.pos.y = -700.0f;
    helper.scale.x = 50.0f;
    helper.scale.y = 50.0f;
    helper.scale.z = 50.0f;
    ShowJalloc();
}

void InitMorePersonal(s32 num) {
    s32 j;
    Platform *platform;

    ResetHand(1);
    ResetBall();
    ResetCamera();
    PRINTF("IMP:level:%d\n", num);
    CalcMaxActorRadius(&hand);
    CalcMaxActorRadius(&ball);
    farPlaneDist = 1000;
    ParseLandscapeFile(num);
    if (num == 8) {
        for (j = saveSlot[currentSaveSlot].hubLevel + 1; j < 8; j++) {
            platform = TagToFirstPlatform(j);
            platform->actor.alpha = 0;
            platform->actor.pos.y = -5000.0f;
            platform->flags &= ~0x20000;
        }
    }
    if (gameInfo.level != SPACEBOSS3) {
        AnimateActor(&hand, HANDANIM_STAND, 1, 0, 0.0f);
    }
    AnimateActor(&ball, 42, 1, 0, 0.0f);
    InitRestartGateShadows();
    FindTexture(&lavaTexture, UpdateCRC("restart.bmp"), 0);
    for (j = 0; j < 50; j++) {
        updateLava(lavaTexture->data, lavaTexture->width, lavaTexture->height);
    }
    FindTexture(&bubbleTexture, UpdateCRC("bubble.bmp"), 0);
    FindTexture(&lightningTexture, UpdateCRC("lightning.bmp"), 0);
}

void FreeScreen(void) {
    Overlay *overlay;
    Overlay *overlay2;
    s32 i;
    s32 j;

    MusStop(3, 1);
    while (MusAsk(3)) {
    }
    StopDrawing("freescn");
    ClearAllText();
    DestroyScreenDebris();
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
        JallocFree(&overlay);
    }
    InitOverlayList();
    FreeBackdropListInline();
    LoadFont();
    DestroyAndFreeActor(&camera);
    DestroyAndFreeActor(&cameraInfo.actor);
    JallocInit((u32)jallocHeapStart, 0x80400000 - (u32)jallocHeapStart);
    LoadObjectBank(0);
    ResetCamera();
}

inline void FreeLevel(void) {
    fog.r = fog.g = fog.b = 0;
    gameInfo.fade = 300;
    MusStop(1, 1);
    while (MusAsk(1) != 0) {}
    FreeLevel2(1);
}

void FreeLevel2(s32 full) {
    PuzzleAction *action;
    PuzzleCond *condition;
    Puzzle *puzzle;
    Footprint *footprint;
    AmbientSound *sound;
    Blur *blur;
    Tip *tip;
    Buzzer *buzzer;
    Cameo *cameo;
    Overlay *overlay;
    Spike *spike;
    Collectable *collectable;
    Water *water;
    Garib *garib;
    Garib *garib2;
    GaribGroup *garibGroup;
    Wind *wind;
    GameLight *light;
    Vent *vent;
    GameSprite *sprite;
    Actor *actor;
    Bullet *bullet;
    Platform *platform;
    Puzzle *puzzle2;
    PuzzleAction *action2;
    PuzzleCond *condition2;
    Footprint *footprint2;
    AmbientSound *sound2;
    Blur *blur2;
    Tip *tip2;
    Buzzer *buzzer2;
    Cameo *cameo2;
    Overlay *overlay2;
    Spike *spike2;
    Collectable *collectable2;
    Water *water2;
    GaribGroup *garibGroup2;
    Wind *wind2;
    GameLight *light2;
    Vent *vent2;
    Actor *actor2;
    Bullet *bullet2;
    Platform *platform2;
    GameSprite *sprite2;
    s32 k;
    s32 j;

    handControl.noControlTimer = 0;
    printController = 0;
    musVol = sfxVol = 0;
    MusSetMasterVolume(2, 0);
    MusSetMasterVolume(1, 0);
    for (k = 0; k < 2; k++) {
        if (handControl.samples[k].handle) {
            StopContinuousSample(&handControl.samples[k]);
        }
        if (ballControl.samples[k].handle) {
            StopContinuousSample(&ballControl.samples[k]);
        }
    }
    StopDrawing("freelevel2");
    gamePaused = 0;
    cameraUpVectAim.x = 0.0f;
    cameraUpVectAim.z = 0.0f;
    cameraUpVect.x = 0.0f;
    cameraUpVect.z = 0.0f;
    cameraUpVectAim.y = 1.0f;
    cameraUpVect.y = 1.0f;
    for (k = 0; k < 4; k++) {
        JallocFree(&plaster[k].state);
        ballControl.numPlasters = 0;
    }
    DestroyScreenDebris();
    fieldOfView = fieldOfViewAim = 45;
    JallocFree(&cameoExtraSphere);
    numCyclingTextures = 0;
    cameraInfo.overrideTimer = 0;
    if (handControl.frog) {
        DefrogHand();
    }
    if (draw_buffer) {
        overlay_dl_next = overlay_dl;
    } else {
        overlay_dl_next = overlay_dl2;
    }
    DeactivateAllCollectables();
    ClearAmbientSfx();
    handControl.noControlTimer = 0;
    gameInfo.cameoActive = 0;
    levelSelectCount = 0;
    frameCount = 0;
    PRINTF("\n\nLevel Cleared\n");
    memclear(camera.obj, 0x18C);
    ClearAllText();
    DestroyOverlay(&garibsGotOL);
    DestroyOverlay(&health1OL);
    DestroyOverlay(&health2OL);
    DestroyOverlay(&numLivesOL);
    for (k = 0; k < 5; k++) {
        DestroyOverlay(&bossHealthOL[k]);
    }
    for (k = 0; k < 4; k++) {
        DestroyOverlay(&pickupTimerOL[k]);
    }
    for (k = 0; k < 11; k++) {
        DestroyOverlay(&controllerOL[k]);
    }
    DestroyOverlay(&tipOverlay);
    DestroyOverlay(&tipOverlay2);
    DestroyOverlay(&tipOverlay3);
    DestroyOverlay(&timePicOL);
    FreeFGObjects();
    for (k = 0; k < 5; k++) {
        for (j = 0; j < objectBanks[k].count; j++) {
            FreeOFTC(objectBanks[k].entries[j].inst->obj);
        }
        JallocFree(&objectBanks[k].data);
        objectBanks[k].count = 0;
    }
    if (full) {
        for (k = 0; k < 5; k++) {
            JallocFree(&textureBanks[k].data);
        }
    }
    for (k = 0; k < NUM_SPRITE_ANIMS; k++) {
        JallocFree(&spriteFrameList[k].textures);
    }
    for (k = 0; k < numberOfEnemies; k++) {
        for (j = 0; j < 30; j++) {
            JallocFree(&enemies[k].brain.normalList[j]);
            JallocFree(&enemies[k].brain.conditionalList[j]);
            JallocFree(&enemies[k].brain.attackList[j]);
        }
        if (enemies[k].samples[0].handle) {
            StopContinuousSample(&enemies[k].samples[0]);
        }
        if (enemies[k].samples[1].handle) {
            StopContinuousSample(&enemies[k].samples[1]);
        }
        RemoveUniqueActor(&enemies[k].actor, enemies[k].actor.type);
        DestroyAndFreeActor(&enemies[k].actor);
    }
    for (k = 0; k < 120; k++) {
        ReleaseActor((Actor *)&debris[k], -1);
    }
    for (puzzle = puzzleList.head.prev; puzzle != &puzzleList.head; puzzle = puzzle2) {
        puzzle2 = puzzle->prev;
        for (action = puzzle->actions.head.prev; action != &puzzle->actions.head; action = action2) {
            action2 = action->prev;
            JallocFree(&action->cam);
            JallocFree(&action);
        }
        for (condition = puzzle->conds.head.prev; condition != &puzzle->conds.head; condition = condition2) {
            condition2 = condition->prev;
            JallocFree(&condition->cam);
            JallocFree(&condition);
        }
        JallocFree(&puzzle);
    }
    for (footprint = footprintList.head.next; footprint != &footprintList.head; footprint = footprint2) {
        footprint2 = footprint->next;
        JallocFree(&footprint);
    }
    for (sound = ambientSoundList.head.next; sound != &ambientSoundList.head; sound = sound2) {
        sound2 = sound->next;
        if (sound->sample.handle) {
            StopContinuousSample(&sound->sample);
        }
        JallocFree(&sound);
    }
    for (blur = blurList.head.next; blur != &blurList.head; blur = blur2) {
        blur2 = blur->next;
        JallocFree(&blur);
    }
    for (tip = tipList.head.next; tip != &tipList.head; tip = tip2) {
        tip2 = tip->next;
        JallocFree(&tip);
    }
    for (buzzer = buzzerList.head.next; buzzer != &buzzerList.head; buzzer = buzzer2) {
        if (buzzer->sample.handle) {
            StopContinuousSample(&buzzer->sample);
        }
        buzzer2 = buzzer->next;
        JallocFree(&buzzer);
    }
    for (cameo = cameoList.head.next; cameo != &cameoList.head; cameo = cameo2) {
        cameo2 = cameo->next;
        for (k = 0; k < cameo->count; k++) {
            JallocFree(&cameo->items[k]);
        }
        JallocFree(&cameo);
    }
    for (overlay = overlayList.head.next; overlay != &overlayList.head; overlay = overlay2) {
        overlay2 = overlay->next;
        DestroyOverlay(overlay);
        overlay->next = NULL;
        JallocFree(&overlay);
    }
    InitOverlayList();
    for (spike = spikeList.head.next; spike != &spikeList.head; spike = spike2) {
        spike2 = spike->next;
        JallocFree(&spike);
    }
    for (collectable = collectableList.head.next; collectable != &collectableList.head; collectable = collectable2) {
        collectable2 = collectable->next;
        JallocFree(&collectable);
    }
    for (water = waterList.head.next; water != &waterList.head; water = water2) {
        water2 = water->next;
        JallocFree(&water);
    }
    waterList.count = 0;
    for (garibGroup = garibGroupList.head.prev; garibGroup != &garibGroupList.head; garibGroup = garibGroup2) {
        garibGroup2 = garibGroup->prev;
        for (garib = garibGroup->garibs.head.prev; garib != &garibGroup->garibs.head; garib = garib2) {
            garib2 = garib->prev;
            JallocFree(&garib);
        }
        JallocFree(&garibGroup);
    }
    for (wind = windList.head.next; wind != &windList.head; wind = wind2) {
        wind2 = wind->next;
        JallocFree(&wind);
    }
    for (light = backgroundLightList.lights.head.next; light != &backgroundLightList.lights.head; light = light2) {
        light2 = light->next;
        JallocFree(&light);
    }
    if (full) {
        FreeBackdropListInline();
    }
    for (vent = ventList.head.next; vent != &ventList.head; vent = vent2) {
        if (vent->sound.handle) {
            StopContinuousSample(&vent->sound);
        }
        vent2 = vent->next;
        JallocFree(&vent);
    }
    DestroyAndFreeActor(&camera);
    DestroyAndFreeActor(&cameraInfo.actor);
    DestroyAndFreeActor(&hand);
    DestroyAndFreeActor(&ball);
    DestroyAndFreeActor(&handControl.edgeProbe);
    DestroyAndFreeActor(&handControl.frontProbe);
    for (actor = actorList.head.next; actor != &actorList.head; actor = actor2) {
        actor2 = actor->next;
        RemoveUniqueActor(actor, actor->type);
        DestroyAndFreeActor(actor);
    }
    for (bullet = bulletList.head.next; bullet != &bulletList.head; bullet = bullet2) {
        bullet2 = bullet->next;
        SubBullet(bullet);
    }
    for (platform = platformList.head.next; platform != &platformList.head; platform = platform2) {
        platform2 = platform->next;
        KillPlatform(platform, 0);
    }
    for (sprite = spriteList.head.next; sprite != &spriteList.head; sprite = sprite2) {
        sprite2 = sprite->next;
        JallocFree(&sprite);
    }
    InitSpriteList();
    ShowJalloc();
    LoadFont();
    gameInfo.exitFlags &= ~1;
    InitBackgroundLightList();
    InitFootprintList();
    numberOfEnemies = 0;
    if (full) {
        InitLinkedLists();
    } else {
        InitLinkedListsNoBackdrops();
    }
    JallocInit((u32)jallocHeapStart, 0x80400000 - (u32)jallocHeapStart);
    LoadObjectBank(0);
    ResetCamera();
}

void LoadLevel(s32 num) {
    s32 x;

    gamePaused = 0;
    StopDrawing("loadlevel");
    doneStaticMatrices = 0;
    gameWonCount = 0;
    player.garibs = 0;
    ballBitTexturePalette = NULL;
    ballBitTextureData = NULL;
    gameInfo.level = num;
    cheatActivated &= 0xFFEFF87F;
    nmeObjectName[HUBCHICKEN][10] = HubChickenType() + '1';
    newDesiredFrameRate = 3;
    sfxVol = 0;
    musVol = 0;
    cameraZoom = 1;
    MusSetMasterVolume(2, 0);
    MusSetMasterVolume(1, 0);
    PRINTF("Loading Level: %s\n-----------------------------------------------\n", levelInfo[num].name);
    numberOfEnemies = 0;
    ClearUniqueEnemyList();
    LoadTextureBank(GENERIC_TEX_BANK);
    printController = 0;
    if (autoPlayback == 2) {
        printController = 1;
        LoadTextureBank(18);
        FindAndSetupOverlay(&controllerOL[0], "padnobut1.bmp", 46, 182, 0, 1);
        FindAndSetupOverlay(&controllerOL[1], "padnobut2.bmp", 94, 182, 0, 1);
        FindAndSetupOverlay(&controllerOL[2], "a_button.bmp", 92, 171, 0, 1);
        FindAndSetupOverlay(&controllerOL[3], "b_button.bmp", 85, 164, 0, 1);
        FindAndSetupOverlay(&controllerOL[4], "z_button.bmp", 70, 200, 0, 1);
        FindAndSetupOverlay(&controllerOL[5], "cu_button.bmp", 99, 152, 0, 1);
        FindAndSetupOverlay(&controllerOL[6], "cd_button.bmp", 99, 164, 0, 1);
        FindAndSetupOverlay(&controllerOL[7], "cl_button.bmp", 93, 158, 0, 1);
        FindAndSetupOverlay(&controllerOL[8], "cr_button.bmp", 105, 158, 0, 1);
        FindAndSetupOverlay(&controllerOL[9], "stick.bmp", 160, 150, 0, 1);
        FindAndSetupOverlay(&controllerOL[10], "r_button.bmp", 103, 142, 0, 1);
    }
    FindTexture(&shadTex, UpdateCRC("newshadow.bmp"), 1);
    FindTexture(&rippleTex, UpdateCRC("ai_ripple.bmp"), 1);
    FindTexture(&blastTex, UpdateCRC("blastring.bmp"), 1);
    FindTexture(&wakeTex, UpdateCRC("ai_wake.bmp"), 1);
    FindTexture(&tipTexture[0], UpdateCRC("mrtip.bmp"), 1);
    FindTexture(&tipTexture[1], UpdateCRC("mrtip2.bmp"), 1);
    FindTexture(&printTex, UpdateCRC("gloveprnt.bmp"), 1);
    FindAndSetupOverlay(&tipOverlay, "mrtip.bmp", 0, 0, 0, 1);
    FindAndSetupOverlay(&tipOverlay2, "tippane.bmp", 0, 0, 0, 1);
    gameInfo.world = LevelToWorld(gameInfo.level);
    if (gameInfo.level < INTRO) {
        if (gameInfo.level == WAYROOM) {
            x = worldInfo[0].texBank;
        } else if (gameInfo.level == PRESENT) {
            x = PRESENT_TEX_BANK;
        } else {
            x = worldInfo[gameInfo.world].texBank;
        }
        LoadTextureBank(x);
        LoadObjectBank(worldInfo[gameInfo.world].objBank);
    } else {
        if (gameInfo.level == INTRO) {
            InitSFXScript(introSfxScript);
        }
        if (gameInfo.level == OUTRO) {
            InitSFXScript(entroSfxScript);
        }
        LoadTextureBank(CAMEO_TEX_BANK);
    }
    if (gameInfo.level == CARNIVALBONUS) {
        if (cheatActivated & 0x200000) {
            carnivalBankToggle = 1 - carnivalBankToggle;
            LoadTextureBank(carnivalBankToggle + 16);
        } else {
            LoadTextureBank(CKBONUS_TEX_BANK);
        }
    }
    if (num != FLYTHRU) {
        x = 1;
    } else {
        LoadTextureBank(FLYTHRU_TEX_BANK);
        x = 69;
    }
    LoadObjectBank(x);
    LoadObjectBank(levelInfo[gameInfo.level].objBank);
    InitObjects();
    InitMorePersonal(num);
    dispFrameCount = 0;
    if (gameInfo.level == WAYROOM) {
        PrepareSong(levelInfo[worldInfo[gameInfo.world].firstLevel].song);
    } else if (autoPlayback != 2 || gameInfo.level == SPACEBOSS_TWEEN) {
        if (gameInfo.mode == 15) {
            PrepareSong(TITLE_TRACK);
        } else {
            PrepareSong(levelInfo[num].song);
        }
    }
    switch (num) {
        case HUB1:
        case HUB2:
        case HUB3:
        case HUB4:
        case HUB5:
        case HUB6:
        case HUB7:
        case HUB8:
        case CAVE:
        case ATLANTISBOSS:
        case ATLANTISBONUS:
        case CARNIVALBOSS:
        case CARNIVALBONUS:
        case PIRATESBOSS:
        case PIRATESBONUS:
        case PREHISTORICBOSS:
        case PREHISTORICBONUS:
        case FEARBOSS:
        case FEARBONUS:
        case SPACEBOSS1:
        case SPACEBOSS3:
        case SPACEBONUS:
        case WAYROOM:
        case PRESENT:
        case FLYTHRU:
        case FLYTHRU2:
            DETAIL_RANGE = farPlaneDist * farPlaneDist;
            break;
        default:
            DETAIL_RANGE = 40000.0f;
            break;
    }
    pad[ActiveController].sincePress[BUTTON_Z] = 99999;
}

void TitleScreen(void) {
    s32 row = 1;
    s32 spacing = 25;
    s32 i;

    gameInfo.mode = 1;
    if (levelSelectCount == 0) {
        SetFont(0);
        gameInfo.exitFlags = 0;
        frameCount = frameCount2 = 0;
        titleMenuSelected = 0;
        titleMenuWait = 0;
        InitLinkedLists();
        cameraInfo.focus = &zero;
        LoadLevel(7);
        fog.on = 0;
        FadeScreenIn();
        for (i = 13; i >= 0; i--) {
            titleMenuText[i] = NULL;
        }
        titleMenuText[13] = NEW_TEXT(160, row * spacing, 255, 255, 255, 255, 0.8f, 0.8f, 0, versionString, 1);
        titleMenuOldSelected = 999;
    }
    if (frameCount > 25) {
        StartDrawing("titlescn");
    }
    TitleScreenCameraSpin();
    UpdatePause();
    if (titleMenuOldSelected != titleMenuSelected) {
        titleMenuOldSelected = titleMenuSelected;
        for (i = 0; i < 11; i++) {
            SubText(titleMenuText[i]);
        }
        titleMenuText[0] = NEW_TEXT(160, 140, 255, 255, 255, (titleMenuSelected == 0) ? 255 : 100, 1.0f, 1.0f, 0,
                                 menuStrings[0][gameInfo.language], 1);
        titleMenuText[1] = NEW_TEXT(160, 170, 255, 255, 255, (titleMenuSelected == 1) ? 255 : 150, 1.0f, 1.0f, 0,
                                 menuStrings[1][gameInfo.language], 1);
        titleMenuText[2] = NEW_TEXT(160, 200, 255, 255, 255, (titleMenuSelected == 2) ? 255 : 150, 1.0f, 1.0f, 0,
                                 menuStrings[2][gameInfo.language], 1);
    }
    levelSelectCount++;
    if (titleMenuWait == 0 && AnyDebouncePressed(ActiveController)) {
        FadeScreenOut();
        titleMenuWait = 200;
    }
    if (pad[ActiveController].stickY > 50) {
        if (titleMenuWait == 0 && titleMenuSelected != 0) {
            titleMenuSelected--;
            titleMenuWait = 10;
        }
    } else if (pad[ActiveController].stickY < -50) {
        if (titleMenuWait == 0 && titleMenuSelected < 2) {
            titleMenuSelected++;
            titleMenuWait = 10;
        }
    } else {
        titleMenuWait = 0;
    }
    if (titleMenuWait != 0) {
        titleMenuWait--;
    }
    if (ScreenFadedOut()) {
        titleMenuWait = 0;
        levelSelectCount = 0;
        for (i = 0; i < 14; i++) {
            SubText(titleMenuText[i]);
        }
        switch (titleMenuSelected) {
            case 0:
                FreeBackdropList();
                frameCount = frameCount2 = 0;
                FreeLevel();
                ContinueGame();
                StopDrawing("titlescn");
                StartNewLevel(gameInfo.level);
                break;
            case 1:
                FreeBackdropList();
                frameCount = frameCount2 = 0;
                FreeLevel();
                StartNewGame();
                StopDrawing("titlescn");
                StartNewLevel(INTRO);
                break;
            case 2:
                gameInfo.mode = 2;
                break;
        }
    }
}

void LevelSelectScreen(void) {
    s32 line = 1;
    s32 spacing = 17;
    s32 alpha;
    s32 x;
    u8 col;

    currentSaveSlot = 6;
    memclear(&saveSlot[6], sizeof(SaveSlot));
    saveSlot[currentSaveSlot].difficultyLevel = DIFFICULTY_MEDIUM;
    if (levelSelectCount == 0) {
        StopDrawing("levelselect");
        FadeScreenIn();
        for (x = 13; x >= 0; x--) {
            textItem[x] = NULL;
        }
        textItem[11] = NEW_TEXT(160, line * (2 * spacing), 255, 255, 255, 255, 1.0f, 1.0f, 0, "SELECT LEVEL", 1);
        StartDrawing("levelseect");
        oldSelectedLevel = 999;
        numLevels = 42;
    }
    line = 0;
    TitleScreenCameraSpin();
    levelSelectCount++;
    if (oldSelectedLevel != selectedLevel) {
        for (x = 0; x < 11; x++) {
            SubText(textItem[x]);
        }
    }
    SetFont(1);
    for (x = selectedLevel - 3; x <= selectedLevel + 3; x++) {
        line++;
        if (x >= 0 && x < numLevels) {
            col = (x & 1) ? 0 : 255;
            sprintf(message, "%s", &levelInfo[x]);
            if (x == selectedLevel) {
                textItem[line - 1] = NEW_TEXT(160, line * spacing + 55, 255, 255, col, levelSelectCount * 40, 1.4f, 1.4f, 1, message, 1);
            } else if (oldSelectedLevel != selectedLevel) {
                // alpha is a variable. The loop optimiser hoists 255 into a saved register.
                alpha = 255;
                textItem[line - 1] = NEW_TEXT(160, line * spacing + 55, 255, 255, col, alpha, 1.0f, 1.0f, 0, message, 1);
            }
        }
    }
    SetFont(0);
    oldSelectedLevel = selectedLevel;
    if (levelSelectWait) {
        levelSelectWait--;
    } else {
        if (pad[ActiveController].stickY > 50) {
            selectedLevel--;
            levelSelectWait = 1;
        }
        if (pad[ActiveController].stickY < -50) {
            levelSelectWait = 1;
            selectedLevel++;
        }
    }
    MusSetMasterVolume(2, 20000);
    if (pad[ActiveController].pressed[BUTTON_Z]) {
        PrepareSong(levelInfo[selectedLevel].song);
    }
    RunFadeN();
    // not MIN(): GCC folds that form into a MIN_EXPR, the ROM evaluates MAX twice
    selectedLevel = (numLevels - 1 > MAX(selectedLevel, 0)) ? MAX(selectedLevel, 0) : numLevels - 1;
    if (levelSelectWait == 0 && AnyDebouncePressed(ActiveController)) {
        FadeScreenOut();
        levelSelectWait = 200;
    }
    if (ScreenFadedOut()) {
        levelSelectWait = 0;
        cameraInfo.overrideTimer = 0;
        FreeBackdropList();
        frameCount = frameCount2 = 0;
        FreeLevel();
        StartNewGame();
        StopDrawing("levelselec");
        ballLeftBehind = -1;
        StartNewLevel(selectedLevel);
    }
}

void TimeTrialScreen(void) {
    s32 line = 1;
    s32 spacing = 18;
    s32 x;
    s32 j;
    u8 col;
    char message2[32];

    currentSaveSlot = 6;
    memclear(&saveSlot[6], sizeof(SaveSlot));
    saveSlot[currentSaveSlot].difficultyLevel = DIFFICULTY_HARD;
    UpdatePause();
    gameInfo.exitFlags = 0;
    if (levelSelectCount == 0) {
        StopDrawing("timetrial");
        timeTrialSaveRequired = 0;
        timeTrialCancelMode = 0;
        FadeScreenIn();
        for (x = 0; x < 14; x++) {
            for (j = 2; j >= 0; j--) {
                timeTrialTextItem[x][j] = NULL;
            }
        }
        timeTrialTextItem[11][0] = NEW_TEXT(160, line * (2 * spacing), 255, 255, 255, 255, 1.0f, 1.0f, 0,
                                     difficultyStrings[2][gameInfo.language], 1);
        timeTrialWait = 0;
        timeTrialOldSelectedLevel = 999;
        timeTrialNumLevels = 0;
        for (j = 0; j < 48; j++) {
            if (IsTimeTrialLevel(j) && IsLevelAvailable(j)) {
                if (timeTrialNumLevels == 0) {
                    timeTrialFirstLevel = j;
                }
                timeTrialLevelArray[timeTrialNumLevels++] = j;
            }
        }
        SetFont(1);
        timeTrialTextItem[12][0] = NEW_TEXT(260 - GetStringPixelLength(startMenuStrings[10][gameInfo.language], 1.0f, 1.0f) / 2, 202,
                                     255, 255, 255, 255, 1.0f, 1.0f, 0, startMenuStrings[10][gameInfo.language], 0);
        if (timeTrialNumLevels != 0) {
            timeTrialTextItem[13][0] = NEW_TEXT(40, 202, 255, 255, 255, 255, 1.0f, 1.0f, 0,
                                         startMenuStrings[13][gameInfo.language], 0);
        }
        SetFont(0);
        StartDrawing("timetrial");
    }
    TitleScreenCameraSpin();
    levelSelectCount++;
    if (timeTrialOldSelectedLevel != timeTrialSelectedLevel) {
        for (x = 0; x < 11; x++) {
            for (j = 0; j < 3; j++) {
                SubText(timeTrialTextItem[x][j]);
            }
        }
    }
    SetFont(1);
    line = 0;
    if (timeTrialNumLevels - 7 >= timeTrialSelectedLevel - 3) {
        x = timeTrialSelectedLevel - 3;
    } else {
        x = timeTrialNumLevels - 7;
    }
    for (; x < timeTrialNumLevels && line < 7; x++) {
        if (x >= 0 && x < 48) {
            line++;
            col = (line & 1) ? 0 : 255;
            sprintf(message, "%s %d", shortWorldNames[(timeTrialLevelArray[x] - 10) / 5][gameInfo.language],
                    (timeTrialLevelArray[x] - 10) % 5 + 1);
            sprintf(message2, "%d:%02d", bestTimes[levelInfo[timeTrialLevelArray[x]].bit].time / 60,
                    bestTimes[levelInfo[timeTrialLevelArray[x]].bit].time % 60);
            if (x == timeTrialSelectedLevel) {
                if (timeTrialCancelMode != 0) {
                    timeTrialTextItem[line - 1][0] = NEW_TEXT(40, line * spacing + 42, 255, 255, 255, levelSelectCount * 40, 1.0f, 1.3f, 1,
                                                      startMenuStrings[13][gameInfo.language], 0);
                    timeTrialTextItem[line - 1][1] = NEW_TEXT(170, line * spacing + 42, 255, 255, 255, levelSelectCount * 40, 1.0f, 1.3f, 1,
                                                      startMenuStrings[10][gameInfo.language], 0);
                } else {
                    timeTrialTextItem[line - 1][0] = NEW_TEXT(40, line * spacing + 42, 255, 255, col, levelSelectCount * 40, 1.0f, 1.3f, 1, message, 0);
                    timeTrialTextItem[line - 1][1] = NEW_TEXT(170, line * spacing + 42, 255, 255, col, levelSelectCount * 40, 1.0f, 1.3f, 1, message2, 0);
                    timeTrialTextItem[line - 1][2] = NEW_TEXT(250, line * spacing + 42, 255, 255, col, levelSelectCount * 40, 1.0f, 1.3f, 1,
                                                      bestTimes[levelInfo[timeTrialLevelArray[x]].bit].name, 0);
                }
            } else if (timeTrialOldSelectedLevel != timeTrialSelectedLevel) {
                timeTrialTextItem[line - 1][0] = NEW_TEXT(40, line * spacing + 45, 255, 255, col, 255, 1.0f, 1.0f, 0, message, 0);
                timeTrialTextItem[line - 1][1] = NEW_TEXT(170, line * spacing + 45, 255, 255, col, 255, 1.0f, 1.0f, 0, message2, 0);
                timeTrialTextItem[line - 1][2] = NEW_TEXT(250, line * spacing + 45, 255, 255, col, 255, 1.0f, 1.0f, 0,
                                                  bestTimes[levelInfo[timeTrialLevelArray[x]].bit].name, 0);
            }
        }
    }
    SetFont(0);
    if (timeTrialNumLevels == 0) {
        switch (gameInfo.language) {
            case 0:
                NEW_TEXT(160, 100, 255, 255, 255, SineWave(5.0f, levelSelectCount, 0) * 95.0f + 160.0f, 1.0f, 1.0f, 1,
                         menuStrings[11][gameInfo.language], 1);
                break;
            case 1:
                NEW_TEXT(160, 100, 255, 255, 255, SineWave(5.0f, levelSelectCount, 0) * 95.0f + 160.0f, 0.8f, 0.8f, 1,
                         menuStrings[11][gameInfo.language], 1);
                break;
            case 2:
                NEW_TEXT(160, 100, 255, 255, 255, SineWave(5.0f, levelSelectCount, 0) * 95.0f + 160.0f, 0.8f, 0.8f, 1,
                         menuStrings[11][gameInfo.language], 1);
                break;
        }
    }
    timeTrialOldSelectedLevel = timeTrialSelectedLevel;
    if (timeTrialWait != 0) {
        timeTrialWait--;
    } else if (timeTrialCancelMode == 0) {
        if (pad[ActiveController].stickY > 50) {
            if (timeTrialSelectedLevel > 0) {
                PlaySampleNot3D(212, 100, 128, 128);
                timeTrialSelectedLevel--;
            }
            timeTrialWait = 3;
        }
        if (pad[ActiveController].stickY < -50) {
            if (timeTrialSelectedLevel < timeTrialNumLevels - 1) {
                PlaySampleNot3D(212, 100, 128, 128);
                timeTrialSelectedLevel++;
            }
            timeTrialWait = 3;
        }
    }
    RunFadeN();
    timeTrialSelectedLevel = (timeTrialNumLevels - 1 > MAX(timeTrialSelectedLevel, 0)) ? MAX(timeTrialSelectedLevel, 0) : timeTrialNumLevels - 1;
    if (timeTrialWait == 0) {
        if (pad[ActiveController].pressed[BUTTON_Z] && timeTrialNumLevels != 0) {
            if (timeTrialCancelMode == 0) {
                PlaySampleNot3D(216, 100, 128, 128);
                timeTrialCancelMode = 1;
            } else {
                PlaySampleNot3D(216, 100, 128, 128);
                bestTimes[levelInfo[timeTrialLevelArray[timeTrialSelectedLevel]].bit] = bestTimesCopy[levelInfo[timeTrialLevelArray[timeTrialSelectedLevel]].bit];
                timeTrialSaveRequired = 1;
                timeTrialCancelMode = 0;
            }
        }
        if (pad[ActiveController].pressed[BUTTON_A] && timeTrialNumLevels != 0) {
            PlaySampleNot3D(216, 100, 128, 128);
            gamePaused = 1;
            FadeScreenOut();
            timeTrialWait = 200;
        } else if (pad[ActiveController].pressed[BUTTON_B] || (pad[ActiveController].pressed[BUTTON_A] && timeTrialNumLevels == 0)) {
            PlaySampleNot3D(217, 100, 128, 128);
            if (timeTrialCancelMode != 0) {
                timeTrialCancelMode = 0;
            } else {
                gamePaused = 1;
                FadeScreenOut();
                timeTrialWait = -200;
            }
        }
    }
    if (ScreenFadedOut()) {
        StopDrawing("timetrial");
        cameraInfo.overrideTimer = 0;
        FreeBackdropList();
        levelSelectCount = 0;
        frameCount = frameCount2 = 0;
        FreeLevel();
        if (timeTrialSaveRequired != 0) {
            SaveBestTimes();
            SaveCRC();
        }
        if (timeTrialWait > 0) {
            StartNewGame();
            StopDrawing("timetrial");
            gameInfo.mode = 4;
            ballLeftBehind = -1;
            StartNewLevel(timeTrialLevelArray[timeTrialSelectedLevel]);
        } else {
            gameInfo.mode = 1;
            currentScreen = 1;
            currentAnim = 0;
        }
    }
}

void StartNewGame(void) {
    u8 *p;
    s32 i;

    ballLeftBehind = -1;
    numGaribs = 0;
    ZERO_VECTOR(&ballEnd);
    saveSlot[currentSaveSlot].curSoundFXVol = curSoundFXVol;
    saveSlot[currentSaveSlot].curSoundMUSVol = curSoundMUSVol;
    for (i = 6, p = &gameInfo.worldComplete[6]; i >= 0; i--) {
        *p-- = 0;
    }
    saveSlot[currentSaveSlot].levels = 0;
    saveSlot[currentSaveSlot].hubTarget = 0;
    saveSlot[currentSaveSlot].hubLevel = 0;
    gameInfo.prevWorld = 0;
    gameInfo.world = HUB;
    gameInfo.exitFlags = 0;
    handControl.idleTime = 0;
    memclear(&player.score, 100);
    saveSlot[currentSaveSlot].lives = player.lives = difficultyStats[saveSlot[currentSaveSlot].difficultyLevel].lives;
    player.health = 3;
    player.spell = 15;
    cameraInfo.focus = &hand.pos;
    cameraInfo.targetDistance = 180.0f;
}

void ContinueGame(void) {
    s32 i;

    numGaribs = 0;
    curSoundFXVol = saveSlot[currentSaveSlot].curSoundFXVol;
    curSoundMUSVol = saveSlot[currentSaveSlot].curSoundMUSVol;
    for (i = 1; i < 7; i++) {
        if ((saveSlot[currentSaveSlot].levels >> (i * 5 - 2)) & 1) {
            gameInfo.worldComplete[i] = 1;
        } else {
            gameInfo.worldComplete[i] = 0;
        }
    }
    gameInfo.prevWorld = 0;
    gameInfo.world = HUB;
    gameInfo.level = saveSlot[currentSaveSlot].hubLevel;
    gameInfo.exitFlags = 0;
    ballLeftBehind = -1;
    handControl.idleTime = 0;
    memclear(&player.score, 100);
    player.lives = saveSlot[currentSaveSlot].lives < 3 ? 3 : saveSlot[currentSaveSlot].lives;
    player.health = 3;
    player.spell = 15;
    player.score = saveSlot[currentSaveSlot].score;
    cameraInfo.focus = &hand.pos;
    cameraInfo.targetDistance = 180.0f;
    currentName[0] = saveSlot[currentSaveSlot].name[0];
    currentName[1] = saveSlot[currentSaveSlot].name[1];
    currentName[2] = saveSlot[currentSaveSlot].name[2];
}

void InitPlayerForLevel(void) {
    handControl.idleTime = 0;
    player.spell = 15;
}

void InitLinkedLists(void) {
    InitOverlayList();
    InitBackgroundLightList();
    InitHandLightList();
    InitFootprintList();
    InitSpriteList();
    InitActorList();
    InitBulletList();
    InitWaterList();
    InitWindList();
    InitPlatformList();
    InitVentList();
    InitPlatformList();
    InitPuzzleList();
    InitSpikeList();
    InitGaribGroupList();
    InitCameoList();
    InitCollectableList();
    InitBlurList();
    InitBuzzerList();
    InitTipList();
    InitAmbientSoundList();
    InitBackdropList();
}

void InitLinkedListsNoBackdrops(void) {
    InitOverlayList();
    InitBackgroundLightList();
    InitHandLightList();
    InitFootprintList();
    InitSpriteList();
    InitActorList();
    InitBulletList();
    InitWaterList();
    InitWindList();
    InitPlatformList();
    InitVentList();
    InitPlatformList();
    InitPuzzleList();
    InitSpikeList();
    InitGaribGroupList();
    InitCameoList();
    InitCollectableList();
    InitBlurList();
    InitBuzzerList();
    InitTipList();
    InitAmbientSoundList();
}

void DestroyOverlay(Overlay *overlay) {
    SubOverlay(overlay);
    JallocFree(&overlay->sprite.bitmap);
    JallocFree(&overlay->bitmaps);
}

void StartNewLevel(s32 num) {
    s32 i;
    s32 slot;

    if (num == 39) {
        autoPlayback = 2;
    }
    numoftc = 0;
    slot = currentSaveSlot;
    if (saveSlot[slot].difficultyLevel == DIFFICULTY_EASY) {
        cheatActivated |= 1;
    }
    if (saveSlot[slot].difficultyLevel == DIFFICULTY_EASY || saveSlot[slot].hubLevel == 7) {
        locateGaribsAllowed = 1;
    } else {
        locateGaribsAllowed = 0;
    }
    SPIKE_RADIUS = (num == 34) ? 15 : 20;
    numCyclingTextures = 0;
    worldExitTimer = 0;
    levelExitTimer = 0;
    SeedRandom();
    cameraInfo.focus = &hand.pos;
    dispFrameCount = frameCount = frameCount2 = 0;
    gravity = 1.2f;
    StopDrawing("stnewlevel");
    ClearAllText();
    InitOverlayList();
    InitBackgroundLightList();
    InitHandLightList();
    InitFootprintList();
    InitSpriteList();
    InitActorList();
    InitBulletList();
    InitWaterList();
    InitWindList();
    InitPlatformList();
    InitVentList();
    InitPlatformList();
    InitPuzzleList();
    InitSpikeList();
    InitGaribGroupList();
    InitCameoList();
    InitCollectableList();
    InitBlurList();
    InitBuzzerList();
    InitTipList();
    InitAmbientSoundList();
    InitBackdropList();
    MAX_SFX_DIST = levelInfo[num].sfxDist;
    if (autoPlayback == 1) {
        recordKeys = JallocAlloc(0x2800, 0, "reckey");
        rkp = recordKeys + 2;
    } else if (autoPlayback == 2) {
        recordKeys = LoadRecordKeyFile(num);
    }
    InitPersonal();
    handControl.idleTime = 0;
    player.spell = 15;
    LoadLevel(num);
    if (num == 42) {
        for (i = 1; i < 4; i++) {
            if ((cheatActivated & 0x2000000) ||
                ((saveSlot[currentSaveSlot].levels >> ((gameInfo.world - 1) * 5 + i - 1)) & 1) ||
                ((saveSlot[currentSaveSlot].levels >> ((gameInfo.world - 1) * 5 + i)) & 1)) {
                TagToFirstPlatform(i)->actor.pos.y = -5000.0f;
            }
        }
        if (BonusOpen()) {
            TagToFirstPlatform(4)->actor.pos.y = -5000.0f;
        }
    }
    gameInfo.fade = 300;
    gameInfo.exitFlags &= ~3;
    InitScreenDebris();
    rain.duration = 0;
    FindAndSetupOverlay(&health1OL, "health01.bmp", 40, 210, 1, 0);
    FindAndSetupOverlay(&health2OL, "health02.bmp", 40, 210, 1, 0);
    reverbOn = gameInfo.level == PIRATESBONUS || gameInfo.level == CAVE || gameInfo.level == PREHISTORICBONUS || gameInfo.level == ACOURSE ||
               gameInfo.level == WAYROOM;
    if (gameInfo.level == SPACEBOSS3 || gameInfo.level == ATLANTISBOSS || gameInfo.level == CARNIVALBOSS || gameInfo.level == PIRATESBOSS ||
        gameInfo.level == PREHISTORICBOSS) {
        FindAndSetupOverlay(&bossHealthOL[0], "hitbar2.bmp", 120, 32, 1, 1);
        FindAndSetupOverlay(&bossHealthOL[1], "hitbar2.bmp", 120, 32, 1, 1);
        FindAndSetupOverlay(&bossHealthOL[2], "hitbar2.bmp", 120, 32, 1, 1);
        FindAndSetupOverlay(&bossHealthOL[3], "hitbar2.bmp", 120, 32, 1, 1);
        FindAndSetupOverlay(&bossHealthOL[4], "hitbar2.bmp", 120, 32, 1, 1);
        switch (gameInfo.level) {
            case ATLANTISBOSS:
                bossHealthOL[4].r = bossHealthOL[3].r = bossHealthOL[2].r = 86;
                bossHealthOL[4].g = bossHealthOL[3].g = bossHealthOL[2].g = 167;
                bossHealthOL[4].b = bossHealthOL[3].b = bossHealthOL[2].b = 221;
                bossHealthOL[1].r = 227;
                bossHealthOL[1].g = 43;
                bossHealthOL[1].b = 43;
                bossHealthOL[0].r = 255;
                bossHealthOL[0].g = 111;
                bossHealthOL[0].b = 0;
                break;
            case PIRATESBOSS:
                bossHealthOL[0].r = bossHealthOL[1].r = bossHealthOL[2].r = 204;
                bossHealthOL[0].g = bossHealthOL[1].g = bossHealthOL[2].g = 96;
                bossHealthOL[0].b = bossHealthOL[1].b = bossHealthOL[2].b = 0;
                break;
            case PREHISTORICBOSS:
                bossHealthOL[0].g = bossHealthOL[1].g = bossHealthOL[2].g = 96;
                bossHealthOL[0].r = bossHealthOL[1].r = bossHealthOL[2].r = 0;
                bossHealthOL[0].b = bossHealthOL[1].b = bossHealthOL[2].b = 200;
                break;
            case SPACEBOSS3:
                bossHealthOL[0].r = 152;
                bossHealthOL[0].b = 164;
                bossHealthOL[0].g = 0;
                bossHealthOL[1].r = 255;
                bossHealthOL[1].g = 139;
                bossHealthOL[1].b = 7;
                bossHealthOL[2].r = 255;
                bossHealthOL[2].g = 139;
                bossHealthOL[2].b = 7;
                break;
            case CARNIVALBOSS:
                bossHealthOL[0].r = 255;
                bossHealthOL[0].g = 0;
                bossHealthOL[0].b = 0;
                bossHealthOL[1].r = 0;
                bossHealthOL[1].g = 255;
                bossHealthOL[1].b = 0;
                bossHealthOL[2].r = 0;
                bossHealthOL[2].g = 0;
                bossHealthOL[2].b = 255;
                break;
        }
    } else {
        for (i = 0; i < 4; i++) {
            FindAndSetupOverlay(&pickupTimerOL[i], "testube.bmp", (i % 2) * 260 + 34, (i / 2) * 80 + 80, 1, 1);
            pickupTimerOL[i].alpha = 180;
        }
    }
    FindAndSetupOverlay(&numLivesOL, "numlives01.bmp", 270, 210, 1, 0);
    FindAndSetupOverlay(&garibsGotOL, "acard0001.bmp", 35, 27, 5, 0);
    garibsGotOL.frames = (SpriteFrames *)&spriteFrameList[NORMAL_GARIB_ANIM];
    for (i = 0; i < MAX_STATS; i++) {
        stats[i].scaleTarget = 0x400;
        stats[i].alphaTarget = 255;
    }
    stats[SCORE_STATS].text = NEW_TEXT(180, 18, 255, 255, 255, stats[SCORE_STATS].alpha, 1.0f, 1.0f, 0, "", 0);
    stats[LIVES_STATS].text = NEW_TEXT(220, 199, 255, 255, 255, stats[LIVES_STATS].alpha, 1.0f, 1.0f, 0, "", 0);
    stats[GARIB_STATS].text = NEW_TEXT(54, 18, 255, 255, 255, 255, 1.0f, 1.0f, 0, "", 0);
    player.levelFrames = 0;
    if (gameInfo.mode != 13) {
        if (gameInfo.level < INTRO) {
            gameInfo.mode = 4;
            fieldOfView = fieldOfViewAim = 45;
        } else {
            gameInfo.mode = 5;
            fieldOfView = fieldOfViewAim = 35;
        }
    }
    if (gameInfo.level == PREHISTORIC1A || gameInfo.level == PREHISTORICBOSS) {
        StartWeather(3, 64, 1, 220, 150, 80, 0, -8, 100, -1);
    }
    if (gameInfo.level == FEAR1A || gameInfo.level == FEAR3B) {
        StartWeather(2, 64, 5, 120, 80, 60, -8, -30, 7, -1);
        AddAmbientSfx(246, 70, 128);
    }
    saveSlot[currentSaveSlot].visited |= 1 << levelInfo[num].bit;
    FadeScreenIn();
}

void InitFadeOverlay(void) {
    s16 i;

    whiteTex.data = fadeTextureData;
    whiteTex.width = whiteTex.height = 8;
    whiteTex.maskS = 3;
    whiteTex.maskT = 3;
    whiteTex.siz = 2;
    whiteTex.fmt = 0;
    InitActor(&fadeOverlay, "fadeplane.ndo", 0, 0, 400.0f, 0);
    fadeOverlay.scale.x = 1.0f;
    fadeOverlay.scale.y = 1.0f;
    fadeOverlay.scale.z = 1.0f;
    fadeOverlay.rot.x = -1.57f;
    fadeOverlay.rot.y = 0.0f;
    fadeOverlay.rot.z = 1.57f;
    fadeOverlay.inst->obj->flags |= 2;
    fadeOverlay.alpha = 128;
    fadeOverlay.inst->obj->mesh->textures[0] = &whiteTex;
    GetQuaternionFromXYZRot(&fadeOverlay.rotQuat, &fadeOverlay.rot);
    InitActor(&starOverlay, "fadestar.ndo", 0, 0, 400.0f, 0);
    starOverlay.rot.x = 1.57f;
    starOverlay.rot.y = 0.0f;
    starOverlay.rot.z = 0.0f;
    starOverlay.pos.x = 0.0f;
    starOverlay.pos.y = 0.0f;
    starOverlay.scale.x = 0.2f;
    starOverlay.scale.y = 0.2f;
    starOverlay.scale.z = 0.2f;
    starOverlay.pos.z = 13.0f;
    starOverlay.inst->obj->flags |= 2;
    starOverlay.alpha = 128;
    for (i = 0; i < starOverlay.inst->obj->mesh->numFaces; i++) {
        starOverlay.inst->obj->mesh->textures[i] = &whiteTex;
    }
    GetQuaternionFromXYZRot(&starOverlay.rotQuat, &starOverlay.rot);
}

void EndOfLevelScreen(void) {
    Backdrop *backdrop;
    s32 i;
    s32 x;
    BestTime *best;
    u8 r;
    u8 g;
    u8 b;
    f32 temp;
    f32 y;
    EolItem *item;

    if (gameInfo.exitFlags & 1) {
        return;
    }
    UpdatePause();
    if (endOfLevelCount == 0) {
        oldBestTime = bestTimes[levelInfo[gameInfo.prevLevel].bit].time;
        conquered = CheckForLevelCompleted();
        newDesiredFrameRate = 2;
        quitTimer = -1;
        gameInfo.fade = 300;
        fog.on = fog.g = fog.b = fog.r = 0;
        farPlaneDist = 1000;
        helper.pos.x = 5000.0f;
        LoadTextureBank(gameInfo.world < 4 ? LEVEND1_TEX_BANK : LEVEND2_TEX_BANK);
        LoadObjectBank(70);
        FadeScreenIn();
        backdrop = JallocAlloc(sizeof(Backdrop), 1, "Backdrop");
        AddBackdrop(backdrop);
        FindTexture(&backdrop->tex, UpdateCRC(worldInfo[gameInfo.world].backdrop), 1);
        backdrop->r = backdrop->g = backdrop->b = backdrop->alpha = 255;
        backdrop->scaleX = 0x400;
        backdrop->scaleY = 0x400;
        InitDebris();
        FindSetupAndAddOverlay(&timePicOL, "eolcl01.bmp", timePicXpos - 210, 60, 2, 1);
        FindSetupAndAddOverlay(&bestPicOL, "trop0001.bmp", bestPicXpos + 210, 110, 2, 1);
        FindSetupAndAddOverlay(&garibPicOL, "eolgb01.bmp", garibPicXpos - 210, 160, 2, 1);
        garibPicOL.frames = (SpriteFrames *)&spriteFrameList[EOL_GARIB_ANIM];
        timePicOL.frames = (SpriteFrames *)&spriteFrameList[EOL_CLOCK_ANIM];
        bestPicOL.frames = (SpriteFrames *)&spriteFrameList[EOL_BEST_TIME_ANIM];
        for (i = 0; i < 7; i++) {
            eolItems[i].vel = 0;
            eolItems[i].unk0C = 0;
            eolItems[i].active = 1;
        }
        initFGObject("titleboxb.ndo", -110.0f, 150.0f, 500.0f, 0.0f, 1.5f, 0.0f, 1.4f, 1.4f, 1.4f, 255);
        initFGObject("titleboxbf.ndo", -110.0f, 150.0f, 500.0f, 0.0f, 1.5f, 0.0f, 1.4f, 1.4f, 1.4f, 128);
        initFGObject("titleboxb.ndo", 100.0f, 20.0f, 500.0f, 0.0f, -1.64f, 0.0f, 1.4f, 1.4f, 1.4f, 255);
        initFGObject("titleboxbf.ndo", 100.0f, 20.0f, 500.0f, 0.0f, -1.64f, 0.0f, 1.4f, 1.4f, 1.4f, 128);
        initFGObject("titleboxb.ndo", -120.0f, -60.0f, 500.0f, 0.0f, 1.5f, 0.0f, 1.4f, 1.4f, 1.4f, 255);
        initFGObject("titleboxbf.ndo", -120.0f, -60.0f, 500.0f, 0.0f, 1.5f, 0.0f, 1.4f, 1.4f, 1.4f, 128);
        initFGObject("titleboxb.ndo", 0.0f, -160.0f, 500.0f, 0.0f, 0.0f, 0.0f, 2.0f, 1.4f, 1.4f, 255);
        initFGObject("titleboxbf.ndo", 0.0f, -160.0f, 500.0f, 0.0f, 0.0f, 0.0f, 2.0f, 1.4f, 1.4f, 128);
        eolItems[0].x = timePicXpos - 200;
        eolItems[0].y = 52.0f;
        eolItems[0].target = timePicXpos;
        eolItems[0].ol = &timePicOL;
        eolItems[0].delay = 1;
        eolItems[0].dir = 0;
        eolItems[1].x = timeBoxXpos + 200;
        eolItems[1].y = 52.0f;
        eolItems[1].target = timeBoxXpos;
        eolItems[1].ol = NULL;
        eolItems[1].delay = 1;
        eolItems[1].dir = 1;
        eolItems[2].x = bestPicXpos + 200;
        eolItems[2].y = 110.0f;
        eolItems[2].target = bestPicXpos;
        eolItems[2].ol = &bestPicOL;
        eolItems[2].delay = 15;
        eolItems[2].dir = 1;
        eolItems[3].x = bestBoxXpos - 200;
        eolItems[3].y = 118.0f;
        eolItems[3].target = bestBoxXpos;
        eolItems[3].ol = NULL;
        eolItems[3].delay = 15;
        eolItems[3].dir = 0;
        eolItems[4].x = garibPicXpos - 200;
        eolItems[4].y = 165.0f;
        eolItems[4].target = garibPicXpos;
        eolItems[4].ol = &garibPicOL;
        eolItems[4].delay = 30;
        eolItems[4].dir = 0;
        eolItems[5].x = garibBoxXpos + 200;
        eolItems[5].y = 158.0f;
        eolItems[5].target = garibBoxXpos;
        eolItems[5].ol = NULL;
        eolItems[5].delay = 30;
        eolItems[5].dir = 1;
        eolItems[6].x = 160.0f;
        eolItems[6].y = 210.0f;
        eolItems[6].delay = 60;
        eolItems[6].ol = &scoreBoxOL;
        scoreBoxOL.scaleX = 0x2000;
        scoreBoxOL.scaleY = 0x2000;
        scoreBoxOL.alpha = 0;
        InitScreenDebris();
        PrepareSong(HASBRO_TRACK);
        gameInfo.exitFlags &= ~1;
        StartDrawing("eolscn");
    }
    if (quitTimer > 0) {
        quitTimer--;
    }
    HandleScreenDebris();
    for (i = 0; i < 6; i++) {
        item = &eolItems[i];
        if (item->active) {
            if (item->delay) {
                item->delay--;
            } else {
                if (item->dir == 0) {
                    if (item->vel < 20.0f) {
                        item->vel += 0.6;
                    }
                    item->x += item->vel;
                    if (item->x > item->target) {
                        if (item->vel < 5.0f) {
                            item->active = 0;
                            item->x = item->target;
                        }
                        item->x = item->target;
                        item->vel = -item->vel * 0.3;
                    }
                } else {
                    if (item->vel > -20.0f) {
                        item->vel -= 0.6;
                    }
                    item->x += item->vel;
                    if (item->x < item->target) {
                        if (item->vel > -5.0f) {
                            item->active = 0;
                            item->x = item->target;
                        }
                        item->x = item->target;
                        item->vel = -item->vel * 0.3;
                    }
                }
                if (item->ol != NULL) {
                    item->ol->x = item->x;
                    item->ol->y = item->y;
                } else {
                    temp = (item->x - item->target) / 100.0f;
                    // assigning through rot keeps the clamped value in a register for both stores
                    foregroundObjects[i]->rot.y = foregroundObjects[i - 1]->rot.y = temp =
                        (temp < -1.64 ? -1.64f : temp) < 1.5 ? (temp < -1.64 ? -1.64f : temp) : 1.5f;
                }
            }
        }
    }
    for (i = 0; i < 8; i++) {
        if (foregroundObjects[i]->rot.y <= -1.63 || foregroundObjects[i]->rot.y >= 1.49) {
            foregroundObjects[i]->alpha = 0;
        } else {
            foregroundObjects[i]->alpha = 255 - (i % 2) * 127;
        }
        foregroundObjects[i]->rot.z = SineWave(15.0f, endOfLevelCount, (i / 2) * 60) / 9.0f;
    }
    for (x = 0; x < numForegroundObjects; x++) {
        GetQuaternionFromXYZRot(&foregroundObjects[x]->rotQuat, &foregroundObjects[x]->rot);
    }
    foregroundObjects[6]->scale.x = foregroundObjects[7]->scale.x =
        (f32)(MIN(eolItems[6].ol->alpha * 2, 255) * 2) / 255.0;
    foregroundObjects[6]->scale.y = foregroundObjects[7]->scale.y =
        (f32)(MIN(eolItems[6].ol->alpha * 2, 255) * 1.4) / 255.0;
    foregroundObjects[6]->scale.z = foregroundObjects[7]->scale.z =
        (f32)(MIN(eolItems[6].ol->alpha * 2, 255) * 1.4) / 255.0;
    item = &eolItems[6];
    if (item->delay != 0) {
        if (--item->delay == 0) {
            scoreBoxOL.alpha = 5;
            goto grow;
        }
    } else {
    grow:
        scoreBoxOL.scaleX -= (scoreBoxOL.scaleX - 0x400) / 3;
        scoreBoxOL.scaleY -= (scoreBoxOL.scaleY - 0x400) / 3;
        if (scoreBoxOL.alpha < 255) {
            scoreBoxOL.alpha += 10;
        }
    }
    if (endOfLevelCount > 150) {
        if (player.garibs != 0) {
            if (endOfLevelCount & 1) {
                player.garibs--;
                PlaySampleNot3D(140, 100, 128, 128);
                AddToScore(10);
            }
        }
        if (player.garibs == 0 && quitTimer == -1) {
            quitTimer = 60;
        }
    }
    if (endOfLevelCount > 0) {
        if (gameInfo.exitFlags & 2) {
            sprintf(message, "%d:%02d", (s16)levelTime / 60, (s16)levelTime % 60);
        } else {
            sprintf(message, "---");
        }
        NEW_TEXT(eolItems[1].x, eolItems[1].y, 255, 255, 255, 255, 1.0f, 1.0f, 1, message, 1);
        best = &bestTimes[levelInfo[gameInfo.prevLevel].bit];
        sprintf(message, "%d:%02d", best->time / 60, best->time % 60);
        NEW_TEXT(eolItems[3].x, eolItems[3].y, 255, 255, 255, 255, 1.0f, 1.0f, 1, message, 1);
        if (conquered != 0) {
            b = 128;
            r = g = SineWave(4.0f, endOfLevelCount, 0) * 64.0f + 127.0f + 64.0f;
        } else {
            r = g = b = 255;
        }
        sprintf(message, "%4d/%d", player.garibs, oldNumGaribs);
        NEW_TEXT(eolItems[5].x, eolItems[5].y, r, g, b, 255, 1.0f, 1.0f, 1, message, 1);
        sprintf(message, "%6d", player.score);
        NEW_TEXT(eolItems[6].x, eolItems[6].y, 255, 255, 255, scoreBoxOL.alpha, 1.0f, 1.0f, 1, message, 1);
    }
    if (oldBestTime >= (s32)levelTime && (gameInfo.exitFlags & 2) && (endOfLevelCount & 1)) {
        y = (s32)(SineWave(15.0f, endOfLevelCount, 0) * 20.0f);
        AddStarBurst(eolItems[1].x + y, eolItems[1].y + Random(11) - 5.0f, Random(3) + 3, SDT_STAR2);
    }
    endOfLevelCount++;
    if ((endOfLevelWait == 0 && pad[ActiveController].pressed[BUTTON_A]) || quitTimer == 0) {
        quitTimer = -1;
        endOfLevelWait = 200;
        FadeScreenOut();
        gamePaused = 1;
    }
    if (ScreenFadedOut()) {
        endOfLevelWait = 0;
        newDesiredFrameRate = 3;
        SubOverlay(&garibPicOL);
        SubOverlay(&timePicOL);
        SubOverlay(&bestPicOL);
        endOfLevelCount = 0;
        FreeFGObjects();
        FreeScreen();
        AddToScore(player.garibs * 10);
        StartNewLevel(gameInfo.level);
    }
}

void EndOfSubLevelScreen(void) {
    StartNewLevel(gameInfo.level);
}

void EndOfWorldScreen(void) {
    s32 i;
    s32 j;
    s32 k;
    u32 letX;
    f32 temp;

    if (++newWorldCount == 1) {
        StopDrawing("eowscn");
        eowWait = 0;
        CheckForLevelCompleted();
        wellDoneBB = 0;
        wellDoneBX = 200.0f;
        wellDoneBV = -1.0f;
        for (i = 0; i < 100; i++) {
            eowLettersX[i] = i * 10 + 200;
            eowLettersV[i] = -1.0f;
            eowLettersB[i] = 0;
        }
        eowLettersN = strlen(timeUpString[gameInfo.prevWorld][gameInfo.language]);
        // i steps through the letter pairs. It, not k, holds the byte offset.
        for (k = 0, i = 0; k < eowLettersN; k++, i += 2) {
            ((char *)eowLetters)[i] = timeUpString[gameInfo.prevWorld][gameInfo.language][k];
            ((char *)eowLetters)[i + 1] = 0;
        }
        fog.g = fog.b = fog.r = 0;
        helper.pos.x = 5000.0f;
        if (gameInfo.prevWorld < 4) {
            LoadTextureBank(LEVEND1_TEX_BANK);
        } else {
            LoadTextureBank(LEVEND2_TEX_BANK);
        }
        LoadObjectBank(70);
        eowBackdrop = JallocAlloc(sizeof(Backdrop), 1, "Backdrop");
        AddBackdrop(eowBackdrop);
        FindTexture(&eowBackdrop->tex, UpdateCRC(worldInfo[gameInfo.prevWorld].backdrop), 1);
        eowBackdrop->r = eowBackdrop->g = eowBackdrop->b = eowBackdrop->alpha = 255;
        eowBackdrop->scaleX = 0x400;
        eowBackdrop->scaleY = 0x400;
        initFGObject("titleboxb.ndo", 1000.0f, -50.0f, 500.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.3f, 1.4f, 255);
        initFGObject("titleboxbf.ndo", 1000.0f, -50.0f, 500.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.3f, 1.4f, 128);
        initFGObject("newwave.ndo", 0.0f, 87.0f, 500.0f, 0.0f, 0.0f, 0.0f, 1.4f, 1.4f, 1.4f, 255);
        InitActorStructures(foregroundObjects[2], 4);
        AnimateActor(foregroundObjects[2], 0, 1, 1, 0.8f);
        foregroundObjects[2]->scale.x = 2.0f;
        farPlaneDist = 10000;
        foregroundObjects[2]->scale.y = 1.5f;
        SetupLight(&eowLight, 255, 130, 255, 255, 130, 255, 4.0f, 1.0f, 0.0f);
        eowLight.next = NULL;
        AddBackgroundLight(&eowLight);
        SetupAmbientLight(&backgroundLightList.ambient, 100, 80, 80);
        SetFadeColour(0, 0, 0);
        InitScreenDebris();
        PrepareSong(HASBRO_TRACK);
        newDesiredFrameRate = 2;
        gameInfo.exitFlags &= ~1;
        FadeScreenIn();
        StartDrawing("eowscn");
    }
    UpdatePause();
    if (newWorldCount >= 71 && (newWorldCount & 1)) {
        temp = (s32)(SineWave(5.0f, newWorldCount, 0) * 40.0f);
        // assigning inside the argument keeps the sum in temp's register
        AddStarBurst(temp += 160.0f, Random(11) + 65, Random(3) + 3, SDT_STAR2);
    }
    HandleScreenDebris();
    if (newWorldCount < 20) {
        wellDoneB = 0;
        wellDoneX = 200.0f;
        wellDoneV = -1.0f;
    }
    NEW_TEXT(eowx1[gameInfo.language] - wellDoneX, 70, 255, 255, 255, 255, 0.8f, 0.8f, 1,
             worldCompleteString[0][gameInfo.language], 1);
    NEW_TEXT(eowx2[gameInfo.language] + wellDoneX, 70, 255, 255, 255, 255, 0.8f, 0.8f, 1,
             worldCompleteString[3][gameInfo.language], 1);
    k = GetStringPixelLength(timeUpString[gameInfo.prevWorld][gameInfo.language], 1.0f, 1.0f);
    letX = 160.0 - (f32)k / 2.0;
    for (i = 0; i < eowLettersN; i++) {
        temp = eowLettersX[i];
        temp += 130.0f;
        temp += __sinf(newWorldCount / D1 + i / D2) * D3;
        NEW_TEXT(letX, temp, 255, 255, 0, 255, 1.0f, 1.0f, 1, eowLetters[i], 0);
        letX += GetStringPixelLength(eowLetters[i], 1.0f, 1.0f) - 3;
    }
    NEW_TEXT(160.0f - eowLettersX[19], 180, 255, 255, 255, 255, 1.0f, 1.0f, 1, worldCompleteString[1][gameInfo.language], 1);
    if (BonusOpen()) {
        NEW_TEXT(eowLettersX[15] + 160.0f, 210, 255, 255, 0, 255, 0.8f, 0.8f, 1, worldCompleteString[2][gameInfo.language], 1);
    } else {
        SetFont(1);
        NEW_TEXT(eowLettersX[15] + 160.0f, 210, 255, 255, 0, 255, 0.8f, 0.8f, 1, worldCompleteString[4][gameInfo.language], 1);
        SetFont(0);
    }
    foregroundObjects[2]->pos.y = wellDoneBX * 1.7 + 87.0;
    foregroundObjects[2]->rot.x = wellDoneBX * 6.28 / 200.0;
    for (i = 0; i < 100; i++) {
        eowLettersX[i] += eowLettersV[i];
        eowLettersV[i] -= 1.3;
        if (eowLettersX[i] <= 0.0f) {
            eowLettersX[i] = 0.0f;
            if (eowLettersB[i] < 2) {
                eowLettersB[i]++;
                eowLettersV[i] *= -0.5;
            } else {
                eowLettersV[i] *= 0.0f;
            }
        }
    }
    wellDoneX += wellDoneV;
    wellDoneV -= 1.3;
    wellDoneBX += wellDoneBV;
    wellDoneBV -= 1.1;
    if (wellDoneX <= 0.0f) {
        wellDoneX = 0.0f;
        if (wellDoneB < 2) {
            wellDoneB++;
            wellDoneV *= -0.5;
        } else {
            wellDoneV *= 0.0f;
        }
    }
    if (wellDoneBX <= 0.0f) {
        wellDoneBX = 0.0f;
        if (wellDoneBB < 2) {
            wellDoneBB++;
            wellDoneBV *= -0.5;
        } else {
            wellDoneBV *= 0.0f;
        }
    }
    for (j = 0; j < numForegroundObjects; j++) {
        GetQuaternionFromXYZRot(&foregroundObjects[j]->rotQuat, &foregroundObjects[j]->rot);
    }
    if (eowWait == 0) {
        if (newWorldCount > 500) {
            eowWait = 200;
            FadeScreenOut();
            gamePaused = 1;
        }
        if (eowWait == 0 && AnyDebouncePressed(ActiveController)) {
            eowWait = 200;
            FadeScreenOut();
            gamePaused = 1;
        }
    }
    if (ScreenFadedOut()) {
        newDesiredFrameRate = 3;
        FreeFGObjects();
        FreeScreen();
        StartNewLevel(gameInfo.level);
        newWorldCount = 0;
        eowWait = 0;
    }
}

void NewWorldScreen(void) {
    StartNewLevel(gameInfo.level);
}

void ErrorScreen(void) {
    SetFont(1);
    if (++errorCount == 1) {
        ClearAllText();
        FreeLevel();
        FadeScreenIn();
        StartDrawing("errorscn");
        NEW_TEXT(160, 60, 255, 255, 155, 255, 1.0f, 1.0f, 0, gameInfo.message, 1);
        NEW_TEXT(160, 100, 255, 255, 255, 255, 1.0f, 1.0f, 0, "PRESS A TO CONTINUE", 1);
    }
    if (!errorWait && pad[ActiveController].pressed[BUTTON_A]) {
        errorWait = 200;
        FadeScreenOut();
    }
    if (ScreenFadedOut()) {
        gameInfo.mode = 1;
        errorCount = 0;
        FreeLevel();
        errorWait = 0;
    }
    SetFont(0);
}

void UpdateFade(void) {
    f32 df;

    if (gameInfo.fade != gameInfo.fade2) {
        df = (f32)(gameInfo.fade2 - gameInfo.fade) / 6.0f;
        gameInfo.fade += df;
        if (gameInfo.fade2 == 0 && gameInfo.fade < 10) {
            gameInfo.fade = 0;
        }
    }
    if (gameInfo.starZ != gameInfo.starZTarget) {
        if (gameInfo.starZ > gameInfo.starZTarget) {
            gameInfo.starZ = (0 > gameInfo.starZ - fadeSpeed3) ? 0 : gameInfo.starZ - fadeSpeed3;
        } else {
            gameInfo.starZ =
                (gameInfo.starZTarget > gameInfo.starZ + fadeSpeed2) ? gameInfo.starZ + fadeSpeed2 : gameInfo.starZTarget;
        }
    }
    fadeOverlay.alpha = MIN(MAX(gameInfo.fade, 0), 255);
}

void GameWonScreen(void) {
    char buf[64];
    f32 scale;
    s32 l;
    s32 missing = 0;

    if (++gameWonCount == 2) {
        gameWonXPos = 320.0f;
        UpdateSaveSlot(0);
        UpdateHiscoreTable(player.score);
    }
    if ((gameWonCount >= 20 && !gameWonWait && pad[ActiveController].pressed[BUTTON_A]) || gameWonCount == 200) {
        gameWonWait = 200;
        FadeScreenOut();
    }
    HandleScreenDebris();
    if (gameWonCount > 20) {
        if (gameWonCount > 40) {
            gameWonXPos += MIN(MAX(-50.0f, -gameWonXPos / 4.0f), 50.0f);
        }
        if (gameWonXPos < 1.0f) {
            gameWonXPos = 1.0f;
        }
        if (gameWonCount == 21) {
            FlashScreen(255, 255, 0, 255, -30);
        }
        if (gameWonCount == 26) {
            for (l = 0; l < 10; l++) {
                AddStarBurst((l << 5) + 8 + Random(8), 30, 4, SDT_STAR2);
            }
        }
        NEW_TEXT(160, 30, 255, 255, 255, 255, 1.0f, 1.0f, 1, gameWonString[0][gameInfo.language], 1);
    }
    for (l = 0; l < 6; l++) {
        missing += !(saveSlot[currentSaveSlot].levels & (1 << (l * 5 + 4)));
    }
    if (missing) {
        sprintf(buf, "%s %d", gameWonString[1][gameInfo.language], missing);
        scale = 0.9f;
        if (gameInfo.language == 0) {
            scale = 1.0f;
        }
        NEW_TEXT(gameWonXPos + 160.0f, 85, 255, 255, 255, 255, scale, scale, 1, buf, 1);
        NEW_TEXT(160.0f - gameWonXPos, 120, 255, 255, 255, 255, scale, scale, 1, gameWonString[2][gameInfo.language], 1);
    }
    if (ScreenFadedOut()) {
        gameWonWait = 0;
        FreeLevel();
        FreeScreen();
        levelSelectCount = 0;
        SetupFlythru2Screen();
    }
}

void CheckForExits(void) {
    if (gameInfo.mode != 4 && gameInfo.mode != 5) {
        return;
    }
    if (!gamePaused) {
        if (newLevExitTimer) {
            if (--newLevExitTimer == 0) {
                ExitLevel(gameInfo.exitType, 1, 1);
            }
        } else if (levelExitTimer) {
            AddScaledVector2D(&ball.pos, &ball.pos, 0.8f, &gameInfo.exitPlatform->actor.pos, 0.2f);
            AddScaledVector2D(&hand.pos, &hand.pos, 0.8f, &gameInfo.exitPlatform->actor.pos, 0.2f);
            ActorHandleRotaim(&hand, 1);
            if (hand.anim->current == HANDANIM_LEVELDONE) {
                if (TriggerAtFrame(1163.0f, &hand)) {
                    hand.state->mode = HAND_NOTHING;
                    ball.vel.y = 9.0f;
                }
                if (TriggerAtFrame(1177.0f, &hand)) {
                    PrepareSong(gameInfo.world * 4 + 32 + Random(2));
                    PlaySample(165, &hand.pos, 100, 140);
                    CreateStarCircle(&ball.pos, 5, 19);
                    ball.alpha = 0;
                    ReleaseBall();
                    ball.pos.y = -8000.0f;
                    ball.flags &= ~1;
                }
                if (hand.anim->time > 1275.0f) {
                    SCALE_VECTOR(&hand.scale, 0.8);
                }
            }
            if (!gamePaused
                && (hand.anim->finishedAnim == 4 || --levelExitTimer == 0
                    || (controllerdata[ActiveController].button & 0x8000))) {
                ExitLevel(gameInfo.exitType, 1, 1);
            }
        } else if (worldExitTimer) {
            if (gameInfo.exitPlatform != NULL) {
                AddScaledVector2D(&ball.pos, &ball.pos, 0.8f, &gameInfo.exitPlatform->actor.pos, 0.2f);
                ball.pos.y = gameInfo.exitPlatform->actor.pos.y + 6.0f + ball.radius;
                hand.state->mode = HAND_JOINED;
            }
            worldExitTimer--;
            if (worldExitTimer == 40) {
                PlaySample(334, &hand.pos, 100, 155);
            }
            if (worldExitTimer == 20) {
                PlaySample(334, &hand.pos, 100, 180);
            }
            if (worldExitTimer == 0) {
                ExitLevel(WORLD_EXIT, 0, 1);
            }
        } else if (timer321 * 20 < frameCount) {
            player.levelFrames++;
        }
    }
    if (gameInfo.level == CAVE) {
        DoCaveStuff();
    } else if (gameInfo.level == OUTRO) {
        DoOutroEffects();
        ProcessSFXScript();
    } else if (gameInfo.level == INTRO) {
        DoIntroEffects();
        ProcessSFXScript();
    }
    if (ScreenFadedOut() && (gameInfo.exitFlags & 1)) {
        if (autoPlayback == 1) {
            PCclose(pcFile);
        }
        if (osTvType == 0) {
            levelTime = (f32)player.levelFrames / (50.0 / 3.0);
        } else {
            levelTime = (f32)player.levelFrames / 20.0f;
        }
        FreeLevel();
        switch (gameInfo.exitType) {
            case GAME_WON_EXIT:
                gameInfo.prevLevel = gameInfo.level;
                gameInfo.level = OUTRO;
                gameInfo.mode = 8;
                break;
            case ASSAULT_EXIT:
                gameInfo.prevLevel = gameInfo.level;
                gameInfo.prevWorld = gameInfo.world;
                gameInfo.level = ACOURSE;
                gameInfo.world = HUB;
                gameInfo.mode = 8;
                break;
            case LEVEL_1_EXIT:
            case LEVEL_2_EXIT:
            case LEVEL_3_EXIT:
            case LEVEL_BOSS_EXIT:
                ballLeftBehind = -1;
            case LEVEL_BONUS_EXIT:
                gameInfo.prevLevel = gameInfo.level;
                gameInfo.level = (&worldInfo[gameInfo.world].firstLevel)[gameInfo.exitType - 16];
                gameInfo.mode = 8;
                break;
            case TIMETRIAL_RESTART_EXIT:
                gameInfo.mode = 8;
                break;
            case QUIT_EXIT:
                levelSelectCount = 0;
                frameCount = 0;
                gameInfo.mode = 1;
                break;
            case CAMEO_EXIT:
                if (gameInfo.level == INTRO) {
                    gameInfo.mode = 8;
                    gameInfo.prevLevel = gameInfo.level;
                    gameInfo.level = HUB1;
                    gameInfo.prevWorld = gameInfo.world;
                    gameInfo.world = HUB;
                }
                break;
            case LEVEL_EXIT:
                ballLeftBehind = -1;
                gameInfo.mode = 7;
                gameInfo.prevLevel = gameInfo.level;
                gameInfo.level++;
                break;
            case SUB_LEVEL_EXIT:
                if (gameInfo.level == SPACEBOSS1) {
                    autoPlayback = 2;
                }
                ballLeftBehind = -1;
                gameInfo.mode = 8;
                gameInfo.prevLevel = gameInfo.level;
                gameInfo.level++;
                break;
            case WORLD_EXIT:
                if (!((saveSlot[currentSaveSlot].levels >> levelInfo[gameInfo.level].bit) & 1)) {
                    saveSlot[currentSaveSlot].hubTarget++;
                }
                gameInfo.worldComplete[gameInfo.world] = 1;
                ballLeftBehind = -1;
                gameInfo.prevLevel = gameInfo.level;
                gameInfo.prevWorld = gameInfo.world;
                if (BonusOpen()) {
                    gameInfo.level = WAYROOM;
                } else {
                    if (cheatActivated & 0x1000000) {
                    gameInfo.level = HUB8;
                } else {
                    gameInfo.level = saveSlot[currentSaveSlot].hubLevel;
                }
                    gameInfo.world = HUB;
                }
                gameInfo.mode = 9;
                break;
            case BONUS_EXIT:
                ballLeftBehind = -1;
                gameInfo.mode = 7;
                gameInfo.prevLevel = gameInfo.level;
                gameInfo.level = worldInfo[gameInfo.world].extraLevels[1];
                break;
            case BONUS_DONE_EXIT:
                gameInfo.prevLevel = gameInfo.level;
                ballLeftBehind = -1;
                gameInfo.mode = 7;
                gameInfo.level = WAYROOM;
                break;
            case CAVE_EXIT:
                gameInfo.mode = 8;
                gameInfo.prevLevel = gameInfo.level;
                gameInfo.level = CAVE;
                gameInfo.prevWorld = 0;
                if (saveSlot[currentSaveSlot].hubTarget == 0) {
                    saveSlot[currentSaveSlot].hubTarget = 1;
                }
                if (ball.link != &hand && !ballStatic) {
                    ballLeftBehind = gameInfo.prevLevel;
                    ballEnd = ball.pos;
                } else {
                    ballLeftBehind = -1;
                }
                break;
            case HUB_EXIT:
                if (gameInfo.level == WAYROOM) {
                    if (ball.link != &hand) {
                        if (!ballStatic) {
                            ballLeftBehind = gameInfo.level;
                            ballEnd = ball.pos;
                        }
                    } else if (!ballStatic) {
                        ballLeftBehind = -1;
                    }
                }
                gameInfo.mode = 8;
                gameInfo.prevLevel = gameInfo.level;
                numGaribs = 0;
                if (cheatActivated & 0x1000000) {
                    gameInfo.level = HUB8;
                } else {
                    gameInfo.level = saveSlot[currentSaveSlot].hubLevel;
                }
                gameInfo.prevWorld = gameInfo.world;
                gameInfo.world = HUB;
                CheckForLevelCompleted();
                if (cameFromTitle) {
                    gameInfo.mode = 1;
                }
                break;
            case ATLANTIS_EXIT:
                gameInfo.mode = 10;
                gameInfo.prevLevel = gameInfo.level;
                gameInfo.prevWorld = gameInfo.world;
                gameInfo.level = WAYROOM;
                gameInfo.world = ATLANTIS;
                if (ball.link != &hand && !ballStatic) {
                    ballEnd = ball.pos;
                    ballLeftBehind = gameInfo.prevLevel;
                } else if (ballLeftBehind != 42) {
                    ballLeftBehind = -1;
                }
                break;
            case CARNIVAL_EXIT:
                gameInfo.mode = 10;
                gameInfo.prevLevel = gameInfo.level;
                gameInfo.prevWorld = gameInfo.world;
                gameInfo.level = WAYROOM;
                gameInfo.world = CARNIVAL;
                if (ball.link != &hand && !ballStatic) {
                    ballEnd = ball.pos;
                    ballLeftBehind = gameInfo.prevLevel;
                } else if (ballLeftBehind != 42) {
                    ballLeftBehind = -1;
                }
                break;
            case PIRATES_EXIT:
                gameInfo.mode = 10;
                gameInfo.prevLevel = gameInfo.level;
                gameInfo.prevWorld = gameInfo.world;
                gameInfo.level = WAYROOM;
                gameInfo.world = PIRATES;
                if (ball.link != &hand && !ballStatic) {
                    ballEnd = ball.pos;
                    ballLeftBehind = gameInfo.prevLevel;
                } else if (ballLeftBehind != 42) {
                    ballLeftBehind = -1;
                }
                break;
            case PREHISTORIC_EXIT:
                gameInfo.mode = 10;
                gameInfo.prevLevel = gameInfo.level;
                gameInfo.prevWorld = gameInfo.world;
                gameInfo.level = WAYROOM;
                gameInfo.world = PREHISTORIC;
                if (ball.link != &hand && !ballStatic) {
                    ballEnd = ball.pos;
                    ballLeftBehind = gameInfo.prevLevel;
                } else if (ballLeftBehind != 42) {
                    ballLeftBehind = -1;
                }
                break;
            case FEAR_EXIT:
                gameInfo.mode = 10;
                gameInfo.prevLevel = gameInfo.level;
                gameInfo.prevWorld = gameInfo.world;
                gameInfo.level = WAYROOM;
                gameInfo.world = FEAR;
                if (ball.link != &hand && !ballStatic) {
                    ballEnd = ball.pos;
                    ballLeftBehind = gameInfo.prevLevel;
                } else if (ballLeftBehind != 42) {
                    ballLeftBehind = -1;
                }
                break;
            case SPACE_EXIT:
                gameInfo.mode = 10;
                gameInfo.prevLevel = gameInfo.level;
                gameInfo.prevWorld = gameInfo.world;
                gameInfo.level = WAYROOM;
                gameInfo.world = SPACE;
                if (ball.link != &hand && !ballStatic) {
                    ballEnd = ball.pos;
                    ballLeftBehind = gameInfo.prevLevel;
                } else if (ballLeftBehind != 42) {
                    ballLeftBehind = -1;
                }
                break;
        }
        if (gameInfo.level == FLYTHRU || gameInfo.level == FLYTHRU2) {
            gameInfo.mode = 1;
            currentScreen = 1;
            booting = 0;
            disablePause = 0;
        }
        if (gameInfo.level == PRESENT) {
            gameInfo.mode = 14;
            frameCount = 0;
        }
        if (saveSlot[currentSaveSlot].difficultyLevel == DIFFICULTY_HARD && gameInfo.exitType != QUIT_EXIT && gameInfo.exitType != TIMETRIAL_RESTART_EXIT) {
            levelSelectCount = 0;
            frameCount = 0;
            if (gameInfo.exitFlags & 2) {
                if (levelTime < bestTimes[levelInfo[gameInfo.prevLevel].bit].time) {
                    bestTimes[levelInfo[gameInfo.prevLevel].bit].time = levelTime;
                    gameInfo.mode = 1;
                    currentScreen = 5;
                    currentAnim = 0;
                    frameCount = 0;
                    levelSelectCount = 0;
                    PrepareSong(TITLE_TRACK);
                    return;
                }
            }
            InitOverlayList();
            InitBackgroundLightList();
            InitHandLightList();
            InitFootprintList();
            InitSpriteList();
            InitActorList();
            InitBulletList();
            InitWaterList();
            InitWindList();
            InitPlatformList();
            InitVentList();
            InitPlatformList();
            InitPuzzleList();
            InitSpikeList();
            InitGaribGroupList();
            InitCameoList();
            InitCollectableList();
            InitBlurList();
            InitBuzzerList();
            InitTipList();
            InitAmbientSoundList();
            InitBackdropList();
            gameInfo.mode = 15;
            LoadLevel(1);
        }
    }
}

void CheckForExit(Platform *platform) {
    Vec3 velocity;
    Vec3 position;
    Vec3 *pos;
    s32 type;

    if (gameInfo.mode != 4) {
        return;
    }
    if ((frameCount & 1) && (platform->exitType == 1 || platform->exitType == 3 || gameInfo.level == ACOURSE)) {
        velocity.z = 0.0f;
        velocity.x = 2.0f;
        velocity.y = 12.0f;
        RotateVector2D(&velocity, &velocity, exitDir);
        exitDir += exitspinspeed;
        AddOneScaledVector2D(&position, &platform->actor.pos, &velocity, 9.0f);
        position.y = platform->actor.pos.y + 4.0f;
        AddPuff(&position, &velocity, 1.0f);
        FlipVector2D(&velocity);
        AddOneScaledVector2D(&position, &platform->actor.pos, &velocity, 9.0f);
        position.y = platform->actor.pos.y + 4.0f;
        AddPuff(&position, &velocity, 1.0f);
    }
    if (ballStatic == 0 && gameInfo.world == HUB && gameInfo.level != CAVE && gameInfo.level != ACOURSE
        && saveSlot[currentSaveSlot].hubTarget != saveSlot[currentSaveSlot].hubLevel && platform->exitType != 12 && platform->exitType != 21
        && gameInfo.worldComplete[platform->exitType - 5] == 0) {
        return;
    }
    if (platform->exitType == 12 || platform->exitType == 21 || platform->exitType == 6 || platform->exitType == 8 || platform->exitType == 7 || platform->exitType == 9
        || platform->exitType == 10 || platform->exitType == 11 || (platform->exitType == 13 && gameInfo.level != ACOURSE) || gameInfo.level == WAYROOM
        || gameInfo.level == ATLANTISBONUS || gameInfo.level == ATLANTIS3A || gameInfo.level == CARNIVAL1) {
        pos = &hand.pos;
    } else {
        pos = &ball.pos;
    }
    type = platform->exitType;
    if (type == 3) {
        if (ball.obj->platform == platform && IsHandHoldingBall()) {
            DisableControl(500);
            ZERO_VECTOR(&ball.vel);
            if (worldExitTimer == 0 && !(gameInfo.exitFlags & 1)) {
                if (hand.state->mode != type) {
                    ApplyAction(HAND_JOINED);
                    ballControl.transformState = 0;
                }
                cameraInfo.destActive = 1;
                cameraInfo.destPos = camera.pos;
                worldExitTimer = 60;
                platform->path->moving = 30;
                PlaySample((Random(4) + 269), &hand.pos, 100, 128);
                PlaySample(334, &hand.pos, 100, 130);
                gameInfo.exitPlatform = platform;
            }
        }
    } else if (gameInfo.level == WAYROOM || gameInfo.level == CAVE || platform->exitType == 12 || platform->exitType == 6 || platform->exitType == 7
               || platform->exitType == 8 || platform->exitType == 9 || platform->exitType == 10 || platform->exitType == 11 || gameInfo.level == ATLANTIS3A
               || gameInfo.level == CARNIVAL1) {
        if (!(gameInfo.exitFlags & 1) && DistanceBetweenPoints(pos, &platform->actor.pos) < platform->actor.radius) {
            if (gameInfo.level == ATLANTIS3A || gameInfo.level == CARNIVAL1) {
                if (newLevExitTimer == 0) {
                    newLevExitTimer = 50;
                    PrepareSong(gameInfo.world * 4 + 32 + Random(2));
                    gameInfo.exitType = platform->exitType;
                }
            } else {
                ExitLevel(platform->exitType, 1, 1);
            }
        }
    } else if (levelExitTimer == 0 && !(gameInfo.exitFlags & 1)
               && DistanceBetweenPoints(pos, &platform->actor.pos) < platform->actor.radius) {
        if (platform->exitType == 21 || gameInfo.level == WAYROOM || gameInfo.level == ATLANTISBONUS
            || (platform->exitType == 13 ? saveSlot[currentSaveSlot].hubLevel == saveSlot[currentSaveSlot].hubTarget || ballStatic
                               : IsHandHoldingBall() || ball.link == &hand)) {
            if ((platform->exitType == 1 || gameInfo.level == ACOURSE) && gameInfo.level != SPACEBOSS1 && gameInfo.level != ATLANTIS3A
                && gameInfo.level != CARNIVAL1) {
                if (gameInfo.level != ACOURSE || IsHandHoldingBall() || ball.link == &hand) {
                    PlaySample((Random(4) + 269), &hand.pos, 100, 128);
                    levelExitTimer = 10000;
                    DeactivateCollectable(HERCULES);
                    gameInfo.exitPlatform = platform;
                    DisableControl(10000);
                    handControl.actionLock = 10000;
                    ApplyAction(HAND_SPIN);
                    hand.rotAim.y = Aabs(camera.rot.y + 3.141592654);
                    ball.flags &= ~0x18;
                    gameInfo.exitType = platform->exitType;
                }
            } else {
                ExitLevel(platform->exitType, 1, 1);
            }
        }
    }
    if (ball.link != &hand && !IsHandHoldingBall()
        && (platform->exitType == 12 || platform->exitType == 6 || platform->exitType == 7 || platform->exitType == 8 || platform->exitType == 9 || platform->exitType == 10
            || platform->exitType == 11)
        && !(gameInfo.exitFlags & 1) && ballStatic != 2 && DistanceBetweenPoints(&ball.pos, &platform->actor.pos) < platform->actor.radius) {
        AddDebris(POPB, &ball.pos, &zero, NULL, NULL);
        CreateStarCircle(&ball.pos, 5, 19);
        ball.pos.x = 0.0f;
        ball.pos.z = 0.0f;
        ball.pos.y = -5000.0f;
        ballStatic = 2;
        if (platform->exitType == 12) {
            ballLeftBehind = 8;
        } else {
            ballLeftBehind = platform->exitType + 36;
        }
    }
}

s32 LevelToWorld(s8 num) {
    if ((num >= 0 && num < 10) || num == 44) {
        return 0;
    }
    if (num == 45) {
        return 0;
    }
    if (num >= 10 && num < 15) {
        return 1;
    }
    if (num >= 15 && num < 20) {
        return 2;
    }
    if (num >= 20 && num < 25) {
        return 3;
    }
    if (num >= 25 && num < 30) {
        return 4;
    }
    if (num >= 30 && num < 35) {
        return 5;
    }
    if (num == 42) {
        return gameInfo.world;
    }
    return (num < 46) ? 6 : 7;
}

s32 GameBeingPlayed(void) {
    if (gameInfo.mode == 1) {
        return FALSE;
    }
    if (gameInfo.mode == 2) {
        return FALSE;
    }
    if (gameInfo.mode == 3) {
        return FALSE;
    }
    if (gameInfo.mode == 7) {
        return FALSE;
    }
    if (gameInfo.mode == 11) {
        return FALSE;
    }
    if (gameInfo.mode == 13) {
        return FALSE;
    }
    return TRUE;
}

void TitleScreenCameraSpin(void) {
    titleSpinVect.x = 0.0f;
    titleSpinVect.y = __sinf(titleSpinCamRot * 3.0f) * 300.0f + 400.0f;
    titleSpinVect.z = __sinf(titleSpinCamRot * 2.4) * 400.0f + 1000.0f;
    RotateVector2D(&titleSpinVect, &titleSpinVect, titleSpinCamRot);
    titleSpinVect2.x = titleSpinVect2.z = 0.0f;
    titleSpinVect2.y = __sinf(titleSpinCamRot) * 400.0f + 400.0f;
    titleSpinCamRot += 0.005;
    SetCameraOverride(titleSpinVect.x, titleSpinVect.y, titleSpinVect.z, &titleSpinVect2, &zero, 5, 0);
}

void TitleScreenCameraSpin2(void) {
    vectC.y += sinf(titleSpin2CamRot * 2.4);
    RotateVector2D(&titleSpin2Vect, &vectC, titleSpin2CamRot);
    RotateVector2D(&titleSpin2Vect2, &vectC2, titleSpin2CamRot);
    titleSpin2CamRot += 0.005;
    SetCameraOverride(titleSpin2Vect.x, titleSpin2Vect.y, titleSpin2Vect.z, &titleSpin2Vect2, &zero, 5, 0);
}

void GameOverScreen(void) {
    Bullet *bullet;

    if (gameInfo.level == SPACEBOSS3) {
        if (gameOverCount == 1) {
            FreeFGObjects();
            gamePaused = FALSE;
            StartCameo(2);
            hand.alpha = 100;
            enemies[0].brain.delayTimer = 200;
        } else if (gameOverCount < 20) {
            hand.pos.x = Random(21) - 10;
            hand.pos.y = Random(21) + 240;
            hand.pos.z = Random(21) - 10;
            gameOverCount++;
            return;
        } else if (gameOverCount == 20) {
            hand.alpha = 0;
            bullet = CreateNewBullet(BOMB_BULLET);
            bullet->actor.pos = hand.pos;
            bullet->actor.rotQuat = zeroQuat;
            bullet->actor.lifeTimer = bombLife;
            AddBullet(bullet);
        } else if (gameOverCount < 40) {
            gameOverCount++;
            return;
        }
    }
    if (gameOverCount == 1) {
        PrepareSong(gameInfo.world * 4 + 30 + Random(2));
        SetFont(0);
        gameOverText[0] = NEW_TEXT(160, -32, 255, 255, 255, 255, 1.0f, 1.0f, 0, gameString[gameInfo.language], 1);
        gameOverText[1] = NEW_TEXT(160, 288, 255, 255, 255, 255, 1.0f, 1.0f, 0, overString[gameInfo.language], 1);
        gameOverVel = 0.0f;
        gameOverY[0] = -32.0f;
        gameOverY[1] = 288.0f;
        UpdateHiscoreTable(player.score);
    }
    gameOverVel += 1.5;
    gameOverY[0] += gameOverVel;
    gameOverY[1] -= gameOverVel;
    gameOverText[0]->y = gameOverY[0];
    gameOverText[1]->y = gameOverY[1];
    gameOverCount++;
    if (gameOverY[0] + gameOverVel >= 104.0f) {
        if (gameOverVel > 0.0f) {
            gameOverText[0]->y = 104;
            gameOverY[0] = 104.0f;
            gameOverText[1]->y = 120;
            gameOverY[1] = 120.0f;
            if (gameOverVel < 5.0f) {
                gameOverVel = 0.0f;
            } else {
                gameOverVel = -gameOverVel * 0.7;
            }
        }
    }
    if ((!gameOverWait && pad[ActiveController].pressed[BUTTON_A] && gameOverCount >= 20) || gameOverCount == 100) {
        gameOverWait = 200;
        FadeScreenOut();
        gameInfo.exitFlags |= 1;
    }
    if (ScreenFadedOut()) {
        gameOverCount = 0;
        gameOverWait = 0;
        gamePaused = FALSE;
        FreeLevel();
        FreeScreen();
        gameInfo.mode = 1;
    }
}

void DoCaveStuff(void) {
    Vec3 tempVect;
    Vec3 tempVect2;
    Platform *platform;
    f32 dist;
    s32 j;
    s32 delay;
    s16 counter;
    s16 r;
    s16 g;
    s16 b;

    if (frameCount == 1) {
        gameWonCount = 0;
        hand.rot.y = hand.rotAim.y = 3.1415927f;
    }
    CalcBallTexture(6);
    BlendBallTexture(6, 0, 1.0f);
    *ballTextureData2 = (u32)ballTextureList[6]->data;
    *ballTexturePalette2 = (u32)ballTextureList[6]->palette;
    if (hand.anim->current == HANDANIM_THUMBSUP) {
        ActorHandleRotaim(&hand, 1);
        GetQuaternionFromXYZRot(&hand.rotQuat, &hand.rot);
    }
    if (saveSlot[currentSaveSlot].hubLevel == 7) {
        if (gameWonCount == 0) {
            StartCameo(0);
            levelCamDistance = 150;
        }
        gameWonCount++;
        DisableControl(2);
        for (j = 1, delay = 0; j < 8; j++, delay += 10) {
            if (gameWonCount - delay >= 0) {
                counter = gameWonCount - delay;
            } else {
                counter = 0;
            }
            if (counter == 0) {
                continue;
            }
            platform = TagToFirstPlatform(j);
            if (platform->actor.pos.y < 30.0f) {
                platform->actor.pos.y += 1.0f;
            } else if (platform->actor.pos.z > 1200.0f) {
                platform->maxSpeed = 0.0f;
                ZERO_VECTOR(&platform->actor.vel);
                if (j == 7) {
                    ExitLevel(GAME_WON_EXIT, 1, 1);
                    return;
                }
            } else if (platform->actor.pos.z < 925.0f) {
                platform->maxSpeed = 20.0f;
                tempVect2.x = 0.0f;
                tempVect2.y = 70.0f;
                tempVect2.z = 925.0f;
                SUB_VECTORS(&tempVect, &tempVect2, &platform->actor.pos);
                MakeUnit(&tempVect);
                AddToOneScaledVector(&platform->actor.vel, &tempVect, 0.5f);
            }
            platform->actor.rot.y += counter / 200.0f;
            GetQuaternionFromXYZRot(&platform->actor.rotQuat, &platform->actor.rot);
            InitPlanes(platform->actor.inst->obj->mesh, &platform->actor.pos, platform->actor.faces, &platform->actor.scale, &platform->actor.rotQuat, 1);
        }
    } else {
        gameWonCount = 0;
    }
    if (player.powerups[CRYSTALBALL]) {
        player.powerups[CRYSTALBALL] = 2;
    }
    if (caveSfx.handle) {
        UpdateContinuousSample(&caveSfx);
    }
    if (frameCount == 1) {
        numBalls = saveSlot[currentSaveSlot].hubLevel;
        caveSfx.handle = 0;
        gameInfo.caveStage = 0;
    }
    for (j = 0; j < numBalls; j++) {
        TagToFirstPlatform(j + 1)->actor.alpha = SineWave(5.0f, j * 10, frameCount) * 30.0f + 110.0f;
    }
    if (saveSlot[currentSaveSlot].hubLevel < saveSlot[currentSaveSlot].hubTarget) {
        if ((ball.pos.z < 500.0f || frameCount >= 71) && ballStatic == 0) {
            if (gameInfo.caveStage == 0) {
                gameInfo.caveStage = 1;
                ball.flags &= ~0x19;
                ball.obj->onGround = 0;
                ball.obj->planes[0] = NULL;
                if (ball.link == &hand) {
                    ApplyAction(HAND_RELEASE);
                }
                ball.link = &hand;
                handControl.releaseTimer = 0;
                ballControl.releaseTimer = 0;
                return;
            }
            DisableControl(2000);
            if (saveSlot[currentSaveSlot].hubLevel != saveSlot[currentSaveSlot].hubTarget && Random(10) < 8) {
                tempVect.y = 0.0f;
                tempVect.z = 0.0f;
                tempVect.x = 25.0f;
                RotateVector2D(&tempVect, &tempVect, TagToFirstPlatform(10)->motion->rot.y);
                if (caveSfx.handle == 0) {
                    PlayContinuousSample(&caveSfx, 60, 100, &zero, 128);
                } else {
                    caveSfx.volume = 100;
                }
                r = Random(160) + 40;
                g = Random(120) + 195;
                b = Random(160) + 40;
                r = MIN(MAX(r, 0), 255);
                g = MIN(MAX(g, 0), 255);
                b = MIN(MAX(b, 0), 255);
                ADD_VECTORS(&tempVect2, &tempVect, &TagToFirstPlatform(10)->actor.pos);
                AddLightning(&tempVect2, &ball.pos, r, g, b, 8, 1, 6, 0.5f, 1.0f);
                r = Random(160) + 40;
                g = Random(120) + 195;
                b = Random(160) + 40;
                r = MIN(MAX(r, 0), 255);
                g = MIN(MAX(g, 0), 255);
                b = MIN(MAX(b, 0), 255);
                SUB_VECTORS(&tempVect2, &TagToFirstPlatform(10)->actor.pos, &tempVect);
                AddLightning(&tempVect2, &ball.pos, r, g, b, 8, 1, 6, 0.5f, 1.0f);
            } else if (caveSfx.handle) {
                caveSfx.volume -= 5;
                if (caveSfx.volume <= 0) {
                    StopContinuousSample(&caveSfx);
                }
            }
            SetVector2D(&tempVect, &TagToFirstPlatform(saveSlot[currentSaveSlot].hubTarget)->actor.pos);
            if (gameInfo.caveStage == 1) {
                tempVect.y = 50.0f;
            } else {
                tempVect.y = -30.0f;
            }
            SUB_VECTORS(&ball.vel, &tempVect, &ball.pos);
            dist = sqrtf(DOT_PRODUCT(&ball.vel, &ball.vel));
            if (dist > ballFlySpeed) {
                SCALE_VECTOR(&ball.vel, ballFlySpeed / dist);
                return;
            }
            if (++gameInfo.caveStage == 2) {
                PlaySample((Random(4) + 269), &hand.pos, 100, 128);
                AnimateActor(&hand, HANDANIM_THUMBSUP, 0, 0, 0.0f);
                hand.rotAim.y = Aabs(camera.rot.y - PI / 2);
                hand.state->mode = HAND_NOTHING;
                handControl.actionLock = 0x3FF;
                DisableControl(20);
                ball.pos = tempVect;
                PlaySample(97, &ball.pos, 100, 128);
                ActivateCollectable(CRYSTALBALL, 2);
                FlashScreen(0, 200, 0, 280, -30);
                ball.flags &= ~0x19;
                return;
            }
            ball.pos = tempVect;
            gameInfo.caveStage = 0;
            saveSlot[currentSaveSlot].hubLevel++;
            ZERO_VECTOR(&ball.vel);
            ballWithHand = 0;
            ball.obj->spheres[0].targetRadius = 2.0f * ball.obj->spheres[0].targetRadius;
            ball.pos.x = 0.0f;
            ball.pos.y = -5000.0f;
            ball.pos.z = 300.0f;
            ball.link = NULL;
            platform = TagToFirstPlatform(saveSlot[currentSaveSlot].hubLevel);
            platform->actor.pos.y = -30.0f;
            ballStart = ball.pos;
            ballStatic = 2;
            numBalls++;
            InitPlanes(platform->actor.inst->obj->mesh, &platform->actor.pos, platform->actor.faces, &platform->actor.scale, &platform->actor.rotQuat, 1);
            handControl.noControlTimer = 1;
        }
    } else if (caveSfx.handle) {
        caveSfx.volume -= 5;
        if (caveSfx.volume <= 0) {
            StopContinuousSample(&caveSfx);
        }
    }
}

void UpdatePause(void) {
    char buf[24];
    Platform *platform;
    s16 yPos;
    s32 i;
    s32 strNum;
    u8 newFontAlpha;

    if (gamePaused || (gameInfo.exitFlags & 1)) {
        if (!timeupTimer && !readingTip && !gameOverCount && (saveSlot[currentSaveSlot].difficultyLevel != DIFFICULTY_HARD || bonusTimer)) {
            musVol *= 0.9;
        }
        sfxVol *= 0.9;
        MusSetMasterVolume(2, musVol);
        SetSfxVols();
    } else {
        musVol += ((curSoundMUSVol * 0x7FFF) / 160 - musVol) * 0.1;
        sfxVol += ((curSoundFXVol * 0x7FFF) / 160 - sfxVol) * 0.1;
        MusSetMasterVolume(2, musVol);
        MusSetMasterVolume(1, sfxVol);
    }
    if (!pauseMode) {
        return;
    }
    SetFont(1);
    if (tickGfx != NULL) {
        UpdateScreenDebris(tickGfx);
    }
    if (crossGfx != NULL) {
        UpdateScreenDebris(crossGfx);
    }
    if (pauseCount == 0) {
        memset(cheatTempString, 0xFF, 8);
        cheatMsgCount = 0;
        tickGfx = crossGfx = NULL;
        pauseNumItems = 1;
        pauseMenuList[0] = 0;
        if (saveSlot[currentSaveSlot].difficultyLevel == DIFFICULTY_HARD
            || (gameInfo.level != ATLANTISBOSS && gameInfo.level != CARNIVALBOSS && gameInfo.level != PIRATESBOSS && gameInfo.level != PREHISTORICBOSS
                && gameInfo.level != FEARBOSS && gameInfo.cameoActive == 0 && gameInfo.level != ATLANTISBONUS && gameInfo.level != CARNIVALBONUS
                && gameInfo.level != PIRATESBONUS && gameInfo.level != PREHISTORICBONUS && gameInfo.level != FEARBONUS && gameInfo.level != SPACEBONUS
                && (hand.obj->onGround || ball.link == &hand) && handControl.deathTimer == 0
                && ballControl.loseTimer == 0
                && (ballStatic || ball.obj->onGround || ((ball.flags & 0x200000) && ball.state->waterState)))) {
            pauseItemActive[0] = 1;
            pauseMenuList[pauseNumItems++] = 1;
        } else {
            pauseItemActive[0] = 0;
        }
        if (!cameFromTitle && (u8)gameInfo.level >= 8) {
            pauseItemActive[1] = 1;
            pauseMenuList[pauseNumItems++] = 2;
        } else {
            pauseItemActive[1] = 0;
        }
        pauseMenuList[pauseNumItems++] = 3;
        switch (pauseNumItems) {
            case 2:
                pauseBaseY = 90;
                break;
            case 3:
                pauseBaseY = 80;
                break;
            case 4:
                pauseBaseY = 70;
                break;
        }
        pauseQuestion = 0;
        gamePaused = 1;
        pauseSelection = 0;
        tipOverlay.x = tipOverlay.y = 0.0f;
        tipOverlay2.x = tipOverlay2.y = 0.0f;
        AddOverlay(&tipOverlay2);
        tipOverlay.scaleX = 0;
        tipOverlay.scaleY = 0;
        tipOverlay2.scaleX = 0;
        tipOverlay2.scaleY = 0;
        tipOverlay2.alpha = 200;
        tipOverlay2.r = 0;
        tipOverlay2.g = 200;
        tipOverlay2.b = 0;
        pauseFontAlpha = 0;
        pauseNumHoops = 0;
        D_801E7660 = 0.5f;
        for (platform = platformList.head.next; platform != &platformList.head; platform = platform->next) {
            if (platform->restartGate != NULL && (!platform->restartGate->untouched || (cheatActivated & 4))) {
                if (handControl.checkpoint == platform) {
                    pauseHoop = pauseNumHoops;
                }
                pauseNumHoops++;
            }
        }
        pauseWait = 0;
        pauseRestart = handControl.checkpoint;
    }
    if (pauseCount >= 7 && pauseCount < 1000 && !IsBonusLevel() && gameInfo.level != SPACEBOSS3 && (u8)CheckForCheats()) {
        pauseCount = 1000;
        if (tickGfx != NULL) {
            tickGfx->flags &= ~8;
        }
        if (crossGfx != NULL) {
            crossGfx->flags &= ~8;
        }
        pauseQuestion = 0;
    }
    if (pauseWait == 0) {
        if (pauseSelection == 1 && pauseNumHoops >= 2 && saveSlot[currentSaveSlot].difficultyLevel != DIFFICULTY_HARD) {
            if (pad[ActiveController].stickX >= 40) {
                PlaySampleNot3D(180, 100, 128, 128);
                pauseWait = 5;
                pauseHoop = (pauseHoop + 1) % pauseNumHoops;
                platform = &platformList.head;
                for (i = 0; i < pauseHoop + 1; i++) {
                    platform = platform->next;
                    while (platform != &platformList.head) {
                        if (platform->restartGate != NULL && (!platform->restartGate->untouched || (cheatActivated & 4))) {
                            break;
                        }
                        platform = platform->next;
                    }
                }
                pauseRestart = platform;
            }
            if (pad[ActiveController].stickX < -39) {
                PlaySampleNot3D(180, 100, 128, 128);
                pauseWait = 5;
                pauseHoop = (pauseHoop + pauseNumHoops - 1) % pauseNumHoops;
                platform = &platformList.head;
                for (i = 0; i < pauseHoop + 1; i++) {
                    platform = platform->next;
                    while (platform != &platformList.head) {
                        if (platform->restartGate != NULL && (!platform->restartGate->untouched || (cheatActivated & 4))) {
                            break;
                        }
                        platform = platform->next;
                    }
                }
                pauseRestart = platform;
            }
        }
    }
    if (pauseWait > 0) {
        pauseWait--;
    }
    if (pauseCount < 6) {
        tipOverlay.x += 26.0f;
        tipOverlay.y += 18.0f;
        tipOverlay.scaleX += 233;
        tipOverlay.scaleY += (pauseNumItems * 128 + 512) / 6;
        tipOverlay2.x += 26.0f;
        tipOverlay2.y += 18.0f;
        tipOverlay2.scaleX += 933;
        tipOverlay2.scaleY += (pauseNumItems * 128 + 512) * 4 / 6;
    } else if (pauseCount < 12) {
        pauseFontAlpha += 42;
        tipOverlay.x = 160.0f;
        tipOverlay.y = 110.0f;
        tipOverlay.scaleX = 1400;
        tipOverlay.scaleY = pauseNumItems * 128 + 512;
        tipOverlay2.x = 160.0f;
        tipOverlay2.y = 110.0f;
        tipOverlay2.scaleX = 5600;
        tipOverlay2.scaleY = (pauseNumItems * 128 + 512) * 4;
    }
    yPos = pauseBaseY;
    SetFont(0);
    NEW_TEXT(160, yPos, 255, 255, 255, pauseFontAlpha, 1.0f, 1.0f, 1, pauseString[gameInfo.language], 1);
    SetFont(1);
    yPos += 30;
    newFontAlpha = (pauseSelection == 0 && !(frameCount2 & 4)) ? pauseFontAlpha / 2 : pauseFontAlpha;
    NEW_TEXT(160, yPos, 255, 255, 255, newFontAlpha,
             (pauseSelection == 0) ? 1.0f : 0.8f, (pauseSelection == 0) ? 1.0f : 0.8f, 1,
             pauseMenuString[0][gameInfo.language], 1);
    yPos += 20;
    if (pauseItemActive[0]) {
        newFontAlpha = (pauseMenuList[pauseSelection] == 1 && !(frameCount2 & 4)) ? pauseFontAlpha / 2 : pauseFontAlpha;
        strNum = 1;
        if (saveSlot[currentSaveSlot].difficultyLevel == DIFFICULTY_HARD) {
            strNum = 4;
        }
        if (pauseNumHoops >= 2 && strNum == 1) {
            sprintf(buf, "%s %d/%d", pauseMenuString[1][gameInfo.language], pauseHoop + 1, pauseNumHoops);
        } else {
            sprintf(buf, "%s", pauseMenuString[strNum][gameInfo.language]);
        }
        NEW_TEXT(160, yPos, 255, 255, 255, newFontAlpha,
                 (pauseMenuList[pauseSelection] == 1) ? 1.0f : 0.8f, (pauseMenuList[pauseSelection] == 1) ? 1.0f : 0.8f, 1,
                 buf, 1);
        yPos += 20;
    }
    if (pauseItemActive[1]) {
        newFontAlpha = (pauseMenuList[pauseSelection] == 2 && !(frameCount2 & 4)) ? pauseFontAlpha / 2 : pauseFontAlpha;
        if (saveSlot[currentSaveSlot].difficultyLevel == DIFFICULTY_HARD) {
            NEW_TEXT(160, yPos, 255, 255, 255,
                     newFontAlpha,
                     (pauseMenuList[pauseSelection] == 2) ? 1.0f : 0.8f, (pauseMenuList[pauseSelection] == 2) ? 1.0f : 0.8f,
                     1, difficultyStrings[2][gameInfo.language], 1);
        } else {
            NEW_TEXT(160, yPos, 255, 255, 255,
                     newFontAlpha,
                     (pauseMenuList[pauseSelection] == 2) ? 1.0f : 0.8f, (pauseMenuList[pauseSelection] == 2) ? 1.0f : 0.8f,
                     1, pauseMenuString[2][gameInfo.language], 1);
        }
        yPos += 20;
    }
    newFontAlpha = (pauseMenuList[pauseSelection] == 3 && !(frameCount2 & 4)) ? pauseFontAlpha / 2 : pauseFontAlpha;
    NEW_TEXT(160, yPos, 255, 255, 255, newFontAlpha,
             (pauseMenuList[pauseSelection] == 3) ? 1.0f : 0.8f, (pauseMenuList[pauseSelection] == 3) ? 1.0f : 0.8f, 1,
             pauseMenuString[3][gameInfo.language], 1);
    if (pad[ActiveController].pressed[BUTTON_START] && pauseCount >= 13 && pauseCount < 1000) {
        pauseCount = 1000;
        pauseQuestion = 0;
        if (tickGfx != NULL) {
            tickGfx->flags &= ~8;
        }
        if (crossGfx != NULL) {
            crossGfx->flags &= ~8;
        }
    }
    if (pad[ActiveController].pressed[BUTTON_A] && pauseCount >= 13 && pauseCount < 1000) {
        PlaySampleNot3D(181, 100, 128, 128);
        pad[ActiveController].latch[BUTTON_A] = 1;
        pausePadWait = 0;
        pad[ActiveController].sincePress[BUTTON_A] = 99999;
        switch (pauseMenuList[pauseSelection]) {
            case 1:
                if (saveSlot[currentSaveSlot].difficultyLevel == DIFFICULTY_HARD) {
                    if (pauseQuestion) {
                        if (pauseQuestion == 1) {
                            pauseCount = 1000;
                            player.lives = difficultyStats[saveSlot[currentSaveSlot].difficultyLevel].lives;
                            player.health = 3;
                            ExitLevel(TIMETRIAL_RESTART_EXIT, 1, 0);
                            numGaribs = 0;
                            player.score = 0;
                            pauseQuestion = 0;
                            break;
                        }
                        pauseQuestion = 0;
                        if (tickGfx != NULL) {
                            tickGfx->flags &= ~8;
                        }
                        if (crossGfx != NULL) {
                            crossGfx->flags &= ~8;
                        }
                    } else {
                        tickGfx = AddScreenDebris(120, yPos - 40, 0, 0, SDT_QUESTION_YES, 1);
                        crossGfx = AddScreenDebris(200, yPos - 40, 0, 0, SDT_QUESTION_NO, 1);
                        pauseQuestion = 1;
                    }
                    break;
                }
                handControl.checkpoint = pauseRestart;
                handControl.checkpoint->restartGate->restartTimer = 65;
                if (pauseRestart->restartGate->untouched) {
                    pauseRestart->restartGate->cameraPos = pauseRestart->actor.pos;
                    pauseRestart->restartGate->cameraPos.z += 100.0f;
                }
            case 0:
                pauseCount = 1000;
                break;
            case 2:
                if (pauseQuestion) {
                    if (pauseQuestion == 1) {
                        pauseCount = 1000;
                        ExitLevel(HUB_EXIT, 1, 0);
                        player.health = 3;
                        pauseQuestion = 0;
                        break;
                    }
                    pauseQuestion = 0;
                    if (tickGfx != NULL) {
                        tickGfx->flags &= ~8;
                    }
                    if (crossGfx != NULL) {
                        crossGfx->flags &= ~8;
                    }
                } else {
                    tickGfx = AddScreenDebris(120, yPos - 20, 0, 0, SDT_QUESTION_YES, 1);
                    crossGfx = AddScreenDebris(200, yPos - 20, 0, 0, SDT_QUESTION_NO, 1);
                    pauseQuestion = 1;
                }
                break;
            case 3:
                if (pauseQuestion) {
                    if (pauseQuestion == 1) {
                        pauseCount = 1000;
                        ExitLevel(QUIT_EXIT, 1, 0);
                        pauseCount = 0;
                        pauseMode = 0;
                        gamePaused = 0;
                        pauseQuestion = 0;
                        frameCount = 0;
                        levelSelectCount = 0;
                        return;
                    }
                    pauseQuestion = 0;
                    if (tickGfx != NULL) {
                        tickGfx->flags &= ~8;
                    }
                    if (crossGfx != NULL) {
                        crossGfx->flags &= ~8;
                    }
                } else {
                    tickGfx = AddScreenDebris(100, yPos, 0, 0, SDT_QUESTION_YES, 1);
                    crossGfx = AddScreenDebris(220, yPos, 0, 0, SDT_QUESTION_NO, 1);
                    pauseQuestion = 1;
                }
                break;
        }
    }
    if (pauseCount < 100 || pauseCount >= 1000) {
        pauseCount++;
    }
    if (pauseCount >= 1000 && pauseCount < 1007) {
        pauseFontAlpha -= 42;
    }
    if (pauseCount >= 1007) {
        tipOverlay.x += 26.0f;
        tipOverlay.y += 20.0f;
        tipOverlay.scaleX -= 233;
        tipOverlay.scaleY -= (pauseNumItems * 128 + 512) / 6;
        tipOverlay2.x += 26.0f;
        tipOverlay2.y += 20.0f;
        tipOverlay2.scaleX -= 933;
        tipOverlay2.scaleY -= (pauseNumItems * 128 + 512) * 4 / 6;
        pauseFontAlpha = 0;
        cheatMsgCount = 0;
        memset(cheatTempString, 0xFF, 8);
    }
    if (pauseCount == 1012) {
        pauseMode = 0;
        gamePaused = 0;
        SubOverlay(&tipOverlay);
        SubOverlay(&tipOverlay2);
        pauseCount = 0;
    }
    if (!pauseQuestion) {
        if (pad[ActiveController].stickY >= 51) {
            if (!pausePadWait && pauseSelection != 0) {
                pauseSelection--;
                PlaySampleNot3D(179, 100, 128, 128);
            }
            pausePadWait = 1;
        } else if (pad[ActiveController].stickY < -50) {
            if (!pausePadWait && pauseSelection < pauseNumItems - 1) {
                pauseSelection++;
                PlaySampleNot3D(179, 100, 128, 128);
            }
            pausePadWait = 1;
        } else {
            pausePadWait = 0;
        }
    } else {
        if (tickGfx != NULL) {
            tickGfx->y = yPos + SineWave(1.0f, frameCount2 / 2, 0) * 6.0f;
        }
        if (crossGfx != NULL) {
            crossGfx->y = yPos + SineWave(1.0f, frameCount2 / 2, 0) * 6.0f;
        }
        if (pauseMenuList[pauseSelection] == 2) {
            if (tickGfx != NULL) {
                tickGfx->y -= 20.0f;
            }
            if (crossGfx != NULL) {
                crossGfx->y -= 20.0f;
            }
        }
        if (pauseMenuList[pauseSelection] == 1) {
            if (tickGfx != NULL) {
                tickGfx->y -= 40.0f;
            }
            if (crossGfx != NULL) {
                crossGfx->y -= 40.0f;
            }
        }
        if (pauseQuestion == 2) {
            if (tickGfx != NULL) {
                tickGfx->x = 400.0f;
            }
            if (crossGfx != NULL) {
                crossGfx->x = 220.0f;
            }
        }
        if (pauseQuestion == 1) {
            if (tickGfx != NULL) {
                tickGfx->x = 220.0f;
            }
            if (crossGfx != NULL) {
                crossGfx->x = 400.0f;
            }
        }
        if ((pad[ActiveController].stickX >= 51 || pad[ActiveController].stickY < -50) && pauseQuestion == 1) {
            pauseQuestion = 2;
            PlaySampleNot3D(179, 100, 128, 128);
        }
        if ((pad[ActiveController].stickX < -50 || pad[ActiveController].stickY >= 51) && pauseQuestion == 2) {
            pauseQuestion = 1;
            PlaySampleNot3D(179, 100, 128, 128);
        }
    }
    SetFont(0);
}

s32 CheckForLevelCompleted(void) {
    s32 allGaribs = FALSE;

    if (levelTime < bestTimes[levelInfo[gameInfo.prevLevel].bit].time && (gameInfo.exitFlags & 2)) {
        bestTimes[levelInfo[gameInfo.prevLevel].bit].time = levelTime;
        bestTimes[levelInfo[gameInfo.prevLevel].bit].name[0] = currentName[0];
        bestTimes[levelInfo[gameInfo.prevLevel].bit].name[1] = currentName[1];
        bestTimes[levelInfo[gameInfo.prevLevel].bit].name[2] = currentName[2];
    }
    if (gameInfo.exitFlags & 2) {
        saveSlot[currentSaveSlot].levels |= 1 << levelInfo[gameInfo.prevLevel].bit;
        if (player.garibs >= numGaribs && numGaribs > 0) {
            saveSlot[currentSaveSlot].allGaribs |= 1 << levelInfo[gameInfo.prevLevel].bit;
            allGaribs = TRUE;
        }
    }
    oldNumGaribs = numGaribs;
    numGaribs = 0;
    saveSlot[currentSaveSlot].score = player.score + player.garibs * 10;
    UpdateSaveSlot(allGaribs);
    SaveBestTimes();
    SaveHiscoreTable();
    SaveCRC();
    return allGaribs;
}

void Update3DTitleGraphics(void) {
    s32 input = 6;
    s32 x;

    if (levelSelectCount == 0) {
        frameCount = frameCount2 = 0;
        InitOverlayList();
        InitBackgroundLightList();
        InitHandLightList();
        InitFootprintList();
        InitSpriteList();
        InitActorList();
        InitBulletList();
        InitWaterList();
        InitWindList();
        InitPlatformList();
        InitVentList();
        InitPlatformList();
        InitPuzzleList();
        InitSpikeList();
        InitGaribGroupList();
        InitCameoList();
        InitCollectableList();
        InitBlurList();
        InitBuzzerList();
        InitTipList();
        InitAmbientSoundList();
        InitBackdropList();
        cameraInfo.focus = &zero;
        LoadLevel(1);
        fog.on = 0;
        FadeScreenIn();
    }
    if (frameCount >= 26) {
        StartDrawing("u3dtg");
    }
    TitleScreenCameraSpin();
    if (levelSelectCount == 0) {
        OLDfieldOfView = fieldOfView;
        fieldOfView = fieldOfViewAim = 45;
        fog.r = 130;
        fog.g = 130;
        fog.b = 200;
        fog.on = 1;
        farPlaneDist = 2500;
        LoadTextureBank(TITLE_TEX_BANK);
        FreeBackdropListInline();
        FadeScreenIn();
        SetFont(1);
    }
    levelSelectCount++;
    if (!disableGraphics) {
        input = 6;
        if (pad[ActiveController].pressed[BUTTON_A] || pad[ActiveController].pressed[BUTTON_B] || pad[ActiveController].pressed[BUTTON_START]) {
            input = 4;
        }
        if (pad[ActiveController].pressed[BUTTON_Z]) {
            input = 5;
        }
        if (waitTimer == 0) {
            if (pad[ActiveController].stickY) {
                x = pad[ActiveController].stickY;
                if (x < 0) {
                    x = -x;
                }
                if (x > 40) {
                    waitTimer = 2;
                }
                if (pad[ActiveController].stickY > 40) {
                    input = 1;
                } else if (pad[ActiveController].stickY < -40) {
                    input = 0;
                }
            }
        } else {
            waitTimer--;
        }
        DrawScreen(&input);
    }
}

void SelectLanguage(void) {
    s32 i;
    s32 x;
    Actor *actor;
    s32 k;

    if (osTvType != 0) {
        gameInfo.language = 0;
        gameInfo.mode = 1;
        frameCount = frameCount2 = 0;
        SetupFlythruScreen();
        return;
    }
    if (frameCount == 0) {
        languageCount = 0;
        gameInfo.language = 0;
        StopDrawing("sellang");
        LoadTextureBank(TITLE_TEX_BANK);
        LoadTextureBank(GENERIC_TEX_BANK);
        InitScreenDebris();
        FadeScreenIn();
        LoadObjectBank(70);
        initFGObject("flaguk.ndo", 400.0f, -200.0f, 1000.0f, 0.0f, 3.1415927f, 0.0f, 1.6f, 1.6f, 1.6f, 255);
        initFGObject("flagfr.ndo", 100.0f, -200.0f, 1000.0f, 0.0f, 3.1415927f, 0.0f, 1.6f, 1.6f, 1.6f, 255);
        initFGObject("flagge.ndo", -200.0f, -200.0f, 1000.0f, 0.0f, 3.1415927f, 0.0f, 1.6f, 1.6f, 1.6f, 255);
        for (i = 0, x = 0; i < 3; i++) {
            InitActorStructures(foregroundObjects[i], 4);
            foregroundObjects[i]->scale.x = 1.6f;
            foregroundObjects[i]->scale.y = 1.6f;
            foregroundObjects[i]->scale.z = 1.6f;
            AnimateActor(foregroundObjects[i], 0, 1, 0, 0);
            foregroundObjects[i]->anim->time = x;
            x += 10;
        }
        pBackground = JallocAlloc(sizeof(Backdrop), 1, "Backdrop");
        pBackground->r = pBackground->g = pBackground->b = pBackground->alpha = 255;
        AddBackdrop(pBackground);
        FindTexture(&pBackground->tex, UpdateCRC("backgr.bmp"), 1);
        pBackground->scaleX = 0x400;
        pBackground->scaleY = 0x400;
        fog.r = 0;
        fog.g = 0;
        fog.b = 0;
        StartDrawing("sellang");
        MusSetMasterVolume(1, 0x55F0);
    }
    HandleScreenDebris();
    if (languageCount < 1000 && (frameCount > 400 || AnyDebouncePressed(ActiveController))) {
        languageCount = 1000;
        FadeScreenOut();
        PlaySampleNot3D(216, 128, 120, 120);
    }
    if (pad[ActiveController].stickX > 50) {
        if (languageWait == 0) {
            languageWait = 5;
            if (gameInfo.language < 2) {
                gameInfo.language++;
                PlaySampleNot3D(212, 128, 120, 120);
            }
        }
    } else if (pad[ActiveController].stickX < -50) {
        if (languageWait == 0) {
            languageWait = 5;
            if (gameInfo.language > 0) {
                gameInfo.language--;
                PlaySampleNot3D(212, 128, 120, 120);
            }
        }
    } else {
        languageWait = 0;
    }
    if (languageWait != 0) {
        languageWait--;
    }
    for (i = 0; i < 3; i++) {
        if (i == gameInfo.language) {
            foregroundObjects[i]->alpha = 100;
            foregroundObjects[i]->rot.z = SineWave(flagFreq, frameCount, 0) / flagVal;
        } else {
            foregroundObjects[i]->alpha = 50;
            actor = foregroundObjects[i];
            if (FindShortestAngle(actor->rot.z, 0.0f) <= 0.01) {
                actor->rot.z = 0.0f;
            } else if (actor->rot.z < 0.0f) {
                if (-actor->rot.z > PI) {
                    actor->rot.z -= MIN(FindShortestAngle(actor->rot.z, 0.0f) / 2, 0.1);
                } else {
                    actor->rot.z += MIN(FindShortestAngle(actor->rot.z, 0.0f) / 2, 0.1);
                }
            } else if (actor->rot.z > 0.0f) {
                if (actor->rot.z > PI) {
                    actor->rot.z += MIN(FindShortestAngle(actor->rot.z, 0.0f) / 2, 0.1);
                } else {
                    actor->rot.z -= MIN(FindShortestAngle(actor->rot.z, 0.0f) / 2, 0.1);
                }
            }
        }
    }
    for (k = 0; k < numForegroundObjects; k++) {
        GetQuaternionFromXYZRot(&foregroundObjects[k]->rotQuat, &foregroundObjects[k]->rot);
    }
    frameCount++;
    frameCount2++;
    if (languageCount < 100 || languageCount >= 1000) {
        languageCount++;
    }
    if (languageCount == 1010 || !controllerPresent) {
        if (osTvType != 0) {
            gameInfo.language = 0;
        }
        for (i = 0; i < 3; i++) {
            SubOverlay(&languageFlagOL[i]);
        }
        FreeFGObjects();
        FreeScreen();
        JallocFree(&pBackground);
        gameInfo.mode = 1;
        frameCount = frameCount2 = 0;
        SetupFlythruScreen();
    }
}

s32 BonusOpen(void) {
    s32 open = FALSE;

    if (cheatActivated & 0x2000000) {
        open = TRUE;
    } else if (saveSlot[currentSaveSlot].difficultyLevel) {
        if ((saveSlot[currentSaveSlot].levels >> ((gameInfo.world - 1) * 5 + 4)) & 1) {
            open = TRUE;
        } else if (((saveSlot[currentSaveSlot].allGaribs >> ((gameInfo.world - 1) * 5)) & 1) &&
                   ((saveSlot[currentSaveSlot].allGaribs >> ((gameInfo.world - 1) * 5 + 1)) & 1) &&
                   ((saveSlot[currentSaveSlot].allGaribs >> ((gameInfo.world - 1) * 5 + 2)) & 1)) {
            open = TRUE;
        }
    }
    return open;
}

void ExitLevel(s32 type, s32 pause, s32 complete) {
    if (!(gameInfo.exitFlags & 1) || type == 15) {
        gameInfo.exitFlags |= 1;
        if (complete) {
            gameInfo.exitFlags |= 2;
        }
        gameInfo.exitType = type;
        if (!gameInfo.fade2) {
            FadeScreenOut();
        }
        endOfLevelCount = 0;
        if (osTvType == 0) {
            levelTime = (f32)player.levelFrames / (50.0 / 3.0);
        } else {
            levelTime = (f32)player.levelFrames / 20.0f;
        }
        if (pause) {
            gamePaused = TRUE;
        }
        if (type == 15) {
            UpdateHiscoreTable(player.score);
        }
    }
}

s32 IsBonusLevel(void) {
    s32 ret = FALSE;

    if (gameInfo.level == ATLANTISBONUS || gameInfo.level == CARNIVALBONUS || gameInfo.level == PIRATESBONUS ||
        gameInfo.level == PREHISTORICBONUS || gameInfo.level == FEARBONUS || gameInfo.level == SPACEBONUS) {
        ret = TRUE;
    }
    return ret;
}

void DoOutroEffects(void) {
    Vec3 tempVect;
    Quat q;
    Actor *actor;
    Debris *debris;
    s32 x;
    f32 time;

    if (gameWonCount) {
        GameWonScreen();
        return;
    }
    if (frameCount == 1) {
        outroBullet = NULL;
        outrospeed = 0;
    }
    if (outrospeed) {
        if (++outrospeed < 4) {
            originalFog.r = originalFog.g = originalFog.b = fog.r = fog.g = fog.b = 255;
            for (actor = actorList.head.next; actor != &actorList.head; actor = actor->next) {
                if (actor != &camera) {
                    actor->pos.y = -8000.0f;
                }
            }
            spriteList.head.next->pos.y = -8000.0f;
            titleSlideOL = JallocAlloc(sizeof(Backdrop), 1, "Backdrop");
            FindTexture(&titleSlideOL->tex, UpdateCRC("wizend2.bmp"), 1);
            titleSlideOL->r = titleSlideOL->g = titleSlideOL->b = 255;
            titleSlideOL->scaleX = 0x400;
            titleSlideOL->scaleY = 0x400;
            FreeBackdropListInline();
            AddBackdrop(titleSlideOL);
        }
        titleSlideOL->alpha = MIN(outrospeed * 30, 255);
        if (titleSlideOL->alpha == 255) {
            gameWonCount = 1;
            outrospeed = 0;
        }
        return;
    }
    if (actorList.head.next != NULL && actorList.head.next->anim != NULL && actorList.head.next->anim->finishedAnim) {
        outrospeed = 1;
        originalFog.r = originalFog.g = originalFog.b = fog.r = fog.g = fog.b = 255;
    }
    time = actorList.head.next->anim->time;
    if (time > 200.0f && time < 260.0f) {
        SUB_VECTORS(&tempVect, &cameoExtraSphere->prevPos, &cameoExtraSphere->pos);
        if (Random(30) < 20) {
            AddEvilGloveFire(&cameoExtraSphere->pos, &tempVect, 6, 3.0f, 2.0f);
        } else {
            AddEvilGloveFire(&cameoExtraSphere->pos, &tempVect, 17, 2.0f, 1.5f);
        }
    }
    if (time >= 428.0f && time < 429.0f) {
        CreateDustCircleScaled(&cameoExtraSphere->pos, 4, 0, 2.0f);
    }
    if (time > 450.0f && time < 500.0f && (frameCount & 1)) {
        tempVect = cameoExtraSphere->pos;
        tempVect.y -= 14.0f;
        tempVect.x += Random(9) - 4;
        tempVect.z += Random(9) - 4;
        AddDebris(DUST, &tempVect, &zero, NULL, NULL);
    }
    if (outroFireTime <= time && time < outroFireTime + 1) {
        FlashScreen(255, 255, 0, 255, -30);
        outroBullet = CreateNewBullet(ETHEL_BULLET);
        outroBullet->actor.pos = cameoExtraSphere[1].pos;
        SUB_VECTORS(&outroBullet->actor.vel, &cameoExtraSphere->pos, &outroBullet->actor.pos);
        MakeUnit(&outroBullet->actor.vel);
        SCALE_VECTOR(&outroBullet->actor.vel, 7.0f);
        InitBulletSpritesOrMesh(outroBullet, 0, 0);
        outroBullet->sprites[0].r = 255;
        outroBullet->sprites[0].g = 50;
        outroBullet->sprites[0].b = 50;
        AddBullet(outroBullet);
        ADD_VECTORS(&tempVect, &outroBullet->actor.pos, &outroBullet->actor.vel);
        AddBulletFlash(&tempVect);
        ZERO_VECTOR(&outroBullet->actor.vel);
        outroBulletPos = outroBullet->actor.pos;
    }
    if (outroBullet != NULL) {
        SUB_VECTORS(&q, &cameoExtraSphere->pos, &outroBulletPos);
        if (LENGTH(&q) < outrofireframe) {
            AddDebris(POPB, &outroBullet->actor.pos, &zero, NULL, NULL);
            CreateStarCircle(&outroBullet->actor.pos, 5, 19);
            SubBullet(outroBullet);
            outroBullet = NULL;
        } else {
            MakeUnit((Vec3 *)&q);
            tempVect.y = 0.0f;
            tempVect.z = 0.0f;
            tempVect.x = 6.0f;
            q.w = (f32)frameCount / outroSpinDiv;
            RotateVectorByRotation(&tempVect, &tempVect, &q);
            SCALE_VECTOR(&q, outrofireframe);
            ADD_TO_VECTOR(&outroBulletPos, &q);
            ADD_VECTORS(&outroBullet->actor.pos, &outroBulletPos, &tempVect);
        }
    }
    if (time > 520.0f && time < 570.0f && !(frameCount & 1)) {
        if (time < 555.0f) {
            tempVect.x = (Random(9) - 4) / 5.0f;
            tempVect.y = 2.0f;
            tempVect.z = (Random(9) - 4) / 5.0f;
        } else {
            tempVect.x = -1.5f;
            tempVect.y = 0.2f;
            tempVect.z = (Random(9) - 4) / 7.0f;
        }
        debris = AddDebris(NEW_DUST, &cameoExtraSphere[1].pos, &tempVect, NULL, NULL);
        if (debris != NULL) {
            debris->pos.y += spiral;
            debris->sprite.pos.y += spiral;
            debris->sprite.anim.def.startScale = (f32)debris->sprite.anim.def.startScale * 0.6;
            debris->sprite.anim.def.endScale = (f32)debris->sprite.anim.def.endScale * 0.6;
            debris->sprite.r = 255;
            debris->sprite.g = 255;
            debris->sprite.b = 255;
            debris->sprite.alpha = 255;
            debris->sprite.anim.def.startAlpha = 255;
            debris->sprite.anim.def.endAlpha = 64;
            debris->sprite.scaleX = (f32)debris->sprite.scaleX * 0.6;
            debris->sprite.scaleY = (f32)debris->sprite.scaleY * 0.6;
        }
    }
    if (time < 200.0f) {
        for (x = 2; x < 9; x++) {
            if (frameCount & 1) {
                tempVect.x = cameoExtraSphere[x].pos.x + Random(19) - 9.0f;
                tempVect.y = cameoExtraSphere[x].pos.y + Random(19) - 9.0f;
                tempVect.z = cameoExtraSphere[x].pos.z + Random(19) - 9.0f;
                AddDebris(FAIRY, &tempVect, &zero, NULL, NULL);
            }
        }
    }
}

void DoIntroEffects(void) {
    Actor *actor = actorList.head.next;
    Vec3 tempVect;
    GameLight *light;
    Bullet *bullet;
    Debris *debris;
    s32 x;

    if (frameCount < 2) {
        crysOff = 0;
        introBullets[0] = introBullets[1] = introBullets[2] = NULL;
        introBulletNum = 0;
    }
    if (crysOff) {
        if (crysOff < 10) {
            introText->a -= 25;
        }
        if (--crysOff == 0) {
            SubText(introText);
            return;
        }
    } else {
        for (x = 0; introPause[x].frame != 0; x++) {
            if (TriggerAtFrame(introPause[x].frame, actor)) {
                FlashScreen(255, 255, 255, 280, 1);
                introText = NEW_TEXT(160, 200, 255, 255, 255, 255, 0.8f, 0.8f, 0, introString[x][gameInfo.language], 1);
                crysOff = introPause[x].time + 10;
            }
        }
    }
    if (TriggerAtFrame(777.0f, actor) || TriggerAtFrame(778.0f, actor) || TriggerAtFrame(779.0f, actor)) {
        FlashScreen(255, 0, 0, 255, 0);
    }
    if (TriggerAtFrame(779.0f, actor)) {
        fog.r = 164;
        fog.g = 16;
        fog.b = 2;
        fog.min = 980;
        originalFog = fog;
        farPlaneDist = 3000;
        FindTexture(&backdropList.head.next->tex, UpdateCRC("hubevil.bmp"), 1);
        backdropList.head.next->tex->height--;
    }
    if (frameCount >= 860 && frameCount <= 876) {
        if (frameCount == 876) {
            for (x = 0; x < 3; x++) {
                SubBullet(introBullets[x]);
            }
        } else {
            tempVect.z = 0.0f;
            tempVect.x = 0.707f;
            tempVect.y = -0.707f;
            AddOneScaledVector(&introBullets[0]->actor.pos, &cameoExtraSphere[2].pos, &tempVect, (875 - frameCount) * 4);
        }
    }
    if (frameCount >= 338 && frameCount <= 420 && !(frameCount & 1)) {
        bullet = CreateNewBullet(SMOKE_BULLET);
        bullet->actor.pos.z = 0.0f;
        bullet->actor.pos.x = -25.0f;
        bullet->actor.pos.y = -40.0f;
        bullet->actor.pos.x += Random(25) - 12;
        bullet->actor.pos.z += Random(25) - 12;
        bullet->actor.vel.x = 0.0f;
        bullet->actor.vel.y = Random(4) + 4;
        bullet->actor.vel.z = 0.0f;
        InitBulletSpritesOrMesh(bullet, 0, 0);
        bullet->sprites[0].anim.def.startScale = (frameCount * 3 - 990 + Random(56)) / 2;
        bullet->sprites[0].anim.def.endScale = (frameCount * 3 - 870 + Random(56)) / 2;
        bullet->sprites[0].r = darkMul[0] + Random(40);
        bullet->sprites[0].g = darkMul[1];
        bullet->sprites[0].b = darkMul[2] + Random(40);
        AddBullet(bullet);
    }
    if (frameCount == 640) {
        CreateDustCircle(&cameoExtraSphere[1].pos, 5, 1.0f);
    }
    if (frameCount == 644) {
        CreateDustCircle(&cameoExtraSphere[1].pos, 3, 1.0f);
    }
    if (frameCount == 445) {
        FlashScreen(255, 128, 0, 200, 0);
        for (x = 0; x < 12; x++) {
            bullet = CreateNewBullet(FLAME_BULLET);
            bullet->actor.pos.z = 0.0f;
            bullet->actor.pos.x = -25.0f;
            bullet->actor.pos.y = -40.0f;
            bullet->actor.vel.x = Random(10) - 5;
            bullet->actor.vel.y = Random(4) + 7;
            bullet->actor.vel.z = Random(10) - 5;
            InitBulletSpritesOrMesh(bullet, 0, 0);
            bullet->sprites[0].anim.def.startScale = (Random(256) + 212) / 3;
            bullet->sprites[0].anim.def.endScale = (Random(56) + 256) / 3;
            bullet->sprites[1].anim.def.startScale = (Random(256) + 212) / 3;
            bullet->sprites[1].anim.def.endScale = (Random(56) + 256) / 3;
            AddBullet(bullet);
        }
    }
    if (frameCount > outroposplus && frameCount <= introDimEndFrame) {
        if (frameCount == introDimEndFrame) {
            backgroundLightList.ambient.l.col[0] = backgroundLightList.ambient.l.colc[0] = originalAmbientLight[0] = 110;
            backgroundLightList.ambient.l.col[1] = backgroundLightList.ambient.l.colc[1] = originalAmbientLight[1] = 110;
            backgroundLightList.ambient.l.col[2] = backgroundLightList.ambient.l.colc[2] = originalAmbientLight[2] = 150;
            for (light = backgroundLightList.lights.head.next, x = 0; light != &backgroundLightList.lights.head;
                 light = light->next, x++) {
                light->col[0] = light->col[3] = 160.0f;
                light->col[1] = light->col[4] = 160.0f;
                light->col[2] = light->col[5] = x * 60 + 160;
            }
        } else {
            backgroundLightList.ambient.l.col[0] = backgroundLightList.ambient.l.colc[0] = originalAmbientLight[0] =
                backgroundLightList.ambient.l.col[0] * startDarkFrame;
            backgroundLightList.ambient.l.col[1] = backgroundLightList.ambient.l.colc[1] = originalAmbientLight[1] =
                backgroundLightList.ambient.l.col[1] * startDarkFrame;
            backgroundLightList.ambient.l.col[2] = backgroundLightList.ambient.l.colc[2] = originalAmbientLight[2] =
                backgroundLightList.ambient.l.col[2] * startDarkFrame;
            for (light = backgroundLightList.lights.head.next; light != &backgroundLightList.lights.head; light = light->next) {
                light->col[0] = light->col[3] = light->col[0] * startDarkFrame;
                light->col[1] = light->col[4] = light->col[1] * startDarkFrame;
                light->col[2] = light->col[5] = light->col[2] * startDarkFrame;
            }
        }
    }
    if (TriggerAtFrame(831.0f, actor) || TriggerAtFrame(841.0f, actor) || TriggerAtFrame(851.0f, actor)) {
        introBullets[introBulletNum] = CreateNewBullet(BALL_SPELL_BULLET);
        introBullets[introBulletNum]->spell = -1;
        introBullets[introBulletNum]->spellType = -1;
        introBullets[introBulletNum]->actor.pos = cameoExtraSphere->pos;
        SUB_VECTORS(&introBullets[introBulletNum]->actor.vel, &cameoExtraSphere->pos, &cameoExtraSphere[1].pos);
        ADD_TO_VECTOR(&introBullets[introBulletNum]->actor.pos, &introBullets[introBulletNum]->actor.vel);
        AddBulletFlash(&introBullets[introBulletNum]->actor.pos);
        InitBulletSpritesOrMesh(introBullets[introBulletNum], 0, 0);
        introBullets[introBulletNum]->sprites[0].r = spellColour[0][0][0];
        introBullets[introBulletNum]->sprites[0].g = spellColour[0][0][1];
        introBullets[introBulletNum]->sprites[0].b = spellColour[0][0][2];
        AddBullet(introBullets[introBulletNum]);
        introBulletNum++;
    }
    for (x = 3; x < 10; x++) {
        if (frameCount & 1) {
            tempVect.x = cameoExtraSphere[x].pos.x + Random(19) - 9.0f;
            tempVect.y = cameoExtraSphere[x].pos.y + Random(19) - 9.0f;
            tempVect.z = cameoExtraSphere[x].pos.z + Random(19) - 9.0f;
            debris = AddDebris(FAIRY, &tempVect, &zero, NULL, NULL);
            if (debris != NULL && actor->anim->time > 373.0f && actor->anim->time < 424.0f) {
                debris->vel.y = 8.0f;
            }
        }
    }
}

void SetFadeColour(u8 r, u8 g, u8 b) {
    Mesh *m = fadeOverlay.inst->obj->mesh;
    u8 *c = m->colours;
    s32 i;

    for (i = 0; i < 4; i++) {
        c[0] = r;
        c[1] = g;
        c[2] = b;
        c += 4;
    }
}

void UpdateHiscoreTable(s32 score) {
    s32 pos = 7;
    s32 i;

    for (i = 0; i < 6; i++) {
        if (score >= hiscoreTable[i].score) {
            pos = i;
            break;
        }
    }
    if (pos < 7) {
        for (i = 5; i > pos; i--) {
            hiscoreTable[i] = hiscoreTable[i - 1];
        }
        hiscoreTable[pos].score = score;
        sprintf(hiscoreTable[pos].name, currentName);
        SaveHiscoreTable();
        SaveCRC();
    }
}

void SetSfxVols(void) {
    Buzzer *buzzer;
    Platform *platform;
    Vent *vent;
    Bullet *bullet;
    s32 i;
    s32 j;

    for (buzzer = buzzerList.head.next; buzzer != &buzzerList.head; buzzer = buzzer->next) {
        SET_SAMPLE_VOLUME(buzzer->sample)
    }
    for (platform = platformList.head.next; platform != &platformList.head; platform = platform->next) {
        SET_SAMPLE_VOLUME(platform->sample)
        if (platform->motion != NULL) {
            SET_SAMPLE_VOLUME(platform->motion->sample)
        }
        if (platform->orbit != NULL) {
            SET_SAMPLE_VOLUME(platform->orbit->sample)
        }
        if (platform->crumble != NULL) {
            SET_SAMPLE_VOLUME(platform->crumble->sample)
        }
    }
    for (i = 0; i < numberOfEnemies; i++) {
        for (j = 0; j < 2; j++) {
            SET_SAMPLE_VOLUME(enemies[i].samples[j])
        }
    }
    for (vent = ventList.head.next; vent != &ventList.head; vent = vent->next) {
        SET_SAMPLE_VOLUME(vent->sound)
    }
    for (bullet = bulletList.head.next; bullet != &bulletList.head; bullet = bullet->next) {
        SET_SAMPLE_VOLUME(bullet->sample)
    }
    for (i = 0; i < 2; i++) {
        SET_SAMPLE_VOLUME(handControl.samples[i])
        SET_SAMPLE_VOLUME(ballControl.samples[i])
    }
}

s32 IsTimeTrialLevel(s32 num) {
    s32 i;

    for (i = 1; i < 7; i++) {
        if (num >= worldInfo[i].firstLevel && num <= worldInfo[i].lastLevel) {
            return TRUE;
        }
    }
    return FALSE;
}

s32 IsLevelAvailable(s32 num) {
    s32 j;

    for (j = 0; j < 6; j++) {
        if (num == 10 || (saveSlot[j].levels >> levelInfo[num].bit) & 1) {
            return TRUE;
        }
    }
    return FALSE;
}

INCLUDE_ASM("asm/nonmatchings/game", pad_8012ACB8);

// zero padding at the end of game.o rodata, after the constants of DoIntroEffects
const u32 D_8010188C[] = { 0 };
