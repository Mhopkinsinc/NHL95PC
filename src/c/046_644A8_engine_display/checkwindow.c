/* Engine display: the camera window and the overlay text that ends in its pops. */
#define LoadTeamPPV LoadTeamPPV_decl
#define UpdateTextOverlay UpdateTextOverlay_decl
#define sub_90354 sub_90354_decl
#include "nhl95.h"
#undef LoadTeamPPV
#undef UpdateTextOverlay
#undef sub_90354

int __cdecl sub_90354(int bank);

#define ABSW(v) ((v) < 0 ? -(v) : (v))

/* DRAFT: 1950 vs 1942, a first pass kept for its pop / ret tail (checkwindow_popebp / checkwindow_popx, which
   LoadTeamPPV, UpdateTextOverlay, DrawCallBanner and DrawTextOverlay use); see docs/UNSURE.md. */
/* checkwindow (65D01, 93G checkwindow): move the camera (camx / camy) toward the point it follows (xc1 / yc1,
   kept in regd3 / regd2). In highlight mode (gmode bit 7) that is a fixed point (-100, 15) with a slow pan. While
   the clock runs: during a penalty count-down or a referee step, follow the referee (object 10h) toward its
   target (temp3 / temp4) and lock onto the faceoff dot (fox / foy) when it is skating to it and close; otherwise
   follow the puck carrier (or the puck) plus the lead yleader, which grows by 2 toward the attacked end up to
   +-32h (the referee's carrier sets it from his Y speed), with a faster catch-up (fast) when the carrier skates
   hard. The camera moves when the point leaves a margin (0Ah / 28h, none at the faceoff), at most 20h per step
   (y only when gmode bit 0 is set), by an eighth or sixteenth of the gap (y, by the speed) or a sixteenth (x),
   at least 1. No arguments, no result. */
void checkwindow(void)
{
    short xm;
    short ym;
    short fast;
    Player *p;
    int dx;
    int dy;

    xm = 0x28;
    ym = 0xA;
    fast = 0;
    regd2.w = yc1;
    regd3.w = xc1;
    if (gmode & 0x80) {
        xc1 = regd3.w = -100;
        yc1 = regd2.w = 0xF;
        ym = 5;
        regd0.w = 0xF - camy;
        if (regd0.w < -5) regd1.w = 0x14;
        else if (regd0.w > ym) regd1.w = 0xA;
        if (ABSW(regd0.w) > ym) {
            regd1.w -= camy;
            if (ABSW(regd1.w) > 0x20) regd1.w = regd1.w < 0 ? -0x20 : 0x20;
            if (regd1.w) {
                regd1.w >>= 5;
                if (!regd1.w) regd1.w = 1;
                camy += regd1.w;
            }
        }
        regd0.w = regd3.w - camx;
        if (regd0.w < -xm) {
            regd1.w = regd3.w + xm;
            if (regd1.w < -0x20) regd1.w = -0x20;
        } else if (xm < regd0.w) {
            regd1.w = regd3.w - xm;
            if (regd1.w > 0x20) regd1.w = 0x20;
        } else return;
        regd1.w -= camx;
        if (ABSW(regd1.w) > 0x20) regd1.w = regd1.w < 0 ? -0x20 : 0x20;
        if (!regd1.w) return;
        regd1.w >>= 5;
        if (!regd1.w) regd1.w = 1;
        camx += regd1.w;
        return;
    }
    if (gameclock || clockticks[0]) {
        if ((gmode & 4) && Pencntdwn < 0 || RefStep[0] > 0) {
            p = &SortCords[0x10];
            xc1 = HIWORD(p->Xpos);
            yc1 = word_E0022;
            regd2.w = xc1 - camx;
            regd3.w = word_E0046 - camx;
            if ((regd2.w ^ regd3.w) < 0) xc1 = camx;
            else if (ABSW(regd2.w) > ABSW(regd3.w)) xc1 = p->temp3;
            regd2.w = yc1 - camy;
            regd3.w = p->temp4 - camy;
            if ((regd2.w ^ regd3.w) < 0) yc1 = regd2.w = camy;
            else if (ABSW(regd2.w) > ABSW(regd3.w)) yc1 = regd2.w = p->temp4;
            else regd2.w = yc1;
            regd3.w = xc1;
            dx = HIWORD(p->Xpos) - fox;
            dy = HIWORD(p->Ypos) - foy;
            if (p->asslist[p->assnum] == 0x22 || p->asslist[p->assnum] == 0x2C) {
                if (dx * dx + dy * dy < 0x640) {
                    yc1 = regd2.w = foy;
                    xc1 = regd3.w = fox;
                    ym = xm = 0;
                }
            }
        } else if (!(sflags & 0x40)) {
            if (*puckc >= 0) {
                p = &SortCords[*puckc];
                regd0.w = 2;
                if (p->SCnum == 0x10) {
                    yleader = -(p->Yvel >> 8);
                } else if (p->pflags & 0x80) {
                    fast = p->Yvel < -100;
                    yleader += regd0.w;
                    if (yleader >= 0x32) yleader = 0x32;
                } else {
                    fast = p->Yvel > 100;
                    yleader -= regd0.w;
                    if (yleader <= -0x32) yleader = -0x32;
                }
            } else {
                p = (Player *)puckstruct;
            }
            yc1 = regd2.w = (p->Yvel >> 8) + HIWORD(p->Ypos) + yleader;
            xc1 = regd3.w = HIWORD(p->Xpos);
        }
    }
    regd0.w = regd2.w - camy;
    if (regd0.w < -ym) {
        regd1.w = regd2.w + ym;
        if (regd1.w < -0xBC) regd1.w = -0xBC;
    } else if (ym < regd0.w) {
        regd1.w = fast ? regd2.w : regd2.w - ym;
        if (regd1.w > 0xEC) regd1.w = 0xEC;
    }
    if (ABSW(regd0.w) > ym) {
        regd1.w -= camy;
        if ((gmode & 1) && ABSW(regd1.w) > 0x20) regd1.w = regd1.w < 0 ? -0x20 : 0x20;
        if (regd1.w) {
            regd1.w >>= (fast == 0) + 3;
            if (!regd1.w) regd1.w = 1;
            camy += regd1.w;
        }
    }
    regd0.w = regd3.w - camx;
    if (regd0.w < -xm) {
        regd1.w = regd3.w + xm;
        if (regd1.w < -0x20) regd1.w = -0x20;
    } else if (xm < regd0.w) {
        regd1.w = regd3.w - xm;
        if (regd1.w > 0x20) regd1.w = 0x20;
    } else return;
    regd1.w -= camx;
    if (ABSW(regd1.w) > 0x20) regd1.w = regd1.w < 0 ? -0x20 : 0x20;
    if (!regd1.w) return;
    regd1.w >>= 4;
    if (!regd1.w) regd1.w = 1;
    camx += regd1.w;
}

