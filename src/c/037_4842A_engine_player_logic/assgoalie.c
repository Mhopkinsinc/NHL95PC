/* Engine player logic: the goalie's assignment. */
/* DRAFT: 3,718 vs 3,774 bytes, 230 differing lines; see docs/UNSURE.md. assgoalie ends in ass_popx (assrefwhistle.c), so its match belongs in
   assrefwhistle.c between asspenalty and CountPuckThreat; the draft sits in its own file so the matched blocks
   there stay as they are. */
#define imul32 imul32_decl
#include "nhl95.h"
#undef imul32

int __cdecl imul32(int a, int b);  /* library: 32-bit product */

#define CROWDUP() \
    if (crowdlevel <= 0x4B0 && (crowdlevel += 0x96) > 0x4B0) crowdlevel = 0x4B0

/* assgoalie (4B774; 93G assgoalie, PC version; ends in ass_popx): the goalie. Count down the save timer (+4Ah),
   lose the puck hold flag (+63h) when he has not got it, and set the crease flags (+48h bits 1 and 2, cleared in
   the crease, the inner one only between x -28h and 28h). Not at the bench (check4bench); a new assignment (bit 1)
   clears temp1, temp2 8 and +2Dh. Behind the goal line (y beyond E4h) skate out in front of the post (x A0h toward
   his side, or his own x beyond 30h). Out of the crease area (x beyond 34h, y from B3h to EFh) skate back to the
   crease (x 4 toward his side, y C4h). In it: the stance (SPA 99h with the puck, 1); with the clock stopped
   nothing else. Holding the puck in the crease counts down temp5 (a delay penalty when it runs out) and moves him
   toward the boards. Every aidef / 3 ticks (temp1): with the puck in the other half stand at the crease (D4h);
   holding it, after a while pass or freeze it (SPA 1135h when an attacker is within 14h, else chk4pass with threat
   set); against a shooter, now and then poke check (SPA 1095h) or dive (SPA 181h, the crowd up). Otherwise play
   the angle: face the puck, move toward the point on the line from the puck through the net (the shot speed and
   ShotLaneOpen pick the depth), or to the post the puck will cross (puckcross), with the save stances for a puck
   coming at him (dword_CCA5A by side and height, the crowd up) and a stop for a wraparound (assinsert 0Fh). Skate
   there (playeracc) or stop (StopNA). p: the goalie. No result. */
