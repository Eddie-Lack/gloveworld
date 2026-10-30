#ifndef TYPES_H
#define TYPES_H

#include "ultra64.h"

typedef struct Vec3 {
    f32 x;
    f32 y;
    f32 z;
} Vec3;

typedef struct Quat {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Quat;

typedef struct Box {
    Vec3 pos;
    Vec3 size;
} Box;

#endif
