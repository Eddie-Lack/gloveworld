#ifndef GAME_H
#define GAME_H

#include "common.h"

typedef struct GameInfo {
    /* 0x00 */ s8 mode;
    /* 0x01 */ s8 level;
    /* 0x02 */ s8 prevLevel;
    /* 0x03 */ s8 world;
    /* 0x04 */ s8 prevWorld;
    /* 0x05 */ s8 worldComplete[0x7];
    /* 0x0C */ s16 fade;
    /* 0x0E */ s16 fade2;
    /* 0x10 */ s8 fadeStep;
    /* 0x11 */ s8 exitType;
    /* 0x12 */ s16 cameoActive;
    /* 0x14 */ char message[0x40];
    /* 0x54 */ u8 exitFlags;
    /* 0x55 */ u8 caveStage;
    /* 0x56 */ s16 language;
    /* 0x58 */ struct Platform *exitPlatform;
    /* 0x5C */ s16 starZ;
    /* 0x5E */ s16 starZTarget;
} GameInfo;

typedef struct Fog {
    /* 0x00 */ u8 r;
    /* 0x01 */ u8 g;
    /* 0x02 */ u8 b;
    /* 0x03 */ u8 on;
    /* 0x04 */ s16 min;
    /* 0x06 */ s16 max;
} Fog;

typedef struct Helper {
    /* 0x00 */ Vec3 pos;
    /* 0x0C */ Vec3 scale;
} Helper;

extern GameInfo gameInfo;
extern Helper helper;
extern Fog fog;

typedef struct LevelInfo {
    /* 0x00 */ char name[0x28];
    /* 0x28 */ s32 objBank;
    /* 0x2C */ s32 bit;
    /* 0x30 */ s32 sfxDist;
    /* 0x34 */ u8 unk34;
    /* 0x35 */ u8 song;
    /* 0x36 */ s16 bonusTime;
} LevelInfo; /* size = 0x38 */

extern LevelInfo levelInfo[];

void FreeBackdropList(void);
void FreeLevel(void);
void FreeLevel2(s32 full);
void LoadLevel(s32 num);
void LevelSelectScreen(void);
void TimeTrialScreen(void);
void StartNewGame(void);
void ContinueGame(void);
void InitLinkedLists(void);
void InitLinkedListsNoBackdrops(void);
void InitFadeOverlay(void);
void EndOfLevelScreen(void);
void EndOfSubLevelScreen(void);
void EndOfWorldScreen(void);
void StartCurrentLevel(void);
void NewWorldScreen(void);
void UpdateFade(void);
void CheckForExits(void);
s32 LevelToWorld(s8 num);
s32 GameBeingPlayed(void);
void TitleScreenCameraSpin(void);
void TitleScreenCameraSpin2(void);
void GameOverScreen(void);
void DoCaveStuff(void);
void UpdatePause(void);
s32 CheckForLevelCompleted(void);
void SelectLanguage(void);
s32 BonusOpen(void);
void ExitLevel(s32 type, s32 pause, s32 complete);
s32 IsBonusLevel(void);
void DoOutroEffects(void);
void DoIntroEffects(void);
void SetFadeColour(u8 r, u8 g, u8 b);
void UpdateHiscoreTable(s32 score);
void SetSfxVols(void);
s32 IsTimeTrialLevel(s32 num);
s32 IsLevelAvailable(s32 num);

#endif
