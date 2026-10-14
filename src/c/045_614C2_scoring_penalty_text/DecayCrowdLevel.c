/* Scoring penalty text: crowd level and rink-object slots. */
/* DRAFT: 705 vs 704. The prologue and the crowdlevel decay match, and the slot
 index is the word at esp. The slot pointer is esi and each spawned id is
 movsx into ebx (original pointer in ebx, id copied with mov esi, eax). Do not
 retry an int copy of crowdlevel, an unsigned slot byte, or dropping the
 address-of on the index. */
#include "nhl95.h"

/* DecayCrowdLevel: ease crowdlevel down (by 2 above 2BCh, by 3 above 15Eh, then always by 1) and, if it
 goes negative, sometimes kick it with a small random. Then walk the 20 rink-object slots (12 bytes at
 word_DEE94). A negative id counts its timer down and spawns a free object; a live id animates or
 releases its bit in word_E9B2C and arms the timer again. */
void DecayCrowdLevel(void)
{
    short i;
    signed char *o;
    int n;

    if (crowdlevel > 0x2BC)
        crowdlevel = crowdlevel - 2;
    else if (crowdlevel > 0x15E)
        crowdlevel = crowdlevel - 3;
    crowdlevel--;
    if (crowdlevel < 0) {
        crowdlevel = 0;
        if (randomd0(0x10) == 0)
            crowdlevel = randomd0(0x20);
    }
    for (i = 0; i < 0x14; i++) {
        o = (signed char *)word_DEE94 + (*(int *)((char *)&i - 2) >> 16) * 12;
        if (*(short *)o < 0) {
            if (--o[2] >= 0)
                continue;
            if (i == 0x12)
                n = randomd0(0x11) + 0x89;
            else if (i == 0x13)
                n = randomd0(0x11) + 0x9A;
            else if (i == 0x11 && (gmode & 1) && (n = randomd0(4)) < 2)
                n += 0x85;
            else {
                n = randomd0(0x85);
                while (word_E9B2C[n / 16] & (1 << (n % 16))) {
                    n++;
                    if (n >= 0x85)
                        n = 0;
                }
            }
            SetRinkObject(i, n);
        } else {
            if (--o[2] >= 0)
                continue;
            if (o[0xB] < 0)
                n = -(*(int *)(o + 8) >> 24);
            else
                n = *(int *)(o + 8) >> 24;
            if ((*(int *)o >> 24) < n) {
                o[2] = 0xC;
                o[3]++;
                o[4] = byte_CCE00[(*(int *)(o + 7) >> 16) + (*(int *)o >> 24)];
                continue;
            }
            if (o[0xB] < 0 && randomd0(2) != 0) {
                o[4] = byte_CCE01[*(int *)(o + 7) >> 16];
                o[2] = 0xC;
                o[3] = 1;
                continue;
            }
            n = *(short *)o;
            word_E9B2C[n / 16] &= ~(1 << (n % 16));
            *(short *)o = -1;
            n = (0x3E8 - (*(int *)((char *)&crowdlevel - 2) >> 16)) / 8;
            if (n < 0x14)
                n = 0x14;
            if (i >= 0x12 && n < 0x64)
                n = 0x64;
            if (i == 0x11 && (gmode & 1))
                n = 0x78;
            else
                n = randomd0(n);
            o[2] = n;
        }
    }
}
