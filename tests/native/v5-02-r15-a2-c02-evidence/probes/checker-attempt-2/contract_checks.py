#!/usr/bin/env python3
"""Contract checker for round V5-02-R15-A2 (Part 2 of the execution contract).

@file contract_checks.py
@brief Literal-anchored and snapshot-diff checks of the R15-A2 contract text.

Entry points: this script (run as `python3 -I contract_checks.py`). Options:
--repo DIR (root holding docs/vernacular/*.md, default the working directory),
--base DIR (the frozen snapshot of ten files, default the sibling base/ folder),
--git-repo DIR (a git repository, used only for `git show`), --base-commit SHA
(used only by CHK-A2-133 and by the link check), --selftest (runs CHK-A2-000
only), --only NAME (runs one check by number or by name).

Output: one line per check, `PASS <name>` or `FAIL <name>: <reason>`, then
`SUMMARY pass=N fail=M`. Exit status 1 when any check fails.

Error boundary: every check runs under its own handler. A missing section,
literal or file is a FAIL with a reason, never a traceback. The checker reads
only docs/vernacular/*.md under --repo, files under --base and `git show`
output; it never reads the round record or evidence logs.

Literal policy: prose literals are matched after collapsing runs of whitespace
(a rewrap is not a change); table rows and help lines are matched exactly. The
literals are copied from DECISIONS.md section 4 and the INVARIANTS.md literal
column, generated into this file by a build step.
"""

import argparse
import difflib
import os
import re
import subprocess
import sys

EC = 'execution-contract.md'
CC = 'command-contract.md'
RC = 'render-contract.md'
LC = 'load-command-contract.md'
LG = 'ledger-contract.md'
XP = 'external-profile-contract.md'
ED = 'execution-design.md'
AU = 'authoring-contract.md'
AM = 'acceptance-matrix.md'
AL = 'v5-alignment.md'
TEN = [EC, CC, RC, LC, LG, XP, ED, AU, AM, AL]
RECORDED_BASE_COMMIT = 'cdcfe829c466bf6338815ff6f5b6ea90ffddc570'

P2_TITLE = '## Part 2: execution verbs, projections and reads (R15-A2)'
P2 = {
    1: '### P2-1 Verbs and services',
    2: '### P2-2 Dispatch projection and dispatch_manifest_v1',
    3: '### P2-3 work_exec_v1',
    4: '### P2-4 Provenance (driver_provenance_v1)',
    5: '### P2-5 History event selector and request results',
    6: '### P2-6 Orchestrator live form',
    7: '### P2-7 Command, render, load, ledger and external-profile pointers',
}
P2_ANCHORS = [
    '#part-2-execution-verbs-projections-and-reads-r15-a2',
    '#p2-1-verbs-and-services',
    '#p2-2-dispatch-projection-and-dispatch_manifest_v1',
    '#p2-3-work_exec_v1',
    '#p2-4-provenance-driver_provenance_v1',
    '#p2-5-history-event-selector-and-request-results',
    '#p2-6-orchestrator-live-form',
    '#p2-7-command-render-load-ledger-and-external-profile-pointers',
]
PART1_AM = '## Part 1 amendments (R15-A2)'
EIGHT = {'active_revision', 'active_job', 'fact_version', 'role_state',
         'generation_current', 'run_active', 'reservation_open', 'request_unused'}
EXCL_PREFIXES = ('## Part 2', '## Part 1 amendments', '## Messages',
                 '## Pointer amendments', '## Traceability',
                 '## Acceptance checklist', '## Decisions table')
ALLOW_RE = re.compile(r'part 2|parts 1 and 2|p2-[1-7]|r15-a2|a-\d{1,2}\b|run\.token', re.I)

# Decision tags of D104 to D152 (DECISIONS.md section 2, Tag column).
DELEGATED = 'operator-decided (delegated 2026-10-31, option A)'
DECISION_TAGS = {
    'D104': DELEGATED, 'D105': 'operator-decided', 'D106': 'operator-decided',
    'D107': 'new-engineering', 'D108': 'new-engineering', 'D109': 'new-engineering',
    'D110': 'new-engineering', 'D111': 'new-engineering', 'D112': 'new-engineering',
    'D113': 'new-engineering', 'D114': 'new-engineering', 'D115': 'new-engineering',
    'D116': 'new-engineering', 'D117': 'new-engineering', 'D118': 'operator-decided',
    'D119': 'new-engineering', 'D120': 'operator-decided', 'D121': 'new-engineering',
    'D122': 'operator-decided', 'D123': 'operator-decided', 'D124': 'new-engineering',
    'D125': 'contract-decided', 'D126': 'new-engineering', 'D127': 'contract-decided',
    'D128': 'new-engineering', 'D129': 'new-engineering', 'D130': 'contract-decided',
    'D131': 'contract-decided', 'D132': 'new-engineering', 'D133': 'new-engineering',
    'D134': DELEGATED, 'D135': 'new-engineering', 'D136': 'new-engineering',
    'D137': 'new-engineering', 'D138': 'contract-decided', 'D139': 'contract-decided',
    'D140': 'contract-decided', 'D141': 'contract-decided', 'D142': DELEGATED,
    'D143': 'new-engineering', 'D144': 'contract-decided', 'D145': 'new-engineering',
    'D146': 'new-engineering', 'D147': 'new-engineering', 'D148': 'operator-decided',
    'D149': 'new-engineering', 'D150': 'new-engineering', 'D151': 'new-engineering',
    'D152': 'new-engineering',
    'D153': 'new-engineering', 'D154': 'new-engineering', 'D155': 'new-engineering',
}

# Design choice and Basis cells of the three rows added by R15-A2-C01 (literal L15, C01-D23).
D153_CELL = ('Preview manifest: `run` is `{kind:"preview"}` with no `request` key; `version`, `revision`, `job`, `role`, '
             '`generation`, `versions`, `work`, `prerequisites`, `registry` and `inputs` are computed as for a dispatch, '
             'except that `generation` is the role\'s current generation ordinal (a preview never creates a generation); '
             '`render`, `sections` and `omitted` are computed from the preview packet\'s own bytes; the manifest is '
             'returned in `preview_v1` and never stored.')
D153_BASIS = ('design-decided (R15-7, R15-6); NEW-engineering for the preview run value and the preview generation '
              '(C01, review R4)')
D154_CELL = ('Preview run-section line is `run request: (preview)`; its characters are outside the request ID grammar '
             '`[A-Za-z0-9_.:-]{1,128}`, so it never equals a legal request ID.')
D154_BASIS = 'NEW-engineering (C01, review R4)'
D155_CELL = ('`dispatch_v1` names the dispatch projection rules of P2-2; the generation is the `generation` key of '
             '`dispatch_manifest_v1` and the `{generation}` of the authority statement, not a W field and not carried '
             'by any identifier.')
D155_BASIS = ('design-decided (render-contract identifier rule; execution-design EX-5 bullet, operator E5 A); '
              'NEW-engineering for the definition (C01, review R5)')
NEW_DECISION_CELLS = {
    'D153': (D153_CELL, D153_BASIS),
    'D154': (D154_CELL, D154_BASIS),
    'D155': (D155_CELL, D155_BASIS),
}

# Literals of the R15-A2-C01 contract text (DECISIONS.md section 2, L1 to L9, L2b, S).
L1_RUN_ROW = ('| `run` | `{request,kind}` for a dispatch: `request` is the claim or takeover request ID and `kind` is '
              '`claim` or `takeover` (takeover adds `previous`, the decimal string of the fenced run id); for `preview` '
              'the value is `{kind:"preview"}` with no `request` key |')
L2_PARA = ('A preview manifest has the same 14 keys as a dispatch manifest. Its keys `version`, `revision`, `job`, '
           '`role`, `generation`, `versions`, `work`, `prerequisites`, `registry` and `inputs` are computed as for a '
           'dispatch, except that `generation` is the role\'s current generation ordinal, the same value as `generation` '
           'in the `roles` entries of `status_v1`, and a preview never creates a generation. Its keys `render`, '
           '`sections` and `omitted` are computed from the preview packet\'s own bytes, which use the preview authority '
           'template and the `run request: (preview)` line. Its `run` is `{kind:"preview"}`. The manifest is returned '
           'in `preview_v1` and never stored.')
L2B = ('The `generation` of a preview is the role\'s current generation ordinal and a preview never creates a '
       'generation.')
L3_SENTENCE = ('The preview spelling `(preview)` contains characters outside the request ID grammar '
               '`[A-Za-z0-9_.:-]{1,128}`, so it never equals a legal request ID.')
L4_BULLET = ('- `dispatch_v1` is the identifier of the dispatch projection rules of this section: packet mode '
             '`dispatch`, the section order with `run` second, the two authority templates, the run-section bytes and '
             'the `dispatch_manifest_v1` recipe. A semantic change to any of them needs a new identifier. The identifier '
             'names these rules and carries no data.')
L5_SENTENCE = ('Generation is not a W field: the generation is the `generation` key of `dispatch_manifest_v1` and the '
               '`{generation}` of the authority statement, not a W field, so a takeover generation never moves W')
L6_SENTENCE = ('The generation is the `generation` key of `dispatch_manifest_v1` and the `{generation}` of the '
               'authority statement, not a W field')
L7_SENTENCE = ('while the generation is the `generation` key of `dispatch_manifest_v1` and the `{generation}` of the '
               'authority statement, not a W field')
L8_SENTENCE = ('the generation is the `generation` key of `dispatch_manifest_v1` and the `{generation}` of the '
               'authority statement, not a W field')
L8_IDENTIFIER = '`dispatch_v1` names the dispatch projection rules'
S_STATEMENT = ('EX-4 does not spell an absent job `none`, because `none` is a legal job ID')
L9_SENTENCE = ('in any packet, in logs or in error details. The claim and takeover results carry it only in the field '
               '`run.token`.')
L9_OLD_FRAGMENT = 'in any packet, and in the claim and takeover results only in the field'
LEAD_SENTENCE = ('The amendments are insertions only, except the reworded F2 bullet of item 8, and no approved clause '
                 'is narrowed.')

