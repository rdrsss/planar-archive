#!/usr/bin/env python3
"""Exact reversible test-only format transformation; optionally remove intern equality for mutation evidence."""
import hashlib
import json
from pathlib import Path
import sys

source, output = map(Path, sys.argv[1:3])
mutation = '--mutate-equality' in sys.argv[3:]
output.mkdir(parents=True, exist_ok=True)
manifest = {'verification_mutation': mutation, 'files': {}}
for name in ('format.cppm', 'format.cpp'):
    original = (source / name).read_text()
    generated = original
    substitutions = [
        ('planar.engine.ledger.format;', 'planar.test.record_format;'),
        ('namespace planar::engine::ledger {', 'namespace planar::engine::record_test {'),
    ]
    if name.endswith('.cppm'):
        substitutions += [('namespace planar::engine::record_test {', 'namespace planar::engine::record_test {\nusing ::planar::engine::ledger::read_guard;\nusing ::planar::engine::ledger::read_gate_status;')]
    if name.endswith('.cpp'):
        substitutions += [
            ('import planar.engine.ledger.sqlite;', 'import planar.engine.ledger.sqlite;\nimport planar.test.record_digest;'),
            ('namespace planar::engine::record_test {', 'namespace planar::engine::record_test {\nusing namespace planar::engine::ledger;'),
            ('digest_record(', 'planar::test::forced_digest('),
        ]
        if mutation:
            substitutions += [
                ('if (!bytes || *bytes != *encoded)\n        return refuse(format_error_code::hash_collision,',
                 'if (!bytes)\n        return refuse(format_error_code::hash_collision,'),
            ]
    counts = []
    for before, after in substitutions:
        count = generated.count(before)
        if count == 0:
            raise SystemExit(f'Missing declared substitution: {name}: {before}')
        counts.append(count)
        generated = generated.replace(before, after)
    restored = generated
    for before, after in reversed(substitutions):
        restored = restored.replace(after, before)
    if restored != original:
        raise SystemExit('Non-reversible source transformation')
    dest = output / name
    if not dest.exists() or dest.read_text() != generated:
        dest.write_text(generated)
    manifest['files'][name] = {
        'source_sha256': hashlib.sha256(original.encode()).hexdigest(),
        'generated_sha256': hashlib.sha256(generated.encode()).hexdigest(),
        'substitutions': [{'before': b, 'after': a, 'count': n} for (b, a), n in zip(substitutions, counts)],
        'reverse_identical': True,
    }
text = json.dumps(manifest, indent=2) + '\n'
if not (output / 'generation.json').exists() or (output / 'generation.json').read_text() != text:
    (output / 'generation.json').write_text(text)
