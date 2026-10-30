#include "common.h"
#include "ident.h"
#include "ambient.h"
#include "sound.h"
#include "camera.h"
#include "maths.h"
#include "game.h"
#include "jalloc.h"
#include "platform.h"

extern s32 MAX_SFX_DIST;
extern s32 reverbOn;
void MusHandleSetReverb(s32 handle, s32 reverb);
extern s16 currentScriptEntry;
typedef struct SFXScriptEntry {
    /* 0x00 */ s16 sec;
    /* 0x02 */ s16 frame;
    /* 0x04 */ s16 endSec;
    /* 0x06 */ s16 endFrame;
    /* 0x08 */ s16 id;
    /* 0x0A */ u8 volume;
    /* 0x0B */ u8 pitch;
    /* 0x0C */ s32 handle;
} SFXScriptEntry; /* size = 0x10 */

extern SFXScriptEntry *activeSFXScript;
extern SFXScriptEntry introSfxScript[];
extern s32 frameCount;
extern s32 numContinuousSamples;
extern Vec3 zero;
extern u8 gamePaused;
extern s32 osTvType;
extern u8 _pbank1SegmentRomEnd[];
extern u8 _pbank2SegmentRomEnd[];
extern u8 atlantisTrackRomStart[];
extern u8 atlantis2TrackRomStart[];
extern u8 atlantis3TrackRomStart[];
extern u8 atlantisBossTrackRomStart[];
extern u8 carnivalTrackRomStart[];
extern u8 carnival2TrackRomStart[];
extern u8 carnival3TrackRomStart[];
extern u8 piratesTrackRomStart[];
extern u8 pirates2TrackRomStart[];
extern u8 piratesBossTrackRomStart[];
extern u8 prehistoricTrackRomStart[];
extern u8 prehistoric2TrackRomStart[];
extern u8 prehistoric3TrackRomStart[];
extern u8 prehistoricBossTrackRomStart[];
extern u8 fortressTrackRomStart[];
extern u8 fortress2TrackRomStart[];
extern u8 fortress3TrackRomStart[];
extern u8 fortressBossTrackRomStart[];
extern u8 ootwTrackRomStart[];
extern u8 ootw2TrackRomStart[];
extern u8 ootw3TrackRomStart[];
extern u8 ootwBossTrackRomStart[];
extern u8 introTrackRomStart[];
extern u8 outroTrackRomStart[];
extern u8 pirates3TrackRomStart[];
extern u8 carnivalBossTrackRomStart[];
extern u8 hub1TrackRomStart[];
extern u8 hub2TrackRomStart[];
extern u8 hub3TrackRomStart[];
extern u8 hub4TrackRomStart[];
extern u8 hub5TrackRomStart[];
extern u8 hub6TrackRomStart[];
extern u8 titleTrackRomStart[];
extern u8 track46RomStart[];
extern u8 track47RomStart[];
extern u8 track48RomStart[];
extern u8 track49RomStart[];
extern u8 track54RomStart[];
extern u8 track55RomStart[];
extern u8 track56RomStart[];
extern u8 ootwWin2TrackRomStart[];
extern u8 track50RomStart[];
extern u8 track51RomStart[];
extern u8 track52RomStart[];
extern u8 track53RomStart[];
extern u8 track42RomStart[];
extern u8 track43RomStart[];
extern u8 track44RomStart[];
extern u8 track45RomStart[];
extern u8 atlantisKill1TrackRomStart[];
extern u8 track35RomStart[];
extern u8 atlantisWin1TrackRomStart[];
extern u8 track37RomStart[];
extern u8 track38RomStart[];
extern u8 track39RomStart[];
extern u8 track40RomStart[];
extern u8 track41RomStart[];
extern u8 bonusTrackRomStart[];
extern u8 hasbroTrackRomStart[];
extern u8 title2TrackRomStart[];
extern u8 recordKeyFilesRomStart[];
s32 MusHandleAsk(s32 handle);
s32 MusBankStartSong(void *bank, void *song);
extern s16 sfxVol;

s32 MusStartEffect2(s32 number, s32 volume, s32 pan, s32 restartflag, s32 priority);
void MusHandleStop(s32 handle, s32 speed);
void MusHandleSetVolume(s32 handle, s32 volume);
void MusHandleSetPan(s32 handle, s32 pan);
void MusHandleSetFreqOffset(s32 handle, f32 offset);
void MusHandleSetTempo(s32 handle, s32 tempo);
struct Platform *TagToFirstPlatform(s32 tag);

