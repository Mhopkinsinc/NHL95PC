/* Season playoffs: a new playoff schedule. */
#include "nhl95.h"

/* POInitSchedule (875A3, PC only; ends in POCreateSchedule_x): set up a new playoff schedule po from the 16 seeded
   teams. Copy the seeds to the start, give the 28 first-round games of each conference (6-byte records at
   +1998h and +1A40h) their dates (month 4, day 17 / 16 plus two days per game in the series, rolling into month 5
   past day 30), pair each conference's teams (teamconf 3, then 0Ch) 1-8, 2-7, 3-6 and 4-5 into its four series
   (POSetSeriesTeams), and clear the 31h later games (+1AE8h: no date, teams FFh). po: the schedule; teams: the
   16 seeds. No result. */
void POInitSchedule(unsigned char *po, int *teams)
{
    int i;
    int n;
    unsigned char *g;
    unsigned char *e;
    int t[8];

    for (i = 0; i < 16; i++) ((int *)po)[i] = teams[i];
    g = po + 0x1998;
    for (i = 0; i < 0x1C; i++) {
        e = g + i * 6;
        e[0] = 4;
        e[1] = i % 7 * 2 + 0x11;
        if (e[1] > 0x1E) {
            e[0]++;
            e[1] -= 0x1E;
        }
    }
    g = po + 0x1A40;
    for (i = 0; i < 0x1C; i++) {
        e = g + i * 6;
        e[0] = 4;
        e[1] = i % 7 * 2 + 0x10;
        if (e[1] > 0x1E) {
            e[0]++;
            e[1] -= 0x1E;
        }
    }
    for (i = 0, n = 0; i < 16; i++)
        if (teamconf[teams[i]] == 3) t[n++] = teams[i];
    POSetSeriesTeams(po + 0x1998, t[0], t[7]);
    POSetSeriesTeams(po + 0x19C2, t[1], t[6]);
    POSetSeriesTeams(po + 0x19EC, t[2], t[5]);
    POSetSeriesTeams(po + 0x1A16, t[3], t[4]);
    for (i = 0, n = 0; i < 16; i++)
        if (teamconf[teams[i]] == 0xC) t[n++] = teams[i];
    POSetSeriesTeams(po + 0x1A40, t[0], t[7]);
    POSetSeriesTeams(po + 0x1A6A, t[1], t[6]);
    POSetSeriesTeams(po + 0x1A94, t[2], t[5]);
    POSetSeriesTeams(po + 0x1ABE, t[3], t[4]);
    g = po + 0x1AE8;
    for (i = 0; i < 0x31; i++) {
        g[0] = g[1] = 0;
        g[2] = g[3] = 0xFF;
        g[4] = g[5] = 0xFF;
        g += 6;
    }
}
