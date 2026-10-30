#include "common.h"
#include "ident.h"
#include "overlay.h"
#include "texture.h"
#include "jalloc.h"
#include "actor.h"
#include "game.h"
#include "maths.h"
#include "display.h"
#include "debug.h"
#include "text.h"
#include "debris.h"
#include "macros.h"
#include "camera.h"
#include "weather.h"
#include "libc.h"
#include "hand.h"
#include "controller.h"
#include "ball.h"
#include "object.h"

extern s32 backbodge;

extern s32 D_801E753C;
extern ScreenDebris screenDebris[40];
extern Sprite template_sprite;
extern s32 overlayBitCount;
extern void *starScreenDebris;
extern void *lifeScreenDebris;
extern void *rainScreenDebris;
extern void *snowScreenDebris;
extern f32 snowFac;
extern f32 snowFac2;
extern f32 snowFac3;
extern f32 snowMaxScale;
extern void *tickTexture;
extern void *crossTexture;

u32 UpdateCRC(char *ptr);
void FindTexture(void *texPtr, u32 texID, s32 verbose);

extern Gfx rspInitForOverlays_dl[];
extern Gfx rdpInitForBackdrops_dl[];
extern Gfx rspInitForBackdrops_dl[];
extern Gfx rdpInitForOverlays_dl[];
extern u8 displayLogos;
extern Text *startText;
extern Text *noControllerTextFrench;
extern Text *noControllerTextGerman;
extern s32 frameCount;
extern u8 draw_buffer;
extern Gfx overlay_dl[];
extern Gfx overlay_dl2[];
extern Gfx *overlay_dl_next[];
extern s32 spriteXluMode[2];
extern Gfx rspInitForSprites_dl[];
extern Gfx rdpInitForSprites_dl[];
extern Gfx setrend_light_fog2[];
extern f32 fm;
extern float printSpritesProj[4][4];
extern u8 gamePaused;
extern Vp vp;
extern s16 farPlaneDist;
extern s16 nearPlaneDist;
extern Texture *lastSpriteTex;
extern s32 redrawActors;
extern void *vtxPtr;
extern s16 fieldOfView;
extern Actor fadeOverlay;
extern Actor starOverlay;

void InitDisplayLists(void);
void SetupViewing(void);
void ApplyGlobalTransformations(void);
void ClearViewing(void);

extern Overlay controllerOL[11];
extern Overlay health1OL;
extern Overlay health2OL;
extern Overlay bossHealthOL[5];
extern Overlay pickupTimerOL[5];
extern Overlay garibsGotOL;
extern Overlay numLivesOL;
extern u8 ActiveController;
extern u8 printController;
extern s16 rcdelay;
extern f32 rcfac;
extern s16 rcxoff;
extern s16 rcyoff;
extern s32 pickupSep;
extern s32 pickupY;
extern s32 pickupX;
extern s32 numPickBubs;
extern s32 numGaribs;
extern u8 spellColour[][2][4];
void SetFont(s32 type);

inline void FindAndSetupOverlay(Overlay *overlay, char *name, s32 xPos, s32 yPos, s32 zPos, s32 attr) {
    FindTexture(&overlay->tex, UpdateCRC(name), 0);
    overlay->frames = NULL;
    overlay->scaleX = overlay->scaleY = 0x400;
    overlay->x = xPos;
    overlay->y = yPos;
    overlay->depth = zPos;
    SetupOverlay(overlay);
    overlay->alpha = 255;
    overlay->sprite.attr = attr;
}

void FindSetupAndAddOverlay(Overlay *overlay, char *name, s32 xPos, s32 yPos, s32 zPos, s32 attr) {
    FindAndSetupOverlay(overlay, name, xPos, yPos, zPos, attr);
    AddOverlay(overlay);
}

void SetupOverlay(Overlay *overlay) {
    Texture *tex = overlay->tex;
    char name[16];
    s32 tileH = 32;
    s32 tileW = 32;
    s32 numTiles;
    s32 tilesY;

    if (tex->width > 64) {
        tileH = 16;
    }
    if (tex->width > 128) {
        tileH = 8;
    }
    numTiles = tex->width / tileW;
    if (tex->width % tileW) {
        numTiles++;
    }
    tilesY = tex->height / tileH;
    if (tex->height % tileH) {
        tilesY++;
    }
    numTiles *= tilesY;
    sprintf(name, "bit-%d", overlayBitCount++);
    overlay->bitmaps = JallocAlloc(numTiles * sizeof(Bitmap), 1, name);
    overlay->r = overlay->g = overlay->b = 255;
    overlay->alpha = 127;
    overlay->sprite = template_sprite;
    overlay->sprite.bmfmt = tex->fmt;
    overlay->sprite.bmsiz = tex->siz;
    overlay->sprite.width = tex->width;
    overlay->sprite.height = tex->height;
    overlay->behind = 1;
    SetupOverlayData(overlay);
}

void SetupOverlayData(Overlay *overlay) {
    Texture *tex = overlay->tex;
    s32 width = tex->width;
    s32 height = tex->height;
    s32 tileW = 32;
    s32 tileH = 32;
    s32 tilesX;
    s32 tilesY;
    s32 numTiles;
    s32 tileX;
    s32 tileY;
    Bitmap *bitmap;

    if (width > 64) {
        tileH = 16;
    }
    if (width > 128) {
        tileH = 8;
    }
    tilesX = width / tileW;
    if (width % tileW) {
        tilesX++;
    }
    tilesY = height / tileH;
    if (height % tileH) {
        tilesY++;
    }
    numTiles = tilesX * tilesY;
    bitmap = overlay->bitmaps;
    for (tileY = 0; tileY < tilesY; tileY++) {
        for (tileX = 0; tileX < tilesX; tileX++) {
            bitmap->s = tileX * tileW;
            bitmap->t = 0;
            if (width < 32) {
                bitmap->width = width;
            } else if (tileX == tilesX - 1 && tileX > 0) {
                bitmap->width = width % tileW;
                if (bitmap->width == 0) {
                    bitmap->width = tileW;
                }
            } else {
                bitmap->width = tileW;
            }
            bitmap->width_img = width;
            bitmap->buf = (u16 *)tex->data + tileY * tileH * width;
            if (height < tileH) {
                bitmap->actualHeight = height;
            } else if (tileY == tilesY - 1 && tileY > 0) {
                bitmap->actualHeight = height % tileH;
                if (bitmap->actualHeight == 0) {
                    bitmap->actualHeight = tileH;
                }
            } else {
                bitmap->actualHeight = tileH;
            }
            bitmap->LUToffset = 0;
            bitmap++;
        }
    }
    overlay->sprite.bmheight = MIN(tex->height, tileH);
    overlay->sprite.bmHreal = MIN(tex->height, tileH);
    overlay->sprite.nbitmaps = numTiles;
    overlay->sprite.ndisplist = numTiles * 12 + 24;
    overlay->sprite.bitmap = overlay->bitmaps;
}

