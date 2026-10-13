/* Computer line change. */
#include "nhl95.h"

int LineEnergy(Team *t, int line);
#pragma aux LineEnergy "getlinee" parm [eax] [edx] value [eax] modify [eax edx];

/* CompLine: other is the other team, team is the one changing lines. When the on-ice counts differ, pick
 line 4 or 6, and the next line if its energy is higher and this one is under F33h. Otherwise walk the
 byte list off_CBD2E[team byte D5h] at the current line, four entries, and keep a line whose energy is at
 most the dword at D6h and better than the line already set. A line under 4 bumps the rotation count.
 Then draw that line and clear bit 6 of the byte at D4h. */
void CompLine(Team *other, Team *team)
{
    Team *o;
    short line;
    short diff;

    o = other;
    diff = team->tmap - o->tmap;
    if (diff != 0) {
        int e1;

        line = 4;
        if (diff < 0) line = 6;
        e1 = LineEnergy(team, line);
        if ((short)e1 < 0xF33) {
            int e2;

            e2 = LineEnergy(team, line + 1);
            if ((short)e1 < (short)e2) line++;
        }
        team->tmline = line;
    } else {
        unsigned char flags;
        signed char *p;
        int best;
        short saved;
        short b;
        short i;

        flags = ((unsigned char *)team)[0xD4];
        if (flags & 1) {
            line = team->tmline;
            if ((gmode & gmclock) && line < 4 && (flags & 0x40) == 0) {
                unsigned lim;

                AvgCline(team);
                if (team->tmap == o->tmap)
                    lim = *(unsigned *)((char *)team + 0xD6);
                else
                    lim = 0xF33;
                if ((unsigned short)regd0.w > lim) return;
            }
            if (line > 3) line = 3;
            if (((unsigned char *)team)[0xD5] == 6 && (line < 4 || line > 5)) line = 3;
        } else {
            line = o->tmline;
            if (line >= 6) line -= 6;
            else if (line >= 4) line -= 4;
        }
        if (((unsigned char *)team)[0xD5] == 1 && (((unsigned char *)team)[0xD4] & 0x80))
            line += 4;
        p = (signed char *)off_CBD2E[((signed char *)team)[0xD5]] + line * 4;
        b = -1;
        saved = team->tmline;
        best = LineEnergy(team, saved);
        for (i = 0; i < 4; i++) {
            short e;

            b = *p++;
            if (b < 0) break;
            e = LineEnergy(team, b);
            if ((unsigned)e > *(unsigned *)((char *)team + 0xD6)) break;
            if (e > (short)best) {
                best = e;
                saved = b;
            }
            b = -1;
        }
        if (b < 0) line = saved;
        else line = b;
        if (line != team->tmline) {
            short n;

            team->tmline = line;
            if (((unsigned char *)team)[0xD5] == 1 && line == 2)
                ((unsigned char *)team)[0xD4] ^= 0x80;
            if (line < 4) {
                n = team->tmlcnt;
                n++;
                team->tmlcnt = n;
                if (n >= 3) team->tmlcnt = n - 3;
            }
        }
    }
    DrawPanelLine(team == &awtmstruct, line);
    ((unsigned char *)team)[0xD4] &= ~0x40;
}
