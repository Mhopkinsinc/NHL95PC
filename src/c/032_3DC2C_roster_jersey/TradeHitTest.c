/* Roster and jersey: trade screen hit test. */
/* DRAFT: 247 vs 252. out is spilled correctly. x is in edi and y in ebp (original esi, edi), so the
 button edge lands in esi instead of ebp and y+28h is an add rather than lea. Declaring the edge first
 or last did not swap x and y. Do not retry that. */
#define TradeHitTest TradeHitTestDecl
#include "nhl95.h"
#undef TradeHitTest

/* TradeHitTest: x is tested 4 pixels to the right. A point over the four trade buttons (tradebtnx/y,
 3Ah by 28h, stride 8) writes the button into out[slot], or when slot is negative writes slot/2 and
 sets flag to -1. A point in the roster columns (y at least 1Dh, under byte_D42C3 * 28 rows) writes
 the side (x past BEh) into slot and the row + 64h into out[slot]. Returns 1 for a hit. */
int TradeHitTest(int x, int y, int *out, int *slot, int *flag)
{
    int hit;
    int i;
    int row;
    int e;

    hit = 0;
    *flag = hit;
    x += 4;
    if (x > 0xBE && x < 0x1C0) {
        for (i = 0; i < 4; i++) {
            e = tradebtnx[i * 2];
            if (x < e) continue;
            e += 0x3A;
            if (x >= e) continue;
            e = tradebtny[i * 2];
            if (y < e) continue;
            if (y >= e + 0x28) continue;
            if (*slot >= 0) out[*slot] = i;
            else {
                *slot = i / 2;
                out[*slot] = i;
                *flag = -1;
            }
            hit = 1;
        }
    } else if (y >= 0x1D) {
        e = byte_D42C3;
        if (y < e * 28 + 0x1D) {
            *slot = x >= 0xBE;
            row = (y - 0x1D) / e;
            out[*slot] = row + 0x64;
            hit = 1;
        }
    }
    return hit;
}