# Literal table rows and help lines, generated from DECISIONS.md section 4.
OPT_ROWS = [
    '| `activate` | mutation | `--ledger`, `--job`, `--expect-revision`, `--expect-job`, `--request-id`, `--actor` | none | `--expect-revision` is `ID_OR_NONE` as load spells it (a canonical positive decimal revision ID or `none`); `--expect-job` is `NONE` or a job ID (D104; its help metavariable is `JOB_OR_NONE`) | `activate_v1` |',
    '| `status` | read | `--ledger` | `--role`, `--request-id`, `--workspace`, `--registry` | `--role` excludes `--request-id` and nothing else (E1); `--request-id` may combine with `--workspace` and `--registry`; `--workspace` and `--registry` both or neither; any violation is `usage` | `status_v1` |',
    '| `preview` | read | `--ledger`, `--role` | none | R16 adds `--since`; not in R15 | `preview_v1` |',
    '| `claim` | mutation | `--ledger`, `--job`, `--role`, `--request-id`, `--actor` | `--provenance` | none | `claim_v1` |',
    '| `takeover` | mutation | `--ledger`, `--job`, `--role`, `--run-id`, `--request-id`, `--actor` | `--provenance` | `--run-id` is the current active run id of the role, not a token | `claim_v1` |',
]
PARAM_ROWS = [
    '| `activate` | `{expect_job, expect_revision, job}`; `expect_job` is the text `NONE` or the job ID, `expect_revision` is the decimal string or `none` |',
    '| `claim` | `{job, role}` plus `provenance_path` only when `--provenance` was supplied |',
    '| `takeover` | `{job, role, run_id}` plus `provenance_path` only when `--provenance` was supplied; `run_id` is a decimal string |',
]
PREC_FORMAT3 = '| format3 load and `activate` | exactly as EX-3 "Format3 load and activate preconditions" (unchanged) |'
PREC_CLAIM = '| `claim` | `request_unused`, `active_revision == R`, `active_job == J`, `fact_version(job, role) == v` for every role of the active job, `role_state(target) == (state, drifted, run)`, `generation_current(target, G)` |'
PREC_TAKEOVER = '| `takeover` | the `claim` set with `role_state(target) == (running, drifted, run)` where `run` is the `--run-id`; no member uses a secret; `run_active` is not used |'
PREC_RESERVED = '| `refresh`, `checkpoint`, `release`, the `check` and `check-recover` commits, `verdict` | not defined in this part (D150): these operations are reserved and have no request body, facts or response here; their member sets are defined by the contract round of each operation (R16, R17, R18), using only the eight closed members |'
PREC_READS = '| `status`, `preview` | none (reads; one read transaction) |'
SERVICE_ROWS = [
    '| `activate` | `read_execution({})`; `commit_execution({change:C})` | no capture |',
    '| `status` | `read_execution({})`; then, only with `--request-id`, `read_request({request_id:TEXT})`; then, only with `--workspace` and `--registry`, `open_workspace({workspace:SLOT})` and `capture_drafts({route:R})` | no commit; the optional services combine and keep this order |',
    '| `preview` | `read_execution({})` | no commit |',
    '| `claim` | `read_execution({})`; `read_provenance({provenance:SLOT})` only when `--provenance` is present; `commit_execution({change:C})` | |',
    '| `takeover` | the `claim` services | |',
    '| format3 `load` | `open_workspace`, `capture_load`, `read_execution({})`, `accept_load({bundle:B,semantic:M,execution:CHANGE})` | format1 and format2 keep three services and `{bundle,semantic}` |',
]
MANIFEST_ROWS = [
    '| `version` | 1 |',
    '| `revision` | revisions id, decimal string |',
    '| `job`, `role` | IDs |',
    '| `generation` | the generation ordinal, decimal string |',
    L1_RUN_ROW,
    '| `versions` | the seven `work_exec_v1` versions then `dispatch_v1` |',
    '| `work` | `{digest}` (the `xxh3_128:` work fingerprint; the W value is the record `dispatches.work_id`) |',
    '| `render` | `{digest,bytes,budget_bytes}` as render M |',
    '| `sections` | ordered array of `{id,mandatory,included,bytes}`, includes `run` with `mandatory:true` |',
    '| `omitted` | ordered array of optional section IDs |',
    '| `prerequisites` | sorted array as render M, statuses from ledger qualities (`available`, `unavailable`, `unverified`, `answered`; `not_captured` never) |',
    '| `registry` | `{id,revision,authority:"driver_selected_trusted_configuration"}` |',
    '| `inputs` | `{source,interpretation,publications,repair}`: `source` and `interpretation` are record ids (decimal strings of `revisions.source_id` and `revisions.interpretation_id`), `publications` is a sorted array of `{work,facet,publication,semantic}` (publication id decimal string, semantic digest text; `[]` in R15), `repair` is the repair revision as a decimal string (`"0"` in R15) |',
]
PURPOSE_ROWS = [
    '| `refresh` | `Re-render this work and acknowledge the current packet.` |',
    '| `checkpoint` | `Record observations of progress on this run.` |',
    '| `check` | `Run the checks of the job against the output of this role.` |',
    '| `release` | `End this run as released or interrupted.` |',
    '| `verdict` | `Record the verdict of the reviewer for this run.` |',
]
MAP_ROWS = [
    '| path missing, unresolvable, or the file cannot be opened | `io` | `Cannot resolve explicit input path` |',
    '| the final path component is a symbolic link | `path_denied` | `Explicit input file is a symbolic link` |',
    '| not a regular file (directory, FIFO, socket, device) | `io` | `Explicit input must be a regular file` |',
    '| the file changed between the capture gate and the end of the read | `capture_changed` | `File changed during read` |',
    '| a bound of EX-8 is exceeded | `resource_limit` | `Execution resource limit exceeded` with `details:{limit,maximum}` |',
    '| JSON grammar or shape violation | `schema` | the existing message of the shared JSON parser or schema normalizer |',
]
ABI_ROWS = [
    '| activate / activate | `shipped.activate.v1` | `activate_v1` | `execution_commit_v1` |',
    '| status / status | `shipped.status.v1` | `status_v1` | `execution_read_v1`, `draft_capture_v1` |',
    '| preview / preview | `shipped.preview.v1` | `preview_v1` | `execution_read_v1` |',
    '| claim / claim | `shipped.claim.v1` | `claim_v1` | `execution_commit_v1`, `provenance_read_v1` |',
    '| takeover / takeover | `shipped.takeover.v1` | `claim_v1` | `execution_commit_v1`, `provenance_read_v1` |',
]
PROD_HELP = [
    'planar activate --ledger PATH --job ID --expect-revision ID_OR_NONE --expect-job JOB_OR_NONE --request-id ID --actor TEXT',
    'planar status --ledger PATH [--role ID | --request-id ID] [--workspace FILE_OR_NAME --registry FILE]',
    'planar preview --ledger PATH --role ID',
    'planar claim --ledger PATH --job ID --role ID --request-id ID --actor TEXT [--provenance FILE]',
    'planar takeover --ledger PATH --job ID --role ID --run-id ID --request-id ID --actor TEXT [--provenance FILE]',
]
AUTH_HELP = [
    'planar spec-prompt --workspace FILE_OR_NAME --registry FILE',
    'planar critique --workspace FILE_OR_NAME --registry FILE',
    'planar ingest --workspace FILE_OR_NAME --registry FILE',
    'planar explain-rejection --diagnostic FILE',
    'planar observe --from DIRECTORY',
    'planar orchestrator --workspace FILE_OR_NAME --registry FILE --job ID',
    '  or --ledger PATH',
]
HISTORY_LINE = '  or --revision ID | --request-id ID | --at-event ID | --event ID'
# Underscore identifiers of the approved vocabulary (DECISIONS.md sections 3 to 6).
WHITELIST = frozenset([
    'accept_load',
    'accept_load_v1',
    'activate_v1',
    'active_job',
    'active_revision',
    'attachment_id',
    'base_label',
    'budget_bytes',
    'budget_exceeded',
    'bytes_hex',
    'canonical_v1',
    'capability_denied',
    'capture_changed',
    'capture_drafts',
    'capture_load',
    'claim_conflict',
    'claim_v1',
    'command_options',
    'commit_execution',
    'contract_checks',
    'declarations_v1',
    'default_root_help',
    'dependencies_v1',
    'dependency_cycle',
    'dispatch_context_v1',
    'dispatch_manifest_v1',
    'dispatch_v1',
    'draft_capture_v1',
    'driver_assertion_unverified',
    'driver_provenance_v1',
    'driver_selected_trusted_configuration',
    'effective_checks',
    'event_not_found',
    'event_seq',
    'exact_bytes_v1',
    'execution_change_v1',
    'execution_commit_v1',
    'execution_read_v1',
    'execution_snapshot_bytes',
    'execution_snapshot_v1',
    'expect_job',
    'expect_revision',
    'fact_version',
    'fenced_runs',
    'foreign_ledger',
    'fragment_v1',
    'generation_current',
    'generation_id',
    'handler_id',
    'interpret_bundle',
    'interpretation_id',
    'interpretation_mismatch',
    'job_activated',
    'job_activations',
    'job_notes',
    'last_event',
    'last_seq',
    'ledger_busy',
    'ledger_corrupt',
    'ledger_initialized',
    'ledger_not_found',
    'lua_syntax',
    'manifest_bytes',
    'manifest_id',
    'no_active_job',
    'not_captured',
    'occurred_at',
    'open_workspace',
    'orchestrator_actions',
    'orchestrator_capture_v1',
    'orchestrator_context_v1',
    'orchestrator_state',
    'orchestrator_v1',
    'output_bytes',
    'output_vector',
    'packet_id',
    'path_denied',
    'pipeline_predecessors',
    'preview_v1',
    'previous_job',
    'previous_run',
    'provenance_bytes',
    'provenance_depth',
    'provenance_id',
    'provenance_path',
    'provenance_read_v1',
    'provenance_settings',
    'provenance_string_bytes',
    'read_execution',
    'read_history',
    'read_provenance',
    'read_request',
    'recorded_history',
    'render_digest',
    'render_fp',
    'request_conflict',
    'request_id',
    'request_not_found',
    'request_unused',
    'request_v3',
    'reservation_open',
    'resource_limit',
    'response_kind',
    'revision_conflict',
    'role_detail',
    'role_output',
    'role_state',
    'role_structure_v1',
    'root_help_lines',
    'run_active',
    'run_claimed',
    'run_id',
    'run_state',
    'run_taken_over',
    'selection_v1',
    'shape_policy',
    'snapshot_seq',
    'source_id',
    'splice_root_help',
    'start_facets',
    'state_conflict',
    'status_v1',
    'stdlib_digest',
    'storage_full',
    'unsupported_interpretation',
    'unsupported_ledger_version',
    'unsupported_version',
    'work_exec_v1',
    'work_fp',
    'work_id',
    'work_or_semantic',
    'work_v1',
    'workspace_notes',
    'xxh3_128',
    'xxh3_128_v1',
])

PROD_BLOCK = '\n'.join(PROD_HELP + ['planar --help'])
AUTH_BLOCK = '\n'.join(AUTH_HELP[:6] + [AUTH_HELP[6]] + ['planar --help'])


class CheckFail(Exception):
    """A check observed content that violates its invariant; the message is the reason."""


def norm(s):
    """Collapse all runs of whitespace to one space."""
    return ' '.join(s.split())


def short(s, n=110):
    s = s.replace('\n', ' ')
    return s if len(s) <= n else s[:n] + '...'


def prim_absent(text, lits, where):
    nt = norm(text)
    for lit in lits:
        if norm(lit) in nt:
            raise CheckFail('%s: forbidden text present: %s' % (where, short(lit)))


def prim_present(text, lits, where):
    nt = norm(text)
    for lit in lits:
        if norm(lit) not in nt:
            raise CheckFail('%s: missing literal: %s' % (where, short(lit)))


def prim_present_raw(text, lits, where):
    for lit in lits:
        if lit not in text:
            raise CheckFail('%s: missing exact text: %s' % (where, short(lit)))


def prim_lines(text, lines, where):
    have = set(l.strip() for l in text.split('\n'))
    for want in lines:
        if want.strip() not in have:
            raise CheckFail('%s: missing exact row or line: %s' % (where, short(want)))


def prim_headings(text, headings, where):
    lines = [l.rstrip() for l in text.split('\n')]
    pos = []
    for h in headings:
        hits = [i for i, l in enumerate(lines) if l == h]
        if len(hits) != 1:
            raise CheckFail('%s: heading %s occurs %d times' % (where, short(h, 80), len(hits)))
        pos.append(hits[0])
    if pos != sorted(pos):
        raise CheckFail('%s: headings out of order' % where)


def prim_ordered(text, markers, where):
    nt = norm(text)
    last = -1
    for m in markers:
        at = nt.find(norm(m))
        if at < 0:
            raise CheckFail('%s: missing step marker: %s' % (where, short(m)))
        if at < last:
            raise CheckFail('%s: step out of order: %s' % (where, short(m)))
        last = at


def split_row(line):
    parts = re.split(r'(?<!\\)\|', line.strip())
    return [p.strip() for p in parts[1:-1]]


def tables(text):
    """Return [(header_cells, [(line, cells), ...]), ...] for every markdown table."""
    lines = text.split('\n')
    out = []
    i = 0
    while i < len(lines):
        if (lines[i].startswith('|') and i + 1 < len(lines)
                and re.match(r'^\|\s*:?-{3,}', lines[i + 1])):
            header = split_row(lines[i])
            rows = []
            j = i + 2
            while j < len(lines) and lines[j].startswith('|'):
                rows.append((lines[j], split_row(lines[j])))
                j += 1
            out.append((header, rows))
            i = j
        else:
            i += 1
    return out


def heading_block(text, heading):
    """Body of a heading (exact line match) up to the next heading of equal or higher rank."""
    lines = text.split('\n')
    level = len(heading) - len(heading.lstrip('#'))
    hits = [i for i, l in enumerate(lines) if l.rstrip() == heading]
    if len(hits) != 1:
        raise CheckFail('heading %s occurs %d times' % (short(heading, 70), len(hits)))
    start = hits[0]
    pat = re.compile(r'^#{1,%d}\s' % level)
    end = len(lines)
    for j in range(start + 1, len(lines)):
        if pat.match(lines[j]):
            end = j
            break
    return '\n'.join(lines[start + 1:end])


def excluded_lines(text):
    """Indices of lines inside sections that R15-A2 may edit beyond Part 1 prose."""
    out = set()
    flag = False
    for i, l in enumerate(text.split('\n')):
        if l.startswith('## '):
            flag = l.startswith(EXCL_PREFIXES)
        if flag:
            out.add(i)
    return out


def member_names(text):
    names = set()
    for header, rows in tables(text):
        if not any('Members' in h for h in header):
            continue
        for _line, cells in rows:
            if len(cells) < 2:
                continue
            for span in re.findall(r'`([^`]*)`', cells[1]):
                m = re.match(r'([a-z_]+)(?:\(| ==)', span)
                if m:
                    names.add(m.group(1))
                elif span in EIGHT:
                    names.add(span)
    return names


def prim_names_subset(names, allowed, where):
    if not names:
        raise CheckFail('%s: no precondition member names found' % where)
    extra = sorted(set(names) - set(allowed))
    if extra:
        raise CheckFail('%s: member names outside the closed set: %s' % (where, ', '.join(extra)))


def code_tokens(text):
    return set(re.findall(r'`([a-z][a-z0-9]*(?:_[a-z0-9]+)+)`', text))


def prim_codes(text, allowed, where):
    extra = sorted(code_tokens(text) - set(allowed))
    if extra:
        raise CheckFail('%s: underscore tokens outside the approved vocabulary: %s' % (where, ', '.join(extra)))


def message_strings(text):
    return set(re.findall(r'messages?\s+`([^`]+)`', norm(text)))


def prim_messages(text, allowed, where):
    extra = sorted(message_strings(text) - set(allowed))
    if extra:
        raise CheckFail('%s: message strings that are not existing: %s' % (where, '; '.join(extra)))


def line_diff(base, new):
    a = base.split('\n')
    b = new.split('\n')
    sm = difflib.SequenceMatcher(None, a, b, autojunk=False)
    return a, b, [op for op in sm.get_opcodes() if op[0] != 'equal']


def prim_removed_zero(base, new, where):
    a, b, ops = line_diff(base, new)
    removed = sum(i2 - i1 for tag, i1, i2, j1, j2 in ops if tag in ('delete', 'replace'))
    if removed:
        raise CheckFail('%s: %d base line(s) removed or replaced' % (where, removed))


