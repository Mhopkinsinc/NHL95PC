/* File dialogs: saved-game name lists. */
#define ScanSavedGames ScanSavedGames_decl
#include "nhl95.h"
#undef ScanSavedGames

typedef struct FindT {
    char reserved[21];
    char attrib;
    unsigned short wr_time;
    unsigned short wr_date;
    unsigned long size;
    char name[13];
} FindT;

/* ScanSavedGames: list up to 32 exhibition (*.LP), playoff (*.PO) and league (*.NHL) save names
 into the three file tables. Each name is copied up to its dot, at most 8 characters, into savefname.
 The tables are sorted, the cursor and top are cleared, and the shown count is count - 1, or 5 when
 there are 6 or more. The league search uses attribute 0. */
void ScanSavedGames(void)
{
    FindT ft;
    int n;
    int j;
    unsigned char c;
    unsigned char b;

    n = 0;
    exhfiles[0] = n;
    if (!unknown_libname_1((char *)str_Lp, 0x10, &ft)) {
        do {
            exhfilenames[exhfiles[0]] = (int)(savefname + n);
            j = 0;
            do {
                c = ft.name[j];
                if (c == 0 || c == '.') break;
                savefname[n] = c;
                j++;
                n++;
            } while (j < 8);
            savefname[n] = 0;
            n++;
            exhfiles[0]++;
            if (exhfiles[0] >= 0x20) break;
        } while (!unknown_libname_2(&ft));
        qsort(exhfilenames, exhfiles[0], 4, CmpFileNames);
        exhfiles[2] = 0;
        exhfiles[1] = 0;
        exhfiles[3] = exhfiles[0] < 6 ? exhfiles[0] - 1 : 5;
    }
    pofiles[0] = 0;
    if (!unknown_libname_1((char *)str_Po, 0x10, &ft)) {
        do {
            pofilenames[pofiles[0]] = (int)(savefname + n);
            j = 0;
            do {
                b = ft.name[j];
                if (b == 0 || b == '.') break;
                savefname[n] = b;
                j++;
                n++;
            } while (j < 8);
            savefname[n] = 0;
            n++;
            pofiles[0]++;
            if (pofiles[0] >= 0x20) break;
        } while (!unknown_libname_2(&ft));
        qsort(pofilenames, pofiles[0], 4, CmpFileNames);
        pofiles[2] = 0;
        pofiles[1] = 0;
        pofiles[3] = pofiles[0] < 6 ? pofiles[0] - 1 : 5;
    }
    lgfiles[0] = 0;
    if (!unknown_libname_1((char *)str_Nhl2, 0, &ft)) {
        do {
            lgfilenames[lgfiles[0]] = (int)(savefname + n);
            j = 0;
            do {
                c = ft.name[j];
                if (c == 0 || c == '.') break;
                savefname[n] = c;
                j++;
                n++;
            } while (j < 8);
            savefname[n] = 0;
            n++;
            lgfiles[0]++;
            if (lgfiles[0] >= 0x20) break;
        } while (!unknown_libname_2(&ft));
        qsort(lgfilenames, lgfiles[0], 4, CmpFileNames);
        lgfiles[2] = 0;
        lgfiles[1] = 0;
        lgfiles[3] = lgfiles[0] < 6 ? lgfiles[0] - 1 : 5;
    }
}
