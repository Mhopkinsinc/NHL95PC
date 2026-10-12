/* Engine physics: shot blocks. */
#include "nhl95.h"

/* BlockShotDive: player p dives to block a shot. dir is the dive direction from CanBlockShot.
 CanBlock (replay95.asm) tests whether a joystick player may block. check4check and
 check4check2 (logic95.asm) then play SPAblock and lock the animation. chkblock, called from
 holdplayer, may replace SPAhook with SPAblock. Those three play that one animation in place.
 This dive locks the animation (pflags bit 5) and turns to face the puck. Directions 3 and 10
 are the high blocks: SPAblocklow while the puck is low and not rising fast, otherwise
 SPAblockhigh. Any other direction is a slide. slideface is the word at player+4Ah. newspd is
 the Y component from word_CCC32, then the X component from word_CCC30 (both indexed by slide2,
 twice that facing, and negated when dir < 3). Damp oldvel, then add newspd. The slide is
 SPAblockslide when handedness and "dir < 3" disagree, otherwise SPAblockslide2. */
void BlockShotDive(Player *p, int dir)
{
    int oldvel;
    short slideface;
    short newspd;
    short slide2;

    p->pflags |= pfalock;          /* animation lock: the dive runs to completion */
    p->facedir = p->puckdir;       /* start by facing the puck */
    if (dir == 10 || dir == 3) {
        /* High block. dir is reused as the puckvz pointer so that load stays in ebx, the
         register dir already occupies; both failing tests share SPAblockhigh. */
        if (*(short *)puckz < 3 && *(short *)(dir = puckvz) < 0x7D0) dir = SPAblocklow;
        else dir = SPAblockhigh;
    } else {
        /* Slide. +4Ah is the facing used for the speed tables; facedir follows it. */
        slideface = *(short *)((char *)p + 0x4A);
        p->facedir = slideface;
        slide2 = (short)(slideface + slideface);   /* table index, kept in si for both columns */
        newspd = word_CCC32[slide2];               /* Y component */
        if (dir < 3) newspd = -newspd;             /* the other way along the ice */
        oldvel = p->Yvel;
        /* Opposite signs: the new speed reverses him, so kill most of the old Y velocity.
         Same sign: halve it only when he is already faster than the new speed. */
        if (((int)newspd ^ oldvel) < 0) p->Yvel >>= 3;
        else {
            int absspd;
            int absvel;
            if (p->Yvel < 0) oldvel = -oldvel;     /* |Yvel| back into oldvel */
            if (newspd < 0) absspd = -newspd;
            else absspd = newspd;                  /* |speed| */
            absvel = oldvel;                       /* compare uses a second register */
            if (absvel > absspd) p->Yvel >>= 1;
            else p->Yvel >>= 3;
        }
        p->Yvel += newspd;
        newspd = word_CCC30[slide2];               /* X component, same index */
        if (dir < 3) newspd = -newspd;
        /* handed == 0 is one hand. Disagreeing with dir < 3 picks the other slide animation.
         This overwrites dir; the speed is already in newspd. */
        if ((p->handed == 0) ^ (dir < 3)) dir = SPAblockslide;
        else dir = SPAblockslide2;
        oldvel = p->Xvel;
        /* Same damping rule as Y. The absolutes are copied into absvel and absspd (velocity, then
         speed) instead of negating oldvel in place, which is what the X registers do. */
        if (((int)newspd ^ oldvel) < 0) p->Xvel >>= 3;
        else {
            int absvel;
            int absspd;
            if (p->Xvel < 0) absvel = -oldvel;
            else absvel = oldvel;
            if (newspd < 0) absspd = -newspd;
            else absspd = newspd;
            if (absvel > absspd) p->Xvel >>= 1;
            else p->Xvel >>= 3;
        }
        p->Xvel += newspd;
    }
    SetSPA(p, dir);                /* dir now holds the animation number */
}
