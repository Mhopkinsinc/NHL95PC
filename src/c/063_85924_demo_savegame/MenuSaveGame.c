/* Demo / save game: saving the game in progress. */
#define SaveGameNameDlg SaveGameNameDlg_decl
#define SaveGameState SaveGameState_decl
#define SaveLeagueGameRef SaveLeagueGameRef_decl
#define MenuSaveGame MenuSaveGame_decl
#include "nhl95.h"
#undef SaveGameNameDlg
#undef SaveGameState
#undef SaveLeagueGameRef
#undef MenuSaveGame

int SaveGameNameDlg(char *path);
void SaveGameState(int h);
void SaveLeagueGameRef(int h);

typedef struct { char c[9]; } Str9;

/* MenuSaveGame (85924, PC only; owner of the SaveGameStub tail SaveGameNameDlg ends in): menu callback that
   saves the game in progress. An exhibition (gamemode 0) needs room for GSUMMARY.DB plus 1328h bytes, then is
   saved under the name from SaveGameNameDlg (DEMO.NHL in demo mode): exhstate, the GSUMMARY.DB bank, the other
   games' teams / scores / periods and the game state (SaveGameState). A league or playoff game needs room for
   GSUMMARY.DB and 4 KB and is saved as GAME.SAV in the league directory with its mode state (the league's
   game reference too), then GSUMMARY.DB is copied to GSUMMARY.SAV there. Not enough room shows the KB needed.
   The mode state gets bit 80h of +5Ah and gameopts bit 15 first. The text state is kept. Result: 0 when the
   name dialog is cancelled, else whatever is in ebp (never set). */
int MenuSaveGame(void)
{
    struct {
        char st[0x40];
        char ft[0x2C];
        char path[0x20];
        int need[3];
        int h;
    } s;
    int r;  /* the original only sets it on cancel */
    int err;
    int p;

    sub_8E9E8(s.st);
    sub_8EA18(mainfont);
    MakePath(s.path, (char *)&curleague, (char *)str_GsummaryDb4, 0);
    unknown_libname_1(s.path, 0, s.ft);
    s.need[0] = *(int *)(s.ft + 0x1A);
    s.need[2] = 0;
    if (gamemode == 0) {
        s.need[0] = (s.need[0] + 0x1328) / 1024;
        s.need[1] = 0;
        if (err = DiskSpaceShort(0, s.need)) {
            SetDialogColors(0xF9, 0xFA, 0xF8, 0xFA, 0);
            sprintf((char *)str_AGameYouRequire3, (char *)str_AGameYouRequire, err);
            MessageBox(-1, -1, (char *)msg_NoSaveSpace, 3, 0, 0, (int)&dword_DC88C, (int)&dword_DC888, 0x320);
            goto done;
        }
        if (demomode) {
            *(Str9 *)s.path = *(Str9 *)str_DemoNhl;
        } else if (SaveGameNameDlg(s.path)) {
            sub_8EA00(s.st);
            r = 0;
            goto out;
        }
        exhstate[0x5A] |= 0x80;
        ((unsigned char *)&gameopts)[1] |= 0x80;
        WriteModeState(exhstate);
        if (FileCreate(s.path, &s.h)) FatalError((char *)unk_C392C);
        if (FileWriteAt(s.h, exhstate, -1, 0x75)) FatalError((char *)unk_C392F);
        MakePath(s.path, 0, (char *)str_Gsummary3, (char *)str_extDB);
        p = sub_8E8A0(s.path, 0);
        if (FileWriteAt(s.h, (void *)p, -1, sub_92DE0(s.path))) FatalError((char *)unk_C393B);
        FreeMemBlock(p);
        if (FileWriteAt(s.h, othergames, -1, 0xC)) FatalError((char *)unk_C393E);
        if (FileWriteAt(s.h, otherscores, -1, 0xC)) FatalError((char *)unk_C3941);
        if (FileWriteAt(s.h, otherperiod, -1, 0x18)) FatalError((char *)unk_C3944);
        SaveGameState(s.h);
        FileClose(&s.h);
    } else {
        s.need[0] = (s.need[0] + 0x3FF) / 1024;
        s.need[1] = 4;
        if (err = DiskSpaceShort(0, s.need)) {
            SetDialogColors(0xF9, 0xFA, 0xF8, 0xFA, 0);
            sprintf((char *)str_AGameYouRequire3, (char *)str_AGameYouRequire2, err);
            MessageBox(-1, -1, (char *)msg_NoSaveSpace, 3, 0, 0, (int)&dword_DC88C, (int)&dword_DC888, 0x320);
            goto done;
        }
        SetTickDeadline(0xC8);
        SetDialogColors(0xF9, 0xFA, 0xF8, 0xFA, 0);
        MakePath(s.path, (char *)&curleague, (char *)str_GameSav5, 0);
        if (FileCreate(s.path, &s.h)) FatalError((char *)unk_C396F);
        if (gamemode == 2) {
            MessageBox(-1, -1, (char *)msg_SavingLeagueGame, 2, 0, 0, 0, 0, 0);
            ((unsigned char *)lgstate)[0x5A] |= 0x80;
            ((unsigned char *)&gameopts)[1] |= 0x80;
            WriteModeState(lgstate);
            SaveLeagueGameRef(s.h);
        } else {
            MessageBox(-1, -1, (char *)msg_SavingPlayoffGame, 2, 0, 0, 0, 0, 0);
            ((unsigned char *)postate)[0x5A] |= 0x80;
            ((unsigned char *)&gameopts)[1] |= 0x80;
            WriteModeState(postate);
        }
        SaveGameState(s.h);
        FileClose(&s.h);
        if (CopyFile((char *)str_Gsummary3, (char *)str_Db4, (char *)str_Sav3, (char *)&curleague, (char *)&curleague))
            FatalError((char *)unk_C397B);
        WaitTickDeadline();
        RestoreDialogBg();
    }
done:
    sub_8EA00(s.st);
out:
    return r;
}
