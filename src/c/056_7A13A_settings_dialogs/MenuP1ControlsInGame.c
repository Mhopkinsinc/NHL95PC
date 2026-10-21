/* Settings dialogs: the controls dialog family (the box bevels, the exhibition, league, in-game and playoff
 dialogs, their drawing and loop) and the in-game control reassignment, in address order: one source block (the
 in-game dialog's end follows ClearCtlBlink, and the family ends in its exits, ControlsDlg_ret). The functions
 from DrawCtlBoxOn to ControlsDlgLeague and from MenuP1ControlsPlayoff on are first passes from Ghidra
 (tools/ghidra_decomp.py). */
#define MenuP1ControlsExh MenuP1ControlsExh_decl
#define MenuP2ControlsExh MenuP2ControlsExh_decl
#define ControlsDlgExh ControlsDlgExh_decl
#define MenuP1ControlsLeague MenuP1ControlsLeague_decl
#define MenuP2ControlsLeague MenuP2ControlsLeague_decl
#define ControlsDlgLeague ControlsDlgLeague_decl
#define MenuP1ControlsPlayoff MenuP1ControlsPlayoff_decl
#define MenuP2ControlsPlayoff MenuP2ControlsPlayoff_decl
#define ControlsDlgPlayoff ControlsDlgPlayoff_decl
#define DrawControlsDlg DrawControlsDlg_decl
#define DrawControlsOpts DrawControlsOpts_decl
#define ControlsDlgLoop ControlsDlgLoop_decl
#include "nhl95.h"
#undef MenuP1ControlsExh
#undef MenuP2ControlsExh
#undef ControlsDlgExh
#undef MenuP1ControlsLeague
#undef MenuP2ControlsLeague
#undef ControlsDlgLeague
#undef MenuP1ControlsPlayoff
#undef MenuP2ControlsPlayoff
#undef ControlsDlgPlayoff
#undef DrawControlsDlg
#undef DrawControlsOpts
#undef ControlsDlgLoop

int ControlsDlgExh(int side);
int ControlsDlgLeague(int side);
int ControlsDlgPlayoff(int side);
void DrawControlsDlg(int alt);
void DrawControlsOpts(int side);
int ControlsDlgLoop(int side);

/* DRAFT (whole file): every function's body matches on its own (ClearCtlBlink and MenuP2ControlsInGame byte for byte,
 ControlsDlgInGame's head and tail, ReassignCtlPlayer up to stack slots the px register). The EXE has ControlsDlgInGame
 split: its code from the ctl1team test on (ControlsDlgInGame_side.. ControlsDlg_ret0) sits after ClearCtlBlink. With
 ControlsDlgInGame defined after ClearCtlBlink (as here), Watcom splits it the same way: MenuP1ControlsInGame falls
 into its entry, the rest stays after ClearCtlBlink (156 -> 3 differing). */

typedef struct Hdr17 { char c[17]; } Hdr17;
#define TEAM(t) ((&hmtmstruct)[t])


/* DRAFT (first pass from Ghidra, tools/ghidra_decomp.py): the box colours come from ctldlgalt; both end in the
   shared ControlsDlg_ret exit of ControlsDlgInGame's split tail. */
/* DrawCtlBoxOn (7C852, PC only): draw the bevel of the controls dialog box r (four ints: left, top, right,
   bottom) pressed in: the left and top edges in the shade colour, the right and bottom ones in the light colour
   (42h / 40h in the alternate palette of the playoff and exhibition dialogs, ctldlgalt; else F8h / FAh). No
   result. */
void DrawCtlBoxOn(int *r)
{
    int lite;
    int shade;

    if (ctldlgalt) {
        lite = 0x40;
        shade = 0x42;
    } else {
        lite = 0xFA;
        shade = 0xF8;
    }
    sub_B4FAC(r[0] + 0xA, r[3] + 0x12, r[0] + 0xA, r[1] + 0x13, shade);
    sub_B4FAC(r[0] + 0xA, r[1] + 0x13, r[2] + 9, r[1] + 0x13, shade);
    sub_B4FAC(r[2] + 0xA, r[1] + 0x14, r[2] + 0xA, r[3] + 0x13, lite);
    sub_B4FAC(r[2] + 0xA, r[3] + 0x13, r[0] + 0xB, r[3] + 0x13, lite);
}

