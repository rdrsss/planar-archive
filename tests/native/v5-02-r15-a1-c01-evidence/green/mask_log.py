#!/usr/bin/env python3
# Masks finished C02 logs in place (D27). Stdlib only.
# Usage: mask_log.py --repo <root> [--clone <dir>]... --home <home> --scratch <SP> <file>...
# Replaces (a) every 64-hex token equal to the sha256 of roadmap.md, of any docs/vernacular/rounds/*.md
# or of any tests/native/*-evidence/**/REPORT* file under the repo and under every clone with
# '<sha256 of NAME>', and (b) the scratchpad with '<scratchpad>', each clone with '<clone>', the
# repository root with '<repo root>' and the home directory with '<home>'. It then makes the mask line
# the LAST line of the file. Idempotent. Exit codes, FAIL ids and PASS ids are never touched.
import argparse
import glob
import hashlib
import os
import re
import sys

MASK_LINE = 'masked: yes (paths and forbidden-file hashes replaced after the run; exit codes and FAIL ids untouched)'


def forbidden_hashes(roots):
    out = {}
    for root in roots:
        files = [os.path.join(root, 'docs/vernacular/roadmap.md')]
        files += glob.glob(os.path.join(root, 'docs/vernacular/rounds/*.md'))
        files += glob.glob(os.path.join(root, 'tests/native/*-evidence/**/REPORT*'), recursive=True)
        for f in files:
            if os.path.isfile(f):
                with open(f, 'rb') as fh:
                    out.setdefault(hashlib.sha256(fh.read()).hexdigest(), os.path.basename(f))
    return out


def variants(p):
    p = p.rstrip('/')
    out = {p, os.path.realpath(p)}
    if p.startswith('/private/'):
        out.add(p[len('/private'):])
    if p.startswith('/tmp/') or p.startswith('/var/'):
        out.add('/private' + p)
    return [v for v in out if v]


def mask_text(text, hashes, subs):
    def rep_hash(m):
        name = hashes.get(m.group(0))
        return '<sha256 of %s>' % name if name else m.group(0)
    text = re.sub(r'(?<![0-9a-fA-F])[0-9a-f]{64}(?![0-9a-fA-F])', rep_hash, text)
    for old, new in subs:
        text = text.replace(old, new)
    return text


def main(argv):
    ap = argparse.ArgumentParser()
    ap.add_argument('--repo', required=True)
    ap.add_argument('--clone', action='append', default=[])
    ap.add_argument('--home', required=True)
    ap.add_argument('--scratch', required=True)
    ap.add_argument('files', nargs='+')
    a = ap.parse_args(argv)
    roots = [a.repo] + a.clone
    hashes = forbidden_hashes(roots)
    subs = []
    for v in variants(a.scratch):
        subs.append((v, '<scratchpad>'))
    for cl in a.clone:
        for v in variants(cl):
            subs.append((v, '<clone>'))
    for v in variants(a.repo):
        subs.append((v, '<repo root>'))
    for v in variants(a.home):
        subs.append((v, '<home>'))
    subs.sort(key=lambda s: -len(s[0]))
    for f in a.files:
        with open(f, encoding='utf-8') as fh:
            text = fh.read()
        lines = [l for l in text.split('\n')]
        while lines and lines[-1] == '':
            lines.pop()
        lines = [l for l in lines if l != MASK_LINE]
        body = mask_text('\n'.join(lines), hashes, subs)
        with open(f, 'w', encoding='utf-8') as fh:
            fh.write(body + ('\n' if body else '') + MASK_LINE + '\n')
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
