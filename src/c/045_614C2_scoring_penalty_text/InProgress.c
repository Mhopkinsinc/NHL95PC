/* Scoring / penalty text: calling the penalties at a stoppage. */
#include "nhl95.h"

void PostPenaltyEvent(int away, int pl, int type, int tag, int min, int sec, int n);

/* InProgress (62EE9, PC only; owner of InProgress_popebp / InProgress_popx): call the next queued penalty when
   play is stopped (RefStep set, no play-by-play talking). With no penalty left (PenBuf empty) and no overlay
   running, update both teams' strength (tmap 6, less one per player in the box down to 4, the box flag bit 13
   cleared) and send the puck to its stoppage assignment (1Bh). Else take the last queued penalty: one already
   called waits for its player to reach the box; a penalty shot (1Ah) starts it; a non-penalty (no minutes)
   only sets the referee's signal; a minus-minute entry (a coincidental minor) and a real penalty put the player
   in the box (tmpdst minutes * 60, bits 13 / 14), count it in the team and player stats, add it to the panel
   and the play-by-play (PostPenaltyEvent, the time left in the period), and send him off (assignment 0Ch); a
   player already in the box has one of his teammates on the ice serve it ("served by"). Then the referee's
   signal (SetPA). No arguments, no result. */
void InProgress(void)
{
    Player *pl;
    Team *tm;
    short pen;
    short mins;
    short pnum;
    short i;
    short t;
    short n;
    short side;
    signed char a;

    if (!RefStep[0] || PaSpeechBusy()) return;
again:
    if (!PenBuf[0]) {
        if (ovlseq != 0 && ovlseq != 2 && ovltimer >= 0) return;
        for (side = 0, tm = &hmtmstruct; side < 2; side++, tm = &awtmstruct) {
            n = 6;
            for (t = 0x1B; t >= 0; t--) {
                if (tm->tmpdst[t] > 0) {
                    ((unsigned char *)&tm->tmpdst[t])[1] &= 0xDF;
                    if (!(((unsigned char *)&tm->tmpdst[t])[1] & 0x10) && n > 4) n--;
                }
            }
            tm->tmap = n;
        }
        gmode &= 0xFB;
        a = ((Player *)puckstruct)->asslist[((Player *)puckstruct)->assnum];
        if (a == 0x1B || a == 0x1C) return;
        assreplace((Player *)puckstruct, 0x1B);
        return;
    }
    for (i = 1; PenBuf[i * 2]; i++);
    i--;
    if (!(PenBuf_pl[i * 2] & 0x80)) {
        pl = &SortCords[PenBuf_pl[i * 2]];
        if (pl->position >= 0) return;
        PenBuf_pl[i * 2] = PenBuf[i * 2] = 0;
        return;
    }
    PenBuf_pl[i * 2] &= 0x7F;
    pen = PenBuf[i * 2];
    if (pen == 0x1A) {
        StartPenaltyShot();
        t = PerTimeTotal - gameclock;
        if (gameclock < 0x3C && clockticks[0]) t--;
        pl = &SortCords[PenBuf_pl[i * 2]];
        PenBuf_pl[i * 2] = PenBuf[i * 2] = 0;
        PostPenaltyEvent((pl->pflags & 0x40) != 0, pl->pnum, pen - 9, 0, t / 60, t % 60, 0);
        goto signal;
    }
    mins = penmintab[pen];
    if (mins < 0) {
        pl = &SortCords[PenBuf_pl[i * 2]];
        tm = pl->tmptr;
        tm->tmroster[pl->pnum * 0x27] = 8;
        if (tm->tmpdst[pl->pnum] > -3) assreplace(pl, 0x1D);
        tm->tmpdst[pl->pnum] = -5;
        t = PerTimeTotal - gameclock;
        if (gameclock < 0x3C && clockticks[0]) t--;
        PostPenaltyEvent((pl->pflags & 0x40) != 0, pl->pnum, pen - 9, mins, t / 60, t % 60, i);
        word_C90D8 = PenBuf_pl[i * 2];
        PenBuf_pl[i * 2] = PenBuf[i * 2] = 0;
        goto signal;
    }
    if (mins == 0) {
        word_C90D8 = PenBuf_pl[i * 2];
        PenBuf_pl[i * 2] = PenBuf[i * 2] = 0;
        if ((sflags & 0x80) || word_CBC44) {
            if (pen == 7) goto again;
        }
        goto signal;
    }
    pl = &SortCords[PenBuf_pl[i * 2]];
    tm = pl->tmptr;
    tm->tmpen++;
    tm->tmpenmin += mins;
    pnum = pl->pnum;
    if (pnum < 0x19) tm->tmplstats[pnum * 8 + 2] += mins;
    PanelAddPenalty((pl->pflags & 0x40) != 0, pl->rostnum, mins);
    t = PerTimeTotal - gameclock;
    if (gameclock < 0x3C && clockticks[0]) t--;
    PostPenaltyEvent((pl->pflags & 0x40) != 0, pl->pnum, pen - 9, mins, t / 60, t % 60, i);
    mins *= 60;
    if (mins == 0x12C) ((unsigned char *)&mins)[1] |= 0x40;
    ((unsigned char *)&mins)[1] |= 0x20;
    if (tm->tmpdst[pnum] == -3 || tm->tmpdst[pnum] == -4) {
        for (pl = tm->tmsort, t = 0; t < 6; t++, pl++) {
            if (pl->position > 0 && tm->tmpdst[pl->pnum] == -1 && !(pl->pflags2 & 0x10)) break;
        }
        if (t == 6) return;
        pnum = pl->pnum;
        PenBuf_pl[i * 2] = PenBuf_pl[i * 2] & 0x80 | pl->SCnum;
        sprintf((char *)&textline5, (char *)str_ServedByD, pl->rostnum);
    }
    if (tm->tmpdst[pnum] >= 0 && (((unsigned char *)&tm->tmpdst[pnum])[1] & 0x10)) ((unsigned char *)&mins)[1] |= 0x10;
    tm->tmpdst[pnum] = mins;
    tm->tmroster[pnum * 0x27] = 5;
    for (i = 0; i < 0x1C; i++)
        if (tm->tmpbox[i] < 0) break;
    tm->tmpbox[i] = pnum;
    tm->tmpbox[i + 1] = -1;
    assreplace(pl, 0xC);
    word_C90D8 = pl->SCnum;
signal:
    SetPA(pen);
}

