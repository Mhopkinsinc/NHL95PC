/* Settings dialogs: game mode settings menu. */
#define ModeSetViewLoop ModeSetViewLoop_decl
#include "nhl95.h"
#undef ModeSetViewLoop

typedef struct Hdr17 { char c[17]; } Hdr17;

short ModeSetViewLoop(void);  /* 7B846 */

/* MenuModeSettings: menu "game settings": save the screen under the 214 x 257 dialog at 10 13h into a
 BKGD bitmap (pointer sprite header), draw it and run it (view only in game mode 2, else edit). With line changes off,
 clear the line change blink box state, every player's pflags2 bit 3 (scpflags2, stride 80h) and both teams' bit 1, and
 reset the 1Ch energy words of both teams to 1000h. Then music (bit 6) and sound effects (bit 7) on or off, restore
 the screen and free the bitmap and the settings file. Returns 0. */
int MenuModeSettings(void)
{
    int bm;
    short i;

    sub_B4BA8();
    LoadSettingsShapes();
    bm = sub_8CCA8((char *)str_BKGD2, 0xE935, 0x20);
    *(Hdr17 *)bm = *(Hdr17 *)pointerspr;
    ((short *)bm)[2] = 0xD6;
    ((short *)bm)[3] = 0x101;
    sub_91400(bm, 0xA, 0x13);
    DrawModeSetDlg();
    ModeOptsToBits();
    if (gamemode == 2) ModeSetViewLoop();
    else ModeSetEditLoop();
    sub_903F0(bm, 0xA, 0x13);
    if (!gameopts.linechanges) {
        lcboxon[0] = lcboxon[1] = lcblink[0] = lcblink[1] = 0;
        for (i = 0; i < 0xC; i++) scpflags2[i * 0x80] &= ~8;
        hmtmflags[0] &= ~2;
        awtmflags &= ~2;
        for (i = 0; i < 0x1C; i++) {
            awtmpde[i] = 0x1000;
            hmtmpde[i] = 0x1000;
        }
    }
    if (gameopts.music) sub_8F979();
    else sub_8F984();
    if (gameopts.sfx) sub_8F963();
    else sub_8F96E();
    FreeMemBlock(bm);
    FreeMemBlock(settingsfile);
    settingsfile = 0;
    return 0;
}

/* ModeSetViewLoop (7B846, PC only; ends in ModeSettings_epilogue): the view-only game settings dialog (game mode
   2). Set four dialog colours (F7h), read the mouse, save the screen under the pointer into a POINTER block with
   the pointer sprite's header (one pixel larger each way) and draw the pointer. Then follow the mouse through
   ptrupdatefn until any button: when the pointer moved, put the old screen back, save the new one and draw the
   pointer again. On a button put the screen back and free the block. Result: 0. The original returns a short:
   with an int result Watcom copies the button bits through edx after the ptrupdatefn call (docs/C_MATCHING.md). */
short ModeSetViewLoop(void)
{
    int ox;
    int oy;
    int x;
    int y;
    int btn;
    int bm;
    int n;
    unsigned char *ev;

    sub_B4B88(0xF7, 4, unk_D16A0);
    sub_B2DCA(&btn, &ox, &oy);
    x = ox;
    y = oy;
    sub_B4BA8();
    n = (((short *)pointerspr)[2] + 1) * (((short *)pointerspr)[3] + 1) + 0x11;
    bm = sub_8CCA8((char *)str_Pointer16, n, 0x20);
    *(Hdr17 *)bm = *(Hdr17 *)pointerspr;
    ((short *)bm)[2] = ((short *)pointerspr)[2] + 1;
    ((short *)bm)[3] = ((short *)pointerspr)[3] + 1;
    sub_91400(bm, x - 4, y);
    sub_91370(pointerspr, x - 4, y);
    ClearInputQueue();
    for (;;) {
        btn = 0;
        for (;;) {
            ev = GetInputEvent();
            if (ev == 0) break;
            btn = ((int (*)(unsigned char *, int *, int *))ptrupdatefn)(ev, &x, &y);
            if (btn & 6) break;
        }
        if (btn != 0) break;
        if (x == ox && y == oy) continue;
        sub_903F0(bm, ox - 4, oy);
        sub_91400(bm, x - 4, y);
        sub_91370(pointerspr, x - 4, y);
        ox = x;
        oy = y;
    }
    sub_903F0(bm, ox - 4, oy);
    FreeMemBlock(bm);
    return 0;
}
