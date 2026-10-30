#ifndef SAVE_H
#define SAVE_H

#include "common.h"

typedef struct SaveSlot {
    /* 0x00 */ u8 name[3];
    /* 0x03 */ u8 unk03;
    /* 0x04 */ s32 levels;
    /* 0x08 */ s32 allGaribs;
    /* 0x0C */ s32 score;
    /* 0x10 */ u8 lives;
    /* 0x11 */ u8 difficultyLevel;
    /* 0x12 */ u8 hubTarget;
    /* 0x13 */ u8 hubLevel;
    /* 0x14 */ s32 visited;
    /* 0x18 */ s16 sfxVolume;
    /* 0x1A */ s16 musicVolume;
} SaveSlot; /* size = 0x1C */

extern SaveSlot saveSlot[6];

typedef struct HiscoreEntry {
    /* 0x00 */ s32 score;
    /* 0x04 */ char name[4];
} HiscoreEntry;

typedef struct BestTime {
    /* 0x00 */ s16 time;
    /* 0x02 */ char name[3];
    /* 0x05 */ u8 unk05;
} BestTime; /* size = 0x6 */


typedef struct DifficultyStats {
    /* 0x00 */ u8 lives;
    /* 0x01 */ u8 garibsPerLife;
    /* 0x02 */ s16 lifeScore;
} DifficultyStats;

extern DifficultyStats difficultyStats[];

#endif
