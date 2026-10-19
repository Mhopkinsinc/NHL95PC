/* Team stats: table sorting. The six team-stats qsort comparators were one source file (22DDE-23597): they share
   one `pop ebp .. ret` epilogue, CmpTeamPK_x, and every other comparator ends with `add esp, N` / `jmp CmpTeamPK_x`.
   That layout needs every body exact; CmpTeamPP, CmpTeamPK and CmpTeamPenalties are still drafts. Until then this
   file is not in address order on purpose: with the CmpTeamPK draft first and the CmpTeamPP draft before
   CmpTeamStandings, wcc386 keeps the shared epilogue in CmpTeamPK, so the marked CmpTeamScoring, CmpTeamDefense and
   CmpTeamStandings jump into its pop/ret tail as in the original (cc.py frag finds that tail in CmpTeamPK by its
   bytes). Standings anywhere but last, or CmpTeamPenalties in the file, moves the epilogue into another function.
   CmpTeamPenalties is a separate draft (CmpTeamPenalties.c). */
#include "nhl95.h"

/* A team record (statsteamrecs, 4Ch bytes each): the season part at +28h, the playoff part at +3Ah. In each part:
   +0 losses, +1 wins, +3 ties (bytes); +4 goals for, +6 goals against, +8/+0Ah power play, +0Ch/+0Eh penalty
   killing, +10h penalty minutes (words). Points are 2 * wins + ties. */

/* CmpTeamPK (232B7, PC only): comparator for Team Penalty Killing (off_C6AE0[2]): the sort key first, then the
   ratio of words +0Ch / +0Eh (smaller first), more of +0Eh, more losses, more points, more wins; else by team
   index. a, b: pointers to the team indexes. Result: as qsort. */
int CmpTeamPK(int *a, int *b)
{
    unsigned char *p;
    unsigned char *q;
    int qp;
    int pp;
    int d;

    if (statssortkeys[*b] != statssortkeys[*a]) return statssortkeys[*b] - statssortkeys[*a];
    p = !statsplayoffs ? statsteamrecs + *a * 0x4C + 0x28 : statsteamrecs + *a * 0x4C + 0x3A;
    q = !statsplayoffs ? statsteamrecs + *b * 0x4C + 0x28 : statsteamrecs + *b * 0x4C + 0x3A;
    if ((d = ((unsigned short *)q)[6] * ((unsigned short *)p)[7] - ((unsigned short *)p)[6] * ((unsigned short *)q)[7])) return d < 0;
    if (((unsigned short *)q)[7] != ((unsigned short *)p)[7]) return ((unsigned short *)q)[7] - ((unsigned short *)p)[7];
    if (q[0] != p[0]) return q[0] - p[0];
    qp = q[1] * 2 + q[3];
    pp = p[1] * 2 + p[3];
    if (qp != pp) return qp - pp;
    if (q[1] != p[1]) return q[1] - p[1];
    return *a - *b;
}

/* CmpTeamScoring (22F2E, PC only): comparator for Team Scoring (off_C6AE0[0]): more goals for (+4) first, then
   fewer losses, more points, more wins; else by team index. a, b: pointers to the team indexes. Result: as qsort. */
int CmpTeamScoring(int *a, int *b)
{
    unsigned char *p;
    unsigned char *q;
    int pp;
    int qp;

    p = !statsplayoffs ? statsteamrecs + *a * 0x4C + 0x28 : statsteamrecs + *a * 0x4C + 0x3A;
    q = !statsplayoffs ? statsteamrecs + *b * 0x4C + 0x28 : statsteamrecs + *b * 0x4C + 0x3A;
    if (((unsigned short *)q)[2] != ((unsigned short *)p)[2]) return ((unsigned short *)q)[2] - ((unsigned short *)p)[2];
    if (q[0] != p[0]) return p[0] - q[0];
    qp = q[1] * 2 + q[3];
    pp = p[1] * 2 + p[3];
    if (pp != qp) return qp - pp;
    if (q[1] != p[1]) return q[1] - p[1];
    return *a - *b;
}

