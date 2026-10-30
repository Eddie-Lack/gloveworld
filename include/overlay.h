#ifndef OVERLAY_H
#define OVERLAY_H

#include "common.h"

typedef struct SpriteFrames {
    /* 0x00 */ void **textures;
    /* 0x04 */ s16 count;
} SpriteFrames;

typedef struct SpriteFrameList {
    /* 0x00 */ void **textures;
    /* 0x04 */ s16 count;
    /* 0x06 */ s16 loaded;
    /* 0x08 */ char name[8];
} SpriteFrameList; /* size = 0x10 */

typedef struct SpriteAnim {
    /* 0x00 */ SpriteFrames *frames;
    /* 0x04 */ u8 frameDelay;
    /* 0x05 */ u8 type;
    /* 0x06 */ u8 startAlpha;
    /* 0x07 */ u8 endAlpha;
    /* 0x08 */ u16 startScale;
    /* 0x0A */ u16 endScale;
    /* 0x0C */ u16 flags;
    /* 0x0E */ u16 life;
} SpriteAnim; /* size = 0x10 */

typedef struct SpriteAnimState {
    /* 0x00 */ SpriteAnim def;
    /* 0x10 */ u16 lifeTimer;
    /* 0x12 */ u8 frame;
    /* 0x13 */ s8 delayTimer;
} SpriteAnimState; /* size = 0x14 */

typedef struct GameSprite {
    /* 0x00 */ struct GameSprite *next;
    /* 0x04 */ struct GameSprite *prev;
    /* 0x08 */ struct Texture *tex;
    /* 0x0C */ Vec3 pos;
    /* 0x18 */ s16 scaleX;
    /* 0x1A */ s16 scaleY;
    /* 0x1C */ u8 unk1C;
    /* 0x1D */ u8 r;
    /* 0x1E */ u8 g;
    /* 0x1F */ u8 b;
    /* 0x20 */ u8 alpha;
    /* 0x21 */ u8 flags;
    /* 0x22 */ s8 offsetX;
    /* 0x23 */ s8 offsetY;
    /* 0x24 */ SpriteAnimState anim;
} GameSprite; /* size = 0x38 */

typedef struct SpriteList {
    /* 0x00 */ GameSprite head;
    /* 0x38 */ s32 count;
} SpriteList;

typedef struct Overlay {
    /* 0x00 */ struct Overlay *next;
    /* 0x04 */ struct Overlay *prev;
    /* 0x08 */ void *tex;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ s16 depth;
    /* 0x16 */ s16 frame;
    /* 0x18 */ s32 scaleX;
    /* 0x1C */ s32 scaleY;
    /* 0x20 */ u8 r;
    /* 0x21 */ u8 g;
    /* 0x22 */ u8 b;
    /* 0x23 */ u8 alpha;
    /* 0x24 */ Bitmap *bitmaps;
    /* 0x28 */ Sprite sprite;
    /* 0x6C */ s32 behind;
    /* 0x70 */ SpriteFrames *frames;
} Overlay; /* size = 0x74 */

typedef struct OverlayList {
    /* 0x00 */ Overlay head;
    /* 0x74 */ s32 count;
} OverlayList;

typedef struct Backdrop {
    /* 0x00 */ struct Backdrop *next;
    /* 0x04 */ struct Backdrop *prev;
    /* 0x08 */ struct Backdrop *parent;
    /* 0x0C */ f32 offsetX;
    /* 0x10 */ f32 offsetY;
    /* 0x14 */ u8 unk14[0x4];
    /* 0x18 */ uSprite sprite[2];
    /* 0x48 */ struct Texture *tex;
    /* 0x4C */ s32 x;
    /* 0x50 */ s32 y;
    /* 0x54 */ s32 depth;
    /* 0x58 */ s16 scaleX;
    /* 0x5A */ s16 scaleY;
    /* 0x5C */ s16 flipX;
    /* 0x5E */ s16 flipY;
    /* 0x60 */ s16 visible;
    /* 0x62 */ s16 wrap;
    /* 0x64 */ s16 unk64;
    /* 0x66 */ s16 yBias;
    /* 0x68 */ s32 scrollX;
    /* 0x6C */ u8 r;
    /* 0x6D */ u8 g;
    /* 0x6E */ u8 b;
    /* 0x6F */ u8 alpha;
} Backdrop; /* size = 0x70 */

