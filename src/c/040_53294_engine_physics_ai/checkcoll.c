/* Engine physics AI: one object's collision pass. */
/* DRAFT: 479 vs 468. x is saved in edi and y in esi. The original saves x in esi and y in edi, so the
 swap is two movs and this draft needs a third. The first checkwallcoll sar sequence matches apart from
 that shift. Do not retry: short parameters, or copying x then y into locals (that saves ebp and the
 cookie grows to 18h). */
#include "nhl95.h"

#define OOB(i) (*(int *)(OOlist + (i) - 2) >> 24)

/* checkcoll: clear the collision flag. Unless pflags bit 2 is set, test the walls at the player's
 position (and, for a skater with no wall normal, again at the hot-spot step), then the other players.
 On a collision, keep the old position unless the animation is in the block range. Otherwise reinsert
 p in the y-sorted object list and store its y. When sflags bit 7 is set, the two coordinates are swapped. */
void checkcoll(Player *p, int x, int y)
{
    short sc;
    short i;
    short pl;

    dword_CCC2C = 0;
    collflag = 0;
    if (sflags & 0x80) {
        pl = x;
        x = y;
        y = pl;
    }
    if (!(p->pflags & 4)) {
        wcradiusx = p->radiusx;
        wcradiusy = p->radiusy;
        checkwallcoll(p, p->Xpos >> 16, p->Ypos >> 16);
        if ((p->Wallcos | p->Wallsin) == 0 && p->SCnum <= 11) {
            GetHot(p);
            wcradiusx = wcradiusy = 1;
            checkwallcoll(p, (short)((p->Xpos >> 16) + regd0.w), (short)((p->Ypos >> 16) + regd1.w));
        }
        checkplcoll(p, x, y);
    }
    if (collflag) {
        if (p->SPA < 0xF0F || p->SPA > 0x1055) {
            *(short *)((char *)&p->Xpos + 2) = *(short *)((char *)&p->OldXpos + 2);
            *(short *)((char *)&p->Ypos + 2) = *(short *)((char *)&p->OldXpos + 6);
        }
        return;
    }
    sc = p->SCnum;
    i = OOlistpos[sc];
    while (i != 16) {
        pl = (signed char)OOlist[i + 1];
        if (y <= Ylist[OOB(i)]) break;
        OOlist[i] = pl;
        OOlist[i + 1] = sc;
        OOlistpos[sc]++;
        OOlistpos[pl]--;
        i++;
    }
    i = OOlistpos[sc];
    if (i) {
        do {
            pl = (signed char)OOlist[i - 1];
            if (y >= Ylist[*(int *)(OOlist + i - 4) >> 24]) break;
            OOlist[i] = pl;
            OOlist[i - 1] = sc;
            OOlistpos[sc]--;
            OOlistpos[pl]++;
        } while (--i);
    }
    Ylist[sc] = y;
}