inline void SubAmbientSound(AmbientSound *ambientSound) {
    ambientSound->prev->next = ambientSound->next;
    ambientSound->next->prev = ambientSound->prev;
    ambientSoundList.count--;
}

void InitAmbientSoundList(void) {
    ambientSoundList.head.next = ambientSoundList.head.prev = &ambientSoundList.head;
    ambientSoundList.count = 0;
}

void PrepareSong(u8 song) {
    u8 *start;
    u8 *end;

    MusHandleSetReverb(audioCtrl.handle, 0);
    MusHandleSetVolume(audioCtrl.handle, 80);
    if (song == audioCtrl.current) {
        return;
    }
    audioCtrl.current = song;
    MusHandleStop(audioCtrl.handle, 1);
    while (MusHandleAsk(audioCtrl.handle)) {}
    if (song == 0) {
        return;
    }
    switch (song) {
        case 28:
            start = hub1TrackRomStart;
            end = hub2TrackRomStart;
            break;
        case 29:
            start = hub2TrackRomStart;
            end = hub3TrackRomStart;
            break;
        case 30:
            start = hub3TrackRomStart;
            end = hub4TrackRomStart;
            break;
        case 31:
            start = hub4TrackRomStart;
            end = hub5TrackRomStart;
            break;
        case 32:
            start = hub5TrackRomStart;
            end = hub6TrackRomStart;
            break;
        case 33:
            start = hub6TrackRomStart;
            end = titleTrackRomStart;
            break;
        case 1:
            start = atlantisTrackRomStart;
            end = atlantis2TrackRomStart;
            break;
        case 2:
            start = atlantis2TrackRomStart;
            end = atlantis3TrackRomStart;
            break;
        case 3:
            start = atlantis3TrackRomStart;
            end = atlantisBossTrackRomStart;
            break;
        case 4:
            start = atlantisBossTrackRomStart;
            end = carnivalTrackRomStart;
            break;
        case 5:
            start = carnivalTrackRomStart;
            end = carnival2TrackRomStart;
            break;
        case 6:
            start = carnival2TrackRomStart;
            end = carnival3TrackRomStart;
            break;
        case 7:
            start = carnival3TrackRomStart;
            end = piratesTrackRomStart;
            break;
        case 8:
            start = carnivalBossTrackRomStart;
            end = hub1TrackRomStart;
            break;
        case 9:
            start = piratesTrackRomStart;
            end = pirates2TrackRomStart;
            break;
        case 10:
            start = pirates2TrackRomStart;
            end = piratesBossTrackRomStart;
            break;
        case 11:
            start = pirates3TrackRomStart;
            end = carnivalBossTrackRomStart;
            break;
        case 12:
            start = piratesBossTrackRomStart;
            end = prehistoricTrackRomStart;
            break;
        case 13:
            start = prehistoricTrackRomStart;
            end = prehistoric2TrackRomStart;
            break;
        case 14:
            start = prehistoric2TrackRomStart;
            end = prehistoric3TrackRomStart;
            break;
        case 15:
            start = prehistoric3TrackRomStart;
            end = prehistoricBossTrackRomStart;
            break;
        case 16:
            start = prehistoricBossTrackRomStart;
            end = fortressTrackRomStart;
            break;
        case 17:
            start = fortressTrackRomStart;
            end = fortress2TrackRomStart;
            break;
        case 18:
            start = fortress2TrackRomStart;
            end = fortress3TrackRomStart;
            break;
        case 19:
            start = fortress3TrackRomStart;
            end = fortressBossTrackRomStart;
            break;
        case 20:
            start = fortressBossTrackRomStart;
            end = ootwTrackRomStart;
            break;
        case 21:
            start = ootwTrackRomStart;
            end = ootw2TrackRomStart;
            break;
        case 22:
            start = ootw2TrackRomStart;
            end = ootw3TrackRomStart;
            break;
        case 23:
            start = ootw3TrackRomStart;
            end = ootwBossTrackRomStart;
            break;
        case 24:
            start = ootwBossTrackRomStart;
            end = introTrackRomStart;
            break;
        case 25:
            start = titleTrackRomStart;
            end = track46RomStart;
            break;
        case 26:
            start = introTrackRomStart;
            end = outroTrackRomStart;
            break;
        case 27:
            start = outroTrackRomStart;
            end = pirates3TrackRomStart;
            break;
        case 42:
            start = track42RomStart;
            end = track43RomStart;
            break;
        case 43:
            start = track43RomStart;
            end = track44RomStart;
            break;
        case 44:
            start = track44RomStart;
            end = track45RomStart;
            break;
        case 45:
            start = track45RomStart;
            end = atlantisKill1TrackRomStart;
            break;
        case 34:
            start = atlantisKill1TrackRomStart;
            end = track35RomStart;
            break;
        case 35:
            start = track35RomStart;
            end = atlantisWin1TrackRomStart;
            break;
        case 36:
            start = atlantisWin1TrackRomStart;
            end = track37RomStart;
            break;
        case 37:
            start = track37RomStart;
            end = track38RomStart;
            break;
        case 38:
            start = track38RomStart;
            end = track39RomStart;
            break;
        case 39:
            start = track39RomStart;
            end = track40RomStart;
            break;
        case 40:
            start = track40RomStart;
            end = track41RomStart;
            break;
        case 41:
            start = track41RomStart;
            end = bonusTrackRomStart;
            break;
        case 46:
            start = track46RomStart;
            end = track47RomStart;
            break;
        case 47:
            start = track47RomStart;
            end = track48RomStart;
            break;
        case 48:
            start = track48RomStart;
            end = track49RomStart;
            break;
        case 49:
            start = track49RomStart;
            end = track54RomStart;
            break;
        case 50:
            start = track50RomStart;
            end = track51RomStart;
            break;
        case 51:
            start = track51RomStart;
            end = track52RomStart;
            break;
        case 52:
            start = track52RomStart;
            end = track53RomStart;
            break;
        case 53:
            start = track53RomStart;
            end = track42RomStart;
            break;
        case 54:
            start = track54RomStart;
            end = track55RomStart;
            break;
        case 55:
            start = track55RomStart;
            end = track56RomStart;
            break;
        case 56:
            start = track56RomStart;
            end = ootwWin2TrackRomStart;
            break;
        case 57:
            start = ootwWin2TrackRomStart;
            end = track50RomStart;
            break;
        case 58:
            start = bonusTrackRomStart;
            end = hasbroTrackRomStart;
            break;
        case 59:
            start = hasbroTrackRomStart;
            end = title2TrackRomStart;
            break;
        case 60:
            start = title2TrackRomStart;
            end = recordKeyFilesRomStart;
            break;
        default:
            PRINTF("Error, so song specified\n");
            return;
    }
    audioCtrl.song = audioCtrl.ptr + (_pbank2SegmentRomEnd - _pbank1SegmentRomEnd);
    DMAMemory(audioCtrl.song, (u32)start, (u32)end);
    audioCtrl.handle = MusBankStartSong(audioCtrl.base, audioCtrl.song);
    while (!MusHandleAsk(audioCtrl.handle)) {}
    MusHandleSetVolume(audioCtrl.handle, 80);
    if (song == 26 && osTvType == 0) {
        MusHandleSetTempo(audioCtrl.handle, 106);
    }
}

