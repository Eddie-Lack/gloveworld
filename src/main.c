#include "common.h"
#include "ident.h"
#include "display.h"
#include "timer.h"
#include "nnsched.h"
#include "game.h"
#include "camera.h"
#include "maths.h"
#include "save.h"
#include "libc.h"
#include "actor.h"
#include "hand.h"
#include "ball.h"
#include "cameo.h"
#include "texture.h"
#include "macros.h"
#include "banks.h"
#include "blur.h"
#include "bullet.h"
#include "buzzer.h"
#include "cheat.h"
#include "collect.h"
#include "controller.h"
#include "debris.h"
#include "footprint.h"
#include "jalloc.h"
#include "light.h"
#include "overlay.h"
#include "platform.h"
#include "puzzle.h"
#include "quake.h"
#include "sound.h"
#include "spike.h"
#include "text.h"
#include "tip.h"
#include "vent.h"
#include "water.h"
#include "wind.h"
#include "boss.h"
#include "crc.h"
#include "dma.h"
#include "eeprom.h"
#include "enemy.h"
#include "frontend.h"
#include "object.h"
#include "personal.h"
#include "shadow.h"

extern OSMesgQueue dmaMessageQ;
extern OSMesgQueue rspMessageQ;
extern OSMesgQueue rdpMessageQ;
extern OSMesgQueue retraceMessageQ;
extern OSMesg dmaMessageBuf;
extern OSMesg rspMessageBuf;
extern OSMesg rdpMessageBuf;
extern OSMesg retraceMessageBuf;
extern s32 lightOffset;
extern u8 animateActors;
extern u8 staticMemory[0x100000];

void MainProc(void *arg);
void IdleProc(void *arg);

extern OSMesgQueue piMessageQ;
extern OSMesg piMessageBuf[];
extern OSThread mainThread;
extern u8 codePriority;
extern u8 idlePriority;
extern OSThread idleThread;
extern u8 currentTask;
extern OSMesg *tlistp;
extern OSMesgQueue *sched_gfxMQ;
extern u16 D_430[];
extern u16 zbuffer[];
extern Vp vp;
extern u8 ShadingMode;
extern u8 ClipRatio;
extern s16 fieldOfView;
extern s16 nearPlaneDist;
extern s16 farPlaneDist;
extern u8 UseZMode;
extern u8 UseAAMode;
extern u8 xluSurf;
extern u8 transparentSurf;
extern s16 xluFact;
extern u8 UseUCode;
extern s32 rdp_output_size;
extern u64 rdp_output[];
extern s32 gfxTasks;
extern OSMesgQueue gfx_msgQ;
extern u32 TicksPerFrame;
extern f32 TicksPerSec;
extern s32 osTvType;
extern u16 *cfb_ptrs[];
extern u8 draw_buffer;
extern u8 gfxIsDrawing;
extern u8 UseTextureMode;
extern s32 actualGFXListLength[];
extern Vec3 actualCamPos[];
extern Vec3 actualCamTargetPos[];
extern Vec3 camVect;
extern s16 controllerBusy;
extern s32 frameStart;
extern OSMesgQueue controllerMsgQ;
extern OSContPad controllerdata[];
extern u8 ActiveController;
extern s32 autoPlayback;
extern u8 eepromMessageQueue[6];
void ReadController(u8 cntlr);
void ReadControllerFromData(u8 cntlr);

void osInitialize(void);

void ControllerProc(void *arg);
void DrawGraphics(void *arg);
extern OSThread ControllerThread;
extern OSThread drawGraphicsThread;
extern OSMesgQueue main_msgQ;
extern OSMesg main_msgbuf[8];
extern NNScClient main_client;
extern u8 graphicsPriority;
extern u8 controllerPriority;
extern s32 levelSelectCount;
extern s32 cheatActivated;
extern u8 validEeprom;
extern HiscoreEntry hiscoreTable[6];
extern HiscoreEntry hiscoreTableCopy[6];
extern BestTime bestTimes[6][5];
extern BestTime bestTimesCopy[6][5];
extern char currentName[3];
extern s32 redrawActors;
extern s32 frameCount;
extern s32 frameCount2;
extern u8 pauseMode;
extern s32 gameOverCount;
extern s32 currentAnim;
void MainGameLoop(void);
extern u8 codeDrawingRequest;
extern CollSphere cameoCameraSphere;
extern CollSphere cameoCameraTargetSphere;
extern u8 dontClearScreen;
extern Gfx *overlay_dl_next;
extern Gfx overlay_dl[];
extern Gfx overlay_dl2[];
extern s32 dispFrameCount;
extern Dynamic dynamic[2];
extern OSMesg tlistPointers[2][10];
extern u8 onlyDrawBackdrops;
extern u8 doneStaticMatrices;
extern u8 gamePaused;
void SetupViewing(void);
void SendLastFrame(void);
void ClearZBuffer(void);
void ClearFrameBuffer(void);
void CleanupAndSendDisplayList(s32 flag, s32 flags);
void RenderObjects(void);
extern NNScClient gfx_client;
extern OSMesg gfx_msgbuf[8];
extern NNScTask tlist[2][10];
extern u8 dram_stack[];
extern u8 gfxYieldBuf[];
extern u32 rdp_output_len;
extern s16 gfx_msg;
extern u16 cfb_16_a[];
extern u8 jallocHeapStart[];
extern u16 cfb_16_b[];
void MeasureFrameTicks(void);
void LoadObjectBank(s32 num);
extern struct Text *cheatText;
extern f32 fovspeed;
extern s16 fieldOfViewAim;
extern Actor fadeOverlay;
extern f32 cameraZoomVal[3];
extern f32 originalCameraZoomVal[3];
extern Texture *lavaTexture;
extern Texture *bubbleTexture;
extern Vec3 oldHandPos;
extern s32 controlMethod;

