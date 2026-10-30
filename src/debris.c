#include "common.h"
#include "ident.h"
#include "debris.h"
#include "actor.h"
#include "hand.h"
#include "camera.h"
#include "game.h"
#include "maths.h"
#include "jalloc.h"
#include "texture.h"
#include "overlay.h"
#include "weather.h"
#include "object.h"

typedef struct DebrisDetails {
    /* 0x00 */ s32 flags;
    /* 0x04 */ u16 life;
    /* 0x06 */ s16 lifeRange;
    /* 0x08 */ f32 radius;
    /* 0x0C */ f32 friction;
    /* 0x10 */ s32 next;
} DebrisDetails; /* size = 0x14 */

extern DebrisDetails debrisDetails[];
extern Vec3 zero;
extern Debris debris[120];
extern ActorPhysics debrisBehaviour[120];
extern Quat zeroQuat;
extern SpriteAnim debrisAnimation[];
extern s32 noDebris;
extern s32 frameCount;
void InitSpriteAnimation(GameSprite *sprite, SpriteAnim *temp, s32 lifespan);
void FindTexture(Texture **texPtr, u32 texID, s32 verbose);
u32 UpdateCRC(char *ptr);

void ReleaseActor(Actor *actor, s32 flags);

void InitDebris(void) {
    char fn[256];
    Texture *tex;
    s32 total;
    s32 more;
    s32 n;
    s32 i;

    total = 0;
    for (i = 0; i < NUM_SPRITE_ANIMS; i++) {
        if (spriteFrameList[i].loaded) {
            total += spriteFrameList[i].count;
        } else {
            more = TRUE;
            n = 0;
            while (more == TRUE) {
                sprintf(fn, "%s%02d.bmp", spriteFrameList[i].name, n + 1);
                FindTexture(&tex, UpdateCRC(fn), 0);
                if (tex == NULL) {
                    more = FALSE;
                } else {
                    n++;
                }
            }
            spriteFrameList[i].count = n;
            total += n;
        }
    }
    spriteFrameList[0].textures = JallocAlloc(total * 4, 0, "SFL_Tex");
    for (i = 0; i < NUM_SPRITE_ANIMS; i++) {
        if (i != 0) {
            spriteFrameList[i].textures = spriteFrameList[i - 1].textures + spriteFrameList[i - 1].count;
        }
        if (spriteFrameList[i].count != 0) {
            for (n = 0; n < spriteFrameList[i].count; n++) {
                sprintf(fn, "%s%02d.bmp", spriteFrameList[i].name, n + 1);
                FindTexture(&tex, UpdateCRC(fn), 1);
                spriteFrameList[i].textures[n] = tex;
            }
        }
    }
    for (n = 0; n < 120; n++) {
        debris[n].active = 0;
        memclear(&debris[n], 0xE4);
        debris[n].info = &debrisBehaviour[n];
        debris[n].actorType = 3;
        debris[n].friction = 0.7f;
        debris[n].info->rotDrag = 1.0f;
        debris[n].flags = 5;
        debris[n].info->mass = 0.0f;
        debris[n].info->maxSpeed = 10.0f;
        debris[n].info->bounce = 0.9f;
        debris[n].info->minSpeed = 0.2f;
        debris[n].info->slideLimit = 0.1f;
        debris[n].info->friction = debris[n].friction;
        debris[n].info->maxSquash = 0.0f;
        debris[n].rotQuat = zeroQuat;
        debris[n].scale.x = 1.0f;
        debris[n].scale.y = 1.0f;
        debris[n].scale.z = 1.0f;
        debris[n].sprite.next = NULL;
    }
}

