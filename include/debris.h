#ifndef DEBRIS_H
#define DEBRIS_H

#include "common.h"
#include "overlay.h"
#include "actor.h"

typedef struct Debris {
    /* 0x000 */ u8 unk000[0x20];
    /* 0x020 */ s16 actorType;
    /* 0x022 */ s8 actorActive;
    /* 0x023 */ u8 unk023;
    /* 0x024 */ s16 lifeTimer;
    /* 0x026 */ s16 alpha;
    /* 0x028 */ s32 flags;
    /* 0x02C */ struct Actor *owner;
    /* 0x030 */ f32 radius;
    /* 0x034 */ Vec3 pos;
    /* 0x040 */ Vec3 prevPos;
    /* 0x04C */ u8 unk04C[0xC];
    /* 0x058 */ Vec3 vel;
    /* 0x064 */ u8 unk064[0xC];
    /* 0x070 */ f32 friction;
    /* 0x074 */ f32 unk074;
    /* 0x078 */ Vec3 rot;
    /* 0x084 */ u8 unk084[0xC];
    /* 0x090 */ Vec3 rotVel;
    /* 0x09C */ Quat rotQuat;
    /* 0x0AC */ u8 unk0AC[0x10];
    /* 0x0BC */ Vec3 scale;
    /* 0x0C8 */ void *inst;
    /* 0x0CC */ struct ActorShadow *shadow;
    /* 0x0D0 */ ActorPhysics *info;
    /* 0x0D4 */ ActorState *state;
    /* 0x0D8 */ ActorObject *obj;
    /* 0x0DC */ u8 unk0DC[0x8];
    /* 0x0E4 */ u8 active;
    /* 0x0E5 */ u8 type;
    /* 0x0E6 */ u16 life;
    /* 0x0E8 */ GameSprite sprite;
} Debris;

Debris *AddDebris(s32 debrisType, Vec3 *position, Vec3 *velocity, GameSprite *after, struct Debris *newDeb);
void CreateDustCircleScaled(Vec3 *pos, s32 numStars, f32 verticalVelocity, f32 scale);
void CreateStarCircle(Vec3 *pos, s32 numStars, s16 type);
ScreenDebris *AddScreenDebris(s32 xPos, s32 yPos, f32 xVel, f32 yVel, s32 type, s32 mustCreate);

void InitDebris(void);
void KillDebris(Debris *index);
void UpdateDebris(void);
void CreateDustCircle(Vec3 *pos, s32 numStars, f32 verticalVelocity);
void CreateSplashCircle(Vec3 *pos, s32 numStars, f32 verticalVelocity);
void CreatePuffCircle(Vec3 *pos);
void CreateBigDustCircle(Vec3 *pos, f32 Yval);

#endif
