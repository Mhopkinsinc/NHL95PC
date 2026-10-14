/* Line editor: show the selected player's season stats. */
#define MenuShowPlayerStats MenuShowPlayerStats_decl
#define RunMenu RunMenu_decl
#define DressPlayer DressPlayer_decl
#include "nhl95.h"
#undef MenuShowPlayerStats
#undef RunMenu
#undef DressPlayer

void RunMenu(void *bar, int n, int col1, int col2, int col3);

/* MenuShowPlayerStats: line-editor menu action. nums is the 28h jersey numbers, side is the roster,
 sel points at the roster row (negative skips the status line). Loads the player, skater and goalie
 stat shapes, runs the stats menu, then redraws the line editor and prints "<num> <name>" for that
 row. A scratched player (status 2) uses colour 7Eh. Returns 0. The caller passes six stack arguments;
 the last two are unused here. */
int MenuShowPlayerStats(unsigned char *nums, int ignored, unsigned char side, int *sel, int art,
                        int zero, int buttons, int extra, int u1, int u2)
{
    char buf[32];
    int z;

    statsplayerbuf = sub_8CCA8((char *)str_Keys3, 0x5B0, 0);
    rosterpstat = sub_8CCA8((char *)str_Pstat3, 0x497, 0);
    rostergstat = (unsigned char *)sub_8CCA8((char *)str_Gstat3, 0x10E, 0);
    ShowRosterStats(0xC0, 0xC1, 0xC2, 0xC3);
    RunMenu(unk_D075E, 2, 0xC0, 0xC1, 0xC2);
    jctime((int)rostergstat);
    jctime(rosterpstat);
    jctime(statsplayerbuf);
    z = 0;
    statsplayerbuf = z;
    rostergstat = (unsigned char *)z;
    rosterpstat = z;
    SetDrawBitmap(fullscrbmp);
    sub_B392C(z);
    DrawLineEditorScreen(side, art, zero, buttons, extra);
    DrawLineJerseys(nums, art, side);
    sub_B4BA8();
    sub_9121C(((int *)fullscrbmp)[0x2C / 4]);
    if (*(volatile int *)sel >= 0) {
        if (rosterstat[side * 0x2F4 + *sel * 27] == 2)
            sub_8E9C0(0x7E, 0xC1);
        else
            sub_8E9C0(0xC0, 0xC1);
        sprintf(buf, (char *)str_C2dS2, rosterlist[side * 0x2F4 + *sel * 27],
                rosterjersey[side * 0x2F4 + *sel * 27], rosterlist + side * 0x2F4 + *sel * 27 + 8);
        PrintLineEdStatus(0x1FA, *sel * 13 + 0x16, buf);
    }
    return 0;
}

/* DressPlayer: line-editor menu action. Marks the selected roster row as dressed (status 3),
   redraws the jerseys, and prints the same "<num> <name>" status line as MenuShowPlayerStats.
   The exit pops land in MenuShowPlayerStats_ret (ret 18h), so the stack arguments match. */
int DressPlayer(unsigned char *nums, int ignored, unsigned char side, int *sel, int art,
                int zero, int buttons, int extra, int u1, int u2)
{
    char buf[32];

    (void)ignored;
    (void)zero;
    (void)buttons;
    (void)extra;
    (void)u1;
    (void)u2;
    rosterstat[side * 0x2F4 + *sel * 27] = 3;
    DrawLineJerseys(nums, art, side);
    sub_8E9C0(0xC0, 0xC1);
    sprintf(buf, (char *)str_C2dS2, rosterlist[side * 0x2F4 + *sel * 27],
            rosterjersey[side * 0x2F4 + *sel * 27],
            rosterlist + side * 0x2F4 + *sel * 27 + 8);
    PrintLineEdStatus(0x1FA, *sel * 13 + 0x16, buf);
    return 0;
}
