#include "common.h"
#include "game.h"
#include "display.h"
#include "footprint.h"
#include "bullet.h"
#include "collect.h"
#include "hand.h"
#include "debris.h"
#include "maths.h"
#include "macros.h"
#include "water.h"
#include "platform.h"
#include "tip.h"
#include "texture.h"
#include "jalloc.h"
#include "shadow.h"

void InitDisplayLists(void);
void SetRenderMode(void);
void SetupViewing(void);
void ApplyGlobalTransformations(void);
extern ShadowVtx shadvtx;
extern Texture *shadTex;
extern Quat vertQ;
extern s32 frameCount;
void QuaternionToMatrix(Quat *squat, Matrix *dmatrix);
f32 Altitude(Vec3 *pos, Vec3 **floor, s32 *plat, ActorObject *coll);

extern Texture *printTex;
extern u8 snowPrintAlpha;
extern u8 snowPrintCol;

extern inline s32 SetupFootprint(Footprint *fp, Vec3 *pos, Vec3 *normal, f32 size, f32 lifetime, Texture *texture, s32 flags, s32 texScale) {
    Quat q;

    if (fp->parent != NULL && normal == NULL) {
        return FALSE;
    }
    fp->texScale = texScale;
    fp->texture = texture;
    fp->width = size;
    fp->length = size;
    fp->scaleSpeed = 0.2f;
    fp->pos = *pos;
    if (normal != NULL) {
        fp->normal = *normal;
    } else {
        fp->normal.x = 0.0f;
        fp->normal.y = 1.0f;
        fp->normal.z = 0.0f;
    }
    fp->flags = flags;
    fp->life = lifetime;
    fp->age = 0;
    fp->alphaStep = 5;
    if (fp->flags & 4) {
        fp->maxAlpha = 80;
    } else {
        fp->maxAlpha = 250;
    }
    if (fp->flags & 0x10) {
        fp->alpha = 0;
    } else {
        fp->alpha = fp->maxAlpha;
    }
    fp->b = fp->g = fp->r = 0xFF;
    if (fp->parent != NULL) {
        SUB_FROM_VECTOR(&fp->pos, &fp->parent->actor.pos);
        fp->normalPtr = normal;
        q.w = fp->parent->actor.rotQuat.w;
        q.x = -fp->parent->actor.rotQuat.x;
        q.y = -fp->parent->actor.rotQuat.y;
        q.z = -fp->parent->actor.rotQuat.z;
        RotateVectorByQuaternion(&fp->pos, &fp->pos, &q);
    }
    return TRUE;
}

extern inline void AddFootprint(Footprint *fp) {
    Footprint *it;
    s32 maxAge = 0;

    for (it = footprintList.head.next; it != &footprintList.head; it = it->next) {
        if (it->texture == fp->texture) {
            break;
        }
    }
    if (it == &footprintList.head) {
        it = footprintList.head.next;
    }
    if (fp->next == NULL) {
        footprintList.count++;
        fp->next = it;
        fp->prev = it->prev;
        it->prev->next = fp;
        it->prev = fp;
        if (footprintList.count > 40) {
            for (it = footprintList.head.next; it != &footprintList.head; it = it->next) {
                if ((it->flags & 2) && maxAge < it->age) {
                    maxAge = it->age;
                    fp = it;
                }
            }
            KillFootprint(fp);
        }
    }
}

void InitRestartGateShadows(void) {
    Platform *ptr;
    Vec3 vect;
    Vec3 *plane;
    s32 plat;
    Footprint *footprint;

    for (ptr = platformList.head.next; ptr != &platformList.head; ptr = ptr->next) {
        if (ptr->restartGate != NULL) {
            vect = ptr->actor.pos;
            vect.y -= Altitude(&vect, &plane, &plat, 0) - 1.5;
            footprint = JallocAlloc(sizeof(Footprint), 1, "Footprint");
            SetupFootprint(footprint, &vect, plane, 20.0f, -1.0f, shadTex, 5, 0x400);
            footprint->parent = ptr;
            footprint->maxAlpha = 200;
            footprint->alpha = 200;
            footprint->r = footprint->g = footprint->b = 0;
            AddFootprint(footprint);
        }
    }
}

static inline f32 GateShadowScale(f32 alt) {
    f32 scale;

    scale = (150.0f - alt) * 3.0f;
    scale /= 150.0f;
    if (scale < 0.5) {
        return 0.5f;
    }
    return scale;
}

