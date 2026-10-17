/* Database merge: importing a master league from the floppy. */
#define CopyHumanTeamDBs CopyHumanTeamDBs_decl
#include "nhl95.h"
#undef CopyHumanTeamDBs

int mkdir(const char *path);  /* Watcom CRT mkdir_ */
int CopyHumanTeamDBs(char *dir, void *teams, int which);

/* ImportMasterLeague (3C3AF, PC only; owner of ImportMasterLeague_ret): copy a league exported by the master
   (PLAYER.ID on the floppy, A:) into a new league directory. Read the league name from PLAYER.ID (+0Ch, 0Dh
   bytes) and the league info; a league of that name already here says so (lgexistsmsg). Ask the master
   password, say what happens (importmastermsg), make the directory and copy the seven databases, PINFO.DB and
   GAME.SET into it; delete any saved game there, read the human team count (PINFO +13h) and copy the human
   teams' databases. On a copy error say so (updateerrmsg) and remove the directory; on any error say the import
   failed (importerrmsg). No arguments. Result: 0, an I/O error, or -1 when the password was refused. */
int ImportMasterLeague(void)
{
    struct {
        unsigned char find[0x2C];
        char path[0x20];
        char name[0x10];
        int x;
        int y;
        int i;
        int rwfh;
        int fh;
        short count;
    } s;
    int err;
    int pw;

    s.rwfh = s.fh = -1;
    MakePath(s.path, (char *)&str_floppydrv, (char *)str_PLAYER, (char *)str_extID);
    err = FileOpenRead(s.path, &s.fh);
    if (!err) err = FileReadAt(s.fh, &curleague, 0xC, 0xD);
    FileClose(&s.fh);
    if (err) goto fail;
    if (!err)
        err = ReadLeagueInfo((char *)&str_floppydrv, leagueteams, (char *)masterpw, (short *)&leaguemaster,
            &humancount, &leagueflags, &leaguesaved, (char *)&savedname);
    strcpy(s.name, (char *)&curleague);
    if (!unknown_libname_1(s.name, 0x10, s.find)) goto exists;
    pw = AskMasterPassword(leaguemaster, (char *)leagueteams, (char *)masterpw);
    if (pw) goto fail;
    MessageBox(-1, -1, (char *)importmastermsg, 1, 0, 0, (int)&s.y, (int)&s.x, 0);
    err = mkdir(s.name);
    if (!err)
        for (s.i = 0; s.i < 7 && !err; s.i++)
            err = CopyFile((char *)leaguedbnames[s.i], (char *)str_extDB, (char *)str_extDB, (char *)&str_floppydrv,
                (char *)&curleague);
    if (!err)
        err = CopyFile((char *)str_PINFO, (char *)str_extDB, (char *)str_extDB, (char *)&str_floppydrv,
            (char *)&curleague);
    if (!err) CopyFile((char *)str_GAME3, (char *)str_SET, (char *)str_SET, (char *)&str_floppydrv, (char *)&curleague);
    RestoreDialogBg();
    strcpy(s.path, (char *)&curleague);
    DeleteFiles(s.path, (char *)str_GAME3, (char *)str_SAV3);
    MakePath(s.path, (char *)&curleague, (char *)str_PINFO, (char *)str_extDB);
    err = FileOpenRW(s.path, &s.rwfh);
    if (!err) err = FileReadAt(s.rwfh, &s.count, 0x13, 2);
    FileClose(&s.rwfh);
    if (!err) err = CopyHumanTeamDBs((char *)&curleague, leagueteams, -1);
    if (err) {
        sub_B2DCA(&s.i, &s.y, &s.x);
        MessageBox(-1, -1, (char *)updateerrmsg, 2, 0, 0, (int)&s.y, (int)&s.x, -1);
    }
    if (err) DeleteDir(s.name);
    goto fail;
exists:
    sub_B2DCA(&s.i, &s.y, &s.x);
    SetDialogColors(0xF9, 0xFA, 0xF8, 0xFA, 0);
    MessageBox(-1, -1, (char *)lgexistsmsg, 2, 0, 0, (int)&s.y, (int)&s.x, -1);
fail:
    if (err) {
        sub_B2DCA(&s.i, &s.y, &s.x);
        SetDialogColors(0xF9, 0xFA, 0xF8, 0xFA, 0);
        MessageBox(-1, -1, (char *)importerrmsg, 1, 0, 0, (int)&s.y, (int)&s.x, -1);
    }
    if (pw) err = -1;
    return err;
}