void assgoalie(Player *p)
{
    Player *q;
    short *pc;
    int px;
    int ay;
    int ok;
    int d;
    int y2;
    int a;
    short k;

    ok = 1;
    if (*(short *)((char *)p + 0x4A) > 0) (*(short *)((char *)p + 0x4A))--;
    if (*puckc != p->SCnum) ((unsigned char *)p)[0x63] = 0;
    ((unsigned char *)p)[0x48] |= 6;
    px = HIWORD(p->Xpos);
    ay = HIWORD(p->Ypos);
    ay = ABS(ay);
    if (px >= -0x28 && px < 0x29 && ay >= 0xC5 && ay < 0xE4) ((unsigned char *)p)[0x48] &= 0xFB;
    if (px >= -0x30 && px < 0x31 && ay >= 0xC5) ((unsigned char *)p)[0x48] &= 0xFD;
    if (check4bench(p)) return;
    if (p->pflags & 2) {
        p->pflags &= 0xFD;
        p->temp1 = 0;
        p->temp2 = 8;
        ((unsigned char *)p)[0x2D] = 0xFF;
    }
    if (ay > 0xE4) {
        regd2.w = 0;
        if (ABS(px) <= 0x30) {
post:
            regd0.w = HIWORD(p->Xpos) < 0 ? -0xA0 : 0xA0;
        } else {
            regd0.w = px;
        }
        goto move;
    }
    if (ABS(px) > 0x34 || ay >= 0xEF || ay < 0xB3) {
        regd0.w = px >= 0 ? 4 : -4;
        regd1.w = p->pflags & 0x80 ? -0xC4 : 0xC4;
        skateto(p, 0);
        return;
    }
    if (!(p->pflags2 & 2) && !(p->pflags & 0x20)) SetSPA(p, *puckc == p->SCnum ? 0x99 : 1);
    if (gmode & 1) return;
    if (p->temp5 >= 0) {
        if (*puckc == p->SCnum) {
            if (ay < 0x9B) p->temp5 = 0;
            if (--p->temp5 < 0) AddPenalty2(p, 4);
        } else {
            p->temp5 = -1;
        }
    }
    if (p->pflags2 & 2) return;
    if (--p->temp1 >= 0) {
        regd2.w = ((signed char *)p)[0x28];
        if (regd2.w <= 7) goto skate;
stop:
        StopNA(p);
        return;
    }
    p->temp1 = p->aidef / 3;
    if (!(p->pflags & 8)) {
        regd1.w = *pucky ^ HIWORD(p->Ypos);
        if (regd1.w < 0) {
            regd0.w = 0;
            regd2.w = p->pflags & 0x80 ? -0xD4 : 0xD4;
            goto move;
        }
        p->temp4--;
        if (*puckc == p->SCnum) {
            if (p->temp5 < 0) p->temp5 = 0x5A;
            p->temp4 = -1;
            if (p->temp5 < 0x5A && !randomd0(4)) {
                q = &SortCords[p->SCnum < 6 ? 6 : 0];
                for (k = 0; k < 6; k++, q++) {
                    if (!(q->pflags2 & 4) && q->position > 0 && q->puckdist < 0x23 && q->puckdist < 0x14) {
                        p->pflags |= 0x20;
                        p->pflags2 |= 2;
                        SetSPA(p, 0x1135);
                        return;
                    }
                }
                threat = 1;
                if (chk4pass(p)) return;
            }
            goto angle;
        }
    }
    regd0.w = *puckx - HIWORD(p->Xpos);
    regd1.w = *pucky - HIWORD(p->Ypos);
    if (*puckc >= 0) {
        if (*puckc >= 0 && (p->SCnum < 6) ^ (*puckc < 6) && *(short *)((char *)p + 0x4A) == 0
            && randomd0(0x100) < 0xA && p->puckdist < 0x23 && ABS(*pucky) < 0xE9) {
            p->facedir = vtoa(regd0.w, regd1.w);
            p->pflags |= 0x20;
            p->pflags2 |= 2;
            SetSPA(p, 0x1095);
            *(short *)((char *)p + 0x4A) = 0xF0;
            return;
        }
    } else {
        if (p->temp4 == 0 && p->puckdist < 0x2D && ABS(*pucky) < 0xE9
            && *(int *)((char *)p->optmptr + 0x3E) < 0x3C && randomd0(0x100) < 0xA) {
            p->facedir = vtoa(regd0.w, regd1.w);
            p->nopuck = 8;
            p->pflags |= 0x20;
            p->pflags2 |= 2;
            SetSPA(p, 0x181);
            *(short *)((char *)p + 0x4A) = 0x168;
            CROWDUP();
            return;
        }
    }
angle:
    pc = puckcross;
    regd3.w = 0xE4;
    regd4.w = 0xE8;
    if (p->pflags & 0x80) {
        regd3.w = -0xE4;
        regd4.w = -0xE8;
        pc += 2;
    }
    if (ABS(HIWORD(p->Ypos)) > 0xE4) {
        regd2.w = regd0.w = 0;
        if (ABS(HIWORD(p->Xpos)) > 0x30) goto move;
        goto post;
    }
    regd0.w = *puckx;
    regd1.w = *pucky;
    ClampYPosition(p);
    regd0.w = vtoa(regd0.w, regd1.w);
    regd1.w = p->facedir;
    if ((regd0.w -= regd1.w) != 0) {
        regd0.w = ((-regd0.w & 4) >> 1) - 1;
        if (1 << regd1.w & 0x42) {
            regd1.w += regd0.w;
            if (1 << regd1.w & (p->pflags & 0x80 ? 0x38 : 0x83)) {
                regd0.w = -regd0.w;
                regd1.w += regd0.w + regd0.w;
            }
        } else {
            regd1.w += regd0.w;
        }
        regd1.w &= 7;
        p->facedir = regd1.w;
    }
    regd0.w = 0xE0;
    if (ABS(*pucky) > 0xBB) regd0.w -= 0x40;
    regd1.w = regd0.w;
    regd0.w = *puckx + (imul32(*puckvx, regd0.w) >> 16);
    regd1.w = *pucky + (imul32(*puckvy, regd1.w) >> 16);
    ClampYPosition(p);
    d = vecdist(regd0.w, regd1.w);
    if (d >= 0x1E) {
        regd2.w = d + 1;
        if (ABS(*pucky) < 0x74 && ShotLaneOpen()) {
            regd4.w = 0x14;
            if (sflags & 8) regd4.w = 0x1C;
        } else {
            regd4.w = 0x12;
            if (sflags & 8) regd4.w = 0x1A;
        }
        regd1.w = regd1.w * regd4.w / regd2.w;
        regd4.w += 8;
        regd0.w = regd0.w * regd4.w / regd2.w;
    }
    regd2.w = regd1.w += regd3.w;
    y2 = (*puckvy >> 9) + *pucky;
    if (*puckc < 0 && ABS(*pucky) < 0xE8 && ABS(HIWORD(p->Ypos)) > 0x74 && p->puckdist < 0x1E
        && ABS(y2) > ABS(HIWORD(p->Ypos)))
        ok = 0;
    if (ok && (unsigned short)pc[1] >= 0x23) goto move;
    d = pc[0];
    if (ok && ABS(d) >= 0x19) {
        if (*puckc >= 0 || iflags & 4 || ABS(*puckx) < 0x44 || ABS(*puckvy) < 0x3800 || ABS(*pucky) < 0x98
            || (p->pflags & 0x80 ? -*puckvy : *puckvy) < -0x190 || *(int *)((char *)p->tmptr + 0x3E) < 0x132
            || *(int *)((char *)p->optmptr + 0x3E) < 0x132)
            goto move;
        assinsert(p, 0xF);
        return;
    }
    d -= HIWORD(p->Xpos);
    if (d < 0) d = -d;
    d -= 8;
    if (d > 0x10) d = 0x10;
    if (d < 0) d = 0;
    d >>= 2;
    ay = ABS(HIWORD(p->Ypos));
    if (y2 < 0) y2 = -y2;
    if (y2 > ay) d += 4;
    if (ok && (p->puckdist > 0x19 || ABS(*pucky) > 0xE8)
        && ((unsigned short)pc[1] >= d + 0xC || ABS(*pucky) > 0xE9)) {
        regd0.w = pc[0];
        if (ABS(*pucky) <= 0xDC) goto move;
        goto side;
    }
    if (*puckc == p->SCnum || ABS(*puckx) >= 0x64 || *(short *)puckz >= 0x14) goto move;
    p->Xvel >>= 1;
    p->Yvel >>= 1;
    if (!(((unsigned char *)p)[0x48] & 2)) {
        regd0.w = (*puckvx >> 10) + *puckx - HIWORD(p->Xpos);
        regd1.w = (*puckvy >> 10) + *pucky - HIWORD(p->Ypos);
        a = vtoa(regd0.w, regd1.w);
        if (a == 8) regd3.w = 0;
        else regd3.w = (a - p->facedir) & 7;
    } else {
        regd3.w = (p->puckdir - p->facedir) & 7;
    }
    if (regd3.w == 4 && ABS(*puckvy) + ABS(*puckvx) <= 0x1200) {
        regd0.w = 8;
    } else {
        if (regd3.w == 0 || regd3.w == 4) {
            a = p->facedir;
            if (a == 0) regd3.w = regd0.w < 0 ? 7 : 1;
            else if (a == 4) regd3.w = regd0.w < 0 ? 1 : 7;
            else if (a == 1 || a == 5) regd3.w = ABS(regd0.w) < ABS(regd1.w) ? 7 : 1;
            else if (a == 3 || a == 7) regd3.w = ABS(regd0.w) < ABS(regd1.w) ? 1 : 7;
            else regd3.w = randomd0(2);
        }
        regd0.w = dword_E03AE >> 0x12;
        if (p->attribute & 8) regd0.w = regd0.w == 0;
        if (regd3.w >= 4) regd3.w = 7 - regd3.w;
        if (regd3.w >= 2 && *(short *)puckz < 0xA && *(short *)puckvz < 0x800) {
            regd0.w += 6;
        } else if (*(short *)puckz < 8 && *(short *)puckvz < 0x800) {
            regd0.w += 4;
            if ((unsigned short)pc[1] >= 9 && p->facedir != 2 && p->facedir != 6 && *puckc >= 0) regd0.w -= 2;
        }
    }
    p->pflags |= 0x20;
    p->pflags2 |= 2;
    SetSPA(p, ((short *)dword_CCA5A)[regd0.w + 1]);
    CROWDUP();
    regd0.w = pc[0];
    if (ABS(*pucky) <= 0xDC) goto move;
side:
    regd0.w = *puckx > 0 ? 0x18 : -0x18;
move:
    regd1.w = regd2.w;
    regd0.w -= HIWORD(p->Xpos) + ((signed char *)p)[0x0D];
    regd1.w -= HIWORD(p->Ypos) + ((signed char *)p)[0x0F];
    k = 5;
    if (ok && ABS(regd0.w) < k && ABS(regd1.w) < k) regd0.w = regd1.w = 0;
    regd2.w = ((signed char *)p)[0x28] = vtoa(regd0.w, regd1.w);
    if (regd2.w > 7) goto stop;
skate:
    playeracc(p, regd2.w);
}
