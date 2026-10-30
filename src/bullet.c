#include "common.h"
#include "ident.h"
#include "bullet.h"
#include "debris.h"
#include "sound.h"
#include "game.h"
#include "maths.h"
#include "camera.h"
#include "light.h"
#include "hand.h"
#include "water.h"
#include "collect.h"
#include "jalloc.h"
#include "ball.h"
#include "macros.h"
#include "footprint.h"
#include "actor.h"
#include "overlay.h"
#include "quake.h"
#include "anim.h"
#include "collision.h"
#include "nmeprocess2.h"
#include "object.h"
#include "spell.h"

extern void *icicleBit;
extern void *boulderBit;
extern Texture *blastTex;
extern s32 spaceBossShield;
extern s32 spellDuration;
extern Actor *foregroundObjects[];
void ActivateSpell(Actor *actor, s16 spell, s16 duration);

void HurtHand(s32 type, s32 damage);
void GainGarib(Vec3 *pos, s32 pitch, s32 fx);

extern Vec3 zero;
extern f32 tongueRad;
extern s16 tTime1;
extern s32 numBubbles;
extern s32 cheatActivated;
void AnimateActor(Actor *actor, s32 animNum, s32 loop, s32 queue, f32 speed);
extern s32 numSmallBubbles;
extern void *snowBallBit;
extern f32 waterBulLimit;
extern f32 smallbubbscale;
extern f32 bubbscale;
extern f32 bubbsquash;
extern u8 spellColour[][2][4];
void SquashActor(Actor *actor, Vec3 *dir, f32 power, f32 maxSquash);
extern s32 frameCount;
extern f32 bovradius;
extern f32 bovradius2;

void DropHand(s32 timer);
// s16 colours: MaintainBulletBomb passes them without u8 masking.
void FlashScreen(s16 r, s16 g, s16 b, s16 fadeVal, s16 fadeStep);
void MusHandleSetFreqOffset(s32 handle, f32 offset);
extern f32 spiderfactor;
extern f32 spiderfactor2;
extern f32 spiderfactor3;
extern Quat vertQ;
extern f32 bombLife;
extern f32 bombScaleRate;
extern f32 bombFac;
extern f32 bombScaleFac;
extern f32 bombXluLimit;
extern ObjectInstance *bombExplosion;
extern f32 handInertia;
void KillHand(s32 type);
void ReleaseActor(Actor *actor, s32 flags);
s32 CheckActorForWater(Actor *aPtr, Water *ptr, s32 wake);

void InitSpriteAnimation(GameSprite *sprite, SpriteAnim *temp, s32 lifespan);

extern BulletInfo standardBullets[];
extern f32 lasscale;
extern Quat zeroQuat;
void BulletAcquireTarget(Bullet *bullet, Vec3 *pos);

void InitBulletList(void) {
    bulletList.head.next = bulletList.head.prev = &bulletList.head;
    bulletList.count = 0;
}

inline void AddBullet(Bullet *bullet) {
    Bullet *ptr = bulletList.head.next;

    bullet->prev = ptr;
    bullet->next = ptr->next;
    ptr->next->prev = bullet;
    ptr->next = bullet;
    bulletList.count++;
    if (bullet->type == BUBBLE_BULLET || bullet->type == SPACEBALL_BULLET) {
        numBubbles++;
    } else if (bullet->type == SMALL_BUBBLE_BULLET || bullet->type == BOMB2_BULLET) {
        numSmallBubbles++;
    }
}

void SubBullet(Bullet *bullet) {
    s32 i;

    if (bullet->next) {
        if (bullet->type == BUBBLE_BULLET || bullet->type == SPACEBALL_BULLET) {
            numBubbles--;
        } else if (bullet->type == SMALL_BUBBLE_BULLET || bullet->type == BOMB2_BULLET) {
            numSmallBubbles--;
        }
        bullet->prev->next = bullet->next;
        bullet->next->prev = bullet->prev;
        bulletList.count--;
        for (i = 0; i < standardBullets[bullet->type].numSprites; i++) {
            SubSprite(&bullet->sprites[i]);
        }
        StopContinuousSample(&bullet->sample);
        ReleaseActor(&bullet->actor, -1);
        bullet->next = NULL;
        JallocFree(&bullet);
    }
}

Bullet *FireBulletFromActor(Actor *actor, s16 type, s16 flash) {
    Vec3 tempVect;
    Bullet *bullet;
    BulletInfo *stdBul = &standardBullets[type];
    s32 j;
    s32 from;
    s32 to;
    s32 gun;

    if (actor->state != NULL && actor->state->index != -1) {
        gun = enemies[actor->state->index].brain.fireState[1];
        if (enemies[actor->state->index].brain.fireState[0] == 2) {
            to = standardBullets[type].numShots;
            from = 0;
        } else {
            from = gun;
            to = from + 1;
            if (enemies[actor->state->index].brain.fireState[0] == 1) {
                enemies[actor->state->index].brain.fireState[1]++;
                if (enemies[actor->state->index].brain.fireState[1] >= standardBullets[type].numShots) {
                    enemies[actor->state->index].brain.fireState[1] = 0;
                }
            }
        }
    } else {
        if (type == 1) {
            from = 0;
            to = standardBullets[1].numShots;
        } else {
            from = 0;
            to = 1;
        }
    }
    for (j = from; j < to; j++) {
        bullet = CreateNewBullet(type);
        bullet->actor.link = actor;
        if (actor == &hand) {
            bullet->spell = player.castSpell;
            bullet->spellType = player.castSpellType;
            player.castSpell = 15;
            bullet->spellTime = handControl.castSpellTime;
        } else {
            bullet->spell = 15;
        }
        if (stdBul->flags2 & 2) {
            RotateVector2D(&bullet->actor.vel, &stdBul->vel[j], actor->rot.y);
        } else if (stdBul->flags2 & 1) {
            RotateVectorByQuaternion(&bullet->actor.vel, &stdBul->vel[j], &actor->rotQuat);
        } else {
            bullet->actor.vel = stdBul->vel[j];
        }
        if (stdBul->flags2 & 4) {
            bullet->actor.vel.x += (Random(11) - 5) / 20.0f;
            bullet->actor.vel.y += (Random(11) - 5) / 20.0f;
            bullet->actor.vel.z += (Random(11) - 5) / 20.0f;
        }
        if (stdBul->flags2 & 0x40) {
            tempVect.x = 0.0f;
            tempVect.y = 0.0f;
            tempVect.z = stdBul->offset[j].z;
            RotateVectorByQuaternion(&tempVect, &tempVect, &actor->rotQuat);
            COPY_VECTOR(&bullet->actor.pos, &actor->obj->spheres[(s32)stdBul->offset[j].x].pos);
            ADD_TO_VECTOR(&bullet->actor.pos, &tempVect);
        } else {
            if (actor == &hand && hand.state->mode == HAND_LOCATE) {
                tempVect.x = 0.0f;
                tempVect.y = 2.0f;
                tempVect.z = 8.0f;
                RotateVectorByQuaternion(&tempVect, &tempVect, &actor->rotQuat);
            } else {
                RotateVectorByQuaternion(&tempVect, &stdBul->offset[j], &actor->rotQuat);
            }
            SCALE_VECTOR(&tempVect, actor->scale.x / 0.05);
            ADD_VECTORS(&bullet->actor.pos, &actor->pos, &tempVect);
        }
        ADD_TO_VECTOR(&bullet->actor.pos, &bullet->actor.vel);
        if (stdBul->flags2 & 8) {
            ADD_TO_VECTOR(&bullet->actor.vel, &actor->vel);
        }
        InitBulletSpritesOrMesh(bullet, actor, j);
        BulletAcquireTarget(bullet, &actor->pos);
        if (actor == &hand && bullet->spellType != 15) {
            bullet->sprites[0].r = spellColour[bullet->spellType + 1][0][0];
            bullet->sprites[0].g = spellColour[bullet->spellType + 1][0][1];
            bullet->sprites[0].b = spellColour[bullet->spellType + 1][0][2];
        }
        if (flash && j == from) {
            AddBulletFlash(&bullet->actor.pos);
        }
        AddBullet(bullet);
    }
    return bullet;
}

