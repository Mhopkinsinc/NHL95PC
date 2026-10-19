/* Season playoffs: seed the first round, the conference finals and the final. */
/* DRAFT (both functions): POSeedConfFinals matches but for its end, POSeedFinal matches byte for byte after it
   (it jumps into POSeedConfFinals' pairing code, POSeedConfFinals_pair); see docs/UNSURE.md. Their match
   belongs in POSeedRound2.c: in the EXE POSeedConfFinals ends with `add esp, 4; jmp POSeedRound2_x` and keeps
   the swapped POSetSeriesTeams call after that jump. POSeedTeams (also a draft here: same size, the team
   compare and the first swap step load in the other order) ends in POSeedConfFinals_x. */
#define SeriesWinner SeriesWinner_decl
#define POSetSeriesTeams POSetSeriesTeams_decl
#define POSeedConfFinals POSeedConfFinals_decl
#define POSeedFinal POSeedFinal_decl
#define POSeedTeams POSeedTeams_decl
#include "nhl95.h"
#undef SeriesWinner
#undef POSetSeriesTeams
#undef POSeedConfFinals
#undef POSeedFinal
#undef POSeedTeams

int SeriesWinner(unsigned char *s, unsigned games);
#pragma aux SeriesWinner parm [eax] [edx] value [eax] modify [eax edx];
void POSetSeriesTeams(unsigned char *s, int a, int b);
#pragma aux POSetSeriesTeams parm [eax] [edx] [ebx] modify [eax edx ebx];
/* POSeedTeams (86E8B, PC only; ends in POSeedConfFinals_x): lay out the 16 first-round seed slots: every slot
   empty (-10h), then eight random pairs of conference placeholders (-3 for the west, -0Ch for the east; a taken
   slot moves the pick to the next one). The two humans' teams (HomeTeam, VisTeam) take placeholders of their
   conferences: in the same conference the first and the last one; else a random one of the first three, and
   one of the later ones for the other team (when it lands before the first, the two slots and every placeholder
   of the two conferences swap). Then POPickConfTeams fills the west (3, teams 0-11) and the east (0Ch, teams
   12-25). slots: the 16 seed dwords. No result. */
void POSeedTeams(int *slots)
{
    int a;
    int i;
    int k;
    int n;
    int c;

    for (i = 0; i < 0x10; i++) slots[i] = -0x10;
    for (i = 0; i < 0x10; i += 2) {
        k = randomd0(0x10);
        for (; slots[k] != -0x10; k = (k + 1) % 0x10) ;
        slots[k] = -3;
        k = randomd0(0x10);
        for (; slots[k] != -0x10; k = (k + 1) % 0x10) ;
        slots[k] = -0xC;
    }
    if (teamconf[HomeTeam] == teamconf[VisTeam]) {
        for (i = 0; -teamconf[HomeTeam] != slots[i]; i++) ;
        slots[i] = HomeTeam;
        for (i = 0xF; -teamconf[VisTeam] != slots[i]; i--) ;
        slots[i] = VisTeam;
    } else {
        c = randomd0(3);
        for (i = 0, n = -1; n != c; i++)
            if (-teamconf[HomeTeam] == slots[i]) n++;
        slots[--i] = HomeTeam;
        a = i;
        c += randomd0(4 - c);
        for (i = 0, n = -1; n != c; i++)
            if (-teamconf[VisTeam] == slots[i]) n++;
        slots[--i] = VisTeam;
        if (i < a) {
            slots[a] ^= slots[i];
            slots[i] = slots[a] ^ slots[i];
            slots[a] = slots[a] ^ slots[i];
            for (i = 0; i < 0x10; i++) {
                if (-teamconf[HomeTeam] == slots[i]) slots[i] = -teamconf[VisTeam];
                else if (-teamconf[VisTeam] == slots[i]) slots[i] = -teamconf[HomeTeam];
            }
        }
    }
    POPickConfTeams(slots, 3, 0, 0xC);
    POPickConfTeams(slots, 0xC, 0xC, 0xE);
}

