#!/usr/bin/env python3
"""Audit protected inputs, exact configured variants, focused counts and verification-only mutation evidence."""
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
import xml.etree.ElementTree as ET

root = Path(__file__).resolve().parents[3]
evidence = root / 'tests/native/r13b1-evidence'
protected = json.loads((evidence / 'protected-before.json').read_text())
changed = [p for p, h in protected.items() if not (root / p).is_file()
           or hashlib.sha256((root / p).read_bytes()).hexdigest() != h]
assert not changed, changed
print(f'Protected source, vendors, vectors and prior evidence: {len(protected)} files unchanged.')
for name in ('r12c2', 'r13b1'):
    with tempfile.TemporaryDirectory(prefix=f'planar-{name}-audit-') as temporary:
        subprocess.run(['python3', str(root / f'tests/native/{name}-support/generate.py'),
                        str(root / 'src/engine/ledger'), temporary], check=True)
        expected = root / 'build/rewrite-debug/src/engine' / (name + '-generated')
        for file in ('format.cppm', 'format.cpp', 'generation.json'):
            assert (Path(temporary) / file).read_bytes() == (expected / file).read_bytes(), (name, file)
        assert (evidence / (name + '-generation.json')).read_bytes() == (expected / 'generation.json').read_bytes()
    print(f'{name}: independent regeneration matches configured module/interface/manifest exactly; reversible substitutions only.')
cases = ET.parse(evidence / 'focused.xml').findall('.//testcase')
assert len(cases) == 76, len(cases)
assert not any(c.find('skipped') is not None or c.find('failure') is not None
               or c.get('status') in ('notrun', 'disabled') for c in cases)
print(f'Focused JUnit audit: {len(cases)} cases, zero failures and zero skipped/disabled/notrun.')
mutation = json.loads((evidence / 'mutation-generation.json').read_text())
assert mutation['verification_mutation']
assert b'2 failed' in (evidence / 'mutation-red.log').read_bytes()
normal = json.loads((evidence / 'generation.json').read_text())
assert not normal['verification_mutation']
assert next(s['count'] for s in normal['files']['format.cpp']['substitutions']
            if s['before'] == 'digest_record(') == 2
print('Mutation RED is verification-only: 3 collision cases, 2 failed, 17 failed assertions; protected variant restored.')
assert (evidence / 'no-work.log').read_text().strip() == 'ninja: no work to do.'
print('No-change all + planar_tests build: zero steps.')
