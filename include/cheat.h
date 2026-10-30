#ifndef CHEAT_H
#define CHEAT_H

#include "common.h"

typedef struct CheatInfo {
    /* 0x00 */ char msg[0x20];
    /* 0x20 */ u8 code[8];
    /* 0x28 */ s32 type;
} CheatInfo; /* size = 0x2C */

typedef struct CheatSample {
    /* 0x00 */ s16 id;
    /* 0x02 */ s16 pitch;
} CheatSample;

extern CheatInfo cheatInfo[];
extern CheatSample cheatSampleData[];

s32 CheckForCheats(void);
s32 FindCheatButton(void);
s8 CompareCheatString(void);
void UpdateCheatText(void);
void PrintCheatString(void);
s32 CheatAllowed(s32 cheat);

void ActivateCheat(u8 num, u8 show);

#endif
