/* Debug dump: the stats log. */
/* DRAFT: 1174 vs 1206 bytes before the exit, first pass; see docs/UNSURE.md. Its match belongs with DebugMonitor
   (DebugMonitor.c), whose DebugMonitor_pop tail it uses. */
#define DebugPrintf DebugPrintf_decl
#define DumpStatsLog DumpStatsLog_decl
#include "nhl95.h"
#undef DebugPrintf
#undef DumpStatsLog

void __cdecl DebugPrintf(char *fmt, ...);
int fprintf(void *fp, const char *fmt, ...);

/* DumpStatsLog (68E7B, PC only; ends in DebugMonitor_pop): append the game state to the text file name (mode "at";
   failing: say so with DebugPrintf). Write the frame counters, both team structures (E8h bytes as dwords), the 17
   sort objects (48h bytes as words) and one line of ratings per object, then for each team its totals (shots,
   score, shot / goal counts, power plays, penalties, attack time, passes), one line per skater (jersey and eight
   stat words) and per goalie (jersey, two stat words). name: the file. No result. */
void DumpStatsLog(char *name)
{
    void *f;
    short i;
    short j;
    int *w;
    short *s;
    Player *p;
    Team *tm;
    short *st;
    unsigned char *r;

    f = fopen(name, (char *)str_fmode_at);
    if (!f) {
        DebugPrintf((char *)str_ErrDumpStats);
        return;
    }
    fprintf(f, (char *)str_dumpPool, hudhundaccum, frameaccum, gamemode);
    fprintf(f, (char *)str_dumpTmstructs);
    i = 0;
    for (; i < 2; i++) {
        w = (int *)((char *)&hmtmstruct + i * 0x100);
        j = 0;
        for (; (unsigned)j < 0xE8; j += 4) fprintf(f, (char *)str_dumpHex, *w++);
        fprintf(f, (char *)str_newline);
    }
    fprintf(f, (char *)str_dumpSortcords);
    i = 0;
    for (; i < 0x11; i++) {
        s = (short *)((i << 7) + (int)SortCords);
        j = 0;
        for (; j < 0x48; j += 2) fprintf(f, (char *)str_dumpHex, *s++);
        fprintf(f, (char *)str_newline);
    }
    i = 0;
    for (; i < 0x11; i++) {
        p = &SortCords[i];
        fprintf(f, (char *)str_dumpSortLine, p->SCnum, p->pnum, p->rostnum, p->weight, p->legstr, p->legspd, p->aioff,
            p->aidef, p->shotspd, p->handed, p->checking, p->stickhand, p->shotacc, p->endurance, p->spodds, p->passacc,
            p->aggress, ((unsigned char *)p)[0x63], *(short *)&p->attrlo);
    }
    tm = &hmtmstruct;
    fprintf(f, (char *)str_dumpRule);
    fprintf(f, (char *)str_dumpTeamHdrH);
    fprintf(f, (char *)str_dumpHome, hmtmstruct.tmshots, hmscore[0], hmshotcnt, hmgoalcnt, hmtmstruct.tmppg,
        hmtmstruct.tmpp, hmtmstruct.tmpen, hmtmstruct.tmpenmin, hmtmstruct.tmATOP, hmtmstruct.tmpass,
        hmtmstruct.tmpasscomp);
    st = (short *)hmtmplstats;
    fprintf(f, (char *)str_dumpPlHdrH);
    j = 0;
    for (; j < 0x19; st += 8, j++) {
        r = tm->tmroster + j * 39;
        fprintf(f, (char *)str_dumpPlLineH, j, r[5], st[0], st[1], st[2], st[3], st[4], st[5], st[6], st[7], r[0]);
    }
    st = tm->tmglstats;
    fprintf(f, (char *)str_dumpGlHdrH);
    j = 0;
    for (; j < 3; st += 3, j++) {
        r = tm->tmroster + (j + 0x19) * 39;
        fprintf(f, (char *)str_dumpGlLine, j, r[5], st[0], st[1], r[0]);
    }
    tm = &awtmstruct;
    fprintf(f, (char *)str_dumpTeamHdrA);
    fprintf(f, (char *)str_dumpAway, awtmstruct.tmshots, awscore, awshotcnt, awgoalcnt, awtmstruct.tmppg,
        awtmstruct.tmpp, awtmstruct.tmpen, awtmstruct.tmpenmin, awtmstruct.tmATOP, awtmstruct.tmpass,
        awtmstruct.tmpasscomp);
    st = (short *)awtmplstats;
    fprintf(f, (char *)str_dumpPlHdrA);
    j = 0;
    for (; j < 0x19; st += 8, j++) {
        r = tm->tmroster + j * 39;
        fprintf(f, (char *)str_dumpPlLineA, j, r[5], st[0], st[1], st[2], st[3], st[4], st[5], st[6], st[7], r[0]);
    }
    st = tm->tmglstats;
    fprintf(f, (char *)str_dumpGlHdrA);
    j = 0;
    for (; j < 3; st += 3, j++) {
        r = tm->tmroster + (j + 0x19) * 39;
        fprintf(f, (char *)str_dumpGlLine, j, r[5], st[0], st[1], r[0]);
    }
    fclose(f);
}