/* POSeedConfFinals (87B33, PC only): walk back over the last three second-round game slots while all four
   series still have an empty score, then date the two conference finals from that game (7 games each, every
   other day, rolling the month at 31; the second one a day earlier). Each pair of second-round winners
   (SeriesWinner) fills one final, the winner listed first in the team order at p getting the home slot
   (POSetSeriesTeams). POSeedFinal shares its last pairing (POSeedConfFinals_pair) and POSeedTeams its exit
   (POSeedConfFinals_x). p: the playoff schedule. No result. */
void POSeedConfFinals(unsigned char *p)
{
    int a;
    unsigned char *g;
    int i;
    int day;
    int *q;
    int c;

    g = p + 0x1B0C;
    i = 0;
    do {
        if (g[4] != 0xFF || g[0x2E] != 0xFF || g[0x58] != 0xFF || g[0x82] != 0xFF)
            break;
        i++;
        g -= 6;
    } while (i < 3);
    a = g[0];
    day = g[1] + 3;
    g = p + 0x1B90;
    for (i = 0; i < 7; i++, g += 6) {
        g[0] = a;
        g[1] = i % 7 * 2 + day;
        if (g[1] > 0x1F) {
            g[0]++;
            g[1] -= 0x1F;
        }
    }
    day--;
    g = p + 0x1BBA;
    for (i = 0; i < 7; i++, g += 6) {
        g[0] = a;
        g[1] = i % 7 * 2 + day;
        if (g[1] > 0x1F) {
            g[0]++;
            g[1] -= 0x1F;
        }
    }
    g = p + 0x1AE8;
    a = SeriesWinner(g, *(int *)(p + 0x70));
    day = SeriesWinner(g + 0x2A, *(int *)(p + 0x74));
    for (q = (int *)p; *q != a && *q != day; q++);
    g = p + 0x1B90;
    c = *q;
    if (c == a) POSetSeriesTeams(g, a, day);
    else POSetSeriesTeams(g, day, a);
    g = p + 0x1B3C;
    a = SeriesWinner(g, *(int *)(p + 0x78));
    day = SeriesWinner(g + 0x2A, *(int *)(p + 0x7C));
    for (q = (int *)p; *q != a && *q != day; q++);
    g = p + 0x1BBA;
    c = *q;
    if (c == a) POSetSeriesTeams(g, a, day);
    else POSetSeriesTeams(g, day, a);
}

/* POSeedFinal (87C9E, PC only): walk back over the last three conference-final game slots while both series
   still have an empty score, date the final from that game (7 games, every other day, rolling the month at
   31) and fill it with the two conference champions (SeriesWinner), the one listed first in the team order
   at p getting the home slot; the pairing is POSeedConfFinals' code. p: the playoff schedule. No result. */
void POSeedFinal(unsigned char *p)
{
    int a;
    unsigned char *g;
    int i;
    int day;
    int *q;
    int c;

    g = p + 0x1BB4;
    i = 0;
    do {
        if (g[4] != 0xFF || g[0x2E] != 0xFF)
            break;
        i++;
        g -= 6;
    } while (i < 3);
    a = g[0];
    day = g[1] + 3;
    g = p + 0x1BE4;
    for (i = 0; i < 7; i++, g += 6) {
        g[0] = a;
        g[1] = i % 7 * 2 + day;
        if (g[1] > 0x1F) {
            g[0]++;
            g[1] -= 0x1F;
        }
    }
    g = p + 0x1B90;
    a = SeriesWinner(g, *(int *)(p + 0x80));
    day = SeriesWinner(g + 0x2A, *(int *)(p + 0x84));
    for (q = (int *)p; *q != a && *q != day; q++);
    g = p + 0x1BE4;
    c = *q;
    if (c == a) POSetSeriesTeams(g, a, day);
    else POSetSeriesTeams(g, day, a);
}