/* CmpTeamDefense (23051, PC only): comparator for Team Defense (off_C6AE0[1]): fewer goals against (+6) first,
   then more losses (sic), more points, more wins; else by team index. a, b: pointers to the team indexes.
   Result: as qsort. */
int CmpTeamDefense(int *a, int *b)
{
    unsigned char *p;
    unsigned char *q;
    int pp;
    int qp;

    p = !statsplayoffs ? statsteamrecs + *a * 0x4C + 0x28 : statsteamrecs + *a * 0x4C + 0x3A;
    q = !statsplayoffs ? statsteamrecs + *b * 0x4C + 0x28 : statsteamrecs + *b * 0x4C + 0x3A;
    if (((unsigned short *)q)[3] != ((unsigned short *)p)[3]) return ((unsigned short *)p)[3] - ((unsigned short *)q)[3];
    if (q[0] != p[0]) return q[0] - p[0];
    qp = q[1] * 2 + q[3];
    pp = p[1] * 2 + p[3];
    if (pp != qp) return qp - pp;
    if (q[1] != p[1]) return q[1] - p[1];
    return *a - *b;
}

/* CmpTeamPP (2312D, PC only): comparator for Team Power Play (off_C6AE0[3]): the precomputed sort key
   (statssortkeys) first, then the better ratio of words +8 / +0Ah, more of +0Ah, more losses, more points, more
   wins; else by team index. a, b: pointers to the team indexes. Result: as qsort. */
int CmpTeamPP(int *a, int *b)
{
    unsigned char *p;
    unsigned char *q;
    int qp;
    int pp;
    int d;

    if (statssortkeys[*b] != statssortkeys[*a]) return statssortkeys[*b] - statssortkeys[*a];
    p = !statsplayoffs ? statsteamrecs + *a * 0x4C + 0x28 : statsteamrecs + *a * 0x4C + 0x3A;
    q = !statsplayoffs ? statsteamrecs + *b * 0x4C + 0x28 : statsteamrecs + *b * 0x4C + 0x3A;
    if ((d = ((unsigned short *)q)[4] * ((unsigned short *)p)[5] - ((unsigned short *)p)[4] * ((unsigned short *)q)[5])) return d > 0;
    if (((unsigned short *)q)[5] != ((unsigned short *)p)[5]) return ((unsigned short *)q)[5] - ((unsigned short *)p)[5];
    if (q[0] != p[0]) return q[0] - p[0];
    qp = q[1] * 2 + q[3];
    pp = p[1] * 2 + p[3];
    if (qp != pp) return qp - pp;
    if (q[1] != p[1]) return q[1] - p[1];
    return *a - *b;
}

/* CmpTeamStandings (22DDE, PC only): qsort comparator of two team indexes for the Team Standings table
   (off_C6AE0[5]): more points first, then fewer losses, more wins, more goals for (+4), fewer goals against (+6);
   else by team index. a, b: pointers to the team indexes. Result: <0 / 0 / >0 as for qsort. */
int CmpTeamStandings(int *a, int *b)
{
    unsigned char *p;
    unsigned char *q;
    int pp;
    int qp;

    p = !statsplayoffs ? statsteamrecs + *a * 0x4C + 0x28 : statsteamrecs + *a * 0x4C + 0x3A;
    q = !statsplayoffs ? statsteamrecs + *b * 0x4C + 0x28 : statsteamrecs + *b * 0x4C + 0x3A;
    qp = q[1] * 2 + q[3];
    pp = p[1] * 2 + p[3];
    if (pp != qp) return qp - pp;
    if (q[0] != p[0]) return p[0] - q[0];
    if (q[1] != p[1]) return q[1] - p[1];
    if (((unsigned short *)q)[2] != ((unsigned short *)p)[2]) return ((unsigned short *)q)[2] - ((unsigned short *)p)[2];
    if (((unsigned short *)q)[3] != ((unsigned short *)p)[3]) return ((unsigned short *)p)[3] - ((unsigned short *)q)[3];
    return *a - *b;
}
