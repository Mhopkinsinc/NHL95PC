/* Misc dialogs: scroll-bar hit tracking. */
#define TrackScrollBars TrackScrollBars_decl
#include "nhl95.h"
#undef TrackScrollBars

/* TrackScrollBars: the same hit test for scroll bars (12 ints each). A bar under the mouse is
 pressed while button bit 0 is down. Bit 1 releases it. The thumb follows the mouse, clamped
 inside the bar, and the item index is thumb * total / (height - 4). Returns the bar under the
 mouse, or -1.
 DRAFT: everything up to the exit matches; the EXE ends in `jmp TrackButtons_x`, so the match needs this
 function in TrackButtons.c after DrawButtons, DrawButton, InitScrollBar and DrawScrollBar (address order). */
int TrackScrollBars(int *list, int n, int x, int y, int buttons)
{
    int yy;
    int hit;
    int i;
    int old;
    volatile int t;

    yy = y;
    hit = -1;
    for (i = 0; i < n; i++) {
        if (x >= (list + i * 12)[0] && x < (list + i * 12)[0] + (list + i * 12)[2]
            && yy >= (list + i * 12)[1] && yy < (list + i * 12)[1] + (list + i * 12)[3]) {
            old = (list + i * 12)[5];
            if ((buttons & 1) && (list + i * 12)[11] == 0) {
                (list + i * 12)[11] = -1;
                old = -1;
            }
            if (buttons & 2) {
                (list + i * 12)[11] = 0;
                old = -1;
            }
            t = yy - (list + i * 12)[1];
            (list + i * 12)[5] = t - (list + i * 12)[7] / 2;
            if ((list + i * 12)[5] < 0)
                (list + i * 12)[5] = 0;
            else if ((list + i * 12)[3] - (list + i * 12)[7] - 4 < (list + i * 12)[5])
                (list + i * 12)[5] = (list + i * 12)[3] - (list + i * 12)[7] - 4;
            (list + i * 12)[8] = (list + i * 12)[5] * (list + i * 12)[10] / (t = (list + i * 12)[3] - 4);
            hit = i;
            if (old != (list + i * 12)[5]) DrawScrollBar(list + i * 12);
        } else {
            if ((list + i * 12)[11] == 0) continue;
            (list + i * 12)[11] = 0;
            DrawScrollBar(list + i * 12);
        }
    }
    return hit;
}
