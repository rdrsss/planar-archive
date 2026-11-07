#!/usr/bin/env python3
"""Record assertions of round V5-02-R15-A1-C03 (ASSERT-C03-*).

Stdlib only, read only, deterministic. Observes record claims that no registered
check of ex_checks-c02.py observes (DECISIONS D12, INVARIANTS section 0).

Usage:
  record_asserts_c03.py --repo <root> [--evidence <dir>] [--record <file>]
      [--plan <dir>] [--phase early|final|all] [--final] [--print-constants]

Prints one line per assertion (PASS ASSERT-C03-<x> or FAIL ASSERT-C03-<x>:
<reason>), then the exemption list (category, line, reason). Prints no host
path (the repository root is shown as <repo>). Exits non-zero on any FAIL.
"""
import argparse
import ast
import fnmatch
import glob
import hashlib
import os
import re
import sys
import urllib.parse

# ---------------------------------------------------------------------------
# Pinned constants. The two blocks below are compared with the pinned blocks of
# P3/plan/INVARIANTS.md sections 7 and 8 (read from --plan).
# ---------------------------------------------------------------------------
PATHSCAN_CONSTANTS = """\
CONST EXT_NAME identity.py
CONST COORD_PREFIX ## Dispatch
CONST COORD_PREFIX ## Corrective dispatch
CONST COORD_PREFIX ## Review outcome
CONST COORD_PREFIX ## C01 review outcome
CONST COORD_PREFIX ## C02 outcome
CONST LIVE_IN_COORD ### Checker amendments (C02)
CONST LIVE_IN_COORD ### Owned files (C02)
CONST LIVE_IN_COORD ### Redaction (C02)
CONST LIVE_IN_COORD ### C02 acceptance status
CONST LIVE_IN_COORD ### Staging list (C02)
CONST LIVE_IN_COORD ### Unstaged paths (C03)
CONST OLD_SECTION_END ## Corrective dispatch V5-02-R15-A1-C01
CONST OLD_EXCLUDE ### (l2)
CONST E2_NAME dev/planar-v2-vernacular
CONST E2_PREFIX build/
CONST E2_REGEX ^[a-z_]+\\.[a-z_]+(/[a-z_]+)*$
CONST E2_REGEX_NOT_FIRST ex_checks,identity,checks
CONST E7_SECTION ### Complete invariant table
CONST E7_SECTION ### (m) Complete invariant table
CONST E7_REGEX ^(probes/raw/)?[A-Za-z]+[0-9]*[a-z]?-m[0-9]+\\.(txt|log)$
CONST NEG_MARKER does not exist|do not exist|nonexistent|is absent|are absent|not written|never created|no such file|is not present
CONST FIN_ONLY green/audit-final.txt
CONST FIN_ONLY green/audit-verify-c02.txt
CONST FIN_ONLY green/redaction-final-c02.txt
CONST FIN_ONLY green/record-asserts-c03.txt
CONST FIN_ONLY green/reprobe-c03.txt
CONST FIN_ONLY green/identity-compare-c03.txt
CONST FIN_ONLY green/record-asserts-final-c03.txt
CONST FIN_ONLY green/superseded-4
CONST FIN_ONLY probes/reprobe
CONST OUTCOME_HEADING ## V5-02-R15-A1-C03 outcome
CONST OUTCOME_AFTER ## Corrective dispatch V5-02-R15-A1-C03
CONST OUTCOME_SUB ### Final results (C03)
CONST OUTCOME_SUB ### C03 closure table
CONST OUTCOME_SUB ### Owned files (C03)
CONST OUTCOME_SUB ### Checker amendments (C03)
CONST UNSTAGED_AFTER ### Staging list (C02)
CONST UNSTAGED_HEADING ### Unstaged paths (C03)
CONST SECT_MENTION_EXEMPT ### Checker amendments (C02)
CONST SECT_MENTION_EXEMPT ### Checker amendments (C03)
CONST SECT_TABLE ### (l2)
CONST SECT_TABLE ### C02 acceptance status
CONST SECT_STATUS met
CONST SECT_STATUS shown by FIN log
CONST SECT_LOGDIR green/
CONST SECT_LOGDIR red/
CONST SECT_LOGDIR probes/
CONST SECT_LOGDIR raw/
CONST SECT_BANNED Provisional (REC1 first draft)
CONST SECT_BANNED REC3 and CLOSE finalize
CONST FIN_REWRITTEN green/linkcheck-c02.txt
CONST FIN_REWRITTEN green/redaction-c02.txt
CONST FIN_REWRITTEN green/checks.txt
CONST FIN_REWRITTEN green/identity-c02.txt
CONST FIN_REWRITTEN green/cmp-c02.txt
CONST FIN_REWRITTEN green/scope-c02.txt
CONST FIN_REWRITTEN green/frozen-c02.txt
CONST HASH_SOURCE docs/vernacular/rounds/*.md
CONST HASH_SOURCE docs/vernacular/roadmap.md
CONST HASH_SOURCE tests/native/*-evidence/**/REPORT*
CONST HASH_SOURCE tests/native/v5-02-r15-a1-c01-evidence/raw/pre-rec-record-c03.md
CONST HASH_SKIP_SELF tests/native/v5-02-r15-a1-c01-evidence/green/record-asserts-final-c03.txt
CONST HIST_PINNED (empty)
"""

STAGING_CONSTANTS = """\
CONST STAGE_ADD docs/vernacular/rounds/V5-02-R15-A1-C02-review.md
CONST STAGE_ADD tests/native/v5-02-r15-a1-evidence/REPORT.md
CONST STAGE_ADD tests/native/v5-02-r15-a1-evidence/README.md
CONST STAGE_ADD tests/native/v5-02-r15-a1-evidence/plan/DECISIONS.md
CONST STAGE_ADD tests/native/v5-02-r15-a1-evidence/plan/INVARIANTS.md
CONST STAGE_ADD tests/native/v5-02-r15-a1-c01-evidence/plan/DECISIONS.md
CONST STAGE_ADD tests/native/v5-02-r15-a1-c01-evidence/plan/INVARIANTS.md
CONST STAGE_ADD tests/native/v5-02-r15-a1-c01-evidence/red/snapshot/SHA256SUMS
CONST STAGE_ADD tests/native/v5-02-r15-a1-c01-evidence/red/snapshot-2/SHA256SUMS
CONST STAGE_ADD tests/native/v5-02-r15-a1-c02-evidence/red/snapshot/SHA256SUMS
CONST STAGE_ADD tests/native/v5-02-r15-a1-c02-evidence/red/snapshot-2/SHA256SUMS
CONST STAGE_ADD tests/native/v5-02-r15-a1-c03-evidence/plan/DECISIONS.md
CONST STAGE_ADD tests/native/v5-02-r15-a1-c03-evidence/plan/INVARIANTS.md
CONST STAGE_ADD tests/native/v5-02-r15-a1-c01-evidence/green/messages-before-c03.txt
CONST STAGE_ADD tests/native/v5-02-r15-a1-c01-evidence/green/freeze-c03.txt
CONST STAGE_ADD tests/native/v5-02-r15-a1-c01-evidence/green/freeze-red-c03.txt
CONST STAGE_ADD tests/native/v5-02-r15-a1-c01-evidence/green/messages-diff-c03.txt
CONST STAGE_ADD tests/native/v5-02-r15-a1-c01-evidence/green/audit-pre-rec-c02.txt
CONST STAGE_ADD tests/native/v5-02-r15-a1-c01-evidence/green/staging-candidates-c03.txt
CONST STAGE_ADD tests/native/v5-02-r15-a1-c01-evidence/green/record_asserts_c03.py
CONST STAGE_ADD tests/native/v5-02-r15-a1-c01-evidence/green/rec_preflight_c03.py
CONST STAGE_ADD tests/native/v5-02-r15-a1-c01-evidence/green/record-asserts-red-c03.txt
CONST STAGE_ADD tests/native/v5-02-r15-a1-c01-evidence/green/record-asserts-c03.txt
CONST STAGE_ADD tests/native/v5-02-r15-a1-c01-evidence/green/reprobe-c03.txt
CONST STAGE_ADD tests/native/v5-02-r15-a1-c01-evidence/green/identity-compare-c03.txt
CONST STAGE_UNSTAGED docs/vernacular/roadmap.md
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-c01-evidence/green/frozen/
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-c01-evidence/green/superseded-*/
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-c01-evidence/plan/superseded-*/
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-evidence/plan/superseded-*/
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-c02-evidence/plan/superseded-*/
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-c03-evidence/plan/superseded-*/
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-c01-evidence/raw/
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-c01-evidence/red/raw/
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-c02-evidence/red/raw/
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-c01-evidence/probes/raw/
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-c01-evidence/probes/reprobe/
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-evidence/inventory.txt
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-evidence/green/
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-evidence/red/
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-evidence/raw/
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-evidence/review/
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-evidence/review-c01/
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-c02-review-evidence/
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-c01-evidence/red/snapshot*/
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-c02-evidence/red/snapshot*/
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-c02-evidence/red/RED.md
CONST STAGE_UNSTAGED tests/native/v5-02-r15-a1-c01-evidence/green/record-asserts-final-c03.txt
"""

E_REL = 'tests/native/v5-02-r15-a1-c01-evidence'
P2_REL = 'tests/native/v5-02-r15-a1-c02-evidence'
P3_REL = 'tests/native/v5-02-r15-a1-c03-evidence'
A1_REL = 'tests/native/v5-02-r15-a1-evidence'
RECORD_REL = 'docs/vernacular/rounds/V5-02-R15-A1.md'