/* DrawCtlBoxOff (7C901, PC only): DrawCtlBoxOn's bevel the other way round (raised: light left and top, shade
   right and bottom). r: the box. No result. */
void DrawCtlBoxOff(int *r)
{
    int lite;
    int shade;

    if (ctldlgalt) {
        lite = 0x40;
        shade = 0x42;
    } else {
        lite = 0xFA;
        shade = 0xF8;
    }
    sub_B4FAC(r[0] + 0xA, r[3] + 0x12, r[0] + 0xA, r[1] + 0x13, lite);
    sub_B4FAC(r[0] + 0xA, r[1] + 0x13, r[2] + 9, r[1] + 0x13, lite);
    sub_B4FAC(r[2] + 0xA, r[1] + 0x14, r[2] + 0xA, r[3] + 0x13, shade);
    sub_B4FAC(r[2] + 0xA, r[3] + 0x13, r[0] + 0xB, r[3] + 0x13, shade);
}

/* MenuP1ControlsExh (7C993, PC only): the exhibition controls dialog for controller 1. Result 0. */
int MenuP1ControlsExh(void)
{
    return ControlsDlgExh(0);
}

/* MenuP2ControlsExh (7C9A1, PC only): the same for controller 2 (falls through into ControlsDlgExh). */
int MenuP2ControlsExh(void)
{
    return ControlsDlgExh(1);
}

/* DRAFT (first pass from Ghidra). */
/* ControlsDlgExh (7C9B0, PC only): the controls dialog of the exhibition settings for controller side: load the
   BKGD4 background, save the screen under it (at 98h, 0ECh), draw the dialog (the alternate colours in
   gamemode 2), its options and run it (DrawControlsDlg, DrawControlsOpts, ControlsDlgLoop), then restore the
   screen and clear ctldlgalt. Result 0. */
int ControlsDlgExh(int side)
{
    int buf;

    buf = sub_8CCA8((char *)str_BKGD4, 0x8C31, 0x20);
    *(Hdr17 *)buf = *(Hdr17 *)pointerspr;
    *(short *)(buf + 4) = 0x98;
    *(short *)(buf + 6) = 0xEC;
    sub_91400(buf, 0xA, 0x13);
    DrawControlsDlg(gamemode == 2);
    DrawControlsOpts(side);
    ControlsDlgLoop(side);
    sub_903F0(buf, 0xA, 0x13);
    FreeMemBlock(buf);
    ctldlgalt = 0;
    return 0;
}

/* MenuP1ControlsLeague (7CA53, PC only): the league controls dialog for controller 1. Result 0. */
int MenuP1ControlsLeague(void)
{
    return ControlsDlgLeague(0);
}

/* MenuP2ControlsLeague (7CA61, PC only): the same for controller 2 (falls through into ControlsDlgLeague). */
int MenuP2ControlsLeague(void)
{
    return ControlsDlgLeague(1);
}

/* DRAFT (first pass from Ghidra): the EXE cross-jumps its end into ControlsDlgExh's (ControlsDlg_done). */
/* ControlsDlgLeague (7CA70, PC only): ControlsDlgExh for the league settings, always in the normal colours.
   side: the controller. Result 0. */
int ControlsDlgLeague(int side)
{
    int buf;

    buf = sub_8CCA8((char *)str_BKGD4, 0x8C31, 0x20);
    *(Hdr17 *)buf = *(Hdr17 *)pointerspr;
    *(short *)(buf + 4) = 0x98;
    *(short *)(buf + 6) = 0xEC;
    sub_91400(buf, 0xA, 0x13);
    DrawControlsDlg(0);
    DrawControlsOpts(side);
    ControlsDlgLoop(side);
    sub_903F0(buf, 0xA, 0x13);
    FreeMemBlock(buf);
    ctldlgalt = 0;
    return 0;
}

/* MenuP1ControlsInGame: the in-game controls dialog for controller 1. */
void MenuP1ControlsInGame(void)
{
    ControlsDlgInGame(0);
}

/* MenuP2ControlsInGame: the in-game controls dialog for controller 2. */
void MenuP2ControlsInGame(void)
{
    ControlsDlgInGame(1);
}