Debris *AddDebris(s32 debrisType, Vec3 *position, Vec3 *velocity, GameSprite *after, Debris *newDeb) {
    s32 index;

    if (noDebris) {
        return NULL;
    }
    if (gameInfo.level != SPACEBOSS3 && DistanceBetweenPointsSquared(&camera.pos, position) > 2250000.0f) {
        return NULL;
    }
    if (!IsActorFacingPoint(&camera, position, 3.1415927f)) {
        return NULL;
    }
    if (newDeb == NULL) {
        for (index = 0; debris[index].active; index++) {
        }
        if (index >= 120) {
            return NULL;
        }
        newDeb = &debris[index];
    }
    newDeb->vel = *velocity;
    if (debrisType >= SCORE) {
        newDeb->type = SCORE;
        newDeb->vel.y = 1.0f;
    } else {
        newDeb->type = debrisType;
        newDeb->sprite.anim.frame = 0;
    }
    newDeb->inst = NULL;
    newDeb->rot.x = 0.0f;
    newDeb->rot.y = 0.0f;
    newDeb->rot.z = 0.0f;
    newDeb->scale.x = 1.0f;
    newDeb->scale.y = 1.0f;
    newDeb->scale.z = 1.0f;
    newDeb->radius = debrisDetails[newDeb->type].radius;
    newDeb->flags = debrisDetails[newDeb->type].flags;
    newDeb->friction = newDeb->info->friction = debrisDetails[newDeb->type].friction;
    newDeb->pos = *position;
    newDeb->prevPos = *position;
    newDeb->active = 1;
    newDeb->actorActive = 1;
    newDeb->lifeTimer = newDeb->life = debrisDetails[newDeb->type].life + Random(debrisDetails[newDeb->type].lifeRange);
    if (debrisAnimation[newDeb->type].frames != NULL && debrisAnimation[newDeb->type].frames->textures != NULL) {
        InitSpriteAnimation(&newDeb->sprite, &debrisAnimation[newDeb->type], newDeb->life);
        if (debrisType >= SCORE) {
            newDeb->sprite.anim.frame = debrisType - SCORE;
            newDeb->sprite.tex = newDeb->sprite.anim.def.frames->textures[newDeb->sprite.anim.frame];
        }
        newDeb->sprite.r = 255;
        newDeb->sprite.g = 255;
        newDeb->sprite.b = 255;
        newDeb->sprite.offsetX = -newDeb->sprite.tex->width / 2;
        newDeb->sprite.offsetY = -newDeb->sprite.tex->height / 2;
        newDeb->sprite.pos = newDeb->pos;
        AddSprite(&newDeb->sprite, after);
    } else {
        newDeb->sprite.tex = NULL;
        newDeb->sprite.anim.def.type = 0;
        newDeb->alpha = 100;
    }
    if (newDeb->flags & 0x18) {
        InitActorStructures((Actor *)newDeb, 2);
        newDeb->obj->spheres[0].radius = newDeb->obj->spheres[0].targetRadius = newDeb->radius;
    }
    if (newDeb->type == FAKEBALL) {
        InitActorStructures((Actor *)newDeb, 0x18);
    }
    return newDeb;
}

inline void KillDebris(Debris *index) {
    if (debrisDetails[index->type].next != 24) {
        index->active = 0;
        SubSprite(&index->sprite);
        AddDebris(debrisDetails[index->type].next, &index->pos, &zero, 0, index);
    } else {
        SubSprite(&index->sprite);
        SubActor((Actor *)index);
        index->active = 0;
        if (index == handControl.fakeball) {
            handControl.fakeball = NULL;
        }
        ReleaseActor((Actor *)index, 0x1E);
    }
}

void UpdateDebris(void) {
    Vec3 v;
    Debris *p;
    f32 s;
    s32 x;

    for (x = 0; x < 120; x++) {
        if (!debris[x].active) {
            continue;
        }
        if (debris[x].type == 13) {
            p = AddDebris(TRAJ, &debris[x].pos, &zero, NULL, NULL);
            if (p != NULL) {
                p->sprite.r = debris[x].sprite.r;
                p->sprite.g = debris[x].sprite.g;
                p->sprite.b = debris[x].sprite.b;
            }
        }
        if (debrisAnimation[debris[x].type].frames != NULL && debrisAnimation[debris[x].type].frames->textures != NULL
            && debris[x].sprite.tex == NULL) {
            KillDebris(&debris[x]);
            continue;
        }
        if (debris[x].type == 4) {
            SCALE_VECTOR(&debris[x].scale, 0.94);
        }
        if (debris[x].sprite.tex != NULL) {
            debris[x].sprite.pos = debris[x].pos;
        }
        if (!(debris[x].flags & 0x800000)) {
            UpdateActor((Actor *)&debris[x]);
        }
        if (debris[x].type == 20) {
            s = SineWave(10.0f, debris[x].lifeTimer * 6, 0) * 7.0f;
            debris[x].sprite.scaleX = s + 32.0f;
            debris[x].sprite.scaleY = 32.0f - s;
            if (Random(3) == 1) {
                v.x = 0.7f;
                v.z = 0.0f;
                RotateVector2D(&v, &v, frameCount);
                debris[x].vel.x *= 0.5;
                debris[x].vel.z *= 0.5;
                debris[x].vel.x += v.x;
                debris[x].vel.z += v.z;
            }
        }
        if (debris[x].lifeTimer > 0 && (debris[x].lifeTimer--, debris[x].lifeTimer == 0)) {
            if (debris[x].type == 4) {
                if (debris[x].flags & 0x2000) {
                    AddDebris(DUST, &debris[x].pos, &zero, NULL, NULL);
                } else {
                    CreateDustCircle(&debris[x].pos, 3, 5.0f);
                    p = AddDebris(DUST, &debris[x].pos, &zero, NULL, NULL);
                    if (p != NULL) {
                        p->sprite.scaleX *= 2;
                        p->sprite.scaleY *= 2;
                        p->sprite.anim.def.startScale *= 2;
                        p->sprite.anim.def.endScale *= 2;
                    }
                }
            }
            KillDebris(&debris[x]);
        }
    }
}

