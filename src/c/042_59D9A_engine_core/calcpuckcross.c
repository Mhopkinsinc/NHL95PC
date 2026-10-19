/* Engine core: calcpuckcross and the functions that end in its pop/ret tail (multi-block file, see cc.py). */
#define IsCupClinched IsCupClinched_decl
#include "nhl95.h"
#undef IsCupClinched
int IsCupClinched(short home, short away);  /* 15C30: clockcont_0 passes the scores as shorts */
#define PUCK ((Player *)puckstruct)

/* calcpuckcross: find puck crossing lines: when (if at all) the puck will cross each goal line
. For the goal lines y = +E8h and -E8h: puckcross[0] = x where the puck crosses (bounced
 off the side boards at +-A0h), puckcross[1] = frames until it crosses (dy * 1000h puckvy); [1] = -1 when it
 will not cross (no y velocity, moving away, or out of range). */
void calcpuckcross(void)
{
    short *pc;
    int goaly;
    short side;
    int t;
    int dy;
    short i;
    short x;

    pc = puckcross;
    goaly = 0xE8;                               /* goal line */
    side = 0xA0;                                /* side boards */
    for (i = 0; i < 2; goaly = -goaly, i++, pc += 2) {
        dy = (short)(goaly - *pucky);
        if (*puckvy != 0) {
            t = (dy << 12) / *puckvy;
            if (t >= 0 && t < 0x10000) {
                pc[1] = t;                      /* time until crossing in frames */
                t = *puckvx * dy / *puckvy;
                if (t < 0x10000) {
                    x = t + *puckx;
                    if (x >= 0xA0) x = side * 2 - x;     /* bounce off the side boards */
                    side = -side;
                    if (x <= side) x = side * 2 - x;
                    side = -side;
                    pc[0] = x;
                    continue;
                }
            }
        }
        pc[1] = -1;                             /* no cross */
    }
}

/* GetHot: sprite hot spot (stick blade) of player p. regd0/regd1 = the x/y bytes
 of the hot spot table (HotList/HotList_y, 2 bytes per frame) for the player's frame (word 12h, frames outside 0-283h/293h give 0, 0); x is negated when the
 sprite is X-flipped (attribute bit 3). Draft: also gives the distance that makes reenergizeteam's exit a near
 jump into calcpuckcross's tail, as in the EXE. */
void GetHot(Player *p)
{
    short f;
    short x;

    regd0.w = regd1.w = 0;
    f = p->frame;
    if (f < 0 || f >= 0x468) return;
    if (f >= 0x378) f -= 0xF4;
    else if (f > 0x283) return;
    if (f >= 0x2DA) f -= 0x46;
    else if (f > 0x293) return;
    f += f;
    x = HotList[f];
    regd0.w = x;
    regd1.w = HotList_y[f];
    if (p->attribute & 8)                       /* X flip */
        regd0.w = -regd0.w;
}

/* reenergizeteam: refill the energy of team t's whole roster:
 tmpde = 1000h for every player; a player marked -3 in tmpdst goes to the bench (tmpdst -2, roster status
 byte 3). */
void reenergizeteam(Team *t)
{
    short i;
    short *de;

    de = t->tmpde;
    for (i = 0; i < 28; i++) {
        *de++ = ENERGYMAX;                      /* full energy */
        if (t->tmpdst[i] == -3) {
            t->tmpdst[i] = PDbench;             /* on bench */
            t->tmroster[i * 0x27] = 3;          /* roster status: bench */
        }
    }
}

/* defaultsprites2: the 17 default sort objects: ClearSortCords, then for each object i from ds2list (9 words each:
 x, y, z, frame, attribute word, radiusx, radiusy, asslist[0] byte, pflags byte; the PC has no crsize column):
 SCnum = i, pnum = newpos = newpnum = -1, the integer words of Xpos/Ypos/Zpos, frame, ATTRWORD, radii, first
 assignment, pflags; tmptr optmptr from pfteam; temp5 = 0; OOlist[i] = i, OOlistpos[i] = i. Then the
 referee (SCref): facedir 2, position -1, weight 7, legstr legspd 0Fh; RefStep = -1; SprSort. */