/* ReassignCtlPlayer:: after the controls changed, give controller
 side the player of its team nearest the puck's next position (puck + velocity 256): skaters on the ice
 (position >= 0, not pflags2 bit 2 or pflags bit 20h), the puck carrier counting as nearest, not the player the
 other controller has. A controller without a team gives its player up (-1); one whose player is already on its
 team keeps it. restorepl swaps the old and the new player. */
void ReassignCtlPlayer(int side)
{
    short *team;
    short *pnum;
    short py;
    short best;
    short px;
    struct Player *p;
    short n;
    int dx;
    int d;
    int bestd;
    short pl;

    if (side) {
        team = &cont2team;
        pnum = c2playernum;
    } else {
        team = &cont1team;
        pnum = c1playernum;
    }
    if (*team == 0) {
        if (*pnum == -1) return;
        best = -1;
    } else {
        if (*team == 1 && *pnum >= 0 && *pnum <= 5) return;
        if (*team == 2 && *pnum >= 6 && *pnum <= 11) return;
        px = (*puckvx >> 8) + *puckx;
        py = *pucky + (*puckvy >> 8);
        bestd = 0x10000000;
        if (*team == 1) p = SortCords;
        else p = SortCords + 6;
        n = 6;
        do {
            if (p->position >= 0 && !(p->pflags2 & 4) && !(p->pflags & 0x20)) {
                dx = px - HIWORD(p->Xpos);
                d = py - HIWORD(p->Ypos);
                d = dx * dx + d * d;
                if ((short)*puckc == p->SCnum) d = -1;
                if (d <= bestd) {
                    pl = p->SCnum;
                    if (pl != (side ? c1playernum[0] : c2playernum[0])) {
                        bestd = d;
                        best = pl;
                    }
                }
            }
            p++;
        } while (--n);
    }
    if (side == 0) {
        if (best != c1playernum[0]) c1playernum[0] = restorepl(best, c1playernum[0]);
    } else {
        if (best != c2playernum[0]) c2playernum[0] = restorepl(best, c2playernum[0]);
    }
}

/* ClearCtlBlink: for each team without a controller: stop its line change blink (lcblink /
 lcboxon), clear pf2lcm on its six players and bit 1 of tmflags, and move it off a line that does not fit the
 strength: short-handed (tmap below the other team's) to line 6 unless already 6 up, power play to line 4 unless
 on 4 5, even strength from line 3 up back to line 0. */
void ClearCtlBlink(void)
{
    int t;
    int i;
    int diff;
    int line;
    int team;

    for (t = 0; t < 2; t++) {
        team = (t != 0) + 1;
        if (cont1team == team || cont2team == team) continue;
        lcboxon[t] = lcblink[t] = 0;
        if (t) {
            for (i = 6; i < 12; i++) SortCords[i].pflags2 &= ~pf2lcm;
        } else {
            for (i = 0; i < 6; i++) SortCords[i].pflags2 &= ~pf2lcm;
        }
        TEAM(t).tmflags &= ~2;
        diff = TEAM(t == 0).tmap - TEAM(t).tmap;
        line = TEAM(t).tmline;
        if (diff > 0 && line < 6) TEAM(t).tmline = 6;
        if (diff < 0 && (line > 5 || line < 4)) TEAM(t).tmline = 4;
        if (diff == 0 && line >= 3) TEAM(t).tmline = 0;
    }
}

/* ControlsDlgInGame: the controls dialog during a game for controller side: load the BKGD4
 background, save the screen under it (at 98h, 0ECh in its header), run the dialog (DrawControlsDlg /
 DrawControlsOpts ControlsDlgLoop) and restore the screen. Then apply the choice: the controller's team
 (cont1team cont2team: 0 none, 1 home, 2 away from ctl1team ctl1side), samesideflag when both controllers
 are on one side, the pad device bit (pad1dev) from ctl1dev, the controlled player (ReassignCtlPlayer) and the
 line change blink (ClearCtlBlink); a team that gets or loses its controllers swaps its score flags
 (CtlSwapScoreFix CtlSwapTeamFlag). */
