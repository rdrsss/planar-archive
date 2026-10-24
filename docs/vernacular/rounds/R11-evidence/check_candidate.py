#!/usr/bin/env python3
"""Documentation-only checks; not evidence of native SQLite/product behavior."""
from pathlib import Path
import hashlib
import re
import sqlite3
from urllib.parse import unquote

root = Path(__file__).resolve().parents[4]
files = [root / 'docs/vernacular' / n for n in (
    'ledger-contract.md', 'implementation.md', 'roadmap.md')]
links = 0
for path in files:
    text = path.read_text()
    assert '\x00' not in text
    assert text.count('```') % 2 == 0, path
    for target in re.findall(r'\]\(([^)]+)\)', text):
        if '://' in target or target.startswith('#'):
            continue
        target = unquote(target.split('#')[0])
        assert (path.parent / target).exists(), (path, target)
        links += 1
print(f'PASS: {len(files)} documents, {links} local links, balanced code fences')

contract = files[0].read_text()
sql = re.search(r'```sql\n(.*?)\n```', contract, re.S).group(1)
db = sqlite3.connect(':memory:')
db.execute('PRAGMA foreign_keys=ON')
db.executescript(sql)
immutable = ['ledger_meta','records','tree_edges','revisions','events','requests']
for table in immutable:
    for action in ['UPDATE', 'DELETE']:
        db.execute(f"CREATE TRIGGER {table}_no_{action.lower()} BEFORE {action} ON {table} BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END")
db.execute("CREATE TRIGGER current_state_no_delete BEFORE DELETE ON current_state BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END")
assert db.execute('PRAGMA foreign_key_check').fetchall() == []
assert db.execute('PRAGMA integrity_check').fetchone()[0] == 'ok'
db.execute("INSERT INTO ledger_meta VALUES (1,1,?, 'quota','2026-10-20T00:00:00.000Z')", ('a'*32,))
for action in ["UPDATE ledger_meta SET workspace_id='other'", 'DELETE FROM ledger_meta']:
    try:
        db.execute(action)
        raise AssertionError('immutable trigger did not reject mutation')
    except sqlite3.IntegrityError as error:
        assert str(error) == 'immutable ledger row'
assert int('504c4e32', 16) == 1347178034
print(f'PASS: normative DDL parses with SQLite {sqlite3.sqlite_version}; 7 STRICT tables, 13 immutable triggers; integrity/FK checks; metadata mutation rejected')
print('LIMIT: host Python SQLite only; no native build, product execution, concurrency, crash or filesystem durability test')
for path in files:
    print(hashlib.sha256(path.read_bytes()).hexdigest(), path.relative_to(root))
