#include "common.h"
#include "maths.h"
#include "debris.h"
#include "weather.h"

void FlashScreen(u8 r, u8 g, u8 b, s16 fadeVal, s16 fadeStep);

extern s32 frameCount;
extern u8 rainLayer;
extern u8 puffCol[][3];

void ProcessRain(void) {
    s16 x;
    ScreenDebris *debris;

    if ((frameCount & 1) && rain.lightning) {
        rain.lightning--;
        if (Random(10) & 1) {
            FlashScreen(220, 220, 255, 230, -30);
        } else {
            FlashScreen(0, 0, 0, 230, -30);
        }
    }
    if (rain.duration == 0) {
        return;
    }
    if (rain.duration > 0) {
        rain.duration--;
    }
    if (rain.type == 2 && rain.lightning == 0 && Random(2000) < 5) {
        rain.lightning = Random(5) + 5;
        PlaySampleNot3D(248, Random(40) + 80, 128, Random(56) + 100);
    }
    rain.accum += rain.rate;
    while (rain.accum > 64) {
        for (x = 0; x < rain.count; x++) {
            debris = AddScreenDebris(Random(350), 0, rain.velX, rain.velY, rain.type, 0);
            if (debris == NULL) {
                rain.accum %= 64;
                return;
            }
            if (rainLayer == 0) {
                debris->curScale = debris->targetScale = 0x400;
                debris->curAlpha = debris->targetAlpha = rain.col0;
            }
            if (rainLayer == 1) {
                debris->curScale = debris->targetScale = 0x300;
                debris->curAlpha = debris->targetAlpha = rain.col1;
                debris->velX *= 0.75;
                debris->velY *= 0.75;
            }
            if (rainLayer == 2) {
                debris->curScale = debris->targetScale = 0x200;
                debris->curAlpha = debris->targetAlpha = rain.col2;
                debris->velX *= 0.5;
                debris->velY *= 0.5;
            }
            debris->lifeTimer = rain.spin + Random(2);
            if (++rainLayer == 3) {
                rainLayer = 0;
            }
        }
        rain.accum -= 64;
    }
}

void StartWeather(s16 type, s16 rate, s16 count, u8 col0, u8 col1, u8 col2, s8 velX, s8 velY, s8 spin, s16 duration) {
    rain.type = type;
    rain.rate = rate;
    rain.count = count;
    rain.col0 = col0;
    rain.col1 = col1;
    rain.col2 = col2;
    rain.velX = velX;
    rain.velY = velY;
    rain.spin = spin;
    rain.duration = duration;
}

void AddPuff(Vec3 *pos, Vec3 *vel, f32 scale) {
    Debris *d;
    s32 x;

    d = AddDebris(PUFF, pos, vel, 0, 0);
    if (d != NULL) {
        x = Random(5) & 0xFF;
        d->sprite.r = puffCol[x][0];
        d->sprite.g = puffCol[x][1];
        d->sprite.b = puffCol[x][2];
        d->sprite.scaleY = 0;
        d->sprite.scaleX = 0;
        d->sprite.anim.def.startScale *= scale;
        d->sprite.anim.def.endScale *= scale;
    }
}

INCLUDE_ASM("asm/nonmatchings/weather", pad_801BB704);
