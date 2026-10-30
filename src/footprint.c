#include "common.h"
#include "ident.h"
#include "game.h"
#include "footprint.h"
#include "jalloc.h"
#include "hand.h"
#include "maths.h"
#include "display.h"
#include "platform.h"
#include "nmeprocess2.h"
#include "shadow.h"

extern s32 frameCount;
extern u16 footprintTexFmt;

void QuaternionToMatrix(Quat *squat, Matrix *dmatrix);

extern Texture *blastTex;

void InitFootprintList(void) {
    footprintList.head.next = footprintList.head.prev = &footprintList.head;
    footprintList.count = 0;
}

s32 SetupFootprint(Footprint *footprint, Vec3 *pos, Vec3 *plane, f32 size, f32 lifetime, Texture *texture, s32 flags, s32 texScale) {
    Quat q;

    if (footprint->parent != NULL && plane == NULL) {
        return FALSE;
    }
    footprint->texScale = texScale;
    footprint->texture = texture;
    footprint->width = size;
    footprint->length = size;
    footprint->scaleSpeed = 0.2f;
    footprint->pos = *pos;
    if (plane != NULL) {
        footprint->normal = *plane;
    } else {
        footprint->normal.x = 0.0f;
        footprint->normal.y = 1.0f;
        footprint->normal.z = 0.0f;
    }
    footprint->flags = flags;
    footprint->life = lifetime;
    footprint->age = 0;
    footprint->alphaStep = 5;
    if (footprint->flags & 4) {
        footprint->maxAlpha = 80;
    } else {
        footprint->maxAlpha = 250;
    }
    if (footprint->flags & 0x10) {
        footprint->alpha = 0;
    } else {
        footprint->alpha = footprint->maxAlpha;
    }
    footprint->b = footprint->g = footprint->r = 0xFF;
    if (footprint->parent != NULL) {
        SUB_FROM_VECTOR(&footprint->pos, &footprint->parent->actor.pos);
        footprint->normalPtr = plane;
        q.w = footprint->parent->actor.rotQuat.w;
        q.x = -footprint->parent->actor.rotQuat.x;
        q.y = -footprint->parent->actor.rotQuat.y;
        q.z = -footprint->parent->actor.rotQuat.z;
        RotateVectorByQuaternion(&footprint->pos, &footprint->pos, &q);
    }
    return TRUE;
}

void AddFootprint(Footprint *footprint) {
    Footprint *ptr;
    s32 i = 0;

    for (ptr = footprintList.head.next; ptr != &footprintList.head; ptr = ptr->next) {
        if (ptr->texture == footprint->texture) {
            break;
        }
    }
    if (ptr == &footprintList.head) {
        ptr = footprintList.head.next;
    }
    if (footprint->next == NULL) {
        footprintList.count++;
        footprint->next = ptr;
        footprint->prev = ptr->prev;
        ptr->prev->next = footprint;
        ptr->prev = footprint;
        if (footprintList.count > 40) {
            for (ptr = footprintList.head.next; ptr != &footprintList.head; ptr = ptr->next) {
                if ((ptr->flags & 2) && i < ptr->age) {
                    i = ptr->age;
                    footprint = ptr;
                }
            }
            KillFootprint(footprint);
        }
    }
}

inline void RemoveFootprint(Footprint *ptr) {
    if (ptr->next != NULL) {
        ptr->prev->next = ptr->next;
        ptr->next->prev = ptr->prev;
        footprintList.count--;
        ptr->next = NULL;
    }
    if (ptr == handControl.footprint) {
        handControl.footprint = NULL;
    }
    JallocFree(&ptr);
}

void UpdateFootprints(void) {
    Footprint *ptr;
    Footprint *ptr2;

    for (ptr = footprintList.head.next; ptr != &footprintList.head; ptr = ptr2) {
        ptr2 = ptr->next;
        if (ptr->timer != 0 && --ptr->timer == 0) {
            RemoveFootprint(ptr);
        }
    }
}

