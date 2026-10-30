#ifndef BUZZER_H
#define BUZZER_H

#include "types.h"
#include "sound.h"

typedef struct Buzzer {
    /* 0x00 */ struct Buzzer *next;
    /* 0x04 */ struct Buzzer *prev;
    /* 0x08 */ Vec3 p1;
    /* 0x14 */ Vec3 p2;
    /* 0x20 */ struct Platform *plat1;
    /* 0x24 */ struct Platform *plat2;
    /* 0x28 */ s16 flags;
    /* 0x2A */ s16 tag;
    /* 0x2C */ u8 r;
    /* 0x2D */ u8 g;
    /* 0x2E */ u8 b;
    /* 0x2F */ u8 a;
    /* 0x30 */ u8 active;
    /* 0x31 */ u8 buzzing;
    /* 0x32 */ s16 numTimes;
    /* 0x34 */ s16 onTime[6];
    /* 0x40 */ s16 offTime[6];
    /* 0x4C */ s16 timer;
    /* 0x4E */ s16 index;
    /* 0x50 */ f32 jitter;
    /* 0x54 */ f32 width;
    /* 0x58 */ ContinuousSample sample;
    /* 0x74 */ Vec3 mid;
} Buzzer; /* size = 0x80 */

typedef struct BuzzerList {
    /* 0x00 */ Buzzer head;
    /* 0x80 */ s32 count;
} BuzzerList;

extern BuzzerList buzzerList;

void InitBuzzerList(void);
void AddBuzzer(Buzzer *buzzer);
Buzzer *FindTaggedBuzzer(Buzzer *startBuzzer, s32 tag);

void UpdateBuzzers(void);
void UpdateBuzzer(Buzzer *buzzer);

#endif
