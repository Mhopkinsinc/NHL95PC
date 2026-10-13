/* Database merge: refresh the human team databases from the current league. */
#define LeagueCheckStub LeagueCheckStub_decl
#define CopyHumanTeamDBs CopyHumanTeamDBs_decl
#define RebuildLeagueDbs RebuildLeagueDbs_decl
#include "nhl95.h"
#undef LeagueCheckStub
#undef CopyHumanTeamDBs
#undef RebuildLeagueDbs

int LeagueCheckStub(char *dir, int mode);
int CopyHumanTeamDBs(char *dir, void *teams, int which);

/* UpdateTeamDbs: highlight the dialog, and if this league passes the stub check, read its header.
 When that fails and a saved game exists, say so at the mouse and stop. Otherwise open PINFO.DB, read
 the word at 13h, and if bit 2 is set copy the human team databases (which == -1). If the bit is clear,
 say the league has not been merged. Any error on that path is the update-error box. Restore the dialog
 colours either way. */
void UpdateTeamDbs(void)
{
    struct {
        char path[0x20];
        char name[0x10];
        int fh;
        int btn;
        int my;
        int mx;
        unsigned char flags[4];
    } s;
    int shown;
    int err;
    int io;

    s.fh = -1;
    shown = 0;
    SetDialogColors(0xF9, 0xFA, 0xF8, 0xFA, shown);
    if (LeagueCheckStub((char *)&curleague, 2))
        goto done;
    err = ReadLeagueInfo((char *)&curleague, leagueteams, (char *)masterpw,
        (short *)&leaguemaster, &humancount, &leagueflags, &leaguesaved, (char *)&savedname);
    io = err;
    strcpy(s.name, (char *)&curleague);
    s.name[_fstrcspn(s.name, (char *)&str_dot)] = 0;
    if (!err && leaguesaved) {
        msg_SavedGame_arg = (int)s.name;
        sub_B2DCA(&s.mx, &s.my, &s.btn);
        MessageBox(-1, -1, (char *)msg_SavedGame, 3, err, err, (int)&s.my, (int)&s.btn, -1);
        shown = -1;
    }
    if (shown)
        goto done;
    if (!io) {
        MakePath(s.path, (char *)&curleague, (char *)str_PINFO, (char *)str_extDB);
        io = FileOpenRW(s.path, &s.fh);
    }
    if (!io)
        io = FileReadAt(s.fh, s.flags, 0x13, 2);
    FileClose(&s.fh);
    if (!io) {
        if (s.flags[0] & 4)
            io = CopyHumanTeamDBs((char *)&curleague, leagueteams, -1);
        else {
            *(char **)(mergestatemsg + 4) = s.name;
            *(char **)(mergestatemsg + 8) = (char *)str_HasNotBeenMerged;
            sub_B2DCA(&s.mx, &s.my, &s.btn);
            MessageBox(-1, -1, (char *)mergestatemsg, 3, io, io, (int)&s.my, (int)&s.btn, -1);
        }
    }
    if (io) {
        sub_B2DCA(&s.mx, &s.my, &s.btn);
        MessageBox(-1, -1, (char *)updateerrmsg, 2, 0, 0, (int)&s.my, (int)&s.btn, -1);
    }
done:
    SetDialogColors(0x2A, 0x3F, 0x17, 0x3F, 0);
}

/* RebuildLeagueDbs: ask each human team for its password, then the master password. When the stub
 check and those passwords all succeed, open PINFO.DB, read the word at 13h, delete the saved game
 and copy the human team databases. Any error on that path is the update-error box. Restores the
 dialog colours either way. */
void RebuildLeagueDbs(void)
{
    struct {
        char path[0x20];
        int fh;
        int btn;
        int my;
        int mx;
        unsigned char flags[4];
    } s;
    int err;
    int pw;

    s.fh = -1;
    SetDialogColors(0xF9, 0xFA, 0xF8, 0xFA, 0);
    err = LeagueCheckStub((char *)&curleague, 2);
    if (!err) {
        err = ReadLeagueInfo((char *)&curleague, leagueteams, (char *)masterpw,
            (short *)&leaguemaster, &humancount, &leagueflags, &leaguesaved, (char *)&savedname);
        pw = 0;
        for (s.mx = pw; s.mx < 0x1A && pw == 0; s.mx++) {
            if (lgteam_17[s.mx * 30] == 1)
                pw = AskTeamPassword(s.mx, (char *)leagueteams);
        }
        if (pw == 0)
            pw = AskMasterPassword(leaguemaster, (char *)leagueteams, (char *)masterpw);
    }
    if (err == 0 && pw == 0) {
        MakePath(s.path, (char *)&curleague, (char *)str_PINFO, (char *)str_extDB);
        err = FileOpenRW(s.path, &s.fh);
        if (err == 0)
            err = FileReadAt(s.fh, s.flags, 0x13, 2);
        FileClose(&s.fh);
        if (err == 0) {
            strcpy(s.path, (char *)&curleague);
            DeleteFiles(s.path, (char *)str_GAME2, (char *)str_SAV2);
            err = CopyHumanTeamDBs((char *)&curleague, leagueteams, -1);
        }
        if (err != 0) {
            sub_B2DCA(&s.mx, &s.my, &s.btn);
            MessageBox(-1, -1, (char *)updateerrmsg, 2, 0, 0, (int)&s.my, (int)&s.btn, -1);
        }
    }
    SetDialogColors(0x2A, 0x3F, 0x17, 0x3F, 0);
}