void InitOverlayList(void) {
    overlayList.head.next = overlayList.head.prev = &overlayList.head;
    overlayList.count = 0;
}

void InitBackdropList(void) {
    backdropList.head.next = backdropList.head.prev = &backdropList.head;
    backdropList.count = 0;
}

void AddBackdrop(Backdrop *backdrop) {
    Backdrop *ptr;

    if (backdrop->next == NULL) {
        for (ptr = backdropList.head.next; ptr != &backdropList.head; ptr = ptr->next) {
            if (ptr->depth < backdrop->depth) {
                break;
            }
            if (ptr->tex == backdrop->tex && ptr->depth == backdrop->depth) {
                break;
            }
        }
        backdrop->next = ptr;
        backdrop->prev = ptr->prev;
        ptr->prev->next = backdrop;
        ptr->prev = backdrop;
        backdropList.count++;
        backdrop->visible = 1;
    }
}

inline void SubOverlay(Overlay *overlay) {
    if (overlay->next) {
        JallocFree(&overlay->bitmaps);
        overlay->prev->next = overlay->next;
        overlay->next->prev = overlay->prev;
        overlay->next = NULL;
        overlayList.count--;
    }
}

inline void AddOverlay(Overlay *overlay) {
    Overlay *ptr;

    if (overlay->next == NULL) {
        for (ptr = overlayList.head.next; ptr != &overlayList.head; ptr = ptr->next) {
            if (ptr == NULL) {
                PRINTF("oo la la\n");
            }
            if (ptr->depth < overlay->depth) {
                break;
            }
        }
        overlay->next = ptr;
        overlay->prev = ptr->prev;
        ptr->prev->next = overlay;
        ptr->prev = overlay;
        overlayList.count++;
    }
}

void InitSpriteList(void) {
    spriteList.head.next = spriteList.head.prev = &spriteList.head;
    spriteList.count = 0;
    spriteList.head.flags = 1;
}

void AddSprite(GameSprite *sprite, GameSprite *after) {
    GameSprite *ptr;

    if (sprite->next == NULL) {
        if (after == NULL) {
            for (ptr = &spriteList.head; ptr != spriteList.head.prev; ptr = ptr->next) {
                if (!(sprite->flags & 1) && (ptr->next->flags & 1)) {
                    break;
                }
            }
            after = ptr;
        }
        sprite->prev = after;
        sprite->next = after->next;
        after->next->prev = sprite;
        after->next = sprite;
        spriteList.count++;
        sprite->unk1C = 0;
    }
}

void SubSprite(GameSprite *sprite) {
    if (sprite->next != NULL) {
        sprite->anim.def.type = 0;
        sprite->prev->next = sprite->next;
        sprite->next->prev = sprite->prev;
        if (sprite->unk1C == 0) {
            sprite->next->unk1C = 0;
        }
        sprite->next = NULL;
        spriteList.count--;
    }
}

void PrintOverlays(void) {
    Overlay *overlay;

    gDPPipeSync(glistp++);
    gSPDisplayList(glistp++, rspInitForOverlays_dl);
    gSPDisplayList(glistp++, rdpInitForOverlays_dl);
    if (overlayList.count && displayLogos) {
        for (overlay = overlayList.head.next; overlay != &overlayList.head; overlay = overlay->next) {
            if (overlay->behind) {
                PrintOverlay(overlay);
            }
        }
    }
    if (gameInfo.mode == 4 && gameInfo.level != PRESENT && gameInfo.level != SPACEBOSS_TWEEN && gameInfo.level != FLYTHRU
        && gameInfo.level != FLYTHRU2) {
        PrintStats();
    }
    if (gameInfo.mode == 4 && gameInfo.level == FLYTHRU) {
        if (startText != NULL) {
            startText->a = (SineWave(2.0f, frameCount / 3.0f, 0) + 1.0f) * 60.0f + 45.0f;
        }
        if (noControllerTextFrench != NULL) {
            noControllerTextFrench->a = (SineWave(2.0f, (frameCount + 5) / 3.0f, 0) + 1.0f) * 60.0f + 45.0f;
        }
        if (noControllerTextGerman != NULL) {
            noControllerTextGerman->a = (SineWave(2.0f, (frameCount + 10) / 3.0f, 0) + 1.0f) * 60.0f + 45.0f;
        }
    }
}

void PrintOverlaysBehind(void) {
    Overlay *overlay;

    gDPPipeSync(glistp++);
    gSPDisplayList(glistp++, rspInitForOverlays_dl);
    gSPDisplayList(glistp++, rdpInitForOverlays_dl);
    if (overlayList.count && displayLogos) {
        for (overlay = overlayList.head.next; overlay != &overlayList.head; overlay = overlay->next) {
            if (!overlay->behind) {
                PrintOverlay(overlay);
            }
        }
    }
}

void PrintOverlay(Overlay *overlay) {
    Sprite *sp = &overlay->sprite;
    Gfx *gfx;
    f32 x1;
    f32 y1;

    if ((draw_buffer ? &overlay_dl[NUM_DL(178)] < overlay_dl_next[0] : &overlay_dl2[NUM_DL(178)] < overlay_dl_next[0]) ||
        !overlay->scaleX || !overlay->scaleY) {
        return;
    }
    if (overlay->frames) {
        if (overlay->sprite.nbitmaps >= 2) {
            overlay->tex = overlay->frames->textures[overlay->frame];
            SetupOverlayData(overlay);
        } else {
            overlay->sprite.bitmap->buf = ((Texture *)overlay->frames->textures[overlay->frame])->data;
        }
        if (++overlay->frame >= overlay->frames->count) {
            overlay->frame = 0;
        }
    }
    gfx = glistp;
    spInit(&gfx);
    sp->rsp_dl_next = overlay_dl_next[0];
    x1 = ((Texture *)overlay->tex)->width / 2.0f;
    y1 = ((Texture *)overlay->tex)->height / 2.0f;
    x1 *= overlay->scaleX / 1024.0f;
    y1 *= overlay->scaleY / 1024.0f;
    x1 = overlay->x - x1;
    y1 = overlay->y - y1;
    spMove(sp, x1, y1);
    spColor(sp, overlay->r, overlay->g, overlay->b, overlay->alpha);
    spScale(sp, overlay->scaleX / 1024.0f, overlay->scaleY / 1024.0f);
    gSPDisplayList(gfx++, spDraw(sp));
    overlay_dl_next[0] = sp->rsp_dl_next;
    spFinish(&gfx);
    glistp = gfx - 1;
}

