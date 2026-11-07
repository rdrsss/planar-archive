#!/usr/bin/env python3
"""REC preflight of round V5-02-R15-A1-C03 (DECISIONS D33, INVARIANTS C3-PF).

Stdlib only, read only. Prints `PASS <path>` or `MISSING <path>` for every
input REC needs (repository-relative paths) and exits non-zero when any line
is MISSING. REC runs it first and stops without editing when it fails.

Usage: rec_preflight_c03.py --repo <root> [--evidence <dir>]
"""
import argparse
import os
import sys

E_REL = 'tests/native/v5-02-r15-a1-c01-evidence'

FIXED = [
    'probes/PROBES.md',
    'probes/mutants.tsv',
    'probes/raw/baseline-c02-default.log',
    'probes/raw/baseline-c02-audit.log',
    'probes/raw/baseline-c02-audit-closed.log',
    'green/checks.txt',
    'green/sql-final.txt',
    'green/linkcheck-c02.txt',
    'green/redaction-c02.txt',
    'green/identity-c02.txt',
    'green/messages-diff-c03.txt',
    'green/audit-pre-rec-c02.txt',
    'green/staging-candidates-c03.txt',
    'green/record_asserts_c03.py',
    'green/record-asserts-red-c03.txt',
    'raw/pre-rec-record-c03.md',
    'raw/ex_checks-c02-preedit-c03.py',
]


def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument('--repo', required=True)
    ap.add_argument('--evidence')
    args = ap.parse_args(argv)
    repo = os.path.abspath(args.repo)
    ev = os.path.abspath(args.evidence) if args.evidence else os.path.join(repo, E_REL)
    prefix = E_REL if not args.evidence else None

    def label(rel):
        return (prefix + '/' + rel) if prefix else ('<evidence>/' + rel)

    missing = 0

    def report(rel):
        nonlocal missing
        ok = os.path.isfile(os.path.join(ev, rel))
        if not ok:
            missing += 1
        print('%s %s' % ('PASS' if ok else 'MISSING', label(rel)))

    for rel in FIXED:
        report(rel)
        if rel == 'probes/mutants.tsv':
            tsv = os.path.join(ev, rel)
            if os.path.isfile(tsv):
                with open(tsv, 'r', encoding='utf-8', errors='replace') as f:
                    for line in f:
                        line = line.rstrip('\n')
                        if not line.strip():
                            continue
                        mid = line.split('\t')[0]
                        if mid == 'mutant id':
                            continue
                        report('probes/raw/%s.log' % mid)
    return 1 if missing else 0


if __name__ == '__main__':
    sys.exit(main())
