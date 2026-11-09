# Saved execution responses bind to original event facts in disposable real workflows.
import pathlib
import sys

source = pathlib.Path('tests/native/r15-r18-c03-vectors-cli.py').read_text()
hook = "        print('PASS', label, 'vectors=' + str(vector_assertions), 'accepted=' + str(len(units)), flush=True)"
extra = r'''        import copy
        calls={}
        def op(key,verb,*op_args):
            req='c08_'+key
            full=(*op_args,'--request-id',req,'--actor','test')
            value,encoded=run(verb,*full)
            calls[key]=(verb,req,full,value,encoded)
            return value
        op('activate','activate','--job','job','--expect-revision',loaded['revision'],'--expect-job','job')
        claim=op('claim','claim','--job','job','--role','s_worker')
        token=claim['run']['token']
        notes=root/'notes.json'
        notes.write_text(json.dumps(dict(version=1,checks=[],findings=[],checkpoint=dict(progress='checked',next='continue',unresolved='',files=[]))))
        checkpoint=op('checkpoint','checkpoint','--run',token,'--observations',notes)
        absent=op('refresh_absent','refresh','--run',token,'--expect-fp',claim['dispatch']['work_fp'])
        assert absent['checkpoint']=='none'
        with sqlite3.connect(ledger) as db:
            assert db.execute('select checkpoint_id from dispatches where id=?',(int(absent['dispatch']['id']),)).fetchone()[0]==db.execute('select body_id from checkpoints where id=?',(int(checkpoint['checkpoint']['id']),)).fetchone()[0]
        noted=op('refresh_note','refresh','--run',token,'--expect-fp',claim['dispatch']['work_fp'],'--observations',notes)
        assert noted['checkpoint']['id']!=checkpoint['checkpoint']['id']
        taken=op('takeover','takeover','--job','job','--role','s_worker','--run-id',claim['run']['id'])
        op('release_note','release','--run',taken['run']['token'],'--reason','released','--observations',notes)
        second=op('claim_second','claim','--job','job','--role','s_worker')
        op('release_absent','release','--run',second['run']['token'],'--reason','interrupted')
        third=op('claim_drift','claim','--job','job','--role','s_worker')
        spec=workspace/'spec.lua';spec.write_text(spec.read_text().replace('Verify grouped and split dependent arithmetic outputs','Changed arithmetic goal'))
        new_load,_=run('load','--workspace',workspace/'workspace.lua','--registry',registry,'--expect-revision',loaded['revision'],'--request-id','c08_load_drift','--actor','test')
        refreshed=op('refresh_drift','refresh','--run',third['run']['token'],'--expect-fp',third['dispatch']['work_fp'])
        assert refreshed['drift_cleared'] is True
        op('release_drift','release','--run',third['run']['token'],'--reason','released')
        (workspace/'other.lua').write_text((workspace/'job.lua').read_text().replace('id="job"','id="other"'))
        ws=workspace/'workspace.lua';ws.write_text(ws.read_text().replace('jobs=L{{id="job",file="job.lua"}}','jobs=L{{id="job",file="job.lua"},{id="other",file="other.lua"}}'))
        last_load,_=run('load','--workspace',ws,'--registry',registry,'--expect-revision',new_load['revision'],'--request-id','c08_load_other','--actor','test')
        switched=op('activate_other','activate','--job','other','--expect-revision',last_load['revision'],'--expect-job','job')
        op('activate_back','activate','--job','job','--expect-revision',last_load['revision'],'--expect-job','other')
        assert switched['previous_job']=='job' and len(switched['roles'])==len(assignments)+1
        # Repair creates another generation before all older responses are replayed.
        repair_targets={}
        for repair_role,repair_owned in assignments:
            repair_claim,_=run('claim','--job','job','--role',repair_role,'--request-id','repair_claim_'+repair_role,'--actor','test')
            repaired_check,_=run('check','--run',repair_claim['run']['token'],'--root',current,'--base-root',base,'--scratch',scratch,'--request-id','repair_check_'+repair_role,'--actor','test')
            assert repaired_check['outcome']=='completed'
            repair_outputs={u['work']:u['output'] for u in repaired_check['observation']['units']}
            for unit in repair_owned:repair_targets[unit]=dict(work=unit,output=repair_outputs[unit],work_fp=repaired_check['completion']['work_fp'])
        repair_reviewer,_=run('claim','--job','job','--role','reviewer','--request-id','repair_review','--actor','test')
        rejection=root/'repair-verdict.json'
        rejection.write_text(json.dumps(dict(version=1,targets=[repair_targets[u] for u in units],policy=repair_reviewer['dispatch']['work_fp'],decision='reject',findings=[],repair=['s_worker'],summary='Exercise a retained repair generation')))
        run('verdict','--run',repair_reviewer['run']['token'],'--verdict',rejection,'--request-id','repair_reject','--actor','test')
        with sqlite3.connect(ledger) as db:
            assert db.execute('select count(*) from generations where id!=ordinal').fetchone()[0]>0
            original_dump=list(db.iterdump())
        for key,(verb,req,full,value,encoded) in calls.items():
            _,again=run(verb,*full)
            assert again==encoded,(key,'historical replay changed')
        # Protected load/check/verdict responses still replay after later activation/load.
        run('load','--workspace',ws,'--registry',registry,'--expect-revision','none','--request-id','load','--actor','test')
        _,old_check=run('check',*args);assert old_check==raw
        _,old_verdict=run('verdict','--run',reviewer['run']['token'],'--verdict',verdict,'--request-id','approve','--actor','test')
        assert old_verdict==first
        changed_request=subprocess.run([str(binary),*prefix,'claim','--ledger',str(ledger),'--job','job','--role','s_worker','--request-id','c08_claim','--actor','different'],capture_output=True,text=True)
        assert changed_request.returncode==3 and 'request_conflict' in changed_request.stderr
        unknown=subprocess.run([str(binary),*prefix,'release','--ledger',str(ledger),'--run','999.999.'+'0'*32,'--reason','released','--request-id','fresh_unknown','--actor','test'],capture_output=True,text=True)
        assert unknown.returncode==3
        with sqlite3.connect(ledger) as db:assert list(db.iterdump())==original_dump
        def leaves(value,path=()):
            if isinstance(value,dict):
                for k,v in value.items():
                    if k not in ('kind','version'):yield from leaves(v,path+(k,))
            elif isinstance(value,list):
                for i,v in enumerate(value):yield from leaves(v,path+(i,))
            else:yield path,value
        def replacement(path,value):
            key=path[-1]
            if isinstance(value,bool):return not value
            if isinstance(value,int):return value+1
            if key=='checkpoint' and value=='none':return {'id':'999'}
            if key=='state':
                if value in ('released','interrupted'):return 'interrupted' if value=='released' else 'released'
                return 'blocked' if value!='blocked' else 'ready'
            if key=='token':return value.rsplit('.',1)[0]+'.'+('0'*32 if not value.endswith('0'*32) else '1'*32)
            if key=='ledger':return 'f'*32 if value!='f'*32 else 'e'*32
            if key in ('work_fp','render_digest'):return 'xxh3_128:'+('f'*32 if value!='xxh3_128:'+'f'*32 else 'e'*32)
            if value.isdecimal():return '999' if value!='999' else '998'
            # A sorted first role name remains grammar-valid and ordered.
            if key=='role' and path[0]=='roles':return 'aaaa' if path[1]==0 else value+'_wrong'
            return value+'_wrong' if value else 'wrong'
        failures=[];cases=0;reads=0
        for key,(verb,req,full,value,encoded) in calls.items():
            with sqlite3.connect(ledger) as db:original=db.execute('select result from requests where request_id=?',(req,)).fetchone()[0]
            saved=json.loads(original)
            variants=[]
            for path,prior in leaves(saved):
                altered=copy.deepcopy(saved);at=altered
                for item in path[:-1]:at=at[item]
                at[path[-1]]=replacement(path,prior)
                if path in (('run','id'),('run','generation')) and 'token' in altered['run']:
                    r=altered['run'];r['token']=r['id']+'.'+r['generation']+'.'+r['token'].rsplit('.',1)[1]
                variants.append((path,altered))
            if verb=='activate':
                altered=copy.deepcopy(saved);altered['roles']=altered['roles'][1:];variants.append((('roles','membership'),altered))
            for path,altered in variants:
                def write(blob):
                    with sqlite3.connect(ledger) as db:
                        triggers=db.execute("select name,sql from sqlite_master where type='trigger' and tbl_name='requests'").fetchall()
                        for name,sql in triggers:db.execute('drop trigger "'+name+'"')
                        db.execute('update requests set result=? where request_id=?',(blob,req))
                        for name,sql in triggers:db.execute(sql)
                write(json.dumps(altered,separators=(',',':'),sort_keys=True).encode())
                try:
                    for name,tail in [('status',['status']),('replay',[verb,*map(str,full)]),('history',['history','--request-id',req])]:
                        result=subprocess.run([str(binary),*prefix,tail[0],'--ledger',str(ledger),*tail[1:]],capture_output=True,text=True,timeout=30)
                        reads+=1
                        if result.returncode!=4 or 'ledger_corrupt' not in result.stderr:failures.append((key,path,name,result.returncode,result.stderr))
                finally:write(original)
                cases+=1
        print('C08',label,'historical replays',len(calls),'corruptions',cases,'reader assertions',reads,'failures',len(failures),flush=True)
        assert not failures,failures
        # Attachment-only corruption leaves the saved result untouched.
        verb,req,full,value,encoded=calls['claim']
        with sqlite3.connect(ledger) as db:
            original_attachment,original_result=db.execute('select attachment_id,result from requests where request_id=?',(req,)).fetchone()
            assert original_attachment is not None
            other_attachment=db.execute('select packet_id from dispatches where packet_id!=? order by id limit 1',(original_attachment,)).fetchone()[0]
            assert other_attachment is not None and other_attachment!=original_attachment
        attachment_failures=[];attachment_cases=0;attachment_reads=0
        def write_attachment(attachment):
            with sqlite3.connect(ledger) as db:
                triggers=db.execute("select name,sql from sqlite_master where type='trigger' and tbl_name='requests'").fetchall()
                for name,sql in triggers:db.execute('drop trigger "'+name+'"')
                db.execute('update requests set attachment_id=? where request_id=?',(attachment,req))
                for name,sql in triggers:db.execute(sql)
                assert db.execute('select result from requests where request_id=?',(req,)).fetchone()[0]==original_result
        for changed_attachment in (None,other_attachment):
            write_attachment(changed_attachment)
            try:
                for name,tail in [('status',['status']),('replay',[verb,*map(str,full)]),('history',['history','--request-id',req])]:
                    result=subprocess.run([str(binary),*prefix,tail[0],'--ledger',str(ledger),*tail[1:]],capture_output=True,text=True,timeout=30)
                    attachment_reads+=1
                    if result.returncode!=4 or 'ledger_corrupt' not in result.stderr:attachment_failures.append((changed_attachment,name,result.returncode,result.stderr))
            finally:write_attachment(original_attachment)
            attachment_cases+=1
        assert attachment_cases==2 and attachment_reads==6
        # Trigger recreation changes sqlite_schema order, not any SQL statement or row.
        with sqlite3.connect(ledger) as db:assert sorted(db.iterdump())==sorted(original_dump)
        _,restored_replay=run(verb,*full)
        assert restored_replay==encoded
        print('C10 attachment',label,'corruptions',attachment_cases,'reader assertions',attachment_reads,'failures',len(attachment_failures),flush=True)
        assert not attachment_failures,attachment_failures
'''
# C12 is independent of the historical/field/attachment matrix above. Its first
# claim has no later reservations or completions that could mask content checks.
claim_hook = "            claim, _ = run('claim', '--job', 'job', '--role', role, '--request-id', 'claim_' + role, '--actor', 'test')"
claim_extra = r'''
            if role == assignments[0][0]:
                request_id = 'claim_' + role
                replay_args = ('--job', 'job', '--role', role, '--request-id', request_id, '--actor', 'test')
                dispatch_id = int(claim['dispatch']['id'])
                zero_fp = 'xxh3_128:' + '0' * 32
                assert claim['dispatch']['work_fp'] != zero_fp
                original_claim_replay = _
                run('status')
                _, pristine_replay = run('claim', *replay_args)
                assert pristine_replay == original_claim_replay
                run('history', '--request-id', request_id)
                with sqlite3.connect(ledger) as db:
                    assert db.execute('select count(*) from dispatches').fetchone()[0] == 1
                    assert db.execute('select count(*) from reservations').fetchone()[0] == 0
                    assert db.execute('select count(*) from completions').fetchone()[0] == 0
                    original_dump = sorted(db.iterdump())
                    original_fp = db.execute('select work_fp from dispatches where id=?', (dispatch_id,)).fetchone()[0]
                    original_result = db.execute('select result from requests where request_id=?', (request_id,)).fetchone()[0]
                    assert len(original_fp) == 16 and original_fp != bytes(16)
                    saved_claim = json.loads(original_result)
                    assert saved_claim['dispatch']['work_fp'] == claim['dispatch']['work_fp']
                    assert json.dumps(saved_claim, sort_keys=True, separators=(',', ':')).encode() == original_result
                    saved_claim['dispatch']['work_fp'] = zero_fp
                    altered_result = json.dumps(saved_claim, sort_keys=True, separators=(',', ':')).encode()
                    # Snapshot every table, retaining row multiplicity and all blob bytes.
                    def table_rows(connection):
                        result = {}
                        tables = connection.execute("select name from sqlite_master where type='table' order by name").fetchall()
                        for (table,) in tables:
                            cursor = connection.execute('select * from "' + table.replace('"', '""') + '"')
                            result[table] = ([column[0] for column in cursor.description], sorted(cursor.fetchall(), key=repr))
                        return result
                    original_rows = table_rows(db)
                    triggers = db.execute("select name,sql from sqlite_master where type='trigger' and name in ('dispatches_no_update','requests_no_update') order by name").fetchall()
                    assert {name for name, _ in triggers} == {'dispatches_no_update', 'requests_no_update'}
                def write_content(fp, result):
                    with sqlite3.connect(ledger) as db:
                        for name, _ in triggers:
                            db.execute('drop trigger "' + name + '"')
                        assert db.execute('update dispatches set work_fp=? where id=?', (fp, dispatch_id)).rowcount == 1
                        assert db.execute('update requests set result=? where request_id=?', (result, request_id)).rowcount == 1
                        for _, sql in triggers:
                            db.execute(sql)
                content_failures = []
                content_reads = 0
                content_cases = 0
                expected_diagnostic = dict(error=dict(code='ledger_corrupt', field='', message='Ledger integrity verification failed'), kind='error', version=1)
                try:
                    write_content(bytes(16), altered_result)
                    with sqlite3.connect(ledger) as db:
                        changed_rows = table_rows(db)
                    # Independently require that the two intended cells are the
                    # complete SQL delta; retained W, packet, manifest and all
                    # attachments/records therefore remain byte-identical.
                    expected_rows = dict(original_rows)
                    for table, key, wanted, column, value in (
                        ('dispatches', 'id', dispatch_id, 'work_fp', bytes(16)),
                        ('requests', 'request_id', request_id, 'result', altered_result),
                    ):
                        columns, rows = original_rows[table]
                        key_index, value_index = columns.index(key), columns.index(column)
                        replacement_rows = []
                        matched = 0
                        for row in rows:
                            updated = list(row)
                            if row[key_index] == wanted:
                                updated[value_index] = value
                                matched += 1
                            replacement_rows.append(tuple(updated))
                        assert matched == 1
                        expected_rows[table] = (columns, sorted(replacement_rows, key=repr))
                    assert changed_rows == expected_rows
                    for reader, tail in (
                        ('status', ['status']),
                        ('replay', ['claim', *replay_args]),
                        ('history', ['history', '--request-id', request_id]),
                    ):
                        result = subprocess.run([str(binary), *prefix, tail[0], '--ledger', str(ledger), *tail[1:]], capture_output=True, text=True, timeout=30)
                        content_reads += 1
                        try:
                            diagnostic = json.loads(result.stderr)
                        except ValueError:
                            diagnostic = None
                        if result.returncode != 4 or result.stdout != '' or diagnostic != expected_diagnostic:
                            content_failures.append((reader, result.returncode, result.stdout, result.stderr))
                    content_cases += 1
                finally:
                    write_content(original_fp, original_result)
                    with sqlite3.connect(ledger) as db:
                        assert sorted(db.iterdump()) == original_dump
                    run('status')
                    _, restored_replay = run('claim', *replay_args)
                    assert restored_replay == pristine_replay
                    run('history', '--request-id', request_id)
                assert content_cases == 1 and content_reads == 3
                print('C12 content', label, 'corruptions', content_cases, 'reader assertions', content_reads, 'failures', len(content_failures), flush=True)
                assert not content_failures, content_failures
'''
assert source.count(claim_hook) == 1
source = source.replace(claim_hook, claim_hook + claim_extra)
extra += r'''        assert len(calls) == 14 and cases == 204 and reads == 612
        assert attachment_cases == 2 and attachment_reads == 6
        assert content_cases == 1 and content_reads == 3
'''
assert source.count(hook)==1
scope={'__name__':'saved_response_fixture'}
exec(compile(source.replace(hook,extra),'saved_response_fixture','exec'),scope)
for profile in (False, True):
    scope['workflow'](pathlib.Path(sys.argv[1]).resolve(),False,profile)
