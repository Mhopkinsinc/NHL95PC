/* Create player: name editing.
 DRAFT: same 323 bytes. stop/first/last sit at esp+0/+4/+8 once they are one struct. The name
 pointer is in edi (original ecx), so the length stays in esi but the copy count stays in ebx
 (original edx) and the word pointer in edx (original edi). An aux that forced ecx reloaded eax
 immediately. Do not retry that. */
#include "nhl95.h"

/* SplitPlayerName: split name into first and last (at most 15 letters each, letters by the character class table
 byte_C4B6C, bits 6-7): skip leading non-letters, copy the first word; if it was cut at 15 skip the rest of it
 skip to the next word and copy it as the last name. */
#define LETTER(c) ((*(int *)((char *)byte_C4B6C + (unsigned char)((c) + 1) - 3) >> 24) & 0xC0)

void SplitPlayerName(char *name, char *first, char *last)
{
    char *nm;
    struct {
        int stop;
        char *first;
        char *last;
    } f;
    int len;
    int i;
    int n;
    char *p;

    nm = name;
    f.first = first;
    f.last = last;
    len = strlen(nm);
    i = 0;
    while (!LETTER(nm[i]) && i < len) i++;
    for (f.stop = n = 0; i < len && n < 15 && !f.stop; i++) {
        p = &nm[i];
        if (LETTER(*p)) {
            f.first[n] = *p;
            n++;
        } else {
            f.stop = -1;
        }
    }
    f.first[n] = 0;
    if (n == 15 && LETTER(nm[i]))
        while (LETTER(nm[i]) && i < len) i++;
    while (!LETTER(nm[i]) && i < len) i++;
    for (f.stop = n = 0; i < len && n < 15 && !f.stop; i++) {
        p = &nm[i];
        if (LETTER(*p)) {
            f.last[n] = *p;
            n++;
        } else {
            f.stop = -1;
        }
    }
    f.last[n] = 0;
}
