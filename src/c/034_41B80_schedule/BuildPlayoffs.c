/* Schedule: build the playoffs at the end of the season. */
/* DRAFT: same size, every instruction but the order of two in the prologue; see docs/UNSURE.md. Its match
   belongs in UpdateSeasonSchedule.c, whose UpdateSeasonSchedule_pop3 tail it uses. */
#define BuildPlayoffs BuildPlayoffs_decl
#define SeedPlayoffRound1 SeedPlayoffRound1_decl
#include "nhl95.h"
#undef BuildPlayoffs
#undef SeedPlayoffRound1

int SeedPlayoffRound1(int *seeds, unsigned char *buf, int rwfh, int series, unsigned char *teams);  /* 42FED */

/* BuildPlayoffs (428AB, PC only): after the last regular season day, seed the playoffs (GetPlayoffSeeds from
   the teams file rwfh) and make the 69h six-byte playoff game slots (all FFh). When a seeded team of either
   conference is human (team byte +17h is 1), only the first round is seeded (SeedPlayoffRound1) and the slots
   are written to the schedule file fh at 199Ah (game 444h). Otherwise every round is seeded and simulated
   (SeedPlayoffRound1..SimPlayoffFinal; the seeding gets the league option bits 12-14), the slots are
   written, the season database is saved under dir / ext, the awards ceremony is shown, byte_DE268 is set to
   FFh and the schedule's game count becomes 4ADh (the end). rdfh: the read-only career teams file the
   simulations use; teams: the league team bytes. Returns 0 or the first error. */
int BuildPlayoffs(int fh, int rwfh, int rdfh, unsigned char *teams, char *dir, char *ext)
{
    struct {
        int seeds[28];
        char path[0x20];
        int human;
        int rdfh;
        int err;
        int n;
    } s;
    unsigned char *buf;
    unsigned char *q;
    int i;
    int r;

    s.rdfh = rdfh;
    s.human = 0;
    if ((r = GetPlayoffSeeds(s.seeds, fh, rwfh)) != 0) return r;
    buf = (unsigned char *)sub_8CCA8((char *)str_sche, 0x276, 0x20);
    for (i = 0; i < 0x69; i++) {
        q = buf + i * 6;
        q[0] = q[1] = q[2] = q[3] = q[4] = q[5] = 0xFF;
    }
    for (i = 0; i < 8; i++) {
        if (teams[s.seeds[i] * 30 + 0x17] == 1) {
            s.human = 1;
            break;
        }
        if (teams[s.seeds[i + 14] * 30 + 0x17] == 1) {
            s.human = 1;
            break;
        }
    }
    if (s.human) {
        if (!(s.err = SeedPlayoffRound1(s.seeds, buf, rwfh, gameopts.optbits12, teams)))
            s.err = FileWriteAt(fh, buf, 0x199A, 0x276);
    } else if (!(s.err = SeedPlayoffRound1(s.seeds, buf, rwfh, gameopts.optbits12, teams))
        && !(s.err = SimPlayoffRound1((int)buf, rwfh, s.rdfh, dir, ext))
        && !(s.err = SeedPlayoffRound2((int)buf, gameopts.optbits12, rwfh, (int)teams, &s.human))
        && !(s.err = SimPlayoffRound2((int)buf, rwfh, s.rdfh, dir, ext))
        && !(s.err = SeedPlayoffRound3((int)buf, gameopts.optbits12, rwfh, (int)teams, &s.human))
        && !(s.err = SimPlayoffRound3((int)buf, rwfh, s.rdfh, dir, ext))
        && !(s.err = SeedPlayoffFinal((int)buf, gameopts.optbits12, rwfh, (int)teams, fh, &s.human))
        && !(s.err = SimPlayoffFinal((int)buf, rwfh, s.rdfh, dir, ext))
        && !(s.err = FileWriteAt(fh, buf, 0x199A, 0x276))) {
        MakePath(s.path, dir, (char *)leaguedbnames[5], ext);
        sub_932D0(s.path, seasondb, seasondb_size);
        AwardsCeremony();
        byte_DE268 = -1;
        s.n = 0x4AD;
        s.err = FileWriteAt(fh, &s.n, 0, 2);
    }
    FreeMemBlock((int)buf);
    return s.err;
}
