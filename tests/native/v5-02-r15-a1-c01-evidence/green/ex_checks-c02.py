#!/usr/bin/env python3
# Scratch checks for the V5-02-R15-A1 execution contract (CHK-I* ids from
# tests/native/v5-02-r15-a1-evidence/plan/INVARIANTS.md). Stdlib only.
# C02 copy (V5-02-R15-A1-C02): derived from the C01 checker red/ex_checks.py, which stays untouched.
# It appends the 17 CHK-C02 ids, amends the audit bodies named in the record's 'Checker amendments (C02)'
# and adds the audit option --pending closing|acceptance.
#
# Usage: python3 -I ex_checks-c02.py --repo <root> [--doc <path>] [--audit [--pending closing|acceptance]...] [CHK-id ...]
# Prints one line per check: "PASS|FAIL CHK-id detail". Exit 1 on any FAIL.
# The contract is read only from --doc (default docs/vernacular/execution-contract.md).
# A missing document is reported as "FAIL <id> document missing: <path>".
import argparse
import glob
import hashlib
import os
import re
import sqlite3
import subprocess
import sys
import tempfile
import urllib.parse

DOC_REL = 'docs/vernacular/execution-contract.md'
DESIGN_REL = 'docs/vernacular/execution-design.md'
V5_REL = 'docs/vernacular/planar-design-spec-v5.md'
LEDGER_REL = 'docs/vernacular/ledger-contract.md'
EXT_REL = 'docs/vernacular/external-profile-contract.md'
LOAD_REL = 'docs/vernacular/load-command-contract.md'
AUTH_REL = 'docs/vernacular/authoring-contract.md'
ALIGN_REL = 'docs/vernacular/v5-alignment.md'
IMPL_REL = 'docs/vernacular/implementation.md'
MATRIX_REL = 'docs/vernacular/acceptance-matrix.md'
RENDER_REL = 'docs/vernacular/render-contract.md'
ROUND_REL = 'docs/vernacular/rounds/V5-02-R15-A1.md'
EVID_REL = 'tests/native/v5-02-r15-a1-evidence'
DECISIONS_REL = EVID_REL + '/plan/DECISIONS.md'
README_REL = EVID_REL + '/README.md'
RESPONSE_REL = 'src/cmd/planar/response.cpp'
SELECTION_REL = 'src/stdlib/planar/selection.lua'
# R15-A1-C01 (documentation corrective): evidence folder, plan tables and frozen copies.
C01_EVID_REL = 'tests/native/v5-02-r15-a1-c01-evidence'
INVARIANTS_REL = EVID_REL + '/plan/INVARIANTS.md'
FROZEN_REVIEW_REL = 'docs/vernacular/rounds/V5-02-R15-A1-review.md'
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
DELEGATED_TAG = 'operator-decided (delegated 2026-10-31, option A)'
C02_EVID_REL = 'tests/native/v5-02-r15-a1-c02-evidence'
C01_REVIEW_REL = 'docs/vernacular/rounds/V5-02-R15-A1-C01-review.md'
C02_REVIEW_REL = 'docs/vernacular/rounds/V5-02-R15-A1-C02-review.md'
C03_REVIEW_REL = 'docs/vernacular/rounds/V5-02-R15-A1-C03-review.md'
MASK_LINE = 'masked: yes (paths and forbidden-file hashes replaced after the run; exit codes and FAIL ids untouched)'
# The six audit ids of the C01 checker (fixed set, used by CHK-C01-red-evidence).
C01_AUDIT_IDS = frozenset([
    'CHK-C01-sql-evidence-recorded', 'CHK-C01-green-evidence', 'CHK-C01-prober-evidence',
    'CHK-C01-record-closure', 'CHK-C01-red-evidence', 'CHK-C01-record-d81'])
# C02 audit ids: they audit files written by the later waves of C02.
C02_AUDIT_IDS = frozenset(['CHK-C02-evidence-logs', 'CHK-C02-identity-block', 'CHK-C02-logs-masked'])
AUDIT_IDS = C01_AUDIT_IDS | C02_AUDIT_IDS
# Mutation-prober rows that the round record names as replaced by a C01 check (DECISIONS section 6).
REPLACED_MAP = {'I54': 'C39', 'I57': 'C40', 'I33': 'C41', 'I32': 'C42', 'I59': 'C43', 'I2': 'C44',
                'I5': 'C45', 'I47': 'C46', 'I53': 'C47', 'I67': 'C48', 'I8': 'C49', 'I39': 'C50',
                'I42': 'C51', 'I46': 'C52', 'I64': 'C53', 'I66': 'C54', 'I60': 'C55', 'I69': 'C56',
                'I48': 'C57'}
# Rows whose expected red outcome (C.33) is a FAIL of the named check on the pre-C01 candidate.
RED_ROWS = {
    'C1': 'CHK-C01-R1-token-status-lookup', 'C2': 'CHK-C01-R1-status-qualifier',
    'C3': 'CHK-C01-R1-a09-lookup', 'C4': 'CHK-C01-R2-event-reason-ordinal1',
    'C6': 'CHK-C01-R3-op1-rule-unchanged', 'C7': 'CHK-C01-R4-role-state-run-drift',
    'C8': 'CHK-C01-R4-transitions-drifted', 'C9': 'CHK-C01-R4-run-id-replay-text',
    'C10': 'CHK-C01-R4-states-text', 'C11': 'CHK-C01-R5-c6-text',
    'C12': 'CHK-C01-R5-evidence-reason-check', 'C13': 'CHK-C01-decisions-rows',
    'C14': 'CHK-C01-R7-header-scope', 'C15': 'CHK-C01-R8-m-note',
    'C16': 'CHK-C01-R9-d56-basis', 'C17': 'CHK-C01-R9-d55-corruption',
    'C18': 'CHK-C01-R10-alignment-pointers', 'C19': 'CHK-C01-R11-cutover-target',
    'C20': 'CHK-C01-R12-ex8-label', 'C21': 'CHK-C01-relabel-contract',
    'C22': 'CHK-C01-relabel-record', 'C23': 'CHK-C01-relabel-no-live',
    'C39': 'CHK-I54-run-token', 'C40': 'CHK-I57-payload-version',
    'C41': 'CHK-I33-confirm-stopped', 'C42': 'CHK-I32-decisions-table',
    'C43': 'CHK-I59-record-additions', 'C48': 'CHK-I67-column-conventions',
    'C57': 'CHK-I48-enum-checks'}
# Mutation-prober row ids of INVARIANTS sections 1 to 3 (C1 to C61 with C59b, and G1 to G8).
C_ROW_IDS = (['C%d' % n for n in range(1, 61)] + ['C59b', 'C61'] + ['G%d' % n for n in range(1, 9)])
# Documents this round edits (link, cite and diagnostic checks).
EDITED_DOCS = ['CLAUDE.md', LEDGER_REL, EXT_REL, MATRIX_REL, ALIGN_REL, IMPL_REL]
OWNED_PATHS = {DOC_REL, 'CLAUDE.md', LEDGER_REL, EXT_REL, MATRIX_REL, ALIGN_REL,
               IMPL_REL, ROUND_REL}
MATRIX_ALLOWED_OLD = {31: 'A09', 32: 'A10', 42: 'A20', 44: 'A22', 45: 'A23', 48: 'A26',
                      55: 'A33'}

H2_EXPECT = ['Common rules', 'EX-1 Format3', 'EX-2 Request framing and replay',
             'EX-3 Execution commit protocol', 'EX-4 Facts, derived state and reconcile rules',
             'EX-5 Role structure, lineage and work_exec_v1', 'EX-6 Interpretation binding',
             'EX-7 Error codes and messages', 'EX-8 Execution bounds', 'State machines',
             'Part 2 placeholders (R15-A2)', 'Messages', 'Pointer amendments', 'Traceability',
             'Acceptance checklist', 'Decisions table']
PART2_TITLES = ['verbs and services', 'dispatch projection and dispatch_manifest_v1',
                'work_exec_v1 field recipe', 'provenance (driver_provenance_v1)',
                'history --event selector', 'orchestrator live form',
                'command, render and load pointer amendments']
TABLES = ['ledger_meta', 'records', 'tree_edges', 'revisions', 'events', 'current_state',
          'requests', 'job_activations', 'generations', 'runs', 'dispatches', 'checkpoints',
          'reservations', 'evidence', 'completions', 'publications', 'verdicts', 'repairs',
          'blockers', 'role_state', 'run_state', 'reservation_state', 'blocker_state',
          'transitions']
MUTABLE = {'current_state', 'role_state', 'run_state', 'reservation_state', 'blocker_state'}
RESERVED_LABEL = {'checkpoints': 'R16', 'reservations': 'R17', 'evidence': 'R17',
                  'completions': 'R17', 'publications': 'R17', 'reservation_state': 'R17',
                  'verdicts': 'R18', 'repairs': 'R18', 'blockers': 'R18',
                  'blocker_state': 'R18'}
EVENT14 = ['ledger_initialized', 'declaration_accepted', 'load_noop', 'job_activated',
           'run_claimed', 'run_taken_over', 'run_refreshed', 'checkpoint_recorded',
           'run_released', 'run_interrupted', 'check_reserved', 'check_recorded',
           'check_interrupted', 'verdict_recorded']
ADDED11 = EVENT14[3:]
REASONS16 = ['activated', 'prerequisite_available', 'prerequisite_unavailable', 'claimed',
             'taken_over', 'fenced', 'work_changed', 'structural_change', 'retired',
             'recreated', 'drift_cleared', 'released', 'interrupted', 'completed',
             'evidence_inapplicable', 'repair_admitted']
STATES6 = ['blocked', 'ready', 'running', 'done', 'stale', 'retired']
RUN_STATES = ['active', 'released', 'interrupted', 'completed', 'fenced']
REQUEST_OPS = ['load', 'activate', 'claim', 'takeover', 'refresh', 'checkpoint', 'release',
               'check', 'check-recover', 'verdict']
OUTCOMES9 = ['accepted', 'noop', 'activated', 'claimed', 'taken_over', 'refreshed',
             'recorded', 'released', 'interrupted']
GEN_REASONS = ['activated', 'structural_change', 'takeover', 'recreated']
DISPATCH_KINDS = ['claim', 'takeover', 'refresh']
EVIDENCE_OUTCOMES = ['pass', 'fail', 'inconclusive']
APPLICABILITY = ['current', 'historical']
DECISIONS_ENUM = ['approve', 'reject']
BLOCKER_KINDS = ['unowned_rejection', 'max_rounds']
MSG_CODES = [
    ('no_active_job', 'No active job matches the request'),
    ('claim_conflict', 'Role state does not allow this operation'),
    ('state_conflict', 'Execution state changed since the request was prepared'),
    ('fenced', 'Run is not current'),
    ('interpretation_mismatch', 'Interpretation does not match the active revision'),
    ('history_unavailable', 'History needed for this request is unavailable'),
    ('drifted', 'Role work changed since it was acknowledged'),
    ('check_in_progress', 'A check is already in progress for this role'),
    ('unsupported_policy', 'Execution policy is not supported'),
    ('resource_limit', 'Execution resource limit exceeded'),
]
MSG_RESOURCE = 'Execution resource limit exceeded'
FALLBACK_OPS = ['Activate', 'Claim', 'Takeover', 'Refresh', 'Checkpoint', 'Release', 'Check',
                'Check-recover', 'Verdict']
CODE_ROUNDS = {'no_active_job': 'R15', 'claim_conflict': 'R15', 'state_conflict': 'R15',
               'fenced': 'R15', 'interpretation_mismatch': 'R15', 'history_unavailable': 'R16',
               'drifted': 'R16', 'check_in_progress': 'R17', 'unsupported_policy': 'R18'}
V5_KEYS = {374: 'Never completed', 375: 'Current token claimed', 376: 'Running inputs change',
           377: 'Session incorporates', 378: 'Applicable checks pass',
           379: 'Required check fails', 380: 'Done work inputs',
           381: 'Start prerequisite absent', 382: 'Blocker clears with unchanged',
           383: 'Blocker clears with obsolete', 384: 'Release or driver', 385: 'Role removed'}
PRECEDENCE = [('retired', {385}), ('running', {375, 376}), ('blocked', {381}),
              ('done', {378, 382}), ('stale', {380, 383}), ('ready', {374})]
DESIGN_IDS_REQUIRED = [
    'D13', 'D15', 'D17', 'D19', 'D34', 'D37', 'D43', 'D44', 'D51', 'D57', 'D58', 'D60', 'D69',
    'D70', 'D71', 'D72', 'D73', 'D74', 'D75', 'D80', 'D82', 'D83', 'D85', 'D86', 'D87', 'D88',
    'D89', 'D90', 'D91', 'D92', 'D93', 'D94', 'D95', 'D96']
READER_VERIFIED = [('events', 'generation_id'), ('events', 'run_id'),
                   ('current_state', 'active_activation_id'), ('reservations', 'request_id'),
                   ('events', 'request_id')]
MESSAGES_19 = [m for _, m in MSG_CODES] + [
    '%s may be complete; retry the same request ID or inspect request history' % op
    for op in FALLBACK_OPS]


class CheckFail(Exception):
    pass


class MissingDoc(Exception):
    pass


CHECKS = {}
ORDER = []


def check(cid):
    def deco(fn):
        CHECKS[cid] = fn
        ORDER.append(cid)
        return fn
    return deco


# ---------------------------------------------------------------- text helpers

def ns(s):
    """Remove whitespace and backticks for layout-insensitive comparison."""
    return re.sub(r'[\s`]', '', s)


def norm(s):
    return ' '.join(s.split())


def clean(c):
    return c.replace('`', '').replace('**', '').strip()


FENCE_RE = re.compile(r'^\s*(```|~~~)')


def heading_list(text):
    """Return (index, level, title) for headings outside fenced code blocks."""
    out = []
    fenced = False
    for i, line in enumerate(text.split('\n')):
        if FENCE_RE.match(line):
            fenced = not fenced
            continue
        if fenced:
            continue
        m = re.match(r'^(#{1,6})\s+(.+?)\s*$', line)
        if m:
            out.append((i, len(m.group(1)), m.group(2).strip()))
    return out


def block(text, prefix, level=2):
    """Return text from the heading (level, startswith prefix) to the next heading of <= level."""
    hs = heading_list(text)
    lines = text.split('\n')
    for k, (i, lv, title) in enumerate(hs):
        if lv == level and title.startswith(prefix):
            end = len(lines)
            for j, lv2, _ in hs[k + 1:]:
                if lv2 <= level:
                    end = j
                    break
            return '\n'.join(lines[i:end])
    raise CheckFail('section missing: %s' % prefix)


def subsections(sec, level=3):
    """Return (title, body) for each heading of exactly `level` inside sec."""
    hs = heading_list(sec)
    lines = sec.split('\n')
    out = []
    for k, (i, lv, title) in enumerate(hs):
        if lv != level:
            continue
        end = len(lines)
        for j, lv2, _ in hs[k + 1:]:
            if lv2 <= level:
                end = j
                break
        out.append((title, '\n'.join(lines[i + 1:end])))
    return out


def split_row(line):
    s = line.strip()
    if s.startswith('|'):
        s = s[1:]
    if s.endswith('|'):
        s = s[:-1]
    return [c.strip() for c in s.split('|')]


def tables(text):
    """Markdown tables outside fences: dicts with header, rows (clean), raw (uncleaned)."""
    out = []
    cur = None
    fenced = False
    for i, line in enumerate(text.split('\n')):
        if FENCE_RE.match(line):
            fenced = not fenced
            if cur:
                out.append(cur)
                cur = None
            continue
        s = line.strip()
        if not fenced and s.startswith('|'):
            cells = split_row(s)
            if cur is None:
                cur = {'line': i, 'header': [clean(c) for c in cells], 'rows': [],
                       'raw': [], 'sep': False}
            elif not cur['sep'] and all(re.fullmatch(r':?-{2,}:?', c) for c in cells if c):
                cur['sep'] = True
            else:
                cur['rows'].append([clean(c) for c in cells])
                cur['raw'].append(cells)
        else:
            if cur:
                out.append(cur)
                cur = None
    if cur:
        out.append(cur)
    return out


def hmatch(header_cell, needle):
    return re.match(r'^%s(\b|$)' % re.escape(needle.lower()), header_cell.lower()) is not None


def find_table(tbls, *needles):
    for t in tbls:
        if all(any(hmatch(h, n) for h in t['header']) for n in needles):
            return t
    return None


def idx(t, needle):
    for k, h in enumerate(t['header']):
        if hmatch(h, needle):
            return k
    raise CheckFail('table column missing: %s (header %s)' % (needle, t['header']))


def cell(row, k):
    return row[k] if k < len(row) else ''


def units(text):
    """Sentence-like units: table rows, list items, and sentences of prose paragraphs."""
    out = []
    for para in re.split(r'\n\s*\n', text):
        if para.lstrip().startswith('|'):
            out.extend(l for l in para.split('\n') if l.strip())
            continue
        for item in re.split(r'\n(?=\s*(?:[-*]|\d+\.)\s)', para):
            flat = norm(item.replace('`', '').replace('**', ''))
            if flat:
                out.extend(s for s in re.split(r'(?<=[.!])\s+(?=[A-Z`*(\[\d])', flat) if s)
    return out


_UNIT_CACHE = {}


def has(text, *terms):
    """True when one unit contains every term (case-insensitive substring)."""
    key = id(text)
    if key not in _UNIT_CACHE:
        _UNIT_CACHE[key] = (text, [u.lower() for u in units(text)])
    us = _UNIT_CACHE[key][1]
    low = [t.lower() for t in terms]
    return any(all(t in u for t in low) for u in us)


def units_with(text, *terms):
    low = [t.lower() for t in terms]
    return [u for u in units(text) if all(t in u.lower() for t in low)]


def need(cond, msg):
    if not cond:
        raise CheckFail(msg)


def numbered_runs(text):
    """Runs of top-level numbered list items: lists of (number, text)."""
    runs = []
    cur = []
    for line in text.split('\n'):
        m = re.match(r'^(\d+)\.\s+(.*)$', line)
        if m:
            cur.append((int(m.group(1)), m.group(2)))
        elif line.strip() == '' or line.startswith(' '):
            continue
        else:
            if cur:
                runs.append(cur)
                cur = []
    if cur:
        runs.append(cur)
    return runs


def sql_block(text):
    blocks = []
    inside = False
    buf = []
    for line in text.split('\n'):
        if not inside and re.match(r'^```sql\s*$', line.strip()):
            inside = True
            buf = []
            continue
        if inside and line.strip() == '```':
            blocks.append('\n'.join(buf))
            inside = False
            continue
        if inside:
            buf.append(line)
    for b in blocks:
        if 'CREATE TABLE ledger_meta' in b:
            return b
    raise CheckFail('SQL block with CREATE TABLE ledger_meta missing')


def strip_comments(stmt):
    """Drop leading blank and comment-only lines from one completed statement."""
    lines = stmt.split('\n')
    while lines and (not lines[0].strip() or lines[0].lstrip().startswith('--')):
        lines.pop(0)
    return '\n'.join(lines).strip()


def sql_statements(sql):
    """Split SQL text into statements, each without the comment lines that precede it."""
    stmts = []
    buf = ''
    for line in sql.split('\n'):
        buf += line + '\n'
        if sqlite3.complete_statement(buf):
            s = strip_comments(buf)
            if s:
                stmts.append(s)
            buf = ''
    s = strip_comments(buf)
    if s:
        stmts.append(s)
    return stmts


def sql_table_names(sql_text):
    """Names of the CREATE TABLE statements of a SQL text, in order."""
    out = []
    for s in sql_statements(sql_text):
        m = TABLE_RE.match(s)
        if m:
            out.append(m.group(1))
    return out


TABLE_RE = re.compile(r'^CREATE\s+TABLE\s+(\w+)\s*\(', re.I)
CONSTRAINT_RE = re.compile(r'^(PRIMARY\s+KEY|UNIQUE\s*\(|CHECK\s*\(|FOREIGN\s+KEY|CONSTRAINT\b)',
                           re.I)


def split_top(body):
    pieces = []
    depth = 0
    cur = ''
    for ch in body:
        if ch == '(':
            depth += 1
        elif ch == ')':
            depth -= 1
        if ch == ',' and depth == 0:
            pieces.append(cur)
            cur = ''
        else:
            cur += ch
    if cur.strip():
        pieces.append(cur)
    return pieces


def parse_create(stmt):
    m = TABLE_RE.match(stmt)
    if not m:
        return None
    name = m.group(1)
    start = stmt.index('(', m.start())
    depth = 0
    end = len(stmt)
    for i in range(start, len(stmt)):
        if stmt[i] == '(':
            depth += 1
        elif stmt[i] == ')':
            depth -= 1
            if depth == 0:
                end = i
                break
    body = stmt[start + 1:end]
    cols = {}
    cons = []
    for p in split_top(body):
        p = ' '.join(p.split())
        if not p:
            continue
        if CONSTRAINT_RE.match(p):
            cons.append(p)
        else:
            cols[re.match(r'"?(\w+)"?', p).group(1)] = p
    return {'name': name, 'cols': cols, 'cons': cons, 'stmt': stmt,
            'lines': [l.strip().rstrip(',') for l in stmt.split('\n')]}


def in_values(defn):
    m = re.search(r'IN\s*\(([^)]*)\)', defn)
    if not m:
        return None
    return [v.strip().strip("'") for v in m.group(1).split(',')]


def fk_of(defn):
    m = re.search(r'REFERENCES\s+(\w+)\s*\((\w+)\)', defn)
    return (m.group(1), m.group(2)) if m else None


def design_ex1_cells(text):
    """Map table -> [(column, arrow target or None)] from the design EX-1 table."""
    sec = block(text, 'EX-1 Format3', 3)
    result = {}
    for t in tables(sec):
        if 'Table' not in t['header'][0]:
            continue
        for raw in t['raw']:
            tnames = re.findall(r'`(\w+)`', raw[0]) if raw else []
            if len(tnames) != 1 or len(raw) < 3:
                continue
            spans = []
            depth = 0
            s = raw[2]
            # Prose after the column list names another table's column (job_activations: the
            # current active job is a current_state column), so only the text before ';' is columns.
            if ';' in s:
                s = s[:s.index(';')]
            j = 0
            while j < len(s):
                ch = s[j]
                if ch == '`':
                    k = s.find('`', j + 1)
                    if k < 0:
                        break
                    # `x` via `y`: the column x lives in the projection y (blockers.cleared_seq).
                    via_after = re.match(r'\s+via\b', s[k + 1:]) is not None
                    via_before = re.search(r'\bvia\s+$', s[:j]) is not None
                    if not (via_after or via_before):
                        spans.append((s[j + 1:k], depth))
                    j = k + 1
                    continue
                if ch == '(':
                    depth += 1
                elif ch == ')':
                    depth = max(0, depth - 1)
                j += 1
            cols = result.setdefault(tnames[0], [])
            for span, d in spans:
                if d != 0:
                    continue
                if span.startswith('('):
                    for nm in re.findall(r'\w+', span.split(')')[0]):
                        cols.append((nm, None))
                    continue
                m = re.match(r'(\w+)(?=$|[\s(])', span)
                if not m:
                    continue
                am = re.search(r'→\s*(\w+)', span)
                cols.append((m.group(1), am.group(1) if am else None))
    return result


def auto_value(defn):
    d = defn.upper()
    if 'PRIMARY KEY' in d and 'INTEGER' in d:
        m = re.search(r'CHECK\s*\(\s*\w+\s*=\s*(\d+)\s*\)', defn)
        return int(m.group(1)) if m else 1
    vals = in_values(defn)
    if vals:
        v = vals[0]
        return int(v) if re.fullmatch(r'-?\d+', v) else v
    m = re.search(r'length\(\s*\w+\s*\)\s*=\s*(\d+)', defn)
    if m:
        n = int(m.group(1))
        return b'\x01' * n if 'BLOB' in d else 'x' * n
    m = re.search(r'length\(\s*\w+\s*\)\s*BETWEEN\s+(\d+)', defn)
    if m:
        return 'x' * int(m.group(1))
    m = re.search(r'CHECK\s*\(\s*\w+\s*=\s*(\d+)\s*\)', defn)
    if m:
        return int(m.group(1))
    if 'BLOB' in d:
        return b'x'
    if 'INTEGER' in d:
        return 1
    return 'x'


def auto_row(tbl, overrides=None):
    row = {}
    for name, d in tbl['cols'].items():
        if re.search(r'\bNOT\s+NULL\b', d, re.I) or re.search(r'PRIMARY\s+KEY', d, re.I):
            row[name] = auto_value(d)
        else:
            row[name] = None
    for c in tbl['cons']:
        m = re.match(r'PRIMARY\s+KEY\s*\(([^)]*)\)', c, re.I)
        if m:
            for nm in [x.strip() for x in m.group(1).split(',')]:
                if nm in tbl['cols'] and row.get(nm) is None:
                    row[nm] = auto_value(tbl['cols'][nm])
    if overrides:
        row.update(overrides)
    return row


def insert(conn, tname, row):
    cols = list(row.keys())
    conn.execute('INSERT INTO %s (%s) VALUES (%s)' % (
        tname, ','.join('"%s"' % c for c in cols), ','.join('?' * len(cols))),
        [row[c] for c in cols])


def fk_map(conn, tname):
    out = {}
    for r in conn.execute('PRAGMA foreign_key_list(%s)' % tname):
        out[r[3]] = (r[2], r[4])
    return out


def body_line_set(stmt):
    lines = [l.strip().rstrip(',') for l in stmt.split('\n')]
    return [l for l in lines if l and not l.startswith(')') and not l.startswith('CREATE')]


def rejects(fn):
    """True when fn raises a sqlite error (constraint, trigger or check failure)."""
    try:
        fn()
    except sqlite3.Error:
        return True
    return False


# ---------------------------------------------------------------- context

class Ctx:
    def __init__(self, repo, doc, evidence=None, frozen=None, pending=()):
        self.pending = frozenset(pending)
        self.repo = repo
        self.doc_path = doc
        self.cache = {}
        self.evidence_dir = evidence or os.path.join(repo, C01_EVID_REL)
        self.frozen_dir = frozen or os.path.join(SCRIPT_DIR, 'frozen')

    def path(self, rel):
        return os.path.join(self.repo, rel)

    def evid(self, *parts):
        return os.path.join(self.evidence_dir, *parts)

    def read(self, path):
        if path in self.cache:
            return self.cache[path]
        try:
            with open(path, encoding='utf-8') as fh:
                text = fh.read()
        except (OSError, UnicodeDecodeError):
            raise MissingDoc(path)
        self.cache[path] = text
        return text

    def doc(self):
        return self.read(self.doc_path)

    def repo_doc(self, rel):
        return self.read(self.path(rel))

    def sec(self, prefix, level=2):
        return block(self.doc(), prefix, level)

    def git(self, *args):
        res = subprocess.run(['git', '-C', self.repo] + list(args),
                             capture_output=True, text=True)
        if res.returncode != 0:
            raise CheckFail('git %s failed: %s' % (' '.join(args), res.stderr.strip()))
        return res.stdout

    def f3(self):
        if 'f3' not in self.cache:
            sql = sql_block(self.doc())
            stmts = sql_statements(sql)
            tbls = [parse_create(s) for s in stmts if TABLE_RE.match(s)]
            tbls = [t for t in tbls if t]
            self.cache['f3'] = {
                'sql': sql,
                'stmts': stmts,
                'tables': tbls,
                'by': {t['name']: t for t in tbls},
                'triggers': [s for s in stmts if re.match(r'^CREATE\s+TRIGGER', s, re.I)],
                'indexes': [s for s in stmts if re.match(r'^CREATE\s+(UNIQUE\s+)?INDEX', s, re.I)],
            }
        return self.cache['f3']

    def db(self):
        """Fresh in-memory database built from the full contract SQL text (comments included)."""
        f = self.f3()
        try:
            conn = sqlite3.connect(':memory:')
            conn.execute('PRAGMA foreign_keys=ON')
            conn.executescript(f['sql'])
        except sqlite3.Error as e:
            raise CheckFail('format3 SQL does not execute: %s' % e)
        return conn

    def base_db(self):
        """Database with parent rows: records 1..8, revision 1, events 1..12."""
        conn = self.db()
        t = self.f3()['by']
        for i in range(1, 9):
            insert(conn, 'records', auto_row(t['records'], {'id': i, 'kind': 1, 'codec': 1}))
        insert(conn, 'revisions', auto_row(t['revisions'], {'id': 1}))
        for s in range(1, 13):
            insert(conn, 'events', auto_row(t['events'], {
                'seq': s, 'type': 'ledger_initialized', 'version': 1, 'revision_id': 1}))
        return conn

    def design(self):
        if 'design' not in self.cache:
            self.cache['design'] = design_ex1_cells(self.read(self.path(DESIGN_REL)))
        return self.cache['design']

    def v5(self):
        return self.read(self.path(V5_REL)).split('\n')

    def m_table(self):
        t = find_table(tables(self.doc()), 'before', 'cause', 'after', 'reason', 'event')
        if t is None:
            raise CheckFail('role state table (before|cause|after|reason|event) missing')
        return t

    def n_table(self):
        t = find_table(tables(self.doc()), 'before', 'cause', 'after', 'event', 'paired')
        if t is None:
            raise CheckFail('run state table (before|cause|after|event|paired) missing')
        return t


