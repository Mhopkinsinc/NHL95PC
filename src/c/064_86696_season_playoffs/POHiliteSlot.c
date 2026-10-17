/* Season playoffs: playoff tree slot highlight. */
#include "nhl95.h"

/* POHiliteSlot (89B69, PC only; ends in POCreateSchedule_x): highlight slot n of the playoff tree: a double frame
   (sub_92F50, colour 80h) 74h wide at the slot position (dword_D2B70 / dword_D2BEC), 1Fh high for the 28
   first-round slots and 3Ch for the later ones, with a second frame one pixel outside it. n: the slot.
   unused: a value the callers pass and it ignores. No result. */
void POHiliteSlot(int n, int unused)
{
    int x2;
    int y2;
    int x;
    int y;

    x = dword_D2B70[n];
    y = dword_D2BEC[n];
    x2 = x + 0x74;
    if (n < 0x1C) y2 = y + 0x1F;
    else y2 = y + 0x3C;
    sub_92F50(x, y, x2, y2, 0x80);
    sub_92F50(x - 1, --y, ++x2, ++y2, 0x80);
}
