/* Sound config: the sound card dialog. */
#define SoundCardDlgLoop SoundCardDlgLoop_decl
#define SetSoundDevice SetSoundDevice_decl
#define sub_8F98F sub_8F98F_decl
#define sub_8FB8E sub_8FB8E_decl
#include "nhl95.h"
#undef SoundCardDlgLoop
#undef SetSoundDevice
#undef sub_8F98F
#undef sub_8FB8E

int SetSoundDevice(int dev);
int sub_8F98F(char *path);  /* sound library: load a song */
void sub_8FB8E(int song, int handle, int a, int b);  /* sound library: start a song */

typedef struct { char b[0x11]; } SprHdr;

/* SoundCardDlgLoop (8291E, PC only; owner of the SoundCardDlg_epilogue tail SetSoundDevice ends in): run the
   sound card dialog with the mouse pointer (a POINTER block holds the screen under it; dword_D2435 is freed
   first). A click (event bit 2) on an available device that is not the chosen one (SoundCardHitTest: setbits
   bit, byte_C541B as a dword of available bits) picks it (bits 1-20h). The OK button (40h) stops the music and
   tries SetSoundDevice: on success the device becomes sounddev, the music restarts (MAINDESK.IFF, from the CD
   when it is there) and gameopts bit 8 follows musicon; on failure the device is marked unavailable, the old
   one is chosen again and the music restarts. Cancel (80h) or event bit 4 leaves. Result: 0. */
int SoundCardDlgLoop(void)
{
    struct {
        char path[0x20];
        int hit;
        int ev;
        int y;
        int x;
        int my;
        int mx;
    } s;
    int bg;
    int t;  /* the size, the event and the song: one variable keeps the song in eax (a separate one copies it to esi) */

    t = (((short *)pointerspr)[2] + 1) * (((short *)pointerspr)[3] + 1) + 0x11;
    bg = sub_8CCA8((char *)str_Pointer21, t, 0);
    *(SprHdr *)bg = *(SprHdr *)pointerspr;
    ((short *)bg)[2] = ((short *)pointerspr)[2] + 1;
    ((short *)bg)[3] = ((short *)pointerspr)[3] + 1;
    if (dword_D2435) {
        FreeMemBlock(dword_D2435);
        dword_D2435 = 0;
    }
    sub_B2DCA(&s.ev, &s.mx, &s.my);
    s.x = s.mx;
    s.y = s.my;
    sub_B4BA8();
    sub_91400(bg, s.x - 4, s.y);
    sub_91370(pointerspr, s.x - 4, s.y);
    ClearInputQueue();
    for (;;) {
        s.ev = 0;
        while ((t = (int)GetInputEvent()) != 0) {
            s.ev = ((int (*)(unsigned char *, int *, int *))ptrupdatefn)((unsigned char *)t, &s.x, &s.y);
            if (s.ev & 6) break;
        }
        if (!(s.ev & 2) && !(s.ev & 4)) {
            if (s.x != s.mx || s.y != s.my) {
                sub_903F0(bg, s.mx - 4, s.my);
                sub_91400(bg, s.x - 4, s.y);
                sub_91370(pointerspr, s.x - 4, s.y);
                s.mx = s.x;
                s.my = s.y;
            }
        } else if (s.ev & 2) {
            sub_903F0(bg, s.mx - 4, s.my);
            if (SoundCardHitTest(s.x, s.y, &s.hit)) {
                s.hit = 1 << s.hit;
                if (!(s.hit & setbits[0]) && (s.hit & *(int *)&byte_C541B)) {
                    switch ((unsigned)s.hit) {
                    case 1:
                    case 2:
                    case 4:
                    case 8:
                    case 0x10:
                    case 0x20:
                        setbits[0] = s.hit;
                        break;
                    case 0x40:
                        DrawSelBoxOn((Rect4 *)unk_D2403);
                        SetDialogColors(0xF9, 0xFA, 0xF8, 0xFA, 0xF7);
                        if (musicon && songdata) {
                            sub_8FCDF(musichandle, 3, 0x64);
                            while (!sub_8FC8A(musicslot, 3));
                            sub_8D2F0(songdata);
                            songdata = 0;
                        }
                        if (SetSoundDevice(setbits[0])) {
                            sounddev = setbits[0];
                            FreeMemBlock(bg);
                            if (musicon && !songdata) {
                                MakePath(s.path, fileoncd[0x1DB] == 1 ? (char *)cddriveptr : 0, (char *)str_Maindesk2, (char *)str_Iff12);
                                t = sub_8F98F(s.path);
                                songdata = t;
                                if (t && gameopts.music) sub_8FB8E(t, musichandle, 3, 0x4C);
                            }
                            if (musicon) gameopts.speech = 1;
                            else gameopts.speech = 0;
                            return 0;
                        }
                        *(int *)&byte_C541B ^= setbits[0];
                        setbits[0] = sounddev;
                        if (musicon && !songdata) {
                            MakePath(s.path, fileoncd[0x1DB] == 1 ? (char *)cddriveptr : 0, (char *)str_Maindesk2, (char *)str_Iff12);
                            t = sub_8F98F(s.path);
                            songdata = t;
                            if (t && gameopts.music) sub_8FB8E(t, musichandle, 3, 0x4C);
                        }
                        DrawSelBoxOff((Rect4 *)unk_D2403);
                        break;
                    case 0x80:
                        DrawSelBoxOn((Rect4 *)unk_D2413);
                        FreeMemBlock(bg);
                        return 0;
                    }
                    RedrawSoundCardOpts();
                }
            }
            sub_91400(bg, s.x - 4, s.y);
            sub_91370(pointerspr, s.x - 4, s.y);
            s.mx = s.x;
            s.my = s.y;
        } else if (s.ev & 4) {
            sub_903F0(bg, s.mx - 4, s.my);
            FreeMemBlock(bg);
            return 0;
        } else {
            sub_903F0(bg, s.mx - 4, s.my);
            sub_91400(bg, s.x - 4, s.y);
            sub_91370(pointerspr, s.x - 4, s.y);
            s.mx = s.x;
            s.my = s.y;
        }
    }
}