def m_expansions(t):
    """Set of (before, after, reason) triples from the role table, comma lists expanded."""
    ib, ia, ir = idx(t, 'before'), idx(t, 'after'), idx(t, 'reason')
    out = set()
    for row in t['rows']:
        for b in cell(row, ib).split(','):
            bs = norm_state(b)
            for s in (bs if isinstance(bs, list) else [bs]):
                out.add((s, norm_state(cell(row, ia)), cell(row, ir)))
    return out


def norm_state(s):
    s = s.strip().replace('+drift', '').strip()
    if s in ('NULL', '', 'none', '-'):
        return None
    return s


def triple_missing(triples, need_triples):
    return [t for t in need_triples if t not in triples]


# ================================================================ checks

@check('CHK-I1-skeleton')
def _i1(c):
    d = c.doc()
    h2 = [t for _, lv, t in heading_list(d) if lv == 2]
    need(h2 == H2_EXPECT, 'H2 headings differ from the 16 required in order: %s' % h2)
    p2 = c.sec('Part 2 placeholders')
    h3 = [t for _, lv, t in heading_list(p2) if lv == 3]
    want = ['Part 2 (R15-A2): ' + s for s in PART2_TITLES]
    need(h3 == want, 'Part 2 placeholders are not exactly the seven ### subsections: %s' % h3)
    return '16 H2 headings in order; 7 placeholder ### subsections inside Part 2'


@check('CHK-I2-tables-strict')
def _i2(c):
    f = c.f3()
    bad = [t['name'] for t in f['tables'] if not t['stmt'].rstrip().endswith(') STRICT;')]
    need(not bad, 'CREATE TABLE not ending ") STRICT;": %s' % bad)
    names = [t['name'] for t in f['tables']]
    need(names == TABLES, 'table set or creation order differs: %s' % names)
    return '24 tables, all STRICT, creation order as required'


@check('CHK-I3-format-markers')
def _i3(c):
    f = c.f3()['by']
    d = c.doc()
    need('CHECK(format=3)' in ' '.join(f['ledger_meta']['stmt'].split()),
         'ledger_meta lacks CHECK(format=3)')
    need('PRAGMA user_version=3' in d, 'PRAGMA user_version=3 text missing')
    codec = 'CHECK((kind=4 AND codec IN (2,3)) OR (kind BETWEEN 1 AND 3 AND codec=1))'
    need(codec in ' '.join(f['records']['stmt'].split()), 'records codec CHECK not byte-exact')
    need('CHECK(version IN (1,2))' in ' '.join(f['events']['stmt'].split()),
         'events version CHECK(version IN (1,2)) missing')
    tv = in_values(f['events']['cols'].get('type', ''))
    need(tv is not None and set(tv) == set(EVENT14) and len(tv) == 14,
         'events.type CHECK is not the 14 values: %s' % tv)
    for col in ('job', 'role', 'generation_id', 'run_id'):
        need(col in f['events']['cols'] and 'NOT NULL' not in f['events']['cols'][col].upper(),
             'events.%s is not a nullable typed column' % col)
    need('active_job' in f['current_state']['cols'] and
         'active_activation_id' in f['current_state']['cols'],
         'current_state lacks active_job or active_activation_id')
    ov = in_values(f['requests']['cols'].get('operation', ''))
    need(ov is not None and set(ov) == set(REQUEST_OPS) and len(ov) == 10,
         'requests.operation CHECK is not the 10 values: %s' % ov)
    need('attachment_id' in f['requests']['cols'], 'requests.attachment_id missing')
    return 'format=3, user_version=3, codec CHECK, events 14-type CHECK, typed columns, requests 10-op CHECK'


@check('CHK-I4-triggers')
def _i4(c):
    f = c.f3()
    expect = set()
    for t in TABLES:
        if t in MUTABLE:
            expect.add((t, 'no_delete'))
        else:
            expect.add((t, 'no_update'))
            expect.add((t, 'no_delete'))
    got = set()
    for s in f['triggers']:
        flat = norm(s)
        m = re.fullmatch(r"CREATE TRIGGER (\w+)_no_(update|delete) BEFORE (UPDATE|DELETE) "
                         r"ON (\w+) BEGIN SELECT RAISE\(ABORT,'immutable ledger row'\); END;",
                         flat)
        need(m is not None, 'trigger not in the D69 explicit form: %s' % flat[:140])
        name, kind, ev, tbl = m.groups()
        need(name == tbl and ev.lower() == kind, 'trigger name/table mismatch: %s' % flat[:140])
        got.add((tbl, 'no_' + kind))
    need(len(f['triggers']) == len(expect) and got == expect,
         'trigger set differs: missing %s extra %s' % (sorted(expect - got), sorted(got - expect)))
    conn = c.db()
    names = {r[0] for r in conn.execute("SELECT name FROM sqlite_master WHERE type='trigger'")}
    want = {'%s_%s' % (t, k) for t, k in expect}
    need(names == want, 'sqlite_master triggers differ: extra %s missing %s' %
         (sorted(names - want), sorted(want - names)))
    return '%d explicit triggers; sqlite_master holds exactly them' % len(expect)


@check('CHK-I5-fk-resolve')
def _i5(c):
    f = c.f3()
    d = c.design()
    need(d, 'design EX-1 table not parsed')
    order = {t: k for k, t in enumerate(TABLES)}
    conn = c.db()
    for t, cols in d.items():
        need(t in order, 'design table %s has no SQL table' % t)
        need(t in f['by'], 'design table %s missing from the SQL' % t)
        fks = fk_map(conn, t)
        for name, tgt in cols:
            if name not in f['by'][t]['cols']:
                continue
            if tgt:
                need(name in fks, '%s.%s has a design arrow but no REFERENCES' % (t, name))
                need(fks[name][0] == tgt, '%s.%s REFERENCES %s, design says %s' %
                     (t, name, fks[name][0], tgt))
                need(tgt in order and order[tgt] < order[t],
                     '%s.%s references %s which is not created earlier' % (t, name, tgt))
            else:
                need(name not in fks, '%s.%s has REFERENCES but no design arrow' % (t, name))
    for t, name in READER_VERIFIED:
        need(name not in fk_map(conn, t), '%s.%s must carry no REFERENCES' % (t, name))
    for tgt_t, tgt_c in [(v[0], v[1]) for t in TABLES for v in fk_map(conn, t).values()]:
        need(tgt_t in order, 'REFERENCES target table %s does not exist' % tgt_t)
    d_text = c.doc()
    need(has(d_text, 'reader-verified', 'reservations.request_id') or
         has(d_text, 'reader verified', 'reservations.request_id'),
         'reader-verified column list (with reservations.request_id) missing')
    need(has(d_text, 'reservations.request_id', 'reservation') and
         (has(d_text, 'request row') or has(d_text, 'before its request')),
         'reservation reason for reservations.request_id missing')
    return 'REFERENCES set matches the design arrows; reader-verified columns carry none'


@check('CHK-I6-reserved-labels')
def _i6(c):
    d = c.doc()
    for t, lab in RESERVED_LABEL.items():
        m = re.search(r'CREATE TABLE %s\s*\(' % t, d)
        need(m is not None, 'reserved table %s missing' % t)
        prev = d.rfind('CREATE TABLE ', 0, m.start())
        region = d[(prev + 1 if prev >= 0 else 0):m.start()]
        labels = re.findall(r'Reserved for (R1[678])', region)
        need(labels and labels[-1] == lab,
             'table %s must be preceded by "Reserved for %s" (found %s)' % (t, lab, labels))
    return 'all ten reserved tables carry their Reserved for R16/R17/R18 label'


@check('CHK-I7-cross-row')
def _i7(c):
    d = c.doc()
    need(has(d, 'nine'), 'the prose does not say nine design invariants')
    need(not re.search(r'eight design invariants', d, re.I), 'prose says eight design invariants')
    need(has(d, 'at most one active run'), 'at-most-one-active-run bullet missing')
    need(has(d, 'exception', 'check'), 'check exception missing')
    need(has(d, 'active_activation_id', 'latest'), 'active_activation_id equals latest bullet missing')
    need(has(d, 'ordinal', 'contiguous'), 'generation ordinal contiguity bullet missing')
    need(has(d, 'running', 'exactly one', 'role_state.run_id'), 'D59 coupling bullet missing')
    need(has(d, 'NULL', 'first', 'before_state'), 'NULL-first-transition rule missing')
    need(has(d, 'other than', 'active_job', 'retired'), 'non-active-job rows retired bullet missing')
    need(has(d, 'active_job', 'NULL', 'retired'), 'all-rows-retired-when-active_job-NULL missing')
    need(has(d, 'fact_version', 'transitions row'), 'fact_version replay rule missing')
    need(has(d, 'run_state', 'derived', 'transition'), 'run_state derivation bullet missing')
    need(has(d, '--at-event', 'Lua'), '--at-event replays without Lua statement missing')
    need(has(d, 'native writer') and has(d, 'reader-verified'),
         'native writer / reader-verified labelling missing')
    return 'nine design invariants plus D89 additions present'


@check('CHK-I8-cutover')
def _i8(c):
    d = c.doc()
    need('before any capture or Lua' in d, 'phrase "before any capture or Lua" missing')
    need(has(d, 'unsupported_ledger_version', 'format1', 'refus'), 'format1/2 refusal missing')
    need(has(d, 'init', 'format3', 'R15-B') or has(d, 'init', 'format3', 'R15-C'),
         'init creates format3 after R15-B/C missing')
    need(has(d, 'unchanged', 'response bytes'), 'init unchanged CLI signature / bytes missing')
    need(has(d, 'no format selector') or has(d, 'no selector'), 'no-selector statement missing')
    need(not re.search(r'init[^.\n]{0,60}\b(keeps|has|accepts|takes) an? (format )?selector', d),
         'init keeps a format selector')
    need('An older binary that opens a format3 ledger refuses it unchanged' in norm(d),
         'old binary refusal missing')
    need(has(d, 'fixtures', 'format1'), 'fixtures keep format1/format2 creation missing')
    need('XD5' in d, 'XD5 test-name rule missing')
    return 'cutover text: format3 only after R15-B/C, refusal before capture or Lua, XD5'


@check('CHK-I9-request-v3')
def _i9(c):
    d = c.doc()
    need('PLANAR-REQUEST' in d, 'PLANAR-REQUEST framing missing')
    need(re.search(r'byte\s+03\b', d) is not None, 'byte 03 framing missing')
    order = 'bytes(operation)||bytes(actor)||bytes(parameters)||selector_tag||[bytes(profile_argument)]'
    need(ns(order) in ns(d), 'request_v3 field order not byte-exact')
    need(has(d, 'kind 4', 'codec 3') and 'request_v3' in d, 'request_v3 record kind 4 codec 3 missing')
    need('normaliz' in d.lower(), 'host-path normalization missing')
    need(has(d, 'contents', 'never', 'body'), 'contents never in body statement missing')
    need(not re.search(r'contents (enter|enters|are written into|are in) the body', d),
         'file contents enter the body')
    need('request_conflict' in d, 'request_conflict missing')
    need(has(d, 'version', 'conflict') and (has(d, 'different', 'version', 'conflict') or
                                           has(d, 'another', 'version', 'conflict')),
         'different-version conflict missing')
    need(has(d, 'reserve', 'nothing') or has(d, 'no reservation'),
         'failed operations reserve nothing missing')
    need(has(d, 'attachment_id', 'packet'), 'native packet assembly missing')
    need(has(d, 'request_v2', 'load'), 'load keeps request_v2 missing')
    return 'request_v3 framing, kind 4 codec 3, contents never in body, replay and conflict'


@check('CHK-I10-history-lookup')
def _i10(c):
    d = c.doc()
    need(has(d, 'history --request-id', 'execution result', 'format3') or
         has(d, 'history --request-id', 'execution result', 'format 3'),
         'format3 history --request-id execution results missing')
    need(has(d, 'format1', 'unchanged') or has(d, 'format 1', 'unchanged') or
         has(d, 'format1', 'byte-unchanged'), 'format1/2 history unchanged statement missing')
    need(not [u for u in units_with(d, 'execution result') if
              re.search(r'format ?[12]', u, re.I) and not re.search(r'\bnot\b|unchanged|no ', u, re.I)],
         'format1/2 history said to return execution results')
    need(has(d, 'disposition', 'no', 'field'), 'history adds no disposition field statement missing')
    need(not [u for u in units(d) if re.search(r'\badds? an? disposition field\b', u, re.I)],
         'history --request-id is said to add a disposition field')
    vocab = [u for u in units(d) if 'disposition' in u.lower() and 'vocabulary' in u.lower()]
    need(any(all(w in u.lower() for w in ('current', 'completed', 'released', 'interrupted', 'fenced'))
             for u in vocab), 'disposition vocabulary of five missing')
    need('history --event selector' in d, 'deferred history --event selector placeholder not named')
    return 'history --request-id execution results on format3 only; disposition vocabulary of five'


@check('CHK-I11-seven-steps')
def _i11(c):
    sec = c.sec('EX-3')
    runs = [r for r in numbered_runs(sec) if r and r[0][0] == 1]
    need(runs, 'EX-3 numbered steps missing')
    run = runs[0]
    need([n for n, _ in run] == list(range(1, 8)), 'EX-3 must have exactly seven numbered steps: %s' %
         [n for n, _ in run])
    kws = ['prelude', 'snapshot', 'capture', 'lua', 'validation', 'commit', 'output']
    for (n, text), kw in zip(run, kws):
        need(kw in text.lower(), 'EX-3 step %d is not %s (order wrong)' % (n, kw))
    return 'seven EX-3 steps in design order'


@check('CHK-I12-precondition-vocab')
def _i12(c):
    sec = c.sec('EX-3')
    members = ['active_revision == R', 'active_job == J', 'fact_version(job, role) == v',
               'role_state(target) == (state, drifted, run)', 'generation_current(target, G)',
               'run_active(run, G, secret)', 'reservation_open(id)', 'request_unused(id)']
    for m in members:
        need(ns(m) in ns(sec), 'precondition member missing: %s' % m)
    need('No other precondition exists' in norm(sec), '"No other precondition exists" missing')
    need('any_run_active' not in c.doc(), 'ninth precondition any_run_active present')
    need(has(sec, 'bump'), 'fact_version bump statement missing')
    for w in ('generation change', 'completion', 'publication', 'repair admission', 'verdict',
              'blocker change', 'revision acceptance'):
        need(w in sec.lower(), 'bump set lacks %s' % w)
    for u in units_with(sec, 'bump'):
        if 'claim' in u.lower() and not re.search(r'\bnot\b|nothing|never|only|do not', u, re.I):
            raise CheckFail('claims listed in the bump set: %s' % u[:120])
    return 'eight precondition members; bump set of seven facts'


@check('CHK-I13-delta-guard')
def _i13(c):
    d = c.doc()
    need(has(d, 'role_state', 'transitions row'), 'delta write only for roles with transitions row missing')
    need('no row exists' in norm(d), '"no row exists" first-transition wording missing')
    need('no run_state row exists for the new run id' in norm(d.replace('`', '')) or
         'no run_state row exists' in norm(d.replace('`', '')),
         'run_state claim "no row exists" wording missing')
    need(has(d, 'locked', 'before'), 'locked before-state check missing')
    need(has(d, 'vector', 'validat'), 'Lua vector validated statement missing')
    for u in units_with(d, 'complete', 'vector', 'written'):
        need(re.search(r'\bnot\b|never|wholesale', u, re.I) is not None,
             'complete vector is written: %s' % u[:120])
    return 'delta write: role_state only for transitioned roles, locked before-state, no row exists'


@check('CHK-I14-conflict-mapping')
def _i14(c):
    d = c.doc()
    need(has(d, 'claim_conflict', 'loses') or has(d, 'claim_conflict', 'loser') or
         has(d, 'claim_conflict', 'lost'), 'loser of concurrent claims is claim_conflict missing')
    need(has(d, 'state_conflict', 'other'), 'other precondition failure as state_conflict missing')
    need('nothing changes' in d.lower(), 'state_conflict nothing-changed statement missing')
    need('same request id' in d.lower() or 'same-id' in d.lower() or 'same id' in d.lower(),
         'same-ID retry statement missing')
    need(not [u for u in units(d) if 'state_conflict' in u and 'internal' in u.lower() and
              'only' not in u.lower() and 'pointer' not in u.lower()],
         'state_conflict is retried internally')
    need(has(d, 'check completion', 'only', 'internal'), 'check completion as only internal retry missing')
    return 'loser is claim_conflict; other precondition failures are state_conflict'


@check('CHK-I15-load-activate')
def _i15(c):
    d = c.doc()
    need(re.search(r'\bCAS\b', d) is not None, 'CAS step of load reconciliation missing')
    need('noop load writes no transitions' in norm(d).lower(), 'noop load writes no transitions missing')
    need(has(d, 'absent', 'retire', 'generation', 'active job'),
         'absent active job retires generations and clears active job missing')
    need(has(d, 'active run', 'acknowledged revision') or has(d, 'active runs', 'acknowledged'),
         'active runs keep acknowledged revision statement missing')
    need(has(d, 'fence', 'previous job') or has(d, 'fenced', 'previous job'),
         'activation fences previous-job runs missing')
    need('no_active_job' in d, 'no_active_job missing')
    need(not [u for u in units(d) if re.search(r'reuse', u, re.I) and 'generation' in u.lower() and
              not re.search(r'\bnot\b|never', u, re.I)],
         're-activation reuses old generations')
    return 'load/activate reconciliation: CAS, noop writes nothing, absent job retires, fencing'


@check('CHK-I16-precedence')
def _i16(c):
    sec = c.sec('EX-4')
    runs = [r for r in numbered_runs(sec) if len(r) == 6 and r[0][0] == 1 and
            'retired' in r[0][1].lower()]
    need(runs, 'ordered six-rule precedence list (starting with retired) missing')
    run = runs[0]
    for k, ((_, text), (kw, cites)) in enumerate(zip(run, PRECEDENCE)):
        need(clean(text).lower().startswith(kw), 'precedence rule %d is not %s' % (k + 1, kw))
        found = set()
        for m in re.finditer(r':\s?(\d{3})(?:\s?-\s?(\d{3}))?\b', text):
            a = int(m.group(1))
            b = int(m.group(2)) if m.group(2) else a
            if 370 <= a <= 390:
                found |= set(range(a, b + 1))
        need(found == cites, 'precedence rule %s cites %s, required %s' % (kw, sorted(found), sorted(cites)))
    need(has(sec, 'new blocker'), 'running-with-new-blocker rule missing')
    need(has(sec, 'prerequisite'), 'prerequisites from ledger facts missing')
    v5 = c.v5()
    for n in V5_KEYS:
        need(v5[n - 1].lstrip('|').strip().startswith(V5_KEYS[n]), 'v5 line %d is not the expected row' % n)
    return 'six precedence rules in order with exact v5 cites'


@check('CHK-I17-event-types')
def _i17(c):
    f = c.f3()['by']
    ev = in_values(c.f3()['by']['events']['cols'].get('type', ''))
    need(ev is not None and sorted(ev) == sorted(EVENT14) and len(ev) == 14,
         'events.type CHECK is not exactly the 14 types: %s' % ev)
    d = c.doc()
    for t in ADDED11:
        need(t in d, 'added event type missing from prose: %s' % t)
    need('role_reconciled' not in d, 'role_reconciled added')
    need(has(d, 'declaration_accepted', 'load_noop', 'transitions'),
         'load keeps declaration_accepted/load_noop with attached transitions missing')
    need(has(d, 'one event') and has(d, 'exception'), 'one request one event statement missing')
    return '14 SQL event types equal the three format2 types plus the 11 added'


@check('CHK-I18-reasons-payload')
def _i18(c):
    f = c.f3()['by']
    rv = in_values(f['transitions']['cols'].get('reason', ''))
    need(rv is not None and sorted(rv) == sorted(REASONS16) and len(rv) == 16,
         'transitions.reason CHECK is not exactly the 16 reasons: %s' % rv)
    d = c.doc()
    secs = norm(c.sec('EX-4') + '\n' + c.sec('State machines'))
    tabs = tables(c.sec('EX-4') + '\n' + c.sec('State machines'))
    cands = []
    for t in tabs:
        if find_table([t], 'reason') is not None:
            k = idx(t, 'reason')
            vals = [clean(r[k]) for r in t['rows'] if len(r) > k]
            toks = {v for v in vals if re.fullmatch(r'[a-z_]+', v)}
            if len(toks & set(REASONS16)) >= 14:
                cands.append(toks)
    for blk in re.split(r'\n\s*\n', c.sec('EX-4') + '\n' + c.sec('State machines')):
        if blk.lstrip().startswith('|'):
            continue
        toks = {clean(m) for m in re.findall(r'`([a-z_]+)`', blk)}
        if len(toks & set(REASONS16)) >= 14:
            cands.append(toks)
    need(cands, 'no prose list of the reasons found in EX-4 or State machines')
    for toks in cands:
        need(toks == set(REASONS16), 'prose reason list differs from SQL: extra %s missing %s' %
             (sorted(toks - set(REASONS16)), sorted(set(REASONS16) - toks)))
    need('version:2,operation,before:{active_revision,active_job},after:{active_revision,active_job},'
         'reason,references' in ns(d), 'payload v2 shape not exact')
    need('after:{...}' not in d, 'after written literally as after:{...}')
    need(has(d, 'after', 'empty string'), 'after active_job empty-string statement missing')
    need(has(d, 'work_changed', 'R15') and has(d, 'work_changed', 'R17'),
         'work_changed R15/R17 reachability missing')
    need(has(d, 'released', 'R16') and has(d, 'interrupted', 'R16'), 'released/interrupted R16 missing')
    need(has(d, 'completed', 'R17'), 'completed R17 missing')
    need(has(d, 'evidence_inapplicable', 'R17'), 'evidence_inapplicable R17 missing')
    need(has(d, 'repair_admitted', 'R18'), 'repair_admitted R18 missing')
    need(has(d, 'drift_cleared', 'R15') and has(d, 'drift_cleared', 'R16'), 'drift_cleared R15/R16 missing')
    return '16 reasons in SQL equal prose; payload v2 shape; reachability split stated'


@check('CHK-I19-explain')
def _i19(c):
    d = c.doc()
    need(has(d, 'explain', 'prerequisite', 'output', 'work'),
         'explain three-causes sentence (work, output applicability, prerequisite) missing')
    need('v5-alignment' in d, 'explain requirement does not cite v5-alignment.md')
    need('explain' in c.repo_doc(ALIGN_REL).lower(), 'v5-alignment explain lines not found by text')
    return 'explain separates work change, output applicability and prerequisites'


@check('CHK-I20-structure-lineage')
def _i20(c):
    d = c.doc()
    need(has(d, 'role_structure_v1'), 'role_structure_v1 missing')
    need('job,role,kind,work,targets,ownership,shape' in ns(d), 'role_structure_v1 field list not exact')
    need(has(d, 'digest', 'generation', 'fence'), 'digest-driven generation fence statement missing')
    need(has(d, 'takeover', 'carr'), 'takeover lineage carry statement missing')
    need(has(d, 'activated', 'structural_change', 'recreated', 'start'),
         'only activated, structural_change and recreated start empty statement missing')
    need(not [u for u in units(d) if 'takeover' in u.lower() and 'start empty' in u.lower() and
              not re.search(r'\bnot\b|never|only', u, re.I)],
         'takeover generations start empty')
    need(has(d, 'work_exec_v1', 'Part 2') or has(d, 'work_exec_v1', 'R15-A2'),
         'work_exec_v1 pointer to Part 2 placeholder missing')
    need(re.search(r'work_exec_v1\s*[:=]\s*\{', d) is None, 'full W recipe spelled out')
    return 'role_structure_v1 fields, digest-driven generations, takeover lineage, work_exec_v1 pointer'


@check('CHK-I21-interpretation-binding')
def _i21(c):
    sec = c.sec('EX-6')
    need(has(sec, 'stdlib', 'digest') and 'interpretation_mismatch' in sec,
         'stdlib digest equality / interpretation_mismatch missing')
    need(has(sec, 'interpretation_mismatch', 'before', 'mutation'),
         'interpretation_mismatch before mutation missing')
    need(has(sec, 'after', 'replay', 'interpretation'), 'check after early replay missing')
    need(not re.search(r'interpretation[^.\n]{0,60}before (early )?replay', sec, re.I),
         'interpretation check moved before replay')
    need(has(sec, 'exempt'), 'exempt-read class sentence missing')
    need(has(sec, 'with-profile', 'same'), 'profile revisions with-profile same bytes missing')
    need(has(sec, 'factory', 'five'), 'factory family closed at five missing')
    return 'EX-6 binding: digest equality, early replay order, exempt reads, with-profile, five factories'


@check('CHK-I22-code-table')
def _i22(c):
    sec = c.sec('EX-7')
    t = find_table(tables(sec), 'code', 'exit')
    need(t is not None, 'EX-7 code table (code|exit) missing')
    rows = [r for r in t['rows'] if r and r[0]]
    codes = [clean(r[0]) for r in rows]
    need(len(rows) == 9 and set(codes) == set(CODE_ROUNDS),
         'EX-7 code table must have exactly the nine codes: %s' % codes)
    ie = idx(t, 'exit')
    ifd = find_table([t], 'field') and idx(t, 'field')
    idet = find_table([t], 'details') and idx(t, 'details')
    imsg = find_table([t], 'message') and idx(t, 'message')
    for raw, row in zip(t['raw'], t['rows']):
        code = clean(row[0])
        need(cell(row, ie) == '3', 'code %s exit is not 3' % code)
        if ifd is not None:
            need(cell(row, ifd).replace('"', '').strip() == '', 'code %s field is not empty' % code)
        det = cell(row, idet).lower() if idet is not None else ''
        if code == 'claim_conflict':
            need('state' in det and all(s in det for s in ('blocked', 'running', 'done', 'retired'))
                 and 'run_state' not in det, 'claim_conflict details must be {state} with four states')
        elif code == 'fenced':
            need('run_state' in det and all(s in det for s in RUN_STATES),
                 'fenced details must be {run_state} with five run states')
        elif idet is not None:
            need(det.strip() in ('none', '-', '—', ''), 'code %s must carry no details' % code)
        need(re.search(r'\b%s\b' % CODE_ROUNDS[code], ' '.join(row)) is not None,
             'code %s introduced in wrong round (want %s)' % (code, CODE_ROUNDS[code]))
        if imsg is not None:
            msg = dict(MSG_CODES).get(code)
            if msg:
                need(msg in cell(row, imsg), 'code %s message not the fixed D67 text' % code)
    d = c.doc()
    need(has(d, 'XD4', 'execution'), 'XD4 amended for execution only missing')
    need('no other new code' in d.lower(), '"no other new code" sentence missing')
    need('E16 A' in d, 'E16 A amendment reference missing')
    return 'nine codes, exit 3, rounds, details per D88, field empty'


