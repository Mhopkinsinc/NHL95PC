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