def prim_added_clean(base, new, bad_re, where):
    a, b, ops = line_diff(base, new)
    for tag, i1, i2, j1, j2 in ops:
        for j in range(j1, j2):
            if bad_re.search(b[j]):
                raise CheckFail('%s: added line contains forbidden wording: %s' % (where, short(b[j], 80)))


def prim_allow_changed(base, new, excluded, where):
    a, b, ops = line_diff(base, new)
    for tag, i1, i2, j1, j2 in ops:
        for j in range(j1, j2):
            if j in excluded:
                continue
            if not ALLOW_RE.search(b[j]):
                raise CheckFail('%s: changed Part 1 line is not on the amendment allow-list: %s' % (where, short(b[j], 90)))


def prim_one_changed(base, new, where):
    """Return (base_line, new_line) when exactly one line changed as a 1-to-1 replacement."""
    a, b, ops = line_diff(base, new)
    if len(ops) != 1 or ops[0][0] != 'replace' or ops[0][2] - ops[0][1] != 1 or ops[0][4] - ops[0][3] != 1:
        raise CheckFail('%s: expected exactly one changed line, found %d change block(s)' % (where, len(ops)))
    return a[ops[0][1]], b[ops[0][3]]


def prim_frozen(base_lines, new_lines, where):
    if base_lines != new_lines:
        raise CheckFail('%s: differs from the base snapshot' % where)


def prim_local_paths(text, where):
    m = re.search(r'/Users/|/home/|C:\\', text)
    if m:
        raise CheckFail('%s: machine-local path text found' % where)


def prim_rows_once(text, label, required, where):
    rows = []
    for header, trows in tables(text):
        for line, cells in trows:
            if cells and cells[0].split() and cells[0].split()[0] == label:
                rows.append(line)
    if len(rows) != 1:
        raise CheckFail('%s: row %s occurs %d times' % (where, label, len(rows)))
    for req in required:
        if req not in rows[0]:
            raise CheckFail('%s: row %s lacks %s' % (where, label, short(req, 60)))


def prim_tags(text, expected, where):
    rows = []
    for header, trows in tables(text):
        for line, cells in trows:
            if cells and cells[0] in expected:
                rows.append((cells[0], cells))
    for did, want in expected.items():
        hits = [c for k, c in rows if k == did]
        if len(hits) != 1:
            raise CheckFail('%s: decision %s occurs %d times' % (where, did, len(hits)))
        cells = hits[0]
        if len(cells) < 4 or cells[2] != want:
            got = cells[2] if len(cells) > 2 else '(none)'
            raise CheckFail('%s: %s tag is %s, expected %s' % (where, did, short(got, 60), short(want, 60)))
        if not cells[3]:
            raise CheckFail('%s: %s has an empty basis' % (where, did))


def prim_paragraph_labels(text, label, minimum, gate, where):
    paras = [p for p in re.split(r'\n\s*\n', text) if label in p]
    if len(paras) < minimum:
        raise CheckFail('%s: %d paragraph(s) labelled %s, need %d' % (where, len(paras), label, minimum))
    for p in paras:
        if gate not in norm(p):
            raise CheckFail('%s: a %s paragraph does not state: %s' % (where, label, gate))


def allowed_messages(texts):
    """Return the backtick strings of the given texts, paired per line and per table cell (never over a whole file)."""
    out = set()
    for text in texts:
        for line in text.split('\n'):
            out.update(re.findall(r'`([^`]+)`', line))
            if line.startswith('|'):
                for cell in split_row(line):
                    out.update(re.findall(r'`([^`]+)`', cell))
    return out


def first_diff(got, want):
    """Describe the first differing fragment of two normalized strings."""
    i = 0
    n = min(len(got), len(want))
    while i < n and got[i] == want[i]:
        i += 1
    return 'offset %d, have "%s", expected "%s"' % (i, got[i:i + 40], want[i:i + 40])


def prim_decision_cells(text, want, where):
    """Compare the Design choice and Basis cells of the named decision rows with their literals."""
    for did, (cell_want, basis_want) in sorted(want.items()):
        hits = [cells for _h, trows in tables(text) for _l, cells in trows if cells and cells[0] == did]
        if len(hits) != 1:
            raise CheckFail('%s: decision %s occurs %d times' % (where, did, len(hits)))
        cells = hits[0]
        if len(cells) < 4:
            raise CheckFail('%s: decision %s has fewer than four cells' % (where, did))
        for label, got, wanted in (('Design choice', cells[1], cell_want), ('Basis', cells[3], basis_want)):
            if norm(got) != norm(wanted):
                raise CheckFail('%s: %s %s cell differs: %s' % (where, did, label, first_diff(norm(got), norm(wanted))))


def decision_row_cells(text, did):
    """Return the cells of the one Decisions table row with the given ID (text is the whole contract)."""
    hits = [cells for _h, trows in tables(heading_block(text, '## Decisions table'))
            for _l, cells in trows if cells and cells[0] == did]
    if len(hits) != 1:
        raise CheckFail('Decisions table: row %s occurs %d times' % (did, len(hits)))
    return hits[0]


def amendment_row(text, aid):
    """Return (header, cells) of the one Part 1 amendments row with the given ID."""
    for header, trows in tables(heading_block(text, PART1_AM)):
        if header and header[0] == 'ID':
            for _l, cells in trows:
                if cells and cells[0] == aid:
                    return header, cells
    raise CheckFail('Part 1 amendments: row %s not found' % aid)


def amendment_cell(header, cells, column, aid):
    """Return the cell under the header that starts with the column name (the Location header carries a note)."""
    found = [i for i, h in enumerate(header) if h == column or h.startswith(column + ' ')]
    if len(found) != 1:
        raise CheckFail('Part 1 amendments: no single %s column' % column)
    idx = found[0]
    if idx >= len(cells):
        raise CheckFail('Part 1 amendments: row %s has no %s cell' % (aid, column))
    return cells[idx]


def prim_msg_table(base, new, where):
    """Compare Messages tables: only the four listed cell edits may differ from the snapshot."""
    def rows_of(text):
        out = {}
        for header, trows in tables(heading_block(text, '## Messages')):
            for line, cells in trows:
                if header[0] == 'Refusal site':
                    key = (tuple(header), cells[1], cells[2])
                else:
                    key = (tuple(header), cells[0], cells[1])
                out[key] = (header, cells)
        return out
    old = rows_of(base)
    cur = rows_of(new)
    for key, (header, cells) in old.items():
        if key not in cur:
            raise CheckFail('%s: Messages row changed or removed: %s' % (where, short(key[1], 60)))
        ncells = cur[key][1]
        if len(ncells) != len(cells):
            raise CheckFail('%s: Messages row column count changed' % where)
        for idx, (x, y) in enumerate(zip(cells, ncells)):
            if x == y:
                continue
            ok = (header[0] == 'Code' and idx == 3 and key[1] in ('`revision_conflict`', '`reference`', '`schema`')) \
                or (header[0] == 'Refusal site' and idx == 0 and key[1] == '`state_conflict`')
            if not ok:
                raise CheckFail('%s: Messages cell changed outside A-8, A-9, A-10, A-13: %s' % (where, short(key[1], 60)))


def find_msg_row(text, code, msg_hint=None):
    hits = []
    for header, trows in tables(heading_block(text, '## Messages')):
        for line, cells in trows:
            if header[0] == 'Refusal site':
                c, m = cells[1], cells[2]
            else:
                c, m = cells[0], cells[1]
            if c == code and (msg_hint is None or msg_hint in m):
                hits.append((header, cells))
    return hits


def fixed_ex7(text):
    return heading_block(text, '## EX-7 Error codes and messages')


def ex_table_lines(lines, heading):
    start = None
    for i, l in enumerate(lines):
        if l.rstrip() == heading:
            start = i
            break
    if start is None:
        raise CheckFail('heading %s missing' % heading)
    out = []
    for l in lines[start + 1:]:
        if l.startswith('## '):
            break
        if l.startswith('|'):
            out.append(l)
    return out


def frozen_view(text):
    lines = text.split('\n')
    if '```sql' not in lines:
        raise CheckFail('sql block missing')
    i = lines.index('```sql')
    j = i + 1
    while j < len(lines) and lines[j] != '```':
        j += 1
    if j >= len(lines):
        raise CheckFail('sql block not closed')
    m_rows = [l for l in lines if re.match(r'^\| M\d+ \|', l)]
    n_rows = [l for l in lines if re.match(r'^\| N\d+ \|', l)]
    ex = ex_table_lines(lines, '## EX-7 Error codes and messages') + ex_table_lines(lines, '## EX-8 Execution bounds')
    dr = []
    for l in lines:
        if l.startswith('|'):
            cells = split_row(l)
            did = cells[0] if cells else ''
            m = re.fullmatch(r'D(\d+)', did)
            if (m and 7 <= int(m.group(1)) <= 103) or re.fullmatch(r'OP[123]', did):
                dr.append(l)
    return {'sql': lines[i:j + 1], 'M': m_rows, 'N': n_rows, 'EX7_EX8': ex, 'D7_OP3': dr}


def xp_block(text):
    lines = text.split('\n')
    hits = [i for i, l in enumerate(lines) if l.startswith('| Registration duplicate/reserved identity')]
    if len(hits) != 1:
        raise CheckFail('XP message table row occurs %d times' % len(hits))
    i = hits[0]
    s = i
    while s > 0 and lines[s - 1].startswith('|'):
        s -= 1
    e = i
    while e + 1 < len(lines) and lines[e + 1].startswith('|'):
        e += 1
    return lines[s:e + 1]


class Ctx(object):
    def __init__(self, repo, base_dir, git_repo, base_commit):
        self.repo = repo
        self.base_dir = base_dir
        self.git_repo = git_repo
        self.base_commit = base_commit

    def _read(self, path, label):
        try:
            with open(path, encoding='utf-8') as fh:
                return fh.read()
        except (OSError, UnicodeDecodeError) as exc:
            raise CheckFail('cannot read %s (%s)' % (label, type(exc).__name__))

    def doc(self, name):
        return self._read(os.path.join(self.repo, 'docs', 'vernacular', name), 'docs/vernacular/' + name)

    def base(self, name):
        return self._read(os.path.join(self.base_dir, name), 'base/' + name)

    def git_bytes(self, path):
        if not self.git_repo:
            raise CheckFail('no --git-repo given')
        if not self.base_commit:
            raise CheckFail('no --base-commit given')
        if not re.fullmatch(r'[0-9a-f]{40}', self.base_commit):
            raise CheckFail('--base-commit must be a full commit SHA')
        proc = subprocess.run(['git', '-C', self.git_repo, 'show', '%s:%s' % (self.base_commit, path)],
                              capture_output=True)
        if proc.returncode != 0:
            raise CheckFail('git show of %s failed' % path)
        return proc.stdout


CHECKS = []


def check(family, expect_red):
    def deco(fn):
        m = re.fullmatch(r'CHK_A2_(\d{3})_(\w+)', fn.__name__)
        if not m:
            raise SystemExit('bad check function name')
        name = 'CHK-A2-%s %s' % (m.group(1), m.group(2))
        CHECKS.append((name, fn, expect_red, family))
        return fn
    return deco


def p2sec(ec, n):
    return heading_block(ec, P2[n])



# ---- Part 1 frozen text, placeholder and heading rows ----

@check('absent', True)
def CHK_A2_001_no_placeholder_text(ctx):
    prim_absent(ctx.doc(EC), ['Not yet written', 'Marked non-normative placeholder',
                              'non-normative placeholder'], EC)


@check('present', True)
def CHK_A2_002_leadin_normative(ctx):
    ec = ctx.doc(EC)
    body = heading_block(ec, P2_TITLE)
    lead = body.split('\n### ')[0]
    prim_present(lead, ['Part 2 is normative'], 'Part 2 lead-in')
    prim_absent(ec, ['They are not normative'], EC)


@check('headings', True)
def CHK_A2_003_seven_headings_once(ctx):
    ec = ctx.doc(EC)
    prim_headings(ec, [P2_TITLE] + [P2[n] for n in range(1, 8)], EC)
    prim_absent(ec, ['### Part 2 (R15-A2):', '## Part 2 placeholders (R15-A2)'], EC)


@check('frozen', False)
def CHK_A2_004_part1_frozen_blocks(ctx):
    new = frozen_view(ctx.doc(EC))
    old = frozen_view(ctx.base(EC))
    for key in ('sql', 'M', 'N', 'EX7_EX8', 'D7_OP3'):
        prim_frozen(old[key], new[key], 'Part 1 frozen block %s' % key)


@check('allow', True)
def CHK_A2_005_part1_amendments_listed(ctx):
    base = ctx.base(EC)
    new = ctx.doc(EC)
    am = heading_block(new, PART1_AM)
    for n in range(1, 14):
        hits = [c for h, rows in tables(am) for _l, c in rows if c and c[0] == 'A-%d' % n]
        if len(hits) != 1:
            raise CheckFail('amendment A-%d listed %d times' % (n, len(hits)))
        if len(hits[0]) < 4 or not hits[0][3]:
            raise CheckFail('amendment A-%d has no reason cell' % n)
    prim_allow_changed(base, new, excluded_lines(new), 'Part 1 prose')


# ---- P2-1 option maps, spelling, exclusions and preconditions ----

@check('lines', True)
def CHK_A2_010_option_maps(ctx):
    prim_lines(p2sec(ctx.doc(EC), 1), OPT_ROWS, 'P2-1 option maps')