ORDER = ['msgs', 'freeze-red', 'paths', 'closure', 'C55', 'R4', 'R5', 'R6', 'R7',
         'probe-column', 'owned', 'staging', 'final-results', 'identity-ref',
         'runner', 'sections', 'order', 'reprobe', 'record-rule']
EARLY = ORDER[:16]
FINAL = ['paths', 'staging', 'identity-ref', 'sections', 'order', 'reprobe',
         'record-rule']


def parse_consts(text):
    out = []
    for line in text.splitlines():
        line = line.rstrip()
        if line.startswith('CONST '):
            rest = line[6:]
            name, _, val = rest.partition(' ')
            out.append((name, val))
    return out


CONSTS = parse_consts(PATHSCAN_CONSTANTS) + parse_consts(STAGING_CONSTANTS)


def cvals(name):
    vals = [v for n, v in CONSTS if n == name]
    if name == 'HIST_PINNED':
        return [v for v in vals if v != '(empty)']
    return vals


def cval(name):
    v = cvals(name)
    return v[0] if v else ''


class Fail(Exception):
    pass


class Ctx:
    pass


# ---------------------------------------------------------------------------
# small helpers
# ---------------------------------------------------------------------------
def read_text(path):
    with open(path, 'r', encoding='utf-8', errors='replace', newline='') as f:
        return f.read()


def read_lines(path):
    return read_text(path).split('\n')


def line_count(path):
    t = read_text(path)
    n = t.count('\n')
    if t and not t.endswith('\n'):
        n += 1
    return n


def sha256_file(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(1 << 20), b''):
            h.update(chunk)
    return h.hexdigest()


def need(cond, msg):
    if not cond:
        raise Fail(msg)


def level_of(line):
    m = re.match(r'^(#{1,6})\s', line)
    return len(m.group(1)) if m else 0


class Record:
    """Line classification of the round record."""

    def __init__(self, path):
        self.path = path
        self.lines = read_text(path).split('\n')
        n = len(self.lines)
        self.h2 = [None] * n      # index of the governing ## heading line
        self.h3 = [None] * n      # index of the governing ### heading line
        self.fence = [False] * n
        cur2 = None
        cur3 = None
        infence = False
        self.headings = []        # (index, level, text)
        for i, line in enumerate(self.lines):
            if line.strip().startswith('```'):
                infence = not infence
                self.fence[i] = True
            elif infence:
                self.fence[i] = True
            if not infence and not self.fence[i]:
                lv = level_of(line)
                if lv == 2:
                    cur2 = i
                    cur3 = None
                    self.headings.append((i, 2, line))
                elif lv == 3:
                    cur3 = i
                    self.headings.append((i, 3, line))
                elif lv == 1:
                    self.headings.append((i, 1, line))
            self.h2[i] = cur2
            self.h3[i] = cur3
        self.first_h2 = min([h[0] for h in self.headings if h[1] == 2] or [n])
        old_end = n
        for i, line in enumerate(self.lines):
            if (not self.fence[i] and level_of(line) == 2
                    and line.startswith(cval('OLD_SECTION_END'))):
                old_end = i
                break
        self.old_end = old_end

    def h2text(self, i):
        j = self.h2[i]
        return self.lines[j] if j is not None else ''

    def h3text(self, i):
        j = self.h3[i]
        return self.lines[j] if j is not None else ''

    def is_coord(self, i):
        h2 = self.h2text(i)
        if not any(h2.startswith(p) for p in cvals('COORD_PREFIX')):
            return False
        h3 = self.h3text(i)
        if h3 and any(h3.startswith(p) for p in cvals('LIVE_IN_COORD')):
            return False
        return True

    def is_old(self, i):
        if i >= self.old_end or i < self.first_h2:
            return False
        h3 = self.h3text(i)
        if h3 and h3.startswith(cval('OLD_EXCLUDE')):
            return False
        return True

    def find_h(self, prefix, level=None):
        for (i, lv, text) in self.headings:
            if text.startswith(prefix) and (level is None or lv == level):
                return i
        return None

    def sub(self, prefix):
        """(start, end) line indexes of the body of the first heading with
        the prefix; the body ends at the next heading of level <= 3."""
        s = self.find_h(prefix)
        if s is None:
            return None
        lv = level_of(self.lines[s])
        e = len(self.lines)
        for (i, l2, _t) in self.headings:
            if i > s and l2 <= max(lv, 3) and l2 <= lv:
                e = i
                break
        return (s, e)

    def sub_text(self, prefix):
        r = self.sub(prefix)
        if r is None:
            return None
        return '\n'.join(self.lines[r[0] + 1:r[1]])

    def next_heading_after(self, idx):
        for (i, _lv, text) in self.headings:
            if i > idx:
                return (i, text)
        return None


def split_cells(line):
    s = line.strip()
    if not s.startswith('|'):
        return None
    s = s.replace('\\|', '\x00')
    s = s[1:]
    if s.endswith('|'):
        s = s[:-1]
    return [c.replace('\x00', '|').strip() for c in s.split('|')]


def table_rows(lines, start=0):
    """Table rows (lineno, cells) of consecutive lines; header first."""
    rows = []
    for i, line in enumerate(lines):
        cells = split_cells(line)
        if cells is None:
            continue
        if all(re.match(r'^:?-{2,}:?$', c) or c == '' for c in cells) and any(cells):
            continue
        rows.append((start + i + 1, cells))
    return rows


BT = re.compile(r'`([^`\n]+)`')


def backticked(line):
    return [(m.start(), m.end(), m.group(1).strip()) for m in BT.finditer(line)]


def strip_punct(tok):
    return tok.rstrip('.,;:)')


def normalise(raw):
    """Return (path, line_lo, line_hi) or None."""
    t = strip_punct(raw)
    t = urllib.parse.unquote(t)
    if '#' in t:
        t = t.split('#', 1)[0]
    lo = hi = None
    m = re.search(r':(\d+)(?:-(\d+))?$', t)
    if m:
        lo = int(m.group(1))
        hi = int(m.group(2)) if m.group(2) else None
        t = t[:m.start()]
    t = t.rstrip('/')
    t = strip_punct(t)
    return (t, lo, hi)


def path_like(raw, norm):
    if not norm:
        return False
    if raw.startswith(('http', '#', '-', '$')):
        return False
    if '/' in norm:
        return True
    if re.search(r'\.[A-Za-z0-9]{1,5}$', norm):
        return True
    if norm.startswith('.'):
        return True
    return False


def glob_chars(tok):
    return any(c in tok for c in '<>*?{}$') or '...' in tok


def fin_only_match(norm):
    for p in cvals('FIN_ONLY'):
        if norm == p or norm.endswith('/' + p) or ('/' + p + '/') in ('/' + norm + '/'):
            return True
        if '/' not in norm and p.endswith('/' + norm):
            return True
    return False


# ---------------------------------------------------------------------------
# context and path resolution
# ---------------------------------------------------------------------------
def make_ctx(args):
    c = Ctx()
    c.repo = os.path.abspath(args.repo)
    c.E = os.path.abspath(args.evidence) if args.evidence else os.path.join(c.repo, E_REL)
    c.P2 = os.path.join(c.repo, P2_REL)
    c.P3 = os.path.join(c.repo, P3_REL)
    c.A1 = os.path.join(c.repo, A1_REL)
    c.record_path = os.path.abspath(args.record) if args.record else os.path.join(c.repo, RECORD_REL)
    c.plan = os.path.abspath(args.plan) if args.plan else os.path.join(c.P3, 'plan')
    c.final = bool(args.final)
    c.rec = None
    c.exempt = []          # (category, line, token, reason)
    c.index = None
    c.scan = None
    return c


def basename_index(c):
    if c.index is None:
        names = set()
        roots = [os.path.join(c.repo, 'docs', 'vernacular'), c.E, c.P2, c.P3, c.A1,
                 os.path.join(c.repo, 'scripts')]
        locs = {}
        for r in roots:
            if not os.path.isdir(r):
                continue
            for dp, dns, fns in os.walk(r):
                for n in dns + fns:
                    names.add(n)
                    locs.setdefault(n, []).append(os.path.join(dp, n))
        c.index = locs
    return c.index


def record(c):
    if c.rec is None:
        need(os.path.isfile(c.record_path), 'the round record is missing')
        c.rec = Record(c.record_path)
    return c.rec


def old_bases(c):
    return [c.repo, os.path.join(c.repo, 'docs/vernacular'),
            os.path.join(c.repo, 'docs/vernacular/rounds'), c.A1, c.E]


def new_bases(c):
    return [c.repo, os.path.join(c.repo, 'docs/vernacular'),
            os.path.join(c.repo, 'docs/vernacular/rounds'), c.E, c.P2, c.P3,
            os.path.join(c.repo, 'tests/native')]


def resolve_token(c, norm, lo, hi, old):
    """True when the token resolves (and its line cite is within the file)."""
    cands = []
    if '/' in norm:
        for b in (old_bases(c) if old else new_bases(c)):
            p = os.path.join(b, norm)
            if os.path.exists(p):
                cands.append(p)
    else:
        cands = [p for p in basename_index(c).get(norm, [])]
        for b in new_bases(c):
            p = os.path.join(b, norm)
            if os.path.exists(p) and p not in cands:
                cands.append(p)
    if not cands:
        return False
    if lo is None:
        return True
    for p in cands:
        if os.path.isfile(p):
            n = line_count(p)
            if lo <= n and (hi is None or hi <= n):
                return True
    return False


