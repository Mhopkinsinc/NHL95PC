/* Season playoffs: creating the playoffs, and seeding the second round from the first-round winners. */
#define POSeedRound2 POSeedRound2_decl
#define SeriesWinner SeriesWinner_decl
#define POSetSeriesTeams POSetSeriesTeams_decl
#define CreatePlayoffs CreatePlayoffs_decl
#define POSeedTeams POSeedTeams_decl
#define LockerRoomScreen LockerRoomScreen_decl
#define savenamedefault savenamedefault_decl
#include "nhl95.h"
#undef POSeedRound2
#undef SeriesWinner
#undef POSetSeriesTeams
#undef CreatePlayoffs
#undef POSeedTeams
#undef LockerRoomScreen
#undef savenamedefault

typedef struct { char c[13]; } PlayoffName;

int CreatePlayoffs(void);
void POSeedTeams(int *seeds);
int LockerRoomScreen(void);
int mkdir(const char *path);  /* Watcom CRT mkdir_ */
extern PlayoffName savenamedefault;  /* 86616: "." and zeros, the default name */

/* DRAFT: 2,037 bytes as in the EXE, 34 differing lines (register picks, the bank store, the epilogue);
   see docs/UNSURE.md. */
/* CreatePlayoffs (86696, PC only; ends in POSeedRound2_x): start a new playoff. Keep and set the dialog colours,
   keep the exhibition mode state, fade the palette out and draw the EMBSCUP background (dword_C65AC when loaded),
   fade the EMBPAL palette in and ask for the playoff name (up to 8 characters, colour 3 white meanwhile). With a
   name: set the playoff mode state (postate: 12 and 21 for two counts, the option word, the first controller in
   ctlavailmask order 2, 4, 8, 1 else 10h), add .PO, offer to replace an existing playoff of that name (no: give
   up), make it the current league (curleague, the database paths) and its directory, ask which database, check
   GAMESET and the disk space, then show the locker room (3 gives up and removes the directory). Then redraw the
   background, seed the teams (POSeedTeams), copy the seven databases, make the schedule (POCreateSchedule), write
   the mode state, rebuild the saved game labels and stats menu and point the menu actions at the playoff screens.
   On an error say so and remove the directory. At the end fade the TEMP2 palette in, restore the exhibition mode
   state and the dialog colours. No arguments; result 2. */
