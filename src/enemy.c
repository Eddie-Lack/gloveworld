#include "common.h"
#include "ident.h"
#include "actor.h"
#include "debug.h"
#include "maths.h"
#include "hand.h"
#include "ball.h"
#include "camera.h"
#include "debris.h"
#include "game.h"
#include "collect.h"
#include "bullet.h"
#include "texture.h"
#include "light.h"
#include "banks.h"
#include "jalloc.h"
#include "overlay.h"
#include "sound.h"
#include "enemy.h"
#include "anim.h"
#include "frontend.h"
#include "nmeprocess.h"
#include "object.h"
#include "spell.h"
#include "nmeprocess2.h"

extern s16 farPlaneDist;

void AnimateActor(Actor *actor, s32 animNum, s32 loop, s32 queue, f32 speed);
Bullet *FireBulletFromActor(Actor *actor, s32 type, s32 flash);
u32 UpdateCRC(char *ptr);
void FlashScreen(u8 r, u8 g, u8 b, s16 fadeVal, s16 fadeStep);
extern ObjectInstance *frogObj;
f32 Altitude(Vec3 *pos, Vec3 **floor, s32 *plat, ActorObject *coll);
extern s32 controlMethod;
void NMEGetNewInstruction(Enemy *enemy, s32 instructionNum, s32 overrideConditions);
void DropHand(s32 timer);
void ActivateSpell(Actor *actor, s32 spell, s32 duration);
void InitSpriteAnimation(GameSprite *sprite, SpriteAnim *temp, s16 lifespan);
void PrepareSong(u8 song);
extern GaribGroup *NMEGaribGroup;
extern SpriteAnim garibAnimation[];
extern s32 cheatActivated;
extern Quat zeroQuat;
extern f32 bombLife;
extern u8 deathR;
extern u8 deathG;
extern u8 deathB;
extern u8 deathSegs;
extern u8 deathLife;
extern u8 deathRandom;
extern f32 deathJag;
extern f32 deathThick;

extern u8 uniqueEnemyCount[0x3B];
extern char *nmeObjectName[];
extern f32 starfreq;
extern s32 frameCount;
extern Vec3 zero;

INCLUDE_RODATA("asm/nonmatchings/enemy", D_80109A74);

extern SpriteAnim bulletAnimation[];

// The parameter is copied to obj and then reused for the new object. This gives the
// register copies where MakeUniqueActor inlines this.
inline Object *MakeUniqueObject(Object *object) {
    Object *obj = object;
    s32 i;

    object = JallocAlloc(sizeof(Object), 1, "UniqAct");
    *object = *obj;
    if (obj->numSprites) {
        for (i = 0; i < obj->numSprites; i++) {
            ObjectSprite **sprites = &object->sprites;
            ObjectSprite *tempSpr;

            sprites += i;
            tempSpr = JallocAlloc(sizeof(ObjectSprite), 1, "UniqSpr");
            *tempSpr = **sprites;
            *sprites = tempSpr;
        }
    }
    if (object->child) {
        object->child = MakeUniqueObject(object->child);
    }
    if (object->sibling) {
        object->sibling = MakeUniqueObject(object->sibling);
    }
    return object;
}

void ClearUniqueEnemyList(void) {
    s32 i;

    for (i = 0; i < 0x3B; i++) {
        uniqueEnemyCount[i] = 0;
    }
}

void MakeUniqueActor(Actor *actor, s32 type) {
    ObjectInstance *objCont;

    if (uniqueEnemyCount[type]) {
        objCont = actor->inst;
        actor->inst = JallocAlloc(sizeof(ObjectInstance), 1, "UniqObjC");
        *actor->inst = *objCont;
        actor->inst->obj = MakeUniqueObject(actor->inst->obj);
        PRINTF("Found duplicate enemy %s\n", nmeObjectName[type]);
    }
    uniqueEnemyCount[type]++;
}

inline void RemoveUniqueObject(Object *object) {
    Object *o = object;
    s32 i;

    if (object->numSprites) {
        for (i = 0; i < object->numSprites; i++) {
            ObjectSprite **sprites = &object->sprites;

            sprites += i;
            JallocFree(&(*sprites)->sprite);
            JallocFree(sprites);
        }
    }
    if (object->child) {
        RemoveUniqueObject(object->child);
    }
    if (object->sibling) {
        RemoveUniqueObject(object->sibling);
    }
    JallocFree(&o);
}

inline void RemoveUniqueActor(Actor *actor, s32 type) {
    if (actor->inst && actor->inst->obj) {
        RemoveUniqueObject(actor->inst->obj);
        JallocFree(&actor->inst);
        uniqueEnemyCount[type]--;
    }
}

void LoadEnemy(s32 enemyType, f32 xpos, f32 ypos, f32 zpos, s32 tag, f32 initYRot) {
    Enemy *enemy = &enemies[numberOfEnemies];

    memclear(enemy, sizeof(Actor));
    memclear(enemy, sizeof(Enemy));
    InitActor(&enemy->actor, nmeObjectName[enemyType], xpos, ypos, zpos, 30);
    enemy->actor.active = 0;
    GetQuaternionFromXYZRot(&enemy->actor.rotQuat, &enemy->actor.rot);
    enemy->actor.type = enemyType;
    enemy->actor.state->index = numberOfEnemies++;
    MakeUniqueActor(&enemy->actor, enemyType);
    enemy->brain.attackCount = enemy->brain.conditionalCount = enemy->brain.normalCount = 0;
    RestartEnemy(enemy, xpos, ypos, zpos, tag, initYRot, enemyType);
}

void RestartEnemy(Enemy *enemy, f32 xpos, f32 ypos, f32 zpos, s32 tag, f32 initYRot, s32 enemyType) {
    Actor *actor = &enemy->actor;
    s32 platform;
    s32 i;

    if (enemy->actor.type == SAMTEX) {
        if (enemy->samples[0].handle) {
            StopContinuousSample(&enemy->samples[0]);
        }
        if (enemy->samples[1].handle) {
            StopContinuousSample(&enemy->samples[1]);
        }
    }
    enemy->brain.flags &= ~0x400;
    if (enemy->brain.flags & 0x10) {
        enemy->brain.spellTimer[2] = 1;
        ZERO_VECTOR(&enemy->actor.scale);
        NMEProcessFroggy(enemy);
    }
    for (i = 0; i < 3; i++) {
        enemy->brain.spellTimer[i] = 0;
    }
    actor->scale.x = actor->scale.y = actor->scale.z = enemyStats[enemyType].scale;
    actor->phys = &defaultBehaviour[enemyType];
    actor->pos.x = enemy->brain.startPos.x = xpos;
    actor->pos.y = enemy->brain.startPos.y = ypos;
    actor->pos.z = enemy->brain.startPos.z = zpos;
    enemy->brain.target = actor->pos;
    enemy->brain.tag = tag;
    enemy->brain.startRot = initYRot;
    if (initYRot == -1.0f && !enemy->actor.active) {
        actor->rot.y = actor->rotAim.y = 0.0f;
    } else {
        actor->active = 1;
        InitEnemy(enemy, enemyType);
        actor->rot.y = actor->rotAim.y = initYRot;
    }
    enemy->brain.instrIndex = 0;
    enemy->brain.attackTime = 0;
    enemy->brain.curList = enemy->brain.normalList;
    enemy->brain.inConditional = 0;
    if (frameCount) {
        NMEGetNewInstruction(enemy, 0, 1);
    }
    if (defaultBehaviour[enemy->actor.type].flags & 0x200000) {
        enemy->actor.flags |= 0x200000;
        enemy->actor.shadow->altitude = Altitude(&enemy->actor.pos, &enemy->actor.shadow->floor, &platform, enemy->actor.obj);
    }
}

