/* EASN stats: skater stats screen. */
#define GameSkaterStatsScreen GameSkaterStatsScreenDecl
#define RunMenu RunMenuDecl
#include "nhl95.h"
#undef GameSkaterStatsScreen
#undef RunMenu

void RunMenu(void *bar, int n, int col1, int col2, int col3);
int LeadersScreen(int cat);

/* GameSkaterStatsScreen: the skater-stats page. Same fade and embpal load as the team page, with
 LeadersScreen, menu bar unk_CF80F, and the palette buffer recorded in statspal while it is current.
 The zero that clears statspal is reused for the fade-back and the redraw hook. Returns 2. */
int GameSkaterStatsScreen(int cat)
{
    unsigned char buf[0x310];
    int one;
    char *dir;
    char *name;
    char *ext;
    int z;

    one = 1;
    dword_DC738 = one;
    statscategory = cat;
    statsredrawcb = (int)LeadersScreen;
    statspalvalid = one;
    statspal = (int)buf;
    sub_8FFB0(0, 0x100, buf);
    FadePalStep(1, buf, 0x10);
    sub_B4BA8();
    InitMenuRemap(1);
    LeadersScreen(cat);
    DrawMenuBar(unk_CF80F, 4, 0x40, 0x41, 0x42);
    ext = 0;
    name = (char *)str_embpal;
    if (fileoncd[0x8E] == 1) dir = (char *)cddriveptr;
    else dir = 0;
    MakePath((char *)buf + 0x300, dir, name, ext);
    statsbgshapes = sub_8E83C((char *)buf + 0x300, 0);
    statspalshape = sub_B30B4(statsbgshapes, (char *)str_Pal6);
    memcpy(buf, (unsigned char *)statspalshape + 0x10, 0x300);
    FreeMemBlock(statsbgshapes);
    FadePalStep(0, buf, 0x10);
    z = 0;
    statspalvalid = z;
    statspal = z;
    RunMenu(unk_CF80F, 4, 0x40, 0x41, 0x42);
    sub_8FFB0(z, 0x100, buf);
    FadePalStep(1, buf, 0x10);
    dword_DC738 = z;
    statsredrawcb = z;
    return 2;
}
