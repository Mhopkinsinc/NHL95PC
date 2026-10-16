/* Engine physics AI: offside banner. */
/* DRAFT: 424 vs 424. The far-side player test is cmp ecx,esi / jg; the original is
 cmp esi,ecx / jl. Do not retry: writing the limit on the left of < hoists that lea
 out of the loop. */
#define GetLowestPen GetLowestPen_decl
#include "nhl95.h"
#undef GetLowestPen

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

/* SkillForAnim: the rating byte that decides p's current save move animation (SPA): the left right pairs
 passacc aggress (SPA C9h, E1h, 11D5h; B1h the other way round) and spodds endurance (119h; 101h and
 121Dh the other way round) are picked by handedness; any other animation uses shotspd.
 It sits between checkob and GetLowestPen so GetLowestPen's jump back to checkob's pops is a near jmp. */
int SkillForAnim(Player *p)
{
    switch (p->SPA) {
    case 0xC9:
    case 0xE1:
    case 0x11D5:
        return !p->handed ? p->passacc : p->aggress;
    case 0xB1:
        return !p->handed ? p->aggress : p->passacc;
    case 0x119:
        return !p->handed ? p->endurance : p->spodds;
    case 0x101:
    case 0x121D:
        return !p->handed ? p->spodds : p->endurance;
    default:
        return p->shotspd;
    }
}

/* DRAFT: 166 vs 166. The trail step is sub eax,ecx / cwde / add ebx,eax; the original
 has no cwde. The power-play return is lea eax,[ebx+ecx]; the original is
 mov eax,ebx / add eax,ecx. Do not retry: a named int for the step pushes ebp,
 an unsigned short store is and eax,0FFFFh, and pen-prev as an expression moves
 the box pointer from edx to eax. */
int GetLowestPen(void)
{
    Team *lead;
    Team *trail;
    short diff;
    short idx;
    short pen;
    int sum;
    int prev;
    signed char *box;

    diff = hmtmap[0] - awtmap;
    if (diff > 0) {
        lead = &hmtmstruct;
        trail = &awtmstruct;
    } else if (diff < 0) {
        lead = &awtmstruct;
        trail = &hmtmstruct;
    } else {
        return 0;
    }
    prev = 0;
    sum = 0;
    box = trail->tmpbox;
    for (;;) {
        idx = *box++;
        if (idx < 0)
            break;
        pen = trail->tmpdst[idx];
        if (pen < 0)
            continue;
        pen &= 0x3FFF;
        pen -= prev;
        sum += pen;
        prev = pen;
    }
    if (lead->tmap != 6)
        goto box2;
    return sum;
box2:
    sum -= prev;
    box = lead->tmpbox;
    for (;;) {
        idx = *box++;
        if (idx < 0)
            return sum;
        pen = lead->tmpdst[idx];
        if (pen < 0)
            continue;
        pen &= 0x3FFF;
        if ((short)sum < pen)
            return sum;
        return sum + prev;
    }
}

/* PC coach-mode settings in the Team structure (InitCoachModes / SetCoachMode write them as byte_DF6E8 /
   dword_DF6EA for home, DF7E8 / DF7EA for away): +0D4h flags (bit 6: the coach changes lines on the clock only),
   +0D6h the line energy limit for a computer line change (CCCh, D9Ah when trailing; 93G used a fixed C00h). */
#define TMCOACHFLAGS(t) (((unsigned char *)(t))[0xD4])
#define TMLCENERGY(t)   (*(unsigned *)((unsigned char *)(t) + 0xD6))