void InitBulletSpritesOrMesh(Bullet *bullet, Actor *actor, s16 num) {
    Vec3 tempVect;
    Quat tempQ;
    Quat tempQ2;
    Bullet *bul;
    f32 scale;
    s32 i;

    if (standardBullets[bullet->type].numSprites == 0) {
        bullet->actor.inst = standardBullets[bullet->type].inst;
        switch (bullet->type) {
            case MISSILE_BULLET:
                PlayContinuousSample(&bullet->sample, 36, 70, &bullet->actor.pos, 100);
                bullet->actor.scale = actor->scale;
                bullet->actor.scale.x *= 4.0f;
                bullet->actor.scale.y *= 4.0f;
                bullet->actor.scale.z *= 4.0f;
                SubVector2D(&tempVect, &hand.pos, &bullet->actor.pos);
                if (Random(24) < (enemies[0].actor.state->hits - 1) * 3) {
                    bullet->actor.vel.y = -2.0f;
                    if (tempVect.x > 0.0f) {
                        tempVect.x += 400.0f;
                    } else {
                        tempVect.x -= 400.0f;
                    }
                    bullet->age = -1;
                }
                MakeUnit2D(&tempVect);
                vertQ.w = Aabs(atan2(tempVect.x, tempVect.z));
                GetQuaternionFromRotation(&bullet->actor.rotQuat, &vertQ);
                vertQ.w = Aabs(vertQ.w + PI);
                RotateVector2D(&bullet->actor.vel, &bullet->actor.vel, vertQ.w);
                bullet->actor.radius = standardBullets[26].radius;
                bullet->actor.spinAxis.x = tempVect.x;
                bullet->actor.spinAxis.y = 0.0f;
                bullet->actor.spinAxis.z = tempVect.z;
                bullet->actor.spinSpeed = 0.2f;
                bullet->actor.phys->rotDrag = 1.0f;
                for (i = 0; i < 3; i++) {
                    bul = CreateNewBullet(FLAME_BULLET);
                    bul->actor.pos = bullet->actor.pos;
                    bul->actor.vel = bullet->actor.vel;
                    bul->actor.vel.x += Random(21) - 10;
                    bul->actor.vel.y += Random(21) - 10;
                    InitBulletSpritesOrMesh(bul, NULL, 0);
                    AddBullet(bul);
                    bul->sprites[0].anim.def.startScale = Random(256) + 312;
                    bul->sprites[0].anim.def.endScale = Random(56) + 356;
                    bul->sprites[1].anim.def.startScale = Random(256) + 312;
                    bul->sprites[1].anim.def.endScale = Random(56) + 356;
                    bul->actor.flags &= ~0x20000;
                }
                break;
            case LASERBOLT_BULLET:
                bullet->actor.scale = actor->scale;
                SCALE_VECTOR(&bullet->actor.scale, lasscale);
                SubVector2D(&tempVect, &bullet->actor.pos, &hand.pos);
                MakeUnit2D(&tempVect);
                vertQ.w = Aabs(atan2(tempVect.x, tempVect.z));
                GetQuaternionFromRotation(&bullet->actor.rotQuat, &vertQ);
                bullet->actor.phys->rotDrag = 1.0f;
                bullet->actor.spinAxis.x = tempVect.x;
                bullet->actor.spinAxis.y = 0.0f;
                bullet->actor.spinAxis.z = tempVect.z;
                bullet->actor.spinSpeed = 1.0f;
                bullet->actor.phys->rotDrag = 1.0f;
                break;
            case CANNONBALL_BULLET:
                bullet->actor.scale.x = enemyStats[CANNONBALL].scale;
                bullet->actor.scale.y = enemyStats[CANNONBALL].scale;
                bullet->actor.scale.z = enemyStats[CANNONBALL].scale;
                GiveActorRandomQRotVel(&bullet->actor);
                bullet->actor.rotQuat = zeroQuat;
                bullet->actor.spinSpeed = 2.0f;
                bullet->actor.phys->rotDrag = defaultBehaviour[0].rotDrag;
                break;
            case SPACEBALL_BULLET:
                PlaySample(27, &bullet->actor.pos, 80, (Random(24) + 116));
            case BALL_BULLET:
                bullet->actor.scale.z = bullet->actor.scale.y = bullet->actor.scale.x =
                    bullet->actor.obj->spheres[0].radius * 0.05 / 9.0;
                GiveActorRandomQRotVel(&bullet->actor);
                bullet->actor.spinSpeed = 2.0f;
                GetQuaternionFromRotation(&bullet->actor.rotQuat, (Quat *)&bullet->actor.spinAxis);
                bullet->actor.type = BALL;
                bullet->actor.spinSpeed = 1.0f;
                *bullet->actor.phys = defaultBehaviour[0];
                if (bullet->type == BALL_BULLET) {
                    bullet->actor.friction = bullet->actor.phys->friction;
                } else {
                    bullet->actor.phys->moveSpeed = bullet->actor.phys->maxSpeed = 1000.0f;
                }
                break;
            case PIE_BULLET:
                bullet->actor.rotQuat = actor->rotQuat;
                bullet->actor.scale = actor->scale;
                break;
            case BOMB2_BULLET:
                bullet->actor.vel.y += (f32)Random(24) / 3.0;
                bullet->actor.state->hits = 2;
                bullet->age = Random(8) * 3;
                SubVector2D(&tempVect, &bullet->actor.pos, &hand.pos);
                MakeUnit2D(&tempVect);
                vertQ.w = Aabs(atan2(tempVect.x, tempVect.z) + PI / 2);
                GetQuaternionFromRotation(&bullet->actor.rotQuat, &vertQ);
            case BOMB_BULLET:
                InitActorStructures(&bullet->actor, 4);
                InitActorAnim(&bullet->actor);
                if (bullet->type == BOMB_BULLET) {
                    AnimateActor(&bullet->actor, 0, 1, 0, 0);
                    bullet->actor.rotQuat = actor->rotQuat;
                } else {
                    bullet->actor.anim->time = 1.0f;
                }
                bullet->actor.phys->bounce = 0.7f;
                bullet->actor.scale = actor->scale;
                bullet->actor.scale.x *= 4.0f;
                bullet->actor.scale.y *= 4.0f;
                bullet->actor.scale.z *= 4.0f;
                break;
            case BOVVA_BULLET:
                bullet->actor.rotQuat = actor->rotQuat;
                bullet->actor.phys->bounce = 0.0f;
                bullet->actor.scale = actor->scale;
                break;
            case CLAW_BULLET:
                bullet->actor.rotQuat = actor->rotQuat;
                bullet->actor.phys->bounce = 0.0f;
                bullet->actor.scale = actor->scale;
                if (num == 0) {
                    tempQ.x = 0.0f;
                    tempQ.y = 0.0f;
                    tempQ.z = 1.0f;
                    tempQ.w = 3.1415927f;
                    GetQuaternionFromRotation(&tempQ2, &tempQ);
                    QuaternionMultiply(&tempQ, &bullet->actor.rotQuat, &tempQ2);
                    bullet->actor.rotQuat = tempQ;
                }
                break;
            case BUBBLE_BULLET:
                bullet->actor.state->hits = 3;
                bullet->actor.phys->bounce = 1.0f;
                bullet->actor.phys->maxSquash = 0.3f;
                bullet->actor.phys->squashScale = 3.0f;
                bullet->actor.phys->squashMin = 0.01f;
                bullet->actor.phys->squashPeriod = -40.0f;
                bullet->actor.phys->squashDecay = 0.95f;
                bullet->actor.scale = actor->scale;
                bullet->actor.scale.x *= 0.3;
                bullet->actor.scale.y *= 0.3;
                bullet->actor.scale.z *= 0.3;
                bullet->actor.rotQuat = zeroQuat;
                SquashActor(&bullet->actor, (Vec3 *)&vertQ, 0.1f, bullet->actor.phys->maxSquash);
                bullet->actor.obj->spheres[0].radius = bullet->actor.obj->spheres[0].targetRadius / 8.0f;
                bullet->actor.phys->radiusRate = 0.5f;
                bullet->actor.shadow->shadowAlpha = 0;
                bullet->actor.shadow->shadowTarget = 180;
                bullet->actor.phys->rotDrag = 1.0f;
                bullet->actor.spinAxis.x = 1.0f;
                bullet->actor.spinAxis.y = 0.0f;
                bullet->actor.spinAxis.z = 0.0f;
                bullet->actor.spinSpeed = 0.1f;
                bullet->actor.link = &hand;
                break;
            case SMALL_BUBBLE_BULLET:
                bullet->actor.phys->bounce = 1.0f;
                bullet->actor.phys->maxSquash = 0.4f;
                bullet->actor.phys->squashScale = 3.0f;
                bullet->actor.phys->squashMin = 0.01f;
                bullet->actor.phys->squashPeriod = -40.0f;
                bullet->actor.phys->squashDecay = 1.0f;
                bullet->actor.scale = actor->scale;
                bullet->actor.scale.x *= 0.3;
                bullet->actor.scale.y *= 0.3;
                bullet->actor.scale.z *= 0.3;
                bullet->actor.rotQuat = zeroQuat;
                tempVect.x = Random(5) + 1;
                tempVect.y = Random(11) - 5;
                tempVect.z = Random(11) - 5;
                MakeUnit(&tempVect);
                SquashActor(&bullet->actor, &tempVect, 0.07f, bullet->actor.phys->maxSquash);
                bullet->actor.obj->spheres[0].radius = bullet->actor.obj->spheres[0].targetRadius / 6.0f;
                bullet->actor.phys->radiusRate = 0.2f;
                bullet->actor.shadow->shadowAlpha = 0;
                bullet->actor.shadow->shadowTarget = 180;
                bullet->actor.phys->rotDrag = 1.0f;
                bullet->actor.spinAxis.x = 1.0f;
                bullet->actor.spinAxis.y = 0.0f;
                bullet->actor.spinAxis.z = 0.0f;
                bullet->actor.spinSpeed = 0.1f;
                break;
            case FIREBALL_BULLET:
                bullet->actor.phys->bounce = 0.0f;
                bullet->actor.phys->slideLimit = 0.99f;
                bullet->actor.rotQuat = zeroQuat;
                bullet->actor.scale.x = 0.43f;
                bullet->actor.scale.y = 0.43f;
                bullet->actor.scale.z = 0.43f;
                bullet->actor.shadow->shadowSize = 14.0f;
                break;
            case SNOWBALL_BULLET:
                PlayContinuousSample(&bullet->sample, 103, 120, &bullet->actor.pos, 80);
            case BOULDER_BULLET:
                if (bullet->type == BOULDER_BULLET) {
                    PlayContinuousSample(&bullet->sample, 74, 120, &bullet->actor.pos, 60);
                }
                bullet->actor.phys->bounce = -1.0f;
                bullet->actor.phys->slideLimit = 0.99f;
                bullet->actor.rotQuat = zeroQuat;
                bullet->actor.scale.x = 0.43f;
                bullet->actor.scale.y = 0.43f;
                bullet->actor.scale.z = 0.43f;
                bullet->actor.shadow->shadowSize = 14.0f;
                break;
            case ICICLE_BULLET:
                bullet->actor.rotQuat = zeroQuat;
                bullet->actor.shadow->shadowSize = 20.0f;
                break;
            case FLOATY_BULLET:
                bullet->actor.rotQuat = zeroQuat;
                bullet->actor.scale.x = 0.05f;
                bullet->actor.scale.y = 0.05f;
                bullet->actor.scale.z = 0.05f;
                break;
        }
        bullet->actor.flags |= 0x14000;
        return;
    }
    if (bullet->actor.flags & 0x200000) {
        bullet->actor.phys->bottomDiv = 1.5f;
    }
    for (i = 0; i < standardBullets[bullet->type].numSprites; i++) {
        InitSpriteAnimation(&bullet->sprites[i], standardBullets[bullet->type].sprites[i].anim, (u16)bullet->actor.lifeTimer);
        bullet->sprites[i].offsetX = -bullet->sprites[i].tex->width / 2;
        bullet->sprites[i].offsetY = -bullet->sprites[i].tex->height / 2;
        bullet->sprites[i].pos = bullet->actor.pos;
        bullet->sprites[i].r = standardBullets[bullet->type].sprites[i].r;
        bullet->sprites[i].g = standardBullets[bullet->type].sprites[i].g;
        bullet->sprites[i].b = standardBullets[bullet->type].sprites[i].b;
        if (i == 0) {
            AddSprite(&bullet->sprites[0], NULL);
        } else {
            AddSprite(&bullet->sprites[i], &bullet->sprites[i - 1]);
        }
    }
}

