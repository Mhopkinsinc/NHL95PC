/* Schedule: playoff round seeding and simulation (one source block: the rounds share SimPlayoffRound1's exit,
 and SeedPlayoffRound1 before it ends there too). */
/* DRAFT: logic and frames right (__STOSD via a local pragma); the game-record index series-length registers differ (cl vs bl, lea edx vs inc eax), plus R1 parameter slots. */
#define WriteTeamRec WriteTeamRec_decl
#include "nhl95.h"
#undef WriteTeamRec

/* Watcom runtime: store n dwords of v at p (the code generator's block fill) */
void __STOSD(void *p, int v, int n);
#pragma aux __STOSD "__STOSD" parm [eax] [edx] [ecx];

int WriteTeamRec(int fh, void *rec, int n);  /* 3A2B8: the callers here test its result */

/* a team record of the league file (2E8h bytes): the game slots of the team's playoff games at +26Ch */
typedef struct POTeamRec {
    unsigned char pad[0x26C];
    int games[0x1C];
    unsigned char pad2[0xC];
} POTeamRec;

/* SeedPlayoffRound1 (42FED, PC only; ends in SimPlayoffRound1_pop3): set up the first round of the playoff
   schedule sched from the 16 seeds (seeds[0-7] one conference, seeds[14-21] the other). For a human team (byte
   +17h of its 1Eh-byte entry in teams) the team record of the league file fh is read, its 28 game slots are reset
   (-1) and then given the file positions of its series' games (series s, game k: (s * 7 + 444h / 460h + k) * 6 + 2;
   s = seed pair 1 v 8, 2 v 7, ...), and written back. Each series gets its two teams (SetSeriesTeams) and every
   game k is dated April (month 4) day 2k + 11h in the first conference, 2k + 10h in the second. games: the
   series length. Returns 0, or the error of a record read / write. */
int SeedPlayoffRound1(int *seeds, unsigned char *sched, int fh, int games, unsigned char *teams)
{
    POTeamRec r0;
    POTeamRec r1;
    int i;
    int k;
    int err;

    for (i = 0; i < 8; i++) {
        if (teams[seeds[i] * 30 + 0x17] == 1 && (err = ReadTeamRec(fh, &r0, seeds[i])) != 0) return err;
        if (teams[seeds[i + 14] * 30 + 0x17] == 1 && (err = ReadTeamRec(fh, &r1, seeds[i + 14])) != 0) return err;
        for (k = 0; k < 0x1C; k++) r0.games[k] = r1.games[k] = -1;
        if (teams[seeds[i] * 30 + 0x17] == 1 && (err = WriteTeamRec(fh, &r0, seeds[i])) != 0) return err;
        if (teams[seeds[i + 14] * 30 + 0x17] == 1 && (err = WriteTeamRec(fh, &r1, seeds[i + 14])) != 0) return err;
    }
    for (i = 0; i < 4; i++) {
        if (teams[seeds[i] * 30 + 0x17] == 1 && (err = ReadTeamRec(fh, &r0, seeds[i])) != 0) return err;
        if (teams[seeds[7 - i] * 30 + 0x17] == 1 && (err = ReadTeamRec(fh, &r1, seeds[7 - i])) != 0) return err;
        for (k = 0; k < games; k++) r0.games[k] = r1.games[k] = (i * 7 + 0x444 + k) * 6 + 2;
        if (teams[seeds[i] * 30 + 0x17] == 1 && (err = WriteTeamRec(fh, &r0, seeds[i])) != 0) return err;
        if (teams[seeds[7 - i] * 30 + 0x17] == 1 && (err = WriteTeamRec(fh, &r1, seeds[7 - i])) != 0) return err;
    }
    for (i = 0; i < 4; i++) {
        if (teams[seeds[i + 14] * 30 + 0x17] == 1 && (err = ReadTeamRec(fh, &r0, seeds[i + 14])) != 0) return err;
        if (teams[seeds[7 - i + 14] * 30 + 0x17] == 1 && (err = ReadTeamRec(fh, &r1, seeds[7 - i + 14])) != 0)
            return err;
        for (k = 0; k < games; k++) r0.games[k] = r1.games[k] = (i * 7 + 0x460 + k) * 6 + 2;
        if (teams[seeds[i + 14] * 30 + 0x17] == 1 && (err = WriteTeamRec(fh, &r0, seeds[i + 14])) != 0) return err;
        if (teams[seeds[7 - i + 14] * 30 + 0x17] == 1 && (err = WriteTeamRec(fh, &r1, seeds[7 - i + 14])) != 0)
            return err;
    }
    SetSeriesTeams(sched, seeds[0], seeds[7], games);
    SetSeriesTeams(sched + 0x2A, seeds[1], seeds[6], games);
    SetSeriesTeams(sched + 0x54, seeds[2], seeds[5], games);
    SetSeriesTeams(sched + 0x7E, seeds[3], seeds[4], games);
    SetSeriesTeams(sched + 0xA8, seeds[14], seeds[21], games);
    SetSeriesTeams(sched + 0xD2, seeds[15], seeds[20], games);
    SetSeriesTeams(sched + 0xFC, seeds[16], seeds[19], games);
    SetSeriesTeams(sched + 0x126, seeds[17], seeds[18], games);
    for (i = 0; i < games; i++) {
        sched[i * 6] = sched[(i + 7) * 6] = sched[(i + 0xE) * 6] = sched[(i + 0x15) * 6] = sched[(i + 0x1C) * 6]
            = sched[(i + 0x23) * 6] = sched[(i + 0x2A) * 6] = sched[(i + 0x31) * 6] = 4;
        sched[i * 6 + 1] = sched[(i + 7) * 6 + 1] = sched[(i + 0xE) * 6 + 1] = sched[(i + 0x15) * 6 + 1] = i * 2 + 0x11;
        sched[(i + 0x1C) * 6 + 1] = sched[(i + 0x23) * 6 + 1] = sched[(i + 0x2A) * 6 + 1] = sched[(i + 0x31) * 6 + 1]
            = i * 2 + 0x10;
    }
    return 0;
}

