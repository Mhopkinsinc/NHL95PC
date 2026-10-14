/* Engine physics AI: offside banner. */
/* DRAFT: 424 vs 424. The far-side player test is cmp ecx,esi / jg; the original is
 cmp esi,ecx / jl. Do not retry: writing the limit on the left of < hoists that lea
 out of the loop. */
#define GetLowestPen GetLowestPen_decl
#include "nhl95.h"
#undef GetLowestPen

/* checkob: with offsides on, and not during a penalty shot, watch the six opponents.
 When one is past the puck on p's attacking side (4 bytes deeper once p has crossed
 -52h or 52h and is not skating back), set gmode2 bit 7 and, if the banner is free
 and the puck is still in p's half, show banner 4. Clearing that bit drops banner 4
 once its timer has run out. */
void checkob(Player *p)
{
    short found;
    int extra;
    Player *o;
    signed char n;
    int y;

    found = 0;
    extra = 0;
    if (!gameopts.offsides) return;
    if (penshotlive) return;
    if (penshotstart) return;
    o = &SortCords[p->SCnum < 6 ? 0 : 6];
    if ((p->pflags & pfgoal) == 0) {
        y = p->Ypos >> 16;
        if (y > -0x52 && p->Yvel <= 0) extra = 4;
        if (-(extra + 0x4E) <= *pucky) {
            for (n = 0; n < 6; o++, n++) {
                if (o->position >= 0 && -(extra + 0x4E) > (o->Ypos >> 16)) {
                    found = 1;
                    break;
                }
            }
        }
    } else {
        if (HIWORD(p->Ypos) < 0x52 && p->Yvel >= 0) extra = 4;
        if (extra + 0x4E >= *pucky) {
            for (n = 0; n < 6; o++, n++) {
                if (o->position >= 0 && (o->Ypos >> 16) > extra + 0x4E) {
                    found = 1;
                    break;
                }
            }
        }
    }
    if (found) {
        if (gmode2 & 0x80) return;
        gmode2 |= 0x80;
        if (bannermsg >= 4) return;
        if (((p->pflags & pfgoal) != 0) ^ (*pucky < 0)) {
            bannermsg = 4;
            bannertimer = 0;
        }
        return;
    }
    if ((gmode2 & 0x80) == 0) return;
    gmode2 &= 0x7F;
    if (bannermsg != 4) return;
    if (bannertimer != 0) return;
    bannermsg = -1;
}

/* SkillForAnim: the rating byte that decides p's current save move animation (SPA): the left right pairs
 passacc aggress (SPA C9h, E1h, 11D5h; B1h the other way round) and spodds endurance (119h; 101h and
 121Dh the other way round) are picked by handedness; any other animation uses shotspd.
 It sits between checkob and GetLowestPen so GetLowestPen's jump back to checkob's pops is a near jmp. */
int SkillForAnim(Player *p)
{
    switch (p->SPA) {
    case 0xC9:
    case 0xE1:
    case 0x11D5:
        return !p->handed ? p->passacc : p->aggress;
    case 0xB1:
        return !p->handed ? p->aggress : p->passacc;
    case 0x119:
        return !p->handed ? p->endurance : p->spodds;
    case 0x101:
    case 0x121D:
        return !p->handed ? p->spodds : p->endurance;
    default:
        return p->shotspd;
    }
}

/* DRAFT: 166 vs 166. The trail step is sub eax,ecx / cwde / add ebx,eax; the original
 has no cwde. The power-play return is lea eax,[ebx+ecx]; the original is
 mov eax,ebx / add eax,ecx. Do not retry: a named int for the step pushes ebp,
 an unsigned short store is and eax,0FFFFh, and pen-prev as an expression moves
 the box pointer from edx to eax. */
int GetLowestPen(void)
{
    Team *lead;
    Team *trail;
    short diff;
    short idx;
    short pen;
    int sum;
    int prev;
    signed char *box;

    diff = hmtmap[0] - awtmap;
    if (diff > 0) {
        lead = &hmtmstruct;
        trail = &awtmstruct;
    } else if (diff < 0) {
        lead = &awtmstruct;
        trail = &hmtmstruct;
    } else {
        return 0;
    }
    prev = 0;
    sum = 0;
    box = trail->tmpbox;
    for (;;) {
        idx = *box++;
        if (idx < 0)
            break;
        pen = trail->tmpdst[idx];
        if (pen < 0)
            continue;
        pen &= 0x3FFF;
        pen -= prev;
        sum += pen;
        prev = pen;
    }
    if (lead->tmap != 6)
        goto box2;
    return sum;
box2:
    sum -= prev;
    box = lead->tmpbox;
    for (;;) {
        idx = *box++;
        if (idx < 0)
            return sum;
        pen = lead->tmpdst[idx];
        if (pen < 0)
            continue;
        pen &= 0x3FFF;
        if ((short)sum < pen)
            return sum;
        return sum + prev;
    }
}
