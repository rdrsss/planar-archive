"""Serialized, source-frozen X05-B4-C integrated native verification gate."""
from pathlib import Path
import datetime, hashlib, json, os, shutil, stat, subprocess, sys
import xml.etree.ElementTree as ET
sys.dont_write_bytecode=True
os.environ['PYTHONDONTWRITEBYTECODE']='1'
ROOT = Path(__file__).resolve().parents[3]
OUT = ROOT / 'tests/native/v5-02-x05-b4-c-evidence'
SUP = ROOT / 'tests/native/v5-02-x05-b4-c-support'
OWNED = ['CLAUDE.md', 'docs/architecture.md', 'docs/testing.md', 'docs/vernacular/x05-shape-evidence.md'] + ['docs/vernacular/'+n+'.md' for n in ['implementation','roadmap','v5-alignment','acceptance-matrix','external-profile-implementation']]
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def write(p,v): p.write_text(json.dumps(v,indent=2,sort_keys=True)+'\n')
def filesystem():
    rows={}
    for current,dirs,files in os.walk(ROOT,followlinks=False):
        base=Path(current)
        dirs[:]=[n for n in dirs if not (base==ROOT and n in ['.git','build']) and base/n!=OUT]
        for n in files+dirs:
            p=base/n; s=p.lstat(); key=str(p.relative_to(ROOT))
            if stat.S_ISLNK(s.st_mode): rows[key]={'symlink':os.readlink(p),'mode':stat.S_IMODE(s.st_mode)}
            elif stat.S_ISREG(s.st_mode): rows[key]={'sha256':sha(p),'mode':stat.S_IMODE(s.st_mode)}
    return rows

def freeze():
    dest=OUT/'attempt01';dest.mkdir(exist_ok=False)
    data=filesystem();write(dest/'source-freeze.json',data)
    for name,row in data.items():
        if (name.startswith(('src/','cmake/','scripts/','tests/stdlib/','tests/cli/','tests/vectors/','fixtures/')) or ('-support/' in name and name.startswith('tests/native/') and '/candidate' not in name and '/before' not in name and '-evidence/' not in name)) or name in ['CMakeLists.txt','CMakePresets.json','Makefile']:
            target=dest/'candidate-source'/name;target.parent.mkdir(parents=True,exist_ok=True)
            if 'symlink' in row:target.symlink_to(row['symlink'])
            else:shutil.copy2(ROOT/name,target)
    print('Frozen',len(data),'filesystem entries',flush=True)

