#ifndef DEBUG_H
#define DEBUG_H

#include "types.h"

extern char outputMessageBuffer[];

int sprintf(char *buf, const char *fmt, ...);
void debugPrintf(s32 len);
void Crash(char *msg);

#define PRINTF(fmt, args...) debugPrintf(sprintf(outputMessageBuffer, fmt, ##args))

#endif