Bullet *CreateNewBullet(s16 type) {
    BulletInfo *info = &standardBullets[type];
    s32 flags = ((info->flags & 0x18) ? 0x13 : 0) | 0x11 | ((info->flags & 0x100) ? 0x8 : 0);
    Bullet *bullet;

    bullet = JallocAlloc(sizeof(Bullet), 1, "Bullet");
    InitActorStructures(&bullet->actor, flags);
    if (info->flags & 0x100) {
        bullet->actor.shadow->shadowSize = info->radius * 0.7;
        bullet->actor.shadow->shadowAlpha = 0;
        bullet->actor.shadow->shadowTarget = 255;
        bullet->actor.shadow->shadowStep = 40;
    }
    bullet->age = 0;
    bullet->type = type;
    bullet->actor.lifeTimer = info->life;
    bullet->actor.phys->mass = info->mass;
    bullet->actor.type = BULLET;
    bullet->actor.flags = info->flags;
    bullet->targetFlags = info->targetFlags;
    bullet->actor.phys->maxSpeed = info->maxSpeed;
    if (bullet->actor.obj) {
        bullet->actor.obj->spheres[0].radius = bullet->actor.obj->spheres[0].targetRadius = bullet->actor.radius = info->radius;
    }
    bullet->actor.friction = info->friction;
    bullet->actor.phys->friction = info->friction;
    bullet->actor.phys->bounce = 1.0f;
    bullet->actor.state->index = -1;
    bullet->actor.active = 1;
    return bullet;
}

void DestroyBulletLaserBolt(Actor *actor) {
    Vec3 tempVect;
    s32 i;

    PlaySample(62, &actor->pos, 100, 160);
    for (i = 0; i < 5; i++) {
        tempVect.x = 0.0f;
        tempVect.z = 0.0f;
        tempVect.y = 40.0f;
        RotateVector2DXYZ(&tempVect, &tempVect, (f32)Random(64) / 10.0, 2);
        ADD_TO_VECTOR(&tempVect, &actor->pos);
        AddLightning(&actor->pos, &tempVect, Random(96) - 96, Random(96) - 96, Random(96) - 96, 8, 2, 22, 6.0f, 1.0f);
    }
}

