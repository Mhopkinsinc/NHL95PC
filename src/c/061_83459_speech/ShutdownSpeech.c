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
