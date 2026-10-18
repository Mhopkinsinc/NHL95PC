/* Team select: the game line editor's line menu actions and the default-lines save that ends them. */
#define MenuUseTheseLines MenuUseTheseLines_decl
#define MenuUseOriginalLines MenuUseOriginalLines_decl
#define MenuUseDefaultLines MenuUseDefaultLines_decl
#define MenuSaveDefaultLines MenuSaveDefaultLines_decl
#define MenuSaveTheseLines MenuSaveTheseLines_decl
#define CheckGameRosterComplete CheckGameRosterComplete_decl
#include "nhl95.h"
#undef MenuUseTheseLines
#undef MenuUseOriginalLines
#undef MenuUseDefaultLines
#undef MenuSaveDefaultLines
#undef MenuSaveTheseLines
#undef CheckGameRosterComplete

int CheckGameRosterComplete(unsigned char side, int x, int y);
int open(const char *path, int access, ...);  /* Watcom CRT open_ */
int read(int fh, void *buf, unsigned len);  /* Watcom CRT read_ */
int write(int fh, const void *buf, unsigned len);  /* Watcom CRT write_ */
void perror(const char *s);  /* Watcom CRT perror_ */
void exit(int status);  /* Watcom CRT exit_ */
int j___close(int fh);  /* CRT close thunk */

typedef struct Lines48 {
    int w[12];
} Lines48;
int MenuSaveTheseLines(unsigned char *nums, unsigned char *dst, unsigned char side, int *sel, int art, int b, int c,
    int d, int mx, int my);
int MenuSaveDefaultLines(unsigned char *nums, unsigned char *dst, unsigned char side, int *sel, int art, int b, int c,
    int d, int mx, int my);

/* MenuUseTheseLines (79188, PC only; ends in MenuUseOriginalLines_ret): game line editor menu "use these lines".
   When side's game roster is complete (CheckGameRosterComplete, which complains at mx / my otherwise), turn the
   40 line slots' jersey numbers (nums) into the roster indexes of the dressed players wearing them (dst), and
   before the game (curperiod < 0) rebuild the 8 scratch slots at dst + 28h (64h for none) from the players whose
   roster status is set but not 3. Result: 1 (close the editor), or 0 with the roster incomplete. */
int MenuUseTheseLines(unsigned char *nums, unsigned char *dst, unsigned char side, int *sel, int art, int b, int c,
    int d, int mx, int my)
{
    int i;
    int j;

    if (CheckGameRosterComplete(side, mx, my)) return 0;
    for (i = 0; i < 0x28; i++) {
        for (j = 0; j < 0x1C; j++) {
            if (hmroster[side * 0x444 + j * 39] && nums[i] == hmrosterjersey[side * 0x444 + j * 39]) {
                dst[i] = j;
                break;
            }
        }
    }
    if (curperiod < 0) {
        for (i = 0; i < 8; i++) dst[i + 0x28] = 0x64;
        i = 0;
        for (j = 0; i < 0x1C; i++) {
            if (hmroster[side * 0x444 + i * 39] && hmroster[side * 0x444 + i * 39] != 3) {
                dst[j + 0x28] = i;
                j++;
            }
        }
    }
    return 1;
}

/* MenuUseOriginalLines (7929C, PC only; ends in MenuSaveDefaultLines_pop): game line editor menu "use original
   lines". Fit side's original lines (team record +ECh) to the roster (PruneGameLines), turn on the save / use
   line menu items and show the lines' jersey numbers in nums (an empty slot, 64h, turns those items off again);
   redraw the line jerseys with art, clear the selection (*sel = -1) and the scratch / dress actions. Result: 0. */
