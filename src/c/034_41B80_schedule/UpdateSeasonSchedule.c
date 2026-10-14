/* Schedule: open the league files and play one schedule day, or advance the playoffs. */
#define UpdateSeasonSchedule UpdateSeasonSchedule_decl
#define SimScheduleDay SimScheduleDay_decl
#define AdvancePlayoffs AdvancePlayoffs_decl
#include "nhl95.h"
#undef UpdateSeasonSchedule
#undef SimScheduleDay
#undef AdvancePlayoffs

int SimScheduleDay(int fh, int rwfh, int rdfh, int day, unsigned char *teams, char *dir, char *ext,
                   int team, int flags);
int AdvancePlayoffs(int fh, int rwfh, int rdfh, unsigned char *teams, char *dir, char *ext,
                    int team, int flags);

/* DRAFT: 672 vs 672 (best of 5). The frame, the three opens, the database loads, the
 0/1 playoff test and both calls match. ext is homed in ebp and err in esi; the original
 homes ext in esi and err in ebp, so careerdb is reloaded through that freed register
 instead of edi. An int or char* copy of ext swaps the day and teams homes and still
 leaves ext in ebp. Making err a pointer puts dir in ebp and ext in edi and drops the
 mov eax, 1 / test eax (655 bytes). Do not retry those, or folding play into the if.
 UpdateSeasonSchedule: open SCHEDULE and TEAMS read/write and CARTEAMS read-only under dir/ext.
 When the schedule is not yet at the end marker 4ADh, read that game. Load the season, career and
 key databases. A day still inside the regular schedule (below 444h), or the first playoff slot
 while either team byte is empty, is simulated; a later day, or that slot with both teams filled,
 advances the playoffs. The season database is written back, the three buffers are freed, and the
 handles are closed. SortStandings and SimPlayoffRound3 jump to the pops (UpdateSeasonSchedule_pop3).
 Returns the last file error. */
int UpdateSeasonSchedule(char *dir, char *ext, int day, unsigned char *teams, int team, int flags)
{
    struct {
        char path[0x20];
        unsigned char game[8];
        int rdfh;
        int rwfh;
        int fh;
    } s;
    int err;
    int play;

    s.rdfh = s.rwfh = s.fh = -1;
    byte_DE268 = 0;
    MakePath(s.path, dir, (char *)leaguedbnames[6], ext);
    err = FileOpenRW(s.path, &s.fh);
    if (!err) {
        MakePath(s.path, dir, (char *)leaguedbnames[4], ext);
        err = FileOpenRW(s.path, &s.rwfh);
    }
    if (!err) {
        MakePath(s.path, dir, (char *)leaguedbnames[3], ext);
        err = FileOpenRead(s.path, &s.rdfh);
    }
    if (!err) {
        if (day < 0x4AD)
            err = ReadSchedGame(s.fh, s.game, day);
    }
    if (!err) {
        MakePath(s.path, dir, (char *)leaguedbnames[5], ext);
        seasondb = (unsigned char *)sub_8E8A0(s.path, 0x20);
        seasondb_size = sub_92DE0(s.path);
        MakePath(s.path, dir, (char *)leaguedbnames[1], ext);
        careerdb = (unsigned char *)sub_8E8A0(s.path, 0x20);
        careerdb_size = sub_92DE0(s.path);
        MakePath(s.path, dir, (char *)leaguedbnames[0], ext);
        keydb = (unsigned char *)sub_8E8A0(s.path, 0x20);
        keydb_size = sub_92DE0(s.path);
    }
    if (!err && day > 0) {
        if (day <= 0x444) {
            play = day != 0x444 || s.game[2] == 0xFF || s.game[3] == 0xFF;
            if (play) {
                err = SimScheduleDay(s.fh, s.rwfh, s.rdfh, day, teams, dir, ext, team, flags);
                goto played;
            }
        }
        err = AdvancePlayoffs(s.fh, s.rwfh, s.rdfh, teams, dir, ext, team, flags);
    played:;
    }
    if (!err && seasondb) {
        MakePath(s.path, dir, (char *)leaguedbnames[5], ext);
        sub_932D0(s.path, seasondb, seasondb_size);
    }
    if (seasondb) jctime((int)seasondb);
    if (careerdb) jctime((int)careerdb);
    if (keydb) jctime((int)keydb);
    keydb_size = careerdb_size = seasondb_size = keydb = careerdb = seasondb = 0;
    FileClose(&s.rdfh);
    FileClose(&s.rwfh);
    FileClose(&s.fh);
    return err;
}