void InitEnemy(Enemy *enemy, s32 type) {
    Actor *actor = &enemy->actor;
    char name[16];
    Object *tempObj;
    s32 j;

    enemy->brain.fireState[0] = 1;
    enemy->brain.fireState[1] = 0;
    enemy->brain.bank = -0.8f;
    actor->friction = MAX(0.0f, actor->phys->friction);
    actor->flags = actor->phys->flags & ~0x100;
    actor->obj->spheres[0].radius = actor->obj->spheres[0].targetRadius = actor->phys->radius;
    actor->shadow->shadowSize = actor->obj->spheres[0].radius * 0.8;
    actor->shadow->altitude = -1.0f;
    if (enemy->brain.startRot != -1.0f) {
        AddActor(actor);
    }
    if (!(actor->flags & 1)) {
        actor->state->flying = 1;
    } else {
        actor->state->flying = 0;
    }
    enemy->brain.minAlt = 1.0f;
    enemy->brain.maxAlt = actor->obj->spheres[0].targetRadius + 5.0f;
    enemy->brain.scratchF = 0.0f;
    enemy->brain.scratch = 0;
    if (enemy->brain.flags & 0x80) {
        enemy->brain.flags = enemyStats[type].flags | 0x80;
    } else {
        enemy->brain.flags = enemyStats[type].flags;
    }
    if (enemy->samples[0].handle) {
        StopContinuousSample(&enemy->samples[0]);
    }
    if (enemy->samples[1].handle) {
        StopContinuousSample(&enemy->samples[1]);
    }
    enemy->actor.shadow->sphere = 0;
    ZERO_VECTOR(&enemy->actor.vel);
    switch (type) {
        case 0x13:
            enemy->brain.maxAlt = actor->obj->spheres[0].targetRadius + 11.0f;
            actor->shadow->shadowSize = 10.0f;
            actor->obj->spheres[0].pos.y = -6.0f;
            actor->obj->spheres[0].flags &= ~0x20;
            actor->obj->spheres[1].flags |= 0x1C;
            actor->obj->spheres[1].flags &= ~0x20;
            actor->obj->spheres[1].pos = actor->pos;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("ballheart"))->sphere = &actor->obj->spheres[1];
            break;
        case 0x1E:
            actor->obj->spheres[0].flags &= ~0x20;
            actor->obj->spheres[0].pos.y = -10.0f;
            enemy->brain.maxAlt += 10.0f;
            InitSpriteAnimation(FindSubObjectFromObject(actor->inst->obj, UpdateCRC("egg1"))->sprites->sprite, &bulletAnimation[4], 0);
            InitSpriteAnimation(FindSubObjectFromObject(actor->inst->obj, UpdateCRC("egg2"))->sprites->sprite, &bulletAnimation[4], 0);
            InitSpriteAnimation(FindSubObjectFromObject(actor->inst->obj, UpdateCRC("egg3"))->sprites->sprite, &bulletAnimation[4], 0);
            InitSpriteAnimation(FindSubObjectFromObject(actor->inst->obj, UpdateCRC("egg4"))->sprites->sprite, &bulletAnimation[4], 0);
            break;
        case 0x15:
            actor->shadow->shadowSize = 10.0f;
            break;
        case 0x16:
            if (actor->active == 1) {
                PlayContinuousSample(&enemy->samples[0], 35, 100, &enemy->actor.pos, 128);
            }
            enemy->brain.bobPeriod = 5;
            enemy->brain.bobAmount = 0.2f;
            break;
        case 0x7:
            if (actor->active == 1) {
                PlayContinuousSample(&enemy->samples[0], 101, 80, &enemy->actor.pos, 40);
            }
            enemy->brain.bobPeriod = 20;
            enemy->brain.bobAmount = 0.05f;
            enemy->brain.bank = -0.5f;
            break;
        case 0x10:
            actor->obj->numSpheres = 2;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("swishblade"))->sphere = &actor->obj->spheres[1];
            actor->obj->spheres[1].flags = 0x14;
            actor->obj->spheres[1].targetRadius = actor->obj->spheres[1].radius = 5.0f;
            actor->obj->spheres[0].flags = 0x28;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("swishboty"))->sphere = &actor->obj->spheres[2];
            break;
        case 0x9:
            AnimateActor(&enemy->actor, 5, 0, 0, 0);
            actor->obj->spheres[0].pos.y = -5.0f;
            actor->obj->spheres[0].flags &= ~0x20;
            enemy->brain.maxAlt = actor->obj->spheres[0].targetRadius + 10.0f;
            actor->obj->spheres[1].pos.x = 0.0f;
            actor->obj->spheres[1].pos.y = 10.0f;
            actor->obj->spheres[1].pos.z = 0.0f;
            actor->obj->spheres[1].targetRadius = actor->obj->spheres[1].radius = 10.0f;
            actor->obj->spheres[1].flags &= ~0x20;
            actor->obj->numSpheres = 2;
            break;
        case 0x1B:
            actor->obj->spheres[1].radius = actor->obj->spheres[1].targetRadius = 1.0f;
            actor->obj->spheres[1].flags |= 0x17;
            actor->obj->spheres[1].flags &= ~0x20;
            actor->obj->spheres[1].pos = actor->pos;
            actor->obj->numSpheres = 2;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("SukFan"))->sphere = &actor->obj->spheres[1];
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("SukWhist"))->sphere = &actor->obj->spheres[2];
            enemy->brain.maxAlt = actor->obj->spheres[0].targetRadius + 20.0f;
            break;
        case 0x1A:
            enemy->brain.bank = -1.5f;
            actor->obj->spheres[0].pos.y = -10.0f;
            actor->obj->spheres[0].flags &= ~0x20;
            enemy->brain.maxAlt = actor->obj->spheres[0].targetRadius + 15.0f;
            actor->obj->spheres[1].pos.x = 0.0f;
            actor->obj->spheres[1].pos.y = 25.0f;
            actor->obj->spheres[1].pos.z = 0.0f;
            actor->obj->spheres[1].targetRadius = actor->obj->spheres[1].radius = 10.0f;
            actor->obj->spheres[1].flags &= ~0x20;
            actor->obj->spheres[2].targetRadius = actor->obj->spheres[1].radius = 1.0f;
            actor->obj->spheres[2].flags &= ~0x20;
            actor->obj->spheres[2].flags |= 0x17;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("CyHandR"))->sphere = &actor->obj->spheres[2];
            actor->obj->numSpheres = 3;
            actor->shadow->shadowSize = 10.0f;
            break;
        case 0x19:
            if (actor->active == 1) {
                PlayContinuousSample(&enemy->samples[0], 85, 100, &enemy->actor.pos, 128);
            }
            enemy->brain.bobPeriod = 10;
            enemy->brain.bobAmount = 0.05f;
            break;
        case 0x18:
            if (actor->active == 1) {
                PlayContinuousSample(&enemy->samples[0], 83, 70, &enemy->actor.pos, 128);
            }
            actor->rotAim.y = 0.0f;
            enemy->brain.bobPeriod = 10;
            enemy->brain.bobAmount = 0.1f;
            enemy->actor.shadow->shadowSize = 15.0f;
            break;
        case 0x5:
            enemy->brain.fireState[0] = 2;
            break;
        case 0xC:
            AnimateActor(actor, 5, 1, 0, 0);
            break;
        case 0xB:
            actor->obj->spheres[0].flags |= 8;
            actor->obj->numSpheres = 2;
            actor->obj->spheres[1].targetRadius = 25.0f;
            actor->obj->spheres[1].radius = 25.0f;
            actor->obj->spheres[1].flags |= 0x1C;
            actor->obj->spheres[1].flags &= ~0x20;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("SkHead"))->sphere = &actor->obj->spheres[1];
            AnimateActor(actor, 0, 1, 0, 0);
            break;
        case 0xA:
            AnimateActor(actor, 5, 1, 0, 0);
            enemy->brain.maxAlt = actor->obj->spheres[0].targetRadius + 0.1;
            break;
        case 0x1C:
            GiveActorRandomQRotVel(actor);
            actor->spinSpeed = 0.5f;
            enemy->brain.bobPeriod = 10;
            enemy->brain.bobAmount = 0.05f;
            break;
        case 0xF:
            actor->obj->numSpheres = 2;
            actor->obj->spheres[0].radius = actor->obj->spheres[0].targetRadius = 5.0f;
            actor->obj->spheres[0].flags = 0x23;
            actor->obj->spheres[1].radius = actor->obj->spheres[1].targetRadius = 12.0f;
            actor->obj->spheres[1].flags = 0x24;
            actor->shadow->shadowSize = 10.0f;
            break;
        case 0x11:
            actor->obj->numSpheres = 2;
            actor->obj->spheres[0].flags = 0x28;
            actor->obj->spheres[0].radius = actor->obj->spheres[0].targetRadius = 8.0f;
            actor->obj->spheres[1].radius = actor->obj->spheres[1].targetRadius = 10.0f;
            actor->obj->spheres[1].flags |= 0x14;
            actor->obj->spheres[1].flags &= ~0x20;
            actor->obj->spheres[1].pos = actor->pos;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("spikeybit"))->sphere = &actor->obj->spheres[1];
            break;
        case 0x14:
            actor->obj->numSpheres = 3;
            actor->obj->spheres[0].flags = 0x2B;
            actor->obj->spheres[0].radius = actor->obj->spheres[0].targetRadius = 8.0f;
            actor->obj->spheres[1].radius = actor->obj->spheres[1].targetRadius = 10.0f;
            actor->obj->spheres[1].flags |= 0x14;
            actor->obj->spheres[1].flags &= ~0x20;
            actor->obj->spheres[1].pos = actor->pos;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("bootleft"))->sphere = &actor->obj->spheres[1];
            actor->obj->spheres[2].radius = actor->obj->spheres[2].targetRadius = 10.0f;
            actor->obj->spheres[2].flags |= 0x14;
            actor->obj->spheres[2].flags &= ~0x20;
            actor->obj->spheres[2].pos = actor->pos;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("bootright"))->sphere = &actor->obj->spheres[2];
            actor->shadow->shadowSize = 20.0f;
            break;
        case 0x2C:
            enemy->brain.bobPeriod = 10;
            enemy->brain.bobAmount = 0.06666667f;
            if (actor->active == 1) {
                PlayContinuousSample(&enemy->samples[0], 101, 40, &enemy->actor.pos, 160);
            }
            break;
        case 0x30:
        case 0x31:
            enemy->brain.minAlt = -2.0f;
            enemy->brain.maxAlt = actor->obj->spheres[0].targetRadius + 15.0f;
            break;
        case 0x37:
            if (actor->active == 1) {
                PlayContinuousSample(&enemy->samples[0], 101, 30, &enemy->actor.pos, 230);
            }
            break;
        case 0x33:
            enemy->brain.bobPeriod = 6;
            enemy->brain.bobAmount = 0.5f;
            break;
        case 0x35:
            enemy->brain.bobPeriod = 6;
            enemy->brain.bobAmount = 0.5f;
            break;
        case 0x36:
            actor->shadow->shadowSize = 5.0f;
            break;
        case 0x3A:
            actor->shadow->shadowSize = 8.0f;
            actor->obj->numSpheres = 1;
            ZERO_VECTOR(&actor->obj->spheres[0].pos);
            actor->obj->spheres[0].flags = 0x20;
            break;
        case 0x12:
            enemy->brain.bobPeriod = 10;
            enemy->brain.bobAmount = 0.05f;
            break;
        case 0x20:
            actor->obj->numSpheres = 4;
            actor->obj->spheres[0].flags = 0xA;
            actor->obj->spheres[0].pos.y = -30.0f;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("GrmHead"))->sphere = &actor->obj->spheres[1];
            actor->obj->spheres[1].radius = actor->obj->spheres[1].targetRadius = 18.0f;
            actor->obj->spheres[1].flags = 0x18;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("Ei_LegL2"))->sphere = &actor->obj->spheres[2];
            actor->obj->spheres[2].radius = actor->obj->spheres[2].targetRadius = 20.0f;
            actor->obj->spheres[2].flags = 0x14;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("Ei_LegR2"))->sphere = &actor->obj->spheres[3];
            actor->obj->spheres[3].radius = actor->obj->spheres[3].targetRadius = 20.0f;
            actor->obj->spheres[3].flags = 0x14;
            break;
        case 0x1F:
            sprintf(name, "HubChick%d", HubChickenType() + 1);
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC(name))->sphere = &actor->obj->spheres[0];
            actor->obj->spheres[0].flags = 0x10;
            enemy->actor.shadow->sphere = 1;
            enemy->actor.shadow->shadowSize = 20.0f;
            break;
        case 0x21:
            actor->state->hits = 3;
            actor->obj->numSpheres = 4;
            actor->obj->spheres[0].flags = 0x2A;
            actor->obj->spheres[0].radius = actor->obj->spheres[0].targetRadius = 40.0f;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("KlosFingL"))->sphere = &actor->obj->spheres[1];
            actor->obj->spheres[1].radius = actor->obj->spheres[1].targetRadius = 1.0f;
            actor->obj->spheres[1].flags = 0x18;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("KloToeL"))->sphere = &actor->obj->spheres[2];
            actor->obj->spheres[2].radius = actor->obj->spheres[2].targetRadius = 15.0f;
            actor->obj->spheres[2].flags = 0x14;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("KloToeR"))->sphere = &actor->obj->spheres[3];
            actor->obj->spheres[3].radius = actor->obj->spheres[3].targetRadius = 15.0f;
            actor->obj->spheres[3].flags = 0x14;
            break;
        case 0x22:
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("RapFootR"))->sphere = &actor->obj->spheres[1];
            actor->obj->spheres[1].flags = 0x1C;
            actor->obj->spheres[1].targetRadius = 25.0f;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("RapFootL"))->sphere = &actor->obj->spheres[2];
            actor->obj->spheres[2].flags = 0x1C;
            actor->obj->spheres[2].targetRadius = 25.0f;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("RapHeart"))->sphere = &actor->obj->spheres[3];
            actor->obj->spheres[3].flags = 0x1C;
            actor->obj->spheres[3].targetRadius = 0.0f;
            enemy->actor.state->hits = 3;
            actor->obj->numSpheres = 4;
            break;
        case 0x25:
            enemy->actor.link = &enemies[1].actor;
            actor->obj->numSpheres = 2;
            actor->obj->spheres[1].radius = actor->obj->spheres[1].targetRadius = 5.0f;
            actor->obj->spheres[1].flags = 0x2B;
            actor->obj->spheres[0].flags = 0x24;
            break;
        case 0x24:
            enemy->brain.fireState[0] = 2;
            enemy->actor.link = &enemies[0].actor;
            actor->obj->numSpheres = 4;
            actor->obj->spheres[0].flags = 0x23;
            actor->obj->spheres[1].radius = actor->obj->spheres[1].targetRadius = 23.0f;
            actor->obj->spheres[1].flags = 0x2C;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("canlftlinkclaw"))->sphere = &actor->obj->spheres[2];
            actor->obj->spheres[2].radius = actor->obj->spheres[2].targetRadius = 10.0f;
            actor->obj->spheres[2].flags = 0x14;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("canrtlinkclaw"))->sphere = &actor->obj->spheres[3];
            actor->obj->spheres[3].radius = actor->obj->spheres[3].targetRadius = 10.0f;
            actor->obj->spheres[3].flags = 0x14;
            actor->shadow->shadowSize = 15.0f;
            break;
        case 0x23:
            if (frameCount == 0) {
                enemy->actor.state->hits = 5;
            }
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("cancerpolly"))->sphere = &actor->obj->spheres[1];
            actor->obj->spheres[1].flags = 0x17;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("kpolly"))->sphere = &actor->obj->spheres[2];
            actor->obj->spheres[2].flags = 0x17;
            break;
        case 0x28:
            if (frameCount == 0) {
                enemy->actor.state->hits = 3;
            }
            enemy->actor.shadow->sphere = 2;
            enemy->actor.obj->numSpheres = 2;
            actor->obj->spheres[0].flags |= 0xB;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("SpkTorso"))->sphere = &actor->obj->spheres[1];
            actor->obj->spheres[1].flags = 0x14;
            actor->obj->spheres[1].targetRadius = 20.0f;
            break;
        case 0x29:
            enemy->actor.shadow->sphere = 2;
            enemy->actor.obj->numSpheres = 2;
            actor->obj->spheres[0].flags |= 0xB;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("SpJTorso"))->sphere = &actor->obj->spheres[1];
            actor->obj->spheres[1].flags = 0x14;
            actor->obj->spheres[1].targetRadius = 12.0f;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("SpJBall"))->sphere = &actor->obj->spheres[2];
            actor->obj->spheres[2].flags = 0x10;
            break;
        case 0x27:
            if (frameCount == 0) {
                enemy->actor.state->hits = 3;
            }
            actor->obj->numSpheres = 1;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("roblsho"))->sphere = &actor->obj->spheres[1];
            actor->obj->spheres[1].flags = 0x17;
            actor->obj->spheres[1].radius = actor->obj->spheres[1].targetRadius = 0.0f;
            actor->obj->spheres[1].platTag[0] = actor->obj->spheres[1].platTag[1] = 1;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("robrsho"))->sphere = &actor->obj->spheres[2];
            actor->obj->spheres[2].flags = 0x17;
            actor->obj->spheres[2].radius = actor->obj->spheres[2].targetRadius = 0.0f;
            actor->obj->spheres[2].platTag[0] = actor->obj->spheres[2].platTag[1] = 2;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("robface"))->sphere = &actor->obj->spheres[3];
            actor->obj->spheres[3].flags = 0x17;
            actor->obj->spheres[3].radius = actor->obj->spheres[3].targetRadius = 0.0f;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("roblgun"))->sphere = &actor->obj->spheres[4];
            actor->obj->spheres[4].flags = 0x17;
            actor->obj->spheres[4].radius = actor->obj->spheres[4].targetRadius = 0.0f;
            actor->obj->spheres[4].platTag[0] = 4;
            tempObj = FindSubObjectFromObject(actor->inst->obj, UpdateCRC("robrgun"));
            tempObj->sphere = &actor->obj->spheres[5];
            actor->obj->spheres[5].flags = 0x17;
            actor->obj->spheres[5].radius = actor->obj->spheres[5].targetRadius = 0.0f;
            actor->obj->spheres[5].platTag[0] = 5;
            break;
        case 0x26:
            enemy->actor.obj->numSpheres = 4;
            if (actor->active == 1) {
                PlayContinuousSample(&enemy->samples[0], 100, 20, &enemy->actor.pos, 128);
            }
            if (frameCount == 0) {
                enemy->actor.state->hits = 3;
            }
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("bot_foot_R"))->sphere = &actor->obj->spheres[1];
            actor->obj->spheres[1].flags = 0x1F;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("bot_foot_L"))->sphere = &actor->obj->spheres[2];
            actor->obj->spheres[2].flags = 0x1F;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("bot_sac01"))->sphere = &actor->obj->spheres[3];
            actor->obj->spheres[3].flags = 0x1F;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("bot_eye_L"))->sphere = &actor->obj->spheres[4];
            actor->obj->spheres[4].flags = 0x1F;
            FindSubObjectFromObject(actor->inst->obj, UpdateCRC("bot_eye_R"))->sphere = &actor->obj->spheres[5];
            actor->obj->spheres[5].flags = 0x1F;
            tempObj = FindSubObjectFromObject(actor->inst->obj, UpdateCRC("bot_hed_light"));
            tempObj->flags |= 0x40;
            break;
        case 0x8:
        case 0xD:
        case 0xE:
        case 0x1D:
        case 0x2A:
        case 0x2B:
        case 0x2D:
        case 0x2E:
        case 0x2F:
        case 0x32:
        case 0x34:
            break;
        default:
            PRINTF("This enemy has no load code\n");
            break;
    }
    CalcMaxActorRadius(actor);
    for (j = 0; j < actor->obj->numSpheres; j++) {
        if (actor->obj->spheres[j].flags & 0x10) {
            actor->obj->spheres[j].pos = actor->pos;
        }
    }
    if (actor->phys->flags & 0x100) {
        actor->flags |= 0x100;
    }
    actor->active = 1;
}

