/* Joystick calibration: the calibration screen. */
#define JoystickCalScreen JoystickCalScreen_decl
#define sub_B340B sub_B340B_decl
#define sub_B39D0 sub_B39D0_decl
#include "nhl95.h"
#undef JoystickCalScreen
#undef sub_B340B
#undef sub_B39D0

int __cdecl sub_B340B(int buttons);
int sub_B39D0(void);

#pragma aux copy17 = "movsd" "movsd" "movsd" "movsd" "movsb" parm [esi] [edi] modify [edi esi];
void copy17(void *src, void *dst);

/* DRAFT: 711 vs 713 (best of 5). The abort test falls through into the screen, and the
 frame (pal, text state, bank, shape, cur) matches. The side-art call is still two calls:
 the arms reload the bank into ebp and edi, so Watcom does not merge them, and sub_8E83C's
 return is stored without mov esi, eax before add esp, 8. A bank register plus duplicated
 side calls did merge that call (mov esi, eax / add esp / store) but then skipped the puck
 store at [esp+344h] and pushed 0 after the width and height reads (698 vs 713). Do not
 retry an abort-first if, a ternary that puts the side string in eax, or a bank register
 used for every sub_B30B4.
 JoystickCalScreen: calibrate one stick. mask is the joyenablemask bit, shift selects that
 stick in the pad word, and side picks the right-hand art when it is not 0. The title is the
 caller's string; this screen does not print it. A button already held aborts and clears the
 bit. Otherwise the puck follows the stick until a button or cancel, then the colours fade
 back. Returns 0. */
int __cdecl JoystickCalScreen(int mask, int shift, int side, char *title)
{
    struct {
        unsigned char pal[0x300];
        char state[0x40];
        int bank;
        int shape;
        int cur;
    } s;
    unsigned char *puck;
    unsigned char *back;
    int prev;
    int h;
    int w;

    (void)title;
    joycalactive = 1;
    ((void (*)(void))joycalprehook)();
    if (side) sub_B387E();
    else sub_B384E();
    sub_8E9C0(0, 0);
    if (((sub_B3464() >> shift) & 0x30) == 0) {
        sub_8E9E8(s.state);
        sub_8EA18((int)unk_D45D8);
        sub_B4BA8();
        sub_B392C(0);
        memset(s.pal, 0, 0x300);
        sub_B4B88(0, 0x100, s.pal);
        s.bank = sub_8E83C((char *)str_Joycal, 0);
        sub_9121C(sub_B30B4(s.bank, (char *)str_Scrn3));
        if (side) s.shape = sub_B30B4(s.bank, (char *)str_Rite);
        else s.shape = sub_B30B4(s.bank, (char *)str_Left);
        sub_913B4(s.shape);
        memcpy(s.pal, (unsigned char *)sub_B30B4(s.bank, (char *)str_Pal24) + 0x10, 0x300);
        puck = (unsigned char *)sub_B30B4(s.bank, (char *)str_Puck);
        h = *(int *)(puck + 2) >> 16;
        w = *(int *)(puck + 4) >> 16;
        back = (unsigned char *)sub_8CCA8((char *)str_PuckBack, w * h + 0x11, 0);
        copy17(puck, back);
        FadePalStep(0, s.pal, 0x10);
        prev = -1;
        for (;;) {
            if (sub_B39D0()) {
                *(int *)&joyenablemask &= ~mask;
                break;
            }
            s.cur = sub_B3464() >> shift;
            if (s.cur & 0x30) {
                *(int *)&joyenablemask &= ~mask;
                break;
            }
            s.cur = sub_B340B(s.cur);
            if (s.cur == prev) continue;
            sub_B4C61();
            if (prev != -1) sub_910E0((int)back, joycalx[prev], joycaly[prev]);
            prev = s.cur;
            sub_91400((int)back, joycalx[prev], joycaly[prev]);
            sub_91370(s.shape, joycalx[prev], joycaly[prev]);
            sub_91964((char *)unk_C27F5, 0, 0x14);
        }
        FreeMemBlock((int)back);
        FreeMemBlock(s.bank);
        FadePalStep(1, s.pal, 0x10);
        sub_8EA00(s.state);
    } else
        *(int *)&joyenablemask &= ~mask;
    ((void (*)(void))joycalposthook)();
    joycalactive = 0;
    return 0;
}