/* LoadTeamPPV (66497, PC only; ends in checkwindow_popebp): start overlay sequence seq when enough memory is free
   (sub_8DAB8 against dword_CD4B0[seq]): with the joystick sampling off, load its PPV art bank (off_CBED0, from the
   CD when fileoncd says so) into dword_E0244 and its frames ("%04d") into dword_DEF8C, then set the overlay
   state: ovlseq, its script (off_CC01D), the first step's timer (word_CC054), step index 1 and the first step.
   seq: the overlay. No result. */
void LoadTeamPPV(short seq)
{
    char name[0x10];
    int i;

    if (sub_8DAB8() < dword_CD4B0[seq]) return;
    joysampling_save = joysampling;
    joysampling = 0;
    MakePath(name, fileoncd[dword_CC080[seq]] == 1 ? (char *)cddriveptr : 0, (char *)off_CBED0[seq], (char *)str_PPV);
    dword_E0244 = sub_8E8A0(name, 0x20);
    for (i = 0; i < sub_90354(dword_E0244); i++) {
        sprintf(name, (char *)str_04d2, i);
        dword_DEF8C[i] = sub_B30B4(dword_E0244, name);
    }
    joysampling = joysampling_save;
    ovlseq = seq;
    ovlscript = off_CC01D[seq];
    ovlsteptimer = ((int *)word_CC054)[seq];
    ovlstepidx = 1;
    ovlstep = ((signed char *)ovlscript)[1];
}

/* UpdateTextOverlay (66E06, PC only; ends in checkwindow_popx): run the overlay once per frame while ovltimer is
   running (not during a penalty shot's count-down 100h-140h before the game ends). Overlay 0 raises the crowd in
   its first steps. At timer 10h step through the script: when a step's timer runs out, the next step, or at the
   script's end free the art (and for some overlays show the text at 258h) and clear the overlay. Otherwise the
   timer counts up (not between 100h and 257h) and past 268h stops and clears the five text lines. No arguments,
   no result. */
void UpdateTextOverlay(void)
{
    signed char *s;

    if (ovltimer < 0) return;
    if (Pencntdwn >= 0x100 && Pencntdwn <= 0x140 && !(gmode & 0x40)) return;
    if (ovlseq == 0 && ovlstep < 5 && crowdlevel < 0x5DC) {
        if (ovlstepidx == 1 && crowdlevel < 0x280) crowdlevel = 0x280;
        if (ovlstepidx >= 1 && ovlstepidx < 5 || ovlstepidx > 7 && ovlstepidx < 0xD || ovlstepidx > 0x10)
            crowdlevel += 8;
    }
    if (ovltimer == 0x10 && ovlseq != -1 && ovlscript) {
        if (--ovlsteptimer >= 0) return;
        s = (signed char *)ovlscript;
        if (*s == ovlstepidx) {
            FreeMemBlock(dword_E0244);
            if (byte_CC049[ovlseq]) ovltimer = 0x258;
            ovlseq = ovlstep = -1;
            ovlscript = 0;
            return;
        }
        ovlstepidx++;
        ovlstep = s[ovlstepidx];
        ovlsteptimer = ((int *)word_CC054)[ovlseq];
        return;
    }
    if (ovltimer < 0x100 || ovltimer >= 0x258) ovltimer++;
    if (ovltimer > 0x268) {
        ovltimer = -1;
        textline3 = textline1 = textline2 = textline4 = textline5 = 0;
    }
}
