#ifndef QUAKE_H
#define QUAKE_H

#include "common.h"

typedef struct Earthquake {
    /* 0x00 */ s32 time;
    /* 0x04 */ s16 start;
    /* 0x06 */ s16 end;
    /* 0x08 */ s32 phase;
    /* 0x0C */ f32 speed;
    /* 0x10 */ s32 flags;
} Earthquake;

extern Earthquake earthQuake;

void StartEarthquake(s16 duration, s16 maxStrength, s16 minStrength, s16 numCycles, s32 flags);
void StartEarthquakeAtPoint(Vec3 *pos, s16 duration, f32 maxStrength, f32 minStrength, s16 numCycles, s32 flags);
void DoEarthquake(void);
void DoExplosion(Vec3 *centre, f32 radius1, f32 radius2, Enemy *actor, f32 maxSpeed, f32 scale);

#endif