s32 PlaySample(s16 num, Vec3 *pos, s16 vol, s16 pitch) {
    f32 scale = vol;
    f32 dist;
    f32 angle;
    f32 pan;
    Vec3 d;
    s32 handle;

    angle = DistanceBetweenPointsSquared(&camera.pos, pos);
    if (angle > MAX_SFX_DIST * MAX_SFX_DIST) {
        return 0;
    }
    dist = sqrtf(angle);
    pan = MAX_SFX_DIST - dist;
    pan /= MAX_SFX_DIST;
    scale *= pan;
    SubVector2D(&d, pos, &camera.pos);
    angle = Aabs(atan2(d.x, d.z));
    pan = FindShortestAngle(Aabs(camera.rot.y + 1.570796327), angle) * 81.16902096626815;
    if (num) {
        scale *= 1.8;
        if (scale > 255.0f) {
            scale = 255.0f;
        }
        handle = MusStartEffect2(num, (u8)scale, pan, 0, -1);
        MusHandleSetFreqOffset(handle, (f32)(pitch - 128) / 10.0);
        MusHandleSetTempo(handle, pitch);
    } else {
        handle = 0;
    }
    if (reverbOn || camera.state->waterState) {
        MusHandleSetReverb(handle, 30);
    }
    return handle;
}

