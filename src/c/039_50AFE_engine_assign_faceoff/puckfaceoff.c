/* Engine assignment faceoff: the faceoff setup. */
/* DRAFT: 1259 vs 1270 bytes, first pass; see docs/UNSURE.md. Its match belongs in TakePlayerFromBox.c, between
   PenShotStart and asstakeposition, since it ends in TakePlayerFromBox_ret6. */
#include "nhl95.h"

/* puckfaceoff (516E1, 93G puckfaceoff; ends in TakePlayerFromBox_ret6): asstab 1Bh, the faceoff setup ("this is
   where the action starts"). On a new assignment (pflags bit 1): free the puck (a penalty-shot goalie carrying it
   gets nopuck 78h and the puck stops), then end the game when the clock and ticks are out or gmode bit 6 is set
   (SetExitGame), play on in overtime when the scores differ (clockcont_0) and stop for a delayed penalty (gmode
   bit 3, Stop4Pen). For a penalty shot send both lines (setpersonel, PenShotStart), clear the line-change
   blinks and go to the end. Otherwise, at the start of a penalty shot move each human controller onto a skater
   of its team (restorepl), return pulled goalies, send both lines when line changes are off (gameopts bit 2),
   start the line-change timers (temp1 / temp2), and with line changes on clear the teams' and players'
   line-change flags, re-sort the sprites for a human team, start the humans' faceoff line changes and let the
   computer pick the away line (CompLine). Then for each team whose timer runs (temp1 / temp2, with the changing
   player's number in temp3 / temp4): out of the change mode cancel it, else count down and at the end take the
   line (lcfound); then send the line (setpersonel, PenShotStart, ForceStartLineup). Once both timers are out the
   computer picks the home line, with line changes off the line numbers are reset and the panel lines redrawn,
   and the assignment becomes pfaceoff2 (1Ch) with temp5 1000. p: the object on the assignment. No result. */
void puckfaceoff(Player *p)
{
    short *tp[2];
    int off;
    signed char k;
    Player *o;
    Team *tm;

    tp[0] = &p->temp1;
    tp[1] = &p->temp2;
    if (p->pflags & 2) {
        if (*puckc != 0x10) {
            if (*puckc >= 0 && penshotstart && SortCords[*puckc].position == 0) {
                SortCords[*puckc].nopuck = 0x78;
                *puckvx = *puckvy = 0;
            }
            *puckc = -1;
        }
        p->pflags &= 0xFD;
        if (!gameclock && !clockticks[0] || gmode & 0x40) {
            SetExitGame();
            return;
        }
        if (gsp == 3 && hmscore[0] != awscore) {
            clockcont_0();
            return;
        }
        if (gmode & 8) {
            Stop4Pen(0);
            return;
        }
        if (penshotmode) {
            ((unsigned char *)p)[0x27] = 0xFF;
            ((unsigned char *)p)[0x29] = 0xFF;
            setpersonel(&awtmstruct);
            PenShotStart(1);
            lcblink[1] = 0;
            energywarn[1] = 0;
            lcboxon[1] = 0;
            setpersonel(&hmtmstruct);
            PenShotStart(0);
            lcblink[0] = 0;
            energywarn[0] = 0;
            lcboxon[0] = 0;
            goto done;
        }
        if (penshotstart) {
            if (c1playernum[0] < 0 && cont1team) {
                o = cont1team == 1 ? SortCords : SortCords + 6;
                for (k = 0; k < 6; k++, o++) {
                    if (o->position >= 0 && o->SCnum != c2playernum[0]) {
                        if (o->SCnum != c1playernum[0]) c1playernum[0] = restorepl(o->SCnum, c1playernum[0]);
                        break;
                    }
                }
            }
            if (c2playernum[0] < 0 && cont2team) {
                o = cont2team == 1 ? SortCords : SortCords + 6;
                for (k = 0; k < 6; k++, o++) {
                    if (o->position >= 0 && o->SCnum != c1playernum[0]) {
                        if (o->SCnum != c2playernum[0]) c2playernum[0] = restorepl(o->SCnum, c2playernum[0]);
                        break;
                    }
                }
            }
        }
        ReturnGoalies();
        if (!gameopts.linechanges) {
            setpersonel(&hmtmstruct);
            setpersonel(&awtmstruct);
        }
        ((unsigned char *)p)[0x27] = 0xFF;
        ((unsigned char *)p)[0x29] = 0xFF;
        if (!gameopts.linechanges) goto lines;
        hmtmflags[0] &= 0xFD;
        awtmflags &= 0xFD;
        for (o = SortCords, k = 0; k < 0xC; k++, o++) o->pflags2 &= 0xF7;
        if ((cont1team | cont2team) && word_CBC44) {
            sflags |= 0x40;
            SprSortVert();
            word_CBC44 = 0;
        }
        if ((regd0.w = c1playernum[0]) >= 0) StartFaceoffLineChange(p, &SortCords[regd0.w]);
        if ((regd0.w = c2playernum[0]) >= 0 && cont2team != cont1team) StartFaceoffLineChange(p, &SortCords[regd0.w]);
        regd0.l = 2;
        if (gameopts.linechanges && regd0.w != cont1team && regd0.w != cont2team) {
            CompLine(&hmtmstruct, &awtmstruct);
            setpersonel(&awtmstruct);
            PenShotStart(1);
        }
    }
lines:
    k = 0;
    do {
        if (*tp[k] >= 0) {
            regd1.w = tp[k][2];
            o = &SortCords[regd1.w];
            off = k << 8;
            tm = (Team *)((char *)&hmtmstruct + off);
            if (!(o->pflags2 & 8)) {
                ((unsigned char *)tp[k])[1] = 0xFF;
                lcboxon[k] = lcblink[k] = 0;
            } else {
                --*tp[k];
                if (*tp[k] >= 0) goto next;
                o = &SortCords[regd1.w];
                if (!(o->pflags2 & 8)) goto next;
                regd2.w = 0;
                lcfound(o);
                lcboxon[k] = lcblink[k] = 0;
            }
            setpersonel(tm);
            PenShotStart(k);
            ForceStartLineup(k);
        }
next:
        ;
    } while (++k != 2);
    if (p->temp1 >= 0 || p->temp2 >= 0) return;
    regd0.w = 1;
    if (gameopts.linechanges && regd0.w != cont1team && regd0.w != cont2team) {
        CompLine(&awtmstruct, &hmtmstruct);
        setpersonel(&hmtmstruct);
        PenShotStart(0);
    }
    if (!gameopts.linechanges) {
        hmtmline[0] = hmtmlcnt = awtmline = awtmlcnt = 0;
        DrawPanelLine(0, 0);
        DrawPanelLine(1, 0);
    }
done:
    assreplace(p, 0x1C);
    p->temp5 = 1000;
}