int MenuUseOriginalLines(unsigned char *nums, int a, unsigned char side, int *sel, int art, int b, int c, int d,
    int mx, int my)
{
    unsigned char *src;
    int i;

    src = hmteamrec + side * 0x2E8 + 0xEC;
    PruneGameLines(side, src);
    menuact_savelines = (int)MenuSaveTheseLines;
    menuact_uselines2 = menuact_uselines = (int)MenuUseTheseLines;
    menuact_savedeflines2 = menuact_savedeflines = (int)MenuSaveDefaultLines;
    for (i = 0; i < 0x28; i++) {
        if (src[i] != 0x64) {
            nums[i] = hmrosterjersey[src[i] * 39 + side * 0x444];
        } else {
            nums[i] = src[i];
            menuact_savedeflines2 = menuact_savedeflines = menuact_uselines2 = menuact_uselines = menuact_savelines = 0;
        }
    }
    DrawGameLineJerseys(nums, art, side);
    *sel = -1;
    menuact_gamedress = menuact_gamescratch = 0;
    return 0;
}

/* MenuUseDefaultLines (793A4, PC only; ends in MenuUseOriginalLines' redraw): game line editor menu "use default
   lines": the same from the default lines (team record +BCh). Result: 0. */
int MenuUseDefaultLines(unsigned char *nums, int a, unsigned char side, int *sel, int art, int b, int c, int d,
    int mx, int my)
{
    unsigned char *src;
    int i;

    src = hmteamrec + side * 0x2E8 + 0xBC;
    PruneGameLines(side, src);
    menuact_savelines = (int)MenuSaveTheseLines;
    menuact_uselines2 = menuact_uselines = (int)MenuUseTheseLines;
    menuact_savedeflines2 = menuact_savedeflines = (int)MenuSaveDefaultLines;
    for (i = 0; i < 0x28; i++) {
        if (src[i] != 0x64) {
            nums[i] = hmrosterjersey[src[i] * 39 + side * 0x444];
        } else {
            nums[i] = src[i];
            menuact_savedeflines2 = menuact_savedeflines = menuact_uselines2 = menuact_uselines = menuact_savelines = 0;
        }
    }
    DrawGameLineJerseys(nums, art, side);
    *sel = -1;
    menuact_gamedress = menuact_gamescratch = 0;
    return 0;
}

/* DRAFT: first pass, kept for its pop / ret tail (MenuSaveDefaultLines_pop, which MenuUseOriginalLines uses; the
   EXE starts it with a `db` push, so it is not in the C queue). */
/* MenuSaveDefaultLines (7947F, PC only; owner of MenuSaveDefaultLines_ret / _pop): game line editor menu "save
   default lines". When side's game roster is complete, read the team's record from its league file (leaguedbfmt,
   leaguedbfmt2 for side 1; any error is fatal, a missing file ends the program), store the lines as roster
   indexes at +BCh (and before the game the scratches at +E4h, as MenuUseTheseLines), write it back and copy the
   default and current lines into the team record in memory. Result: 0. */
int MenuSaveDefaultLines(unsigned char *nums, unsigned char *dst, unsigned char side, int *sel, int art, int b, int c,
    int d, int mx, int my)
{
    unsigned char rec[0x2E8];
    char path[0x20];
    int i;
    int j;
    int fh;
    int r;

    if (CheckGameRosterComplete(side, mx, my)) return 0;
    if (side) sprintf(path, (char *)&leaguedbfmt2, str_Teams2);
    else sprintf(path, (char *)&leaguedbfmt, str_Teams2);
    fh = open(path, 0x202);
    if (fh < 0) {
        sub_B4B58();
        perror((char *)str_ErrB2Perror);
        WaitKey();
        exit(1);
    }
    if (lseek(fh, 0, 0)) FatalError((char *)str_ErrB3);
    if (lseek(fh, (side ? VisTeam : HomeTeam) * 0x2E8, 0) < 0) FatalError((char *)str_ErrB4);
    r = read(fh, rec, 0x2E8);
    if (r < 0 || r != 0x2E8) FatalError((char *)str_ErrB5);
    r = lseek(fh, -0x2E8, 1);
    if (r < 0 && (r != HomeTeam * 0x2E8 && !side || r != VisTeam * 0x2E8 && side)) FatalError((char *)str_ErrB6);
    for (i = 0; i < 0x28; i++) {
        for (j = 0; j < 0x1C; j++) {
            if (hmroster[side * 0x444 + j * 39] && nums[i] == hmrosterjersey[side * 0x444 + j * 39]) {
                rec[0xBC + i] = j;
                break;
            }
        }
    }
    if (curperiod < 0) {
        for (i = 0; i < 8; i++) rec[0xE4 + i] = 0x64;
        i = 0;
        for (j = 0; i < 0x1C; i++) {
            if (hmroster[side * 0x444 + i * 39] && hmroster[side * 0x444 + i * 39] != 3) {
                rec[0xE4 + j] = i;
                j++;
            }
        }
    }
    r = write(fh, rec, 0x2E8);
    if (r < 0 || r != 0x2E8) FatalError((char *)str_ErrB7);
    *(Lines48 *)(hmteamrec + side * 0x2E8 + 0xBC) = *(Lines48 *)(rec + 0xBC);
    *(Lines48 *)(unk_DBD1C + side * 0x2E8) = *(Lines48 *)(rec + 0xEC);
    j___close(fh);
    return 0;
}

