/* Speech: shut down, 24-bit reads, slot loading (one source block: the slot loaders end in ShutdownSpeech's and ReadBE24's exits). */
#include "nhl95.h"

#define BANKD(off) (*(int *)((char *)speechbank + (off)))

/* the speech queue (speechq, 100 bytes): clip slots to say in order, then the playback state */
typedef struct SpeechQ {
    int clip[20];               /* 00h: speechbank slots, -1 ends the sentence */
    int delay;                  /* 50h: ticks until the next clip */
    int count;                  /* 54h: clips queued */
    int pos;                    /* 58h: next clip */
    int idle;                   /* 5Ch: ticks of silence after a sentence */
    int playing;                /* 60h: a sentence is being said */
} SpeechQ;
#define SPEECHQ ((SpeechQ *)speechq)

/* SpeechTimerTick (832BC, PC only; segment 060, the same source file: it ends in ShutdownSpeech_ret5): the timer
   handler InitSpeech installs (addtimer). With the speech system up: count the silence after a sentence down;
   while a sentence plays, count the clip delay down, then start the next clip: its sample (data, size) plays on
   channel dword_D27B2 (toggled 0 / 1 per clip) through sub_8FBE5 for kind 'G' or sub_8FC37 (twice the size), at
   2B11h, volume 7Fh, and its delay is loaded. A slot of -1 ends the sentence; after the last clip the sentence
   ends and the silence is the bank's value at +3B70h. No arguments, no result. */
void SpeechTimerTick(void)
{
    SpeechSlot *s;
    int n;
    int clip;
    int data;
    int len;

    if (!speechinit) return;
    if (SPEECHQ->idle) SPEECHQ->idle--;
    if (!SPEECHQ->playing) return;
    if (SPEECHQ->delay) {
        SPEECHQ->delay--;
        return;
    }
    n = SPEECHQ->pos++;
    if (n < SPEECHQ->count) {
        clip = SPEECHQ->clip[n];
        if (clip == -1) {
            SPEECHQ->playing = 0;
            return;
        }
        s = &speechbank[clip];
        data = s->data;
        len = s->size;
        SPEECHQ->delay = s->delay;
        dword_D27B2++;
        dword_D27B2 &= 1;
        if (s->kind == 'G') sub_8FBE5(data, musichandle, dword_D27B2, len, 0x2B11, 0x7F);
        else {
            len *= 2;
            sub_8FC37(data, musichandle, dword_D27B2, len, 0x2B11, 0x7F);
        }
    } else {
        SPEECHQ->playing = 0;
        SPEECHQ->idle = BANKD(0x3B70);
    }
}

/* ShutdownSpeech: when the speech system is up, remove the SpeechTimerTick timer handler,
 close the open bank file (flag +3B78h, handle +3B7Ch of speechbank), free the speech queue, the sample memory
 (+3B60h, sub_8D2F0), the bank and the request block, and clear speechinit. */
void ShutdownSpeech(void)
{
    if (speechinit != 0) {
        RemoveTimerCallback(SpeechTimerTick);
        if (BANKD(0x3B78) != 0) _dos_close(BANKD(0x3B7C));
        FreeMemBlock(speechq);
        sub_8D2F0(BANKD(0x3B60));
        FreeMemBlock((int)speechbank);
        FreeMemBlock((int)samplereq);
        speechinit = 0;
    }
}


/* ReadBE24: read three bytes from file fh, most significant first (the VIV bank headers are
 big-endian), and return them as a number. */
int ReadBE24(int fh)
{
    unsigned got;
    int value;

    ((unsigned char *)&value)[3] = 0;
    _dos_read(fh, (unsigned char *)&value + 2, 1, &got);
    _dos_read(fh, (unsigned char *)&value + 1, 1, &got);
    _dos_read(fh, (unsigned char *)&value, 1, &got);
    return value;
}

/* an int field of speech slot n (26h bytes each) */
#define SLOTD(n, off) (*(int *)((char *)speechbank + (n) * 0x26 + (off)))
#define SLOTFREQ (*(double *)&qword_C37C8)
#define SLOTRATE (*(double *)&qword_C37D0)

/* LoadSpeechSlot: load sample n into the speech memory right after slot n - 1 (slot 0 at the
 memory start +3B64h): its address (+0Eh), size (ReadSpeechSample, +16h), its play time size * C37C8 C37D0 less
 the bank's +3B70h (+1Eh), loaded (+22h); count the bytes (+3B6Ch) and loaded slots (+3B74h). */
void LoadSpeechSlot(int n)
{
    int addr;
    int size;
    int off;
    int t;
    char *p;

    if (n == 0) addr = BANKD(0x3B64);
    else {
        p = (n - 1) * 0x26 + (char *)speechbank;
        addr = *(int *)(p + 0xE) + *(int *)(p + 0x16);
    }
    off = n * 0x26;
    *(int *)(off + (char *)speechbank + 0xE) = addr;
    size = ReadSpeechSample(n, addr);
    *(int *)(off + (char *)speechbank + 0x16) = size;
    t = BANKD(0x3B70);
    *(int *)(off + (char *)speechbank + 0x1E) = (unsigned)((unsigned)size * SLOTFREQ / SLOTRATE) - t;
    *(int *)(off + (char *)speechbank + 0x22) = 1;
    BANKD(0x3B6C) += size;
    BANKD(0x3B74)++;
}