void defaultsprites2(void)
{
    Player *p;
    short *src;
    unsigned char *ol;
    short *op;
    short i;

    ClearSortCords();
    src = (short *)ds2list;
    p = SortCords;
    ol = OOlist;
    op = OOlistpos;
    i = 0;
    do {
        p->SCnum = i;
        p->pnum = -1;                           /* st pnum */
        p->newpos = -1;
        p->newpnum = p->newpos;
        HIWORD(p->Xpos) = *src; src++;
        HIWORD(p->Ypos) = *src; src++;
        HIWORD(p->Zpos) = *src; src++;
        p->frame = *src; src++;
        ATTRWORD(p) = *src; src++;
        p->radiusx = *src; src++;
        p->radiusy = *src; src++;
        p->asslist[0] = *(signed char *)src; src++;
        p->pflags = *(signed char *)src; src++;
        p->tmptr = (p->pflags & pfteam) ? &awtmstruct : &hmtmstruct;
        p->optmptr = !(p->pflags & pfteam) ? &awtmstruct : &hmtmstruct;
        p->temp5 = 0;
        *ol = i;
        *op = i;
        ol++;
        op++;
        p++;
        i++;
    } while (i != 17);
    SortCords[SCref].facedir = 2;
    SortCords[SCref].position = -1;
    SortCords[SCref].weight = 7;
    SortCords[SCref].legspd = SortCords[SCref].legstr = 0x0F;
    RefStep[0] = -1;
    SprSort();
}

/* SetPlList (5BBFA, 93G SetPlList; ends in calcpuckcross_ret6): build PlList, the roster indexes (pnum) of the
   players team t wants on the ice now, -1 for none, with their positions at PlList+6 (byte_E038A). The tmap
   positions come from priolist, past the goalie when he is pulled (tmgoalie < 0; not while the other team takes
   a penalty shot). Each player comes from the team's line table through the sublist of that position: the
   goalie by tmgoalie, the defence by tmlcnt while the line is below 4, the rest by the line; the extra attacker
   (position 6) is slot 26h. Then, from the last slot down, a listed player in the penalty box or held out
   (tmpdst > 0 or <= -3), or in one of the last two slots one that TryAddPlayerToList refuses (already listed),
   is replaced by the first one TryAddPlayerToList takes from his position's sublist, then from the side's
   25-byte list for defence or forwards (unk_E9D50 / byte_E9E4A, from line 6 on byte_E9DB4 / unk_E9EE0, always
   the first pair for position 6), then any roster index from 18h down to 1. tmextra gets the pnum listed for
   position 6 (-1 with the goalie in). No result. */
