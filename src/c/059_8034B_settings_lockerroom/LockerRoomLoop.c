/* Settings: the locker room loop. */
/* DRAFT: same size; the team steps of boxes 2 and 4 are hoisted above the switch; see docs/UNSURE.md. */
#define LockerRoomLoop LockerRoomLoop_decl
#include "nhl95.h"
#undef LockerRoomLoop

typedef struct { char c[17]; } Hdr17;

/* LockerRoomLoop (81C4E, PC only; ends in LockerStub): the locker room. Start from the home and visiting teams'
   locker indexes (dword_D20E0), draw the mouse pointer (sub_91FE0) over a saved POINTER block and follow it.
   A click (event bit 2) on a box (LockerHitTest): 0 / 3 and 2 step the home team forward and back, 1 / 5 and 4
   the visitors (28 teams in exhibition mode, where the two extra teams may not meet themselves, else 26 and the
   teams must differ); each step redraws that side's jersey (DrawLockerJersey with both teams' division flags)
   and puts the mouse back. 6 (OK) sets HomeTeam / VisTeam (dword_D2150), in exhibition mode writes both
   abbreviations into the matchup string, points the controllers at their sides' teams and writes the mode state;
   7 (Cancel) and event bit 4 leave without that. Leaving fades the music and the palette, frees the block and
   waits for the song to stop. Result: 0 after OK, else 3. */
