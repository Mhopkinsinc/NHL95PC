/* Game state save / load: write the game state. */
/* DRAFT: same size, every instruction but one register (the rink-object loop computes i * 3 in ebx, the EXE in
   ecx); see docs/UNSURE.md. Owner of SaveGameState_popx (LoadGameState ends there). */
#define SaveGameState SaveGameState_decl
#include "nhl95.h"
#undef SaveGameState

typedef struct { int d[4]; } Rec16;
typedef struct { short w[3]; } Rec6;

/* SaveGameState (5FB03, PC only; owner of SaveGameState_popx): write the running game to the open saved-game
   file fh (each write failing is fatal: str_ErrSaveGame): the controller sides, the 17 sort objects (66h bytes
   each), both team structures (D4h), both line tables, then 80h-byte blocks of packed words (teams, camera,
   faceoff data, pass and shot state, controllers, clocks, overlay, penalty-shot and crowd state, line-change
   state, HUD clock, ...), the 20 rink objects, the penalty buffer and the event / HUD penalty records, and the
   player and goalie game stats in 80h-byte chunks. Result: 0. */
int SaveGameState(int fh)
{
    char buf[0x8C];
    short *p;
    char *q;
    int i;
    unsigned n;

    if (FileWriteAt(fh, ctl1side, -1, 4)) FatalError((char *)str_ErrSaveGame);
    for (i = 0; i < 0x11; i++)
        if (FileWriteAt(fh, (Player *)((i << 7) + (int)SortCords), -1, 0x66)) FatalError((char *)str_ErrSaveGame);
    for (i = 0; i < 2; i++)
        if (FileWriteAt(fh, (char *)((i << 8) + (int)&hmtmstruct), -1, 0xD4)) FatalError((char *)str_ErrSaveGame);
    if (FileWriteAt(fh, hmlinetab, -1, 0x30)) FatalError((char *)str_ErrSaveGame);
    if (FileWriteAt(fh, awlinetab, -1, 0x30)) FatalError((char *)str_ErrSaveGame);
    p = (short *)buf;
    *p++ = HomeTeam;
    *p++ = *(short *)(&VisTeam);
    *p++ = *(short *)(&camx);
    *p++ = *(short *)(&camy);
    for (i = 0; i < 6; i++) *p++ = fofdata2[i];
    *p++ = *(short *)(&shotplayer);
    *p++ = *(short *)(&lastplayer);
    *p++ = *(short *)(&passdir);
    *p++ = *(short *)(&passspeed);
    *p++ = *(short *)(&passplayer);
    *p++ = *(short *)(&threat);
    for (i = 0; i < 4; i++) *p++ = ((short *)puckcross)[i];
    *p++ = *(short *)(&lj1);
    *p++ = *(short *)(&lj2);
    *p++ = *(short *)(&lasttouch);
    *p++ = *(short *)(&ltx);
    *p++ = *(short *)(&lty);
    *p++ = *(short *)(&xc1);
    *p++ = *(short *)(&yc1);
    *p++ = *(short *)(&yleader);
    *p++ = *(short *)(&fox);
    *p++ = *(short *)(&foy);
    *p++ = *(short *)(&fodir1);
    *p++ = *(short *)(&fodir2);
    *p++ = *(short *)(&sflags);
    *p++ = *(short *)(&gmode2);
    *p++ = *(short *)(&sflags3);
    *p++ = *(short *)(&c1playernum);
    *p++ = *(short *)(&c2playernum);
    *p++ = *(short *)(&cont1team);
    *p++ = *(short *)(&cont2team);
    *p++ = *(short *)(&Pencntdwn);
    *p++ = *(short *)(&Penaltytimer);
    *p++ = *(short *)(&refsignal);
    *p++ = *(short *)(&RefStep);
    *p++ = *(short *)(&RefPen);
    *p++ = *(short *)(&word_C90D8);
    *p++ = *(short *)(&gsp);
    *p++ = *(short *)(&gameclock);
    *p++ = *(short *)(&clockticks);
    *p++ = *(short *)(&shownumbers);
    *p++ = *(short *)(&word_CBC44);
    *p++ = *(short *)(&exitgame);
    *p++ = *(short *)(&gameover);
    *p++ = *(short *)(&ovltimer);
    *p++ = *(short *)(&word_CBEC2);
    *p++ = *(short *)(&fadeinpending);
    *p++ = *(short *)(&deferpending);
    *p++ = *(short *)(&word_CBEC6);
    *p++ = *(short *)(&word_E9AB0);
    *p++ = *(short *)(&bannermsg);
    *p++ = *(short *)(&bannertimer);
    *(int *)p = *(int *)(&seed);
    p += 2;
    *p++ = *(short *)(&PBnum);
    *p++ = *(short *)(&iflags);
    if (FileWriteAt(fh, buf, -1, 0x80)) FatalError((char *)str_ErrSaveGame);
    p = (short *)buf;
    for (i = 0; i < 6; i++) *p++ = ((short *)PlList)[i];
    ((char *)p)[0] = *(char *)(&pad1dev);
    ((char *)p)[1] = *(char *)(&pad2dev);
    p++;
    ((char *)p)[0] = *(char *)(&collflag);
    ((char *)p)[1] = *(char *)(&gmode);
    p++;
    *p++ = *(short *)(&energywarn);
    *p++ = *(short *)((char *)&energywarn + 2);
    *p++ = *(short *)(&lcblink);
    *p++ = *(short *)((char *)&lcblink + 2);
    *p++ = *(short *)(&lcblinktime);
    *p++ = *(short *)((char *)&lcblinktime + 2);
    *p++ = *(short *)(&lcsel);
    *p++ = *(short *)(&word_CBC60);
    *p++ = *(short *)(&lcline);
    *p++ = *(short *)((char *)&lcline + 2);
    *p++ = *(short *)(&lctimer);
    *p++ = *(short *)((char *)&lctimer + 2);
    *p++ = *(short *)(&lcboxon);
    *p++ = *(short *)((char *)&lcboxon + 2);
    *p++ = *(short *)(&lcrequest);
    *p++ = *(short *)((char *)&lcrequest + 2);
    *p++ = *(short *)(&lcreqchoice);
    *p++ = *(short *)((char *)&lcreqchoice + 2);
    *p++ = *(short *)(&savedmousex);
    *p++ = *(short *)(&savedmousey);
    *p++ = *(short *)(&word_DEF84);
    *p++ = *(short *)(&word_DEF88);
    *p++ = *(short *)(&curperiod);
    *p++ = *(short *)(&lldispodd);
    *p++ = *(short *)(&dword_E9AB6);
    *p++ = *(short *)(&word_E9AAE);
    *p++ = *(short *)(&periodendtime);
    *p++ = *(short *)(&dword_E9A9E);
    *p++ = *(short *)(&ltack);
    *p++ = *(short *)(&lastsfx);
    *p++ = *(short *)(&word_E9A9C);
    *p++ = *(short *)(&crowdlevel);
    *p++ = *(short *)(&word_E9AAA);
    *p++ = *(short *)(&word_E9AA2);
    *p++ = *(short *)(&CwdExciteLvl);
    *p++ = *(short *)(&word_E9AA4);
    *p++ = *(short *)(&word_E9AA6);
    *p++ = *(short *)(&dword_E009C);
    *p++ = *(short *)(&ctl1team);
    *p++ = *(short *)(&ctl2team);
    *p++ = *(short *)(&ctl1dev);
    *p++ = *(short *)(&ctl2dev);
    *p++ = *(short *)(&hudclockmin);
    *p++ = *(short *)(&hudclocksec);
    *p++ = *(short *)(&hudclockhund);
    *p++ = *(short *)(&word_CC0B0);
    *p++ = *(short *)(&hudhundaccum);
    *p++ = *(short *)(&frameaccum);
    *p++ = *(short *)(&dword_CC0AC);
    *p++ = *(short *)(&byte_E024C);
    *p++ = *(short *)(&word_E024E);
    *p++ = *(short *)(&gamemode);
    ((char *)p)[0] = *(char *)(&byte_DC268);
    ((char *)p)[1] = *(char *)(&byte_DC267);
    p++;
    ((char *)p)[0] = *(char *)(&byte_DC264);
    ((char *)p)[1] = *(char *)(&byte_DC265);
    p++;
    ((char *)p)[0] = *(char *)(&byte_DC266);
    ((char *)p)[1] = *(char *)(&onetimerflag);
    if (FileWriteAt(fh, buf, -1, 0x80)) FatalError((char *)str_ErrSaveGame);
    p = (short *)buf;
    for (i = 0; i < 0x14; i++) {
        *p++ = word_DEE94[i * 6];
        *p++ = word_DEE96[i * 6];
    }
    *p++ = *(short *)(&ovlseq);
    *p++ = *(short *)(&ovlstep);
    *p++ = *(short *)(&ovlsteptimer);
    *p++ = *(short *)(&ovlstepidx);
    *p++ = *(short *)(&shotongoal);
    *p++ = *(short *)(&penshotplayer);
    *p++ = *(short *)(&penshotpnum);
    *p++ = *(short *)(&penshotteam);
    *p++ = *(short *)(&penshotctl);
    *p++ = *(short *)(&word_CC10C);
    *p++ = *(short *)(&penshotfox);
    *p++ = *(short *)(&penshotfoy);
    *p++ = *(short *)(&penshotmode);
    *p++ = *(short *)(&penshotstart);
    *p++ = *(short *)(&penshottimer);
    *p++ = *(short *)(&shotontarget);
    *p++ = *(short *)(&penshotlive);
    *p++ = *(short *)(&psendcount);
    *p++ = *(short *)(&sopathx);
    *p++ = *(short *)(&sopathy);
    *p++ = *(short *)(&sopathend);
    *p++ = *(short *)(&sopathpoint);
    *p++ = *(short *)(&pspathside);
    *p++ = *(short *)(&pspathdir);
    if (FileWriteAt(fh, buf, -1, 0x80)) FatalError((char *)str_ErrSaveGame);
    if (FileWriteAt(fh, PenBuf, -1, 0x42)) FatalError((char *)str_ErrSaveGame);
    if (FileWriteAt(fh, &byte_E9AC8, -1, 0xB)) FatalError((char *)str_ErrSaveGame);
    if (FileWriteAt(fh, &byte_E9AD3, -1, 0xB)) FatalError((char *)str_ErrSaveGame);
    if (FileWriteAt(fh, unk_C5423, -1, 0xB)) FatalError((char *)str_ErrSaveGame);
    if (FileWriteAt(fh, unk_C542E, -1, 0xB)) FatalError((char *)str_ErrSaveGame);
    if (FileWriteAt(fh, hudpenhome, -1, 0x40)) FatalError((char *)str_ErrSaveGame);
    if (FileWriteAt(fh, hudpenaway, -1, 0x40)) FatalError((char *)str_ErrSaveGame);
    q = buf;
    n = 0;
    for (i = 0; i < 0x32; i++) {
        if (n + 0x10 > 0x80) {
            if (FileWriteAt(fh, buf, -1, n)) FatalError((char *)str_ErrSaveGame);
            q = buf;
            n = 0;
        }
        *(Rec16 *)q = *(Rec16 *)((char *)dword_DB088 + i / 25 * 25 * 16 + i % 25 * 16);
        q += 0x10;
        n += 0x10;
    }
    for (i = 0; i < 6; i++) {
        if (n + 6 > 0x80) {
            if (FileWriteAt(fh, buf, -1, n)) FatalError((char *)str_ErrSaveGame);
            q = buf;
            n = 0;
        }
        *(Rec6 *)q = *(Rec6 *)((char *)unk_DC240 + i / 3 * 9 * 2 + i % 3 * 3 * 2);
        q += 6;
        n += 6;
    }
    if (n && FileWriteAt(fh, buf, -1, n)) FatalError((char *)str_ErrSaveGame);
    return 0;
}
