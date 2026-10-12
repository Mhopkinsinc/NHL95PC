/* EASN stats: team stats screen from the calendar. */
#define CalTeamStatsScreen CalTeamStatsScreenDecl
#define RunMenu RunMenuDecl
#include "nhl95.h"
#undef CalTeamStatsScreen
#undef RunMenu

void RunMenu(void *bar, int n, int col1, int col2, int col3);

/* CalTeamStatsScreen: the calendar's team-stats page. Fade out, free the calendar shapes if memory is
 low, draw TeamStatsScreen, and load embpal3 (from the CD when fileoncd 8Eh says so). Pal8 is faded in
 from the shape. After the menu (CalTeamStatsScreen_common) the calendar shapes are loaded again.
 Returns 0. */
int CalTeamStatsScreen(int cat)
{
    unsigned char buf[0x310];
    char *dir;
    char *name;
    char *ext;
    int z;

    sub_8FFB0(0, 0x100, buf);
    dword_DC738 = 1;
    statscategory = cat;
    statsredrawcb = (int)TeamStatsScreen;
    sub_B4BA8();
    InitMenuRemap(1);
    FadePalStep(1, buf, 0x10);
    FreeCalendarIfLowMem();
    TeamStatsScreen(cat);
    DrawMenuBar(unk_CF78F, 4, 0x40, 0x41, 0x42);
    ext = 0;
    name = (char *)str_embpal3;
    if (fileoncd[0x8E] == 1) dir = (char *)cddriveptr;
    else dir = 0;
    MakePath((char *)buf + 0x300, dir, name, ext);
    statsbgshapes = sub_8E83C((char *)buf + 0x300, 0);
    statspalshape = sub_B30B4(statsbgshapes, (char *)str_Pal8);
    FadePalStep(0, (unsigned char *)statspalshape + 0x10, 0x10);
    jctime(statsbgshapes);
    RunMenu(unk_CF78F, 4, 0x40, 0x41, 0x42);
    sub_8FFB0(0, 0x100, buf);
    FadePalStep(1, buf, 0x10);
    dword_DDD2C = -1;
    LoadCalendarShapes();
    z = 0;
    dword_DC738 = z;
    statsredrawcb = z;
    return 0;
}
