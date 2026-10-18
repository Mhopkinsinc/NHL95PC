/* Asset loading: the end-of-season awards ceremony. */
/* DRAFT: 1349 vs 1346 bytes before the exit, 336 of 362 instructions, first pass; see docs/UNSURE.md. Its match
   belongs with PickAwardWinners (untouched), whose PickAwardWinners_x tail it uses. */
#define sub_8F1FE sub_8F1FE_decl
#define sub_8F13B sub_8F13B_decl
#define ShowAwardScreens ShowAwardScreens_decl
#define PickAwardWinners PickAwardWinners_decl
#define sub_8F98F sub_8F98F_decl
#define sub_8FB8E sub_8FB8E_decl
#include "nhl95.h"
#undef sub_8F1FE
#undef sub_8F13B
#undef ShowAwardScreens
#undef PickAwardWinners
#undef sub_8F98F
#undef sub_8FB8E

void sub_8F1FE(void *smp);
void *sub_8F13B(char *path);
int ShowAwardScreens(void);
int PickAwardWinners(void);
int sub_8F98F(char *path);
void sub_8FB8E(int song, int handle, int a, int b);

/* AwardsCeremony (13320, PC only; ends in PickAwardWinners_x): the awards ceremony after the season. Fade out
   (the music too), save the text state, draw the AWARDSI title screen and play the AWARDS song (or the fanfare
   sample on a sound card) while it fades in, until a click or the song ends; then pick the award winners
   (PickAwardWinners). With winners, start the AWASONG music (or the awards sample), show the award screens
   (ShowAwardScreens) and, unless they were left, the awards summary (DrawAwardsSummary) for up to 9000 ticks.
   Then fade the music out (or stop the sample) and restore the text state. No arguments, no result. */
void AwardsCeremony(void)
{
    struct {
        char text[0x40];
        char path[0x10];
    } s;
    unsigned char *pal;
    unsigned char *pal2;
    int bank;
    int bank2;
    void *smp;
    int r;
    int slot;
    int done;

    ClearInputQueue();
    pal = (unsigned char *)sub_8CCA8((char *)str_Palmem, 0x300, 0x20);
    sub_8FFB0(0, 0x100, pal);
    if (musicon && songdata) sub_8FCDF(musichandle, 3, 0x32);
    FadePalStep(1, pal, 0x10);
    if (musicon && songdata) {
        do
            slot = musicslot;
        while (sub_8FC8A(slot, 3) == 0);
        sub_8D2F0(songdata);
        songdata = 0;
    }
    sub_8E9E8(s.text);
    sub_8EA18(kaufmfont);
    MakePath(s.path, fileoncd[0x1DE] == 1 ? (char *)cddriveptr : 0, (char *)str_Awardsi, 0);
    bank = sub_8E83C(s.path, 0);
    sub_9061C(sub_B30B4(bank, (char *)str_Scrn));
    sub_9061C(sub_B30B4(bank, (char *)str_Titl));
    memcpy(pal, (unsigned char *)sub_B30B4(bank, (char *)str_Pal3) + 0x10, 0x300);
    FreeMemBlock(bank);
    if (musicon && !songdata) {
        MakePath(s.path, fileoncd[0x220] == 1 ? (char *)cddriveptr : 0, (char *)str_Awards, (char *)str_Iff);
        songdata = sub_8F98F(s.path);
        if (songdata && gameopts.music) sub_8FB8E(songdata, musichandle, 3, 0x4C);
    } else if (sounddev != 1) {
        if (sounddev == 8)
            MakePath(s.path, fileoncd[0xFA] == 1 ? (char *)cddriveptr : 0, (char *)str_Mtafan, 0);
        else
            MakePath(s.path, fileoncd[0x18] == 1 ? (char *)cddriveptr : 0, (char *)str_Adafan, 0);
        smp = sub_8F13B(s.path);
        PlayDigiSample(smp);
    }
    FadePalStep(0, pal, 0x10);
    if (!musicon && sounddev != 1) {
        WaitDigiSample();
        FadePalStep(1, pal, 0x10);
        StopDigiSample();
        sub_8F1FE(smp);
    } else if (musicon) {
        done = 0;
        while (!done && sub_8FC8A(musicslot, 3) == 0) done = WaitClickTimeout(0xA);
        sub_8FCDF(musichandle, 3, 0x32);
        FadePalStep(1, pal, 0x10);
        do
            slot = musicslot;
        while (sub_8FC8A(slot, 3) == 0);
        if (songdata) sub_8D2F0(songdata);
        songdata = 0;
    } else {
        WaitClickTimeout(0x140);
        FadePalStep(1, pal, 0x10);
    }
    FreeMemBlock((int)pal);
    r = PickAwardWinners();
    if (musicon && !songdata) {
        MakePath(s.path, fileoncd[0x224] == 1 ? (char *)cddriveptr : 0, (char *)str_Awasong, (char *)str_Iff);
        songdata = sub_8F98F(s.path);
        if (songdata && gameopts.music) sub_8FB8E(songdata, musichandle, 3, 0x7F);
    } else {
        if (sounddev == 8)
            MakePath(s.path, fileoncd[0xFC] == 1 ? (char *)cddriveptr : 0, (char *)str_Mtawards, 0);
        else
            MakePath(s.path, fileoncd[0x1A] == 1 ? (char *)cddriveptr : 0, (char *)str_Adawards, 0);
        smp = sub_8F13B(s.path);
        PlayDigiSample(smp);
    }
    if (!r) {
        if (ShowAwardScreens() < 3) {
            MakePath(s.path, fileoncd[0x1DE] == 1 ? (char *)cddriveptr : 0, (char *)str_Awardsi, 0);
            bank2 = sub_8E83C(s.path, 0);
            ClearInputQueue();
            sub_B4BA8();
            SetTextColors(1, 0);
            sub_9061C(sub_B30B4(bank2, (char *)str_Scrn));
            sub_9061C(sub_B30B4(bank2, (char *)str_Summ));
            DrawAwardsSummary();
            sub_B4BA8();
            pal2 = (unsigned char *)sub_B30B4(bank2, (char *)str_Pal3) + 0x10;
            FadePalStep(0, pal2, 0x10);
            WaitClickTimeout(0x2328);
            FadePalStep(1, pal2, 0x10);
            FreeMemBlock(bank2);
        }
        sub_B4BA8();
        sub_B392C(0);
    }
    if (musicon) {
        sub_8FCDF(musichandle, 3, 0x78);
        do
            slot = musicslot;
        while (sub_8FC8A(slot, 3) == 0);
        if (songdata) sub_8D2F0(songdata);
        songdata = 0;
    } else {
        StopDigiSample();
        sub_8F1FE(smp);
    }
    sub_8EA00(s.text);
}
