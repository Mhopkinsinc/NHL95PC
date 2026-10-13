/* Database merge: refresh the human team databases from the current league. */
#define LeagueCheckStub LeagueCheckStub_decl
#define CopyHumanTeamDBs CopyHumanTeamDBs_decl
#include "nhl95.h"
#undef LeagueCheckStub
#undef CopyHumanTeamDBs

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