int CreatePlayoffs(void)
{
    struct {
        int seeds[16];
        char ft[0x2C];
        char path[0x20];
        PlayoffName name;
        char pad[3];
        int btn;
        int y;
        int x;
        int bank2;
        int fill;
        int lite;
        int bg;
        int shade;
        int fg;
        unsigned char rgb[4];
        unsigned char orgb[4];
    } s;
    int p;
    int pal;
    int bank;
    int r;
    int i;

    s.name = savenamedefault;
    s.fill = boxfillcolor;
    s.lite = boxlitecolor;
    s.shade = boxshadecolor;
    s.fg = dlgtextfg;
    s.bg = dlgtextbg;
    boxfillcolor = 0x41;
    boxlitecolor = 0x40;
    boxshadecolor = 0x42;
    dlgtextfg = 0x40;
    dlgtextbg = 0x43;
    SaveModeState(exhstate);
    pal = p = sub_8CCA8((char *)str_Palette, 0x300, 0x20);
    sub_8FFB0(0, 0x100, (unsigned char *)p);
    FadePalStep(1, (unsigned char *)p, 0x10);
    sub_B4BA8();
    if (!dword_C65AC) {
        MakePath(s.path, fileoncd[0x8D] == 1 ? (char *)cddriveptr : 0, (char *)str_Embscup, 0);
        p = sub_8E83C(s.path, 0);
        sub_B4BC4(0, 0x280, 0, 0x1E0);
        sub_913B4(sub_B30B4(p, (char *)str_Bkgd));
        FreeMemBlock(p);
    } else {
        sub_9121C(sub_B30B4(dword_C65AC, (char *)str_Bkgd));
    }
    MakePath(s.path, fileoncd[0x8E] == 1 ? (char *)cddriveptr : 0, (char *)str_Embpal, 0);
    bank = sub_8E83C(s.path, 0);
    FadePalStep(0, (unsigned char *)sub_B30B4(bank, (char *)str_BangPal) + 0x10, 0x10);
    sub_8E9C0(0x40, 0x41);
    SetDialogColors(0x41, 0x40, 0x42, 0x40, 0x43);
    s.rgb[0] = s.rgb[1] = s.rgb[2] = 0x3F;
    sub_8FFB0(3, 1, s.orgb);
    sub_B4B88(3, 1, s.rgb);
    r = TextInputDialog((char *)str_PleaseEnterNewPlay, s.name.c, 8, 0x36, 0, 0, 0, 0, 5);
    sub_B4B88(3, 1, s.orgb);
    sub_B2DCA(&s.btn, &s.x, &s.y);
    if (r == 0x1B || !s.name.c[0]) goto cancel;
    postate[0] = 1;
    *(int *)((char *)postate + 0x51) = 0xC;
    *(int *)((char *)postate + 0x55) = 0x15;
    *(unsigned short *)((char *)postate + 0x59) &= 0x8000;
    *(unsigned short *)((char *)postate + 0x59) |= 0x79FF;
    if (ctlavailmask & 2) *(int *)((char *)postate + 0x65) = 2;
    else if (ctlavailmask & 4) *(int *)((char *)postate + 0x65) = 4;
    else if (ctlavailmask & 8) *(int *)((char *)postate + 0x65) = 8;
    else if (ctlavailmask & 1) *(int *)((char *)postate + 0x65) = 1;
    else *(int *)((char *)postate + 0x65) = 0x10;
    if (*(int *)((char *)postate + 0x65) != 0x10) *(int *)((char *)postate + 0x5D) = 0xC;
    else *(int *)((char *)postate + 0x5D) = -1;
    *(int *)((char *)postate + 0x69) = 0x10;
    *(int *)((char *)postate + 0x61) = -2;
    *(int *)((char *)postate + 0x6D) = r = 0;
    *(int *)((char *)postate + 0x71) = 1;
    LoadModeState(postate);
    strcat(s.name.c, (char *)str_PO);
    if (!unknown_libname_1(s.name.c, 0x10, s.ft)) {
        if ((r = !MessageBox(-1, -1, (char *)msg_POExists, 2, (int)btn_LeagueExists, 2, (int)&s.x, (int)&s.y, -1)) != 0)
            goto cancel;
        DeleteDir(s.name.c);
    }
    goto named;
cancel:
    FreeMemBlock(bank);
    FreeMemBlock(pal);
    r = 1;
named:
    if (!r) {
        strcpy((char *)&curleague, s.name.c);
        strcpy((char *)&leaguedbfmt, s.name.c);
        strcat((char *)&leaguedbfmt, (char *)str_fmtSlashDb);
        strcpy((char *)&leaguedbfmt2, (char *)&leaguedbfmt);
        if (mkdir(s.name.c)) {
            MessageBox(-1, -1, (char *)msg_POSetupError, 4, 0, 0, (int)&s.x, (int)&s.y, 0x190);
            r = 1;
        }
        if (!r) {
            if ((r = AskDatabaseChoice()) >= 0) {
                GetLeagueDBSizes(0, (int *)unk_D2864, (char *)dbextension);
                MakePath(s.path, (char *)&curleague, (char *)str_GameSet, 0);
                if (FileExists(s.path)) dword_D2884 = 0;
                if ((r = DiskSpaceShort(0, (int *)unk_D2864)) != 0) {
                    sprintf((char *)msg_PODiskSpace_arg, (char *)str_4dKbytesOfFree, r);
                    MessageBox(-1, -1, (char *)msg_PODiskSpace, 3, 0, 0, (int)&s.x, (int)&s.y, 0x258);
                    r = 1;
                }
            }
            if (!r && LockerRoomScreen() != 3) {
                sub_8FFB0(0, 0x100, (unsigned char *)pal);
                FadePalStep(1, (unsigned char *)pal, 0x10);
                if (!dword_C65AC) {
                    MakePath(s.path, fileoncd[0x8D] == 1 ? (char *)cddriveptr : 0, (char *)str_Embscup, 0);
                    s.bank2 = sub_8E83C(s.path, 0);
                    sub_9121C(sub_B30B4(s.bank2, (char *)str_Bkgd));
                    FreeMemBlock(s.bank2);
                } else {
                    sub_9121C(sub_B30B4(dword_C65AC, (char *)str_Bkgd));
                }
                FadePalStep(0, (unsigned char *)sub_B30B4(bank, (char *)str_BangPal) + 0x10, 0x10);
                FreeMemBlock(bank);
                FreeMemBlock(pal);
                POSeedTeams(s.seeds);
                MessageBox(-1, -1, (char *)msg_POSettingUp, 2, 0, 0, (int)&s.x, (int)&s.y, 0);
                for (i = 0; i < 7; i++) {
                    if (CopyFile((char *)leaguedbnames[i], (char *)dbextension, (char *)str_extDB, (char *)&str_dot,
                            (char *)&curleague)) {
                        SetTickDeadline(0x190);
                        DeleteDir(s.name.c);
                        r = 1;
                        WaitTickDeadline();
                        break;
                    }
                }
                if (!r) {
                    if (POCreateSchedule(s.seeds, s.name.c)) {
                        SetTickDeadline(0x190);
                        MessageBox(-1, -1, (char *)msg_POSetupError, 4, 0, 0, (int)&s.x, (int)&s.y, 0);
                        DeleteDir(s.name.c);
                        WaitTickDeadline();
                        RestoreDialogBg();
                    }
                    WriteModeState(postate);
                    BuildSavedGameLabels();
                    SetupStatsSourceMenu(0);
                    menuact_nextpo = (int)PlayoffModeLoop;
                    menuact_posettings = (int)EditPlayoffSettings;
                    menuact_pohilights = (int)ViewPlayoffHilights;
                    seriesgameno = r = 0;
                }
                RestoreDialogBg();
            } else {
                rmdir(s.name.c);
                FreeMemBlock(bank);
                FreeMemBlock(pal);
            }
        }
    }
    pal = sub_8CCA8((char *)str_Temp2, 0x300, 0x20);
    sub_8FFB0(0, 0x100, (unsigned char *)pal);
    FadePalStep(1, (unsigned char *)pal, 0x10);
    FreeMemBlock(pal);
    LoadModeState(exhstate);
    boxfillcolor = s.fill;
    boxlitecolor = s.lite;
    boxshadecolor = s.shade;
    dlgtextfg = s.fg;
    dlgtextbg = s.bg;
    return 2;
}