int ControlsDlgInGame(int side)
{
    int bkgd;
    int buf;
    int result;
    int team;
    int old1;
    int old2;

    result = 0;
    buf = bkgd = sub_8CCA8((char *)str_BKGD4, 0x8C31, 0x20);
    *(Hdr17 *)buf = *(Hdr17 *)pointerspr;
    *(short *)(buf + 4) = 0x98;
    *(short *)(buf + 6) = 0xEC;
    sub_91400(buf, 0xA, 0x13);
    DrawControlsDlg(0);
    DrawControlsOpts(side);
    ControlsDlgLoop(side);
    sub_903F0(buf, 0xA, 0x13);
    if (ctl1team[side] < 0) team = 0;
    else if (ctl1side[side] == 0) team = 1;
    else team = 2;
    old1 = cont1team;
    old2 = cont2team;
    if (side) cont2team = team;
    else cont1team = team;
    if (ctl1team[0] >= 0 && ctl2team >= 0 && ctl1side[0] == ctl2side) samesideflag = 1;
    else samesideflag = 0;
    switch (ctl1dev[side]) {
    case 1: pad1dev[side] = 1; break;
    case 2: pad1dev[side] = 2; break;
    case 4: pad1dev[side] = 4; break;
    case 8: pad1dev[side] = 8; break;
    case 0x10: pad1dev[side] = 0; break;
    }
    ReassignCtlPlayer(side);
    ClearCtlBlink();
    if (puckasslist[puckstruct[6] >> 16] == 0x1B && team == 1 && old1 != team && old2 != team) pucktemp1 = 0;
    for (team = 1; team <= 2; team++) {
        if ((old1 == team || old2 == team) && cont1team != team && cont2team != team)
            CtlSwapScoreFix(team == 2);
        else if (old1 != team && old2 != team && (cont1team == team || cont2team == team))
            CtlSwapTeamFlag(team == 2);
    }
    FreeMemBlock(buf);
    return result;
}

/* MenuP1ControlsPlayoff (7D137, PC only): the playoff controls dialog for controller 1. Result 0. */
int MenuP1ControlsPlayoff(void)
{
    return ControlsDlgPlayoff(0);
}

/* MenuP2ControlsPlayoff (7D145, PC only): the same for controller 2 (falls through into ControlsDlgPlayoff). */
int MenuP2ControlsPlayoff(void)
{
    return ControlsDlgPlayoff(1);
}

/* DRAFT (first pass from Ghidra). */
/* ControlsDlgPlayoff (7D154, PC only): ControlsDlgExh for the playoffs (the alternate colours), and then the
   controller's team as ControlsDlgInGame sets it (cont1team / cont2team: 0 none, 1 home, 2 away from ctl1team
   ctl1side) and samesideflag when both controllers are on one side. side: the controller. Result 0. */
int ControlsDlgPlayoff(int side)
{
    int buf;
    short team;

    buf = sub_8CCA8((char *)str_BKGD4, 0x8C31, 0x20);
    *(Hdr17 *)buf = *(Hdr17 *)pointerspr;
    *(short *)(buf + 4) = 0x98;
    *(short *)(buf + 6) = 0xEC;
    sub_91400(buf, 0xA, 0x13);
    DrawControlsDlg(1);
    DrawControlsOpts(side);
    ControlsDlgLoop(side);
    sub_903F0(buf, 0xA, 0x13);
    if (ctl1team[side] < 0) team = 0;
    else if (ctl1side[side] == 0) team = 1;
    else team = 2;
    if (side) cont2team = team;
    else cont1team = team;
    if (ctl1team[0] >= 0 && ctl2team >= 0 && ctl1side[0] == ctl2side) samesideflag = 1;
    else samesideflag = 0;
    FreeMemBlock(buf);
    return 0;
}

/* DRAFT (first pass from Ghidra). */
/* DrawControlsDlg (7D254, PC only): draw the controls dialog: an identity colour remap (with alt, the
   alternate palette: F7h-FAh to 43h-40h; alt is kept in ctldlgalt), the DBOX5 art of the PLAYIMG bank (from the
   CD when fileoncd+138h is 1) at 0Ah, 13h, then in the light colour (40h alt, else FAh) the home and away team
   boxes (ctldlgrects 0 and 1, filled in remap colour F9h) with the team city names (HomeTeam, VisTeam). alt:
   the alternate colours. No result. */
