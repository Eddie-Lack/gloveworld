#include "common.h"
#include "text.h"
#include "font.h"
#include "jalloc.h"

extern Sprite template_sprite;
extern s32 fontmagicnum;
extern u8 draw_buffer;
extern Gfx overlay_dl[];
extern Gfx overlay_dl2[];
extern Gfx *overlay_dl_next;

void text_sprite(Sprite *txt, char *str, Font *fnt, s32 xlen, s32 ylen);
extern u8 font_col[4];
extern s32 font_xpos;
extern s32 font_ypos;
extern s32 font_win_w;
extern s32 font_win_h;
extern f64 font_xscale;
extern f64 font_yscale;
extern f32 globalFontScale;
extern char D_8010614C[]; // "Text" followed by linker junk

void font_init(Gfx **glistp) {
    Gfx *gp;

    gp = *glistp;
    spInit(&gp);
    template_sprite.rsp_dl_next = template_sprite.rsp_dl;
    *glistp = gp;
    font_col[0] = 255;
    font_col[1] = 255;
    font_col[2] = 255;
    font_col[3] = 255;
    font_xpos = 0;
    font_ypos = 0;
    font_win_w = 40;
    font_win_h = 1;
    font_xscale = 1.0;
    font_yscale = 1.0;
}

void font_finish(Gfx **glistp) {
    Gfx *gp;

    gp = *glistp;
    spFinish(&gp);
    *glistp = gp - 1;
}

void font_set_win(s32 w, s32 h) {
    font_win_w = w;
    font_win_h = h;
}

void font_set_pos(s32 px, s32 py) {
    font_xpos = px;
    font_ypos = py;
}

void font_set_scale(f64 sx, f64 sy) {
    font_xscale = sx;
    font_yscale = sy;
}

void font_set_color(u8 rr, u8 gg, u8 bb, u8 aa) {
    font_col[0] = rr;
    font_col[1] = gg;
    font_col[2] = bb;
    font_col[3] = aa;
}

void font_set_transparent(s32 flag) {
    if (flag) {
        spSetAttribute(&template_sprite, SP_TRANSPARENT | SP_CUTOUT);
    } else {
        spClearAttribute(&template_sprite, SP_TRANSPARENT | SP_CUTOUT);
    }
}

void font_show_string(Gfx **glistp, char *str, Font *font) {
    Sprite *sp;
    Gfx *gxp;

    fontmagicnum = strlen(str);
    if (draw_buffer) {
        if (&overlay_dl[NUM_DL(180 - fontmagicnum)] < overlay_dl_next) {
            return;
        }
    } else {
        if (&overlay_dl2[NUM_DL(180 - fontmagicnum)] < overlay_dl_next) {
            return;
        }
    }
    gxp = *glistp;
    sp = &template_sprite;
    sp->LUT = font->tex->palette;
    sp->width = font_win_w * 8 + 8;
    sp->height = font_win_h * 16 + 8;
    text_sprite(sp, str, font, font_win_w, font_win_h);
    sp->expx = font->spacing;
    sp->rsp_dl_next = overlay_dl_next;
    spMove(sp, font_xpos, font_ypos);
    spColor(sp, font_col[0], font_col[1], font_col[2], font_col[3]);
    spScale(sp, font_xscale, font_yscale);
    gSPDisplayList(gxp++, spDraw(sp));
    overlay_dl_next = sp->rsp_dl_next;
    *glistp = gxp;
    sp->expx = 0;
}

void mysprintf(char *dst, char *src) {
    s32 i;

    for (i = 0; src[i] != '\0'; i++) {
        if (src[i] >= 'a' && src[i] <= 'z') {
            dst[i] = src[i] - ('a' - 'A');
        } else {
            dst[i] = src[i];
        }
    }
    dst[i] = '\0';
}

Text *NEW_TEXT(s16 x, s16 y, u8 r, u8 g, u8 b, u8 a, f32 sx, f32 sy, u8 oneShot, char *str, u8 centre) {
    globText = JallocAlloc(sizeof(Text), 0, D_8010614C);
    globText->x = x;
    globText->y = y;
    globText->r = r;
    globText->g = g;
    globText->b = b;
    globText->a = a;
    globText->scaleX = sx * globalFontScale;
    globText->scaleY = sy * globalFontScale;
    globText->oneShot = oneShot;
    mysprintf(globText->str, str);
    AddText(globText, centre);
    return globText;
}

void KanafyNumberString(char *str) {
    u8 i;
    u8 len;

    len = strlen(str);
    for (i = 0; i < len; i++) {
        if (str[i] >= '1' && str[i] <= '9') {
            str[i] += 0x90;
        }
        if (str[i] == ' ') {
            str[i] = 0xE4;
        }
        if (str[i] == '0') {
            str[i] = 0xCA;
        }
    }
}

void KanafyLetterString(char *str) {
    u8 i;
    u8 len;

    len = strlen(str);
    for (i = 0; i < len; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] += 0x60;
        } else if (str[i] >= '1' && str[i] <= '9') {
            str[i] += 0x90;
        } else if (str[i] == ' ') {
            str[i] = 0xE4;
        } else if (str[i] == '0') {
            str[i] = 0xCA;
        } else if (str[i] == '/') {
            str[i] = 0xC0;
        } else if (str[i] == ':') {
            str[i] = 0xD2;
        } else if (str[i] == '-') {
            str[i] = 0xBE;
        } else if (str[i] == '[') {
            str[i] = 0xCF;
        } else if (str[i] == ']') {
            str[i] = 0xD0;
        } else if (str[i] == '!') {
            str[i] = 0xBB;
        } else if (str[i] == '.') {
            str[i] = 0xCD;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/font", pad_80146FF4);

INCLUDE_RODATA("asm/nonmatchings/font", D_8010614C);
