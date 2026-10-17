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

typedef struct FileList {
    int count;          /* +0: files in the list */
    int sel;            /* +4: the selected file */
    int top;            /* +8: the first file shown */
    int bottom;         /* +0Ch: the last file shown */
    char *names[1];     /* +10h: the file names */
} FileList;

typedef struct FRect {
    int x1, y1, x2, y2;
} FRect;

/* DrawFileDlg (2C18F, PC only; ends in LoadFileDlgShapes_x): draw the saved-games dialog. Load the DIALOGBX bank
   (from the CD when listed there) and draw its DBOX art at 0Ah, 13h. The tab is the last of exhibition (2),
   playoffs (1) and league (0) that has saved files (exhfiles, pofiles, lgfiles), each tab's bit 7 / 6 / 5 in
   fdlgmask (bit 4 always); with none, stop there. Else enable the buttons (fdlgmask 20Ch), print the selected
   file's name in the title rect (8) and clear the list rect (0Ah) in colour F8h, print the shown files (top to
   bottom) into rects 0Ah on, and enable one list row per file (fdlgmask bits 0Ah-0Fh); with more than six files
   enable them all and the arrows (FF03h) and draw the scroll bar: rect 8's frame lines in FAh / F8h with the
   thumb 1E0h / count high. No arguments, no result. */
void DrawFileDlg(void)
{
    char path[0x20];
    int bank;
    FileList *l;
    int i;
    int j;
    int x;
    int y;
    int x2;
    int y2;

    sub_B4BA8();
    MakePath(path, fileoncd[0x63] == 1 ? (char *)cddriveptr : 0, (char *)str_Dialogbx, 0);
    bank = sub_8E83C(path, 0);
    sub_91284(sub_B30B4(bank, (char *)str_Dbox), 0xA, 0x13);
    FreeMemBlock(bank);
    *(int *)fdlgmask = 0x10;
    fdlgtab = -1;
    if (exhfiles[0]) {
        *(int *)fdlgmask = 0x90;
        fdlgtab = 2;
    }
    if (pofiles[0]) {
        fdlgmask[0] |= 0x40;
        fdlgtab = 1;
    }
    if (lgfiles[0]) {
        fdlgmask[0] |= 0x20;
        fdlgtab = 0;
    }
    if (fdlgtab < 0) return;
    *(short *)fdlgmask |= 0x20C;
    sub_8E9C0(0xFA, 0xFF);
    l = (FileList *)off_C6F7C[fdlgtab];
    PrintTextCopy(l->names[l->sel], ((FRect *)fdlgrects)[9].x1 + 0xD, ((FRect *)fdlgrects)[9].y1 + 0x13);
    sub_90D20(((FRect *)fdlgrects)[0xA].x1 + 0xA, ((FRect *)fdlgrects)[0xA].y1 + 0x13,
        ((FRect *)fdlgrects)[0xA].x2 - ((FRect *)fdlgrects)[0xA].x1 + 1,
        ((FRect *)fdlgrects)[0xA].y2 - ((FRect *)fdlgrects)[0xA].y1 + 1, 0xF8);
    for (i = l->top, j = 0xA; i <= l->bottom; i++, j++)
        PrintTextCopy(l->names[i], ((FRect *)fdlgrects)[j].x1 + 0xA, ((FRect *)fdlgrects)[j].y1 + 0x10);
    if (l->count > 6) {
        *(short *)fdlgmask |= 0xFF03;
        x = ((FRect *)fdlgrects)[8].x1 + 0xA;
        y = ((FRect *)fdlgrects)[8].y1 + 0x13;
        x2 = ((FRect *)fdlgrects)[8].x2 + 0xA;
        i = 0x1E0 / l->count;
        y2 = ((FRect *)fdlgrects)[8].y1 + 0x11 + i;
        sub_B4FAC(x, y, ((FRect *)fdlgrects)[8].x2 + 9, y, 0xFA);
        sub_B4FAC(x, y, x, y2 - 1, 0xFA);
        sub_B4FAC(x2, y - 1, x2, y2, 0xF8);
        sub_B4FAC(x - 1, y2, x2, y2, 0xF8);
        return;
    }
    switch (l->count) {
    case 6: fdlgmask[1] |= 0x80;
    case 5: fdlgmask[1] |= 0x40;
    case 4: fdlgmask[1] |= 0x20;
    case 3: fdlgmask[1] |= 0x10;
    case 2: fdlgmask[1] |= 8;
    case 1: fdlgmask[1] |= 4;
    }
}
