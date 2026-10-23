/* Import / export: posting a played game to a team's databases. */
/* DRAFT under 10.0 GA: 2,341 vs 2,342 bytes, 191 differing instructions. The extra spill
   slot and the file-result register (edx instead of ebx) still differ. */
#define PostGameToTeamDb PostGameToTeamDb_decl
#define WriteTeamRec WriteTeamRec_decl
#define ReadGSummaryRec ReadGSummaryRec_decl
#include "nhl95.h"
#undef PostGameToTeamDb
#undef WriteTeamRec
#undef ReadGSummaryRec

int PostGameToTeamDb(char *dir, char *ext, int team, int side, int game, char *gdir);
int WriteTeamRec(int fh, void *rec, int n);  /* 3A2B8 */
int ReadGSummaryRec(int fh, void *rec, int n);  /* 147FF */

/* PostGameToTeamDb (3626D, PC only): post a played game to one team's league databases. Open the team (read write),
   key, season and game summary databases (the summary in gdir); read the team record and count the game in its
   season or playoff block (game 444h and on): games, wins / losses / ties (side 0 home, 1 visitors), goals for and
   against, power plays and power-play goals for and against, penalty minutes against. A team's three stars get a
   star count in their season records (goalies in the goalie records). Find the goalie of record: for a tie the
   goalies in net at the end, else the goalies in net at the winning goal (counted from the game summary's goal
   events, home and visitors). Then each dressed skater who played (not on the scratch list) adds his game stats
   (games, goals, assists, points, the other columns), and each goalie who played (from the lines' goalie slots, with
   playing time) his games, minutes, shots and goals against, the decision and shutout for the goalie of record, the
   goals-against average (per hundred) and save percentage (per thousand); write the records back and close the
   files. dir / ext: the league directory and database extension; team: the team; side: 0 home, 1 visitors; game:
   the schedule index; gdir: the game summary directory. Result: 0, or the first file error. */