int SeriesWinner(unsigned char *s, unsigned games);
#pragma aux SeriesWinner parm [eax] [edx] value [eax] modify [eax edx];
void POSetSeriesTeams(unsigned char *s, int a, int b);
#pragma aux POSetSeriesTeams parm [eax] [edx] [ebx] modify [eax edx ebx];

void __STOSD(void *p, int v, int n);
#pragma aux __STOSD "__STOSD" parm [eax] [edx] [ecx] modify [eax edx ecx];

/* DRAFT: 736 vs 720 (best of 5). The frame, the eight-score scan, both date fills and
 __STOSD match. Each SeriesWinner result is copied with mov edx, eax before the home byte
 is loaded, so the byte lands in eax and the compare is swapped (16 bytes, eight calls).
 Do not retry: named unsigned chars for the scan (al/ah/dh/dl instead of ah/dl/dh/al),
 one int for both the day remainder and the home byte (spills, sub esp, 34h), or
 assigning the home byte to k, the filter counter that lives in edx.
 */
/* POSeedRound2: walk back from the last first-round game while all eight series
 still have an empty score, then date the two second-round series (14 games each,
 every other day, rolling the month at 30). The four winners on each side fill
 the next series; a winner that is the home team is kept in the low slots and
 the other winner in the high slots, and only team numbers below 26 are passed
 on. POSeedRound2_x30 is the add esp. */
