/* Scoreboard: play one demo game and restore the exhibition state. */
/* DRAFT: 673 vs 673. The setup, the matchup switch, the option bytes and the
 restore copy match. 0x10 and the later 1 stay in esi (original ebp, so the
 prologue does not push ebp) and the zeros stay in edi (original xor eax, eax,
 and demomode is xor ecx, ecx). The fixed pair index is cwde with eax*2
 (original movsx edx, ax and edx*2). Do not retry declaring that constant
 first, holding 0x10 across the copy, or using the team local as the pair index. */
#define RunDemoGame RunDemoGame_decl
#define LoadRockMusic LoadRockMusic_decl
#define ReloadGameGfx ReloadGameGfx_decl
#define GameLoop GameLoop_decl
#include "nhl95.h"
#undef RunDemoGame
#undef LoadRockMusic
#undef ReloadGameGfx
#undef GameLoop

int RunDemoGame(void);
void LoadRockMusic(void);
void ReloadGameGfx(void);
void GameLoop(void);

typedef struct ExhRec { unsigned char b[0x75]; } ExhRec;

/* RunDemoGame: save the 75h-byte exhibition record, pick a demo matchup from dword_C5886
 (all-stars, a fixed pair, or two random teams), run a one-minute period at 320x200, then
 put the record and the 5-minute period length back. Returns 1 when the game result is 2. */
int RunDemoGame(void)
{
    ExhRec saved;
    int one;
    int which;
    int team;
    int home;
    int vis;

    ShowLoadingScreen();
    InputRemove();
    PerTimeTab[0] = 0x3C;
    DemoSetupStub();
    demomode = 1;
    seed[0] = (rand() << 16) + rand();
    which = (dword_C5886 + 1) % 3;
    dword_C5886 = which;
    saved = *(ExhRec *)exhstate;
    if (which == 2) {
        *(int *)(exhstate + 0x51) = 0x1B;
        *(int *)(exhstate + 0x55) = 0x1A;
    } else if (which == 1) {
        team = randomd0(0x11);
        *(int *)(exhstate + 0x51) = *(int *)((char *)dword_C5861 + team * 2) >> 24;
        *(int *)(exhstate + 0x55) = *(int *)((char *)dword_C5862 + team * 2) >> 24;
    } else {
        team = rand() % 0x1A;
        *(int *)(exhstate + 0x51) = team;
        team = team + 1;
        *(int *)(exhstate + 0x55) = (team + randomd0(0x19)) % 0x1A;
    }
    one = 0x10;
    *(int *)(exhstate + 0x65) = one;
    gamemode = 0;
    *(int *)(exhstate + 0x69) = one;
    *(int *)(exhstate + 0x6D) = 0;
    *(int *)(exhstate + 0x71) = 1;
    *(int *)(exhstate + 0x5D) = *(int *)(exhstate + 0x51);
    *(int *)(exhstate + 0x5D) = -1;
    *(int *)(exhstate + 0x61) = -2;
    LoadModeState(exhstate);
    *(unsigned char *)&gameopts |= 0xFF;
    if (musicon) ((unsigned char *)&gameopts)[1] |= 1;
    else ((unsigned char *)&gameopts)[1] &= 0xFE;
    ((unsigned char *)&gameopts)[1] |= 2;
    ((unsigned char *)&gameopts)[1] &= 3;
    vis = *(int *)((char *)&VisTeam - 2) >> 16;
    vis = teamabbrevs[vis];
    home = *(int *)((char *)&HomeTeam - 2) >> 16;
    home = teamabbrevs[home];
    PaPreloadClips(home, vis);
    SetupControllers();
    InitGameSummary(0, 0);
    if (HomeTeam >= 0x1A) LoadRink(0x0C);
    else LoadRink(*(int *)((char *)&HomeTeam - 2) >> 16);
    LoadRockMusic();
    ReloadGameGfx();
    FadeOutPalCycle();
    one = 1;
    dword_CC0EC = one;
    SetupDemoGame();
    dword_CC0EC = 0;
    introskipped = 0;
    gameresult = 0;
    SetScreenSize(0x140, 0xC8);
    vis = *(int *)((char *)&VisTeam - 2) >> 16;
    home = *(int *)((char *)&HomeTeam - 2) >> 16;
    LoadTeamPalette(home, vis, gamepal);
    vis = *(int *)((char *)&VisTeam - 2) >> 16;
    home = *(int *)((char *)&HomeTeam - 2) >> 16;
    DrawHudPanel(home, vis, one, one);
    joyq_flush();
    GameLoop();
    SetScreenSize(0x280, 0x1E0);
    PerTimeTab[0] = 0x12C;
    demomode = 0;
    *(ExhRec *)exhstate = saved;
    LoadModeState(exhstate);
    InputInstall();
    return gameresult == 2;
}
