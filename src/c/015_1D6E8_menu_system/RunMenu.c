/* Menu system: the menu bar loops. */
#define RunMenu RunMenu_decl
#define MenuHitTest MenuHitTest_decl
#include "nhl95.h"
#undef RunMenu
#undef MenuHitTest

typedef struct MenuItem {
    int x1, y1, x2, y2;
    char *text;
    int on1, on2;
    int pad;
} MenuItem;

typedef struct { char c[17]; } MenuHdr17;

int MenuHitTest(int x, int y, MenuItem **menus, int i, int *counts, int *origin, int *hitMenu, int *hitItem);
int RunMenu(MenuItem *bar, int n, int col1, int col2, int col3);

/* DRAFT: 2,024 vs 2,075 bytes, 305 lines (the EXE keeps the pointer block and position in registers early on;
   see docs/UNSURE.md). */
/* RunMenu (1D6E8, PC only; owner of RunMenu_ret4): run a menu bar of n items in the colours col1-col3 until an
   action returns 1. Save the screen under the pointer (POINTERM block, the pointer bitmap's size plus one) and
   centre the pointer on the bar. Then follow the mouse: a click on an item with an action runs it on the second
   click of the selected item (dropdowns closed first; the menu state reset after it); another item of an open
   menu moves the selection; an item with a submenu opens its dropdown (the screen under it saved in a MENUBUFF
   block); a click outside closes the dropdowns. bar: the items; n: their number; col1-col3: the item colours.
   Result: 0. */
