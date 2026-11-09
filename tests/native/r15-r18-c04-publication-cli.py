# Real grouped output-vector workflow and isolated retained-vector corruption regressions.
import pathlib, sqlite3, subprocess, json, ctypes, runpy
source = pathlib.Path('tests/native/r17-r18-cli-draft.py').read_text().split("reviewer,_=run(")[0]
source = source.replace('work=L{{id="unit",summary=p.md"Work",paths=L{"src/"},checks=L{"test"}}}', 'work=L{{id="unit",summary=p.md"Work",paths=L{"src/"},checks=L{"integration"}},{id="other",summary=p.md"Other",paths=L{"other/"},checks=L{"test"}}}')
source = source.replace('work=L{"unit"}', 'work=L{"unit","other"}').replace('targets=L{"unit"}', 'targets=L{"unit","other"}')
source = source.replace('facets=L{},dependencies=L{}', 'facets=L{{id="other_files",producer="other",paths=L{"other/a"}},{id="other_second",producer="other",paths=L{"other/b"}}},dependencies=L{{id="integrate",consumer="unit",source="facet",ref="other_files",phase="complete",quality="done"}}')
source = source.replace('checks=L{"test"}}', 'checks=L{"test","integration"}}', 1)
source = source.replace("(root/'current/src').mkdir();", "data=json.loads(registry.read_text());entry=dict(data['checks'][0]);entry['id']='integration';entry['inputs']=['own_output','output_vector'];data['checks'].append(entry);registry.write_text(json.dumps(data))\n(root/'current/other').mkdir();(root/'current/other/a').write_text('producer one\\n');(root/'current/other/b').write_text('second facet\\n')\n(root/'current/src').mkdir();")
exec(compile(source, 'grouped-fixture', 'exec'))
assert check['outcome']=='completed'
assert {u['work'] for u in check['observation']['units']}=={'unit','other'}
# A fresh completed run supplies a different, still-valid historical vector record.
(w/'spec.lua').write_text((w/'spec.lua').read_text().replace('Draft check smoke','Revised goal'))
run('load','--workspace',w/'workspace.lua','--registry',registry,'--expect-revision','1','--request-id','load-two','--actor','draft')
claim,_=run('claim','--job','job','--role','worker','--request-id','claim-two','--actor','draft')
(root/'current/other/a').write_text('producer two\n')
args=['--run',claim['run']['token'],'--root',root/'current','--base-root',root/'base','--scratch',root/'scratch','--request-id','check-two','--actor','draft']
checked,_=run('check',*args)
assert checked['outcome']=='completed'
# A third check reuses the current facet payload exactly.
(w/'spec.lua').write_text((w/'spec.lua').read_text().replace('Revised goal','Third goal'))
run('load','--workspace',w/'workspace.lua','--registry',registry,'--expect-revision','2','--request-id','load-three','--actor','draft')
claim,_=run('claim','--job','job','--role','worker','--request-id','claim-three','--actor','draft')
args=['--run',claim['run']['token'],'--root',root/'current','--base-root',root/'base','--scratch',root/'scratch','--request-id','check-three','--actor','draft']
checked,_=run('check',*args)
assert checked['outcome']=='completed'
backup=root/'backup.db'
with sqlite3.connect(ledger) as db, sqlite3.connect(backup) as dest:
    pubs=db.execute("SELECT id,payload_id,hex(semantic) FROM publications WHERE facet='other_files' ORDER BY id").fetchall()
    assert len(pubs)==3 and pubs[0][1]!=pubs[1][1] and pubs[1][1]==pubs[2][1]
    source_id=db.execute('SELECT source_id FROM revisions ORDER BY id LIMIT 1').fetchone()[0]
    db.backup(dest)
print('PASS valid historical and duplicate payload reuse')
failures=0
for label,sqls in [
 ('historical_unrelated_payload', [('UPDATE publications SET payload_id=? WHERE id=?',(source_id,pubs[0][0]))]),
 ('current_unrelated_payload', [('UPDATE publications SET payload_id=? WHERE id=?',(source_id,pubs[2][0]))]),
 ('current_old_bytes_same_paths', [('UPDATE publications SET payload_id=? WHERE id=?',(pubs[0][1],pubs[2][0]))]),
 ('semantic_and_response', [('UPDATE publications SET semantic=? WHERE id=?',(bytes.fromhex('42'*16),pubs[2][0])),("UPDATE requests SET result=CAST(replace(CAST(result AS TEXT),?,?) AS BLOB) WHERE request_id='check-three'",('xxh3_128:'+pubs[2][2].lower(),'xxh3_128:'+'42'*16))])]:
    with sqlite3.connect(backup) as src, sqlite3.connect(ledger) as db: src.backup(db)
    with sqlite3.connect(ledger) as db:
        triggers=db.execute("SELECT name,sql FROM sqlite_master WHERE type='trigger' AND tbl_name IN ('publications','requests')").fetchall()
        for name,_ in triggers: db.execute('DROP TRIGGER "'+name+'"')
        for sql,params in sqls: db.execute(sql,params)
        for _,sql in triggers: db.execute(sql)
    for verb,options in [('status',[]),('check',args)]:
        p=subprocess.run([str(binary),verb,'--ledger',str(ledger),*map(str,options)],capture_output=True,text=True,timeout=30)
        good=p.returncode==4 and 'ledger_corrupt' in p.stderr
        failures+=not good
        print(('PASS' if good else 'FAIL'),label,verb,'exit='+str(p.returncode))
