#include "common.h"
#include "ident.h"
#include "nnsched.h"
#include "timer.h"
#include "actor.h"
#include "sound.h"
#include "game.h"
#include "overlay.h"

extern s32 osTvType;
extern u64 nnScStack[];
extern u64 nnScAudioStack[];
extern u64 nnScGraphicsStack[];
extern OSViFieldRegs originalRegs;
extern u8 originalVideoMode;
extern u8 preNmiCount;
extern u8 desiredFrameRate;
extern u8 newDesiredFrameRate;
extern NNScMsg gfxRetraceMsg;
extern s32 gfxTasks;
extern s16 schedRetraceCounts[2];
extern s16 timerRetraceCounts[2];
extern OSThread mainThread;
extern OSThread drawGraphicsThread;
extern OSThread ControllerThread;
extern Actor fadeOverlay;

void MusHandleStop(s32 handle, s32 speed);

void SetRetraceSpeed(NNSched *sc, u8 numFields) {
    osViSetEvent(&sc->retraceMQ, (OSMesg)VIDEO_MSG, numFields);
}

void nnScCreateScheduler(NNSched *sc, u8 videoMode, u8 numFields) {
    OSViFieldRegs regs;
    s16 vStart = 60;
    s16 vEnd = 600;
    f32 yScale = 0.88f;

    sc->curGraphicsTask = NULL;
    sc->curAudioTask = NULL;
    sc->graphicsTaskSuspended = NULL;
    sc->clientList = NULL;
    sc->firstTime = TRUE;
    sc->retraceMsg = NN_SC_RETRACE_MSG;
    sc->prenmiMsg = NN_SC_PRE_NMI_MSG;
    osCreateMesgQueue(&sc->retraceMQ, sc->retraceMsgBuf, NN_SC_MAX_MESGS);
    osCreateMesgQueue(&sc->rspMQ, sc->rspMsgBuf, NN_SC_MAX_MESGS);
    osCreateMesgQueue(&sc->rdpMQ, sc->rdpMsgBuf, NN_SC_MAX_MESGS);
    osCreateMesgQueue(&sc->graphicsRequestMQ, sc->graphicsRequestBuf, NN_SC_MAX_MESGS);
    osCreateMesgQueue(&sc->audioRequestMQ, sc->audioRequestBuf, NN_SC_MAX_MESGS);
    osCreateMesgQueue(&sc->waitMQ, sc->waitMsgBuf, NN_SC_MAX_MESGS);
    osCreateViManager(OS_PRIORITY_VIMGR);
    osViSetMode(&osViModeTable[videoMode]);
    osViBlack(TRUE);
    if (osTvType == 0) {
        memcpy(&regs, &osViModeTable[videoMode].fldRegs[0], sizeof(OSViFieldRegs));
        memcpy(&originalRegs, &osViModeTable[videoMode].fldRegs[0], sizeof(OSViFieldRegs));
        regs.vStart = (vStart << 16) + vEnd;
        memcpy(&osViModeTable[videoMode].fldRegs[0], &regs, sizeof(OSViFieldRegs));
        osViSetYScale(yScale);
        originalVideoMode = videoMode;
    }
    osViSetEvent(&sc->retraceMQ, (OSMesg)VIDEO_MSG, numFields);
    osSetEventMesg(OS_EVENT_SP, &sc->rspMQ, (OSMesg)RSP_DONE_MSG);
    osSetEventMesg(OS_EVENT_DP, &sc->rdpMQ, (OSMesg)RDP_DONE_MSG);
    osSetEventMesg(OS_EVENT_PRENMI, &sc->retraceMQ, (OSMesg)PRE_NMI_MSG);
    osCreateThread(&sc->schedulerThread, 19, (void (*)(void *))nnScEventHandler, sc, nnScStack + 0x400, 120);
    osStartThread(&sc->schedulerThread);
    osCreateThread(&sc->audioThread, 18, (void (*)(void *))nnScExecuteAudio, sc, nnScAudioStack + 0x400, 110);
    osStartThread(&sc->audioThread);
    osCreateThread(&sc->graphicsThread, 17, (void (*)(void *))nnScExecuteGraphics, sc, nnScGraphicsStack + 0x400, 100);
    osStartThread(&sc->graphicsThread);
}