typedef struct BackdropList {
    /* 0x00 */ Backdrop head;
    /* 0x70 */ s32 count;
} BackdropList;

typedef struct ScreenDebris {
    /* 0x00 */ struct ScreenDebris *next;
    /* 0x04 */ struct ScreenDebris *prev;
    /* 0x08 */ void *tex;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ s16 depth;
    /* 0x16 */ s16 frame;
    /* 0x18 */ s32 scaleX;
    /* 0x1C */ s32 scaleY;
    /* 0x20 */ u8 r;
    /* 0x21 */ u8 g;
    /* 0x22 */ u8 b;
    /* 0x23 */ u8 alpha;
    /* 0x24 */ Bitmap *bitmaps;
    /* 0x28 */ Sprite sprite;
    /* 0x6C */ s32 behind;
    /* 0x70 */ SpriteFrames *frames;
    /* 0x74 */ f32 velX;
    /* 0x78 */ f32 velY;
    /* 0x7C */ u8 unk7C[0x4];
    /* 0x80 */ s16 curAlpha;
    /* 0x82 */ s16 targetAlpha;
    /* 0x84 */ s16 curScale;
    /* 0x86 */ s16 targetScale;
    /* 0x88 */ s16 active;
    /* 0x8A */ s16 lifeTimer;
    /* 0x8C */ u8 age;
    /* 0x8D */ u8 flags;
    /* 0x8E */ u8 fadeTimer;
    /* 0x8F */ u8 phase;
} ScreenDebris; /* size = 0x90 */

typedef struct Stat {
    /* 0x00 */ s32 scale;
    /* 0x04 */ s32 scaleTarget;
    /* 0x08 */ s32 alpha;
    /* 0x0C */ s32 alphaTarget;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ struct Text *text;
} Stat; /* size = 0x18 */

extern SpriteList spriteList;
extern Stat stats[MAX_STATS];
extern SpriteFrameList spriteFrameList[NUM_SPRITE_ANIMS];
extern OverlayList overlayList;
extern BackdropList backdropList;

void SubOverlay(Overlay *overlay);
void FindSetupAndAddOverlay(Overlay *overlay, char *name, s32 xPos, s32 yPos, s32 zPos, s32 attr);
void SetFadeColour(u8 r, u8 g, u8 b);

void FindAndSetupOverlay(Overlay *overlay, char *name, s32 xPos, s32 yPos, s32 zPos, s32 attr);
void SetupOverlay(Overlay *overlay);
void SetupOverlayData(Overlay *overlay);
void InitOverlayList(void);
void InitBackdropList(void);
void AddBackdrop(Backdrop *backdrop);
void AddOverlay(Overlay *overlay);
void InitSpriteList(void);
void AddSprite(GameSprite *sprite, GameSprite *after);
void SubSprite(GameSprite *sprite);
void PrintOverlays(void);
void PrintOverlaysBehind(void);
void PrintOverlay(Overlay *overlay);
void PrintBackdrops(void);
void PrintBackdrop(Backdrop *backdrop);
void AnimateSprite(GameSprite *sprite);
GameSprite * PrintSpritesOpaque(void);
void PrintSpritesTranslucent(GameSprite *sprite);
void PrintSprite(GameSprite *sprite);
void TileRectangle(Gfx **gdl, GameSprite *sprite, f32 x, f32 y, s32 z, s32 sx, s32 sy);
void UpdateBackSprites(void);
void PrintFadeOverlay(void);
void FadeScreenOut(void);
void FadeScreenIn(void);
s32 ScreenFadedOut(void);
void PrintStats(void);
void InitScreenDebris(void);
void AddStarBurst(s32 xPos, s32 yPos, s32 numStars, s32 type);
void HandleScreenDebris(void);
void UpdateScreenDebris(ScreenDebris *debris);
void DestroyScreenDebris(void);

#endif