void DestroyBulletSpaceBall(Actor *actor) {
    CreateDustCircleScaled(&actor->pos, 3, 0, 5.0f);
}

void DestroyBulletMissile(Actor *actor) {
    Bullet *bullet;
    s32 i;

    PlaySample(106, &actor->pos, 100, 128);
    for (i = 0; i < 2; i++) {
        bullet = CreateNewBullet(FLAME_BULLET);
        bullet->actor.pos = actor->pos;
        bullet->actor.vel.x = Random(7) - 3;
        bullet->actor.vel.y = Random(7) - 3;
        bullet->actor.vel.z = Random(7) - 3;
        InitBulletSpritesOrMesh(bullet, 0, 0);
        AddBullet(bullet);
        bullet->sprites[0].anim.def.startScale = Random(256) + 196;
        bullet->sprites[0].anim.def.endScale = Random(56) + 220;
        bullet->sprites[1].anim.def.startScale = Random(256) + 196;
        bullet->sprites[1].anim.def.endScale = Random(56) + 220;
        bullet->actor.flags &= ~0x20000;
    }
}

void DestroyBulletCannonBall(Actor *actor) {
    PlaySample(106, &actor->pos, 100, 128);
    CreateDustCircle(&actor->pos, 5, 1.0f);
    if (DistanceBetweenPointsSquared(&hand.pos, &actor->pos) < 1600.0f) {
        HurtHand(HURT_FALL, 1);
    }
}

void DestroyBulletIcicle(Actor *actor) {
    ShatterActor(actor, icicleBit, 5, 1.0f);
}

void DestroyBulletEgg(Actor *actor) {
    PlaySample(22, &actor->pos, 100, 128);
    PlaySample(31, &actor->pos, 50, 128);
}

void DestroyBulletPie(Actor *actor) {
    CreateStarCircle(&actor->pos, 5, 0x13);
    PlaySample(31, &actor->pos, 100, 128);
}

void DestroyBulletBoulder(Actor *actor) {
    ShatterActor(actor, boulderBit, 10, 1.8f);
}

void DestroyBulletFireball(Actor *actor) {
    ShatterActor(actor, boulderBit, 10, 1.8f);
}

void DestroyBulletSnowball(Actor *actor) {
    if (handControl.attachedTo == actor) {
        DropHand(1);
        hand.flags &= ~0x4000;
        cameraInfo.focus = &hand.pos;
        cameraInfo.focusVel = &hand.vel;
        HurtHand(HURT_FALL, 1);
    }
    ShatterActor(actor, snowBallBit, 10, 1.8f);
    PlaySample(136, &actor->pos, 255, 40);
}

void DestroyBulletWater(Actor *actor) {
    if (Random(10) < 5) {
        if (gameInfo.level == ATLANTISBOSS && Random(10) < 5) {
            PlaySample(2, &actor->pos, 80, 150);
        }
        CreateSplashCircle(&actor->pos, 1, 2.0f);
    }
}

void DestroyBulletFighter(Actor *actor) {
    Debris *d = AddDebris(FAIRY_HALF, &actor->prevPos, &zero, 0, 0);

    if (d) {
        d->sprite.anim.def.type = 2;
        d->sprite.scaleX *= 3;
        d->sprite.scaleY *= 3;
        d->lifeTimer = 0;
        d->life = 0;
        d->sprite.anim.def.endScale *= 3;
        d->sprite.anim.def.startScale *= 3;
    }
}

void DestroyBulletBomb2(Actor *actor) {
    DestroyBulletMissile(actor);
}

void DestroyBulletBovva(Actor *actor) {
    Vec3 tempVect;
    f32 ang;
    Debris *d;
    s32 x;

    for (x = 0; x < 6; x++) {
        ang = (f32)Random(63) / 10.0;
        tempVect.x = 4.0f;
        tempVect.y = 10.0f;
        tempVect.z = 0.0f;
        RotateVector2D(&tempVect, &tempVect, ang);
        d = AddDebris(PLATBIT, &actor->pos, &tempVect, 0, 0);
        if (d) {
            d->sprite.r = 255;
            d->sprite.g = 100;
            d->sprite.b = 0;
        }
    }
    FlashScreen(255, 200, 50, 200, -20);
    PlaySample(106, &actor->pos, 255, 128);
    DoExplosion(&actor->pos, bovradius, bovradius2, (Enemy *)actor->link, 25.0f, 0.2f);
    StartEarthquakeAtPoint(&actor->pos, 14, 500.0f, 250.0f, 1, 3);
    AddDebris(POPA, &actor->pos, &zero, 0, 0);
    AddDebris(POPB, &actor->pos, &zero, 0, 0);
}

s32 MaintainBulletMissile(Bullet *bullet) {
    f32 speed;

    if (bullet->target) {
        return FALSE;
    }
    speed = LENGTH_2D(&bullet->actor.vel);
    if (speed != 0.0f) {
        SCALE_VECTOR_2D(&bullet->actor.vel, (speed + standardBullets[bullet->type].accel) / speed);
    }
    return FALSE;
}

s32 MaintainBulletLaserBolt(Actor *bullet) {
    return FALSE;
}

s32 MaintainBulletBoulder(Bullet *bullet) {
    s32 t;

    if (bullet->actor.obj->onGround) {
        t = bullet->sample.volume + 40;
        if (t > 120) {
            t = 120;
        }
        bullet->sample.volume = t;
    } else {
        t = bullet->sample.volume;
        t -= 10;
        if (t < 0) {
            t = 0;
        }
        bullet->sample.volume = t;
    }
    if (bullet->age > 20 && bullet->actor.obj->onGround && !bullet->actor.obj->wasOnGround &&
        bullet->actor.prevVel.y <= -8.0f) {
        return TRUE;
    }
    return FALSE;
}

s32 MaintainBulletFireball(Bullet *bullet) {
    Bullet *bull;

    if (!(frameCount & 1)) {
        bull = CreateNewBullet(CRUMPET_BULLET);
        bull->actor.pos = bullet->actor.pos;
        InitBulletSpritesOrMesh(bull, 0, 0);
        AddBullet(bull);
        bull->actor.vel.x = Random(10) - 5.0f;
        bull->actor.vel.y = -1.0f;
        bull->actor.vel.z = 0.0f;
        bull->sprites[0].anim.def.startScale *= 7;
        bull->sprites[1].anim.def.startScale *= 7;
    }
    if (bullet->actor.obj->onGround && !bullet->actor.obj->wasOnGround) {
        PlaySample(198, &bullet->actor.pos, 100, 128);
    }
    if (bullet->age > 20 && bullet->actor.obj->onGround && !bullet->actor.obj->wasOnGround) {
        return TRUE;
    }
    return FALSE;
}

s32 MaintainBulletSpaceBall(Bullet *bullet) {
    Bullet *bul;

    for (bul = bulletList.head.next; bul != &bulletList.head; bul = bul->next) {
        if ((bul->type == MISSILE_BULLET || bul->type == BOMB2_BULLET) &&
            DistanceBetweenPointsSquared(&bullet->actor.pos, &bul->actor.pos) <=
                (bullet->actor.radius + bul->actor.radius) * (bullet->actor.radius + bul->actor.radius)) {
            if (bul->actor.state->hits) {
                bul->actor.state->hits--;
                AddToOneScaledVector2D(&bul->actor.vel, &bullet->actor.vel, 0.9f);
                PlaySample(119, &bullet->actor.pos, 100, 128);
                return TRUE;
            }
            AddToScore(500);
            bul->actor.lifeTimer = 1;
            return TRUE;
        }
    }
    return FALSE;
}

