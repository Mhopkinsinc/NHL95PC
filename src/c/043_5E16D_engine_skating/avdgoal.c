/* Engine skating: keep a skater's target out of the goals. */
/* DRAFT: first pass; see docs/UNSURE.md. It ends in EvadePlayers_popebp, so its match belongs in EvadePlayers.c. */
#define avdgoal avdgoal_decl
#define avdgoal_box avdgoal_box_decl
#include "nhl95.h"
#undef avdgoal
#undef avdgoal_box

int avdgoal_box(Player *p, int x1, int x2, int y, int *ps, int *pt);

/* the goalie's SortCords number in a team block (the word at +FAh) */
#define TMGOALIE(t) (*(short *)((char *)(t) + 0xFA))

/* avdgoal (5F151, 93G avdgoal, rewritten on the PC; ends in EvadePlayers_popebp): don't skate through a goal. p
   skates to the target in regd0 / regd1 (x / y, changed in place). A goalie or a player on assignment 27h uses a
   tighter box (margin -6, else 0Ah). The referee with the clock stopped keeps a target in front of the goal on his
   own side. avdgoal_box tests the line to the target against the goal at +E8h (then the y values are mirrored
   and the side bits 1 / 4 swapped) and else the one at -E8h; nothing is changed when it crosses neither. ps / pt
   are the side bits of p and the target (1 behind the goal line, 4 in front, 2 / 8 right / left of the posts):
   with the target in the box, it moves beside the posts (x +-(2Ch + margin)) and to the goal mouth (yl) or
   behind the net (yb), round the goalie (gp) when the team has one; else, by p's side, a corner case of a
   13-way switch moves the target to a post side, the goal mouth or behind the net. No result. */
