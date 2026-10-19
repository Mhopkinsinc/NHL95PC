/* Engine display: stepping the replay. */
#define ReplayStep ReplayStep_decl
#define replayplay replayplay_decl
#define CrowdNoiseUpdate CrowdNoiseUpdate_decl
#include "nhl95.h"
#undef ReplayStep
#undef replayplay
#undef CrowdNoiseUpdate

extern unsigned char *replayplay;  /* E03A4: the replay frame being played */
void CrowdNoiseUpdate(short step);

#define RB(r) (*(unsigned char *)&(r))
#define RSB(r) (*(signed char *)&(r))

/* DRAFT: 1217 vs 1230, 274 of 308 instructions; see docs/UNSURE.md. Owner of the ReplayStep_popx tail
   (DrawRinkSideObjects). */
/* ReplayStep (67900, PC only): move the replay n frames (80h bytes each) forward or back, wrapping in the
   9600h-byte buffer when it has wrapped (sflags sfwrap) and stopping at its ends (n becomes -1); going forward
   with sound, play each passed frame's two sound effects (bytes 58h / 59h, FFh none). Then unpack the frame:
   17 players' x / y (10 bits, sign-extended) and frame (11 bits, 7FFh none) plus a flag bit, 12 bytes, six
   pairs of nibbles (Fh none), the puck words and nibbles, the camera, the crowd level, ten nibble pairs and
   twenty bytes (FFh none). Update the crowd noise (off at the end or without sound), step back to the frame
   and sort the 17 players by y (sub_93540 into dword_E9FF0). Result: n, or -1 at an end. */
short ReplayStep(short n, short sound)
{
    int keys[0x11];
    short n0;
    unsigned char *last;
    unsigned char *start;
    short i;

    n0 = n;
    if (n >= 0) {
        last = replayplay;
        while (n >= 1) {
            n--;
            replayplay += 0x80;
            if (replaystart + 0x9600 == replayplay) {
                if (!(sflags & sfwrap)) {
                    n = -1;
                    replayplay -= 0x80;
                    break;
                }
                replayplay = replaystart;
            }
            if (replayplay == recbpr) {
                n = -1;
                replayplay -= 0x80;
                break;
            }
            if (sound && last != replayplay) {
                if (replayplay[0x58] != 0xFF) sfx(replayplay[0x58]);
                if (replayplay[0x59] != 0xFF) sfx(replayplay[0x59]);
            }
        }
    } else {
        last = recbpr;
        start = replaystart;
        while (n < 0) {
            n++;
            if (start == replayplay) {
                if (!(sflags & sfwrap)) {
                    n = -1;
                    break;
                }
                replayplay = start + 0x9600;
            }
            if (replayplay == last) {
                n = -1;
                break;
            }
            replayplay -= 0x80;
        }
    }
    for (i = 0; i < 0x11; i++) {
        regd0.l = *(long *)replayplay;
        regd1.w = regd0.w & 0x3FF;
        replayplay += 4;
        if (HIBYTE(regd1.w) & 2) HIBYTE(regd1.w) |= 0xFC;
        word_E9F18[i] = regd1.w;
        regd0.l >>= 10;
        regd1.w = regd0.w & 0x3FF;
        if (HIBYTE(regd1.w) & 2) HIBYTE(regd1.w) |= 0xFC;
        word_E9F3A[i] = regd1.w;
        regd0.l >>= 10;
        regd1.w = regd0.w & 0x7FF;
        if (regd1.w == 0x7FF) regd1.w = -1;
        word_E9F5C[i] = regd1.w;
        byte_E9F7E[i] = (HIBYTE(regd0.w) & 8) != 0;
    }
    for (i = 0; i < 0xC; i++) byte_E9F8F[i] = *replayplay++;
    for (i = 0; i < 6; i++) {
        RB(regd0) = *replayplay++;
        RB(regd1) = RB(regd0) & 0xF;
        byte_E9F9B[i * 2] = RB(regd1) == 0xF ? -1 : RSB(regd1);
        RB(regd1) = (RSB(regd0) >> 4) & 0xF;
        byte_E9F9C[i * 2] = RB(regd1) == 0xF ? -1 : RSB(regd1);
    }
    word_E9FA7 = *(signed char *)replayplay++;
    word_E9FA9 = *(signed char *)replayplay++;
    replayplay += 2;
    RB(regd0) = *replayplay++;
    RB(regd1) = RB(regd0) & 0xF;
    byte_E9FAB = RB(regd1) == 0xF ? -1 : RSB(regd1);
    RB(regd1) = (RSB(regd0) >> 4) & 0xF;
    byte_E9FAC = RB(regd1) == 0xF ? -1 : RSB(regd1);
    byte_E9FAD = *replayplay++;
    RB(regd0) = *replayplay++;
    byte_E9FAE = RB(regd0) & 0xF;
    byte_E9FAF = (RSB(regd0) >> 4) & 0xF;
    word_E9FB0 = camx = *(signed char *)replayplay++;
    word_E9FB2 = camy = *(short *)replayplay;
    replayplay += 2;
    crowdlevel = *(short *)replayplay;
    replayplay += 2;
    for (i = 0; i < 0xA; i++) {
        RB(regd0) = *replayplay++;
        byte_E9FDC[i * 2] = RB(regd0) >> 4;
        byte_E9FDD[i * 2] = RB(regd0) & 0xF;
    }
    for (i = 0; i < 0x14; i++) {
        regd0.w = *replayplay++;
        if (regd0.w == 0xFF) regd0.w = -1;
        word_E9FB4[i] = regd0.w;
    }
    if (sound && replayplay != recbpr) CrowdNoiseUpdate(n0 * 2);
    else CrowdNoiseOff();
    replayplay -= 0x80;
    for (i = 0; i < 0x11; i++) {
        keys[i] = word_E9F3A[i];
        dword_E9FF0[i] = i;
    }
    sub_93540(0x11, keys, dword_E9FF0);
    return n;
}
