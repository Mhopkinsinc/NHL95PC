/* Demo / save game: the saved game's name. */
#define SaveGameNameDlg SaveGameNameDlg_decl
#include "nhl95.h"
#undef SaveGameNameDlg

typedef struct { char c[17]; } Hdr17;

/* DRAFT: same size in MenuSaveGame.c (own file: the draft there breaks MenuSaveGame's block); the out-of-row
   click gets its own redraw copy (see docs/UNSURE.md). */
/* SaveGameNameDlg (85D6C, PC only; ends in SaveGameStub, MenuSaveGame's exit): ask for the saved game's name.
   Save the screen under the mouse pointer (a POINTER block, the pointer's width + 8 by its height) and draw the
   pointer, load the SAVEGAME bank and show its PRMT box in the middle of the screen (the screen under it kept in a
   DIALOG block); edit the name there (EditTextField, 8 characters) and add .NHL. While the name is not empty and a
   file of that name exists, show the ERR1 box with the name and follow the mouse: in the button row (y F8h-108h)
   the first button (x DBh-106h) keeps the name and overwrites, the second (115h-147h) cancels, the third
   (15Eh-1A1h) goes back to the PRMT box to edit the name again; an empty name is edited again too. path: the name
   buffer (the name with .NHL on return). Result: 0 to save, 1 when cancelled. */
