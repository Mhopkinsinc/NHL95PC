/* Database save: the edit-rosters screen. */
#define RunEditRosters RunEditRosters_decl
#define EditRosters EditRosters_decl
#include "nhl95.h"
#undef RunEditRosters
#undef EditRosters

void EditRosters(unsigned char *menu, int mode, int c, int d, int e);

typedef struct { char c[8]; } Hdr8;
typedef struct { char c[17]; } Hdr17;

/* RunEditRosters: paint the roster editor, run EditRosters, then put the dialog
 colours back. The text state and the five saved colours are one 54h-byte local.
 RunEditRosters_ret6 and _ret5 are the pops other screens jump to. */
void RunEditRosters(void)
{
    struct {
        unsigned char state[0x40];
        int lite;
        int bg;
        int fg;
        int shade;
        int fill;
    } s;
    unsigned char *pal;
    int c40;
    int c42;
    int sz;

    s.lite = boxlitecolor;
    s.shade = boxshadecolor;
    s.fill = boxfillcolor;
    s.fg = dlgtextfg;
    s.bg = dlgtextbg;
    boxfillcolor = 0x41;
    c40 = 0x40;
    boxlitecolor = c40;
    c42 = 0x42;
    boxshadecolor = c42;
    dlgtextfg = c40;
    dlgtextbg = c42;
    pal = 0;
    editrosters_exit = (int)pal;
    dword_D0B12 = (int)pal;
    pal = (unsigned char *)sub_8CCA8((char *)str_Temp7, 0x300, 0x20);
    if (musicon && songdata)
        sub_8FCDF(musichandle, 3, 0x32);
    sub_8FFB0(0, 0x100, pal);
    FadePalStep(1, pal, 0x10);
    *(Hdr8 *)curdbname = *(Hdr8 *)str_Current3;
    sub_B4BA8();
    dword_EA2B0 = mainfont;
    sub_8EA18(dword_EA2B0);
    sub_8E9E8(s.state);
    if (musicon && songdata) {
        while (!sub_8FC8A(*(int *)((char *)&musicslot - 3) >> 24, 3));
        sub_8D2F0(songdata);
        songdata = 0;
    }
    fullscrbmp = sub_B4F8C(0x280, 0x1E0, 0x20);
    sz = ((*(int *)(pointerspr + 2) >> 16) + 1) * ((*(int *)(pointerspr + 4) >> 16) + 1) + 0x11;
    editptrspr = sub_8CCA8((char *)str_Pointer11, sz, 0x20);
    *(Hdr17 *)editptrspr = *(Hdr17 *)pointerspr;
    ((short *)editptrspr)[2] = ((short *)pointerspr)[2] + 1;
    ((short *)editptrspr)[3] = ((short *)pointerspr)[3] + 1;
    LoadLeagueDbsMem((char *)str_extDB);
    EditRosters(unk_D0450, 6, 0x40, 0x41, 0x42);
    FreeLeagueDbsMem();
    FreeMemBlock(editptrspr);
    if (musicon && songdata)
        sub_8FCDF(musichandle, 3, 0x32);
    sub_8FFB0(0, 0x100, pal);
    FadePalStep(1, pal, 0x10);
    FreeMemBlock((int)pal);
    sub_8EA00(s.state);
    FreeMemBlock(fullscrbmp);
    boxfillcolor = s.fill;
    boxlitecolor = s.lite;
    boxshadecolor = s.shade;
    dlgtextfg = s.fg;
    dlgtextbg = s.bg;
    if (musicon && songdata) {
        while (!sub_8FC8A(*(int *)((char *)&musicslot - 3) >> 24, 3));
        sub_8D2F0(songdata);
        songdata = 0;
    }
}

typedef struct MenuItem20 { int f[5]; int handler; int g[2]; } MenuItem20;  /* 20h-byte menu item, handler at +14h */
typedef struct DivMenu { MenuItem20 *items; int pad[7]; } DivMenu;  /* roster1divmenus / roster2divmenus: 20h per division */

/* ShowFreeAgents (6D6DB, PC only; segment 052, the same source file: it ends in RunEditRosters_ret5): the
   "Free Agents" item of a roster menu. The side whose menu it is (side 0 when *which is 1) gives its current team
   menu item back (handler SelectRosterTeam in both division menus; division teamdivflags >> 1, 4 counts as 3),
   disables both "Free Agents" items and its "Edit Team Lines" item, shows the free agents (rosterteam FFh,
   rosterisfa 1), reloads the list (LoadRosterList) and redraws (DrawEditRosters). which: the menu callback
   argument. No result. */
void ShowFreeAgents(int *which)
{
    int side;
    int team;
    int d;
    int i;

    side = *which != 1;
    team = rosterteam[side];
    d = teamdivflags[team] >> 1;
    if (d == 4) d--;
    i = 0;
    do {
        if (((unsigned char (*)[7])divisionteams)[d][i] == team) break;
    } while (++i < 7);
    ((DivMenu *)roster1divmenus)[d].items[i].handler = (int)SelectRosterTeam;
    ((DivMenu *)roster2divmenus)[d].items[i].handler = (int)SelectRosterTeam;
    menu_r2_freeagents = menu_r1_freeagents = 0;
    if (!side) menu_r1_editlines = 0;
    else menu_r2_editlines = 0;
    rosterteam[side] = 0xFF;
    rosterisfa[side] = 1;
    LoadRosterList(side);
    DrawEditRosters();
}
