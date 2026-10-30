#include "common.h"
#include "ident.h"
#include "jalloc.h"

extern s32 frameCount;
extern s32 dispFrameCount;
extern OSMesgQueue dmaMessageQ;
extern OSIoMesg dmaIOMessageBuf;
extern u8 ActiveController;
extern OSContPad controllerdata[];

s32 DecompressBuffer(void *src, void *dst);
void MusSetMasterVolume(s32 flags, s32 volume);
s32 PlaySampleNot3D(s32 num, s32 volume, s32 pan, s32 pitch);

void memclear(void *ptr, s32 size) {
    u8 *p = ptr;
    s32 i;

    for (i = 0; i < size; i++) {
        p[i] = 0;
    }
}

void JallocInit(u32 start, u32 size) {
    JallocControl *jc = &jallocControl;

    if (size % 16) {
        size -= size % 16;
    }
    jc->heapStart = start;
    jc->heapSize = size;
    JallocReset();
    PRINTF("Jalloc initialised\n");
}

void *JallocAlloc(s32 size, s32 clear, char *name) {
    void *ptr;

    if (size == 0) {
        PRINTF("ZERO MALLOC!!\n");
        return NULL;
    }
    if (size % 16) {
        size += 16 - size % 16;
    }
    if (frameCount != 0 && dispFrameCount >= 2) {
        ptr = JallocAllocDynamic(size, clear, name);
    } else {
        ptr = JallocAllocStatic(size, clear, name);
    }
    return ptr;
}

void *JallocAllocStatic(s32 size, s32 clear, char *name) {
    u32 j;
    u32 oldSize;
    s32 i;
    JallocControl *jc;
    u32 ptr;

    jc = &jallocControl;
    if (jc->lastStatic == -1) {
        ptr = jc->heapStart + jc->heapSize - size;
        jc->blocks[0].used = TRUE;
        jc->blocks[0].size = size;
        jc->blocks[0].addr = ptr;
        jc->lastStatic = 0;
        i = 0;
        goto done;
    }
    for (i = 0; i < jc->lastStatic + 1; i++) {
        if (!jc->blocks[i].used && jc->blocks[i].size >= size) {
            break;
        }
    }
    if (i >= jc->firstDynamic) {
        goto oom;
    }
    if (i == jc->lastStatic + 1) {
        if (jc->heapSize - jc->staticUsed - jc->dynamicUsed < size) {
            goto oom;
        }
        ptr = jc->blocks[jc->lastStatic].addr - size;
        jc->lastStatic++;
        jc->blocks[jc->lastStatic].used = TRUE;
        jc->blocks[jc->lastStatic].size = size;
        jc->blocks[jc->lastStatic].addr = ptr;
        i = jc->lastStatic;
        goto done;
    }
    oldSize = jc->blocks[i].size;
    jc->blocks[i].size = size;
    jc->blocks[i].used = TRUE;
    if (size < oldSize && jc->lastStatic < jc->firstDynamic - 1) {
        for (j = jc->lastStatic; j > i; j--) {
            jc->blocks[j + 1] = jc->blocks[j];
        }
        j = i + 1;
        jc->lastStatic++;
        jc->blocks[j].used = FALSE;
        jc->blocks[j].size = oldSize - size;
        jc->blocks[j].addr = jc->blocks[i].addr;
        jc->blocks[i].addr = jc->blocks[j].addr + jc->blocks[j].size;
    } else if (size < oldSize) {
        jc->blocks[i].size = oldSize;
    }
    ptr = jc->blocks[i].addr;
    goto done;

oom:
    if (size == 0) {
        PRINTF("Oops! trying to malloc 0 bytes\n");
    } else {
        PRINTF("Cannot malloc %d - out of memory\n", size);
    }
    PRINTF("Jalloc size: %d\nStatic: %d\nDynamic: %d\n", jc->heapSize, jc->staticUsed, jc->dynamicUsed);
    Crash("JALLOC");
    ShowJalloc();
    while (TRUE) {
    }
    return NULL;

done:
    jc->staticUsed += jc->blocks[i].size;
    memcpy(jc->blocks[i].name, name, strlen(name));
    if (strlen(name) < 10) {
        jc->blocks[i].name[strlen(name)] = '\0';
    }
    if (clear == TRUE) {
        memclear((void *)ptr, size);
    }
    return (void *)ptr;
}