@check('present', True)
def CHK_A2_011_expect_job_spelling(ctx):
    p = p2sec(ctx.doc(EC), 1)
    prim_lines(p, [OPT_ROWS[0]], 'P2-1 activate option row')
    prim_present(p, ['`--expect-job` is `NONE` or a job ID', 'JOB_OR_NONE'], 'P2-1 spelling')
    if '`none` is a legal job ID' not in norm(p) and '`none` stays an ordinary job ID' not in norm(p):
        raise CheckFail('P2-1 spelling: no sentence that none is a legal job ID')
    prim_absent(p, ['--expect-job ID_OR_NONE'], 'P2-1 metavariable')


@check('present', True)
def CHK_A2_012_status_exclusions(ctx):
    p = p2sec(ctx.doc(EC), 1)
    prim_lines(p, [OPT_ROWS[1]], 'P2-1 status option row')
    prim_present(p, ['`--role` excludes `--request-id` and nothing else',
                     '`--request-id` may combine with `--workspace` and `--registry`'], 'P2-1 status')
    if re.search(r'`--request-id`\s+excludes', norm(p)):
        raise CheckFail('P2-1 status: --request-id is written as excluding another option')


@check('lines', True)
def CHK_A2_013_claim_precondition_set(ctx):
    prim_lines(p2sec(ctx.doc(EC), 1), [PREC_CLAIM], 'P2-1 claim precondition row')


@check('lines', True)
def CHK_A2_014_takeover_precondition_set(ctx):
    p = p2sec(ctx.doc(EC), 1)
    prim_lines(p, [PREC_TAKEOVER], 'P2-1 takeover precondition row')


@check('names', True)
def CHK_A2_015_closed_vocabulary_only(ctx):
    p = p2sec(ctx.doc(EC), 1)
    prim_names_subset(member_names(p), EIGHT, 'P2-1 precondition table')
    prim_present(p, ['using only the eight closed members'], 'P2-1 reserved rows')


@check('present', True)
def CHK_A2_016_exempt_reads(ctx):
    p = p2sec(ctx.doc(EC), 1)
    prim_present(p, ['`status` without `--role` (bare, `--request-id`, `--workspace` with `--registry`, '
                     'or both of the last two), `history`, `export` and `orchestrator --ledger` skip step (4) only',
                     '`init` is not an exempt read',
                     'not bound (no active revision exists; not an execution verb)',
                     '`preview` is bound, not exempt'], 'P2-1 exempt reads')


@check('present', True)
def CHK_A2_017_reference_field_values(ctx):
    ec = ctx.doc(EC)
    prim_present(p2sec(ec, 1), ['`reference` with message `Unknown selected job` and field `selection.job`, '
                                'then `Unknown selected role` and field `selection.role`',
                                'message `Unknown selected role`, field `selection.role`'], 'P2-1 reference rows')
    prim_present(ec, ['The `schema` field is `provenance` alone for a grammar violation'], 'P2-4 schema field')


@check('lines', True)
def CHK_A2_018_service_lists_ordered(ctx):
    prim_lines(p2sec(ctx.doc(EC), 1), SERVICE_ROWS, 'P2-1 service table')


@check('ordered', True)
def CHK_A2_019_refusal_order(ctx):
    p = p2sec(ctx.doc(EC), 1)
    prim_ordered(p, ['(1) `usage`', '(2) `unsupported_ledger_version` for a format1 or format2 ledger',
                     '(3) early request replay', '(4) `interpretation_mismatch`', '(5) expectations',
                     '(6) `no_active_job` when no job is active',
                     '(7) `reference` with message `Unknown selected job`', '(8) role-state refusals',
                     '(9) for `claim` and `takeover` with `--provenance`', '(10) commit preconditions'],
                 'P2-1 refusal order')
    prim_present(p, ['`--expect-job JOB` naming a job that is not the active job',
                     '`--expect-job NONE` while a job is active'], 'P2-1 expectations')


@check('codes', True)
def CHK_A2_020_error_codes_closed(ctx):
    ec = ctx.doc(EC)
    part2 = heading_block(ec, P2_TITLE)
    # A closed-set scan of an empty section passes, so the step-(6) and step-(8) code
    # literals of P2-1 must be present first (they are pinned by CHK-A2-019 and CHK-A2-119).
    prim_present(p2sec(ec, 1), ['(6) `no_active_job` when no job is active',
                                'otherwise `state_conflict`'], 'P2-1 code positions')
    prim_codes(part2, WHITELIST, 'Part 2 code tokens')


@check('frozen', False)
def CHK_A2_021_ex7_messages_identical(ctx):
    new = ctx.doc(EC)
    old = ctx.base(EC)
    if fixed_ex7(new) != fixed_ex7(old):
        raise CheckFail('EX-7 section differs from the base snapshot')
    prim_msg_table(old, new, 'Messages')


@check('messages', True)
def CHK_A2_022_no_new_message(ctx):
    ec = ctx.doc(EC)
    part2 = heading_block(ec, P2_TITLE)
    prim_present(p2sec(ec, 1), ['No twentieth message'], 'P2-1')
    allowed = allowed_messages(ctx.base(name) for name in TEN)
    prim_messages(part2, allowed, 'Part 2 message strings')


@check('frozen', False)
def CHK_A2_023_xp_message_table_identical(ctx):
    new = xp_block(ctx.doc(XP))
    old = xp_block(ctx.base(XP))
    prim_frozen(old, new, 'external-profile message table')


# ---- P2-1 result schemas ----

@check('present', True)
def CHK_A2_024_activate_v1_schema(ctx):
    prim_present(p2sec(ctx.doc(EC), 1), [
        '`{version:1,kind:"activate",ledger,request_id,revision,job,previous_job,activation,event,roles}`'],
        'P2-1 activate_v1')


@check('present', True)
def CHK_A2_025_status_v1_schema(ctx):
    prim_present(p2sec(ctx.doc(EC), 1), [
        '`{version:1,kind:"status",ledger,active_revision,active_job,last_event,roles,drafts?,role_detail?,request?}`',
        '`{role,acknowledged,current,changed,explain,reasons}`',
        '`{role,kind,generation,state,drifted,run}`',
        '`{status,changed,error?}`',
        '`{request_id,operation,outcome,result,disposition?}`',
        '`{work_or_semantic,output_vector,prerequisite}`',
        '`current` is `{work}` (or the string `none` for a retired role the active revision no longer declares, D152)',
    ], 'P2-1 status_v1')


@check('present', True)
def CHK_A2_026_preview_v1_schema(ctx):
    prim_present(p2sec(ctx.doc(EC), 1), [
        '`{version:1,kind:"preview",ledger,revision,job,role,generation,state,work_fp,render_digest,bytes,packet,manifest}`',
        'writes no event and no request'], 'P2-1 preview_v1')


@check('present', True)
def CHK_A2_027_claim_v1_schema(ctx):
    prim_present(p2sec(ctx.doc(EC), 1), [
        '`{version:1,kind:"claim",ledger,request_id,job,role,revision,run:{id,generation,token},dispatch:{id,work_fp,render_digest,bytes},packet,manifest}`',
        '`previous_run` (decimal string of the fenced run id) present only for takeover'], 'P2-1 claim_v1')


@check('present', True)
def CHK_A2_028_takeover_mapping(ctx):
    p = p2sec(ctx.doc(EC), 1)
    prim_present(p, ['`claim_conflict` is never used by takeover',
                     'At step (8) the named run must be a run of that job and role, otherwise `state_conflict`',
                     'the refusal is `fenced` with `{run_state}` of the named run',
                     '`takeover` is never `claim_conflict`'], 'P2-1 takeover mapping')


@check('lines', True)
def CHK_A2_029_request_parameter_maps(ctx):
    prim_lines(p2sec(ctx.doc(EC), 1), PARAM_ROWS, 'P2-1 parameter maps')


@check('present', True)
def CHK_A2_030_read_execution_schema(ctx):
    prim_present(p2sec(ctx.doc(EC), 1), [
        '`{version:1,ledger,last_seq,snapshot}`',
        '`{id,generation,dispatch:{id,work_fp,render_fp,revision,work}}`',
        'Run secrets and tokens never appear',
        '`publications` (array, `[]` in R15)'], 'P2-1 read_execution')


@check('present', True)
def CHK_A2_031_change_schema(ctx):
    prim_present(p2sec(ctx.doc(EC), 1), [
        '`execution_change_v1` with exactly `version` (1), `operation` (`load`, `activate`, `claim` or `takeover`), '
        '`snapshot_seq` (decimal string), `facts` (operation facts: activate `{job,generations}`; claim `{role,provenance?}`; '
        'takeover `{role,previous_run,provenance?}`; load `{generations,fenced_runs}`), `vector` (the complete next '
        'role-state vector of the active job, validated and never written wholesale), `transitions`, '
        '`packet` (claim, takeover only: `{markdown,manifest,work}`) and `expected`',
        '`expected` is an array of objects with `kind` one of the eight closed members '
        '(`active_revision`, `active_job`, `fact_version`, `role_state`, `generation_current`, `run_active`, '
        '`reservation_open`, `request_unused`)'], 'P2-1 execution_change_v1')


@check('present', True)
def CHK_A2_032_accept_load_format3(ctx):
    p = p2sec(ctx.doc(EC), 1)
    prim_present(p, ['format1 and format2 keep three services and `{bundle,semantic}`',
                     '`accept_load({bundle:B,semantic:M,execution:CHANGE})`',
                     'the extra key `execution` is `CHANGE` of the same shape with `operation:"load"`'],
                 'P2-1 accept_load')
    prim_present(p, ['format1 and format2 keep three services'], 'P2-1 format1 and format2 services')


# ---- P2-2 dispatch projection ----

@check('present', True)
def CHK_A2_040_dispatch_mode(ctx):
    prim_present(p2sec(ctx.doc(EC), 2), ['`mode` is `"dispatch"` for claim, takeover and `preview`',
                                         'Its packet has `mode:"dispatch"`'], 'P2-2 packet mode')


@check('present', True)
def CHK_A2_041_authority_templates(ctx):
    prim_present(p2sec(ctx.doc(EC), 2), [
        'This packet was dispatched by Planar for run request {request_id}, generation {generation} of role {role}, '
        'under revision {revision} of job {job}. Checks have not run in this dispatch. Upstream facet content comes '
        'from recorded publications, and prerequisite status comes from ledger qualities, not from a checkout.',
        'This packet is a preview of a dispatch for generation {generation} of role {role}, under revision {revision} '
        'of job {job}. No run exists and nothing was retained. Checks have not run in this dispatch. Upstream facet '
        'content comes from recorded publications, and prerequisite status comes from ledger qualities, not from a checkout.',
    ], 'P2-2 authority templates')


@check('present', True)
def CHK_A2_042_run_section(ctx):
    prim_present(p2sec(ctx.doc(EC), 2), [
        '`authority`, `run`, `goal`, `acceptance`, `requirements`, `contracts`, `boundary`, `dependencies`, '
        '`shape_policy`, `checks`, `outputs`, `observations`, `preferences`, `rationale`, `workspace_notes`, '
        '`job_notes`, `checkpoint`, `omissions`, `recorded_history`',
        '`run` is mandatory and is never omitted by the budget algorithm',
        'run request: {request_id}', 'run request: (preview)', '- {verb}: {purpose}',
        'worker `refresh`, `checkpoint`, `check`, `release`, reviewer `refresh`, `checkpoint`, `verdict`, `release`',
        'The run section never contains a token or secret.',
        'includes `run` with `mandatory:true`',
    ], 'P2-2 run section')


def manifest_table(text):
    for header, rows in tables(text):
        if header and header[0] == 'Key':
            return rows
    raise CheckFail('dispatch_manifest_v1 table not found')


@check('lines', True)
def CHK_A2_043_manifest_fields(ctx):
    p = p2sec(ctx.doc(EC), 2)
    prim_lines(p, MANIFEST_ROWS, 'P2-2 manifest table')
    keys = 0
    for _line, cells in manifest_table(p):
        keys += len(re.findall(r'`([^`]+)`', cells[0])) if cells else 0
    if keys != 14:
        raise CheckFail('dispatch_manifest_v1 table has %d keys, expected 14' % keys)


@check('lines', True)
def CHK_A2_044_manifest_run_by_request(ctx):
    p = p2sec(ctx.doc(EC), 2)
    prim_lines(p, [r for r in MANIFEST_ROWS if r.startswith('| `run`')], 'P2-2 run row')
    prim_present(p, ['`request` is the claim or takeover request ID'], 'P2-2 run row')


@check('rows_absent', True)
def CHK_A2_045_manifest_no_bytes_no_secret(ctx):
    p = p2sec(ctx.doc(EC), 2)
    prim_present(p, ['There is no `files` array, no `bytes_hex`, no `base`, `observation`, `outputs`, '
                     '`role_output`, `targets`, `assertions`, token or secret key.'], 'P2-2 manifest')
    for _line, cells in manifest_table(p):
        if cells and cells[0].strip('`') in ('files', 'bytes_hex', 'token', 'secret'):
            raise CheckFail('manifest table has a forbidden key row: %s' % cells[0])


@check('present', True)
def CHK_A2_046_projection_ids(ctx):
    ec = ctx.doc(EC)
    prim_present(p2sec(ec, 2), ['No new projection ID exists, and external projections do not apply to dispatch.',
                                '`dispatch_context_v1` (not an admissible external projection input)',
                                'the shipped-owned `worker` and `reviewer` projections produce dispatch packets'],
                 'P2-2 projections')
    prim_present(ctx.doc(XP), ['no new projection ID'], 'XP pointer')
    for name in (EC, XP):
        if re.search(r'(?i)projection ID\W{0,3}`?dispatch', ctx.doc(name)):
            raise CheckFail('%s: a dispatch projection ID is named' % name)