void SetPlList(Team *t)
{
    signed char *q;
    short g;
    short pos;
    short line;
    signed char c;
    unsigned char *lines;
    signed char *p;
    short side;
    short i;
    short k;

    g = 0;
    line = t->tmline;
    lines = t->tmlines;
    for (side = 0; side < 6; side++) PlList[side] = -1;  /* side as the counter: the EXE's eax */
    if ((!penshotmode || (t == &awtmstruct) == penshotteam) && t->tmgoalie < 0)
        g = 1;                                  /* goalie pulled: skip position 0 */
    else
        t->tmextra = -1;
    i = t->tmap;
    while (--i >= 0) {
        pos = priolist[i + g + 1];
        byte_E038A[i] = priolist[i + g + 1];
        if (pos == 0)
            PlList[i] = lines[((signed char *)unk_CCCC8)[sublist[pos] + t->tmgoalie]];
        else if (pos == 6)
            PlList[i] = lines[0x26];
        else if (!(line >= 4 || pos > 2))       /* this form and line + sublist below give the EXE's registers */
            PlList[i] = lines[((signed char *)unk_CCCC8)[sublist[pos] + t->tmlcnt]];
        else
            PlList[i] = lines[((signed char *)unk_CCCC8)[line + sublist[pos]]];
    }
    for (i = 5; i >= 0; i--) {
        k = PlList[i];
        if (k < 0) continue;
        pos = byte_E038A[i];
        if (t->tmpdst[k] <= -3 || t->tmpdst[k] > 0 || i >= t->tmap - 2 && !TryAddPlayerToList(t, k, i)) {
            p = (signed char *)unk_CCCC8 + sublist[pos];
            do {
                c = *p++;
                if (c < 0) {                    /* sublist used up: the side's list */
                    side = (t == &awtmstruct);
                    /* an int sum (multiple first) is add reg, offset as in the EXE; list + side * 25 loads the offset */
                    if (pos == 6 || line < 6) {
                        if (pos == 1 || pos == 2) q = (signed char *)(side * 25 + (int)unk_E9D50);
                        else q = (signed char *)(side * 25 + (int)byte_E9E4A);
                    } else {
                        if (pos == 1 || pos == 2) q = (signed char *)(side * 25 + (int)byte_E9DB4);
                        else q = (signed char *)(side * 25 + (int)unk_E9EE0);
                    }
                    k = 0;
                    do {
                        c = q[k];
                        k++;
                        if (c < 0) {            /* list used up: any roster index from 18h down */
                            k = 0x18;
                            do {
                                c = k;
                                if (--k < 0) goto next;
                            } while (!TryAddPlayerToList(t, c, i));
                        }
                    } while (!TryAddPlayerToList(t, c, i));
                    break;
                }
                c = lines[c];
            } while (!TryAddPlayerToList(t, c, i));
        }
next:
        if (pos == 6) t->tmextra = PlList[i];
    }
}

/* setpersonel: set the personnel of team t: clear newpos /
 newpnum of the team's 6 sort objects, build PlList (SetPlList: pnum per wanted player, -1 none, the new
 positions at PlList+6 = byte_E038A). Pass 1: a wanted player already on the ice keeps his sort object (newpos
 from the list, newpnum = pnum). Pass 2: each remaining wanted player takes the last free sort object (newpnum
 < 0), stopping at the first one with a position; a sort object without a position (off the ice) first gets
 assignment assbenchwait and position 5. Used entries of PlList are set to -1. */
void setpersonel(Team *t)
{
    Player *p;
    Player *q;
    short i;
    short k;
    short pn;

    i = 6;
    p = t->tmsort;
    do {
        p->newpos = -1;                         /* st newpos(a3) */
        p->newpnum = -1;
        p++;
    } while (--i != 0);
    SetPlList(t);
    i = 5;
    do {                                        /* pass 1: players already on the ice */
        pn = PlList[i];
        if (pn >= 0) {
            p = t->tmsort;
            for (k = 0, p--; k < 6; k++) {
                p++;
                if (p->pnum == pn) {            /* find player pn */
                    p->newpos = byte_E038A[i];
                    p->newpnum = pn;
                    PlList[i] = -1;
                    break;
                }
            }
        }
    } while (--i != (short)-1);
    i = 5;
    do {                                        /* pass 2: the rest take free sort objects */
        pn = PlList[i];
        if (pn >= 0) {
            p = t->tmsort;
            for (k = 0, p--; k < 6; k++) {
                p++;
                if (p->newpnum < 0) {
                    q = p;
                    if (p->position >= 0) break;
                }
            }
            if (q->position < 0) {
                assreplace(q, ASSbenchwait);
                q->position = 5;
            }
            q->newpos = byte_E038A[i];
            q->newpnum = pn;
            PlList[i] = -1;
        }
    } while (--i != (short)-1);
}