int PostGameToTeamDb(char *dir, char *ext, int team, int side, int game, char *gdir)
{
    struct {
        unsigned char team[0x2E8];
        unsigned char grec[0x38];
        unsigned char key[0x34];
        unsigned char srec[0x30];
        char path[0x20];
        unsigned char played[0x19];
        unsigned char gplayed[3];
        unsigned char gsum[0xC];
        unsigned char *lines[2];
        int fgsum;
        int fseason;
        int fkey;
        int fteam;
        int sideofs;
        int ngames;
        int i;
        unsigned char *rec;
        int res;
        int score;
        int j;
        unsigned char gk[2];
    } f;
    unsigned short *gs;
    short *ss;
    short *st;
    int r;
    int k;
    int goalieslot;
    int scratchslot;

    f.fgsum = f.fseason = f.fkey = f.fteam = -1;
    f.lines[0] = hmlinetab;
    f.lines[1] = awlinetab;
    if (game < 0x444) {
        f.rec = f.team + 0x28;
        ss = (short *)f.srec;
        gs = (unsigned short *)f.grec;
    } else {
        f.rec = f.team + 0x3A;
        ss = (short *)(f.srec + 0x12);
        gs = (unsigned short *)(f.grec + 0x16);
    }
    MakePath(f.path, dir, (char *)leaguedbnames[4], ext);
    r = FileOpenRW(f.path, &f.fteam);
    if (!r) {
        MakePath(f.path, dir, (char *)leaguedbnames[0], ext);
        r = FileOpenRead(f.path, &f.fkey);
    }
    if (!r) {
        MakePath(f.path, dir, (char *)leaguedbnames[5], ext);
        r = FileOpenRW(f.path, &f.fseason);
    }
    if (!r) {
        MakePath(f.path, gdir, (char *)str_GSUMMARY, (char *)str_extDB);
        r = FileOpenRW(f.path, &f.fgsum);
    }
    if (!r) r = ReadTeamRec(f.fteam, f.team, team);
    if (!r) {
        f.rec[0]++;
        if (hmscore[0] > awscore) {
            f.score = awscore + 1;
            f.res = 0;
            if (!side) f.rec[1]++;
            else f.rec[2]++;
        } else if (hmscore[0] < awscore) {
            f.score = hmscore[0] + 1;
            f.res = 1;
            if (side) f.rec[1]++;
            else f.rec[2]++;
        } else {
            f.res = -1;
            f.rec[3]++;
            f.score = hmscore[0];
        }
        st = (short *)f.rec;
        f.sideofs = side << 8;
        st[2] += *(short *)((char *)hmscore + f.sideofs);
        st[3] += *(short *)((char *)hmscore + ((side ^ 1) << 8));
        st[5] += *(short *)((char *)&hmtmstruct + f.sideofs + 4);
        st[4] += *(short *)((char *)&hmtmstruct + f.sideofs + 2);
        st[7] += *(short *)((char *)&hmtmstruct + ((side ^ 1) << 8) + 4);
        st[6] += *(short *)((char *)&hmtmstruct + ((side ^ 1) << 8) + 2);
        st[8] = st[8] + *(short *)((char *)&hmtmstruct + ((side ^ 1) << 8) + 0xC);
        for (f.i = 0; f.i < 3 && !r; f.i++) {
            if (startm[f.i * 2] == side) {
                r = ReadKeyRec(f.fkey, f.key, *(int *)(f.team + 0x4C + starpl[f.i * 2] * 4));
                if (!r) {
                    if (f.key[2] == 'G') r = ReadGoalieSeasonRec(f.fseason, f.grec, *(int *)(f.key + 0x2C));
                    else r = ReadSeasonRec(f.fseason, f.srec, *(int *)(f.key + 0x2C));
                }
                if (!r) {
                    if (f.key[2] == 'G') {
                        ((short *)(f.grec + 0x30))[f.i]++;
                        r = WriteGoalieSeasonRec(f.fseason, f.grec, *(int *)(f.key + 0x2C));
                    } else {
                        ((short *)(f.srec + 0x28))[f.i]++;
                        r = WriteSeasonRec(f.fseason, f.srec, *(int *)(f.key + 0x2C));
                    }
                }
            }
        }
    }
    if (f.res < 0) {
        f.gk[0] = ((unsigned char *)hmtmlines[0])[(hmtmgoalie[0] & 1) + 0x24];
        f.gk[1] = ((unsigned char *)awtmlines)[(awtmgoalie & 1) + 0x24];
    } else {
        if (!r) r = ReadGSummaryRec(f.fgsum, f.gsum, 0);
        if (!r) {
            k = 0;
            f.j = 0;
            f.gk[1] = f.gk[0] = 0xFF;
            f.ngames = *(short *)(f.gsum + 5);
            for (f.i = 0; f.i <= f.ngames && !r; f.i++) {
                r = ReadGSummaryRec(f.fgsum, f.gsum, f.i);
                if (!r && f.gsum[0] == 1) {
                    if (f.gsum[1] == 0) k++;
                    else f.j++;
                    if (f.res == 0) {
                        if (k == f.score) {
                            f.gk[0] = f.gsum[9];
                            f.gk[1] = f.gsum[10];
                        }
                    } else if (f.j == f.score) {
                        f.gk[0] = f.gsum[10];
                        f.gk[1] = f.gsum[9];
                    }
                }
            }
        }
    }
    for (f.i = 0; f.i < 0x19; f.i++) f.played[f.i] = 1;
    f.i = 0;
    do {
        scratchslot = f.lines[side][f.i + 0x28];
        if (scratchslot < 0x19) f.played[scratchslot] = 0;
        f.i++;
    } while (f.i < 8);
    for (f.i = 0; f.i < 0x19 && !r; f.i++) {
        k = *(int *)(f.team + 0x4C + f.i * 4);
        if (k != -1) {
            r = ReadKeyRec(f.fkey, f.key, k);
            if (!r) r = ReadSeasonRec(f.fseason, f.srec, *(int *)(f.key + 0x2C));
            if (!r && f.played[f.i]) {
                k = (int)((char *)dword_DB088 + side * 0x190 + f.i * 16);
                ss[0]++;
                ss[1] += (*(short *)(k + 0));
                ss[2] += (*(short *)(k + 2));
                ss[3] += (*(short *)(k + 0));
                ss[3] += (*(short *)(k + 2));
                ss[6] += (*(short *)(k + 4));
                ss[8] += (*(short *)(k + 6));
                ss[4] += (*(short *)(k + 8));
                ss[5] += (*(short *)(k + 10));
                ss[7] += (*(short *)(k + 14));
                r = WriteSeasonRec(f.fseason, f.srec, *(int *)(f.key + 0x2C));
            }
        }
    }
    for (f.i = 0; f.i < 3; f.i++) f.gplayed[f.i] = 0;
    f.i = 0;
    do {
        goalieslot = f.lines[side][f.i + 0x24];
        if (goalieslot < 0x1C) f.played[goalieslot]++;
        f.i++;
    } while (f.i < 2);
    for (f.i = 0; f.i < 3 && !r; f.i++) {
        k = *(int *)(f.team + 0xB0 + f.i * 4);
        if (k != -1) {
            r = ReadKeyRec(f.fkey, f.key, k);
            if (!r) r = ReadGoalieSeasonRec(f.fseason, f.grec, *(int *)(f.key + 0x2C));
            if (!r && f.gplayed[f.i]) {
                k = (int)((char *)unk_DC240 + side * 18 + f.i * 6);
                if ((*(short *)(k + 0))) {
                    gs[0]++;
                    gs[6] = gs[6] + (*(short *)(k + 0)) / 0x3C;
                    gs[9] += (*(short *)(k + 2));
                    gs[7] += (*(short *)(k + 4));
                    if ((signed char)f.gk[side] == (unsigned char)(f.i + 0x19)) {
                        if (hmscore[0] > awscore) {
                            if (!side) gs[1]++;
                            else gs[2]++;
                        } else if (hmscore[0] < awscore) {
                            if (!(side)) gs[2]++;
                            else gs[1]++;
                        } else {
                            gs[3]++;
                        }
                        if (awscore == 0 && !side || hmscore[0] == 0 && side == 1) gs[4]++;
                    }
                    if (gs[0]) gs[8] = gs[7] * 100 / gs[0];
                    else gs[8] = gs[0];
                    if (gs[9]) gs[10] = ((gs[9] - gs[7]) * 1000 + gs[9] / 2) / gs[9];
                    else gs[10] = gs[9];
                    r = WriteGoalieSeasonRec(f.fseason, f.grec, *(int *)(f.key + 0x2C));
                }
            }
        }
    }
    if (!r) r = WriteTeamRec(f.fteam, f.team, team);
    FileClose(&f.fgsum);
    FileClose(&f.fseason);
    FileClose(&f.fkey);
    FileClose(&f.fteam);
    return r;
}
