/* Schedule: advancing the league playoffs. */
#define AdvancePlayoffs AdvancePlayoffs_decl
#include "nhl95.h"
#undef AdvancePlayoffs

/* AdvancePlayoffs (44A41, PC only): after a schedule day of the league's playoffs, move them on when a round is
   over. Scan the 69h playoff games (schedule records from 444h): a game between two scheduled teams one of whom
   is human (teams +17h == 1) that is not yet played means the round goes on. Else, when called for the whole
   day (flags 7, team -1): read the playoff game count (fh +0) and the tree (276h bytes at +199Ah into
   dword_C8C61, from SCH), and by the count simulate the rest of round 1 / 2 / 3 / the final (SimPlayoffRound1 ..
   SimPlayoffFinal), seed the next round (SeedPlayoffRound2 / 3 / Final) and, when no human team is left in it,
   finish the playoffs from that round (FinishPlayoffs 3 / 4 / 5); after the final save the season database and
   hold the awards ceremony. Write back the count and the tree, and free the tree. fh: the schedule file; rwfh,
   rdfh: the league files; teams: the team table; dir, ext: the league directory and extension; team, flags: the
   call's team and mode. Result: 0 or the I/O error. */
int AdvancePlayoffs(int fh, int rwfh, int rdfh, unsigned char *teams, char *dir, char *ext, int team, int flags)
{
    struct {
        char path[0x20];
        unsigned char game[8];
        int busy;
        int rdfh;
        int rwfh;
        short n;
    } s;
    int err;
    int i;

    s.rwfh = rwfh;
    s.rdfh = rdfh;
    s.busy = 0;
    err = 0;
    for (i = 0; i < 0x69 && !err && !s.busy; i++) {
        err = ReadSchedGame(fh, s.game, i + 0x444);
        if (s.game[2] == 0xFF || s.game[3] == 0xFF) continue;
        if (teams[s.game[2] * 30 + 0x17] != 1 && teams[s.game[3] * 30 + 0x17] != 1) continue;
        if (s.game[0] == 0xFF || s.game[1] == 0xFF) continue;
        if (s.game[4] == 0xFF || s.game[5] == 0xFF) s.busy = 1;
    }
    if (err || s.busy || flags != 7 || team != -1) goto save;
    if (err = FileReadAt(fh, &s.n, 0, 2)) return err;
    dword_C8C61 = sub_8CCA8((char *)str_Sch, 0x276, 0x20);
    if (err = FileReadAt(fh, (void *)dword_C8C61, 0x199A, 0x276)) goto done;
    if (s.n < 0x47C) {
        s.n = 0x47C;
        if (err = SimPlayoffRound1(dword_C8C61, s.rwfh, s.rdfh, dir, ext)) goto done;
        if (err = SeedPlayoffRound2(dword_C8C61, gameopts.optbits12, s.rwfh, (int)teams, &s.busy)) goto done;
        if (!s.busy) FinishPlayoffs(fh, s.rwfh, s.rdfh, (int)teams, dir, ext, 3);
    } else if (s.n < 0x498) {
        s.n = 0x498;
        if (err = SimPlayoffRound2(dword_C8C61, s.rwfh, s.rdfh, dir, ext)) goto done;
        if (err = SeedPlayoffRound3(dword_C8C61, gameopts.optbits12, s.rwfh, (int)teams, &s.busy)) goto done;
        if (!s.busy) FinishPlayoffs(fh, s.rwfh, s.rdfh, (int)teams, dir, ext, 4);
    } else if (s.n < 0x4A6) {
        s.n = 0x4A6;
        if (err = SimPlayoffRound3(dword_C8C61, s.rwfh, s.rdfh, dir, ext)) goto done;
        if (err = SeedPlayoffFinal(dword_C8C61, gameopts.optbits12, s.rwfh, (int)teams, fh, &s.busy)) goto done;
        if (!s.busy) {
            FinishPlayoffs(fh, s.rwfh, s.rdfh, (int)teams, dir, ext, 5);
            s.n = 0x4AD;
        }
    } else if (s.n < 0x4AD) {
        s.n = 0x4AD;
        if (err = SimPlayoffFinal(dword_C8C61, s.rwfh, s.rdfh, dir, ext)) goto done;
        MakePath(s.path, dir, (char *)leaguedbnames[5], ext);
        sub_932D0(s.path, seasondb, seasondb_size);
        AwardsCeremony();
        byte_DE268 = -1;
    }
    if (err = FileWriteAt(fh, &s.n, 0, 2)) goto done;
save:
    if (dword_C8C61) err = FileWriteAt(fh, (void *)dword_C8C61, 0x199A, 0x276);
done:
    if (dword_C8C61) {
        FreeMemBlock(dword_C8C61);
        dword_C8C61 = 0;
    }
    return err;
}
