"""One frozen focused debug gate; never invokes a full test suite."""
from pathlib import Path
import hashlib, json, os, shutil, subprocess, sys, time, xml.etree.ElementTree as ET
root = Path(__file__).resolve().parents[3]
build = root / 'build/rewrite-debug'
evidence = root / 'tests/native/v5-02-x05-b2-c-c02-evidence'
out = evidence / 'final-gate'
out.mkdir(exist_ok=False)
def dump(name, value): (out / name).write_text(json.dumps(value, indent=2) + '\n')
def snapshot():
    paths=[]
    for directory in ['src','cmake','scripts','fixtures','tests']:
        for p in (root / directory).rglob('*'):
            rel=p.relative_to(root)
            if any('evidence' in part or part in {'__pycache__','build'} for part in rel.parts): continue
            if p.is_file() or p.is_symlink(): paths.append(p)
    paths += [root / name for name in ['CLAUDE.md','AGENTS.md','CMakeLists.txt','CMakePresets.json','Makefile','.clang-format']]
    paths += [root / name for name in ['docs/architecture.md','docs/testing.md','docs/vernacular/implementation.md','docs/vernacular/roadmap.md','docs/vernacular/external-profile-implementation.md']]
    result={}
    for p in sorted(set(paths)):
        if p.is_symlink(): result[str(p.relative_to(root))]={'symlink':os.readlink(p)}
        else: result[str(p.relative_to(root))]={'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'mode':p.stat().st_mode & 0o777}
    return result
freeze=snapshot(); dump('source-freeze.json',freeze)
commands=[]
def run(label, args):
    start=time.time()
    with (out / (label+'.log')).open('w') as stream:
        completed=subprocess.run([str(a) for a in args],cwd=root,stdout=stream,stderr=subprocess.STDOUT)
    commands.append({'label':label,'argv':[str(a) for a in args],'exit_code':completed.returncode,'seconds':round(time.time()-start,3)})
    dump('commands.json',commands)
    print(label,completed.returncode,flush=True)
    assert completed.returncode==0,label
run('shape-definition-vector',[sys.executable,'tests/native/v5-02-x05-b2-a-support/vector.py'])
assert json.loads((out/'shape-definition-vector.log').read_text())==json.loads((root/'tests/native/v5-02-x05-b2-a-evidence/independent-vector.json').read_text())
run('configure',['cmake','--preset','debug'])
run('build',['cmake','--build',build,'--target','all','planar_tests'])
run('inventory',['ctest','--test-dir',build,'--show-only=json-v1'])
inventory=json.loads((out/'inventory.log').read_text())['tests']
names={t['name'] for t in inventory}
assert len(names)==len(inventory)==620
prior619=json.loads((root/'tests/native/v5-02-x05-b2-c-evidence/final-gate/inventory.log').read_text())['tests']
assert {t['name'] for t in prior619}<=names
b2b=json.loads((root/'tests/native/v5-02-x05-b2-b-c01-evidence/final-gate/inventory.log').read_text())['tests']
assert len(b2b)==612 and {t['name'] for t in b2b}<=names
b2a=json.loads((root/'tests/native/v5-02-x05-b2-a-evidence/final-gate/inventory.log').read_text())['tests']
assert len(b2a)==606 and {t['name'] for t in b2a}<=names
baseline=json.loads((root/'tests/native/v5-02-x05-a4-evidence/attempt01/debug/inventory.log').read_text())['tests']
old={t['name'] for t in baseline}; assert len(old)==582 and old<=names
b1=json.loads((root/'tests/native/v5-02-x05-b1-evidence/final-gate/inventory.log').read_text())['tests']
assert len(b1)==600 and {t['name'] for t in b1}<=names
selected_executables={'planar_json_codec_tests','planar_ledger_tests','planar_ledger_format_tests','planar_record_tests','planar_runtime_tests','planar_command_tests','planar_history_interpretation_tests','planar_acceptance_load_tests','planar_hash_tests','planar_canonical_tests','planar_capture_tests','planar_effective_provenance_tests','planar_effective_catalog_tests','planar_x05_closure_audit_tests','python3.15'}
selected=[t['name'] for t in inventory if Path(t['command'][0]).name in selected_executables]
assert len(selected)==354
prior=(root/'tests/native/v5-02-x05-b2-b-evidence/final-gate/selected-tests.txt').read_text().splitlines()
assert set(prior)<=set(selected) and len(set(selected)-set(prior))==8
(out/'selected-tests.txt').write_text('\n'.join(selected)+'\n')
dump('inventory-preservation.json',{'registered':len(names),'old_preserved':len(old),'added':sorted(names-old),'focused':len(selected),'executables':sorted(selected_executables)})
run('focused',['ctest','--test-dir',build,'--tests-from-file',out/'selected-tests.txt','--output-on-failure','--output-junit',out/'focused.xml'])
for filename in ['LastTest.log','CTestCostData.txt']:
    shutil.copyfile(build/'Testing/Temporary'/filename,out/filename)
cases=ET.parse(out/'focused.xml').findall('.//testcase')
assert len(cases)==len(selected)
assert {c.get('name') for c in cases}==set(selected)
assert all(c.get('status')=='run' and not any(c.find(tag) is not None for tag in ['failure','error','skipped']) for c in cases)
dump('test-summary.json',{'executed':len(cases),'failures':0,'errors':0,'skips':0,'disabled':0,'preserved_codec_reservation_cases':6})
for label,path in [('canonical','tests/vectors/generate.py'),('record-v1','tests/vectors/records/generate.py'),('request-v2','tests/vectors/records/generate-request-v2.py'),('effective-provenance','tests/vectors/effective-provenance/generate.py')]:
    run(label,[sys.executable,path,'--check'])
for label,path in [('historical','tests/native/v5-02-b2-support/audit-variants.py'),('d6','tests/native/r14-d6-support/audit.py'),('d6-c01','tests/native/r14-d6-c01-support/audit.py'),('d7-b','tests/native/r14-d7-b-support/audit.py'),('b1','tests/native/v5-02-x05-b1-support/audit.py')]:
    run(label+'-audit',[sys.executable,path,build,out/(label+'-audit.json')])
run('b2-a-audit',[sys.executable,'tests/native/v5-02-x05-b2-a-support/audit.py',build,out/'b2-a-audit.json'])
history=(root/'tests/native/v5-02-x05-b2-b-evidence/history-codec-selected.txt').read_text().splitlines()
assert len(history)==11 and set(history)<=names
(out/'history-codec-selected.txt').write_text('\n'.join(history)+'\n')
run('history-codec',['ctest','--test-dir',build,'--tests-from-file',out/'history-codec-selected.txt','--output-on-failure','--output-junit',out/'history-codec.xml'])
shutil.copyfile(build/'Testing/Temporary/LastTest.log',out/'history-codec-LastTest.log')
history_cases=ET.parse(out/'history-codec.xml').findall('.//testcase')
assert len(history_cases)==11 and {c.get('name') for c in history_cases}==set(history)
assert all(c.get('status')=='run' and not any(c.find(tag) is not None for tag in ['failure','error','skipped']) for c in history_cases)
dump('history-codec-summary.json',{'executed':11,'failures':0,'errors':0,'skips':0,'disabled':0})
run('exact-original-observer-green',[sys.executable,'tests/native/v5-02-x05-b2-c-c02-support/exact_green.py','final-exact-green'])
run('sticky-regression',[build/'src/engine/planar_runtime_tests','[shape-sticky]','--reporter','junit','--out',out/'sticky.xml'])
run('registry-observer',[sys.executable,'tests/native/v5-02-x05-b2-c-c02-support/registry.py','final-registry'])
run('bridge-probes',[sys.executable,'tests/native/v5-02-x05-b2-c-c02-support/probes.py','final-probes'])
run('partial-snapshot',[sys.executable,'tests/native/v5-02-x05-b2-c-c02-support/partial_snapshot.py','final-partial-snapshot'])
run('pinned-lint',[sys.executable,'scripts/rewrite-check.py','lint','--build-dir',build])
run('no-work',['cmake','--build',build,'--target','all','planar_tests'])
assert 'ninja: no work to do.' in (out/'no-work.log').read_text()
assert snapshot()==freeze
dump('source-stability.json',{'entries':len(freeze),'final_snapshot_equal':True,'single_pre_gate_freeze':True})
print('FROZEN FOCUSED GATE PASS',flush=True)
