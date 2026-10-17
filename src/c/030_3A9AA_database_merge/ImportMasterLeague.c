/* Database merge: importing a master league from the floppy. */
#define CopyHumanTeamDBs CopyHumanTeamDBs_decl
#define LeagueCheckStub LeagueCheckStub_decl
#define LeagueTeamSelect LeagueTeamSelect_decl
#define ExportTeamToFloppy ExportTeamToFloppy_decl
#include "nhl95.h"
#undef CopyHumanTeamDBs
#undef LeagueCheckStub
#undef LeagueTeamSelect
#undef ExportTeamToFloppy

int mkdir(const char *path);  /* Watcom CRT mkdir_ */
int CopyHumanTeamDBs(char *dir, void *teams, int which);
int LeagueCheckStub(char *dir, int mode);
int LeagueTeamSelect(char *dir, int *count, unsigned char *names, unsigned char *teams, int mode, char *title,
    int *sel);
void ExportTeamToFloppy(char *dir, char *ext, char *ext2, int t, unsigned char *teams, int full, int a);

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

/* ExportDbs (3D108, PC only; ends in ImportMasterLeague_ret): export a human team's databases from the current
   league to the floppy. Read the league (LeagueCheckStub 3, ReadLeagueInfo); with team databases waiting
   (leagueflags bits 2 / 3) offer to update them first (playerdbsmsg, UpdateTeamDbs) and read again; a saved
   game in progress stops it (msg_SavedGame). With the team names read, the team is the one this machine plays
   (GetLeagueId) or one picked (LeagueTeamSelect); a team that has already exported (lgteamflags 2) may only
   export again with the master password (forceexportmsg, AskMasterPassword), which bumps its version and
   exports the full databases (.DB instead of .xx). The extension is the team number (".%02d" into str_extxx);
   after the floppy is chosen (SelectFloppyDrive) ExportTeamToFloppy writes it. The dialog colours are reset at
   the end. No arguments, no result. */
void ExportDbs(void)
{
    struct {
        char path[0x20];
        char name[0x10];
        char ext[4];
        int x;
        int id;
        int btn;
        int t;
        int y;
    } s;
    int err;
    int shown;
    int ask;
    char *e;

    err = LeagueCheckStub((char *)&curleague, 3);
    do {
        shown = 0;
        if (!err) {
            err = ReadLeagueInfo((char *)&curleague, leagueteams, (char *)masterpw, (short *)&leaguemaster,
                &humancount, &leagueflags, &leaguesaved, (char *)&savedname);
            strcpy(s.name, (char *)&curleague);
            s.name[_fstrcspn(s.name, (char *)&str_dot)] = 0;
            SetDialogColors(0xF9, 0xFA, 0xF8, 0xFA, shown);
            sub_B2DCA(&s.btn, &s.y, &s.x);
            if (leagueflags & 0xC) {
                ((int *)playerdbsmsg)[1] = (int)s.name;
                s.btn = MessageBox(-1, -1, (char *)playerdbsmsg, 4, (int)btn_MasterDB, 2, (int)&s.y, (int)&s.x,
                    -1);
                if (!s.btn) {
                    UpdateTeamDbs();
                    shown = -1;
                } else
                    err = -1;
            } else if (leaguesaved) {
                msg_SavedGame_arg = (int)s.name;
                MessageBox(-1, -1, (char *)msg_SavedGame, 3, shown, shown, (int)&s.y, (int)&s.x, -1);
                err = -1;
            }
        }
    } while (shown);
    if (err) goto done;
    MakePath(s.path, (char *)&curleague, (char *)leaguedbnames[4], (char *)str_extDB);
    if (ReadTeamNames(s.path, (char *)treeteamnames, 1)) goto done;
    ask = -1;
    s.t = GetLeagueId((char *)&curleague, &s.id);
    if (s.t < 0) {
        LeagueTeamSelect((char *)&curleague, &humancount, treeteamnames, leagueteams, 8, (char *)str_SelectATeamTo2,
            &s.t);
        if (humancount == 1) SetDialogColors(0xF9, 0xFA, 0xF8, 0xFA, shown);
    } else
        ask = 0;
    if (s.t >= 0 && lgteamflags[s.t * 30] == 2) {
        forceexportmsg[0] = (int)s.name;
        forceexportmsg[2] = (int)(leagueteams + s.t * 30);
        sub_B2DCA(&s.btn, &s.y, &s.x);
        s.btn = MessageBox(-1, -1, (char *)forceexportmsg, 4, (int)forceexportbtns, 2, (int)&s.y, (int)&s.x, -1);
        err = -1;
        if (!s.btn) err = AskMasterPassword(leaguemaster, (char *)leagueteams, (char *)masterpw);
        if (!err) {
            shown = -1;
            lgteamver_hi[s.t * 15]++;
        } else
            s.t = -1;
    }
    if (s.t < 0) goto done;
    if (humancount == 1 || !ask) SetDialogColors(0xF9, 0xFA, 0xF8, 0xFA, 0);
    sprintf(s.ext, (char *)str_02d3, s.t);
    strcpy((char *)str_extxx, s.ext);
    e = shown ? (char *)str_extDB : (char *)str_extxx;
    if (!SelectFloppyDrive((int)&curleague, (int)(leagueteams + s.t * 30)))
        ExportTeamToFloppy((char *)&curleague, e, (char *)str_extxx, s.t, leagueteams, shown, -1);
done:
    SetDialogColors(0x2A, 0x3F, 0x17, 0x3F, 0);
}
