/* Engine core: raise the coach lines as the score and the clock change.
 DRAFT: 818 vs 818 (best of 4). Every instruction matches except the duplicated
 line count, mov bh,3 / stores from bh where the original is mov cl,3. A named
 unsigned char, a chained assignment, a reload of the just-stored byte, a block
 scope and ~(1) stay on bh. An int or a short for that 3 is mov eax,3 and shifts
 the period-2 byte temps. Naming the period-2 decrement stores in place (dec bl)
 and drops two bytes. Do not retry those. */
#define SetCoachMode SetCoachMode_decl
#include "nhl95.h"
#undef SetCoachMode

short SetCoachMode(int side);
#pragma aux SetCoachMode parm [eax] value [ax] modify [eax ecx];

/* UpdateCoachModes: while the game is running, set each coach from the score
 and, late in the period, push the lines toward a lead or a deficit. diff is
 the home score minus the away score and stays in dx across SetCoachMode.
 Period 2 only adjusts at 10:00. Period 3 walks the clock down and shares the
 line setups. */
void UpdateCoachModes(void)
{
    short diff;
    unsigned char prev;

    if (gmode & 0x10)
        return;
    diff = hmscore[0];
    diff -= awscore;
    if (curperiod == 2 && gameclock <= 0x258) {
        prev = byte_DF6E9;
        SetCoachMode(0);
        if (prev != byte_DF6E9)
            byte_DF6E8 |= 0x40;
        prev = byte_DF7E9;
        SetCoachMode(1);
        if (prev != byte_DF7E9)
            byte_DF7E8 |= 0x40;
        if (gameclock != 0x258)
            return;
        if ((int)diff < -1) {
            if (byte_DF6E6 < 4)
                byte_DF6E6++;
            if (byte_DF6E7 > 1)
                byte_DF6E7--;
        }
        if (diff > 1) {
            if (byte_DF7E6 < 4)
                byte_DF7E6++;
            if (byte_DF7E7 > 1)
                byte_DF7E7--;
        }
        return;
    }
    if (curperiod != 3)
        return;
    prev = byte_DF6E9;
    if (gameclock > 0x3C)
        goto h8;
    if ((int)diff > -1)
        goto h8;
h7:
    dword_DF6EA = 0xB33;
    byte_DF6E8 |= 1;
    byte_DF6E9 = 6;
    goto h11;
h8:
    if (gameclock > 0x78)
        goto h9;
    if ((int)diff < -1)
        goto h7;
h9:
    if (gameclock > 0xB4)
        goto h12;
    if ((int)diff > -1)
        goto h12;
h10:
    dword_DF6EA = 0xB33;
    byte_DF6E8 |= 1;
    byte_DF6E9 = 7;
h11:
    byte_DF6E6 = 4;
    byte_DF6E7 = 1;
    goto h18;
h12:
    if (gameclock > 0x12C)
        goto h13;
    if ((int)diff < -1)
        goto h10;
h13:
    if (gameclock > 0x258)
        goto h17;
    if ((int)diff < -1) {
        dword_DF6EA = 0xB33;
        byte_DF6E8 |= 1;
        byte_DF6E9 = 0x0A;
        goto h11;
    }
    if (diff > 2) {
        dword_DF6EA = 0xCCC;
        goto h16;
    }
    dword_DF6EA = 0xD9A;
h16:
    byte_DF6E8 &= 0xFE;
    byte_DF6E9 = 3;
    byte_DF6E6 = 3;
    byte_DF6E7 = 2;
    goto h18;
h17:
    SetCoachMode(0);
h18:
    if (prev != byte_DF6E9)
        byte_DF6E8 |= 0x40;
    prev = byte_DF7E9;
    if (gameclock > 0x3C)
        goto a21;
    if (diff < 1)
        goto a21;
a20:
    dword_DF7EA = 0xB33;
    byte_DF7E8 |= 1;
    byte_DF7E9 = 6;
    goto a24;
a21:
    if (gameclock > 0x78)
        goto a22;
    if (diff > 1)
        goto a20;
a22:
    if (gameclock > 0xB4)
        goto a25;
    if (diff <= 1)
        goto a25;
a23:
    dword_DF7EA = 0xB33;
    byte_DF7E8 |= 1;
    byte_DF7E9 = 7;
a24:
    byte_DF7E6 = 4;
    byte_DF7E7 = 1;
    goto a31;
a25:
    if (gameclock > 0x12C)
        goto a26;
    if (diff > 1)
        goto a23;
a26:
    if (gameclock > 0x258)
        goto a30;
    if (diff > 1) {
        dword_DF7EA = 0xB33;
        byte_DF7E8 |= 1;
        byte_DF7E9 = 0x0A;
        goto a24;
    }
    if ((int)diff < -2) {
        dword_DF7EA = 0xCCC;
        byte_DF7E9 = 9;
        goto a29;
    }
    dword_DF7EA = 0xD9A;
    byte_DF7E9 = 8;
a29:
    byte_DF7E8 |= 1;
    byte_DF7E6 = 3;
    byte_DF7E7 = 2;
    goto a31;
a30:
    SetCoachMode(1);
a31:
    if (prev != byte_DF7E9)
        byte_DF7E8 |= 0x40;
}