@check('CHK-I23-message-table')
def _i23(c):
    d = c.doc()
    for m in MESSAGES_19:
        need(m in d, 'closed message missing or altered: %s' % m)
    for bad in ('Role was claimed by another request', 'No active job is set',
                'message fixed by the implementing round', 'to be fixed by the implementing round'):
        need(bad not in d, 'forbidden message text present: %s' % bad)
    need('no other new message' in d.lower(), '"no other new message" sentence missing')
    return '19 new messages byte-exact; no 20th message'


@check('CHK-I24-no-removed-diagnostics')
def _i24(c):
    removed_total = 0
    for rel in ['CLAUDE.md', LEDGER_REL, EXT_REL, ALIGN_REL, IMPL_REL]:
        for h in parse_diff(c.git('diff', 'HEAD', '-U0', '--', rel)):
            removed_total += len(h['removed'])
            if h['removed']:
                raise CheckFail('%s has removed lines (%s)' % (rel, h['removed'][0][1][:80]))
    for rel in [RENDER_REL, LOAD_REL]:
        for h in parse_diff(c.git('diff', 'HEAD', '-U0', '--', rel)):
            bad = [t for _, t in h['removed'] if t.lstrip().startswith('|')]
            need(not bad, '%s removes a message table row' % rel)
    for h in parse_diff(c.git('diff', 'HEAD', '-U0', '--', MATRIX_REL)):
        for n, t in h['removed']:
            need(n in MATRIX_ALLOWED_OLD, 'acceptance-matrix removes line %d (not one of the seven)' % n)
    return 'no removed diagnostic lines in edited docs (acceptance-matrix seven lines allowed)'


def parse_diff(out):
    hunks = []
    cur = None
    oldno = newno = 0
    for line in out.split('\n'):
        m = re.match(r'^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@', line)
        if m:
            cur = {'removed': [], 'added': []}
            hunks.append(cur)
            oldno = int(m.group(1))
            newno = int(m.group(3))
            continue
        if cur is None:
            continue
        if line.startswith('-'):
            cur['removed'].append((oldno, line[1:]))
            oldno += 1
        elif line.startswith('+'):
            cur['added'].append((newno, line[1:]))
            newno += 1
    return hunks


@check('CHK-I25-bounds')
def _i25(c):
    sec = c.sec('EX-8')
    t = find_table(tables(sec), 'boundary')
    need(t is not None, 'EX-8 bounds table missing')
    need(len(t['rows']) == 10, 'EX-8 table must have ten rows, found %d' % len(t['rows']))
    names = ['execution snapshot', 'dispatch packet', 'checkpoint', 'provenance', 'verdict file',
             'observe directory', 'check output', 'facet payload', 'check observation',
             'history event expansion']
    rows = {r[0].lower(): ' '.join(r) for r in t['rows']}
    for nm in names:
        hit = [v for k, v in rows.items() if k.startswith(nm)]
        need(hit, 'EX-8 row missing: %s' % nm)
    flat = {nm: [v for k, v in rows.items() if k.startswith(nm)][0] for nm in names}
    need('16 MiB' in flat['execution snapshot'], 'snapshot bound not 16 MiB')
    need('1 MiB' in flat['dispatch packet'] and '16 MiB' in flat['dispatch packet'], 'packet bounds')
    need('8 KiB' in flat['checkpoint'], 'checkpoint bound not 8 KiB')
    need('64 KiB' in flat['provenance'] and '1024' in flat['provenance'] and 'depth 4' in flat['provenance'],
         'provenance bounds changed')
    need('1 MiB' in flat['verdict file'] and '256' in flat['verdict file'] and '4096' in flat['verdict file']
         and '16 KiB' in flat['verdict file'], 'verdict bounds changed')
    need('1024' in flat['observe directory'] and '64 KiB' in flat['observe directory'] and
         '4 MiB' in flat['observe directory'], 'observe bounds changed')
    need('16 MiB' in flat['check output'] and '64 MiB' in flat['check output'], 'check output bounds changed')
    need('1 MiB' in flat['facet payload'] and '4 MiB' in flat['facet payload'], 'facet bounds changed')
    e15 = flat['check observation']
    need('As E15 decides' not in e15, 'E15 row is "As E15 decides"')
    need('1 GiB' in e15 and '100,000' in e15.replace('100000', '100,000') and 'depth 64' in e15,
         'E15 row numbers missing')
    need('operator-decided' in e15 and 'R17' in e15 and 'digest' in e15.lower(),
         'E15 row lacks operator-decided, R17 or digest-only limit')
    need('16 MiB' in flat['history event expansion'], 'history expansion bound changed')
    need(has(sec, 'before allocation', 'checked addition'), 'checked before allocation / checked addition missing')
    need(has(sec, 'lower inherited bound wins'), 'lower inherited bound rule missing')
    need(has(sec, 'budget') and ('no budget reset' in sec.lower() or 'nothing resets' in sec.lower()),
         'no budget reset statement missing')
    need(has(sec, MSG_RESOURCE, 'new'), 'resource message not scoped to new sites')
    need('eleven rows' not in sec.lower(), 'eleven rows claimed')
    need(not has(sec, 'every row', MSG_RESOURCE), 'every row exhausts to the resource message')
    need('Capture limit exceeded' in flat['observe directory'] and
         MSG_RESOURCE not in flat['observe directory'], 'observe row message changed')
    return 'ten EX-8 rows with design numbers; E15 row numbers; new-site message scope'


@check('CHK-I26-role-transitions')
def _i26(c):
    t = c.m_table()
    need(len(t['rows']) == 41, 'role table must have 41 rows M1-M41, found %d' % len(t['rows']))
    ir = idx(t, 'reason')
    ia = idx(t, 'after')
    ib = idx(t, 'before')
    ic = idx(t, 'cause')
    il = find_table([t], 'label') and idx(t, 'label')
    seen_reasons = set()
    afters = set()
    for n, row in enumerate(t['rows'], 1):
        r = cell(row, ir)
        need(r in REASONS16, 'M%d reason %r is not one of the 16' % (n, r))
        seen_reasons.add(r)
        for s in str(cell(row, ia)).split(','):
            afters.add(norm_state(s))
        if 'first-ever' in cell(row, ic).lower():
            need(norm_state(cell(row, ib)) is None, 'M%d first-ever row has a before state' % n)
        if norm_state(cell(row, ib)) == 'retired':
            need(r in ('activated', 'recreated') and norm_state(cell(row, ia)) in ('blocked', 'ready'),
                 'M%d has before retired with reason %s' % (n, r))
        if il is not None and cell(row, il) in ('R16', 'R17', 'R18'):
            need('fact defined by %s' % cell(row, il) in ' '.join(row),
                 'M%d labelled %s without "fact defined by"' % (n, cell(row, il)))
    need(set(STATES6) <= afters, 'after states missing: %s' % (set(STATES6) - afters))
    need(set(REASONS16) <= seen_reasons, 'reasons never used: %s' % (set(REASONS16) - seen_reasons))
    tr = m_expansions(t)
    need_t = [('blocked', 'done', 'prerequisite_available'), ('done', 'blocked', 'prerequisite_unavailable'),
              ('stale', 'blocked', 'prerequisite_unavailable'), ('blocked', 'stale', 'prerequisite_available'),
              ('ready', 'running', 'claimed'), ('stale', 'running', 'claimed'),
              ('done', 'stale', 'work_changed'), ('done', 'stale', 'repair_admitted'),
              ('stale', 'done', 'work_changed'), ('stale', 'done', 'evidence_inapplicable')]
    for b in ('running',):
        for a in ('ready', 'stale', 'blocked', 'done'):
            need_t.append((b, a, 'released'))
            need_t.append((b, a, 'interrupted'))
    miss = triple_missing(tr, need_t)
    need(not miss, 'required role transitions missing: %s' % miss)
    return '41 role rows; every state and reason used; required transitions present'


@check('CHK-I27-run-transitions')
def _i27(c):
    t = c.n_table()
    need(len(t['rows']) == 11, 'run table must have 11 rows N1-N11, found %d' % len(t['rows']))
    ib, ia, ie = idx(t, 'before'), idx(t, 'after'), idx(t, 'event')
    for n, row in enumerate(t['rows'], 1):
        need(norm_state(cell(row, ia)) in RUN_STATES, 'N%d after state not a run state' % n)
    need(not [r for r in t['raw'] if 'check-recover' in ' '.join(r)], 'check-recover appears in a run row')
    for n, row in enumerate(t['rows'], 1):
        if cell(row, ia) == 'interrupted':
            need('run_interrupted' in cell(row, ie), 'interrupted not reached by run_interrupted (N%d)' % n)
    row10 = t['rows'][9]
    need('check_recorded' in cell(row10, ie) and 'R17' in ' '.join(row10), 'N10 must be check_recorded labelled R17')
    row11 = t['rows'][10]
    need('verdict_recorded' in cell(row11, ie) and 'R18' in ' '.join(row11), 'N11 must be verdict_recorded labelled R18')
    need(not any(cell(r, ib) == 'fenced' for r in t['rows']), 'fenced run has an outgoing row')
    row5 = t['rows'][4]
    need(cell(row5, ib) == 'active' and cell(row5, ia) == 'fenced' and 'job_activated' in cell(row5, ie),
         'N5 re-activation row missing')
    d = c.doc()
    need(has(d, 'no outgoing'), 'terminal states have no outgoing transitions statement missing')
    need(has(d, 'at most one active run'), 'at most one active run statement missing')
    need('the event R17 defines' not in d, 'completion row says the event R17 defines')
    return '11 run rows; interrupted via run_interrupted only; terminal states have no outgoing rows'


@check('CHK-I28-run-role-coupling')
def _i28(c):
    t = c.n_table()
    ip = idx(t, 'paired')
    expect = ['claimed', 'taken_over', 'taken_over', 'fenced', 'activated', 'structural_change',
              'fenced', 'released', 'interrupted', 'completed', 'R18']
    need(len(t['rows']) == 11, 'run table rows != 11')
    for n, (row, want) in enumerate(zip(t['rows'], expect), 1):
        val = cell(row, ip)
        need(want in val, 'N%d paired role reason must name %s (cell: %s)' % (n, want, val))
    need('fenced' not in cell(t['rows'][4], ip), 'N5 re-activation paired with fenced')
    return 'run rows paired with the matching role reason'


@check('CHK-I29-placeholders')
def _i29(c):
    p2 = c.sec('Part 2 placeholders')
    subs = subsections(p2, 3)
    want = ['Part 2 (R15-A2): ' + s for s in PART2_TITLES]
    need([t for t, _ in subs] == want, 'Part 2 subsections differ from the seven required titles')
    bodies = dict((t, b) for t, b in subs)
    for t, b in subs:
        need('Not yet written' in b, 'placeholder lacks "Not yet written": %s' % t)
        need(not re.search(r'\b(must|shall|exactly|refuses)\b', b, re.I),
             'placeholder contains a normative word: %s' % t)
    hist = bodies['Part 2 (R15-A2): history --event selector']
    need('--request-id' in hist, 'history selector clause list lacks --request-id execution-result shape')
    need('command-surface' in hist, 'history selector clause list lacks command-surface pointer')
    ptr = bodies['Part 2 (R15-A2): command, render and load pointer amendments']
    need('X-CLI' in ptr, 'pointer amendments list lacks X-CLI family')
    need('R15-6' in ptr or '`run`' in ptr, 'pointer amendments list lacks the reserved run section')
    verbs = bodies['Part 2 (R15-A2): verbs and services']
    need('--expect-job' in verbs, 'verbs list lacks --expect-job')
    need('none is a legal job ID' in verbs, 'verbs list lacks "none is a legal job ID" reason')
    return 'seven placeholders with clause lists; no normative words'


@check('CHK-I30-no-part2-leak')
def _i30(c):
    d = c.doc()
    for t in tables(d):
        for r in t['raw']:
            need(not any('--claim' in x for x in r), 'a --claim grammar table row is present')
    need(re.search(r'dispatch_manifest_v1\s*[:=]?\s*\{', d) is None, 'dispatch_manifest_v1 field list present')
    need(re.search(r'driver_provenance_v1\s*[:=]?\s*\{', d) is None, 'driver_provenance_v1 field list present')
    outside = 0
    p2 = c.sec('Part 2 placeholders')
    dec = c.sec('Decisions table')
    for line in d.split('\n'):
        if '--confirm-stopped' in line and line not in p2 and line not in dec:
            outside += 1
    need(outside <= 1, '--confirm-stopped spelled outside the one OP1 mention (%d times)' % outside)
    return 'no Part 2 content in normative text'


@check('CHK-I31-traceability')
def _i31(c):
    sec = c.sec('Traceability')
    rows = [r for t in tables(sec) for r in t['rows'] if r and r[0]]
    tokens = ['EX-%d' % k for k in range(1, 9)] + ['R15-1', 'R15-2', 'R15-3', 'R15-9', 'E1', 'E2', 'E3',
              'E5', 'E11', 'E14', 'E16', 'A09', 'A10', 'A20', 'A22', 'A23', 'A26', 'A33']
    for tok in tokens:
        need(any(re.search(r'(?<![\w-])%s(?![\w-])' % re.escape(tok), r[0]) for r in rows),
             'traceability row missing for %s' % tok)
    v5rows = ' '.join(' '.join(r) for r in rows if 'v5' in ' '.join(r).lower())
    for num in ('110', '241', '364', '497', '507', '561'):
        need(num in v5rows, 'traceability v5 line %s missing' % num)
    slugs = {slug(t) for _, _, t in heading_list(c.doc())}
    links = re.findall(r'\]\(#([^)\s]+)\)', sec)
    need(links, 'traceability has no section anchors')
    for a in links:
        need(a in slugs, 'traceability anchor does not resolve: #%s' % a)
    return 'traceability rows for every required id; anchors resolve'


def slug(title):
    t = title.strip().lower()
    t = re.sub(r'[^\w\- ]', '', t)
    return t.replace(' ', '-')


@check('CHK-I32-decisions-table')
def _i32(c):
    rows = decision_rows(c)
    for did in DESIGN_IDS_REQUIRED:
        need(did in rows, 'decisions table lacks row %s' % did)
    for did in ('OP1', 'OP2', 'OP3', 'D54', 'D67', 'D68'):
        need(did in rows, 'decisions table lacks %s' % did)
        tag = cell(rows[did], 2)
        need(tag == DELEGATED_TAG, '%s tag is not the delegated tag (%s)' % (did, tag))
    op3 = ' '.join(rows['OP3'])
    need('(A)' in op3 and '(B)' in op3 and '(C)' in op3, 'OP3 options A, B, C missing')
    return 'decisions table complete; OP1-OP3, D54, D67, D68 carry the delegated tag'


@check('CHK-I33-confirm-stopped')
def _i33(c):
    d = c.doc()
    need(has(d, '--confirm-stopped', 'required', 'true', 'usage'),
         '--confirm-stopped required scalar (only true, else usage) statement missing')
    need('remains a flag' not in d, '--confirm-stopped stated to remain a flag')
    ext = c.repo_doc(EXT_REL).split('\n')
    loc = None
    for i in range(len(ext) - 1):
        if 'flags cannot be required' in norm(ext[i] + ' ' + ext[i + 1]).lower():
            loc = i + 1
            break
    need(loc is not None, 'external-profile "Flags cannot be required" not found by text')
    ok = False
    for m in re.finditer(r'external-profile-contract\.md:(\d+)(?:-(\d+))?', d):
        a = int(m.group(1))
        b = int(m.group(2)) if m.group(2) else a
        if a <= loc <= b:
            ok = True
    need(ok, 'OP1 does not cite the external-profile line (%d) by text' % loc)
    need('changes that rule' not in norm(d).lower(), 'OP1 says it changes the external-profile rule')
    need(has(d, 'external-profile rule', 'unchanged'),
         'OP1 does not say the external-profile rule is unchanged')
    return 'OP1 requires --confirm-stopped true; rule unchanged; cites the located external-profile line'


@check('CHK-I34-acceptance-checklist')
def _i34(c):
    sec = c.sec('Acceptance checklist')
    for tok in ('A09', 'A10', 'A20', 'A22', 'A33', 'A26', 'A23', 'A02', 'A31', 'A32', 'EX-6'):
        need(tok in sec, 'checklist lacks %s' % tok)
    need('lost-update' in sec.lower(), 'checklist lacks the lost-update item')
    items = [l for l in sec.split('\n') if re.match(r'^\s*[-*]\s', l)]
    need(len(items) >= 9, 'checklist has fewer than nine items')
    return 'acceptance checklist lists the R15 tests and lost-update guard'


@check('CHK-I35-claude-md-symlink')
def _i35(c):
    out = c.git('diff', '--numstat', 'HEAD', '--', 'CLAUDE.md').split()
    need(len(out) >= 2 and out[0] == '1' and out[1] == '0', 'git diff --numstat CLAUDE.md is %s, want 1 0' % out[:2])
    lines = c.repo_doc('CLAUDE.md').split('\n')
    exact = '   [execution](docs/vernacular/execution-contract.md),'
    k = [i for i, l in enumerate(lines) if '[authoring](docs/vernacular/authoring-contract.md)' in l]
    need(k and lines[k[0] + 1] == exact, 'execution line not inserted after the authoring line')
    link = c.path('AGENTS.md')
    need(os.path.islink(link) and os.readlink(link) == 'CLAUDE.md', 'AGENTS.md is not a symlink to CLAUDE.md')
    return 'one inserted line in CLAUDE.md; AGENTS.md still a symlink'


@check('CHK-I36-ledger-pointers')
def _i36(c):
    removed = 0
    added = 0
    for h in parse_diff(c.git('diff', 'HEAD', '-U0', '--', LEDGER_REL)):
        removed += len(h['removed'])
        added += len([t for _, t in h['added'] if t.strip()])
    need(removed == 0, 'ledger-contract.md has %d removed lines' % removed)
    need(added >= 3, 'ledger-contract.md has fewer than three inserted pointer lines')
    d = c.repo_doc(LEDGER_REL)
    ex = c.doc()
    anchors = {slug(t) for _, _, t in heading_list(ex)}
    for a in ('ex-1-format3', 'ex-4-facts-derived-state-and-reconcile-rules', 'ex-2-request-framing-and-replay'):
        need('execution-contract.md#%s' % a in d, 'ledger pointer to #%s missing' % a)
        need(a in anchors, 'ledger pointer target #%s does not exist' % a)
    for a in re.findall(r'execution-contract\.md#([\w\-]+)', d):
        need(a in anchors, 'ledger links a nonexistent anchor #%s' % a)
    return 'ledger pointers link existing anchors; zero removed lines'


@check('CHK-I37-xstore-row')
def _i37(c):
    removed = 0
    for h in parse_diff(c.git('diff', 'HEAD', '-U0', '--', EXT_REL)):
        removed += len(h['removed'])
    need(removed == 0, 'external-profile-contract.md has removed or altered lines (%d)' % removed)
    d = c.repo_doc(EXT_REL)
    need(has(d, 'format3', 'format2', 'user_version') or has(d, 'format 3', 'user_version 3'),
         'format3 row lacks user_version statement')
    need('IN (2,3)' in ns(d).replace('IN(2,3)', 'IN (2,3)') or 'IN(2,3)' in ns(d),
         'format3 row lacks codec CHECK IN (2,3)')
    need(has(d, 'request_v3', 'kind 4', 'codec 3'), 'format3 row lacks request_v3 kind 4 codec 3')
    need('execution-contract.md#ex-1-format3' in d, 'format3 row does not link EX-1')
    need(has(d, 'supersede', 'New init creates format2 only'), 'supersede statement missing')
    need('New init creates format2 only' in d, 'the original init sentence was edited away')
    need(has(d, 'load', 'request_v2', 'format3'), 'load on format3 keeps request_v2 missing')
    return 'format3 row inserted; supersede statement; load keeps request_v2'


@check('CHK-I38-matrix-pointers')
def _i38(c):
    hunks = parse_diff(c.git('diff', 'HEAD', '-U0', '--', MATRIX_REL))
    removed = []
    for h in hunks:
        removed.extend(h['removed'])
    got = sorted(n for n, _ in removed)
    need(got == sorted(MATRIX_ALLOWED_OLD), 'acceptance-matrix changed lines %s, want %s' %
         (got, sorted(MATRIX_ALLOWED_OLD)))
    status = r'\b(Pending|Partial|Done|Open|Blocked|Passed|Complete|Failed)\b'
    for h in hunks:
        for (n, old), (m, new) in zip(h['removed'], h['added']):
            need(MATRIX_ALLOWED_OLD.get(n, '') in old, 'line %d is not %s' % (n, MATRIX_ALLOWED_OLD.get(n)))
            need('execution-contract.md#' in new, 'line %d lacks an appended execution-contract link' % n)
            need(old.count('|') == new.count('|'), 'line %d changed its pipe-cell count' % n)
            ow = re.findall(status, old)
            nw = re.findall(status, new)
            need(nw[:len(ow)] == ow, 'line %d changed a status word' % n)
    return 'seven matrix lines changed in place with appended links; status words unchanged'


@check('CHK-I39-v5-alignment')
def _i39(c):
    removed = 0
    for h in parse_diff(c.git('diff', 'HEAD', '-U0', '--', ALIGN_REL)):
        removed += len(h['removed'])
    need(removed == 0, 'v5-alignment.md has removed lines (%d)' % removed)
    d = c.repo_doc(ALIGN_REL)
    e2 = bullet_block(d, '- Recorded deviation E2')
    e5 = bullet_block(d, '- Recorded narrowing E5')
    need('110' in e2 and '497' in e2 and 'execution-contract' in e2,
         'E2 deviation (v5 :110, :497) entry missing')
    need('241' in e5 and 'execution-contract' in e5, 'E5 narrowing of v5 :241 entry missing')
    need('2026-10-28' in e2 and '2026-10-28' in e5, 'operator answer date 2026-10-28 missing')
    return 'v5-alignment records E2 and E5 with the execution contract'


@check('CHK-I40-implementation-pointer')
def _i40(c):
    removed = 0
    added_link = 0
    for h in parse_diff(c.git('diff', 'HEAD', '-U0', '--', IMPL_REL)):
        removed += len(h['removed'])
        added_link += len([t for _, t in h['added'] if 'execution-contract.md' in t])
    need(removed == 0, 'implementation.md has removed or rewrapped lines (%d)' % removed)
    need(added_link == 1, 'implementation.md must gain exactly one pointer line (found %d)' % added_link)
    return 'one inserted pointer sentence; zero removed lines'


@check('CHK-I41-citation-range')
def _i41(c):
    v5 = c.v5()
    for n in V5_KEYS:
        need(v5[n - 1].lstrip('|').strip().startswith(V5_KEYS[n]), 'v5 row %d changed' % n)
    decisions = c.read(c.path(DECISIONS_REL))
    dm = find_table(tables(block(decisions, 'M. Role state machine', 2)), 'row', 'v5')
    need(dm is not None, 'DECISIONS section M v5 column not parsed')
    want = []
    for row in dm['rows']:
        want.append(set(int(x) for x in re.findall(r':\s?(\d{3})', ' '.join(row[idx(dm, 'v5'):idx(dm, 'v5') + 1]))))
    got_t = c.m_table()
    iv = idx(got_t, 'v5')
    got = []
    for row in got_t['rows']:
        got.append(set(int(x) for x in re.findall(r':\s?(\d{3})', cell(row, iv))))
    need(len(got) == len(want) == 41, 'M rows: contract %d, decisions %d' % (len(got), len(want)))
    for k, (g, w) in enumerate(zip(got, want), 1):
        need(g == w, 'M%d v5 cites %s, DECISIONS says %s' % (k, sorted(g), sorted(w)))
    docs = [c.doc()] + [c.repo_doc(r) for r in EDITED_DOCS]
    stale = ['external-profile-contract.md:258-259', 'external-profile-contract.md:552-562',
             'external-profile-contract.md:564-573', 'external-profile-contract.md:526-541',
             'external-profile-contract.md:647-650', 'x07-addendum.md:60']
    for text in docs:
        for s in stale:
            need(s not in text, 'stale citation %s used' % s)
        for m in re.finditer(r'([\w\-]+\.md):(\d+)(?:-(\d+))?', text):
            name, a = m.group(1), int(m.group(2))
            b = int(m.group(3)) if m.group(3) else a
            path = c.path('docs/vernacular/' + name)
            if name == os.path.basename(c.doc_path):
                lines = c.doc().split('\n')
            else:
                try:
                    lines = c.read(path).split('\n')
                except MissingDoc:
                    raise CheckFail('citation to missing file %s' % name)
            need(1 <= a and b <= len(lines), 'citation %s:%d points past end of file' % (name, a))
            if name == 'planar-design-spec-v5.md' and 374 <= a <= 385:
                need(v5[a - 1].lstrip('|').strip().startswith(V5_KEYS.get(a, '\0')),
                     'citation %s:%d is not the exact v5 row (window 0)' % (name, a))
    return 'M-table v5 cites equal DECISIONS section M; state-table cites hit exact rows'


@check('CHK-I42-links-anchors')
def _i42(c):
    ex = c.doc()
    fixed = ['ex-1-format3', 'ex-2-request-framing-and-replay', 'ex-3-execution-commit-protocol',
             'ex-4-facts-derived-state-and-reconcile-rules',
             'ex-5-role-structure-lineage-and-work_exec_v1', 'ex-6-interpretation-binding',
             'ex-7-error-codes-and-messages', 'ex-8-execution-bounds', 'state-machines',
             'part-2-placeholders-r15-a2', 'messages', 'pointer-amendments', 'traceability',
             'acceptance-checklist', 'decisions-table']
    own = {slug(t) for _, _, t in heading_list(ex)}
    for a in fixed:
        need(a in own, 'fixed anchor missing: #%s' % a)
    for rel in EDITED_DOCS + [DOC_REL]:
        text = c.doc() if rel == DOC_REL else c.repo_doc(rel)
        base = c.path('docs/vernacular') if rel == DOC_REL else os.path.dirname(c.path(rel))
        # Links inside inline code spans are quoted text, not links; percent escapes are decoded.
        for link in re.findall(r'\]\(([^)\s]+)\)', re.sub(r'`[^`\n]*`', '', text)):
            link = urllib.parse.unquote(link)
            if link.startswith('http') or link.startswith('mailto:'):
                continue
            target, _, anchor = link.partition('#')
            if target == '':
                tdoc = text
                tpath = rel
            else:
                tpath = os.path.normpath(os.path.join(base, target))
                need(os.path.exists(tpath), '%s links a missing file %s' % (rel, target))
                tdoc = c.read(tpath) if tpath.endswith('.md') else ''
            if anchor and tpath.endswith('.md'):
                ts = {slug(t) for _, _, t in heading_list(tdoc)}
                need(anchor in ts, '%s links missing anchor #%s in %s' % (rel, anchor, target or rel))
    return 'relative links and fixed anchors resolve'


@check('CHK-I43-redaction')
def _i43(c):
    script = c.path('scripts/redact-local-paths.py')
    need(os.path.exists(script), 'scripts/redact-local-paths.py missing')
    doc_abs = os.path.abspath(c.doc_path)
    c.doc()
    home = os.path.expanduser('~')
    res = subprocess.run([sys.executable, script, '--repo-path', c.repo, '--home', home, 'check', doc_abs],
                         capture_output=True, text=True)
    need(res.returncode == 0, 'redaction check failed for the contract: %s' % res.stdout.strip()[:200])
    tmp = tempfile.mkdtemp(prefix='ex_checks_')
    try:
        ctl = os.path.join(tmp, 'control.md')
        with open(ctl, 'w', encoding='utf-8') as fh:
            fh.write('control path %s/docs/x.md\n' % c.repo)
        res2 = subprocess.run([sys.executable, script, '--repo-path', c.repo, '--home', home, 'check', ctl],
                              capture_output=True, text=True)
        need(res2.returncode != 0, 'positive control with the repository root was not refused')
    finally:
        for f in glob.glob(os.path.join(tmp, '*')):
            os.remove(f)
        os.rmdir(tmp)
    return 'redaction check passes for the contract; positive control refused'