s32 PlaySampleRadius(s16 num, Vec3 *pos, s16 vol, s16 pitch, f32 radius) {
    f32 scale = vol;
    f32 pan;
    Vec3 d;
    s32 handle;
    f32 dist;

    if (DistanceBetweenPoints(&camera.pos, pos) - radius < 0.0f) {
        dist = 0.0f;
    } else {
        dist = DistanceBetweenPoints(&camera.pos, pos) - radius;
    }
    if (dist > MAX_SFX_DIST) {
        return 0;
    }
    pan = MAX_SFX_DIST - dist;
    pan /= MAX_SFX_DIST;
    scale *= pan;
    SubVector2D(&d, pos, &camera.pos);
    dist = Aabs(atan2(d.x, d.z));
    pan = FindShortestAngle(Aabs(camera.rot.y + 1.570796327), dist) * 81.16902096626815;
    if (num) {
        scale *= 1.8;
        if (scale > 255.0f) {
            scale = 255.0f;
        }
        handle = MusStartEffect2(num, scale, pan, 0, -1);
        MusHandleSetFreqOffset(handle, (f32)(pitch - 128) / 10.0);
        MusHandleSetTempo(handle, pitch);
    } else {
        handle = 0;
    }
    if (reverbOn || camera.state->waterState) {
        MusHandleSetReverb(handle, 30);
    }
    return handle;
}

inline void PlayContinuousSample(ContinuousSample *sfx, s16 num, s16 volume, Vec3 *pos, s16 radius) {
    if (num) {
        if (sfx->handle) {
            StopContinuousSample(sfx);
        }
        sfx->handle = PlaySampleRadius(num, pos, volume, radius, sfx->range);
        volume = (f32)volume * 1.8;
        if (volume > 255) {
            volume = 255;
        }
        sfx->volume = volume;
        sfx->pos = pos;
        sfx->id = num;
        sfx->pitch = radius;
        if (sfx->handle) {
            numContinuousSamples++;
        }
    }
}

inline void StopContinuousSample(ContinuousSample *sfx) {
    if (sfx->handle) {
        MusHandleStop(sfx->handle, 0);
        numContinuousSamples--;
    }
    sfx->outVolume = sfx->volume;
    sfx->handle = sfx->volume = 0;
}

void UpdateContinuousSample(ContinuousSample *sfx) {
    f32 dist;
    f32 angle;
    f32 pan;
    Vec3 d;

    if (sfx->id == 0 || (sfx->handle == 0 && sfx->volume == 0)) {
        return;
    }
    dist = 0.0f;
    if (sfx->range != 0.0f) {
        if (!(DistanceBetweenPoints(&camera.pos, sfx->pos) - sfx->range < 0.0f)) {
            dist = DistanceBetweenPoints(&camera.pos, sfx->pos) - sfx->range;
        }
        if (dist > MAX_SFX_DIST) {
            if (sfx->handle) {
                MusHandleStop(sfx->handle, 0);
                sfx->handle = 0;
                numContinuousSamples--;
            }
            return;
        }
    } else {
        dist = DistanceBetweenPointsSquared(&camera.pos, sfx->pos);
        if (dist > MAX_SFX_DIST * MAX_SFX_DIST) {
            if (sfx->handle) {
                MusHandleStop(sfx->handle, 0);
                sfx->handle = 0;
                numContinuousSamples--;
            }
            return;
        }
        dist = sqrtf(dist);
    }
    dist = MAX_SFX_DIST - dist;
    dist /= MAX_SFX_DIST;
    sfx->outVolume = sfx->volume * dist;
    SubVector2D(&d, sfx->pos, &camera.pos);
    angle = Aabs(atan2(d.x, d.z));
    pan = FindShortestAngle(Aabs(camera.rot.y + 1.570796327), angle) * 81.16902096626815;
    if (sfx->handle == 0) {
        sfx->handle = MusStartEffect2(sfx->id, sfx->outVolume, pan, 0, -1);
        if (reverbOn || camera.state->waterState) {
            MusHandleSetReverb(sfx->handle, 30);
        }
        numContinuousSamples++;
    } else {
        MusHandleSetVolume(sfx->handle, sfx->outVolume);
    }
    MusHandleSetPan(sfx->handle, pan);
    MusHandleSetFreqOffset(sfx->handle, (f32)(sfx->pitch - 128) / 10.0);
    MusHandleSetTempo(sfx->handle, sfx->pitch);
}

void SetMaxSfxDist(s32 dist) {
    MAX_SFX_DIST = dist;
}

