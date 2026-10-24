"""Recompile preserved R1/R2 detectors; controlled bad_alloc is not host exhaustion."""
from pathlib import Path
import json,subprocess,sys,time,hashlib
R=Path.cwd();B=R/'build/rewrite-debug';O=R/'tests/native/v5-02-x05-a3-c03-evidence'/sys.argv[1];O.mkdir(exist_ok=False);records=[]
def run(group,label,argv,expected=0):
 t=time.monotonic();p=subprocess.run(argv,cwd=B,capture_output=True,text=True);(O/group/(label+'.log')).write_text(p.stdout+p.stderr);records.append(dict(group=group,label=label,argv=argv,exit_code=p.returncode,expected_exit=expected,seconds=time.monotonic()-t));(O/'commands.json').write_text(json.dumps(records,indent=2)+'\n');assert p.returncode==expected,(group,label,p.returncode,expected,p.stdout+p.stderr)
for group in ['adversarial','allocation-mapping','resource-mapping']:
 old=R/'tests/native/v5-02-x05-a3-review-evidence'/group;new=O/group;new.mkdir()
 main=(old/'main.cpp').read_text()
 if group!='adversarial':
  for v in ['v','vm']:
   a=v+'.error().detail.code=="resource_limit"';main=main.replace(a,a+'&&'+v+'.error().detail.message=="External profile resource limit exceeded"&&'+v+'.error().field=="catalog"')
 (new/'main.cpp').write_text(main)
 rows=json.loads((old/'commands.json').read_text());deltas=[]
 if group!='adversarial':
  base=(R/'src/engine/runtime/runtime.cpp').read_text();text=base
  a='    if (lua)\n      lua_close(lua);';b='    if (lua) {lua_close(lua);if(runtime.memory!=0)std::abort();}'
  assert text.count(a)==1;text=text.replace(a,b);deltas.append((a,b))
  if group=='resource-mapping':
   a='  auto embedded = history::prepare_stdlib(c.originals, c.separator, c.assembly);';b='  auto embedded = std::expected<history::prepared_provenance, history::provenance_error>(std::unexpected(history::provenance_error{history::provenance_error_code::resource_limit}));'
   assert text.count(a)==1;text=text.replace(a,b);deltas.append((a,b))
  filename='typed-refusal-runtime' if group=='resource-mapping' else 'cleanup-runtime'
  (new/(filename+'.cpp')).write_text(text)
  reverse=text
  for a,b in reversed(deltas):reverse=reverse.replace(b,a)
  assert reverse==base
  (new/'runtime-delta.json').write_text(json.dumps(dict(original_sha256=hashlib.sha256(base.encode()).hexdigest(),reverse_identical=True,substitutions=[dict(before=a,after=b) for a,b in deltas]),indent=2)+'\n')
  if group=='allocation-mapping':
   d=json.loads((old/'delta.json').read_text());canonical=(R/'src/engine/canonical/canonical.cpp').read_text();assert canonical.count(d['before'])==1
   (new/'allocation-fault-canonical.cpp').write_text(canonical.replace(d['before'],d['after']));(new/'canonical-delta.json').write_text(json.dumps(d,indent=2)+'\n')
   template=json.loads((R/'tests/native/v5-02-x05-a3-review-evidence/resource-mapping/commands.json').read_text())[0]
   argv=[x.replace(str(R/'tests/native/v5-02-x05-a3-review-evidence/resource-mapping'),str(new)).replace('typed-refusal-runtime',filename) for x in template['argv']]
   run(group,'compile-cleanup-runtime',argv)
 for row in rows:
  argv=[a.replace(str(old),str(new)) for a in row['argv']];label=row['label']
  if group=='allocation-mapping' and label=='link':
   argv.remove('src/engine/libplanar_runtime.a');argv.insert(argv.index(str(new/'main.o'))+1,str(new/'cleanup-runtime.o'))
  run(group,label,argv)
print('R1 ordinary runtime and R2 caught bad_alloc/direct typed-result exact diagnostics plus cleanup pass')
