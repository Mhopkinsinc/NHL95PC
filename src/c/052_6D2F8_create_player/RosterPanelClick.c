/* Create player: roster list click. */
#include "nhl95.h"

#define INVERT ((void (__cdecl *)(int x, int y, int w, int h, int col))sub_90F38)

/* RosterPanelClick: x in the left list (23h..113h) is side 0, x in the right list (15Eh..24Eh)
 is side 1. A y in the 28 rows under 35h, each byte_D42C3 tall, toggles that row. A free-agent
 list toggles falistsel[scroll + row] when the row is on screen. A team list toggles
 rostersel[side * 28 + row] when that rosterlist entry is filled. Then, if the list has a
 selection, point the move-to-free-agents item at MoveToFreeAgents when the other pool has
 room, and the move-to-the-other-roster item at MoveToOtherRoster when the other side is not
 itself the free-agent list. An empty selection clears both items. */
void RosterPanelClick(int x, int y)
{
    int side;
    int h;
    int row;
    int rowy;

    side = -1;
    if (x >= 0x23 && x <= 0x113) side = 0;
    else if (x >= 0x15E && x <= 0x24E) side = 1;
    if (side < 0) return;
    if (y >= 0x35) {
    h = byte_D42C3;
    if (y < h * 28 + 0x35) {
        row = (y - 0x35) / h;
        rowy = row * h + 0x35;
        if (rosterisfa[side] == 1) {
            if (dword_EBCA4 + row < facount) {
                unsigned char *fa;

                INVERT(side ? 0x15E : 0x23, rowy, 0xF0, h, 0x80);
                row += dword_EBCA4;
                fa = (unsigned char *)falistsel;
                fa[row] = ~fa[row];
            }
        } else {
            int base;

            base = side * 756;
            if (rosterlist[base + row * 27]) {
                INVERT(side ? 0x15E : 0x23, rowy, 0xF0, h, 0x80);
                rostersel[side * 28 + row] = ~rostersel[side * 28 + row];
            }
        }
    }
    }
    if (CountSelected(rosterisfa[side] == 1 ? (char *)falistsel : (char *)&rostersel[side * 28]) > 0) {
        if (side == 0) {
            if (rosterisfa[0] != 1 && facount < 0x1E)
                *(void (**)(int *, int *, int *))&menu_r1_tofa = MoveToFreeAgents;
            if (rosterisfa[1] != 1)
                *(void (**)())&menu_r1_toroster2 = MoveToOtherRoster;
        } else {
            if (rosterisfa[1] != 1 && facount < 0x1E)
                *(void (**)(int *, int *, int *))&menu_r2_tofa = MoveToFreeAgents;
            if (rosterisfa[0] != 1)
                *(void (**)())&menu_r2_toroster1 = MoveToOtherRoster;
        }
    } else if (side == 0) {
        menu_r1_tofa = side;
        menu_r1_toroster2 = side;
    } else {
        menu_r2_tofa = 0;
        menu_r2_toroster1 = 0;
    }
}