/* StartPer: start a period. PC order: no line change
 request, InitCoachModes, setupice, ResetClock; the controlled player of pad 1 (cont1team 1 home: 2, 2 away:
 8) and pad 2 (the one before pad 1's on the same team); no banner overlay, no penalty shot, faceoff at
 centre ice (fox = foy = 0, puck assignment puckfaceoff); crowd quiet unless in the regular season (gsp 0)
 gmode2 bit 2, sflags bits 4 (sfwrap) and 6 cleared, replay recording restarts; two DoGameFrame before the
 loop; CwdExciteLvl 10h; sflags3 bit 7 before the playoffs (gsp < 2); puck uncontrolled, camera at 0, fade
 in, rink scroll positions + 3E8h. */
void StartPer(void)
{
    short t1;
    short t2;

    lcrequest[1] = lcrequest[0] = 0;            /* no line change request */
    InitCoachModes();
    dword_E9A9E = 0;
    setupice();
    ResetClock();
    t1 = cont1team;
    if (t1 == 1) c1playernum[0] = 2;            /* home: player 2 */
    else if (t1 == 2) c1playernum[0] = 8;       /* away: player 8 */
    t2 = cont2team;
    if (t2 != 0) {
        if (t2 == cont1team) c2playernum[0] = c1playernum[0] - 1;
        else if (t2 == 1) c2playernum[0] = 2;
        else if (t2 == 2) c2playernum[0] = 8;
    }
    byte_E9AD3 = 0xFF;
    bannermsg = -1;
    bannertimer = 0;
    ovltimer = -1;
    penshotmode = 0;
    penshotlive = 0;
    penshotstart = 0;
    shotontarget = 0;
    shotongoal = 0;
    penshotplayer = -1;
    fox = 0;                                    /* face off at center ice */
    foy = 0;
    assreplace((Player *)puckstruct, ASSpuckfaceoff);
    if (gsp != 0) crowdlevel = 0;
    gmode2 |= 4;
    sflags &= ~0x50;                            /* sfwrap: reset replay stuff */
    recbpr = replaystart;
    ReplayRecordReset();
    lastsfx = -1;
    DoGameFrame();                              /* run two frames before the loop */
    DoGameFrame();
    CwdExciteLvl = 0x10;
    if (gsp < 2) sflags3 |= 0x80;
    *puckc = -1;
    camy = camx = yleader = yc1 = xc1 = 0;
    fadeinpending = 1;
    rinkscrollx += 1000;
    rinkscrolly += 1000;
}

/* ResetBench: Per team (home, then visitors): PBnum[team] = the players
 still in the box (tmpdst > 0; a time with bit 1000h set counts only while its low 11 bits are nonzero)
 every other player not on the ice or in a pending state (tmpdst > -3) goes back to the bench
 (tmpdst -2, roster status byte 3). */
void ResetBench(void)
{
    short i;
    Team *tm;
    short t;

    t = 0;
    tm = &hmtmstruct;
    for (; t < 2; t++) {
        PBnum[t] = 0;
        for (i = 27; i >= 0; i--) {
            if (tm->tmpdst[i] > 0) {
                if (!(HIBYTE(tm->tmpdst[i]) & 0x10) || (tm->tmpdst[i] & 0x7FF)) PBnum[t]++;
            } else if (tm->tmpdst[i] > -3) {
                tm->tmpdst[i] = PDbench;        /* on bench */
                tm->tmroster[i * 0x27] = 3;     /* roster status: bench */
            }
        }
        tm = &awtmstruct;
    }
}

/* updateanim (5CAEF; 93G hockey93 updateanim, "frame switch control on struct a3"; ends in calcpuckcross_ret5):
   step p's animation. No SPA: unlock (pfalock, pf2aip) and stop. The SPA's table (SPAtab + SPA; attribute word
   first, bit 15 = loops) gives per facing (facedir; on the horizontal rink, sflags bit 7, 2 less for the skaters
   and object 10h; mirrored when the x-flip attribute bit 3 is set) a list of frame / time pairs. When the time
   (SPAcnt) runs out, step to the next pair; at the end of the list start over, unlock and (not looping) clear the
   SPA; PC: SPAs 1027h-1055h then put the player at x -A0h facing 2, locked, in SPA 7A1h. The frame changes at most
   every 4 ticks (glitch, +46h); PC: some new frames (skate strides) play sfx B3h. p: the object. No result. */