s32 MaintainBulletBall(Bullet *bullet) {
    GaribGroup *garibGroup;
    Garib *garib;

    if (bullet->actor.pos.y < -80.0f) {
        return TRUE;
    }
    for (garibGroup = garibGroupList.head.prev; garibGroup != &garibGroupList.head; garibGroup = garibGroup->prev) {
        for (garib = garibGroup->garibs.head.prev; garib != &garibGroup->garibs.head; garib = garib->prev) {
            if (garib->active && DistanceBetweenPointsSquared(&bullet->actor.pos, &garib->sprite.pos) <
                                     (bullet->actor.radius + 10.0f) * (bullet->actor.radius + 10.0f)) {
                if (garib->type == EXTRA_LIFE) {
                    GainExtraLife();
                    AddToScore(500);
                    garib->active = 0;
                    SubSprite(&garib->sprite);
                    AddDebris(TRAIL, &garib->sprite.pos, &zero, 0, NULL);
                    AddDebris(SCORE + 4, &garib->sprite.pos, &zero, 0, NULL);
                } else {
                    GainGarib(&garib->sprite.pos, 128, 140);
                    AddToScore(10);
                    garib->active = 0;
                    SubSprite(&garib->sprite);
                    AddDebris(TRAIL, &garib->sprite.pos, &zero, 0, NULL);
                    AddDebris(SCORE, &garib->sprite.pos, &zero, 0, NULL);
                }
            }
        }
    }
    return FALSE;
}

s32 MaintainBulletSnowball(Bullet *bullet) {
    s32 t;

    if (bullet->actor.obj->onGround) {
        t = bullet->sample.volume + 40;
        if (t > 120) {
            t = 120;
        }
        bullet->sample.volume = t;
    } else {
        t = bullet->sample.volume;
        t -= 10;
        if (t < 0) {
            t = 0;
        }
        bullet->sample.volume = t;
    }
    if (handControl.attachedTo != &bullet->actor && (bullet->actor.flags & 0x4000000)) {
        if (ball.link == &hand && Random(100) < 50) {
            bullet->actor.flags |= 0x40000000;
            ReleaseBall();
        } else {
            if (ballControl.snow == 0.0f && !(cheatActivated & 0x80) && ballControl.type != BALL_MODE_CRYSTAL) {
                ActivateCollectable(SNOWBALL, 1);
            }
            ball.obj->spheres[0].targetRadius = MIN(ball.obj->spheres[0].targetRadius + 3.0f, 18.0f);
            ZERO_VECTOR(&ball.vel);
            ZERO_VECTOR(&ball.prevVel);
            return TRUE;
        }
    }
    if (handControl.attachedTo == NULL && (bullet->actor.flags & 0x40000000)) {
        handControl.attachOffset.x = 0.0f;
        handControl.attachOffset.y = 0.0f;
        handControl.attachOffset.z = 13.0f;
        hand.link = &bullet->actor;
        handControl.status = HAND_HELD;
        handControl.dropTimer = 0;
        handControl.releaseTimer = 0;
        handControl.attachTag = -1;
        handControl.attachedTo = &bullet->actor;
        AnimateActor(&hand, HANDANIM_JUMP, 0, 0, 0);
        cameraInfo.focus = &bullet->actor.pos;
        cameraInfo.focusVel = &bullet->actor.vel;
    }
    if (handControl.attachedTo == &bullet->actor) {
        hand.state->mode = HAND_NOTHING;
        hand.anim->current = HANDANIM_JUMP;
        hand.anim->loop = 0;
        hand.rotQuat = bullet->actor.rotQuat;
        hand.flags = (hand.flags | 0x4000) & ~0x10;
    }
    if (bullet->age > 20 && bullet->actor.obj->onGround && !bullet->actor.obj->wasOnGround
        && bullet->actor.prevVel.y <= -8.0f) {
        return TRUE;
    }
    return FALSE;
}

s32 MaintainBulletBomb2(Bullet *bullet) {
    Vec3 dir;
    f32 speed;
    f32 t;
    f32 t2;
    Bullet *bul;
    s32 r;

    if (bullet->actor.obj->planes[0] != NULL && !bullet->actor.obj->resting[0]) {
        PlaySample(190, &bullet->actor.pos, MIN(ABS(bullet->actor.prevVel.y * 8.0f), 100.0f),
                   (Random(60) + 50));
    }
    if (bullet->actor.anim->speed == 0.0f) {
        SubVector2D(&dir, &bullet->actor.pos, &hand.pos);
        MakeUnit2D(&dir);
        vertQ.w = Aabs(atan2(dir.x, dir.z) + PI / 2);
        GetQuaternionFromRotation(&bullet->actor.rotQuat, &vertQ);
        if (-standardBullets[28].targetDelay < bullet->age) {
            AnimateActor(&bullet->actor, 0, 1, 0, 0.0f);
            PlayContinuousSample(&bullet->sample, 43, 20, &bullet->actor.pos, 240);
        }
    } else {
        speed = LENGTH_2D(&bullet->actor.vel);
        t = speed * spiderfactor2;
        bullet->actor.anim->speed = speed / spiderfactor;
        MusHandleSetFreqOffset(bullet->sample.handle, (t < 159.0f) ? (t - 32.0f) / 10.0 : 12.7f);
        t2 = speed * spiderfactor3 + 20.0f;
        r = (t2 < 60.0f) ? (s32)t2 : 60;
        bullet->sample.volume = r;
    }
    if (bullet->actor.state->hits < 2 && (bullet->actor.state->hits == 0 || Random(20) < 7)) {
        bul = CreateNewBullet(SMOKE_BULLET);
        bul->actor.pos = bullet->actor.pos;
        bul->actor.pos.y += 25.0f;
        bul->actor.vel.x = bullet->actor.vel.x + (Random(4) - 2);
        bul->actor.vel.z = bullet->actor.vel.z + (Random(4) - 2);
        bul->actor.vel.y = bullet->actor.vel.y + 7.0f;
        InitBulletSpritesOrMesh(bul, 0, 0);
        bul->sprites[0].scaleX = bul->sprites[0].scaleY = bul->sprites[0].anim.def.startScale = bul->sprites[0].anim.def.endScale = Random(56) + 156;
        bul->sprites[0].anim.def.frameDelay = 32;
        AddBullet(bul);
    }
    if (DistanceBetweenPoints2D(&bullet->actor.pos, &hand.pos) < hand.radius) {
        StartEarthquake(10, 50, 25, 1, 2);
        HurtHand(HURT_GO_BACKWARDS, 1);
        return TRUE;
    }
    return FALSE;
}