int RunMenu(MenuItem *bar, int n, int col1, int col2, int col3)
{
    struct {
        int pos[4][2];
        int sel[4];
        MenuItem *menus[4];
        int bg[4];
        int nitems[4];
        int item;
        int level;
        int my;
        int mx;
        int h;
        int w;
        int at;
        int ptr;
        int oldmx;
        int oldmy;
        int depth;
    } s;
    int i;
    int bits;
    int r;
    unsigned char *e;
    MenuItem *m;

    s.depth = 0;
    s.menus[0] = bar;
    s.nitems[0] = n;
    s.menus[1] = s.menus[2] = s.menus[3] = 0;
    s.bg[0] = s.bg[1] = s.bg[2] = s.bg[3] = 0;
    s.sel[0] = s.sel[1] = s.sel[2] = s.sel[3] = 0;
    s.pos[0][0] = s.pos[0][1] = 0;
    s.ptr = sub_8CCA8((char *)str_PointerMenu, (((short *)pointerspr)[2] + 1) * (((short *)pointerspr)[3] + 1) + 0x11,
        0x20);
    *(MenuHdr17 *)s.ptr = *(MenuHdr17 *)pointerspr;
    ((short *)s.ptr)[2] = ((short *)pointerspr)[2] + 1;
    ((short *)s.ptr)[3] = ((short *)pointerspr)[3] + 1;
    s.mx = s.oldmx = (s.menus[0]->x1 + s.menus[0]->x2) / 2;
    s.my = s.oldmy = (s.menus[0]->y1 + s.menus[0]->y2) / 2;
    sub_91400(s.ptr, s.oldmx, s.oldmy);
    sub_91370(pointerspr, s.oldmx, s.oldmy);
    sub_B2E1B(0, 0, 0x280, 0x1E0);
    MouseSetPos(s.oldmx, s.oldmy);
    ((void (*)(void))mousepollfn)();
    ClearInputQueue();
    for (;;) {
        bits = 0;
        while ((e = GetInputEvent()) != 0) {
            bits = ((int (*)(unsigned char *, int *, int *))ptrupdatefn)(e, &s.mx, &s.my);
            if (bits & 2) break;
        }
        if (!(bits & 2)) {
            if (s.mx == s.oldmx && s.my == s.oldmy) continue;
            sub_903F0(s.ptr, s.oldmx, s.oldmy);
            sub_91400(s.ptr, s.mx, s.my);
            sub_91370(pointerspr, s.mx, s.my);
            s.oldmx = s.mx;
            s.oldmy = s.my;
            continue;
        }
        if (MenuHitTest(s.mx, s.my, s.menus, s.depth, s.nitems, &s.pos[0][0], &s.level, &s.item)) {
            m = &s.menus[s.level][s.item];
            if (m->on1) {
                if (s.item == s.sel[s.level]) {
                    sub_903F0(s.ptr, s.oldmx, s.oldmy);
                    for (i = 3; i >= 0; i--) {
                        if (s.bg[i]) {
                            sub_903F0(s.bg[i], s.pos[i][0], s.pos[i][1]);
                            s.pos[i][0] = s.pos[i][1] = 0;
                            FreeMemBlock(s.bg[i]);
                        }
                    }
                    s.depth = 0;
                    r = ((int (*)(void))s.menus[s.level][s.item].on1)();
                    s.menus[1] = s.menus[2] = s.menus[3] = 0;
                    s.bg[0] = s.bg[1] = s.bg[2] = s.bg[3] = 0;
                    s.sel[1] = s.sel[2] = s.sel[3] = 0;
                    s.nitems[1] = s.nitems[2] = s.nitems[3] = 0;
                    if (r == 1) {
                        FreeMemBlock(s.ptr);
                        return 0;
                    }
                    MouseSetPos(s.oldmx, s.oldmy);
                    s.mx = s.oldmx;
                    s.my = s.oldmy;
                    ClearInputQueue();
                } else {
                    sub_903F0(s.ptr, s.oldmx, s.oldmy);
                    DrawMenuItemSel((struct MenuItem *)&s.menus[s.level][s.sel[s.level]], s.pos[s.level][0],
                        s.pos[s.level][1], col1, col2, col3);
                    s.sel[s.level] = s.item;
                    DrawMenuItemNorm((struct MenuItem *)&s.menus[s.level][s.item], s.pos[s.level][0],
                        s.pos[s.level][1], col1, col2, col3);
                }
            } else if (m->on2) {
                sub_903F0(s.ptr, s.oldmx, s.oldmy);
                if (s.depth != s.level) {
                    for (i = s.depth; i > s.level; i--) {
                        s.sel[i] = 0;
                        if (s.bg[i]) {
                            sub_903F0(s.bg[i], s.pos[i][0], s.pos[i][1]);
                            s.pos[i][0] = s.pos[i][1] = 0;
                            FreeMemBlock(s.bg[i]);
                            s.bg[i] = 0;
                            s.menus[i] = 0;
                            s.nitems[i] = 0;
                        }
                    }
                    s.depth = s.level;
                }
                i = s.depth;
                DrawMenuItemSel((struct MenuItem *)&s.menus[i][s.sel[i]], s.pos[i][0], s.pos[i][1], col1, col2, col3);
                s.sel[i] = s.item;
                DrawMenuItemNorm((struct MenuItem *)&s.menus[i][s.item], s.pos[i][0], s.pos[i][1], col1, col2, col3);
                m = &s.menus[s.level][s.item];
                s.depth = i + 1;
                s.menus[s.depth] = (MenuItem *)m->on2;
                s.nitems[s.depth] = m->pad;
                s.pos[s.depth][0] = (s.depth == 1 ? m->x1 : m->x2) + s.pos[s.depth - 1][0];
                s.pos[s.depth][1] = (s.depth == 1 ? m->y2 : m->y1) + s.pos[s.depth - 1][1];
                s.w = s.menus[s.depth][s.nitems[s.depth] - 1].x2 - s.menus[s.depth][0].x1 + 1;
                s.h = s.menus[s.depth][s.nitems[s.depth] - 1].y2 - s.menus[s.depth][0].y1 + 1;
                s.bg[s.depth] = sub_8CCA8((char *)str_menubuff, s.w * s.h + 0x11, 0x20);
                *(MenuHdr17 *)s.bg[s.depth] = *(MenuHdr17 *)pointerspr;
                ((short *)s.bg[s.depth])[2] = s.w;
                ((short *)s.bg[s.depth])[3] = s.h;
                sub_91400(s.bg[s.depth], s.pos[s.depth][0], s.pos[s.depth][1]);
                DrawMenuDropdown(s.menus[s.depth], s.nitems[s.depth], s.pos[s.depth][0], s.pos[s.depth][1], col1, col2,
                    col3);
                s.sel[s.depth] = 0;
                DrawMenuItemNorm((struct MenuItem *)s.menus[s.depth], s.pos[s.depth][0], s.pos[s.depth][1], col1, col2,
                    col3);
            } else {
                sub_903F0(s.ptr, s.oldmx, s.oldmy);
                DrawMenuItemSel((struct MenuItem *)&s.menus[s.level][s.sel[s.level]], s.pos[s.level][0],
                    s.pos[s.level][1], col1, col2, col3);
                s.sel[s.level] = s.item;
                DrawMenuItemNorm((struct MenuItem *)&s.menus[s.level][s.item], s.pos[s.level][0], s.pos[s.level][1],
                    col1, col2, col3);
            }
        } else {
            sub_903F0(s.ptr, s.oldmx, s.oldmy);
            for (i = 3; i >= 0; i--) {
                if (s.bg[i]) {
                    sub_903F0(s.bg[i], s.pos[i][0], s.pos[i][1]);
                    s.pos[i][0] = s.pos[i][1] = 0;
                    FreeMemBlock(s.bg[i]);
                }
            }
            s.depth = 0;
            s.menus[1] = s.menus[2] = s.menus[3] = 0;
            s.bg[0] = s.bg[1] = s.bg[2] = s.bg[3] = 0;
            s.sel[1] = s.sel[2] = s.sel[3] = 0;
            s.nitems[1] = s.nitems[2] = s.nitems[3] = 0;
        }
        sub_91400(s.ptr, s.oldmx, s.oldmy);
        sub_903F0(s.ptr, s.oldmx, s.oldmy);
        sub_91400(s.ptr, s.mx, s.my);
        sub_91370(pointerspr, s.mx, s.my);
        s.oldmx = s.mx;
        s.oldmy = s.my;
    }
}
