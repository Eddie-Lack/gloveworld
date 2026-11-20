#ifndef BULLET_H
#define BULLET_H

#include "common.h"
#include "actor.h"
#include "overlay.h"
#include "sound.h"

typedef struct Bullet {
    /* 0x000 */ struct Bullet *next;
    /* 0x004 */ struct Bullet *prev;
    /* 0x008 */ Actor actor;
    /* 0x0EC */ GameSprite sprites[2];
    /* 0x15C */ s16 age;
    /* 0x15E */ s16 spellTime;
    /* 0x160 */ u8 targetFlags;
    /* 0x161 */ u8 type;
    /* 0x162 */ s8 spell;
    /* 0x163 */ s8 spellType;
    /* 0x164 */ Actor *target;
    /* 0x168 */ ContinuousSample sample;
} Bullet; /* size = 0x184 */

typedef struct BulletInfo {
    /* 0x00 */ char name[0x10];
    /* 0x10 */ ObjectInstance *inst;
    /* 0x14 */ s16 numSprites;
    /* 0x16 */ u8 unk16[0x2];
    /* 0x18 */ struct {
        /* 0x00 */ SpriteAnim *anim;
        /* 0x04 */ u8 r;
        /* 0x05 */ u8 g;
        /* 0x06 */ u8 b;
        /* 0x07 */ u8 pad;
    } sprites[2];
    /* 0x28 */ Vec3 vel[4];
    /* 0x58 */ Vec3 offset[4];
    /* 0x88 */ f32 accel;
    /* 0x8C */ f32 turn;
    /* 0x90 */ u8 trailDebris;
    /* 0x91 */ u8 deathDebris;
    /* 0x92 */ u8 numShots;
    /* 0x93 */ u8 unk93;
    /* 0x94 */ f32 mass;
    /* 0x98 */ u8 unk98[0x4];
    /* 0x9C */ s16 life;
    /* 0x9E */ s16 targetDelay;
    /* 0xA0 */ u8 targetFlags;
    /* 0xA1 */ u8 unkA1[0x3];
    /* 0xA4 */ s32 flags;
    /* 0xA8 */ f32 maxSpeed;
    /* 0xAC */ f32 friction;
    /* 0xB0 */ f32 radius;
    /* 0xB4 */ s32 flags2;
    /* 0xB8 */ void (*hitFunc)(Actor *actor);
    /* 0xBC */ s32 (*maintainFunc)(struct Bullet *bullet);
} BulletInfo; /* size = 0xC0 */

extern BulletInfo standardBullets[];

typedef struct BulletList {
    /* 0x000 */ Bullet head;
    /* 0x184 */ s32 count;
} BulletList;

extern BulletList bulletList;

Bullet *CreateNewBullet(s16 type);
void InitBulletSpritesOrMesh(Bullet *bullet, Actor *actor, s16 num);
void AddBullet(Bullet *bullet);

void InitBulletList(void);
void SubBullet(Bullet *bullet);
void DestroyBulletMissile(Actor *actor);
void UpdateBullets(void);
void BulletHitTarget(Bullet *bullet, Actor *target);

#endif
