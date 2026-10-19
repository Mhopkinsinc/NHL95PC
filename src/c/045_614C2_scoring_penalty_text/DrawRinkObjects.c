/* Scoring penalty text: draw the rink objects. */
/* DRAFT: same size, all but 4 instructions (two index temps); see docs/UNSURE.md. Its match belongs in
   DecayCrowdLevel.c, whose sfxslots_popx tail it uses. */
#include "nhl95.h"

/* one rink-object slot, 12 bytes at word_DEE94 (DecayCrowdLevel animates them) */
typedef struct RinkObj {
    short id;           /* photo id, -1 free */
    signed char timer;
    signed char step;
    signed char frame;  /* photoptrsf index */
    short x;
    short y;
    unsigned char pad[3];
} RinkObj;

typedef struct { short x, y; } Pt;

/* DrawRinkObjects (61862, PC only): draw the live rink objects of slots 0-11h (photoptrsf[frame] at x, y; the
   DrawSprite flag is x > C0h). Then slots 13h and 12h (i = 1, 0): a free slot or a frame over 64h first gets
   the dword_E0220 shape at x 4, y from dword_CD2F8[i], then the live frame is drawn; slot 12h adds the
   dword_E0230 shape, slot 13h the penalty-box door (sortobj15, DrawFrameSprite) when gmode bit 7 is set and its
   frame is 362h or more. No arguments, no result. */
void DrawRinkObjects(void)
{
    RinkObj *o;
    short i;
    short x;
    short y;
    short id;

    o = (RinkObj *)word_DEE94;
    for (i = 0; i < 0x12; i++, o++) {
        if (o->id >= 0) DrawSprite(photoptrsf[o->frame], o->x, o->y, (short)(o->x > 0xC0), -1, 0);
    }
    o++;
    for (i = 1; i >= 0; i--, o--) {
        if (o->id < 0) id = -1;
        else id = o->frame;
        x = o->x;
        y = o->y;
        if (id > 0x64 || o->id < 0) DrawSprite(dword_E0220, 4, ((Pt *)dword_CD2F8)[i].y, 0, i, 0);
        if (id >= 0) DrawSprite(photoptrsf[id], x, y, 0, i, 0);
        if (i == 0) DrawSprite(dword_E0230, 4, ((Pt *)dword_CD2F8)[i].y, 0, -1, 0);
        else if ((gmode & 0x80) && ((Player *)sortobj15)->frame >= 0x362)
            DrawFrameSprite(((Player *)sortobj15)->frame, HIWORD(((Player *)sortobj15)->Xpos),
                HIWORD(((Player *)sortobj15)->Ypos), 0, 0);
    }
}
