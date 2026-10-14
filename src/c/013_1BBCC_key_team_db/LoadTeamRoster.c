/* Key and team database: load one team's skaters and goalies. */
#define LoadTeamRoster LoadTeamRoster_decl
#include "nhl95.h"
#undef LoadTeamRoster

/* LoadTeamRoster: read one team's 25 skaters and 3 goalies into the roster. mode 0 and 2 rate each
 player from the overall bytes; any other mode copies the lineup slot. The exit jumps to OpenPlayerDBs_x. */
void LoadTeamRoster(int mode, int f1, int f2, int f3, int f4, int n4, int team)
{
    unsigned char buf[0xF0];
    int i;
    int id;
    int slot;
    int base;
    unsigned char *p;

    for (i = 0; i < 25; i++) {
        id = dword_DBC7C[team * 186 + i];
        slot = i * 39;
        base = team * 1092;
        p = hmroster + base + slot;
        if (id < 0) {
            p[0] = 0;
        } else {
            ReadPlayerRecs(f1, f2, f3, f4, id, buf + 0x29, buf + 0x39, 0x14,
                buf + 0x50, 0x2F, buf + 0xA0, n4);
            hmrosterjersey[base + slot] = buf[0x39];
            byte_DB3AE[base + slot] = buf[0x3A];
            strcpy((char *)p + 7, (char *)buf + 0x3B);
            strcpy((char *)p + 0x17, (char *)buf + 0x4B);
            if (mode == 0 || mode == 2) {
                if (byte_DC268 < buf[0x92])
                    p[0] = 1;
                else if (byte_DC268 > buf[0x92])
                    p[0] = 3;
                else if (byte_DC267 < buf[0x93])
                    p[0] = 1;
                else
                    p[0] = 3;
            } else {
                id = ((short *)((char *)hmtmpdst_m2 + (team << 8)))[i];
                if (id > 0)
                    p[0] = 5;
                else
                    p[0] = byte_C66B4[(-id) * 2];
            }
            *(struct { unsigned char b[20]; } *)(unk_DACA0 + team * 484 + i * 20) =
                *(struct { unsigned char b[20]; } *)(buf + 0xC4);
            if ((mode & 6) == 0) {
                base = team * 400 + i * 16;
                if (((unsigned char *)&gameopts)[1] & 2) {
                    dword_DEB74[base / 4] = *(unsigned short *)(buf + 0xC0);
                    dword_DEB78[base / 4] = *(unsigned short *)(buf + 0xC0) + *(unsigned short *)(buf + 0xC2);
                    dword_DEB7C[base / 4] = *(unsigned short *)(buf + 0x6E);
                    dword_DEB80[base / 4] = *(unsigned short *)(buf + 0x6E) + *(unsigned short *)(buf + 0x70);
                } else {
                    dword_DEB74[base / 4] = 0;
                    dword_DEB78[base / 4] = 0;
                    dword_DEB7C[base / 4] = *(unsigned short *)(buf + 0x80);
                    dword_DEB80[base / 4] = *(unsigned short *)(buf + 0x80) + *(unsigned short *)(buf + 0x82);
                }
            }
        }
    }
    for (i = 0; i < 3; i++) {
        id = dword_DBCE0[team * 186 + i];
        slot = (i + 25) * 39;
        base = team * 1092;
        p = hmroster + base + slot;
        if (id < 0) {
            p[0] = 0;
        } else {
            ReadPlayerRecs(f1, f2, f3, f4, id, buf + 0x29, buf + 0x39, 0x10,
                buf + 0x50, 0x36, 0, 0);
            hmrosterjersey[base + slot] = buf[0x39];
            byte_DB3AE[base + slot] = buf[0x3A];
            strcpy((char *)p + 7, (char *)buf + 0x3B);
            strcpy((char *)p + 0x17, (char *)buf + 0x4B);
            if (mode == 0 || mode == 2) {
                if (byte_DC268 < buf[0x2E])
                    p[0] = 1;
                else if (byte_DC268 > buf[0x2E])
                    p[0] = 3;
                else if (byte_DC267 < buf[0x2F])
                    p[0] = 1;
                else
                    p[0] = 3;
            } else {
                id = ((short *)((char *)dword_DF6C2 + (team << 8)))[i];
                if (id > 0)
                    p[0] = 5;
                else
                    p[0] = byte_C66B4[(-id) * 2];
            }
            *(struct { unsigned char b[16]; } *)(unk_DAC40 + team * 48 + i * 16) =
                *(struct { unsigned char b[16]; } *)(buf + 0xD8);
        }
    }
}