void debugPrintf(s32 len) {
}

void Crash(char *msg) {
    PRINTF("\n\nCRASHED %s!!!!!!!!!!!!!!!\n", msg);
}

void boot(void) {
    u32 i;
    u32 *addr;
    u32 buf[16];

    osInitialize();
    addr = (u32 *)0xFFB000;
    for (i = 0; i < 16; i++, addr++) {
        osPiRawReadIo((u32)addr, &buf[i]);
    }
    osCreateThread(&idleThread, 1, IdleProc, NULL, piMessageBuf, idlePriority);
    osStartThread(&idleThread);
}

void IdleProc(void *arg) {
    osCreatePiManager(OS_PRIORITY_PIMGR, &piMessageQ, piMessageBuf, 12);
    osCreateThread(&mainThread, 3, MainProc, arg, &mainThread, codePriority);
    osStartThread(&mainThread);
    osSetThreadPri(NULL, 0);
    for (;;) {
    }
}

void CreateMessageQueues(void) {
    osCreateMesgQueue(&dmaMessageQ, &dmaMessageBuf, 1);
    osCreateMesgQueue(&rspMessageQ, &rspMessageBuf, 1);
    osCreateMesgQueue(&rdpMessageQ, &rdpMessageBuf, 1);
    osCreateMesgQueue(&retraceMessageQ, &retraceMessageBuf, 1);
}

void CreateTaskStructure(s32 i, s32 flag) {
    ((NNScTask *)tlistp[i])->list.t.ucode_boot = (u64 *)rspbootTextStart;
    ((NNScTask *)tlistp[i])->list.t.ucode_boot_size = (u32)rspbootTextEnd - (u32)rspbootTextStart;
    switch (UseUCode) {
        case 4:
            ((NNScTask *)tlistp[i])->list.t.ucode = (u64 *)gspF3DEX_fifoTextStart;
            ((NNScTask *)tlistp[i])->list.t.ucode_data = (u64 *)gspF3DEX_fifoDataStart;
            ((NNScTask *)tlistp[i])->list.t.output_buff_size = &rdp_output[rdp_output_size];
            ((NNScTask *)tlistp[i])->list.t.flags = 0;
            break;
    }
    if (flag) {
        ((NNScTask *)tlistp[i])->list.t.ucode = (u64 *)gspSprite2DTextStart;
        ((NNScTask *)tlistp[i])->list.t.ucode_data = (u64 *)gspSprite2DDataStart;
    }
    ((NNScTask *)tlistp[i + 1])->list.t.data_ptr = (u64 *)glistp;
}

void ApplyGlobalTransformations(void) {
    guTranslate(&dynamicp->translation, 0.0f, 0.0f, 0.0f);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&dynamicp->translation), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
}

void SetupViewing(void) {
    u16 perspNorm;

    guPerspective(&dynamicp->projection, &perspNorm, fieldOfView, 4.0f / 3.0f, nearPlaneDist, farPlaneDist, 1.0f);
    RotateVector2D(&camVect, &cameraUpVect, camera.rot.y);
    if (ShadingMode == 2) {
        guLookAtHilite(&dynamicp->cameraView, &dynamicp->lookAt, &dynamicp->hilite, actualCamPos[draw_buffer].x,
                       actualCamPos[draw_buffer].y, actualCamPos[draw_buffer].z, actualCamTargetPos[draw_buffer].x,
                       actualCamTargetPos[draw_buffer].y, actualCamTargetPos[draw_buffer].z, camVect.x, camVect.y,
                       camVect.z, 0, 0, 127.0f, 0, 0, 127.0f, 32, 32);
    } else {
        guLookAt(&dynamicp->cameraView, actualCamPos[draw_buffer].x, actualCamPos[draw_buffer].y,
                 actualCamPos[draw_buffer].z, actualCamTargetPos[draw_buffer].x, actualCamTargetPos[draw_buffer].y,
                 actualCamTargetPos[draw_buffer].z, camVect.x, camVect.y, camVect.z);
    }
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&dynamicp->projection), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&dynamicp->cameraView), G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
    gSPPerspNormalize(glistp++, perspNorm);
}

void ClearViewing(void) {
    u16 perspNorm;

    gDPPipeSync(glistp++);
    guPerspective(&dynamicp->projection, &perspNorm, fieldOfView, 4.0f / 3.0f, nearPlaneDist, farPlaneDist, 1.0f);
    guLookAt(&dynamicp->viewing, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&dynamicp->projection), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&dynamicp->viewing), G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
    gSPPerspNormalize(glistp++, perspNorm);
    UseZMode = 0;
}

