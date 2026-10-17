/* Main startup: in-game hot keys. */
#include "nhl95.h"

/* HandleHotKey (1118F, PC only; owner of HandleHotKey_ret): act on a hot key from the input queue (bit 7 set, the
   scan code in bits 0-6). F1-F4 / F5-F8 (3Bh-3Eh / 3Fh-42h) request a line change for the home / away team
   (lcrequest, lcreqchoice), F9 / F10 show the home / away goalie banner, Tab (0Fh) toggles the jersey numbers
   (shownumbers), S (1Fh) toggles the sound effects and M (32h) the music (gameopts bits 7 / 6, starting or
   stopping the sound library), Esc (1) requests the exit (escrequest), and R (13h), while the game runs, plays the
   instant replay: save the joystick sampling and the mouse, fade out, stop the crowd, music and speech, run
   InstantReplay with the input handler, then restore the mouse, redraw the HUD panel, fade back in and scroll
   the rink view away so it is redrawn. key: the queued key. Result: 1 for Esc and after a replay, else 0. */
int HandleHotKey(int key)
{
    unsigned char k;

    k = key;
    if (!(k & 0x80)) return 0;
    k &= 0x7F;
    switch (k) {
    case 0x3B:
    case 0x3C:
    case 0x3D:
    case 0x3E:
        lcrequest[0] = 1;
        lcreqchoice[0] = k - 0x3B;
        break;
    case 0x3F:
    case 0x40:
    case 0x41:
    case 0x42:
        lcrequest[1] = 1;
        lcreqchoice[1] = k - 0x3F;
        break;
    case 0x43:
        ShowGoalieBanner(0);
        break;
    case 0x44:
        ShowGoalieBanner(1);
        break;
    case 0x0F:
        shownumbers = !shownumbers;
        break;
    case 0x13:
        if (gmode & 0x10) break;
        if (curperiod == -1) break;
        joysampling_save = joysampling;
        joysampling = 0;
        if (pad1dev[0] == 1 || pad2dev == 1) {
            savedmousex = mousex;
            savedmousey = mousey;
        }
        sub_8FFB0(0, 0x100, savepal);
        FadePalette(1, savepal, 0x10);
        CrowdFadeOut();
        if (musicon) MusicChanReset();
        else sub_8F633();
        SpeechStopQueue();
        StopDigiSample();
        InputInstall();
        InstantReplay(1);
        ClearInputQueue();
        InputRemove();
        if (pad1dev[0] == 1 || pad2dev == 1) {
            MouseSetPos(savedmousex, savedmousey);
            sub_B2E1B(0, 0, 0x140, 0xC8);
        }
        fadeinpending = 1;
        DrawHudPanel(HomeTeam, VisTeam, curperiod, 0);
        joysampling = joysampling_save;
        rinkscrollx += 0x3E8;
        rinkscrolly += 0x3E8;
        return 1;
    case 0x32:
        gameopts.music = !gameopts.music;
        if (gameopts.music) sub_8F979();
        else {
            StopDigiSample();
            sub_8F984();
        }
        break;
    case 0x1F:
        if (gameopts.sfx) {
            CrowdNoiseOff();
            if (musicon) MusicChanReset();
            else sub_8F633();
            sub_8F96E();
        }
        gameopts.sfx = !gameopts.sfx;
        if (gameopts.sfx) {
            CrowdNoiseReset();
            sub_8F963();
        }
        break;
    case 1:
        escrequest = 1;
        return 1;
    }
    return 0;
}

/* SetFullPalette (11550, PC only; ends in HandleHotKey_ret): set all 256 colours from the palette pal (3 bytes
   each) in two halves of 80h, each after a vertical retrace (sub_B4C84). pal: the palette. No result. */
void SetFullPalette(unsigned char *pal)
{
    int i;
    int o;

    for (i = 0, o = 0; i < 0x100; i += 0x80, o += 0x180) {
        sub_B4C84();
        sub_B4B88(i, 0x80, pal + o);
    }
}
