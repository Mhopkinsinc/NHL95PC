/* Highlights: choosing a saved highlight. */
#define SelectHilight SelectHilight_decl
#define ListDialog ListDialog_decl
#include "nhl95.h"
#undef SelectHilight
#undef ListDialog

int ListDialog(char **items, int n, char *title, int a, int b, unsigned char *marks);

/* SelectHilight (7FCF8, PC only): pick a saved highlight of the league in dir. List the teams that have a
   highlight file (*.HI2, TeamFromHiName, sorted, by city) and, once one is chosen, the highlights in that team's
   file (9652h bytes each; FormatHilightDesc of each 4Ch-byte header into a 51h-byte string table, HLT / HLTL).
   With multi set the second list can mark several (a cleared byte per highlight, HLTS, returned in *marks) and
   *idx becomes the count. With no highlight files say so (msg_NoHilights). name: gets the team's file name;
   idx: gets the chosen highlight; dir: the league directory; multi, marks: as above. Result: 0, an I/O error,
   or -1 when nothing was chosen. */
int SelectHilight(char *name, int *idx, unsigned char *dir, int multi, unsigned char **marks)
{
    struct {
        char *names[0x1A];
        int teams[0x1A];
        char path[0x54];
        unsigned char hdr[0x4C];
        char ft[0x2C];
        unsigned char tmarks[0x1C];
        int fh;
        int x;
        int y;
        int i;
        char **tbl;
        char *strs;
    } s;
    int n;
    int err;
    int r;
    int size;

    s.fh = -1;
    n = 0;
    MakePath(s.path, (char *)dir, (char *)str_star, (char *)str_HI2);
    if (!unknown_libname_1(s.path, 0, s.ft)) {
        s.teams[0] = TeamFromHiName(s.ft + 0x1E);
        s.names[0] = (char *)teamcitynames[s.teams[0]];
        n = 1;
        do {
            err = unknown_libname_2(s.ft);
            if (!err) {
                s.teams[n] = TeamFromHiName(s.ft + 0x1E);
                n++;
            }
        } while (!err);
    }
    if (n) {
        qsort(s.teams, n, 4, CmpInt);
        for (s.i = 0; s.i < n; s.i++) s.names[s.i] = (char *)teamcitynames[s.teams[s.i]];
        r = ListDialog(s.names, n, (char *)str_SelectATeam, 2, 0, s.tmarks);
        if (r >= 0) {
            MakePath(name, 0, (char *)teamabbrevs[s.teams[r]], (char *)str_HI2);
            MakePath(s.path, (char *)dir, name, 0);
            size = sub_92DE0(s.path);
            err = FileOpenRead(s.path, &s.fh);
            if (!err) {
                n = size / 0x9652;
                s.tbl = (char **)sub_8CCA8((char *)str_HLTL, n * 4, 0x20);
                s.strs = (char *)sub_8CCA8((char *)str_HLT, n * 0x51, 0x20);
                if (multi) {
                    *marks = (unsigned char *)sub_8CCA8((char *)str_HLTS, n, 0x20);
                    for (s.i = 0; s.i < n && !err; s.i++) (*marks)[s.i] = 0;
                }
                for (s.i = 0; s.i < n && !err; s.i++) {
                    err = FileReadAt(s.fh, s.hdr, s.i * 0x9652, 0x4C);
                    s.tbl[s.i] = s.strs + s.i * 0x51;
                    FormatHilightDesc(s.path, s.hdr);
                    strcpy(s.strs + s.i * 0x51, s.path);
                }
                if (!err) {
                    if ((*idx = ListDialog(s.tbl, n, (char *)str_SelectAHilight, 2, multi, *marks)) < 0) err = -1;
                    if (multi) *idx = n;
                }
                FreeMemBlock((int)s.strs);
                FreeMemBlock((int)s.tbl);
            }
            FileClose(&s.fh);
            return err;
        }
    } else {
        sub_B2DCA(&s.i, &s.y, &s.x);
        MessageBox(-1, -1, (char *)msg_NoHilights, 1, 0, 0, (int)&s.y, (int)&s.x, -1);
    }
    return -1;
}
