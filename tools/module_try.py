#!/usr/bin/env python3
"""module_try.py - compile a segment's C functions as one translation unit, as the original module was.

  python3 tools/module_try.py SEGMENT_STEM [--only F1,F2,...] [--keep build/module/<stem>.c]

Watcom merges identical function tails across a whole source file, and which copy of a shared exit survives
depends on every function in the file (docs/C_MATCHING.md, shared exits). The original source files were about one
per segment, but a function's C file here holds only part of its segment, so a shared exit can land in another
owner than in the EXE (POSeedRound2_x, RunDemoGame, PickAwardWinners). This takes every C function defined for the
segment (from all of src/c/<stem>/*.c, or the --only ones), in address order, with the files' preambles merged
(includes, prototypes, pragmas, types; a typedef or struct defined twice is kept once), compiles that one file and
compares each function with the EXE. It prints, per function, the result in its own file and in the module file:
a function that matches only in the module file needs the functions between it and its exit's owner together in
one C file (in address order) to match."""
import argparse, glob, os, re, sys, tempfile, shutil
HERE = os.path.dirname(os.path.abspath(__file__)); ROOT = os.path.dirname(HERE)
sys.path.insert(0, HERE)
import cc, permute


def split_file(path):
    """(preamble lines, [(name, chunk lines)]): each chunk is a function definition with the comment before it"""
    lines = open(path).read().split('\n')
    defs = []
    for k, l in enumerate(lines):
        m = re.match(r'^[A-Za-z_][\w \t\*]*?\b([A-Za-z_]\w*)\s*\([^;{]*\)\s*$', l)
        if m and k + 1 < len(lines) and lines[k + 1].strip() == '{' and m.group(1) not in ('if', 'while', 'for', 'switch'):
            defs.append((k, m.group(1)))
    chunks = []; pre_end = defs[0][0] if defs else len(lines)
    for i, (k, name) in enumerate(defs):
        s = k
        while s > 0 and (lines[s - 1].startswith('/*') or lines[s - 1].startswith(' ') and chunks_comment(lines, s - 1)):
            s -= 1
        b0, b1 = permute.body(lines, name)
        if i == 0: pre_end = s
        chunks.append((name, lines[s:b1 + 1], s, b1))
    pre = lines[:pre_end]
    # the lines between chunks that are not comments of the next one (prototypes, pragmas) go to the preamble too
    for (n1, c1, s1, e1), (n2, c2, s2, e2) in zip(chunks, chunks[1:]):
        pre += [l for l in lines[e1 + 1:s2] if l.strip() and not l.startswith(('/*', ' '))]
    return pre, [(n, c) for n, c, s, e in chunks]


def chunks_comment(lines, k):
    """line k is inside a /* ... */ comment block that ends right before a definition"""
    j = k
    while j >= 0 and not lines[j].startswith('/*'):
        if '*/' in lines[j] and j != k: return False
        j -= 1
    return j >= 0


PROTO = re.compile(r'^(?:extern\s+)?[A-Za-z_][\w\s\*]*?\b([A-Za-z_]\w*)\s*\([^;{]*\)\s*;')
EXTERN = re.compile(r'^extern\b[^(]*?\b([A-Za-z_]\w*)\s*(?:\[[^\]]*\]\s*)*;')
PRAGMA = re.compile(r'^#\s*pragma\s+aux\s+(\w+)')