/* SimPlayoffRound1: simulate the 8 first-round series (7 games, 2Ah bytes each) of the playoff schedule sched. Each game of a series not yet played (score byte +4 = FFh)
 is simulated (SimulateGame, schedule slot s * 7 + 0x444 + game, mode 2) and the winner (more goals: +4 vs +5) counted
 once a team has won len 2 + 1 (SeriesLength) the remaining games are blanked (FFh). The win counts index
 by the series' first game's teams. Returns 0. */
int SimPlayoffRound1(int sched, int rwfh, int rdfh, char *dir, char *ext)
{
    int wins[26];
    int s;
    int i;
    int len;
    unsigned char *g;

    g = (unsigned char *)sched;
    __STOSD(wins, 0, 26);
    len = SeriesLength(g);
    for (s = 0; s < 8; s++, g += 0x2A) {
        for (i = 0; i < len; i++) {
            if (wins[g[2]] == len / 2 + 1 || wins[g[3]] == len / 2 + 1) {
                g[i * 6 + 3] = 0xFF;
                g[i * 6] = g[i * 6 + 1] = g[i * 6 + 2] = g[i * 6 + 3];
            } else {
                if (g[i * 6 + 4] == 0xFF) SimulateGame(dir, ext, s * 7 + 0x444 + i, g + i * 6, rwfh, rdfh, 2);
                wins[g[i * 6 + 4] > g[i * 6 + 5] ? g[i * 6 + 2] : g[i * 6 + 3]]++;
            }
        }
    }
    return 0;
}

/* SeedPlayoffRound2: not decompiled yet (stays asm); this stub only keeps the file's layout so the
 rounds after it reach SimPlayoffRound1's exit with a near jump as in the EXE. */
int SeedPlayoffRound2(int buf, int series, int a, int c, int *out)
{
    return 0;
}

/* SimPlayoffRound2: the same for the 4 second-round series (sched + 150h). Each game of a series not yet played (score byte +4 = FFh)
 is simulated (SimulateGame, schedule slot s * 7 + 0x47C + game, mode 2) and the winner (more goals: +4 vs +5) counted
 once a team has won len 2 + 1 (SeriesLength) the remaining games are blanked (FFh). The win counts index
 by the series' first game's teams. Returns 0. */