void UpdateRestartGateShadows(Footprint *ptr) {
    Vec3 *plane;
    s32 plat;
    f32 alt;

    if (ptr->parent->restartGate->groundShadow) {
        alt = Altitude(&ptr->parent->actor.pos, &plane, &plat, 0);
        ptr->pos = ptr->parent->actor.pos;
        ptr->pos.y -= alt - 1.5;
        ptr->normal = *plane;
    } else {
        alt = ptr->parent->restartGate->shadowAlt;
    }
    ptr->width = ptr->parent->actor.scale.x * GateShadowScale(alt);
}

void CastShadow(Vec3 *pos, Vec3 *plane, f32 size, f32 altitude, u16 alpha, ShadowVtx *vert, Vec3 *lightDir) {
    Quat q;
    Matrix rotmat;
    Matrix scalemat;
    Matrix transmat;
    Matrix tempmat;
    Vec3 shadowPos;
    f32 scale;
    s32 i;

    if (plane == NULL || altitude <= 0.0f) {
        return;
    }
    scale = (MIN(size * sqrtf(size / altitude), size) * 7.0f + size) / 8.0f;
    gDPSetPrimColor(glistp++, 0, 0, 0, 0, 0, alpha);
    if (vert != &shadvtx) {
        for (i = 0; i < 4; i++) {
            vert->v[i].v.cn[3] = alpha;
        }
    }
    NormalToQuaternion(&q, plane);
    QuaternionToMatrix(&q, &rotmat);
    AddOneScaledVector(&shadowPos, pos, lightDir, 1.5 - altitude);
    guTranslateF(transmat.m, shadowPos.x, shadowPos.y, shadowPos.z);
    guScaleF(scalemat.m, scale, 1.0f, scale);
    guMtxCatF(rotmat.m, transmat.m, tempmat.m);
    guMtxCatF(scalemat.m, tempmat.m, tempmat.m);
    guMtxF2L(tempmat.m, &dynamicp->objectMtx[objectMatrix]);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&dynamicp->objectMtx[objectMatrix++]), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPVertex(glistp++, vert, 4, 0);
    gSP2Triangles(glistp++, 2, 0, 1, 0, 3, 0, 2, 0);
    gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
}

