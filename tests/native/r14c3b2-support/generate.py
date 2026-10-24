#!/usr/bin/env python3
"""Audited reversible namespace/digest variant plus an exact private allocator export for verification."""
import hashlib
import json
from pathlib import Path
import sys

source, output = map(Path, sys.argv[1:3])
mutation = "--mutate-final-receipt" in sys.argv[3:]
output.mkdir(parents=True, exist_ok=True)
manifest = {}
for name in ('format.cppm', 'format.cpp'):
    original = (source / name).read_text()
    changes = [
        ('planar.engine.ledger.format;', 'planar.test.atomic_format;'),
        ('namespace planar::engine::ledger {', 'namespace planar::engine::atomic_test {'),
    ]
    if name.endswith('.cppm'):
        changes += [('namespace planar::engine::atomic_test {', 'namespace planar::engine::atomic_test {\nusing ::planar::engine::ledger::read_guard;\nusing ::planar::engine::ledger::read_gate_status;')]
    if name.endswith('.cpp'):
        changes += [
            ('import planar.engine.ledger.sqlite;', 'import planar.engine.ledger.sqlite;\nimport planar.test.atomic_digest;'),
            ('namespace planar::engine::atomic_test {', 'namespace planar::engine::atomic_test {\nusing namespace planar::engine::ledger;'),
            ('digest_record(', 'planar::test::atomic_digest::digest('),
            ('digest_request_v2(', 'planar::test::atomic_digest::digest_v2('),
            ('\n#undef LEDGER_TRY', '\nstd::expected<std::int64_t, format_error> allocator_boundary(std::int64_t maximum) {\n  return next_identifier(maximum);\n}\n#undef LEDGER_TRY'),
        ]
    else:
        changes += [('struct format_state;', '''/// @brief Invoke the unchanged private allocator boundary in the isolated test variant.
/// @param maximum Exact currently allocated signed-64 maximum.
/// @return Next ID or the production exhaustion refusal.
std::expected<std::int64_t, format_error> allocator_boundary(std::int64_t maximum);
struct format_state;''')]
    if mutation and name.endswith(".cpp"):
        changes.append(("if (!candidate.verify())", "if (false)"))
    generated = original
    counts = []
    for before, after in changes:
        count = generated.count(before)
        if before in ('digest_record(', 'digest_request_v2(') and count != 2:
            raise SystemExit(f'Expected exactly one reader and one writer digest site: {name}: {before}: {count}')
        if not count:
            raise SystemExit(f'Missing exact substitution: {name}: {before}')
        counts.append(count)
        generated = generated.replace(before, after)
    restored = generated
    for before, after in reversed(changes):
        restored = restored.replace(after, before)
    if restored != original:
        raise SystemExit('Source transformation is not byte reversible')
    target = output / name
    if not target.exists() or target.read_text() != generated:
        target.write_text(generated)
    manifest[name] = {
        'source_sha256': hashlib.sha256(original.encode()).hexdigest(),
        'generated_sha256': hashlib.sha256(generated.encode()).hexdigest(),
        'reverse_identical': True,
        'substitutions': [{'before': b, 'after': a, 'count': n} for (b, a), n in zip(changes, counts)],
    }
report = json.dumps(manifest, indent=2) + '\n'
if not (output / 'generation.json').exists() or (output / 'generation.json').read_text() != report:
    (output / 'generation.json').write_text(report)