@check('CHK-I44-identity-src-tests')
def _i44(c):
    tracked = c.git('diff', '--name-only', 'HEAD', '--', 'src', 'tests').split()
    untracked = c.git('ls-files', '--others', '--exclude-standard', '--', 'src', 'tests').split()
    need(not tracked and not untracked, 'src or tests changed: %s %s' % (tracked[:5], untracked[:5]))
    return 'no tracked or untracked change under src or tests (evidence folder is gitignored)'


@check('CHK-I45-diff-scope')
def _i45(c):
    changed = set(c.git('diff', '--name-only', 'HEAD').split())
    changed |= set(c.git('ls-files', '--others', '--exclude-standard').split())
    # The roadmap and the round review are not owned files: they must stay byte-identical to
    # the frozen copies (G8), so they are compared with cmp and excluded from the owned list.
    # C02 amendment (D26): the untracked C01 review is a frozen file too. C04 amendment: so is the C03 review.
    frozen = {'docs/vernacular/roadmap.md': 'roadmap.md', FROZEN_REVIEW_REL: 'V5-02-R15-A1-review.md',
              C01_REVIEW_REL: 'V5-02-R15-A1-C01-review.md',
              C02_REVIEW_REL: 'V5-02-R15-A1-C02-review.md',
              C03_REVIEW_REL: 'V5-02-R15-A1-C03-review.md'}
    for rel, name in frozen.items():
        need(same_bytes(c.path(rel), os.path.join(c.frozen_dir, name)),
             '%s differs from its frozen copy' % rel)
    extra = sorted(p for p in changed if p not in OWNED_PATHS and p not in frozen
                   and not p.startswith(EVID_REL + '/') and not p.startswith(C01_EVID_REL + '/'))
    need(not extra, 'changes outside owned files: %s' % extra[:8])
    return 'changed and untracked paths are within the owned file list (%d)' % len(changed)


@check('CHK-I46-record-no-hash')
def _i46(c):
    targets = [c.read(c.path(ROUND_REL))]
    if os.path.isfile(c.path(README_REL)):
        targets.append(c.read(c.path(README_REL)))
    # This round's review and the old evidence tree's REPORT and review files.
    pats = [c.path(FROZEN_REVIEW_REL), os.path.join(c.repo, EVID_REL, '**', 'REPORT*'),
            os.path.join(c.repo, EVID_REL, '**', '*review*')]
    for pat in pats:
        for f in glob.glob(pat, recursive=True):
            if os.path.isfile(f):
                try:
                    targets.append(open(f, encoding='utf-8').read())
                except (OSError, UnicodeDecodeError):
                    pass
    # The C01 evidence folder: README, REPORT, plan files (incl. superseded-*) and every text file
    # under red/, green/, raw/ and probes/.
    ev = c.evidence_dir
    for pat in (os.path.join(ev, 'README*'), os.path.join(ev, 'REPORT*'),
                os.path.join(ev, 'plan', '**', '*.md')):
        for f in glob.glob(pat, recursive=True):
            if os.path.isfile(f):
                try:
                    targets.append(open(f, encoding='utf-8').read())
                except (OSError, UnicodeDecodeError):
                    pass
    for sub in ('red', 'green', 'raw', 'probes'):
        for _, text in text_files(os.path.join(ev, sub)):
            targets.append(text)
    tokens = set()
    for t in targets:
        tokens |= set(re.findall(r'\b[0-9a-f]{64}\b', t))
    files = glob.glob(os.path.join(c.repo, 'docs/vernacular/rounds/*.md')) + [c.path('docs/vernacular/roadmap.md')]
    hashes = {}
    for f in files:
        try:
            with open(f, 'rb') as fh:
                hashes[hashlib.sha256(fh.read()).hexdigest()] = f
        except OSError:
            pass
    hit = [hashes[t] for t in tokens if t in hashes]
    need(not hit, 'record contains a sha256 of a forbidden file: %s' % hit[:3])
    return 'round record, C01 evidence and REPORT/review files carry no sha256 of rounds or roadmap'


@check('CHK-I47-sql-executes')
def _i47(c):
    need(sqlite3.sqlite_version_info >= (3, 37, 0), 'SQLite older than 3.37 (STRICT)')
    conn = c.base_db()
    t = c.f3()['by']
    try:
        conn.execute("UPDATE records SET encoded = ? WHERE id = 1", [b'z'])
        raise CheckFail('UPDATE of immutable records succeeded')
    except sqlite3.Error as e:
        need('immutable ledger row' in str(e), 'UPDATE of records error is not immutable ledger row: %s' % e)
    conn2 = c.base_db()
    insert(conn2, 'current_state', auto_row(t['current_state'], {'singleton': 1, 'last_seq': 1}))
    try:
        conn2.execute('DELETE FROM current_state')
        raise CheckFail('DELETE of mutable current_state succeeded')
    except sqlite3.Error as e:
        need('immutable ledger row' in str(e), 'DELETE error is not immutable ledger row: %s' % e)
    conn3 = c.base_db()
    insert(conn3, 'reservations', auto_row(t['reservations'], {'id': 1, 'request_id': 'req-1'}))
    names = {r[0] for r in conn3.execute("SELECT name FROM sqlite_master WHERE type='index' AND sql IS NOT NULL")}
    need(names == {'records_digest', 'run_state_one_active'}, 'explicit indexes differ: %s' % sorted(names))
    return 'SQL executes; immutable UPDATE/DELETE refused; reservation before request row inserts; indexes exact'


@check('CHK-I48-enum-checks')
def _i48(c):
    t = c.f3()['by']

    def bad(table, over):
        conn = c.base_db()
        row = auto_row(t[table], over)
        return rejects(lambda: insert(conn, table, row))

    def ok(table, over):
        conn = c.base_db()
        try:
            insert(conn, table, auto_row(t[table], over))
        except sqlite3.Error:
            return False
        return True

    need(bad('transitions', {'reason': 'foo'}), 'transitions reason outside the 16 accepted')
    for r in REASONS16:
        need(ok('transitions', {'reason': r, 'before_state': None, 'before_drifted': None}),
             'transitions reason %s rejected' % r)
    need(bad('transitions', {'after_state': 'bogus'}), 'transitions after_state outside six accepted')
    need(bad('transitions', {'before_state': None, 'before_drifted': 0}),
         'before_state NULL with before_drifted NOT NULL accepted')
    need(bad('run_state', {'state': 'bogus'}), 'run_state.state outside five accepted')
    for s in RUN_STATES:
        need(ok('run_state', {'state': s}), 'run_state state %s rejected' % s)
    need(bad('requests', {'operation': 'bogus'}), 'requests.operation outside ten accepted')
    need(bad('requests', {'outcome': 'reserved'}), 'requests.outcome reserved accepted')
    need(bad('requests', {'outcome': 'zzz'}), 'requests.outcome outside nine accepted')
    for k, o in enumerate(OUTCOMES9):
        conn = c.base_db()
        insert(conn, 'requests', auto_row(t['requests'], {'request_id': 'r%d' % k, 'event_seq': k + 1,
                                                          'outcome': o}))
    need(bad('generations', {'reason': 'bogus'}), 'generations.reason outside four accepted')
    for r in GEN_REASONS:
        need(ok('generations', {'reason': r}), 'generations reason %s rejected' % r)
    # D97 role_state coupling (run_id and drifted to state) and D98 transitions drifted CHECKs.
    for over in ({'state': 'ready', 'run_id': 5}, {'state': 'ready', 'drifted': 1},
                 {'state': 'done', 'drifted': 1}, {'state': 'running', 'run_id': None}):
        need(bad('role_state', over), 'role_state accepts %s (D97)' % over)
    need(ok('role_state', {'state': 'running', 'run_id': 1, 'drifted': 0}), 'running role_state rejected (D97)')
    need(ok('role_state', {'state': 'running', 'run_id': 1, 'drifted': 1}), 'running drifted role_state rejected (D97)')
    for s in ('blocked', 'ready', 'done', 'stale', 'retired'):
        need(ok('role_state', {'state': s, 'run_id': None, 'drifted': 0}), 'role_state %s rejected (D97)' % s)
    need(bad('transitions', {'after_state': 'ready', 'after_drifted': 1}), 'transitions after ready drifted accepted (D98)')
    need(bad('transitions', {'after_state': 'done', 'after_drifted': 1}), 'transitions after done drifted accepted (D98)')
    need(bad('transitions', {'before_state': 'ready', 'before_drifted': 1, 'after_state': 'ready'}),
         'transitions before ready drifted accepted (D98)')
    need(ok('transitions', {'after_state': 'running', 'after_drifted': 1}), 'transitions after running drifted rejected (D98)')
    need(ok('transitions', {'before_state': 'running', 'before_drifted': 1, 'after_state': 'ready'}),
         'transitions before running drifted rejected (D98)')
    # The reason of evidence is native-writer enforced and reader-verified only (D100): no CHECK.
    need(ok('evidence', {'reason': 'bogus'}), 'evidence reason outside the writer set refused (D100 has no CHECK)')
    return 'enum CHECKs refuse out-of-set values and accept every design value; D97/D98 probes'


@check('CHK-I49-active-run-uniqueness')
def _i49(c):
    t = c.f3()['by']
    want_ix = "CREATE UNIQUE INDEX run_state_one_active ON run_state(job, role) WHERE state='active';"
    need(any(norm(x) == want_ix for x in c.f3()['indexes']),
         'run_state_one_active index statement is not the D85 partial unique index')
    conn = c.base_db()
    base = {'job': 'j', 'role': 'r', 'generation_id': 1, 'last_seq': 1}
    insert(conn, 'run_state', auto_row(t['run_state'], dict(base, run_id=1, state='active')))
    dup = rejects(lambda: insert(conn, 'run_state', auto_row(t['run_state'], dict(base, run_id=2, state='active'))))
    need(dup, 'two active runs for one job-role accepted')
    conn.execute("UPDATE run_state SET state='fenced' WHERE run_id=1")
    insert(conn, 'run_state', auto_row(t['run_state'], dict(base, run_id=3, state='active')))
    return 'partial unique index refuses a second active run; fenced row frees the slot'


@check('CHK-I50-no-tbd')
def _i50(c):
    d = c.doc()
    for w in ('TODO', 'TBD', 'to be decided', 'to be fixed', 'fixed by the implementing round'):
        need(w.lower() not in d.lower(), 'closed-text hygiene: %s present' % w)
    fence = False
    for line in d.split('\n'):
        if FENCE_RE.match(line):
            fence = not fence
            continue
        if not fence and '?' in line:
            raise CheckFail('question mark in normative text: %s' % line[:80])
    need(re.search('[\U0001F300-\U0001FAFF☀-➿]', d) is None, 'emoji present')
    return 'no TBD, no question marks in normative text, no emoji'


@check('CHK-I51-header')
def _i51(c):
    d = c.doc()
    pre = d.split('\n## ', 1)[0]
    need('Status' in pre, 'header lacks Status')
    need('Authority' in pre and 'E1' in pre and 'E17' in pre, 'header lacks Authority E1..E17')
    need('v1 bod' in pre or 'v1 bodies' in pre, 'header lacks the v1-bodies amendment statement')
    need('design' in pre.lower() and 'source' in pre.lower(), 'header does not name the design as source')
    need(not has(pre, 'narrow', ':241') or has(pre, 'E5'), 'header claims narrowing of v5 :241 without citing E5')
    need(has(pre, 'specifies no') or has(pre, 'no check'), 'header lacks the no-check/verdict statement')
    need('narrows no clause of any approved contract' in norm(pre), 'header lacks the statement that it narrows no clause of any approved contract')
    return 'header states status, authority, amendment and source rules'


def source_tree_has(c, msg):
    root = c.path('src')
    for dp, _, fns in os.walk(root):
        for fn in fns:
            if not fn.endswith(('.cpp', '.cppm', '.lua', '.h', '.hpp')):
                continue
            try:
                with open(os.path.join(dp, fn), encoding='utf-8') as fh:
                    if contains_exact(fh.read(), msg):
                        return True
            except (OSError, UnicodeDecodeError):
                continue
    return False


def f2_statements(c):
    sql = sql_block(c.repo_doc(LEDGER_REL))
    tbls = {}
    for s in sql_statements(sql):
        m = TABLE_RE.match(s)
        if m:
            tbls[m.group(1)] = s
    return tbls


@check('CHK-I52-format2-exact')
def _i52(c):
    f2 = f2_statements(c)
    f3 = c.f3()['by']
    for t in ('tree_edges', 'revisions'):
        need(t in f2 and body_line_set(f2[t]) == body_line_set(f3[t]['stmt']),
             '%s differs from format2 SQL' % t)
    need('CHECK(format=3)' in f3['ledger_meta']['stmt'], 'ledger_meta format check not format3')
    for t in ('ledger_meta', 'records'):
        a = [l for l in body_line_set(f2[t]) if not l.startswith('format ') and not l.startswith('codec ')]
        b = set(body_line_set(f3[t]['stmt']))
        for l in a:
            need(l in b, '%s format2 line not byte-exact in format3: %s' % (t, l[:60]))
    changed = {'events': ('type ', 'version '), 'requests': ('outcome ',), 'current_state': ()}
    for t, skip in changed.items():
        b = set(body_line_set(f3[t]['stmt']))
        for l in body_line_set(f2[t]):
            if skip and l.startswith(skip):
                continue
            need(l in b, '%s format2 column line not byte-exact: %s' % (t, l[:60]))
    need(any(norm(x) == 'CREATE INDEX records_digest ON records(codec,kind,digest);'
             for x in c.f3()['indexes']), 'format2 records_digest index statement not byte-exact')
    return 'unmodified format2 SQL lines byte-exact in format3'


@check('CHK-I53-ex1-columns')
def _i53(c):
    f = c.f3()['by']
    d = c.design()
    missing = []
    for t, cols in d.items():
        if t not in f:
            missing.append(t)
            continue
        for name, _ in cols:
            if name not in f[t]['cols']:
                missing.append('%s.%s' % (t, name))
    need(not missing, 'design EX-1 columns missing from SQL: %s' % missing[:10])
    for t, col, expr in [('runs', 'secret', 'length(secret)=16'), ('dispatches', 'work_fp', 'length(work_fp)=16'),
                         ('dispatches', 'render_fp', 'length(render_fp)=16'),
                         ('publications', 'semantic', 'length(semantic)=16'),
                         ('requests', 'result', 'length(result)<=1048576'),
                         ('events', 'payload', 'length(payload)<=1048576')]:
        need(expr in ns(f[t]['cols'].get(col, '')), '%s.%s lacks CHECK %s' % (t, col, expr))
    for t, col in [('requests', 'event_seq'), ('reservations', 'request_id'), ('verdicts', 'request_id')]:
        need('UNIQUE' in f[t]['cols'].get(col, ''), '%s.%s lacks UNIQUE' % (t, col))
    need('PRIMARYKEY(job,role)' in ns(' '.join(f['role_state']['cons'])), 'role_state PK (job, role) missing')
    need('PRIMARYKEY(event_seq,ordinal)' in ns(' '.join(f['transitions']['cons'])),
         'transitions PK (event_seq, ordinal) missing')
    return 'design columns present; BLOB, UNIQUE and key choices as required'


@check('CHK-I54-run-token')
def _i54(c):
    sec = c.sec('EX-2')
    need('<run id>.<generation ordinal>.<32 lowercase hex>' in sec.replace('`', ''),
         'run token grammar missing')
    need(has(sec, 'secret', '16') and (has(sec, 'OS') or has(sec, 'random')), 'secret 16 OS-random bytes missing')
    need('8 bytes' not in sec, 'secret stated as 8 bytes')
    shown = [u for u in units(sec) if 'token' in u.lower() and 'shown' in u.lower()]
    need(any('status --request-id' in u.lower() for u in shown),
         'run token shown places omit status --request-id')
    for u in units(sec):
        ul = u.lower()
        if 'token' in ul and 'shown in status' in ul and not re.search(r'\bnot\b|never|no ', ul):
            raise CheckFail('token shown in status without the request-lookup qualifier')
    need(has(sec, 'secret', 'fenced'), 'wrong secret fenced statement missing')
    need(not has(sec, 'secret', 'state_conflict'), 'wrong secret mapped to state_conflict')
    need(any(h[1] == 3 and 'token' in h[2].lower() for h in heading_list(sec)),
         'run token has no ### subsection in EX-2')
    return 'run token grammar, 16-byte secret, places shown and never shown, fenced on mismatch'


@check('CHK-I55-request-outcomes')
def _i55(c):
    t = c.f3()['by']
    ov = in_values(t['requests']['cols'].get('outcome', ''))
    need(ov is not None and sorted(ov) == sorted(OUTCOMES9), 'requests.outcome CHECK is not the nine-value union')
    d = c.doc()
    pairs = {'load': ['accepted', 'noop'], 'activate': ['activated'], 'claim': ['claimed'],
             'takeover': ['taken_over'], 'refresh': ['refreshed'], 'checkpoint': ['recorded'],
             'release': ['released', 'interrupted'], 'check': ['recorded', 'interrupted'],
             'check-recover': ['interrupted'], 'verdict': ['recorded']}
    rows = [r for tb in tables(d) for r in tb['rows'] if r]
    for op, outs in pairs.items():
        hit = [r for r in rows if r[0].strip().lower() == op]
        need(hit, 'pairing row for %s missing' % op)
        txt = ' '.join(hit[0])
        for o in outs:
            need(o in txt, '%s pairing lacks %s' % (op, o))
        need('reserved' not in txt, '%s paired with reserved' % op)
    need(has(d, 'no check request row', 'reserve') or has(d, 'no request', 'reserve'),
         'no check request row at reserve time statement missing')
    need(has(d, 'fenced', 'not pre-decided') or has(d, 'fenced', 'not decided'),
         'R17-A fenced-completion not pre-decided statement missing')
    return 'nine-value union CHECK; pairing table; no reserved value'


@check('CHK-I56-cause-reason')
def _i56(c):
    t = c.m_table()
    tr = m_expansions(t)
    need_t = [(None, 'blocked', 'activated'), (None, 'ready', 'activated'),
              ('retired', 'blocked', 'activated'), ('retired', 'ready', 'activated'),
              ('blocked', 'retired', 'retired'), ('ready', 'retired', 'retired'),
              ('done', 'retired', 'retired'), ('stale', 'retired', 'retired'),
              ('running', 'retired', 'fenced'), ('blocked', 'blocked', 'structural_change'),
              ('running', 'blocked', 'structural_change'), ('running', 'ready', 'structural_change'),
              ('retired', 'blocked', 'recreated'), ('retired', 'ready', 'recreated'),
              ('ready', 'blocked', 'prerequisite_unavailable'), ('done', 'blocked', 'prerequisite_unavailable'),
              ('stale', 'blocked', 'prerequisite_unavailable'), ('blocked', 'ready', 'prerequisite_available'),
              ('blocked', 'done', 'prerequisite_available'), ('blocked', 'stale', 'prerequisite_available'),
              ('running', 'running', 'work_changed'), ('done', 'stale', 'work_changed'),
              ('stale', 'done', 'work_changed'), ('stale', 'done', 'evidence_inapplicable'),
              ('done', 'stale', 'evidence_inapplicable'), ('done', 'stale', 'repair_admitted'),
              ('running', 'running', 'drift_cleared'), ('ready', 'running', 'claimed'),
              ('stale', 'running', 'claimed'), ('running', 'running', 'taken_over'),
              ('running', 'done', 'completed'),
              ('done', 'stale', 'evidence_inapplicable')]
    miss = triple_missing(tr, need_t)
    need(not miss, 'cause-to-reason rows missing or wrong: %s' % miss[:6])
    ir = idx(t, 'reason')
    ia = idx(t, 'after')
    for n, row in enumerate(t['rows'], 1):
        if cell(row, ir) == 'fenced':
            need(norm_state(cell(row, ia)) == 'retired', 'M%d fenced reason outside retirement' % n)
    return 'every cause carries its pinned reason; fenced only for retirement with an active run'


@check('CHK-I57-payload-version')
def _i57(c):
    d = c.doc()
    need(has(d, 'load_noop', 'version 2'), 'load_noop payload version 2 missing')
    need(has(d, 'ledger_initialized', 'version 1'), 'ledger_initialized payload version 1 missing')
    need(not has(d, 'load_noop', 'version 1'), 'load_noop stated as payload version 1')
    need(has(d, 'reason', 'ordinal 1', 'empty string'), 'event reason is ordinal 1 or empty string missing')
    need(not re.search(r'\bordinal[\s-]+0\b', d.replace('`', ''), re.I), 'event reason stated from ordinal 0')
    need(not has(d, 'operation name', 'reason'), 'operation name allowed as event reason')
    return 'payload versions pinned; event reason rule'


@check('CHK-I58-fallbacks')
def _i58(c):
    d = c.doc()
    names = set(re.findall(r'([A-Za-z][A-Za-z\-]*) may be complete; retry the same request ID or inspect request history', d))
    need(set(FALLBACK_OPS) <= names, 'fallback messages missing: %s' % sorted(set(FALLBACK_OPS) - names))
    need(names - {'Load'} == set(FALLBACK_OPS), 'fallback message set differs: %s' % sorted(names))
    for line in d.split('\n'):
        if 'may be complete;' in line and line.lstrip().startswith('|'):
            need(re.search(r'\bio\b', line) and re.search(r'\b4\b', line), 'fallback row is not io exit 4: %s' % line[:80])
    need('stdout' in d and has(d, 'stdout', 'may be complete'), 'stdout variant missing')
    need('Load may be complete; retry the same request ID or inspect request history' in d,
         'load fallback not stated byte-exact')
    return 'nine execution fallbacks, io exit 4, stdout variant, load fallback unchanged'


@check('CHK-I59-record-additions')
def _i59(c):
    r = c.read(c.path(ROUND_REL))
    need(has(r, 'CHK-', 'mutation prober') or has(r, 'ex_checks'), 'verification-addition paragraph missing')
    need(has(r, 'HALT-1', 'delegat', '2026-10-31'), 'HALT-1 delegated decision (2026-10-31) missing')
    halt = flat(block(r, '(b) HALT-1', 3))
    need(DELEGATED_TAG in halt, 'HALT-1 section (b) lacks the delegated tag text')
    hb = bullet_block(block(r, '(k) Delegated decisions', 3), '- **HALT-1 (D54).**')
    need(DELEGATED_TAG in hb, 'HALT-1 bullet lacks the delegated tag text')
    cc = block(r, '(c) ', 3)
    for op in ('OP1', 'OP2', 'OP3'):
        ob = bullet_block(cc, '- **%s,' % op)
        need(DELEGATED_TAG in ob, '%s bullet in section (c) lacks the delegated tag text' % op)
    hits = [h for h in live_answer_hits(r) if 'HALT-1' in h]
    need(not hits, 'HALT-1 recorded as a live answer: %s' % hits[:1])
    need(has(r, 'OP1') and has(r, 'OP2'), 'OP1/OP2 not flagged')
    return 'round record carries the agreed verification additions and HALT-1 delegation'


@check('CHK-I60-role-rows')
def _i60(c):
    t = c.m_table()
    need(len(t['rows']) == 41, 'M table row count != 41')
    ie = idx(t, 'event')
    iv = idx(t, 'v5')
    rows = t['rows']

    def ev(n):
        return cell(rows[n - 1], ie).lower()

    for n in range(17, 23):
        for e in ('declaration_accepted', 'check_recorded', 'verdict_recorded'):
            need(e in ev(n), 'M%d event cell lacks %s' % (n, e))
    for n in (23, 24, 25, 40):
        for e in ('declaration_accepted', 'check_recorded'):
            need(e in ev(n), 'M%d event cell lacks %s' % (n, e))
    for n in (38, 41):
        need('check_recorded' in ev(n), 'M%d event cell lacks check_recorded' % n)
    need('verdict_recorded' in ev(39), 'M39 event cell lacks verdict_recorded')
    need(sorted(int(x) for x in re.findall(r':\s?(\d{3})', cell(rows[17], iv))) == [381],
         'M18 v5 cite is not :381')
    tr = m_expansions(t)
    need_t = [('ready', 'blocked', 'prerequisite_unavailable'), ('done', 'blocked', 'prerequisite_unavailable'),
              ('stale', 'blocked', 'prerequisite_unavailable'), ('blocked', 'ready', 'prerequisite_available'),
              ('blocked', 'done', 'prerequisite_available'), ('blocked', 'stale', 'prerequisite_available'),
              ('ready', 'running', 'claimed'), ('stale', 'running', 'claimed'),
              ('done', 'stale', 'work_changed'), ('stale', 'done', 'work_changed'),
              ('stale', 'done', 'evidence_inapplicable'), ('done', 'stale', 'repair_admitted'),
              ('done', 'stale', 'evidence_inapplicable')]
    for e in ('ready', 'stale', 'blocked', 'done'):
        need_t.append(('running', e, 'released'))
    miss = triple_missing(tr, need_t)
    need(not miss, 'coverage rows missing: %s' % miss[:5])
    bad = stale_ready_rows(c)
    need(not bad, 'stale -> ready rows without a new-generation cause and an allowed reason: %s' % bad)
    return 'coverage rows, carrying-event cells and v5 cites as pinned'


@check('CHK-I61-load-activate-preconditions')
def _i61(c):
    d = c.doc()
    sec = d
    need('request_unused' in sec and 'active_revision' in sec and 'active_job' in sec,
         'load and activate precondition set missing')
    need(has(sec, 'fact_version', 'every role', 'role_state') or has(sec, 'fact_version', 'every role'),
         'fact_version and role_state for every role missing')
    need(has(sec, 'state_conflict') and (has(sec, 'same request ID') or has(sec, 'same-ID') or
                                         has(sec, 'same ID')),
         'claim during snapshot-commit refusal as state_conflict missing')
    need(has(sec, 'R15-A2', 'precondition'), 'per-verb precondition sets pointer missing')
    need(not [u for u in units(sec) if re.search(r'\bonly\b', u, re.I) and 'transition' in u.lower() and
              re.search(r'\bload\b|activate|precondition', u, re.I) and 'role_state' in u],
         'load checks role_state only for transitioned roles')
    return 'load and activate assert fact_version and role_state for every role'


@check('CHK-I62-reactivation')
def _i62(c):
    t = c.m_table()
    tr = m_expansions(t)
    need_t = []
    for b in ('blocked', 'ready', 'done', 'stale', 'running'):
        need_t.append((b, 'blocked', 'activated'))
        need_t.append((b, 'ready', 'activated'))
    miss = triple_missing(tr, need_t)
    need(not miss, 're-activation rows (M7/M8) missing: %s' % miss[:4])
    ie = idx(t, 'event')
    need('job_activated' in cell(t['rows'][6], ie) and 'job_activated' in cell(t['rows'][7], ie),
         'M7/M8 event is not job_activated')
    n = c.n_table()
    row5 = n['rows'][4]
    need(norm_state(cell(row5, idx(n, 'before'))) == 'active' and cell(row5, idx(n, 'after')) == 'fenced'
         and 'job_activated' in ' '.join(row5) and 'activated' in ' '.join(row5),
         'N5 re-activation run row missing')
    d = c.doc()
    need(not [u for u in units(d) if 'reuse' in u.lower() and 'generation' in u.lower() and
              're-activation' in u.lower() and not re.search(r'\bnot\b|never', u, re.I)],
         'literal re-activation reuses generations')
    need(not [u for u in units(d) if 're-activation' in u.lower() and 'noop' in u.lower() and
              not re.search(r'\bnot\b', u, re.I)], 're-activation is a noop')
    return 'M7/M8 re-activation rows with reason activated; N5 fenced by job_activated'