def merge_pre(pres, sigs=None):
    """the preambles merged: lines kept once, a typedef / struct defined twice kept once, one prototype / extern /
    pragma aux per name (the first; for a function defined in the module, its definition's signature: sigs)"""
    sigs = sigs or {}
    out, seen, names, decl = [], set(), set(), set(sigs)
    for pre in pres:
        pre = re.sub(r'/\*.*?\*/', '', '\n'.join(pre), flags=re.S).split('\n')
        k = 0
        while k < len(pre):
            l = pre[k]
            m = re.match(r'^typedef\s+struct\s*(\w*)\s*\{\s*$|^struct\s+(\w+)\s*\{\s*$', l)
            if m:
                j = k
                while j < len(pre) and not re.match(r'^\}', pre[j]): j += 1
                block = pre[k:j + 1]
                nm = re.match(r'^\}\s*(\w+)?', pre[j]).group(1) if j < len(pre) else None
                key = nm or m.group(1) or m.group(2)
                if key not in names: names.add(key); out += block
                k = j + 1; continue
            m = re.match(r'^typedef\b.*\b(\w+)\s*;', l)
            if m:
                if m.group(1) not in names: names.add(m.group(1)); out.append(l)
                k += 1; continue
            if l.startswith('/*') or not l.strip():
                k += 1; continue
            m = PROTO.match(l) or EXTERN.match(l)
            if m:
                key = m.group(1)
                if key not in decl: decl.add(key); out.append(l)
                k += 1; continue
            m = PRAGMA.match(l)
            if m:
                if ('pragma', m.group(1)) not in decl: decl.add(('pragma', m.group(1))); out.append(l)
                k += 1; continue
            if l not in seen: seen.add(l); out.append(l)
            k += 1
    # `#define X X_decl` hides the header's declaration of X: all of them before the includes, the #undefs after
    hide = [l for l in out if re.match(r'^#\s*define\s+(\w+)\s+\1_decl\b', l)]
    undef = [l for l in out if re.match(r'^#\s*undef\b', l)]
    inc = [l for l in out if re.match(r'^#\s*include\b', l)]
    rest = [l for l in out if l not in hide and l not in undef and l not in inc]
    return hide + inc + undef + rest + [s + ';' for s in sigs.values()]


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('stem'); ap.add_argument('--only'); ap.add_argument('--keep')
    a = ap.parse_args()
    os.chdir(ROOT)
    files = sorted(glob.glob(os.path.join('src', 'c', a.stem, '*.c')))
    only = set(a.only.split(',')) if a.only else None
    pres, funcs = [], []
    for f in files:
        pre, chunks = split_file(f)
        pres.append(pre)
        for name, chunk in chunks:
            if only and name not in only: continue
            addr = cc.label_addr(cc.c_paths(f, name)[2], name)
            if addr is None: continue
            funcs.append((addr, name, f, chunk))
    funcs.sort()
    sigs = {}
    for _, name, f, chunk in funcs:
        for l in chunk:
            if re.match(r'^[A-Za-z_][\w \t\*]*?\b%s\s*\(' % re.escape(name), l): sigs[name] = l.strip(); break
    text = '\n'.join(merge_pre(pres, sigs) + [''] + sum(([''] + c for _, _, _, c in funcs), [])) + '\n'
    work = tempfile.mkdtemp(prefix='module_')
    try:
        mod = os.path.join(work, a.stem.split('_', 2)[-1][:8].upper() + '.c'); open(mod, 'w').write(text)
        if a.keep:
            os.makedirs(os.path.dirname(os.path.abspath(a.keep)), exist_ok=True); open(a.keep, 'w').write(text)
        items = [(mod, mod[:-2] + '.obj', '')] + [(f, os.path.join(work, 'own%d.obj' % i), '') for i, f in enumerate(files)]
        oks = cc.compile_parallel(items, 8)
        if not oks[0]: raise SystemExit('the module file does not compile (--keep it to look)')
        own = {f: (o if ok else None) for (f, o, _), ok in zip(items[1:], oks[1:])}
        print('%-24s %6s  %-28s %s' % ('function', 'addr', 'own file', 'module file'))
        for addr, name, f, chunk in funcs:
            def res(obj):
                if obj is None: return 'no compile'
                try:
                    s = permute.score(f, obj, name, f) if obj != mod[:-2] + '.obj' else permute.score(mod, obj, name, f)
                    return 'MATCH' if s[2] else '%d differing (%+d)' % (s[0], s[1])
                except (SystemExit, Exception) as e:
                    return 'error: %s' % str(e)[:40]
            print('%-24s %06X  %-28s %s' % (name, addr, res(own[f]), res(mod[:-2] + '.obj')))
    finally:
        shutil.rmtree(work, ignore_errors=True)


if __name__ == '__main__':
    main()
