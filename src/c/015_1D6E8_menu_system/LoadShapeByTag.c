/* Menu system: load one shape out of a tagged shape file. */
/* DRAFT: 392 vs 392. The index, the directory byte count, and the shape pointer stay in edi.
 The file position stays in esi. The original uses esi for the index and the shape pointer, and edi
 for the file position. The width*height size is `mov eax, [esp+6]` / `sar eax, 10h` / `mov ebp, [esp+8]`
 / `sar ebp, 10h` / `imul ebp, eax` / `add ebp, 11h`. The original is `mov edx, [esp+6]` / `sar edx, 10h`
 / `mov eax, [esp+8]` / `sar eax, 10h` / `imul eax, edx` / `lea ebp, [eax+11h]`. Do not retry: declaring
 pos before bytes, declaring i first, naming the product, and adding 11h through a char pointer did not
 change either difference. A memcpy/memcmp pragma that clobbers edi spilled dir and grew the frame to 407. */
#include "nhl95.h"

/* LoadShapeByTag: open path and return the shape whose 4-byte tag matches, or 0.
 The file has a short count at offset 4, then at offset 6 a directory of count*8 bytes:
 count tags followed by count dword offsets. The shape itself starts at
 count*8 + 6 + offset. Its 17-byte header holds width at +4 and height at +6, and the
 allocated block is width*height + 11h (the header plus the pixels). The directory is
 allocated as "sfh" and the shape as "shape". A failed read of the shape frees it. */
int LoadShapeByTag(char *path, char *tag)
{
    int dir;
    int bytes;
    int pos;
    int i;
    int off;
    int base;
    int p;
    int n;
    struct {
        char hdr[0x14];
        char tagbuf[4];
        int handle;
        int count;
        int found;
        int shape;
        int err;
    } s;

    s.handle = -1;
    s.shape = dir = 0;
    s.err = FileOpenRead(path, &s.handle);
    if (s.err == 0) {
        s.err = FileReadAt(s.handle, &s.count, 4, 2);
        *(short *)((char *)&s.count + 2) = 0;
    }
    if (s.err == 0) {
        bytes = s.count << 3;
        dir = sub_8CCA8((char *)str_sfh, bytes, 0x20);
        s.err = FileReadAt(s.handle, (void *)dir, 6, bytes);
    }
    if (s.err == 0) {
        s.found = 0;
        for (i = 0; i < s.count && s.found == 0; i++) {
            off = i << 2;
            memcpy(s.tagbuf, (char *)dir + off, 4);
            if (memcmp(s.tagbuf, tag, 4) == 0) {
                s.found = -1;
                base = s.count << 2;
                base += dir;
                pos = s.count << 3;
                pos += 6;
                pos += *(int *)(base + off);
                s.err = FileReadAt(s.handle, &s, pos, 0x11);
            }
        }
    }
    if (dir) FreeMemBlock(dir);
    if (s.err == 0 && s.found != 0) {
        n = ((short *)s.hdr)[2] * ((short *)s.hdr)[3] + 0x11;
        p = sub_8CCA8((char *)str_shape, n, 0x20);
        s.shape = p;
        if (FileReadAt(s.handle, (void *)p, pos, n) < 0) {
            FreeMemBlock(p);
            s.shape = p = 0;
        }
    }
    FileClose(&s.handle);
    return s.shape;
}