void DrawControlsDlg(int alt)
{
    unsigned char remap[0x100];
    char path[0x20];
    int i;
    int bank;
    int x;
    int y;

    sub_B4BA8();
    for (i = 0; i < 0x100; i++) remap[i] = i;
    ctldlgalt = alt;
    if (alt) {
        remap[0xFA] = 0x40;
        remap[0xF9] = 0x41;
        remap[0xF8] = 0x42;
        remap[0xF7] = 0x43;
    }
    sub_B4DD4(remap);
    MakePath(path, fileoncd[0x138] == 1 ? (char *)cddriveptr : 0, (char *)str_PlayerImg, 0);
    bank = sub_8E83C(path, 0);
    sub_931FC(sub_B30B4(bank, (char *)str_Dbox5), 0xA, 0x13);
    FreeMemBlock(bank);
    sub_8E9C0(alt ? 0x40 : 0xFA, 0xFF);
    x = ctldlgrects[0];
    y = ctldlgrects[1] + 0x14;
    sub_90D20(x + 0xB, y, ctldlgrects[2] + 0xA - (x + 0xB), ctldlgrects[3] + 0x13 - y, remap[0xF9]);
    sub_91964((char *)teamcitynames[HomeTeam], x + 0xE, y);
    x = ctldlgrects[4];
    y = ctldlgrects[5] + 0x14;
    sub_90D20(x + 0xB, y, ctldlgrects[6] + 0xA - (x + 0xB), ctldlgrects[7] + 0x13 - y, remap[0xF9]);
    sub_91964((char *)teamcitynames[VisTeam], x + 0xE, y);
}

/* DRAFT (first pass from Ghidra). */
/* DrawControlsOpts (7D3F0, PC only): the controls dialog's choices for controller side: the title (ONE / TWO),
   the available devices in setbits[1] (always the teams, none and the buttons; the mouse, joysticks and
   keyboard from ctlavailmask, less the other controller's device unless that is none, 10h), the current choice
   in setbits[0] (ctl1team -1: none away 41h, -2: none home 42h, else the side bit (1 home, 2 away) and the
   device bit), every box drawn on or off (DrawCtlBoxOn / DrawCtlBoxOff) and the names of the devices that are
   not available, in the shade colour. side: the controller. No result. */
void DrawControlsOpts(int side)
{
    int i;

    sub_8E9C0(ctldlgalt ? 0x40 : 0xFA, 0xFF);
    sub_91964(side ? (char *)str_TwoS : (char *)str_OneS, 0x4A, 0x20);
    setbits[0] = 0;
    setbits[1] = 0x1C3 | (ctlavailmask & 1) << 2 | ((ctlavailmask & 3) >> 1) << 3 | ((ctlavailmask & 7) >> 2) << 4
                 | ((ctlavailmask & 0xF) >> 3) << 5;
    if (ctl1dev[side == 0] != 0x10) setbits[1] &= ~(ctl1dev[side == 0] << 2);
    if (ctl1team[side] < 0) {
        if (ctl1team[side] == -1) setbits[0] = 0x41;
        else if (ctl1team[side] == -2) setbits[0] = 0x42;
    } else {
        setbits[0] = (ctl1side[side] != 0) + 1;
        if (setbits[1] & ctl1dev[side] << 2) setbits[0] |= ctl1dev[side] << 2;
    }
    for (i = 0; i < 9; i++) {
        if (setbits[0] & 1 << i) DrawCtlBoxOn(&ctldlgrects[i * 4]);
        else DrawCtlBoxOff(&ctldlgrects[i * 4]);
    }
    sub_8E9C0(ctldlgalt ? 0x42 : 0xF8, 0xFF);
    if (!(setbits[1] & 4)) sub_91964((char *)str_TheMouse, ctldlgrects[8] + 0x19, ctldlgrects[9] + 0x14);
    if (!(setbits[1] & 8)) sub_91964((char *)str_JoystickOne, ctldlgrects[0xC] + 0x12, ctldlgrects[0xD] + 0x14);
    if (!(setbits[1] & 0x10)) sub_91964((char *)str_JoystickTwo, ctldlgrects[0x10] + 0x11, ctldlgrects[0x11] + 0x14);
    if (!(setbits[1] & 0x20)) sub_91964((char *)str_TheKeyboard, ctldlgrects[0x14] + 0xF, ctldlgrects[0x15] + 0x14);
}

