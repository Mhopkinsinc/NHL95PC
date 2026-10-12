/* Schedule: playoff series length. */
/* DRAFT: 116 vs 116. The loop, the jump table, and every case body match. The shared
 `mov eax, esi / pop esi / pop edx / pop ecx / ret` is emitted after `mov esi, 5` (ja and the other
 returns jump forward to +57). The EXE falls out of `mov esi, 1` into that epilogue at +39 and the
 later cases jump back. Do not retry: Watcom sinks this multi-predecessor return onto the first
 conditional fall-through (the best-of-5 arm), and a return glued to case 0 is either folded to
 `mov eax, 1` or jumped to an epilogue at the end. */
#include "nhl95.h"

/* SeriesLength: how many games a series record s (7 games, 6 bytes each) is played to. Count games until a
 blank date (byte 0 = FFh), at most 7. One game is best of 1; two is best of 3; three is best of 5 when
 SeriesWinner over 5 games has a winner, else best of 3; four or five is best of 7 when SeriesWinner over
 7 games has a winner, else best of 5; six or seven is best of 7. No games played leaves the length local
 unset, so the result is whatever esi held on entry. */
int SeriesLength(unsigned char *s)
{
    int n;
    unsigned char *p;
    int len;

    p = s;
    n = 0;
    do {
        if (p[n * 6] == 0xFF) break;
        n++;
    } while (n < 7);
    switch (n - 1) {
    default:
        goto done;
    case 0:
        goto one;
    case 1:
        goto three;
    case 2:
        goto n3;
    case 3:
        goto n4;
    case 4:
        goto n4;
    case 5:
        goto n7;
    case 6:
        goto n7;
    }
one:
    len = 1;
done:
    return len;
three:
    len = 3;
    goto done;
n3:
    if (SeriesWinner(p, 5) < 0) goto three;
    goto five;
n4:
    if (SeriesWinner(p, 7) < 0) goto five;
n7:
    len = 7;
    goto done;
five:
    len = 5;
    goto done;
}
