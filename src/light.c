#include "common.h"
#include "ident.h"
#include "light.h"
#include "jalloc.h"
#include "debug.h"
#include "maths.h"
#include "camera.h"
#include "game.h"
#include "texture.h"
#include "display.h"

typedef struct LightSet {
    /* 0x00 */ u8 ambient[0x8];
    /* 0x08 */ LightList list;
} LightSet;

extern u8 originalAmbientLight[3];
extern Lightning lightningList[50];
extern s16 farPlaneDist;
extern Texture *lightningTexture;
extern s32 lightOffset;
extern u8 draw_buffer;
extern Light lightArray[100][2];
extern s32 lastLitActorFlags;
extern Vec3 lightningVect[];
extern Vtx *vtxPtr;

void InitBackgroundLightList(void) {
    bgLighting.lights.head.next = bgLighting.lights.head.prev = &bgLighting.lights.head;
    bgLighting.lights.count = 0;
}

void InitHandLightList(void) {
    handLightList.head.next = handLightList.head.prev = &handLightList.head;
    handLightList.count = 0;
}

void AddBackgroundLight(GameLight *light) {
    GameLight *ptr = bgLighting.lights.head.next;

    if (bgLighting.lights.count < 7 && light->next == NULL) {
        light->next = ptr;
        light->prev = ptr->prev;
        ptr->prev->next = light;
        ptr->prev = light;
        light->index = bgLighting.lights.count;
        bgLighting.lights.count++;
    }
}

inline void RemoveLight(GameLight *ptr) {
    if (ptr->next != NULL) {
        ptr->prev->next = ptr->next;
        ptr->next->prev = ptr->prev;
        bgLighting.lights.count--;
        ptr->next = NULL;
    }
    JallocFree(&ptr);
}

void AddHandLight(GameLight *light) {
    GameLight *first = handLightList.head.next;

    if (light->next == NULL) {
        light->next = first;
        light->prev = first->prev;
        first->prev->next = light;
        first->prev = light;
        light->index = handLightList.count;
        handLightList.count++;
    }
}

void SetupLight(GameLight *light, u8 initR, u8 initG, u8 initB, u8 r, u8 g, u8 b, f32 x, f32 y, f32 speed) {
    light->col[0] = initR;
    light->col[1] = initG;
    light->col[2] = initB;
    light->col[3] = r;
    light->col[4] = g;
    light->col[5] = b;
    light->colourSpeed = speed;
    light->target[0] = light->angle[0] = x;
    light->target[1] = light->angle[1] = y;
    light->origX = x;
    light->origY = y;
    UpdateLightDirections(light);
    light->type = 0;
}

inline void SetupLightNew(Light *l, u8 r, u8 g, u8 b, u8 x, u8 y, u8 z) {
    l->l.col[0] = r;
    l->l.col[1] = g;
    l->l.col[2] = b;
    l->l.pad1 = 0;
    l->l.colc[0] = r;
    l->l.colc[1] = g;
    l->l.colc[2] = b;
    l->l.pad2 = 0;
    l->l.pad3 = 0;
    l->l.dir[0] = x;
    l->l.dir[1] = y;
    l->l.dir[2] = z;
}

void SetupAmbientLight(Ambient *a, u8 r, u8 g, u8 b) {
    a->l.col[0] = r;
    a->l.col[1] = g;
    a->l.col[2] = b;
    a->l.pad1 = 0;
    a->l.colc[0] = r;
    a->l.colc[1] = g;
    a->l.colc[2] = b;
    a->l.pad2 = 0;
    originalAmbientLight[0] = r;
    originalAmbientLight[1] = g;
    originalAmbientLight[2] = b;
}