s32 MaintainBulletBomb(Bullet *bullet) {
    Vec3 toHand;
    Vec3 toBall;
    f32 handDist;
    f32 ballDist;
    f32 half;
    f32 t;
    s16 r;
    s16 g;
    s16 b;

    if (bullet->actor.lifeTimer <= bombLife) {
        if (bullet->actor.lifeTimer == bombLife) {
            PlaySample(99, &bullet->actor.pos, 255, 128);
            StartEarthquakeAtPoint(&bullet->actor.pos, 50, 3000.0f, 0.0f, 1, 6);
            bullet->actor.scale.z = 0.0f;
            bullet->actor.scale.y = 0.0f;
            bullet->actor.scale.x = 0.0f;
            bullet->actor.inst = bombExplosion;
            bullet->actor.flags &= ~0x19;
        }
        half = bombLife / 2.0f;
        if (half < bullet->actor.lifeTimer) {
            t = bombLife * 7.0f;
            if (t / 8.0f < bullet->actor.lifeTimer) {
                r = 255;
                g = 255;
                b = 255;
            } else {
                r = 255;
                g = (bullet->actor.lifeTimer * 8) / t * 255.0f;
                b = (bullet->actor.lifeTimer - half) * 16.0f / t * 255.0f;
            }
            FlashScreen(r, g, b, bullet->actor.lifeTimer / bombLife * 255.0f, -30);
        }
        if (gameInfo.level == SPACEBOSS3) {
            bullet->actor.scale.x += (2.0f * bombScaleRate) / (bombLife * bombFac - bullet->actor.lifeTimer);
            bullet->actor.lifeTimer--;
        } else {
            bullet->actor.scale.x += bombScaleRate / (bombLife * bombFac - bullet->actor.lifeTimer);
        }
        bullet->actor.scale.z = bullet->actor.scale.x;
        bullet->actor.scale.y = bullet->actor.scale.x * 0.9;
        bullet->actor.alpha = bullet->actor.lifeTimer / bombLife * 100.0f;
        bullet->actor.rot.y = Aabs(bullet->actor.rot.y + 0.2);
        GetQuaternionFromXYZRot(&bullet->actor.rotQuat, &bullet->actor.rot);
        if (gameInfo.level != SPACEBOSS3) {
            SUB_VECTORS(&toHand, &hand.pos, &bullet->actor.pos);
            handDist = LENGTH_2D(&toHand);
            SUB_VECTORS(&toBall, &ball.pos, &bullet->actor.pos);
            ballDist = LENGTH_2D(&toBall);
            if (handControl.deathTimer == 0 && handDist <= bullet->actor.scale.x * bombScaleFac) {
                if (bombXluLimit <= bullet->actor.alpha) {
                    KillHand(HURT_GO_BACKWARDS);
                }
                AddToOneScaledVector(&hand.vel, &toHand, 14.0f / handDist);
                hand.vel.y = (hand.vel.y < 5.0f) ? 5.0f : hand.vel.y;
                hand.flags |= 0x01002010;
                hand.friction = handInertia;
                hand.pos.y += 1.0f;
            }
            if (ballDist <= bullet->actor.scale.x * bombScaleFac) {
                if (bombXluLimit <= bullet->actor.alpha) {
                    BurstBall();
                }
                AddToOneScaledVector(&ball.vel, &toBall, 7.0f / ballDist);
                ball.vel.y = (ball.vel.y < 5.0f) ? 5.0f : ball.vel.y;
            }
        }
    } else {
        bullet->actor.anim->speed = ((standardBullets[15].life - bullet->actor.lifeTimer) * 0.03 < 0.6) ? 0.6f : (standardBullets[15].life - bullet->actor.lifeTimer) * 0.03;
        if (TriggerAtFrame(10.0f, &bullet->actor)) {
            PlaySample(182, &bullet->actor.pos, 255, 128);
        }
    }
    return FALSE;
}

s32 MaintainBulletWater(Bullet *bullet) {
    Water *ptr;

    if (gameInfo.level == ATLANTISBOSS && bullet->actor.pos.y < waterBulLimit) {
        bullet->actor.lifeTimer = 1;
    }
    for (ptr = waterList.head.next; ptr != &waterList.head; ptr = ptr->next) {
        if (CheckActorForWater(&bullet->actor, ptr, 0)) {
            return TRUE;
        }
    }
    return FALSE;
}

s32 MaintainBulletSmallBubble(Bullet *bullet) {
    f32 dist = SineWave(12.0f, 0.0f, frameCount) / 20.0f;

    if (dist > 0.0f) {
        dist *= 1.2;
    }
    bullet->actor.vel.y += dist;
    RotateVector2D(&bullet->actor.vel, &bullet->actor.vel, (Random(21) - 10) / 100.0f);
    SCALE_VECTOR_2D(&bullet->actor.vel, standardBullets[bullet->type].accel + 1.0f);
    if (bullet->actor.scale.x != smallbubbscale) {
        bullet->actor.scale.x = bullet->actor.scale.y = bullet->actor.scale.z =
            bullet->actor.obj->spheres[0].radius * smallbubbscale / bullet->actor.obj->spheres[0].targetRadius;
        bullet->actor.shadow->shadowSize = bullet->actor.radius * 0.85;
    }
    return FALSE;
}

s32 MaintainBulletBubble(Bullet *bullet) {
    Vec3 tempVect;
    f32 dist;

    bullet->actor.alpha = 100;
    if (handControl.flags & 1) {
        bullet->target = NULL;
        dist = SineWave(12.0f, 0.0f, frameCount) / 10.0f;
        if (dist > 0.0f) {
            dist *= 2.5;
        }
        bullet->actor.vel.y += dist;
        if (LENGTH_2D(&bullet->actor.vel) == 0.0f) {
            bullet->actor.vel.x = 0.1f;
        }
        RotateVector2D(&bullet->actor.vel, &bullet->actor.vel, (Random(21) - 10) / 100.0f);
        SCALE_VECTOR_2D(&bullet->actor.vel, standardBullets[bullet->type].accel + 1.0f);
    } else {
        bullet->target = &hand;
    }
    if (bullet->actor.pos.y > 320.0f) {
        return TRUE;
    }
    if (bullet->actor.scale.x != bubbscale) {
        bullet->actor.scale.x = bullet->actor.scale.y = bullet->actor.scale.z =
            bullet->actor.obj->spheres[0].radius * bubbscale / bullet->actor.obj->spheres[0].targetRadius;
        bullet->actor.shadow->shadowSize = bullet->actor.radius * 0.85;
    }
    if (bullet->actor.state->squash < bubbsquash) {
        bullet->actor.state->squash = bubbsquash;
    }
    if (bullet->actor.radius > hand.radius && !(handControl.flags & 1)) {
        dist = DistanceBetweenPoints(&bullet->actor.pos, &hand.pos);
        if (dist < bullet->actor.radius - hand.radius / 2.0f) {
            bullet->target = NULL;
            if (bullet->actor.obj->contactFlags) {
                bullet->actor.vel.y = bullet->actor.pos.y - bullet->actor.prevPos.y;
            }
            handControl.joinTimer = 2;
            handControl.flags |= 1;
            if (bullet->targetFlags & 2) {
                bullet->targetFlags &= ~2;
                bullet->actor.vel.y = 0.0f;
            }
            AddScaledVector2D(&bullet->actor.vel, &bullet->actor.vel, 0.9f, &hand.vel, 0.1f);
            CollideActorInsideSphere(&hand, &bullet->actor.pos, bullet->actor.radius);
            if (hand.obj->onGround) {
                SetVector2D(&hand.vel, &bullet->actor.vel);
                SCALE_VECTOR_2D(&hand.vel, 0.6);
                hand.vel.y = bullet->actor.vel.y;
            }
            if (hand.state->mode == HAND_SLAM2ST && handControl.actionTime == 1) {
                SquashActor(&bullet->actor, (Vec3 *)&vertQ, 3.0f, bullet->actor.phys->maxSquash);
                bullet->actor.state->squashTimer = bullet->actor.phys->squashPeriod * -3.141592654 / 10.0;
                bullet->actor.alpha = 200;
                bullet->actor.flags |= 0x1000000;
                bullet->actor.vel.y -= 5.0f;
                if (--bullet->actor.state->hits == 0) {
                    bullet->actor.lifeTimer = 1;
                }
            } else if (!(bullet->actor.flags & 0x1000000) || bullet->actor.vel.y >= -0.5) {
                bullet->actor.flags &= ~0x1000000;
                dist = SineWave(12.0f, 0.0f, frameCount) / 10.0f;
                if (dist > 0.0f) {
                    dist *= 2.5;
                }
                bullet->actor.vel.y += dist;
            }
        } else if (dist < bullet->actor.radius + hand.radius) {
            if (hand.state->mode == HAND_SLAM || hand.state->mode == HAND_SLAM2ST) {
                handControl.flags |= 1;
            }
            if (ball.link == &hand && !handControl.releaseTimer) {
                ApplyAction(HAND_RELEASE);
            }
            SUB_VECTORS(&tempVect, &hand.pos, &bullet->actor.pos);
            MakeUnit(&tempVect);
            SCALE_VECTOR(&tempVect, 1.5);
            ADD_VECTORS(&bullet->actor.vel, &tempVect, &hand.vel);
            FlipVector(&tempVect);
            if (hand.state->mode == HAND_SLAM) {
                hand.vel = tempVect;
                return FALSE;
            }
            AddToOneScaledVector2D(&hand.vel, &tempVect, 0.2f);
        }
    }
    return FALSE;
}

