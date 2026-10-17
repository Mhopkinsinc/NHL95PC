/* Engine physics AI: is the pass lane to a team-mate open. */
/* DRAFT: 344 vs 346. The side copy is mov esi,edi (original mov edx,edi), so the team-mate's
 y stays in edx. The other gap is inc esi where the original does lea eax,[esi+1].
 Do not retry: declaring limit first produced movzx edi; declaring oy before side did not move the copy to edx. */
#define PassLaneChk PassLaneChk_decl
#include "nhl95.h"
#undef PassLaneChk

#define W(v) (*(int *)((char *)&(v) - 2) >> 16)

/* ShotLaneOpen saves edx and ecx. The header declares it void, so this alias is what
 lets the caller test eax and keep edx. */
int LaneOpen(void);
#pragma aux LaneOpen "ShotLaneOpen" value [eax] modify [eax];

/* PassLaneChk: 1 when team-mate o is ahead of p toward p's goal. p's position must be set and
 ShotLaneOpen must report the lane clear. o is deeper than p, inside the 4Eh line, within 3Ch in
 y and 28h in x, both skate toward that goal, and the pass direction is within a step of the way
 o is travelling (or the way it faces, when that direction is 8). */
int PassLaneChk(Player *p, Player *o)
{
    int limit;
    int side;
    int y;
    int oy;
    int dy;
    int ang;
    int wrapped;

    side = (p->pflags & pfgoal) != 0;
    if (p->position == 0) return 0;
    if (LaneOpen()) return 0;
    oy = o->Ypos >> 16;
    y = p->Ypos >> 16;
    if (side) limit = 0x4E;
    else limit = -0x4E;
    if ((y > limit) ^ side) return 0;
    if (ABS(y) <= ABS(oy)) return 0;
    if (ABS(y) > 0x76) return 0;
    if (side) {
        if (o->Yvel < 0) return 0;
        if (p->Yvel < 0) return 0;
    }
    if (!side) {
        if (o->Yvel > 0) return 0;
        if (p->Yvel > 0) return 0;
    }
    if (((HIBYTE(o->Yvel) + oy - 0x4E) ^ (y - 0x4E)) < 0) return 0;
    dy = oy - y;
    if (dy < 0) dy = -dy;
    if (dy > 0x3C) return 0;
    if (ABS((p->Xpos >> 16) - (o->Xpos >> 16)) > 0x28) return 0;
    ang = vtoa(W(o->Xvel), W(o->Yvel));
    if (ang == 8) ang = W(o->facedir);
    ang -= vtoa((short)(HIWORD(p->Xpos) - HIWORD(o->Xpos)), (short)(HIWORD(p->Ypos) - HIWORD(o->Ypos)));
    wrapped = ang + 1;
    if ((wrapped & 7) >= 3) return 0;
    return 1;
}

/* NetCollide (5601D, PC only; ends in PassLaneChk_ret5): player p near a net (60h, CAh from centre on his side):
   unless he is at least 18h from the net centre nothing happens; else his unit offset (x4) picks the side he
   hits: front or back (facing within the range for that side, dir + 2 / + 1 & 7) or the posts (dir 5+ / 1-3);
   he is snapped to the net edge for his facing (offset tables word_CCBEC, word_CCBFC, word_CCBCC, word_CCBDC;
   mirrored by the x-flip attribute bit 3), stopped, and the bump SPA for that side is returned (F0Fh, F55h,
   F9Bh / FE1h). p: the player; dir: his direction. Result: the new SPA, or his SPA when nothing changes.
   DRAFT: 714 vs 700 (147 of 200 instructions). The original keeps dir in ebp, the net y in edi and dx / dy / net x
   on the stack; the first x-flip test materializes a bool (sete). See docs/UNSURE.md. */
int NetCollide(Player *p, int dir)
{
    int gx;
    int dx;
    int dy;
    int gy;
    int dist;
    int ux;
    int uy;

    gx = HIWORD(p->Xpos) >= 0 ? 0x60 : -0x60;
    dx = (p->Xpos >> 16) - gx;
    gy = HIWORD(p->Ypos) >= 0 ? 0xCA : -0xCA;
    dy = (p->Ypos >> 16) - gy;
    dist = vecdist(dx, dy);
    if (dist < 0x18) goto keep;
    ux = (dx << 8) / dist >> 2;
    uy = (dy << 8) / dist >> 2;
    if ((uy < 0 ? -uy : uy) >= (ux < 0 ? -ux : ux)) {
        if (gy >= 0) {
            dir = dir + 2 & 7;
            if (dir < 5) goto keep;
            HIWORD(p->Xpos) = gx + ux;
            if (gx < 0 && (p->attribute & 8) == 0) HIWORD(p->Xpos) += 6;
            if (gx > 0 && (p->attribute & 8)) HIWORD(p->Xpos) -= 6;
            HIWORD(p->Ypos) = uy + gy - 8;
            p->Xvel = 0;
            p->Yvel = 0;
            HIWORD(p->Ypos) += word_CCBEC[p->facedir] - 0x108;
            return 0xF0F;
        }
        dir = dir + 1 & 7;
        if (dir >= 3) goto keep;
        HIWORD(p->Xpos) = gx + ux;
        HIWORD(p->Ypos) = uy + gy;
        p->Xvel = 0;
        p->Yvel = 0;
        HIWORD(p->Ypos) += word_CCBFC[p->facedir] + 0x108;
        return 0xF55;
    }
    if (gx >= 0) {
        if (dir < 5) goto keep;
        ux -= 6;
        HIWORD(p->Xpos) = gx + ux;
        HIWORD(p->Ypos) = uy + gy + (gy > 0 ? -10 : 0);
        p->Xvel = 0;
        p->Yvel = 0;
        if (p->attribute & 8) HIWORD(p->Xpos) -= word_CCBDC[8 - p->facedir & 7] + 0x9A;
        else HIWORD(p->Xpos) += word_CCBCC[p->facedir] - 0x9A;
        if (p->attribute & 8) return 0xFE1;
        return 0xF9B;
    }
    if (dir == 0) goto keep;
    if (--dir >= 3) goto keep;
    ux += 6;
    HIWORD(p->Xpos) = gx + ux;
    HIWORD(p->Ypos) = uy + gy + (gy > 0 ? -10 : 0);
    p->Xvel = 0;
    p->Yvel = 0;
    if (p->attribute & 8) HIWORD(p->Xpos) -= word_CCBCC[8 - p->facedir & 7] - 0x9A;
    else HIWORD(p->Xpos) += word_CCBDC[p->facedir] + 0x9A;
    if (p->attribute & 8) return 0xF9B;
    return 0xFE1;
keep:
    return p->SPA;
}