void *JallocAllocDynamic(s32 size, s32 clear, char *name) {
    u32 j;
    u32 oldSize;
    s32 i;
    JallocControl *jc;
    u32 ptr;

    jc = &jallocControl;
    if (jc->firstDynamic == JALLOC_NUM_BLOCKS) {
        ptr = jc->heapStart;
        i = JALLOC_NUM_BLOCKS - 1;
        jc->blocks[i].used = TRUE;
        jc->blocks[i].size = size;
        jc->blocks[i].addr = ptr;
        jc->firstDynamic = i;
        goto done;
    }
    for (i = JALLOC_NUM_BLOCKS - 1; i > jc->firstDynamic - 1; i--) {
        if (!jc->blocks[i].used && jc->blocks[i].size >= size) {
            break;
        }
    }
    if (i <= jc->lastStatic) {
        goto oom;
    }
    if (i == jc->firstDynamic - 1) {
        if (jc->heapSize - jc->staticUsed - jc->dynamicUsed < size) {
            goto oom;
        }
        ptr = jc->blocks[jc->firstDynamic].addr + jc->blocks[jc->firstDynamic].size;
        jc->firstDynamic--;
        jc->blocks[jc->firstDynamic].used = TRUE;
        jc->blocks[jc->firstDynamic].size = size;
        jc->blocks[jc->firstDynamic].addr = ptr;
        i = jc->firstDynamic;
        goto done;
    }
    oldSize = jc->blocks[i].size;
    jc->blocks[i].size = size;
    jc->blocks[i].used = TRUE;
    if (size < oldSize && jc->lastStatic < jc->firstDynamic - 1) {
        for (j = jc->firstDynamic; j < i; j++) {
            jc->blocks[j - 1] = jc->blocks[j];
        }
        j = i - 1;
        jc->firstDynamic--;
        jc->blocks[j].used = FALSE;
        jc->blocks[j].size = oldSize - size;
        jc->blocks[j].addr = jc->blocks[i].addr + size;
    } else if (size < oldSize) {
        jc->blocks[i].size = oldSize;
    }
    ptr = jc->blocks[i].addr;
    goto done;

oom:
    if (size == 0) {
        PRINTF("Oops! trying to malloc 0 bytes\n");
    } else {
        PRINTF("Cannot malloc %d - out of memory\n", size);
    }
    Crash("JALLOC");
    ShowJalloc();
    while (TRUE) {
    }
    return NULL;

done:
    jc->dynamicUsed += jc->blocks[i].size;
    memcpy(jc->blocks[i].name, name, strlen(name));
    if (strlen(name) < 10) {
        jc->blocks[i].name[strlen(name)] = '\0';
    }
    if (clear == TRUE) {
        memclear((void *)ptr, size);
    }
    return (void *)ptr;
}

s32 JallocFreeStatic(void *ptrRef) {
    s32 i;
    s32 j;
    JallocControl *jc;

    jc = &jallocControl;
    if (jc->lastStatic == -1) {
        return FALSE;
    }
    for (i = 0; i <= jc->lastStatic; i++) {
        if (jc->blocks[i].addr == *(u32 *)ptrRef) {
            break;
        }
    }
    if (i > jc->lastStatic) {
        return FALSE;
    }
    *(u32 *)ptrRef = 0;
    if (!jc->blocks[i].used) {
        return TRUE;
    }
    if (i == jc->lastStatic) {
        jc->staticUsed -= jc->blocks[jc->lastStatic].size;
        jc->lastStatic--;
        if (jc->lastStatic >= 0 && !jc->blocks[jc->lastStatic].used) {
            jc->lastStatic--;
        }
        return TRUE;
    }
    jc->blocks[i].used = FALSE;
    jc->staticUsed -= jc->blocks[i].size;
    if (!jc->blocks[i + 1].used) {
        jc->blocks[i].size += jc->blocks[i + 1].size;
        jc->blocks[i].addr = jc->blocks[i + 1].addr;
        for (j = i + 1; j < jc->lastStatic; j++) {
            jc->blocks[j] = jc->blocks[j + 1];
        }
        jc->blocks[jc->lastStatic--].used = FALSE;
    }
    if (i > 0 && !jc->blocks[i - 1].used) {
        jc->blocks[i - 1].size += jc->blocks[i].size;
        jc->blocks[i - 1].addr = jc->blocks[i].addr;
        for (j = i; j < jc->lastStatic; j++) {
            jc->blocks[j] = jc->blocks[j + 1];
        }
        jc->blocks[jc->lastStatic--].used = FALSE;
    }
    return TRUE;
}

