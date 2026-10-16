# Unsure

Moved verbatim from HANDOFF.md. Triage plan: [HANDOFF.md](../HANDOFF.md) (Unsure plan).

### Unsure

Left unnamed or capped. Do not retry the listed C shapes.

* `byte_D42C3` is the line height `DrawTradeRow` multiplies by the cursor row (`y = byte_D42C3 * row + 0x1D`). SEGMENT_AGENT calls it font height. No Genesis store was confirmed this pass, so the auto name stays.
* LoadShapeByTag: 392 vs 392. The index, the directory byte count, and the shape pointer stay in edi; the file position stays in esi. The original uses esi for the index and the shape pointer, and edi for the file position. The width*height size is `imul ebp, eax` / `add ebp, 11h`. The original is `imul eax, edx` / `lea ebp, [eax+11h]`. Declaring pos before bytes, declaring i first, naming the product, and adding 11h through a char pointer did not change either. A memcpy/memcmp pragma that clobbers edi spilled dir (407 bytes). Do not retry those.
* POPickConfTeams: 361 vs 361. count stays in esi and the bitmask in edi (`mov esi, ecx` / `xor edi, edi`). The original is `mov edi, ecx` / `xor esi, esi`. The division compare then loads pick[1] into edx and pick[0] into eax. The pick swap uses ebx. The final loop hoists slots into ecx and uses edx as the write index. Declaring i before used, a cnt local before or after used, and `used = 0` before `cnt = count` did not swap esi and edi. Do not retry those.
* BlitClipRect: x in esi, y in edi. Five shapes. Do not retry.
* SetCoachMode: away compare is `movsx ebx, ax` vs `cwde`.
* FadeOutPalCycle: `mov bh, bl; dec bh` vs `dec bl`.
* QueueGoalieNote: tail keeps 1 in esi. `-od`, ternary, and `side - side + 1` do not emit two immediates.
* ChkDelayedOffside: `je` to the shared `xor eax, eax` vs `jne` / `xor eax, edx`. Do not invert the compare.
* getlchoice: `tm` must stay in edx across the tmap subtract. Declaration order and inlining did not.
* faceoffinput: `pflags2` in al vs cl.
* SetShotMode: tail copies `vtoa` into edx before the `regd0` store. A short local did not stop it.
* BlitJerseyDigit: colour store and the right-edge load are swapped. `volatile` and a goto did not.
* StubRet4b: `push 10h; call __CHK; ret 4`. An unused array is dropped; `-od` spills the frame.
* LeagueSettingsDlg.c (five functions): bodies match; Watcom anchors the shared pops on MenuPlayoffSettings. Reordering the functions did not flip that.
* LoadCupFinalSeries: `db = db + 2` is `add [esp], 2`. The original loads, adds, and stores.
* ClearPanelPenalties: the zero is hoisted into si. An inner `short z = 0` was still hoisted.
* MenuP1ControlsInGame: ControlsDlgInGame is split in the EXE (it begins in the middle of MenuP1ControlsInGame), so those two cannot be marked. MenuP2ControlsInGame is marked on its own.
* SetGameSides: `g * 6` is shifted in eax (`mov eax, ebx`) and the original shifts edx. Declaring `o` first did not change it. `games += g * 6` kept the pointer in edx but widened the byte with `movzx edi` and an extra `push edi` (103 vs 106). Do not retry those.
* CmpShootPct: the season record is `mov edx, [statsskaterbuf]; add eax, edx` (277). C copies the index first (`mov edx, eax; mov eax, [buf]; add eax, edx`, 278). Operand order and an if/else did not flip that add.
* CmpTeamStandings: the body matches. The tail is `add esp, 0Ch; jmp CmpTeamPK_x` (336) and C inlines `pop ebp` .. `ret` (337). It needs CmpTeamScoring, CmpTeamDefense, CmpTeamPP and CmpTeamPK in the same file.
* CmpRosterSkaters: season `*a * 0x28` is `add ebx, eax; shl ebx, 3; mov eax, [rosterpstat]; add eax, ebx` (262). C keeps the product in eax and folds `add eax, [rosterpstat]` (260). A named `n += (n << 2)`, declaration order, and a comma expression did not put `*a` in ebx.
* DrawScrollBar: x is in ebp and y in edi (207). The original has x in edi and y in ebp (209). Declaring x before y did not swap them.
* DrawEditCursor: y is `movzx edi, [byte_D42C5]` (229). C zero-extends the byte into edx (230). Computing y before h, and reusing the width local, did not select edi.
* assepen: the penalty flags are `mov cl, dl; or cl, 4; mov ch, cl; and ch, 0xED` (255). C does `or dl, 4` in place (239). A separate `signed char` for the or-then-and still updated dl in place.
* ChkTwoLinePass: same size. `lasttouch` stays in si and the side test is `xor edx, ebx`. C keeps `lasttouch` in bx and compares. Writing `^` instead of `!=` dropped the esi save (190 vs 195). Do not retry that.
* CtlSwapScoreFix: same size. The puck byte is `mov ch, [ebx]` with gmode still in cl. Adjacent char locals stayed in bl. A short's high byte, and `c = f` before the load, paired bh/bl and moved gmode out of cl. Do not retry those.
* OneTimerChk: the y-sign stays in eax and the goal flag is `setne dl` (`and eax, 0FFh` first). Swapping the xor operands tested the flag first. Do not retry that.
* SetTeamGoalie: `side + 1` is the same 204 bytes. The widened side stays in ecx and the address copy is ebx; the original widens into ebx and copies to ecx. `int s = side; s++` saved g into ebx (206). A team pointer dropped the recomputed address (167). Do not retry those.
* ChkGoalies: declaring the other team first left that pointer in edx (original uses esi). Do not retry declaration order.
* MenuLeagueSettings: the body matches. The exit is `mov eax, esi; jmp MainDeskLoop_x2` in the main-desk segment. `return ((int (*)(int))MainDeskLoop_x2)(r)` is a `call` plus the inlined pops. Do not retry the cast.
* periodicevents: `(signed char)--lldisp` makes `test dl, dl` match. The add is `add al, 18h` (2 bytes) and the original is `add bl, 18h` (3). A second byte local copied into al before the test. Do not retry those.
* sfx: `dword_CCC98` is set by SayOneMinuteLeft when that clip is queued. `dword_ED7A4` is the sample for effect AAh and `dword_D2427` is the other argument to sub_8F270. No Genesis names were confirmed, so the auto names stay.
* CopyFile: same size. srcext is parked in eax; the original parks srcdir (`mov eax, ecx`). A local for srcdir did not change the shuffle. Do not retry that.
* DrawMenuItemNorm: x is in ebp and y in edi (original edi, ebp). The exit is inlined; the original jumps to DrawMenuItemSel_ret.
* DrawListItems: the compiled bytes match (95). It branches to DrawListItem+DEh, so it cannot be marked until DrawListItem is.
* unk13_722FE is 13 zero bytes between InputDialog and LoadDbDialogShapes. OpenSelectedDb copies them as the empty directory. A struct copy reloaded ds from cs; an inlined movsd/movsb with the destination argument last matches. No Genesis name was confirmed.
* MenuHitTest: same size. The item pointer is in edx and the origin pointer in eax (original item in eax, origin in edx). Writing the item on the left of the add loaded x2 before x1. Do not retry that add order.
* SimPlayoffFinal: same size. The team index is bl (original cl) and `i + 1` is `lea ecx, [eax+1]` (original `inc eax`). A while loop moved the compare above the index load. Do not retry the while.
* FadePalette: 224 vs 227. The clamp compares steps in ebx; the original reloads the stack home into ecx. A volatile parameter, a local copy, and an inlined `mov ecx, [esp]` rewrote the ternary. Do not retry those.
* BuildFreeAgentList: same 249 bytes. The key offset `s * 34h` is in ebp; the original uses edi, so the two falist reloads that also need cl use edi instead of ebp. Declaration order, a pointer cast, and a falist pointer local did not swap them. Do not retry those.
* assshoot: 242 vs 248. The pfna clear is `and bl, 0FDh`; the original copies bl to ah first. The word at player+4Ch is counted in ax (original di), so edi is not saved. A separate byte local moved the and into al. Do not retry that.
* TradeHitTest: 247 vs 252. out is spilled. x is in edi and y in ebp (original esi, edi), so the button edge is in esi and y+28h is an add. Declaring the edge first or last did not swap x and y. Do not retry that.
* EasnGoalieStatsScreen: 467 vs 474. The page matches through the first embpal2 load. After the menu, the gamemode==1 arm stays beside the test (`je` short) and the fileoncd test is `je` to the drive load. The original `je near` lands after the ret, the 6Ah test is `jne` to `xor edx` after the ret, and the 8Eh arm is only a `jmp` to that `jne`. A flipped compare, a duplicated MakePath, and gotos did not move it. Do not retry those.
* CalStandingsScreen: same 279 bytes. dword_C65AC is tested in edx and copied to ecx for MakePath. The original tests it in ecx and that zero is the extension. Same split as GameStandingsScreen. Do not retry one MakePath per branch.
* LoadJerseyColours: 348 vs 352. The palette pointer is in ebx (write it back into the dst parameter) and both copy loops match. The remap load is `mov dl,[ebx+eax]`; the original is `lea edx,[ebx+eax]` / `mov cl,[edx]`, and the else arm is `add dl,8` rather than `mov dl,cl` / `add dl,8`. A named team index, volatile p, and pragma aux byte loads did not produce that lea without a spill or a hoisted `mov dl,cl`. Do not retry those.
* BuildTeamRosterList: 338 vs 344. Team stays in eax through the index loop; the original is `mov esi, eax` and `mov eax, esi` just before TeamRecPtr. The dressed byte is `mov al, [ebx+eax+0E4h]`; the original adds the index into edx and loads ah. A named team was eliminated. An aux returning esi reloaded eax at once. Naming the fill byte lengthened the copies. Do not retry those.
* BuildTradeRoster: 473 vs 473. The fill loop keeps the index in ebx and stores it through al; the original keeps the index in eax and reloads the byte into dl. A separate unsigned char is the same pair. Storing the index directly reloads eax (477 bytes). An aux that reloads dl parks list in edi; passing list in esi pushes the incoming args and shifts the frame. Do not retry those.
* checkwallcoll: 470 vs 481. The prologue, both radius loads, and the y test match. xlim reloads in edx (original ecx). The x test negates that limit in ecx and compares dx with cx; the original negates it in edx, reloads x, and compares with regd4 in memory. Non-volatile limits sign-extend the radii and spill y. Declaring ylim before xlim swaps the two slots. Do not retry those.
* SplitPlayerName: same 323 bytes. stop, first and last sit at esp+0/+4/+8 once they are one struct. The name pointer is in edi (original ecx), so the copy count stays in ebx (original edx). An aux that forced ecx reloaded eax immediately. Do not retry that.
* SetRosterTeamMenus: 293 vs 292. d*7 is hoisted into ebx, so ebx is pushed. Writing the index as `(d << 3) - d + i` inside the loop put the team byte in bl and dropped to 288. Do not retry that flat index.

