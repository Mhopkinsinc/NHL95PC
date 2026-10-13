/* DeleteSavedGame: confirm, then delete the selected save. A directory clears the league or playoff
   menu hooks when its name is the current one. The name list is packed, and an empty list picks another
   tab. Returns 0 when the user cancels or the name is already gone. */
#define DeleteSavedGame DeleteSavedGame_decl
#define savefileexts savefileexts_decl
#include "nhl95.h"
#undef DeleteSavedGame
#undef savefileexts

extern int savefileexts[3];
int unlink(char *path);

typedef struct Ext12 { char c[12]; } Ext12;

struct SavedList {
    int count;
    int cursor;
    int top;
    int shown;
    int name[1];
};

int DeleteSavedGame(struct SavedList *list)
{
    struct {
        char find[0x2C];
        char path[0x10];
        int ext[3];
        int y;
        int x;
        int result;
    } s;
    int eq;

    *(Ext12 *)s.ext = *(Ext12 *)savefileexts;
    s.x = 0x140;
    s.y = 0xF0;
    if (fdlgtab == 0)
        *(char **)(delmsgbox + 4) = (char *)str_ExhibitionGameCalled;
    else if (fdlgtab == 1)
        *(char **)(delmsgbox + 4) = (char *)str_PlayOffSeriesCalled;
    else
        *(char **)(delmsgbox + 4) = (char *)str_LeagueCalled;
    *(char **)(delmsgbox + 8) = (char *)list->name[list->cursor];
    SetDialogColors(0xF9, 0xFA, 0xF8, 0xFA, 0);
    if (MessageBox(-1, -1, (char *)delmsgbox, 3, (int)btn_POHumanOut, 2, (int)&s.x, (int)&s.y, -1) != 1) {
        s.result = 0;
        goto done;
    }
    {
        int i;

        i = list->cursor;
        strcpy(s.path, (char *)list->name[i]);
        i = fdlgtab;
        strcat(s.path, (char *)s.ext[i]);
    }
    if (unknown_libname_1(s.path, 0x10, s.find)) {
        s.result = 0;
        goto done;
    }
    if (s.find[0x15] & 0x10) {
        eq = stricmp(s.path, (char *)lgstate + 4);
        if (eq == 0) {
            *((char *)lgstate + 4) = 0;
            menuact_export = eq;
            menuact_nextlg = eq;
            menusub_lgmgr = eq;
        } else {
            eq = stricmp(s.path, (char *)postate + 4);
            if (eq == 0) {
                *((char *)postate + 4) = 0;
                menuact_nextpo = eq;
                menuact_posettings = eq;
                menuact_pohilights = eq;
            }
        }
        DeleteDir(s.find + 0x1E);
    } else {
        if (unlink(s.find + 0x1E))
            FatalError((char *)str_fec6);
    }
    if (--list->count != 0) {
        int i;

        for (i = list->cursor; i < list->count; i++)
            list->name[i] = list->name[i + 1];
        if (list->count < 6) {
            list->shown = list->count - 1;
            list->top = 0;
        } else if (list->count == list->shown) {
            list->top--;
            list->shown--;
        }
        if (list->cursor == list->count)
            list->cursor--;
    } else {
        switch (fdlgtab) {
        case 2:
            *(unsigned int *)fdlgmask &= 0xFF7F;
            break;
        case 1:
            *(unsigned int *)fdlgmask &= 0xFFBF;
            break;
        case 0:
            *(unsigned int *)fdlgmask &= 0xFFDF;
            break;
        }
        if (lgfiles[0] != 0)
            fdlgtab = 0;
        else if (pofiles[0] != 0)
            fdlgtab = 1;
        else if (exhfiles[0] != 0)
            fdlgtab = 2;
        else {
            fdlgtab = -1;
            *(unsigned int *)fdlgmask = 0x10;
        }
        DrawFileDlgTabs();
    }
done:
    return s.result;
}