def sentence_of(line, pos):
    bounds = [0]
    for m in re.finditer(r'[.;!?](?=\s)|\|', line):
        bounds.append(m.end())
    bounds.append(len(line) + 1)
    for a, b in zip(bounds, bounds[1:]):
        if a <= pos < b:
            return line[a:b]
    return line


def scan_record(c):
    """Whole-record path scan; returns (hits, exemptions)."""
    if c.scan is not None:
        return c.scan
    rec = record(c)
    hits = []
    exempts = []
    neg = re.compile(cval('NEG_MARKER'), re.I)
    e2re = re.compile(cval('E2_REGEX'))
    not_first = set(cval('E2_REGEX_NOT_FIRST').split(','))
    e7re = re.compile(cval('E7_REGEX'))
    for i, line in enumerate(rec.lines):
        if not line.strip():
            continue
        toks = []
        for (s, e, t) in backticked(line):
            if re.search(r'\s', t):
                continue
            m = re.match(r'^\[[^\]]*\]\(([^)]*)\)[.,;:]*$', t)
            if m:
                t = m.group(1)
            toks.append((s, t))
        plain = BT.sub(lambda m: ' ' * len(m.group(0)), line)
        for m in re.finditer(r'\]\(([^)\s]+)\)', plain):
            toks.append((m.start(1), m.group(1)))
        for (pos, raw) in toks:
            norm, lo, hi = normalise(raw)
            if not path_like(raw, norm):
                continue
            if raw.startswith('http') or raw.startswith('mailto:'):
                continue
            old = rec.is_old(i)
            if resolve_token(c, norm, lo, hi, old):
                continue
            ln = i + 1
            cat = None
            reason = None
            if glob_chars(raw) or glob_chars(norm) or raw.startswith('.') or norm.startswith('.'):
                cat, reason = 'E1', 'placeholder or glob'
            elif (norm == cval('E2_NAME') or norm.startswith(cval('E2_PREFIX'))
                  or (e2re.match(norm) and norm.split('.', 1)[0] not in not_first)):
                cat, reason = 'E2', 'non-file identifier'
            elif norm == cval('EXT_NAME'):
                cat, reason = 'E3', 'external tool name'
            elif rec.is_coord(i):
                cat, reason = 'E5', 'coordinator dispatch or review-outcome section'
            else:
                seg = sentence_of(line, pos)
                seg = BT.sub(' ', seg)
                if neg.search(seg):
                    cat, reason = 'E6', 'negative existence sentence'
            if cat is None:
                h3 = rec.h3text(i)
                if (e7re.match(norm) and line.lstrip().startswith('|')
                        and any(h3.startswith(p) for p in cvals('E7_SECTION'))):
                    cat, reason = 'E7', 'mutant-log name in an invariant table cell'
            if cat is None and norm in cvals('HIST_PINNED'):
                cat, reason = 'HIST', 'pinned historical exemption'
            if cat is None and not c.final and fin_only_match(norm):
                cat, reason = 'FIN', 'FIN-only path, written after the record (non-final mode)'
            if cat is not None:
                exempts.append((cat, ln, raw, reason))
            else:
                hits.append((ln, raw))
    c.scan = (hits, exempts)
    return c.scan


def placement_problems(c, subs):
    """Problems with the outcome heading and the named OUTCOME_SUB headings."""
    rec = record(c)
    probs = []
    oh = cval('OUTCOME_HEADING')
    h2s = [(i, t) for (i, lv, t) in rec.headings if lv == 2]
    outs = [(i, t) for (i, t) in h2s if t.startswith(oh)]
    if not outs:
        probs.append('heading "%s" is missing' % oh)
        out_i = None
    else:
        out_i, out_t = outs[0]
        if any(out_t.startswith(p) for p in cvals('COORD_PREFIX')):
            probs.append('the outcome heading starts with a COORD prefix')
        if h2s and h2s[-1][0] != out_i:
            probs.append('the outcome heading is not the last ## heading')
        after = [i for (i, t) in h2s if t.startswith(cval('OUTCOME_AFTER'))]
        if not after:
            probs.append('heading "%s" is missing' % cval('OUTCOME_AFTER'))
        elif after[0] > out_i:
            probs.append('the outcome heading does not come after the C03 dispatch heading')
    for sub in subs:
        j = rec.find_h(sub, 3)
        if j is None:
            probs.append('heading "%s" is absent' % sub)
            continue
        if rec.h2[j] is None or out_i is None or rec.h2[j] != out_i:
            probs.append('heading "%s" does not lie under the outcome heading' % sub)
        elif any(rec.lines[rec.h2[j]].startswith(p) for p in cvals('COORD_PREFIX')):
            probs.append('heading "%s" lies in a COORD section' % sub)
    return probs


def parse_block(text, begin, end):
    m = re.search(re.escape(begin) + r'(.*?)' + re.escape(end), text, re.S)
    if not m:
        return None
    return [l.rstrip() for l in m.group(1).splitlines() if l.startswith('CONST ')]


def plan_text(c):
    p = os.path.join(c.plan, 'INVARIANTS.md')
    need(os.path.isfile(p), 'plan/INVARIANTS.md is missing')
    return read_text(p)


def consts_lines(text):
    return [('CONST %s %s' % (n, v)) for n, v in parse_consts(text)]


def compare_consts(c, which):
    t = plan_text(c)
    if which == 'path':
        blk = parse_block(t, '<!-- PATHSCAN-CONSTANTS begin -->', '<!-- PATHSCAN-CONSTANTS end -->')
        mine = consts_lines(PATHSCAN_CONSTANTS)
        word = 'exemption'
    else:
        blk = parse_block(t, '<!-- STAGING-CONSTANTS begin -->', '<!-- STAGING-CONSTANTS end -->')
        mine = consts_lines(STAGING_CONSTANTS)
        word = 'staging'
    need(blk is not None, 'the pinned %s block is not found in the plan' % word)
    if sorted(blk) != sorted(mine):
        a = sorted(set(blk) - set(mine))
        b = sorted(set(mine) - set(blk))
        raise Fail('%s constants differ from the pinned set (only in plan: %d, only in script: %d)'
                   % (word, len(a), len(b)))


# ---------------------------------------------------------------------------
# message extraction (same algorithm as FRZ extract_msgs.py)
# ---------------------------------------------------------------------------
def extract_messages(path):
    tree = ast.parse(read_text(path))
    out = []

    def chk_id(fn):
        for d in fn.decorator_list:
            if (isinstance(d, ast.Call) and getattr(d.func, 'id', '') == 'check'
                    and d.args and isinstance(d.args[0], ast.Constant)):
                return d.args[0].value
        return None

    def strs(node):
        return [n.value for n in ast.walk(node)
                if isinstance(n, ast.Constant) and isinstance(n.value, str)]

    for fn in ast.walk(tree):
        if isinstance(fn, ast.FunctionDef):
            cid = chk_id(fn) or ('(helper) ' + fn.name)
            for n in ast.walk(fn):
                if isinstance(n, ast.Call) and getattr(n.func, 'id', '') in ('need', 'fail'):
                    args = n.args[1:] if n.func.id == 'need' else n.args
                    for a in args[:1]:
                        out.append('%s\t%s' % (cid, ''.join(strs(a)).replace('\n', '\\n')))
    return out


# ---------------------------------------------------------------------------
# assertions
# ---------------------------------------------------------------------------
def a_msgs(c):
    before = os.path.join(c.E, 'green', 'messages-before-c03.txt')
    chk = os.path.join(c.E, 'green', 'ex_checks-c02.py')
    need(os.path.isfile(before), 'green/messages-before-c03.txt is missing')
    need(os.path.isfile(chk), 'green/ex_checks-c02.py is missing')
    have = set(extract_messages(chk))
    want = [l for l in read_text(before).split('\n') if l.strip()]
    missing = [l for l in want if l not in have]
    if missing:
        raise Fail('%d message(s) of messages-before-c03.txt are no longer present in the checker, first: %s'
                   % (len(missing), missing[0].replace('\t', ' / ')[:120]))
    return '%d messages present' % len(want)


def checker_sha_of_checks(c):
    p = os.path.join(c.E, 'green', 'checks.txt')
    if not os.path.isfile(p):
        return None
    for l in read_lines(p):
        m = re.match(r'^checker sha256: ([0-9a-f]{64})\s*$', l)
        if m:
            return m.group(1)
    return None