void SetAIRenderMode(u8 alpha) {
    gDPPipeSync(glistp++);
    if (fog.on) {
        gDPSetRenderMode(glistp++, G_RM_FOG_SHADE_A, G_RM_ZB_CLD_SURF2);
        gDPSetCombineLERP(glistp++, TEXEL0, 0, 1, SHADE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, COMBINED, 0, 0, 0, COMBINED);
    } else {
        gDPSetRenderMode(glistp++, G_RM_ZB_CLD_SURF, G_RM_ZB_CLD_SURF2);
        gDPSetCombineMode(glistp++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    }
    gDPSetPrimColor(glistp++, 0, 0, 255, 255, 255, alpha);
}

#define FADE_SHADOW(alpha, target, step)                                               \
    if ((alpha) > (target)) {                                                          \
        (alpha) = MAX((target), (alpha) - (s16)(step));                                \
    } else if ((alpha) < (target)) {                                                   \
        (alpha) = (target) > (alpha) + (s16)(step) ? (alpha) + (s16)(step) : (target); \
    }

void ProcessShadows(void) {
    Bullet *bullet;
    Actor *actor;
    Water *water;
    Platform *plat;
    GaribGroup *garibGroup;
    Garib *garib;
    Tip *tip;
    s32 isHand;

    if (fog.on) {
        gDPSetRenderMode(glistp++, G_RM_FOG_SHADE_A, G_RM_ZB_CLD_SURF2);
        gDPSetCombineMode(glistp++, G_CC_MODULATEIA_PRIM, G_CC_PASS2);
    } else {
        gDPSetRenderMode(glistp++, G_RM_ZB_CLD_SURF, G_RM_ZB_CLD_SURF2);
        gDPSetCombineMode(glistp++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    }
    gDPSetTextureLUT(glistp++, G_TT_NONE);
    gDPLoadTextureBlock(glistp++, shadTex->data, G_IM_FMT_IA, G_IM_SIZ_16b, shadTex->width, shadTex->height, 0,
                        G_TX_CLAMP, G_TX_CLAMP, shadTex->maskS, shadTex->maskT, G_TX_NOLOD, G_TX_NOLOD);
    for (bullet = bulletList.head.next; bullet != &bulletList.head; bullet = bullet->next) {
        if ((bullet->actor.flags & 0x100) && bullet->actor.distSq < 360000.0f) {
            FADE_SHADOW(bullet->actor.shadow->shadowAlpha, bullet->actor.shadow->shadowTarget, bullet->actor.shadow->shadowStep);
            if (bullet->actor.shadow->altitude != -1.0f) {
                CastShadow(&bullet->actor.pos, bullet->actor.shadow->floor, bullet->actor.shadow->shadowSize,
                           bullet->actor.shadow->altitude, bullet->actor.shadow->shadowAlpha, &bullet->actor.shadow->shadow,
                           (Vec3 *)&vertQ);
            }
        }
    }
    for (actor = actorList.head.next; actor != &actorList.head; actor = actor->next) {
        if ((actor->flags & 0x100) && (actor->distSq < 360000.0f || actor == &hand || actor == &ball)) {
            if (actor->type >= 3 && actor->type < 59) {
                if (actor->distSq > 324900.0f) {
                    actor->shadow->shadowTarget = 0;
                } else {
                    actor->shadow->shadowTarget = 255;
                }
            }
            FADE_SHADOW(actor->shadow->shadowAlpha, actor->shadow->shadowTarget, actor->shadow->shadowStep);
            if (actor->shadow->altitude != -1.0f && actor->shadow->shadowAlpha) {
                // a flag keeps CSE from replacing actor with &hand in this branch
                isHand = actor == &hand;
                if (isHand && handControl.suctionPlane != NULL) {
                    CastShadow(&actor->pos, handControl.suctionPlane, hand.shadow->shadowSize, actor->radius,
                               hand.shadow->shadowAlpha, &actor->shadow->shadow, handControl.suctionPlane);
                } else if (actor->shadow->sphere == 0) {
                    CastShadow(&actor->pos, actor->shadow->floor, actor->shadow->shadowSize, actor->shadow->altitude,
                               actor->shadow->shadowAlpha, &actor->shadow->shadow, (Vec3 *)&vertQ);
                } else {
                    CastShadow(&actor->obj->spheres[actor->shadow->sphere - 1].prevPos, actor->shadow->floor,
                               actor->shadow->shadowSize, actor->shadow->altitude, actor->shadow->shadowAlpha,
                               &actor->shadow->shadow, (Vec3 *)&vertQ);
                }
            }
        }
    }
    if (gameInfo.level == PIRATESBONUS) {
        water = FindTaggedWater(159);
        CastShadow(&hand.pos, hand.shadow->floor, hand.shadow->shadowSize,
                   hand.pos.y - water->actor->pos.y, hand.shadow->shadowAlpha, &hand.shadow->shadow,
                   (Vec3 *)&vertQ);
        if (frameCount > 0) {
            Vec3 up; // never read, but the ROM fills it in on the stack

            up.x = up.z = 0.0f;
            up.y = 1.0f;
            for (plat = platformList.head.next; plat != &platformList.head; plat = plat->next) {
                if (plat->actor.pos.y > water->actor->pos.y) {
                    CastShadow(&plat->actor.pos, plat->actor.shadow->floor, 30.0f,
                               plat->actor.pos.y - water->actor->pos.y, 200, &plat->actor.shadow->shadow,
                               (Vec3 *)&vertQ);
                }
            }
        }
    }
    for (garibGroup = garibGroupList.head.prev; garibGroup != &garibGroupList.head; garibGroup = garibGroup->prev) {
        for (garib = garibGroup->garibs.head.prev; garib != &garibGroup->garibs.head; garib = garib->prev) {
            if (!(garib->distSq > 360000.0f)) {
                FADE_SHADOW(garib->shadowAlpha, garib->shadowTarget, 10);
                if (garib->active && garib->altitude != -1.0f) {
                    if (garib->distSq > 324900.0f) {
                        garib->shadowTarget = 0;
                    } else {
                        garib->shadowTarget = 255;
                    }
                    CastShadow(&garib->sprite.pos, garib->normal, garib->scale * garib->sprite.tex->width * 0.25,
                               garib->altitude, garib->shadowAlpha, &shadvtx, (Vec3 *)&vertQ);
                }
            }
        }
    }
    if (handControl.fakeball != NULL && handControl.fakeball->shadow->altitude != -1.0f) {
        CastShadow(&handControl.fakeball->pos, handControl.fakeball->shadow->floor, ball.radius * 0.85,
                   handControl.fakeball->shadow->altitude, 255, &shadvtx, (Vec3 *)&vertQ);
    }
    for (tip = tipList.head.next; tip != &tipList.head; tip = tip->next) {
        CastShadow(&tip->pos, tip->normal, 10.0f, tip->altitude, tip->shadowAlpha, &shadvtx, (Vec3 *)&vertQ);
    }
}

void CalculateShadows(void) {
    Bullet *bullet;
    Actor *actor;
    GaribGroup *garibGroup;
    Garib *garib;
    s32 platform;

    if (gameInfo.level < INTRO) {
        for (bullet = bulletList.head.next; bullet != &bulletList.head; bullet = bullet->next) {
            if ((bullet->actor.flags & 0x100) && bullet->actor.distSq < 360000.0f) {
                bullet->actor.shadow->altitude = Altitude(&bullet->actor.pos, &bullet->actor.shadow->floor, &platform, 0);
            }
        }
        for (actor = actorList.head.next; actor != &actorList.head; actor = actor->next) {
            if (actor->type != COLLECTABLE_ACTOR && (actor->flags & 0x100)
                && (actor->distSq < 360000.0f || actor == &hand || actor == &ball)) {
                if (actor->shadow->sphere == 0) {
                    actor->shadow->altitude = Altitude(&actor->pos, &actor->shadow->floor, &platform, actor->obj);
                } else {
                    actor->shadow->altitude = Altitude(&actor->obj->spheres[actor->shadow->sphere - 1].pos, &actor->shadow->floor, &platform, actor->obj);
                }
            }
        }
        for (garibGroup = garibGroupList.head.prev; garibGroup != &garibGroupList.head; garibGroup = garibGroup->prev) {
            for (garib = garibGroup->garibs.head.prev; garib != &garibGroup->garibs.head; garib = garib->prev) {
                if (!(garib->distSq > 360000.0f) && garib->active && garib->mode == 1) {
                    garib->altitude = Altitude(&garib->sprite.pos, &garib->normal, &platform, 0);
                }
            }
        }
        if (handControl.fakeball != NULL) {
            handControl.fakeball->shadow->altitude = Altitude(&handControl.fakeball->pos, &handControl.fakeball->shadow->floor, &platform, 0);
        }
    }
}

void DrawHelper(void) {
    InitDisplayLists();
    SetRenderMode();
    SetupViewing();
    ApplyGlobalTransformations();
}

void AddGlovePrint(s16 side) {
    Vec3 tempVect;
    Vec3 tempVect2;
    Debris *debris;
    Footprint *footprint;
    s16 alpha;
    f32 size;

    if (hand.state->waterState == INWATER_PADDLING || hand.state->waterState == INWATER_WADING) {
        tempVect.x = side;
        tempVect.y = 0.0f;
        tempVect.z = 0.0f;
        RotateVector2D(&tempVect, &tempVect, hand.rot.y);
        ADD_TO_VECTOR(&tempVect, &hand.pos);
        tempVect.y += 3.0f;
        tempVect2.x = hand.vel.x / 1.5;
        tempVect2.y = MIN(2.0f * hand.speed, 6.0f);
        tempVect2.z = hand.vel.z / 1.5;
        debris = AddDebris(SPLASH, &tempVect, &tempVect2, NULL, NULL);
        if (debris != NULL) {
            debris->sprite.anim.def.startScale *= 0.5;
            debris->sprite.anim.def.endScale *= 0.5;
            if (Random(10) < 5) {
                debris->sprite.flags |= 2;
            }
        }
    } else if (hand.state->waterState == 0) {
        if (handControl.glovePrintTimer == 0) {
            if (hand.obj->planes[0] != NULL && (((CollFace *)hand.obj->planes[0])->terrain == 10 || ((CollFace *)hand.obj->planes[0])->terrain == 11)) {
                alpha = snowPrintAlpha;
            } else {
                return;
            }
        } else {
            alpha = MIN(handControl.glovePrintTimer, 40);
        }
        if (hand.obj->planes[0] == NULL || ((CollFace *)hand.obj->planes[0])->terrain != 5) {
            footprint = JallocAlloc(sizeof(Footprint), 1, "Footprint");
            footprint->parent = hand.obj->platform;
            tempVect.x = side;
            tempVect.y = -(hand.radius - 2.0f);
            tempVect.z = 0.0f;
            RotateVector2D(&tempVect, &tempVect, hand.rot.y);
            size = 0.3f;
            ADD_VECTORS(&tempVect, &hand.pos, &tempVect);
            if (side == 0) {
                size = 0.6f;
            }
            if (SetupFootprint(footprint, &tempVect, (Vec3 *)hand.obj->planes[0], size, 60.0f, printTex, 14, 0x200)) {
                footprint->alpha = footprint->maxAlpha = alpha;
                if (alpha == snowPrintAlpha) {
                    footprint->r = footprint->g = footprint->b = snowPrintCol;
                    footprint->alphaStep = 2;
                } else {
                    footprint->alphaStep = 1;
                }
                AddFootprint(footprint);
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/shadow", pad_801A8E28);

// linker junk that follows the 1.0f constant emitted by AddGlovePrint
const u32 D_8010B1F4[] = { 0x0001403A, 0x00013D22, 0x00013A10 };