def gate(preset):
    attempt=OUT/'attempt01';dest=attempt/preset;dest.mkdir(exist_ok=False)
    frozen=json.loads((attempt/'source-freeze.json').read_text());rows=[]
    def stable():
        current=filesystem()
        assert current==frozen, {'source_changed':sorted(k for k in current.keys()|frozen.keys() if current.get(k)!=frozen.get(k))}
    def run(label,args,after=None):
        stable();start=datetime.datetime.now(datetime.timezone.utc)
        with (dest/(label+'.log')).open('w') as stream:
            result=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=stream,stderr=subprocess.STDOUT)
        # Preserve the full-suite CTest temporaries before any later command/check.
        if after:after()
        end=datetime.datetime.now(datetime.timezone.utc)
        rows.append(dict(label=label,argv=list(map(str,args)),cwd=str(ROOT),start_utc=start.isoformat(),end_utc=end.isoformat(),seconds=(end-start).total_seconds(),exit_code=result.returncode))
        write(dest/'commands.json',rows);print(preset,label,result.returncode,flush=True)
        stable()
        if result.returncode:raise SystemExit((label,result.returncode))
        return (dest/(label+'.log')).read_text()
    build=ROOT/'build'/('rewrite-'+preset)
    run('configure',['cmake','--preset',preset])
    run('build',['cmake','--build',build,'--target','all','planar_tests'])
    listing=json.loads(run('inventory',['ctest','--test-dir',build,'--show-only=json-v1']))
    names=[t['name'] for t in listing['tests']]
    previous=json.loads((ROOT/'tests/native/v5-02-x05-b4-b-c01-evidence/final-gate/inventory.json').read_text())
    prior=[t['name'] for t in previous['tests']]
    assert len(prior)==676 and len(names)==len(set(names))==676 and set(prior)<=set(names)
    write(dest/'inventory-preservation.json',dict(previous=676,current=676,missing=sorted(set(prior)-set(names)),added=sorted(set(names)-set(prior))))
    def retain():
        for n in ['LastTest.log','CTestCostData.txt']:
            shutil.copy2(build/'Testing/Temporary'/n,dest/n)
    run('full',['ctest','--test-dir',build,'--output-on-failure','--output-junit',dest/'full.xml'],retain)
    root=ET.parse(dest/'full.xml').getroot();cases=root.findall('.//testcase')
    assert len(cases)==676 and sorted(c.get('name') for c in cases)==sorted(names)
    assert all(c.get('status')=='run' and all(c.find(t) is None for t in ['failure','error','skipped']) for c in cases)
    assert all(int(s.get(k,'0'))==0 for s in root.iter('testsuite') for k in ['failures','errors','skipped','disabled'])
    baseline=json.loads((ROOT/'tests/native/v5-02-x05-a4-evidence/attempt01/debug/inventory.log').read_text())
    assert len(baseline['tests'])==582 and {t['name'] for t in baseline['tests']} <= set(names)
    last=(dest/'LastTest.log').read_text()
    assert last.count('Test Passed.')==676
    import re
    assert sorted(re.findall(r'^\d+/\d+ Testing: (.*)$',last,re.M))==sorted(names)
    cost=(dest/'CTestCostData.txt').read_text().split('---')[0].splitlines()
    cost_names=[line.rsplit(' ',2)[0] for line in cost if line.strip()]
    assert len(cost_names)==len(set(cost_names))==676 and set(cost_names)==set(names), 'cost names differ'
    assert not any(any(p['name']=='DISABLED' and p['value'] for p in t.get('properties',[])) for t in listing['tests'])
    write(dest/'test-summary.json',dict(registered=676,executed=676,failures=0,errors=0,skips=0,disabled=0,baseline_582_preserved=True,last_test_names_exact=True,all_status_run=True,prior_names_preserved=676,names=names))
    for label,script in ([] if preset!='debug' else [('selection-vectors','tests/native/v5-02-x05-b3-c-support/vector.py'),('shape-definition-vectors','tests/native/v5-02-x05-b2-a-support/vector.py'),('canonical-vectors','tests/vectors/generate.py'),('record-v1-vectors','tests/vectors/records/generate.py'),('request-v2-vectors','tests/vectors/records/generate-request-v2.py'),('effective-provenance-vectors','tests/vectors/effective-provenance/generate.py')]):run(label,[sys.executable,script]+([] if label=='shape-definition-vectors' else ['--check']))
    for label,script in [('historical','v5-02-b2-support/audit-variants.py'),('d6','r14-d6-support/audit.py'),('d6-c01','r14-d6-c01-support/audit.py'),('d7-b','r14-d7-b-support/audit.py'),('b1','v5-02-x05-b1-support/audit.py'),('b2-a','v5-02-x05-b2-a-support/audit.py')]:
        run(label+'-audit',[sys.executable,ROOT/'tests/native'/script,build,dest/(label+'-audit-report.json')])
    variants={}
    for manifest in (build/'src/engine').glob('*-generated/generation.json'):
        if manifest.parent.name=='v5-02-x05-a0-generated':continue
        data=json.loads(manifest.read_text());data=data.get('files',data)
        for name,row in data.items():
            original=ROOT/'src/engine'/('export' if name.startswith('source_export') else 'ledger')/name;generated=manifest.parent/name
            assert sha(original)==row['source_sha256'] and sha(generated)==row['generated_sha256'] and row['reverse_identical']
            text=original.read_text()
            for sub in row['substitutions']:
                assert text.count(sub['before'])==sub['count'];text=text.replace(sub['before'],sub['after'])
            assert text==generated.read_text()
            for sub in reversed(row['substitutions']):text=text.replace(sub['after'],sub['before'])
            assert text==original.read_text()
        variants[manifest.parent.name]=data
    assert len(variants)==5;write(dest/'five-storage-export-variants.json',variants)
    generated=build/'src/engine/v5-02-x05-a0-generated';generation=json.loads((generated/'generation.json').read_text())
    for name,row in generation.items():
     if name=='lua':continue
     original=ROOT/'src/engine/runtime'/('runtime.cppm' if name.endswith('.cppm') else 'runtime.cpp');assert sha(original)==row['source_sha256'] and sha(generated/name)==row['generated_sha256'] and row['reverse_identical'];text=original.read_text()
     for sub in row['substitutions']:assert text.count(sub['before'])==sub['count'];text=text.replace(sub['before'],sub['after'])
     assert text==(generated/name).read_text()
     for sub in reversed(row['substitutions']):assert text.count(sub['after'])==sub['count'];text=text.replace(sub['after'],sub['before'])
     assert text==original.read_text()
    assert (generated/'commands_0.lua').read_bytes()==(ROOT/'src/stdlib/planar/commands.lua').read_bytes();write(dest/'a0-runtime-audit.json',generation)
    run('pinned-lint',[sys.executable,'scripts/rewrite-check.py','lint','--build-dir',build])
    text=run('no-work',['cmake','--build',build,'--target','all','planar_tests']);assert 'ninja: no work to do.' in text
    write(dest/'build-identities.json',{str(p.relative_to(ROOT)):sha(p) for p in sorted(build.rglob('*')) if p.is_file() and not p.is_symlink() and (os.access(p,os.X_OK) or p.suffix in ('.o','.pcm','.a','.dylib') or p.name in ('CMakeCache.txt','compile_commands.json','build.ninja'))})
    stable();write(dest/'source-stability.json',dict(unchanged=True,freeze_sha256=sha(attempt/'source-freeze.json'),entries=len(frozen)))
    write(dest/'pass-manifest.json',{str(p.relative_to(dest)):sha(p) for p in sorted(dest.rglob('*')) if p.is_file() and p.name!='pass-manifest.json'})
    print(preset,'676 passed; zero failures/errors/skips; no work; stable',flush=True)

if __name__=='__main__':
    if sys.argv[1]=='freeze':freeze()
    else:gate(sys.argv[1])
