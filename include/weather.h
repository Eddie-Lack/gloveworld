#ifndef WEATHER_H
#define WEATHER_H

#include "common.h"

typedef struct Weather {
    /* 0x00 */ s16 rate;
    /* 0x02 */ s16 count;
    /* 0x04 */ s16 duration;
    /* 0x06 */ s16 type;
    /* 0x08 */ s32 accum;
    /* 0x0C */ u8 col0;
    /* 0x0D */ u8 col1;
    /* 0x0E */ u8 col2;
    /* 0x0F */ u8 lightning;
    /* 0x10 */ s8 velX;
    /* 0x11 */ s8 velY;
    /* 0x12 */ s8 spin;
} Weather;

extern Weather rain;

void ProcessRain(void);
void StartWeather(s16 type, s16 rate, s16 count, u8 col0, u8 col1, u8 col2, s8 velX, s8 velY, s8 spin, s16 duration);
void AddPuff(Vec3 *pos, Vec3 *vel, f32 scale);

#endif
