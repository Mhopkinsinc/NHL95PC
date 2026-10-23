/* Calendar: game setup. */
#include "nhl95.h"

/* SetGameSides: set the controlled teams for game g of the schedule (6-byte records, home +2, away +3): team
 and its opponent, the opponent only when it is a human team (lgteam_17 == 1), else -1.
 DRAFT (10.0 GA, 5 differing, same size): o holds the record address first and is then overwritten with the
 opponent (the EXE keeps both in edx; the compare value stays in eax for the else arm). Left: the record sum
 is `mov eax, ebx` .. `add edx, eax` (games still in its incoming edx), the EXE builds g * 6 in edx and adds the
 ecx copy of games. `o = g * 6; o += (int)games;` gives those registers but a dead `mov eax, ebx` first. */
void SetGameSides(int team, unsigned char *games, int g)
{
    int o;

    o = (int)(games + g * 6);
    if (team == ((unsigned char *)o)[2]) {
        o = ((unsigned char *)o)[3];
    } else {
        team = ((unsigned char *)o)[3];
        o = ((unsigned char *)o)[2];
    }
    if (lgteam_17[o * 30] != 1) o = -1;
    SetCtlTeams(team, o, games[g * 6 + 2], games[g * 6 + 3]);
}
