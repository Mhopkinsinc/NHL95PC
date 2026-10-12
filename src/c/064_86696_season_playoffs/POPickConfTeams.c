/* Season playoffs: fill one conference's seed slots. */
/* DRAFT: 361 vs 361. count stays in esi and the bitmask in edi (`mov esi, ecx` / `xor edi, edi`).
 The original is `mov edi, ecx` / `xor esi, esi`. The division compare then loads pick[1] into edx and
 pick[0] into eax (`cmp eax, [edx*4+teamdivflags]`); the original loads pick[1] into eax and pick[0]
 into edx. The pick swap uses ebx; the original uses esi. The final loop hoists slots into ecx and
 uses edx as the write index. The original is `xor ebx, ebx` / `xor esi, esi` / `jmp` to `cmp ebx, 10h`
 and `add eax, [esp+4]`. Do not retry: declaring i before used, a cnt local before or after used, and
 `used = 0` before `cnt = count` did not swap esi and edi. Without the cnt local the scan from i = 2
 matches, because esi is free once the bitmask lives there. */
#include "nhl95.h"

/* POPickConfTeams: slots holds the 16 seed dwords. confflag is 3 for the west and 0Ch for the
 east; the empty slots are -confflag. first and count select confteams[first .. first+count).
 HomeTeam and VisTeam are skipped when their division flags overlap confflag. Eight distinct
 indexes are drawn from the rest. If the first two are the same division, the second is swapped
 with a later pick from another division. Those teams replace the -confflag placeholders. */
void POPickConfTeams(int *slots, int confflag, int first, int count)
{
    int pick[8];
    int used;
    int i;

    used = 0;
    if (teamdivflags[HomeTeam] & confflag) {
        for (i = 0; i < count; i++)
            if (HomeTeam == confteams[i + first]) break;
        used |= 1 << i;
    }
    if (teamdivflags[VisTeam] & confflag) {
        for (i = 0; i < count; i++)
            if (VisTeam == confteams[i + first]) break;
        used |= 1 << i;
    }
    for (i = 0; i < 8; i++) {
        pick[i] = randomd0(count);
        while (used & (1 << pick[i]))
            pick[i] = randomd0(count);
        used |= 1 << pick[i];
    }
    if (teamdivflags[confteams[pick[0]]] == teamdivflags[confteams[pick[1]]]) {
        for (i = 2; teamdivflags[confteams[pick[0]]] == teamdivflags[confteams[pick[i]]]; i++)
            ;
        {
            int t;
            t = pick[1];
            pick[1] = pick[i];
            pick[i] = t;
        }
    }
    {
        int n;
        n = 0;
        for (i = 0; i < 16; i++) {
            if (slots[i] == -confflag) {
                slots[i] = confteams[pick[n] + first];
                n++;
            }
        }
    }
}
