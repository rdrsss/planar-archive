"""Read-only current generated graph, oracle and retained refusal evidence checks."""
import importlib.util,json,sys,shutil
from pathlib import Path
p=Path(__file__).with_name('verify.py');spec=importlib.util.spec_from_file_location('verify',p);v=importlib.util.module_from_spec(spec);spec.loader.exec_module(v)
def variants(preset):
 build=v.ROOT/'build'/('rewrite-'+preset);rows={}
 for manifest in (build/'src/engine').glob('*-generated/generation.json'):
  destination=v.OUT/(preset+'-generated-source')/manifest.parent.name
  destination.mkdir(parents=True,exist_ok=True)
  shutil.copyfile(manifest,destination/'generation.json')
  data=json.loads(manifest.read_text());data=data.get('files',data)
  for name,record in data.items():
   src=v.ROOT/'src/engine'/('export' if name.startswith('source_export') else 'ledger')/name
   gen=manifest.parent/name
   shutil.copyfile(gen,destination/name)
   assert v.sha(src)==record['source_sha256'] and v.sha(gen)==record['generated_sha256'] and record['reverse_identical']
   text=src.read_text()
   for sub in record['substitutions']:
    assert text.count(sub['before'])==sub['count'];text=text.replace(sub['before'],sub['after'])
   assert text==gen.read_text()
   for sub in reversed(record['substitutions']):
    assert text.count(sub['after'])==sub['count'];text=text.replace(sub['after'],sub['before'])
   assert text==src.read_text()
  rows[manifest.parent.name]=list(data)
 assert len(rows)==5
 v.write(preset+'-five-variants.json',dict(exact_forward_reverse=True,variants=rows))
 for label,script in [('historical','v5-02-b2-support/audit-variants.py'),('d6','r14-d6-support/audit.py'),('d6-c01','r14-d6-c01-support/audit.py'),('d7-b','r14-d7-b-support/audit.py')]:
  v.run(preset+'-'+label+'-audit',[sys.executable,v.ROOT/'tests/native'/script,build,v.OUT/(preset+'-'+label+'-audit-report.json')])
 commands=json.loads((build/'compile_commands.json').read_text());cache=(build/'CMakeCache.txt').read_text().splitlines()
 compiler=next(r.split('=',1)[1] for r in cache if r.startswith('CMAKE_CXX_COMPILER:'))
 support=sorted({str(Path(row['file']).resolve()) for row in commands if Path(row['file']).resolve().is_relative_to(v.ROOT/'tests/native') and Path(row['file']).suffix in ('.cpp','.cppm')})
 assert support
 v.run(preset+'-support-format',[str(Path(compiler).parent/'clang-format'),'--dry-run','--Werror',*support])
 v.write(preset+'-support-format-files.json',support)
 print('Five exact storage/export and all inherited runtime/CLI graphs audited',flush=True)
def common():
 v.run('vectors-v1',[sys.executable,'tests/vectors/records/generate.py','--check'])
 v.run('vectors-v2',[sys.executable,'tests/vectors/records/generate-request-v2.py','--check'])
 source=v.ROOT/'tests/native/v5-02-x03-b1-evidence';d=json.loads((source/'old-binary-results.json').read_text())
 assert v.sha(v.ROOT/d['executable'])==d['executable_sha256']=='be1c4cee1b358208831faa919707ce5fd01de3ea31bc63db333a027f8844464a'
 assert len(d['runs'])==8
 for row in d['runs']:
  assert row['exit_code']==3 and row['stdout']=='' and json.loads(row['stderr'])['error']['code']=='unsupported_ledger_version'
  assert row['before']==row['after'] and row['unchanged']
 v.write('retained-old-binary-proof.json',dict(reused_not_rerun=True,source='tests/native/v5-02-x03-b1-evidence/old-binary-results.json',proof_sha256=v.sha(source/'old-binary-results.json'),executable_sha256=d['executable_sha256'],runs=8,all_directory_snapshots_equal=True,source_provenance={str(p.relative_to(v.ROOT)):v.sha(p) for p in [source/'pre-b1-planar.sha256',source/'baseline-build.log',source/'pre-b1-dependencies.txt',source/'README.md',v.ROOT/'docs/vernacular/rounds/V5-02-X03-B1-review.md']}))
if sys.argv[1]=='common':common()
else:variants(sys.argv[1])
