/* Engine physics AI: offside banner. */
/* DRAFT: 424 vs 424. The far-side player test is cmp ecx,esi / jg; the original is
 cmp esi,ecx / jl. Do not retry: writing the limit on the left of < hoists that lea
 out of the loop. */
#include "nhl95.h"

/* checkob: with offsides on, and not during a penalty shot, watch the six opponents.
 When one is past the puck on p's attacking side (4 bytes deeper once p has crossed
 -52h or 52h and is not skating back), set gmode2 bit 7 and, if the banner is free
 and the puck is still in p's half, show banner 4. Clearing that bit drops banner 4
 once its timer has run out. */
void checkob(Player *p)
{
    short found;
    int extra;
    Player *o;
    signed char n;
    int y;

    found = 0;
    extra = 0;
    if (!gameopts.offsides) return;
    if (penshotlive) return;
    if (penshotstart) return;
    o = &SortCords[p->SCnum < 6 ? 0 : 6];
    if ((p->pflags & pfgoal) == 0) {
        y = p->Ypos >> 16;
        if (y > -0x52 && p->Yvel <= 0) extra = 4;
        if (-(extra + 0x4E) <= *pucky) {
            for (n = 0; n < 6; o++, n++) {
                if (o->position >= 0 && -(extra + 0x4E) > (o->Ypos >> 16)) {
                    found = 1;
                    break;
                }
            }
        }
    } else {
        if (HIWORD(p->Ypos) < 0x52 && p->Yvel >= 0) extra = 4;
        if (extra + 0x4E >= *pucky) {
            for (n = 0; n < 6; o++, n++) {
                if (o->position >= 0 && (o->Ypos >> 16) > extra + 0x4E) {
                    found = 1;
                    break;
                }
            }
        }
    }
    if (found) {
        if (gmode2 & 0x80) return;
        gmode2 |= 0x80;
        if (bannermsg >= 4) return;
        if (((p->pflags & pfgoal) != 0) ^ (*pucky < 0)) {
            bannermsg = 4;
            bannertimer = 0;
        }
        return;
    }
    if ((gmode2 & 0x80) == 0) return;
    gmode2 &= 0x7F;
    if (bannermsg != 4) return;
    if (bannertimer != 0) return;
    bannermsg = -1;
}
