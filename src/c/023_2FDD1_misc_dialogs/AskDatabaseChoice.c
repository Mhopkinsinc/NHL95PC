/* Misc dialogs: database choice. */
#include "nhl95.h"

/* AskDatabaseChoice: ask which database to use (original or edited) at the mouse position; sets dbextension
 to.ORG for answer 1, else.DB. Returns the answer. */
int AskDatabaseChoice(void)
{
    int r;
    int y;
    int x;
    int k;

    sub_B2DCA(&r, &y, &x);
    k = MessageBox(-1, -1, (char *)dbchoicelines, 3, (int)dbchoicebtns, 2, (int)&y, (int)&x, -1);
    r = k;
    if (k == 1) dbextension = (int)str_ORG;
    else dbextension = (int)str_extDB;
    return r + (k - k);             /* k kept live to the end (folded away): the EXE's register picks */
}