/* DRAFT: first pass; with it after the owner Watcom keeps the shared pops in MenuSaveDefaultLines as in the EXE (the
   EXE starts it with `db` bytes, so it is not in the C queue). */
/* MenuSaveTheseLines (797B4, PC only; ends in MenuSaveDefaultLines_ret): game line editor menu "save these lines":
   the same as MenuSaveDefaultLines, with the scratches always rebuilt and a missing file only fatal. Result: 1. */
int MenuSaveTheseLines(unsigned char *nums, unsigned char *dst, unsigned char side, int *sel, int art, int b, int c,
    int d, int mx, int my)
{
    unsigned char rec[0x2E8];
    char path[0x20];
    int i;
    int j;
    int fh;
    int r;

    if (CheckGameRosterComplete(side, mx, my)) return 0;
    if (side) sprintf(path, (char *)&leaguedbfmt2, str_Teams2);
    else sprintf(path, (char *)&leaguedbfmt, str_Teams2);
    fh = open(path, 0x202);
    if (fh < 0) FatalError((char *)str_ErrB2);
    if (lseek(fh, 0, 0)) FatalError((char *)str_ErrB3);
    if (lseek(fh, (side ? VisTeam : HomeTeam) * 0x2E8, 0) < 0) FatalError((char *)str_ErrB4);
    r = read(fh, rec, 0x2E8);
    if (r < 0 || r != 0x2E8) FatalError((char *)str_ErrB5);
    r = lseek(fh, -0x2E8, 1);
    if (r < 0 && (r != HomeTeam * 0x2E8 && !side || r != VisTeam * 0x2E8 && side)) FatalError((char *)str_ErrB6);
    for (i = 0; i < 0x28; i++) {
        for (j = 0; j < 0x1C; j++) {
            if (hmroster[side * 0x444 + j * 39] && nums[i] == hmrosterjersey[side * 0x444 + j * 39]) {
                rec[0xBC + i] = j;
                break;
            }
        }
    }
    for (i = 0; i < 8; i++) rec[0xE4 + i] = 0x64;
    i = 0;
    for (j = 0; i < 0x1C; i++) {
        if (hmroster[side * 0x444 + i * 39] && hmroster[side * 0x444 + i * 39] != 3) {
            rec[0xE4 + j] = i;
            j++;
        }
    }
    r = write(fh, rec, 0x2E8);
    if (r < 0 || r != 0x2E8) FatalError((char *)str_ErrB7);
    *(Lines48 *)(hmteamrec + side * 0x2E8 + 0xBC) = *(Lines48 *)(rec + 0xBC);
    *(Lines48 *)(unk_DBD1C + side * 0x2E8) = *(Lines48 *)(rec + 0xEC);
    j___close(fh);
    return 1;
}
