/* Gadgets: load the team rock cues and the three tune banks. */
#define LoadRockMusic LoadRockMusic_decl
#define sub_8F13B sub_8F13B_decl
#include "nhl95.h"
#undef LoadRockMusic
#undef sub_8F13B

int sub_8F13B(char *path);

/* LoadRockMusic: team 26 and above uses cue row 13. Six team cues, then three random cues that are
 not already on that row and not repeated, then the device bank, the team tune, and ROCKDITI.
 dword_CCC94 is 20h while the banks load and 0 afterwards. */
void LoadRockMusic(void)
{
    struct {
        char path[0x10];
        int picked[3];
        int team;
    } s;
    int i;
    int slot;

    dword_CCC94 = 0x20;
    s.team = *(int *)((char *)&HomeTeam - 2) >> 16;
    if (s.team >= 0x1A) s.team = 0x0D;
    i = 0;
    do {
        int team;
        int t3;
        int cue;

        team = s.team;
        t3 = team << 2;
        t3 -= team;
        cue = ((signed char *)teamrockcuetbl)[i + t3 * 2 + 3];
        if (cue >= 0) {
            MakePath(s.path, fileoncd[rockcuepathsel[cue]] == 1 ? (char *)cddriveptr : 0,
                (char *)rockcuefiles[cue], 0);
            rockteamcues[i] = sub_8F13B(s.path);
        } else {
            slot = 0;
            rockteamcues[i] = slot;
        }
        i++;
    } while (i < 6);
    i = 0;
    do {
        int ok;
        int j;

        do {
            ok = -1;
            slot = (rand() & 0x7FFF) % 0x12;
            for (j = 0; j < 6 && ok; j++) {
                int team;
                int t3;
                int cue;

                team = s.team;
                t3 = team << 2;
                t3 -= team;
                cue = ((signed char *)teamrockcuetbl)[j + t3 * 2 + 3];
                if (cue == rockcuepool[slot]) ok = 0;
            }
            for (j = 0; j < i && ok; j++) {
                if (rockcuepool[slot] == s.picked[j]) ok = 0;
            }
        } while (ok == 0);
        s.picked[i] = rockcuepool[slot];
        MakePath(s.path, fileoncd[rockcuepathsel[rockcuepool[slot]]] == 1 ? (char *)cddriveptr : 0,
            (char *)rockcuefiles[rockcuepool[slot]], 0);
        rockrandcues[i] = sub_8F13B(s.path);
        i++;
    } while (i < 3);
    if (sounddev == 8)
        MakePath(s.path, fileoncd[0x100] == 1 ? (char *)cddriveptr : 0, (char *)str_MTROCKU, 0);
    else if (musicon)
        MakePath(s.path, fileoncd[0x166] == 1 ? (char *)cddriveptr : 0, (char *)str_SBROCKU, 0);
    else
        MakePath(s.path, fileoncd[0x1F] == 1 ? (char *)cddriveptr : 0, (char *)str_ADROCKU, 0);
    rocktunes[0] = sub_8F13B(s.path);
    {
        int tune;

        tune = ((signed char *)byte_CC9B0)[*(int *)((char *)&HomeTeam - 2) >> 16];
        MakePath(s.path, fileoncd[teamtunepathsel[tune]] == 1 ? (char *)cddriveptr : 0,
            (char *)teamtunefiles[tune], 0);
    }
    rocktunes[1] = sub_8F13B(s.path);
    MakePath(s.path, fileoncd[0x161] == 1 ? (char *)cddriveptr : 0, (char *)str_ROCKDITI, 0);
    rocktunes[2] = sub_8F13B(s.path);
    dword_CCC94 = 0;
}