OSMesgQueue *nnScGetAudioMQ(NNSched *sc) {
    return &sc->audioRequestMQ;
}

OSMesgQueue *nnScGetGfxMQ(NNSched *sc) {
    return &sc->graphicsRequestMQ;
}

void nnScEventHandler(NNSched *sc) {
    OSMesg msg = NULL;

    while (TRUE) {
        osRecvMesg(&sc->retraceMQ, &msg, OS_MESG_BLOCK);
        switch ((s32)msg) {
            case VIDEO_MSG:
                schedRetraceCounts[0]++;
                schedRetraceCounts[1]++;
                if (schedRetraceCounts[0] >= desiredFrameRate && gfxTasks == 0) {
                    desiredFrameRate = newDesiredFrameRate;
                    schedRetraceCounts[0] = 0;
                    nnScEventBroadcast(sc, &gfxRetraceMsg);
                } else if (gfxTasks != 0 && schedRetraceCounts[0] >= desiredFrameRate) {
                    schedRetraceCounts[0] = desiredFrameRate - 1;
                }
                if (schedRetraceCounts[1] >= 3) {
                    schedRetraceCounts[1] = 0;
                    nnScEventBroadcast(sc, &sc->retraceMsg);
                }
                if (preNmiCount != 0) {
                    preNmiCount--;
                    if (preNmiCount < 8) {
                        fadeOverlay.alpha = 255;
                        gameInfo.fade = 300;
                    }
                    if (preNmiCount == 0) {
                        osStopThread(&mainThread);
                        osStopThread(&drawGraphicsThread);
                        osStopThread(&ControllerThread);
                        osStopThread(&sched.audioThread);
                        if (osTvType == 0) {
                            memcpy(&osViModeTable[originalVideoMode].fldRegs[0], &originalRegs, sizeof(OSViFieldRegs));
                            osViSetYScale(1.0f);
                        }
                    }
                }
                break;
            case PRE_NMI_MSG:
                nnScEventBroadcast(sc, &sc->prenmiMsg);
                MusHandleStop(audioCtrl.handle, 20);
                preNmiCount = 20;
                FadeScreenOut();
                break;
        }
    }
}

void nnScAddClient(NNSched *sc, NNScClient *client, OSMesgQueue *msgQ) {
    OSIntMask mask;

    mask = osSetIntMask(OS_IM_NONE);
    client->msgQ = msgQ;
    client->next = sc->clientList;
    sc->clientList = client;
    osSetIntMask(mask);
}

void nnScRemoveClient(NNSched *sc, NNScClient *c) {
    NNScClient *client = sc->clientList;
    NNScClient *prev = NULL;
    OSIntMask mask;

    mask = osSetIntMask(OS_IM_NONE);
    while (client != NULL) {
        if (client == c) {
            if (prev) {
                prev->next = c->next;
            } else {
                sc->clientList = c->next;
            }
            break;
        }
        prev = client;
        client = client->next;
    }
    osSetIntMask(mask);
}

void nnScEventBroadcast(NNSched *sc, NNScMsg *msg) {
    NNScClient *client;

    for (client = sc->clientList; client != NULL; client = client->next) {
        osSendMesg(client->msgQ, (OSMesg)msg, OS_MESG_NOBLOCK);
    }
}

