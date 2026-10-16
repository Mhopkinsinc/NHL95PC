/* Engine display: replay recording. */
#include "nhl95.h"

/* updatereplay (675D6; 94G replay94 updatereplay, "save replay events every frame"): outside highlight playback
   (gmode bit 4), every second tick (replaytick) append one replay frame at recbpr (in reverse mode, gmode2 bit 2,
   first step back with ReplayPrevFrame): per sort object a dword of y (10 bits), x (10 bits, shifted 10), the word
   at +12h (11 bits, shifted 20) and bit 11 of the word at +54h; the frame byte (+5Eh) of the 12 players; the six
   position pairs as nibbles (Fh for off the ice); the puck height, byte_DFFA6, the two sound effect bytes
   (replaysfx, lastsfx, then both -1), the controlled player numbers and the puck carrier, the penalty box nibbles
   (PBnum), camx (byte), camy and crowdlevel (words), the ten rink objects' frames as nibbles and the twenty
   object bytes of word_DEE94. When the buffer (9600h bytes from replaystart) is full it wraps (sflags bit 4). On
   the other tick only the sound effect is carried over (replaysfx). No arguments, no result.
   DRAFT: 810 vs 812 (198 of 216 instructions). A volatile lo gives the original stack byte and frame. Left: the
   tick - 2 goes through edi (here ecx, so edi is not saved), the y / x packing swaps ebx and ecx, and the nibble
   loop shares 2i between the two indexes where the original computes 2i + 1 in one lea and 24i fresh. See
   docs/UNSURE.md. */
void updatereplay(void)
{
    Player *p;
    short i;
    volatile unsigned char lo;

    if (gmode & 0x10) return;
    if (++replaytick < 2) {
        if (gmode2 & 4) replaysfx = -1;
        else replaysfx = lastsfx;
        lastsfx = -1;
        return;
    }
    replaytick -= 2;
    if (gmode2 & 4) recbpr = ReplayPrevFrame();
    for (p = SortCords, i = 0; i < 0x11; i++, p++) {
        *(int *)recbpr = (short)(HIWORD(p->Ypos) & 0x3FF) << 10 | (short)(HIWORD(p->Xpos) & 0x3FF)
                       | (short)(((short *)p)[9] & 0x7FF) << 20 | (short)(((short *)p)[0x2A] & 0x800) << 20;
        recbpr += 4;
    }
    for (p = SortCords, i = 0; i < 0xC; i++, p++) *recbpr++ = ((unsigned char *)p)[0x5E];
    for (p = SortCords, i = 0; i < 6; i++) {
        if (p->position < 0) regd0.w = 0xF;
        else regd0.w = p->position;
        p++;
        if (p->position < 0) *(unsigned char *)&regd0 |= 0xF0;
        else regd0.w |= p->position << 4;
        p++;
        *recbpr++ = *(unsigned char *)&regd0;
    }
    *recbpr++ = *(unsigned char *)puckz;
    *recbpr++ = byte_DFFA6;
    *recbpr++ = replaysfx;
    *recbpr++ = lastsfx;
    replaysfx = lastsfx = -1;
    *recbpr++ = (c2playernum[0] & 0xF) << 4 | (c1playernum[0] & 0xF);
    *recbpr++ = *puckc;
    *recbpr++ = (PBnum[1] & 0xF) << 4 | (PBnum[0] & 0xF);
    *recbpr++ = camx;
    *(short *)recbpr = camy;
    recbpr += 2;
    *(short *)recbpr = crowdlevel;
    recbpr += 2;
    for (i = 0; i < 0xA; i++) {
        lo = byte_DEE97[(i + i + 1) * 12] & 0xF;
        *recbpr++ = lo | (byte_DEE97[i * 3 * 8] & 0xF) << 4;
    }
    for (i = 0; i < 0x14; i++) *recbpr++ = ((unsigned char *)word_DEE94)[i * 12];
    if (replaystart + 0x9600 <= recbpr) {
        sflags |= sfwrap;
        recbpr = replaystart;
    }
}