@check('CHK-I63-event-payload-columns')
def _i63(c):
    d = c.doc()
    need('after:{...}' not in d, 'after written literally')
    rows = {}
    qhead = None
    for tb in tables(d):
        if find_table([tb], 'version') and find_table([tb], 'role'):
            qhead = tb
            break
    need(qhead is not None, 'Q payload/typed-column table missing')
    iv = idx(qhead, 'version')
    ik = find_table([qhead], 'references') and idx(qhead, 'references')
    ij = idx(qhead, 'job')
    ir = idx(qhead, 'role')
    ig = idx(qhead, 'generation')
    irun = idx(qhead, 'run')
    il = idx(qhead, 'label')
    for r in qhead['rows']:
        if r:
            rows[clean(r[0])] = r
    need('ledger_initialized' in rows and cell(rows['ledger_initialized'], iv).startswith('1'),
         'ledger_initialized payload version 1 missing')
    for ty in ('declaration_accepted', 'load_noop'):
        r = rows.get(ty)
        need(r is not None and cell(r, iv).startswith('2'), '%s payload version 2 missing' % ty)
        need(all(cell(r, k).upper().startswith('NULL') for k in (ij, ir, ig, irun)),
             '%s typed columns must be NULL' % ty)
    r = rows.get('job_activated')
    need(r is not None and cell(r, iv).startswith('2') and 'activation' in ' '.join(r) and 'revision' in ' '.join(r),
         'job_activated v2 keys activation, revision missing')
    need(cell(r, ir).upper().startswith('NULL') and not cell(r, ij).upper().startswith('NULL'),
         'job_activated must set job only')
    r = rows.get('run_claimed')
    need(r is not None and all(k in ' '.join(r) for k in ('run', 'dispatch', 'packet', 'manifest', 'work')),
         'run_claimed reference keys missing')
    need(all(not cell(r, k).upper().startswith('NULL') for k in (ij, ir, ig, irun)),
         'run_claimed must set job, role, generation and run')
    r = rows.get('run_taken_over')
    need(r is not None and 'previous_run' in ' '.join(r), 'run_taken_over lacks previous_run')
    need(all(not cell(r, k).upper().startswith('NULL') for k in (ij, ir, ig, irun)),
         'run_taken_over must set job, role, generation and run')
    for ty in ('run_refreshed', 'checkpoint_recorded', 'run_released', 'run_interrupted',
               'check_reserved', 'check_recorded', 'check_interrupted', 'verdict_recorded'):
        need(ty in rows and 'reserved' in ' '.join(rows[ty]).lower(), '%s not labelled reserved' % ty)
    return 'Q table: payload versions, reference keys, typed columns and reserved types'


@check('CHK-I64-limit-names')
def _i64(c):
    sec = c.sec('EX-8')
    pairs = [('execution_snapshot_bytes', '16777216'), ('manifest_bytes', '16777216'),
             ('provenance_bytes', '65536'), ('provenance_depth', '4'), ('provenance_settings', '32'),
             ('provenance_string_bytes', '1024'), ('verdict_bytes', '1048576'),
             ('verdict_findings', '256'), ('verdict_finding_bytes', '4096'),
             ('verdict_summary_bytes', '16384'), ('check_retained_bytes', '67108864'),
             ('facet_file_bytes', '1048576'), ('facet_bytes', '4194304'),
             ('check_observation_bytes', '1073741824'), ('check_observation_files', '100000'),
             ('check_observation_depth', '64')]
    text_all = sec.replace(',', '')
    for name, num in pairs:
        lines = [l for l in text_all.split('\n') if re.search(r'(?<!\w)%s(?!\w)' % name, l)]
        need(lines and any(num in l for l in lines), 'limit %s not pinned to %s' % (name, num))
    for name, num in [('observation_entries', '1024'), ('observation_file_bytes', '65536'),
                      ('observation_total_bytes', '4194304'), ('observation_bytes', '1048576')]:
        lines = [l for l in sec.split('\n') if re.search(r'(?<!\w)%s(?!\w)' % name, l)]
        need(lines and all('Capture limit exceeded' in l and MSG_RESOURCE not in l for l in lines),
             'observe limit %s not kept with Capture limit exceeded' % name)
    need(has(sec, 'details', 'limit', 'maximum'), 'details limit/maximum statement missing')
    # The field statement ("The field of every code below is the empty string") sits in EX-7.
    need(has(c.doc(), 'field', 'empty string'), 'field empty statement missing')
    return 'limit names pinned; existing observe names keep Capture limit exceeded'


@check('CHK-I65-fact-version')
def _i65(c):
    d = c.doc()
    f = c.f3()['by']
    need('fact_version INTEGER NOT NULL CHECK(fact_version>=1)' in ' '.join(f['role_state']['cols'].get('fact_version', '').split()) or
         'CHECK(fact_version>=1)' in ns(f['role_state']['cols'].get('fact_version', '')),
         'fact_version column lacks CHECK(fact_version>=1)')
    need(has(d, 'fact_version', 'exactly 1') or has(d, 'by exactly one'),
         'exactly-one increment rule missing')
    need(has(d, 'no other role') or has(d, 'for no other role'), 'no-other-role rule missing')
    need(has(d, 'nothing', 'claim') or has(d, 'increments nothing'), 'no-bump-no-increment statement missing')
    need(has(d, 'first row', 'fact_version') or has(d, 'first', 'fact_version', '1'),
         'first row written with fact_version 1 missing')
    need(has(d, 'role_state', 'transitions row', 'never'), 'no exception to the delta rule statement missing')
    need(has(d, 'load_noop', 'nothing') or has(d, 'load_noop', 'bumps nothing'), 'load_noop bumps nothing missing')
    need(has(d, 'blocker', 'transition row') or has(d, 'blocker', 'amendment'),
         'blocker change deferral to R18 missing')
    for u in units(d):
        ul = u.lower()
        every_inc = re.search(r'(increments?|bumps?|bumped|incremented)\b[^.]{0,60}\b(every|all) roles?\b', ul) or \
            re.search(r'\b(every|all) roles?\b[^.]{0,40}\b(increments?|bumps?|bumped|incremented)\b', ul)
        if every_inc and 'transitions row' not in ul and 'transition row' not in ul and \
                not re.search(r'\bnot\b|never|no other', ul):
            raise CheckFail('an increment applies to roles without a transitions row: %s' % u[:120])
    for u in units(d):
        if 'bump' in u.lower() and 'claim' in u.lower() and not re.search(r'\bnot\b|nothing|never|only', u, re.I):
            raise CheckFail('claim listed as a bumping fact: %s' % u[:120])
    return 'fact_version rules: exactly-one, no other role, first row 1, no exception, SQL >=1'


@check('CHK-I66-reused-messages')
def _i66(c):
    d = c.doc()
    msec = c.sec('Messages')
    t = find_table(tables(msec), 'code', 'existing')
    need(t is not None, 'Reused existing codes table missing')
    codes = {}
    for r in t['rows']:
        if r and r[0]:
            codes.setdefault(r[0], []).append(r)
    need(len(codes) >= 10, 'fewer than the ten code families in the reused table: %s' % sorted(codes))
    srcs = {'response': c.read(c.path(RESPONSE_REL)), 'selection': c.read(c.path(SELECTION_REL)),
            'load': c.repo_doc(LOAD_REL), 'auth': c.repo_doc(AUTH_REL)}
    msg_col = idx(t, 'existing')
    own_col = idx(t, 'owner')
    for code, rows in codes.items():
        for r in rows:
            m = clean(cell(r, msg_col))
            need(m, 'reused row %s has no message' % code)
            owner = cell(r, own_col)
            need(owner, 'reused row %s has no owner' % code)
            if 'no single string' in m:
                continue  # explanatory prose in the schema row, not a message
            need(m not in MESSAGES_19, 'reused row %s is in the 19-message set' % code)
            need(any(contains_exact(s, m) for s in srcs.values()) or source_tree_has(c, m),
                 'reused message not found in any owner: %s' % m)
            if 'load-command' in owner:
                need(contains_exact(srcs['load'], m), 'message %r not byte-equal in load-command-contract' % m)
            if 'authoring' in owner:
                need(contains_exact(srcs['auth'], m), 'message %r not byte-equal in authoring-contract' % m)
            if 'existing mapping' in owner:
                need(any(contains_exact(srcs[k], m) for k in ('response', 'selection', 'auth', 'load')) or
                     source_tree_has(c, m),
                     'message %r not found in existing source' % m)
    need('Ledger version or state is unsupported' in srcs['response'], 'response.cpp lacks unsupported text')
    return 'reused existing codes named byte-exact with owners; none in the 19-message set'


@check('CHK-I67-column-conventions')
def _i67(c):
    f = c.f3()
    by = f['by']
    for t in f['tables']:
        if t['name'] in ('ledger_meta', 'current_state'):
            continue
        for name, d in t['cols'].items():
            if re.search(r'PRIMARY\s+KEY', d) and 'INTEGER' in d and name not in ('request_id',):
                need('CHECK(%s>0)' % name in ns(d), 'PK %s.%s lacks CHECK(%s>0)' % (t['name'], name, name))
        for name in ('drifted', 'before_drifted', 'after_drifted', 'output_truncated'):
            if name in t['cols']:
                need('%sIN(0,1)' % name in ns(t['cols'][name]), '%s.%s lacks boolean CHECK' % (t['name'], name))
        for name in ('job', 'role'):
            if name in t['cols']:
                need('length(%s)BETWEEN1AND64' % name in ns(t['cols'][name]),
                     '%s.%s lacks length 1..64 CHECK' % (t['name'], name))
    enums = [('generations', 'reason', GEN_REASONS), ('dispatches', 'kind', DISPATCH_KINDS),
             ('evidence', 'outcome', EVIDENCE_OUTCOMES), ('verdicts', 'applicability', APPLICABILITY),
             ('verdicts', 'decision', DECISIONS_ENUM), ('blockers', 'kind', BLOCKER_KINDS),
             ('role_state', 'state', STATES6), ('run_state', 'state', RUN_STATES),
             ('requests', 'operation', REQUEST_OPS), ('requests', 'outcome', OUTCOMES9),
             ('transitions', 'reason', REASONS16), ('events', 'type', EVENT14)]
    for tb, col, vals in enums:
        got = in_values(by[tb]['cols'].get(col, ''))
        need(got is not None and sorted(got) == sorted(vals), '%s.%s enum differs: %s' % (tb, col, got))
    need(sorted(by['reservation_state']['cols']) == sorted(['reservation_id', 'state', 'last_seq']),
         'reservation_state columns differ')
    need(sorted(by['blocker_state']['cols']) == sorted(['blocker_id', 'state', 'cleared_seq', 'last_seq']),
         'blocker_state columns differ')
    need(in_values(by['reservation_state']['cols'].get('state', '')) == ['open', 'completed', 'interrupted'],
         'reservation_state.state enum differs')
    need(in_values(by['blocker_state']['cols'].get('state', '')) == ['open', 'cleared'],
         'blocker_state.state enum differs')
    # Executed conventions.
    def bad(table, over):
        conn = c.base_db()
        return rejects(lambda: insert(conn, table, auto_row(by[table], over)))

    def good(table, over):
        conn = c.base_db()
        try:
            insert(conn, table, auto_row(by[table], over))
        except sqlite3.Error:
            return False
        return True

    need(bad('records', {'id': 0}), 'zero id accepted')
    need(bad('role_state', {'drifted': 2}), 'boolean 2 accepted')
    need(bad('role_state', {'role': 'r' * 65}), 'role of 65 bytes accepted')
    need(bad('role_state', {'state': 'zz'}), 'role_state enum outside set accepted')
    need(bad('role_state', {'fact_version': 0}), 'role_state fact_version 0 accepted')
    need(bad('blocker_state', {'state': 'open', 'cleared_seq': 3}), 'open blocker with cleared_seq accepted')
    need(bad('blocker_state', {'state': 'cleared', 'cleared_seq': None}), 'cleared blocker without cleared_seq accepted')
    need(bad('run_state', {'state': 'zz'}), 'run_state enum outside set accepted')
    need(bad('reservation_state', {'state': 'zz'}), 'reservation_state enum outside set accepted')
    need(bad('generations', {'ordinal': 0}), 'ordinal 0 accepted')
    need(bad('dispatches', {'kind': 'zz'}), 'dispatch kind outside set accepted')
    need(bad('evidence', {'outcome': 'zz'}), 'evidence outcome outside set accepted')
    need(bad('verdicts', {'decision': 'zz'}), 'verdict decision outside set accepted')
    need(bad('blockers', {'kind': 'zz'}), 'blocker kind outside set accepted')
    need(bad('transitions', {'reason': 'zz'}), 'transition reason outside set accepted')
    need(good('role_state', {'job': 'j', 'role': 'r', 'drifted': 0, 'fact_version': 1}), 'conforming role_state rejected')
    need(good('run_state', {'state': 'active'}), 'conforming run_state rejected')
    need(good('reservation_state', {'state': 'open'}), 'conforming reservation_state rejected')
    need(good('blocker_state', {'state': 'open', 'cleared_seq': None}), 'conforming blocker_state rejected')
    need(good('blocker_state', {'blocker_id': 2, 'state': 'cleared', 'cleared_seq': 3}),
         'conforming cleared blocker_state rejected')
    # D100: evidence.reason is TEXT NOT NULL with no CHECK (native-writer and reader enforced).
    reason = by['evidence']['cols'].get('reason', '')
    need('NOT NULL' in reason.upper() and 'CHECK' not in reason.upper(),
         'evidence.reason is not TEXT NOT NULL without a CHECK (D100)')
    need(not [k for k in by['evidence']['cons'] if re.search(r'\breason\b', k)],
         'evidence has a table-level CHECK on reason (D100)')
    # D97 and D98: table-level coupling CHECKs (role_state, transitions).
    rs_cons = ns(' '.join(by['role_state']['cons']))
    need(ns("CHECK((state='running' AND run_id IS NOT NULL) OR (state<>'running' AND run_id IS NULL AND drifted=0))")
         in rs_cons, 'role_state run_id/drifted coupling CHECK missing (D97)')
    tr_cons = ns(' '.join(by['transitions']['cons']))
    need(ns("CHECK(after_state='running' OR after_drifted=0)") in tr_cons,
         'transitions after_drifted CHECK missing (D98)')
    need(ns("CHECK(before_state IS NULL OR before_state='running' OR before_drifted=0)") in tr_cons,
         'transitions before_drifted CHECK missing (D98)')
    return 'column conventions: PK, booleans, identifiers, enums, projection tables, D97/D98, D100'


@check('CHK-I68-carrying-events')
def _i68(c):
    d = c.doc()
    need(has(d, 'carried by') or has(d, 'carry'), 'derived-transition carrying rule missing')
    need(has(d, 'operation whose committed fact') or has(d, 'event of the operation'),
         'carrying-event rule sentence missing')
    need(has(d, 'declaration_accepted', 'only', 'reconcile') or has(d, 'declaration_accepted', 'only legal'),
         'reconcile must not read declaration_accepted as the only event')
    t = c.m_table()
    ie = idx(t, 'event')
    for n, row in enumerate(t['rows'], 1):
        if 'check_recorded' in cell(row, ie).lower():
            need('R17' in ' '.join(row), 'M%d carries check_recorded without the R17 label' % n)
        if 'verdict_recorded' in cell(row, ie).lower():
            need('R18' in ' '.join(row), 'M%d carries verdict_recorded without the R18 label' % n)
    for n in range(17, 26):
        need('check_recorded' in cell(t['rows'][n - 1], ie).lower(), 'M%d lacks check_recorded' % n)
    return 'carrying-event rule stated; check_recorded and verdict_recorded labelled'


@check('CHK-I69-stale-done')
def _i69(c):
    t = c.m_table()
    tr = m_expansions(t)
    need(('stale', 'done', 'work_changed') in tr, 'M40 stale -> done work_changed missing')
    need(('stale', 'done', 'evidence_inapplicable') in tr, 'M41 stale -> done evidence_inapplicable missing')
    bad = stale_ready_rows(c)
    need(not bad, 'stale -> ready rows without a new-generation cause and an allowed reason: %s' % bad)
    ie = idx(t, 'event')
    ir = idx(t, 'reason')
    for n, row in enumerate(t['rows'], 1):
        if cell(row, ir) == 'work_changed' and norm_state(cell(row, idx(t, 'before'))) == 'stale':
            need('declaration_accepted' in cell(row, ie) and 'check_recorded' in cell(row, ie),
                 'M40 event cell lacks declaration_accepted and check_recorded')
        if cell(row, ir) == 'evidence_inapplicable' and norm_state(cell(row, idx(t, 'before'))) == 'stale':
            need('check_recorded' in cell(row, ie).lower(), 'M41 event cell lacks check_recorded')
    d = c.doc()
    need(has(d, 'no stale to ready row exists within one generation'),
         'no stale -> ready statement missing')
    need(has(d, 'M8', 'M10', 'new generation'), 'M8 and M10 open-a-new-generation statement missing')
    need(has(d, 'R17-6', 'not narrowed'), 'R17-6 not narrowed statement missing')
    dec = c.sec('Decisions table')
    op3 = [r for t2 in tables(dec) for r in t2['rows'] if r and r[0] == 'OP3']
    need(op3, 'OP3 missing from the decisions table')
    txt = ' '.join(op3[0])
    need('(A)' in txt and '(B)' in txt and '(C)' in txt, 'OP3 options A, B, C missing')
    return 'M40 and M41 rows present, no stale -> ready, OP3 options in decisions table'


@check('CHK-I70-absent-active-job')
def _i70(c):
    t = c.m_table()
    ic = idx(t, 'cause')
    rows = t['rows']
    for n in (13, 14):
        cause = cell(rows[n - 1], ic).lower()
        need('role' in cause and ('absent' in cause or 'active job' in cause) and 'removed' in cause,
             'M%d must name both causes (role removed and active job absent)' % n)
    n7 = c.n_table()
    cause7 = cell(n7['rows'][6], idx(n7, 'cause')).lower()
    need('removed' in cause7 and 'absent' in cause7, 'N7 names only one cause')
    d = c.doc()
    need(has(d, 'current_state.active_job', 'NULL', 'active_activation_id'),
         'NULL active_job after commit with active_activation_id unchanged missing')
    need(has(d, 'active_job', 'empty string'), 'after payload active_job empty-string statement missing')
    need(has(d, 'other than', 'active_job', 'retired'), 'non-active-job rows retired invariant missing')
    # Scripted replay over the contract's SQL and transitions.
    conn = c.base_db()
    f = c.f3()['by']
    tr = m_expansions(t)
    need(('ready', 'retired', 'retired') in tr and ('blocked', 'retired', 'retired') in tr,
         'transitions for M5 (non-running previous-job roles) missing')
    need(('running', 'retired', 'fenced') in tr, 'transitions for M6 (previous-job running role) missing')
    insert(conn, 'current_state', auto_row(f['current_state'], {'singleton': 1, 'last_seq': 1,
                                                                 'active_job': 'J1', 'active_activation_id': 1}))
    # Activate J1 with roles a (ready) and b (blocked).
    for i, (role, st) in enumerate([('a', 'ready'), ('b', 'blocked')], 1):
        insert(conn, 'role_state', auto_row(f['role_state'], {'job': 'J1', 'role': role, 'generation_id': i,
                                                              'state': st, 'drifted': 0, 'fact_version': 1,
                                                              'last_seq': 1}))

    def invariant(active):
        q = "SELECT COUNT(*) FROM role_state WHERE state != 'retired' AND (job IS NOT ? OR ? IS NULL)"
        return conn.execute(q, [active, active]).fetchone()[0] == 0

    need(invariant('J1'), 'invariant fails after activating J1')
    # Activate J2 with role x: J1 roles retire (M5 for non-running, M6 for running).
    conn.execute("UPDATE current_state SET active_job='J2', active_activation_id=2 WHERE singleton=1")
    insert(conn, 'role_state', auto_row(f['role_state'], {'job': 'J2', 'role': 'x', 'generation_id': 3,
                                                          'state': 'ready', 'drifted': 0, 'fact_version': 1,
                                                          'last_seq': 2}))
    for role, before in (('a', 'ready'), ('b', 'blocked')):
        if ((before, 'retired', 'retired') in tr):
            conn.execute("UPDATE role_state SET state='retired', fact_version=fact_version+1, last_seq=2 "
                         "WHERE job='J1' AND role=?", [role])
    need(invariant('J2'), 'invariant fails after activating J2 (M5 transitions omitted?)')
    # Load without J2: the active job is absent, every J2 role retires (M13 / M14(b)).
    conn.execute("UPDATE current_state SET active_job=NULL WHERE singleton=1")
    if ('ready', 'retired', 'retired') in tr or ('blocked', 'retired', 'retired') in tr:
        conn.execute("UPDATE role_state SET state='retired', fact_version=fact_version+1, last_seq=3 "
                     "WHERE job='J2'")
    need(invariant(None), 'invariant fails after load without J2 (M14(b) transitions omitted?)')
    return 'absent-job rows named; scripted replay keeps non-active rows retired'


# ================================================================ C01 helpers

LIVE_RE = re.compile(r'\blive (?:operator )?answers?\b', re.I)
NEG_RE = re.compile(r'\b(?:not|no|never|nor)\b', re.I)
PENDING_PHRASES = ('operator-pending', 'op-pending', 'recommended option applied', 'op2 is pending')
DISPATCH_START = ('Under the standing delegation of 2026-10-31 these are accepted at their '
                  'recommended options')
PREAMBLE_TEXT = ('The tag set is closed: `contract-decided`, `operator-decided` and `new-engineering`. '
                 'OP1, OP2, OP3, D54, D67 and D68 are operator-decided under the standing delegation of '
                 '2026-10-31, option A (`operator-decided (delegated 2026-10-31, option A)`), recorded as '
                 'delegated, not as live answers; the operator may overrule them.')
E2_SENTENCE = ('The contract is [execution contract EX-4](execution-contract.md#ex-4-facts-derived-state-and-reconcile-rules) '
               'for activation and re-activation and [EX-5](execution-contract.md#ex-5-role-structure-lineage-and-work_exec_v1) '
               'for the generations and fencing that activation causes.')
E5_SENTENCE = ('The contract is [execution contract EX-5](execution-contract.md#ex-5-role-structure-lineage-and-work_exec_v1) '
               'for the fingerprint.')
FROZEN_DOCS = ['CLAUDE.md', LEDGER_REL, EXT_REL, MATRIX_REL, IMPL_REL]
# (case id, outcome) rows that the SQL audit (C29) must find in green/sql-c01.txt.
SQL_CASES = [
    ('R-RS1', 'REFUSED'), ('R-RS2', 'REFUSED'), ('R-RS3', 'REFUSED'), ('R-RS4', 'REFUSED'),
    ('A-RS1', 'ACCEPTED'), ('A-RS2', 'ACCEPTED'), ('A-RS3-blocked', 'ACCEPTED'),
    ('A-RS3-ready', 'ACCEPTED'), ('A-RS3-done', 'ACCEPTED'), ('A-RS3-stale', 'ACCEPTED'),
    ('A-RS3-retired', 'ACCEPTED'), ('R-TR1', 'REFUSED'), ('R-TR2', 'REFUSED'), ('R-TR3', 'REFUSED'),
    ('R-TR4', 'REFUSED'), ('A-TR1', 'ACCEPTED'), ('A-TR2', 'ACCEPTED'), ('R-OR0', 'REFUSED'),
    ('A-OR1', 'ACCEPTED'), ('R-EV-NULL', 'REFUSED'), ('A-EV-PASS-EMPTY', 'ACCEPTED'),
    ('A-EV-PASS-X', 'ACCEPTED'), ('A-EV-FAIL-TIMEOUT', 'ACCEPTED'), ('A-EV-FAIL-EMPTY', 'ACCEPTED'),
    ('A-EV-INCONCLUSIVE', 'ACCEPTED'), ('A-EV-FAIL-BOGUS', 'ACCEPTED')]
# Check ids of the repaired and new checks, in registration order (CHK-C01 ids appended at the end).
C01_NEW_IDS = [
    'CHK-C01-R1-token-status-lookup', 'CHK-C01-R1-status-qualifier', 'CHK-C01-R1-a09-lookup',
    'CHK-C01-R2-event-reason-ordinal1', 'CHK-C01-R2-ordinal-check', 'CHK-C01-R3-op1-rule-unchanged',
    'CHK-C01-R4-role-state-run-drift', 'CHK-C01-R4-transitions-drifted',
    'CHK-C01-R4-run-id-replay-text', 'CHK-C01-R4-states-text', 'CHK-C01-R5-c6-text',
    'CHK-C01-R5-evidence-reason-check', 'CHK-C01-decisions-rows', 'CHK-C01-R7-header-scope',
    'CHK-C01-R8-m-note', 'CHK-C01-R9-d56-basis', 'CHK-C01-R9-d55-corruption',
    'CHK-C01-R10-alignment-pointers', 'CHK-C01-R11-cutover-target', 'CHK-C01-R12-ex8-label',
    'CHK-C01-relabel-contract', 'CHK-C01-relabel-record', 'CHK-C01-relabel-no-live',
    'CHK-C01-sql-counts', 'CHK-C01-sql-legal-rows', 'CHK-C01-names-preserved',
    'CHK-C01-frozen-files', 'CHK-C01-message-bytes', 'CHK-C01-sql-evidence-recorded',
    'CHK-C01-green-evidence', 'CHK-C01-prober-evidence', 'CHK-C01-record-closure',
    'CHK-C01-red-evidence', 'CHK-C01-extractor-comments', 'CHK-C01-record-d81']
# Three-line subjects of the closure table rows R1 to R12 (C32); R6 is handled separately.
CLOSURE_SUBJECTS = {1: 'status --request-id', 2: 'ordinal', 3: 'required scalar', 4: 'drifted',
                    5: 'evidence.reason', 7: 'ex-1 to ex-8', 8: 'non-running', 9: 'new-engineering',
                    10: 'ex-4', 11: 'at a new explicit target', 12: 'none yet'}


def flat(s):
    """Whitespace collapsed, backticks and bold markers removed; for phrase matching."""
    return norm(clean(s))


def bullet_block(text, prefix):
    """Flat text of the bullet or paragraph whose first line starts with prefix."""
    lines = text.split('\n')
    for i, line in enumerate(lines):
        if line.startswith(prefix):
            out = [line]
            for nxt in lines[i + 1:]:
                if not nxt.startswith(' ') or not nxt.strip():
                    break
                out.append(nxt)
            return flat(' '.join(out))
    raise CheckFail('bullet not found: %s' % prefix)


def numbered_item(text, marker):
    """Flat text of the numbered list item whose first line holds marker (continuation lines indented)."""
    lines = text.split('\n')
    for i, line in enumerate(lines):
        if re.match(r'^\d+\.\s', line) and marker in line:
            out = [line]
            for nxt in lines[i + 1:]:
                if not nxt.startswith(' ') or not nxt.strip():
                    break
                out.append(nxt)
            return flat(' '.join(out))
    raise CheckFail('numbered item not found: %s' % marker)


def decision_rows(c):
    """Decisions-table rows keyed by ID, cells cleaned (ID, Choice, Tag, Basis)."""
    out = {}
    for t in tables(c.sec('Decisions table')):
        for r in t['rows']:
            if r and re.fullmatch(r'(D\d+|OP\d+|HALT-\d+)', r[0]):
                out[r[0]] = r
    return out


def drow(c, did):
    rows = decision_rows(c)
    need(did in rows, 'decisions table lacks row %s' % did)
    return rows[did]


def preamble(c):
    """Flat text of the Decisions table paragraph that precedes the table."""
    out = []
    for line in c.sec('Decisions table').split('\n')[1:]:
        if line.lstrip().startswith('|'):
            break
        out.append(line)
    return flat(' '.join(out))


def accepts(c, table, over):
    """True when a row built by auto_row(over) inserts into a fresh database from the contract SQL."""
    conn = c.base_db()
    row = auto_row(c.f3()['by'][table], over)
    try:
        insert(conn, table, row)
    except sqlite3.Error:
        return False
    return True


def same_bytes(a, b):
    """Byte comparison (cmp semantics); no hash is produced."""
    for p in (a, b):
        if not os.path.isfile(p):
            raise CheckFail('file missing: %s' % p)
    with open(a, 'rb') as fa, open(b, 'rb') as fb:
        return fa.read() == fb.read()


def file_sha(path):
    with open(path, 'rb') as fh:
        return hashlib.sha256(fh.read()).hexdigest()


def text_files(root):
    """(path, text) for every UTF-8 file under root (recursive); binary files are skipped."""
    out = []
    for dp, _, fns in os.walk(root):
        for fn in sorted(fns):
            p = os.path.join(dp, fn)
            try:
                with open(p, encoding='utf-8') as fh:
                    out.append((p, fh.read()))
            except (OSError, UnicodeDecodeError):
                continue
    return out


