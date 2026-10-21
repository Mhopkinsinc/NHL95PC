#!/usr/bin/env python3
"""unsure_rows.py - turn the "tried / do not retry" shapes of docs/UNSURE.md into rows of docs/unsure_tried.csv.

  python3 tools/unsure_rows.py [--check]

One row per tried shape: func, transform, shape, result. `shape` is the UNSURE wording, `result` the outcome given
with it (a line count, "no change", "worse", ...). `transform` is a tools/permute.py transform prefix when the shape
is one the permuter would generate (a short or int result, a local or parameter type, a local declared in an inner
block, a declaration order): permute.py skips candidates whose transform starts with it. Rows whose transform is
filled in by hand (or kept from an earlier run) survive a rerun: the CSV is merged, never overwritten. --check only
prints what would be added."""
import os, re, csv, sys, argparse
HERE = os.path.dirname(os.path.abspath(__file__)); ROOT = os.path.dirname(HERE)
SRC = os.path.join(ROOT, 'docs', 'UNSURE.md'); OUT = os.path.join(ROOT, 'docs', 'unsure_tried.csv')
FIELDS = ['func', 'transform', 'shape', 'result']


def bullets(text):
    """(function name, bullet text) for every bullet that starts with a function name"""
    for l in text.split('\n'):
        m = re.match(r'^\s*\*\s+`?([A-Za-z_]\w*)`?(?:\s*/\s*`?[A-Za-z_]\w*`?)*\s*(\([^)]*\))?\s*:\s*(.*)$', l)
        if m and not m.group(1)[0].islower() or (m and re.match(r'^[a-z]\w*[A-Z0-9]|^[a-z]+\d|^ass|^chk|^check|^get|^set|^do|^puck|^skate|^wall|^upd|^per|^face|^opp', m.group(1))):
            yield m.group(1), m.group(3)


def split_shapes(t):
    """split a "Tried:" list at `; ` and at `, ` before a new shape, never inside `code`"""
    parts, cur, code = [], '', False
    k = 0
    while k < len(t):
        c = t[k]
        if c == '`': code = not code
        if not code and t.startswith('; ', k):
            parts.append(cur); cur = ''; k += 2; continue
        if not code and t.startswith(', ', k) and re.match(r'(?:a|an|the|plus|declaring|moving|swapping|with|without)\b', t[k + 2:]):
            parts.append(cur); cur = ''; k += 2; continue
        cur += c; k += 1
    return parts + [cur]


def shapes(body):
    """the tried shapes of one bullet: [(shape, result)]"""
    out = []
    for m in re.finditer(r'(?:Tried|tried)(?: so far)?:?\s+(.*?)(?:\.\s+(?:Do not retry|Not retried|None of)|$)', body):
        t = m.group(1).strip()
        if not t: continue
        for part in split_shapes(t):
            part = part.strip().rstrip('.')
            if len(part) < 4: continue
            r = re.search(r'\(([^()]*?(?:\d+|no change|worse|same|better|lines?|bytes?)[^()]*)\)\s*$', part)
            out.append((part, r.group(1) if r else ''))
    if not out and re.search(r'[Dd]o not retry', body):
        m = re.search(r'([A-Z][a-z]+) shapes', body)
        out.append(((m.group(0) if m else 'shapes listed in docs/UNSURE.md') + ' (not itemized)', ''))
    return out


def transform_of(func, shape):
    s = shape.lower()
    if re.search(r'\bshort result\b|result `?short`?|declared `short`', s): return 'rettype:short'
    if re.search(r'\bint result\b|result `?int`?', s): return 'rettype:int'
    m = re.search(r'`?(\w+)`? declared in the (?:loop|block|inner)', shape)
    if m: return 'scope:%s' % m.group(1)
    return ''


def main():
    ap = argparse.ArgumentParser(); ap.add_argument('--check', action='store_true'); a = ap.parse_args()
    old = []
    if os.path.exists(OUT): old = list(csv.DictReader(open(OUT, newline='')))
    have = {(r['func'], r['shape']) for r in old}
    new = []
    for func, body in bullets(open(SRC).read()):
        for shape, result in shapes(body):
            if (func, shape) in have: continue
            have.add((func, shape))
            new.append(dict(func=func, transform=transform_of(func, shape), shape=shape, result=result))
    if a.check:
        for r in new: print('%s | %s | %s | %s' % (r['func'], r['transform'], r['shape'][:90], r['result']))
        print('%d new rows (%d kept)' % (len(new), len(old)))
        return
    with open(OUT, 'w', newline='') as f:
        w = csv.DictWriter(f, FIELDS, lineterminator='\n'); w.writeheader()
        for r in old + new: w.writerow({k: r.get(k, '') for k in FIELDS})
    print('%s: %d rows (%d new)' % (os.path.relpath(OUT, ROOT), len(old) + len(new), len(new)))


if __name__ == '__main__':
    main()