s32 MaintainBulletBovva(Bullet *bullet) {
    Vec3 tempVect = { 0.0f, 4.0f, 0.0f };
    Vec3 tempPos;

    if (bullet->actor.obj->planes[0]) {
        ZERO_VECTOR(&bullet->actor.vel);
        if (!(bullet->actor.flags & 0x1000000)) {
            bullet->actor.phys->rotDrag = 1.06f;
            bullet->actor.lifeTimer = 20;
            bullet->actor.flags |= 0x1000000;
            bullet->actor.spinAxis.x = Random(21) - 10;
            bullet->actor.spinAxis.y = Random(21) - 10;
            bullet->actor.spinAxis.z = Random(21) - 10;
            MakeUnit(&bullet->actor.spinAxis);
            bullet->actor.spinSpeed = 0.2f;
        } else {
            bullet->actor.spinSpeed = -bullet->actor.spinSpeed;
        }
        tempVect.x = Random(5) - 2;
        tempVect.z = Random(5) - 2;
        tempVect.y = 6.0f;
        tempPos.x = 0.0f;
        tempPos.y = 0.0f;
        tempPos.z = 9.0f;
        RotateVectorByQuaternion(&tempPos, &tempPos, &bullet->actor.rotQuat);
        ADD_TO_VECTOR(&tempPos, &bullet->actor.pos);
        AddDebris(NEW_DUST, &tempPos, &tempVect, 0, 0);
    } else {
        tempVect = bullet->actor.vel;
        SCALE_VECTOR(&tempVect, 0.7);
        AddDebris(NEW_DUST, &bullet->actor.pos, &tempVect, 0, 0);
    }
    return FALSE;
}

void UpdateBullets(void) {
    Bullet *bullet;
    Bullet *bullet2;
    Water *ptr;
    Debris *debris;
    Actor *owner;
    Vec3 sep;
    f32 turnSpeed;
    f32 speed;
    f32 aim;
    f32 theta;
    f32 diff;
    s32 i;

    for (bullet = bulletList.head.next; bullet != &bulletList.head; bullet = bullet2) {
        bullet2 = bullet->next;
        if (bullet->sample.volume) {
            UpdateContinuousSample(&bullet->sample);
        }
        if (standardBullets[bullet->type].maintainFunc != NULL && standardBullets[bullet->type].maintainFunc(bullet)) {
            bullet->actor.lifeTimer = 1;
        }
        if (bullet->actor.flags & 0x200000) {
            bullet->actor.state->waterState = 0;
            for (ptr = waterList.head.next; ptr != &waterList.head; ptr = ptr->next) {
                if (IsPointWithinBox(&ptr->box, &bullet->actor.pos)) {
                    CheckActorForWater(&bullet->actor, ptr, 1);
                }
            }
        }
        if (bullet->actor.lifeTimer > 0 && --bullet->actor.lifeTimer <= 0) {
            if (standardBullets[bullet->type].hitFunc != NULL) {
                standardBullets[bullet->type].hitFunc(&bullet->actor);
            }
            if (standardBullets[bullet->type].deathDebris != 24) {
                AddDebris(standardBullets[bullet->type].deathDebris, &bullet->actor.pos, &zero, NULL, NULL);
            }
            SubBullet(bullet);
            continue;
        }
        if (bullet->actor.inst == NULL && bullet->sprites[0].tex == NULL) {
            if (standardBullets[bullet->type].hitFunc != NULL) {
                standardBullets[bullet->type].hitFunc(&bullet->actor);
            }
            SubBullet(bullet);
            continue;
        }
        owner = bullet->actor.link;
        bullet->actor.flags &= ~0x44000000;
        UpdateActor(&bullet->actor);
        if (owner != bullet->actor.link) {
            BulletHitTarget(bullet, bullet->actor.link);
            continue;
        }
        if (gameInfo.level != SPACEBOSS3 && !(bullet->actor.flags & 8)) {
            if (bullet->actor.flags & 0x20000) {
                if (DistanceBetweenPointsSquared(&hand.pos, &bullet->actor.pos)
                    <= (hand.radius + bullet->actor.radius) * (hand.radius + bullet->actor.radius)) {
                    if (bullet->targetFlags & 0x10) {
                        handControl.smokeTimer = 40;
                    }
                    HurtHand(HURT_GO_BACKWARDS, 1);
                    if (bullet->type == MISSILE_BULLET) {
                        bullet->actor.lifeTimer = 1;
                    }
                }
            }
            if (bullet->actor.flags & 0x140000) {
                if (DistanceBetweenPointsSquared(&ball.pos, &bullet->actor.pos)
                    <= (hand.radius + bullet->actor.radius) * (ball.radius + bullet->actor.radius)) {
                    if (bullet->actor.flags & 0x100000) {
                        BurstBall();
                    } else {
                        HurtBall();
                    }
                }
            }
        }
        for (i = 0; i < standardBullets[bullet->type].numSprites; i++) {
            bullet->sprites[i].pos = bullet->actor.pos;
        }
        if (bullet->actor.active == 0) {
            SubBullet(bullet);
            continue;
        }
        bullet->age++;
        if (standardBullets[bullet->type].trailDebris != 24) {
            debris = AddDebris(standardBullets[bullet->type].trailDebris, &bullet->actor.prevPos, &zero, NULL, NULL);
            if (debris != NULL) {
                debris->sprite.r = spellColour[bullet->spell + 1][1][0];
                debris->sprite.g = spellColour[bullet->spell + 1][1][1];
                debris->sprite.b = spellColour[bullet->spell + 1][1][2];
                if (bullet->type == MISSILE_BULLET) {
                    debris->sprite.scaleX *= 5;
                    debris->sprite.scaleY *= 5;
                    debris->sprite.anim.def.startScale *= 5;
                    debris->sprite.anim.def.endScale *= 5;
                }
            }
        }
        if (bullet->targetFlags & 8) {
            BulletAcquireTarget(bullet, &bullet->actor.pos);
        }
        if (bullet->type == NME_SPELL_BULLET) {
            sep.z = 0.0f;
            sep.y = 0.0f;
            sep.x = 4.0f;
            RotateVector2D(&sep, &sep, Random(7));
            AddSpellDebris(10, &bullet->actor.prevPos, &sep, bullet->spellType);
        }
        if (bullet->target == NULL) {
            continue;
        }
        if (bullet->type == TONGUE_BULLET) {
            SUB_VECTORS(&sep, (Vec3 *)bullet->target, &bullet->actor.pos);
        } else {
            SUB_VECTORS(&sep, &bullet->target->pos, &bullet->actor.pos);
        }
        // two tests: GCC folds `type == 26 || type == 27` into a range check
        if (bullet->type == MISSILE_BULLET) {
            sep.y += 150.0f;
        } else if (bullet->type == LASERBOLT_BULLET) {
            sep.y += 150.0f;
        }
        if (!(bullet->actor.flags & 8) && bullet->age > standardBullets[bullet->type].targetDelay) {
            if (DOT_PRODUCT(&sep, &sep) <= (bullet->target->radius + bullet->actor.radius) * (bullet->target->radius + bullet->actor.radius)) {
                BulletHitTarget(bullet, bullet->target);
                continue;
            }
        }
        if (bullet->age < -standardBullets[bullet->type].targetDelay) {
            continue;
        }
        if (standardBullets[bullet->type].flags2 & 0x20) {
            MakeUnit(&sep);
            AddToOneScaledVector(&bullet->actor.vel, &sep, standardBullets[bullet->type].accel);
            continue;
        }
        if (standardBullets[bullet->type].flags2 & 0x10) {
            turnSpeed = standardBullets[bullet->type].turn * bullet->age;
        } else {
            turnSpeed = standardBullets[bullet->type].turn;
        }
        speed = LENGTH_2D(&bullet->actor.vel);
        aim = Aabs(atan2(sep.x, sep.z));
        if (speed != 0.0f) {
            SCALE_VECTOR_2D(&bullet->actor.vel, 1.0f / speed);
            theta = Aabs(atan2(bullet->actor.vel.x, bullet->actor.vel.z));
        } else {
            theta = aim;
        }
        diff = FindShortestAngleSigned(theta, aim);
        if (diff > 0.0f) {
            theta = Aabs(theta - MIN(turnSpeed, diff));
        } else if (diff < 0.0f) {
            theta = Aabs(turnSpeed < -diff ? theta + turnSpeed : theta - diff);
        }
        bullet->actor.vel.x = __sinf(theta);
        bullet->actor.vel.z = __cosf(theta);
        diff = (standardBullets[bullet->type].accel + ABS(bullet->actor.vel.y)) * bullet->actor.vel.y;
        diff /= 2.0f * standardBullets[bullet->type].accel;
        if (diff < sep.y) {
            turnSpeed = standardBullets[bullet->type].accel;
        } else {
            turnSpeed = -standardBullets[bullet->type].accel;
        }
        if (bullet->age >= 0) {
            bullet->actor.vel.y += turnSpeed;
            SCALE_VECTOR_2D(&bullet->actor.vel, speed + standardBullets[bullet->type].accel);
        }
    }
}