void ProcessBackgroundLights(Actor *actor) {
    GameLight *ptr;
    s32 lights = 1;

    if (lightOffset >= 100) {
        return;
    }
    if (actor->flags & 0x10000) {
        gSPNumLights(glistp++, bgLighting.lights.count - bgLighting.lights.numSkipped);
        for (ptr = bgLighting.lights.head.next; ptr != &bgLighting.lights.head; ptr = ptr->next) {
            if (ptr->type != 2) {
                SetupLightNew(&lightArray[lightOffset][draw_buffer], ptr->col[0], ptr->col[1], ptr->col[2], ptr->dir.x,
                              ptr->dir.y, ptr->dir.z);
                gSPLight(glistp++, &lightArray[lightOffset][draw_buffer], lights++);
                if (++lightOffset >= 100) {
                    return;
                }
            }
        }
    } else {
        gSPNumLights(glistp++, bgLighting.lights.count);
        for (ptr = bgLighting.lights.head.next; ptr != &bgLighting.lights.head; ptr = ptr->next) {
            SetupLightNew(&lightArray[lightOffset][draw_buffer], ptr->col[0], ptr->col[1], ptr->col[2], ptr->dir.x,
                          ptr->dir.y, ptr->dir.z);
            gSPLight(glistp++, &lightArray[lightOffset][draw_buffer], lights++);
            if (++lightOffset >= 100) {
                return;
            }
        }
    }
    gSPLight(glistp++, &bgLighting.ambient, lights);
    lastLitActorFlags = actor->flags;
}

void UpdateLights(void) {
    GameLight *ptr;
    GameLight *ptr2;

    bgLighting.lights.numSkipped = 0;
    for (ptr = bgLighting.lights.head.next; ptr != &bgLighting.lights.head; ptr = ptr2) {
        ptr2 = ptr->next;
        if (ptr->timer > 0) {
            if (--ptr->timer == 0) {
                ptr->col[3] = ptr->col[4] = ptr->col[5] = 0.0f;
            }
        } else if (ptr->timer == 0 &&
                   sqrtf(ptr->col[0] * ptr->col[0] + ptr->col[1] * ptr->col[1] + ptr->col[2] * ptr->col[2]) < 1.0f) {
            RemoveLight(ptr);
            continue;
        }
        if (ptr->type == 2) {
            bgLighting.lights.numSkipped++;
        }
        UpdateLightDirections(ptr);
        UpdateLightColours(ptr);
    }
}

void UpdateLightDirections(GameLight *light) {
    Vec3 v;
    f32 angle;
    s32 i;

    light->angle[0] += light->spin[0];
    light->angle[1] += light->spin[1];
    for (i = 0; i < 2; i++) {
        angle = FindShortestAngleSigned(light->target[i], light->angle[i]);
        if (angle > 0.0f) {
            light->angle[i] += (angle < light->turnSpeed) ? angle : light->turnSpeed;
        } else {
            light->angle[i] += (angle < -light->turnSpeed) ? -light->turnSpeed : angle;
        }
        light->angle[i] = Aabs(light->angle[i]);
    }
    v.x = v.y = 0.0f;
    v.z = 127.0f;
    RotateVector2DXYZ(&v, &v, light->angle[0], 0);
    RotateVector2D(&v, &v, light->angle[1]);
    light->dir.x = v.x;
    light->dir.y = v.y;
    light->dir.z = v.z;
}

void UpdateLightColours(GameLight *ptr) {
    Vec3 tempVect;

    tempVect.x = ptr->col[3] - ptr->col[0];
    tempVect.y = ptr->col[4] - ptr->col[1];
    tempVect.z = ptr->col[5] - ptr->col[2];
    MakeUnit(&tempVect);
    SCALE_VECTOR(&tempVect, ptr->colourSpeed);
    ptr->col[0] += tempVect.x;
    ptr->col[1] += tempVect.y;
    ptr->col[2] += tempVect.z;
    if (ptr->col[0] < 0.0f) {
        ptr->col[0] = 0.0f;
    }
    if (ptr->col[1] < 0.0f) {
        ptr->col[1] = 0.0f;
    }
    if (ptr->col[2] < 0.0f) {
        ptr->col[2] = 0.0f;
    }
}