def a_freeze_red(c):
    p = os.path.join(c.E, 'green', 'freeze-red-c03.txt')
    need(os.path.isfile(p), 'green/freeze-red-c03.txt is missing')
    lines = read_text(p).split('\n')
    text = '\n'.join(lines)
    cmd = [l for l in lines if l.startswith('command:')]
    need(cmd and 'ex_checks-c02' in cmd[0] and '--repo' in cmd[0] and '--frozen-dir' in cmd[0],
         'the exact command line (ex_checks-c02 --repo --frozen-dir) is missing')
    need('identity (documents):' in text and 'identity (src tests):' in text
         and 'combined sha256' in text, 'the identity.py output is not pasted')
    f = [l for l in lines if l.startswith('pre-edit checker file:')]
    need(len(f) == 1, 'there is not exactly one "pre-edit checker file:" line')
    named = f[0].split(':', 1)[1].strip()
    want = E_REL + '/raw/ex_checks-c02-preedit-c03.py'
    need(named == want, 'the pre-edit checker file line does not name raw/ex_checks-c02-preedit-c03.py')
    s = [l for l in lines if l.startswith('pre-edit checker sha256:')]
    need(len(s) == 1, 'the "pre-edit checker sha256:" line is missing')
    m = re.match(r'^pre-edit checker sha256: ([0-9a-f]{64})\s*$', s[0])
    need(m is not None, 'the pre-edit checker sha256 is not a 64-hex value')
    pinned = m.group(1)
    saved = os.path.join(c.E, 'raw', 'ex_checks-c02-preedit-c03.py')
    need(os.path.isfile(saved), 'raw/ex_checks-c02-preedit-c03.py is missing')
    need(sha256_file(saved) == pinned,
         'the pinned pre-edit checker sha256 differs from the saved file')
    amended = checker_sha_of_checks(c)
    if amended is not None:
        need(pinned != amended, 'the pre-edit checker sha256 equals the checker sha256 of green/checks.txt')
    need([l for l in lines if l.strip() == 'Command exit code: 1'] and
         not [l for l in lines if l.startswith('Command exit code:') and l.strip() != 'Command exit code: 1'],
         'the line "Command exit code: 1" is missing or another exit code is present')
    fails = [l for l in lines if l.startswith('FAIL ')]
    need(len(fails) == 1 and fails[0].startswith('FAIL CHK-I45-diff-scope')
         and 'V5-02-R15-A1-C02-review.md' in fails[0],
         'expected exactly one FAIL CHK-I45-diff-scope line naming V5-02-R15-A1-C02-review.md (found %d FAIL lines)' % len(fails))
    npass = len([l for l in lines if l.startswith('PASS CHK-')])
    need(npass == 112, 'expected 112 PASS lines, found %d' % npass)
    nonempty = [l for l in lines if l.strip()]
    need(nonempty and nonempty[-1].startswith('masked:'), 'the mask line is not last')
    return 'pre-edit checker hash matches, 1 FAIL, 112 PASS'


def a_paths(c):
    compare_consts(c, 'path')
    rec = record(c)
    probs = placement_problems(c, cvals('OUTCOME_SUB'))
    hits, exempts = scan_record(c)
    c.exempt.extend(exempts)
    msgs = list(probs)
    if hits:
        shown = ['line %d `%s`' % (ln, t) for ln, t in hits[:40]]
        extra = '' if len(hits) <= 40 else ' (+%d more)' % (len(hits) - 40)
        msgs.append('%d unresolved path token(s): %s%s' % (len(hits), '; '.join(shown), extra))
    need(not msgs, ' | '.join(msgs))
    return '%d exemption(s) used' % len(exempts)


def a_closure(c):
    probs = placement_problems(c, ['### C03 closure table'])
    need(not probs, '; '.join(probs))
    rec = record(c)
    r = rec.sub('### C03 closure table')
    rows = table_rows(rec.lines[r[0] + 1:r[1]], r[0] + 1)
    hits, exempts = scan_record(c)
    hitlines = set(ln for ln, _t in hits)
    byid = {}
    for (ln, cells) in rows:
        if cells:
            byid[cells[0].strip('` ')] = (ln, cells)
    bad = []
    for k in range(1, 8):
        rid = 'C02-R%d' % k
        if rid not in byid:
            bad.append('no row for %s' % rid)
            continue
        ln, cells = byid[rid]
        line = rec.lines[ln - 1]
        toks = [t for (_s, _e, t) in backticked(line)
                if path_like(t, normalise(t)[0]) and not re.search(r'\s', t)]
        if not toks:
            bad.append('row %s cites no file' % rid)
        if (ln) in hitlines:
            bad.append('row %s cites a file that does not exist' % rid)
    need(not bad, '; '.join(bad))
    return 'rows C02-R1..C02-R7 present'


def a_c55(c):
    pr = os.path.join(c.E, 'probes', 'PROBES.md')
    need(os.path.isfile(pr), 'probes/PROBES.md is missing')
    ok = [l for l in read_lines(pr) if 'C55-m1' in l and 'row-count' in l and 'CHK-I60-role-rows' in l]
    need(ok, 'no line of probes/PROBES.md says that C55-m1 is killed by the row-count pin of CHK-I60-role-rows')
    rec = record(c)
    t = rec.sub_text('### D81 resolution')
    need(t is not None, 'the record has no "D81 resolution" subsection')
    need('C55-m1' in t and 'row-count' in t and 'CHK-I60-role-rows' in t,
         'the record D81 resolution does not say that C55-m1 is killed by the row-count pin of CHK-I60-role-rows')
    return 'PROBES.md and D81 text agree'


R4_NEEDLES = ['irrelevant first attempt', 'planar_tests', 'ctest', 'tests/native', 'red-c02-run.txt']


def r4_text_problems(t):
    probs = []
    for n in R4_NEEDLES:
        if n not in t:
            probs.append('lacks "%s"' % n)
    if not re.search(r'\b58\b', t):
        probs.append('lacks the number 58 (ctest runs)')
    return probs


def closure_row(c, rid):
    rec = record(c)
    r = rec.sub('### C03 closure table')
    if r is None:
        return None
    for (ln, cells) in table_rows(rec.lines[r[0] + 1:r[1]], r[0] + 1):
        if cells and cells[0].strip('` ') == rid:
            return rec.lines[ln - 1]
    return None


def a_r4(c):
    red = os.path.join(c.P2, 'red', 'RED.md')
    need(os.path.isfile(red), 'the C02 plan-tree red/RED.md is missing')
    p = r4_text_problems(read_text(red))
    need(not p, 'C02 red/RED.md ' + '; '.join(p))
    row = closure_row(c, 'C02-R4')
    need(row is not None, 'the record has no C02-R4 row in the C03 closure table')
    p = r4_text_problems(row)
    need(not p, 'the record C02-R4 row ' + '; '.join(p))
    return 'label present in RED.md and in the record row'


def log_fail_ids(path):
    ids = set()
    pass_ids = set()
    for l in read_lines(path):
        m = re.match(r'^(PASS|FAIL) (CHK-\S+)', l)
        if m:
            (ids if m.group(1) == 'FAIL' else pass_ids).add(m.group(2))
    return ids, pass_ids


def a_r5(c):
    rec = record(c)
    r = rec.sub('### Complete invariant table (C02)')
    need(r is not None, 'the record has no "Complete invariant table (C02)" subsection')
    dpath = os.path.join(c.E, 'red', 'red-c02-run.txt')
    apath = os.path.join(c.E, 'red', 'red-c02-audit-run.txt')
    need(os.path.isfile(dpath) and os.path.isfile(apath), 'red/red-c02-run.txt or red/red-c02-audit-run.txt is missing')
    dfail, dpass = log_fail_ids(dpath)
    afail, apass = log_fail_ids(apath)
    body = rec.lines[r[0] + 1:r[1]]
    rows = table_rows(body, r[0] + 1)
    need(len(rows) > 1, 'the table has no rows')
    hdr = [h.lower() for h in rows[0][1]]
    ncol = len(hdr)
    ri = [i for i, h in enumerate(hdr) if h.startswith('red')]
    ti = [i for i, h in enumerate(hdr) if h == 'test']
    need(ri and ti, 'the table has no red-check column or no test column')
    rneg = ri[0] - ncol
    tneg = ti[0] - ncol
    head_text = rows[0][1][ri[0]]
    intro = '\n'.join(rec.lines[r[0] + 1:rows[0][0] - 1])
    sect = '\n'.join(body)
    probs = []
    if 'red/RED.md, pre-fix tree' in sect:
        probs.append('stale header text "red/RED.md, pre-fix tree" is still present')
    hay = head_text + '\n' + intro
    if 'red-c02-run.txt' not in hay or 'red-c02-audit-run.txt' not in hay:
        probs.append('the red-check column header does not name red-c02-run.txt and red-c02-audit-run.txt')
    audit_only = (afail | apass) - (dfail | dpass)
    wrong = []
    for (ln, cells) in rows[1:]:
        rid = cells[0].strip('` ')
        try:
            red_cell = cells[rneg]
            test_cell = cells[tneg]
        except IndexError:
            wrong.append('%s: malformed row' % rid)
            continue
        ids = re.findall(r'CHK-[A-Za-z0-9-]+', test_cell)
        if not ids:
            wrong.append('%s: no test id' % rid)
            continue
        audit_row = rid.startswith('AU-') or rid in ('N9', 'N16', 'N17')
        is_red = False
        for i in ids:
            use_audit = audit_row or i in audit_only
            if i in (afail if use_audit else dfail):
                is_red = True
        want = 'red (FAIL' if is_red else 'PASS (guard)'
        if not red_cell.startswith(want):
            wrong.append('%s: cell starts "%s", expected "%s"' % (rid, red_cell[:24], want))
    msgs = probs + (['%d row(s) disagree with the red logs: %s' % (len(wrong), '; '.join(wrong[:12]))] if wrong else [])
    need(not msgs, ' | '.join(msgs))
    return '%d rows agree with the red logs' % (len(rows) - 1)


