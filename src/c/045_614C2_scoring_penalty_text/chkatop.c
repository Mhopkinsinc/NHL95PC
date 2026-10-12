/* Scoring penalty text: attack time of possession. */
#include "nhl95.h"

/* chkatop: unless play is stopped (gmode bit 4), count a tick of attack time for the team in the
 offensive zone (puck y beyond +-4Eh; gmode bit 1 swaps the ends): tmATOP of the visitors (top) or home. */
void chkatop(void)
{
    short t;
    short y;

    if (gmode & 0x10) return;
    y = *pucky;
    if (y > 0x4E) t = 0;
    else if (y < -0x4E) t = 1;
    else return;
    if (gmode & 2) t ^= 1;
    if (t > 0) awtmstruct.tmATOP++;
    else hmtmstruct.tmATOP++;
}
