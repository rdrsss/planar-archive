"""Serialized A1 debug verification, source stability, inventory, audits and limits."""
from pathlib import Path
import datetime,hashlib,json,subprocess,sys,xml.etree.ElementTree as ET
ROOT=Path(__file__).resolve().parents[3];OUT=ROOT/'tests/native/v5-02-x04-a1-evidence';BUILD=ROOT/'build/rewrite-debug'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(name,data):(OUT/name).write_text(json.dumps(data,indent=2,sort_keys=True)+'\n')
owned=['src/engine/capture/admission.cppm','src/engine/capture/admission.cpp','src/engine/capture/admission.t.cpp',
       'src/engine/capture/native.cppm','src/engine/capture/native.cpp','src/engine/CMakeLists.txt','docs/architecture.md']
owned += [str(p.relative_to(ROOT)) for p in sorted((ROOT/'tests/native/v5-02-x04-a1-support').glob('*')) if p.is_file()]
freeze={n:sha(ROOT/n) for n in owned};write('source-freeze.json',freeze)
results=[]
def run(label,args):
 start=datetime.datetime.now(datetime.timezone.utc)
 with (OUT/(label+'.log')).open('w') as f:
  result=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=f,stderr=subprocess.STDOUT)
 row=dict(label=label,argv=list(map(str,args)),exit_code=result.returncode,start=start.isoformat(),seconds=(datetime.datetime.now(datetime.timezone.utc)-start).total_seconds())
 results.append(row);write('commands.json',results)
 assert {n:sha(ROOT/n) for n in owned}==freeze,('Source changed during command',label)
 if result.returncode:raise SystemExit((label,result.returncode))
 return (OUT/(label+'.log')).read_text()
run('debug-build',['cmake','--build',BUILD,'--target','all','planar_tests'])
listing=json.loads(run('debug-inventory',['ctest','--test-dir',BUILD,'--show-only=json-v1']))
old=json.loads((ROOT/'tests/native/v5-02-x03-b3-evidence/debug-inventory.log').read_text())
names={t['name'] for t in listing['tests']};prior={t['name'] for t in old['tests']}
assert len(prior)==430 and prior<=names
write('inventory-preservation.json',dict(previous=len(prior),current=len(names),missing=sorted(prior-names),added=sorted(names-prior)))
run('debug-focused',['ctest','--test-dir',BUILD,'-R','replacement.*(capture|path|acceptance)','--output-on-failure','--output-junit',OUT/'debug-focused.xml'])
cases=ET.parse(OUT/'debug-focused.xml').findall('.//testcase')
assert cases and not any(c.find('skipped') is not None or c.find('failure') is not None or c.find('error') is not None for c in cases)
write('focused-results.json',dict(tests=len(cases),failures=0,errors=0,skips=0,names=[c.get('name') for c in cases]))
for label,script in [('vectors','tests/vectors/generate.py'),('record-vectors','tests/vectors/records/generate.py'),('request-v2-vectors','tests/vectors/records/generate-request-v2.py')]:run(label,[sys.executable,script,'--check'])
for label,script in [('historical','v5-02-b2-support/audit-variants.py'),('d6','r14-d6-support/audit.py'),('d6-c01','r14-d6-c01-support/audit.py'),('d7-b','r14-d7-b-support/audit.py')]:
 run(label+'-audit',[sys.executable,ROOT/'tests/native'/script,BUILD,OUT/(label+'-audit-report.json')])
variants={}
for manifest in (BUILD/'src/engine').glob('*-generated/generation.json'):
 data=json.loads(manifest.read_text());data=data.get('files',data)
 for name,row in data.items():
  original=ROOT/'src/engine'/('export' if name.startswith('source_export') else 'ledger')/name
  generated=manifest.parent/name
  assert sha(original)==row['source_sha256'] and sha(generated)==row['generated_sha256'] and row['reverse_identical']
  text=original.read_text()
  for sub in row['substitutions']:
   assert text.count(sub['before'])==sub['count'];text=text.replace(sub['before'],sub['after'])
  assert text==generated.read_text()
  for sub in reversed(row['substitutions']):text=text.replace(sub['after'],sub['before'])
  assert text==original.read_text()
 variants[manifest.parent.name]={n:row['generated_sha256'] for n,row in data.items()}
assert len(variants)==5;write('five-storage-export-variants.json',variants)
run('pinned-lint',['python3','scripts/rewrite-check.py','lint','--build-dir',BUILD])
run('support-format',['/opt/homebrew/opt/llvm/bin/clang-format','--dry-run','--Werror',ROOT/'tests/native/v5-02-x04-a1-support/syscalls.cpp'])
text=run('debug-no-work',['cmake','--build',BUILD,'--target','all','planar_tests']);assert 'ninja: no work to do.' in text
obj=BUILD/'src/engine/CMakeFiles/planar_capture_admission.dir/capture/admission.cpp.o'
text=run('admission-undefined-symbols',['nm','-u',obj])
assert not any(line.strip().endswith('_'+symbol) for line in text.splitlines() for symbol in ('read','pread','readv','mmap','fread','fopen','opendir','readdir'))
write('metadata-content-api-audit.json',dict(object_sha256=sha(obj),no_undefined_content_or_enumeration_apis=True,scope='Compiled-object audit complements positive-controlled dynamic read/pread/readv/mmap observation; not universal syscall tracing.'))
write('source-stability.json',dict(unchanged=True,files=freeze))
write('debug-build-identities.json',{str(p.relative_to(ROOT)):sha(p) for p in [BUILD/'bin/planar',BUILD/'src/engine/planar_capture_admission_tests',BUILD/'src/engine/libplanar_admission_syscalls.dylib',obj]})
print(json.dumps(dict(inventory=len(names),preserved=len(prior),focused=len(cases),skips=0,source_unchanged=True)))