s32 JallocFreeDynamic(void *ptrRef) {
    s32 i;
    s32 j;
    JallocControl *jc;

    jc = &jallocControl;
    if (jc->firstDynamic == JALLOC_NUM_BLOCKS) {
        return FALSE;
    }
    for (i = JALLOC_NUM_BLOCKS - 1; i >= jc->firstDynamic; i--) {
        if (jc->blocks[i].addr == *(u32 *)ptrRef) {
            break;
        }
    }
    if (i < jc->firstDynamic) {
        return FALSE;
    }
    *(u32 *)ptrRef = 0;
    if (!jc->blocks[i].used) {
        return TRUE;
    }
    if (i == jc->firstDynamic) {
        jc->dynamicUsed -= jc->blocks[jc->firstDynamic].size;
        jc->firstDynamic++;
        if (jc->firstDynamic < JALLOC_NUM_BLOCKS && !jc->blocks[jc->firstDynamic].used) {
            jc->firstDynamic++;
        }
        return TRUE;
    }
    jc->blocks[i].used = FALSE;
    jc->dynamicUsed -= jc->blocks[i].size;
    if (!jc->blocks[i - 1].used) {
        jc->blocks[i].size += jc->blocks[i - 1].size;
        for (j = i - 1; j > jc->firstDynamic; j--) {
            jc->blocks[j] = jc->blocks[j - 1];
        }
        jc->blocks[jc->firstDynamic++].used = FALSE;
    }
    if (i < JALLOC_NUM_BLOCKS - 1 && !jc->blocks[i + 1].used) {
        jc->blocks[i + 1].size += jc->blocks[i].size;
        for (j = i; j > jc->firstDynamic; j--) {
            jc->blocks[j] = jc->blocks[j - 1];
        }
        jc->blocks[jc->firstDynamic++].used = FALSE;
    }
    return TRUE;
}

void JallocFree(void *ptrRef) {
    if (*(void **)ptrRef == NULL) {
        return;
    }
    if (JallocFreeDynamic(ptrRef)) {
        return;
    }
    JallocFreeStatic(ptrRef);
}

void JallocReset(void) {
    u32 i;
    JallocControl *jc = &jallocControl;

    for (i = 0; i < JALLOC_NUM_BLOCKS; i++) {
        jc->blocks[i].used = FALSE;
        jc->blocks[i].addr = 0;
        jc->blocks[i].size = 0;
    }
    jc->lastStatic = -1;
    jc->firstDynamic = JALLOC_NUM_BLOCKS;
    jc->dynamicUsed = 0;
    jc->staticUsed = 0;
}

void ShowJalloc(void) {
    u32 used = 0;
    u32 empty = 0;
    u32 i;
    JallocControl *jc = &jallocControl;

    PRINTF("\nJalloc block list...\n");
    PRINTF("-------------------------\n");
    PRINTF("%d blocks:\n", jc->lastStatic + 1);
    if (jc->lastStatic == -1) {
        return;
    }
    for (i = 0; i <= jc->lastStatic; i++) {
        if (jc->blocks[i].used) {
            used++;
        } else {
            empty++;
        }
        PRINTF("    block #%d (%s): at (&%x), size %d status %d\n", i, jc->blocks[i].name,
               jc->blocks[i].addr, jc->blocks[i].size, jc->blocks[i].used);
    }
    for (i = JALLOC_NUM_BLOCKS - 1; i >= jc->firstDynamic; i--) {
        if (jc->blocks[i].used) {
            used++;
        } else {
            empty++;
        }
        PRINTF("    block #%d (%s): at (&%x), size %d status %d\n", i, jc->blocks[i].name,
               jc->blocks[i].addr, jc->blocks[i].size, jc->blocks[i].used);
    }
    PRINTF("\n");
    PRINTF("%d blocks used; %d blocks empty\n", used, empty);
}