void updateanim(Player *p)
{
    short d;
    short *t;
    short loop;
    short fr;
    short *e;
    int c;

    if (p->SPA == 0) {
        p->pflags &= ~pfalock;
        p->pflags2 &= ~pf2aip;
        return;
    }
    d = p->facedir;
    if ((sflags & 0x80) && (p->SCnum < 0xC || p->SCnum == 0x10)) d = d - 2 & 7;
    if (p->attribute & 8) d = 8 - d & 7;
    t = (short *)SPAtab + p->SPA;
    loop = *t & 0x8000;
    if (d) c = t[d];
    else c = *t & 0x7FFF;
    t = (short *)((char *)t + (c * 2 + 0x10));
    d = p->SPAnum;
    e = &t[d];
    fr = *e;
    if (p->SPAcnt >= 0) {
        if (--p->SPAcnt >= 0) goto frame;
        p->SPAnum += 2;
        d += 2;
        if (t[d - 1] < 0) {
            p->SPAnum = 0;
            d = p->SPAnum;
            p->pflags &= ~pfalock;
            p->pflags2 &= ~pf2aip;
            if (p->SPA >= 0x1027 && p->SPA <= 0x1055) {
                p->Yvel = 0;
                p->Xvel = p->Yvel;
                HIWORD(p->Xpos) = -0xA0;
                p->facedir = 2;
                p->pflags |= pfalock;
                SetSPA(p, 0x7A1);
                return;
            }
            if (!loop) p->SPA = loop;
        }
        d = t[d + 1];
        if (d < 0) d = -d;
        p->SPAcnt = d;
    } else {
        d = e[1];
        if (d < 0) d = -d;
        p->SPAcnt = d;
    }
frame:
    if (--((signed char *)p)[0x46] >= 0) return;
    ((signed char *)p)[0x46] = 0;
    if (fr == p->frame) return;
    p->frame = fr;
    if ((fr >= 0x112 && fr <= 0x14E && !((fr - 0x112) & 3)) || (fr >= 0x207 && fr <= 0x215 && (fr & 1))
        || (fr >= 0x315 && fr <= 0x32A && !(fr % 3)) || (fr >= 0x38A && fr <= 0x3AD && !((fr - 0x38A) % 5))
        || fr == 0x36F || fr == 0x373 || fr == 0x377 || fr == 0x3BF || fr == 0x3C3 || fr == 0x3CE || fr == 0x3D1)
        sfx(0xB3);
    ((signed char *)p)[0x46] = 4;
}

/* clockcont_0 (5D852, 93G clockcont_0; ends in calcpuckcross_ret5): end of a period, also entered from
   puckfaceoff. The puck gets assignment 18h. Periods 1-2 (gsp < 2) just end the period (AddPenalty2 type 1 on the
   puck). Later: regd0 = 7 (score), 8 when the result clinches the cup (IsCupClinched); regd1 = hmscore - awscore.
   A tie in overtime (gsp 3) with fullot on ends the game: clear the penalty buffer, crowd up 384h (to at most
   4B0h), gmode bits 0 and 6, excitement +28h, all 12 players off the joysticks, AddPenalty2 type 2; any other tie
   ends the period (type 1). A decided game: both controlled players and the puck carrier cleared, a puck still in
   assignment 18h gets 1Ah, all players off the joysticks, SetBoxDoorObject(away team leads). For the cup:
   byte_CCCA0 = 1, gmode bit 7, sortobj15 gets assignment 2Bh and the winning team's skater nearest (-91h, 6) is
   picked. Then the winning team's six: skaters celebrate (7; the picked one 8 then 2Ah), the goalie stays, an empty
   slot takes the first player of the team's penalty box list (position 5, 3 for the next ones; assignment 7,
   setplayer, the list shifted down). Then the penalty buffer is cleared, the crowd raised (home win: +3E8h to at
   most 5DCh, +320h more for the cup; else at least 320h), gmode bits 0 and 6, excitement +28h, and AddPenalty2
   type 2 (p past the six). No result. */
