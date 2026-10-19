/* Roster / jersey: carrying out a trade. */
#define ExecuteTrade ExecuteTrade_decl
#define FixJerseyConflict FixJerseyConflict_decl
#define WriteKeyRec WriteKeyRec_decl
#define WriteTeamRec WriteTeamRec_decl
#include "nhl95.h"
#undef ExecuteTrade
#undef FixJerseyConflict
#undef WriteKeyRec
#undef WriteTeamRec

int FixJerseyConflict(int fh, int k, unsigned char *tr);
int WriteKeyRec(int fh, void *rec, long pos);
int WriteTeamRec(int fh, void *rec, int n);

#define KEY(r, k) (*((k) < 0x19 ? &((int *)((r) + 0x4C))[k] : &((int *)((r) + 0xB0))[(k) - 0x19]))
#define KEYR(r, k) ((k) < 0x19 ? ((int *)((r) + 0x4C))[k] : ((int *)((r) + 0xB0))[(k) - 0x19])

/* ExecuteTrade (3E390, PC only): carry out the trade set up on the trade screen. Read both teams' records from
   the league's team database, then for each of the two trade rows with a player chosen on both sides (sel[i] and
   sel[i + 2] not FFh) swap the two players: their key records' team bytes, the teams' key numbers (+4Ch), take
   each off his old team's lines (+BCh / +ECh, ClearPlayerFromLines), write the keys and both team records back
   and fix any jersey number clash (FixJerseyConflict). After a trade, set the two teams as home / visitor, point
   the game databases at the league (leaguedbfmt, S4.DB), load them (LoadGameTeams) and open both teams' line
   editors (GameLineEditor, fading the PAL22 palette back in after each). dir: the league directory; teams: the
   two team numbers; sel: the trade rows' roster slots. Result: 0 or the I/O error. */
int ExecuteTrade(char *dir, int *teams, unsigned char *sel)
{
    struct {
        unsigned char reca[0x2E8];
        unsigned char recb[0x2E8];
        unsigned char keyb[0x34];
        unsigned char keya[0x34];
        char path[0x20];
        int kfh;
        int tfh;
        int kb;
    } s;
    int traded;
    int err;
    int ka;
    int i;
    int pal;

    s.kfh = s.tfh = -1;
    traded = 0;
    MakePath(s.path, dir, (char *)leaguedbnames[4], (char *)str_extDB);
    err = FileOpenRW(s.path, &s.tfh);
    if (!err) err = ReadTeamRec(s.tfh, s.reca, teams[0]);
    if (!err) err = ReadTeamRec(s.tfh, s.recb, teams[1]);
    if (!err) {
        MakePath(s.path, dir, (char *)leaguedbnames[0], (char *)str_extDB);
        err = FileOpenRW(s.path, &s.kfh);
    }
    if (!err) {
        for (i = 0; i < 2 && !err; i++) {
            if (sel[i] == 0xFF || sel[i + 2] == 0xFF) continue;
            traded = -1;
            ka = KEYR(s.reca, sel[i]);
            err = ReadKeyRec(s.kfh, s.keya, ka);
            if (!err) {
                s.kb = KEYR(s.recb, sel[i + 2]);
                err = ReadKeyRec(s.kfh, s.keyb, s.kb);
            }
            if (err) continue;
            s.keya[0] = teams[1];
            s.keyb[0] = teams[0];
            KEY(s.reca, sel[i]) = s.kb;
            KEY(s.recb, sel[i + 2]) = ka;
            ClearPlayerFromLines(sel[i], s.reca + 0xBC);
            ClearPlayerFromLines(sel[i], s.reca + 0xEC);
            ClearPlayerFromLines(sel[i + 2], s.recb + 0xBC);
            ClearPlayerFromLines(sel[i + 2], s.recb + 0xEC);
            err = WriteKeyRec(s.kfh, s.keya, ka);
            if (!err) err = WriteKeyRec(s.kfh, s.keyb, s.kb);
            if (!err) err = WriteTeamRec(s.tfh, s.reca, teams[0]);
            if (!err) err = WriteTeamRec(s.tfh, s.recb, teams[1]);
            if (!err) err = FixJerseyConflict(s.kfh, sel[i], s.reca);
            if (!err) err = FixJerseyConflict(s.kfh, sel[i + 2], s.recb);
        }
    }
    FileClose(&s.kfh);
    FileClose(&s.tfh);
    if (!err && traded) {
        HomeTeam = teams[0];
        VisTeam = teams[1];
        strcpy((char *)&leaguedbfmt, (char *)&curleague);
        strcat((char *)&leaguedbfmt, (char *)str_backslash2);
        strcat((char *)&leaguedbfmt, (char *)str_S4);
        strcat((char *)&leaguedbfmt, (char *)str_extDB);
        strcpy((char *)&leaguedbfmt2, (char *)&leaguedbfmt);
        LoadGameTeams(0);
        curperiod = -2;
        GameLineEditor(0, hmlinetab, unk_CF3CF, 3);
        pal = sub_8CCA8((char *)str_Pal22, 0x300, 0x20);
        sub_8FFB0(err, 0x100, (unsigned char *)pal);
        FadePalStep(1, (unsigned char *)pal, 0x10);
        FreeMemBlock(pal);
        GameLineEditor(1, awlinetab, unk_CF3CF, 3);
        pal = sub_8CCA8((char *)str_Pal22, 0x300, 0x20);
        sub_8FFB0(err, 0x100, (unsigned char *)pal);
        FadePalStep(1, (unsigned char *)pal, 0x10);
        FreeMemBlock(pal);
    }
    return err;
}
