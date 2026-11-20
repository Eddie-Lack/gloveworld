/*
 * Code that debugpad2.py writes over ScreenShot (which the game never calls), PCcreat and PCclose.
 * The section attributes say which of the three places each function goes to.
 * Build it with mkdebugio.sh; the script embeds the result.
 *
 * Output goes to the IS-Viewer: text is written at cartridge address 0x13FF0020 and its length at
 * 0x13FF0014. ares, mupen64plus and SummerCart64 show it in their logs. Recordings are sent as text
 * lines (MOOFILE, MOODATA, MOOEND) that moolog.py turns back into .moo files.
 */
typedef unsigned char u8;
typedef unsigned int u32;
typedef int s32;

extern char outputMessageBuffer[];
extern s32 autoPlayback;
void ReadController(s32 cntlr);
void EndTimer(s32 id);
u32 __osDisableInt(void);
void __osRestoreInt(u32 mask);

#define PI_STATUS (*(volatile u32 *)0xA4600010)
#define ISV(offset) (*(volatile u32 *)(0xB3FF0000 + (offset)))
#define ISV_SIZE 0xFFE0

s32 IsvPCcreat(char *name, s32 mode) __attribute__((section(".text.pccreat")));
s32 IsvPCclose(s32 fd) __attribute__((section(".text.pcclose")));
void IsvDebugPrintf(s32 len) __attribute__((section(".text.pcclose")));

extern char mooFile[];
extern char hexDigits[];

/* ControllerProc calls this in place of EndTimer(2). autoPlayback is 0 during the read so that
   controller 2 never goes into an input recording; interrupts stay off so that no other thread
   sees the change. */
void DebugReadController2(void) {
    u32 mask = __osDisableInt();
    s32 mode = autoPlayback;

    autoPlayback = 0;
    ReadController(1);
    autoPlayback = mode;
    __osRestoreInt(mask);
    EndTimer(2);
}

/* s must be word-aligned. Interrupts stay off so that no other thread starts a PI transfer between
   the check and the write. */
static void IsvSend(void *s, s32 len) {
    u32 *src = s;
    volatile u32 *dst = &ISV(0x20);
    u32 mask;

    if (len > ISV_SIZE) {
        len = ISV_SIZE;
    }
    mask = __osDisableInt();
    while (dst < &ISV(0x20 + len)) {
        while (PI_STATUS & 3) {
        }
        *dst++ = *src++;
    }
    while (PI_STATUS & 3) {
    }
    ISV(0x14) = len;
    __osRestoreInt(mask);
}

static s32 PutText(u8 *line, s32 n, char *s) {
    while (*s != '\0') {
        line[n++] = *s++;
    }
    return n;
}

/* Replaces PCcreat, in its place. The file name is sent; the one file handle is 1. */
s32 IsvPCcreat(char *name, s32 mode) {
    u32 line[32];
    s32 n;

    n = PutText((u8 *)line, 0, mooFile);
    n = PutText((u8 *)line, n, name);
    ((u8 *)line)[n++] = '\n';
    IsvSend(line, n);
    return 1;
}

/* Replaces PCwrite. The data is sent as hexadecimal, 32 bytes on each line. */
s32 IsvPCwrite(s32 fd, u8 *buf, s32 size) {
    u32 line[19];
    u8 *p;
    s32 i;

    for (i = 0; i < size;) {
        p = (u8 *)line + PutText((u8 *)line, 0, "MOODATA ");
        do {
            *p++ = hexDigits[buf[i] >> 4];
            *p++ = hexDigits[buf[i] & 15];
            i++;
        } while (i < size && (i & 31));
        *p++ = '\n';
        IsvSend(line, p - (u8 *)line);
    }
    return size;
}

/* Replaces PCclose, in its place. */
s32 IsvPCclose(s32 fd) {
    u32 line[2];

    IsvSend(line, PutText((u8 *)line, 0, "MOOEND\n"));
    return 0;
}

/* Replaces debugPrintf; it sits after IsvPCclose. PRINTF has just formatted len characters into
   outputMessageBuffer. */
void IsvDebugPrintf(s32 len) {
    IsvSend(outputMessageBuffer, len);
}

/* Defined last so that they go after IsvPCcreat, where there is room left. */
char mooFile[] __attribute__((section(".text.pccreat"))) = "MOOFILE ";
char hexDigits[16] __attribute__((section(".text.pccreat"))) = "0123456789ABCDEF";
