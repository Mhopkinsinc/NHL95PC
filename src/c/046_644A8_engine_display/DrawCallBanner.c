/* Engine display: the call banner. */
/* DRAFT: 523 vs 514 bytes, first pass; see docs/UNSURE.md. Its match belongs in checkwindow.c, since it ends in
   checkwindow_popebp. */
#define DrawCallBanner DrawCallBanner_decl
#define sub_6AB7C sub_6AB7C_decl
#include "nhl95.h"
#undef DrawCallBanner
#undef sub_6AB7C

void sub_6AB7C(int n, int x0, int y0, int x1, int y1);

/* DrawCallBanner (66FE2, PC only; ends in checkwindow_popebp): draw call banner n (bannerstrs[n], in the SCOR3
   font) near the bottom left of the view (scrollx + 0Ch, scrolly + 95h, moved into the clip window): a dark box
   (colour 10h, dithered with 9) as wide as the text plus 4 and 9 high with a black border of 4, the text in
   colour 25h, then again in the SCOR2 font in colour 27h, and record the box as a drawn sprite (sub_6AB7C,
   spritedrawcount). n: the banner. No result. */
void DrawCallBanner(short n)
{
    short y2;
    short y;
    short x2;
    short d;
    short x;
    short w;

    x = scrollx + 0xC;
    y = scrolly + 0x95;
    y2 = scrolly + 0x9E;
    sub_8EA18(scor3font);
    w = fputchar((char *)bannerstrs[n]) + 4;
    x2 = w + x;
    d = cliprect_x0 - scrollx;
    x += d;
    x2 += d;
    d = cliprect_y0 - scrolly;
    y += d;
    y2 += d;
    w += 8;
    sub_90D20(x, y, x2 - x, y2 - y, 0x10);
    DitherRect(x, y, x2 - x, y2 - y, 9);
    sub_90D20(x - 4, y - 4, w, 4, 0);
    sub_90D20(x - 4, y, 4, 9, 0);
    sub_90D20(x2, y, 4, 9, 0);
    sub_90D20(x - 4, y2, w, 4, 0);
    sub_8E9C0(0x25, 0xFF);
    y2 = y + 1;
    sub_91964((char *)bannerstrs[n], x + 2, y2);
    sub_8EA18(scor2font);
    sub_8E9C0(0x27, 0xFF);
    sub_91964((char *)bannerstrs[n], x + 2, y2);
    x -= 4;
    y -= 4;
    sub_6AB7C(spritedrawcount++, x, y, x + w, y + 0x11);
}