def contains_exact(s, m):
    """True when m occurs in s as a whole string (no word character on either side)."""
    return re.search(r'(?<!\w)%s(?!\w)' % re.escape(m), s) is not None


def live_answer_hits(text):
    """Snippets of 'live answer' phrases that are not negated within the preceding 30 characters.

    Rows of an invariant table (`| I<n> |`) quote the mutant text of their check; that quoted defect
    is not a claim, so those rows are not scanned. Every other line, including decisions rows, is.
    """
    kept = [l for l in text.split('\n') if not re.match(r'^\|\s*I\d+\s*\|', l)]
    flat_text = '\n'.join(kept).replace('`', '')
    hits = []
    for m in LIVE_RE.finditer(flat_text):
        before = flat_text[max(0, m.start() - 30):m.start()]
        if not NEG_RE.search(before):
            hits.append(' '.join(flat_text[max(0, m.start() - 60):m.end() + 10].split()))
    return hits


def pending_hits(text):
    """Pending wording outside the quoted dispatch paragraph (case-insensitive)."""
    keep = [p for p in re.split(r'\n\s*\n', text) if not norm(p).startswith(DISPATCH_START)]
    low = norm(' '.join(keep)).lower()
    return [p for p in PENDING_PHRASES if p in low]


def stale_ready_rows(c):
    """Role-table rows with a stale -> ready row that lack an allowed reason or a new-generation cause."""
    t = c.m_table()
    ib, ia, ir, ic = idx(t, 'before'), idx(t, 'after'), idx(t, 'reason'), idx(t, 'cause')
    causes = {n: cell(row, ic) for n, row in enumerate(t['rows'], 1)}
    out = []
    for n, row in enumerate(t['rows'], 1):
        befores = [norm_state(b) for b in cell(row, ib).split(',')]
        if 'stale' not in befores or norm_state(cell(row, ia)) != 'ready':
            continue
        cause = clean(causes[n])
        m = re.search(r'same as M(\d+)', cause)
        if m:
            cause = clean(causes.get(int(m.group(1)), ''))
        ok_reason = cell(row, ir) in ('activated', 'structural_change')
        ok_cause = re.search(r'new (?:empty )?generation', cause.lower()) is not None
        if not (ok_reason and ok_cause):
            out.append('M%d' % n)
    return out


def ext_loc(c):
    """Line number (1-based) of the external-profile 'flags cannot be required' statement, found by text."""
    ext = c.repo_doc(EXT_REL).split('\n')
    for i in range(len(ext) - 1):
        if 'flags cannot be required' in norm(ext[i] + ' ' + ext[i + 1]).lower():
            return i + 1
    raise CheckFail('external-profile "Flags cannot be required" not found by text')


def evid_text(c, *parts):
    """Text of a C01 evidence file; an absent file is a named absence (audit rows only)."""
    p = c.evid(*parts)
    need(os.path.isfile(p), 'missing file: %s' % '/'.join(parts))
    with open(p, encoding='utf-8') as fh:
        return fh.read()


def heading_block(text, pred):
    """Text from the first heading whose title satisfies pred to the next heading of its level or higher."""
    lines = text.split('\n')
    hs = heading_list(text)
    for k, (i, lv, title) in enumerate(hs):
        if pred(title):
            end = len(lines)
            for j, lv2, _ in hs[k + 1:]:
                if lv2 <= lv:
                    end = j
                    break
            return '\n'.join(lines[i:end])
    return None


def c01_section(text):
    return heading_block(text, lambda t: 'C01' in t and 'closure' in t.lower())


def fail_ids(text):
    return set(re.findall(r'^FAIL (CHK-\S+)', text, re.M))


def resolve_repo_file(c, name):
    for base in ('', 'docs/vernacular', 'docs/vernacular/rounds', C01_EVID_REL):
        p = os.path.join(c.repo, base, name)
        if os.path.isfile(p):
            return p
    return None


# ================================================================ C01 checks (appended, C.27 order)

@check('CHK-C01-R1-token-status-lookup')
def _c01_r1_token_status(c):
    shown = [u for u in units(c.sec('EX-2')) if 'shown only in' in u.lower()]
    need(shown, 'run token "shown only in" sentence missing')
    need(any('status --request-id' in u.lower() and 'claim' in u.lower() for u in shown),
         'shown-only list of the run token omits status --request-id of a claim or takeover request')
    return 'run token shown-only list names status --request-id of claim and takeover requests'


@check('CHK-C01-R1-status-qualifier')
def _c01_r1_status_qualifier(c):
    d = flat(c.doc()).lower()
    need(re.search(r'never shown in status(?! other than its request lookup)', d) is None,
         'token is never shown in status without the request-lookup qualifier')
    need('never shown in status other than its request lookup' in d,
         'the never-shown list lacks the qualified status wording')
    return 'token is never shown in status other than its request lookup'


@check('CHK-C01-R1-a09-lookup')
def _c01_r1_a09(c):
    a09 = [u for u in units(c.sec('Acceptance checklist')) if 'lost response' in u.lower()]
    need(any('request lookup' in u.lower() and 'status --request-id' in u.lower() and
             'run token' in u.lower() for u in a09),
         'A09 item does not name status --request-id as the recovery lookup returning the run token')
    need(has(c.doc(), 'saved result', 'status --request-id', 'run token'),
         'Disposition paragraph does not say the saved result carries the run token')
    return 'A09 names status --request-id as the recovery lookup; Disposition says the saved result carries the token'


@check('CHK-C01-R2-event-reason-ordinal1')
def _c01_r2_event_reason(c):
    d = c.doc()
    para = [p for p in re.split(r'\n\s*\n', d) if 'Event payload.' in p]
    need(len(para) == 1, 'EX-4 event payload paragraph not found once')
    need(re.search(r'lowest ordinal.*ordinal 1', flat(para[0]).lower()) is not None,
         'EX-4 event payload does not take the reason from the lowest ordinal, ordinal 1')
    need('contiguous from 1' in bullet_block(d, '- C5.').lower(),
         'C5 does not say the ordinals of one event are contiguous from 1')
    need('ordinal 1' in ' '.join(drow(c, 'D43')).lower(), 'D43 does not take the reason from ordinal 1')
    need('ordinal 1' in ' '.join(drow(c, 'D101')).lower(), 'D101 does not name ordinal 1')
    need(re.search(r'\bordinal[\s-]+0\b', flat(d), re.I) is None,
         'the contract still names ordinal 0 for the event reason')
    return 'event reason from the lowest ordinal (ordinal 1); C5 contiguous from 1; no ordinal 0'


@check('CHK-C01-R2-ordinal-check')
def _c01_r2_ordinal_sql(c):
    need(not accepts(c, 'transitions', {'ordinal': 0}), 'transitions accepts ordinal 0')
    need(accepts(c, 'transitions', {'ordinal': 1}), 'transitions refuses ordinal 1')
    return 'transitions ordinal 0 refused, ordinal 1 accepted'


@check('CHK-C01-R3-op1-rule-unchanged')
def _c01_r3_op1(c):
    op1 = ' '.join(drow(c, 'OP1'))
    fop1 = flat(op1).lower()
    need('required scalar' in fop1, 'OP1 does not say --confirm-stopped is a required scalar')
    need('external-profile rule' in fop1 and 'unchanged' in fop1,
         'OP1 does not say the external-profile rule is unchanged')
    loc = ext_loc(c)
    cites = [(int(m.group(1)), int(m.group(2) or m.group(1)))
             for m in re.finditer(r'external-profile-contract\.md:(\d+)(?:-(\d+))?', fop1)]
    need(any(a <= loc <= b for a, b in cites),
         'OP1 does not cite the located external-profile line (%d)' % loc)
    need('this changes that rule' not in flat(c.doc()).lower(),
         'the contract says OP1 changes the external-profile rule')
    return 'OP1 is a required scalar; the external-profile rule is unchanged; located cite'


@check('CHK-C01-R4-role-state-run-drift')
def _c01_r4_role_state(c):
    refused = [{'state': 'ready', 'run_id': 5, 'drifted': 0}, {'state': 'ready', 'run_id': None, 'drifted': 1},
               {'state': 'done', 'run_id': None, 'drifted': 1}, {'state': 'running', 'run_id': None, 'drifted': 0}]
    for over in refused:
        need(not accepts(c, 'role_state', over), 'role_state accepts %s (D97)' % (over,))
    for over in ({'state': 'running', 'run_id': 5, 'drifted': 0}, {'state': 'running', 'run_id': 5, 'drifted': 1}):
        need(accepts(c, 'role_state', over), 'role_state refuses running %s (D97)' % (over,))
    for s in ('blocked', 'ready', 'done', 'stale', 'retired'):
        over = {'state': s, 'run_id': None, 'drifted': 0}
        need(accepts(c, 'role_state', over), 'role_state refuses %s with run_id NULL and drifted 0' % s)
    return 'role_state: running needs run_id, other states need run_id NULL and drifted 0'


@check('CHK-C01-R4-transitions-drifted')
def _c01_r4_transitions(c):
    refused = [{'after_state': 'ready', 'after_drifted': 1}, {'after_state': 'done', 'after_drifted': 1},
               {'before_state': 'ready', 'before_drifted': 1, 'after_state': 'ready', 'after_drifted': 0},
               {'before_state': None, 'before_drifted': 0}]
    for over in refused:
        need(not accepts(c, 'transitions', over), 'transitions accepts %s (D98)' % (over,))
    for over in ({'after_state': 'running', 'after_drifted': 1},
                 {'before_state': 'running', 'before_drifted': 1, 'after_state': 'ready', 'after_drifted': 0}):
        need(accepts(c, 'transitions', over), 'transitions refuses %s (D98)' % (over,))
    return 'transitions: drifted 1 only for the running state; NULL-first coupling still refuses'


@check('CHK-C01-R4-run-id-replay-text')
def _c01_r4_replay(c):
    d = c.doc()
    inv6 = numbered_item(d, '`role_state` equals the replay of all')
    low = inv6.lower()
    need('the run_id of the events row that carries that row when that event\'s type is run_claimed or '
         'run_taken_over' in low, 'invariant 6 lacks the run_claimed or run_taken_over source sentence')
    need('and its events.role (and events.job) is that role, that is the target of the operation' in low,
         'invariant 6 lacks the target-role restriction')
    need('whose after_state is also running' in low, 'invariant 6 does not restrict the previous row to running')
    need('takes the run_id derived for that role\'s previous transitions row' in low,
         'invariant 6 lacks the previous-row derivation for other running rows')
    need('cross-checks the result against the one run_state row with state active' in low,
         'invariant 6 lacks the run_state cross-check')
    runs = [r for r in numbered_runs(d) if any(n == 6 for n, _ in r)]
    need(runs and not any(n >= 21 for n, _ in runs[0]), 'a bullet 21 was added to the invariants')
    return 'invariant 6 restricts the run_id source to the target role and cross-checks run_state'


@check('CHK-C01-R4-states-text')
def _c01_r4_states(c):
    d = c.doc()
    states = [p for p in re.split(r'\n\s*\n', d) if '**States.**' in p]
    need(len(states) == 1, 'EX-4 States paragraph not found once')
    sf = flat(states[0]).lower()
    need('for every state other than running, drifted is 0 and run_id is null' in sf,
         'EX-4 States does not say drifted is 0 and run_id NULL for every state other than running')
    need('a running role has a non-null run_id' in sf, 'EX-4 States lacks the running non-NULL run_id')
    need('after_drifted is 0 unless after_state is running' in sf, 'EX-4 States lacks after_drifted 0 unless running')
    inv12 = numbered_item(d, 'Coupling: a role is').lower()
    need('every role state other than running has run_id null and drifted 0' in inv12,
         'invariant 12 lacks the non-running run_id NULL and drifted 0 rule')
    need('a running role has a non-null run_id' in inv12, 'invariant 12 lacks the running non-NULL run_id')
    need('is null when before_state is null and 0 when before_state is a non-running state' in inv12,
         'invariant 12 lacks the before_drifted NULL and 0 rule')
    need('after_drifted is 0 unless after_state is running' in inv12,
         'invariant 12 lacks the after_drifted rule')
    need(re.search(r'before_drifted\s+(?:is|=)\s+0\s+when\s+before_state\s+is\s+null', flat(d).lower()) is None,
         'text says before_drifted is 0 when before_state is NULL')
    return 'EX-4 States and invariant 12 state the drifted and run_id coupling (D97) and before_drifted (D97)'


@check('CHK-C01-R5-c6-text')
def _c01_r5_c6(c):
    d = c.doc()
    c6 = bullet_block(d, '- C6.').lower()
    need('native-writer enforced and reader-verified only' in c6,
         'C6 does not say evidence.reason is native-writer enforced and reader-verified only')
    need('no sql check' in c6, 'C6 does not say there is no SQL CHECK on evidence.reason')
    need('r17-3 lists no reason value for a pass outcome, so r17 states that value' in c6,
         'C6 lacks the R17-3 pass-outcome sentence')
    need('has no value list' not in flat(d).lower(), 'C6 still says evidence.reason has no value list')
    return 'C6 states evidence.reason is writer enforced, reader verified, with no SQL CHECK'


@check('CHK-C01-R5-evidence-reason-check')
def _c01_r5_evidence(c):
    ev = c.f3()['by']['evidence']
    reason = ev['cols'].get('reason', '')
    need('CHECK' not in reason.upper() and 'NOT NULL' in reason.upper(),
         'evidence.reason is not TEXT NOT NULL without a CHECK')
    need(not [k for k in ev['cons'] if re.search(r'\breason\b', k)], 'evidence has a table-level CHECK on reason')
    for outcome, rsn in [('pass', ''), ('pass', 'exit_status'), ('fail', 'timeout'), ('fail', ''),
                         ('inconclusive', 'input_changed'), ('pass', 'x'), ('fail', 'bogus')]:
        need(accepts(c, 'evidence', {'outcome': outcome, 'reason': rsn}),
             'evidence refuses (%s, %r)' % (outcome, rsn))
    need(not accepts(c, 'evidence', {'reason': None}), 'evidence accepts a NULL reason')
    dr = ' '.join(drow(c, 'D100')).lower()
    dr = flat(dr)
    for phrase in ('native-writer enforced and reader-verified only', 'no sql check',
                   'r17-3 lists no reason value for pass'):
        need(phrase in dr, 'D100 row lacks %r' % phrase)
    c6 = bullet_block(c.doc(), '- C6.').lower()
    need('native-writer enforced and reader-verified only' in c6 and 'no sql check' in c6,
         'C6 lacks the writer-enforced and no-SQL-CHECK wording')
    return 'evidence.reason has no CHECK; NULL refused; any text value accepted (D100)'


@check('CHK-C01-decisions-rows')
def _c01_decisions(c):
    rows = decision_rows(c)
    for did in ('D97', 'D98', 'D99', 'D100', 'D101', 'D102', 'D103'):
        need(did in rows, 'decisions table lacks %s' % did)
    need('D104' not in rows and re.search(r'^\|\s*D104\s*\|', c.doc(), re.M) is None,
         'decisions table has a D104 row (D104 belongs in the plan only)')
    tags = {'D97': 'new-engineering', 'D98': 'new-engineering', 'D99': 'new-engineering',
            'D100': 'new-engineering', 'D101': 'new-engineering', 'D102': 'operator-decided',
            'D103': 'contract-decided'}
    for did, tag in tags.items():
        need(cell(rows[did], 2) == tag, '%s tag is not %s' % (did, tag))
    need(cell(rows['D102'], 3) == 'E11 A; R15-9; R15-7', 'D102 basis is not E11 A; R15-9; R15-7')
    d100 = flat(' '.join(rows['D100'])).lower()
    need('no check' in d100 and 'native-writer enforced and reader-verified only' in d100 and
         'no sql check' in d100 and 'r17-3 lists no reason value for pass' in d100,
         'D100 row lacks the fallback wording (no CHECK, writer enforced, no SQL CHECK, R17-3 pass)')
    need('that the native writer enforces and the reader verifies' not in d100,
         'D100 uses the superseded phrase')
    d99 = flat(' '.join(rows['D99'])).lower()
    need("only for the role that event's events.role names" in d99, 'D99 lacks the target-role restriction')
    d101 = flat(' '.join(rows['D101'])).lower()
    need('the ordinals of one event are contiguous from 1' in d101, 'D101 lacks contiguous from 1')
    need('ordinal 1' in flat(' '.join(rows['D43'])).lower(), 'D43 does not name ordinal 1')
    d55 = rows.get('D55')
    need(d55 is not None and cell(d55, 2) == 'contract-decided' and
         cell(d55, 3) == 'ledger-contract.md:480-481' and
         'I/O, corruption, busy and disk-full 4' in flat(' '.join(d55)),
         'D55 is not the contract-decided corruption wording with its basis')
    d56 = rows.get('D56')
    need(d56 is not None and cell(d56, 2) == 'new-engineering' and cell(d56, 3) == 'EX-8; the E15 row is D78',
         'D56 is not new-engineering with basis EX-8; the E15 row is D78')
    return 'D97 to D103 present with closed tags; no D104; D99, D100, D101, D43, D55, D56 as required'


@check('CHK-C01-R7-header-scope')
def _c01_r7_header(c):
    h = flat(c.doc().split('\n## ', 1)[0]).lower()
    need('ex-1 to ex-8' in h, 'header does not say part 1 holds EX-1 to EX-8')
    need('state machines' in h, 'header does not name the state machines')
    need(re.search(r'(reconcile rules|execution bounds)[^.]*\bfollow\b', h) is None,
         'header says the reconcile rules or the execution bounds follow')
    return 'header states part 1 holds EX-1 to EX-8 and the state machines'


@check('CHK-C01-R8-m-note')
def _c01_r8_note(c):
    t = c.m_table()
    f = flat(c.doc()).lower()
    need('the before cell of m5 is every non-running, non-retired state' in f,
         'role-table note lacks the M5 before-cell sentence')
    need('the before cells of m7 and m8 are every non-retired state, including running and running+drift' in f,
         'role-table note lacks the M7 and M8 before-cell sentence')
    need('m13 has the same before states as m5, and m14 is the running counterpart of m13' in f,
         'role-table note lacks the M13 and M14 sentence')
    need('m5, m7 and m8 are every non-running' not in f, 'the old role-table note sentence remains')
    ib = idx(t, 'before')
    for n in (7, 8):
        bef = cell(t['rows'][n - 1], ib)
        need('running' in bef and 'running+drift' in bef, 'M%d before cell lacks running and running+drift' % n)
    return 'role-table note matches C.16; M7 and M8 cells list running and running+drift'


@check('CHK-C01-R9-d56-basis')
def _c01_r9_d56(c):
    rows = decision_rows(c)
    d56 = rows.get('D56')
    need(d56 is not None and cell(d56, 2) == 'new-engineering' and 'EX-8' in cell(d56, 3) and
         'D78' in cell(d56, 3), 'D56 is not new-engineering with a basis naming EX-8 and D78')
    op = [did for did, r in rows.items() if cell(r, 2).startswith('operator-decided')]
    need(len(op) >= 10, 'fewer than ten operator-decided rows (%d)' % len(op))
    bad = [did for did in op if not (re.search(r'\bE\d{1,2}\b|\bQ(?:-[A-Za-z0-9]+|\d+)\b', cell(rows[did], 3)) or
                                     'delegated' in cell(rows[did], 2))]
    need(not bad, 'operator-decided rows without an E or Q basis and without a delegated tag: %s' % bad)
    return 'D56 is new-engineering (EX-8, D78); %d operator-decided rows all valid under C.23' % len(op)


@check('CHK-C01-R9-d55-corruption')
def _c01_r9_d55(c):
    d55 = flat(' '.join(drow(c, 'D55')))
    need('I/O, corruption, busy and disk-full 4' in d55, 'D55 exit 4 is not I/O, corruption, busy and disk-full')
    need('internal' not in d55.lower(), 'D55 still lists internal for exit 4')
    need('I/O, corruption, busy and disk-full' in flat(c.doc()), 'EX-7 exit 4 wording is not I/O, corruption, busy and disk-full')
    return 'D55 and EX-7 say I/O, corruption, busy and disk-full for exit 4'


E2_SENT_FLAT = flat(E2_SENTENCE)
E5_SENT_FLAT = flat(E5_SENTENCE)


@check('CHK-C01-R10-alignment-pointers')
def _c01_r10_align(c):
    al = c.repo_doc(ALIGN_REL)
    e2 = bullet_block(al, '- Recorded deviation E2')
    e5 = bullet_block(al, '- Recorded narrowing E5')
    need(e2.endswith(E2_SENT_FLAT), 'E2 does not end with the EX-4 and EX-5 sentence (C.20)')
    need(e5.endswith(E5_SENT_FLAT), 'E5 does not end with the EX-5 sentence (C.20)')
    need('EX-2' not in e2 and '#ex-2-request-framing-and-replay' not in e2,
         'E2 still points to EX-2')
    need('EX-2' not in e5 and '#ex-2-request-framing-and-replay' not in e5,
         'E5 still points to EX-2')
    removed = 0
    for h in parse_diff(c.git('diff', 'HEAD', '-U0', '--', ALIGN_REL)):
        removed += len(h['removed'])
    need(removed == 0, 'v5-alignment.md has %d removed lines against HEAD' % removed)
    anchors = {slug(t) for _, _, t in heading_list(c.doc())}
    for a in ('ex-4-facts-derived-state-and-reconcile-rules', 'ex-5-role-structure-lineage-and-work_exec_v1'):
        need(a in anchors, 'anchor #%s does not resolve' % a)
    return 'E2 and E5 point to EX-4 and EX-5 with the C.20 sentences; zero removed lines'


@check('CHK-C01-R11-cutover-target')
def _c01_r11_cutover(c):
    b = bullet_block(c.doc(), '- Public `init` creates format3 only')
    need('at a new explicit target' in b, 'cutover bullet does not say format3 is created at a new explicit target')
    need('same explicit target as format2' not in b, 'cutover bullet still says the same explicit target as format2')
    return 'cutover: init creates format3 only at a new explicit target'


@check('CHK-C01-R12-ex8-label')
def _c01_r12_ex8(c):
    t = find_table(tables(c.sec('EX-8')), 'boundary', 'existing site')
    need(t is not None, 'EX-8 bounds table missing')
    rows = [r for r in t['rows'] if r and r[0].startswith('Check output per check')]
    need(rows, 'EX-8 check-output row missing')
    site = flat(cell(rows[0], idx(t, 'existing'))).lower()
    need('none yet' in site and 'r17 fact' in site and 'output_limit' in site,
         'EX-8 check-output existing-site cell does not say None yet and label output_limit an R17 fact')
    need('over max_output_bytes: a recorded fail outcome' not in site,
         'EX-8 check-output lists the output_limit outcome as an existing site')
    return 'EX-8 check-output row: None yet; output_limit is an R17 fact'


@check('CHK-C01-relabel-contract')
def _c01_relabel_contract(c):
    d = c.doc()
    low = flat(d).lower()
    for p in ('operator-pending', 'recommended option applied', 'op2 is pending'):
        need(p not in low, 'contract still says %r' % p)
    for did in ('OP1', 'OP2', 'OP3', 'D54', 'D67', 'D68'):
        need(cell(drow(c, did), 2) == DELEGATED_TAG, '%s tag cell is not the delegated tag' % did)
    m41 = [l for l in d.split('\n') if l.startswith('| M41 |')]
    need(m41 and DELEGATED_TAG in flat(m41[0]), 'M41 row lacks the delegated tag')
    need(DELEGATED_TAG.lower() in bullet_block(d, '- M41 is tagged').lower(), 'M41 note lacks the delegated tag')
    need(preamble(c) == flat(PREAMBLE_TEXT), 'Decisions preamble differs from DECISIONS C.8')
    return 'contract relabelled: delegated tags on OP1-OP3, D54, D67, D68; M41 and preamble agree'


def _record_and_old_texts(c):
    return [('round record', c.read(c.path(ROUND_REL))),
            ('old DECISIONS', c.read(c.path(DECISIONS_REL))),
            ('old INVARIANTS', c.read(c.path(INVARIANTS_REL)))]


def _row_line(text, rid):
    for line in text.split('\n'):
        if line.startswith('| %s |' % rid):
            return line
    return None


@check('CHK-C01-relabel-record')
def _c01_relabel_record(c):
    for name, text in _record_and_old_texts(c):
        hits = pending_hits(text)
        need(not hits, '%s still has pending wording: %s' % (name, hits))
    rec = c.read(c.path(ROUND_REL))
    for rid in ('I33', 'I54', 'I57', 'I60', 'I67', 'I69'):
        line = _row_line(rec, rid)
        need(line is not None and 'Corrected by C01:' in line, 'record row %s lacks a Corrected by C01 note' % rid)
    old_inv = c.read(c.path(INVARIANTS_REL))
    for rid in ('I33', 'I54', 'I57', 'I60', 'I67', 'I69'):
        line = _row_line(old_inv, rid)
        need(line is not None and 'Corrected by C01:' in line, 'old INVARIANTS row %s lacks a correction note' % rid)
    old_dec = c.read(c.path(DECISIONS_REL))
    for rid in ('D43', 'D55', 'D56'):
        line = _row_line(old_dec, rid)
        need(line is not None and 'Corrected by C01:' in line, 'old DECISIONS row %s lacks a correction note' % rid)
    return 'record and old plan files carry no pending wording; correction notes present'


@check('CHK-C01-relabel-no-live')
def _c01_relabel_no_live(c):
    need('delegated' in preamble(c), 'Decisions preamble does not say delegated')
    for did in ('D54', 'D67', 'D68', 'OP1', 'OP2', 'OP3'):
        need('delegated' in cell(drow(c, did), 2).lower(), '%s is not recorded as delegated' % did)
    for name, text in [('contract', c.doc())] + _record_and_old_texts(c):
        hits = live_answer_hits(text)
        need(not hits, '%s states a live answer: %s' % (name, hits[:1]))
    return 'delegation recorded as delegated; no live-answer phrase in contract, record or old plan'


@check('CHK-C01-sql-counts')
def _c01_sql_counts(c):
    f = c.f3()
    need([t['name'] for t in f['tables']] == TABLES, 'table set or creation order differs')
    need(all(t['stmt'].rstrip().endswith(') STRICT;') for t in f['tables']),
         'a CREATE TABLE does not end with ) STRICT;')
    expect_triggers = sum(1 if t in MUTABLE else 2 for t in TABLES)
    need(len(f['triggers']) == expect_triggers, 'SQL trigger count %d differs from %d' % (len(f['triggers']), expect_triggers))
    conn = c.db()
    trig = conn.execute("SELECT COUNT(*) FROM sqlite_master WHERE type='trigger'").fetchone()[0]
    need(trig == expect_triggers, 'sqlite_master trigger count %d differs from %d' % (trig, expect_triggers))
    idxs = {r[0] for r in conn.execute("SELECT name FROM sqlite_master WHERE type='index' AND sql IS NOT NULL")}
    need(idxs == {'records_digest', 'run_state_one_active'}, 'explicit indexes differ: %s' % sorted(idxs))
    need(c.base_db().execute('PRAGMA foreign_key_check').fetchall() == [], 'PRAGMA foreign_key_check reports rows')
    need('PRAGMA user_version=3' in c.doc(), 'PRAGMA user_version=3 text missing')
    return '24 STRICT tables in order; %d triggers; two explicit indexes; foreign_key_check clean' % expect_triggers


@check('CHK-C01-sql-legal-rows')
def _c01_sql_legal(c):
    for over in ({'state': 'running', 'run_id': 5, 'drifted': 0}, {'state': 'running', 'run_id': 5, 'drifted': 1}):
        need(accepts(c, 'role_state', over), 'legal running role_state row refused: %s' % (over,))
    for s in ('blocked', 'ready', 'done', 'stale', 'retired'):
        need(accepts(c, 'role_state', {'state': s, 'run_id': None, 'drifted': 0}),
             'legal non-running role_state row refused: %s' % s)
    for s in STATES6:
        need(accepts(c, 'transitions', {'after_state': s, 'after_drifted': 0}),
             'legal first transition into %s refused' % s)
    need(accepts(c, 'transitions', {'before_state': 'running', 'before_drifted': 1,
                                    'after_state': 'ready', 'after_drifted': 0}),
         'legal running before cell with before_drifted 1 refused')
    _i47(c)
    return 'legal rows insert; I47 SQL probes pass'