void SetRenderMode(void) {
    gDPPipeSync(glistp++);
    gSPSetGeometryMode(glistp++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH);
    if (fog.on == 1) {
        gSPSetGeometryMode(glistp++, G_FOG);
        gDPSetCycleType(glistp++, G_CYC_2CYCLE);
        if (xluSurf) {
            if (transparentSurf) {
                gDPSetCombineLERP(glistp++, 0, 0, 0, TEXEL0, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, COMBINED, 0, 0, 0, COMBINED);
            } else {
                gDPSetCombineLERP(glistp++, TEXEL0, 0, SHADE, 0, 0, 0, 0, PRIMITIVE, 0, 0, 0, COMBINED, 0, 0, 0, COMBINED);
            }
            gDPSetPrimColor(glistp++, 0, 0, 0, 0, 0, xluFact);
            if (UseAAMode == 0 && UseZMode != 0) {
                gSPSetGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_FOG_SHADE_A, G_RM_AA_ZB_XLU_SURF2);
            } else if (UseAAMode == 2 && UseZMode != 0) {
                gSPSetGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_FOG_SHADE_A, G_RM_ZB_XLU_SURF2);
            } else if (UseAAMode == 0 && UseZMode == 0) {
                gSPClearGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_FOG_SHADE_A, G_RM_AA_XLU_SURF2);
            } else if (UseAAMode == 1 && UseZMode != 0) {
                gSPSetGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_FOG_SHADE_A, G_RM_RA_ZB_OPA_SURF2);
            } else if (UseAAMode == 1 && UseZMode == 0) {
                gSPClearGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_FOG_SHADE_A, G_RM_RA_OPA_SURF2);
            } else {
                gSPClearGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_FOG_SHADE_A, G_RM_XLU_SURF2);
            }
        } else if (transparentSurf) {
            gDPSetCombineMode(glistp++, G_CC_MODULATEIDECALA, G_CC_PASS2);
            if (UseAAMode == 0 && UseZMode != 0) {
                gSPSetGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_FOG_SHADE_A, G_RM_AA_ZB_TEX_EDGE2);
            } else if (UseAAMode == 2 && UseZMode != 0) {
                gSPSetGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_FOG_SHADE_A, G_RM_AA_ZB_TEX_EDGE2);
            } else if (UseAAMode == 0 && UseZMode == 0) {
                gSPClearGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_FOG_SHADE_A, G_RM_AA_TEX_EDGE2);
            } else if (UseAAMode == 1 && UseZMode != 0) {
                gSPSetGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_FOG_SHADE_A, G_RM_AA_ZB_TEX_EDGE2);
            } else if (UseAAMode == 1 && UseZMode == 0) {
                gSPClearGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_FOG_SHADE_A, G_RM_AA_TEX_EDGE2);
            } else {
                gSPClearGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_FOG_SHADE_A, G_RM_TEX_EDGE2);
            }
        } else {
            gDPSetCombineLERP(glistp++, TEXEL0, 0, SHADE, 0, 0, 0, 0, PRIMITIVE, 0, 0, 0, COMBINED, 0, 0, 0, COMBINED);
            if (UseAAMode == 0 && UseZMode != 0) {
                gSPSetGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_FOG_SHADE_A, G_RM_AA_ZB_OPA_SURF2);
            } else if (UseAAMode == 2 && UseZMode != 0) {
                gSPSetGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_FOG_SHADE_A, G_RM_ZB_OPA_SURF2);
            } else if (UseAAMode == 0 && UseZMode == 0) {
                gSPClearGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_FOG_SHADE_A, G_RM_AA_OPA_SURF2);
            } else if (UseAAMode == 1 && UseZMode != 0) {
                gSPSetGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_FOG_SHADE_A, G_RM_RA_ZB_OPA_SURF2);
            } else if (UseAAMode == 1 && UseZMode == 0) {
                gSPClearGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_FOG_SHADE_A, G_RM_RA_OPA_SURF2);
            } else {
                gSPClearGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_FOG_SHADE_A, G_RM_OPA_SURF2);
            }
        }
    } else {
        if (transparentSurf) {
            gDPSetCombineMode(glistp++, G_CC_MODULATEI, G_CC_MODULATEIA);
        } else {
            gDPSetCombineLERP(glistp++, TEXEL0, 0, SHADE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0, SHADE, 0, 0, 0, 0, PRIMITIVE);
        }
        gDPSetPrimColor(glistp++, 0, 0, 0, 0, 0, xluFact);
        if (xluSurf) {
            if (UseAAMode == 0 && UseZMode != 0) {
                gSPSetGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2);
            } else if (UseAAMode == 2 && UseZMode != 0) {
                gSPSetGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2);
            } else if (UseAAMode == 0 && UseZMode == 0) {
                gSPClearGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
            } else if (UseAAMode == 1 && UseZMode != 0) {
                gSPSetGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_RA_ZB_OPA_SURF, G_RM_RA_ZB_OPA_SURF2);
            } else if (UseAAMode == 1 && UseZMode == 0) {
                gSPClearGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_RA_OPA_SURF, G_RM_RA_OPA_SURF2);
            } else {
                gSPClearGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_XLU_SURF, G_RM_XLU_SURF2);
            }
        } else if (transparentSurf) {
            gDPSetCombineMode(glistp++, G_CC_MODULATEI, G_CC_MODULATEIA);
            if (UseAAMode == 0 && UseZMode != 0) {
                gSPSetGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_AA_ZB_TEX_EDGE, G_RM_AA_ZB_TEX_EDGE2);
            } else if (UseAAMode == 2 && UseZMode != 0) {
                gSPSetGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_AA_ZB_TEX_EDGE, G_RM_AA_ZB_TEX_EDGE2);
            } else if (UseAAMode == 0 && UseZMode == 0) {
                gSPClearGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_AA_TEX_EDGE, G_RM_AA_TEX_EDGE2);
            } else if (UseAAMode == 1 && UseZMode != 0) {
                gSPSetGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_AA_ZB_TEX_EDGE, G_RM_AA_ZB_TEX_EDGE2);
            } else if (UseAAMode == 1 && UseZMode == 0) {
                gSPClearGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_AA_TEX_EDGE, G_RM_AA_TEX_EDGE2);
            } else {
                gSPClearGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_TEX_EDGE, G_RM_TEX_EDGE2);
            }
        } else {
            if (UseAAMode == 0 && UseZMode != 0) {
                gSPSetGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
            } else if (UseAAMode == 2 && UseZMode != 0) {
                gSPSetGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
            } else if (UseAAMode == 0 && UseZMode == 0) {
                gSPClearGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_AA_OPA_SURF, G_RM_AA_OPA_SURF2);
            } else if (UseAAMode == 1 && UseZMode != 0) {
                gSPSetGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_RA_ZB_OPA_SURF, G_RM_RA_ZB_OPA_SURF2);
            } else if (UseAAMode == 1 && UseZMode == 0) {
                gSPClearGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_RA_OPA_SURF, G_RM_RA_OPA_SURF2);
            } else {
                gSPClearGeometryMode(glistp++, G_ZBUFFER);
                gDPSetRenderMode(glistp++, (UseTextureMode == 2) ? G_RM_PASS : G_RM_OPA_SURF, G_RM_OPA_SURF2);
            }
        }
    }
}

