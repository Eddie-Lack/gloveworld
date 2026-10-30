#include "common.h"
#include "vent.h"
#include "buzzer.h"
#include "ambient.h"
#include "maths.h"
#include "bullet.h"
#include "debris.h"
#include "game.h"

extern s32 frameCount;
extern Vec3 zero;

void InitVentList(void) {
    ventList.head.next = ventList.head.prev = &ventList.head;
    ventList.count = 0;
}

void AddVent(Vent *vent) {
    Vent *ptr = ventList.head.next;

    vent->prev = ptr;
    vent->next = ptr->next;
    ptr->next->prev = vent;
    ptr->next = vent;
    ventList.count++;
}

void SubVent(Vent *vent) {
    vent->prev->next = vent->next;
    vent->next->prev = vent->prev;
    ventList.count--;
}

void UpdateVents(void) {
    Vent *vent;

    for (vent = ventList.head.next; vent != &ventList.head; vent = vent->next) {
        if (vent->active) {
            UpdateVent(vent);
        }
    }
}

void UpdateVent(Vent *vent) {
    Vec3 tempVect2;
    Vec3 tempVect;
    Bullet *bullet;
    Debris *d;
    s32 x;

    if (vent->timer > 0 && --vent->timer == 0) {
        if (vent->on) {
            vent->on = FALSE;
            vent->timer = vent->framesOff[vent->cycle];
        } else {
            if (vent->sound.handle == 0 && vent->sound.id != 0) {
                PlayContinuousSample(&vent->sound, vent->sound.id, 70, &vent->soundPos, vent->sound.pitch);
            }
            vent->on = TRUE;
            vent->timer = vent->framesOn[vent->cycle];
            vent->cycle = (vent->cycle + 1) % vent->numCycles;
            if (vent->timer == 0) {
                vent->on = FALSE;
                vent->timer = 1;
                UpdateVent(vent);
                return;
            }
        }
    }
    if (vent->on && frameCount != 0) {
        if (vent->type == BUBBLE_VENT) {
            if (Random(10) < 5) {
                tempVect2 = vent->velocity;
                tempVect2.y += 1.0f;
                tempVect = vent->origin;
                tempVect.x += Random(10) - 5;
                tempVect.y += Random(10) - 5;
                tempVect.z += Random(10) - 5;
                AddDebris(BUBBLE, &tempVect, &tempVect2, 0, 0);
            }
        } else {
            bullet = CreateNewBullet(vent->bulletType);
            if (vent->parent != NULL) {
                RotateVectorByQuaternion(&bullet->actor.vel, &vent->velocity, &vent->parent->actor.rotQuat);
                RotateVectorByQuaternion(&bullet->actor.pos, &vent->origin, &vent->parent->actor.rotQuat);
                ADD_TO_VECTOR(&bullet->actor.vel, &vent->parent->actor.vel);
                ADD_TO_VECTOR(&bullet->actor.pos, &vent->parent->actor.pos);
            } else {
                bullet->actor.pos = vent->origin;
                bullet->actor.vel = vent->velocity;
            }
            if (vent->bulletType != SNOWBALL_BULLET) {
                bullet->actor.vel.x += (Random(11) - 5) / 20.0f;
                bullet->actor.vel.y += (Random(11) - 5) / 20.0f;
                bullet->actor.vel.z += (Random(11) - 5) / 20.0f;
            }
            InitBulletSpritesOrMesh(bullet, 0, 0);
            if (vent->bulletType == FLAME_BULLET && gameInfo.level == CARNIVAL1) {
                for (x = 0; x < 2; x++) {
                    bullet->sprites[x].scaleX *= 2;
                    bullet->sprites[x].scaleY *= 2;
                    bullet->sprites[x].anim.def.startScale *= 2;
                    bullet->sprites[x].anim.def.endScale *= 2;
                }
            } else if (vent->bulletType == CANNONBALL_BULLET) {
                PlaySample(27, &bullet->actor.pos, 100, 100);
                AddDebris(POPB, &bullet->actor.pos, &zero, 0, 0);
                for (x = 0; x < 5; x++) {
                    tempVect2.x = bullet->actor.vel.x / 2.0f + Random(7) - 3.0f;
                    tempVect2.y = bullet->actor.vel.y / 2.0f + Random(7) - 3.0f;
                    tempVect2.z = bullet->actor.vel.z / 2.0f + Random(7) - 3.0f;
                    d = AddDebris(WIND_BIT, &bullet->actor.pos, &tempVect2, 0, 0);
                    if (d != NULL) {
                        d->lifeTimer = 5;
                        d->info->friction = 0.6f;
                        d->friction = 0.6f;
                    }
                }
            }
            AddBullet(bullet);
            vent->soundPos = bullet->actor.pos;
            bullet->spell = bullet->spellType = 0xF;
        }
    } else if (vent->sound.handle != 0 && gameInfo.level != CARNIVAL1) {
        vent->sound.volume -= 5;
        if (vent->sound.volume <= 0) {
            StopContinuousSample(&vent->sound);
        }
    }
    UpdateContinuousSample(&vent->sound);
}

Vent *FindTaggedVent(Vent *startVent, s32 tag) {
    Vent *ventPtr;

    for (ventPtr = startVent->next; ventPtr != &ventList.head; ventPtr = ventPtr->next) {
        if (ventPtr->tag == tag) {
            return ventPtr;
        }
    }
    return NULL;
}

AmbientSound *FindTaggedAmbientSound(AmbientSound *start, s32 tag) {
    AmbientSound *sound;

    for (sound = start->next; sound != &ambientSoundList.head; sound = sound->next) {
        if (sound->tag == tag) {
            return sound;
        }
    }
    return NULL;
}

Buzzer *FindTaggedBuzzer(Buzzer *startBuzzer, s32 tag) {
    Buzzer *buzzerPtr;

    for (buzzerPtr = startBuzzer->next; buzzerPtr != &buzzerList.head; buzzerPtr = buzzerPtr->next) {
        if (buzzerPtr->tag == tag) {
            return buzzerPtr;
        }
    }
    return NULL;
}
