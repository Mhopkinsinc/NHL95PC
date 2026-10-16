/* Misc dialogs: text edit cursor. */
/* DRAFT: all but the final call matches; the EXE has x y w in eax edi esi for the sub_91044 pushes (C picks other registers). */
#include "nhl95.h"

#define TEXTWIDTHN ((int (__cdecl *)(char *s, int n))sub_90A40)
#define FILLRECT ((void (__cdecl *)(int x, int y, int w, int h, int col))sub_91044)

/* DrawEditCursor: draw the cursor of the text edit field (when editcuron): clamp editpos to the text, then a
 bar the width of the character under it (a space at the end; the width of "." in fixed-pitch mode, mode bit 2)
 and editcurw high in fontcolor, under the text at editx + the width up to editpos. */
void DrawEditCursor(int mode)
{
    int cw;
    int w;
    int x;
    int y;
    int h;
    char *s;

    cw = fputchar((char *)str_Dot);
    if (!editcuron) return;
    if ((int)strlen((char *)editbuf) < editpos) editpos = strlen((char *)editbuf);
    s = (char *)editbuf + editpos;
    if (*s) {
        if (mode & 2) w = cw;
        else w = TEXTWIDTHN(s, 1);
    } else w = fputchar((char *)str_Space);
    if (mode & 2) x = editpos * cw;
    else x = TEXTWIDTHN((char *)editbuf, editpos);
    x += editx;
    h = editcurw;
    y = byte_D42C5 + edity - h - 2;
    FILLRECT(x, y, w, h, fontcolor);
}

#define CLEARRECT ((void (__cdecl *)(int x, int y, int w, int h, int col))sub_90EC0)

/* DrawEditText (31599, PC only; ends in DrawEditCursor's exits): draw the text of the edit field. With a field
   width (editmaxw), first cut editbuf from the end until it fits (in fixed-pitch mode, mode bit 2, by the width of
   "." per character, else by fputchar); clamp editpos to the text; print it at editx, edity (fixed-pitch: a "."
   per character); then clear the rest of the field (sub_90EC0, byte_D42C5 - 2 high, colour dword_D42AC).
   mode: the edit mode bits. No result.
   DRAFT: 369 vs 372. The width of "." stays in edi (original ebp, with the text length in edi); see UNSURE. */
void DrawEditText(int mode)
{
    int cw;
    int len;
    int i;
    int w;

    cw = fputchar((char *)str_Dot);
    if (editmaxw) {
        if (mode & 2) {
            while ((int)strlen((char *)editbuf) * cw > editmaxw && strlen((char *)editbuf))
                ((char *)editbuf)[strlen((char *)editbuf) - 1] = 0;
        } else {
            while (fputchar((char *)editbuf) > editmaxw && strlen((char *)editbuf))
                ((char *)editbuf)[strlen((char *)editbuf) - 1] = 0;
        }
    }
    len = strlen((char *)editbuf);
    if (len < editpos) editpos = len;
    if (mode & 2) {
        for (i = 0; i < len; i++) sub_92CD0((char *)str_Dot, editx + cw * i, edity);
    } else {
        sub_92CD0((char *)editbuf, editx, edity);
    }
    if (!editmaxw) return;
    if (mode & 2) w = len * cw;
    else w = fputchar((char *)editbuf);
    if (editmaxw - w <= 0) return;
    CLEARRECT(editx + w, edity, editmaxw - w, byte_D42C5 - 2, dword_D42AC);
}
