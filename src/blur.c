#include "common.h"
#include "blur.h"
#include "jalloc.h"
#include "display.h"

extern Vtx templateVert;
extern Gfx blurdl[];

inline void InitBlurFX(Blur *blur) {
    s32 i;

    for (i = 0; i < 15; i++) {
        blur->points[i].alpha1 = 0;
        blur->points[i].alpha2 = 0;
    }
    for (i = 0; i < 30; i++) {
        blur->vtx[0][i] = templateVert;
        blur->vtx[1][i] = templateVert;
    }
}

void StartBlurFX(Blur *blur, Vec3 *p1, Vec3 *p2, u8 alpha1, u8 alpha2, u8 fade1, u8 fade2) {
    blur->index = 0;
    blur->oldest = 0;
    blur->alpha1 = alpha1;
    blur->alpha2 = alpha2;
    blur->fade1 = fade1;
    blur->fade2 = fade2;
    blur->points[blur->index].p1 = *p1;
    blur->points[blur->index].p2 = *p2;
    blur->points[blur->index].alpha1 = alpha1;
    blur->points[blur->index].alpha2 = alpha2;
    blur->index++;
}

void IncrementBlurFX(Blur *blur, Vec3 *point1, Vec3 *point2) {
    blur->points[blur->index].p1 = *point1;
    blur->points[blur->index].p2 = *point2;
    blur->points[blur->index].alpha1 = blur->alpha1;
    blur->points[blur->index].alpha2 = blur->alpha2;
    if (++blur->index == 15) {
        blur->index = 0;
    }
}

void UpdateBlurFX(Blur *blur) {
    s32 i;

    for (i = 0; i < 15; i++) {
        if (blur->points[i].alpha1 < 2) {
            blur->points[i].alpha1 = 0;
        } else if (blur->points[i].alpha1 > blur->fade1) {
            blur->points[i].alpha1 -= blur->fade1;
        } else {
            blur->points[i].alpha1 = 1;
        }
        if (blur->points[i].alpha2 < 2) {
            blur->points[i].alpha2 = 0;
        } else if (blur->points[i].alpha2 > blur->fade2) {
            blur->points[i].alpha2 -= blur->fade2;
        } else {
            blur->points[i].alpha2 = 1;
        }
    }
}

#define ADD_BLUR_POINT(i)                                                    \
    if (blur->points[i].alpha1 == 0 && blur->points[i].alpha2 == 0) {        \
        if (++blur->oldest == 15) {                                          \
            blur->oldest = 0;                                                \
        }                                                                    \
    } else {                                                                 \
        blur->vtx[blur->buffer][curVert].v.ob[0] = blur->points[i].p1.x;     \
        blur->vtx[blur->buffer][curVert].v.ob[1] = blur->points[i].p1.y;     \
        blur->vtx[blur->buffer][curVert].v.ob[2] = blur->points[i].p1.z;     \
        blur->vtx[blur->buffer][curVert++].v.cn[3] = blur->points[i].alpha1; \
        blur->vtx[blur->buffer][curVert].v.ob[0] = blur->points[i].p2.x;     \
        blur->vtx[blur->buffer][curVert].v.ob[1] = blur->points[i].p2.y;     \
        blur->vtx[blur->buffer][curVert].v.ob[2] = blur->points[i].p2.z;     \
        blur->vtx[blur->buffer][curVert++].v.cn[3] = blur->points[i].alpha2; \
    }

#define DRAW_STRIP(count, end)                                               \
    gSPVertex(glistp++, &blur->vtx[blur->buffer][i], count, 0);              \
    for (j = 0; j < (end); j += 2) {                                         \
        gSP2Triangles(glistp++, j, j + 2, j + 1, 0, j + 1, j + 2, j + 3, 0); \
    }                                                                        \
    i += (count) - 2;

void DrawBlurFX(Blur *blur) {
    s32 i;
    s32 j;
    s32 k;
    s32 curVert = 0;

    if (blur->index < blur->oldest) {
        for (i = blur->oldest; i < 15; i++) {
            ADD_BLUR_POINT(i);
        }
        for (i = 0; i < blur->index; i++) {
            ADD_BLUR_POINT(i);
        }
    } else {
        for (i = blur->oldest; i < blur->index; i++) {
            ADD_BLUR_POINT(i);
        }
    }
    i = 0;
    if (curVert < 4) {
        if (blur->idleCount != 255) {
            blur->idleCount++;
        }
        return;
    }
    while (TRUE) {
        if (curVert > 30) {
            DRAW_STRIP(30, 27);
            curVert -= 28;
        } else {
            if (curVert != 0) {
                k = ((curVert - 4) / 2 + 1) * 2;
                DRAW_STRIP(curVert, k);
            }
            break;
        }
    }
    blur->buffer = 1 - blur->buffer;
}

void InitBlurList(void) {
    blurList.count = 0;
    blurList.head.next = blurList.head.prev = &blurList.head;
}

void AddBlur(Blur *blur) {
    Blur *ptr = blurList.head.next;

    InitBlurFX(blur);
    blur->idleCount = 0;
    blur->prev = ptr;
    blur->next = ptr->next;
    ptr->next->prev = blur;
    ptr->next = blur;
    blurList.count++;
}

inline void SubBlur(Blur *blur) {
    if (blur->next != NULL) {
        blur->prev->next = blur->next;
        blur->next->prev = blur->prev;
        blur->next = NULL;
        blurList.count--;
    }
}

void DrawBlurs(void) {
    Blur *blur;

    if (blurList.count) {
        gSPDisplayList(glistp++, blurdl);
        for (blur = blurList.head.next; blur != &blurList.head; blur = blur->next) {
            DrawBlurFX(blur);
        }
    }
}

void UpdateBlurs(void) {
    Blur *blur;
    Blur *next;

    for (blur = blurList.head.next; blur != &blurList.head; blur = next) {
        next = blur->next;
        UpdateBlurFX(blur);
        if (blur->idleCount != 255 && blur->idleCount > 10) {
            SubBlur(blur);
        }
    }
}
