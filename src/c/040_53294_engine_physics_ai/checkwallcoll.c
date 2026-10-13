/* Engine physics AI: wall collision. */
/* DRAFT: 470 vs 481. The prologue, both radius loads, and the y test match. xlim is reloaded in edx
 (original ecx). The x test negates that limit in ecx and compares dx with cx; the original negates it
 in edx, reloads x, and compares with regd4 in memory.
 Do not retry: non-volatile limits sign-extend the radii and spill y. Declaring ylim before xlim swaps
 the two stack slots. */
#include "nhl95.h"

#define W(v) (*(int *)((char *)&(v) - 2) >> 16)

/* 0xA0 - wcradiusx and 0x108 - wcradiusy. The word load stays in dx/bx, the argument registers. */
int RinkLimitX(void);
#pragma aux RinkLimitX = "mov eax,0A0h" "mov dx,[wcradiusx]" "sub eax,edx" value [eax] modify exact [eax edx ebx];
int RinkLimitY(void);
#pragma aux RinkLimitY = "mov eax,108h" "mov bx,[wcradiusy]" "sub eax,ebx" value [eax] modify exact [eax ebx];

/* checkwallcoll: keep a player inside the rink. x and y are tested against the wall inset
 (160 and 264 minus the wall radii, then 64 minus those). A point past a corner is pushed
 back along the corner normal when it is at least 64 away (vecdist); a point in the goal
 mouth is handed to checkgoal for net 12 or 13. If the player has no wall vector yet, one
 is built from the nearest boards and wallcollb runs. */
void checkwallcoll(Player *p, short x, short y)
{
    volatile int xlim;
    volatile int ylim;
    short ry0;
    int dist;

    xlim = RinkLimitX();
    ylim = RinkLimitY();
    regd4.w = 0x40 - xlim;
    ry0 = word_E03B8 = 0x40 - ylim;
    if (y > word_E03B8) {
        word_E03B8 = -word_E03B8;
        if (y < word_E03B8) goto dot4;
    }
    if (x < regd4.w) goto dot2;
    regd4.w = -regd4.w;
    if (x <= regd4.w) goto dot3;
dot2:
    regd1.w = x - regd4.w;
    regd0.w = y - word_E03B8;
    dist = vecdist(W(regd0), W(regd1));
    regd3.w = dist;
    regd1.w = -regd1.w;
    if ((short)dist >= 0x40) {
        regd0.w = (W(regd0) << 8) / (short)dist;
        regd1.w = (W(regd1) << 8) / (short)dist;
        wallcollb(p);
    }
    goto dot4;
dot3:
    checkgoal(p, &SortCords[0x0C + (ry0 == word_E03B8)], x, y);
dot4:
    regd0.w = p->Wallsin | p->Wallcos;
    if (regd0.w != 0) return;
    regd0.w = 0x100;
    regd1.w = 0;
    if (y < ylim) {
        ylim = -ylim;
        regd0.w = (short)0xFF00;
        if (y > ylim) {
            regd0.w = 0;
            regd1.w = (short)0xFF00;
            if (x < xlim) {
                xlim = -xlim;
                regd1.w = 0x100;
                if (x > xlim) return;
            }
        }
    }
    wallcollb(p);
}
