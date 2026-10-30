#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "common.h"

typedef struct Pad {
    /* 0x000 */ s16 stickX;
    /* 0x002 */ s16 stickY;
    /* 0x004 */ f32 mag;
    /* 0x008 */ f32 angle[4];
    /* 0x018 */ u8 unk018[0x2];
    /* 0x01A */ u8 button[14];
    /* 0x028 */ u8 prevButton[14];
    /* 0x036 */ u8 pressed[14];
    /* 0x044 */ u8 unk044[0x1C];
    /* 0x060 */ u8 latch[14];
    /* 0x06E */ u8 unk06E[0x2];
    /* 0x070 */ s32 holdTime[14];
    /* 0x0A8 */ s32 lastHoldTime[14];
    /* 0x0E0 */ s32 sincePress[14];
    /* 0x118 */ s32 present;
    /* 0x11C */ u8 unk11C[0x6C];
    /* 0x188 */ f32 prevMag;
    /* 0x18C */ f32 prevMag2;
} Pad; /* size = 0x190 */

extern Pad pad[4];

s32 initControllers(void);
void ActivateCameraControlMode(void);
void DeactivateCameraControlMode(void);
void ProcessInput(void);
s32 AnyButtonPressed(u8 padNum);
s32 AnyDebouncePressed(u8 padNum);

#endif