void PrintBackdrops(void) {
    Backdrop *backdrop;

    if (backdropList.count) {
        gDPPipeSync(glistp++);
        gSPDisplayList(glistp++, rdpInitForBackdrops_dl);
        gSPDisplayList(glistp++, rspInitForBackdrops_dl);
        for (backdrop = backdropList.head.next; backdrop != &backdropList.head; backdrop = backdrop->next) {
            if (backdrop->visible && backdrop->tex) {
                gDPSetPrimColor(glistp++, 0, 0, backdrop->r, backdrop->g, backdrop->b, backdrop->alpha);
                PrintBackdrop(backdrop);
            }
        }
    }
    gDPPipeSync(glistp++);
}

void PrintBackdrop(Backdrop *backdrop) {
    guSprite2DInit(&backdrop->sprite[0], backdrop->tex->data, backdrop->tex->palette, backdrop->tex->width,
                   backdrop->tex->width, backdrop->tex->height, (s16)backdrop->tex->fmt, (s16)backdrop->tex->siz, 0, 0);
    gSPSprite2DBase(glistp++, OS_K0_TO_PHYSICAL(&backdrop->sprite[0]));
    gSPSprite2DScaleFlip(glistp++, 0x100000 / backdrop->scaleX, 0x100000 / backdrop->scaleY, backdrop->flipX,
                         backdrop->flipY);
    gSPSprite2DDraw(glistp++, backdrop->x, backdrop->y);
    if (backdrop->wrap) {
        guSprite2DInit(&backdrop->sprite[1], backdrop->tex->data, backdrop->tex->palette, backdrop->tex->width,
                       backdrop->tex->width, backdrop->tex->height, (s16)backdrop->tex->fmt, (s16)backdrop->tex->siz, 0, 0);
        gSPSprite2DBase(glistp++, OS_K0_TO_PHYSICAL(&backdrop->sprite[1]));
        gSPSprite2DScaleFlip(glistp++, 0x100000 / backdrop->scaleX, 0x100000 / backdrop->scaleY, backdrop->flipX,
                             backdrop->flipY);
        if (backdrop->x < 0) {
            if (backdrop->x + (backdrop->tex->width * backdrop->scaleX >> 8) < 0x500) {
                gSPSprite2DDraw(glistp++, backdrop->x + (backdrop->tex->width * backdrop->scaleX >> 8), backdrop->y);
            }
        } else if (backdrop->x > 0) {
            gSPSprite2DDraw(glistp++, backdrop->x - (backdrop->tex->width * backdrop->scaleX >> 8), backdrop->y);
        }
    }
}

void InitSpriteAnimation(GameSprite *sprite, SpriteAnim *temp, s16 lifespan) {
    sprite->anim.def = *temp;
    sprite->anim.delayTimer = 0;
    sprite->flags = sprite->anim.def.flags;
    sprite->alpha = sprite->anim.def.startAlpha;
    if (sprite->anim.def.type == 6) {
        sprite->anim.frame = sprite->anim.def.frames->count - 1;
        sprite->scaleX = sprite->scaleY = sprite->anim.def.endScale;
        sprite->flags |= 0x10;
    } else {
        sprite->anim.frame = 0;
        sprite->scaleX = sprite->scaleY = sprite->anim.def.startScale;
        sprite->flags &= ~0x10;
    }
    sprite->anim.lifeTimer = sprite->anim.def.life = lifespan;
    if (sprite->anim.def.type == 4) {
        sprite->anim.frame = Random(sprite->anim.def.frames->count);
        sprite->anim.def.type = 1;
    }
    sprite->tex = temp->frames->textures[sprite->anim.frame];
}

void AnimateSprite(GameSprite *sprite) {
    SpriteAnimState *anim = &sprite->anim;

    if (anim->def.life && --anim->lifeTimer == 0) {
        sprite->tex = NULL;
        sprite->anim.def.type = 0;
        return;
    }
    if (anim->def.type == 7) {
        return;
    }
    if (anim->def.endAlpha != anim->def.startAlpha) {
        if (anim->def.life) {
            sprite->alpha = anim->def.endAlpha + (anim->def.startAlpha - anim->def.endAlpha) * anim->lifeTimer / anim->def.life;
        } else {
            sprite->alpha = anim->def.endAlpha + (anim->def.startAlpha - anim->def.endAlpha) * (anim->def.frames->count - anim->frame - 1) /
                                              (anim->def.frames->count - 1);
        }
    }
    if (anim->def.endScale != anim->def.startScale) {
        if (anim->def.life) {
            sprite->scaleX = sprite->scaleY = anim->def.endScale + (anim->def.startScale - anim->def.endScale) * anim->lifeTimer / anim->def.life;
        } else {
            sprite->scaleX = sprite->scaleY =
                anim->def.endScale + (anim->def.startScale - anim->def.endScale) * (anim->def.frames->count - anim->frame - 1) /
                                  (anim->def.frames->count - 1);
        }
    }
    if (anim->def.type == 5) {
        return;
    }
    if (anim->frame >= anim->def.frames->count) {
        if (anim->def.type == 1) {
            anim->frame = 0;
        } else if (anim->def.type == 2) {
            sprite->tex = NULL;
            return;
        }
    } else if (anim->def.type == 6 && anim->frame == 0) {
        sprite->tex = NULL;
        return;
    }
    sprite->tex = anim->def.frames->textures[anim->frame];
    if (anim->delayTimer > 0) {
        anim->delayTimer -= 32;
        return;
    }
    if (sprite->flags & 0x10) {
        anim->frame--;
    } else {
        anim->frame++;
    }
    anim->delayTimer += anim->def.frameDelay;
    if (anim->def.type == 3) {
        if (anim->frame >= anim->def.frames->count) {
            sprite->flags |= 0x10;
            anim->frame--;
        } else if (anim->frame == 0) {
            sprite->flags &= ~0x10;
        }
    }
}

