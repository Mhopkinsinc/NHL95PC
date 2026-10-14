/* Settings: locker room screen. */
#define LockerRoomScreen LockerRoomScreen_decl
#define LockerRoomLoop LockerRoomLoop_decl
#include "nhl95.h"
#undef LockerRoomScreen
#undef LockerRoomLoop

typedef struct { char c[17]; } Hdr17;

int LockerRoomLoop(void);

/* LockerRoomScreen: fade the music and the palette, save the text state, load the locker-room
 backdrops, draw the room and run it. Exhibition (gamemode 0) returns 2; any other mode returns
 the loop result. */
int LockerRoomScreen(void)
{
    unsigned char buf[0x340];
    int picked;
    int r;

    if (musicon && songdata)
        sub_8FCDF(musichandle, 3, 0x64);
    sub_8FFB0(0, 0x100, buf);
    FadePalStep(1, buf, 0x10);
    if (musicon && songdata) {
        int slot;

        do
            slot = musicslot;
        while (sub_8FC8A(slot, 3) == 0);
        sub_8D2F0(songdata);
        songdata = 0;
    }
    ShowLoadingScreen();
    sub_8E9E8(buf + 0x300);
    homebckspr = sub_8CCA8((char *)str_HomeBck, 0x1B811, 0x20);
    visbckspr = sub_8CCA8((char *)str_VisBck, 0x1A6F9, 0x20);
    *(Hdr17 *)homebckspr = *(Hdr17 *)pointerspr;
    *(Hdr17 *)visbckspr = *(Hdr17 *)pointerspr;
    ((short *)homebckspr)[2] = 0x140;
    ((short *)visbckspr)[2] = 0x138;
    ((short *)homebckspr)[3] = 0x160;
    ((short *)visbckspr)[3] = 0x15B;
    homenamebckspr = sub_8CCA8((char *)str_HomeNameBck, 0xA07, 0x20);
    visnamebckspr = sub_8CCA8((char *)str_VisNameBck, 0xA07, 0x20);
    *(Hdr17 *)homenamebckspr = *(Hdr17 *)pointerspr;
    *(Hdr17 *)visnamebckspr = *(Hdr17 *)pointerspr;
    ((short *)homenamebckspr)[2] = 0xAA;
    ((short *)visnamebckspr)[2] = 0xAA;
    ((short *)homenamebckspr)[3] = 0xF;
    ((short *)visnamebckspr)[3] = 0xF;
    titlebckspr = sub_8CCA8((char *)str_TitleBck, 0x35C7, 0x20);
    *(Hdr17 *)titlebckspr = *(Hdr17 *)pointerspr;
    ((short *)titlebckspr)[2] = 0x226;
    ((short *)titlebckspr)[3] = 0x19;
    DrawLockerRoom();
    picked = LockerRoomLoop();
    FreeMemBlock(titlebckspr);
    FreeMemBlock(homenamebckspr);
    FreeMemBlock(visnamebckspr);
    FreeMemBlock(homebckspr);
    FreeMemBlock(visbckspr);
    sub_8EA00(buf + 0x300);
    if (gamemode == 0)
        r = 2;
    else
        r = picked;
    return r;
}