void AddAmbientSfx(s32 num, s32 vol, s32 pan) {
    AmbientSound *ptr = ambientSoundList.head.next;
    AmbientSound *ambientSound = JallocAlloc(sizeof(AmbientSound), 1, "AmbSnd");

    ambientSound->prev = ptr;
    ambientSound->next = ptr->next;
    ptr->next->prev = ambientSound;
    ptr->next = ambientSound;
    ambientSoundList.count++;
    vol = (f32)vol * 1.8;
    if (vol > 255) {
        vol = 255;
    }
    ambientSound->sample.handle = MusStartEffect2(num, vol, pan, 0, -1);
    ambientSound->sample.pos = &zero;
    ambientSound->sample.outVolume = ambientSound->sample.baseVolume = vol;
    ambientSound->sample.id = num;
    numContinuousSamples++;
}

void AddAmbientSfxAtPoint(s32 num, s32 volume, Vec3 *pos, s16 radius, s16 freq, s16 randFreq, s16 onTime, s16 platTag,
                          s16 tag, f32 range) {
    AmbientSound *ptr = ambientSoundList.head.next;
    AmbientSound *ambientSound = JallocAlloc(sizeof(AmbientSound), 1, "AmbSnd");

    ambientSound->prev = ptr;
    ambientSound->next = ptr->next;
    ptr->next->prev = ambientSound;
    ptr->next = ambientSound;
    ambientSoundList.count++;
    if (platTag) {
        ambientSound->offset = *pos;
        ambientSound->platform = TagToFirstPlatform(platTag);
    } else {
        ambientSound->pos = *pos;
        ambientSound->platform = NULL;
    }
    ambientSound->sample.volume = volume;
    ambientSound->sample.id = num;
    ambientSound->sample.pitch = radius;
    if (onTime || (freq == 0 && randFreq == 0)) {
        PlayContinuousSample(&ambientSound->sample, num, volume, &ambientSound->pos, radius);
        if (ambientSound->sample.handle && ambientSound->sample.id == 28) {
            MusHandleSetReverb(ambientSound->sample.handle, 40);
        }
        ambientSound->timer = onTime;
    } else {
        ambientSound->timer = freq + Random(randFreq);
        ambientSound->sample.pos = &ambientSound->pos;
    }
    ambientSound->sample.range = range;
    ambientSound->volume = volume;
    ambientSound->minDelay = freq;
    ambientSound->randDelay = randFreq;
    ambientSound->duration = onTime;
    ambientSound->tag = tag;
}

void ClearAmbientSfx(void) {
    AmbientSound *ambientSound;

    for (ambientSound = ambientSoundList.head.next; ambientSound != &ambientSoundList.head; ambientSound = ambientSound->next) {
        if (ambientSound->sample.handle) {
            MusHandleStop(ambientSound->sample.handle, 0);
            numContinuousSamples--;
            ambientSound->sample.outVolume = ambientSound->sample.volume;
            ambientSound->sample.handle = ambientSound->sample.volume = 0;
        }
        ambientSound->sample.pos = &zero;
    }
}

