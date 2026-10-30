#ifndef CRC_H
#define CRC_H

#include "common.h"
#include "actor.h"

void InitCRCTable(void);
u32 UpdateCRCData(u8 *data, s32 len);

u32 UpdateCRC(u8 *ptr);

#endif