def a_r6(c):
    red = os.path.join(c.E, 'red', 'RED.md')
    readme = os.path.join(c.E, 'README.md')
    d = os.path.join(c.E, 'raw', 'red-checks')
    need(os.path.isdir(d), 'raw/red-checks is missing')
    n = len([f for f in os.listdir(d) if os.path.isfile(os.path.join(d, f))])
    for name, p in (('red/RED.md', red), ('README.md', readme)):
        need(os.path.isfile(p), '%s is missing' % name)
        t = read_text(p)
        need('one log per failing check' not in t, '%s still says "one log per failing check"' % name)
        blocks = re.split(r'\n\s*\n', t)
        ok = False
        for b in blocks:
            if 'raw/red-checks' in b:
                if (re.search(r'(?<![0-9])%d(?![0-9])' % n, b) and 'log' in b
                        and 'invariant row' in b):
                    ok = True
                # a table row stands alone in its block; also test per line
            for l in b.split('\n'):
                if 'raw/red-checks' in l and re.search(r'(?<![0-9])%d(?![0-9])' % n, l) \
                        and 'log' in l and 'invariant row' in (l + b):
                    ok = True
        need(ok, '%s does not say that raw/red-checks holds %d logs, one per C01 invariant row' % (name, n))
    return 'raw/red-checks holds %d logs; both files say so' % n


def sub_by_prefix(rec, prefix):
    t = rec.sub_text(prefix)
    return t


def a_r7(c):
    rec = record(c)
    t = rec.sub_text('### Checker amendments (C02)')
    need(t is not None, 'the record has no "Checker amendments (C02)" subsection')
    need(not re.search(r'no\s+message\s+changed', t, re.I), 'Checker amendments (C02) still says "no message changed"')
    need('messages-diff-c03.txt' in t, 'Checker amendments (C02) does not cite green/messages-diff-c03.txt')
    need(os.path.isfile(os.path.join(c.E, 'green', 'messages-diff-c03.txt')),
         'green/messages-diff-c03.txt is missing')
    ok = False
    for para in re.split(r'\n\s*\n', t):
        if 'messages-diff-c03.txt' in para:
            p2 = para.replace('messages-diff-c03.txt', ' ').replace('messages-before-c03.txt', ' ')
            if re.search(r'(?<![A-Za-z0-9-])\d+(?![A-Za-z0-9-])', p2) and 'message' in p2.lower():
                ok = True
    need(ok, 'Checker amendments (C02) does not state counts of the message changes')
    return 'no-message-changed sentence gone, diff cited with counts'


def read_tsv(c):
    p = os.path.join(c.E, 'probes', 'mutants.tsv')
    need(os.path.isfile(p), 'probes/mutants.tsv is missing')
    rows = []
    for l in read_lines(p):
        if not l.strip():
            continue
        f = l.split('\t')
        if f[0] == 'mutant id':
            continue
        if len(f) < 4:
            continue
        rows.append({'id': f[0], 'src': f[1], 'test': f[2], 'verdict': f[3]})
    return rows


F_MAP = {'F-C01-R1': 'AU-C31', 'F-C01-R2': 'AU-C30', 'F-C01-R3': 'N7', 'F-C01-R4': 'N3',
         'F-C01-R5': 'N5', 'F-C01-R6': 'N9', 'F-C01-R7': 'N1', 'F-C01-R8': 'N6'}


def a_probe_column(c):
    rec = record(c)
    tsv = read_tsv(c)
    nrows = 0
    problems = []
    for title in ('### Complete invariant table (C02)', '### Complete invariant table (C01)'):
        r = rec.sub(title)
        need(r is not None, 'the record has no "%s" subsection' % title[4:])
        rows = table_rows(rec.lines[r[0] + 1:r[1]], r[0] + 1)
        need(len(rows) > 1, 'the table of %s has no rows' % title[4:])
        hdr = [h.lower() for h in rows[0][1]]
        pi = [i for i, h in enumerate(hdr) if 'probe outcome' in h]
        need(pi, 'the table of %s has no probe outcome column' % title[4:])
        pneg = pi[0] - len(hdr)
        for (ln, cells) in rows[1:]:
            rid = cells[0].strip('` ')
            try:
                cell = cells[pneg]
            except IndexError:
                problems.append('%s: malformed row' % rid)
                continue
            nrows += 1
            low = cell.lower()
            if 'not run' in low or 'pending' in low or 'placeholder' in low:
                problems.append('%s: cell reads not run, pending or placeholder' % rid)
                continue
            if rid in F_MAP:
                srcs = {F_MAP[rid]}
            else:
                base = re.sub(r'^(PR|AU)-', '', rid)
                srcs = {rid, base, 'AU-' + base}
            sel = [r_ for r_ in tsv if r_['src'] in srcs and r_['verdict'] != 'control-ok']
            if not sel:
                if 'not probed' not in low:
                    problems.append('%s: no mutants in mutants.tsv, the cell must read "not probed: <reason>"' % rid)
                continue
            k = len([r_ for r_ in sel if r_['verdict'] == 'KILLED'])
            m = len(sel)
            if not re.search(r'KILLED\s+%d/%d(?!\d)' % (k, m), cell):
                problems.append('%s: expected "KILLED %d/%d"' % (rid, k, m))
                continue
            tests = sorted(set(r_['test'] for r_ in sel if r_['verdict'] == 'KILLED'))
            miss = [t for t in tests if t not in cell]
            if miss:
                problems.append('%s: killing check %s is not named' % (rid, miss[0]))
    need(not problems, '%d cell(s) disagree with mutants.tsv: %s' % (len(problems), '; '.join(problems[:12])))
    return '%d cells agree with mutants.tsv' % nrows


def bullet_paths(text):
    """Backticked path-like tokens of a text block."""
    out = []
    for line in text.split('\n'):
        for (_s, _e, t) in backticked(line):
            if re.search(r'\s', t):
                continue
            out.append(t)
    return out


def owned_expected(c):
    t = plan_text(c)
    m = re.search(r'^## 5\. Package ownership.*?(?=^## 6\.)', t, re.S | re.M)
    need(m is not None, 'section 5 of the plan INVARIANTS cannot be parsed')
    out = []
    for line in m.group(0).split('\n'):
        cells = split_cells(line)
        if not cells or len(cells) < 4 or cells[0] in ('package',) or set(cells[0]) <= set('-'):
            continue
        for tok in re.findall(r'`([^`]+)`', cells[3]):
            tok = tok.strip()
            if tok.startswith('S/'):
                continue
            if '<' in tok or '*' in tok:
                if '/' not in tok:
                    continue
                tok = tok.rsplit('/', 1)[0] + '/'
            if '/' not in tok:
                continue
            out.append(tok)
    seen = []
    for tok in out:
        if tok not in seen:
            seen.append(tok)
    return seen


def expand_owned(tok):
    for pre, full in (('E/', E_REL + '/'), ('P2/', P2_REL + '/'), ('P3/', P3_REL + '/'),
                      ('A1/', A1_REL + '/')):
        if tok.startswith(pre):
            return full + tok[len(pre):], pre == 'E/'
    return tok, False


def a_owned(c):
    probs = placement_problems(c, ['### Owned files (C03)', '### Checker amendments (C03)'])
    need(not probs, '; '.join(probs))
    rec = record(c)
    ot = rec.sub_text('### Owned files (C03)')
    toks = set(x.rstrip('/') for x in bullet_paths(ot))
    missing = []
    for tok in owned_expected(c):
        full, is_e = expand_owned(tok)
        cands = {full.rstrip('/')}
        if is_e:
            cands.add(full[len(E_REL) + 1:].rstrip('/'))
        if not (cands & toks):
            missing.append(full)
    msgs = []
    if missing:
        msgs.append('Owned files (C03) does not name %d path(s), first: %s' % (len(missing), missing[0]))
    at = rec.sub_text('### Checker amendments (C03)')
    low = at.lower()
    for need_s, what in (('CHK-I45-diff-scope', 'the CHK-I45-diff-scope frozen-map change'),
                         ('messages-diff-c03.txt', 'a citation of messages-diff-c03.txt'),
                         ('messages-before-c03.txt', 'a citation of messages-before-c03.txt'),
                         ('CHK-C02-evidence-logs', 'CHK-C02-evidence-logs (D4b)'),
                         ('CHK-C02-record-status', 'CHK-C02-record-status (D4b)')):
        if need_s not in at:
            msgs.append('Checker amendments (C03) lacks ' + what)
    if 'decided by wave 1' not in low:
        msgs.append('Checker amendments (C03) lacks "decided by wave 1"')
    if 'new engineering choice' not in low:
        msgs.append('Checker amendments (C03) lacks "new engineering choice"')
    if low.count('exception to the owned files clause') < 2:
        msgs.append('Checker amendments (C03) has fewer than two "exception to the Owned files clause" phrases')
    if 'survivor' not in low:
        msgs.append('Checker amendments (C03) does not state the survivor repairs')
    if not re.search(r'no\s+(existing\s+)?(checker\s+)?message(\s+string)?s?\s+(was\s+|were\s+)?changed', low):
        msgs.append('Checker amendments (C03) does not state that no message changed')
    need(not msgs, ' | '.join(msgs))
    return 'owned paths and amendment statements present'


def glob_hits(pattern, tok):
    pat = pattern.rstrip('/')
    t = tok.rstrip('/')
    if any(ch in pat for ch in '*?['):
        parts = t.split('/')
        for k in range(1, len(parts) + 1):
            if fnmatch.fnmatchcase('/'.join(parts[:k]), pat):
                return True
        return False
    return t == pat or t.startswith(pat + '/')


