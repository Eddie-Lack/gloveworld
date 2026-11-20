#ifndef NMEPROCESS2_H
#define NMEPROCESS2_H

#include "common.h"
#include "actor.h"
#include "footprint.h"

void NMEProcessSwish(Enemy *enemy);
void NMEProcessCrumpet(Enemy *enemy);
void NMEProcessCannonball(Enemy *enemy);
void NMEProcessSnail(Enemy *enemy);
void NMEProcessSamtex(Enemy *enemy);
void NMEProcessSucker(Enemy *enemy);
void NMEProcessCaterpillar(Enemy *enemy);
void NMEProcessDroid(Enemy *enemy);
void NMEProcessRobes(Enemy *enemy);
void NMEProcessRaptor(Enemy *enemy);
void NMEProcessChuck(Enemy *enemy);
void NMEProcessFrog(Enemy *enemy);
void NMEProcessBovva(Enemy *enemy);
void NMEProcessTracey(Enemy *enemy);
void NMEProcessCylon(Enemy *enemy);
void NMEProcessFumble(Enemy *enemy);
void NMEProcessSelwynWhale(Enemy *enemy);
void NMEProcessSelwynCrab(Enemy *enemy);
void NMEProcessSelwynFish(Enemy *enemy);
void UpdateEnemyBlastring(Footprint *footprint);
void CreateEnemyBlastring(Vec3 *pos, s32 time, f32 yAdd, f32 scaleAimSpeed);
void NMEProcessRobot(Enemy *enemy);
void NMEProcessSpank(Enemy *enemy);
void NMEProcessBabySpank(Enemy *enemy);
void StartBabiesAttacking(void);
s32 TriggerAtFrame(f32 frame, Actor *actor);
void NMEProcessReggie(Enemy *enemy);
void NMEProcessEvilGlove(Enemy *enemy);
void NMEProcessKloset(Enemy *enemy);
void AddEvilGloveFire(Vec3 *pos, Vec3 *vel, s16 type, f32 scale1, f32 scale2);
void NMEProcessBat(Enemy *enemy);
void NMEProcessEvilRobot(Enemy *enemy);
void AddEvilGloveSmoke(Actor *enemy, Vec3 *vel);
void NMEProcessHubChicken(Enemy *enemy);
s32 HubChickenType(void);

#endif
