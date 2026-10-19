/* Sound config: choosing the sound device. */
#define SetSoundDevice SetSoundDevice_decl
#define sub_97079 sub_97079_decl
#define sub_8EB5B sub_8EB5B_decl
#define sub_8EB93 sub_8EB93_decl
#define sub_8ECC0 sub_8ECC0_decl
#define sub_8F13B sub_8F13B_decl
#define sub_8F1FE sub_8F1FE_decl
#include "nhl95.h"
#undef SetSoundDevice
#undef sub_97079
#undef sub_8EB5B
#undef sub_8EB93
#undef sub_8ECC0
#undef sub_8F13B
#undef sub_8F1FE

int fprintf(void *fp, const char *fmt, ...);  /* Watcom CRT fprintf_ */
int sub_97079(int dev);                       /* sound library: open a driver, -1 on failure */
void sub_8EB5B(void);
void sub_8EB93(void);
void sub_8ECC0(char *drv, char *port);        /* sound library: driver and port names */
int sub_8F13B(char *path);                    /* sound library: load a sample */
void sub_8F1FE(int sample);                   /* sound library: free a sample */

/* DRAFT: 1346 vs 1346, 314 of 326 instructions; see docs/UNSURE.md. Ends in SoundCardDlgLoop's epilogue
   (SoundCardDlg_epilogue), so a match goes into SoundCardDlgLoop.c. */
/* SetSoundDevice (82D7A, PC only): switch the sound to device dev (setbits bits: 1 PC speaker, 2 Sound Blaster,
   4 AdLib, 8 MT-32, 20h Ultrasound, 10h none). Shut the current one down first (speech and its copy buffer for
   2 / 20h, the AdLib sample for 4). Then open dev's music and digital drivers (sub_97079 of dword_D2427 /
   dword_D242B); on success name them to the sound library, load the AdLib SLAPSHOT sample, play the MT32HOCK
   sample once for the MT-32 (with a message), make dev sounddev and setbits, set the slots, write its index in
   sounddevids to NHL.CFG and turn music on for 2 / 20h. When a device fails, say so (its name in sndcardname)
   until a click and fall back to 10h. Then start speech for 2 / 20h. Result: 1. */
int SetSoundDevice(int dev)
{
    struct {
        char path[0x10];
        char port[0x10];
        char drv[0x10];
        int y;
        int x;
        int btn;
        int h2;
        int ev;
    } s;
    int h1;
    void *fp;
    int i;
    int t;
    unsigned char *e;

    h1 = 0;
    s.h2 = 1;
    if (dword_D2423) {
        if (sounddev == 2 || sounddev == 0x20) {
            ShutdownSpeech();
            FreeMemBlock(dword_ED7A8);
            dword_ED7A8 = 0;
        } else if (sounddev == 4) {
            sub_8F1FE(dword_ED7A4);
        }
        sub_8EB93();
    }
    dword_CCC94 = 0x20;
    dword_D2423 = 0;
    do {
        switch (dev) {
        case 1:
            strcpy(s.drv, (char *)str_PCBEEP);
            strcpy(s.port, (char *)str_PC);
            dword_D2427 = 1;
            dword_D242B = -1;
            byte_D2439 = 0;
            break;
        case 2:
            strcpy(s.drv, (char *)str_SBDAC);
            strcpy(s.port, (char *)str_PC);
            dword_D2427 = 2;
            dword_D242B = 3;
            byte_D2439 = musicslot = 1;
            break;
        case 4:
            strcpy(s.drv, (char *)str_ADLIB);
            strcpy(s.port, (char *)str_PC);
            dword_D2427 = 2;
            dword_D242B = -1;
            byte_D2439 = 0;
            break;
        case 8:
            strcpy(s.drv, (char *)str_MT322);
            strcpy(s.port, (char *)str_PC);
            dword_D2427 = 4;
            dword_D242B = -1;
            byte_D2439 = 0;
            break;
        case 0x20:
            strcpy(s.drv, (char *)str_SBDAC2);
            strcpy(s.port, (char *)str_PC);
            dword_D2427 = 5;
            dword_D242B = 6;
            byte_D2439 = musicslot = 1;
            break;
        default:
            strcpy(s.drv, (char *)str_PCBEEP2);
            strcpy(s.port, (char *)str_PC);
            dword_D2427 = 0;
            dword_D242B = -1;
            byte_D2439 = 0;
            break;
        }
        sub_8EB5B();
        if (dword_D2427 >= 0 && (h1 = sub_97079(dword_D2427)) != -1
            && (dword_D242B < 0 || (s.h2 = sub_97079(dword_D242B)) != -1))
            dword_D2423 = -1;
        if (dword_D2423) {
            sub_8ECC0(s.drv, s.port);
            if (dev == 4) {
                MakePath(s.path, fileoncd[0x18F] == 1 ? (char *)cddriveptr : 0, (char *)str_Slapshot, 0);
                dword_ED7A4 = sub_8F13B(s.path);
            }
            if (dev == 8 && !dword_D2350) {
                MessageBox(-1, -1, (char *)msg_InitMT32, 2, 0, 0, (int)&s.x, (int)&s.y, 0);
                MakePath(s.path, fileoncd[0xF7] == 1 ? (char *)cddriveptr : 0, (char *)str_MT32HOCK, 0);
                t = sub_8F13B(s.path);
                dword_D2350 = -1;
                sounddev = 8;
                PlayDigiSample((void *)t);
                WaitDigiSample();
                sub_8F1FE(t);
                RestoreDialogBg();
            }
            sounddev = dev;
            byte_D2439 = byte_D2439 ? s.h2 : h1;
            musicslot = s.h2;
            setbits[0] = sounddev;
            MakePath(s.path, 0, (char *)str_NHL3, (char *)str_CFG);
            fp = fopen(s.path, (char *)unk_C36AC);
            if (!fp) FatalError((char *)str_CantOpenNhlCfg);
            i = 0;
            while (dev != sounddevids[i]) i++;
            fprintf(fp, (char *)asc_C36C3, i);
            fclose(fp);
            if (sounddev == 2 || sounddev == 0x20) {
                musicon = 1;
                musichandle = dword_D242B;
            } else {
                musicon = 0;
            }
        } else {
            if (dev != 0x10 && dev != 1) {
                switch (dev) {
                case 2:
                    sndcardname = (int)str_SoundBlaster2;
                    break;
                case 0x20:
                    sndcardname = (int)str_ULTRASOUND;
                    break;
                case 4:
                    sndcardname = (int)str_Adlib;
                    break;
                case 8:
                    sndcardname = (int)str_MT323;
                    break;
                }
                s.ev = 0;
                sub_B2DCA(&s.btn, &s.x, &s.y);
                MessageBox(-1, -1, (char *)unk_D249C, 3, 0, 0, (int)&s.x, (int)&s.y, 0);
                do {
                    if ((e = GetInputEvent()) != 0)
                        s.ev = ((int (*)(unsigned char *, int *, int *))ptrupdatefn)(e, &s.x, &s.y);
                } while (!(s.ev & 2));
                RestoreDialogBg();
                dev = 0x10;
            } else {
                dword_D2423 = -1;
            }
            sub_8EB93();
            SetTickDeadline(0x64);
            WaitTickDeadline();
        }
    } while (!dword_D2423);
    if (sounddev == 2 || sounddev == 0x20) {
        dword_ED7A8 = sub_8CCA8((char *)str_Emmcopybuf, 0x400, 0x20);
        InitSpeech(musicslot, dword_C4CFC, dword_ED7A8, 0x400);
    }
    dword_CCC94 = 0;
    return 1;
}
