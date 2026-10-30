#include "common.h"
#include "ident.h"
#include "maths.h"
#include "wind.h"
#include "jalloc.h"
#include "actor.h"
#include "debris.h"
#include "platform.h"

extern s32 frameCount;

void InitWindList(void) {
    windList.count = 0;
    windList.head.next = windList.head.prev = &windList.head;
}

void AddWind(Wind *wind) {
    if (wind->next == NULL) {
        windList.count++;
        wind->prev = &windList.head;
        wind->next = windList.head.next;
        windList.head.next->prev = wind;
        windList.head.next = wind;
        MakeUnit((Vec3 *)wind->dir);
    }
}

void UpdateFan(Platform *platform) {
    Vec3 force;
    Vec3 angsep;
    Vec3 dir;
    Fan *fan;
    Actor *actor;
    f32 ang2;
    f32 dot;
    f32 cosang;
    f32 distSq;
    f32 dist;
    f32 val;

    if (platform->fan == NULL) {
        return;
    }
    fan = platform->fan;
    if (platform->motion != NULL) {
        if ((ang2 = platform->motion->rot.y - platform->motion->oldRot.y) != 0.0f) {
            RotateVector2D(&fan->dir, &fan->dir, ang2);
        } else if ((ang2 = platform->motion->rot.x - platform->motion->oldRot.x) != 0.0f) {
            RotateVector2DXYZ(&fan->dir, &fan->dir, ang2, 0);
        } else if ((ang2 = platform->motion->rot.z - platform->motion->oldRot.z) != 0.0f) {
            RotateVector2DXYZ(&fan->dir, &fan->dir, ang2, 2);
        }
    }
    if (fan->on != 1) {
        return;
    }
    for (actor = actorList.head.next; actor != &actorList.head; actor = actor->next) {
        if ((fan->strength > 0.0f && (actor->flags & 0x800)) || (fan->strength < 0.0f && (actor->flags & 0x400))) {
            SUB_VECTORS(&angsep, &actor->pos, &platform->actor.pos);
            dot = DOT_PRODUCT(&angsep, &fan->dir);
            if (!(dot <= 0.0f)) {
                AddOneScaledVector(&dir, &angsep, &fan->dir, fan->length);
                MakeUnit(&dir);
                cosang = DOT_PRODUCT(&dir, &fan->dir);
                if (fan->spread < cosang) {
                    distSq = DOT_PRODUCT(&angsep, &angsep);
                    dist = sqrtf(distSq);
                    dot /= dist;
                    if (!(dot <= 0.1) && !(distSq < actor->radius * actor->radius)) {
                        val = fan->strength * (cosang - fan->spread) / (distSq * dot * dot * (1.0f - fan->spread));
                        ang2 = actor->phys->mass > 0.1 ? actor->phys->mass : 0.1f;
                        val *= 10.0f / ang2;
                        if (ABS(val * (fan->blend + 1.0f)) > 0.01) {
                            AddScaledVector(&force, &fan->dir, 1.0f - fan->blend, &angsep, fan->blend / dist);
                            AddOneScaledVector(&actor->vel, &actor->vel, &force, val);
                            actor->vel.y = actor->vel.y > 15.0 ? 15.0f : actor->vel.y;
                            actor->flags |= 0x1000;
                        }
                    }
                }
            }
        }
    }
}

void UpdateWinds(void) {
    Wind *wind;
    Actor *actor;
    Actor *actPtr;
    s8 side[3];
    f32 dist[3];
    Vec3 tempVect;
    s32 i;
    s32 x;
    s32 lifetime;
    s32 num;
    f32 vel;
    f32 mass;
    Debris *debris;

    for (wind = windList.head.next; wind != &windList.head; wind = wind->next) {
        if (wind->type != 1) {
            continue;
        }
        if (frameCount & 1) {
            for (i = 0; i < wind->strength; i++) {
                lifetime = 0;
                vel = wind->strength * (Random(10) / 4.0f + 12.0f);
                for (x = 0; x < 3; x++) {
                    if (wind->dir[x] == 0.0f) {
                        (&tempVect.x)[x] = Random(wind->size[x]);
                    } else {
                        if (lifetime < wind->size[x] / vel) {
                            lifetime = wind->size[x] / vel;
                        }
                        if (wind->dir[x] < 0.0f) {
                            (&tempVect.x)[x] = wind->size[x];
                        } else {
                            (&tempVect.x)[x] = 0.0f;
                        }
                    }
                    (&tempVect.x)[x] += wind->pos[x];
                }
                debris = AddDebris(WIND_BIT, &tempVect, (Vec3 *)wind->dir, 0, 0);
                if (debris != NULL) {
                    debris->lifeTimer = lifetime;
                    SCALE_VECTOR(&debris->vel, vel);
                    if (Random(2) == 0) {
                        debris->sprite.flags |= 2;
                    }
                }
            }
        }
        for (actor = actorList.head.next; actor != &actorList.head; actor = actor->next) {
            if (!(actor->flags & 0x800)) {
                continue;
            }
            actPtr = actor;
            num = 3;
            mass = actPtr->phys->mass > 0.1 ? actPtr->phys->mass : 0.1f;
            for (i = 0; i < 3; i++) {
                if ((&actPtr->pos.x)[i] < wind->pos[i]) {
                    side[i] = -1;
                    num--;
                } else if (wind->pos[i] + wind->size[i] < (&actPtr->pos.x)[i]) {
                    side[i] = 1;
                    num--;
                } else {
                    side[i] = 0;
                }
            }
            if (num == 3) {
                AddOneScaledVector(&actPtr->vel, &actPtr->vel, (Vec3 *)wind->dir, wind->strength * 10.0f / mass);
                actPtr->flags |= 0x1000;
            } else if (num != 0) {
                for (i = 0; i < 3; i++) {
                    if (side[i] == 1) {
                        dist[i] = (&actPtr->pos.x)[i] - wind->pos[i] - wind->size[i];
                    } else if (side[i] == -1) {
                        dist[i] = wind->pos[i] - (&actPtr->pos.x)[i];
                    } else {
                        dist[i] = 0.0f;
                    }
                }
                vel = sqrtf(dist[0] * dist[0] + dist[1] * dist[1] + dist[2] * dist[2]);
                if (vel < wind->radius) {
                    vel = wind->strength * (wind->radius - vel) * 10.0f / (wind->radius * mass);
                    if (vel > 0.01) {
                        AddOneScaledVector(&actPtr->vel, &actPtr->vel, (Vec3 *)wind->dir, vel);
                        actPtr->flags |= 0x1000;
                    }
                }
            }
        }
    }
}

Wind *FindTaggedWind(Wind *startWind, s32 tag) {
    Wind *wind;

    for (wind = startWind->next; wind != &windList.head; wind = wind->next) {
        if (wind->tag == tag) {
            return wind;
        }
    }
    return NULL;
}

INCLUDE_ASM("asm/nonmatchings/wind", pad_8018B30C);
