/* LoadFileDlgShapes - unless loaded, load the file dialog button bank OPENBUT (from the CD when listed there) into fdlgshapes and look up its
   shapes: none, delete, open, cancel, no arrow, up, down, arrow and the four game-type tabs (none, exhibition, league, playoffs). */
#define DrawFileDlgTabs DrawFileDlgTabs_decl
#include "nhl95.h"
#undef DrawFileDlgTabs

void LoadFileDlgShapes(void)
{
    char path[32];

    if (!fdlgshapes) {
        MakePath(path, fileoncd[0x1C6] == 1 ? (char *)cddriveptr : 0, (char *)str_OpenBut, 0);
        fdlgshapes = sub_8E83C(path, 0);
        fdlg_none = sub_B30B4(fdlgshapes, (char *)str_None);
        fdlg_del = sub_B30B4(fdlgshapes, (char *)str_Del);
        fdlg_open = sub_B30B4(fdlgshapes, (char *)str_Open);
        fdlg_cancel = sub_B30B4(fdlgshapes, (char *)str_Can);
        fdlg_noarrow = sub_B30B4(fdlgshapes, (char *)str_Noar);
        fdlg_up = sub_B30B4(fdlgshapes, (char *)str_Up);
        fdlg_down = sub_B30B4(fdlgshapes, (char *)str_Down);
        fdlg_arrow = sub_B30B4(fdlgshapes, (char *)str_Arro);
        fdlg_tabnone = sub_B30B4(fdlgshapes, (char *)str_Gtno);
        fdlg_tabexh = sub_B30B4(fdlgshapes, (char *)str_Gtex);
        fdlg_tablp = sub_B30B4(fdlgshapes, (char *)str_Gtlp);
        fdlg_tabpo = sub_B30B4(fdlgshapes, (char *)str_Gtpo);
    }
}

/* DrawFileDlgTabs: file-dialog captions and the tab sprite. A negative tab is the open/delete/done
 page (no arrow, the none tab). Otherwise the league, playoff and exhibition captions are drawn when
 their mask bits are clear, the arrow sprite follows the selected list's count, and tabs 0, 1 and 2
 are the exhibition, playoff and league sprites. */
void DrawFileDlgTabs(void)
{
    sub_8E9C0(0xF8, 0xFF);
    if (fdlgtab < 0) {
        sub_91964((char *)str_Open2, 0x2A, 0xC0);
        sub_91964((char *)str_Delete, 0x6B, 0xC0);
        sub_8E9C0(0xFA, 0xFF);
        sub_91964((char *)str_Done, 0xB8, 0xC0);
        sub_8E9C0(0xF8, 0xFF);
        sub_91964((char *)str_Exhibition, 0xA7, 0x4C);
        sub_91964((char *)str_Playoffs, 0xAB, 0x64);
        sub_91964((char *)str_League, 0xB0, 0x7C);
        sub_91370(fdlg_noarrow, 0x18, 0x46);
        sub_91370(fdlg_tabnone, 0xA3, 0x4B);
    } else {
        if ((fdlgmask[0] & 0x80) == 0)
            sub_91964((char *)str_League, 0xB0, 0x7C);
        if ((fdlgmask[0] & 0x40) == 0)
            sub_91964((char *)str_Playoffs, 0xAB, 0x64);
        if ((fdlgmask[0] & 0x20) == 0)
            sub_92CD0((char *)str_Exhibition, 0xA7, 0x4C);
        if (*(int *)off_C6F7C[fdlgtab] <= 6)
            sub_91370(fdlg_noarrow, 0x18, 0x46);
        else
            sub_91370(fdlg_arrow, 0x18, 0x46);
        switch (fdlgtab) {
        case 0:
            sub_91370(fdlg_tabexh, 0xA3, 0x4B);
            break;
        case 1:
            sub_91370(fdlg_tabpo, 0xA3, 0x4B);
            break;
        case 2:
            sub_91370(fdlg_tablp, 0xA3, 0x4B);
            break;
        }
    }
}