s32 NMEOverstepEdge(Enemy *enemy) {
    Vec3 tempVect;
    Vec3 *tempPlane;
    s32 tempPlat;
    f32 alt;
    s32 i;
    s32 doIt = FALSE;
    Plane *plane;

    if (enemy->brain.delayTimer == 0 && !(enemy->brain.flags & 4)) {
        tempVect = enemy->actor.vel;
        tempVect.y = 0.0f;
        MakeUnit(&tempVect);
        SCALE_VECTOR_2D(&tempVect, 20.0f);
        ADD_VECTORS(&tempVect, &enemy->actor.pos, &tempVect);
        alt = Altitude(&tempVect, &tempPlane, &tempPlat, NULL);
        if (enemy->actor.obj->onGround == 1 && (alt > enemy->brain.maxAlt || alt < enemy->brain.minAlt)
            && enemy->actor.state->flying == 0) {
            doIt = TRUE;
            ZERO_VECTOR(&enemy->actor.vel);
        }
        for (i = 0; i < 4; i++) {
            plane = enemy->actor.obj->planes[i];
            if (plane != NULL && plane->y < 0.8) {
                ZERO_VECTOR(&enemy->actor.vel);
                if (enemy->actor.type == BUGLE) {
                    tempVect = *(Vec3 *)plane;
                    SCALE_VECTOR(&tempVect, 2.0f);
                    enemy->actor.vel = tempVect;
                } else {
                    AddToVector2D(&enemy->actor.vel, (Vec3 *)plane);
                }
                doIt = TRUE;
                break;
            }
        }
        if (doIt) {
            if (!(enemy->brain.instr.flags & 0x1000)) {
                if (FindShortestAngle(enemy->actor.rotAim.y, enemy->actor.rot.y) < 1.570796327) {
                    enemy->actor.rotAim.y += 3.141592654;
                }
                enemy->actor.rotAim.y = Aabs(enemy->actor.rotAim.y);
            }
            enemy->brain.delayTimer = 30;
            return TRUE;
        }
    }
    return FALSE;
}