void clockcont_0(void)
{
    Player *p;
    Team *t;
    short c;
    short best;
    int d;
    int dmin;
    int dx;
    int dy;

    assinsert(PUCK, 0x18);
    if (gsp < 2) {
        AddPenalty2(PUCK, 1);
        return;
    }
    regd0.w = 7;
    if (IsCupClinched(hmscore[0], awscore)) regd0.w = 8;
    p = SortCords;
    if ((regd1.w = hmscore[0] - awscore) == 0) {
        if (gsp == 3 && gameopts.fullot) {
            ClearPenaltyBuffer();
            if (crowdlevel <= 0x4B0) {
                crowdlevel += 0x384;
                if (crowdlevel > 0x4B0) crowdlevel = 0x4B0;
            }
            gmode |= 0x41;
            regd2.w = 0;
            CwdExciteLvl += 0x28;
            for (; regd2.w < 12; regd2.w++) SortCords[regd2.w].pflags &= ~pfjoy;
            AddPenalty2(p, 2);
        } else {
            AddPenalty2(p, 1);
        }
        return;
    }
    c1playernum[0] = c2playernum[0] = -1;
    *puckc = -1;
    d = (int)PUCK + PUCK->assnum;               /* the address through d: add eax, puckstruct as in the EXE */
    if (((signed char *)d)[0x1E] == 0x18) assreplace(PUCK, 0x1A);   /* asslist[assnum] */
    for (regd2.w = 0; regd2.w < 12; regd2.w++) SortCords[regd2.w].pflags &= ~pfjoy;
    p = &SortCords[regd1.w < 0 ? 6 : 0];
    SetBoxDoorObject(regd1.w < 0);
    if (regd0.w == 8) {
        byte_CCCA0 = 1;
        gmode |= 0x80;
        assinsert((Player *)sortobj15, 0x2B);
        dmin = 40000;
        best = p->SCnum;
        for (regd2.w = 0; regd2.w < 6; regd2.w++, p++) {
            if (p->position > 0) {
                dx = HIWORD(p->Xpos) + 0x91;
                dy = HIWORD(p->Ypos) - 6;
                dx *= dx;                           /* squared first: the EXE's register order */
                d = dx + dy * dy;
                if (d <= dmin) {
                    dmin = d;
                    best = p->SCnum;
                }
            }
        }
    }
    p = &SortCords[regd1.w < 0 ? 6 : 0];
    shotplayer = -1;
    regd1.w = 0;
    regd2.w = 6;
    do {
        if (p->position > 0) {
            if (regd0.w == 8 && best == p->SCnum) {
                assinsert(p, 8);
                assinsert(p, 0x2A);
            } else {
                assinsert(p, 7);
            }
        } else if (p->position < 0) {
            t = p->tmptr;
            c = t->tmpbox[0];
            if (c >= 0) {
                p->position = regd1.w ? 3 : 5;
                regd1.w++;
                assinsert(p, 7);
                setplayer(p, c);
                for (c = 1; ; c++) if ((t->tmpbox[c - 1] = t->tmpbox[c]) < 0) break;
            }
        }
        p++;
    } while (--regd2.w != 0);
    ClearPenaltyBuffer();
    if (hmscore[0] > awscore) {
        if (crowdlevel <= 0x5DC) {
            crowdlevel += 0x3E8;
            if (crowdlevel > 0x5DC) crowdlevel = 0x5DC;
        }
        if (regd0.w == 8) crowdlevel += 0x320;
    } else if (crowdlevel < 0x320) {
        crowdlevel = 0x320;
    }
    gmode |= 0x41;
    CwdExciteLvl += 0x28;
    AddPenalty2(p, 2);
}
