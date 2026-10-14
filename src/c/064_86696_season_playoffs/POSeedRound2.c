/* Season playoffs: seed the second round from the first-round winners. */
#define POSeedRound2 POSeedRound2_decl
#define SeriesWinner SeriesWinner_decl
#define POSetSeriesTeams POSetSeriesTeams_decl
#include "nhl95.h"
#undef POSeedRound2
#undef SeriesWinner
#undef POSetSeriesTeams

int SeriesWinner(unsigned char *s, unsigned games);
#pragma aux SeriesWinner parm [eax] [edx] value [eax] modify [eax edx];
void POSetSeriesTeams(unsigned char *s, int a, int b);
#pragma aux POSetSeriesTeams parm [eax] [edx] [ebx] modify [eax edx ebx];

void __STOSD(void *p, int v, int n);
#pragma aux __STOSD "__STOSD" parm [eax] [edx] [ecx] modify [eax edx ecx];

/* DRAFT: 736 vs 720 (best of 5). The frame, the eight-score scan, both date fills and
 __STOSD match. Each SeriesWinner result is copied with mov edx, eax before the home byte
 is loaded, so the byte lands in eax and the compare is swapped (16 bytes, eight calls).
 Do not retry: named unsigned chars for the scan (al/ah/dh/dl instead of ah/dl/dh/al),
 one int for both the day remainder and the home byte (spills, sub esp, 34h), or
 assigning the home byte to k, the filter counter that lives in edx.
 */
/* POSeedRound2: walk back from the last first-round game while all eight series
 still have an empty score, then date the two second-round series (14 games each,
 every other day, rolling the month at 30). The four winners on each side fill
 the next series; a winner that is the home team is kept in the low slots and
 the other winner in the high slots, and only team numbers below 26 are passed
 on. POSeedRound2_x30 is the add esp. */
void POSeedRound2(unsigned char *p)
{
    struct {
        int slot[8];
        int out[4];
    } s;
    unsigned char *g;
    unsigned char *ser;
    int i;
    int k;
    int div;
    int rem;

    g = p + 0x19BC;
    i = 0;
    do {
        if (g[4] != 0xFF || g[0x2E] != 0xFF || g[0x58] != 0xFF || g[0x82] != 0xFF
            || g[0xAC] != 0xFF || g[0xD6] != 0xFF || g[0x100] != 0xFF || g[0x12A] != 0xFF)
            break;
        i++;
        g -= 6;
    } while (i < 6);
    s.out[0] = g[0];
    s.out[1] = g[1] + 3;
    ser = p + 0x1AE8;
    for (i = 0; i < 0xE;) {
        div = 7;
        ser[0] = s.out[0];
        rem = i % div;
        rem += rem;
        rem += s.out[1];
        ser[1] = rem;
        if (ser[1] > 0x1E) {
            ser[0]++;
            ser[1] -= 0x1E;
        }
        i++;
        ser += 6;
    }
    s.out[1]--;
    ser = p + 0x1B3C;
    i = 0;
    div = 7;
    do {
        ser[0] = s.out[0];
        rem = i % div;
        rem += rem;
        rem += s.out[1];
        ser[1] = rem;
        if (ser[1] > 0x1E) {
            ser[0]++;
            ser[1] -= 0x1E;
        }
        i++;
        ser += 6;
    } while (i < 0xE);
    __STOSD(s.slot, -1, 8);
    ser = p + 0x1998;
    s.out[0] = SeriesWinner(ser, *(int *)(p + 0x50));
    k = ser[2];
    if (s.out[0] == k) s.slot[0] = k;
    else s.slot[7] = s.out[0];
    ser += 0x2A;
    s.out[1] = SeriesWinner(ser, *(int *)(p + 0x54));
    k = ser[2];
    if (s.out[1] == k) s.slot[1] = k;
    else s.slot[6] = s.out[1];
    ser += 0x2A;
    s.out[2] = SeriesWinner(ser, *(int *)(p + 0x58));
    k = ser[2];
    if (s.out[2] == k) s.slot[2] = k;
    else s.slot[5] = s.out[2];
    ser += 0x2A;
    s.out[3] = SeriesWinner(ser, *(int *)(p + 0x5C));
    k = ser[2];
    if (s.out[3] == k) s.slot[3] = k;
    else s.slot[4] = s.out[3];
    i = 0;
    k = 0;
    for (; i < 8; i++) {
        if (s.slot[i] >= 0 && s.slot[i] < 0x1A)
            s.out[k++] = s.slot[i];
    }
    ser = p + 0x1AE8;
    POSetSeriesTeams(ser, s.out[0], s.out[3]);
    POSetSeriesTeams(ser + 0x2A, s.out[1], s.out[2]);
    __STOSD(s.slot, -1, 8);
    ser = p + 0x1A40;
    s.out[0] = SeriesWinner(ser, *(int *)(p + 0x60));
    k = ser[2];
    if (s.out[0] == k) s.slot[0] = k;
    else s.slot[7] = s.out[0];
    ser += 0x2A;
    s.out[1] = SeriesWinner(ser, *(int *)(p + 0x64));
    k = ser[2];
    if (s.out[1] == k) s.slot[1] = k;
    else s.slot[6] = s.out[1];
    ser += 0x2A;
    s.out[2] = SeriesWinner(ser, *(int *)(p + 0x68));
    k = ser[2];
    if (s.out[2] == k) s.slot[2] = k;
    else s.slot[5] = s.out[2];
    ser += 0x2A;
    s.out[3] = SeriesWinner(ser, *(int *)(p + 0x6C));
    k = ser[2];
    if (s.out[3] == k) s.slot[3] = k;
    else s.slot[4] = s.out[3];
    i = 0;
    k = 0;
    for (; i < 8; i++) {
        if (s.slot[i] >= 0 && s.slot[i] < 0x1A)
            s.out[k++] = s.slot[i];
    }
    ser = p + 0x1B3C;
    POSetSeriesTeams(ser, s.out[0], s.out[3]);
    POSetSeriesTeams(ser + 0x2A, s.out[1], s.out[2]);
}