void NMEActionStartAttack(Enemy *enemy) {
    if (enemy->brain.attackCount <= 0) {
        PRINTF("error:no attack instructions present\n");
        return;
    }
    enemy->brain.attackTime++;
    if (!enemy->brain.inConditional) {
        enemy->brain.returnIndex = enemy->brain.instrIndex + 1;
    }
    enemy->brain.instrIndex = 0;
    enemy->brain.inConditional = 0;
    enemy->brain.curList = enemy->brain.attackList;
}

s32 NMEActionMoveTo(Enemy *enemy, Vec3 *dest, f32 proximity, s32 flags) {
    Actor *actor = &enemy->actor;
    Vec3 tempVect;
    f32 dist;
    f32 tempProx;
    f32 scale;
    f32 temp;

    if (NMEOverstepEdge(enemy)) {
        return TRUE;
    }
    if (DistanceBetweenPointsSquared(&actor->pos, dest) < proximity * proximity) {
        if (actor->type != BUGLE) {
            if (actor->type == DIBBER) {
                SCALE_VECTOR(&actor->vel, 0.5);
            }
            actor->rotAim.z = 0.0f;
        }
        return TRUE;
    }
    SUB_VECTORS(&tempVect, &actor->pos, dest);
    if (!(flags & 0x1000)) {
        actor->rotAim.y = Aabs(atan2(tempVect.x, tempVect.z) + 3.141592654);
    }
    if (actor->type == ROBES) {
        if (actor->anim->current == 15) {
            if (actor->anim->time <= 103.0f || actor->anim->time >= 134.0f) {
                return FALSE;
            }
        } else if (actor->anim->current != 9) {
            return FALSE;
        }
    }
    MakeUnit(&tempVect);
    SCALE_VECTOR(&tempVect, actor->phys->accel);
    if (enemy->actor.type != BUGLE && FindShortestAngle(actor->rot.y, actor->rotAim.y) > 1.0471975513333334) {
        SCALE_VECTOR(&actor->vel, 0.8);
    }
    if (!(flags & 4)) {
        dist = DistanceBetweenPoints(&actor->pos, dest);
        tempProx = proximity * 1.3;
        if (dist < tempProx) {
            scale = dist / tempProx;
            SCALE_VECTOR(&actor->vel, scale);
            SCALE_VECTOR(&tempVect, 0.7);
        }
    }
    if (flags & 1) {
        AccelerateActor2D(actor, LENGTH_2D(&tempVect), actor->phys->moveSpeed);
    }
    if (flags & 2) {
        actor->vel.x -= tempVect.x;
        actor->vel.z -= tempVect.z;
    }
    actor->vel.y -= tempVect.y;
    ActorMaxOutVel(actor, actor->phys->moveSpeed);
    if (flags & 0x2000) {
        temp = FindShortestAngleSigned(actor->rotAim.y, actor->rot.y) * enemy->brain.bank;
        if (ABS(temp) > 0.08) {
            actor->rotAim.z = Aabs(temp);
        } else {
            actor->rotAim.z = 0.0f;
        }
    }
    return FALSE;
}

s32 NMEActionMoveTo2D(Enemy *enemy, Vec3 *dest, f32 proximity, s32 flags) {
    Actor *actor = &enemy->actor;
    Vec3 tempVect;
    f32 temp;

    actor->rotAim.z = 0.0f;
    if (NMEOverstepEdge(enemy)) {
        return TRUE;
    }
    if (DistanceBetweenPoints2D(&actor->pos, dest) < proximity) {
        SCALE_VECTOR_2D(&actor->vel, 0.5);
        return TRUE;
    }
    if (flags & 1) {
        SubVector2D(&tempVect, &actor->pos, dest);
        MakeUnit2D(&tempVect);
        SCALE_VECTOR_2D(&tempVect, actor->phys->accel);
        if (!(flags & 0x1000)) {
            actor->rotAim.y = Aabs(atan2(tempVect.x, tempVect.z) + 3.141592654);
        }
        AccelerateActor2D(actor, actor->phys->accel, actor->phys->moveSpeed);
    }
    if (flags & 2) {
        if (!(actor->flags & 0x1000000)
            && LENGTH_2D(&actor->vel) >= actor->phys->moveSpeed) {
            return FALSE;
        }
        SUB_VECTORS(&tempVect, &actor->pos, dest);
        tempVect.y = 0.0f;
        MakeUnit(&tempVect);
        SCALE_VECTOR(&tempVect, actor->phys->accel);
        actor->vel.x -= tempVect.x;
        actor->vel.z -= tempVect.z;
    }
    if (flags & 0x2000) {
        temp = FindShortestAngleSigned(actor->rotAim.y, actor->rot.y) * enemy->brain.bank;
        if (ABS(temp) > 0.08) {
            actor->rotAim.z = temp;
        } else {
            actor->rotAim.z = 0.0f;
        }
    }
    return FALSE;
}

void NMEActionWait(Enemy *enemy) {
    if (enemy->actor.type == DIBBER) {
        SCALE_VECTOR(&enemy->actor.vel, 0.5);
    }
    if (enemy->brain.instrTime == 0) {
        if (enemy->brain.instr.params.flags & 2) {
            if (enemy->brain.instr.params.flags & 4) {
                enemy->actor.anim->loop = 0;
                AnimateActor(&enemy->actor, 14, 0, 1, 0.0f);
            } else {
                AnimateActor(&enemy->actor, 14, 0, 0, 0.0f);
            }
            if (!(enemy->brain.instr.flags & 0x4000)) {
                AnimateActor(&enemy->actor, 5, (u8)enemy->brain.instr.arg.i, 1, 0.0f);
            }
        } else if (!(enemy->brain.instr.flags & 0x4000)) {
            if (enemy->brain.instr.params.flags & 4) {
                enemy->actor.anim->loop = 0;
                AnimateActor(&enemy->actor, 5, (u8)enemy->brain.instr.arg.i, 1, 0.0f);
            } else {
                AnimateActor(&enemy->actor, 5, (u8)enemy->brain.instr.arg.i, 0, 0.0f);
            }
        }
    }
    if ((enemy->brain.instr.params.flags & 1) && enemy->brain.instrTime < 5) {
        if (enemy->brain.instrTime == 2) {
            PlaySample(150, &enemy->actor.pos, 100, 128);
        }
        CreateDustCircleScaled(&enemy->actor.pos, 2, 0, 0.8f);
    }
}

s32 NMEActionFaceTo(Enemy *enemy, Vec3 *dest) {
    Vec3 d;
    Enemy *self = enemy;

    if (!(self->brain.instr.flags & 0x4000) && !self->brain.instrTime) {
        AnimateActor(&self->actor, 15, 1, 0, 0);
    }
    if (!enemy->brain.instr.arg.i) {
        SUB_VECTORS(&d, dest, &self->actor.pos);
        self->actor.rotAim.y = Aabs(atan2(d.x, d.z));
    } else if (!enemy->brain.instrTime) {
        self->actor.rotAim.y = Random(628) / 100.0f;
    }
    if (FindShortestAngle(self->actor.rotAim.y, self->actor.rot.y) < 0.1) {
        return TRUE;
    }
    return FALSE;
}

