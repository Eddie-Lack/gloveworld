#ifndef NMEPROCESS_H
#define NMEPROCESS_H

#include "common.h"
#include "actor.h"

void NMEAddNormalInstruction(u8 **data);
void NMEAddConditionalInstruction(u8 **data);
void NMEAddAttackInstruction(u8 **data);
void NMEProcessInstruction(Enemy *enemy);
void NMEEndAttack(Enemy *enemy);
s32 NMEConditionsMet(Enemy *enemy, NMEInstruction *instruction);
void NMEProcessActorScript(Enemy *enemy);
void NMEProcessMallet(Enemy *enemy);
void NMEProcessYoofow(Enemy *enemy);
void NMEProcessMike(Enemy *enemy);
void NMEProcessChester(Enemy *enemy);
void NMEProcessKnight(Enemy *enemy);
void NMEProcessKeg(Enemy *enemy);
void NMEProcessDenis(Enemy *enemy);
void NMEProcessBugle(Enemy *enemy);
void NMEProcessGeneralWu(Enemy *enemy);
void NMEProcessLionFish(Enemy *enemy);
void NMEProcessFrogged(Enemy *enemy);
void NMEProcessBigRaptor(Enemy *enemy);

#endif