void BulletHitTarget(Bullet *bullet, Actor *target) {
    Footprint *footprint;

    if (target == &hand) {
        if (bullet->actor.flags & 0x20000) {
            if (bullet->type == MISSILE_BULLET) {
                bullet->actor.lifeTimer = 1;
                StartEarthquake(10, 50, 25, 1, 2);
            } else if (bullet->type == LASERBOLT_BULLET) {
                if (spaceBossShield) {
                    PlaySample(88, &bullet->actor.pos, 70, 250);
                    FlipVector(&bullet->actor.vel);
                    bullet->target = NULL;
                    footprint = JallocAlloc(sizeof(Footprint), 1, "Footprint");
                    SetupFootprint(footprint, &bullet->actor.pos, NULL, 5.0f, 8.0f, blastTex, 5, 0x800);
                    footprint->alpha = 150;
                    footprint->parent = NULL;
                    footprint->width = 0.0f;
                    footprint->maxAlpha = 0;
                    footprint->alphaStep = (f32)footprint->alpha / 8.0;
                    footprint->normal.x = 0.0f;
                    footprint->normal.y = 0.0f;
                    footprint->normal.z = -1.0f;
                    RotateVectorByQuaternion(&footprint->normal, &footprint->normal, &hand.rotQuat);
                    AddFootprint(footprint);
                    if (foregroundObjects[0] != NULL) {
                        foregroundObjects[0]->alpha = 200;
                    }
                    return;
                }
                bullet->actor.lifeTimer = 1;
                ZERO_VECTOR(&bullet->actor.vel);
                StartEarthquake(10, 50, 25, 1, 2);
            }
            HurtHand(HURT_GO_BACKWARDS, 1);
        }
    } else if (target == &ball) {
        if (bullet->actor.flags & 0x100000) {
            BurstBall();
        } else if (bullet->actor.flags & 0x40000) {
            HurtBall();
        }
    }
    if (standardBullets[bullet->type].hitFunc != NULL) {
        standardBullets[bullet->type].hitFunc(&bullet->actor);
    }
    switch (bullet->spell) {
        case 1:
            if (target->state != NULL && target->state->index != -1) {
                ActivateSpell(target, 1, 50);
            } else if (target == &hand) {
                ActivateSpellHand(bullet->spell);
            }
            break;
        case 0:
        case 2:
            if (target->state != NULL && target->state->index != -1) {
                ActivateSpell(target, bullet->spell, spellDuration);
            } else if (target == &hand) {
                ActivateSpellHand(bullet->spell);
            }
            break;
        case 15:
            break;
        default:
            if (target != &ball || !ballStatic) {
                ActivateCollectable(bullet->spell, bullet->spellTime);
            }
            break;
    }
    if (bullet->type != BUBBLE_BULLET) {
        SubBullet(bullet);
    }
}

void BulletAcquireTarget(Bullet *bullet, Vec3 *pos) {
    f32 bestAng;
    f32 ang;
    s32 x;
    GaribGroup *garibGroup;
    Garib *garib;

    if (bullet->targetFlags & 1) {
        bullet->target = &ball;
    } else if (bullet->targetFlags & 2) {
        if (bullet->type != MISSILE_BULLET || bullet->age == 0) {
            bullet->target = &hand;
        }
    } else if (bullet->targetFlags & 4) {
        bestAng = 1e9f;
        bullet->target = NULL;
        for (x = 0; x < numberOfEnemies; x++) {
            if ((enemies[x].actor.type < 44 && bullet->type != DENIS)
                || (bullet->type == TONGUE_BULLET && enemies[x].actor.radius <= tongueRad)) {
                if (enemies[x].actor.active != 0 && enemies[x].actor.active != 2 && enemies[x].brain.spellTimer[0] == 0
                    && enemies[x].brain.spellTimer[2] == 0) {
                    ang = DistanceBetweenPointsSquared(&enemies[x].actor.pos, &hand.pos);
                    if (ang < bestAng) {
                        bestAng = ang;
                        bullet->target = &enemies[x].actor;
                    }
                }
            }
        }
        if (bullet->type == TONGUE_BULLET) {
            if (tTime1 * standardBullets[TONGUE_BULLET].maxSpeed * (tTime1 * standardBullets[TONGUE_BULLET].maxSpeed) < bestAng) {
                bullet->target = NULL;
            } else {
                bullet->target = (Actor *)&bullet->target->pos;
            }
            for (garibGroup = garibGroupList.head.prev; garibGroup != &garibGroupList.head; garibGroup = garibGroup->prev) {
                for (garib = garibGroup->garibs.head.prev; garib != &garibGroup->garibs.head; garib = garib->prev) {
                    if (garib->active) {
                        ang = DistanceBetweenPointsSquared(&garib->sprite.pos, &hand.pos);
                        if (ang < bestAng) {
                            bestAng = ang;
                            bullet->target = (Actor *)&garib->sprite.pos;
                        }
                    }
                }
            }
            if (tTime1 * standardBullets[TONGUE_BULLET].maxSpeed * (tTime1 * standardBullets[TONGUE_BULLET].maxSpeed) < bestAng) {
                bullet->target = NULL;
            }
        }
    }
}
