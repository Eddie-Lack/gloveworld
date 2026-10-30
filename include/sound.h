#ifndef SOUND_H
#define SOUND_H

#include "common.h"

typedef struct ContinuousSample {
    /* 0x00 */ s32 handle;
    /* 0x04 */ s32 volume;
    /* 0x08 */ s32 outVolume;
    /* 0x0C */ s32 baseVolume;
    /* 0x10 */ s16 pitch;
    /* 0x12 */ s16 id;
    /* 0x14 */ Vec3 *pos;
    /* 0x18 */ f32 range;
} ContinuousSample; /* size = 0x1C */

typedef struct AudioBanks {
    /* 0x00 */ u8 *ptr;
    /* 0x04 */ u8 *unk04;
    /* 0x08 */ u8 *base;
    /* 0x0C */ u8 unk0C[0x4];
    /* 0x10 */ u8 *song;
    /* 0x14 */ s32 handle;
    /* 0x18 */ s32 current;
    /* 0x1C */ u8 unk1C[0x4];
} AudioBanks; /* size = 0x20 */

extern AudioBanks audioCtrl;

s32 PlaySample(s16 num, Vec3 *pos, s16 vol, s16 pitch);
s32 PlaySampleNot3D(s32 num, u8 volume, u8 pan, u8 pitch);
void PlayContinuousSample(ContinuousSample *sfx, s16 num, s16 volume, Vec3 *pos, s16 radius);
void StopContinuousSample(ContinuousSample *sfx);
void UpdateContinuousSample(ContinuousSample *sfx);

void InitAmbientSoundList(void);
s32 PlaySampleRadius(s16 num, Vec3 *pos, s16 vol, s16 pitch, f32 radius);
void AddAmbientSfx(s32 num, s32 vol, s32 pan);
void AddAmbientSfxAtPoint(s32 num, s32 volume, Vec3 *pos, s16 radius, s16 freq, s16 randFreq, s16 onTime, s16 platTag, s16 tag,
                          f32 range);
void ClearAmbientSfx(void);
void UpdateAmbientSounds(void);
void InitSFXScript(void *script);
void ProcessSFXScript(void);

#endif