void DrawFootprints(void) {
    Footprint *ptr;
    Footprint *ptr2;
    Texture *lastTex;

    gSPClearGeometryMode(glistp++, G_LIGHTING);
    gDPPipeSync(glistp++);
    lastTex = NULL;
    if (fog.on) {
        gDPSetRenderMode(glistp++, G_RM_FOG_SHADE_A, G_RM_ZB_CLD_SURF2);
        gDPSetCombineMode(glistp++, G_CC_MODULATEIA_PRIM, G_CC_PASS2);
    } else {
        gDPSetRenderMode(glistp++, G_RM_ZB_CLD_SURF, G_RM_ZB_CLD_SURF2);
        gDPSetCombineMode(glistp++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    }
    for (ptr = footprintList.head.next; ptr != &footprintList.head; ptr = ptr->next) {
        if (ptr->texture != NULL && ptr->timer == 0 && ptr->alpha != 0) {
            if (ptr->texture != lastTex) {
                LoadTexture(ptr->texture);
            }
            lastTex = ptr->texture;
            DrawFootprint(ptr);
        }
    }
    for (ptr = footprintList.head.next; ptr != &footprintList.head; ptr = ptr2) {
        ptr2 = ptr->next;
        ptr->age++;
        if (ptr->maxAlpha == 0 && ptr->alpha == 0 && ptr->timer == 0) {
            KillFootprint(ptr);
        }
        if (ptr->life > 0 && --ptr->life == 0) {
            if (ptr->flags & 8) {
                ptr->maxAlpha = 0;
            } else {
                KillFootprint(ptr);
            }
        }
        if (ptr->alpha < ptr->maxAlpha) {
            ptr->alpha += ptr->alphaStep;
        } else if (ptr->alpha > ptr->maxAlpha) {
            ptr->alpha -= ptr->alphaStep;
        }
        if (ptr->parent != NULL && ptr->parent->restartGate != 0) {
            UpdateRestartGateShadows(ptr);
        } else if (ptr->scaleSpeed > 0.0f) {
            ptr->width += (ptr->length - ptr->width) * ptr->scaleSpeed;
        } else {
            ptr->width -= MIN(MAX(-ptr->scaleSpeed, (ptr->length - ptr->width) / 2.0f), ptr->scaleSpeed);
        }
        if (gameInfo.level != SPACEBOSS3 && ptr->texture == blastTex && ptr != handControl.footprint && ptr->normal.y == 1.0f) {
            UpdateEnemyBlastring(ptr);
        }
    }
}

void KillFootprint(Footprint *ptr) {
    ptr->alpha = 0;
    ptr->timer = 5;
}

void DrawFootprint(Footprint *ptr) {
    Quat q;
    Matrix rotmat;
    Matrix scalemat;
    Matrix transmat;
    Matrix tempmat;
    Vec3 pos;
    Vec3 *normal;
    Vtx *vtx;
    Vtx *vtx2;

    footprintTexFmt = ptr->texture->fmt;
    gDPSetPrimColor(glistp++, 0, 0, ptr->r, ptr->g, ptr->b, ptr->alpha);
    normal = &ptr->normal;
    if (ptr->parent != NULL && ptr->parent->restartGate == 0) {
        RotateVectorByQuaternion(&pos, &ptr->pos, &ptr->parent->actor.rotQuat);
        ADD_TO_VECTOR(&pos, &ptr->parent->actor.pos);
        if (ptr->parent->lastUpdate < frameCount) {
            RecalculatePlatformInfo(ptr->parent);
        }
        normal = ptr->normalPtr;
    } else {
        pos = ptr->pos;
    }
    NormalToQuaternion(&q, normal);
    QuaternionToMatrix(&q, &rotmat);
    guTranslateF(transmat.m, pos.x, pos.y, pos.z);
    guScaleF(scalemat.m, ptr->width, 1.0f, ptr->width);
    guMtxCatF(rotmat.m, transmat.m, tempmat.m);
    guMtxCatF(scalemat.m, tempmat.m, tempmat.m);
    guMtxF2L(tempmat.m, &dynamicp->objectMtx[objectMatrix]);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&dynamicp->objectMtx[objectMatrix++]), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    vtx = ptr->vtx;
    vtx[0].v.ob[0] = 10;
    vtx[0].v.ob[1] = 0;
    vtx[0].v.ob[2] = 10;
    vtx[0].v.flag = 0;
    vtx[0].v.tc[0] = 0;
    vtx[0].v.tc[1] = 0;
    vtx[0].v.cn[0] = 0xFF;
    vtx[0].v.cn[1] = 0xFF;
    vtx[0].v.cn[2] = 0xFF;
    vtx[0].v.cn[3] = ptr->alpha;
    vtx[1].v.ob[0] = 10;
    vtx[1].v.ob[1] = 0;
    vtx[1].v.ob[2] = -10;
    vtx[1].v.flag = 0;
    vtx[1].v.tc[0] = 0;
    vtx[1].v.tc[1] = ptr->texScale;
    vtx[1].v.cn[0] = 0xFF;
    vtx[1].v.cn[1] = 0xFF;
    vtx[1].v.cn[2] = 0xFF;
    vtx[1].v.cn[3] = ptr->alpha;
    vtx2 = &ptr->vtx[2];
    vtx2[0].v.ob[0] = -10;
    vtx2[0].v.ob[1] = 0;
    vtx2[0].v.ob[2] = -10;
    vtx2[0].v.flag = 0;
    vtx2[0].v.tc[0] = ptr->texScale;
    vtx2[0].v.tc[1] = ptr->texScale;
    vtx2[0].v.cn[0] = 0xFF;
    vtx2[0].v.cn[1] = 0xFF;
    vtx2[0].v.cn[2] = 0xFF;
    vtx2[0].v.cn[3] = ptr->alpha;
    vtx2[1].v.ob[0] = -10;
    vtx2[1].v.ob[1] = 0;
    vtx2[1].v.ob[2] = 10;
    vtx2[1].v.flag = 0;
    vtx2[1].v.tc[0] = ptr->texScale;
    vtx2[1].v.tc[1] = 0;
    vtx2[1].v.cn[0] = 0xFF;
    vtx2[1].v.cn[1] = 0xFF;
    vtx2[1].v.cn[2] = 0xFF;
    vtx2[1].v.cn[3] = ptr->alpha;
    gSPVertex(glistp++, ptr->vtx, 4, 0);
    gSP2Triangles(glistp++, 2, 0, 1, 0, 3, 0, 2, 0);
    if (ptr->flags & 0x20) {
        gSP2Triangles(glistp++, 0, 2, 1, 0, 2, 0, 3, 0);
    }
    gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
}
