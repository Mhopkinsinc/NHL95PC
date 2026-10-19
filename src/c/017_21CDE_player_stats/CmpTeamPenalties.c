/* Team stats: table sorting. */
#include "nhl95.h"

/* CmpTeamPenalties (23437, PC only): comparator for Team Penalties (off_C6AE0[4]): the smaller sort key first, then
   more losses, fewer penalty minutes (+10h), more points, more wins; else by team index. a, b: pointers to the
   team indexes. Result: as qsort. */
int CmpTeamPenalties(int *a, int *b)
{
    unsigned char *p;
    unsigned char *q;
    int pp;
    int qp;

    if (statssortkeys[*b] != statssortkeys[*a]) return statssortkeys[*a] - statssortkeys[*b];
    p = !statsplayoffs ? statsteamrecs + *a * 0x4C + 0x28 : statsteamrecs + *a * 0x4C + 0x3A;
    q = !statsplayoffs ? statsteamrecs + *b * 0x4C + 0x28 : statsteamrecs + *b * 0x4C + 0x3A;
    if (q[0] != p[0]) return q[0] - p[0];
    if (((unsigned short *)q)[8] != ((unsigned short *)p)[8]) return ((unsigned short *)p)[8] - ((unsigned short *)q)[8];
    qp = q[1] * 2 + q[3];
    pp = p[1] * 2 + p[3];
    if (qp != pp) return qp - pp;
    if (q[1] != p[1]) return q[1] - p[1];
    return *a - *b;
}
