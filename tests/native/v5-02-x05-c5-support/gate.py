"""One serialized full suite per actual configuration, with immediate raw retention."""
from freeze import *
import subprocess,time,xml.etree.ElementTree as ET,re
config=sys.argv[1];assert config in ['debug','release'];B=R/('build/rewrite-'+config);O=E/config;O.mkdir(exist_ok=False);commands=[]
def run(label,args,after=None,required=True):
 stable();start=time.time()
 with (O/(label+'.log')).open('w') as f:p=subprocess.run(list(map(str,args)),cwd=R,stdout=f,stderr=subprocess.STDOUT)
 if after:after()
 commands.append(dict(label=label,argv=list(map(str,args)),cwd=str(R),exit=p.returncode,seconds=time.time()-start));write(O/'commands.json',commands);print(config,label,p.returncode,flush=True);stable()
 if required:assert p.returncode==0,label
 return (O/(label+'.log')).read_text()
run('configure',['cmake','--preset',config]);build=['cmake','--build',B,'--target','all','planar_tests'];run('build',build)
listing=json.loads(run('inventory',['ctest','--test-dir',B,'--show-only=json-v1']));names=[t['name'] for t in listing['tests']];assert len(names)==len(set(names))==876
current=json.loads((R/'tests/native/v5-02-x05-c4-d4-evidence/inventory-final.json').read_text());assert {t['name'] for t in current['tests']}==set(names)
prior=json.loads((R/'tests/native/v5-02-x05-b4-c-evidence/attempt01/debug/inventory.log').read_text());old={t['name'] for t in prior['tests']};assert len(old)==676
renames={};sources=[]
for p in sorted((R/'tests/native').glob('*-evidence/**/inventory-preservation.json')):
 try:d=json.loads(p.read_text())
 except (ValueError,OSError):continue
 if isinstance(d.get('renames'),dict):
  for a,b in d['renames'].items():
   if a!=b:renames[a]=b;sources.append(str(p.relative_to(R)))
def mapped(n):
 seen=set()
 while n in renames:assert n not in seen;seen.add(n);n=renames[n]
 return n
missing=sorted(mapped(n) for n in old if mapped(n) not in names);write(O/'inventory-preservation.json',dict(current=876,historical=676,missing=missing,renames={a:b for a,b in renames.items() if a in old},rename_records=sorted(set(sources)),current_names_exact=True));assert not missing,missing
cost=B/'Testing/Temporary/CTestCostData.txt'
if cost.exists():
 shutil.copy2(cost,O/'preexisting-CTestCostData.txt')
 oldcost=[x.rsplit(' ',2)[0] for x in cost.read_text().split('---')[0].splitlines() if x.strip()]
 if len(oldcost)!=876 or set(oldcost)!=set(names):
  write(O/'cost-cache-removal.json',{'path':str(cost),'before':entry(cost),'stale_names':sorted(set(oldcost)-set(names)),'missing_current_names':sorted(set(names)-set(oldcost)),'reason':'Build-local accumulated scheduler names differ from exact current inventory.'});cost.unlink()
def retain():
 for n in ['LastTest.log','CTestCostData.txt']:shutil.copy2(B/'Testing/Temporary'/n,O/n)
run('full',['ctest','--test-dir',B,'--output-on-failure','--output-junit',O/'full.xml'],retain)
root=ET.parse(O/'full.xml').getroot();cases=root.findall('.//testcase');assert len(cases)==876 and sorted(c.get('name') for c in cases)==sorted(names)
assert all(c.get('status')=='run' and not any(c.find(t)is not None for t in ['failure','error','skipped']) for c in cases)
assert all(int(s.get(k,'0'))==0 for s in root.iter('testsuite') for k in ['failures','errors','skipped','disabled'])
last=(O/'LastTest.log').read_text();assert last.count('Test Passed.')==876 and sorted(re.findall(r'^\d+/\d+ Testing: (.*)$',last,re.M))==sorted(names)
costnames=[x.rsplit(' ',2)[0] for x in (O/'CTestCostData.txt').read_text().split('---')[0].splitlines() if x.strip()];assert len(costnames)==876 and set(costnames)==set(names)
assert not any(any(p['name']=='DISABLED' and p['value'] for p in t.get('properties',[])) for t in listing['tests'])
write(O/'test-summary.json',dict(registered=876,executed=876,failures=0,errors=0,skips=0,disabled=0,all_status_run=True,inventory_junit_lasttest_cost_agree=True))
if config=='debug':
 for label,script,args in [('selection','tests/native/v5-02-x05-b3-c-support/vector.py',['--check']),('shape-definition','tests/native/v5-02-x05-b2-a-support/vector.py',[]),('canonical','tests/vectors/generate.py',['--check']),('record-v1','tests/vectors/records/generate.py',['--check']),('request-v2','tests/vectors/records/generate-request-v2.py',['--check']),('effective-provenance','tests/vectors/effective-provenance/generate.py',['--check'])]:run(label+'-vectors',['python3','-B',script,*args])
for label,script in [('historical','v5-02-b2-support/audit-variants.py'),('d6','r14-d6-support/audit.py'),('d6-c01','r14-d6-c01-support/audit.py'),('d7-b','r14-d7-b-support/audit.py'),('b1','v5-02-x05-b1-support/audit.py')]:run(label+'-audit',['python3','-B',R/'tests/native'/script,B,O/(label+'-audit.json')])
run('retained-native-audit',['python3','-B',R/'tests/native/v5-02-x05-c4-d2-b2-c01-support/audit.py',O/'retained-native-audit.json'])
run('current-runtime-audit',['python3','-B',R/'tests/native/v5-02-x05-c4-d4-support/audit.py',O/'current-runtime-audit.json'])
run('generated-audit',['python3','-B',S/'generated.py',config])
run('pinned-lint',['python3','-B','scripts/rewrite-check.py','lint','--build-dir',B])
run('private',['python3','-B',S/'private.py',config]);run('private-cleanup',['python3','-B',S/'cleanup.py',config])
assert 'ninja: no work to do.' in run('no-work',build)
write(O/'source-stability.json',stable());write(O/'build-identities.json',{str(p.relative_to(R)):entry(p) for p in sorted(B.rglob('*')) if p.is_file() and not p.is_symlink() and (os.access(p,os.X_OK) or p.suffix in ['.o','.pcm','.a','.dylib'] or p.name in ['CMakeCache.txt','compile_commands.json','build.ninja'])})
print(config,'COMPLETE',flush=True)