def a_staging(c):
    compare_consts(c, 'staging')
    rec = record(c)
    r = rec.sub('### Staging list (C02)')
    need(r is not None, 'the record has no "Staging list (C02)" subsection')
    stext = '\n'.join(rec.lines[r[0] + 1:r[1]])
    msgs = []
    if 'First draft (REC1; REC3 finalizes)' in stext:
        msgs.append('the Staging list intro still says "First draft (REC1; REC3 finalizes)"')
    nxt = rec.next_heading_after(r[0])
    uh = cval('UNSTAGED_HEADING')
    if nxt is None or not nxt[1].startswith(uh):
        msgs.append('"%s" is not the heading directly after the Staging list' % uh)
    toks = []
    for t in bullet_paths(stext):
        n, _lo, _hi = normalise(t)
        if path_like(t, n):
            toks.append(n)
    tokset = set(toks)
    adds = cvals('STAGE_ADD')
    uns = cvals('STAGE_UNSTAGED')
    # unstaged subsection
    ur = rec.sub(uh)
    bullets = {}
    if ur is not None:
        for l in rec.lines[ur[0] + 1:ur[1]]:
            m = re.match(r'^\s*[-*]\s+`([^`]+)`\s*:\s*(\S.*)$', l)
            if m:
                bullets[m.group(1).strip()] = m.group(2).strip()
    else:
        msgs.append('the record has no "%s" subsection' % uh)
    # candidates log
    cpath = os.path.join(c.E, 'green', 'staging-candidates-c03.txt')
    cand = {}
    if os.path.isfile(cpath):
        for l in read_lines(cpath):
            m = re.match(r'^exit (\d+) (\S+)\s*$', l)
            if m:
                cand[m.group(2)] = int(m.group(1))
    else:
        msgs.append('green/staging-candidates-c03.txt is missing')
    late = {'tests/native/v5-02-r15-a1-c01-evidence/green/' + n for n in (
        'audit-pre-rec-c02.txt', 'staging-candidates-c03.txt', 'record_asserts_c03.py',
        'rec_preflight_c03.py', 'record-asserts-red-c03.txt', 'record-asserts-c03.txt',
        'reprobe-c03.txt', 'identity-compare-c03.txt')}
    for a in adds:
        failed = cand.get(a, 0) != 0
        if failed:
            if a in tokset:
                msgs.append('%s is listed although its redaction exit in staging-candidates-c03.txt is non-zero' % a)
            reason = bullets.get(a, '')
            if 'fails redaction' not in reason:
                msgs.append('%s fails redaction but is not named with that reason in the unstaged subsection' % a)
            continue
        if a not in tokset:
            msgs.append('the Staging list does not name %s' % a)
        if os.path.isfile(cpath) and a not in cand and a not in late \
                and os.path.exists(os.path.join(c.repo, a)):
            msgs.append('staging-candidates-c03.txt has no line for %s' % a)
    for u in uns:
        reason = bullets.get(u, None)
        if reason is None:
            msgs.append('the unstaged subsection has no bullet for %s' % u)
        elif not reason.strip():
            msgs.append('the unstaged bullet for %s has no reason' % u)
    for t in toks:
        if t in adds:
            if not os.path.exists(os.path.join(c.repo, t)) and not (not c.final and fin_only_match(t)):
                msgs.append('the Staging list names a path that does not exist: %s' % t)
            continue
        pn = '/' + t + '/'
        bad = ('/raw/' in pn or '/frozen/' in pn or '/superseded-' in pn
               or t.endswith('inventory.txt') or t == 'docs/vernacular/roadmap.md'
               or any(glob_hits(u, t) for u in uns))
        if bad:
            msgs.append('the Staging list holds a forbidden path: %s' % t)
        if not os.path.exists(os.path.join(c.repo, t)) and not (not c.final and fin_only_match(t)) \
                and t not in ('',):
            msgs.append('the Staging list names a path that does not exist: %s' % t)
    seen = []
    for m_ in msgs:
        if m_ not in seen:
            seen.append(m_)
    need(not seen, ' | '.join(seen[:10]) + ('' if len(seen) <= 10 else ' (+%d more)' % (len(seen) - 10)))
    return '%d additions, %d unstaged entries' % (len(adds), len(uns))


def probes_totals(c):
    p = os.path.join(c.E, 'probes', 'PROBES.md')
    need(os.path.isfile(p), 'probes/PROBES.md is missing')
    t = read_text(p)
    trip = [tuple(int(x) for x in m) for m in
            re.findall(r'totals:\s*killed (\d+), survived (\d+), not run (\d+)', t)]
    return trip


def a_final_results(c):
    probs = placement_problems(c, ['### Final results (C03)'])
    need(not probs, '; '.join(probs))
    rec = record(c)
    r = rec.sub('### Final results (C03)')
    t = '\n'.join(rec.lines[r[0] + 1:r[1]])
    msgs = []

    def has(s, why=None):
        if s not in t:
            msgs.append('lacks "%s"' % s)

    for s in ('PASS count: 113', 'FAIL count: 0', 'exit 0', 'green/checks.txt',
              '25 of 25', 'green/sql-final.txt', 'files checked: 11', 'broken: 0',
              'negative control', 'green/linkcheck-c02.txt', 'every line exit 0',
              'positive control', 'green/redaction-c02.txt', 'green/audit-final.txt',
              'FIN2', 'probes/PROBES.md', 'probes/mutants.tsv', 'green/identity-c02.txt'):
        has(s)
    if re.search(r'links checked:?\s*\d', t):
        msgs.append('states a "links checked" figure for a log FIN2 rewrites')
    if re.search(r'exit 0 lines?[:=]?\s*\d', t) or re.search(r'(?<![0-9])\d+\s+exit 0 lines?', t):
        msgs.append('states a count of exit 0 lines')
    if re.search(r'audit-final\.txt`?[:\s,(]*\d', t) or re.search(r'audit-final\.txt[^\n]{0,40}\b\d+\s*(PASS|FAIL)', t):
        msgs.append('states a count after audit-final.txt')
    # identity combined sha256 as pasted
    ip = os.path.join(c.E, 'green', 'identity-c02.txt')
    if os.path.isfile(ip):
        comb = None
        inblock = False
        for l in read_lines(ip):
            if l.startswith('identity begin'):
                inblock = True
            elif l.startswith('identity end'):
                break
            elif inblock and l.startswith('| combined sha256'):
                mm = re.findall(r'`([0-9a-f]{64})`', l)
                if mm:
                    comb = mm[-1]
        if comb is None:
            msgs.append('green/identity-c02.txt has no combined sha256 row')
        elif comb not in t:
            msgs.append('the combined sha256 of green/identity-c02.txt is not pasted')
    else:
        msgs.append('green/identity-c02.txt is missing')
    # totals
    allowed = set(probes_totals(c))
    pt = list(allowed)
    if len(pt) >= 2:
        allowed.add(tuple(sum(x[k] for x in pt) for k in range(3)))
    rp = os.path.join(c.E, 'green', 'reprobe-c03.txt')
    if os.path.isfile(rp):
        for mm in re.findall(r're-probe totals:\s*killed (\d+), survived (\d+), not run (\d+)', read_text(rp)):
            allowed.add(tuple(int(x) for x in mm))
    got = [tuple(int(x) for x in mm) for mm in
           re.findall(r'killed (\d+), survived (\d+), not run (\d+)', t)]
    if not got:
        msgs.append('lacks the totals "killed K, survived S, not run N"')
    elif not any(g in allowed for g in got):
        msgs.append('the totals differ from the PROBES.md totals lines')
    # eight-row table
    rows = table_rows(r_lines(rec, r), r[0] + 1)
    crit = {}
    for (ln, cells) in rows:
        if cells and re.match(r'^\d+$', cells[0]):
            crit[int(cells[0])] = (ln, cells)
    for k in range(1, 9):
        if k not in crit:
            msgs.append('criterion row %d is missing' % k)
            continue
        ln, cells = crit[k]
        row = rec.lines[ln - 1]
        if any('not met' in x.lower() for x in cells):
            msgs.append('criterion row %d reads "not met"' % k)
            continue
        st = [x for x in cells[1:] if x in cvals('SECT_STATUS')]
        if len(st) != 1:
            msgs.append('criterion row %d has no status word met or "shown by FIN log"' % k)
            continue
        problem = cite_problem(c, row, st[0])
        if problem:
            msgs.append('criterion row %d: %s' % (k, problem))
    need(not msgs, ' | '.join(msgs))
    return 'Final results (C03) holds the pinned facts'


def r_lines(rec, r):
    return rec.lines[r[0] + 1:r[1]]


def log_token(norm):
    if any(norm.startswith(d) for d in cvals('SECT_LOGDIR')):
        return True
    return bool(re.match(r'^tests/native/v5-02-r15-a1-c0[123]-evidence/(green|red|probes|raw)/', norm))


def cite_exists(c, norm):
    if glob_chars(norm):
        return None
    for b in ([c.repo] if norm.startswith('tests/') else [c.E, c.P2, c.P3, c.repo]):
        if os.path.exists(os.path.join(b, norm)):
            return True
    return False


def cite_problem(c, row, status):
    """None when the row's log cites satisfy the status rule."""
    toks = []
    for (_s, _e, t) in backticked(row):
        if re.search(r'\s', t):
            continue
        n, _lo, _hi = normalise(t)
        if log_token(n):
            toks.append(n)
    if not toks:
        return 'cites no log under green/, red/, probes/ or raw/'
    existing_non_fin = 0
    for n in toks:
        ex = cite_exists(c, n)
        if ex is None:
            continue
        fin = fin_only_match(n)
        if ex:
            if not fin:
                existing_non_fin += 1
            continue
        if fin and not c.final and status == 'shown by FIN log':
            continue
        return 'cites a log that does not exist: %s' % n
    if status == 'met' and existing_non_fin == 0:
        return 'reads met but cites no existing log that is not FIN-only'
    return None


