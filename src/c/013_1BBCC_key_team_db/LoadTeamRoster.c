/* Key and team database: load one team's skaters and goalies. */
#define LoadTeamRoster LoadTeamRoster_decl
#include "nhl95.h"
#undef LoadTeamRoster

/* LoadTeamRoster: read one team's 25 skaters and 3 goalies into the roster. mode 0 and 2 rate each
 player from the overall bytes; any other mode copies the lineup slot. The exit jumps to OpenPlayerDBs_x. */
void LoadTeamRoster(int mode, int f1, int f2, int f3, int f4, int n4, int team)
{
    unsigned char buf[0xEC];
    int i;
    int id;
    int slot;
    int base;
    int offset;
    unsigned char *p;
    int goalieIndex;

    for (i = 0; i < 25; i++) {
        id = *(int *)((char *)dword_DBC7C + team * 744 + i * 4);
        slot = i * 39;
        base = team * 1092;
        offset = base + slot;
        if (id >= 0) {
            ReadPlayerRecs(f1, f2, f3, f4, id, buf + 0x38, buf + 0xC4, 0x14,
                buf + 0x6C, 0x2F, buf + 0xA0, n4);
            hmrosterjersey[offset] = buf[0x39];
            byte_DB3AE[offset] = buf[0x3A];
            p = hmroster + base + slot;
            strcpy((char *)p + 7, (char *)buf + 0x3B);
            strcpy((char *)p + 0x17, (char *)buf + 0x4B);
            if (mode == 0 || mode == 2) {
                offset = i * 39 + team * 1092;
                if (byte_DC268 < buf[0x92])
                    hmroster[offset] = 1;
                else if (byte_DC268 > buf[0x92])
                    hmroster[offset] = 3;
                else if (byte_DC267 < buf[0x93])
                    hmroster[offset] = 1;
                else
                    hmroster[offset] = 3;
            } else {
                id = ((short *)((char *)hmtmpdst_m2 + (team << 8)))[i + 1];
                if (id > 0)
                    hmroster[offset] = 5;
                else
                    hmroster[offset] = byte_C66B4[(-id) * 2];
            }
            *(struct { unsigned char b[20]; } *)(unk_DACA0 + team * 484 + i * 20) =
                *(struct { unsigned char b[20]; } *)(buf + 0xC4);
            if ((mode & 6) == 0) {
                base = team * 100 + i * 4;
                if (((unsigned char *)&gameopts)[1] & 2) {
                    dword_DEB74[base] = *(unsigned short *)(buf + 0xC0);
                    dword_DEB78[base] = *(unsigned short *)(buf + 0xC0) + *(unsigned short *)(buf + 0xC2);
                    dword_DEB7C[base] = *(unsigned short *)(buf + 0x6E);
                    dword_DEB80[base] = *(unsigned short *)(buf + 0x6E) + *(unsigned short *)(buf + 0x70);
                } else {
                    dword_DEB78[base] = dword_DEB74[base] = 0;
                    dword_DEB7C[base] = *(unsigned short *)(buf + 0x80);
                    dword_DEB80[base] = *(unsigned short *)(buf + 0x80) + *(unsigned short *)(buf + 0x82);
                }
            }
        } else {
            hmroster[offset] = 0;
        }
    }
    i = 0;
    goto goalie_test;
goalie_loop:
        goalieIndex = i + 25;
        id = *(int *)((char *)dword_DBCE0 + team * 744 + i * 4);
        slot = (goalieIndex) * 39;
        base = team * 1092;
        offset = base + slot;
        if (id >= 0) {
            ReadPlayerRecs(f1, f2, f3, 0, id, buf + 0x38, buf + 0xD8, 0x10,
                buf, 0x36, 0, 0);
            hmrosterjersey[offset] = buf[0x39];
            byte_DB3AE[offset] = buf[0x3A];
            p = hmroster + base + slot;
            strcpy((char *)p + 7, (char *)buf + 0x3B);
            strcpy((char *)p + 0x17, (char *)buf + 0x4B);
            if (mode == 0 || mode == 2) {
                base = i + 25;
                offset = team * 1092 + (base) * 39;
                if (byte_DC268 < buf[0x2E])
                    hmroster[offset] = 1;
                else if (byte_DC268 > buf[0x2E])
                    hmroster[offset] = 3;
                else if (byte_DC267 < buf[0x2F])
                    hmroster[offset] = 1;
                else
                    hmroster[offset] = 3;
            } else {
                id = ((short *)((char *)dword_DF6C2 + (team << 8)))[i + 1];
                if (id > 0)
                    hmroster[offset] = 5;
                else
                    hmroster[offset] = byte_C66B4[(-id) * 2];
            }
            *(struct { unsigned char b[16]; } *)(unk_DAC40 + team * 48 + i * 16) =
                *(struct { unsigned char b[16]; } *)(buf + 0xD8);
        } else {
            hmroster[offset] = 0;
        }
        i++;
goalie_test:
    if (i < 3) goto goalie_loop;
}
