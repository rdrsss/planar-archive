#!/usr/bin/env python3
"""Link and anchor check for the seven R15-A2 contract documents.

@file link_check.py
@brief Resolves every relative markdown link and #anchor in the seven edited docs.

Adapted from the R15-A1 linkcheck.py. Scope: execution-contract.md,
command-contract.md, render-contract.md, load-command-contract.md,
ledger-contract.md, external-profile-contract.md and execution-design.md under
--repo/docs/vernacular, plus the required anchors of the execution contract (the
seven P2 anchors and #part-1-amendments-r15-a2). A link target that is not one of
the files under --repo is read with `git show <base-commit>:<path>` from --git-repo.
Anchors use GitHub slug rules. The check never reads the round record.

Entry points: `python3 -I link_check.py --repo DIR --git-repo DIR --base-commit SHA`
and `--selftest`. Output: `PASS LINK-A2-001 anchors_resolve` or
`FAIL LINK-A2-001 anchors_resolve: N problem(s)` followed by one line per
problem. Exit status 1 on any problem.
"""

import argparse
import os
import posixpath
import re
import subprocess
import sys
import urllib.parse

SEVEN = [
    'execution-contract.md', 'command-contract.md', 'render-contract.md',
    'load-command-contract.md', 'ledger-contract.md', 'external-profile-contract.md',
    'execution-design.md',
]
REQUIRED_EC_ANCHORS = [
    '#part-2-execution-verbs-projections-and-reads-r15-a2',
    '#p2-1-verbs-and-services',
    '#p2-2-dispatch-projection-and-dispatch_manifest_v1',
    '#p2-3-work_exec_v1',
    '#p2-4-provenance-driver_provenance_v1',
    '#p2-5-history-event-selector-and-request-results',
    '#p2-6-orchestrator-live-form',
    '#p2-7-command-render-load-ledger-and-external-profile-pointers',
    '#part-1-amendments-r15-a2',
]
DOCS_DIR = posixpath.join('docs', 'vernacular')
LINK_RE = re.compile(r'\]\(([^)\s]+)\)')


def slug(heading):
    """GitHub slug: lower case, drop code ticks and punctuation, spaces become hyphens."""
    h = heading.strip().lower()
    h = re.sub(r'`', '', h)
    h = re.sub(r'[^\w\- ]', '', h)
    return h.replace(' ', '-')


def anchors_of(text):
    found = set()
    counts = {}
    fenced = False
    for line in text.split('\n'):
        if line.startswith('```'):
            fenced = not fenced
        if fenced:
            continue
        m = re.match(r'#{1,6}\s+(.*)', line)
        if m:
            base = slug(m.group(1))
            n = counts.get(base, 0)
            counts[base] = n + 1
            found.add(base if n == 0 else '%s-%d' % (base, n))
    return found


def links_in(text):
    """Yield (line_number, target) for every markdown link outside code fences and code spans."""
    fenced = False
    for ln, line in enumerate(text.split('\n'), 1):
        if line.startswith('```'):
            fenced = not fenced
            continue
        if fenced:
            continue
        for m in LINK_RE.finditer(re.sub(r'`[^`]*`', '', line)):
            yield ln, m.group(1)


class Resolver(object):
    def __init__(self, repo, git_repo, base_commit):
        self.repo = repo
        self.git_repo = git_repo
        self.base_commit = base_commit
        self._cache = {}

    def read(self, rel):
        """Return (text or None, problem or None) for a repository-relative path."""
        key = rel
        if key in self._cache:
            return self._cache[key]
        local = os.path.join(self.repo, *rel.split('/'))
        result = (None, 'missing file %s' % rel)
        if os.path.isfile(local):
            try:
                with open(local, encoding='utf-8') as fh:
                    result = (fh.read(), None)
            except (OSError, UnicodeDecodeError):
                result = (None, 'unreadable file %s' % rel)
        elif self.git_repo and re.fullmatch(r'[0-9a-f]{40}', self.base_commit or ''):
            proc = subprocess.run(['git', '-C', self.git_repo, 'show',
                                   '%s:%s' % (self.base_commit, rel)], capture_output=True)
            if proc.returncode == 0:
                result = (proc.stdout.decode('utf-8', 'replace'), None)
        self._cache[key] = result
        return result