/* ProcessPenaltyList (63A37, PC only; ends in InProgress_popx): once a second, count down the penalties of team
   tm's players in the box (tmpbox, ended by a negative byte): only the first two (regd1 from 2) serve time; a
   player whose minutes (tmpdst, low 14 bits) run out is back on the bench (roster status 7, tmpdst bit 14
   cleared) and the box list shifts down over him. Then the first one or two of them (by how many served) may be
   released onto the ice (CheckAndReleasePlayer). tm: the team. No result. */
void ProcessPenaltyList(Team *tm)
{
    signed char out[4];
    short n;
    short i;
    short k;
    short t;

    regd1.l = 2;
    regd3.l = 2;
    for (n = 0, i = 0; i < 0x1C; i++) {
        k = tm->tmpbox[i];
        if (k < 0) break;
        if (--regd1.w < 0) continue;
        out[n++] = k;
        t = --tm->tmpdst[k];
        if (t & 0x3FFF) continue;
        tm->tmroster[k * 0x27] = 7;
        ((unsigned char *)&tm->tmpdst[k])[1] &= 0xBF;
        for (regd2.w = 1;; regd2.w++) {
            if ((((signed char *)tm)[0xB5 + regd2.w + i] = tm->tmpbox[regd2.w + i]) < 0) break;
        }
        i--;
    }
    if (regd1.w == 1) CheckAndReleasePlayer(tm, out[0]);
    else if (regd1.w == 0) {
        CheckAndReleasePlayer(tm, out[0]);
        CheckAndReleasePlayer(tm, out[1]);
    } else if (regd1.w == -1) CheckAndReleasePlayer(tm, out[1]);
}

/* ShotLaneOpen (64102, PC only; ends in InProgress_popebp): is the puck carrier's shooting lane open? Not when
   nobody has the puck or he is in his own half. Else his distance to the net he attacks (y E8h, vecdist) must be
   shorter than every opposing skater's (position above 0) who is nearer to that net than he is (4 behind him)
   and not skating away from it, and no such skater may be beyond him. Sets shotontarget. No arguments.
   Result: 1 when the lane is open, else 0. */
int ShotLaneOpen(void)
{
    Player *p;
    int y;
    int ty;
    int x;
    volatile int d;
    volatile int m;
    int i;

    shotontarget = 0;
    if (*puckc < 0) return 0;
    p = &SortCords[*puckc];
    y = HIWORD(p->Ypos);
    ty = y;
    x = HIWORD(p->Xpos);
    if (p->pflags & 0x80) {
        if (y < 0) return 0;
        y += 4;
        d = vecdist(ty - 0xE8, x);
        m = 0;
        for (i = 0, p = &SortCords[p->SCnum < 6 ? 6 : 0]; i < 6; i++, p++) {
            if (p->position <= 0) continue;
            if (HIWORD(p->Ypos) > y) return 0;
            if (p->Yvel < 0) continue;
            if (vecdist(HIWORD(p->Ypos) - 0xE8, HIWORD(p->Xpos)) <= d - m) return 0;
        }
    } else {
        if (y > 0) return 0;
        y -= 4;
        d = vecdist(ty + 0xE8, x);
        m = 0;
        for (i = 0, p = &SortCords[p->SCnum < 6 ? 6 : 0]; i < 6; i++, p++) {
            if (p->position <= 0) continue;
            if (HIWORD(p->Ypos) < y) return 0;
            if (p->Yvel > 0) continue;
            if (vecdist(HIWORD(p->Ypos) + 0xE8, HIWORD(p->Xpos)) <= d - m) return 0;
        }
    }
    shotontarget = 1;
    return 1;
}
