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

/* CountPuckThreat (4C632, PC only; ends in ass_popx): count into threat the opposing skaters within 14h (x and
   y) of where the puck goes next (the puck plus the goalie's velocity / 100h), each skater also taken one step
   ahead. p: the goalie (SCnum below 6 is the home side: look at the away skaters, SortCords + 6). No result. */
void CountPuckThreat(Player *p)
{
    short y;
    Player *q;
    short x;
    short i;
    short d;

    x = (p->Xvel >> 8) + *puckx;
    y = (p->Yvel >> 8) + *pucky;
    threat = 0;
    q = &SortCords[p->SCnum < 6 ? 6 : 0];
    for (i = 0; i < 6; q++, i++) {
        d = (q->Xvel >> 8) + HIWORD(q->Xpos) - x;
        if ((d < 0 ? -d : d) > 0x14) continue;
        d = (q->Yvel >> 8) + HIWORD(q->Ypos) - y;
        if ((d < 0 ? -d : d) > 0x14) continue;
        threat++;
    }
}

/* asswingd (4A1A6; 94G assign94 asswingd, PC version; ends in ass_popx, owner of asswingd_skate0 /
   asswingd_skateto): the winger's defensive assignment. Not while locked (pflags bit 5), at the bench
   (check4bench) or joystick controlled (bit 3); with the clock stopped (gmode bit 0) just stop (StopIfFree). A new
   assignment (bit 1) clears temp1 and sets temp2 8. Every aioff ticks (the high byte of temp1) switch to the
   offensive winger (assignment 4) when his own team has the puck. Else skate to the wing (x 78h, -78h for the
   right wing, toward the puck's side when the puck is on it) between the blue line (8Eh) and the slot (44h,
   following the puck down to the goal line E8h; offside, tmflags bit 4: stay at 44h); mirrored for the other
   end (pflags bit 7). p: the winger. No result. */
void asswingd(Player *p)
{
    if (p->pflags & 0x20) return;
    if (check4bench(p)) return;
    if (gmode & 1) {
        StopIfFree(p);
        return;
    }
    if (p->pflags & 8) return;
    if (p->pflags & 2) {
        p->pflags &= 0xFD;
        p->temp1 = 0;
        p->temp2 = 8;
    }
    if (--((signed char *)p)[0x27] < 0) {
        ((signed char *)p)[0x27] = p->aioff;
        if (*puckc >= 0 && (*puckc < 6) == (p->SCnum < 6)) {
            assreplace(p, 4);
            return;
        }
    }
    regd0.l = p->position == 5 ? -0x78 : 0x78;
    regd1.w = 0x8E;
    regd2.w = 0x44;
    if (p->pflags & 0x80) {
        regd0.w = -regd0.w;
        regd1.w = -0x8E;
        regd2.w = -0x44;
        if (p->tmptr->tmflags & 0x10) regd1.w = regd2.w;
        else if (*pucky >= regd2.w) regd1.w = *pucky;
        else {
            regd2.w = -0xE8;
            if (regd2.w >= *pucky) regd1.w = regd2.w;
        }
    } else {
        if (p->tmptr->tmflags & 0x10) regd1.w = regd2.w;
        else if (*pucky <= regd2.w) regd1.w = *pucky;
        else {
            regd2.w = 0xE8;
            if (regd2.w <= *pucky) regd1.w = regd2.w;
        }
    }
    if ((*puckx ^ regd0.w) >= 0) regd0.w = *puckx;
    skateto(p, 0);
}

/* Endfaceoff (4DA7B; 93G logic93 / 94G checks94 Endfaceoff "drop the puck: pick the faceoff winner and send the
   puck off", PC version; ends in ass_popx): stop the sample, play sfx ABh, end the faceoff (gmode2 bits 0 and 2,
   the clock bit of gmode, p's pflags2 bit 0; sflags3 bit 4). The odds (regd2) are 10h less the first centre's
   faceoff stance (byte_CCA95 by fofdata2) plus the second's, plus the away centre's faceoff rating less the home
   centre's (byte_DAEA7 / byte_DACB3 by pnum; the centres are the skaters with position 4): randomd0(21h) below
   them gives the puck to the away centre (lasttouch, his input word_E0396), else to the home centre (word_E0392).
   The winner's input bit 0Bh picks the direction (fodir2 with y speed -800h, else fodir1 / +800h); a held
   direction (bit 3 clear) is used 3 times in 4, else a random one of the 5 around the y direction (mirrored by
   the sign of the y speed). The puck p gets that velocity (dirtab * 20h, plus the y speed), a random z velocity
   (below 800h), z 0, loses pflags bit 2 and takes assignment 18h (pucknorm); the referee (SortCords + 16) takes
   1Fh and word_E0028 is set to 500h toward his side. p: the puck. No result. */
void Endfaceoff(Player *p)
{
    short i;
    short j;
    short w;

    StopDigiSample();
    sfx(0xAB);
    gmode2 &= 0xFA;
    gmode &= 0xFE;
    p->pflags2 &= 0xFE;
    sflags3 |= 0x10;
    regd2.w = 0x10 - byte_CCA95[fofdata2[0]] + byte_CCA95[word_E0394];
    for (i = 0; i < 6; i++) {
        if (SortCords[i].position == 4) break;
    }
    for (j = 6; j < 12; j++) {
        if (SortCords[j].position == 4) break;
    }
    regd2.w += byte_DAEA7[SortCords[j].pnum * 20] - byte_DACB3[SortCords[i].pnum * 20];
    if (randomd0(0x21) < regd2.w) {
        w = word_E0396;
        lasttouch = j;
    } else {
        w = word_E0392;
        lasttouch = i;
    }
    if (w & 0x800) {
        regd3.w = fodir2;
        regd4.w = -0x800;
    } else {
        regd3.w = fodir1;
        regd4.w = 0x800;
    }
    regd0.w = regd3.w;
    if (!(regd0.w & 8)) {
        regd0.w &= 7;
        if (!randomd0(4)) {
            regd0.w = (randomd0(5) - 2) & 7;
            if (regd4.w < 0) *(unsigned char *)&regd0 ^= 4;
        }
    } else {
        regd0.w = (randomd0(5) - 2) & 7;
        if (regd4.w < 0) *(unsigned char *)&regd0 ^= 4;
    }
    regd0.w += regd0.w;
    p->Xvel = dirtab[regd0.w] << 5;
    p->Yvel = (dirtab_y[regd0.w] << 5) + regd4.w;
    p->Zvel = randomd0(0x800);
    *(short *)puckz = 0;
    p->pflags &= 0xFB;
    assreplace(p, 0x18);
    assreplace(&SortCords[16], 0x1F);
    word_E0028 = HIWORD(SortCords[16].Xpos) > 0 ? 0x500 : -0x500;
}