void nnScExecuteAudio(NNSched *sc) {
    OSMesg msg = NULL;
    NNScTask *task = NULL;
    NNScTask *gfxTask = NULL;
    u32 yieldFlag = 0;

    while (TRUE) {
        osRecvMesg(&sc->audioRequestMQ, (OSMesg *)&task, OS_MESG_BLOCK);
        osWritebackDCacheAll();
        yieldFlag = 0;
        gfxTask = sc->curGraphicsTask;
        if (gfxTask) {
            osSpTaskYield();
            osRecvMesg(&sc->rspMQ, &msg, OS_MESG_BLOCK);
            if (osSpTaskYielded(&gfxTask->list)) {
                yieldFlag = 1;
            } else {
                yieldFlag = 2;
            }
        }
        StartTimer(7, "MUSIC");
        sc->curAudioTask = task;
        osSpTaskStart(&task->list);
        osRecvMesg(&sc->rspMQ, &msg, OS_MESG_BLOCK);
        sc->curAudioTask = NULL;
        EndTimer(7);
        if (sc->graphicsTaskSuspended) {
            osSendMesg(&sc->waitMQ, &msg, OS_MESG_BLOCK);
        }
        if (yieldFlag == 1) {
            osSpTaskStart(&gfxTask->list);
        } else if (yieldFlag == 2) {
            osSendMesg(&sc->rspMQ, &msg, OS_MESG_BLOCK);
        }
        osSendMesg(task->msgQ, task->msg, OS_MESG_BLOCK);
    }
}

void nnScExecuteGraphics(NNSched *sc) {
    OSMesg msg = NULL;
    NNScTask *task;

    while (TRUE) {
        osRecvMesg(&sc->graphicsRequestMQ, (OSMesg *)&task, OS_MESG_BLOCK);
        nnScWaitTaskReady(sc, task);
        if (sc->curAudioTask) {
            sc->graphicsTaskSuspended = task;
            osRecvMesg(&sc->waitMQ, &msg, OS_MESG_BLOCK);
            sc->graphicsTaskSuspended = NULL;
        }
        sc->curGraphicsTask = task;
        osSpTaskStart(&task->list);
        StartTimer(8, "RSP");
        StartTimer(9, "RDP");
        osRecvMesg(&sc->rspMQ, &msg, OS_MESG_BLOCK);
        sc->curGraphicsTask = NULL;
        EndTimer(8);
        osRecvMesg(&sc->rdpMQ, &msg, OS_MESG_BLOCK);
        EndTimer(9);
        if (sc->firstTime) {
            osViBlack(FALSE);
            sc->firstTime = FALSE;
        }
        if (task->flags & NN_SC_SWAPBUFFER) {
            osViSwapBuffer(task->framebuffer);
        }
        osSendMesg(task->msgQ, task->msg, OS_MESG_BLOCK);
    }
}

void nnScWaitTaskReady(NNSched *sc, NNScTask *task) {
    OSMesg msg = NULL;
    NNScClient client;
    void *fb = task->framebuffer;

    while (osViGetCurrentFramebuffer() == fb || osViGetNextFramebuffer() == fb) {
        nnScAddClient(sc, &client, &sc->waitMQ);
        osRecvMesg(&sc->waitMQ, &msg, OS_MESG_BLOCK);
        nnScRemoveClient(sc, &client);
    }
}

void TimerProc(void *arg) {
    OSMesgQueue mq;
    OSMesg buf[NN_SC_MAX_MESGS];
    NNScClient client;
    OSTimer timer;
    OSMesg msg = NULL;
    NNSched *sc = arg;

    osCreateMesgQueue(&mq, buf, NN_SC_MAX_MESGS);
    nnScAddClient(&sched, &client, &mq);
    osSetTimer(&timer, 0, osClockRate * 19667 / 1000000, &mq, (OSMesg)10);
    while (TRUE) {
        osRecvMesg(&mq, &msg, OS_MESG_BLOCK);
        if ((s32)msg == 10) {
            timerRetraceCounts[0]++;
            timerRetraceCounts[1]++;
            if (timerRetraceCounts[0] >= desiredFrameRate && gfxTasks == 0) {
                desiredFrameRate = newDesiredFrameRate;
                timerRetraceCounts[0] = 0;
                nnScEventBroadcast(sc, &gfxRetraceMsg);
            } else if (gfxTasks != 0 && timerRetraceCounts[0] >= desiredFrameRate) {
                timerRetraceCounts[0] = desiredFrameRate - 1;
            }
            if (timerRetraceCounts[1] >= 3) {
                timerRetraceCounts[1] = 0;
                nnScEventBroadcast(sc, &sc->retraceMsg);
            }
        }
    }
}

INCLUDE_RODATA("asm/nonmatchings/sched", pad_8010899C);
