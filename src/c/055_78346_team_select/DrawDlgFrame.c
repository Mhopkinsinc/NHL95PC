/* Team select: the dialog frame. */
/* DRAFT: 579 vs 578, the bevel offsets get other registers / slots; see docs/UNSURE.md. */
#define DrawDlgFrame DrawDlgFrame_decl
#include "nhl95.h"
#undef DrawDlgFrame

/* DrawDlgFrame (78BE7, PC only; ends in StubRet4a, DrawLineEditorScreen's exit): draw a dialog frame from x1 y1
   to x2 y2: the fill (dlgfillcol), the top and left edges in dlgedgecol1, the right and bottom edges in
   dlgedgecol2, the two other corners in dlgedgecol3; with bevel a 2 x 2 pixel bevel inside each corner
   (dlgbevelcol1 / 2). No result. */
void DrawDlgFrame(int x1, int y1, int x2, int y2, int bevel)
{
    sub_90D20(x1, y1, x2 - x1 + 1, y2 - y1 + 1, dlgfillcol);
    sub_B4FAC(x1, y1, x2 - 1, y1, dlgedgecol1);
    sub_B4FAC(x1, y1, x1, y2 - 1, dlgedgecol1);
    sub_B4FAC(x2, y1 + 1, x2, y2, dlgedgecol2);
    sub_B4FAC(x1 + 1, y2, x2, y2, dlgedgecol2);
    sub_B5DB0(x1, y2, dlgedgecol3);
    sub_B5DB0(x2, y1, dlgedgecol3);
    if (bevel) {
        sub_B5DB0(x1 + 2, y1 + 2, dlgbevelcol2);
        sub_B5DB0(x1 + 3, y1 + 3, dlgbevelcol2);
        sub_B5DB0(x1 + 3, y1 + 2, dlgbevelcol1);
        sub_B5DB0(x1 + 2, y1 + 3, dlgbevelcol1);
        sub_B5DB0(x1 + 2, y2 - 2, dlgbevelcol1);
        sub_B5DB0(x1 + 3, y2 - 3, dlgbevelcol1);
        sub_B5DB0(x1 + 3, y2 - 2, dlgbevelcol2);
        sub_B5DB0(x1 + 2, y2 - 3, dlgbevelcol2);
        sub_B5DB0(x2 - 3, y1 + 2, dlgbevelcol2);
        sub_B5DB0(x2 - 2, y1 + 3, dlgbevelcol2);
        sub_B5DB0(x2 - 2, y1 + 2, dlgbevelcol1);
        sub_B5DB0(x2 - 3, y1 + 3, dlgbevelcol1);
        sub_B5DB0(x2 - 3, y2 - 2, dlgbevelcol1);
        sub_B5DB0(x2 - 2, y2 - 3, dlgbevelcol1);
        sub_B5DB0(x2 - 2, y2 - 2, dlgbevelcol2);
        sub_B5DB0(x2 - 3, y2 - 3, dlgbevelcol2);
    }
}
