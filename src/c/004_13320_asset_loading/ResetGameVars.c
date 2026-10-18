/* ResetGameVars: clear the in-game clocks, line-change slots, penalty and banner state, then
   start the period. Each photo-bank crowd slot gets a fresh timer. A keyboard pad recentres
   the mouse. */
#define ResetGameVars ResetGameVars_decl
#include "nhl95.h"
#undef ResetGameVars

void __STOSB(void *p, int v, int n);
#pragma aux __STOSB "__STOSB" parm [eax] [edx] [ecx];
#pragma aux randomd0 modify [eax ecx];

void ResetGameVars(void)
{
    unsigned char ff;
    short neg;
    short z;

    ff = 0xFF;
    byte_E9AC8 = ff;
    byte_E9AD3 = ff;
    dword_CC0AC = 0;
    byte_E024F = 0;
    *(unsigned char *)&word_E024E = 0;
    byte_E024D[0] = 0;
    *(unsigned char *)byte_E024C = 0;
    neg = -1;
    ovltimer = neg;
    word_C90D8 = neg;
    RefStep[0] = neg;
    Pencntdwn = neg;
    word_CC0B0[0] = neg;
    z = 0;
    threat = z;
    passplayer = z;
    passspeed = z;
    passdir = z;
    lastplayer = z;
    shotplayer = z;
    word_CBC44 = z;
    RefPen = z;
    refsignal = z;
    Penaltytimer = z;
    lcblinktime[1] = z;
    lcblinktime[0] = z;
    lcblink[1] = z;
    lcblink[0] = z;
    energywarn[1] = z;
    energywarn[0] = z;
    lcreqchoice[1] = z;
    lcreqchoice[0] = z;
    lcrequest[1] = z;
    lcrequest[0] = z;
    lcboxon[1] = z;
    lcboxon[0] = z;
    lctimer[1] = z;
    lctimer[0] = z;
    lcline[1] = z;
    lcline[0] = z;
    word_CBC60[0] = z;
    lcsel[0] = z;
    savedmousey = 0;
    savedmousex = 0;
    bannertimer = z;
    word_CBEC6 = z;
    word_CBEC2 = z;
    bannermsg = neg;
    ovlstep = neg;
    ovlseq = neg;
    textline5 = 0;
    textline4 = 0;
    textline2 = 0;
    textline1 = 0;
    textline3 = 0;
    {
        int s;
        int a;

        s = 0;
        penshotmode = 0;
        penshotlive = 0;
        penshotstart = 0;
        a = 0;
        shotontarget = 0;
        shotongoal = 0;
        penshotplayer = -1;
        a ^= s;
        CwdExciteLvl = a;
    }
    {
        int i;
        char *p;

        i = 0;
        do {
            p = (char *)word_DEE94 + i * 12;
            *(short *)p = -1;
            *(p + 2) = randomd0(0x28) + 0x3C;
            i++;
        } while (i < 20);
    }
    __STOSB(word_E9B2C, 0, 0x20);
    dword_E9A9E = 0;
    word_E9AAE = 0;
    dword_E9AB6 = 0;
    word_E9AA6 = 0;
    word_E9AA4 = 0;
    word_E9AA2 = 0;
    word_E9AAA = 0;
    dword_E009C = 0;
    word_E9A9C = -1;
    StartGame();
    curperiod = 1;
    if (pad1dev[0] == 1 || pad2dev == 1)
        MouseSetPos(0xA0, 0x64);
    deferpending = 0;
    inputframes = 0;
    exitgame = 0;
    gameover = 0;
    hudhundaccum = 0;
    frameaccum = 0;
    joysampling = 0;
    joysampling_save = 0;
    joyq_flush();
}