inline void NMEChooseRandomPoint(Enemy *enemy) {
    Vec3 tempVect;
    EnemyBox box = enemy->brain.instr.params.box;
    f32 slop = enemy->brain.instr.arg.i;

    do {
        tempVect.x = box.min.x + Random(box.size.x);
        if (enemy->actor.state->flying) {
            tempVect.y = box.min.y + Random(box.size.y);
        } else {
            tempVect.y = enemy->actor.pos.y;
        }
        tempVect.z = box.min.z + Random(box.size.z);
    } while (DistanceBetweenPointsSquared(&enemy->brain.target, &tempVect) < slop * slop);
    enemy->brain.target = tempVect;
}

void NMEActionRandomMove(Enemy *enemy) {
    Actor *actor = &enemy->actor;
    f32 prox = enemy->actor.speed + 20.0f;
    s32 reachedDestination;

    if (enemy->brain.instrTime > 0) {
        if (enemy->actor.state->flying == 1) {
            reachedDestination = NMEActionMoveTo(enemy, &enemy->brain.target, prox, enemy->brain.instr.flags);
        } else {
            reachedDestination = NMEActionMoveTo2D(enemy, &enemy->brain.target, prox, enemy->brain.instr.flags);
        }
    } else if (!(enemy->brain.instr.flags & 0x4000)) {
        reachedDestination = FALSE;
        if (enemy->actor.type == YOOFOW) {
            AnimateActor(&enemy->actor, 13, 0, 0, 0.0f);
            AnimateActor(&enemy->actor, 15, 1, 1, 0.0f);
        } else if (enemy->brain.instr.flags & 0x40000) {
            AnimateActor(&enemy->actor, 9, 1, 0, 0.0f);
        } else {
            AnimateActor(&enemy->actor, 15, 1, 0, 0.0f);
        }
    }
    if (enemy->brain.instrTime == 0 || reachedDestination) {
        NMEChooseRandomPoint(enemy);
        if (reachedDestination && actor->type == YOOFOW && enemy->brain.instrTime > 0) {
            NMEGetNewInstruction(enemy, 1, 0);
        }
    }
}

s32 NMEActionFacePlayer(Enemy *enemy, s32 flags) {
    Vec3 *tempVectPtr;
    Enemy *self;
    Vec3 d;

    if (flags & 0x40) {
        tempVectPtr = &hand.pos;
    } else if (flags & 0x80) {
        tempVectPtr = &ball.pos;
    } else if (flags & 0x100) {
        if (DistanceBetweenPointsSquared(&hand.pos, &enemy->actor.pos) <
            DistanceBetweenPointsSquared(&ball.pos, &enemy->actor.pos)) {
            tempVectPtr = &hand.pos;
        } else {
            tempVectPtr = &ball.pos;
        }
    } else {
        PRINTF("NME_FACEPLAYER:error no player specified\n");
    }
    self = enemy;
    if (!(self->brain.instr.flags & 0x4000) && self->brain.instrTime == 0) {
        AnimateActor(&self->actor, 15, 1, 0, 0.0f);
    }
    if (enemy->brain.instr.arg.i == 0) {
        SUB_VECTORS(&d, tempVectPtr, &self->actor.pos);
        self->actor.rotAim.y = Aabs(atan2(d.x, d.z));
    } else if (enemy->brain.instrTime == 0) {
        self->actor.rotAim.y = Random(628) / 100.0f;
    }
    if (!(FindShortestAngle(self->actor.rotAim.y, self->actor.rot.y) < 0.1)) {
        return FALSE;
    }
    return TRUE;
}

s32 NMEActionFollowPlayer(Enemy *enemy) {
    NMEInstruction *ins = &enemy->brain.instr;
    Vec3 *tempVectPtr;
    Vec3 tempVect;

    if (ins->flags & 0x40) {
        if (handControl.deathTimer || hand.state->mode == HAND_HURT) {
            return FALSE;
        }
        tempVectPtr = &hand.pos;
    } else if (ins->flags & 0x80) {
        if (ballControl.state == BALL_BURST) {
            return FALSE;
        }
        tempVectPtr = &ball.pos;
    } else if (ins->flags & 0x100) {
        if (DistanceBetweenPointsSquared(&hand.pos, &enemy->actor.pos) <
            DistanceBetweenPointsSquared(&ball.pos, &enemy->actor.pos)) {
            if (handControl.deathTimer || hand.state->mode == HAND_HURT) {
                return FALSE;
            }
            tempVectPtr = &hand.pos;
        } else {
            if (ballControl.state == BALL_BURST) {
                return FALSE;
            }
            tempVectPtr = &ball.pos;
        }
    } else {
        PRINTF("NME_FOLLOWPLAYER:error no player specified\n");
    }
    if (enemy->brain.area.size.x > 0.0f && !IsPointWithinBox(&enemy->brain.area, tempVectPtr)) {
        return FALSE;
    }
    ADD_VECTORS(&tempVect, tempVectPtr, &ins->params.box.min);
    if (enemy->actor.state->flying != 1) {
        return NMEActionMoveTo2D(enemy, &tempVect, ins->arg.f, ins->flags);
    }
    return NMEActionMoveTo(enemy, &tempVect, ins->arg.f, ins->flags);
}

s32 NMEActionFleePlayer(Enemy *enemy) {
    NMEInstruction *ins = &enemy->brain.instr;
    Vec3 *tempVectPtr;
    Vec3 tempVect;
    f32 dist;

    if (enemy->brain.instr.flags & 0x40) {
        tempVectPtr = &hand.pos;
    } else if (enemy->brain.instr.flags & 0x80) {
        tempVectPtr = &ball.pos;
    } else if (enemy->brain.instr.flags & 0x100) {
        if (DistanceBetweenPointsSquared(&hand.pos, &enemy->actor.pos) <
            DistanceBetweenPointsSquared(&ball.pos, &enemy->actor.pos)) {
            tempVectPtr = &hand.pos;
        } else {
            tempVectPtr = &ball.pos;
        }
    } else {
        PRINTF("NME_FLEEPLAYER:error no player specified\n");
    }
    SUB_VECTORS(&tempVect, &enemy->actor.pos, tempVectPtr);
    dist = LENGTH_2D(&tempVect);
    if (ins->params.flags < dist) {
        return TRUE;
    }
    SCALE_VECTOR(&tempVect, 256.0f);
    ADD_VECTORS(&tempVect, &enemy->actor.pos, &tempVect);
    if (enemy->actor.state->flying != 1) {
        NMEActionMoveTo2D(enemy, &tempVect, 2.0f, ins->flags);
        return FALSE;
    }
    NMEActionMoveTo(enemy, &tempVect, 2.0f, ins->flags);
    return FALSE;
}

s32 NMEActionCharge(Enemy *enemy) {
    NMEBrain *br = &enemy->brain;
    NMEInstruction *ins = &br->instr;
    Vec3 *tempVectPtr;
    Vec3 dest;

    if (ins->flags & 0x40) {
        tempVectPtr = &hand.pos;
    } else if (ins->flags & 0x80) {
        tempVectPtr = &ball.pos;
    } else if (ins->flags & 0x100) {
        if (DistanceBetweenPointsSquared(&hand.pos, &enemy->actor.pos) <
            DistanceBetweenPointsSquared(&ball.pos, &enemy->actor.pos)) {
            tempVectPtr = &hand.pos;
        } else {
            tempVectPtr = &ball.pos;
        }
    } else {
        PRINTF("NME_CHARGE:error no player specified\n");
    }
    if (br->instrTime == 0) {
        br->scratch = 0;
    }
    if (br->scratch == 0) {
        if (!IsActorFacingPoint(&enemy->actor, tempVectPtr, 1.0f) || br->instrTime == 0) {
            SUB_VECTORS(&dest, &enemy->actor.pos, tempVectPtr);
            SCALE_VECTOR(&dest, 256.0f);
            SUB_VECTORS(&dest, &enemy->actor.pos, &dest);
            br->target = dest;
        } else {
            br->scratch = 1;
        }
    }
    if (enemy->actor.state->flying != 1) {
        return NMEActionMoveTo2D(enemy, &br->target, 2.0f, ins->flags);
    }
    return NMEActionMoveTo(enemy, &br->target, 2.0f, ins->flags);
}

void NMEActionJump(Enemy *enemy) {
    f32 dist;

    if (enemy->brain.instrTime == 0) {
        if (!(enemy->brain.instr.flags & 1)) {
            ZERO_VECTOR(&enemy->actor.vel);
        }
        enemy->actor.vel.y = enemy->brain.instr.params.box.min.y;
        AccelerateActor2D(&enemy->actor, enemy->brain.instr.params.box.min.z, 50.0f);
        if (!(enemy->brain.instr.flags & 0x4000)) {
            AnimateActor(&enemy->actor, 7, 0, 0, 0.0f);
        }
        if (enemy->actor.type == DENIS) {
            PlaySample(19, &enemy->actor.pos, 100, 128);
        }
    }
    if (enemy->brain.instr.flags & 0x800000) {
        if (enemy->brain.instr.flags & 0x40) {
            dist = DistanceBetweenPoints2D(&enemy->actor.pos, &hand.pos);
        } else if (enemy->brain.instr.flags & 0x80) {
            dist = DistanceBetweenPoints2D(&enemy->actor.pos, &ball.pos);
        }
        if (dist < 15.0f) {
            SCALE_VECTOR_2D(&enemy->actor.vel, 0.5);
            enemy->actor.vel.y -= 0.8;
        }
    }
}