/* chk4lc (54AF9; 93G logic93 chk4lc, "see if computer should call line change"): for the computer puck carrier p
   with line changes on and no penalty shot: the puck in p's own half below the blue line (pucky, negated when p
   shoots at the bottom goal, 0..4Eh), 1 in 4 (randomd0), not in line change mode (pf2lcm). PC additions: the
   game clock must have 15 or more left; unless the coach flag is set, the current line's average energy (AvgCline
   into regd0) must be under the limit (F33h when the teams' tmap differ, else the coach-mode limit at +0D6h; at
   15..1Dh on the clock also under 800h); when the tmap differ, the shortest penalty (GetLowestPen) must have 15 or
   more left. Then CompLine picks the line, setpersonel sends it and CompShoot dumps the puck (chk4lc_shoot, which
   chk4shot also jumps to). Result: 1 when the change was called, else 0.
   DRAFT: 273 vs 272. gameclock is loaded into di in the energy arm; the original uses ax. See docs/UNSURE.md. */
short chk4lc(Player *p)
{
    Team *t;
    Team *o;
    short d;

    if (!gameopts.linechanges || penshotlive) goto no;
    regd0.w = *pucky;
    if (!(p->pflags & pfgoal)) regd0.w = -regd0.w;
    if (regd0.w < 0 || regd0.w > 0x4E) goto no;
    if (randomd0(4)) goto no;
    if (p->pflags2 & pf2lcm) goto no;
    t = p->tmptr;
    o = p->optmptr;
    d = t->tmap - o->tmap;
    if (!(TMCOACHFLAGS(t) & 0x40)) goto energy;
    if (gameclock < 0xF) goto no;
pen:
    if (!d || (short)GetLowestPen() >= 0xF) goto change;
no:
    return 0;
energy:
    AvgCline(t);
    if (gameclock < 0xF) goto no;
    if (gameclock < 0x1E && (unsigned short)regd0.w > 0x800) goto no;
    if ((unsigned short)regd0.w > (d ? 0xF33 : TMLCENERGY(t))) goto no;
    goto pen;
change:
    CompLine(o, t);
    setpersonel(t);
    CompShoot(p);
    return 1;
}

/* ReturnGoalies: a pulled goalie (tmgoalie low byte under 10h) goes back in net.
 The pulled menu item is unchecked and the goalie item checked, then a computer
 team may pull again. */
void ReturnGoalies(void)
{
    Team *t;
    Team *o;
    short i;

    i = 0;
    t = &hmtmstruct;
    o = &awtmstruct;
    for (; i < 2; i++) {
        if ((((unsigned char *)&t->tmgoalie)[0] & 0xF0) == 0) {
            ((unsigned char *)&t->tmgoalie)[1] = 0;
            *(unsigned char *)((int (*)[3])off_CD4A0)[i][0] = 2;
            *(unsigned char *)((int (*)[3])off_CD498)[i][(short)(t->tmgoalie & 0xF)] = 1;
            if (cont1team != i + 1 && cont2team != i + 1)
                CPgoalie(t, o, *(int *)((char *)&foy - 2) >> 16);
        }
        t = &awtmstruct;
        o = &hmtmstruct;
    }
}

/* The clamped energy is passed through ebx so the team pointer reloads into ecx. */
int burst_bx(int v);
#pragma aux burst_bx = "mov ebx, edx" parm [edx] value [ebx] modify exact [ebx];

/* burst: fatigue on, spend CCh energy (not below 0), then add facedir's dirtab step
 times energy>>7 to Xvel and Yvel, lock the animation, and start SPA 621h. */
void burst(Player *p)
{
    regd0.w = p->tmptr->tmpde[p->pnum];
    if (gameopts.linechanges) {
        regd0.w -= 0xCC;
        p->tmptr->tmpde[p->pnum] = burst_bx(regd0.w < 0 ? 0 : regd0.w);
        if (regd0.w < 0)
            regd0.w = 0;
    }
    regd0.w >>= 7;
    p->Xvel += dirtab[p->facedir * 2] * regd0.w;
    p->Yvel += dirtab_y[p->facedir * 2] * regd0.w;
    p->pflags |= pfalock;
    SetSPA(p, 0x621);
}
