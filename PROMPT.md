# Prompts

The **segment naming queue is complete** ([SEGMENT_AGENT.md](SEGMENT_AGENT.md)). Use the **C matching prompt** below for normal work. Use the segment prompt only for leftover auto/data names in a segment.

Standing rules: [AGENTS.md](AGENTS.md). Handoff: [HANDOFF.md](HANDOFF.md).

## Segment prompt (leftover auto/data names only)

Paste this into a new Copilot Agent session only when doing leftover auto or data names. One segment only. Do not edit this prompt.

```text
Read SEGMENT_AGENT.md and AGENTS.md before you edit any asm. The naming queue is complete; this session is leftover auto/data names only. Take one segment file (or the leftovers named in Current segment). Do not start unrelated segments.

Build before you edit anything. The retail HOCKEY.EXE is in the repo root (sha1 3961e0eba6b0338fad1613bb534efafd9406ed4a). Run make. It must end with "MATCH: build/HOCKEY.EXE is byte-identical to the retail HOCKEY.EXE". If HOCKEY.EXE is missing or make does not MATCH, stop and say what you ran and what it printed.

src/ is the source. There is no listing in the repo. Every instruction line ends with "; <address>", the address in the EXE. Do not disassemble HOCKEY.EXE. Do not write a disassembler. Do not run tools/gen_src.py on src/. Do not delete an asm file. Edit the segment file in place. Do not edit src/segments.txt or a section line. Do not rewrite SEGMENT_AGENT.md as a whole file. Edit the current row in place. Do not append a history entry.

Follow the rules already in SEGMENT_AGENT.md. Naming source order: if the routine matches NHLPA93Genesis, take that name; otherwise NHL94Genesis; otherwise a name from what it does (95PC-only code such as the PC season, trades and create-player screens). Match on the body, not the old name map. NHL95Genesis is a loose reference only, not a naming source.

1. The data segments (src/dseg02) are not queue segments. Data is not off limits: name every data and BSS label the current segment uses, in the dseg02 file that defines it, this session. src/inc/symbols.inc is the shared index of those names; tools/rename_symbol.py and tools/update_symbols.py keep it current. Never edit it by hand.
2. Library segments (Watcom runtime, DOS/4GW glue, EACSNDF, sound drivers, EA graphics/file/memory libraries) are not in the queue. You may name a library routine or its data when you know what it is.
3. Run python3 tools/auto_names.py <module>. For each function in the file, read its name_map_94.csv row, open the Genesis routine it points to (93G, else 94G; not 95G), and line up the instruction sequences (calls, constants, structure fields, branch shape). Keep the Genesis name only when the body is the same routine. Reject a wrong generated name with a manual-reject row in tools/manual_names.csv.
4. Rename with tools/rename_symbol.py (one name) or tools/rename_symbol.py --batch (many; local labels in address order; per-line {source_game=..,confidence=..} or {no_record} options). Give --evidence for every function. A -5r alias label folds into its base with NEW = BASE-2 (dword_E03BA regd0-2). Function entries get the Genesis name or a name from what they do. loc_ labels become NASM locals (.x exit, .loop back-branch target, .1 .2 ... else; the 93G local where the routine lines up). A loc_ another function jumps to stays global with a name. Write structure fields as src/inc/structs.inc names with the same displacement size ([byte ecx+SCnum]).
5. Do not change a byte. Keep every encoding hint (byte, dword, short, near, strict, nosplit), every LD macro line, every db fallback, every "equ $+k" and every "; <address>" comment. Add comments: the 93G comment when the routine matches, else what it does, its register arguments (eax, edx, ebx, ecx) and its result.
6. Run make. It must print MATCH. Run python3 tools/update_symbols.py --check (up to date) and python3 tools/auto_names.py <module> (0 auto names defined in the file).
7. In SEGMENT_AGENT.md, note what was named and anything left. Commit: its file, the data files and src/inc files its names touched, the other src files where a renamed global is used, tools/manual_names.csv, name_map_94.csv (repo root, so `git add src tools` misses it), tools/global_map.csv, tools/struct_fieldmap.csv, SEGMENT_AGENT.md.

Stop after 5 failed builds. Report the file, the first differing address make printed, the built byte, the retail byte, and the line you changed. Do not continue.
```

## C matching prompt

Paste this into a new session for the C phase (one batch of functions per session). This is the primary prompt. Do not edit this prompt.

```text
Read docs/C_MATCHING.md, HANDOFF.md (Current state, Next, Unsure plan) and AGENTS.md first. Build before you change anything: make must end with "MATCH: build/HOCKEY.EXE is byte-identical to the retail HOCKEY.EXE" and make cinfo must say CBUILD=1 (DOSBox is needed; Watcom files are in tools/watcom). If not, stop and say what you ran and what it printed.

1. Pick compiled-C functions (prologue push dword N / call __CHK) from C_PROGRESS.md's segments and HANDOFF Next / Unsure plan, starting with Group 2 unblocks and small untouched leaves. Skip hand-written asm (no __CHK) and parked Group 1 register near-misses. Check for shared tails and fall-through tail calls; such a group goes into one file. Timebox about 8-10 minutes per stubborn function; leave a draft and note what was tried in tools/c_functions.csv / docs/UNSURE.md.
2. Write src/c/<segment>/<Func>.c: #include "nhl95.h", the asm label names, structs.inc fields, and a name from NHLPA93Genesis, else NHL94Genesis, else from behaviour. NHL95Genesis is a loose reference only, not a naming source. A comment on every function (address, Genesis routine or PC only, what it does, arguments, result) with the asm comments brought over. Add its prototype to src/c/include/protos.h; constants to consts.h, typed globals to vars.h, then python3 tools/gen_cheaders.py.
3. Iterate with python3 tools/cdiff.py <file> until it prints MATCH. Do not use compiler flags other than the defaults without writing down why in docs/C_MATCHING.md.
4. python3 tools/cc.py mark <file> [--end Label]. make must MATCH, and make CBUILD=0 must MATCH too. Do not edit the asm inside the marker.
5. python3 tools/c_progress.py; put the reason for any non-matching draft in the notes column of tools/c_functions.csv. Record new compiler learnings in docs/C_MATCHING.md. Update HANDOFF Current state / Next if the headline numbers or Next item change.
6. Commit: the C files, src/c/include, the marked asm files, C_PROGRESS.md, tools/c_functions.csv. Never commit objects, .inc fragments or listings. The retail HOCKEY.EXE and the three Watcom files under tools/watcom/ stay committed. Never force-push.

Stop after 5 failed builds and report the file, the first differing address, and the change.
```
