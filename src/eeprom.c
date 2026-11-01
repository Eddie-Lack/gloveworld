#include "common.h"
#include "ident.h"
#include "save.h"
#include "hand.h"
#include "eeprom.h"
#include "crc.h"

extern u8 eepromMessageNum;
extern u8 eepromMessageQueue[6];
extern s16 eepromPresent;
extern OSMesgQueue controllerMsgQ;
extern u8 hiscoreTable[0x30];
extern BestTime bestTimes[6][5];
extern BestTime bestTimesCopy[6][5];
extern u8 hiscoreTableCopy[0x30];
extern SaveSlot blankSlot;
extern s16 currentSaveSlot;
extern u8 currentName[3];
extern u32 eepromCRCS[8];
extern char *cartID;
extern u8 validEeprom;

void PostEepromMessage(s16 msg) {
    eepromMessageQueue[eepromMessageNum++] = msg;
}

void GetEepromMessage(void) {
    s16 i;

    for (i = 0; i < 5; i++) {
        eepromMessageQueue[i] = eepromMessageQueue[i + 1];
    }
    eepromMessageQueue[i] = 0;
    eepromMessageNum--;
}

void InitEeprom(void) {
    eepromPresent = osEepromProbe(&controllerMsgQ);
    PostEepromMessage(8);
    while (eepromMessageQueue[0] != 0) {}
}

void InitEepromMessageQueue(void) {
    u8 i;

    eepromMessageNum = 0;
    for (i = 0; i < 6; i++) {
        eepromMessageQueue[i] = 0;
    }
}

void LoadHiscoreTable(void) {
    PostEepromMessage(2);
    while (eepromMessageQueue[0] != 0) {}
}

void SaveHiscoreTable(void) {
    PostEepromMessage(1);
    while (eepromMessageQueue[0] != 0) {}
}

void LoadBestTimes(void) {
    PostEepromMessage(5);
    while (eepromMessageQueue[0] != 0) {}
}

void SaveBestTimes(void) {
    PostEepromMessage(4);
    while (eepromMessageQueue[0] != 0) {}
}

void LoadSlots(void) {
    PostEepromMessage(7);
    while (eepromMessageQueue[0] != 0) {}
}

void SaveSlots(void) {
    PostEepromMessage(6);
    while (eepromMessageQueue[0] != 0) {}
}

void SaveID(void) {
    PostEepromMessage(9);
    while (eepromMessageQueue[0] != 0) {}
}

void LoadCRC(void) {
    PostEepromMessage(10);
    while (eepromMessageQueue[0] != 0) {}
}

void SaveCRC(void) {
    PostEepromMessage(11);
    while (eepromMessageQueue[0] != 0) {}
}

void Wait(s16 time) {
    u64 now;
    u64 start;

    start = now = osGetTime() * 1000000 / osClockRate;
    while (now - start < 20000) {
        now = osGetTime() * 1000000 / osClockRate;
    }
}

void EepromSaveHiscoreTable(void) {
    s16 ret;

    if (eepromPresent) {
        ret = osEepromLongWrite(&controllerMsgQ, 1, hiscoreTable, 0x30);
    }
    Wait(10000);
}

void EepromLoadHiscoreTable(void) {
    s16 ret;

    if (eepromPresent) {
        ret = osEepromLongRead(&controllerMsgQ, 1, hiscoreTable, 0x30);
    }
    Wait(10000);
}

void EepromSaveBestTimes(void) {
    s16 ret;

    if (eepromPresent) {
        ret = osEepromLongWrite(&controllerMsgQ, 0x7, (u8 *)bestTimes, 0xc0);
        Wait(10000);
    }
}

void EepromLoadBestTimes(void) {
    s16 ret;

    if (eepromPresent) {
        ret = osEepromLongRead(&controllerMsgQ, 0x7, (u8 *)bestTimes, 0xc0);
        Wait(10000);
    }
}

