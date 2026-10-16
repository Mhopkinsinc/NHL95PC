# Prompts

## C matching prompt

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