void ChangeLightDir(LightSet *set, s32 index, f32 x, f32 y, f32 turnSpeed) {
    GameLight *light;

    for (light = set->list.head.next; light != &set->list.head; light = light->next) {
        if (light->index == index) {
            light->target[0] = x;
            light->target[1] = y;
            light->turnSpeed = turnSpeed;
            PRINTF("new light direction %f %f\n", x, y);
            return;
        }
    }
}

void RotateLight(LightSet *set, s32 index, f32 x, f32 y) {
    GameLight *light;

    for (light = set->list.head.next; light != &set->list.head; light = light->next) {
        if (light->index == index) {
            light->target[0] += x;
            light->angle[0] += x;
            light->target[1] += y;
            light->angle[1] += y;
            light->target[0] = Aabs(light->target[0]);
            light->angle[0] = Aabs(light->angle[0]);
            light->target[1] = Aabs(light->target[1]);
            light->angle[1] = Aabs(light->angle[1]);
            return;
        }
    }
}

void UpdateLightning(void) {
    s32 i;
    s32 loaded = FALSE;

    for (i = 0; i < 50; i++) {
        if (lightningList[i].timer) {
            if (!loaded) {
                gSPClearGeometryMode(glistp++, G_LIGHTING);
                if (fog.on) {
                    gDPSetRenderMode(glistp++, G_RM_FOG_SHADE_A, G_RM_ZB_CLD_SURF2);
                    gDPSetCombineMode(glistp++, G_CC_MODULATEIA_PRIM, G_CC_PASS2);
                } else {
                    gDPSetRenderMode(glistp++, G_RM_ZB_CLD_SURF, G_RM_ZB_CLD_SURF2);
                    gDPSetCombineMode(glistp++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
                }
                LoadTexture(lightningTexture);
                loaded = TRUE;
            }
            DrawLightning(&lightningList[i].from, &lightningList[i].to, lightningList[i].r, lightningList[i].g,
                          lightningList[i].b, lightningList[i].segments, lightningList[i].flags,
                          lightningList[i].jitter, lightningList[i].width);
            lightningList[i].timer--;
        }
    }
}

void AddLightning(Vec3 *start, Vec3 *end, u8 r, u8 g, u8 b, u8 segments, s16 lifetime, s16 flags, f32 jagginess,
                  f32 thickness) {
    Vec3 mid;
    s32 x;

    AddScaledVector(&mid, start, 0.5f, end, 0.5f);
    if (DistanceBetweenPointsSquared(&mid, &camera.pos) > farPlaneDist * farPlaneDist) {
        return;
    }
    for (x = 0; x < 50; x++) {
        if (!lightningList[x].timer) {
            break;
        }
    }
    if (x == 50) {
        return;
    }
    COPY_VECTOR(&lightningList[x].from, start);
    lightningList[x].to = *end;
    lightningList[x].r = r;
    lightningList[x].g = g;
    lightningList[x].b = b;
    lightningList[x].segments = segments;
    lightningList[x].flags = flags;
    lightningList[x].jitter = jagginess;
    lightningList[x].width = thickness;
    lightningList[x].timer = lifetime;
}

void DrawLightning(Vec3 *start, Vec3 *end, u8 r, u8 g, u8 b, s16 numSegments, s16 flags, f32 jagginess, f32 thickness) {
    Vec3 d;
    f32 step;
    s32 x;

    gDPSetPrimColor(glistp++, 0, 0, r, g, b, 255);
    step = 1.0f / numSegments;
    lightningVect[0] = *start;
    lightningVect[numSegments] = *end;
    SUB_VECTORS(&d, end, start);
    SCALE_VECTOR(&d, step);
    for (x = 1; x < numSegments + 1; x++) {
        ADD_VECTORS(&lightningVect[x], &lightningVect[x - 1], &d);
    }
    step = LENGTH(&d) * 1.4 * jagginess;
    for (x = 1; x < numSegments; x++) {
        if (flags & 4) {
            d.x = Random((u32)step) - step / 2.0f;
            d.y = Random((u32)step) - step / 2.0f;
            d.z = Random((u32)step) - step / 2.0f;
        } else if (flags & 8) {
            d.x = -Random((u32)step);
            d.y = -Random((u32)step);
            d.z = -Random((u32)step);
        } else {
            d.x = Random((u32)step);
            d.y = Random((u32)step);
            d.z = Random((u32)step);
        }
        if (x == 1 || x == numSegments - 1) {
            SCALE_VECTOR(&d, 0.5);
        }
        ADD_VECTORS(&lightningVect[x], &lightningVect[x], &d);
    }
    for (x = 0; x < numSegments + 1; x++) {
        vtxPtr[x * 2].v.ob[0] = (s32)lightningVect[x].x;
        vtxPtr[x * 2].v.ob[1] = (s32)lightningVect[x].y;
        vtxPtr[x * 2].v.ob[2] = (s32)lightningVect[x].z;
        vtxPtr[x * 2].v.flag = 0;
        vtxPtr[x * 2].v.tc[0] = 0;
        vtxPtr[x * 2].v.tc[1] = (x & 1) ? 0x200 : 0;
        vtxPtr[x * 2].v.cn[0] = 255;
        vtxPtr[x * 2].v.cn[1] = 255;
        vtxPtr[x * 2].v.cn[2] = 255;
        vtxPtr[x * 2].v.cn[3] = 255;
        if (flags & 1) {
            d.x = (f32)x * (5.0 / numSegments) + 3.0;
        } else if (flags & 2) {
            if (x < numSegments / 2) {
                d.x = (f32)x * (10.0 / numSegments) + 3.0;
            } else {
                d.x = (f32)(numSegments - x) * (10.0 / numSegments) + 3.0;
            }
        } else {
            d.x = 8.0f;
        }
        d.y = 0.0f;
        d.z = 0.0f;
        d.x *= thickness;
        RotateVector2D(&d, &d, camera.rot.y);
        ADD_VECTORS(&d, &d, &lightningVect[x]);
        vtxPtr[x * 2 + 1].v.ob[0] = (s32)d.x;
        vtxPtr[x * 2 + 1].v.ob[1] = (s32)d.y;
        vtxPtr[x * 2 + 1].v.ob[2] = (s32)d.z;
        vtxPtr[x * 2 + 1].v.flag = 0;
        vtxPtr[x * 2 + 1].v.tc[0] = 0x200;
        vtxPtr[x * 2 + 1].v.tc[1] = (x & 1) ? 0x200 : 0;
        vtxPtr[x * 2 + 1].v.cn[0] = 255;
        vtxPtr[x * 2 + 1].v.cn[1] = 255;
        vtxPtr[x * 2 + 1].v.cn[2] = 255;
        vtxPtr[x * 2 + 1].v.cn[3] = 255;
    }
    gSPVertex(glistp++, vtxPtr, x * 2, 0);
    vtxPtr += x * 2;
    for (x = 0; x < numSegments; x++) {
        gSP2Triangles(glistp++, x * 2, x * 2 + 2, x * 2 + 1, 0, x * 2 + 2, x * 2 + 3, x * 2 + 1, 0);
        gSP2Triangles(glistp++, x * 2, x * 2 + 1, x * 2 + 2, 0, x * 2 + 2, x * 2 + 1, x * 2 + 3, 0);
    }
}

INCLUDE_ASM("asm/nonmatchings/light", pad_8018A9C8);

// linker junk that follows the 8.0f constant emitted by DrawLightning
const u32 D_80109584[] = { 0x00000000, 0x00000000, 0x00000000 };
