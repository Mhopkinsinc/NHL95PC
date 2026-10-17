/* Engine player logic: offensive winger. */
#include "nhl95.h"

/* asswingo (4A343; 94G assign94 asswingo, PC version; ends in asswingd_skateto): the winger's offensive
   assignment. Not while locked (pflags bit 5), at the bench (check4bench) or joystick controlled (bit 3); with the
   clock stopped (gmode bit 0) just stop (StopIfFree). A new assignment (bit 1) sets the high byte of temp5 to -1,
   clears temp1 and sets temp2 8. Every aioff ticks (the high byte of temp1) switch to the defensive winger
   (assignment 3) when the other team has the puck. The puck's zone (its y plus the y velocity / 40h, toward the
   attacked end: 0 before the far blue line, 4 neutral, 8 in the zone, 0Ch below the goal line; 4 while offside,
   tmflags bit 4) picks a spot from word_CCA18 (x, x range, y, y range per zone): a new zone, or 1 in 80h ticks,
   picks a random spot into temp3 (+ aioff) / temp4. Skate there (x mirrored for the right wing, both for the
   other end, pflags bit 7) with EvadePC. p: the winger. No result. */
void asswingo(Player *p)
{
    short d;

    if (p->pflags & 0x20) return;
    if (check4bench(p)) return;
    if (gmode & 1) {
        StopIfFree(p);
        return;
    }
    if (p->pflags & 8) return;
    if (p->pflags & 2) {
        p->pflags &= 0xFD;
        ((signed char *)p)[0x2F] = -1;
        p->temp1 = 0;
        p->temp2 = 8;
    }
    if (--((signed char *)p)[0x27] < 0) {
        ((signed char *)p)[0x27] = p->aioff;
        if (*puckc >= 0 && (p->SCnum < 6) ^ (*puckc < 6)) {
            assreplace(p, 3);
            return;
        }
        regd0.w = *pucky;
        regd3.w = (*puckvy >> 6) + regd0.w;
        if (!(p->pflags & 0x80)) {
            regd0.w = -regd0.w;
            regd3.w = -regd3.w;
        }
        d = 0;
        if (regd3.w >= -0x4E) {
            d = 4;
            if (regd0.w >= 0x4E && !(p->tmptr->tmflags & 0x10)) {
                d = 8;
                if (regd3.w >= 0xE8) d = 0xC;
            }
        }
        if (d != p->temp5 || !(regd0.w = randomd0(0x80))) {
            p->temp5 = d;
            p->temp3 = word_CCA18[d] + (randomd0(word_CCA1A[d] * 2) - word_CCA1A[d]) + p->aioff;
            p->temp4 = randomd0(word_CCA1E[d] * 2) - word_CCA1E[d] + word_CCA1C[d];
        }
    }
    regd0.w = p->temp3;
    if (p->position != 5) regd0.w = -regd0.w;
    regd1.w = p->temp4;
    if (!(p->pflags & 0x80)) {
        regd0.w = -regd0.w;
        regd1.w = -regd1.w;
    }
    skateto(p, (void (*)(Player *))EvadePC);
}