s32 NMEActionCircle(Enemy *enemy) {
    NMEBrain *br = &enemy->brain;
    NMEInstruction *ins = &br->instr;
    Vec3 tempVect;
    f32 rotationFactor = 1.12f;
    s32 axis = 1;
    s32 tempFlags;

    if (enemy->actor.type == YOOFOW) {
        rotationFactor = 0.7f;
    }
    if (br->instrTime == 0) {
        if (ins->flags & 0x400) {
            br->scratchF = br->tiltAngle = 3.1415927f;
        } else {
            br->scratchF = br->tiltAngle = 0.0f;
        }
    }
    tempFlags = ins->flags;
    if (DistanceBetweenPointsSquared(&br->target, &enemy->actor.pos) < 400.0f) {
        SCALE_VECTOR(&enemy->actor.vel, 0.9);
        if (br->instrTime > 0) {
            if (br->instr.flags & 0x400) {
                br->scratchF = Aabs(br->scratchF - enemy->actor.phys->moveSpeed * rotationFactor / ins->arg.f);
            } else {
                br->scratchF = Aabs(br->scratchF + enemy->actor.phys->moveSpeed * rotationFactor / ins->arg.f);
            }
            br->tiltAngle = Aabs(br->tiltAngle + enemy->actor.phys->moveSpeed * rotationFactor / ins->arg.f);
        }
        if (ins->flags & 0x20) {
            if (ins->flags & 0x400) {
                enemy->actor.rotAim.y = 4.712389f;
                enemy->actor.rot.x = Aabs(br->tiltAngle + PI);
            } else {
                enemy->actor.rotAim.y = 1.5707964f;
                enemy->actor.rot.x = Aabs(br->scratchF + PI);
            }
        } else if (ins->flags & 8) {
            if (ins->flags & 0x400) {
                enemy->actor.rotAim.y = 0.0f;
                enemy->actor.rotAim.x = Aabs(br->tiltAngle + PI);
            } else {
                enemy->actor.rotAim.y = 3.1415927f;
                enemy->actor.rotAim.x = Aabs(br->scratchF + PI);
            }
        }
    } else {
        enemy->actor.rotAim.x = 0.0f;
        if (br->instrTimer > 0) {
            br->instrTimer++;
        }
        if (tempFlags & 0x200) {
            tempFlags &= ~0x200;
        }
        if (tempFlags & 0x1000) {
            tempFlags &= ~0x1000;
        }
    }
    if (!(tempFlags & 0x1000)) {
        if (tempFlags & 0x10) {
            SubVector2D(&tempVect, &br->target, &enemy->actor.pos);
            MakeUnit2D(&tempVect);
            enemy->actor.rotAim.y = Aabs(atan2(tempVect.x, tempVect.z));
        }
    }
    ZERO_VECTOR(&tempVect);
    if (ins->flags & 8) {
        axis = 0;
        tempVect.y = 1.0f;
    }
    if (ins->flags & 0x10) {
        axis = 1;
        tempVect.x = 1.0f;
    }
    if (ins->flags & 0x20) {
        axis = 2;
        tempVect.y = 1.0f;
    }
    RotateVector2DXYZ(&tempVect, &tempVect, br->scratchF, axis);
    SCALE_VECTOR(&tempVect, ins->arg.f);
    ADD_VECTORS(&br->target, &tempVect, &ins->params.box.min);
    if (enemy->actor.state->flying != 1) {
        NMEActionMoveTo2D(enemy, &br->target, -1.0f, tempFlags);
        return FALSE;
    }
    NMEActionMoveTo(enemy, &br->target, -1.0f, tempFlags);
    return FALSE;
}

void NMEActionFire1(Enemy *enemy) {
    switch (enemy->actor.type) {
        case 24: // an empty case shapes the comparison tree
            break;
        case 26:
            if (enemy->brain.instrTime == 1) {
                AnimateActor(&enemy->actor, 0, 0, 0, 0);
            }
            break;
        case 25:
            FireBulletFromActor(&enemy->actor, DROID_BULLET, 0);
            break;
    }
}

void NMEActionHoldPlayer(Actor *enemy, Vec3 *pos, s32 num, s32 flags) {
    if (controlMethod != CONTROL_BALL) {
        if (cameraInfo.focus == &ball.pos) {
            cameraInfo.focus = &hand.pos;
            cameraInfo.focusVel = &hand.vel;
        }
        if (ball.link != NULL) {
            if (ball.link == &hand) {
                ball.link = NULL;
                hand.flags &= ~0x40;
                ball.pos.x += 0.1;
                if (hand.state->mode != HAND_ROTOR) {
                    ApplyAction(HAND_RELEASE);
                    handControl.actionLock = 10;
                }
            } else {
                EnemyDropBall();
            }
        }
        if (flags & 0x80) {
            if (ballControl.enemy != NULL) {
                EnemyDropBall();
            }
            handControl.releaseTimer = 0;
            ball.link = enemy;
            ballControl.state = BALL_HELD;
            ballControl.enemy = enemy;
            ballControl.enemyOffset = *pos;
            if (flags & 0x20000) {
                ballControl.enemySphere = num;
            } else {
                ballControl.enemySphere = -1;
            }
            ballControl.releaseTimer = 0;
            handControl.releaseTimer = 0;
            ball.flags &= ~1;
            ballControl.enemyRot = enemy->rot;
        } else if (flags & 0x40) {
            if (handControl.attachedTo != NULL) {
                EnemyDropHand();
            }
            handControl.status = HAND_HELD;
            hand.link = enemy;
            handControl.dropTimer = 0;
            handControl.releaseTimer = 0;
            handControl.attachedTo = enemy;
            if (flags & 0x20000) {
                handControl.attachTag = num;
            } else {
                handControl.attachTag = -1;
            }
            handControl.attachOffset = *pos;
            hand.flags &= ~1;
        }
    }
}

void UpdateEnemies(void) {
    s32 x;
    s32 j;

    for (x = 0; x < numberOfEnemies; x++) {
        UpdateContinuousSample(&enemies[x].samples[0]);
        UpdateContinuousSample(&enemies[x].samples[1]);
        if ((defaultBehaviour[enemies[x].actor.type].flags & 0x200000) || !frameCount ||
            ballControl.enemy == &enemies[x].actor ||
            DistanceBetweenPointsSquared(&enemies[x].actor.pos, &camera.pos) <=
                farPlaneDist * farPlaneDist + 40000 ||
            (enemies[x].actor.type == SAMTEX && enemies[x].samples[0].handle)) {
            UpdateEnemy(x);
        } else if (enemies[x].actor.inst->obj->numSprites > 0) {
            for (j = 0; j < enemies[x].actor.inst->obj->numSprites; j++) {
                if (enemies[x].actor.inst->obj->sprites[j].sprite) {
                    enemies[x].actor.inst->obj->sprites[j].sprite->pos = enemies[x].actor.pos;
                }
            }
        }
        enemies[x].brain.flags &= ~8;
        enemies[x].actor.flags &= ~0x4000000;
    }
}

void UpdateEnemy(s32 x) {
    Vec3 tempVect;
    s32 i;
    f32 dist;

    if (enemies[x].actor.active && !(enemies[x].brain.flags & 0x100)) {
        if (enemies[x].actor.type == CANNONBALL && ABS(enemies[x].actor.spinSpeed) <= 0.05) {
            enemies[x].brain.spellTimer[0] = 1;
            enemies[x].brain.curList = enemies[x].brain.normalList;
            NMEGetNewInstruction(&enemies[x], 0, 1);
        }
        for (i = 0; i < 3; i++) {
            if (enemies[x].brain.spellTimer[i] > 0 && --enemies[x].brain.spellTimer[i] == 0) {
                DeactivateSpell(&enemies[x].actor, i);
            }
        }
        if (ballControl.enemy == &enemies[x].actor) {
            ballControl.enemyRot = enemies[x].actor.rot;
        }
        UpdateActor(&enemies[x].actor);
        if ((enemies[x].brain.flags & 0x200) && (enemies[x].actor.flags & 0x4000000) && ballControl.type == BALL_MODE_BOWLING
            && ball.speed > 5.0f && (ball.link == NULL || (handControl.releaseTimer && player.hurtTimer == 0))) {
            NMEHitByBlastRing(&enemies[x]);
        }
        if (enemies[x].actor.active) {
            if (enemies[x].brain.spellTimer[1]) {
                NMEHandleDeathSpell(&enemies[x]);
            } else if (enemies[x].brain.spellTimer[0] == 0) {
                if (enemies[x].brain.spellTimer[2] == 0) {
                    NMEProcessActorScript(&enemies[x]);
                } else {
                    NMEProcessFroggy(&enemies[x]);
                }
            } else {
                SetVector2D(&tempVect, &enemies[x].actor.pos);
                tempVect.y = enemies[x].actor.pos.y + enemies[x].actor.radius;
                if (enemies[x].actor.type == FRANKIE) {
                    tempVect = enemies[x].actor.obj->spheres[1].pos;
                    tempVect.y += 20.0f;
                }
                DoDizzyStars(&tempVect);
            }
        } else {
            if (enemies[x].samples[0].handle) {
                StopContinuousSample(&enemies[x].samples[0]);
            }
            if (enemies[x].samples[1].handle) {
                StopContinuousSample(&enemies[x].samples[1]);
            }
        }
        if (enemies[x].actor.type == DENIS && hand.state->mode != HAND_LOCATE && !IsHandHoldingBall()) {
            if (hand.state->mode != HAND_HOPPING && hand.state->mode != HAND_RELEASE && hand.state->mode != HAND_HURT && hand.state->mode != HAND_SLAM && handControl.frog == 0) {
                dist = DistanceBetweenPoints2D(&hand.pos, &enemies[x].actor.pos);
                if (hand.pos.y - enemies[x].actor.pos.y > 0.0f) {
                    if (hand.pos.y - enemies[x].actor.pos.y < 40.0f) {
                        if (dist < 20.0f) {
                            hand.link = &enemies[x].actor;
                            handControl.dropTimer = 0;
                            handControl.releaseTimer = 0;
                            ApplyAction(HAND_HOPPING);
                            AnimateActor(&enemies[x].actor, 5, 1, 0, 0.0f);
                            enemies[x].brain.spellTimer[0] = 0;
                        }
                    }
                }
            }
        }
    } else if (enemies[x].actor.state->hitFadeTimer) {
        enemies[x].actor.state->hitFadeTimer--;
    }
}

