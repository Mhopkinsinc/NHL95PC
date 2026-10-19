/* Announcer: fixed-clip say free wrappers and FreeClip. */
#include "nhl95.h"

/* SayNhlIntro: say the NHL intro clip (SayClip). */
int SayNhlIntro(void)
{
    return SayClip((char *)str_NhlInt);
}

/* SayGoodnight: say the goodnight clip. */
int SayGoodnight(void)
{
    return SayClip((char *)str_GoodniteInt);
}

/* SayLineups: say the lineups clip. */
int SayLineups(void)
{
    return SayClip((char *)str_LineupsInt);
}

/* SayNowBack: say the "now back" clip. */
int SayNowBack(void)
{
    return SayClip((char *)str_NowbackInt);
}

/* FreeNowBack: release the "now back" clip. */
void FreeNowBack(void)
{
    FreeClip((char *)str_NowbackInt);
}

/* SayBackMoment: say the "back in a moment" clip. */
int SayBackMoment(void)
{
    return SayClip((char *)str_BackmomtInt);
}

/* FreeBackMoment: release the "back in a moment" clip. */
void FreeBackMoment(void)
{
    FreeClip((char *)str_BackmomtInt);
}

/* SayCoachClip: say the coach clip. */
int SayCoachClip(void)
{
    return SayClip((char *)str_CoachclpInt);
}

/* FreeCoachClip: release the coach clip (falls into FreeClip). */
void FreeCoachClip(void)
{
    FreeClip((char *)str_CoachclpInt);
}

/* FreeClip: when clip name is loaded in the speech bank (FindSpeechSlot; 38-byte slots) with
 its in-use flag (+22h) 1: clear the flag, take its size (+16h) off the used sample bytes (+3B6Ch of
 speechbank) and one off the clip count (+3B74h), then EnsureSampleRoom. */
void FreeClip(char *name)
{
    int slot;
    unsigned char *e;
    int used;
    int off;

    if ((slot = FindSpeechSlot(name)) == -1) return;
    off = slot * 38;
    e = (unsigned char *)speechbank + off;
    if ((used = *(int *)(e + 0x22)) != 1) return;
    *(int *)(e + 0x22) = 0;
    *(int *)((char *)speechbank + 0x3B6C) -= *(int *)((char *)speechbank + off + 0x16);
    *(int *)((char *)speechbank + 0x3B74) -= used;
    EnsureSampleRoom();
}

/* SayElseNhl: say the "elsewhere in the NHL" clip. */
int SayElseNhl(void)
{
    return SayClip((char *)str_ElsenhlInt);
}

/* SayTheGame (84A6C, PC only): say "the game" (thegame.bar). Result: as SayClip.
   DRAFT (kept for the layout: it sits between FreeClip and CompactSpeechMem's near jump to FreeClip_ret). */
int SayTheGame(void)
{
    return SayClip((char *)str_ThegameBar);
}

/* SayScoringPeriod (84A7D, PC only): say the scoring summary clip for a period: "the game" when over is set; in
   overtime (ot) the 1st, 2nd, 3rd or later overtime clip; else the 1st-3rd period clip (other periods: nothing).
   period: 1-based; ot, over: flags. Result: as SayClip, 0 when nothing is said.
   DRAFT: the original's pieces are interleaved with SayClip (SayScoringPeriod_ot2 .. _reg after SayClip's ret),
   which a C file cannot reproduce yet; kept for the layout. */
int SayScoringPeriod(int period, int ot, int over)
{
    if (over) return SayClip((char *)str_ThegameBar);
    if (ot) {
        switch (period) {
        case 1: return SayClip((char *)str_Scor1otpBar);
        case 2: return SayClip((char *)str_Scor2otpBar);
        case 3: return SayClip((char *)str_Scor3otpBar);
        default: return SayClip((char *)str_ScortotpBar);
        }
    }
    switch (period) {
    case 1: return SayClip((char *)str_Scor1perBar);
    case 2: return SayClip((char *)str_Scor2perBar);
    case 3: return SayClip((char *)str_Scor3perBar);
    }
    return 0;
}

/* EnsureSampleRoom (8426F, PC only; speech segment 061, the same source file): make room in the sample memory
   (MakeSampleRoom) and pack it (CompactSpeechMem, which follows: the tail call falls through). No arguments,
   no result. */
void EnsureSampleRoom(void)
{
    MakeSampleRoom();
    CompactSpeechMem();
}

/* CompactSpeechMem (8427E, PC only; segment 061, ends in FreeClip_ret): pack the loaded samples down: while a free
   slot (FindFreeSpeechSlot, from slot 0) has a loaded slot after it (FindLoadedSpeechSlot), move that one into it
   (CompactSpeechSlot) and continue from the next free slot. When no slot is free at all, the loaded-slot test uses
   whatever free was left in edx (as in the original). FindLoadedSpeechSlot is called without an argument: the
   original loads no eax there and relies on f still being in eax from the FindFreeSpeechSlot call. No arguments,
   no result. */
void CompactSpeechMem(void)
{
    int f;
    int l;

    f = FindFreeSpeechSlot(0);
    if (f == -1) goto test;
again:
    l = ((int (*)(void))FindLoadedSpeechSlot)();
test:
    if (l == -1) return;
    CompactSpeechSlot(f, l);
    f = FindFreeSpeechSlot(f);
    goto again;
}
