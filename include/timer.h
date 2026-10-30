#ifndef TIMER_H
#define TIMER_H

#include "common.h"

#define NUM_TIMERS 10
#define TIMER_SAMPLES 10

typedef struct Timer {
    /* 0x00 */ s32 running;
    /* 0x04 */ u32 start[TIMER_SAMPLES];
    /* 0x2C */ u32 end[TIMER_SAMPLES];
    /* 0x54 */ u32 total[TIMER_SAMPLES];
    /* 0x7C */ u32 frame;
    /* 0x80 */ char name[31];
    /* 0x9F */ u8 count;
} Timer; /* size = 0xA0 */

extern Timer timers[NUM_TIMERS];
extern Timer printTimer[NUM_TIMERS];
extern u8 timerMode;

void InitTimers(void);
void ClearTimers(void);
void StartTimer(s32 id, char *name);
void EndTimer(s32 id);
void PrintTimers(void);

#endif
