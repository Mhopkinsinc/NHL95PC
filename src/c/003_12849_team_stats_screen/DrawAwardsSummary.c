/* Team stats screen: the awards summary. */
/* DRAFT: same size, all but two register picks (the team byte in dl, the EXE dh; the counter increment in ebx,
   the EXE ecx); see docs/UNSURE.md. Its match belongs with PickAwardWinners (untouched), whose
   PickAwardWinners_x tail it uses. */
#define DrawAwardsSummary DrawAwardsSummary_decl
#include "nhl95.h"
#undef DrawAwardsSummary

int strnicmp(const char *a, const char *b, unsigned n);  /* Watcom CRT strnicmp_ */

typedef struct { char c[25]; } Str25;

/* DrawAwardsSummary (13188, PC only; ends in PickAwardWinners_x): list the 11 awards (awardnames) at x 14h,
   one row of 19 pixels each from y 8Ch: for a team award (awardtype 2) the cup or Presidents trophy team
   (cupteam / presidentsteam + 5, with the C03C4 or C03C6 suffix; "Mighty Ducks" for ANA), else the winner
   (FitPlayerName at x B4h) and the winner's team (treeteamnames, suffix by team) at x 190h. No arguments, no
   result. */
void DrawAwardsSummary(void)
{
    struct {
        char buf[0x2C];
        int i;
    } s;
    int y;
    int k;

    s.i = 0;
    do {
        k = s.i;
        y = k * 19 + 0x8C;
        PrintShadowText(0x14, y, (char *)awardnames[k]);
        if (awardtype[k] == 2) {
            if (!strnicmp(s.buf, (char *)str_ANA, 3)) {
                PrintShadowText(0x190, y, (char *)str_MightyDucks);
            } else {
                strcpy(s.buf, k == 0xA ? (char *)cupteam + 5 : (char *)presidentsteam + 5);
                if (!strnicmp(s.buf, (char *)str_Cal, 3) || !strnicmp(s.buf, (char *)str_Flo, 3))
                    strcat(s.buf, (char *)unk_C03C4);
                else
                    strcat(s.buf, (char *)unk_C03C6);
                PrintShadowText(0x190, y, s.buf);
            }
        } else {
            FitPlayerName(s.buf, (char *)awardwinners + k * 52 + 3, (char *)awardwinners + k * 52 + 0x13, 0xD2);
            PrintShadowText(0xB4, y, s.buf);
            strcpy(s.buf, (char *)treeteamnames + awardwinners[k * 52] * 21);
            if (!strnicmp(s.buf, (char *)str_ANA, 3)) {
                *(Str25 *)s.buf = *(Str25 *)str_MightyDucks;
            } else if (awardwinners[k * 52] == 2 || awardwinners[k * 52] == 0x18 || awardwinners[k * 52] == 0x19) {
                strcat(s.buf, (char *)unk_C03C4);
            } else {
                strcat(s.buf, (char *)unk_C03C6);
            }
            PrintShadowText(0x190, y, s.buf);
        }
    } while (++s.i < 0xB);
}