/* PlaceSpeechSlot: read sample n (file offset +12h, length +1Ah) from the open bank file into the
 speech memory after slot n - 1 and mark it loaded; count the bytes read (+3B6Ch) and loaded slots (+3B74h). */
void PlaceSpeechSlot(int n)
{
    unsigned got;
    unsigned len;
    int addr;
    int fh;

    if (n == 0) addr = BANKD(0x3B64);
    else addr = SLOTD(n - 1, 0xE) + SLOTD(n - 1, 0x16);
    SLOTD(n, 0xE) = addr;
    fh = BANKD(0x3B7C);
    len = SLOTD(n, 0x1A);
    lseek(fh, SLOTD(n, 0x12), 0);
    _dos_read(fh, (void *)addr, len, &got);
    SLOTD(n, 0x22) = 1;
    BANKD(0x3B6C) += got;
    BANKD(0x3B74)++;
}

/* QueueSpeechClip (84418, PC only; announcer segment 062, the same source file: it ends in ShutdownSpeech_ret6):
   append the named clip to the speech queue. A clip whose sample is in memory is queued as it is; otherwise its
   bank slot is swapped with a free slot (FindFreeSpeechSlot) and, when the sample still fits the speech memory
   (bank +3B6Ch used + its size, up to +3B68h), loaded (kind 'G': LoadSpeechSlot) or placed (PlaceSpeechSlot)
   there and that slot queued. name: the clip name. Result: 1 when queued; 0 (and the queue entry dropped) when
   the clip is unknown, no slot is free or the sample does not fit.
   DRAFT: 293 vs 289. slot is in ebp, free in ebx and the swapped slot pointer in edx (original ebx, edx, ebp);
   pos and free, and off and bank, sit in swapped stack slots; the first queue store reloads pos. See UNSURE. */
int QueueSpeechClip(char *name)
{
    int pos;
    int slot;
    int free;
    int loaded;
    SpeechSlot tmp;
    int off;
    SpeechSlot *bank;
    SpeechSlot *dst;

    pos = SPEECHQ->count++;
    slot = FindSpeechSlot(name);
    if (slot == -1) {
fail:
        SPEECHQ->count--;
        return 0;
    }
    loaded = SpeechSlotLoaded(slot);
    if (loaded) {
        SPEECHQ->clip[pos] = slot;
        return 1;
    }
    free = FindFreeSpeechSlot(loaded);
    if (free == -1) goto fail;
    off = free * 0x26;
    bank = speechbank;
    dst = (SpeechSlot *)((char *)bank + off);
    tmp = *dst;
    *dst = bank[slot];
    speechbank[slot] = tmp;
    dst = (SpeechSlot *)((char *)speechbank + off);
    if ((unsigned)(BANKD(0x3B6C) + dst->size) > (unsigned)BANKD(0x3B68)) goto fail;
    if (dst->kind == 'G') LoadSpeechSlot(free);
    else PlaceSpeechSlot(free);
    SPEECHQ->clip[pos] = free;
    return 1;
}

/* LoadClipSlot: swap the named clip's slot with a free slot when the sample still fits,
 then load it (goalie clips, type byte 'G') or place it from the bank. */
int LoadClipSlot(char *name)
{
    int slot;
    int free;
    int loaded;
    SpeechSlot tmp;
    int off;
    SpeechSlot *src;

    slot = FindSpeechSlot(name);
    if (slot == -1)
        return 0;
    loaded = SpeechSlotLoaded(slot);
    if (loaded)
        return 1;
    free = FindFreeSpeechSlot(loaded);
    if (free == -1)
        return 0;
    off = slot * 0x26;
    src = (SpeechSlot *)((char *)speechbank + off);
    if ((unsigned)(BANKD(0x3B6C) + src->size) > (unsigned)BANKD(0x3B68))
        return 0;
    tmp = speechbank[free];
    speechbank[free] = *src;
    speechbank[slot] = tmp;
    if (((unsigned char *)&speechbank[free])[0xD] == 0x47)
        LoadSpeechSlot(free);
    else
        PlaceSpeechSlot(free);
    return 1;
}

/* DRAFT (first pass from Ghidra): the EXE splits it: its entry (84AAE) sits in SayScoringPeriod, which
   tail-calls it, and the rest (SayClip_body) here after LoadClipSlot; that needs SayScoringPeriod in this file. */
/* SayClip (84AAE, PC only): say the speech clip name now: when the speech system is up (speechinit) and idle
   (SpeechIdle), clear the queue and the sample requests, reset the bank's loaded count (+3B70h), request and
   make room for the clip and queue it, then mark the queue busy (speechq +60h). Result: 1 when it was queued,
   else 0. */
int SayClip(char *name)
{
    if (!speechinit) return 0;
    if (!SpeechIdle()) return 0;
    ResetSpeechQueue();
    ResetSampleReq();
    BANKD(0x3B70) = 0;
    RequestSample(name);
    EnsureSampleRoom();
    QueueSpeechClip(name);
    *(int *)((char *)speechq + 0x60) = 1;
    return 1;
}
