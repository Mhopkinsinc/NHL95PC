/* Key team database: fill the goalie menu items from the dressed goalies.
 DRAFT: 797 vs 813 (best of 2). Cookie 0x38 and sub esp,20h match, and the
 template copy, the ah jersey compare, strncpy and fputchar+5 are in the right
 order. The roster index is mov al / and eax,0ffh (original xor edx,edx / mov dl)
 and the tens-digit store is sunk to the join, so the add stays in eax. An int
 dig for the digit add pushes edi and grows the cookie to 0x3c. Do not retry a
 named int for that add. */
#include "nhl95.h"

/* SetupGoalieMenu: for each dressed goalie, copy the menu template, patch the jersey
 number into the digit placeholders, the byte at player+7, and 16 bytes of the name,
 then store the text width plus 5. The two goalie rows share the wider width, and the
 dressed goalie's item (or "none" when the goalie index is negative) is marked on. */
void SetupGoalieMenu(void)
{
    unsigned char buf[0x20];
    unsigned char *m;
    unsigned char *p;
    unsigned char g;
    int off;
    int text;

    m = unk_CEE4F;
    g = hmgoalieidx[0];
    off = g;
    off <<= 2;
    off += g;
    off <<= 3;
    off -= g;
    p = hmroster + off;
    strcpy((char *)buf, (char *)mi_HomeGoalie1);
    if (p[5] < 10)
        buf[2] = buf[1];
    else
        buf[2] = (*(int *)(buf - 1) >> 24) + p[5] / 10;
    buf[3] = (*(int *)buf >> 24) + p[5] % 10;
    buf[5] = p[7];
    strncpy((char *)buf + 8, (char *)p + 0x17, 16);
    *(int *)(m + 8) = fputchar((char *)buf) + 5;
    strcpy((char *)*(int *)(m + 0x10), (char *)buf);

    g = hmgoalieidx[1];
    off = g;
    off <<= 2;
    off += g;
    off <<= 3;
    off -= g;
    p = hmroster + off;
    strcpy((char *)buf, (char *)*(int *)(m + 0x30));
    if (p[5] < 10)
        buf[2] = buf[1];
    else
        buf[2] = (*(int *)(buf - 1) >> 24) + p[5] / 10;
    buf[3] = (*(int *)buf >> 24) + p[5] % 10;
    buf[5] = p[7];
    strncpy((char *)buf + 8, (char *)p + 0x17, 16);
    *(int *)(m + 0x28) = fputchar((char *)buf) + 5;
    strcpy((char *)*(int *)(m + 0x30), (char *)buf);

    if (*(int *)(m + 8) > *(int *)(m + 0x28)) {
        *(int *)(m + 0x48) = *(int *)(m + 8);
        *(int *)(m + 0x28) = *(int *)(m + 8);
    } else {
        *(int *)(m + 0x48) = *(int *)(m + 0x28);
        *(int *)(m + 8) = *(int *)(m + 0x28);
    }
    if (hmtmgoalie[0] < 0)
        text = *(int *)(m + 0x50);
    else
        text = *(int *)(m + 0x10 + ((*(int *)((char *)&hmtmgoalie[0] - 2) >> 16) << 5));
    *(unsigned char *)text = 1;

    m = unk_CEEAF;
    g = awgoalieidx[0];
    off = g;
    off <<= 2;
    off += g;
    off <<= 3;
    off -= g;
    p = awroster + off;
    strcpy((char *)buf, (char *)mi_AwayGoalie1);
    if (p[5] < 10)
        buf[2] = buf[1];
    else
        buf[2] = (*(int *)(buf - 1) >> 24) + p[5] / 10;
    buf[3] = (*(int *)buf >> 24) + p[5] % 10;
    buf[5] = p[7];
    strncpy((char *)buf + 8, (char *)p + 0x17, 16);
    *(int *)(m + 8) = fputchar((char *)buf) + 5;
    strcpy((char *)*(int *)(m + 0x10), (char *)buf);

    g = awgoalieidx[1];
    off = g;
    off <<= 2;
    off += g;
    off <<= 3;
    off -= g;
    p = awroster + off;
    strcpy((char *)buf, (char *)*(int *)(m + 0x30));
    if (p[5] < 10)
        buf[2] = buf[1];
    else
        buf[2] = (*(int *)(buf - 1) >> 24) + p[5] / 10;
    buf[3] = (*(int *)buf >> 24) + p[5] % 10;
    buf[5] = p[7];
    strncpy((char *)buf + 8, (char *)p + 0x17, 16);
    *(int *)(m + 0x28) = fputchar((char *)buf) + 5;
    strcpy((char *)*(int *)(m + 0x30), (char *)buf);

    if (*(int *)(m + 8) > *(int *)(m + 0x28)) {
        *(int *)(m + 0x48) = *(int *)(m + 8);
        *(int *)(m + 0x28) = *(int *)(m + 8);
    } else {
        *(int *)(m + 0x48) = *(int *)(m + 0x28);
        *(int *)(m + 8) = *(int *)(m + 0x28);
    }
    if (awtmgoalie < 0)
        text = *(int *)(m + 0x50);
    else
        text = *(int *)(m + 0x10 + ((*(int *)((char *)&awtmgoalie - 2) >> 16) << 5));
    *(unsigned char *)text = 1;
}
