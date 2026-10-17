/* Season playoffs: creating the playoff schedule. */
#include "nhl95.h"

void POInitSchedule(unsigned char *po, int *teams);
void POSeedRound2(unsigned char *po);
void POSeedConfFinals(unsigned char *po);
void POSeedFinal(unsigned char *po);

/* POCreateSchedule (8721F, PC only; owner of POCreateSchedule_x): create the playoff schedule file of the league
   in dir. Load the SCHEDULE.DB template (2-byte header), fill the 15 round settings at +50h-+88h with 7 (best of
   seven) and set up the series from the 16 seeded teams (POInitSchedule). When the two human teams (HomeTeam,
   VisTeam) are in different conferences, load the league's season, career and key databases (leaguedbnames,
   .DB) and pre-simulate the rounds before they can meet (POPreSimRound1 / 2 / ConfFinals with the seeding of each
   next round), save the season database and free the three. Then find the series (7 games apart, 2Ah bytes each
   from +1998h) that matches the two up, store its game number (+40h) and round (+44h), and write the schedule
   back. teams: the 16 seeds; dir: the league directory. Result: 1 when the template cannot be loaded or the
   file cannot be opened, else FileWriteAt's result. */
int POCreateSchedule(int *teams, char *dir)
{
    char path[0x40];
    char dbpath[0x40];
    int fh;
    unsigned char *po;
    unsigned char *s;
    int n;
    int size;
    int r;

    MakePath(path, dir, (char *)str_ScheduleDb, 0);
    po = (unsigned char *)sub_8E8B8(path, 0x20) + 2;
    if (!po) return 1;
    *(int *)(po + 0x50) = *(int *)(po + 0x54) = *(int *)(po + 0x58) = *(int *)(po + 0x5C) = *(int *)(po + 0x60)
        = *(int *)(po + 0x64) = *(int *)(po + 0x68) = *(int *)(po + 0x6C) = *(int *)(po + 0x70) = *(int *)(po + 0x74)
        = *(int *)(po + 0x78) = *(int *)(po + 0x7C) = *(int *)(po + 0x80) = *(int *)(po + 0x84) = *(int *)(po + 0x88)
        = 7;
    POInitSchedule(po, teams);
    if (teamconf[HomeTeam] != teamconf[VisTeam]) {
        MakePath(dbpath, dir, (char *)leaguedbnames[5], (char *)str_extDB);
        seasondb = (unsigned char *)sub_8E8A0(dbpath, 0x20);
        seasondb_size = sub_92DE0(dbpath);
        MakePath(dbpath, dir, (char *)leaguedbnames[1], (char *)str_extDB);
        careerdb = (unsigned char *)sub_8E8A0(dbpath, 0x20);
        careerdb_size = sub_92DE0(dbpath);
        MakePath(dbpath, dir, (char *)leaguedbnames[0], (char *)str_extDB);
        keydb = (unsigned char *)sub_8E8A0(dbpath, 0x20);
        keydb_size = sub_92DE0(dbpath);
        POPreSimRound1(po, HomeTeam, VisTeam);
        POSeedRound2(po);
        POPreSimRound2(po, HomeTeam, VisTeam);
        POSeedConfFinals(po);
        POPreSimConfFinals(po, HomeTeam, VisTeam);
        POSeedFinal(po);
        MakePath(dbpath, dir, (char *)leaguedbnames[5], (char *)str_extDB);
        sub_932D0(dbpath, seasondb, seasondb_size);
        if (seasondb) FreeMemBlock((int)seasondb);
        if (careerdb) FreeMemBlock((int)careerdb);
        if (keydb) FreeMemBlock((int)keydb);
        keydb = careerdb = seasondb = 0;
        keydb_size = careerdb_size = seasondb_size = 0;
    }
    for (n = 0, s = po + 0x1998; s[2] != HomeTeam || s[3] != VisTeam; n += 7, s += 0x2A);
    *(int *)(po + 0x40) = n;
    *(int *)(po + 0x44) = n / 7;
    size = sub_92DE0(path);
    if (FileOpenWrite(path, &fh)) {
        po -= 2;
        r = 1;
    } else {
        po -= 2;
        r = FileWriteAt(fh, po, 0, size);
        FileClose(&fh);
    }
    FreeMemBlock((int)po);
    return r;
}
