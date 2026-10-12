/* Engine physics: shot blocks. */
#include "nhl95.h"

/* BlockShotDive: player p dives to block a shot. dir is the dive direction from CanBlockShot. Sets the
 animation lock (pflags bit 5) and faces puckdir. Directions 3 and 10 are the high blocks: SPA 14ADh when
 the puck is low and not rising fast, else 1475h. Any other direction faces the word at +4Ah, takes the
 slide speeds word_CCC32 / word_CCC30 (negated when dir < 3), damps Yvel then Xvel (halved when the
 velocity already exceeds the new speed and they share a sign, else divided by 8), and adds the speed.
 The slide animation is 13C5h when handedness and dir < 3 disagree, else 141Dh. */
void BlockShotDive(Player *p, int dir)
{
    int vel;
    short face;
    short spd;
    short twice;

    p->pflags |= pfalock;
    p->facedir = p->puckdir;
    if (dir == 10 || dir == 3) {
        if (*(short *)puckz < 3 && *(short *)(dir = puckvz) < 0x7D0) dir = 0x14AD;
        else dir = 0x1475;
    } else {
        face = *(short *)((char *)p + 0x4A);
        p->facedir = face;
        twice = (short)(face + face);
        spd = word_CCC32[twice];
        if (dir < 3) spd = -spd;
        vel = p->Yvel;
        if (((int)spd ^ vel) < 0) p->Yvel >>= 3;
        else {
            int a;
            int b;
            if (p->Yvel < 0) vel = -vel;
            if (spd < 0) a = -spd;
            else a = spd;
            b = vel;
            if (b > a) p->Yvel >>= 1;
            else p->Yvel >>= 3;
        }
        p->Yvel += spd;
        spd = word_CCC30[twice];
        if (dir < 3) spd = -spd;
        if ((p->handed == 0) ^ (dir < 3)) dir = 0x13C5;
        else dir = 0x141D;
        vel = p->Xvel;
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
    SetSPA(p, dir);
}