void CreateDustCircleScaled(Vec3 *pos, s32 numStars, f32 verticalVelocity, f32 scale) {
    f32 angleStep = 6.283185308 / numStars;
    f32 angle = Random(6);
    f32 dist = 4.0f;
    Vec3 tempVect;
    Debris *d;
    s32 x;

    for (x = 0; x < numStars; x++) {
        tempVect = zero;
        tempVect.x = dist * scale;
        RotateVector2D(&tempVect, &tempVect, angle);
        tempVect.y = verticalVelocity;
        angle = Aabs(angle + angleStep);
        d = AddDebris(DUST, pos, &tempVect, 0, 0);
        if (d) {
            if (x & 1) {
                d->sprite.flags |= 4;
            }
            d->sprite.scaleX *= scale;
            d->sprite.scaleY *= scale;
            d->sprite.anim.def.endScale *= scale;
            d->sprite.anim.def.startScale *= scale;
        }
    }
}

void CreateDustCircle(Vec3 *pos, s32 numStars, f32 verticalVelocity) {
    f32 angleStep = 6.283185308 / numStars;
    f32 angle = Random(6);
    f32 dist = 4.0f;
    Vec3 tempVect;
    Debris *d;
    s32 x;

    for (x = 0; x < numStars; x++) {
        tempVect = zero;
        tempVect.x = dist;
        RotateVector2D(&tempVect, &tempVect, angle);
        tempVect.y = verticalVelocity;
        angle = Aabs(angle + angleStep);
        d = AddDebris(DUST, pos, &tempVect, 0, 0);
        if (d && (x & 1)) {
            d->sprite.flags |= 4;
        }
    }
}

void CreateSplashCircle(Vec3 *pos, s32 numStars, f32 verticalVelocity) {
    f32 angleStep = 6.283185308 / numStars;
    f32 angle = Random(6);
    f32 dist = 4.0f;
    Vec3 tempVect;
    Debris *d;
    s32 x;

    for (x = 0; x < numStars; x++) {
        tempVect = zero;
        tempVect.x = dist + Random(3);
        RotateVector2D(&tempVect, &tempVect, angle);
        tempVect.y = verticalVelocity + Random(9);
        angle = Aabs(angle + angleStep);
        d = AddDebris(SPLASH, pos, &tempVect, 0, 0);
        if ((x & 1) && d) {
            d->sprite.flags |= 2;
        }
    }
}

void CreatePuffCircle(Vec3 *pos) {
    Vec3 tempVect2;
    Vec3 tempVect;
    s32 y;

    SUB_VECTORS(&tempVect, &camera.pos, pos);
    MakeUnit(&tempVect);
    SCALE_VECTOR(&tempVect, 5.0f);
    ADD_TO_VECTOR(&tempVect, pos);
    for (y = 0; y < 6; y++) {
        tempVect2 = zero;
        tempVect2.x = 6.0f;
        tempVect2.y = 3.0f;
        RotateVector2D(&tempVect2, &tempVect2, y);
        AddPuff(&tempVect, &tempVect2, 4.0f);
    }
}

void CreateStarCircle(Vec3 *pos, s32 numStars, s16 type) {
    f32 angleStep = 6.283185308 / numStars;
    f32 angle = (f32)Random(62) / 10.0;
    f32 dist = 4.0f;
    Vec3 tempVect;
    Debris *d;
    s32 x;

    for (x = 0; x < numStars; x++) {
        tempVect = zero;
        tempVect.x = dist;
        RotateVector2D(&tempVect, &tempVect, angle);
        tempVect.y = (Random(10) / 40.0f + 1.0f) * 10.0f;
        angle += angleStep;
        d = AddDebris(type, pos, &tempVect, 0, 0);
        if (type == 19 && d) {
            d->friction = 0.9f;
            d->flags &= ~0x800000;
            d->info->friction = 0.9f;
            d->vel.y = 8.0f;
            d->sprite.anim.def.startScale *= 2;
            SCALE_VECTOR_2D(&d->vel, 2.0f);
            d->sprite.anim.def.endScale *= 2;
            d->sprite.scaleX *= 2;
            d->sprite.scaleY *= 2;
        }
    }
}

void CreateBigDustCircle(Vec3 *pos, f32 Yval) {
    f32 step = 1.256637096f;
    f32 angle = Random(6);
    Vec3 tempVel;
    Vec3 tempVect;
    Debris *d;
    s32 x;

    for (x = 0; x < 5; x++) {
        tempVel.x = 16.0f;
        tempVel.z = 0.0f;
        RotateVector2D(&tempVel, &tempVel, angle);
        tempVel.y = Yval;
        SetVector2D(&tempVect, &tempVel);
        SCALE_VECTOR_2D(&tempVect, 0.2);
        tempVect.y = 0.0f;
        ADD_TO_VECTOR(&tempVel, pos);
        angle += step;
        d = AddDebris(DUST, &tempVel, &tempVect, 0, 0);
        if (d) {
            if (x & 1) {
                d->sprite.flags |= 2;
            }
            d->sprite.scaleX *= 2;
            d->sprite.scaleY *= 2;
            d->sprite.anim.def.endScale *= 2;
            d->sprite.anim.def.startScale *= 2;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/debris", pad_801B7B34);

INCLUDE_RODATA("asm/nonmatchings/debris", pad_8010BDA8);
