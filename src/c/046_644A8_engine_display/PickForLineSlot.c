/* Engine display: picking a player for a line slot. */
#define PickForLineSlot PickForLineSlot_decl
#define CanFillLineSlot CanFillLineSlot_decl
#include "nhl95.h"
#undef PickForLineSlot
#undef CanFillLineSlot

short CanFillLineSlot(int side, int slot, int player);

/* DRAFT: 1073 vs 1142, 305 of 341 instructions; see docs/UNSURE.md. Owner of the lines_popx2 tail
   (PickExtraSkater). */
/* PickForLineSlot (64E60, PC only): choose a player for line slot slot of team side (home hmlinetab, away
   awlinetab). First the players of the slots after slot in its byte_CD421 group (from byte_CD473[slot], up to the
   -1 end) that CanFillLineSlot allows. Else by the position letter of the slot's current player (byte_DB3AE)
   the first FillDressedSlots result from the side's 25-byte lists: D the E9D50 list for slots 12h-1Bh, else
   E9DB4; C the E9CEC, E9DE6 and E9EAE lists, R E9EAE, E9DE6, E9CEC, L E9DE6, E9EAE, E9CEC (from slot 1Ch on
   E9D82, E9E7C and E9D1E instead). Else the group's players from its start up to slot, then the first player
   of two more lists (by the letter and slot: E9D50 / E9DB4 and E9E4A / E9EE0, in either order) that
   CanFillLineSlot allows. Result: the player (a short), or 0. */
short PickForLineSlot(short side, short slot)
{
    short pos;
    short start;
    signed char *tab;
    short i;
    short p;
    short r;
    signed char *list;

    tab = (signed char *)(side ? awlinetab : hmlinetab);
    i = start = byte_CD473[slot];
    while (byte_CD421[i++] != slot);
    while (byte_CD421[i] >= 0) {
        p = tab[byte_CD421[i]];
        if (CanFillLineSlot(side, slot, tab[byte_CD421[i]])) return p;
        i++;
    }
    pos = byte_DB3AE[side * 0x444 + tab[slot] * 39];
    if (pos == 'D') {
        p = FillDressedSlots(slot < 0x1C && slot >= 0x12 ? (signed char *)unk_E9D50 + side * 25 : byte_E9DB4 + side * 25, side, slot);
        if (p >= 0) return p;
    } else if (pos == 'C') {
        p = FillDressedSlots(slot < 0x1C ? (signed char *)unk_E9CEC + side * 25 : (signed char *)unk_E9D82 + side * 25, side, slot);
        if (p >= 0) return p;
        p = FillDressedSlots(slot < 0x1C ? (signed char *)unk_E9DE6 + side * 25 : (signed char *)unk_E9E7C + side * 25, side, slot);
        if (p >= 0) return p;
        p = FillDressedSlots(slot < 0x1C ? (signed char *)unk_E9EAE + side * 25 : (signed char *)unk_E9D1E + side * 25, side, slot);
        if (p >= 0) return p;
    } else if (pos == 'R') {
        p = FillDressedSlots(slot < 0x1C ? (signed char *)unk_E9EAE + side * 25 : (signed char *)unk_E9D1E + side * 25, side, slot);
        if (p >= 0) return p;
        p = FillDressedSlots(slot < 0x1C ? (signed char *)unk_E9DE6 + side * 25 : (signed char *)unk_E9E7C + side * 25, side, slot);
        if (p >= 0) return p;
        p = FillDressedSlots(slot < 0x1C ? (signed char *)unk_E9CEC + side * 25 : (signed char *)unk_E9D82 + side * 25, side, slot);
        if (p >= 0) return p;
    } else if (pos == 'L') {
        p = FillDressedSlots(slot < 0x1C ? (signed char *)unk_E9DE6 + side * 25 : (signed char *)unk_E9E7C + side * 25, side, slot);
        if (p >= 0) return p;
        p = FillDressedSlots(slot < 0x1C ? (signed char *)unk_E9EAE + side * 25 : (signed char *)unk_E9D1E + side * 25, side, slot);
        if (p >= 0) return p;
        p = FillDressedSlots(slot < 0x1C ? (signed char *)unk_E9CEC + side * 25 : (signed char *)unk_E9D82 + side * 25, side, slot);
        if (p >= 0) return p;
    }
    i = start;
    while (byte_CD421[i] != slot) {
        p = tab[byte_CD421[i]];
        if (CanFillLineSlot(side, slot, tab[byte_CD421[i]])) return p;
        i++;
    }
    if (pos == 'D')
        list = slot >= 0x12 && slot < 0x1C ? (signed char *)unk_E9D50 + side * 25 : byte_E9DB4 + side * 25;
    else
        list = slot < 0x1C ? byte_E9E4A + side * 25 : (signed char *)unk_E9EE0 + side * 25;
    i = 0;
    for (; i < 0x19; i++) {
        p = list[i];
        if (p < 0) break;
        if (CanFillLineSlot(side, slot, p)) return p;
    }
    if (pos == 'D')
        list = slot >= 0x12 && slot < 0x1C ? byte_E9E4A + side * 25 : (signed char *)unk_E9EE0 + side * 25;
    else
        list = slot < 0x1C ? (signed char *)unk_E9D50 + side * 25 : byte_E9DB4 + side * 25;
    i = 0;
    for (; i < 0x19; i++) {
        p = list[i];
        if (p < 0) break;
        if (CanFillLineSlot(side, slot, p)) return p;
    }
    return 0;
}
