/* Misc dialogs: the text input box. */
#define EditTextField EditTextField_decl
#define unknown_libname_4 unknown_libname_4_decl
#include "nhl95.h"
#undef EditTextField
#undef unknown_libname_4

int EditTextField(char *buf, int maxlen, int w, int x, int y, int (*cb)(void), int a, int b);
int unknown_libname_4(char *s);  /* Watcom CRT atoi */

/* TextInputDialog (2FEDF, PC only; owner of DlgNullCallback): ask for a line of text in a centred box: the prompt
   above a field wide enough for maxlen "M"s (at least minw), the box sized to fit both. Wait for Enter and Esc
   to be released, then edit the field (EditTextField). With numeric set the text must read as a number from lo
   to hi (digits, or a leading space or minus; atoi) or the field is asked again; with required set an empty
   field is only taken with Esc. Then wait for the keys again and restore the screen. prompt: the prompt; buf:
   the text (cleared first); maxlen: its length; minw: the field's minimum width; numeric, lo, hi, required: as
   above; arg: passed on to EditTextField. Result: the number, or EditTextField's last key. */
int TextInputDialog(char *prompt, char *buf, int maxlen, int minw, int numeric, int lo, int hi, int required,
    int arg)
{
    struct {
        char probe[0x100];
        int h;
        int ty;
        int y;
        int x;
        int w;
    } s;
    int i;
    int r;

    InputRemove();
    sub_B4BA8();
    for (i = 0; i < maxlen; i++) s.probe[i] = 'M';
    s.probe[maxlen] = 0;
    s.w = fputchar(s.probe);
    if (s.w < minw) s.w = minw;
    s.w += 0xC;
    minw = fputchar(prompt) + 0x10;
    if (minw < s.w) minw = s.w + 0x10;
    s.h = byte_D42C3 * 2 + 0x20;
    s.x = (0x280 - minw) / 2;
    s.y = (0x1E0 - s.h) / 2;
    SaveDialogBg(s.x, s.y, minw, s.h);
    SetTextColors(dlgtextfg, dlgtextbg);
    DrawBevelRect(s.x, s.y, minw, s.h, boxfillcolor, boxlitecolor, boxshadecolor);
    sub_8E9C0(dlgtextfg, dlgtextbg);
    s.ty = s.y + 7;
    PrintShadowText((minw - fputchar(prompt)) / 2 + s.x, s.ty, prompt);
    s.x += (minw - s.w) / 2;
    DrawBevelRect(s.x, s.y + 0x1F, s.w, 0x12, dlgtextbg, boxshadecolor, boxlitecolor);
    sub_8E9C0(dlgtextfg, dlgtextbg);
    while (sub_B2CBE(1) || sub_B2CBE(0x1C));
    ClearInputQueue();
    if (numeric) {
        do {
            *buf = 0;
            EditTextField(buf, maxlen, s.w - 8, s.x + 6, s.y + 0x20, DlgReturnZero, 0, arg);
            r = 0;
            for (i = 0; i < maxlen; i++) {
                if (!(byte_C4B6C[(unsigned char)(buf[i] + 1)] & 0x20) && buf[0] != ' ' && buf[0] && buf[0] != '-') break;
                r = unknown_libname_4(buf);
            }
        } while (r < lo || r > hi);
    } else {
        do {
            *buf = 0;
            r = EditTextField(buf, maxlen, s.w - 8, s.x + 6, s.y + 0x20, DlgReturnZero, 0, arg);
        } while (!(required ? *buf || r == 0x1B : 1));
    }
    while (sub_B2CBE(1) || sub_B2CBE(0x1C));
    ClearInputQueue();
    RestoreDialogBg();
    InputInstall();
    return r;
}
