/* Engine sound interface: digital samples. */
#include "nhl95.h"

/* DRAFT: 4 bytes short. The EXE moves the unused argument into edx at entry (push ebx / push edx / mov edx, eax)
   and so loads digihandle into ebx: something kept the argument alive through the register allocator and was
   dropped after it. Not it: (void) / self-assignment / dead stores / u++ / u = 0 after the body, a dead
   if (dbg) call with it (keeps dbg in ebx too). */
/* FreeDigiSample: free the digital sample (digihandle) unless it has already finished, and mark no sample. The
 argument is not used. */
void FreeDigiSample(int unused)
{
    if (digihandle != -1 && !sub_8F80E(digihandle)) {
        sub_8F7AE(digihandle);
        digihandle = -1;
    }
}
