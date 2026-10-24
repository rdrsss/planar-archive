from pathlib import Path
import hashlib,json,difflib,subprocess,re,os
from urllib.parse import unquote
R=Path(__file__).resolve().parents[3];O=R/'tests/native/v5-02-x06-a3-evidence';base=json.loads((O/'baseline.json').read_text())
DOCS=['CLAUDE.md','docs/architecture.md','docs/testing.md']+['docs/vernacular/'+x+'.md' for x in ['implementation','roadmap','acceptance-matrix','v5-alignment','external-profile-implementation','x06-implementation']]
owned=DOCS+['src/engine/CMakeLists.txt','src/engine/ledger/format.cpp','src/engine/history/read.cpp','src/engine/ledger/history_verification.t.cpp']
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
old=(O/'preedit/src/engine/ledger/format.cpp').read_text();new=(R/'src/engine/ledger/format.cpp').read_text().replace('import planar.engine.history.recorded_interpretation;\n','')
def scrub(s):
 a=s.index('std::expected<',s.index('\nstd::expected<void, format_error> verify_content')+1) if '\nstd::expected<void, format_error> verify_content' in s else s.index('std::expected<recorded_family, format_error> verify_content')
 b=s.index('// Compare exact bounded record bytes',a)
 s=s[:a]+'CONTENT\n'+s[b:]
 a=s.index('std::expected<void, format_error> verify_history(');b=s.index('std::expected<ledger_metadata, format_error> verify(',a)
 return s[:a]+'HISTORY\n'+s[b:]
assert scrub(old)==scrub(new),'format edit escaped owned functions'
for p in ['src/engine/history/interpretation_codec.cpp','src/engine/history/interpretation_v2_codec.cpp','src/engine/history/interpretation_v2_codec.cppm','src/engine/history/recorded_interpretation.cpp','src/engine/history/recorded_interpretation.cppm','src/engine/history/recorded_interpretation.t.cpp','src/engine/history/effective_provenance.cpp','src/engine/history/provenance_codec.cpp','docs/vernacular/external-profile-contract.md','src/engine/acceptance/prepared.cpp','src/engine/acceptance/load.cpp','src/engine/runtime/runtime.cpp','src/engine/export/source_export.cpp']:
 assert sha(R/p)==base[p]['sha256'],p
for p in DOCS[:-1]:
 a=(O/'preedit'/p).read_text();b=(R/p).read_text();marker='The checkpoints below';assert a[a.index(marker):]==b[b.index(marker):],p
p=DOCS[-1];a=(O/'preedit'/p).read_text();b=(R/p).read_text();marker='## Existing boundaries';assert a[a.index(marker):]==b[b.index(marker):]
assert os.readlink(R/'AGENTS.md')=='CLAUDE.md'
assert os.readlink(R/'compile_commands.json')==base['compile_commands.json']['target']
# Match the complete CMake diff to required production/variant dependencies plus test-vector macro.
p='src/engine/CMakeLists.txt';a=(O/'preedit'/p).read_text();b=(R/p).read_text()
a=a.replace('planar_history_tree_core planar_history_provenance_codec planar_history_interpretation_codec planar_canonical','planar_history_tree_core planar_history_provenance_codec planar_history_interpretation_codec planar_history_recorded_interpretation planar_canonical')
a=a.replace('  DEPENDS planar_ledger_format planar_history_interpretation_codec planar_json_codec','  DEPENDS planar_ledger_format planar_history_interpretation_codec planar_history_recorded_interpretation planar_history_tree planar_json_codec')
a=a.replace('  PLANAR_HISTORY_FIXTURE="${PROJECT_SOURCE_DIR}/fixtures/render/quota")','  PLANAR_HISTORY_FIXTURE="${PROJECT_SOURCE_DIR}/fixtures/render/quota"\n  PLANAR_V2_VECTORS="${PROJECT_SOURCE_DIR}/tests/vectors/interpretation-v2")')
assert a==b,'unexpected CMake change'
# Read the actual compiled dynamic dependency graph. Every reached module's implementation and interface imports count.
module_dirs={}
for d in (R/'build/rewrite-debug').glob('src/**/CMakeFiles/*.dir'):
 f=d/'CXXModules.json'
 if f.exists():
  data=json.loads(f.read_text())
  for name in data.get('modules',{}):module_dirs[name]=d
# Compiler-produced dyndeps list exact consumed PCM paths, independent of CMake link declarations.
closure=set();queue=['planar_history_read','planar_ledger_format'];dyndeps={}
while queue:
 target=queue.pop()
 if target in closure:continue
 closure.add(target)
 f=R/'build/rewrite-debug/src/engine/CMakeFiles'/ (target+'.dir')/'CXX.dd'
 assert f.exists(),f
 text=f.read_text();dyndeps[str(f.relative_to(R))]=text
 queue += re.findall(r'src/engine/CMakeFiles/(planar_[^/]+)\.dir/[^\s]+\.pcm',text)
assert not any((any(t in x for t in ['runtime','lua','closure_guard']) or x=='planar_capture_profile') for x in closure),closure
symbols=subprocess.run(['nm','-u',str(R/'build/rewrite-debug/src/engine/libplanar_history_read.a'),str(R/'build/rewrite-debug/src/engine/libplanar_ledger_format.a')],capture_output=True,text=True,check=True)
assert not re.search(r'\b_?lua(?:L)?_',symbols.stdout)
(O/'undefined-symbols.log').write_text(symbols.stdout+symbols.stderr)
(O/'no-lua-proof.json').write_text(json.dumps(dict(actual_compiler_dyndeps=dyndeps,closure=sorted(closure),no_lua_runtime_module_dependency=True,no_lua_undefined_symbols=True,existing_native_capture_dependency_for_writer_retained=True),indent=2)+'\n')
links=[]
for p in DOCS:
 text=(R/p).read_text()
 for match in re.finditer(r'(?<!!)\[[^\]]+\]\(([^)]+)\)',text):
  target=unquote(match.group(1).split('#')[0])
  if not target or '://' in target or target.startswith('mailto:'):continue
  assert (R/p).parent.joinpath(target).exists(),(p,target)
  links.append([p,target])
(O/'link-audit.json').write_text(json.dumps(dict(local_links=len(links),unresolved=[]),indent=2)+'\n')
assert sha(R/'src/engine/ledger/format.cpp')!=base['src/engine/ledger/format.cpp']['sha256']
red=json.loads((O/'red-source.json').read_text())
for p in ['src/engine/ledger/format.cpp','src/engine/history/read.cpp']:
 assert red[p]==base[p]['sha256'],p
 assert sha(O/('red-'+Path(p).name))==red[p]
diff=[]
for p in owned:
 before=(O/'preedit'/p).read_text()
 diff+=difflib.unified_diff(before.splitlines(True),(R/p).read_text().splitlines(True),fromfile='preedit/'+p,tofile=p)
(O/'candidate.diff').write_text(''.join(diff))
(O/'candidate.json').write_text(json.dumps(dict(files={p:sha(R/p) for p in owned},diff_sha256=sha(O/'candidate.diff')),sort_keys=True,indent=2)+'\n')
(O/'source-audit.json').write_text(json.dumps(dict(owned_files=len(owned),format_only_two_functions_and_import=True,pure_A1_A2_modules_unchanged=True,transaction_prepared_load_runtime_Lua_export_exact_contract_unchanged=True,CMake_only_required_dependencies_and_test_vector=True,historical_bodies_unchanged=9,AGENTS_symlink='CLAUDE.md',red_readers_match_original_baseline=True),indent=2)+'\n')
print('source, compiled dependency, symbol and link audits pass; candidate frozen')
