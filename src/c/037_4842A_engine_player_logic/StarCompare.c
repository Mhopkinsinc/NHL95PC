/* Three stars: qsort order for the player keys. */
#define StarCompare StarCompare_decl
#include "nhl95.h"
#undef StarCompare

int StarElig(int side, int pl);
#pragma aux StarElig "StarEligible" parm [eax] [edx] value [eax] modify [eax edx];

/* StarCompare: qsort comparator. Each key is team * 25 + player. Points are the first two words of the
 16-byte row at dword_DB088; the game-winner row gets two extra. Ties go to the low word, then the word
 at +0Eh, then the word at +6 when either is positive, then which player is eligible, then the byte total
 at unk_DACA0 (20 bytes per player, 500 per team). A higher total sorts first. */
int StarCompare(int *a, int *b)
{
    int ta;
    int tb;
    int pa;
    int pb;
    int sa;
    int sb;
    char *ra;
    char *rb;
    unsigned char *ua;
    unsigned char *ub;

    ta = *a / 25;
    tb = *b / 25;
    pa = *a % 25;
    pb = *b % 25;
    ra = (char *)dword_DB088 + ta * 25 * 16;
    ra += pa * 16;
    rb = (char *)dword_DB088 + tb * 25 * 16;
    rb += pb * 16;
    sa = ((short *)ra)[1] + ((short *)ra)[0];
    if (ta == gwgteam && pa == gwgplayer) sa += 2;
    sb = ((short *)rb)[1] + ((short *)rb)[0];
    if (tb == gwgteam && pb == gwgplayer) sb += 2;
    if (sa != sb) return sb - sa;
    if (*(short *)rb != *(short *)ra) return *(short *)rb - *(short *)ra;
    if (sa > 0 && ta != tb) {
        int ds;

        ds = hmscore[tb * 128] - hmscore[ta * 128];
        if (ds != 0 && ((ds > 0) ^ tb)) return ds;
    }
    if (*(short *)(rb + 0xE) != *(short *)(ra + 0xE)) {
        int hb;
        int ha;

        hb = *(int *)(rb + 0xC);
        ha = *(int *)(ra + 0xC);
        return HIWORD(hb) - HIWORD(ha);
    }
    if (*(short *)(rb + 6) > 0 || *(short *)(ra + 6) > 0) {
        int hb;
        int ha;

        hb = *(int *)(rb + 4);
        ha = *(int *)(ra + 4);
        return HIWORD(hb) - HIWORD(ha);
    }
    sa = StarElig(ta, pa);
    sb = StarElig(tb, pb);
    if (sa != sb) {
        if (sb) return 1;
        return -1;
    }
    ua = unk_DACA0 + ta * 25 * 20;
    ua += pa * 20;
    ub = unk_DACA0 + tb * 25 * 20;
    ub += pb * 20;
    sb = ub[1] + ub[2] + ub[4] + ub[6] + ub[9] + ub[10] + ub[3] + ub[13] + ub[11] + ub[5];
    sa = ua[1] + ua[2] + ua[4] + ua[6] + ua[9] + ua[10] + ua[3] + ua[13] + ua[11] + ua[5];
    return sb - sa;
}
