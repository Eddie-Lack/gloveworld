#ifndef TEXT_H
#define TEXT_H

#include "common.h"

typedef struct Text {
    /* 0x00 */ struct Text *next;
    /* 0x04 */ struct Text *prev;
    /* 0x08 */ char str[0x40];
    /* 0x48 */ s32 x;
    /* 0x4C */ s32 y;
    /* 0x50 */ f32 scaleX;
    /* 0x54 */ f32 scaleY;
    /* 0x58 */ u8 r;
    /* 0x59 */ u8 g;
    /* 0x5A */ u8 b;
    /* 0x5B */ u8 a;
    /* 0x5C */ s32 oneShot;
    /* 0x60 */ struct Font *font;
} Text; /* size = 0x64 */

typedef struct TextList {
    /* 0x00 */ Text head;
    /* 0x64 */ s32 count;
} TextList;

extern TextList textList;
extern Text *globText;

void AddText(Text *text, s32 centre);
void mysprintf(char *dst, char *src);

void LoadFont(void);
void InitTextList(void);
s16 GetStringPixelLength(char *str, f32 sx, f32 sy);
void ClearText(void);
void ClearAllText(void);
void ShowText(void);
char * my_strchr(char *s, char c);

#endif
