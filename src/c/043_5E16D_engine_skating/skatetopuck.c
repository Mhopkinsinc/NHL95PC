/* Skating: skate to the puck. */
/* DRAFT: in EvadePlayers.c it comes out the same size with two register picks left; see docs/UNSURE.md.
   Its match belongs in EvadePlayers.c (it ends in EvadePC_x), but there it moves the shared pops (StopNA
   then branches into EvadePC), so it stays in its own file for now. */
#define skatetopuck skatetopuck_decl
#define imul32 imul32_decl
#include "nhl95.h"
#undef skatetopuck
#undef imul32

int __cdecl imul32(int a, int b);

/* skatetopuck (5EB17, 93G skatetopuck; ends in EvadePC_x): player p skates to the puck. The target is the puck
   plus half the high byte of its velocity (regd0 / regd1, 93G skatetopuckinit). Every 10 ticks (5 for a
   goalie; the temp2 high byte): avoid the goal (avdgoal), then with a loose puck, a puck faster than 12Ch or a
   target farther than 18h, aim with GetHot (the target less p's position and velocity lead) through vtoa, else
   take p's direction to the puck (puckdir); keep it in the temp2 low byte. A skater then checks for a sweep:
   with the puck coming straight at it (fast pucks only one time in four), facing the puck and between 30 and 38
   away, Sweepcheck. Otherwise doplayeracc in the kept direction. p: the player. No result. */
void skatetopuck(Player *p)
{
    short sx;
    short sy;

    regd0.w = (short)(HIBYTE(*puckvx) >> 1) + *puckx;
    regd1.w = (short)(HIBYTE(*puckvy) >> 1) + *pucky;
    if (--TEMP2_TICKS(p) >= 0) goto ex;
    TEMP2_TICKS(p) += p->position == 0 ? 5 : 10;
    avdgoal(p);
    sx = regd0.w;
    sy = regd1.w;
    if (*puckc < 0 || ABS(*puckvx) > 0x12C || ABS(*puckvy) > 0x12C
        || (unsigned)vecdist(sx - HIWORD(p->Xpos), sy - HIWORD(p->Ypos)) > 0x18) {
        GetHot(p);
        regd0.w = sx - ((signed char)regd0.w + HIBYTE(p->Xvel)) - HIWORD(p->Xpos);
        regd1.w = sy - ((signed char)regd1.w + HIBYTE(p->Yvel)) - HIWORD(p->Ypos);
        regd0.w = vtoa(regd0.w, regd1.w);
    } else {
        regd0.w = p->puckdir;
    }
    TEMP2_DIR(p) = regd0.w;
    if (p->position == 0) goto ex;
    regd0.w = vtoa(*puckvx, *puckvy) ^ 4;
    if (regd0.w == p->facedir) {
        if (ABS(*puckvx) >= 0xFA0 || ABS(*puckvy) >= 0xFA0) {
            if (randomd0(4)) goto ex;
        }
    }
    regd0.w = *puckx - HIWORD(p->Xpos);
    regd1.w = *pucky - HIWORD(p->Ypos);
    if (vtoa(regd0.w, regd1.w) != p->facedir) goto ex;
    regd0.l = imul32(regd0.w, regd0.w) + imul32(regd1.w, regd1.w);
    if (regd0.ul <= 30 * 30 || regd0.ul > 38 * 38) goto ex;
    Sweepcheck(p);
    return;
ex:
    doplayeracc(p, TEMP2_DIR(p));
}