GameSprite *PrintSpritesOpaque(void) {
    GameSprite *sprite;
    Mtx m;

    spriteXluMode[1] = 0;
    gSPDisplayList(glistp++, rspInitForSprites_dl);
    gSPDisplayList(glistp++, rdpInitForSprites_dl);
    fm = 128000 / (fog.max - fog.min);
    if (fog.on) {
        gSPDisplayList(glistp++, setrend_light_fog2);
    }
    guMtxCatL(&dynamicp->cameraView, &dynamicp->projection, &m);
    guMtxL2F(printSpritesProj, &m);
    spriteXluMode[0] = 0;
    if (fog.on) {
        gDPSetRenderMode(glistp++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_TEX_EDGE2);
        gDPSetCombineMode(glistp++, G_CC_MODULATEIA_PRIM, G_CC_PASS2);
    } else {
        gDPSetRenderMode(glistp++, G_RM_AA_ZB_TEX_EDGE, G_RM_AA_ZB_TEX_EDGE2);
        gDPSetCombineMode(glistp++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    }
    if (gamePaused) {
        for (sprite = spriteList.head.next; sprite != &spriteList.head && !(sprite->flags & 1);
             sprite = sprite->next) {
            PrintSprite(sprite);
        }
    } else {
        for (sprite = spriteList.head.next; sprite != &spriteList.head && !(sprite->flags & 1);
             sprite = sprite->next) {
            PrintSprite(sprite);
            if (sprite->anim.def.type) {
                AnimateSprite(sprite);
            }
        }
    }
    return sprite;
}

void PrintSpritesTranslucent(GameSprite *sprite) {
    Mtx m;

    spriteXluMode[1] = 0;
    gSPDisplayList(glistp++, rspInitForSprites_dl);
    gSPDisplayList(glistp++, rdpInitForSprites_dl);
    fm = 128000 / (fog.max - fog.min);
    if (fog.on) {
        gSPDisplayList(glistp++, setrend_light_fog2);
    }
    guMtxCatL(&dynamicp->cameraView, &dynamicp->projection, &m);
    guMtxL2F(printSpritesProj, &m);
    if (fog.on) {
        gDPSetRenderMode(glistp++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_TEX_EDGE2);
        gDPSetCombineMode(glistp++, G_CC_MODULATEIA_PRIM, G_CC_PASS2);
    } else {
        gDPSetRenderMode(glistp++, G_RM_AA_ZB_TEX_EDGE, G_RM_AA_ZB_TEX_EDGE2);
        gDPSetCombineMode(glistp++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    }
    if (gamePaused) {
        for (; sprite != &spriteList.head; sprite = sprite->next) {
            PrintSprite(sprite);
        }
    } else {
        for (; sprite != &spriteList.head; sprite = sprite->next) {
            PrintSprite(sprite);
            if (sprite->anim.def.type) {
                AnimateSprite(sprite);
            }
        }
    }
}

void PrintSprite(GameSprite *sprite) {
    f32 dist;
    f32 depth;
    f32 sx;
    f32 sy;
    f32 fogLevel;
    f32 screenDist;
    f32 clipW;
    s32 scaleX;
    s32 scaleY;
    s32 distInt;
    s32 stepX;
    s32 stepY;

    if (sprite->tex == NULL || sprite->scaleX == 0 || sprite->scaleY == 0) {
        return;
    }
    dist = DistanceBetweenPointsSquared(&camera.pos, &sprite->pos);
    if (farPlaneDist * farPlaneDist < dist || dist < nearPlaneDist * nearPlaneDist) {
        return;
    }
    clipW = printSpritesProj[0][3] * sprite->pos.x + printSpritesProj[1][3] * sprite->pos.y + printSpritesProj[2][3] * sprite->pos.z + printSpritesProj[3][3];
    if (clipW <= 0.0f) {
        return;
    }
    depth = printSpritesProj[0][2] * sprite->pos.x + printSpritesProj[1][2] * sprite->pos.y + printSpritesProj[2][2] * sprite->pos.z + printSpritesProj[3][2];
    if (depth < 0.0f) {
        return;
    }
    sx = printSpritesProj[0][0] * sprite->pos.x + printSpritesProj[1][0] * sprite->pos.y + printSpritesProj[2][0] * sprite->pos.z + printSpritesProj[3][0];
    sx = sx * vp.vp.vscale[0] / clipW + vp.vp.vtrans[0];
    sy = printSpritesProj[0][1] * sprite->pos.x + printSpritesProj[1][1] * sprite->pos.y + printSpritesProj[2][1] * sprite->pos.z
         + printSpritesProj[3][1];
    sy = sy * vp.vp.vscale[1] / clipW + vp.vp.vtrans[1];
    depth = (depth * vp.vp.vscale[2] / clipW + vp.vp.vtrans[2]) * 32.0f;
    fogLevel = MAX(0.0f, (depth - 16360.0f) / 16360.0f * fm + 255.0f - fm);
    if (fogLevel >= 255.0f) {
        return;
    }
    sy = 960.0f - sy;
    dist = sqrtf(dist);
    scaleX = sprite->scaleX * 33;
    screenDist = dist * fieldOfView;
    sx += (f32)(sprite->offsetX * scaleX * 16) / screenDist;
    if (sx > 1280.0f || sx + (f32)(sprite->tex->width * scaleX * 16) / screenDist <= 0.0f) {
        return;
    }
    scaleY = sprite->scaleY * 33;
    sy += (f32)(sprite->offsetY * scaleY * 16) / screenDist;
    if (sy > 960.0f || sy + (f32)(sprite->tex->height * scaleY * 16) / screenDist <= 0.0f) {
        return;
    }
    if (spriteXluMode[0] == 0 && (sprite->flags & 1)) {
        spriteXluMode[0] = 1;
        if (fog.on) {
            gDPSetRenderMode(glistp++, G_RM_FOG_PRIM_A, G_RM_ZB_CLD_SURF2);
        } else {
            gDPSetRenderMode(glistp++, G_RM_ZB_CLD_SURF, G_RM_ZB_CLD_SURF2);
        }
    }
    if (fog.on) {
        gDPSetFogColor(glistp++, fog.r, fog.g, fog.b, (u32)fogLevel);
    }
    gDPSetPrimColor(glistp++, 0, 0, sprite->r, sprite->g, sprite->b, sprite->alpha);
    distInt = dist * fieldOfView;
    stepX = distInt << 8;
    stepX /= sprite->scaleX * 33;
    stepY = distInt << 8;
    stepY /= sprite->scaleY * 33;
    TileRectangle(&glistp, sprite, sx, sy, depth, stepX, stepY);
    gDPPipeSync(glistp++);
    lastSpriteTex = sprite->tex;
}

void TileRectangle(Gfx **gdl, GameSprite *sprite, f32 x, f32 y, s32 z, s32 sx, s32 sy) {
    Gfx *g = *gdl;
    s32 dsdx;
    s32 dtdy;
    s32 s;
    s32 t;
    Texture *tex;
    f32 xh;
    s32 s2;
    s32 t2;
    f32 yh;

    gDPSetPrimDepth(g++, z, 0);
    dsdx = sx;
    if (sprite->flags & 2) {
        dsdx = -dsdx;
    }
    dtdy = sy;
    if (sprite->flags & 4) {
        dtdy = -dtdy;
    }
    t = -16;
    if (sx >= 512) {
        t = -16 - ((s32)y & 3) * 8;
    }
    s = -16;
    if (sprite->tex != lastSpriteTex) {
        switch ((s16)sprite->tex->siz) {
            case G_IM_SIZ_4b:
                gDPSetTextureLUT(g++, G_TT_RGBA16);
                gDPLoadTLUT_pal16(g++, 0, sprite->tex->palette);
                gDPLoadTextureTile_4b(g++, sprite->tex->data, sprite->tex->fmt, sprite->tex->width,
                                      sprite->tex->height, 0, 0, sprite->tex->width - 1, sprite->tex->height - 1, 0,
                                      G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                break;
            case G_IM_SIZ_8b:
                gDPSetTextureLUT(g++, G_TT_RGBA16);
                gDPLoadTLUT_pal256(g++, sprite->tex->palette);
                gDPLoadTextureTile(g++, sprite->tex->data, sprite->tex->fmt, G_IM_SIZ_8b, sprite->tex->width,
                                   sprite->tex->height, 0, 0, sprite->tex->width - 1, sprite->tex->height - 1, 0,
                                   G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                break;
            case G_IM_SIZ_16b:
                gDPSetTextureLUT(g++, G_TT_NONE);
                gDPLoadTextureTile(g++, sprite->tex->data, sprite->tex->fmt, G_IM_SIZ_16b, sprite->tex->width,
                                   sprite->tex->height, 0, 0, sprite->tex->width - 1, sprite->tex->height - 1, 0,
                                   G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                break;
            case G_IM_SIZ_32b:
                gDPLoadTextureTile(g++, sprite->tex->data, sprite->tex->fmt, G_IM_SIZ_32b, sprite->tex->width,
                                   sprite->tex->height, 0, 0, sprite->tex->width - 1, sprite->tex->height - 1, 0,
                                   G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                break;
        }
    }
    tex = sprite->tex;
    xh = tex->width - 1;
    xh *= 4096.0f;
    xh /= sx;
    xh += x;
    yh = tex->height - 1;
    yh *= 4096.0f;
    yh /= sy;
    yh += y;
    s2 = (sprite->flags & 2) ? (tex->width << 5) - s - 32 : s;
    t2 = (sprite->flags & 4) ? (sprite->tex->height << 5) - t - 32 : t;
    gSPScisTextureRectangle(g++, x, y, xh, yh, G_TX_RENDERTILE, s2, t2, dsdx, dtdy);
    *gdl = g;
}

void UpdateBackSprites(void) {
    f32 mtxF[4][4];
    Mtx mtx;
    Vec3 dir;
    Vec3 tilt;
    Backdrop *backdrop;
    f32 x;
    f32 y;
    f32 centreY;
    s32 width;
    s32 height;

    guMtxCatL(&dynamicp->cameraView, &dynamicp->projection, &mtx);
    guMtxL2F(mtxF, &mtx);
    for (backdrop = backdropList.head.next; backdrop != &backdropList.head; backdrop = backdrop->next) {
        if (backdrop->wrap) {
            SUB_VECTORS(&dir, &cameraInfo.actor.pos, &camera.pos);
            MakeUnit(&dir);
            width = (backdrop->tex->width * backdrop->scaleX) >> 9;
            height = (backdrop->tex->height * backdrop->scaleY) >> 9;
            x = -(atan2(dir.z, dir.x) * backdrop->wrap * width) / PI;
            backdrop->scrollX = x + width * backdrop->wrap;
            while (x < -(width << 1)) {
                x += width * 2;
            }
            while (x > width * 2) {
                x -= width * 2;
            }
            tilt.x = 0.0f;
            tilt.z = 500.0f;
            RotateVector2D(&tilt, &tilt, camera.rot.x);
            centreY = (tilt.x + backdrop->yBias) / 2.0f + 136.0f;
            if (centreY < height + backbodge) {
                y = centreY;
            } else {
                y = height + backbodge;
            }
        } else if (backdrop->parent == NULL) {
            width = (backdrop->tex->width * backdrop->scaleX) >> 9;
            x = width;
            height = (backdrop->tex->height * backdrop->scaleY) >> 9;
            y = height;
        } else {
            width = (backdrop->parent->tex->width * backdrop->parent->scaleX) >> 9;
            width *= backdrop->parent->wrap;
            x = backdrop->parent->scrollX + backdrop->offsetX * 4.0f;
            if (x > width) {
                x -= width * 2;
            }
            width = (backdrop->tex->width * backdrop->scaleX) >> 9;
            if (x - width > 1280.0f || x + width < 0.0f) {
                backdrop->visible = 0;
                continue;
            }
            y = backdrop->parent->y + backdrop->offsetY * 4.0f;
            height = (backdrop->tex->height * backdrop->scaleY) >> 9;
            if (y - height > 960.0f || y + height < 0.0f) {
                backdrop->visible = 0;
                continue;
            }
        }
        backdrop->visible = 1;
        backdrop->x = (s32)x - width;
        backdrop->y = (s32)y - height;
    }
}

void PrintFadeOverlay(void) {
    u8 oldFogMode = fog.on;

    fog.on = FALSE;
    InitDisplayLists();
    redrawActors = TRUE;
    SetupViewing();
    ApplyGlobalTransformations();
    ClearViewing();
    if (vtxPtr) {
        DrawActor(&fadeOverlay);
        if (fieldOfView > 70) {
            starOverlay.pos.z = gameInfo.starZ > 150 ? 150.0f : gameInfo.starZ;
        } else {
            starOverlay.pos.z = gameInfo.starZ;
        }
        DrawActor(&starOverlay);
        starOverlay.rot.z += 0.12;
        GetQuaternionFromXYZRot(&starOverlay.rotQuat, &starOverlay.rot);
    }
    fog.on = oldFogMode;
}

void FadeScreenOut(void) {
    SetFadeColour(0, 0, 0);
    gameInfo.fade = 0;
    gameInfo.fade2 = 300;
    gameInfo.starZ = 0;
    if (gameInfo.mode == 4 || gameInfo.mode == 1) {
        gameInfo.starZTarget = 600;
    } else {
        gameInfo.starZTarget = 0;
    }
}

void FadeScreenIn(void) {
    SetFadeColour(0, 0, 0);
    gameInfo.fade = 300;
    gameInfo.fade2 = 0;
    gameInfo.starZTarget = 0;
    if (gameInfo.mode == 4 || gameInfo.mode == 1 || gameInfo.mode == 9 || gameInfo.mode == 7) {
        gameInfo.starZ = 600;
    } else {
        gameInfo.starZ = 0;
    }
}

s32 ScreenFadedOut(void) {
    if (gameInfo.fade >= 276 && gameInfo.fade2 >= 276) {
        return TRUE;
    }
    return FALSE;
}

s32 ScreenFadedIn(void) {
    return D_801E753C == 0;
}

void FlashScreen(u8 r, u8 g, u8 b, s16 fadeVal, s16 fadeStep) {
    SetFadeColour(r, g, b);
    gameInfo.fade = fadeVal;
    gameInfo.fadeStep = fadeStep;
}

#define BUTTON_COLOUR(o, k)                                                                                  \
    do {                                                                                                     \
        u8 c;                                                                                                \
        if (pad[ActiveController].button[k] || rcdelay >= pad[ActiveController].sincePress[k]) {             \
            c = 255;                                                                                         \
        } else {                                                                                             \
            c = 100;                                                                                         \
        }                                                                                                    \
        controllerOL[o].r = controllerOL[o].g = controllerOL[o].b = c;                                             \
    } while (0)

void PrintStats(void) {
    s16 xOffset[4]; // the frame only fits four, although up to five are written
    s32 x;
    s32 j;
    s32 num;
    s32 xpos;
    s32 ypos;
    s32 buby;
    s32 bubx;
    s32 bubHeight;
    Overlay *ol;
    Stat *stat;
    u8 r, g, b;

    if (frameCount <= 0) {
        return;
    }
    SetFont(0);
    if (printController) {
        PrintOverlay(&controllerOL[0]);
        if (pad[ActiveController].stickX == 0 && abs(controllerOL[9].x - controllerOL[0].x - rcxoff) < 2) {
            controllerOL[9].x = controllerOL[0].x + rcxoff;
        } else {
            controllerOL[9].x += (controllerOL[0].x + rcxoff + pad[ActiveController].stickX / rcfac - controllerOL[9].x) / 2.0;
        }
        if (pad[ActiveController].stickY == 0 && abs(controllerOL[9].y - controllerOL[0].y - rcyoff) < 2) {
            controllerOL[9].y = controllerOL[0].y + rcyoff;
        } else {
            controllerOL[9].y += (controllerOL[0].y + rcyoff - pad[ActiveController].stickY / rcfac - controllerOL[9].y) / 2.0;
        }
        for (x = 0; x < 11; x++) {
            PrintOverlay(&controllerOL[x]);
        }
        BUTTON_COLOUR(2, 0);
        BUTTON_COLOUR(3, 1);
        BUTTON_COLOUR(4, 2);
        BUTTON_COLOUR(5, 12);
        BUTTON_COLOUR(6, 13);
        BUTTON_COLOUR(7, 10);
        BUTTON_COLOUR(8, 11);
        BUTTON_COLOUR(10, 5);
        return;
    }
    for (x = 0; x < MAX_STATS; x++) {
        stat = &stats[x];
        if (stat->scaleTarget != stat->scale) {
            stat->scale += (stat->scaleTarget - stat->scale) / 2;
        }
        if (stat->alphaTarget != stat->alpha) {
            stat->alpha += (stat->alphaTarget - stat->alpha) / 2;
        }
        if (stat->text != NULL) {
            stat->text->a = stat->alpha;
        }
    }
    sprintf(stats[SCORE_STATS].text->str, "%8d", player.score);
    for (x = 0; x < 3; x++) {
        ol = &health2OL;
        if (x < player.health) {
            ol = &health1OL;
        }
        ol->scaleX = ol->scaleY = stats[HEALTH_STATS].scale;
        ol->alpha = stats[HEALTH_STATS].alpha;
        ol->x = x * 16 + 32;
        PrintOverlay(ol);
    }
    numLivesOL.y = 210.0f;
    numLivesOL.scaleX = numLivesOL.scaleY = stats[LIVES_STATS].scale;
    numLivesOL.alpha = stats[LIVES_STATS].alpha;
    numLivesOL.x = 280.0f;
    PrintOverlay(&numLivesOL);
    for (x = 0; x < 4; x++) {
        stats[LIVES_STATS].text->str[x] = 0;
    }
    sprintf(stats[LIVES_STATS].text->str, "%2d", MAX(player.lives, 0));
    stats[LIVES_STATS].text->str[strlen(stats[LIVES_STATS].text->str)] = '*';
    stats[LIVES_STATS].text->str[strlen(stats[LIVES_STATS].text->str) + 1] = 0;
    if (gameInfo.level == ATLANTISBOSS || gameInfo.level == CARNIVALBOSS || gameInfo.level == PIRATESBOSS || gameInfo.level == SPACEBOSS3
        || gameInfo.level == PREHISTORICBOSS) {
        for (x = 0; x < enemies[0].actor.state->hits; x++) {
            bossHealthOL[x].x = x * 32 + 44;
            PrintOverlay(&bossHealthOL[x]);
        }
        if (enemies[0].actor.state->hitFadeTimer) {
            bossHealthOL[x].scaleY = bossHealthOL[x].scaleX = (20 - enemies[0].actor.state->hitFadeTimer) * 128 + 1024;
            bossHealthOL[x].alpha = enemies[0].actor.state->hitFadeTimer * 255 / 20;
            bossHealthOL[x].x = x * 32 + 64;
            if (enemies[0].actor.state->hitFadeTimer >= 20) {
                r = bossHealthOL[x].r;
                g = bossHealthOL[x].g;
                b = bossHealthOL[x].b;
                bossHealthOL[x].r = 255;
                bossHealthOL[x].g = 255;
                bossHealthOL[x].b = 255;
            }
            PrintOverlay(&bossHealthOL[x]);
            if (enemies[0].actor.state->hitFadeTimer >= 20) {
                bossHealthOL[x].r = r;
                bossHealthOL[x].g = g;
                bossHealthOL[x].b = b;
            }
        }
        return;
    }
    if (numGaribs) {
        garibsGotOL.alpha = stats[GARIB_STATS].alpha;
        garibsGotOL.scaleX = garibsGotOL.scaleY = stats[GARIB_STATS].scale;
        PrintOverlay(&garibsGotOL);
        if (player.garibs >= numGaribs && numGaribs > 0) {
            stats[GARIB_STATS].text->b = 128;
            stats[GARIB_STATS].text->r = stats[GARIB_STATS].text->g = SineWave(3.0f, frameCount, 0) * 64.0f + 127.0f + 64.0f;
        } else {
            stats[GARIB_STATS].text->b = 0;
            stats[GARIB_STATS].text->r = stats[GARIB_STATS].text->g = 255;
        }
        sprintf(stats[GARIB_STATS].text->str, "%d/%d", player.garibs, numGaribs);
    }
    gDPPipeSync(glistp++);
    gDPSetRenderMode(glistp++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
    gDPPipeSync(glistp++);
    gDPSetCycleType(glistp++, G_CYC_FILL);
    for (x = 0, num = 0; x < 15; x++) {
        if (player.powerups[x] >= 3 && (!ballControl.transformState || x < 7 || x >= 13)) {
            gDPSetFillColor(glistp++, GPACK_RGBA5551(spellColour[x + 1][0][0], spellColour[x + 1][0][1], spellColour[x + 1][0][2], 1) << 16
                                          | GPACK_RGBA5551(spellColour[x + 1][0][0], spellColour[x + 1][0][1], spellColour[x + 1][0][2], 1));
            if (player.powerups[x] < 10) {
                xOffset[num] = (player.powerups[x] - 10) * 3;
                if (num & 1) {
                    xOffset[num] = -xOffset[num];
                }
            } else {
                xOffset[num] = 0;
            }
            xpos = (num % 2) * 260 + pickupX + xOffset[num];
            ypos = (num / 2) * pickupSep + pickupY;
            gDPFillRectangle(glistp++, xpos, ypos - MIN(player.powerups[x] / 18, 56), xpos + 8, ypos);
            if (player.powerups[x] >= 10) {
                for (j = 0; j < numPickBubs; j++) {
                    if (j < numPickBubs / 2) {
                        bubx = xpos + Random(8) + xOffset[num];
                        bubHeight = MIN(player.powerups[x] / 18, 56) + 1;
                        buby = ypos - bubHeight;
                        gDPFillRectangle(glistp++, bubx, buby, bubx + 1, buby + 1);
                    } else {
                        bubx = xpos + Random(8) + xOffset[num];
                        if (Random(2) == 1) {
                            bubHeight = MIN(player.powerups[x] / 18, 56) + 3;
                            buby = ypos - bubHeight;
                            gDPFillRectangle(glistp++, bubx, buby, bubx, buby);
                        } else {
                            bubHeight = MIN(player.powerups[x] / 18, 56) + 2;
                            buby = ypos - bubHeight;
                            gDPFillRectangle(glistp++, bubx, buby, bubx + 1, buby + 1);
                        }
                    }
                }
            }
            gDPPipeSync(glistp++);
            num++;
            if (num == 5) {
                break;
            }
        }
    }
    gDPPipeSync(glistp++);
    gSPDisplayList(glistp++, rspInitForOverlays_dl);
    gSPDisplayList(glistp++, rdpInitForOverlays_dl);
    for (x = 0, num = 0; x < 15; x++) {
        if (player.powerups[x] >= 3 && (!ballControl.transformState || x < 7 || x >= 13)) {
            pickupTimerOL[num].x = (num % 2) * 260 + 34 + xOffset[num];
            PrintOverlay(&pickupTimerOL[num]);
            num++;
            if (num == 5) {
                break;
            }
        }
    }
}

void InitScreenDebris(void) {
    ScreenDebris *debris;
    s32 i;

    for (i = 0; i < 40; i++) {
        screenDebris[i].active = 0;
        screenDebris[i].lifeTimer = 0;
        debris = &screenDebris[i];
        debris->scaleX = debris->scaleY = 0x400;
        debris->x = 0.0f;
        debris->y = 0.0f;
        debris->depth = 1;
        debris->alpha = 255;
        debris->next = NULL;
    }
    FindTexture(&starScreenDebris, UpdateCRC("sspark1.bmp"), 0);
    FindTexture(&lifeScreenDebris, UpdateCRC("numlives01.bmp"), 0);
    FindTexture(&rainScreenDebris, UpdateCRC("raindrop.bmp"), 0);
    FindTexture(&snowScreenDebris, UpdateCRC("ai_snow.bmp"), 0);
    FindTexture(&tickTexture, UpdateCRC("yes.bmp"), 0);
    FindTexture(&crossTexture, UpdateCRC("no.bmp"), 0);
}

void AddStarBurst(s32 xPos, s32 yPos, s32 numStars, s32 type) {
    Vec3 v;
    f32 angleStep;
    f32 angle;
    s32 i;

    angleStep = 6.28 / numStars;
    angle = Random(6);
    angle /= 3.0f;
    for (i = 0; i < numStars; i++) {
        v.z = 3.0f;
        v.x = 0.0f;
        RotateVector2D(&v, &v, angle);
        if (!type) {
            v.z += 1.2;
        }
        AddScreenDebris(xPos, yPos, v.x, v.z, type, 0);
        angle += angleStep;
    }
}

ScreenDebris *AddScreenDebris(s32 xPos, s32 yPos, f32 xVel, f32 yVel, s32 type, s32 mustCreate) {
    ScreenDebris *debris;
    s32 x;

    for (x = 0; x < 40; x++) {
        if (!screenDebris[x].active) {
            break;
        }
    }
    if (x == 39) {
        if (!mustCreate) {
            return NULL;
        }
        x = type;
    }
    debris = &screenDebris[x];
    debris->scaleX = debris->scaleY = 0x400;
    if (type == SDT_STAR || type == SDT_STAR2) {
        debris->curScale = 0x400;
        debris->targetScale = 0x2000;
        debris->curAlpha = 255;
        debris->fadeTimer = 15;
        debris->lifeTimer = 20;
        debris->targetAlpha = 0;
        debris->flags = 5;
        if (!type) {
            debris->flags |= 2;
        } else {
            debris->flags |= 16;
        }
        debris->tex = starScreenDebris;
    } else if (type == SDT_LIFE) {
        debris->fadeTimer = 15;
        debris->flags = 7;
        debris->tex = lifeScreenDebris;
        debris->curScale = 0x400;
        debris->targetScale = 3000;
        debris->curAlpha = 255;
        debris->targetAlpha = 0;
        debris->lifeTimer = 60;
    } else if (type == SDT_RAIN) {
        debris->fadeTimer = 15;
        debris->flags = 0;
        debris->tex = rainScreenDebris;
        debris->curScale = 0x400;
        debris->targetScale = 0x400;
        debris->curAlpha = 100;
        debris->targetAlpha = 100;
        debris->lifeTimer = 60;
    } else if (type == SDT_SNOW) {
        debris->fadeTimer = 15;
        debris->flags = 0;
        debris->tex = snowScreenDebris;
        debris->curScale = 0x400;
        debris->targetScale = 0x400;
        debris->curAlpha = 100;
        debris->targetAlpha = 100;
        debris->lifeTimer = 60;
    } else if (type == SDT_QUESTION_YES || type == SDT_QUESTION_NO) {
        debris->fadeTimer = 0;
        debris->flags = 8;
        debris->tex = tickTexture;
        debris->curScale = 0x400;
        debris->targetScale = 0x400;
        debris->curAlpha = 255;
        debris->targetAlpha = 255;
        debris->lifeTimer = 2;
        if (type == SDT_QUESTION_NO) {
            debris->tex = crossTexture;
        }
    } else {
        Crash("SCREEN DEBRIS");
    }
    SetupOverlay((Overlay *)debris);
    debris->active = 1;
    debris->x = xPos;
    debris->y = yPos;
    debris->depth = 0;
    debris->velX = xVel;
    debris->velY = yVel;
    debris->age = 0;
    debris->phase = Random(50);
    AddOverlay((Overlay *)debris);
    debris->behind = 0;
    return debris;
}

void HandleScreenDebris(void) {
    s32 i;

    for (i = 0; i < 40; i++) {
        if (screenDebris[i].active) {
            UpdateScreenDebris(&screenDebris[i]);
        }
    }
}

void UpdateScreenDebris(ScreenDebris *debris) {
    Vec3 dir;
    f32 temp;
    s32 size;

    if (debris->tex == snowScreenDebris || debris->tex == rainScreenDebris) {
        dir.z = 0.0f;
        dir.x = 1.0f;
        RotateVector2D(&dir, &dir, camera.rot.y);
        debris->x -= ((dir.x * camera.vel.x + dir.z * camera.vel.z) * debris->curScale * debris->curScale) / snowFac;
        debris->x += (FindShortestAngleSigned(cameraInfo.prevRotY, camera.rot.y) * debris->curScale * debris->curScale) / snowFac2;
        if (debris->x > 360.0f) {
            debris->x -= 400.0f;
        } else if (debris->x < -40.0f) {
            debris->x += 400.0f;
        }
        if (debris->tex == snowScreenDebris) {
            debris->velX = SineWave(10.0f, debris->lifeTimer, 0) * debris->curScale / 256.0;
            RotateVector2D(&dir, &dir, PI / 2);
            temp = dir.x * camera.vel.x + dir.z * camera.vel.z;
            debris->curScale = debris->targetScale = debris->targetScale + temp * snowFac3;
            if (snowMaxScale < debris->targetScale || debris->targetScale < 100) {
                debris->curScale = debris->targetScale = snowMaxScale;
            }
            debris->curAlpha = debris->targetAlpha = (rain.col2 >> 1) + (255 - (rain.col2 >> 1)) * debris->curScale / snowMaxScale;
            debris->velY = rain.velY * 0.25 + rain.velY * debris->curScale / snowMaxScale;
        }
    }
    if (debris->curScale < debris->targetScale) {
        debris->curScale += MIN((debris->targetScale - debris->curScale) / 2, 100);
    } else {
        debris->curScale -= (debris->curScale - debris->targetScale) / 2 < -100 ? -100 : (debris->curScale - debris->targetScale) / 2;
    }
    if (debris->fadeTimer) {
        debris->fadeTimer--;
    } else if (debris->lifeTimer & 1) {
        debris->curAlpha = debris->curAlpha < debris->targetAlpha ? debris->curAlpha + MIN((debris->targetAlpha - debris->curAlpha) / 2, 1)
                                       : debris->curAlpha - ((debris->curAlpha - debris->targetAlpha) / 2 < -1 ? -1 : (debris->curAlpha - debris->targetAlpha) / 2);
    }
    debris->alpha = debris->curAlpha;
    debris->scaleX = debris->scaleY = debris->curScale;
    if (!(debris->flags & 8)) {
        debris->lifeTimer--;
    }
    if (debris->lifeTimer == 0 || ((debris->tex == rainScreenDebris || debris->tex == snowScreenDebris) && debris->y > 240.0f)) {
        debris->active = 0;
        debris->age = 0;
        SubOverlay((Overlay *)debris);
    }
    debris->x += debris->velX;
    debris->y -= debris->velY;
    if (debris->flags & 1) {
        debris->velX *= 0.9;
        if (debris->velY > 0.0f || (debris->flags & 0x10)) {
            debris->velY *= 0.9;
        }
    }
    if (debris->flags & 2) {
        debris->velY -= 0.5;
    }
    if (debris->flags & 4) {
        temp = debris->phase;
        size = debris->scaleX * ABS(__sinf(temp / 2.0f));
        debris->scaleX = size + 1;
    }
    debris->age++;
    debris->phase++;
}

void DestroyScreenDebris(void) {
    s32 i;

    for (i = 0; i < 40; i++) {
        JallocFree(&screenDebris[i].sprite.bitmap);
        JallocFree(&screenDebris[i].bitmaps);
    }
}

INCLUDE_ASM("asm/nonmatchings/overlay", pad_8014F3F4);

// linker junk that follows the 2.0f constant emitted by UpdateScreenDebris
const u32 D_801065F4[] = { 0x06000200, 0x0A044F10, 0x06000200 };
