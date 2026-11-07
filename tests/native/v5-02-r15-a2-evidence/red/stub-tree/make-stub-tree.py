#!/usr/bin/env python3
"""Build the skeleton stub tree used by the red run (outside the checkout).

@file make-stub-tree.py
@brief Copies the frozen base snapshot into a scratch tree and replaces the Part 2
placeholder section of execution-contract.md with the seven P2 headings and the
Part 1 amendments heading, each with an empty body.

Usage: python3 -I make-stub-tree.py BASE_DIR DEST_DIR. DEST_DIR must not exist. The
stub adds no contract text, so every expectRed check that needs literal text still
fails on it, and each failure names the literal it pins instead of a missing heading.
Only the scratch tree is written; the checkout is never touched.
"""

import os
import shutil
import sys

HEADINGS = [
    '## Part 2: execution verbs, projections and reads (R15-A2)', '',
    '### P2-1 Verbs and services', '',
    '### P2-2 Dispatch projection and dispatch_manifest_v1', '',
    '### P2-3 work_exec_v1', '',
    '### P2-4 Provenance (driver_provenance_v1)', '',
    '### P2-5 History event selector and request results', '',
    '### P2-6 Orchestrator live form', '',
    '### P2-7 Command, render, load, ledger and external-profile pointers', '',
    '## Part 1 amendments (R15-A2)', '',
]


def main(argv):
    base, dst = argv[1], argv[2]
    if os.path.exists(dst):
        raise SystemExit('destination exists: %s' % dst)
    os.makedirs(os.path.join(dst, 'docs', 'vernacular'))
    shutil.copytree(base, os.path.join(dst, 'base'))
    for name in os.listdir(base):
        shutil.copy(os.path.join(base, name), os.path.join(dst, 'docs', 'vernacular', name))
    ec_path = os.path.join(dst, 'docs', 'vernacular', 'execution-contract.md')
    with open(ec_path, encoding='utf-8') as fh:
        lines = fh.read().split('\n')
    start = lines.index('## Part 2 placeholders (R15-A2)')
    end = lines.index('## Messages')
    out = lines[:start] + HEADINGS + lines[end:]
    with open(ec_path, 'w', encoding='utf-8') as fh:
        fh.write('\n'.join(out))
    print('stub written: %s' % dst)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