@check('present', True)
def CHK_A2_047_manifest_bound(ctx):
    prim_present(p2sec(ctx.doc(EC), 2), ['Bound `manifest_bytes` 16777216 (EX-8)'], 'P2-2 manifest bound')


# ---- P2-3 work_exec_v1 ----

@check('present', True)
def CHK_A2_050_wexec_fields(ctx):
    prim_present(p2sec(ctx.doc(EC), 3), [
        'The W value has exactly these 24 fields (listed in W\'s presentation order; canonical_v1 sorts the keys '
        'when W is encoded): `versions`, `workspace`, `spec`, `job`, `role`, `root`, `base`, `goal`, `units`, '
        '`milestones`, `constraints`, `prescriptions`, `contracts`, `dependencies`, `facets`, '
        '`pipeline_predecessors`, `questions`, `ownership`, `effective_checks`, `checks`, `toolchains`, `shape`, '
        '`start_facets`, `repair`.',
        'Generation is not a W field',
    ], 'P2-3 field list')


@check('present', True)
def CHK_A2_051_wexec_digest(ctx):
    prim_present(p2sec(ctx.doc(EC), 3), ['`work_fp = H("work_exec_v1", W)`', '16 raw bytes',
                                          '`xxh3_128:`'], 'P2-3 digest recipe')


@check('present', True)
def CHK_A2_052_wexec_start_facets(ctx):
    prim_present(p2sec(ctx.doc(EC), 3), ['of the current publication of the facet\'s producer',
                                         '`{dependency,unavailable:true}` when no publication exists',
                                         'unavailable:true'], 'P2-3 start_facets')


@check('present', True)
def CHK_A2_053_wexec_base(ctx):
    prim_present(p2sec(ctx.doc(EC), 3), ['`base`: always the declared form `{status:"declared",root:<source ID>,'
                                         'label:<base label>}`', 'as execution-design EX-5 fixes'], 'P2-3 base')


@check('present', True)
def CHK_A2_054_wexec_versions(ctx):
    prim_present(p2sec(ctx.doc(EC), 3), [
        '`["declarations_v1","selection_v1","dependencies_v1","exact_bytes_v1","canonical_v1","xxh3_128_v1","work_exec_v1"]`'],
        'P2-3 versions')


@check('present', True)
def CHK_A2_127_wexec_repair_string(ctx):
    ec = ctx.doc(EC)
    prim_present(p2sec(ec, 3), ['as a decimal string; `"0"` before any repair'], 'P2-3 repair')
    prim_present(p2sec(ec, 2), ['the repair revision as a decimal string (`"0"` in R15)'], 'P2-2 inputs repair')


# ---- P2-4 provenance ----

@check('present', True)
def CHK_A2_060_prov_fields(ctx):
    prim_present(p2sec(ctx.doc(EC), 4), [
        '`{version:1,driver:{name,version},trial:{id},view:{id},model:{requested,actual},session:{requested,actual},settings:[{name,value}]}`',
        'Every key of the shape is required'], 'P2-4 document shape')


@check('present', True)
def CHK_A2_061_prov_leaf_rule(ctx):
    prim_present(p2sec(ctx.doc(EC), 4), ['a UTF-8 string of 1 to 1024 bytes without NUL, or exactly the object '
                                         '`{"status":"unknown"}`'], 'P2-4 leaf rule')


@check('present', True)
def CHK_A2_062_prov_absent(ctx):
    prim_present(p2sec(ctx.doc(EC), 4), ['Absence of the file stores no record: `runs.provenance_id` is NULL',
                                         'every field reads as unknown'], 'P2-4 absence')


@check('present', True)
def CHK_A2_063_prov_label(ctx):
    prim_present(p2sec(ctx.doc(EC), 4), ['value record, codec ID `driver_provenance_v1`',
                                         '`{version:1,authority:"driver_assertion_unverified",document:<normalized document>}`'],
                 'P2-4 stored record')


@check('present', True)
def CHK_A2_064_prov_attachment(ctx):
    prim_present(p2sec(ctx.doc(EC), 4), [
        'Attachment points: `claim` and `takeover` (live in R15, option `--provenance`); `release` and `check` are '
        'specified as fields only (their rounds R16 and R17 pin the event reference key); `check-recover` and '
        '`verdict` attach none.'], 'P2-4 attachment points')


@check('present', True)
def CHK_A2_065_prov_bounds(ctx):
    prim_present(p2sec(ctx.doc(EC), 4), [
        'Bounds (EX-8 row "Provenance file"): at most 65536 bytes (`provenance_bytes`), depth 4 (`provenance_depth`), '
        'at most 32 settings (`provenance_settings`), strings at most 1024 bytes (`provenance_string_bytes`)',
        'a stored value that violates them is `ledger_corrupt`'], 'P2-4 bounds')


@check('present', True)
def CHK_A2_066_prov_native_fields(ctx):
    ec = ctx.doc(EC)
    prim_present(p2sec(ec, 5), ['`{ledger,revision,source,job,role,generation,base_label,work_fp,render_fp,preferences}`'],
                 'P2-5 native list')
    prim_present(p2sec(ec, 4), ['is derived from rows and reported by the',
                                'never taken from the driver file'], 'P2-4 native provenance')


# ---- P2-5 history selector ----

@check('present', True)
def CHK_A2_070_event_selector_grammar(ctx):
    prim_present(p2sec(ctx.doc(EC), 5), [
        '`--event` is a canonical positive decimal 1..9223372036854775807; it excludes `--revision`, `--request-id`, '
        '`--at-event`, `--after-event` and `--limit` (any combination is `usage`); it works on format1, format2 and '
        'format3 ledgers; an unknown event is `event_not_found`.',
        '`read_history({selector:"event",id:ID,request_id:"",limit:50})`',
        'The selector set of the history family becomes `page`, `revision`, `request`, `state`, `event`.'],
        'P2-5 event selector')


@check('present', True)
def CHK_A2_071_event_expansion_fields(ctx):
    prim_present(p2sec(ctx.doc(EC), 5), [
        '`data` = `{selector:"event",event,transitions,execution?}`',
        '`execution` is present only for `job_activated`, `run_claimed` and `run_taken_over` events',
        '`job_activated`: `{activation:{id,job,revision}}`',
        '`run_claimed`, `run_taken_over`: `{run:{id,generation,request_id},dispatch:{id,kind,work_fp,render_fp,revision},'
        'packet:{id,bytes,markdown},manifest:{id,value},work:{id,value},provenance:{native,driver}}`'],
        'P2-5 expansion')


@check('present', True)
def CHK_A2_072_event_no_lua_bounded(ctx):
    prim_present(p2sec(ctx.doc(EC), 5), [
        'Records are decoded natively with no Lua, as `--at-event` is.',
        'The whole stdout including the final LF is at most 16777216 bytes',
        '`resource_limit` with `details:{limit:"output_bytes",maximum:16777216}` and empty stdout applies and nothing is truncated'],
        'P2-5 bound')


@check('present', True)
def CHK_A2_073_event_no_secret(ctx):
    p = p2sec(ctx.doc(EC), 5)
    prim_present(p, ['run:{id,generation,request_id}',
                     'The expansion never contains the run token or `runs.secret`; the reader does not select the secret column.'],
                 'P2-5 expansion secret rule')
    for bad in ('run:{id,generation,request_id,token', 'run:{id,generation,request_id,secret'):
        if bad in norm(p):
            raise CheckFail('P2-5: the run expansion carries a token or secret key')


@check('present', True)
def CHK_A2_074_request_id_results(ctx):
    prim_present(p2sec(ctx.doc(EC), 5), [
        'the data keeps `{selector:"request",result}`, no `disposition`',
        'for `load` the existing result; for `activate` the `activate_v1` value of'], 'P2-5 request results')


@check('present', True)
def CHK_A2_075_event_format12(ctx):
    prim_present(p2sec(ctx.doc(EC), 5), ['Format1 and format2 event summaries are unchanged.',
                                         'and is `[]` for format1 and format2 events and for events without transitions'],
                 'P2-5 format1 and format2 events')


@check('present', True)
def CHK_A2_076_token_exposure_list(ctx):
    ec = ctx.doc(EC)
    prim_present(p2sec(ec, 1), ['never in a packet, manifest, `--event` expansion, status other than lookup, '
                                'preview or log'], 'P2-1 token exposure')
    prim_present(ec, [L9_SENTENCE], 'EX-2 amended sentence')


# ---- P2-6 orchestrator ledger form ----

@check('present', True)
def CHK_A2_080_orch_forms(ctx):
    prim_present(p2sec(ctx.doc(EC), 6), [
        '`planar orchestrator --ledger PATH` exists only where the orchestrator is compiled, which is the authoring bundle (D148)',
        'Exactly one form is accepted.',
        'Ledger form: `--ledger` alone; any other combination is `usage`.',
        'The descriptor\'s four options are all `required:false` and the presence rule is native.',
        'the production binary has no `orchestrator` verb and nothing in this section applies to it'], 'P2-6 forms')


@check('present', True)
def CHK_A2_081_orch_live_keys(ctx):
    prim_present(p2sec(ctx.doc(EC), 6), [
        '`live` = `{ledger,revision,last_event,activation,roles,runs,blockers}`',
        '`runs` is the array of active runs sorted by role `{id,role,generation,request_id}`',
        'no token, no secret', '`blockers` is `[]` in R15'], 'P2-6 live keys')


@check('present', True)
def CHK_A2_082_orch_actions(ctx):
    prim_present(p2sec(ctx.doc(EC), 6), [
        'the future list is `refresh`, `checkpoint`, `release`, `recover`, `check`, `check-recover`, `verdict`',
        '`activate`, `status`, `preview`, `claim` and `takeover` are `available`',
        '`with-profile` is never an action',
        'R16 flips `refresh`, `checkpoint`, `release`, `recover` (and `preview --since`), R17 flips `check` and `check-recover`, R18 flips `verdict`.'],
        'P2-6 action labels')


@check('present', True)
def CHK_A2_083_orch_state_section(ctx):
    prim_present(p2sec(ctx.doc(EC), 6), [
        'An additional mandatory section `orchestrator_state` follows `orchestrator_actions` in the ledger form only',
        '(`ready` or `stale`: `claim`; `running`: `status`, or `takeover` when the driver lost the session; '
        '`running` with drift: `status` and, from R16, `refresh`; `blocked`: wait for prerequisites; `done`: none; '
        '`retired`: none)'], 'P2-6 orchestrator_state')


@check('present', True)
def CHK_A2_084_orch_ra7_pointer(ctx):
    prim_present(p2sec(ctx.doc(EC), 6), ['Pointer: `authoring-contract.md` RA-7 is the base definition; this contract '
                                         'extends it by insertion and `authoring-contract.md` is not edited by this round.'],
                 'P2-6 RA-7 pointer')
    if ctx.doc(AU) != ctx.base(AU):
        raise CheckFail('authoring-contract.md differs from the base snapshot')


# ---- P2-7 pointers ----

@check('lines', True)
def CHK_A2_090_cc_abi_rows(ctx):
    cc = ctx.doc(CC)
    prim_lines(cc, ABI_ROWS, 'command-contract ABI rows')
    prim_present(cc, ['only where the orchestrator is compiled, `orchestrator` gains capability `execution_read_v1` '
                      'beside `orchestrator_capture_v1`', '`orchestrator --ledger` exists only there.'],
                 'command-contract orchestrator capability')


@check('present', True)
def CHK_A2_091_cc_option_signatures(ctx):
    prim_present(ctx.doc(CC), [
        'activate text `--ledger!`, `--job!`, `--expect-revision!`, `--expect-job!`, `--request-id!`, `--actor!`',
        'status text `--ledger!`, `--role`, `--request-id`, `--workspace`, `--registry` (`--role` excludes `--request-id` '
        'and nothing else; `--workspace` and `--registry` both or neither)',
        'preview text `--ledger!`, `--role!`',
        'claim text `--ledger!`, `--job!`, `--role!`, `--request-id!`, `--actor!`, `--provenance`',
        'takeover text `--ledger!`, `--job!`, `--role!`, `--request-id!`, `--actor!`, `--provenance` and decimal `--run-id!`',
        'history gains decimal `--event` (excluding `--revision`, `--request-id`, `--at-event`, `--after-event`, `--limit`)',
        'orchestrator text `--ledger`, `--workspace`, `--registry`, `--job` all optional with the native both-or-exactly-one-form rule.',
    ], 'command-contract option signatures')


@check('present', True)
def CHK_A2_092_cc_services_rows(ctx):
    prim_present(ctx.doc(CC), [
        'Services: the five families have the service lists of',
        '`orchestrator` gains `read_execution({})`',
        '`read_history` gains the selector `event` and is called as `read_history({selector:"event",id:ID,request_id:"",limit:50})`'],
        'command-contract services')


@check('present', True)
def CHK_A2_093_cc_opaque_slot(ctx):
    prim_present(ctx.doc(CC), ['Opaque slots gain `provenance`.'], 'command-contract opaque slots')


@check('raw', True)
def CHK_A2_094_root_help_bytes(ctx):
    cc = ctx.doc(CC)
    prim_present_raw(cc, [HISTORY_LINE, PROD_BLOCK, AUTH_BLOCK], 'command-contract root help')


