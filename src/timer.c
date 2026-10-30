#include "common.h"
#include "ident.h"
#include "timer.h"

extern u32 frameStart;
extern u32 TicksPerFrame;
extern Gfx *glistp;

INCLUDE_ASM("asm/nonmatchings/timer", pad_801802D8);

void InitTimers(void) {
    s32 i;
    s32 j;

    for (i = 0; i < NUM_TIMERS; i++) {
        timers[i].running = FALSE;
        for (j = 0; j < TIMER_SAMPLES; j++) {
            timers[i].start[j] = 0;
            timers[i].end[j] = 0;
            timers[i].total[j] = 0;
        }
        timers[i].name[0] = '\0';
        timers[i].count = 0;
    }
}

void ClearTimers(void) {
    s32 i;
    s32 j;
    u8 busy;

    busy = FALSE;
    for (i = 0; i < NUM_TIMERS; i++) {
        if (!timers[i].running) {
            printTimer[i] = timers[i];
        } else {
            busy = TRUE;
        }
    }
    if (busy != TRUE) {
        for (i = 0; i < NUM_TIMERS; i++) {
            timers[i].count = 0;
            timers[i].name[0] = '\0';
            timers[i].frame = frameStart;
            for (j = 0; j < TIMER_SAMPLES; j++) {
                timers[i].start[j] = 0;
                timers[i].end[j] = 0;
                timers[i].total[j] = 0;
            }
        }
    }
}

void StartTimer(s32 id, char *name) {
    s32 n = timers[id].count;

    if (n < TIMER_SAMPLES) {
        timers[id].start[n] = osGetCount();
        timers[id].running = TRUE;
        sprintf(timers[id].name, name);
    }
}

void EndTimer(s32 id) {
    s32 n = timers[id].count;
    u32 now;

    if (n < TIMER_SAMPLES) {
        now = osGetCount();
        timers[id].end[n] = now;
        if (timers[id].start[n] < now) {
            timers[id].total[n] += now - timers[id].start[n];
        } else {
            timers[id].total[n] += ~timers[id].start[n] + now;
        }
        timers[id].running = FALSE;
        if (n < TIMER_SAMPLES) {
            timers[id].count++;
        }
    }
}

void PrintTimers(void) {
    s32 i;
    s32 j;
    s32 x;
    u32 len;
    s32 top;
    s32 bottom;
    u32 max = 0xFFFFFFFF;
    s32 left;

    if (timerMode) {
        gDPPipeSync(glistp++);
        gDPSetRenderMode(glistp++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
        gDPPipeSync(glistp++);
        gDPSetCycleType(glistp++, G_CYC_FILL);
        gDPSetFillColor(glistp++, max);
        gDPFillRectangle(glistp++, 100, 30, 101, 195);
        gDPFillRectangle(glistp++, TicksPerFrame / 10000 + 100, 30, TicksPerFrame / 10000 + 101, 195);
        gDPFillRectangle(glistp++, TicksPerFrame / 20000 + 100, 30, TicksPerFrame / 20000 + 101, 195);
        gDPFillRectangle(glistp++, TicksPerFrame + 100, 30, TicksPerFrame + 101, 195);
        left = 100;
        top = 30;
        bottom = 35;
        for (i = 0; i < NUM_TIMERS; i++) {
            for (j = 0; j < printTimer[i].count; j++) {
                if (printTimer[i].start[j] == 0) {
                    continue;
                }
                gDPPipeSync(glistp++);
                switch (j & 3) {
                    case 0:
                        gDPSetFillColor(glistp++, 0xF001F001);
                        break;
                    case 1:
                        gDPSetFillColor(glistp++, 0x07810781);
                        break;
                    case 2:
                        gDPSetFillColor(glistp++, 0x003D003D);
                        break;
                    case 3:
                        gDPSetFillColor(glistp++, 0xF783F783);
                        break;
                }
                len = printTimer[i].total[j] / 20000;
                if (printTimer[i].start[j] > printTimer[i].frame) {
                    x = (printTimer[i].start[j] - printTimer[i].frame) / 20000;
                } else {
                    x = (max - printTimer[i].frame + printTimer[i].start[j]) / 20000;
                }
                len += 100;
                if (timerMode) {
                    gDPFillRectangle(glistp++, left + x, top, len + x, bottom);
                }
            }
            top += 16;
            bottom += 16;
        }
        gDPPipeSync(glistp++);
        gDPSetCycleType(glistp++, G_CYC_1CYCLE);
        gDPPipeSync(glistp++);
        gDPSetRenderMode(glistp++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
    }
}

INCLUDE_ASM("asm/nonmatchings/timer", pad_80180A7C);

INCLUDE_RODATA("asm/nonmatchings/timer", pad_801089C4);
