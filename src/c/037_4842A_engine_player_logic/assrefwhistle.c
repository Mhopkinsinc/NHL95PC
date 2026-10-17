/* Engine player logic: referee whistles, signals and drops. */
#include "nhl95.h"

/* assrefwhistle (4E8EF; 93G hockey93 assrefwhistle?, PC version): referee p blows the whistle. Not while he is
   locked (pflags bit 5). A new assignment (pflags bit 1): when the play is over (word_CBEC6, the clock at 0, or a
   tied shootout-period game gsp 3 decided), end the stoppage (RefStep -1, overlay timer 100h -> 258h) and hand the
   puck and p their next assignments (1Bh, 1Eh); else start the whistle (RefStep 1, PenShotAssign, spot A0h, 0 in
   temp3 / temp4, SPA A5Bh) unless the crowd reacts (CrowdOnStoppage). Then skate to the spot (SkateToSpot) until
   within 6 / -10 of it, turn toward facing 2 every 8 ticks (temp1) and, facing it, lock with SPA C1Bh and take
   assignment 22h. p: the referee. No result. */
void assrefwhistle(Player *p)
{
    short d;

    if (p->pflags & 0x20) return;
    if (p->pflags & 2) {
        if (word_CBEC6 || (!gameclock && !clockticks[0]) || (gsp == 3 && hmscore[0] != awscore)) {
            RefStep[0] = -1;
            if (ovltimer == 0x100) ovltimer = 0x258;
            assreplace((Player *)puckstruct, 0x1B);
            assreplace(p, 0x1E);
            return;
        }
        RefStep[0] = 1;
        p->pflags &= 0xFD;
        PenShotAssign();
        p->temp2 = 8;
        p->temp4 = 0;
        p->temp3 = 0xA0;
        p->temp1 = 0;
        p->pflags2 |= 0x20;
        p->Wallcos = 0;
        p->Wallsin = 0;
        SetSPA(p, 0xA5B);
        if (CrowdOnStoppage()) return;
        if (ovltimer == 0x100) ovltimer = 0x258;
    }
    regd0.w = HIWORD(p->Ypos) - p->temp4;
    if ((regd0.w < 0 ? -regd0.w : regd0.w) > 6) goto skate;
    regd0.w = HIWORD(p->Xpos) - p->temp3;
    if (regd0.w < -10) goto skate;
    if (--p->temp1 >= 0) return;
    p->temp1 += 8;
    SetSPA(p, 0xA5B);
    regd2.l = 2;
    if (regd2.w != p->facedir) {
        regd2.w = (regd2.w - p->facedir) & 7;
        p->facedir = (regd2.w < 5 ? 1 : -1) + (*(int *)((char *)&p->facedir - 2) >> 16) & 7;
    }
    p->Yvel = 0;
    p->Xvel = 0;
    if (regd2.w != p->facedir) return;
    p->pflags |= 0x20;
    SetSPA(p, 0xC1B);
    assreplace(p, 0x22);
    return;
skate:
    regd0.w = p->temp3;
    regd1.w = p->temp4;
    SkateToSpot(p, 0);
}

/* RequestLineChange (4D938, PC only; ends in ass_popx): open the line change box for p's team (pflags bit 6 =
   away): the choice 1 through getlchoice gives the line (lcsel 1, lcline), or when there is none lcsel 0 and the
   team's current line (tmline); then the box shows (lctimer 3Ch, lcblink 1, lcblinktime 0Ch, lcboxon 1) and that
   side's dword_C585C / dword_C5860 is cleared. p: a player of the team. No result. */
void RequestLineChange(Player *p)
{
    short side;
    short s;
    short one;

    side = (p->pflags & 0x40) != 0;
    s = side;
    one = 1;
    regd0.w = one;
    getlchoice(p);
    if (regd0.w < 0) {
        lcsel[side] = 0;
        lcline[s] = (side > 0 ? &awtmstruct : &hmtmstruct)->tmline;
    } else {
        lcsel[side] = one;
        lcline[side] = regd0.w;
    }
    lctimer[s] = 0x3C;
    lcblink[s] = 1;
    lcblinktime[s] = 0xC;
    lcboxon[s] = 1;
    if (s) dword_C5860 = 0;
    else dword_C585C = 0;
}
