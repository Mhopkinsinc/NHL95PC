/* Line editor rosters: skater sorting. */
/* DRAFT: same size (262) with the playoff address written rosterpstat + 0x12 + *a * 0x28 (rosterpstat then comes
   into ebx). Left: the non-playoff product is built in eax and adds rosterpstat as a memory operand (EXE: product
   in ebx, mov eax, [rosterpstat] / add eax, ebx), and the playoff +12h is added before the product (EXE after). */
#include "nhl95.h"

typedef struct { char c[0x28]; } PStat;  /* a roster skater stats record */

/* CmpRosterSkaters: qsort compare for the roster screen's skaters: records in rosterpstat (28h bytes each,
 playoffs at +12h); no games (word +0) last, then more points (+6), fewer games, more goals (+2), more +10h. */
int CmpRosterSkaters(int *a, int *b)
{
    unsigned short *p;
    unsigned short *q;

    p = !statsplayoffs ? (unsigned short *)&((PStat *)rosterpstat)[*a]
                       : (unsigned short *)(rosterpstat + 0x12 + *a * 0x28);
    q = !statsplayoffs ? (unsigned short *)&((PStat *)rosterpstat)[*b]
                       : (unsigned short *)(rosterpstat + 0x12 + *b * 0x28);
    if ((q[0] == 0) ^ (p[0] == 0)) return q[0] - p[0];
    if (q[3] != p[3]) return q[3] - p[3];
    if (p[0] != q[0]) return p[0] - q[0];
    if (q[1] != p[1]) return q[1] - p[1];
    return ((short *)q)[8] - ((short *)p)[8];
}
