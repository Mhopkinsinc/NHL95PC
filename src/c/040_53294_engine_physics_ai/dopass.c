/* Engine physics AI: passing (dopass) and the shot aim at the open side (shotdiradj). */
/* DRAFT: both are first passes; see docs/UNSURE.md. Their match belongs in checkob.c: dopass ends in
   checkob_ret5 and shotdiradj in dopass_ret6. Appended to checkob.c as they are, they move the shared pops
   (burst then branches into dopass), so they stay in their own file for now. */
#define dopass dopass_decl
#define shotdiradj shotdiradj_decl
#define PassLaneChk PassLaneChk_decl
#define passtoa0 passtoa0_decl
#define puckvz puckvz_decl
#define sfx sfx_decl
#include "nhl95.h"
#undef dopass
#undef shotdiradj
#undef PassLaneChk
#undef passtoa0
#undef puckvz
#undef sfx

int PassLaneChk(Player *p, Player *to);
void passtoa0(Player *to);
void sfx(int n);
extern int puckvz;

/* dopass (54DF4, 93G dopass; ends in checkob_ret5): pass the puck in passdir. Clear the one-timer flag and the
   spin flag, drop the puck (nopuck 10h, lastplayer). A computer player with a pass lane result (passlane) scatters
   the pass to its target. Otherwise the pass speed is A0h + 4 * passacc (8 for a goalie), and the best teammate
   is the closest one (squared distance, plus 10000h unless straight ahead) within one step of passdir, skipping
   p, goalies and unavailable players (pflags2 bit 2): a human player's pass checks the lane (PassLaneChk) and
   scatters when it is blocked, else the target gets the pass (passtoa0); with no target the puck goes along
   passdir at the pass speed plus p's velocity, with a random lift. A goalie's pass is turned up the ice (puckvy
   sign by side); with a human goalie the controller moves to the receiver (restorepl) or, without one, to the
   next player (changeplayer). Then the swing animation (goalie 1B9h, else Findhittype: 3C1h or 389h), pflags
   bit 5 and the pass sound (98h, 99h for a high pass). p: the passer. No result. */
void dopass(Player *p)
{
    struct {
        int t;
        unsigned best;
        short w;
    } s;
    Player *o;
    Player *target;
    int dx;
    int dy;
    short d;
    int sp;

    onetimerflag = 0;
    sflags &= 0xFB;
    *puckc = -1;
    p->nopuck = 0x10;
    lastplayer = p->SCnum;
    if (!(p->pflags & 8) && p->passlane) {
        ScatterPass(p, &SortCords[((short *)p)[0x24]]);
        return;
    }
    if (p->position == 0) s.w = 8;
    else s.w = p->passacc;
    passspeed = (s.w << 2) + 0xA0;
    o = &SortCords[p->SCnum < 6 ? 0 : 6];
    s.best = -1;
    target = 0;
    s.w = 6;
    do {
        if (p != o && o->position > 0 && !(o->pflags2 & 4)) {
            dx = (short)(HIWORD(o->Xpos) - *puckx);
            dy = (short)(HIWORD(o->Ypos) - *pucky);
            d = (vtoa(dx, dy) - passdir) & 7;
            if (d <= 1 || d == 7) {
                s.t = dx * dx + dy * dy + (!d ? 0 : 0x10000);
                if (s.t <= s.best) {
                    s.best = s.t;
                    target = o;
                }
            }
        }
        o++;
    } while (--s.w);
    if (target) {
        if ((p->pflags & 8) && PassLaneChk(p, target)) {
            ScatterPass(p, target);
            return;
        }
        passtoa0(target);
    } else {
        sp = passspeed;
        s.t = dirtab_y[passdir * 2] * sp << 10;
        s.t = s.t / 0xBB8;
        *puckvy = s.t + p->Yvel;
        s.t = dirtab[passdir * 2] * sp << 10;
        s.t = s.t / 0xBB8;
        *puckvx = s.t + p->Xvel;
        *(short *)puckvz = randomd0(0x1000);
    }
    if (p->position == 0) {
        if (((p->pflags & 0x80) == 0) ^ (*puckvy < 0)) *puckvy = -*puckvy;
        s.w = 0x1B9;
        p->pflags2 |= 2;
        if (p->pflags & 8) {
            if (target && !(target->pflags & 8)) {
                if (p->SCnum == c1playernum[0]) {
                    if (c1playernum[0] != target->SCnum) c1playernum[0] = restorepl(target->SCnum, c1playernum[0]);
                } else {
                    if (target->SCnum != c2playernum[0]) c2playernum[0] = restorepl(target->SCnum, c2playernum[0]);
                }
            } else {
                changeplayer(p, p->SCnum == c1playernum[0] ? 0 : 2);
            }
        }
    } else {
        if (!Findhittype(p, vtoa(*puckvx, *puckvy))) s.w = 0x389;
        else s.w = 0x3C1;
    }
    SetSPA(p, s.w);
    p->pflags |= 0x20;
    sfx((short)(((signed char *)puckvz)[1] >= 6 ? 0x99 : 0x98));
}

/* shotdiradj (57A98, 93G shotdiradj; ends in dopass_ret6): a human shooter aims at the side of the net the
   goalie leaves open: from the opposing goalie's position plus half its velocity (relative to the puck), the
   offsets of the two posts (x +-12h at the goal line, E8h / -E8h by side) from the puck-to-goalie line, divided
   by the goalie's distance; beyond 2Ch in total passdir 0 (straight), else 2 or 6 by the sign (mirrored by
   side). No goalie: passdir 8. Computer players are skipped. p: the shooter. No result. */
void shotdiradj(Player *p)
{
    struct {
        int x0;
        int cross;
        int gdx;
        short dist;
    } s;
    Player *g;
    short i;
    int gdy;
    short gy;
    short postx;
    short a;
    int q;

    if (p->pflags & 8) return;
    g = (Player *)(((p->SCnum < 6 ? 6 : 0) << 7) + (int)SortCords);
    for (i = 0; i < 6; i++, g++) {
        if (g->position == 0) break;
    }
    if (i == 6) {
        passdir = 8;
        return;
    }
    s.gdx = (short)((g->Xvel >> 9) + (g->Xpos >> 16) - *puckx);
    gdy = (short)((g->Yvel >> 9) + (g->Ypos >> 16) - *pucky);
    s.dist = vecdist(s.gdx, gdy);
    if (s.dist == 0) s.dist = 1;
    postx = 0x12;
    gy = 0xE8;
    if (!(p->pflags & 0x80)) gy = -0xE8;
    s.x0 = postx;
    s.cross = s.gdx * (gy - *pucky);
    a = (s.cross - (s.x0 - *puckx) * gdy) / s.dist;
    a += (s.cross - (-s.x0 - *puckx) * gdy) / s.dist;
    if (ABS(a) > 0x2C) {
        passdir = 0;
        return;
    }
    if (p->pflags & 0x80) a = -a;
    if (a >= 0) passdir = 2;
    else passdir = 6;
}