def a_identity_ref(c):
    rec = record(c)
    ip = os.path.join(c.E, 'green', 'identity-c02.txt')
    need(os.path.isfile(ip), 'green/identity-c02.txt is missing')
    block = []
    inb = False
    for l in read_lines(ip):
        if l.startswith('identity begin (seven documents)'):
            inb = True
            continue
        if l.startswith('identity end'):
            break
        if inb:
            block.append(l)
    need(block, 'green/identity-c02.txt has no seven-document identity block')
    head = tracked = comb = None
    files = []
    for l in block:
        if l.startswith('| HEAD'):
            mm = re.findall(r'`([0-9a-f]{40})`', l)
            head = mm[-1] if mm else None
        elif l.startswith('| sha256(git diff'):
            mm = re.findall(r'`([0-9a-f]{64})`', l)
            tracked = mm[-1] if mm else None
        elif l.startswith('| combined sha256'):
            mm = re.findall(r'`([0-9a-f]{64})`', l)
            comb = mm[-1] if mm else None
        else:
            mm = re.match(r'^\|\s*`([^`]+)`\s*\|\s*([A-Za-z ]+?)\s*\|\s*`([0-9a-f]{64})`\s*\|\s*$', l)
            if mm:
                files.append((mm.group(1), mm.group(3)))
    need(head and tracked and comb and files, 'the identity block of green/identity-c02.txt is incomplete')
    msgs = []
    # fact 1
    ct = rec.sub_text('### Candidate currency')
    if ct is None:
        msgs.append('the record has no "Candidate currency" subsection')
    else:
        flat = re.sub(r'\s+', ' ', ct)
        got = {
            'HEAD': re.search(r'HEAD `([0-9a-f]{40})`', flat),
            'tracked': re.search(r'tracked diff sha256[^`]*`([0-9a-f]{64})`', flat),
            'combined': re.search(r'combined sha256 `([0-9a-f]{64})`', flat),
            'execution-contract.md': re.search(r'`execution-contract\.md`[^`]*`([0-9a-f]{64})`', flat),
        }
        ec = [h for p, h in files if p.endswith('execution-contract.md')]
        want = {'HEAD': head, 'tracked': tracked, 'combined': comb,
                'execution-contract.md': ec[0] if ec else None}
        for k in ('HEAD', 'tracked', 'combined', 'execution-contract.md'):
            if got[k] is None:
                msgs.append('Candidate currency lacks the %s value' % k)
            elif got[k].group(1) != want[k]:
                msgs.append('Candidate currency %s differs from green/identity-c02.txt' % k)
    # fact 2
    fi = rec.sub_text('### Final candidate identity')
    if fi is None:
        msgs.append('the record has no "Final candidate identity" subsection')
    else:
        rows = set()
        for l in fi.split('\n'):
            mm = re.match(r'^\|\s*`([^`]+)`\s*\|\s*([A-Za-z ]+?)\s*\|\s*`([0-9a-f]{64})`\s*\|\s*$', l)
            if mm:
                rows.add((mm.group(1), mm.group(3)))
        for p, h in files:
            if p.endswith('execution-contract.md'):
                continue
            if (p, h) not in rows:
                msgs.append('per-file row of %s differs between green/identity-c02.txt and Final candidate identity' % p)
    # fact 3
    snap = os.path.join(c.E, 'raw', 'pre-rec-record-c03.md')
    if not os.path.isfile(snap):
        msgs.append('raw/pre-rec-record-c03.md is missing')
    else:
        def idrows(lines):
            out = {}
            for l in lines:
                s = l.strip()
                if (re.match(r'^\|\s*`[^`]+`\s*\|\s*[A-Za-z ]+\|\s*`[0-9a-f]{64}`\s*\|\s*$', s)
                        or re.match(r'^\|\s*(HEAD|sha256\(git diff|combined sha256)', s)):
                    out[s] = out.get(s, 0) + 1
            return out
        a = idrows(read_lines(snap))
        b = idrows(rec.lines)
        lost = [k for k in a if b.get(k, 0) < a[k]]
        if lost:
            msgs.append('%d identity row(s) of the pre-REC record changed or lost, first: %s'
                        % (len(lost), lost[0][:60]))
    need(not msgs, ' | '.join(msgs))
    return 'identity values agree'


def parse_counts(path):
    lines = read_lines(path)
    cnt = [l for l in lines if re.match(r'^runner counts: PASS count: \d+; FAIL count: \d+; registered count: \d+\s*$', l)]
    if len(cnt) != 1:
        return None, 'expected one "runner counts:" line, found %d' % len(cnt)
    m = re.match(r'^runner counts: PASS count: (\d+); FAIL count: (\d+); registered count: (\d+)', cnt[0])
    nonempty = [i for i, l in enumerate(lines) if l.strip()]
    ci = lines.index(cnt[0])
    mi = [i for i in nonempty if lines[i].startswith('masked:')]
    if not mi or ci > mi[-1]:
        return None, 'the counts line does not precede the mask line'
    return tuple(int(x) for x in m.groups()), None


def a_runner(c):
    tsv = read_tsv(c)
    raw = os.path.join(c.E, 'probes', 'raw')
    need(tsv, 'probes/mutants.tsv has no rows')
    bad = []
    for r_ in tsv:
        p = os.path.join(raw, r_['id'] + '.log')
        if not os.path.isfile(p):
            bad.append('%s: log missing' % r_['id'])
            continue
        cnt, err = parse_counts(p)
        if err:
            bad.append('%s: %s' % (r_['id'], err))
            continue
        P, F, R = cnt
        if r_['verdict'] == 'KILLED':
            if not (F >= 1 and P + F == R):
                bad.append('%s: KILLED needs F >= 1 and P + F = R (P %d, F %d, R %d)' % (r_['id'], P, F, R))
        elif r_['verdict'] == 'control-ok':
            if not (F == 0 and P == R):
                bad.append('%s: control needs F = 0 and P = R (P %d, F %d, R %d)' % (r_['id'], P, F, R))
    for b in ('baseline-c02-default.log', 'baseline-c02-audit.log', 'baseline-c02-audit-closed.log'):
        p = os.path.join(raw, b)
        if not os.path.isfile(p):
            bad.append('%s: baseline missing' % b)
            continue
        cnt, err = parse_counts(p)
        if err:
            bad.append('%s: %s' % (b, err))
            continue
        P, F, R = cnt
        want = 113 if 'default' in b else 122
        if not (F == 0 and P == R and R == want):
            bad.append('%s: baseline needs F = 0, P = R = %d (P %d, F %d, R %d)' % (b, want, P, F, R))
    need(not bad, '%d log(s) break the runner rule: %s' % (len(bad), '; '.join(bad[:10])))
    return '%d logs and 3 baselines satisfy the runner rule' % len(tsv)


def section_status_rows(c, rec, title, ten):
    r = rec.sub(title)
    if r is None:
        raise Fail('the record has no "%s" subsection' % title[4:])
    rows = table_rows(r_lines(rec, r), r[0] + 1)
    if len(rows) < 2:
        raise Fail('"%s" has no table' % title[4:])
    hdr = [h.lower() for h in rows[0][1]]
    si = [i for i, h in enumerate(hdr) if h == 'status']
    if not si:
        raise Fail('"%s" has no status column' % title[4:])
    out = []
    for (ln, cells) in rows[1:]:
        first = cells[0].strip('` ')
        if ten:
            if not re.match(r'^\d+$', first) or not (1 <= int(first) <= 10):
                continue
        out.append((ln, first, cells, si[0]))
    return out


def a_sections(c):
    rec = record(c)
    msgs = []
    # (1) section references
    heads = [t.lstrip('#').strip() for (_i, _lv, t) in rec.headings]
    ex_ranges = []
    for p in cvals('SECT_MENTION_EXEMPT'):
        r = rec.sub(p)
        if r is not None:
            ex_ranges.append(r)
    for i, line in enumerate(rec.lines):
        if rec.is_coord(i):
            continue
        if any(a <= i < b for (a, b) in ex_ranges):
            continue
        for m in re.finditer(r'Final results \((C0[0-9])\)|(C0[0-9]) Final results', line):
            n = m.group(1) or m.group(2)
            if not any(h.startswith('Final results (%s)' % n) for h in heads):
                msgs.append('line %d mentions "%s" but no heading "Final results (%s)" exists' % (i + 1, m.group(0), n))
    # (2) status rows
    banned_words = re.compile(r'\b(not met|partly met|pending|provisional)\b', re.I)
    for title, ten in (('### (l2)', True), ('### C02 acceptance status', False)):
        try:
            rows = section_status_rows(c, rec, title, ten)
        except Fail as e:
            msgs.append(str(e))
            continue
        if ten:
            have = set(int(x[1]) for x in rows)
            for k in range(1, 11):
                if k not in have:
                    msgs.append('(l2) row %d is missing' % k)
        if not rows:
            msgs.append('"%s" has no rows' % title[4:])
        for (ln, first, cells, si) in rows:
            label = '%s row %s' % (title[4:], first)
            row = rec.lines[ln - 1]
            try:
                status = cells[si]
            except IndexError:
                msgs.append('%s: malformed' % label)
                continue
            words = [s for s in cvals('SECT_STATUS') if status == s]
            if len(words) != 1 or banned_words.search(status):
                msgs.append('%s: the status "%s" is not exactly one of met or "shown by FIN log"' % (label, status[:20]))
                continue
            if banned_words.search(row):
                msgs.append('%s: the row holds a banned status word (not met, partly met, pending, provisional)' % label)
                continue
            prob = cite_problem(c, row, status)
            if prob:
                msgs.append('%s: %s' % (label, prob))
    # (3) banned phrases
    for i, line in enumerate(rec.lines):
        if rec.is_coord(i):
            continue
        for ph in cvals('SECT_BANNED'):
            if ph in line:
                msgs.append('line %d still holds "%s"' % (i + 1, ph))
    seen = []
    for m_ in msgs:
        if m_ not in seen:
            seen.append(m_)
    need(not seen, '%d problem(s): %s' % (len(seen), '; '.join(seen[:12])))
    return 'section references and status rows agree'


