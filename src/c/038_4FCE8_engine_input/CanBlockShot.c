/* Engine input: the defender's hold / shot-block checks (one source block: OppInReach ends in CanBlockShot's
   exits). */
#include "nhl95.h"

/* CanBlockShot (4FFEE, PC only): may player p dive to block a shot? Not when his own team has the puck. The puck
   must be 4Eh-E8h from centre ice, within 50h of him (+4Eh), coming at his goal (puckcross of his side at least
   10), and the shooter (the carrier, or shotplayer of the other team when it is loose and moving at his goal)
   in a shooting SPA. p must be between the puck and his goal (y), facing the shooter, and nearer the goalie
   (SortCords 12 + side) than the puck is; outside line change mode (pflags bit 3) not straight sideways and
   80h+ deep and within 64h of the middle. Result: 0, or the dive direction + 1 (0Ah straight), the direction
   also in the word at +4Ah (holdplayer passes it to BlockShotDive).
   DRAFT: the same-team test returns 0 through `xor eax, edx` after comparing two bools, the unsolved pattern
   in HANDOFF; kept as the owner of the CanBlockShot_ret5 / _ret6 exits. */
int CanBlockShot(Player *p)
{
    int dx;
    int dist;
    int dir;
    int side;
    int y;
    int d;
    int a;
    int b;
    int e;
    int spa;
    Player *g;
    short s;

    if (*puckc >= 0 && (p->SCnum < 6) == (*puckc < 6)) return 0;
    s = *pucky;
    if ((s < 0 ? -s : s) > 0xE8 || (s < 0 ? -s : s) < 0x4E) return 0;
    if (p->puckdist > 0x50) return 0;
    side = (p->pflags & pfgoal) != 0;
    g = &SortCords[side + 12];
    if (puckcross[side ? 3 : 1] < 10) return 0;
    if (*puckc >= 0) s = *puckc;
    else {
        if (side && *puckvy > 0) return 0;
        if (!side && *puckvy < 0) return 0;
        if (shotplayer < 0) return 0;
        if (!((p->SCnum < 6) ^ (shotplayer < 6))) return 0;
        s = shotplayer;
    }
    spa = SortCords[s].SPA;
    if (spa != 0x3F9 && spa != 0x491 && spa != 0xDD3 && spa != 0xE2B && spa != 0x1265 && spa != 0x12DD
        && spa != 0x1355 && spa != 0x138D) return 0;
    dx = *puckx - HIWORD(g->Xpos);
    y = *pucky - HIWORD(g->Ypos);
    dist = vecdist(dx, y);
    dir = vtoa((short)dx, (short)y);
    y = HIWORD(p->Ypos);
    if (!(side && y < -0x4E && y > -0xE8 && y < *pucky)) {
        if (side || y <= 0x4E || y >= 0xE8 || y <= *pucky) return 0;
    }
    dx = HIWORD(p->Xpos) - HIWORD(g->Xpos);
    y -= HIWORD(g->Ypos);
    d = vecdist(dx, y);
    a = vtoa((short)dx, (short)y);
    if (!(p->pflags & pfjoy)) {
        if (a == 2 || a == 6) return 0;
        if ((HIWORD(p->Ypos) < 0 ? -HIWORD(p->Ypos) : HIWORD(p->Ypos)) < 0x80) return 0;
        if ((HIWORD(p->Xpos) < 0 ? -HIWORD(p->Xpos) : HIWORD(p->Xpos)) > 0x64) return 0;
    }
    e = ((signed char *)p)[0x52];
    b = (e - p->facedir + 2) & 7;
    a = (dir - a + 2) & 7;
    if (((e - dir + 1) & 7) > 2) return 0;
    if (!((p->pflags & pfjoy) && b <= 4 && a <= 4)) {
        if (p->pflags & pfjoy) return 0;
        if (b < 1 || a < 1 || b > 3 || a > 3) return 0;
    }
    if (d >= dist) return 0;
    d += p->puckdist;
    if (d >= dist * 2) return 0;
    if (b == 2 && a == b) return 10;
    ((short *)p)[0x25] = dir;
    return a + 1;
}

/* OppInReach (5033D, PC only; ends in CanBlockShot_ret6): is one of the six opponents within reach of p for a
   hold: in the front half of p's facing (vtoa of the offset against facedir) and under 1Eh away in x and "y". The
   original computes both offsets from Xpos (the y offset is the x one again), which this keeps. p: the player.
   Result: 1 or 0.
   DRAFT: 139 vs 144. The original computes the offset in edx and copies it to ebx and ecx (and its exits are near
   jumps); here it is computed in ebx. See docs/UNSURE.md. */
int OppInReach(Player *p)
{
    Player *q;
    int i;
    int dx;
    int dy;
    int t;

    q = &SortCords[p->SCnum < 6 ? 6 : 0];
    i = 0;
    do {
        dx = HIWORD(q->Xpos) - HIWORD(p->Xpos);
        dy = HIWORD(q->Xpos) - HIWORD(p->Xpos);
        t = vtoa((short)dx, (short)dy);
        if (((t - p->facedir + 2) & 7) <= 4) {
            if (dx < 0) dx = -dx;
            if (dx < 0x1E) {
                if (dy < 0) dy = -dy;
                if (dy < 0x1E) return 1;
            }
        }
        i++;
        q++;
    } while (i < 6);
    return 0;
}