@check('raw', True)
def CHK_A2_095_help_cross_file(ctx):
    ec = ctx.doc(EC)
    for name, text in ((CC, ctx.doc(CC)), (XP, ctx.doc(XP)), (LC, ctx.doc(LC)), ('execution-contract P2-7', p2sec(ec, 7))):
        prim_present_raw(text, [HISTORY_LINE] + PROD_HELP, name + ' help lines')


@check('present', True)
def CHK_A2_096_rc_pointers(ctx):
    rc = ctx.doc(RC)
    prim_present(rc, ['`repair` enters W', 'not a W field', 'Later execution must add those through a versioned recipe',
                      'render stays database-free', L8_SENTENCE,
                      'the rule that takeover must not move W'], 'render-contract pointers')
    prim_present(p2sec(ctx.doc(EC), 3), [L5_SENTENCE], 'P2-3 generation')


@check('present', True)
def CHK_A2_097_lc_services_format3(ctx):
    prim_present(ctx.doc(LC), [
        'format1 and format2 loads keep three services and `{bundle,semantic}`',
        'with a `read_execution({})` row `{version:1,ledger,last_seq,snapshot}` described in the pointer '
        '(not inserted into the services table)',
        '`accept_load` takes the extra key `execution`'], 'load-command-contract pointer')


@check('present', True)
def CHK_A2_098_lg_selector_row(ctx):
    prim_present(ctx.doc(LG), ['the command-surface row text `planar history --ledger <path> --event <id>`',
                               'the data selector `event` joins `page`, `revision`, `request`, `state` in the `data` sentence'],
                 'ledger-contract pointer')


@check('present', True)
def CHK_A2_099_xp_family_and_ids(ctx):
    xp = ctx.doc(XP)
    prim_present(xp, ['the shipped families admitted under `with-profile` are the five of X08 plus `activate`, '
                      '`status`, `preview`, `claim`, `takeover` (ten)',
                      'the six RA01 verbs stay embedded-only (RA-9 item 5)',
                      'Reserved section IDs gain `run` and `orchestrator_state` in the X-PROJ union'],
                 'external-profile pointer')
    if re.search(r'admitted[^.]*`spec-prompt`', norm(xp)):
        raise CheckFail('external-profile pointer admits spec-prompt')


@check('diff', False)
def CHK_A2_100_pointers_narrow_nothing(ctx):
    for name in (CC, RC, LC, LG, XP):
        prim_removed_zero(ctx.base(name), ctx.doc(name), name)
        a, b, ops = line_diff(ctx.base(name), ctx.doc(name))
        added = []
        for tag, i1, i2, j1, j2 in ops:
            added.extend(b[j1:j2])
        for line in added:
            if 'no longer' in line:
                raise CheckFail('%s: added line contains the word combination no longer' % name)
    ed_base = ctx.base(ED)
    ed_new = ctx.doc(ED)
    a, b, ops = line_diff(ed_base, ed_new)
    for tag, i1, i2, j1, j2 in ops:
        if tag != 'replace' or i2 - i1 != 1 or j2 - j1 != 1:
            raise CheckFail('execution-design.md: a change other than the F2 bullet')
        if 'no longer' in b[j1]:
            raise CheckFail('execution-design.md: added line contains no longer')
    for name in (AM, AL):
        if ctx.doc(name) != ctx.base(name):
            raise CheckFail('%s differs from the base snapshot' % name)


@check('present', True)
def CHK_A2_101_cross_file_constants(ctx):
    ec2 = p2sec(ctx.doc(EC), 7)
    cc = ctx.doc(CC)
    xp = ctx.doc(XP)
    constants = [
        ('shipped.activate.v1', cc), ('shipped.status.v1', cc), ('shipped.preview.v1', cc),
        ('shipped.claim.v1', cc), ('shipped.takeover.v1', cc),
        ('activate_v1', cc), ('status_v1', cc), ('preview_v1', cc), ('claim_v1', cc),
        ('execution_commit_v1', cc), ('execution_read_v1', cc), ('draft_capture_v1', cc),
        ('provenance_read_v1', cc),
        ('ten mandatory rows in the production build', cc),
        ('sixteen in a build that compiles the authoring bundle', cc),
        ('`activate`, `status`, `preview`, `claim`, `takeover`', xp),
        ('`orchestrator_state`', xp),
        ('`run` and `orchestrator_state`', xp),
    ]
    if not constants:
        raise CheckFail('constant set is empty')
    for lit, owner in constants:
        if lit not in ec2:
            raise CheckFail('P2-7 lacks constant %s' % short(lit, 60))
        if lit not in owner:
            raise CheckFail('owning pointer lacks constant %s' % short(lit, 60))


@check('traceability', True)
def CHK_A2_110_traceability_rows(ctx):
    trace = heading_block(ctx.doc(EC), '## Traceability')
    required = {
        'R15-2': (['#p2-1-verbs-and-services'], ['D104', 'D111']),
        'R15-3': (['#p2-1-verbs-and-services', '#p2-7-command-render-load-ledger-and-external-profile-pointers'], ['D136']),
        'R15-9': (['#part-1-amendments-r15-a2'], ['D118']),
        'E1': (['#p2-1-verbs-and-services'], ['D105', 'D106']),
        'E2': (['#p2-1-verbs-and-services'], ['D104']),
        'E3': (['#p2-1-verbs-and-services', '#p2-6-orchestrator-live-form'], ['D108', 'D142']),
        'E5': (['#p2-3-work_exec_v1'], ['D120']),
        'E11': (['#p2-1-verbs-and-services'], ['D118']),
        'E16': (['#p2-1-verbs-and-services'], ['D114']),
        'R15-4': (['#p2-1-verbs-and-services', '#acceptance-checklist'], ['D107', 'D114']),
        'R15-5': (['#p2-1-verbs-and-services'], ['D107', 'D114']),
        'R15-6': (['#p2-2-dispatch-projection-and-dispatch_manifest_v1', '#p2-3-work_exec_v1'], []),
        'R15-7': (['#p2-1-verbs-and-services'], ['D115', 'D116']),
        'R15-8': (['#p2-4-provenance-driver_provenance_v1'], ['D121', 'D122', 'D123', 'D146']),
        'R15-10': (['#p2-5-history-event-selector-and-request-results'], []),
        'R15-11': (['#p2-6-orchestrator-live-form'], ['D128', 'D129', 'D130', 'D148']),
        'A29': (['#p2-5-history-event-selector-and-request-results'], []),
        'A34': (['#p2-4-provenance-driver_provenance_v1'], []),
        'E6': (['#p2-4-provenance-driver_provenance_v1'], []),
    }
    for label, (anchors, ids) in required.items():
        prim_rows_once(trace, label, anchors + [''], 'Traceability')
        line = [l for h, rows in tables(trace) for l, c in rows if c and c[0].split()[:1] == [label]][0]
        for dec in ids:
            if not re.search(r'\b%s\b' % dec, line):
                raise CheckFail('Traceability %s lacks decision %s' % (label, dec))
        for anc in anchors:
            if anc not in line:
                raise CheckFail('Traceability %s lacks anchor %s' % (label, anc))
    part1 = [l for h, rows in tables(trace) for l, c in rows if c and c[0].startswith('Part 1 amendments')]
    if len(part1) != 1 or '#part-1-amendments-r15-a2' not in part1[0]:
        raise CheckFail('Traceability has no single Part 1 amendments row linking #part-1-amendments-r15-a2')
    r10 = [l for h, rows in tables(trace) for l, c in rows if c and c[0].split()[:1] == ['R15-10']][0]
    prim_present(r10, ['event summary', '`--at-event`', '`--event`'], 'Traceability R15-10')


@check('decisions', True)
def CHK_A2_111_decisions_rows(ctx):
    dec = heading_block(ctx.doc(EC), '## Decisions table')
    prim_tags(dec, DECISION_TAGS, 'Decisions table')
    prim_decision_cells(dec, NEW_DECISION_CELLS, 'Decisions table')


@check('present', True)
def CHK_A2_112_checklist_items(ctx):
    prim_present(heading_block(ctx.doc(EC), '## Acceptance checklist'), [
        'status explain separation (A20/A22)', '`claim` retry bytes (A23)', 'history `--event` (A29)',
        'provenance unknown rule (A34)', 'F3 (render-level insertion-order byte test)',
        'F4 (mandatory-file identity test)', 'F5 (change between listing and first read)',
        'F6 (probe set stays five)', 'F7 (envelope comparisons to literals)',
        'F8 (no-ledger snapshot beyond the scratch root)'], 'Acceptance checklist')


@check('diff_one', True)
def CHK_A2_113_design_f2_only(ctx):
    old, new = prim_one_changed(ctx.base(ED), ctx.doc(ED), 'execution-design.md')
    if 'capture races' not in old:
        raise CheckFail('the changed execution-design line is not the F2 bullet')
    if not re.match(r'^- ', new) or 'RA01' not in new:
        raise CheckFail('the F2 bullet does not mention RA01')
    if not re.search(r'tests?/[\w./-]+|[\w.-]+\.t\.cpp', new):
        raise CheckFail('the F2 bullet does not name a test file')


@check('present', True)
def CHK_A2_114_pointer_item7(ctx):
    ec = ctx.doc(EC)
    prim_absent(ec, ['Not made by this round', '#part-2-placeholders-r15-a2'], EC)
    prim_present(heading_block(ec, '## Pointer amendments'), ['recorded in P2-7'], 'Pointer amendments item 7')


@check('present', True)
def CHK_A2_115_reserved_verbs(ctx):
    p = p2sec(ctx.doc(EC), 1)
    prim_present(p, ['The R16 to R19 verbs keep their E1 rows as reserved names and are unknown commands (`usage`) until their rounds.',
                     'R16 adds `--since`; not in R15'], 'P2-1 reserved verbs')
    for line in p.split('\n'):
        if re.match(r'^\|\s*`(refresh|checkpoint|release|recover|check|check-recover|verdict)`\s*\|', line):
            raise CheckFail('P2-1 has an option or precondition row for a reserved verb')


@check('local', False)
def CHK_A2_116_no_local_paths(ctx):
    control = 'see /Users/example/planar for the root'
    if not re.search(r'/Users/|/home/|C:\\', control):
        raise CheckFail('positive control was not flagged; the detector is broken')
    for name in (EC, CC, RC, LC, LG, XP, ED):
        prim_local_paths(ctx.doc(name), name)


# ---- P2-1 closed details, lock mapping, reads ----

@check('present', True)
def CHK_A2_117_closed_details_rule(ctx):
    p = p2sec(ctx.doc(EC), 1)
    prim_present(p, ['details of EX-7: `blocked`, `running`, `done`, `retired`',
                     'which is one of `blocked`, `running`, `done`, `retired`',
                     '`claim_conflict` never carries `ready` or `stale`'], 'P2-1 closed details')


@check('present', True)
def CHK_A2_118_lock_mismatch_mapping(ctx):
    prim_present(p2sec(ctx.doc(EC), 1), [
        'only when the locked role is taken by another run (`running`) or is in another closed-set state '
        '(`blocked`, `done`, `retired`)',
        'When the locked state is `ready` or `stale` (for example a stale role turned ready by a concurrent structural load) '
        'it is `state_conflict`, as is every other failed member'], 'P2-1 lock mapping')


@check('present', True)
def CHK_A2_119_takeover_unknown_run(ctx):
    p = p2sec(ctx.doc(EC), 1)
    prim_present(p, ['At step (8) the named run must be a run of that job and role, otherwise `state_conflict` '
                     '(no run state exists, so none is invented).'], 'P2-1 takeover unknown run')
    prim_absent(p, ['names none', 'or `fenced` when it names none'], 'P2-1 unknown run')


@check('present', True)
def CHK_A2_120_event_summary_format3(ctx):
    prim_present(p2sec(ctx.doc(EC), 5), [
        '`job` and `role` are IDs, and each is the empty string when the column is NULL',
        '`generation_id` and `run_id` are canonical decimal strings, and each is `"none"` when the column is NULL',
        'that rule covers numeric database IDs only'], 'P2-5 event summary')


@check('present', True)
def CHK_A2_121_at_event_state_format3(ctx):
    prim_present(p2sec(ctx.doc(EC), 5), [
        '`state` is `{active_revision,active_job,activation,roles,runs}`',
        '`roles` are the roles of the active job at that event, sorted by role ID, each `{role,generation,state,drifted,run}`',
        '`runs` are the runs active at that event, sorted by id, each `{id,role,generation}`',
        'no token or secret appears'], 'P2-5 at-event state')


@check('present', True)
def CHK_A2_122_history_format12_unchanged(ctx):
    prim_present(p2sec(ctx.doc(EC), 5), ['Format1 and format2 `state` data stays exactly `{active_revision}`.',
                                         'Format1 and format2 event summaries are unchanged.'],
                 'P2-5 format1 and format2')


@check('present', True)
def CHK_A2_123_event_native_provenance_always(ctx):
    prim_present(p2sec(ctx.doc(EC), 5), ['`provenance` is always present for these two event types.',
                                         'without one, `id` is `"none"`, `authority` is `"none"` and `document` is the all-unknown form'],
                 'P2-5 provenance rule')


