#ifndef EEPROM_H
#define EEPROM_H

#include "common.h"
#include "actor.h"

void GetEepromMessage(void);
void InitEeprom(void);
void InitEepromMessageQueue(void);
void LoadHiscoreTable(void);
void SaveHiscoreTable(void);
void LoadBestTimes(void);
void SaveBestTimes(void);
void LoadSlots(void);
void SaveSlots(void);
void SaveID(void);
void LoadCRC(void);
void SaveCRC(void);
void Wait(s16 time);
void EepromSaveHiscoreTable(void);
void EepromLoadHiscoreTable(void);
void EepromSaveBestTimes(void);
void EepromLoadBestTimes(void);
void EepromSaveSlots(void);
void EepromLoadSlots(void);
void EepromValid(void);
void EepromSaveID(void);
void EepromSaveCRC(void);
void EepromLoadCRC(void);
void CheckCRC(void);

#endif
