/* File utils: per-frame redraw of the top panel. */
#include "nhl95.h"

/* UpdateHudPanel: on the screen bitmap, tick the panel clock and both penalty lists (unless the
 clock was already stopped, or a replay is up), refresh both teams' line energies, and draw the
 clock, the two energy bars (energy / 500) and either the line-change bars or the penalty list.
 A side whose line box is on, whose bars were forced, or that is in replay draws the line bars
 and clears the "penalties showing" flag. A side with a negative penalty slot forces the bars
 once. Otherwise the panel shape is drawn the first time and the penalty list follows. */
void UpdateHudPanel(void)
{
    int n;

    SelectScreenBM();
    if (hudclockmin < 0) return;
    n = dword_DC28C;
    dword_C583C = n;
    dword_DC28C = 0;
    if (TickPanelClock(n) && (gmode & 0x10) == 0) {
        TickPanelPenalties(hudpenhome, dword_C583C);
        TickPanelPenalties(hudpenaway, dword_C583C);
    }
    GetLineEnergies(0, dword_C56E4);
    GetLineEnergies(1, dword_C56C4);
    DrawPanelClockDigits();
    DrawEnergyBar(dword_C56E4[hudhomeline] / 500, 0x2B);
    DrawEnergyBar(dword_C56C4[hudawayline] / 500, 0xFC);
    if (lcboxon[0] || dword_C5844 || (gmode & 0x10)) {
        DrawLineGroupBars(&dword_C585C, (int *)hmlineind, hudhomeline, dword_C56E4, 0x4A);
        dword_C5854 = 0;
    } else if (hudpenhome[0] < 0) {
        dword_C5844 = 1;
        DrawLineGroupBars(&dword_C585C, (int *)hmlineind, hudhomeline, dword_C56E4, 0x4A);
        dword_C5854 = 0;
    } else {
        if (!dword_C5854) {
            sub_B4CF2(hmpanelspr);
            dword_C5854 = 1;
        }
        DrawPanelPenalties(hudpenhome, 0x48);
        dword_C585C = 0;
    }
    if (lcboxon[1] || dword_C5848 || (gmode & 0x10)) {
        DrawLineGroupBars(&dword_C5860, (int *)awlineind, hudawayline, dword_C56C4, 0xC1);
        dword_C5858 = 0;
    } else if (hudpenaway[0] < 0) {
        dword_C5848 = 1;
        DrawLineGroupBars(&dword_C5860, (int *)awlineind, hudawayline, dword_C56C4, 0xC1);
        dword_C5858 = 0;
    } else {
        if (!dword_C5858) {
            sub_B4CF2(awpanelspr);
            dword_C5858 = 1;
        }
        DrawPanelPenalties(hudpenaway, 0xBF);
        dword_C5860 = 0;
    }
}
