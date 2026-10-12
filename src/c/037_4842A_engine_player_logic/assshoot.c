/* Engine player logic: shoot assignment. */
/* DRAFT: 242 vs 248. The pfna clear is `and bl, 0FDh` (original copies bl to ah first) and the
 word at +4Ch is counted in ax (original di), so edi is never saved. Do not retry a separate
 byte local for the flag: that moved the and into al. */
#define assshoot assshootDecl
#include "nhl95.h"
#undef assshoot

/* assshoot: asstab slot 18. While pfalock, leave. A new assignment (pfna) clears that bit and
 SetShotMode. Otherwise, unless sflags bit 3, assexit. Then count temp2 down (regd2 bit 5 when it
 passes 0) and the word at player+4Ch (held at 0). Inside the y band 54h..9Dh, with the other
 team's dword at +3Eh under 24h and that word 0, a 1-in-14h roll sets the word to 12Ch and regd1
 bit 4; any miss clears regd1 bits 4 and 6 (mask AFh). Then ShotMode. */
void assshoot(Player *p)
{
    short w;
    int y;

    if (p->pflags & pfalock) return;
    if (p->pflags & pfna) {
        p->pflags &= (signed char)~pfna;
        SetShotMode(p);
        return;
    }
    if (!(sflags & 8)) {
        assexit(p);
        return;
    }
    regd2.w = 0;
    if (--p->temp2 < 0) *(unsigned char *)&regd2 |= 0x20;
    w = *(short *)((char *)p + 0x4C);
    *(short *)((char *)p + 0x4C) = --w;
    if (w < 0) *(short *)((char *)p + 0x4C) = 0;
    if (HIWORD(p->Ypos) < 0) y = -(p->Ypos >> 16);
    else y = p->Ypos >> 16;
    if (y < 0x9E) {
        if (HIWORD(p->Ypos) < 0) y = -(p->Ypos >> 16);
        else y = p->Ypos >> 16;
        if (y > 0x53 && *(int *)((char *)p->optmptr + 0x3E) < 0x24
            && *(short *)((char *)p + 0x4C) == 0 && randomd0(0x14) == 0) {
            *(short *)((char *)p + 0x4C) = 0x12C;
            *(unsigned char *)&regd1 |= 0x10;
        } else *(unsigned char *)&regd1 &= 0xAF;
    } else *(unsigned char *)&regd1 &= 0xAF;
    ShotMode(p);
}
