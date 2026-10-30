#include "common.h"
#include "sound.h"
#include "ident.h"
#include "jalloc.h"
#include "dma.h"

extern OSMesgQueue dmaMessageQ;

typedef struct MusConfig {
    /* 0x00 */ u32 control_flag;
    /* 0x04 */ s32 channels;
    /* 0x08 */ void *sched;
    /* 0x0C */ s32 thread_priority;
    /* 0x10 */ u8 *heap;
    /* 0x14 */ s32 heap_length;
    /* 0x18 */ u8 *ptr;
    /* 0x1C */ u8 *wbk;
    /* 0x20 */ void *default_fxbank;
    /* 0x24 */ void *priority;
    /* 0x28 */ u8 unk28[0x8];
    /* 0x30 */ s32 fifo_length;
    /* 0x34 */ s32 syn_output_rate;
    /* 0x38 */ s32 syn_updates;
    /* 0x3C */ s32 syn_rsp_cmds;
    /* 0x40 */ s32 syn_retraceCount;
    /* 0x44 */ s32 syn_dma_buf_size;
} MusConfig;

extern MusConfig musicPlayer;
extern u8 musicPriority;
extern s32 musicMemoryUsed;
extern u8 pbankBuffer[];
extern u8 musicHeap[];
extern u8 _pbank1SegmentRomStart[];
extern u8 _pbank1SegmentRomEnd[];
extern u8 _pbank2SegmentRomStart[];
extern u8 _pbank2SegmentRomEnd[];
extern u8 _wbank1SegmentRomStart[];
extern u8 _wbank2SegmentRomStart[];
extern u8 sched[];
extern u8 libmus_sfx_list[];
extern u8 libmus_priority_list[];

s32 MusInitialize(MusConfig *config);
void MusBankInitialize(void *pbank, void *wbank);

void InitMusicDriver(void) {
    audioCtrl.base = pbankBuffer;
    DMAMemory(pbankBuffer, (u32)_pbank1SegmentRomStart, (u32)_pbank1SegmentRomEnd);
    audioCtrl.ptr = audioCtrl.base - (_pbank1SegmentRomStart - _pbank1SegmentRomEnd);
    DMAMemory(audioCtrl.ptr, (u32)_pbank2SegmentRomStart, (u32)_pbank2SegmentRomEnd);
    musicPlayer.control_flag = 0;
    musicPlayer.channels = 24;
    musicPlayer.sched = sched;
    musicPlayer.thread_priority = musicPriority;
    musicPlayer.heap = musicHeap;
    musicPlayer.heap_length = 0x493E0;
    musicPlayer.ptr = audioCtrl.ptr;
    musicPlayer.wbk = _wbank2SegmentRomStart;
    musicPlayer.default_fxbank = libmus_sfx_list;
    musicPlayer.priority = libmus_priority_list;
    musicPlayer.syn_output_rate = 22050;
    musicPlayer.fifo_length = 0x400;
    musicPlayer.syn_updates = 0x2000;
    musicPlayer.syn_rsp_cmds = 3;
    musicPlayer.syn_retraceCount = 40;
    musicPlayer.syn_dma_buf_size = 0x600;
    musicMemoryUsed = MusInitialize(&musicPlayer);
    MusBankInitialize(audioCtrl.base, _wbank1SegmentRomStart);
}

void DmaRomToRam(u32 rom, u8 *ram, s32 size) {
    OSIoMesg io;
    OSMesg msg;
    s32 len;

    while (size) {
        len = size;
        if (size > 0x4000) {
            len = 0x4000;
        }
        osInvalDCache(ram, len);
        osPiStartDma(&io, 0, 0, rom, ram, len, &dmaMessageQ);
        osRecvMesg(&dmaMessageQ, &msg, 1);
        rom += len;
        size -= len;
        ram += len;
    }
}

INCLUDE_ASM("asm/nonmatchings/dma", pad_80132934);

INCLUDE_RODATA("asm/nonmatchings/dma", pad_80101F94);
