# Prompts

## C matching prompt (primary)

Paste this into a new session for the C phase. Do not edit this prompt.

```text
You're working on the NHL 95 PC (DOS) decompilation in this repo: matching C compiled with Watcom 10.0 wcc386. First, install nasm, make and DOSBox if they're missing, then confirm `python3 tools/rebuild_exe.py verify` prints MATCH and `python3 tools/cc.py check` passes. Read AGENTS.md, HANDOFF.md, docs/C_MATCHING.md and docs/UNSURE.md before doing anything else.

Order of work:
1. The "Group 2" items in HANDOFF.md's Unsure plan, which are blocked only by file layout or tooling: the CmpTeamStandings family in one file, DrawListItem then DrawListItems, DrawMenuItemNorm next to DrawMenuItemSel, and the cdiff.py --func cross-segment lookup for TickPanelPenalties and DrawEnergyBar.
2. Then untouched functions, starting from HANDOFF.md's "Next" (chk4lc into checkob.c).
3. Leave the register-choice near misses ("Group 1") parked. Never retry a shape already listed in docs/UNSURE.md.

Rules:
- Every commit must build MATCH with `make` and `make CBUILD=0`. Main only takes byte-identical changes.
- Make small commits, one per function or batch. Never force-push.
- Timebox each function to about 8-10 minutes. If it doesn't match, keep the draft, add an Unsure entry saying exactly what you tried, and move on.
- Names: 93G first, then 94G, else a name from behaviour, with the evidence in the comment. Batch renames with tools/rename_symbol.py --batch.
- Every C function gets a comment: address, Genesis routine (or PC only), what it does, its arguments and its result.
- At the end of each batch, run `python3 tools/c_progress.py`, update HANDOFF.md's current state, next steps and Unsure sections, commit and push.
```

## Segment prompt (secondary — leftover auto/data names only)

The segment naming queue is complete ([SEGMENT_AGENT.md](SEGMENT_AGENT.md)). Use this prompt only for leftover auto/data names in a segment. Standing rules: [AGENTS.md](AGENTS.md).

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
