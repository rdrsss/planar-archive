#!/usr/bin/env python3
"""Independent SQL/order probe only; symbolic blobs are not product codec/hash evidence."""
from pathlib import Path
import re, sqlite3, json
root=Path(__file__).resolve().parents[4]
contract=(root/'docs/vernacular/ledger-contract.md').read_text()
sql=re.search(r"```sql\n(.*?)\n```",contract,re.S).group(1)
db=sqlite3.connect(':memory:')
db.execute('PRAGMA foreign_keys=ON')
db.executescript(sql)
for table in ['ledger_meta','records','tree_edges','revisions','events','requests']:
    for action in ['UPDATE','DELETE']:
        db.execute(f"CREATE TRIGGER {table}_no_{action.lower()} BEFORE {action} ON {table} BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END")
db.execute("CREATE TRIGGER current_state_no_delete BEFORE DELETE ON current_state BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END")
def event(seq,typ,rev,req,before,after):
    payload=json.dumps({'version':1,'before':{'active_revision':before},'after':{'active_revision':after},'reason':typ,'references':{}},separators=(',',':')).encode()
    db.execute('INSERT INTO events VALUES (?,?,1,?,?,?, ?,?)',(seq,typ,rev,req,'review','2026-10-20T00:00:00.000Z',payload))
with db:
    db.execute("INSERT INTO ledger_meta VALUES (1,1,?,'quota','2026-10-20T00:00:00.000Z')",('a'*32,))
    event(1,'ledger_initialized',None,None,'none','none')
    db.execute('INSERT INTO current_state VALUES (1,NULL,1)')
print('PASS 1: initialization has no circular FK dependency')
# Symbolic records check SQL references/order, not record_v1 validity.
with db:
    for i,kind in [(1,1),(2,2),(3,3),(4,4)]:
        db.execute('INSERT INTO records VALUES (?,?,1,?,?)',(i,kind,bytes([i])*16,b'symbolic'))
    db.execute("INSERT INTO tree_edges VALUES (2,0,'file.lua',1)")
    db.execute("INSERT INTO revisions VALUES (1,NULL,2,2,2,3,3,'review','2026-10-20T00:00:00.000Z')")
    event(2,'declaration_accepted',1,'first','none','1')
    db.execute("INSERT INTO requests VALUES ('first',4,2,1,'accepted',?)",(b'{"revision":"1"}',))
    db.execute('UPDATE current_state SET active_revision_id=1,last_seq=2')
print('PASS 2: child-first record, directory edge, revision, event, request and projection insert without deferred constraints')
with db:
    event(3,'load_noop',1,'noop','1','1')
    db.execute("INSERT INTO requests VALUES ('noop',4,3,1,'noop',?)",(b'{"revision":"1"}',))
    db.execute('UPDATE current_state SET last_seq=3')
print('PASS 3: no-op can append request/event while retaining revision')
with db:
    for revision,seq,request,parent in [(2,4,'change',1),(3,5,'revert',2)]:
        db.execute("INSERT INTO revisions VALUES (?,?,2,2,2,3,3,'review','2026-10-20T00:00:00.000Z')",(revision,parent))
        event(seq,'declaration_accepted',revision,request,str(parent),str(revision))
        db.execute("INSERT INTO requests VALUES (?,4,?,?,'accepted',?)",(request,seq,revision,b'{}'))
        db.execute('UPDATE current_state SET active_revision_id=?,last_seq=?',(revision,seq))
assert db.execute('SELECT id,parent_id,source_id FROM revisions').fetchall()==[(1,None,2),(2,1,2),(3,2,2)]
print('PASS 4: immutable content may be reused with fresh revision chronology')
rejected=0
for table in ['ledger_meta','records','tree_edges','revisions','events','requests']:
    column=db.execute(f'PRAGMA table_info({table})').fetchone()[1]
    for statement in [f'UPDATE {table} SET {column}={column}',f'DELETE FROM {table}']:
        try: db.execute(statement)
        except sqlite3.IntegrityError as exc:
            assert str(exc)=='immutable ledger row'
            rejected+=1
        else: raise AssertionError(statement)
try: db.execute('DELETE FROM current_state')
except sqlite3.IntegrityError: rejected+=1
assert rejected==13
print('PASS 5: all 13 specified immutability triggers reject matching mutations')
prior=db.execute('SELECT * FROM current_state').fetchall()
try:
    with db:
        event(6,'load_noop',3,'rollback','3','3')
        db.execute('UPDATE current_state SET last_seq=6')
        raise RuntimeError('injected precommit refusal')
except RuntimeError: pass
assert db.execute('SELECT * FROM current_state').fetchall()==prior
assert db.execute('SELECT max(seq) FROM events').fetchone()[0]==5
assert db.execute('PRAGMA foreign_key_check').fetchall()==[]
assert db.execute('PRAGMA integrity_check').fetchone()[0]=='ok'
print('PASS 6: injected refusal rolls back event and projection; final FK/integrity checks pass')
print('LIMIT: host Python SQLite '+sqlite3.sqlite_version+'; symbolic record/event payloads test SQL insertion order only. No native codecs, invariant checker, actual hashes, process crash, disk-full, concurrency or filesystem durability tested.')