void NMEProcessFroggy(Enemy *enemy) {
    f32 target;
    s32 flags;

    NMEProcessFrogged(enemy);
    if (enemy->brain.flags & 0x10) {
        if (enemy->brain.spellTimer[2] < 50) {
            enemy->actor.scale.x *= 0.5;
            enemy->actor.scale.y *= 0.5;
            enemy->actor.scale.z *= 0.5;
            if (enemy->actor.scale.x <= 0.001) {
                SubActor(&enemy->actor);
                RemoveUniqueActor(&enemy->actor, NME_FROG);
                FindObject(&enemy->actor.inst, UpdateCRC(nmeObjectName[enemy->actor.type]),
                           nmeObjectName[enemy->actor.type], 1);
                enemy->brain.flags &= ~0x10;
                CreateStarCircle(&enemy->actor.pos, 5, 19);
                TurnEnemyInto(enemy, enemy->actor.type);
                NMEGetNewInstruction(enemy, 0, 1);
                enemy->actor.anim->time = 1.0f;
                enemy->actor.pos.y += HeightToCentre(&enemy->actor) - defaultBehaviour[0x3A].radius;
            }
        } else {
            target = enemyStats[NME_FROG].scale;
            if (enemy->actor.scale.x < target - 0.001) {
                enemy->actor.scale.y = enemy->actor.scale.x =
                    enemy->actor.scale.x + (target - enemy->actor.scale.x) / 2.0f;
                enemy->actor.scale.z = enemy->actor.scale.x;
            } else {
                enemy->actor.scale.x = enemy->actor.scale.y = enemy->actor.scale.z = target;
            }
        }
    } else {
        if (enemy->brain.spellTimer[2] < 50) {
            target = enemyStats[enemy->actor.type].scale;
            if (enemy->actor.scale.x < target - 0.001) {
                enemy->actor.scale.y = enemy->actor.scale.x =
                    enemy->actor.scale.x + (target - enemy->actor.scale.x) / 2.0f;
                enemy->actor.scale.z = enemy->actor.scale.x;
            } else {
                enemy->actor.scale.x = enemy->actor.scale.y = enemy->actor.scale.z = target;
                enemy->brain.spellTimer[2] = 1;
            }
        } else {
            enemy->actor.scale.x *= 0.5;
            enemy->actor.scale.y *= 0.5;
            enemy->actor.scale.z *= 0.5;
            if (enemy->actor.scale.x <= 0.001) {
                SubActor(&enemy->actor);
                RemoveUniqueActor(&enemy->actor, enemy->actor.type);
                enemy->actor.inst = frogObj;
                if (gameInfo.level != PIRATESBOSS) {
                    FlashScreen(0, 200, 0, 250, -30);
                }
                flags = enemy->brain.flags;
                TurnEnemyInto(enemy, 0x3A);
                enemy->brain.flags |= 0x10;
                CreateStarCircle(&enemy->actor.pos, 5, 19);
                if (flags & 0x40) {
                    enemy->brain.flags |= 0x40;
                }
                if (flags & 0x80) {
                    enemy->brain.flags |= 0x80;
                }
                if (ballControl.enemy == &enemy->actor) {
                    EnemyDropBall();
                }
                if (handControl.attachedTo == &enemy->actor) {
                    EnemyDropHand();
                }
                if (gameInfo.level == PIRATESBOSS) {
                    enemy->brain.scratch = enemy->actor.state->index;
                } else {
                    enemy->brain.scratch = 0;
                }
                enemy->actor.state->squash = enemy->actor.rot.x = enemy->actor.rotAim.x = enemy->actor.rot.z =
                    enemy->actor.rotAim.z = 0.0f;
                AnimateActor(&enemy->actor, 5, 0, 0, 0.0f);
            }
        }
    }
}

void TurnEnemyInto(Enemy *enemy, s32 type) {
    enemy->actor.phys = &defaultBehaviour[type];
    MakeUniqueActor(&enemy->actor, type);
    InitActorAnim(&enemy->actor);
    AddActor(&enemy->actor);
    ZERO_VECTOR(&enemy->actor.scale);
    InitEnemy(enemy, type);
}

void KillEnemy(Enemy *enemy) {
    Garib *garib;
    Bullet *bullet;
    Debris *debris;
    Vec3 vect;
    s32 platform;
    s32 x;

    AddToScore(enemyStats[enemy->actor.type].score);
    enemy->brain.flags &= ~0x400;
    if (enemy->samples[0].handle) {
        StopContinuousSample(&enemy->samples[0]);
    }
    if (enemy->samples[1].handle) {
        StopContinuousSample(&enemy->samples[1]);
    }
    if ((enemy->brain.flags & 0xC0) == 0x40) {
        enemy->brain.flags |= 0x80;
        if (NMEGaribGroup == NULL) {
            NMEGaribGroup = JallocAlloc(sizeof(GaribGroup), 1, "GaribGroup");
            AddGaribGroup(NMEGaribGroup);
            InitGaribList(&NMEGaribGroup->garibs);
            NMEGaribGroup->startActive = 1;
            NMEGaribGroup->pos = enemy->actor.pos;
            NMEGaribGroup->radius = 30000.0f;
        }
        NMEGaribGroup->suspended = 0;
        garib = JallocAlloc(sizeof(Garib), 1, "Garib");
        AddGarib(NMEGaribGroup, garib);
        SetVector2D(&garib->sprite.pos, &enemy->actor.pos);
        garib->sprite.pos.y = enemy->actor.pos.y + 3.0f;
        garib->mode = 0;
        garib->altitude = Altitude(&garib->sprite.pos, &garib->normal, &platform, NULL);
        garib->active = 12;
        garib->scale = 1.0f;
        garib->scaleTarget = 1.0f;
        if (cheatActivated & 0x2000) {
            InitSpriteAnimation(&garib->sprite, &garibAnimation[3], 0);
        } else {
            InitSpriteAnimation(&garib->sprite, &garibAnimation[garib->type], 0);
        }
        garib->sprite.flags = 32;
        garib->sprite.r = 255;
        garib->sprite.g = 255;
        garib->sprite.b = 255;
        garib->sprite.offsetX = -garib->sprite.tex->width / 2;
        garib->sprite.offsetY = -garib->sprite.tex->height / 2;
        AddSprite(&garib->sprite, NULL);
        garib->velY = 12.0f;
    }
    if (ball.link == &enemy->actor) {
        DropBall(5);
        ball.flags |= 0x18;
    }
    enemy->actor.active = 2;
    if (enemy->brain.spellTimer[2]) {
        CreateStarCircle(&enemy->actor.pos, 5, 19);
        PlaySample(69, &enemy->actor.pos, 100, 128);
        return;
    }
    if (enemy->actor.type >= 43) {
        AddDebris(TRAIL, &enemy->actor.pos, &zero, NULL, NULL);
    }
    CreateStarCircle(&enemy->actor.pos, 5, 19);
    switch (enemy->actor.type) {
        case 39:
            PrepareSong(gameInfo.world * 4 + 32 + Random(2));
            FreeFGObjects();
            bullet = CreateNewBullet(BOMB_BULLET);
            bullet->actor.pos = enemy->actor.pos;
            bullet->actor.rotQuat = zeroQuat;
            bullet->actor.lifeTimer = bombLife;
            AddBullet(bullet);
            break;
        case 40:
            PrepareSong(gameInfo.world * 4 + 32 + Random(2));
            CreateDustCircleScaled(&enemy->actor.pos, 4, 0, 5.0f);
            CreateStarCircle(&enemy->actor.pos, 5, 8);
            break;
        case 35:
            CreateDustCircleScaled(&enemy->actor.pos, 4, 0, 5.0f);
            break;
        case 28:
            PlaySample(27, &enemy->actor.pos, 100, 128);
        case 37:
            if (enemy->actor.type != BUGLE) {
                PrepareSong(gameInfo.world * 4 + 32 + Random(2));
            }
        case 36:
            if (enemy->actor.type != BUGLE) {
                PlaySample(69, &enemy->actor.pos, 100, 100);
            }
            enemies[0].actor.state->hits--;
            enemies[0].actor.state->hitFadeTimer = 20;
        case 9:
            for (x = 0; x < 10; x++) {
                vect.x = Random(21) - 10;
                vect.z = Random(21) - 10;
                vect.y = Random(10) + 5;
                debris = AddDebris(PLATBIT, &enemy->actor.pos, &vect, NULL, NULL);
                if (enemy->actor.type == SAMTEX && debris != NULL) {
                    debris->sprite.r = 255;
                    debris->sprite.g = 0;
                    debris->sprite.b = 0;
                    debris->sprite.scaleX *= 2;
                    debris->sprite.scaleY *= 2;
                    debris->sprite.anim.def.startScale *= 2;
                    debris->sprite.anim.def.endScale *= 2;
                    debris->flags &= ~1;
                    debris->friction = 1.0f;
                    SCALE_VECTOR_2D(&debris->vel, 1.6);
                    debris->lifeTimer = 9;
                    debris->life = 9;
                    debris->sprite.anim.lifeTimer = 9;
                    debris->sprite.anim.def.life = 9;
                }
            }
            AddDebris(POPA, &enemy->actor.pos, &zero, NULL, NULL);
            AddDebris(POPB, &enemy->actor.pos, &zero, NULL, NULL);
            break;
        case 10:
        case 12:
        case 15:
        case 19:
        case 43:
            PlaySample(69, &enemy->actor.pos, 100, 128);
            break;
    }
}

