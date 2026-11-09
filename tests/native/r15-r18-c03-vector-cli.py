# Real grouped output-vector workflow and isolated retained-vector corruption regressions.
import pathlib, sqlite3, subprocess, json
source = pathlib.Path('tests/native/r17-r18-cli-draft.py').read_text().split("reviewer,_=run(")[0]
source = source.replace('work=L{{id="unit",summary=p.md"Work",paths=L{"src/"},checks=L{"test"}}}', 'work=L{{id="unit",summary=p.md"Work",paths=L{"src/"},checks=L{"integration"}},{id="other",summary=p.md"Other",paths=L{"other/"},checks=L{"test"}}}')
source = source.replace('work=L{"unit"}', 'work=L{"unit","other"}').replace('targets=L{"unit"}', 'targets=L{"unit","other"}')
source = source.replace('facets=L{},dependencies=L{}', 'facets=L{{id="other_files",producer="other",paths=L{"other/a"}}},dependencies=L{{id="integrate",consumer="unit",source="facet",ref="other_files",phase="complete",quality="done"}}')
source = source.replace('checks=L{"test"}}', 'checks=L{"test","integration"}}', 1)
source = source.replace("(root/'current/src').mkdir();", "data=json.loads(registry.read_text());entry=dict(data['checks'][0]);entry['id']='integration';entry['inputs']=['own_output','output_vector'];data['checks'].append(entry);registry.write_text(json.dumps(data))\n(root/'current/other').mkdir();(root/'current/other/a').write_text('producer one\\n')\n(root/'current/src').mkdir();")
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
backup=root/'backup.db'
with sqlite3.connect(ledger) as db, sqlite3.connect(backup) as dest: db.backup(dest)
for label,mutation in [
 ('wrong_own', "UPDATE evidence SET vector_id=(SELECT vector_id FROM evidence WHERE work='other' ORDER BY seq DESC LIMIT 1) WHERE work='unit' AND \"check\"='planar.boundary_v1'"),
 ('missing_grouped_producer', "UPDATE evidence SET vector_id=(SELECT vector_id FROM evidence e2 WHERE e2.work='unit' AND e2.\"check\"='planar.boundary_v1' AND e2.seq=evidence.seq) WHERE work='unit' AND \"check\"='integration'"),
 ('old_observation_output', "UPDATE evidence SET vector_id=(SELECT vector_id FROM evidence WHERE work='unit' AND \"check\"='integration' ORDER BY seq ASC LIMIT 1) WHERE work='unit' AND \"check\"='integration' AND seq=(SELECT MAX(seq) FROM evidence)")]:
    with sqlite3.connect(backup) as src, sqlite3.connect(ledger) as db: src.backup(db)
    with sqlite3.connect(ledger) as db:
        triggers=db.execute("SELECT name,sql FROM sqlite_master WHERE type='trigger' AND tbl_name='evidence'").fetchall()
        for name,_ in triggers: db.execute('DROP TRIGGER "'+name+'"')
        db.execute(mutation)
        for _,sql in triggers: db.execute(sql)
    for verb,options in [('status',[]),('check',args)]:
        result=subprocess.run([str(binary),verb,'--ledger',str(ledger),*map(str,options)],capture_output=True,text=True,timeout=30)
        assert result.returncode==4 and 'ledger_corrupt' in result.stderr,(label,verb,result.returncode,result.stderr)
    print('PASS',label)
print('SUMMARY scenarios=4 corruption_assertions=6 failures=0 skips=0')