def a_order(c):
    names = [('docs/vernacular/rounds/V5-02-R15-A1.md', c.record_path),
             ('green/record-asserts-c03.txt', os.path.join(c.E, 'green/record-asserts-c03.txt')),
             ('green/reprobe-c03.txt', os.path.join(c.E, 'green/reprobe-c03.txt')),
             ('green/identity-compare-c03.txt', os.path.join(c.E, 'green/identity-compare-c03.txt')),
             ('green/checks.txt', os.path.join(c.E, 'green/checks.txt')),
             ('green/redaction-c02.txt', os.path.join(c.E, 'green/redaction-c02.txt')),
             ('green/audit-final.txt', os.path.join(c.E, 'green/audit-final.txt')),
             ('green/audit-verify-c02.txt', os.path.join(c.E, 'green/audit-verify-c02.txt')),
             ('green/redaction-final-c02.txt', os.path.join(c.E, 'green/redaction-final-c02.txt'))]
    msgs = []
    present = []
    for (lab, p) in names:
        if os.path.isfile(p):
            present.append((lab, os.stat(p).st_mtime_ns))
        elif c.final:
            msgs.append('%s is absent in final mode' % lab)
    for i in range(len(present)):
        for j in range(i + 1, len(present)):
            if present[i][1] > present[j][1]:
                msgs.append('%s is newer than %s' % (present[i][0], present[j][0]))
    rp = os.path.join(c.E, 'green/redaction-c02.txt')
    if os.path.isfile(rp):
        rt = read_text(rp)
        for n in ('record-asserts-c03.txt', 'reprobe-c03.txt', 'identity-compare-c03.txt'):
            exists = os.path.isfile(os.path.join(c.E, 'green', n))
            if (exists or c.final) and not re.search(r'^exit \d+ .*' + re.escape(n) + r'\s*$', rt, re.M):
                msgs.append('green/redaction-c02.txt has no line for %s' % n)
    need(not msgs, '; '.join(msgs[:10]))
    return '%d of 9 files present, mtime chain holds' % len(present)


RP_IDS = (['N3-m%d' % k for k in range(1, 6)] + ['N4-m1'] + ['N5-m%d' % k for k in range(1, 4)]
          + ['N6-m1', 'N6-m2'] + ['N7-m%d' % k for k in range(1, 9)] + ['N10-m1']
          + ['N11-m%d' % k for k in range(1, 5)] + ['N12-m1', 'N12-m2', 'N13-m1', 'N13-m2',
                                                   'N14-m1', 'N14-m2', 'C3-A6-m1',
                                                   'C32-m1', 'C32-m2']
          + ['C61-m%d' % k for k in range(1, 5)] + ['N16-m1', 'N16-m2', 'N9-m9', 'N9-m14'])


def a_reprobe(c):
    p = os.path.join(c.E, 'green', 'reprobe-c03.txt')
    if not os.path.isfile(p):
        need(not c.final, 'green/reprobe-c03.txt is absent in final mode')
        return 'deferred (reprobe-c03.txt absent in non-final mode)'
    t = read_text(p)
    msgs = []
    verdicts = {}
    for l in t.split('\n'):
        m = re.match(r'^(\S+)-r\s+(KILLED|SURVIVED|not run)\s*$', l)
        if m:
            verdicts[m.group(1)] = m.group(2)
    for i in RP_IDS:
        if i not in verdicts:
            msgs.append('no line for %s-r' % i)
        elif verdicts[i] != 'KILLED':
            msgs.append('%s-r is %s' % (i, verdicts[i]))
        if not os.path.isfile(os.path.join(c.E, 'probes', 'reprobe', i + '-r.log')):
            msgs.append('probes/reprobe/%s-r.log is missing' % i)
    tot = re.findall(r'^re-probe totals: killed (\d+), survived (\d+), not run (\d+)\s*$', t, re.M)
    if len(tot) != 1:
        msgs.append('the "re-probe totals:" line is missing or repeated')
    else:
        K = len([v for v in verdicts.values() if v == 'KILLED'])
        S = len([v for v in verdicts.values() if v == 'SURVIVED'])
        N = len([v for v in verdicts.values() if v == 'not run'])
        if tuple(int(x) for x in tot[0]) != (K, S, N):
            msgs.append('the re-probe totals differ from the lines (%d, %d, %d)' % (K, S, N))
    need(not msgs, '; '.join(msgs[:10]))
    return '%d re-probes verified' % len(RP_IDS)


def hash_sources(c):
    files = {}
    for pat in cvals('HASH_SOURCE'):
        for p in glob.glob(os.path.join(c.repo, pat), recursive=True):
            if os.path.isfile(p):
                files[os.path.relpath(p, c.repo)] = p
    return files


def a_record_rule(c):
    rec = record(c)
    srcs = hash_sources(c)
    hashes = {}
    for rel, p in srcs.items():
        hashes.setdefault(sha256_file(p), rel)
    r = rec.sub('### Staging list (C02)')
    need(r is not None, 'the record has no Staging list (C02) subsection')
    targets = []
    for t in bullet_paths('\n'.join(r_lines(rec, r))):
        n, _lo, _hi = normalise(t)
        p = os.path.join(c.repo, n)
        if os.path.isfile(p):
            targets.append(n)
    ur = rec.sub(cval('UNSTAGED_HEADING'))
    if ur is not None:
        for l in r_lines(rec, ur):
            m = re.match(r'^\s*[-*]\s+`([^`]+)`', l)
            if m:
                n = normalise(m.group(1))[0]
                if glob_chars(n):
                    continue
                p = os.path.join(c.repo, n)
                if os.path.isfile(p):
                    targets.append(n)
    skip = set(cvals('HASH_SKIP_SELF'))
    msgs = []
    scanned = 0
    for n in sorted(set(targets)):
        if n in srcs or n in skip:
            continue
        p = os.path.join(c.repo, n)
        try:
            data = open(p, 'rb').read()
        except OSError:
            continue
        if b'\x00' in data:
            continue
        try:
            text = data.decode('utf-8')
        except UnicodeDecodeError:
            continue
        scanned += 1
        for ln, line in enumerate(text.split('\n'), 1):
            for m in re.finditer(r'(?<![0-9a-fA-F])[0-9a-fA-F]{64}(?![0-9a-fA-F])', line):
                h = m.group(0).lower()
                if h in hashes:
                    msgs.append('%s line %d holds the sha256 of %s' % (n, ln, hashes[h]))
    need(not msgs, '; '.join(msgs[:10]))
    return '%d files scanned against %d hash sources' % (scanned, len(hashes))


FUNCS = {'msgs': a_msgs, 'freeze-red': a_freeze_red, 'paths': a_paths, 'closure': a_closure,
         'C55': a_c55, 'R4': a_r4, 'R5': a_r5, 'R6': a_r6, 'R7': a_r7,
         'probe-column': a_probe_column, 'owned': a_owned, 'staging': a_staging,
         'final-results': a_final_results, 'identity-ref': a_identity_ref, 'runner': a_runner,
         'sections': a_sections, 'order': a_order, 'reprobe': a_reprobe,
         'record-rule': a_record_rule}


def main(argv=None):
    ap = argparse.ArgumentParser(add_help=True)
    ap.add_argument('--repo', required=False)
    ap.add_argument('--evidence')
    ap.add_argument('--record')
    ap.add_argument('--plan')
    ap.add_argument('--phase', choices=['early', 'final', 'all'], default='all')
    ap.add_argument('--final', action='store_true')
    ap.add_argument('--print-constants', action='store_true')
    args = ap.parse_args(argv)
    if args.print_constants:
        for n, v in CONSTS:
            print('CONST %s %s' % (n, v))
        return 0
    if not args.repo:
        print('FAIL ASSERT-C03-args: --repo is required')
        return 2
    c = make_ctx(args)
    names = {'early': EARLY, 'final': FINAL, 'all': ORDER}[args.phase]
    names = [n for n in ORDER if n in names]

    def clean(s):
        s = str(s).replace(c.repo, '<repo>')
        return s.replace('\n', ' ')

    nfail = 0
    for n in names:
        try:
            note = FUNCS[n](c)
            print('PASS ASSERT-C03-%s%s' % (n, (': ' + clean(note)) if note else ''))
        except Fail as e:
            nfail += 1
            print('FAIL ASSERT-C03-%s: %s' % (n, clean(e)))
        except Exception as e:  # report, never a traceback
            nfail += 1
            print('FAIL ASSERT-C03-%s: error %s: %s' % (n, type(e).__name__, clean(e)))
    ex = sorted(set(c.exempt), key=lambda x: (x[1], x[0], x[2]))
    print('exemptions used: %d' % len(ex))
    for (cat, ln, tok, reason) in ex:
        print('EXEMPT %s line %d `%s`: %s' % (cat, ln, clean(tok), reason))
    print('mode: %s, phase: %s' % ('final' if c.final else 'non-final', args.phase))
    return 1 if nfail else 0


if __name__ == '__main__':
    sys.exit(main())
