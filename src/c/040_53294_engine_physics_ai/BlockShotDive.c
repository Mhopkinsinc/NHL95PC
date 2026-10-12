/* Engine physics: shot blocks. */
#include "nhl95.h"

/* BlockShotDive: player p dives to block a shot. dir is the dive direction from CanBlockShot.
 Lock the current animation (pflags bit 5) and turn to face the puck. Directions 3 and 10 are the
 high blocks: SPA 14ADh while the puck is low and not rising fast, otherwise 1475h. Any other
 direction is a slide. Face the word at player+4Ah, take Y speed from word_CCC32 and X speed from
 word_CCC30 (both indexed by twice that facing, and negated when dir < 3), damp the old velocity,
 then add the new speed. The slide animation is 13C5h when handedness and "dir < 3" disagree,
 otherwise 141Dh. */
void BlockShotDive(Player *p, int dir)
{
    int vel;
    short face;
    short spd;
    short twice;

    p->pflags |= pfalock;          /* animation lock: the dive runs to completion */
    p->facedir = p->puckdir;       /* start by facing the puck */
    if (dir == 10 || dir == 3) {
        /* High block. dir is reused as the puckvz pointer so that load stays in ebx, the
         register dir already occupies; both failing tests share the 1475h animation. */
        if (*(short *)puckz < 3 && *(short *)(dir = puckvz) < 0x7D0) dir = 0x14AD;
        else dir = 0x1475;
    } else {
        /* Slide. +4Ah is the facing used for the speed tables; facedir follows it. */
        face = *(short *)((char *)p + 0x4A);
        p->facedir = face;
        twice = (short)(face + face);          /* table index, kept in si for both columns */
        spd = word_CCC32[twice];               /* Y component */
        if (dir < 3) spd = -spd;               /* the other way along the ice */
        vel = p->Yvel;
        /* Opposite signs: the new speed reverses him, so kill most of the old Y velocity.
         Same sign: halve it only when he is already faster than the new speed. */
        if (((int)spd ^ vel) < 0) p->Yvel >>= 3;
        else {
            int a;
            int b;
            if (p->Yvel < 0) vel = -vel;       /* |Yvel| back into vel */
            if (spd < 0) a = -spd;
            else a = spd;                      /* |speed| */
            b = vel;                           /* compare uses a second register */
            if (b > a) p->Yvel >>= 1;
            else p->Yvel >>= 3;
        }
        p->Yvel += spd;
        spd = word_CCC30[twice];               /* X component, same index */
        if (dir < 3) spd = -spd;
        /* handed == 0 is one hand. Disagreeing with dir < 3 picks the other slide animation.
         This overwrites dir; the speed is already in spd. */
        if ((p->handed == 0) ^ (dir < 3)) dir = 0x13C5;
        else dir = 0x141D;
        vel = p->Xvel;
        /* Same damping rule as Y. The absolutes are copied into a and b (velocity, then speed)
         instead of negating vel in place, which is what the X registers do. */
        if (((int)spd ^ vel) < 0) p->Xvel >>= 3;
        else {
            int a;
            int b;
            if (p->Xvel < 0) a = -vel;
            else a = vel;
            if (spd < 0) b = -spd;
            else b = spd;
            if (a > b) p->Xvel >>= 1;
            else p->Xvel >>= 3;
        }
        p->Xvel += spd;
    }
    SetSPA(p, dir);                /* dir now holds the animation number */
}