void POSeedRound2(unsigned char *p)
{
    struct {
        int slot[8];
        int out[4];
    } s;
    unsigned char *g;
    unsigned char *ser;
    int i;
    int k;
    int div;
    int rem;

    g = p + 0x19BC;
    i = 0;
    do {
        if (g[4] != 0xFF || g[0x2E] != 0xFF || g[0x58] != 0xFF || g[0x82] != 0xFF
            || g[0xAC] != 0xFF || g[0xD6] != 0xFF || g[0x100] != 0xFF || g[0x12A] != 0xFF)
            break;
        i++;
        g -= 6;
    } while (i < 6);
    s.out[0] = g[0];
    s.out[1] = g[1] + 3;
    ser = p + 0x1AE8;
    for (i = 0; i < 0xE;) {
        div = 7;
        ser[0] = s.out[0];
        rem = i % div;
        rem += rem;
        rem += s.out[1];
        ser[1] = rem;
        if (ser[1] > 0x1E) {
            ser[0]++;
            ser[1] -= 0x1E;
        }
        i++;
        ser += 6;
    }
    s.out[1]--;
    ser = p + 0x1B3C;
    i = 0;
    div = 7;
    do {
        ser[0] = s.out[0];
        rem = i % div;
        rem += rem;
        rem += s.out[1];
        ser[1] = rem;
        if (ser[1] > 0x1E) {
            ser[0]++;
            ser[1] -= 0x1E;
        }
        i++;
        ser += 6;
    } while (i < 0xE);
    __STOSD(s.slot, -1, 8);
    ser = p + 0x1998;
    s.out[0] = SeriesWinner(ser, *(int *)(p + 0x50));
    k = ser[2];
    if (s.out[0] == k) s.slot[0] = k;
    else s.slot[7] = s.out[0];
    ser += 0x2A;
    s.out[1] = SeriesWinner(ser, *(int *)(p + 0x54));
    k = ser[2];
    if (s.out[1] == k) s.slot[1] = k;
    else s.slot[6] = s.out[1];
    ser += 0x2A;
    s.out[2] = SeriesWinner(ser, *(int *)(p + 0x58));
    k = ser[2];
    if (s.out[2] == k) s.slot[2] = k;
    else s.slot[5] = s.out[2];
    ser += 0x2A;
    s.out[3] = SeriesWinner(ser, *(int *)(p + 0x5C));
    k = ser[2];
    if (s.out[3] == k) s.slot[3] = k;
    else s.slot[4] = s.out[3];
    i = 0;
    k = 0;
    for (; i < 8; i++) {
        if (s.slot[i] >= 0 && s.slot[i] < 0x1A)
            s.out[k++] = s.slot[i];
    }
    ser = p + 0x1AE8;
    POSetSeriesTeams(ser, s.out[0], s.out[3]);
    POSetSeriesTeams(ser + 0x2A, s.out[1], s.out[2]);
    __STOSD(s.slot, -1, 8);
    ser = p + 0x1A40;
    s.out[0] = SeriesWinner(ser, *(int *)(p + 0x60));
    k = ser[2];
    if (s.out[0] == k) s.slot[0] = k;
    else s.slot[7] = s.out[0];
    ser += 0x2A;
    s.out[1] = SeriesWinner(ser, *(int *)(p + 0x64));
    k = ser[2];
    if (s.out[1] == k) s.slot[1] = k;
    else s.slot[6] = s.out[1];
    ser += 0x2A;
    s.out[2] = SeriesWinner(ser, *(int *)(p + 0x68));
    k = ser[2];
    if (s.out[2] == k) s.slot[2] = k;
    else s.slot[5] = s.out[2];
    ser += 0x2A;
    s.out[3] = SeriesWinner(ser, *(int *)(p + 0x6C));
    k = ser[2];
    if (s.out[3] == k) s.slot[3] = k;
    else s.slot[4] = s.out[3];
    i = 0;
    k = 0;
    for (; i < 8; i++) {
        if (s.slot[i] >= 0 && s.slot[i] < 0x1A)
            s.out[k++] = s.slot[i];
    }
    ser = p + 0x1B3C;
    POSetSeriesTeams(ser, s.out[0], s.out[3]);
    POSetSeriesTeams(ser + 0x2A, s.out[1], s.out[2]);
}
