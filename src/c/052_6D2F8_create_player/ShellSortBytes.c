/* Create player: sort records. */
#include "nhl95.h"

/* ShellSortBytes: shell sort n records of width bytes at base. cmp(left, right) < 0 leaves the pair; otherwise
 the records are swapped a byte at a time (xor). The gap starts at n/2 and halves. */
void ShellSortBytes(unsigned char *base, int n, int width, int (*cmp)(void *, void *))
{
    int gap;
    int i;
    int j;
    int k;
    unsigned char *b;
    unsigned char *a;
    unsigned char *p;

    p = base;
    for (gap = n >> 1; gap > 0; gap >>= 1) {
        for (i = gap; i < n; i++) {
            for (j = i - gap; j >= 0; j -= gap) {
                a = p + j * width;
                b = p + (j + gap) * width;
                if (cmp(a, b) < 0) break;
                for (k = 0; k < width; k++) {
                    a[k] ^= b[k];
                    b[k] ^= a[k];
                    a[k] ^= b[k];
                }
            }
        }
    }
}