int SimPlayoffRound2(int sched, int rwfh, int rdfh, char *dir, char *ext)
{
    int wins[26];
    int s;
    int i;
    int len;
    unsigned char *g;

    g = (unsigned char *)sched + 0x150;
    __STOSD(wins, 0, 26);
    len = SeriesLength(g);
    for (s = 0; s < 4; s++, g += 0x2A) {
        for (i = 0; i < len; i++) {
            if (wins[g[2]] == len / 2 + 1 || wins[g[3]] == len / 2 + 1) {
                g[i * 6 + 3] = 0xFF;
                g[i * 6] = g[i * 6 + 1] = g[i * 6 + 2] = g[i * 6 + 3];
            } else {
                if (g[i * 6 + 4] == 0xFF) SimulateGame(dir, ext, s * 7 + 0x47C + i, g + i * 6, rwfh, rdfh, 2);
                wins[g[i * 6 + 4] > g[i * 6 + 5] ? g[i * 6 + 2] : g[i * 6 + 3]]++;
            }
        }
    }
    return 0;
}

/* SeedPlayoffRound3: not decompiled yet (stays asm); this stub only keeps the file's layout so the
 rounds after it reach SimPlayoffRound1's exit with a near jump as in the EXE. */
int SeedPlayoffRound3(int buf, int series, int a, int c, int *out)
{
    return 0;
}

/* SimPlayoffRound3: the same for the 2 conference finals (sched + 1F8h). Each game of a series not yet played (score byte +4 = FFh)
 is simulated (SimulateGame, schedule slot s * 7 + 0x498 + game, mode 2) and the winner (more goals: +4 vs +5) counted
 once a team has won len 2 + 1 (SeriesLength) the remaining games are blanked (FFh). The win counts index
 by the series' first game's teams. Returns 0. */
int SimPlayoffRound3(int sched, int rwfh, int rdfh, char *dir, char *ext)
{
    int wins[26];
    int s;
    int i;
    int len;
    unsigned char *g;

    g = (unsigned char *)sched + 0x1F8;
    __STOSD(wins, 0, 26);
    len = SeriesLength(g);
    for (s = 0; s < 2; s++, g += 0x2A) {
        for (i = 0; i < len; i++) {
            if (wins[g[2]] == len / 2 + 1 || wins[g[3]] == len / 2 + 1) {
                g[i * 6 + 3] = 0xFF;
                g[i * 6] = g[i * 6 + 1] = g[i * 6 + 2] = g[i * 6 + 3];
            } else {
                if (g[i * 6 + 4] == 0xFF) SimulateGame(dir, ext, s * 7 + 0x498 + i, g + i * 6, rwfh, rdfh, 2);
                wins[g[i * 6 + 4] > g[i * 6 + 5] ? g[i * 6 + 2] : g[i * 6 + 3]]++;
            }
        }
    }
    return 0;
}

/* SeedPlayoffFinal: not decompiled yet (stays asm); this stub only keeps the file's layout so the
 rounds after it reach SimPlayoffRound1's exit with a near jump as in the EXE. */
int SeedPlayoffFinal(int buf, int series, int a, int c, int fh, int *out)
{
    return 0;
}

/* SimPlayoffFinal: the same for the final series (sched + 24Ch). Each game of a series not yet played (score byte +4 = FFh)
 is simulated (SimulateGame, schedule slot game + 0x4A6, mode 2) and the winner (more goals: +4 vs +5) counted
 once a team has won len 2 + 1 (SeriesLength) the remaining games are blanked (FFh). The win counts index
 by the series' first game's teams. Returns 0. */
int SimPlayoffFinal(int sched, int rwfh, int rdfh, char *dir, char *ext)
{
    int wins[26];
    int i;
    int len;
    unsigned char *g;

    g = (unsigned char *)sched + 0x24C;
    __STOSD(wins, 0, 26);
    len = SeriesLength(g);
    for (i = 0; i < len; i++) {
        if (wins[g[2]] == len / 2 + 1 || wins[g[3]] == len / 2 + 1) {
            g[i * 6 + 3] = 0xFF;
            g[i * 6] = g[i * 6 + 1] = g[i * 6 + 2] = g[i * 6 + 3];
        } else {
            if (g[i * 6 + 4] == 0xFF) SimulateGame(dir, ext, i + 0x4A6, g + i * 6, rwfh, rdfh, 2);
            wins[g[i * 6 + 4] > g[i * 6 + 5] ? g[i * 6 + 2] : g[i * 6 + 3]]++;
        }
    }
    return 0;
}