void EepromSaveSlots(void) {
    s16 ret;

    if (eepromPresent) {
        ret = osEepromLongWrite(&controllerMsgQ, 0x1f, (u8 *)saveSlot, 0xa8);
        Wait(10000);
    }
}

void EepromLoadSlots(void) {
    s16 ret;

    if (eepromPresent) {
        ret = osEepromLongRead(&controllerMsgQ, 0x1f, (u8 *)saveSlot, 0xa8);
        Wait(10000);
    }
}

void EepromValid(void) {
    u8 buf[8];
    s16 ret;

    if (eepromPresent) {
        ret = osEepromLongRead(&controllerMsgQ, 0, buf, 8);
        Wait(10000);
        if (buf[0] == cartID[0] && buf[1] == cartID[1] && buf[2] == cartID[2] && buf[3] == cartID[3]) {
            validEeprom = TRUE;
        } else {
            validEeprom = FALSE;
        }
    }
}

void EepromSaveID(void) {
    s16 ret;
    char buf[8];

    sprintf(buf, cartID);
    if (eepromPresent) {
        ret = osEepromLongWrite(&controllerMsgQ, 0, buf, 8);
        Wait(10000);
    }
}

void EepromSaveCRC(void) {
    s16 ret;
    s32 i;

    eepromCRCS[6] = UpdateCRCData(hiscoreTable, 0x30);
    eepromCRCS[7] = UpdateCRCData((u8 *)bestTimes, 0xC0);
    for (i = 0; i < 6; i++) {
        eepromCRCS[i] = UpdateCRCData((u8 *)&saveSlot[i], sizeof(SaveSlot));
    }
    if (eepromPresent) {
        ret = osEepromLongWrite(&controllerMsgQ, 0x34, (u8 *)eepromCRCS, 0x20);
        Wait(10000);
    }
}

void EepromLoadCRC(void) {
    s16 ret;

    if (eepromPresent) {
        ret = osEepromLongRead(&controllerMsgQ, 0x34, (u8 *)eepromCRCS, 0x20);
        Wait(10000);
    }
}

void LoadGame(s16 slot) {
}

void SaveGame(s16 slot) {
}

void UpdateSaveSlot(u8 allGaribs) {
    saveSlot[currentSaveSlot].name[0] = currentName[0];
    saveSlot[currentSaveSlot].name[1] = currentName[1];
    saveSlot[currentSaveSlot].name[2] = currentName[2];
    saveSlot[currentSaveSlot].score = player.score + player.garibs * 10;
    saveSlot[currentSaveSlot].lives = player.lives;
    SaveSlots();
    SaveCRC();
}

void CheckCRC(void) {
    s32 i;
    u32 crc;

    for (i = 0; i < 6; i++) {
        crc = UpdateCRCData((u8 *)&saveSlot[i], sizeof(SaveSlot));
        if (crc != eepromCRCS[i]) {
            memcpy(&saveSlot[i], &blankSlot, sizeof(SaveSlot));
        }
    }
    crc = UpdateCRCData(hiscoreTable, 0x30);
    if (crc != eepromCRCS[6]) {
        memcpy(hiscoreTable, hiscoreTableCopy, 0x30);
    }
    crc = UpdateCRCData((u8 *)bestTimes, 0xC0);
    if (crc != eepromCRCS[7]) {
        memcpy(bestTimes[0], bestTimesCopy[0], sizeof(bestTimes[0]));
        memcpy(bestTimes[1], bestTimesCopy[1], sizeof(bestTimes[1]));
        memcpy(bestTimes[2], bestTimesCopy[2], sizeof(bestTimes[2]));
        memcpy(bestTimes[3], bestTimesCopy[3], sizeof(bestTimes[3]));
        memcpy(bestTimes[4], bestTimesCopy[4], sizeof(bestTimes[4]));
        memcpy(bestTimes[5], bestTimesCopy[5], sizeof(bestTimes[5]));
    }
}

INCLUDE_RODATA("asm/nonmatchings/eeprom", pad_80101FC4);
