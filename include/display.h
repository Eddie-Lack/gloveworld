#ifndef DISPLAY_H
#define DISPLAY_H

#include "common.h"

typedef struct Dynamic {
    /* 0x0000 */ Mtx projection;
    /* 0x0040 */ Mtx translation;
    /* 0x0080 */ Mtx objectMtx[256];
    /* 0x4080 */ Mtx cameraView;
    /* 0x40C0 */ Mtx viewing;
    /* 0x4100 */ Gfx glist[20000];
    /* 0x2B200 */ LookAt lookAt;
    /* 0x2B220 */ Hilite hilite;
} Dynamic;

extern Gfx *glistp;
extern Dynamic *dynamicp;
extern s32 objectMatrix;

#endif
