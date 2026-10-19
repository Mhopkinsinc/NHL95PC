/* Engine physics AI: pass to a player. */
/* DRAFT: 684 vs 703 bytes before the tail, first pass; see docs/UNSURE.md. Its match belongs in checkob.c,
   whose checkob_ret5 tail it uses. */
#define sub_93470 sub_93470_decl
#include "nhl95.h"
#undef sub_93470

int __cdecl sub_93470(int v);  /* sroot: integer square root */

/* passtoa0 (551CF, 94G passtoa0): pass the puck to p. Count the pass for p's team (not in highlight mode),
   make p the pass target (passplayer) and give p the pass receive assignment (13h). From p's hot spot (GetHot)
   minus the puck, p's velocity in pixels per quarter second and the pass speed (passspeed / 4), solve for the
   time t (in 1/8 s, 1..18h) at which a pass meets p: the square root (sroot) of j * j - 4ak, tried with
   both signs. The puck's height step (puckvz byte 1) is t up to 12, over 6 halved plus a random half; p's
   temp1 high byte is 8t - 10 and pucknopuck that minus 6; regd0 / regd1 and p's temp3 / temp4 get the meeting
   point, and the puck's velocity (puckvx / puckvy) the distance to it over 120t. p: the receiver. No result. */
void passtoa0(Player *p)
{
    short vx;
    short t;
    short j;
    short vy;
    short dy;
    short dx;
    short k;
    short ax;
    short ay;
    short sp;
    short n;
    int r;

    if (!(gmode & gmhl)) p->tmptr->tmpass++;
    passplayer = p->SCnum;
    assinsert(p, 0x13);
    GetHot(p);
    dx = regd0.w + HIWORD(p->Xpos) - *puckx;
    ax = dx >> 2;
    dy = regd1.w + HIWORD(p->Ypos) - *pucky;
    ay = dy >> 2;
    vx = p->Xvel * 0xF0 >> 16;
    vy = p->Yvel * 0xF0 >> 16;
    j = (vx * ax + vy * ay) * 2;
    sp = passspeed >> 2;
    k = vx * vx + vy * vy - sp * sp;
    r = j * j - (ax * ax + ay * ay) * k * 4;
    if (r > 0) r = sub_93470(r);
    else r ^= r;
    k >>= 2;
    if (!k) k = 1;
    for (n = 0; n < 2; n++) {
        t = r;
        r = -r;
        t = (t - j) / k;
        if (t >= 0) break;
    }
    if (t <= 0) t = 1;
    if (t > 0x18) t = 0x18;
    n = (signed char)(t < 0xC ? t : 0xC);
    if (n > 6) n = (n + 1) / 2 + randomd0(n / 2);
    ((char *)puckvz)[1] = n;
    ((char *)&p->temp1)[1] = (t << 3) - 0xA;
    pucknopuck = ((signed char *)&p->temp1)[1] - 6;
    regd0.l = (vx * t >> 1) + dx;
    p->temp3 = *puckx + regd0.w;
    regd1.l = (vy * t >> 1) + dy;
    p->temp4 = *pucky + regd1.w;
    t *= 0x78;
    *puckvx = ToFixed(regd0.l) / t;
    *puckvy = ToFixed(regd1.l) / t;
}