void EnemyDropBall(void) {
    Enemy *enemy;

    if (ballControl.enemy) {
        enemy = &enemies[ballControl.enemy->state->index];
        if (enemy->brain.attackTime) {
            NMEEndAttack(enemy);
            NMEGetNewInstruction(enemy, enemy->brain.instrIndex, 0);
        }
        if (enemy->actor.type == BABY_SPANK) {
            enemy->actor.anim->loop = 0;
            FlushQueue(&enemy->actor);
            enemy->brain.delayTimer = 40;
            AnimateActor(&enemy->actor, 11, 0, 0, 0);
            AnimateActor(&enemy->actor, 6, 0, 1, 0);
            AnimateActor(&enemy->actor, 1, 0, 1, 0);
        }
        enemy->brain.cooldownTimer = (enemy->brain.cooldownTimer < 60) ? 60 : enemy->brain.cooldownTimer;
        DropBall(1);
    }
}

void EnemyDropHand(void) {
    Enemy *enemy = &enemies[handControl.attachedTo->state->index];

    if (enemy->brain.attackTime) {
        NMEEndAttack(enemy);
        NMEGetNewInstruction(enemy, enemy->brain.instrIndex, 0);
    }
    if (hand.state->mode == HAND_HOPPING || hand.state->mode == HAND_SWISH) {
        DismountEnemy();
    } else {
        DropHand(1);
    }
}

f32 HeightToCentre(Actor *actor) {
    f32 height = 0.0f;
    s32 i;

    for (i = 0; i < actor->obj->numSpheres; i++) {
        if (actor->obj->spheres[i].flags & 0x20) {
            if (actor->obj->spheres[i].targetRadius > height) {
                height = actor->obj->spheres[i].targetRadius;
            }
        } else if (actor->obj->spheres[i].flags & 0x10) {
            if (actor->pos.y - actor->obj->spheres[i].pos.y + actor->obj->spheres[i].targetRadius > height) {
                height = actor->pos.y - actor->obj->spheres[i].pos.y + actor->obj->spheres[i].targetRadius;
            }
        } else {
            if (actor->obj->spheres[i].targetRadius - actor->obj->spheres[i].pos.y > height) {
                height = actor->obj->spheres[i].targetRadius - actor->obj->spheres[i].pos.y;
            }
        }
    }
    return height;
}

void NMEHitByBlastRing(Enemy *enemy) {
    switch (enemy->actor.type) {
        case 27:
            if (ball.link == &enemy->actor) {
                enemy->brain.flags |= 8;
            } else {
                enemy->brain.spellTimer[0] = 0;
                ActivateSpell(&enemy->actor, 0, 100);
            }
            break;
        case 19:
            if (enemy->actor.obj->onGround) {
                enemy->brain.flags |= 8;
            }
            break;
        case 30:
        case 41:
            enemy->brain.flags |= 8;
            break;
        case 43:
            KillEnemy(enemy);
            break;
        default:
            if (enemy->actor.obj->onGround || enemy->actor.type == KNIGHT) {
                if (enemy->actor.type >= 44) {
                    KillEnemy(enemy);
                } else {
                    enemy->brain.spellTimer[0] = 0;
                    ActivateSpell(&enemy->actor, 0, 100);
                }
            }
            break;
    }
}

#define DEATH_BOLT(from, to)                                                    \
    r = deathR + Random(deathRandom * 2 + 1) - deathRandom;                     \
    g = deathG + Random(deathRandom * 2 + 1) - deathRandom;                     \
    b = deathB + Random(deathRandom * 2 + 1) - deathRandom;                     \
    r = MIN(MAX(r, 0), 255);                                                    \
    g = MIN(MAX(g, 0), 255);                                                    \
    b = MIN(MAX(b, 0), 255);                                                    \
    AddLightning(from, to, r, g, b, deathSegs, deathLife, 6, deathJag, deathThick)

void NMEHandleDeathSpell(Enemy *enemy) {
    Vec3 tempVect[6];
    s32 r;
    s32 g;
    s32 b;
    s32 j;

    SCALE_VECTOR(&enemy->actor.scale, 0.94);
    if (enemy->actor.shadow != NULL) {
        enemy->actor.shadow->shadowSize *= 0.94;
    }
    for (j = 0; j < enemy->actor.obj->numSpheres; j++) {
        enemy->actor.obj->spheres[j].targetRadius *= 0.96;
    }
    tempVect[0].x = 0.0f;
    tempVect[0].y = 0.0f;
    tempVect[0].z = 1.0f;
    RotateVector2D(&tempVect[0], &tempVect[0], (f32)frameCount / 6.0);
    tempVect[1] = tempVect[0];
    ADD_TO_VECTOR(&tempVect[0], &enemy->actor.pos);
    SCALE_VECTOR(&tempVect[1], enemy->actor.radius * 1.5);
    ADD_TO_VECTOR(&tempVect[1], &enemy->actor.pos);
    DEATH_BOLT(&tempVect[1], &tempVect[0]);
    tempVect[2].x = 0.0f;
    tempVect[2].y = 1.0f;
    tempVect[2].z = 0.0f;
    RotateVector2DXYZ(&tempVect[2], &tempVect[2], (f32)frameCount / 4.0, 0);
    tempVect[3] = tempVect[2];
    ADD_TO_VECTOR(&tempVect[2], &enemy->actor.pos);
    SCALE_VECTOR(&tempVect[3], enemy->actor.radius * 1.5);
    ADD_TO_VECTOR(&tempVect[3], &enemy->actor.pos);
    DEATH_BOLT(&tempVect[3], &tempVect[2]);
    tempVect[4].x = 1.0f;
    tempVect[4].y = 0.0f;
    tempVect[4].z = 0.0f;
    RotateVector2DXYZ(&tempVect[4], &tempVect[4], (f32)frameCount / 5.0, 2);
    tempVect[5] = tempVect[4];
    ADD_TO_VECTOR(&tempVect[4], &enemy->actor.pos);
    SCALE_VECTOR(&tempVect[5], enemy->actor.radius * 1.5);
    ADD_TO_VECTOR(&tempVect[5], &enemy->actor.pos);
    DEATH_BOLT(&tempVect[5], &tempVect[4]);
    DEATH_BOLT(&tempVect[5], &tempVect[3]);
    DEATH_BOLT(&tempVect[3], &tempVect[1]);
    DEATH_BOLT(&tempVect[5], &tempVect[1]);
}

void DoDizzyStars(Vec3 *centre) {
    Vec3 tempVect;

    tempVect.x = 0.0f;
    tempVect.z = 15.0f;
    tempVect.y = SineWave(starfreq, frameCount, 0) * 5.0f;
    RotateVector2D(&tempVect, &tempVect, (f32)frameCount / 3.0);
    tempVect.x += centre->x;
    tempVect.y += centre->y;
    tempVect.z += centre->z;
    AddDebris(STAR, &tempVect, &zero, 0, NULL);
    tempVect.x = 0.0f;
    tempVect.z = -15.0f;
    tempVect.y = -SineWave(starfreq, frameCount, 0) * 5.0f;
    RotateVector2D(&tempVect, &tempVect, (f32)frameCount / 3.0);
    tempVect.x += centre->x;
    tempVect.y += centre->y;
    tempVect.z += centre->z;
    AddDebris(STAR, &tempVect, &zero, 0, NULL);
}

INCLUDE_ASM("asm/nonmatchings/enemy", pad_8019735C);

INCLUDE_RODATA("asm/nonmatchings/enemy", pad_8010A47C);
