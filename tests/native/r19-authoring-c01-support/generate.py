#!/usr/bin/env python3
"""Private exact-runtime budget fixture: identity renames and one checked preconsumption insertion."""
import hashlib
import json
from pathlib import Path
import sys


ROOT, OUTPUT = map(Path, sys.argv[1:])
OUTPUT.mkdir(parents=True, exist_ok=True)
ANCHOR = '    bind_embedded_execution(s, c, services);'
PRECONSUME = '''    // Verification-only: leave the requested native room using real cumulative charges, never a reset/refund.
    std::string_view binding_budget_room;
    if (const auto* activate = std::get_if<command_activate_options>(&c.options))
      binding_budget_room = activate->request_id;
    else if (const auto* status = std::get_if<command_status_options>(&c.options); status && status->request_id)
      binding_budget_room = *status->request_id;
    if (!binding_budget_room.empty()) {
      std::size_t remaining = 0;
      const auto parsed = std::from_chars(binding_budget_room.data(), binding_budget_room.data() + binding_budget_room.size(), remaining);
      if (binding_budget_room.size() != 12 || parsed.ec != std::errc{} || parsed.ptr != binding_budget_room.data() + binding_budget_room.size() ||
          remaining > memory_max - s.native_bytes)
        command_refuse();
      charge(s, memory_max - s.native_bytes - remaining);
    }
'''
rows = {}
for name in ('runtime.cpp', 'runtime.cppm'):
    original = (ROOT / 'src/engine/runtime' / name).read_text()
    changes = [
        ('module planar.engine.runtime;', 'module planar.engine.binding_budget_test;'),
        ('namespace planar::engine {', 'namespace planar::engine::binding_budget_test {'),
    ]
    if name == 'runtime.cpp':
        assert original.count(ANCHOR) == 1, 'The private preconsumption anchor must occur exactly once'
        changes.append((ANCHOR, PRECONSUME + ANCHOR))
    transformed = original
    counts = []
    for before, after in changes:
        count = transformed.count(before)
        assert count == 1, (name, before, count)
        counts.append(count)
        transformed = transformed.replace(before, after)
    restored = transformed
    for before, after in reversed(changes):
        assert restored.count(after) == 1, (name, after)
        restored = restored.replace(after, before)
    assert restored == original, name
    path = OUTPUT / name
    if not path.exists() or path.read_text() != transformed:
        path.write_text(transformed)
    rows[name] = {
        'source_sha256': hashlib.sha256(original.encode()).hexdigest(),
        'sha256': hashlib.sha256(transformed.encode()).hexdigest(),
        'reverse_identical': True,
        'substitutions': changes,
        'substitution_counts': counts,
        'verification_insertions': int(name == 'runtime.cpp'),
    }
manifest = json.dumps({'scope': 'verification-only; no product source hook', 'files': rows}, indent=2) + '\n'
path = OUTPUT / 'generation.json'
if not path.exists() or path.read_text() != manifest:
    path.write_text(manifest)
print('Audited private binding-budget runtime: two identity renames and one checked preconsumption insertion')