# Independent path-equality witness: keep count and hash internally consistent so
# neither the payload-count nor semantic-recomputation guard masks path checking.
with sqlite3.connect(backup) as src, sqlite3.connect(ledger) as db:
    src.backup(db)
oracle = runpy.run_path(str(pathlib.Path('tests/vectors/generate.py').resolve()))
library = root / 'path-oracle.so'
subprocess.run(['cc', '-shared', '-fPIC', '-O2',
                str(pathlib.Path('vendor/xxhash/0.8.3/xxhash.c').resolve()),
                '-o', str(library)], check=True, timeout=60)
xx = ctypes.CDLL(str(library))
xx.XXH3_128bits.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
xx.XXH3_128bits.restype = oracle['Hash']
def digest(domain, value):
    encoded = oracle['encode'](['planar', 'canonical_v1', domain, value])
    hashed = xx.XXH3_128bits(encoded, len(encoded))
    return f'xxh3_128:{hashed.high:016x}{hashed.low:016x}'
def facet_semantic(declared):
    payload = (root / 'current/other/b').read_bytes()
    return digest('facet_v1', {
        'kind': 'files', 'normalizer': 'exact_bytes_v1', 'root': 'source',
        'paths': [declared], 'entries': [{
            'root': 'source', 'path': 'other/b', 'kind': 'file', 'mode': '0644',
            'size': len(payload), 'content': digest('file_bytes_v1', payload)}]})
with sqlite3.connect(ledger) as db:
    a = db.execute("SELECT id,payload_id,hex(semantic) FROM publications WHERE facet='other_files' ORDER BY id DESC LIMIT 1").fetchone()
    b = db.execute("SELECT id,payload_id,hex(semantic) FROM publications WHERE facet='other_second' ORDER BY id DESC LIMIT 1").fetchone()
    assert a and b and a[0] != b[0] and a[1] != b[1] and a[2] != b[2]
    assert db.execute("SELECT count(*) FROM publications WHERE facet='other_second'").fetchone()[0] == 3
    for publication in (a, b):
        assert db.execute('SELECT count(*) FROM tree_edges WHERE parent_id=?', (publication[1],)).fetchone()[0] == 1
        leaves = db.execute('''WITH RECURSIVE descendants(id) AS (
            SELECT ? UNION ALL SELECT edge.child_id FROM tree_edges edge
            JOIN descendants ON edge.parent_id=descendants.id)
            SELECT count(*) FROM descendants JOIN records ON records.id=descendants.id
            WHERE records.kind=1''', (publication[1],)).fetchone()[0]
        assert leaves == 1
    # Verify the independent encoder/hash against a genuine recorded publication.
    assert facet_semantic('other/b') == 'xxh3_128:' + b[2].lower()
    mixed = facet_semantic('other/a')
    original = 'xxh3_128:' + a[2].lower()
    assert mixed not in (original, 'xxh3_128:' + b[2].lower())
    saved = db.execute("SELECT result FROM requests WHERE request_id='check-three'").fetchone()[0]
    assert saved.count(original.encode()) == 1 and mixed.encode() not in saved
    triggers = db.execute("SELECT name,sql FROM sqlite_master WHERE type='trigger' AND tbl_name IN ('publications','requests')").fetchall()
    for name, _ in triggers:
        db.execute('DROP TRIGGER "' + name + '"')
    db.execute('UPDATE publications SET payload_id=?,semantic=? WHERE id=?',
               (b[1], bytes.fromhex(mixed.removeprefix('xxh3_128:')), a[0]))
    changed = saved.replace(original.encode(), mixed.encode())
    assert changed != saved and original.encode() not in changed
    assert db.execute("UPDATE requests SET result=? WHERE request_id='check-three'", (changed,)).rowcount == 1
    assert db.execute("SELECT result FROM requests WHERE request_id='check-three'").fetchone()[0] == changed
    for _, sql in triggers:
        db.execute(sql)
print('PASS genuine facet hash oracle and equal-count distinct payload controls')
for verb, options in [('status', []), ('check', args)]:
    result = subprocess.run([str(binary), verb, '--ledger', str(ledger), *map(str, options)],
                            capture_output=True, text=True, timeout=30)
    good = result.returncode == 4 and 'ledger_corrupt' in result.stderr
    failures += not good
    print(('PASS' if good else 'FAIL'), 'declared_path_with_matched_semantic', verb,
          'exit=' + str(result.returncode))
print(f'SUMMARY scenarios=6 corruption_assertions=10 failures={failures} skips=0')
raise SystemExit(bool(failures))