@check('present', True)
def CHK_A2_124_exempt_read_formats(ctx):
    prim_present(p2sec(ctx.doc(EC), 1), [
        'which applies to every form of `status` (including `status --role`) and to `orchestrator --ledger`',
        'On a format1 or format2 ledger every form of `status` is `unsupported_ledger_version` (D142).',
        'its format refusal is step (2), before the binding'], 'P2-1 exempt read formats')


@check('messages', True)
def CHK_A2_125_messages_amendments_reasoned(ctx):
    ec = ctx.doc(EC)
    rc = find_msg_row(ec, '`revision_conflict`')
    if not rc or 'The format3 load compare-and-swap and the `--expect-revision` check of `activate`' not in rc[0][1][3]:
        raise CheckFail('revision_conflict sites cell lacks the A-8 text')
    for field, label in (('selection.job', 'Unknown selected job'), ('selection.role', 'Unknown selected role')):
        hits = find_msg_row(ec, '`reference`', label)
        if not hits or field not in hits[0][1][3]:
            raise CheckFail('reference row %s lacks field %s' % (label, field))
    sc = find_msg_row(ec, '`schema`')
    if not sc or 'provenance' not in sc[0][1][3]:
        raise CheckFail('schema row lacks the provenance field (A-10)')
    st = find_msg_row(ec, '`state_conflict`')
    if not st or 'also, at the snapshot before the lock, an `activate` `--expect-job NONE` mismatch (a job is active) '\
            'and a `takeover` `--run-id` that names no run of the job and role' not in st[0][1][0]:
        raise CheckFail('state_conflict refusal-site cell lacks the A-13 text')
    am = heading_block(ec, PART1_AM)
    rows = {c[0]: c for h, rows_ in tables(am) for _l, c in rows_ if c}
    if 'R15-A1 I66' not in rows.get('A-8', [''] * 4)[3]:
        raise CheckFail('A-8 does not cite R15-A1 I66')
    if '(D149)' not in rows.get('A-13', [''] * 4)[3]:
        raise CheckFail('A-13 does not cite D149')
    for n in ('A-9', 'A-10', 'A-13'):
        if n not in rows:
            raise CheckFail('amendment %s missing' % n)


@check('present', True)
def CHK_A2_134_prov_input_handling(ctx):
    ec = ctx.doc(EC)
    p = p2sec(ec, 4)
    prim_lines(p, MAP_ROWS, 'P2-4 provenance input table')
    prim_present(p, ['Check order, first failure wins:', '(1) the captured size exceeds 65536 bytes, `provenance_bytes`, before any parse',
                     '(5) any string longer than 1024 bytes, `provenance_string_bytes`, so a 1025-byte leaf is `resource_limit` and never `schema`',
                     'The `schema` field is `provenance` alone for a grammar violation',
                     'so an early replay never reads the file',
                     'The provenance refusals are step (9) of the refusal order in',
                     'after the role-state refusals and before the commit preconditions'], 'P2-4 input handling')


@check('present', True)
def CHK_A2_135_run_section_purposes(ctx):
    p = p2sec(ctx.doc(EC), 2)
    prim_present(p, ['`## run`, a blank line, the body, one structural LF',
                     'Actions not shipped in this build are unavailable.',
                     'worker `refresh`, `checkpoint`, `check`, `release`, reviewer `refresh`, `checkpoint`, `verdict`, `release`',
                     'each at most 200 bytes'], 'P2-2 run section pins')
    for row in PURPOSE_ROWS:
        cells = split_row(row)
        if len(cells) != 2:
            raise CheckFail('purpose table row malformed')
        purpose = cells[1].strip('`')
        if '`' + purpose + '`' not in p:
            raise CheckFail('purpose text missing for %s' % cells[0].strip('`'))


@check('present', True)
def CHK_A2_136_build_variant_bytes_counts(ctx):
    ec = ctx.doc(EC)
    prim_present(ctx.doc(CC), ['ten mandatory rows in the production build (the current five plus the five R15 rows) '
                               'and sixteen in a build that compiles the authoring bundle'], 'command-contract counts')
    prim_present(p2sec(ec, 6), ['exists only where the orchestrator is compiled, which is the authoring bundle (D148)'],
                 'P2-6 build variant')
    prim_present(p2sec(ec, 7), ['ten mandatory rows in the production build',
                                'sixteen in a build that compiles the authoring bundle'], 'P2-7 counts')


@check('present', True)
def CHK_A2_137_state_conflict_snapshot_sites(ctx):
    ec = ctx.doc(EC)
    p = p2sec(ec, 1)
    prim_present(p, ['`--expect-job NONE` while a job is active is `state_conflict` at the snapshot, before the lock, '
                     'a site named by amendment A-13',
                     'Driver-visible limit (D149): a same-ID retry cannot succeed after the `state_conflict` of `activate` '
                     '`--expect-job NONE` while a job is active, or after the `state_conflict` of a `takeover` `--run-id` that names no run',
                     'An `activate` `--expect-job JOB` mismatch is `no_active_job` for the same reason and carries the same limit.'],
                 'P2-1 snapshot sites')
    st = find_msg_row(ec, '`state_conflict`')
    if not st or 'also, at the snapshot before the lock, an `activate` `--expect-job NONE` mismatch (a job is active) '\
            'and a `takeover` `--run-id` that names no run of the job and role' not in st[0][1][0]:
        raise CheckFail('Messages state_conflict cell lacks the snapshot sites')


@check('present', True)
def CHK_A2_138_status_roles_retired_kind(ctx):
    ec = ctx.doc(EC)
    prim_present(p2sec(ec, 1), ['taken from the `role_structure_v1` record of the role\'s current generation '
                                '(`snapshot.roles[].generation.structure`), never from the interpretation',
                                'a retired role that the active revision no longer declares stays listed with state `retired`'],
                 'P2-1 status roles')
    prim_present(p2sec(ec, 1), ['`kind` equals its `kind`; retired roles keep their rows'], 'P2-1 snapshot roles')


@check('present', True)
def CHK_A2_139_reserved_op_preconditions_deferred(ctx):
    ec = ctx.doc(EC)
    p = p2sec(ec, 1)
    prim_present(p, ['not defined in this part (D150)', 'P2-1 defines the member sets of `claim` and `takeover` only'],
                 'P2-1 reserved operations')
    prim_present(heading_block(ec, PART1_AM), ['are defined by the R16 to R18 contract rounds', '(D150)'],
                 'Part 1 amendments A-4')
    if re.search(r'\|\s*`release`\s*\|[^\n]*`request_unused`', p):
        raise CheckFail('P2-1 writes a member list for a reserved operation')


@check('present', True)
def CHK_A2_140_native_enforcement_expected(ctx):
    p = p2sec(ctx.doc(EC), 1)
    prim_present(p, ['Native refuses a `claim` change whose target before-state in the snapshot is not `ready` or `stale`, '
                     'and a `takeover` change whose named run is not the target\'s active run.',
                     'A handler that sends any of these changes is refused with code `internal`',
                     'Native validates `expected` as an exact fixed set',
                     'refuses a non-claimable claim target or a takeover whose run is not the active run'],
                 'P2-1 enforcement')


@check('present', True)
def CHK_A2_141_role_reference_reads(ctx):
    prim_present(p2sec(ctx.doc(EC), 1), [
        '"The role exists in the job" means declared by the active revision\'s recorded interpretation for the active job.',
        'A role that `status_v1` lists as `retired` but that the active revision no longer declares has no current W',
        '`status --role R` returns `role_detail` with `current` the string `none`',
        '`preview --role R` is `reference` with the same message and field',
        '(or the string `none` for a retired role the active revision no longer declares, D152)'], 'P2-1 role references')


@check('present', True)
def CHK_A2_142_request_id_output_bound(ctx):
    prim_present(p2sec(ctx.doc(EC), 5), [
        'a format3 `claim` or `takeover` result assembles the packet (up to 1 MiB) and the manifest (up to 16 MiB), '
        'so it can exceed the ceiling',
        'the existing refusal `resource_limit` with `details:{limit:"output_bytes",maximum:16777216}` and empty stdout applies with no truncation',
        'The records are decoded natively with no Lua.'], 'P2-5 request-id bound')


@check('present', True)
def CHK_A2_143_section_ids_every_build(ctx):
    ec = ctx.doc(EC)
    prim_present(ctx.doc(XP), ['in every build including production'], 'external-profile section IDs')
    prim_present(p2sec(ec, 6), ['applies in every build, production included'], 'P2-6 section IDs')
    prim_present(p2sec(ec, 7), ['in every build including production'], 'P2-7 section IDs')


# ---- R15-A2-C01 checks: preview manifest, dispatch_v1, EX-4 wording, token sentence, pointers, traceability ----

@check('lines', True)
def CHK_A2_144_preview_manifest_run(ctx):
    p = p2sec(ctx.doc(EC), 2)
    prim_lines(p, [L1_RUN_ROW], 'P2-2 preview manifest run row')
    keys = 0
    for _line, cells in manifest_table(p):
        keys += len(re.findall(r'`([^`]+)`', cells[0])) if cells else 0
    if keys != 14:
        raise CheckFail('dispatch_manifest_v1 table has %d keys, expected 14' % keys)
    run_rows = [line for line, cells in manifest_table(p) if cells and cells[0] == '`run`']
    if len(run_rows) != 1:
        raise CheckFail('dispatch_manifest_v1 table has %d run rows, expected 1' % len(run_rows))
    prim_present(run_rows[0], ['kind:"preview"', 'with no `request` key'], 'P2-2 preview manifest run row')


@check('present', True)
def CHK_A2_145_preview_manifest_same_keys(ctx):
    ec = ctx.doc(EC)
    p2 = p2sec(ec, 2)
    paras = [x for x in re.split(r'\n\s*\n', p2) if 'A preview manifest has the same 14 keys' in norm(x)]
    if len(paras) != 1:
        raise CheckFail('P2-2 preview manifest paragraph occurs %d times' % len(paras))
    prim_present(paras[0], [L2_PARA], 'P2-2 preview manifest paragraph')
    prim_absent(p2, ['always differ'], 'P2-2 preview manifest')
    prim_present(p2sec(ec, 1), [L2B], 'P2-1 preview_v1 definition')
    prim_absent(ec, ['only `run` differs', 'of the same role at the same state'], 'execution contract')


@check('present', True)
def CHK_A2_146_preview_run_spelling_no_collision(ctx):
    ec = ctx.doc(EC)
    tokens = [t for t in re.findall(r'`run request: ([^`]+)`', p2sec(ec, 2)) if t != '{request_id}']
    if not tokens:
        raise CheckFail('no preview run token')
    for token in tokens:
        if re.fullmatch(r'[A-Za-z0-9_.:-]{1,128}', token) is not None:
            raise CheckFail('preview run token `%s` is a legal request ID spelling' % token)
    prim_absent(ec, ['run request: none'], 'execution contract')
    prim_present(p2sec(ec, 2), [L3_SENTENCE], 'P2-2 preview spelling sentence')


@check('present', True)
def CHK_A2_147_dispatch_v1_defined(ctx):
    prim_present(p2sec(ctx.doc(EC), 2), [L4_BULLET], 'P2-2 dispatch_v1 bullet')


@check('present', True)
def CHK_A2_148_render_pointer_dispatch_v1(ctx):
    prim_present(ctx.doc(RC), [L8_IDENTIFIER], 'render-contract identifier pointer')
    prim_present(p2sec(ctx.doc(EC), 7), [L8_IDENTIFIER], 'P2-7 render bullet')


@check('present', True)
def CHK_A2_149_no_identifier_carries_data(ctx):
    ec = ctx.doc(EC)
    rc = ctx.doc(RC)
    # Presence first: the absence test below must not pass on a text that lacks the sentence.
    prim_present(p2sec(ec, 3), [L5_SENTENCE], 'P2-3 generation bullet')
    for label, text in ((EC, ec), (RC, rc)):
        m = re.search(r'carried by `[a-z0-9_]+_v1', norm(text))
        if m:
            raise CheckFail('%s: an identifier is said to carry data: %s' % (label, m.group(0)))


@check('present', True)
def CHK_A2_150_generation_is_manifest_key(ctx):
    ec = ctx.doc(EC)
    prim_present(p2sec(ec, 3), [L5_SENTENCE], 'P2-3 generation bullet')
    prim_present(p2sec(ec, 7), [L6_SENTENCE], 'P2-7 render bullet')
    prim_present(' '.join(decision_row_cells(ec, 'D120')), [L7_SENTENCE], 'Decisions row D120')
    prim_present(ctx.doc(RC), [L8_SENTENCE], 'render-contract pointer')


@check('present', True)
def CHK_A2_151_ex4_none_statement_p25(ctx):
    ec = ctx.doc(EC)
    prim_present(p2sec(ec, 5), [S_STATEMENT], 'P2-5 event summary')
    prim_absent(ec, ['EX-4 rejects'], 'execution contract')
    prim_present(p2sec(ec, 1), ['`none` is a legal job ID and stays an ordinary job ID'], 'P2-1 none spelling')


@check('present', True)
def CHK_A2_152_ex4_none_statement_d143(ctx):
    row = ' '.join(decision_row_cells(ctx.doc(EC), 'D143'))
    prim_present(row, [S_STATEMENT], 'Decisions row D143')
    prim_absent(row, ['EX-4 rejects'], 'Decisions row D143')


@check('present', True)
def CHK_A2_153_a3_token_sentence_split(ctx):
    ec = ctx.doc(EC)
    prim_present(ec, [L9_SENTENCE], 'run token sentence')
    prim_absent(ec, [L9_OLD_FRAGMENT], 'execution contract')