@check('CHK-C01-names-preserved')
def _c01_names(c):
    orig = c.read(c.evid('raw', 'ex_checks-r15a1-orig.py'))
    old_ids = re.findall(r"@check\('(CHK-[^']+)'\)", orig)
    need(old_ids and len(old_ids) == len(set(old_ids)), 'the preserved original registers no unique check ids')
    need(ORDER[:len(old_ids)] == old_ids, 'registered old check ids differ from the preserved original (names or order)')
    need(ORDER[len(old_ids):] == C01_NEW_IDS + C02_NEW_IDS,
         'CHK-C01 and CHK-C02 ids are not appended in the stated order')
    return '%d old ids kept in order; %d CHK-C01 and %d CHK-C02 ids appended; %d registered' % (
        len(old_ids), len(C01_NEW_IDS), len(C02_NEW_IDS), len(ORDER))


@check('CHK-C01-frozen-files')
def _c01_frozen(c):
    base = c.read(c.evid('raw', 'baseline-identity.txt'))
    hashes = dict(re.findall(r'^\| `([^`]+)` \| \w+ \| `([0-9a-f]{64})` \|', base, re.M))
    for rel in FROZEN_DOCS:
        need(rel in hashes, '%s has no baseline hash in raw/baseline-identity.txt' % rel)
        need(file_sha(c.path(rel)) == hashes[rel], '%s differs from the baseline candidate' % rel)
    link = c.path('AGENTS.md')
    need(os.path.islink(link) and os.readlink(link) == 'CLAUDE.md', 'AGENTS.md is not a symlink to CLAUDE.md')
    need(same_bytes(c.path('docs/vernacular/roadmap.md'), os.path.join(c.frozen_dir, 'roadmap.md')),
         'roadmap.md differs from its frozen copy')
    changed = c.git('diff', '--name-only', 'HEAD', '--', 'src', 'tests/native').split()
    changed += c.git('ls-files', '--others', '--exclude-standard', '--', 'src', 'tests/native').split()
    changed = [p for p in changed if not p.startswith((EVID_REL + '/', C01_EVID_REL + '/'))]
    need(not changed, 'src or tests/native changed outside the evidence folders: %s' % changed[:5])
    return 'frozen docs keep their baseline hashes; roadmap matches its frozen copy; src and tests unchanged'


@check('CHK-C01-message-bytes')
def _c01_messages(c):
    d = c.doc()
    for m in MESSAGES_19:
        need(m in d, 'message not byte-identical in the contract: %s' % m)
    need(MSG_RESOURCE in d, 'resource message not byte-identical in the contract')
    removed = []
    for h in parse_diff(c.git('diff', 'HEAD', '-U0', '--', EXT_REL)):
        removed.extend(h['removed'])
    need(not removed, 'external-profile-contract.md removes %d diagnostic lines' % len(removed))
    return '19 new messages and the resource message byte-identical; external profile removes no line'


# ---------------------------------------------------------------- C02 helpers (identity block, logs, tsv)

SEVEN_DOCS = [DOC_REL, ALIGN_REL, LEDGER_REL, EXT_REL, MATRIX_REL, IMPL_REL, 'CLAUDE.md']
ROW_RE = re.compile(r'^\|\s*`([^`]+)`\s*\|\s*([^|]*?)\s*\|\s*`([0-9a-f]{64})`\s*\|')
EXIT0_RE = re.compile(r'exit code\D*0\s*$', re.I | re.M)
EMPTY_SHA = 'e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855'
E_ = C01_EVID_REL
P_ = C02_EVID_REL
# Owned files of C02 (INVARIANTS section 5); the record's '### Owned files (C02)' names every one.
C02_OWNED = [
    DOC_REL, ROUND_REL, E_ + '/README.md', E_ + '/red/RED.md', E_ + '/green/ex_checks-c02.py',
    E_ + '/green/mask_log.py', E_ + '/raw/pre-c02-contract.md', E_ + '/raw/pre-c02-record.md',
    E_ + '/raw/pre-c02-manifest.txt', E_ + '/red/red-c02-run.txt', E_ + '/red/red-c02-audit-run.txt',
    E_ + '/probes/PROBES.md', E_ + '/probes/mutants.tsv', E_ + '/probes/raw', E_ + '/green/checks.txt',
    E_ + '/green/sql-final.txt', E_ + '/green/sql_final.sh', E_ + '/green/audit-final.txt',
    E_ + '/green/audit-verify-c02.txt', E_ + '/green/redaction-final-c02.txt',
    E_ + '/green/identity-c02.txt', E_ + '/green/redaction-c02.txt', E_ + '/green/linkcheck-c02.txt',
    E_ + '/green/linkcheck_c02.py', E_ + '/green/scope-c02.txt', E_ + '/green/frozen-c02.txt',
    E_ + '/green/cmp-c02.txt', E_ + '/green/superseded-1', E_ + '/green/superseded-2',
    P_ + '/plan/DECISIONS.md', P_ + '/plan/INVARIANTS.md', P_ + '/plan/superseded-1',
    P_ + '/plan/superseded-2', P_ + '/plan/superseded-3']
# Written only by the last wave (GREEN2): exempt from existence rules while absent.
C02_GREEN2_ONLY = [E_ + '/green/audit-final.txt', E_ + '/green/audit-verify-c02.txt',
                   E_ + '/green/redaction-final-c02.txt', E_ + '/green/superseded-2']
GUARD_IDS = ['C5', 'C24', 'C25', 'C26', 'C27', 'C28', 'C34', 'C35', 'C36', 'C37', 'C38', 'C44', 'C45',
             'C46', 'C47', 'C49', 'C50', 'C51', 'C52', 'C53', 'C54', 'C55', 'C56', 'C58', 'C59', 'C59b',
             'C60'] + ['G%d' % n for n in range(1, 9)]
N_MAIN_IDS = ['N%d' % n for n in (1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 15)]
CLOSING_IDS = ['C29', 'C30', 'C31', 'C32', 'C33', 'C61', 'N9', 'N16', 'N17']


def identity_block(text):
    """Lines strictly between 'identity begin (seven documents)' and 'identity end', or None."""
    lines = text.split('\n')
    a = next((i for i, l in enumerate(lines) if l.strip() == 'identity begin (seven documents)'), None)
    if a is None:
        return None
    b = next((i for i in range(a + 1, len(lines)) if lines[i].strip() == 'identity end'), None)
    if b is None:
        return None
    return [l.rstrip() for l in lines[a + 1:b]]


def check_identity_rows(c, lines, label):
    """Per-file rows of an identity table: the seven candidate documents, each equal to its live sha256."""
    rows = [m.groups() for m in (ROW_RE.match(l) for l in lines) if m]
    paths = {r[0] for r in rows}
    need(paths == set(SEVEN_DOCS), '%s identity rows differ from the seven candidate documents: %s' % (
        label, sorted(set(SEVEN_DOCS) ^ paths)))
    for path, _, hexv in rows:
        need(os.path.isfile(c.path(path)) and file_sha(c.path(path)) == hexv,
             '%s identity row for %s differs from the live file' % (label, path))


def check_identity_block(c, text, label):
    """D39: markers, a block equal to green/identity-c02.txt, per-file rows equal to the live files."""
    blk = identity_block(text)
    need(blk is not None, '%s lacks the identity markers (identity begin (seven documents) / identity end)' % label)
    ref = identity_block(evid_text(c, 'green', 'identity-c02.txt'))
    need(ref is not None, 'green/identity-c02.txt lacks the identity markers')
    need(blk == ref, '%s identity block differs from green/identity-c02.txt' % label)
    check_identity_rows(c, blk, label)


def last_line(text):
    ls = [l for l in text.split('\n') if l.strip()]
    return ls[-1].rstrip() if ls else ''


def parse_tsv(text):
    lines = text.split('\n')
    head = [h.strip().lower() for h in lines[0].split('\t')]
    rows = {}
    for line in lines[1:]:
        if line.strip():
            rec = dict(zip(head, line.split('\t')))
            rows[rec.get('mutant id', '').strip()] = rec
    return head, rows


def is_control(k):
    return re.search(r'-c\d+$', k) is not None


def is_closing(rec):
    src = rec.get('source row', '').strip()
    return src.startswith('AU-') or src in ('N9', 'N16', 'N17')


def log_pass_set(text):
    return set(re.findall(r'^PASS (CHK-\S+)', text, re.M))


def split_tests(s):
    return [t.strip() for t in re.split(r'[;,]', s) if t.strip()]


def baseline_pass(c, name):
    """(text, PASS ids) of a baseline log: exit 0 and zero FAIL lines are required."""
    text = evid_text(c, 'probes', 'raw', name)
    need(re.search(r'^exit code of the command: 0\s*$', text, re.M) is not None,
         'probes/raw/%s lacks "exit code of the command: 0"' % name)
    need(not re.search(r'^FAIL\b', text, re.M), 'probes/raw/%s holds a FAIL line' % name)
    return log_pass_set(text)


# ================================================================ amended C01 audit checks (D6)

@check('CHK-C01-sql-evidence-recorded')
def _c01_sql_recorded(c):
    text = evid_text(c, 'green', 'sql-final.txt')
    lines = text.split('\n')
    need(any(re.match(r'^sqlite3 version: \d+\.\d+', l) for l in lines), 'sqlite3 version not recorded')
    for pat, msg in [
            (r'^PRAGMA foreign_keys after ON in session: 1\s*$', 'PRAGMA foreign_keys 1 is not recorded'),
            (r'^tables: 24\s*$', 'the 24-table count is not recorded'),
            (r'^strict tables: 24\s*$', 'the 24 STRICT tables are not recorded'),
            (r'^triggers: 43\s*$', 'the 43-trigger count is not recorded'),
            (r'^explicit indexes: records_digest, run_state_one_active\s*$',
             'the two explicit indexes are not recorded'),
            (r'^foreign_key_check rows: 0\s*$', 'a clean foreign_key_check is not recorded'),
            (r'^probe statements: 25\s*$', 'the 25 probe statements are not counted')]:
        need(any(re.match(pat, l) for l in lines), msg)
    stm = {}
    for l in lines:
        m = re.match(r'^\[(\d+)\] expect (accept|refuse) \| .* \| (accepted|refused)\b', l)
        if m:
            stm[int(m.group(1))] = (m.group(2), m.group(3))
    need(sorted(stm) == list(range(1, 26)), 'statements [1] to [25] are not all recorded: %s' % sorted(stm))
    for n, (exp, got) in sorted(stm.items()):
        need({'accept': 'accepted', 'refuse': 'refused'}[exp] == got,
             'statement [%d] expected %s but the log records %s' % (n, exp, got))
    check_identity_block(c, text, 'green/sql-final.txt')
    return 'sql-final.txt records versions, counts, foreign_key_check, 25 of 25 statements and the identity block'


@check('CHK-C01-green-evidence')
def _c01_green(c):
    text = evid_text(c, 'green', 'checks.txt')
    lines = text.split('\n')
    core = len([i for i in ORDER if i not in AUDIT_IDS])
    need(any('ex_checks-c02.py' in l and '--repo' in l for l in lines),
         'green/checks.txt does not record the checker command (ex_checks-c02.py --repo)')
    need(any(l.strip() == 'audit checks not run: %d (use --audit)' % len(AUDIT_IDS) for l in lines),
         'the audit-not-run line is missing from green/checks.txt')
    passes = [l for l in lines if l.startswith('PASS CHK-')]
    fails = [l for l in lines if l.startswith('FAIL ')]
    need(len(passes) == core, 'green run PASS count %d differs from %d core checks' % (len(passes), core))
    need(not fails, 'green run has %d FAIL lines' % len(fails))
    need(EXIT0_RE.search(text) is not None, 'green run exit code 0 is not recorded')
    m = re.search(r'^checker sha256: ([0-9a-f]{64})\s*$', text, re.M)
    need(m is not None, 'green/checks.txt lacks the "checker sha256:" line')
    cp = c.evid('green', 'ex_checks-c02.py')
    need(os.path.isfile(cp), 'missing file: green/ex_checks-c02.py')
    need(m.group(1) == file_sha(cp), 'checker sha256 in green/checks.txt differs from green/ex_checks-c02.py')
    check_identity_block(c, text, 'green/checks.txt')
    need(last_line(text) == MASK_LINE, 'green/checks.txt does not end with the mask line')
    return 'green/checks.txt: default-mode run of the C02 checker, exit 0, PASS equals the core count, checker and identity tie'


@check('CHK-C01-prober-evidence')
def _c01_prober(c):
    pend_closing = 'closing' in c.pending
    head, rows = parse_tsv(evid_text(c, 'probes', 'mutants.tsv'))
    for h in ('mutant id', 'source row', 'test', 'verdict', 'mutant'):
        need(h in head, 'mutants.tsv header lacks %s' % h)
    active = {k: r for k, r in rows.items() if not (pend_closing and is_closing(r))}
    for gid in GUARD_IDS + N_MAIN_IDS:
        need(any(k.startswith(gid + '-m') for k in rows), 'mutants.tsv lacks row %s' % gid)
    need('C35-c1' in rows, 'mutants.tsv lacks the control C35-c1')
    if not pend_closing:
        for gid in CLOSING_IDS:
            need(any(k.startswith(gid + '-m') for k in rows), 'mutants.tsv lacks closing row %s' % gid)
    bd = baseline_pass(c, 'baseline-c02-default.log')
    ba = set()
    if not pend_closing:
        ba = baseline_pass(c, 'baseline-c02-audit.log')
        if os.path.isfile(c.evid('probes', 'raw', 'baseline-c02-audit-closed.log')):
            ba |= baseline_pass(c, 'baseline-c02-audit-closed.log')
    ctl_pass = {}
    for k, r in rows.items():
        if is_control(k):
            lp = c.evid('probes', 'raw', k + '.log')
            if os.path.isfile(lp):
                with open(lp, encoding='utf-8') as fh:
                    ctl_pass.setdefault(r.get('source row', '').strip(), set()).update(log_pass_set(fh.read()))
    for k, rec in active.items():
        verdict = rec.get('verdict', '').strip()
        test = rec.get('test', '').strip()
        if is_control(k):
            need(verdict == 'control-ok', 'control %s has verdict %r, not control-ok' % (k, verdict))
        else:
            need(verdict in ('KILLED', 'SURVIVED', 'not run'), 'mutant %s has no verdict' % k)
        raw = c.evid('probes', 'raw', k + '.log')
        need(os.path.isfile(raw) and os.path.getsize(raw) > 0, 'probes/raw/%s.log is missing or empty' % k)
        with open(raw, encoding='utf-8') as fh:
            log = fh.read()
        need(not log.startswith('PENDING') and log.strip() != 'PENDING', 'probes/raw/%s.log is a PENDING log' % k)
        if is_control(k):
            need('control: yes' in log.split('\n'), 'control log %s lacks the line "control: yes"' % k)
            need(re.search(r'^exit code of the command: 0\s*$', log, re.M) is not None,
                 'control log %s lacks "exit code of the command: 0"' % k)
            need(not re.search(r'^FAIL\b', log, re.M), 'control log %s holds a FAIL line' % k)
            need(any(t in log_pass_set(log) for t in split_tests(test)),
                 'control log %s lacks a PASS line for %s' % (k, test))
            continue
        if verdict == 'not run':
            continue
        ll = log.split('\n')
        need('scratch differs from pristine: yes' in ll,
             'probes/raw/%s.log lacks the line "scratch differs from pristine: yes"' % k)
        after = ll[ll.index('scratch differs from pristine: yes') + 1:]
        need([l for l in after if l.strip() and l != MASK_LINE], 'probes/raw/%s.log holds no applied diff' % k)
        if verdict == 'KILLED':
            m = re.search(r'^exit code of the command: (\d+)\s*$', log, re.M)
            need(m is not None and int(m.group(1)) != 0, 'killed mutant %s has no non-zero command exit code' % k)
            named = [t for t in split_tests(test) if any(l.startswith('FAIL') and t in l for l in ll)]
            need(named, 'killed mutant %s has no FAIL line naming its test' % k)
            ok = bd | ctl_pass.get(rec.get('source row', '').strip(), set())
            if is_closing(rec):
                ok = ba | ctl_pass.get(rec.get('source row', '').strip(), set())
            for t in named:
                need(t in ok, 'killed mutant %s: test %s is not a PASS line of its baseline' % (k, t))
    probes = evid_text(c, 'probes', 'PROBES.md')
    for k in active:
        need(re.search(r'(?<![\w-])%s(?![\w-])' % re.escape(k), probes) is not None,
             'PROBES.md lacks a line for %s' % k)
    main = [r for k, r in rows.items() if not is_control(k) and not is_closing(r)]
    cnt = {v: len([r for r in main if r.get('verdict', '').strip() == v]) for v in ('KILLED', 'SURVIVED', 'not run')}
    tm = re.search(r'main section totals:\s*killed (\d+),\s*survived (\d+),\s*not run (\d+)', probes)
    need(tm is not None, 'PROBES.md lacks the "main section totals" line')
    need((int(tm.group(1)), int(tm.group(2)), int(tm.group(3))) == (cnt['KILLED'], cnt['SURVIVED'], cnt['not run']),
         'PROBES.md totals (%s, %s, %s) differ from mutants.tsv (%d, %d, %d)' % (
             tm.group(1), tm.group(2), tm.group(3), cnt['KILLED'], cnt['SURVIVED'], cnt['not run']))
    surv = heading_block(probes, lambda t: t.lower().startswith('survivors'))
    need(surv is not None, 'PROBES.md has no Survivors section')
    for f in sorted(glob.glob(c.evid('probes', 'raw', '*.survived-1.log'))):
        k = os.path.basename(f)[:-len('.survived-1.log')]
        if pend_closing and k in rows and is_closing(rows[k]):
            continue
        need(re.search(r'(?<![\w-])%s(?![\w-])' % re.escape(k), surv) is not None,
             'the Survivors section does not name %s' % k)
    if not pend_closing:
        need(re.search(r'^#+\s.*closing probes', probes, re.I | re.M) is not None,
             'PROBES.md has no closing probes section')
        for f in sorted(glob.glob(c.evid('probes', 'raw', '*.log'))):
            with open(f, encoding='utf-8') as fh:
                first = fh.readline().strip()
            mm = re.match(r'^mutant id: (\S+)$', first)
            if mm:
                need(mm.group(1) in rows, 'probes/raw/%s has no row in mutants.tsv' % os.path.basename(f))
        for f in sorted(glob.glob(c.evid('probes', 'raw', '*.log'))):
            with open(f, encoding='utf-8') as fh:
                body = fh.read()
            need(not body.startswith('PENDING') and body.strip() != 'PENDING',
                 'PENDING log: probes/raw/%s' % os.path.basename(f))
    return 'probes/mutants.tsv covers every row; logs, baselines, controls and totals agree'


def closing_section(text):
    idx_c = text.lower().find('closing probes')
    return text[idx_c:] if idx_c >= 0 else None


@check('CHK-C01-record-closure')
def _c01_closure(c):
    rec = c.read(c.path(ROUND_REL))
    sec = c01_section(rec)
    need(sec is not None, 'the record has no C01 closure section')
    rows = {}
    for t in tables(sec):
        for raw in t['raw']:
            m = re.fullmatch(r'R(\d{1,2})', clean(raw[0])) if raw else None
            if m:
                rows.setdefault(int(m.group(1)), ' '.join(raw))
    need(sorted(rows) == list(range(1, 13)), 'closure table lacks one row each for R1 to R12: %s' % sorted(rows))
    green2 = [e[len(E_) + 1:] for e in C02_GREEN2_ONLY]
    for n in range(1, 13):
        text = rows[n]
        if n == 6:
            files = re.findall(r'((?:red|green|probes)/[\w.-]+\.(?:txt|md|tsv|py))', text)
            need(files, 'R6 row cites no evidence file')
            for f in files:
                need(f in green2 or os.path.isfile(c.evid(f)), 'R6 row cites a missing evidence file: %s' % f)
            continue
        ok = False
        for name, line_s in re.findall(r'([\w./-]+\.(?:md|py|txt|tsv|cpp|cppm|lua))`?:(\d+)', text):
            p = resolve_repo_file(c, name)
            if p is None:
                continue
            lines = c.read(p).split('\n')
            line = int(line_s)
            if 1 <= line <= len(lines):
                window = ' '.join(lines[max(0, line - 4):min(len(lines), line + 3)]).lower()
                if CLOSURE_SUBJECTS[n].lower() in window:
                    ok = True
                    break
        need(ok, 'R%d row cites no file:line whose text within three lines contains %r' % (n, CLOSURE_SUBJECTS[n]))
    need('green/audit-final.txt' in rec, 'the record does not cite green/audit-final.txt')
    return 'closure table R1 to R12 with cites; the record cites green/audit-final.txt'


@check('CHK-C01-red-evidence')
def _c01_red(c):
    red = evid_text(c, 'red', 'red-run.txt')
    base = evid_text(c, 'raw', 'baseline-identity.txt')
    need('ex_checks.py' in red and '--repo' in red, 'red-run.txt lacks the exact command')
    red_lines = set(red.split('\n'))
    missing = [l for l in base.split('\n') if l.startswith('|') and l not in red_lines]
    need(not missing, 'identity.py output is not pasted verbatim in red-run.txt: %s' % missing[:1])
    need(not [l for l in red.split('\n') if l.startswith('|') and ('docs/vernacular/rounds/' in l or 'roadmap.md' in l)],
         'identity output in red-run.txt has a round-record or roadmap row')
    fails = fail_ids(red)
    expect = set(RED_ROWS.values())
    need(fails == expect, 'red FAIL set differs: extra %s, missing %s' % (sorted(fails - expect), sorted(expect - fails)))
    need(not re.search(r'document missing|internal error|Traceback', red), 'red-run.txt has a non-assertion failure')
    audit = evid_text(c, 'red', 'red-audit-run.txt')
    afails = fail_ids(audit)
    need(afails & C01_AUDIT_IDS == C01_AUDIT_IDS,
         'red-audit-run.txt does not FAIL every audit id: %s' % sorted(C01_AUDIT_IDS - afails))
    need(afails - C01_AUDIT_IDS == fails, 'red-audit-run.txt core FAIL set differs from red-run.txt')
    orig = evid_text(c, 'raw', 'orig-checker-run.txt')
    need(re.search(r'^# exit code of the command: 1\s*$', orig, re.M) is not None,
         'orig-checker-run.txt does not record exit code 1')
    return 'red-run and red-audit-run carry the expected FAIL sets with assertion messages'


@check('CHK-C01-extractor-comments')
def _c01_extractor(c):
    sql = sql_block(c.doc())
    marker = 'CREATE TABLE evidence ('
    need(sql.count(marker) == 1, 'the CREATE TABLE evidence line is not unique in the SQL block')
    extra = sql.replace(marker, '-- Second leading comment line before evidence (C35 probe)\n' + marker, 1)
    need(sql_table_names(extra) == TABLES,
         'a second leading comment line hides a table: %s' % sql_table_names(extra)[:4])
    gone = sql.replace(marker, '', 1)
    need(sql_table_names(gone) != TABLES, 'deleting the CREATE TABLE evidence line is not detected')
    return 'the extractor keeps 24 tables past comment lines and detects a deleted table'


@check('CHK-C01-record-d81')
def _c01_d81(c):
    rec = c.read(c.path(ROUND_REL))
    sec = c01_section(rec)
    need(sec is not None, 'the record has no C01 closure section')
    d81 = heading_block(rec, lambda t: t.lower().startswith('d81 resolution'))
    need(d81 is not None, 'the record has no D81 resolution subsection')
    flat81 = norm(d81)
    mk = re.search(r'killed (\d+)', flat81)
    ms = re.search(r'survived (\d+)', flat81)
    need(mk and ms, 'the D81 resolution lacks killed and survived counts')
    probes = evid_text(c, 'probes', 'PROBES.md')
    tot = re.search(r'main section totals:\s*killed (\d+),\s*survived (\d+)', probes)
    need(tot is not None, 'PROBES.md lacks the main section totals line')
    need(mk.group(1) == tot.group(1) and ms.group(1) == tot.group(2),
         'D81 counts (%s killed, %s survived) differ from PROBES.md totals' % (mk.group(1), ms.group(1)))
    need('probes/PROBES.md' in flat81, 'the D81 resolution does not name probes/PROBES.md')
    need('PLACEHOLDER' not in d81, 'PLACEHOLDER marker remains in the D81 resolution')
    need('not RUN, open' not in flat81, 'the D81 resolution still says "not RUN, open"')
    need('This record does not choose' not in flat81, 'the D81 resolution still says "This record does not choose"')
    return 'record D81 resolution matches PROBES.md totals and names probes/PROBES.md'


# ================================================================ C02 checks (appended after the CHK-C01 ids, D7)

C02_NEW_IDS = [
    'CHK-C02-op2-history-tag', 'CHK-C02-contract-one-line', 'CHK-C02-record-status',
    'CHK-C02-record-section-c', 'CHK-C02-record-post-c01-ac', 'CHK-C02-record-r8', 'CHK-C02-red-md',
    'CHK-C02-raw-preserved', 'CHK-C02-evidence-logs', 'CHK-C02-snapshot-baseline',
    'CHK-C02-record-c02-ac', 'CHK-C02-record-owned-files', 'CHK-C02-record-reviewer-control',
    'CHK-C02-record-d6-labels', 'CHK-C02-red-logs', 'CHK-C02-identity-block', 'CHK-C02-logs-masked']
EXACT_OP2_TAG = 'operator-decided (delegated 2026-10-31, option A)'
STATUS_WORDS = ('met', 'partly met', 'not met')
RED_DEFAULT_EXPECT = {
    'CHK-C02-op2-history-tag', 'CHK-C02-contract-one-line', 'CHK-C02-record-status',
    'CHK-C02-record-section-c', 'CHK-C02-record-post-c01-ac', 'CHK-C02-record-r8', 'CHK-C02-red-md',
    'CHK-C02-record-c02-ac', 'CHK-C02-record-owned-files', 'CHK-C02-record-reviewer-control',
    'CHK-C02-record-d6-labels', 'CHK-C02-red-logs'}
RED_AUDIT_EXTRA = {
    'CHK-C01-sql-evidence-recorded', 'CHK-C01-green-evidence', 'CHK-C01-prober-evidence',
    'CHK-C01-record-closure', 'CHK-C01-record-d81', 'CHK-C02-evidence-logs', 'CHK-C02-logs-masked'}
ABSENCE_RE = re.compile(r'document missing|internal error|Traceback|missing file:|file missing:|No such file')
HOST_PATH_RE = re.compile(r'/Users/|/home/|/private/|/var/folders/|/tmp/')


def rec_text(c):
    return c.read(c.path(ROUND_REL))


def rec_sub(c, pred, what):
    blk = heading_block(rec_text(c), pred)
    need(blk is not None, 'the record has no %s subsection' % what)
    return blk


def e_short(p):
    return p[len(E_) + 1:] if p.startswith(E_ + '/') else p


def is_green2_only(p):
    s = e_short(p)
    return s in ('green/audit-final.txt', 'green/audit-verify-c02.txt', 'green/redaction-final-c02.txt') \
        or s == 'green/superseded-2' or s.startswith('green/superseded-2/')


def ac_rows(blk, ids, what):
    """Rows of the first (AC | criterion | status | basis) table of blk keyed by their first cell."""
    t = find_table(tables(blk), 'status', 'basis')
    need(t is not None, '%s has no table with status and basis columns' % what)
    got = {}
    for r in t['rows']:
        if r:
            got[r[0]] = r
    for i in ids:
        need(i in got, '%s lacks the row for %s' % (what, i))
        st, basis = cell(got[i], 2), cell(got[i], 3)
        need(st in STATUS_WORDS, '%s row %s status %r is not one of met, partly met, not met' % (what, i, st))
        need(basis.strip() != '', '%s row %s has an empty basis' % (what, i))
    return got