def check_document(name, text, resolver, problems):
    """Resolve every link of one document. `name` is repository-relative."""
    doc_dir = posixpath.dirname(name)
    for ln, target in links_in(text):
        if re.match(r'[a-zA-Z][a-zA-Z0-9+.-]*:', target):
            continue
        path, _, anchor = target.partition('#')
        path = urllib.parse.unquote(path)
        if path:
            rel = posixpath.normpath(posixpath.join(doc_dir, path))
            if rel.startswith('..'):
                problems.append('%s:%d link leaves the repository: %s' % (name, ln, target))
                continue
            body, problem = resolver.read(rel)
            if problem:
                problems.append('%s:%d %s (link %s)' % (name, ln, problem, target))
                continue
        else:
            rel = name
            body, problem = (text, None)
        if anchor and rel.endswith('.md'):
            if anchor not in anchors_of(body):
                problems.append('%s:%d missing anchor %s' % (name, ln, target))


def run(repo, git_repo, base_commit):
    resolver = Resolver(repo, git_repo, base_commit)
    problems = []
    texts = {}
    for name in SEVEN:
        rel = posixpath.join(DOCS_DIR, name)
        body, problem = resolver.read(rel)
        if problem:
            problems.append(problem)
            continue
        texts[name] = body
    for name, body in sorted(texts.items()):
        check_document(posixpath.join(DOCS_DIR, name), body, resolver, problems)
    ec = texts.get('execution-contract.md')
    if ec is None:
        problems.append('execution-contract.md unreadable; required anchors not checked')
    else:
        present = anchors_of(ec)
        for anchor in REQUIRED_EC_ANCHORS:
            if anchor[1:] not in present:
                problems.append('required anchor missing from execution-contract.md: %s' % anchor)
    return problems


def selftest():
    here = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'fixtures')
    failures = []
    for kind, want_ok in (('good', True), ('bad', False)):
        with open(os.path.join(here, 'link.%s.txt' % kind), encoding='utf-8') as fh:
            text = fh.read()
        problems = []
        check_document('fixture.md', text, Resolver('', None, None), problems)
        if (not problems) != want_ok:
            failures.append('link %s fixture gave %s' % (kind, 'no problem' if not problems else 'problems'))
    return failures


def main(argv=None):
    ap = argparse.ArgumentParser(description='R15-A2 link and anchor check')
    ap.add_argument('--repo', default=os.getcwd())
    ap.add_argument('--git-repo', default=None)
    ap.add_argument('--base-commit', default=None)
    ap.add_argument('--selftest', action='store_true')
    args = ap.parse_args(argv)
    if args.selftest:
        failures = selftest()
        if failures:
            print('FAIL LINK-A2-001 anchors_resolve selftest: %s' % '; '.join(failures))
            return 1
        print('PASS LINK-A2-001 anchors_resolve selftest')
        return 0
    if args.base_commit and args.base_commit.upper() == 'HEAD':
        print('FAIL LINK-A2-001 anchors_resolve: --base-commit must be a full SHA, not HEAD')
        return 1
    try:
        problems = run(args.repo, args.git_repo, args.base_commit)
    except Exception as exc:  # per-check boundary: never a traceback
        print('FAIL LINK-A2-001 anchors_resolve: internal error (%s)' % type(exc).__name__)
        return 1
    if problems:
        print('FAIL LINK-A2-001 anchors_resolve: %d problem(s)' % len(problems))
        for p in problems:
            print('  ' + p)
        return 1
    print('PASS LINK-A2-001 anchors_resolve')
    return 0


if __name__ == '__main__':
    try:
        sys.stdout.reconfigure(errors='backslashreplace')
    except AttributeError:
        pass
    sys.exit(main())