@check('paragraphs', True)
def CHK_A2_154_load_pointer_help_order(ctx):
    paras = re.split(r'\n\s*\n', ctx.doc(LC))
    hits = [i for i, p in enumerate(paras) if 'Pointer (R15-A2): on format3 only' in norm(p)]
    if len(hits) != 1:
        raise CheckFail('load pointer paragraph occurs %d times' % len(hits))
    i = hits[0]
    para = norm(paras[i])
    if not para.endswith('and are:'):
        raise CheckFail('load pointer paragraph does not end with the colon before the help block')
    if i + 1 >= len(paras) or not paras[i + 1].lstrip().startswith('```text'):
        raise CheckFail('the help block fence does not follow the load pointer paragraph')
    gate = para.find('until the R15 integrated gate')
    helps = para.find('The production root help lines')
    if gate < 0 or helps < 0 or gate > helps:
        raise CheckFail('the R15 integrated gate sentence does not precede the help lines in the pointer paragraph')


@check('present', True)
def CHK_A2_155_checklist_ra01c(ctx):
    items = [l for l in heading_block(ctx.doc(EC), '## Acceptance checklist').split('\n') if re.match(r'^- F[3-8] ', l)]
    ids = sorted(re.match(r'^- F([3-8]) ', l).group(1) for l in items)
    if len(items) != 6 or ids != ['3', '4', '5', '6', '7', '8']:
        raise CheckFail('Acceptance checklist has %d F3 to F8 items, expected exactly one each of F3 to F8' % len(items))
    for l in items:
        if 'RA01-C coverage gaps' not in norm(l):
            raise CheckFail('checklist item %s does not say RA01-C coverage gaps' % l[:6])
        if 'R15-A1 coverage gaps' in norm(l):
            raise CheckFail('checklist item %s says R15-A1 coverage gaps' % l[:6])


@check('traceability', True)
def CHK_A2_156_trace_r15_4_claim(ctx):
    ec = ctx.doc(EC)
    rows = [line for _h, trows in tables(heading_block(ec, '## Traceability')) for line, cells in trows
            if cells and cells[0] == 'R15-4 Claim']
    if len(rows) != 1:
        raise CheckFail('Traceability row R15-4 Claim occurs %d times' % len(rows))
    prim_present(rows[0], ['claim_v1', '#p2-1-verbs-and-services', '#acceptance-checklist', 'D107', 'D114'],
                 'Traceability R15-4 Claim')
    prim_absent(ec, ['R15-4 Claim and takeover precondition sets'], 'execution contract')


@check('traceability', True)
def CHK_A2_157_trace_r15_5_takeover(ctx):
    ec = ctx.doc(EC)
    rows = [line for _h, trows in tables(heading_block(ec, '## Traceability')) for line, cells in trows
            if cells and cells[0] == 'R15-5 Takeover']
    if len(rows) != 1:
        raise CheckFail('Traceability row R15-5 Takeover occurs %d times' % len(rows))
    prim_present(rows[0], ['#p2-1-verbs-and-services', 'D107', 'D114'], 'Traceability R15-5 Takeover')
    prim_absent(ec, ['R15-5 Lock-time mismatch mapping for claim and takeover'], 'execution contract')


@check('present', True)
def CHK_A2_158_lead_sentence_listed(ctx):
    ec = ctx.doc(EC)
    prim_present(heading_block(ec, '## Pointer amendments'), [LEAD_SENTENCE], 'Pointer amendments lead sentence')
    header, cells = amendment_row(ec, 'A-11')
    prim_present(amendment_cell(header, cells, 'Location', 'A-11'), ['and the lead sentence (base line 966)'],
                 'A-11 Location cell')
    prim_present(amendment_cell(header, cells, 'Change', 'A-11'),
                 ['"The amendments are insertions only" to "The amendments are insertions only, except the reworded F2 '
                  'bullet of item 8"'], 'A-11 Change cell')


@check('present', True)
def CHK_A2_159_a2_line_cite(ctx):
    ec = ctx.doc(EC)
    header, cells = amendment_row(ec, 'A-2')
    prim_present(amendment_cell(header, cells, 'Location', 'A-2'), ['Lines 518 and 527 to 528'], 'A-2 Location cell')
    prim_absent(ec, ['Lines 518, 527 and 528'], 'execution contract')


@check('messages', False)
def CHK_A2_160_allowed_messages_extraction(ctx):
    allowed = allowed_messages(ctx.base(name) for name in TEN)
    for want in ('Unknown selected job', 'Unknown selected role', 'Run is not current'):
        if want not in allowed:
            raise CheckFail('allowed message set lacks %s' % want)
    for header, trows in tables(heading_block(ctx.base(EC), '## Messages')):
        idx = 2 if header and header[0] == 'Refusal site' else 1
        for _l, cells in trows:
            if len(cells) <= idx:
                continue
            for msg in re.findall(r'`([^`]+)`', cells[idx]):
                if msg not in allowed:
                    raise CheckFail('message cell string not extracted: %s' % msg)


# ---- P2-1 remaining rows, Part 1 amendments ----

@check('present', True)
def CHK_A2_128_xp_consequences(ctx):
    prim_present(ctx.doc(XP), ['valid today, refused after R15',
                               'cuts external command capacity by five (59 to 54)',
                               '`with-profile P history --event ID` is `usage` today and accepted after R15',
                               'the with-profile completeness rule moves from five to ten',
                               'the X-LIMIT table is unedited'], 'external-profile consequences')


@check('present', True)
def CHK_A2_129_authoring_ref_amendments(ctx):
    am = heading_block(ctx.doc(EC), '## Pointer amendments')
    prim_present(am, ['RA-9', 'valid until the R15 integrated gate',
                      'R15 later adds a ledger form by its own contract amendment',
                      'D134 option A', 'RA02', '`execution-design.md:499`',
                      'item 1 (the embedded probe holds only the five X08 families) reads ten families',
                      'reads ten, ten and sixteen (authoring bundle)',
                      'so external capacity is unchanged) reads capacity 54',
                      '`authoring-contract.md`, `acceptance-matrix.md` and `v5-alignment.md` are not edited by this round'],
                 'Pointer amendments authoring items')


@check('paragraphs', True)
def CHK_A2_130_pointers_marked_not_shipped(ctx):
    gate = 'until the R15 integrated gate'
    for name, minimum in ((CC, 1), (RC, 3), (LC, 1), (LG, 1), (XP, 1)):
        prim_paragraph_labels(ctx.doc(name), 'Pointer (R15-A2):', minimum, gate, name)


@check('present', True)
def CHK_A2_131_cc_decimal_and_history_limit(ctx):
    cc = ctx.doc(CC)
    prim_present(cc, ['extends to `run-id` and `event`',
                      '`read_history({selector:"event",id:ID,request_id:"",limit:50})`',
                      'the event selector ignores it, no `limit:1`'], 'command-contract history limit')
    if re.search(r'limit:1[,}]', norm(cc)):
        raise CheckFail('command-contract pointer states limit:1')


@check('present', True)
def CHK_A2_132_lc_capability_format3(ctx):
    prim_present(ctx.doc(LC), ['the load family\'s capability set stays `accept_load_v1`; it covers '
                               '`read_execution({})` on format3'], 'load-command-contract capability')


@check('base', False)
def CHK_A2_133_base_snapshot_complete(ctx):
    if not os.path.isdir(ctx.base_dir):
        raise CheckFail('base directory missing')
    names = sorted(os.listdir(ctx.base_dir))
    if names != sorted(TEN):
        raise CheckFail('base/ does not hold exactly the ten snapshot files')
    for name in TEN:
        want = ctx.git_bytes('docs/vernacular/' + name)
        with open(os.path.join(ctx.base_dir, name), 'rb') as fh:
            have = fh.read()
        if have != want:
            raise CheckFail('base/%s differs from the recorded base commit' % name)




# ---- self-test: one good and one bad fixture per check family ----

FIXTURES = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'fixtures')


def read_fixture(name):
    with open(os.path.join(FIXTURES, name), encoding='utf-8') as fh:
        return fh.read()


def pair(text):
    if '\n---NEW---\n' not in text:
        raise CheckFail('fixture lacks the base and new separator')
    base, new = text.split('\n---NEW---\n', 1)
    return base, new


def _names(text):
    return member_names(text)


def _allowed_runner(text):
    if 'Unknown selected job' not in allowed_messages([text]):
        raise CheckFail('fixture: Unknown selected job is not extracted as an allowed message')


SELFTEST = [
    ('absent', lambda t: prim_absent(t, ['Not yet written'], 'fixture')),
    ('present', lambda t: prim_present(t, ['Part 2 is normative'], 'fixture')),
    ('raw', lambda t: prim_present_raw(t, [HISTORY_LINE], 'fixture')),
    ('lines', lambda t: prim_lines(t, [OPT_ROWS[0]], 'fixture')),
    ('headings', lambda t: prim_headings(t, [P2_TITLE, P2[1], P2[2]], 'fixture')),
    ('ordered', lambda t: prim_ordered(t, ['(1) `usage`', '(2) `format`', '(3) `replay`'], 'fixture')),
    ('names', lambda t: prim_names_subset(_names(t), EIGHT, 'fixture')),
    ('codes', lambda t: prim_codes(t, WHITELIST, 'fixture')),
    ('messages', lambda t: prim_messages(t, {'Unknown selected job'}, 'fixture')),
    ('frozen', lambda t: prim_frozen(pair(t)[0].split('\n'), pair(t)[1].split('\n'), 'fixture')),
    ('removed', lambda t: prim_removed_zero(pair(t)[0], pair(t)[1], 'fixture')),
    ('allow', lambda t: prim_allow_changed(pair(t)[0], pair(t)[1], set(), 'fixture')),
    ('one_change', lambda t: prim_one_changed(pair(t)[0], pair(t)[1], 'fixture')),
    ('msg_table', lambda t: prim_msg_table(pair(t)[0], pair(t)[1], 'fixture')),
    ('local', lambda t: prim_local_paths(t, 'fixture')),
    ('rows', lambda t: prim_rows_once(t, 'R15-2', ['#p2-1-verbs-and-services'], 'fixture')),
    ('tags', lambda t: prim_tags(t, {'D104': DELEGATED}, 'fixture')),
    ('paragraphs', lambda t: prim_paragraph_labels(t, 'Pointer (R15-A2):', 1,
                                                    'until the R15 integrated gate', 'fixture')),
    ('allowed', _allowed_runner),
]


def run_selftest(verbose):
    problems = []
    for family, runner in SELFTEST:
        for kind, want_pass in (('good', True), ('bad', False)):
            fname = '%s.%s.txt' % (family, kind)
            try:
                text = read_fixture(fname)
            except OSError:
                problems.append('%s: fixture %s missing' % (family, fname))
                continue
            try:
                runner(text)
                got = True
            except CheckFail:
                got = False
            except Exception as exc:  # the runner itself must not crash
                problems.append('%s %s: runner error %s' % (family, kind, type(exc).__name__))
                continue
            if got != want_pass:
                problems.append('%s %s fixture gave %s' % (family, kind, 'PASS' if got else 'FAIL'))
            elif verbose:
                print('selftest %s %s: %s' % (family, kind, 'PASS' if got else 'FAIL'))
    return problems


@check('selftest', False)
def CHK_A2_000_selftest_positive_negative(ctx):
    problems = run_selftest(False)
    if problems:
        raise CheckFail('; '.join(problems))


# ---- runner ----

def _check_number(name):
    return name.split(' ')[0].replace('CHK-A2-', '')


def main(argv=None):
    ap = argparse.ArgumentParser(description='R15-A2 contract checker')
    here = os.path.dirname(os.path.abspath(__file__))
    ap.add_argument('--repo', default=os.getcwd(), help='root holding docs/vernacular/*.md')
    ap.add_argument('--base', default=os.path.join(here, '..', 'base'), help='frozen snapshot directory')
    ap.add_argument('--git-repo', default=None, help='git repository for git show')
    ap.add_argument('--base-commit', default=None, help='full base commit SHA')
    ap.add_argument('--selftest', action='store_true', help='run only the self-test check')
    ap.add_argument('--only', default=None, help='run one check by number or name')
    args = ap.parse_args(argv)

    ctx = Ctx(args.repo, args.base, args.git_repo, args.base_commit)
    selected = CHECKS
    if args.selftest:
        selected = [c for c in CHECKS if c[0].startswith('CHK-A2-000 ')]
    if args.only:
        selected = [c for c in CHECKS if args.only in (c[0], _check_number(c[0]), c[0].split(' ', 1)[1])]
        if not selected:
            print('FAIL %s: no such check' % args.only)
            print('SUMMARY pass=0 fail=1')
            return 1

    passed = 0
    failed = 0
    for name, fn, _expect_red, _family in selected:
        try:
            fn(ctx)
        except CheckFail as exc:
            print('FAIL %s: %s' % (name, exc))
            failed += 1
            continue
        except Exception as exc:  # per-check boundary: never a traceback
            print('FAIL %s: internal error (%s)' % (name, type(exc).__name__))
            failed += 1
            continue
        print('PASS %s' % name)
        passed += 1
    print('SUMMARY pass=%d fail=%d' % (passed, failed))
    return 1 if failed else 0


if __name__ == '__main__':
    try:
        sys.stdout.reconfigure(errors='backslashreplace')
    except AttributeError:
        pass
    sys.exit(main())
