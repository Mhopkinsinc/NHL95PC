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
#define sub_8F98F sub_8F98F_decl
#define sub_8F13B sub_8F13B_decl
#include "nhl95.h"
#undef RunDemoGame
#undef LoadRockMusic
#undef ReloadGameGfx
#undef GameLoop
#undef sub_8F98F
#undef sub_8F13B

int RunDemoGame(void);
void LoadRockMusic(void);
void ReloadGameGfx(void);
void GameLoop(void);
int sub_8F98F(char *path);  /* sound library: load a song */
int sub_8F13B(char *path);  /* sound library: load a digitized sample */

typedef struct ExhRec { unsigned char b[0x75]; } ExhRec;

/* RunDemoGame: save the 75h-byte exhibition record, pick a demo matchup from demoteammode
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
    which = (demoteammode + 1) % 3;
    demoteammode = which;
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

typedef struct { char c[17]; } Hdr17;

#pragma pack(1)
typedef struct CreditPage { signed char n; int time; char *bank; char *lines[1]; } CreditPage;
#pragma pack()

/* DRAFT (ShowCredits): 1320 vs 1320. The first rand() % 8 loads the 8 into ecx before the sar (EXE after), the
   line count is loaded before byte_D42C3 for the y multiply (EXE after), and the exit keeps its own pop ebp because
   RunDemoGame does not push ebp yet (EXE jumps to RunDemoGame_x, which pops it). See docs/UNSURE.md. */
/* ShowCredits (16F9A, PC only; ends in RunDemoGame_x): the credits. Keep the text state, load the BACKWIN block
   and the CREDITS bank (from the CD when listed there): its palette (kept for the fade and copied to unk_C4E30)
   and BKGD art, drawn after the palette cycle fades out. Start the credits song when there is music, else the
   ADTITLE and ROCKDITI samples (playing the first), fade the palette in and say goodnight. Then show the 29 pages
   of off_C6399 (from the eighth on in the Kaufmann font): after the first, wait 62h ticks for a click; a page
   other than unk_C638C restores the backdrop and draws its art (a bank named at +5, its SHP0 shape at a random
   spot) and centres its lines (count at +0, strings from +9) from DAh up by half their height, then waits its
   time (+1) for a click. A click ends the credits: restore the backdrop, fade the music and the palette out,
   free the song or the samples, the text state and the block, and clear the screen. No result. */
void ShowCredits(void)
{
    CreditPage *cur;
    int page;
    int bg;
    int stop;
    int wait;
    int sample1;
    int sample2;
    unsigned char pal[0x300];
    char st[0x40];
    char path[0x10];
    int bank;
    int art;
    unsigned char *p;
    int y;
    int i;
    char *s;

    TextGridFree();
    PaOpenBank();
    sub_8E9E8(st);
    sub_8EA18(mainfont);
    SetTextColors(0x7F, 0);
    bg = sub_8CCA8((char *)str_Backwin, 0x19337, 0);
    MakePath(path, fileoncd[0x21D] == 1 ? (char *)cddriveptr : 0, (char *)str_Credits, 0);
    bank = sub_8E83C(path, 0);
    p = (unsigned char *)sub_B30B4(bank, (char *)str_Pal4) + 0x10;
    memcpy(pal, p, 0x300);
    memcpy(unk_C4E30, p, 0x300);
    art = sub_B30B4(bank, (char *)str_Bkgd3);
    *(Hdr17 *)bg = *(Hdr17 *)pointerspr;
    ((short *)bg)[2] = 0x135;
    ((short *)bg)[3] = 0x14E;
    FadeOutPalCycle();
    sub_9061C(art);
    FreeMemBlock(bank);
    sub_91400(bg, 0x14A, 0x3F);
    if (musicon && !songdata) {
        MakePath(path, fileoncd[0x1E1] == 1 ? (char *)cddriveptr : 0, (char *)str_Credits, (char *)str_Iff2);
        wait = sub_8F98F(path);
        songdata = wait;
        if (wait) sub_8FB8E(wait, musichandle, 3, 0x4C);
    } else {
        if (sounddev == 8)
            MakePath(path, fileoncd[0x104] == 1 ? (char *)cddriveptr : 0, (char *)str_Adtitle, (char *)str_KMS);
        else
            MakePath(path, fileoncd[0x104] == 1 ? (char *)cddriveptr : 0, (char *)str_Adtitle, (char *)str_KMS);
        sample1 = sub_8F13B(path);
        MakePath(path, fileoncd[0x161] == 1 ? (char *)cddriveptr : 0, (char *)str_Rockditi, (char *)str_KMS);
        sample2 = sub_8F13B(path);
        PlayDigiSample((void *)sample1);
    }
    FadePalStep(0, pal, 0x10);
    PaGoodnight();
    stop = 0;
    for (page = 0; page < 0x1D; page++) {
        if (page == 7) sub_8EA18(kaufmfont);
        cur = (CreditPage *)off_C6399[page];
        ClearInputQueue();
        if (page != 0) stop = WaitClickTimeout(0x62);
        if (stop) break;
        if (cur == (CreditPage *)unk_C638C) continue;
        if (*cur->bank != 0) {
            MakePath(path, fileoncd[0x21E] == 1 ? (char *)cddriveptr : 0, cur->bank, 0);
            bank = sub_8E83C(path, 0);
            art = sub_B30B4(bank, (char *)str_Shp0);
            sub_903F0(bg, 0x14A, 0x3F);
            sub_91370(art, rand() % 8 + 0x85, rand() % 8 + 0x6A);
            FreeMemBlock(bank);
        } else {
            sub_903F0(bg, 0x14A, 0x3F);
        }
        wait = cur->time;
        y = 0xDA - (cur->n * byte_D42C3 >> 1);
        for (i = 0; i < cur->n; i++) {
            s = cur->lines[i];
            RenderTextLine(0x1EA - (fputchar(s) >> 1), y, s);
            y += byte_D42C3;
        }
        ClearInputQueue();
        if (WaitClickTimeout(wait)) break;
    }
    sub_903F0(bg, 0x14A, 0x3F);
    if (musicon && songdata) sub_8FCDF(musichandle, 3, 0x64);
    SpeechStopQueue();
    FadePalStep(1, pal, 0x10);
    if (musicon && songdata) {
        while (!sub_8FC8A(musicslot, 3));
        sub_8D2F0(songdata);
        songdata = 0;
    } else {
        StopDigiSample();
        sub_8F1FE(sample1);
        sub_8F1FE(sample2);
    }
    sub_8EA00(st);
    FreeMemBlock(bg);
    sub_B4BA8();
    sub_B392C(0);
}
