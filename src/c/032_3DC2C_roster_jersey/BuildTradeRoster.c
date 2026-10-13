/* Roster jersey: trade list. */
#include "nhl95.h"

/* ReadKeyRec saves ecx, so the roster entry pointer stays there across the read. */
#pragma aux ReadKeyRec parm [eax] [edx] [ebx] value [eax] modify [eax edx];

/* DRAFT: 473 vs 473. Prologue, both file opens, the skater and goalie walks, and the key-record
 copies match. The fill loop keeps the index in ebx and stores it through al; the original keeps
 the index in eax and reloads the byte into dl.
 Do not retry: a separate unsigned char for that byte is the same ebx/al pair. Writing the byte
 from the index with no copy reloads eax after al kills it (477 bytes). An aux that reloads dl
 matches the fill, then list is parked in edi; passing list in esi pushes the incoming args and
 shifts the frame. */
/* BuildTradeRoster: fill list (28 entries of 22 bytes) from the team database named in dir.
 Entry i gets index i at +2. The 25 skater slots (team record +4Ch) and the 3 goalie slots (+B0h)
 become entries 0-24 and 25-27. An empty slot (-1) stores a zero position byte; otherwise the key
 record supplies position (+2), number (+1) and "<initial>. <last name>" (+3 and +13h). Returns 0
 or the first file error. Both file handles are closed. */
int BuildTradeRoster(char *dir, int team, unsigned char *list, unsigned char *rec)
{
    struct {
        unsigned char key[0x34];
        char path[0x20];
        int keyfh;
        int teamfh;
        int i;
    } f;
    int err;
    unsigned char *e;
    int k;
    int n;

    f.teamfh = -1;
    f.keyfh = -1;
    f.i = 0;
    do {
        n = f.i;
        list[(n * 11) * 2 + 2] = (unsigned char)f.i;
        f.i = n + 1;
    } while (f.i < 0x1C);
    MakePath(f.path, dir, (char *)leaguedbnames[4], (char *)str_extDB);
    err = FileOpenRead(f.path, &f.teamfh);
    if (!err) err = ReadTeamRec(f.teamfh, rec, team);
    FileClose(&f.teamfh);
    if (!err) {
        MakePath(f.path, dir, (char *)leaguedbnames[0], (char *)str_extDB);
        err = FileOpenRead(f.path, &f.keyfh);
    }
    if (!err) {
        for (f.i = 0; f.i < 0x19 && !err; f.i++) {
            k = ((int *)(rec + 0x4C))[f.i];
            e = list + (f.i * 11) * 2;
            if (k == -1) {
                e[0] = 0;
            } else {
                err = ReadKeyRec(f.keyfh, f.key, k);
                if (!err) {
                    e[0] = f.key[2];
                    e[1] = f.key[1];
                    e[3] = f.key[3];
                    e[4] = str_dot;
                    e[5] = str_space;
                    e[6] = 0;
                    strcat((char *)e + 3, (char *)f.key + 0x13);
                }
            }
        }
        for (f.i = 0; f.i < 3 && !err; f.i++) {
            k = ((int *)(rec + 0xB0))[f.i];
            e = list + ((f.i + 0x19) * 11) * 2;
            if (k == -1) {
                e[0] = 0;
            } else {
                err = ReadKeyRec(f.keyfh, f.key, k);
                if (!err) {
                    e[0] = f.key[2];
                    e[1] = f.key[1];
                    e[3] = f.key[3];
                    e[4] = str_dot;
                    e[5] = str_space;
                    e[6] = 0;
                    strcat((char *)e + 3, (char *)f.key + 0x13);
                }
            }
        }
    }
    FileClose(&f.keyfh);
    return err;
}