@check('CHK-C02-op2-history-tag')
def _c02_op2_tag(c):
    lines = [l for l in c.doc().split('\n') if l.startswith('`history --request-id` on format3.')]
    need(len(lines) == 1, 'the history --request-id format3 line is not unique or is missing (%d)' % len(lines))
    need(EXACT_OP2_TAG in lines[0], 'the OP2 line lacks the exact tag %s' % EXACT_OP2_TAG)
    need('(operator-decided, delegated' not in lines[0], 'the OP2 line still has the nested parenthesis tag')
    return 'the OP2 line carries the exact delegated tag'


@check('CHK-C02-contract-one-line')
def _c02_one_line(c):
    pre = c.read(c.evid('raw', 'pre-c02-contract.md')).split('\n')
    cur = c.doc().split('\n')
    need(len(pre) == len(cur), 'contract line count %d differs from the snapshot %d' % (len(cur), len(pre)))
    diff = [i + 1 for i, (a, b) in enumerate(zip(pre, cur)) if a != b]
    need(diff == [520], 'contract lines differing from the snapshot are %s, expected [520]' % diff)
    return 'the contract differs from the C01 snapshot at line 520 only'


@check('CHK-C02-record-status')
def _c02_status(c):
    rec = rec_text(c)
    lines = rec.split('\n')
    l3 = lines[2] if len(lines) > 2 else ''
    low = l3.lower()
    need('c02' in low, 'record line 3 does not mention C02')
    need('post-c01 status' in low, 'record line 3 does not name the post-C01 status')
    for ph in ('D81 is neither run nor withdrawn', 'corrective V5-02-R15-A1-C02 next',
               'no final-candidate evidence log exists'):
        need(ph.lower() not in low, 'record line 3 still says %r' % ph)

    def note_ok(i, rng, what):
        for j in rng:
            if 0 <= j < len(lines):
                t = lines[j].lstrip()
                if t.startswith('Superseded in C02:') and 'probes/PROBES.md' in t and ('Final results (C02)' in t or 'Final results (C03)' in t):
                    return True
        need(False, 'no "Superseded in C02:" note naming probes/PROBES.md and Final results (C02) %s' % what)

    for prefix in ('- Mutation prober and closing probes C31-m1', '- **Mutation prober: not run.**'):
        idxs = [i for i, l in enumerate(lines) if l.startswith(prefix)]
        need(len(idxs) == 1, 'bullet not found exactly once: %s' % prefix)
        i = idxs[0]
        if prefix.startswith('- Mutation'):
            need('not run, no verdict' in lines[i], 'the original text "not run, no verdict" was rewritten')
        note_ok(i, range(i + 1, i + 4), 'within three lines after the bullet "%s"' % prefix)
    hdr = [i for i, l in enumerate(lines) if l.lstrip().startswith('|') and 'probe outcome (probes/PROBES.md)' in l]
    need(hdr, 'the C01 invariant table header (probe outcome (probes/PROBES.md)) is missing')
    note_ok(hdr[0], range(hdr[0] - 3, hdr[0]), 'within three lines before the invariant table header')
    return 'record line 3 states the C02 status; the three C01 passages carry superseding notes'


@check('CHK-C02-record-section-c')
def _c02_section_c(c):
    blk = rec_sub(c, lambda t: t.startswith('(c) Operator choices'), 'section (c)')
    f = norm(blk.replace('`', ''))
    for ph in ('does not record an operator decision', 'confirms each one at commit review'):
        need(ph not in f, 'section (c) still says %r' % ph)
    for ph in ('recorded as delegated, not as live answers', 'the operator may overrule them'):
        need(ph in f, 'section (c) lacks %r' % ph)
    return 'section (c) uses the contract preamble wording'


@check('CHK-C02-record-post-c01-ac')
def _c02_post_c01(c):
    blk = rec_sub(c, lambda t: 'post-c01 status' in t.lower(), 'Post-C01 status')
    got = ac_rows(blk, [str(n) for n in range(1, 11)], 'Post-C01 status')
    r3 = ' '.join(got['3'])
    need('HALT-1' in r3 and 'amended' in r3.lower(), 'Post-C01 status row 3 does not state the HALT-1 amendment')
    return 'Post-C01 status holds criteria 1 to 10 with a status and a basis; row 3 states the HALT-1 amendment'


@check('CHK-C02-record-r8')
def _c02_r8(c):
    rec = rec_text(c)
    need('are not reconciled here' not in rec, 'the record still says "are not reconciled here"')
    # Paragraphs and bullets only: table rows (the C01 review disposition) are not an explanation.
    paras = []
    for p in re.split(r'\n\s*\n', rec):
        prose = '\n'.join(l for l in p.split('\n') if not l.lstrip().startswith('|'))
        paras.extend(x for x in re.split(r'\n(?=- )', prose) if x.strip())
    ok = False
    for p in paras:
        if 'orig-checker-run.txt' in p and re.search(r'\b15\b', p) and re.search(r'\b13\b', p) \
                and 'I45' in p and 'I59' in p and re.search(r'\bgrown\b|\bgrew\b', p, re.I):
            ok = True
            break
    need(ok, 'no paragraph or bullet explains raw/orig-checker-run.txt (15 FAIL against 13, I45 and I59 ran on a grown tree)')
    return 'the record explains 15 FAIL lines against 13 (I45 and I59)'


@check('CHK-C02-red-md')
def _c02_red_md(c):
    red = evid_text(c, 'red', 'RED.md')
    for t in ('red-run.txt', 'red-audit-run.txt', 'red-c02-run.txt', 'red-c02-audit-run.txt',
              'raw/red-checks', 'ex_checks.py --repo', 'irrelevant first attempt'):
        need(t in red, 'red/RED.md lacks %r' % t)
    for name in sorted(RED_ROWS.values()):
        need(name in red, 'red/RED.md lacks the failing check name %s' % name)
    rl = evid_text(c, 'red', 'red-run.txt').split('\n')
    redl = set(red.split('\n'))
    for l in rl:
        if l.startswith('|'):
            need(l in redl, 'red/RED.md lacks an identity line of red-run.txt: %s' % l[:60])
    ptr = rec_sub(c, lambda t: t.lower().startswith('evidence by pointer'), 'Evidence by pointer')
    for t in ('red/red-run.txt', 'red/red-audit-run.txt', 'raw/red-checks', 'red/red-c02-run.txt',
              'red/red-c02-audit-run.txt', 'irrelevant first attempt'):
        need(t in ptr, 'the record block Evidence by pointer lacks %r' % t)
    readme = evid_text(c, 'README.md')
    row = [l for l in readme.split('\n') if l.startswith('| `red/` ')]
    need(len(row) == 1, 'README.md has no single path-table row for red/')
    for t in ('red-run.txt', 'red-audit-run.txt', 'raw/red-checks', 'red-c02-run.txt',
              'red-c02-audit-run.txt', 'irrelevant first attempt'):
        need(t in row[0], 'README.md red/ row lacks %r' % t)
    need('per-check logs' not in row[0], 'README.md red/ row still says "per-check logs"')
    need('Nothing here is committed' not in readme, 'README.md still says "Nothing here is committed"')
    return 'RED.md, the record pointer block and the README row describe the real red and the first attempt'


@check('CHK-C02-raw-preserved')
def _c02_raw(c):
    man = evid_text(c, 'raw', 'pre-c02-manifest.txt').split('\n')
    need('# rewritten-by-c02' in man, 'the manifest lacks the "# rewritten-by-c02" section')
    k = man.index('# rewritten-by-c02')
    n = 0
    allow = set(l.strip() for l in man[k + 1:] if l.strip())
    for line in man[:k]:
        if not line.strip():
            continue
        parts = line.split('\t')
        need(len(parts) == 3, 'manifest line has %d columns: %s' % (len(parts), line[:80]))
        rel, size, h = parts
        p = c.path(rel)
        need(os.path.isfile(p), 'raw evidence file deleted: %s' % rel)
        if rel in allow:
            n += 1
            continue
        data = open(p, 'rb').read()
        need(len(data) == int(size), 'raw evidence file changed size: %s' % rel)
        if h == '-':
            fz = os.path.join(c.frozen_dir, 'reports', rel.split('/')[2] + '__' + os.path.basename(rel))
            need(same_bytes(p, fz), 'REPORT file differs from its frozen copy: %s' % rel)
        else:
            need(hashlib.sha256(b'planar-c02-manifest\0' + data).hexdigest() == h,
                 'raw evidence file edited: %s' % rel)
        n += 1
    need(n > 0, 'the manifest lists no file')
    for line in man[k + 1:]:
        if line.strip():
            need(os.path.exists(c.path(line.strip())), 'rewritten file missing: %s' % line.strip())
    return '%d manifest files exist unchanged; allowlisted files exist' % n


def staging_paths(c):
    blk = rec_sub(c, lambda t: t.startswith('Staging list'), 'Staging list')
    out = []
    for tok in re.findall(r'`([^`\n]+)`', blk):
        if '/' in tok and ' ' not in tok and not re.search(r'[*<>]', tok) and tok not in out:
            out.append(tok)
    # C04: a bullet whose whole content is one slashless token (CLAUDE.md) is a staged path too.
    for tok in re.findall(r'^\s*[-*]\s+`([^`\n/ ]+)`\s*$', blk, re.M):
        if not re.search(r'[*<>]', tok) and tok not in out:
            out.append(tok)
    return blk, out


@check('CHK-C02-evidence-logs')
def _c02_evidence_logs(c):
    # Clause 1: staging list.
    blk, paths = staging_paths(c)
    need(paths, 'the Staging list holds no backticked path')
    for p in paths:
        need(os.path.basename(p) != 'inventory.txt', 'the Staging list names inventory.txt')
        need(not re.search(r'plan/superseded-[^/]*/INVARIANTS\.md$', p),
             'the Staging list names a superseded INVARIANTS.md: %s' % p)
        need('raw' not in p.split('/'), 'the Staging list names a path under raw/: %s' % p)
        if not is_green2_only(p):
            need(os.path.exists(c.path(p)), 'a Staging list path does not exist: %s' % p)
    # Clause 2: redaction-c02.txt.
    red = evid_text(c, 'green', 'redaction-c02.txt')
    ok0 = set(m.group(1) for m in re.finditer(r'^exit 0 (\S+)', red, re.M))
    for p in paths:
        if is_green2_only(p) or e_short(p) == 'green/redaction-c02.txt' or os.path.isdir(c.path(p)):
            continue
        need(p in ok0 or e_short(p) in ok0, 'green/redaction-c02.txt lacks "exit 0 %s"' % p)
    need(re.search(r'^exit [1-9]\d*\b.*positive control', red, re.M) is not None,
         'green/redaction-c02.txt lacks the positive control line (non-zero exit)')
    need(last_line(red) == MASK_LINE, 'green/redaction-c02.txt does not end with the mask line')
    # Clause 3: link check.
    lk = evid_text(c, 'green', 'linkcheck-c02.txt')
    files11 = SEVEN_DOCS + [ROUND_REL, E_ + '/README.md', E_ + '/red/RED.md', E_ + '/probes/PROBES.md']
    cmd = [l for l in lk.split('\n') if 'linkcheck' in l and 'files checked' not in l]
    need(cmd, 'green/linkcheck-c02.txt records no command')
    for f in files11:
        need(any(f in l for l in cmd), 'green/linkcheck-c02.txt command does not name %s' % f)
    need('files checked: 11' in lk.split('\n'), 'green/linkcheck-c02.txt lacks "files checked: 11"')
    sums = re.findall(r'links checked: (\d+); ok: (\d+); broken: (\d+)', lk)
    need(any(int(a) > 0 and a == b and z == '0' for a, b, z in sums),
         'green/linkcheck-c02.txt lacks "links checked: X; ok: X; broken: 0" with X above zero')
    need(re.search(r'^negative control exit 1\b', lk, re.M) is not None and
         any(z == '1' for _, _, z in sums), 'green/linkcheck-c02.txt lacks the negative control')
    need(EXIT0_RE.search(lk) is not None, 'green/linkcheck-c02.txt does not record exit code 0')
    # Clause 4: identity.
    idt = evid_text(c, 'green', 'identity-c02.txt')
    blk_id = identity_block(idt)
    need(blk_id is not None, 'green/identity-c02.txt lacks the identity markers')
    check_identity_rows(c, blk_id, 'green/identity-c02.txt')
    after = idt.split('identity end', 1)[1]
    need(EMPTY_SHA in after, 'green/identity-c02.txt lacks the empty-input src tests hash')
    # Clause 5: cmp.
    cm = evid_text(c, 'green', 'cmp-c02.txt')
    for role in ('checker', 'sql script', 'link checker', 'mask script'):
        need(re.search(r'^cmp %s exit 0\b' % re.escape(role), cm, re.M) is not None,
             'green/cmp-c02.txt lacks "cmp %s exit 0"' % role)
    # Clause 6: owned files exist.
    for o in C02_OWNED:
        if not is_green2_only(o):
            need(os.path.exists(c.path(o)), 'owned file does not exist: %s' % o)
    # Clause 7: acceptance bases without pending.
    ac = rec_sub(c, lambda t: 'c02 acceptance status' in t.lower(), 'C02 acceptance status')
    if 'acceptance' not in c.pending:
        t = find_table(tables(ac), 'status', 'basis')
        need(t is not None, 'the C02 acceptance table is missing')
        for r in t['rows']:
            need('pending' not in cell(r, 3).lower(), 'C02 acceptance row %s basis still says pending' % cell(r, 0))
    # Clause 8: PASS count.
    fin = heading_block(rec_text(c), lambda t: t.startswith('Final results (C02)') or t.startswith('Final results (C03)'))
    need(fin is not None, 'the record has no Final results (C02) subsection')
    m = re.search(r'PASS count:\s*(\d+)', fin)
    need(m is not None, 'Final results (C02) lacks "PASS count: N"')
    npass = len(re.findall(r'^PASS CHK-', evid_text(c, 'green', 'checks.txt'), re.M))
    need(int(m.group(1)) == npass, 'Final results (C02) PASS count %s differs from green/checks.txt (%d)' % (m.group(1), npass))
    # Clause 9: no audit-final count in the record (C04: every line is judged, table rows and indented lines included).
    for l in rec_text(c).split('\n'):
        for mm in re.finditer('audit-final', l):
            win = l[max(0, mm.start() - 60):mm.end() + 60]
            need(not re.search(r'\bPASS\b|\bFAIL\b|\bexit\b|\d+\s+checks', win),
                 'the record quotes a result of audit-final: %s' % win.strip())
    # Clause 10: audit-final.txt when present.
    if os.path.isfile(c.evid('green', 'audit-final.txt')):
        af = evid_text(c, 'green', 'audit-final.txt')
        cl = [l for l in af.split('\n') if 'ex_checks-c02.py' in l]
        need(cl, 'green/audit-final.txt records no command')
        need(any('--audit' in l for l in cl), 'green/audit-final.txt command lacks --audit')
        need(not any('--pending' in l for l in cl), 'green/audit-final.txt command carries --pending')
        need(EXIT0_RE.search(af) is not None, 'green/audit-final.txt does not record exit code 0')
        need(not re.search(r'^FAIL\b', af, re.M), 'green/audit-final.txt holds a FAIL line')
        check_identity_block(c, af, 'green/audit-final.txt')
        need(last_line(af) == MASK_LINE, 'green/audit-final.txt does not end with the mask line')
    # Clause 11: redaction-final-c02.txt when present.
    if os.path.isfile(c.evid('green', 'redaction-final-c02.txt')):
        rf = evid_text(c, 'green', 'redaction-final-c02.txt')
        need(re.search(r'^exit 0 green/audit-final\.txt\b', rf, re.M) is not None and
             os.path.isfile(c.evid('green', 'audit-final.txt')),
             'green/redaction-final-c02.txt lacks "exit 0 green/audit-final.txt" or audit-final.txt is absent')
        if os.path.isfile(c.evid('green', 'audit-verify-c02.txt')):
            need(re.search(r'^exit 0 green/audit-verify-c02\.txt\b', rf, re.M) is not None,
                 'green/redaction-final-c02.txt lacks "exit 0 green/audit-verify-c02.txt"')
        need(last_line(rf) == MASK_LINE, 'green/redaction-final-c02.txt does not end with the mask line')
    return 'staging list, redaction, links, identity, cmp, owned files and final logs agree'


@check('CHK-C02-snapshot-baseline')
def _c02_snapshot(c):
    sha = file_sha(c.evid('raw', 'pre-c02-contract.md'))
    blk = rec_sub(c, lambda t: t.startswith('Final candidate identity'), 'Final candidate identity')
    need(any('execution-contract.md' in l and sha in l for l in blk.split('\n')),
         'no line of the Final candidate identity block names execution-contract.md with the snapshot sha256')
    return 'raw/pre-c02-contract.md is the C01 candidate named in the identity block'


@check('CHK-C02-record-c02-ac')
def _c02_c02_ac(c):
    blk = rec_sub(c, lambda t: 'c02 acceptance status' in t.lower(), 'C02 acceptance status')
    ac_rows(blk, [str(n) for n in range(1, 6)] + ['G%d' % n for n in range(1, 6)], 'C02 acceptance status')
    c01 = rec_sub(c, lambda t: t.startswith('Acceptance (C01)'), 'Acceptance (C01)')
    t = find_table(tables(c01), 'status', 'basis')
    need(t is not None, 'the Acceptance (C01) table is missing')
    r3 = [r for r in t['rows'] if r and r[0] == '3']
    need(r3, 'the Acceptance (C01) table lacks row 3')
    txt = norm(' '.join(r3[0]))
    need('D81 neither run nor withdrawn' not in txt, 'Acceptance (C01) row 3 still says "D81 neither run nor withdrawn"')
    need('C02' in txt, 'Acceptance (C01) row 3 does not mention C02')
    return 'C02 acceptance status holds rows 1 to 5 and G1 to G5; Acceptance (C01) row 3 points to C02'


@check('CHK-C02-record-owned-files')
def _c02_owned(c):
    blk = rec_sub(c, lambda t: t.startswith('Owned files (C02)'), 'Owned files (C02)')
    for o in C02_OWNED:
        need(o in blk, 'Owned files (C02) does not name %s' % o)
    return 'Owned files (C02) names all %d owned paths' % len(C02_OWNED)


@check('CHK-C02-record-reviewer-control')
def _c02_reviewer(c):
    rel = 'tests/native/v5-02-r15-a1-evidence/review-c01/redaction-c01-review.txt'
    rec = rec_text(c)
    blk = heading_block(rec, lambda t: t.startswith('Staging list')) or ''
    outside = rec.replace(blk, '') if blk else rec
    need(rel in outside, 'the record does not cite the reviewer redaction positive control outside the Staging list')
    need(os.path.isfile(c.path(rel)), 'the reviewer redaction file does not exist: %s' % rel)
    txt = c.read(c.path(rel))
    need(re.search(r'^exit [1-9]\d*\b.*positive control', txt, re.M) is not None,
         'the reviewer file holds no non-zero "exit" line for the positive control')
    return 'the record cites the reviewer positive control, which exits non-zero'


@check('CHK-C02-record-d6-labels')
def _c02_d6(c):
    blk = rec_sub(c, lambda t: t.startswith('Checker amendments (C02)'), 'Checker amendments (C02)')
    for n in ('ex_checks-c02.py', 'CHK-C01-sql-evidence-recorded', 'CHK-C01-green-evidence',
              'CHK-C01-prober-evidence', 'CHK-C01-record-closure', 'CHK-C01-record-d81',
              'CHK-C01-red-evidence', 'CHK-C01-names-preserved', 'CHK-I45-diff-scope'):
        need(n in blk, 'Checker amendments (C02) does not name %s' % n)
    return 'Checker amendments (C02) labels every amended check'


def check_red_log(c, name, expect, absence_ok, audit):
    text = evid_text(c, 'red', name)
    lines = text.split('\n')
    cl = [l for l in lines if 'ex_checks-c02.py --repo' in l]
    need(cl, 'red/%s lacks the exact command (ex_checks-c02.py --repo)' % name)
    need(any('--audit' in l for l in cl) == audit, 'red/%s command %s --audit' % (name, 'lacks' if audit else 'carries'))
    need(any(l.startswith('| HEAD |') for l in lines), 'red/%s lacks the identity.py output (HEAD row)' % name)
    rows = {m.group(1): m.group(3) for m in (ROW_RE.match(l) for l in lines) if m}
    need(set(rows) == set(SEVEN_DOCS), 'red/%s identity rows differ from the seven candidate documents' % name)
    pre = file_sha(c.evid('raw', 'pre-c02-contract.md'))
    need(rows[DOC_REL] == pre, 'red/%s contract row differs from the sha256 of raw/pre-c02-contract.md' % name)
    for d in SEVEN_DOCS:
        if d != DOC_REL:
            need(file_sha(c.path(d)) == rows[d], 'red/%s identity row for %s differs from the file' % (name, d))
    need(EMPTY_SHA in text, 'red/%s lacks the src tests identity (empty-input hash)' % name)
    need('Command exit code: 1' in lines, 'red/%s lacks "Command exit code: 1"' % name)
    fails = {}
    for l in lines:
        m = re.match(r'^FAIL (CHK-\S+) ?(.*)$', l)
        if m:
            fails[m.group(1)] = m.group(2)
    need(set(fails) == expect, 'red/%s FAIL set differs: extra %s, missing %s' % (
        name, sorted(set(fails) - expect), sorted(expect - set(fails))))
    for cid, detail in fails.items():
        if cid not in absence_ok:
            need(not ABSENCE_RE.search(detail), 'red/%s: %s fails by absence, not on an assertion' % (name, cid))
    need(not re.search(r'Traceback', text), 'red/%s holds a traceback' % name)
    need(last_line(text) == MASK_LINE, 'red/%s does not end with the mask line' % name)
    home = os.path.expanduser('~')
    need(c.repo not in text and (home in ('', '/') or home not in text) and not HOST_PATH_RE.search(text),
         'red/%s holds a host path' % name)


@check('CHK-C02-red-logs')
def _c02_red_logs(c):
    check_red_log(c, 'red-c02-run.txt', RED_DEFAULT_EXPECT, {'CHK-C02-red-logs'}, False)
    check_red_log(c, 'red-c02-audit-run.txt', RED_DEFAULT_EXPECT | RED_AUDIT_EXTRA,
                  RED_AUDIT_EXTRA | {'CHK-C02-red-logs'}, True)
    # D34: the record describes both logs in the paragraph 'C02 red evidence' (REC1 writes it).
    rec = rec_text(c)
    para = heading_block(rec, lambda t: t.startswith('C02 red evidence'))
    if para is None:
        m = re.search(r'^[#*\s]*C02 red evidence.*?(?=\n\s*\n|\Z)', rec, re.M | re.S)
        para = m.group(0) if m else None
    need(para is not None, "the record has no 'C02 red evidence' paragraph describing the C02 red logs")
    for n in ('red-c02-run.txt', 'red-c02-audit-run.txt'):
        need(n in para, "the record's 'C02 red evidence' paragraph does not name %s" % n)
    return 'the C02 red logs carry the expected FAIL sets on assertion messages and the record describes them'


@check('CHK-C02-identity-block')
def _c02_identity_block(c):
    blk = rec_sub(c, lambda t: t.startswith('Final candidate identity'), 'Final candidate identity')
    rows = []
    for l in blk.split('\n'):
        m = ROW_RE.match(l)
        if m and 'before C02' not in l:
            rows.append(m.groups())
    paths = {r[0] for r in rows}
    need(set(SEVEN_DOCS) <= paths, 'the identity block lacks a current row for: %s' % sorted(set(SEVEN_DOCS) - paths))
    for path, _, hexv in rows:
        need(os.path.isfile(c.path(path)) and file_sha(c.path(path)) == hexv,
             'identity block row for %s differs from the live file' % path)
    return 'every current per-file row of the Final candidate identity block equals the live sha256'


@check('CHK-C02-logs-masked')
def _c02_masked(c):
    logs = sorted(glob.glob(c.evid('probes', 'raw', '*.log')))
    need(logs, 'no probes/raw log exists')
    logs += sorted(glob.glob(c.evid('red', 'red-c02-*.txt')))
    logs += sorted(glob.glob(c.evid('green', '*-c02.txt')))
    for n in ('checks.txt', 'sql-final.txt', 'audit-final.txt'):
        if os.path.isfile(c.evid('green', n)):
            logs.append(c.evid('green', n))
    logs = sorted(set(logs))
    forbid = {}
    files = [c.path('docs/vernacular/roadmap.md')] + glob.glob(c.path('docs/vernacular/rounds/*.md'))
    files += glob.glob(c.path('tests/native/*-evidence/**/REPORT*'), recursive=True)
    for f in files:
        if os.path.isfile(f):
            forbid[file_sha(f)] = os.path.basename(f)
    home = os.path.expanduser('~')
    for f in logs:
        rel = os.path.relpath(f, c.evidence_dir)
        with open(f, encoding='utf-8') as fh:
            text = fh.read()
        need(last_line(text) == MASK_LINE, '%s does not end with the mask line' % rel)
        need(c.repo not in text and (home in ('', '/') or home not in text) and SCRIPT_DIR not in text
             and not HOST_PATH_RE.search(text), '%s holds a host path' % rel)
        hits = [forbid[t] for t in set(re.findall(r'\b[0-9a-f]{64}\b', text)) if t in forbid]
        need(not hits, '%s holds the sha256 of a forbidden file (%s)' % (rel, hits[0] if hits else ''))
    return '%d C02 logs end with the mask line and hold no host path or forbidden hash' % len(logs)


# ---------------------------------------------------------------- driver

def run(ctx, cid):
    fn = CHECKS[cid]
    try:
        detail = fn(ctx)
        return 'PASS', cid, str(detail or 'ok')
    except MissingDoc as e:
        return 'FAIL', cid, 'document missing: %s' % e.args[0]
    except CheckFail as e:
        return 'FAIL', cid, str(e)
    except KeyError as e:
        return 'FAIL', cid, 'SQL object missing from the contract: %s' % e.args[0]
    except Exception as e:  # reported as a FAIL, never a traceback
        return 'FAIL', cid, 'internal error: %s: %s' % (type(e).__name__, e)


def main(argv):
    ap = argparse.ArgumentParser(description='R15-A1 scratch checks')
    ap.add_argument('--repo', default=os.getcwd())
    ap.add_argument('--doc', default=None)
    ap.add_argument('--evidence-dir', default=None)
    ap.add_argument('--frozen-dir', default=None)
    ap.add_argument('--audit', action='store_true',
                    help='also run the nine audit checks (they audit files written by later packages)')
    ap.add_argument('--pending', action='append', choices=['closing', 'acceptance'], default=[],
                    help='audit mode only: skip the closing-row clauses (closing) or the pending-basis clause (acceptance)')
    ap.add_argument('ids', nargs='*')
    args = ap.parse_args(argv)
    if args.pending and not args.audit:
        ap.error('--pending is only valid with --audit')
    repo = os.path.abspath(args.repo)
    doc = os.path.abspath(args.doc) if args.doc else os.path.join(repo, DOC_REL)
    evidence = os.path.abspath(args.evidence_dir) if args.evidence_dir else os.path.join(repo, C01_EVID_REL)
    frozen = os.path.abspath(args.frozen_dir) if args.frozen_dir else os.path.join(SCRIPT_DIR, 'frozen')
    if args.ids:
        ids = list(args.ids)
    elif args.audit:
        ids = list(ORDER)
    else:
        ids = [i for i in ORDER if i not in AUDIT_IDS]
    unknown = [i for i in ids if i not in CHECKS]
    if unknown:
        print('FAIL %s unknown check id' % ' '.join(unknown))
        return 2
    failures = 0
    print('command: %s' % ' '.join([os.path.basename(sys.argv[0])] + list(argv)))
    for cid in ids:
        status, name, detail = run(Ctx(repo, doc, evidence, frozen, args.pending), cid)
        detail = ' '.join(detail.split())
        print('%s %s %s' % (status, name, detail))
        if status == 'FAIL':
            failures += 1
    if not args.ids and not args.audit:
        print('audit checks not run: %d (use --audit)' % len([i for i in ORDER if i in AUDIT_IDS]))
    return 1 if failures else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
