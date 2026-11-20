#ifndef SHADOW_H
#define SHADOW_H

#include "common.h"
#include "actor.h"

void InitRestartGateShadows(void);
void UpdateRestartGateShadows(Footprint *ptr);
void SetAIRenderMode(u8 alpha);
void ProcessShadows(void);
void CalculateShadows(void);
void DrawHelper(void);

#endif