void SetClipRatio(void) {
    switch (ClipRatio) {
        case 1:
            gSPClipRatio(glistp++, FRUSTRATIO_1);
            break;
        case 2:
            gSPClipRatio(glistp++, FRUSTRATIO_2);
            break;
        case 3:
            gSPClipRatio(glistp++, FRUSTRATIO_3);
            break;
        case 4:
            gSPClipRatio(glistp++, FRUSTRATIO_4);
            break;
        case 5:
            gSPClipRatio(glistp++, FRUSTRATIO_5);
            break;
        case 6:
            gSPClipRatio(glistp++, FRUSTRATIO_6);
            break;
    }
}

void RenderObjects(void) {
    StartTimer(3, "NEAR");
    lightOffset = 0;
    EndTimer(3);
    CreateObjects();
    animateActors = FALSE;
}

void InitViewport(void) {
    vp.vp.vscale[0] = 640;
    vp.vp.vscale[1] = 480;
    vp.vp.vscale[2] = 511;
    vp.vp.vscale[3] = 0;
    vp.vp.vtrans[0] = 640;
    vp.vp.vtrans[1] = 480;
    vp.vp.vtrans[2] = 511;
    vp.vp.vtrans[3] = 0;
    gSPViewport(glistp++, &vp);
    gSPClearGeometryMode(glistp++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BOTH | G_FOG | G_LIGHTING |
                                       G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD);
    gSPSetGeometryMode(glistp++, G_CULL_BACK);
    if (ShadingMode == 1) {
        gSPSetGeometryMode(glistp++, G_SHADE | G_SHADING_SMOOTH);
    }
    if (ShadingMode == 2) {
        gSPSetGeometryMode(glistp++, G_SHADE | G_SHADING_SMOOTH | G_LIGHTING);
    }
}

void InitRdpState(void) {
    gDPPipelineMode(glistp++, G_PM_NPRIMITIVE);
    gDPSetScissor(glistp++, G_SC_NON_INTERLACE, 0, 0, 320, 240);
    gDPSetTextureLOD(glistp++, G_TL_TILE);
    gDPSetTextureLUT(glistp++, G_TT_NONE);
    gDPSetTextureDetail(glistp++, G_TD_CLAMP);
    gDPSetTexturePersp(glistp++, G_TP_PERSP);
    gDPSetTextureFilter(glistp++, G_TF_BILERP);
    gDPSetTextureConvert(glistp++, G_TC_FILT);
    gDPSetCombineKey(glistp++, G_CK_NONE);
    gDPSetAlphaCompare(glistp++, G_AC_NONE);
    gDPSetBlendColor(glistp++, 0xFF, 0xFF, 0xFF, 0);
    gDPSetColorDither(glistp++, G_CD_MAGICSQ);
    gDPSetDepthImage(glistp++, zbuffer);
    gDPSetPrimColor(glistp++, 0, 0, 0, 0, 0, 255);
    gDPSetDepthSource(glistp++, G_ZS_PIXEL);
    gDPPipeSync(glistp++);
}