void UpdateAmbientSounds(void) {
    AmbientSound *ambientSound;
    AmbientSound *ambientSound2;
    f32 pitch;

    if (gamePaused || (gameInfo.exitFlags & 1)) {
        for (ambientSound = ambientSoundList.head.next; ambientSound != &ambientSoundList.head; ambientSound = ambientSound->next) {
            if (ambientSound->sample.handle) {
                MusHandleSetVolume(ambientSound->sample.handle, ambientSound->sample.outVolume * sfxVol / 32768);
            }
        }
        return;
    }
    for (ambientSound = ambientSoundList.head.next; ambientSound != &ambientSoundList.head; ambientSound = ambientSound2) {
        ambientSound2 = ambientSound->next;
        if (ambientSound->sample.pos == &zero) {
            if (ambientSound->sample.id == 223) {
                pitch = SineWave(50.0f, frameCount, 0) * 30.0f + 128.0f;
                MusHandleSetFreqOffset(ambientSound->sample.handle, (pitch - 128.0f) / 10.0);
                MusHandleSetTempo(ambientSound->sample.handle, pitch);
                pitch = (SineWave(60.0f, frameCount, 0) + 1.0f) / 4.0f + 0.5;
                pitch *= ambientSound->sample.baseVolume;
                MusHandleSetVolume(ambientSound->sample.handle, pitch);
                ambientSound->sample.outVolume = pitch;
            } else if (ambientSound->sample.handle) {
                MusHandleSetVolume(ambientSound->sample.handle, ambientSound->sample.outVolume * sfxVol / 32768);
            }
            continue;
        }
        if (ambientSound->platform != NULL) {
            RotateVectorByQuaternion(&ambientSound->pos, &ambientSound->offset, &ambientSound->platform->actor.rotQuat);
            ADD_TO_VECTOR(&ambientSound->pos, &ambientSound->platform->actor.pos);
        }
        if (ambientSound->sample.volume) {
            UpdateContinuousSample(&ambientSound->sample);
        }
        if (ambientSound->duration) {
            if (ambientSound->timer == 0 || --ambientSound->timer == 0) {
                if (ambientSound->sample.handle) {
                    if (ambientSound->sample.id == 28) {
                        ambientSound->sample.volume -= 2;
                    } else {
                        ambientSound->sample.volume -= 20;
                    }
                    if (ambientSound->sample.volume <= 0) {
                        StopContinuousSample(&ambientSound->sample);
                        if (ambientSound->minDelay || ambientSound->randDelay) {
                            ambientSound->timer = ambientSound->minDelay + Random(ambientSound->randDelay);
                        } else {
                            SubAmbientSound(ambientSound);
                            JallocFree(&ambientSound);
                        }
                    }
                } else {
                    PlayContinuousSample(&ambientSound->sample, ambientSound->sample.id, ambientSound->volume, &ambientSound->pos, ambientSound->sample.pitch);
                    ambientSound->timer = ambientSound->duration;
                    if (ambientSound->sample.handle && ambientSound->sample.id == 28) {
                        MusHandleSetReverb(ambientSound->sample.handle, 40);
                    }
                }
            }
        } else if (ambientSound->minDelay) {
            if (--ambientSound->timer == 0) {
                if (ambientSound->sample.range != 0.0f) {
                    PlaySampleRadius(ambientSound->sample.id, &ambientSound->pos, ambientSound->sample.volume, ambientSound->sample.pitch, ambientSound->sample.range);
                } else {
                    PlaySample(ambientSound->sample.id, &ambientSound->pos, ambientSound->sample.volume, ambientSound->sample.pitch);
                }
                ambientSound->timer = ambientSound->minDelay + Random(ambientSound->randDelay);
            }
        }
    }
}

s32 PlaySampleNot3D(s32 num, u8 volume, u8 pan, u8 pitch) {
    s32 handle = MusStartEffect2(num, volume, pan, 0, -1);

    MusHandleSetFreqOffset(handle, (f32)(pitch - 128) / 10.0);
    MusHandleSetTempo(handle, pitch);
    return handle;
}

void InitSFXScript(void *script) {
    currentScriptEntry = 0;
    activeSFXScript = script;
}

inline s32 ScriptTriggerSound(s16 second, s16 frame) {
    f32 temp;

    if (activeSFXScript == introSfxScript) {
        if (frameCount == second + 4) {
            return TRUE;
        }
    } else {
        temp = (f32)frame * 0.8;
        if (frameCount == (s16)((s16)(second * 20) + temp)) {
            return TRUE;
        }
    }
    return FALSE;
}

void ProcessSFXScript(void) {
    SFXScriptEntry *script = activeSFXScript;
    s32 handle;
    u8 pitch;
    s32 id;

    if (script != NULL) {
        for (; script->sec >= 0; script++) {
            if (script->endSec != -1 || (gameInfo.exitFlags & 1)) {
                if ((gameInfo.exitFlags & 1) || ScriptTriggerSound(script->endSec, script->endFrame)) {
                    MusHandleStop(script->handle, 0);
                }
            }
        }
        if (!(gameInfo.exitFlags & 1)) {
            for (script = activeSFXScript; script->sec >= 0;) {
                if (ScriptTriggerSound(script->sec, script->frame)) {
                    id = script->id;
                    pitch = script->pitch;
                    if (pitch == 0) {
                        pitch = 0x80;
                    }
                    handle = MusStartEffect2(id, script->volume, 0x80, 0, -1);
                    MusHandleSetFreqOffset(handle, (f32)(pitch - 128) / 10.0);
                    MusHandleSetTempo(handle, pitch);
                    script->handle = handle;
                    if (script->id == 28) {
                        MusHandleSetReverb(handle, 60);
                    }
                    currentScriptEntry++;
                    if (script->sec == -1) {
                        activeSFXScript = NULL;
                        return;
                    }
                }
                script++;
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/sound", pad_80179808);

// linker junk that follows the 10.0 constant emitted by ProcessSFXScript
const u32 D_8010877C[] = { 0x00000000 };
