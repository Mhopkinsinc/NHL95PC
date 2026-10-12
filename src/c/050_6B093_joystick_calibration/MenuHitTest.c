/* Joystick calibration file: menu hit test. */
#define MenuHitTest MenuHitTestDecl
#include "nhl95.h"
#undef MenuHitTest

typedef struct MenuItem {
    int x1, y1, x2, y2;
    char *text;
    int on1, on2;
    int pad;
} MenuItem;

/* MenuHitTest: from menu i down to 0, the first item whose box (origin + x1,y1,x2,y2) contains
 (x + 4, y). Writes the menu and item indexes. Returns 1 for a hit. */
int MenuHitTest(int x, int y, MenuItem **menus, int i, int *counts, int *origin,
                int *hitMenu, int *hitItem)
{
    int j;
    int *o;

    x += 4;
    for (; i >= 0; i--) {
        if (!menus[i]) continue;
        for (j = 0; j < counts[i]; j++) {
            o = origin + i * 2;
            if (x < o[0] + menus[i][j].x1) continue;
            if (x > o[0] + menus[i][j].x2) continue;
            if (y < o[1] + menus[i][j].y1) continue;
            if (y > o[1] + menus[i][j].y2) continue;
            *hitMenu = i;
            *hitItem = j;
            return 1;
        }
    }
    return 0;
}