void InitDisplayLists(void) {
    InitRdpState();
    InitViewport();
    if (UseTextureMode) {
        gDPSetTextureFilter(glistp++, G_TF_BILERP);
        if (UseTextureMode == 2) {
            gSPTexture(glistp++, 0xFFFF, 0xFFFF, 5, G_TX_RENDERTILE, G_ON);
            gDPSetTextureDetail(glistp++, G_TD_CLAMP);
            gDPSetTextureLOD(glistp++, G_TL_LOD);
            gDPPipelineMode(glistp++, G_PM_1PRIMITIVE);
            if (ShadingMode == 0) {
                gDPSetCombineMode(glistp++, G_CC_TRILERP, G_CC_DECALRGB2);
            } else {
                gDPSetCombineMode(glistp++, G_CC_TRILERP, G_CC_MODULATERGB2);
            }
        } else {
            gSPTexture(glistp++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
            if (ShadingMode == 0) {
                gDPSetCombineMode(glistp++, G_CC_DECALRGB, G_CC_DECALRGB);
            } else {
                gDPSetCombineMode(glistp++, G_CC_MODULATERGB, G_CC_MODULATERGB);
            }
        }
    } else {
        gSPTexture(glistp++, 0, 0, 0, G_TX_RENDERTILE, G_OFF);
        if (ShadingMode == 0) {
            gDPSetCombineMode(glistp++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
        } else {
            gDPSetCombineMode(glistp++, G_CC_SHADE, G_CC_SHADE);
        }
    }
}

void ClearFrameBuffer(void) {
    gDPSetCycleType(glistp++, G_CYC_FILL);
    gDPSetColorImage(glistp++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, (u32) cfb_ptrs[draw_buffer] + 0x80000000);
    if (gfxIsDrawing) {
        gDPSetFillColor(glistp++, GPACK_RGBA5551(fog.r, fog.g, fog.b, 1) << 16 | GPACK_RGBA5551(fog.r, fog.g, fog.b, 1));
    } else {
        gDPSetFillColor(glistp++, GPACK_RGBA5551(0, 0, 0, 1) << 16 | GPACK_RGBA5551(0, 0, 0, 1));
    }
    gDPFillRectangle(glistp++, 0, 0, 319, 239);
    gDPPipeSync(glistp++);
    if (UseTextureMode == 2) {
        gDPSetCycleType(glistp++, G_CYC_2CYCLE);
    } else {
        gDPSetCycleType(glistp++, G_CYC_1CYCLE);
    }
}

void ClearZBuffer(void) {
    gDPSetColorImage(glistp++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, D_430);
    gDPPipeSync(glistp++);
    gDPSetCycleType(glistp++, G_CYC_FILL);
    gDPSetFillColor(glistp++, 0xFFFCFFFC);
    gDPFillRectangle(glistp++, 0, 0, 319, 239);
    gDPPipeSync(glistp++);
}

void CleanupAndSendDisplayList(s32 flag, s32 flags) {
    s32 len;

    gDPFullSync(glistp++);
    gSPEndDisplayList(glistp++);
    ((NNScTask *)tlistp[currentTask])->list.t.data_size = (glistp - (Gfx *)((NNScTask *)tlistp[currentTask])->list.t.data_ptr) * sizeof(Gfx);
    actualGFXListLength[currentTask] = ((NNScTask *)tlistp[currentTask])->list.t.data_size;
    if (((NNScTask *)tlistp[currentTask])->list.t.data_size >= 20000) {
        PRINTF("Overran the graphics list.......");
        return;
        Crash("GFX LIST");
    }
    CreateTaskStructure(currentTask, flag);
    ((NNScTask *)tlistp[currentTask])->flags = flags;
    ((NNScTask *)tlistp[currentTask])->framebuffer = cfb_ptrs[draw_buffer];
    if (flags == 0x40) {
        len = (glistp - dynamicp->glist) * sizeof(Gfx);
        draw_buffer ^= 1;
    }
    currentTask++;
}

void SendLastFrame(void) {
    s32 i;

    if (currentTask == 1) {
        return;
    }
    osWritebackDCache(dynamicp, 0x2B230);
    for (i = 1; i < currentTask; i++) {
        osSendMesg(sched_gfxMQ, tlistp[i], OS_MESG_BLOCK);
        gfxTasks++;
    }
}

void RunGame(void *arg) {
    s16 *msg = NULL;

    ActiveController = initControllers();
    ActiveController = initControllers();
    InitEepromMessageQueue();
    osCreateThread(&ControllerThread, 6, ControllerProc, arg, &ControllerThread, controllerPriority);
    osStartThread(&ControllerThread);
    InitEeprom();
    osCreateMesgQueue(&main_msgQ, main_msgbuf, 8);
    nnScAddClient(&sched, &main_client, &main_msgQ);
    osCreateThread(&drawGraphicsThread, 4, DrawGraphics, arg, &drawGraphicsThread, graphicsPriority);
    osStartThread(&drawGraphicsThread);
    levelSelectCount = 0;
    cheatActivated = 0;
    if (validEeprom) {
        LoadHiscoreTable();
        LoadBestTimes();
        LoadSlots();
        LoadCRC();
        CheckCRC();
    } else {
        memcpy(hiscoreTable, hiscoreTableCopy, sizeof(hiscoreTable));
        memcpy(bestTimes[0], bestTimesCopy[0], sizeof(bestTimes[0]));
        memcpy(bestTimes[1], bestTimesCopy[1], sizeof(bestTimes[1]));
        memcpy(bestTimes[2], bestTimesCopy[2], sizeof(bestTimes[2]));
        memcpy(bestTimes[3], bestTimesCopy[3], sizeof(bestTimes[3]));
        memcpy(bestTimes[4], bestTimesCopy[4], sizeof(bestTimes[4]));
        memcpy(bestTimes[5], bestTimesCopy[5], sizeof(bestTimes[5]));
        SaveHiscoreTable();
        SaveBestTimes();
        SaveSlots();
        SaveID();
        SaveCRC();
        currentName[0] = 'B';
        currentName[1] = 'T';
        currentName[2] = 'A';
    }
    gameInfo.mode = 1;
    while (TRUE) {
        osRecvMesg(&main_msgQ, (OSMesg *)&msg, OS_MESG_BLOCK);
        ClearText();
        if (*msg == 9) {
            if (gameInfo.mode == 2) {
                LevelSelectScreen();
            } else if (gameInfo.mode == 1) {
                DoMenu();
            } else if (gameInfo.mode == 15) {
                TimeTrialScreen();
            }
        }
        if (*msg == 9) {
            switch (gameInfo.mode) {
                case 2:
                case 15:
                    break;
                case 1:
                    animateActors = 1;
                    redrawActors = 0;
                    frameCount++;
                    frameCount2++;
                    break;
                case 5:
                    if (!(gameInfo.exitFlags & 1) && gameInfo.level != OUTRO) {
                        if (AnyButtonPressed(ActiveController)
                            || (actorList.head.next->anim != NULL && actorList.head.next->anim->finishedAnim != 0)) {
                            ExitLevel(CAMEO_EXIT, 1, 1);
                        }
                    }
                case 4:
                    if (frameCount == 1) {
                        StartDrawing("doPolu");
                    }
                    MainGameLoop();
                    ProcessPersonalLoop();
                    animateActors = 1;
                    redrawActors = 1;
                    if (!pauseMode) {
                        frameCount++;
                    }
                    frameCount2++;
                    CheckForExits();
                    if (gameOverCount) {
                        GameOverScreen();
                    }
                    currentAnim = 0;
                    break;
                case 7:
                    EndOfLevelScreen();
                    break;
                case 8:
                    EndOfSubLevelScreen();
                    break;
                case 9:
                    EndOfWorldScreen();
                    break;
                case 10:
                    StartCurrentLevel();
                    break;
                case 13:
                    NewWorldScreen();
                    break;
                case 14:
                    animateActors = 1;
                    SelectLanguage();
                    break;
            }
        }
    }
}

void ControllerProc(void *arg) {
    OSMesgQueue msgQ;
    OSMesg msgBuf[8];
    NNScClient client;
    NNScMsg *msg = NULL;
    s32 busy;

    osCreateMesgQueue(&msgQ, msgBuf, 8);
    nnScAddClient(&sched, &client, &msgQ);
    while (TRUE) {
        if (!controllerBusy) {
            osRecvMesg(&msgQ, (OSMesg *)&msg, OS_MESG_BLOCK);
            if (*msg == 9) {
                frameStart = osGetCount();
                ClearTimers();
                StartTimer(2, "CONTROL");
                osContStartReadData(&controllerMsgQ);
                osRecvMesg(&controllerMsgQ, NULL, OS_MESG_BLOCK);
                osContGetReadData(controllerdata);
                if (autoPlayback == 2) {
                    ReadControllerFromData(ActiveController);
                } else {
                    ReadController(ActiveController);
                }
                EndTimer(2);
                if (eepromMessageQueue[0]) {
                    busy = controllerBusy;
                    controllerBusy = 1;
                    switch (eepromMessageQueue[0]) {
                        case 1:
                            EepromSaveHiscoreTable();
                            break;
                        case 2:
                            EepromLoadHiscoreTable();
                            break;
                        case 4:
                            EepromSaveBestTimes();
                            break;
                        case 5:
                            EepromLoadBestTimes();
                            break;
                        case 6:
                            EepromSaveSlots();
                            break;
                        case 7:
                            EepromLoadSlots();
                            break;
                        case 8:
                            EepromValid();
                            break;
                        case 9:
                            EepromSaveID();
                            break;
                        case 11:
                            EepromSaveCRC();
                            break;
                        case 10:
                            EepromLoadCRC();
                            break;
                    }
                    GetEepromMessage();
                    controllerBusy = busy;
                }
            }
        }
    }
}

void DrawGraphics(void *arg) {
    s32 len = 0; // computed but never used
    OSMesg msg = NULL;
    f32 moved;
    f32 dist;

    while (TRUE) {
        osRecvMesg(&gfx_msgQ, &msg, OS_MESG_BLOCK);
        switch (*(s16 *)msg) {
            case 9:
                if (!gfxTasks) {
                    if (codeDrawingRequest == 1) {
                        gfxIsDrawing = 1;
                    }
                    if (codeDrawingRequest == 0) {
                        gfxIsDrawing = 0;
                    }
                }
                if (gameInfo.level >= INTRO) {
                    if (actorList.head.next->anim == NULL || actorList.head.next->anim->finishedAnim == 0) {
                        camera.pos = cameoCameraSphere.pos;
                        cameraInfo.actor.pos = cameoCameraTargetSphere.pos;
                    }
                    hand.alpha = 0;
                    ball.alpha = 0;
                }
                actualCamPos[draw_buffer] = camera.pos;
                actualCamTargetPos[draw_buffer] = cameraInfo.actor.pos;
                CalcCameraRot();
                if (gameInfo.mode != 1 && gameInfo.mode != 2 && frameCount > 10 && gameInfo.level >= INTRO) {
                    if (gameInfo.level == SPACEBOSS3) {
                        dist = 1e8f;
                    } else {
                        dist = 2500.0f;
                    }
                    if (FindShortestAngle(camera.rot.x, cameraInfo.prevRotX) > 0.5
                        || FindShortestAngle(camera.rot.y, cameraInfo.prevRotY) > 0.5
                        || (DistanceBetweenPointsSquared(&camera.pos, &camera.prevPos) > dist
                            && (moved = DistanceBetweenPointsSquared(&cameraInfo.actor.pos, &cameraInfo.actor.prevPos) > dist))) {
                        camera.prevPos = camera.pos;
                        cameraInfo.actor.prevPos = cameraInfo.actor.pos;
                        SetupViewing();
                    }
                }
                if (dontClearScreen) {
                    break;
                }
                if (draw_buffer) {
                    overlay_dl_next = overlay_dl;
                } else {
                    overlay_dl_next = overlay_dl2;
                }
                dispFrameCount++;
                StartTimer(1, "GRAPHICS");
                SendLastFrame();
                dynamicp = &dynamic[draw_buffer];
                tlistp = tlistPointers[draw_buffer];
                glistp = dynamicp->glist;
                currentTask = 1;
                ((NNScTask *)tlistp[currentTask])->list.t.data_ptr = (u64 *)glistp;
                InitDisplayLists();
                ClearZBuffer();
                ClearFrameBuffer();
                CleanupAndSendDisplayList(0, 0);
                if (gfxIsDrawing || onlyDrawBackdrops) {
                    EndTimer(1);
                    if (!doneStaticMatrices) {
                        doneStaticMatrices = 1;
                        CalculateStaticMatrices();
                    }
                    StartTimer(1, "GRAPHICS");
                    SetupViewing();
                    if (gameInfo.mode == 2 || gameInfo.mode == 15 || GameBeingPlayed()) {
                        UpdateBackSprites();
                    }
                    PrintBackdrops();
                    CleanupAndSendDisplayList(1, 0);
                    EndTimer(1);
                    if (onlyDrawBackdrops) {
                        // empty branch forces the original jump layout
                    } else {
                        if (gameInfo.mode != 13) {
                            if (!gamePaused && frameCount > 2) {
                                DeformTextures();
                            }
                            RenderObjects();
                        }
                        StartTimer(1, "GRAPHICS");
                        PrintOverlays();
                        ShowText();
                        PrintOverlaysBehind();
                        PrintFadeOverlay();
                        CleanupAndSendDisplayList(0, 0);
                    }
                }
                PrintTimers();
                CleanupAndSendDisplayList(0, 0x40);
                len = glistp - dynamicp->glist;
                UpdateFade();
                EndTimer(1);
                break;
            case 2:
                gfxTasks--;
                break;
        }
    }
}

void MeasureFrameTicks(void) {
    u32 start;
    u32 end;

    osRecvMesg(&gfx_msgQ, NULL, OS_MESG_BLOCK);
    osRecvMesg(&gfx_msgQ, NULL, OS_MESG_BLOCK);
    osRecvMesg(&gfx_msgQ, NULL, OS_MESG_BLOCK);
    osRecvMesg(&gfx_msgQ, NULL, OS_MESG_BLOCK);
    osRecvMesg(&gfx_msgQ, NULL, OS_MESG_BLOCK);
    osRecvMesg(&gfx_msgQ, NULL, OS_MESG_BLOCK);
    osRecvMesg(&gfx_msgQ, NULL, OS_MESG_BLOCK);
    osRecvMesg(&gfx_msgQ, NULL, OS_MESG_BLOCK);
    osRecvMesg(&gfx_msgQ, NULL, OS_MESG_BLOCK);
    osRecvMesg(&gfx_msgQ, NULL, OS_MESG_BLOCK);
    osRecvMesg(&gfx_msgQ, NULL, OS_MESG_BLOCK);
    osRecvMesg(&gfx_msgQ, NULL, OS_MESG_BLOCK);
    start = osGetCount();
    osRecvMesg(&gfx_msgQ, NULL, OS_MESG_BLOCK);
    end = osGetCount();
    TicksPerFrame = end - start;
    TicksPerSec = (end - start) * (osTvType == 0 ? 50.0 : 60.0);
}

void MainProc(void *arg) {
    s32 i;
    s32 numFields = 1;

    InitTimers();
    switch (osTvType) {
        case 0:
            while (TRUE) {
            }
            break;
        case 1:
            nnScCreateScheduler(&sched, OS_VI_NTSC_LAN1, numFields);
            break;
        case 2:
            nnScCreateScheduler(&sched, OS_VI_MPAL_LAN1, numFields);
            break;
    }
    fog.r = fog.g = fog.b = 0;
    osCreateMesgQueue(&gfx_msgQ, gfx_msgbuf, 8);
    nnScAddClient(&sched, &gfx_client, &gfx_msgQ);
    sched_gfxMQ = nnScGetGfxMQ(&sched);
    gfx_msg = 2;
    osViSetSpecialFeatures(OS_VI_GAMMA_OFF | OS_VI_GAMMA_DITHER_ON | OS_VI_DIVOT_OFF | OS_VI_DITHER_FILTER_ON);
    JallocInit((u32)jallocHeapStart, 0x80400000 - (u32)jallocHeapStart);
    tlistp = tlistPointers[0];
    for (i = 0; i < 10; i++) {
        tlistp[i] = &tlist[0][i];
        ((NNScTask *)tlistp[i])->list.t.type = M_GFXTASK;
        ((NNScTask *)tlistp[i])->list.t.flags = 0;
        ((NNScTask *)tlistp[i])->list.t.ucode_size = SP_UCODE_SIZE;
        ((NNScTask *)tlistp[i])->list.t.ucode_data_size = SP_UCODE_DATA_SIZE;
        ((NNScTask *)tlistp[i])->list.t.dram_stack = (u64 *)(((u32)dram_stack + 15) & ~15);
        ((NNScTask *)tlistp[i])->list.t.dram_stack_size = SP_DRAM_STACK_SIZE8;
        ((NNScTask *)tlistp[i])->list.t.output_buff = rdp_output;
        ((NNScTask *)tlistp[i])->list.t.output_buff_size = (u64 *)&rdp_output_len;
        ((NNScTask *)tlistp[i])->list.t.yield_data_ptr = (u64 *)gfxYieldBuf;
        ((NNScTask *)tlistp[i])->list.t.yield_data_size = OS_YIELD_DATA_SIZE;
        ((NNScTask *)tlistp[i])->next = NULL;
        ((NNScTask *)tlistp[i])->flags = 0;
        ((NNScTask *)tlistp[i])->msgQ = &gfx_msgQ;
        ((NNScTask *)tlistp[i])->msg = (OSMesg)&gfx_msg;
    }
    tlistp = tlistPointers[1];
    for (i = 0; i < 10; i++) {
        tlistp[i] = &tlist[1][i];
        ((NNScTask *)tlistp[i])->list.t.type = M_GFXTASK;
        ((NNScTask *)tlistp[i])->list.t.flags = 0;
        ((NNScTask *)tlistp[i])->list.t.ucode_size = SP_UCODE_SIZE;
        ((NNScTask *)tlistp[i])->list.t.ucode_data_size = SP_UCODE_DATA_SIZE;
        ((NNScTask *)tlistp[i])->list.t.dram_stack = (u64 *)(((u32)dram_stack + 15) & ~15);
        ((NNScTask *)tlistp[i])->list.t.dram_stack_size = SP_DRAM_STACK_SIZE8;
        ((NNScTask *)tlistp[i])->list.t.output_buff = rdp_output;
        ((NNScTask *)tlistp[i])->list.t.output_buff_size = (u64 *)&rdp_output_len;
        ((NNScTask *)tlistp[i])->list.t.yield_data_ptr = (u64 *)gfxYieldBuf;
        ((NNScTask *)tlistp[i])->list.t.yield_data_size = OS_YIELD_DATA_SIZE;
        ((NNScTask *)tlistp[i])->next = NULL;
        ((NNScTask *)tlistp[i])->flags = 0;
        ((NNScTask *)tlistp[i])->msgQ = &gfx_msgQ;
        ((NNScTask *)tlistp[i])->msg = (OSMesg)&gfx_msg;
    }
    dynamicp = &dynamic[draw_buffer];
    tlistp = tlistPointers[draw_buffer];
    cfb_ptrs[0] = cfb_16_a;
    cfb_ptrs[1] = cfb_16_b;
    CreateMessageQueues();
    MeasureFrameTicks();
    InitCRCTable();
    InitLinkedLists();
    InitAtan();
    InitMatrixStack();
    ResetCamera();
    InitTextList();
    LoadFont();
    InitMusicDriver();
    StopDrawing("main");
    LoadObjectBank(0);
    RunGame(arg);
}

void ClearStaticMemory(void) {
    s32 i;

    for (i = 0; i < 0x100000; i++) {
        staticMemory[i] = 0;
    }
}

void MainGameLoop(void) {
    Vec3 tempVect;
    f32 mul;

    UpdateAmbientSounds();
    if (cheatText) {
        UpdateCheatText();
    }
    StartTimer(0, "Main");
    if (!gamePaused) {
        fieldOfView += MIN(MAX(-fovspeed * 3, (fieldOfViewAim - fieldOfView) / 4), fovspeed);
        if (fieldOfView > 48) {
            fadeOverlay.scale.x = fadeOverlay.scale.y = fadeOverlay.scale.z = 4.0f;
        } else {
            fadeOverlay.scale.x = fadeOverlay.scale.y = fadeOverlay.scale.z = 1.0f;
        }
        mul = 301.87 / sqrtf(fieldOfView * fieldOfView * fieldOfView);
        cameraZoomVal[0] = originalCameraZoomVal[0] * mul;
        cameraZoomVal[1] = originalCameraZoomVal[1] * mul < 0.3 ? 0.3f : originalCameraZoomVal[1] * mul;
        cameraZoomVal[2] = originalCameraZoomVal[2] * mul < 0.5 ? 0.5f : originalCameraZoomVal[2] * mul;
        HandleScreenDebris();
        CycleTextures();
        updateLava(lavaTexture->data, lavaTexture->width, lavaTexture->height);
        if (gameInfo.level == ATLANTISBOSS) {
            updateLava(bubbleTexture->data, bubbleTexture->width, bubbleTexture->height);
        }
    }
    if (GameBeingPlayed()) {
        if (!gamePaused) {
            RestartPlayer();
        }
        cameraInfo.noCameraAction = 1;
        if (!gamePaused) {
            ProcessCollectables();
            UpdateWater();
            UpdateSpikes();
            UpdateDebris();
            CheckPuzzles();
            UpdateWinds();
            UpdatePlatforms();
            UpdateBuzzers();
        }
        UpdateTips();
        EndTimer(0);
    }
    if (!gamePaused) {
        StartTimer(0, "Main");
        if (!cameraInfo.overrideTimer || !(cameraInfo.overrideFlags & 4)) {
            UpdateEnemies();
        }
        EndTimer(0);
        UpdateFootprints();
    }
    if (GameBeingPlayed() && !gamePaused) {
        StartTimer(0, "Main");
        handControl.touchedActor = 0;
        tempVect = ball.vel;
        oldHandPos = hand.pos;
        ballControl.hitFlags &= ~8;
        ball.flags &= ~0x40000000;
        UpdateActor(&hand);
        UpdateActor(&ball);
        ball.prevVel = tempVect;
    }
    if (!gamePaused) {
        if (controlMethod == CONTROL_CAMERA) {
            cameraInfo.prevRotY = camera.rot.y;
        }
        UpdateActor(&camera);
        handControl.flags &= ~1;
        if (!cameraInfo.overrideTimer || !(cameraInfo.overrideFlags & 2)) {
            UpdateBullets();
        }
    }
    EndTimer(0);
    StartTimer(0, "Main");
    if (!gamePaused) {
        if (GameBeingPlayed()) {
            if (gameInfo.mode != 5) {
                ProcessInput();
                UpdateActorPositionDependencies();
                MoveBall();
                MoveHand();
                hand.flags &= ~0x1000;
                handControl.prevRot = hand.rot;
            }
        } else {
            UpdateActorPositionDependencies();
        }
    }
    if (!gamePaused) {
        MoveCam();
        PlayerSpinCamera();
        CalculateShadows();
        UpdateLights();
        if (!cameraInfo.overrideTimer || !(cameraInfo.overrideFlags & 2)) {
            UpdateVents();
        }
        DoEarthquake();
        UpdateBlurs();
    }
    DoBoss();
    if (cameoList.count && !gamePaused) {
        UpdateCameos();
    }
    EndTimer(0);
}

INCLUDE_ASM("asm/nonmatchings/main", pad_8013FBB8);

INCLUDE_RODATA("asm/nonmatchings/main", D_80105D58);
