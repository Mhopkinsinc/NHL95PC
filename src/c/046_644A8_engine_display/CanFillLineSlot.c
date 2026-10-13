/* Engine display: whether a roster player can take a line slot. */
#define CanFillLineSlot CanFillLineSlot_decl
#include "nhl95.h"
#undef CanFillLineSlot

/* CanFillLineSlot: side 0 is hmlinetab, anything else awlinetab. The roster byte at
 side*444h + player*39 selects a position class in byte_CD418. Class 0 cannot play.
 Class 2 can play only when byte_E9E18[side*25 + player] is 2. Forwards (slots 0-0Bh)
 may already occupy one other slot of a different line-trio third, unless word_E9F12 is
 above 6. Defence pairs (0Ch-11h) are the same with word_E9F14 and a limit of one.
 Slots 12h-1Bh and 1Ch-23h then scan a window that skips the slot itself and its pair
 (slot+5 or slot+4). Returns 0 when the player is already in that window, else 1.
 The position byte read for the window choice is byte_DB3AE. */
int CanFillLineSlot(short side, short slot, short player)
{
    signed char *tab;
    int sidev;
    short st;
    int pos;
    int result;

    tab = (signed char *)(side ? awlinetab : hmlinetab);
    sidev = (short)side;
    {
        unsigned char *row;

        row = hmroster + sidev * 0x444;
        st = byte_CD418[row[(short)player * 39]];
    }
    if (st == 0)
        goto zero;
    if (st == 2 && byte_E9E18[sidev * 25 + (short)player] != 2)
        goto zero;
    pos = byte_DB3AE[(short)side * 0x444 + (short)player * 39];
    {
    short i;
    short n;
    short from;
    short to;

    if (slot < 0x0C) {
        n = 0;
        for (i = 0; i < 0x0C; i++) {
            if (i % 3 != slot % 3 || i < slot) {
                if ((short)tab[i] == player)
                    n++;
            }
        }
        if (n >= 2 && word_E9F12 > 6)
            goto zero;
        from = (slot / 3) * 3;
        to = from + 2;
    } else if (slot < 0x12) {
        if (word_E9F14 >= 3) {
            n = 0;
            for (i = 0x0C; i < 0x12; i++) {
                if (i % 2 != slot % 2 || i < slot) {
                    if ((short)tab[i] == player)
                        n++;
                }
            }
            if (n >= 2)
                goto zero;
        }
        from = slot & ~1;
        to = from + 1;
    } else if (slot < 0x1C) {
        if ((pos == 0x44 && word_E9F14 < 4) || (pos != 0x44 && word_E9F12 < 6)) {
            from = slot < 0x17 ? 0x12 : 0x17;
            to = from + 4;
        } else {
            from = 0x12;
            to = 0x1B;
        }
    } else if (pos == 0x44 && word_E9F14 < 4) {
        from = slot < 0x20 ? 0x1C : 0x20;
        to = from + 3;
    } else {
        from = 0x1C;
        to = 0x23;
    }
    for (i = from; i <= to; i++) {
        if (i == slot)
            continue;
        if (slot >= 0x1C) {
            if (i == slot + 4)
                continue;
        } else if (slot >= 0x12) {
            if (i == slot + 5)
                continue;
        }
        if ((short)tab[i] == player) {
            result = (short)tab[i] ^ player;
            goto out;
        }
    }
    result = 1;
    goto out;
    }
zero:
    result = 0;
out:
    return result;
}