/* DRAFT (first pass from Ghidra): the EXE's shared exits (ControlsDlg_ret) and the bit switch are not shaped yet. */
/* ControlsDlgLoop (7D6B1, PC only): run the controls dialog for controller side with the mouse pointer (its
   background saved in a POINTER18 block). A click on an available box not already chosen (CtlDlgHitTest)
   moves the choice: the side bits 1 / 2 and none 40h replace each other, as do the device bits 4-20h, and the
   boxes are redrawn (DrawCtlBoxes). OK (box 7) stores the choice (ctl1dev, ctl1team / ctl1side from the home or
   away team, -1 / -2 for none, moving the other controller off a side it can no longer share), applies it
   (SetSideControls) and writes the mode state (WriteCurModeState); Cancel (box 8) or the right button leaves it
   unchanged. Result 0. */
int ControlsDlgLoop(int side)
{
    int btn;
    int x;
    int y;
    int ox;
    int oy;
    int hit;
    unsigned bit;
    int save;
    unsigned char *ev;
    int other;

    sub_B2DCA(&btn, &oy, &ox);
    x = ox;
    y = oy;
    save = sub_8CCA8((char *)str_Pointer18, (*(short *)(pointerspr + 4) + 1) * (*(short *)(pointerspr + 6) + 1) + 0x11, 0x20);
    *(Hdr17 *)save = *(Hdr17 *)pointerspr;
    *(short *)(save + 4) = *(short *)(pointerspr + 4) + 1;
    *(short *)(save + 6) = *(short *)(pointerspr + 6) + 1;
    sub_91400(save, x - 4, y);
    sub_91370(pointerspr, x - 4, y);
    ClearInputQueue();
    other = side == 0;
    for (;;) {
        btn = 0;
        while ((ev = GetInputEvent()) != 0) {
            btn = ((int (*)(unsigned char *, int *, int *))ptrupdatefn)(ev, &x, &y);
            if (btn & 6) break;
        }
        if (btn & 4) {
            sub_903F0(save, ox - 4, oy);
            break;
        }
        if (btn & 2) {
            sub_903F0(save, ox - 4, oy);
            if (CtlDlgHitTest(x, y, &hit)) {
                bit = 1 << hit;
                if (!(bit & setbits[0]) && (bit & setbits[1])) {
                    if (bit == 0x80) {
                        DrawCtlBoxOn(&ctldlgrects[7 * 4]);
                        ctl1dev[side] = setbits[0] >> 2;
                        if (setbits[0] == 0x41) {
                            ctl1team[side] = -1;
                            ctl1side[side] = 0;
                            if (ctl1team[other] == -1) {
                                ctl1team[other] = -2;
                                ctl1side[other] = 1;
                            } else if (ctl1side[other] == 0) {
                                ctl1team[other] = VisTeam;
                                ctl1side[other] = 1;
                            }
                        } else if (setbits[0] == 0x42) {
                            ctl1team[side] = -2;
                            ctl1side[side] = 1;
                            if (ctl1team[other] == -2) {
                                ctl1team[other] = -1;
                                ctl1side[other] = 0;
                            } else if (ctl1side[other] == 1) {
                                ctl1team[other] = HomeTeam;
                                ctl1side[other] = 0;
                            }
                        } else if (setbits[0] & 1) {
                            ctl1team[side] = HomeTeam;
                            ctl1side[side] = 0;
                            if (ctl1team[other] == -1) {
                                ctl1team[other] = -2;
                                ctl1side[other] = 1;
                            }
                        } else {
                            ctl1team[side] = VisTeam;
                            ctl1side[side] = 1;
                            if (ctl1team[other] == -2) {
                                ctl1team[other] = -1;
                                ctl1side[other] = 0;
                            }
                        }
                        SetSideControls();
                        WriteCurModeState();
                        break;
                    }
                    if (bit == 0x100) {
                        DrawCtlBoxOn(&ctldlgrects[8 * 4]);
                        break;
                    }
                    if (bit == 1 || bit == 2) setbits[0] = (setbits[0] & ~3) | bit;
                    else if (bit >= 4 && bit <= 0x40) setbits[0] = (setbits[0] & ~0x7C) | bit;
                    DrawCtlBoxes();
                }
            }
        } else {
            if (x == ox && y == oy) continue;
            sub_903F0(save, ox - 4, oy);
        }
        sub_91400(save, x - 4, y);
        sub_91370(pointerspr, x - 4, y);
        ox = x;
        oy = y;
    }
    FreeMemBlock(save);
    return 0;
}
