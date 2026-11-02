#ifndef LIGHT_H
#define LIGHT_H

#include "types.h"

typedef struct GameLight {
    /* 0x00 */ struct GameLight *next;
    /* 0x04 */ struct GameLight *prev;
    /* 0x08 */ u8 index;
    /* 0x09 */ u8 type;
    /* 0x0A */ s16 timer;
    /* 0x0C */ Vec3 dir;
    /* 0x18 */ f32 spin[2];
    /* 0x20 */ f32 col[6];
    /* 0x38 */ f32 colourSpeed;
    /* 0x3C */ f32 angle[2];
    /* 0x44 */ f32 target[2];
    /* 0x4C */ f32 origX;
    /* 0x50 */ f32 origY;
    /* 0x54 */ f32 turnSpeed;
} GameLight; /* size = 0x58 */

typedef struct LightList {
    /* 0x00 */ GameLight head;
    /* 0x58 */ s16 count;
    /* 0x5A */ s16 numSkipped;
} LightList;

typedef struct Lightning {
    /* 0x00 */ Vec3 from;
    /* 0x0C */ Vec3 to;
    /* 0x18 */ u8 r;
    /* 0x19 */ u8 g;
    /* 0x1A */ u8 b;
    /* 0x1B */ u8 segments;
    /* 0x1C */ s16 timer;
    /* 0x1E */ s16 flags;
    /* 0x20 */ f32 jitter;
    /* 0x24 */ f32 width;
} Lightning; /* size = 0x28 */

typedef struct BackgroundLighting {
    /* 0x00 */ Ambient ambient;
    /* 0x08 */ LightList lights;
} BackgroundLighting;

extern BackgroundLighting backgroundLightList;
extern LightList handLightList;

void AddLightning(Vec3 *start, Vec3 *end, u8 r, u8 g, u8 b, u8 segments, s16 lifetime, s16 flags, f32 jagginess,
                  f32 thickness);

void InitBackgroundLightList(void);
void InitHandLightList(void);
void SetupAmbientLight(Ambient *a, u8 r, u8 g, u8 b);
void UpdateLights(void);
void UpdateLightDirections(GameLight *light);
void UpdateLightColours(GameLight *ptr);
void UpdateLightning(void);
void DrawLightning(Vec3 *start, Vec3 *end, u8 r, u8 g, u8 b, s16 numSegments, s16 flags, f32 jagginess, f32 thickness);

#endif