int SaveGameNameDlg(char *path)
{
    int cur;
    int dlgbm;
    int bank;
    int spr;
    int y;
    int n;      /* the block sizes, and the buttons from sub_B2DCA */
    int ox;
    int oy;
    int x;
    char ft[0x2C];
    char name[0x40];
    int bm;
    int tx;
    int ty;
    short b;    /* short: an int copies the ptrupdatefn result through edx */
    int i;
    unsigned char *ev;

    n = ((short *)pointerspr)[3] * (((short *)pointerspr)[2] + 8) + 0x11;
    bm = sub_8CCA8((char *)str_Pointer22, n, 0x20);
    *(Hdr17 *)bm = *(Hdr17 *)pointerspr;
    sub_B4BA8();
    sub_B2DCA(&n, &ox, &oy);
    x = ox;
    y = oy;
    sub_91400(bm, ox, oy);
    sub_91284(pointerspr, ox, oy);
    MakePath(name, fileoncd[0x163] == 1 ? (char *)cddriveptr : 0, (char *)str_Savegame, 0);
    bank = sub_8E83C(name, 0);
    cur = spr = sub_B30B4(bank, (char *)str_Prmt);
    n = (((short *)spr)[2] + 8) * ((short *)spr)[3] + 0x11;
    dlgbm = sub_8CCA8((char *)str_Dialog, n, 0x20);
    *(Hdr17 *)dlgbm = *(Hdr17 *)spr;
    tx = 0x140 - ((short *)spr)[2] / 2;
    ty = 0xF0 - ((short *)spr)[3] / 2;
    sub_91400(dlgbm, tx, ty);
    sub_910E0(spr, tx, ty);
    sub_8E9C0(0xFA, 0xF8);
    tx += ((short *)spr)[4];
    ty += ((short *)spr)[5];
    *path = 0;
    EditTextField(path, 8, 0x64, tx, ty, 0, 0, 1);
    strcat(path, (char *)str_Nhl3);
    sub_910E0(bm, ox, oy);
    while (*path == '.' || unknown_libname_1(path, 0, ft) == 0) {
        if (*path != '.') {
            tx = 0x140 - ((short *)cur)[2] / 2;
            ty = 0xF0 - ((short *)cur)[3] / 2;
            sub_910E0(dlgbm, tx, ty);
            FreeMemBlock(dlgbm);
            cur = spr = sub_B30B4(bank, (char *)str_Err1);
            n = (((short *)spr)[2] + 8) * ((short *)spr)[3] + 0x11;
            dlgbm = sub_8CCA8((char *)str_Dialog, n, 0x20);
            *(Hdr17 *)dlgbm = *(Hdr17 *)spr;
            tx = 0x140 - ((short *)spr)[2] / 2;
            ty = 0xF0 - ((short *)spr)[3] / 2;
            sub_91400(dlgbm, tx, ty);
            sub_91284(spr, tx, ty);
            tx += ((short *)spr)[4];
            ty += ((short *)spr)[5];
            for (i = 0; path[i] != '.'; i++);
            path[i] = 0;
            sub_8E9C0(0xFA, 0xF9);
            sub_92CD0(path, tx, ty);
            ClearInputQueue();
            MouseSetPos(0x181, 0xFE);
            sub_91400(bm, 0x181, 0xFE);
            sub_91284(pointerspr, 0x181, 0xFE);
            ox = x = 0x181;
            oy = y = 0xFE;
            ClearInputQueue();
            for (;;) {
                b = 0;
                for (;;) {
                    ev = GetInputEvent();
                    if (ev == 0) break;
                    b = ((int (*)(unsigned char *, int *, int *))ptrupdatefn)(ev, &x, &y);
                    if (b & 2) break;
                }
                if (!(b & 2)) {
                    if (x != ox || y != oy) {
                        sub_903F0(bm, ox, oy);
                        sub_91400(bm, x, y);
                        sub_91370(pointerspr, x, y);
                        ox = x;
                        oy = y;
                    }
                } else if (y >= 0x109 || y < 0xF8) {
                    sub_903F0(bm, ox, oy);
                    sub_91400(bm, x, y);
                    sub_91370(pointerspr, x, y);
                    ox = x;
                    oy = y;
                } else if (x > 0xDA && x < 0x107) {
                    sub_903F0(bm, ox, oy);
                    tx = 0x140 - ((short *)cur)[2] / 2;
                    ty = 0xF0 - ((short *)cur)[3] / 2;
                    sub_910E0(dlgbm, tx, ty);
                    FreeMemBlock(dlgbm);
                    FreeMemBlock(bm);
                    FreeMemBlock(bank);
                    strcat(path, (char *)str_Nhl3);
                    return 0;
                } else if (x > 0x114 && x < 0x148) {
                    sub_903F0(bm, ox, oy);
                    tx = 0x140 - ((short *)cur)[2] / 2;
                    ty = 0xF0 - ((short *)cur)[3] / 2;
                    sub_910E0(dlgbm, tx, ty);
                    FreeMemBlock(dlgbm);
                    FreeMemBlock(bm);
                    FreeMemBlock(bank);
                    return 1;
                } else if (x > 0x15D && x < 0x1A2) {
                    sub_903F0(bm, ox, oy);
                    tx = 0x140 - ((short *)cur)[2] / 2;
                    ty = 0xF0 - ((short *)cur)[3] / 2;
                    sub_910E0(dlgbm, tx, ty);
                    FreeMemBlock(dlgbm);
                    cur = spr = sub_B30B4(bank, (char *)str_Prmt);
                    n = (((short *)spr)[2] + 8) * ((short *)spr)[3] + 0x11;
                    dlgbm = sub_8CCA8((char *)str_Dialog, n, 0x20);
                    *(Hdr17 *)dlgbm = *(Hdr17 *)spr;
                    tx = 0x140 - ((short *)spr)[2] / 2;
                    ty = 0xF0 - ((short *)spr)[3] / 2;
                    sub_91400(dlgbm, tx, ty);
                    sub_910E0(spr, tx, ty);
                    tx += ((short *)spr)[4];
                    ty += ((short *)spr)[5];
                    sub_91400(bm, ox, oy);
                    break;
                } else {
                    sub_903F0(bm, ox, oy);
                    sub_91400(bm, x, y);
                    sub_91370(pointerspr, x, y);
                    ox = x;
                    oy = y;
                }
            }
        }
        for (i = 0; path[i] != '.' && path[i] != 0; i++);
        path[i] = 0;
        sub_8E9C0(0xFA, 0xF8);
        EditTextField(path, 8, 0x64, tx, ty, 0, 0, 1);
        strcat(path, (char *)str_Nhl3);
    }
    sub_903F0(bm, ox, oy);
    tx = 0x140 - ((short *)cur)[2] / 2;
    ty = 0xF0 - ((short *)cur)[3] / 2;
    sub_910E0(dlgbm, tx, ty);
    FreeMemBlock(dlgbm);
    FreeMemBlock(bm);
    FreeMemBlock(bank);
    return 0;
}