int LockerRoomLoop(void)
{
    int vis;
    int bm;
    int ox;
    int oy;
    int x;
    int y;
    int btn;
    int hit;
    unsigned char buf[0x300];
    int hm;
    int n;
    char *p;
    unsigned char *ev;

    hm = dword_D20E0[HomeTeam];
    vis = dword_D20E0[VisTeam];
    sub_B2DCA(&btn, &ox, &oy);
    x = ox;
    y = oy;
    sub_B4BA8();
    n = (((short *)pointerspr)[2] + 1) * (((short *)pointerspr)[3] + 1) + 0x11;
    bm = sub_8CCA8((char *)str_Pointer20, n, 0x20);
    *(Hdr17 *)bm = *(Hdr17 *)pointerspr;
    ((short *)bm)[2] = ((short *)pointerspr)[2] + 1;
    ((short *)bm)[3] = ((short *)pointerspr)[3] + 1;
    sub_91400(bm, x - 4, y);
    sub_91FE0(pointerspr, x - 4, y);
    ClearInputQueue();
    for (;;) {
        btn = 0;
        for (;;) {
            ev = GetInputEvent();
            if (ev == 0) break;
            btn = ((int (*)(unsigned char *, int *, int *))ptrupdatefn)(ev, &x, &y);
            if (btn & 6) break;
        }
        if (!(btn & 2) && !(btn & 4)) {
            if (x != ox || y != oy) {
                sub_903F0(bm, ox - 4, oy);
                sub_91400(bm, x - 4, y);
                sub_91FE0(pointerspr, x - 4, y);
                ox = x;
                oy = y;
            }
        } else if (btn & 2) {
            sub_903F0(bm, ox - 4, oy);
            if (LockerHitTest(x, y, &hit)) {
                switch (hit) {
                case 2:
                    if (gamemode == 0) {
                        hm = (hm + 0x1B) % 0x1C;
                        if (hm >= 0x1A && hm == vis) hm = (vis + 0x1B) % 0x1C;
                    } else {
                        hm = (hm + 0x19) % 0x1A;
                        if (hm == vis) hm = (vis + 0x19) % 0x1A;
                    }
                    DrawLockerJersey(0, hm, teamdivflags[dword_D2150[vis]] | teamdivflags[dword_D2150[hm]], hm, vis);
                    ClearInputQueue();
                    MouseSetPos(x, y);
                    break;
                case 0:
                case 3:
                    if (gamemode == 0) {
                        hm = (hm + 1) % 0x1C;
                        if (hm >= 0x1A && hm == vis) hm = (vis + 1) % 0x1C;
                    } else {
                        hm = (hm + 1) % 0x1A;
                        if (hm == vis) hm = (vis + 1) % 0x1A;
                    }
                    DrawLockerJersey(0, hm, teamdivflags[dword_D2150[vis]] | teamdivflags[dword_D2150[hm]], hm, vis);
                    ClearInputQueue();
                    MouseSetPos(x, y);
                    break;
                case 4:
                    if (gamemode == 0) {
                        vis = (vis + 0x1B) % 0x1C;
                        if (vis >= 0x1A && hm == vis) vis = (vis + 0x1B) % 0x1C;
                    } else {
                        vis = (vis + 0x19) % 0x1A;
                        if (hm == vis) vis = (vis + 0x19) % 0x1A;
                    }
                    DrawLockerJersey(1, vis, teamdivflags[dword_D2150[vis]] | teamdivflags[dword_D2150[hm]], hm, vis);
                    ClearInputQueue();
                    MouseSetPos(x, y);
                    break;
                case 1:
                case 5:
                    if (gamemode == 0) {
                        vis = (vis + 1) % 0x1C;
                        if (vis >= 0x1A && hm == vis) vis = (vis + 1) % 0x1C;
                    } else {
                        vis = (vis + 1) % 0x1A;
                        if (hm == vis) vis = (vis + 1) % 0x1A;
                    }
                    DrawLockerJersey(1, vis, teamdivflags[dword_D2150[vis]] | teamdivflags[dword_D2150[hm]], hm, vis);
                    ClearInputQueue();
                    MouseSetPos(x, y);
                    break;
                case 6:
                    HomeTeam = dword_D2150[hm];
                    VisTeam = dword_D2150[vis];
                    if (gamemode == 0) {
                        p = (char *)off_D21C0[hm];
                        if (p[2] != 0) {
                            strncpy((char *)str_LAAtMTL + 7, p, 3);
                        } else {
                            strncpy((char *)str_LAAtMTL + 7, p, 2);
                            str_LAAtMTL[9] = ' ';
                        }
                        p = (char *)off_D21C0[vis];
                        if (p[2] != 0) {
                            strncpy((char *)str_LAAtMTL, p, 3);
                        } else {
                            strncpy((char *)str_LAAtMTL, p, 2);
                            str_LAAtMTL[2] = ' ';
                        }
                    }
                    if (ctl1team[0] >= 0) ctl1team[0] = ctl1side[0] == 0 ? HomeTeam : VisTeam;
                    if (ctl2team >= 0) ctl2team = ctl2side == 0 ? HomeTeam : VisTeam;
                    WriteCurModeState();
                    if (musicon && songdata) sub_8FCDF(musichandle, 3, 0x64);
                    sub_8FFB0(0, 0x100, buf);
                    FadePalStep(1, buf, 0x10);
                    FreeMemBlock(bm);
                    if (musicon && songdata) {
                        while (!sub_8FC8A(musicslot, 3));
                        sub_8D2F0(songdata);
                        songdata = 0;
                    }
                    return 0;
                case 7:
                    if (musicon && songdata) sub_8FCDF(musichandle, 3, 0x64);
                    sub_8FFB0(0, 0x100, buf);
                    FadePalStep(1, buf, 0x10);
                    FreeMemBlock(bm);
                    if (musicon && songdata) {
                        while (!sub_8FC8A(musicslot, 3));
                        sub_8D2F0(songdata);
                        songdata = 0;
                    }
                    return 3;
                }
            }
            sub_91400(bm, x - 4, y);
            sub_91FE0(pointerspr, x - 4, y);
            ox = x;
            oy = y;
        } else {
            if (musicon && songdata) sub_8FCDF(musichandle, 3, 0x64);
            sub_8FFB0(0, 0x100, buf);
            FadePalStep(1, buf, 0x10);
            FreeMemBlock(bm);
            if (musicon && songdata) {
                while (!sub_8FC8A(musicslot, 3));
                sub_8D2F0(songdata);
                songdata = 0;
            }
            return 3;
        }
    }
}
