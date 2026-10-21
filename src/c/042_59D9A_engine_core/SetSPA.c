/* SetSPA: start sprite animation 'spa' (an SPAtab index) on player p, unless it is
 already playing. Restarting clears the frame number and sets the frame counter to -1 so the next updateanim
 loads the first frame. Callers: assignments, skating, faceoffs, input. */
#include "nhl95.h"

void SetSPA(Player *p, short spa)
{
    if (spa != p->SPA) {        /* already running: leave it alone */
        p->SPAnum = 0;
        p->SPA = spa;
        p->SPAcnt = -1;         /* restart animation */
    }
}

/* DRAFT (first pass from Ghidra): the EXE splits it. Its body (SprSort_loop, SprSort_test) follows SetSPA and its
   entry (5DD7C) follows SprSortVert, which falls into it. */
/* SprSort (5DD7C; 93G SprSort): sort the 17 sort objects (SortCords) for drawing: copy each one's y (its x when
   sflags bit 7, sfhor) to Ylist, then bubble sort OOlist by Ylist and record each object's place in OOlistpos.
   No arguments and no result. */
void SprSort(void)
{
    short *y;
    Player *p;
    short i;
    signed char *o;
    short a;
    short b;
    short n;
    char swapped;

    y = Ylist;
    p = SortCords;
    for (i = 0; i < 0x11; i++) {
        *y++ = (sflags & 0x80) ? HIWORD(p->Xpos) : HIWORD(p->Ypos);
        p++;
    }
    do {
        swapped = 0;
        o = (signed char *)OOlist;
        n = 0;
        for (i = 0; i < 0x10; i++) {
            a = *o;
            o++;
            b = *o;
            n++;
            if (Ylist[o[-1]] > Ylist[*o]) {
                o[0] = a;
                o[-1] = b;
                OOlistpos[a] = n;
                OOlistpos[b] = n - 1;
                swapped = 1;
            }
        }
    } while (swapped);
}

/* SprSortVert (5DD6B; 93G SprSortVert): sort the objects by y (clear sflags bit 7) and SprSort. No result. */
void SprSortVert(void)
{
    sflags &= 0x7F;
    SprSort();
}