void avdgoal(Player *p)
{
    int pt;
    int ps;
    int c3;
    int c1;
    int s1;
    int gl;
    int py;
    int ty;
    int tight;
    Player *gp;
    int g;
    int tx;
    int c2;
    int m;
    int px;
    int yl;
    int yb;

    tight = p->position == 0 || p->asslist[p->assnum] == 0x27 ? 1 : 0;
    m = tight ? -6 : 0xA;
    if ((gmode & 1) && p->SCnum == 0x10 && ABS(regd1.w) < 0xE8 && (HIWORD(p->Ypos) ^ regd1.w) >= 0) return;
    px = HIWORD(p->Xpos);
    py = HIWORD(p->Ypos);
    tx = regd0.w;
    ty = regd1.w;
    if (avdgoal_box(p, 0xE8 - (tight ? 6 : 0x23), 0xFC, (tight ? -8 : 0) + 0x2C, &ps, &pt)) {
        yb = 0xFD;
        yl = 0xE8 - (tight ? 7 : 0x24);
        py = -py;
        ty = -ty;
        gl = 0xE8;
        g = TMGOALIE(p->pflags & 0x80 ? p->optmptr : p->tmptr);
        if (g >= 0) gp = &SortCords[g];
        if (*(unsigned char *)&ps & 5) *(unsigned char *)&ps ^= 5;
        if (*(unsigned char *)&pt & 5) *(unsigned char *)&pt ^= 5;
    } else {
        if (!avdgoal_box(p, -0xFC, (tight ? 6 : 0x23) - 0xE8, (tight ? -8 : 0) + 0x2C, &ps, &pt)) return;
        yb = -0xFD;
        yl = (tight ? 7 : 0x24) - 0xE8;
        gl = -0xE8;
        g = TMGOALIE(p->pflags & 0x80 ? p->tmptr : p->optmptr);
        if (g >= 0) gp = &SortCords[g];
    }
    if (pt == 0) {
        if (py >= -0xE8) {
            if (tight) {
                if (ty < -0xE8) return;
                regd1.w = yl;
                if (tx >= 0) {
                    if (m + 0x2C > tx) regd0.w = m + 0x2C;
                } else if (-0x2C - m + 1 <= tx) regd0.w = -0x2C - m;
                if (ABS(px) > 0x26) regd1.w = yb;
                return;
            }
            if (ty < -0xE8) {
                regd1.w = yl;
                if (g >= 0 && ((tx - HIWORD(gp->Xpos)) ^ (px - tx)) < 0) {
                    if (px < HIWORD(gp->Xpos)) regd0.w = -0x2C - m;
                    else regd0.w = m + 0x2C;
                } else {
                    if (ABS(px) >= m + 0x2C) return;
                    if (px < 0) regd0.w = -0x2C - m;
                    else regd0.w = m + 0x2C;
                }
                if (ABS(px) > 0x26) regd1.w = yb;
                return;
            }
            if (g < 0) return;
            if (px < tx) {
                if (px >= HIWORD(gp->Xpos)) return;
            } else if (px < HIWORD(gp->Ypos)) return;
            regd1.w = yl;
            return;
        }
        if (ty < -0xE8) return;
        if (px < -0x2C - m + 1) regd0.w = -0x2C - m;
        else if (px >= m + 0x2C) regd0.w = m + 0x2C;
        else {
            regd1.w = yb;
            if (tx >= 0) {
                if (m + 0x2C > tx) regd0.w = m + 0x2C;
            } else if (-0x2C - m + 1 <= tx) regd0.w = -0x2C - m;
        }
        if (ABS(px) > 0x26) regd1.w = yl;
        return;
    }
    s1 = m + 0x2C;
    c1 = m + 0x2C;
    c2 = -0x2C - m;
    c3 = -0x2C - m + 1;
    switch (ps) {
    case 0:
        if (py < -0xE8) {
            if (px < -0x2C - m + 1) regd0.w = -0x2C - m;
            else if (px >= m + 0x2C) regd0.w = m + 0x2C;
            else {
                regd1.w = yb;
                if (tx >= 0) {
                    if (m + 0x2C > tx) regd0.w = m + 0x2C;
                } else if (-0x2C - m + 1 <= tx) regd0.w = -0x2C - m;
            }
            return;
        }
        if (tight) {
            if (pt & 1) return;
            regd1.w = yl;
            if (tx >= 0) {
                if (m + 0x2C > tx) regd0.w = m + 0x2C;
            } else if (-0x2C - m + 1 <= tx) regd0.w = -0x2C - m;
            return;
        }
        if (ty >= -0xE8) {
            if (g >= 0 && ((tx - HIWORD(gp->Xpos)) ^ (px - tx)) < 0) {
                if (px < HIWORD(gp->Xpos)) regd0.w = -0x2C - m;
                else regd0.w = m + 0x2C;
            } else if (ABS(px) < m + 0x2C) {
                if (px < 0) regd0.w = -0x2C - m;
                else regd0.w = m + 0x2C;
            }
            if (ABS(px) > 0x26) regd1.w = yb;
            return;
        }
        regd1.w = gl;
        if (g >= 0) {
            if (px < HIWORD(gp->Ypos)) regd0.w = -0x2C - m;
            else regd0.w = m + 0x2C;
        } else if (tx >= 0) {
            if (m + 0x2C > tx) regd0.w = m + 0x2C;
        } else if (-0x2C - m + 1 <= tx) regd0.w = -0x2C - m;
        return;
    case 1:
        regd1.w = yl;
        if (pt & 4) return;
        if (tx >= 0) {
            if (tx < c1) regd0.w = s1;
        } else if (tx >= c3) regd0.w = c2;
        return;
    case 2:
        regd0.w = s1;
        if (pt & 5) return;
        regd1.w = yl;
        return;
    case 3:
        if (pt == 8) regd1.w = yl;
        else regd0.w = s1;
        return;
    case 4:
        regd1.w = yb;
        if (!(pt & 1)) return;
        if (tx >= 0) {
            if (tx < c1) regd0.w = s1;
        } else if (tx >= c3) regd0.w = c2;
        return;
    case 6:
        if (pt != 8) regd0.w = s1;
        else regd1.w = yb;
        return;
    case 8:
        regd0.w = c2;
        if (!(pt & 5)) return;
        regd1.w = yl;
        return;
    case 9:
        if (pt == 2) regd1.w = yl;
        else regd0.w = c2;
        return;
    case 12:
        if (pt == 2) regd1.w = yb;
        return;
    }
}
