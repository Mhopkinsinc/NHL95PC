/* Settings locker room: a filled panel with a light and dark edge. */
/* DRAFT: 430 vs 430. x1 stays in esi and y1 in edi. The original saves x1 in edi and y1 in esi.
 x2 stays in ebp and every edge and corner call matches once those two are swapped. Do not retry:
 copying y before x put y in ebp and x2 in esi; copying only y put x2 in edi; copying only x put x
 in ebp and x2 in edi; copying x then y put x in ebp, y in edi and x2 in esi. */
#include "nhl95.h"

/* DrawPanel3D: fill x1,y1..x2,y2 in colour 70h. The top and left edges are 77h and the right and
 bottom edges are 62h. When corners is set, the four corners also get a pair of pixels two and three
 steps in: 77h on the outer diagonal and 62h on the inner one. */
void DrawPanel3D(int x1, int y1, int x2, int y2, int corners)
{
    int xa;
    int ya;
    int yb;
    int yc;

    sub_90D20(x1, y1, x2 - x1 + 1, y2 - y1 + 1, 0x70);
    sub_B4FAC(x1, y1, x2, y1, 0x77);
    sub_B4FAC(x1, y1, x1, y2, 0x77);
    sub_B4FAC(x2, y1, x2, y2, 0x62);
    sub_B4FAC(x1, y2, x2, y2, 0x62);
    if (corners) {
        sub_B5DB0(xa = x1 + 2, ya = y1 + 2, 0x77);
        sub_B5DB0(x1 += 3, y1 += 3, 0x77);
        sub_B5DB0(x1, ya, 0x62);
        sub_B5DB0(xa, y1, 0x62);
        sub_B5DB0(xa, yb = y2 - 2, 0x62);
        sub_B5DB0(x1, yc = y2 - 3, 0x62);
        sub_B5DB0(x1, yb, 0x77);
        sub_B5DB0(xa, yc, 0x77);
        sub_B5DB0(x1 = x2 - 3, ya, 0x77);
        sub_B5DB0(x2 -= 2, y1, 0x77);
        sub_B5DB0(x2, ya, 0x62);
        sub_B5DB0(x1, y1, 0x62);
        sub_B5DB0(x1, yb, 0x62);
        sub_B5DB0(x2, yc, 0x62);
        sub_B5DB0(x2, yb, 0x77);
        sub_B5DB0(x1, y1 = yc, 0x77);
    }
}
