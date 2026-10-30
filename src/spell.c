#include "common.h"
#include "hand.h"
#include "debris.h"
#include "sound.h"
#include "bullet.h"
#include "game.h"
#include "ball.h"
#include "collect.h"
#include "spell.h"
#include "enemy.h"

extern Vec3 zero;

extern s32 spellDuration;

void ActivateSpell(Actor *actor, s16 spell, s16 duration) {
    s32 i;

    if (actor->type == HUBCHICKEN) {
        return;
    }
    enemies[actor->state->index].brain.spellTimer[spell] += duration;
    switch (spell) {
        case 1:
            break;
        case 2:
            if (gameInfo.level == PIRATESBOSS) {
                if (actor->type == SPANK) {
                    actor->obj->spheres[7].targetRadius = actor->phys->radius;
                    enemies[actor->state->index].brain.spellTimer[spell] = 0;
                } else if (!enemies[1].brain.spellTimer[2] && !enemies[2].brain.spellTimer[2]) {
                    enemies[0].actor.obj->spheres[7].targetRadius = enemies[0].actor.phys->radius;
                    enemies[0].brain.spellTimer[2] = 0;
                } else {
                    enemies[actor->state->index].brain.spellTimer[spell] = 50;
                }
            } else if (actor->type == KLOSET || actor->type == ROBOT) {
                actor->obj->spheres[0].targetRadius = actor->phys->radius;
                enemies[actor->state->index].brain.spellTimer[spell] = 0;
            }
            break;
        case 0:
            for (i = 0; i < 2; i++) {
                if (enemies[actor->state->index].samples[i].handle) {
                    StopContinuousSample(&enemies[actor->state->index].samples[i]);
                }
            }
            break;
    }
}

void ActivateSpellHand(s16 spell) {
    s32 i;

    handControl.spellTimer[spell] += spellDuration;
    if (spell == 2) {
        if (ball.link == &hand) {
            ApplyAction(HAND_RELEASE);
            handControl.releaseTimer = 0;
            ball.link = NULL;
        }
        for (i = 0; i < 15; i++) {
            if (player.powerups[i] > 40 && !(i >= 7 && i < 13)) {
                if (i != 4) {
                    player.powerups[i] = 40;
                } else {
                    DeactivateCollectable(HERCULES);
                    handControl.morphStep = 0;
                }
            }
        }
    }
}

void DeactivateSpell(Actor *actor, s16 spell) {
    s32 i;

    switch (spell) {
        case 0:
            if (enemies[actor->state->index].actor.type != FRANKIE) {
                for (i = 0; i < 2; i++) {
                    if (enemies[actor->state->index].samples[i].outVolume) {
                        PlayContinuousSample(&enemies[actor->state->index].samples[i],
                                             enemies[actor->state->index].samples[i].id,
                                             enemies[actor->state->index].samples[i].outVolume,
                                             enemies[actor->state->index].samples[i].pos,
                                             enemies[actor->state->index].samples[i].pitch);
                    }
                }
            }
            break;
        case 1:
            KillEnemy(&enemies[actor->state->index]);
            break;
        case 2:
            break;
    }
}

void DeactivateSpellHand(s16 spell) {
    if (spell == 2 && handControl.frog) {
        DefrogHand();
    }
}

void RemoveHandAndBallBullets(void) {
    Bullet *bullet;
    Bullet *bullet2;

    for (bullet = bulletList.head.next; bullet != &bulletList.head; bullet = bullet2) {
        bullet2 = bullet->next;
        if (bullet->target == &hand || bullet->target == &ball || gameInfo.level == PREHISTORICBOSS) {
            SubBullet(bullet);
        }
    }
}

void AddSpellDebris(s16 debrisType, Vec3 *pos, Vec3 *vel, s32 spell) {
    Debris *d = AddDebris(debrisType, pos, vel, 0, 0);

    if (d) {
        d->sprite.anim.def.life = d->sprite.anim.lifeTimer = 15;
        d->flags |= 1;
        d->pos.y -= 5.0f;
    }
}

void AddBulletFlash(Vec3 *pos) {
    Debris *deb = AddDebris(GLOW, pos, &zero, NULL, NULL);

    if (deb) {
        deb->sprite.r = 0;
        deb->sprite.g = 160;
        deb->sprite.b = 255;
        deb->sprite.scaleX *= 4;
        deb->sprite.scaleY *= 4;
        deb->sprite.anim.def.endScale *= 0.5;
        deb->sprite.anim.def.type = 5;
        deb->life = deb->lifeTimer = 12;
        deb->sprite.anim.def.life = deb->sprite.anim.lifeTimer = 6;
        deb->sprite.anim.def.startScale *= 4;
    }
    deb = AddDebris(FAIRY_HALF, pos, &zero, &deb->sprite, NULL);
    if (deb) {
        deb->sprite.scaleX *= 4;
        deb->sprite.scaleY *= 4;
        deb->sprite.anim.def.endScale *= 0.5;
        deb->sprite.anim.def.type = 5;
        deb->life = deb->lifeTimer = 12;
        deb->sprite.anim.def.life = deb->sprite.anim.lifeTimer = 6;
        deb->sprite.anim.def.startScale *= 4;
    }
}

INCLUDE_ASM("asm/nonmatchings/spell", pad_801AE508);

INCLUDE_RODATA("asm/nonmatchings/spell", pad_8010B568);