s32 DMAGetSize(u32 start, u32 end) {
    s32 size;
    s32 result;
    u32 *p;
    u32 word;
    u32 header[2];

    size = end - start;
    osInvalDCache(header, 8);
    osWritebackDCache(header, 8);
    osPiStartDma(&dmaIOMessageBuf, 0, 0, start, header, 8, &dmaMessageQ);
    osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
    if (header[0] == 0x464C4132) {
        PRINTF("Compressed file found");
        p++;
        word = header[1];
        result = (word >> 24) & 0xFF;
        result += ((word >> 16) & 0xFF) << 8;
        result += ((word >> 8) & 0xFF) << 16;
        result += (word & 0xFF) << 24;
    } else {
        result = size;
    }
    PRINTF("DMAGetSize:%d\n", result);
    return result;
}

void DMAMemory(void *dest, u32 start, u32 end) {
    s32 size;
    s32 result;
    u32 *p;
    void *buf;
    u32 word;
    u32 header[2];

    size = end - start;
    osInvalDCache(header, 8);
    osWritebackDCache(header, 8);
    osPiStartDma(&dmaIOMessageBuf, 0, 0, start, header, 8, &dmaMessageQ);
    osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
    if (header[0] == 0x464C4132) {
        PRINTF("Compressed file found");
        p++;
        word = header[1];
        result = (word >> 24) & 0xFF;
        result += ((word >> 16) & 0xFF) << 8;
        result += ((word >> 8) & 0xFF) << 16;
        result += (word & 0xFF) << 24;
        buf = JallocAlloc(size, 1, "tempdecomp");
        osInvalDCache(buf, size);
        osWritebackDCache(buf, size);
        osPiStartDma(&dmaIOMessageBuf, 0, 0, start, buf, size, &dmaMessageQ);
        osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
        osInvalDCache(dest, result);
        osWritebackDCache(dest, result);
        DecompressBuffer((u8 *)buf + 8, dest);
        JallocFree(&buf);
        PRINTF("DMAMemory:%d\n", result);
    } else {
        osInvalDCache(dest, size);
        osWritebackDCache(dest, size);
        osPiStartDma(&dmaIOMessageBuf, 0, 0, start, dest, size, &dmaMessageQ);
        osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
        PRINTF("DMAMemory:%d\n", size);
    }
}

void CheckJalloc(void) {
    JallocControl *jc;
    u32 i;
    s32 total;

    jc = &jallocControl;
    total = 0;
    for (i = 0; i <= jc->lastStatic; i++) {
        if (jc->blocks[i].used) {
            total += jc->blocks[i].size;
        }
        if (i != 0 && jc->blocks[i].addr + jc->blocks[i].size != jc->blocks[i - 1].addr) {
            PRINTF("Uh-oh - memory crobbed on frame %d\n", frameCount);
            MusSetMasterVolume(1, 0x55F0);
            PlaySampleNot3D(248, 100, 128, 128);
            while (!(controllerdata[ActiveController].button & A_BUTTON)) {}
        }
    }
    if (total != jc->staticUsed) {
        PRINTF("Uh-oh - memory clodged on frame %d\n", frameCount);
        MusSetMasterVolume(1, 0x55F0);
        PlaySampleNot3D(248, 100, 128, 128);
        while (!(controllerdata[ActiveController].button & A_BUTTON)) {}
    }
    total = 0;
    for (i = JALLOC_NUM_BLOCKS - 1; i >= jc->firstDynamic; i--) {
        if (jc->blocks[i].used) {
            total += jc->blocks[i].size;
        }
        if (i < JALLOC_NUM_BLOCKS - 1 && jc->blocks[i + 1].addr + jc->blocks[i + 1].size != jc->blocks[i].addr) {
            PRINTF("Uh-oh - memory crobbed on frame %d\n", frameCount);
            MusSetMasterVolume(1, 0x55F0);
            PlaySampleNot3D(248, 100, 128, 128);
            while (!(controllerdata[ActiveController].button & A_BUTTON)) {}
        }
    }
    if (total != jc->dynamicUsed) {
        PRINTF("Uh-oh - memory clodged on frame %d\n", frameCount);
        MusSetMasterVolume(1, 0x55F0);
        PlaySampleNot3D(248, 100, 128, 128);
        while (!(controllerdata[ActiveController].button & A_BUTTON)) {}
    }
}

INCLUDE_RODATA("asm/nonmatchings/jalloc", pad_80105F44);
