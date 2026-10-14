/* Settings dialogs: view-only game settings. */
#define ModeSetViewLoop ModeSetViewLoop_decl
#include "nhl95.h"
#undef ModeSetViewLoop

typedef struct Hdr17 { char c[17]; } Hdr17;

/* ModeSetViewLoop: view-only game settings. Grab the screen under the pointer, follow the mouse
 until a button (bits 1 or 2), and put the screen back. Returns 0. */
int ModeSetViewLoop(void)
{
    int oy;
    int ox;
    int y;
    int x;
    int btn;
    int bm;
    int n;
    unsigned char *ev;

    sub_B4B88(0xF7, 4, unk_D16A0);
    sub_B2DCA(&btn, &oy, &ox);
    y = oy;
    x = ox;
    sub_B4BA8();
    n = (((short *)pointerspr)[2] + 1) * (((short *)pointerspr)[3] + 1) + 0x11;
    bm = sub_8CCA8((char *)str_Pointer16, n, 0x20);
    *(Hdr17 *)bm = *(Hdr17 *)pointerspr;
    ((short *)bm)[2] = ((short *)pointerspr)[2] + 1;
    ((short *)bm)[3] = ((short *)pointerspr)[3] + 1;
    sub_91400(bm, y - 4, x);
    sub_91370(pointerspr, y - 4, x);
    ClearInputQueue();
    for (;;) {
        btn = 0;
        for (;;) {
            ev = GetInputEvent();
            if (ev == 0) break;
            btn = ((int (*)(unsigned char *, int *, int *))ptrupdatefn)(ev, &y, &x);
            if (btn & 6) break;
        }
        if (btn != 0) break;
        if (y == oy && x == ox) continue;
        sub_903F0(bm, oy - 4, ox);
        sub_91400(bm, y - 4, x);
        sub_91370(pointerspr, y - 4, x);
        oy = y;
        ox = x;
    }
    sub_903F0(bm, oy - 4, ox);
    FreeMemBlock(bm);
    return 0;
}
