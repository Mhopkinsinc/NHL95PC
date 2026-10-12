/* Line editor: clip a rectangle before the page blit. */
#include "nhl95.h"

/* BlitClipRect: clip x, y, w, h to the blit clip box. When the rectangle misses the box, return.
 Otherwise set the draw target (sub_B4BA8) and copy the clipped rectangle between VGA pages.
 Draft: the adds match; x is saved in esi and y in edi (the original saves x in edi and y in esi). */
void BlitClipRect(int x, int y, int w, int h)
{
    int x2;
    int y2;

    y2 = y;
    x2 = x;
    x2 += w;
    y2 += h;
    if (x >= blitclipright) return;
    if (y >= blitclipbottom) return;
    if (x2 < blitclipleft) return;
    if (y2 < blitcliptop) return;
    if (x < blitclipleft) x = blitclipleft;
    if (y < blitcliptop) y = blitcliptop;
    if (x2 > blitclipright) x2 = blitclipright;
    if (y2 > blitclipbottom) y2 = blitclipbottom;
    w = x2 - x;
    h = y2 - y;
    sub_B4BA8();
    stub_8C218(vgapage == 0, x, y, w, h, 0);
}
