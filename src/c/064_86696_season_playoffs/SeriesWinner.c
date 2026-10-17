/* Season playoffs: series winner. */
#include "nhl95.h"

/* SeriesWinner (87760, PC only; ends in POCreateSchedule_x): the winner of the playoff series s (6-byte games:
   +2 home team, +3 away team, +4 / +5 home / away score, FFh for not played). Count each team's wins over the
   games until one has more than half of games; a game not yet played first means no winner. s: the series;
   games: its length. Result: the winning team (s[2] or s[3] of the first game), or -1. */
int SeriesWinner(unsigned char *s, unsigned games)
{
    int home;
    int away;
    int t;
    unsigned w1;
    unsigned w2;

    games = games / 2 + 1;
    home = s[2];
    away = s[3];
    for (w2 = 0, w1 = 0; w1 < games && w2 < games; s += 6) {
        if (s[4] == 0xFF) return -1;
        t = s[2];
        if (t == home) {
            if (s[4] > s[5]) w1++;
            else w2++;
        } else {
            if (s[4] > s[5]) w2++;
            else w1++;
        }
    }
    if ((int)w1 > (int)w2) return home;
    return away;
}
