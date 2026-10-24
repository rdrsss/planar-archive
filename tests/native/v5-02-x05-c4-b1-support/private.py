"""Reversible SQLite copies: copied build inputs, explicit modules and original/RAII backup probes."""
from pathlib import Path
import json,shlex,shutil,subprocess,time,hashlib,stat,sys
R=Path(__file__).resolve().parents[3];B=R/'build/rewrite-debug';S=Path(__file__).parent;E=R/'tests/native/v5-02-x05-c4-b1-evidence';P=E/'private';D=E/'dependencies';P.mkdir(exist_ok=True)
def write(p,x):p.write_text(json.dumps(x,sort_keys=True,indent=2)+'\n')
def entry(p):return dict(mode=stat.S_IMODE(p.stat().st_mode),sha256=hashlib.sha256(p.read_bytes()).hexdigest())
def compiled():return {str(p.resolve()):entry(p) for p in B.rglob('*') if p.is_file() and p.suffix in {'.o','.pcm','.a','.dylib'}}
def expand(args):
 out=[]
 for a in args:
  if a.startswith('@'):out+=expand(shlex.split((B/a[1:]).read_text()))
  else:out.append(a)
 return out
cc=json.loads((B/'compile_commands.json').read_text())
def arguments(tail):return expand(shlex.split(next(x['command'] for x in cc if x['file'].endswith(tail))))
iface=arguments('ledger/sqlite.cppm');impl=arguments('ledger/sqlite.cpp');test=arguments('ledger/sqlite.t.cpp')
profile_args=arguments('capture/profile.t.cpp')
test += [a for a in profile_args if a.startswith('-fmodule-file=') and a not in test]
line=subprocess.check_output(['ninja','-t','commands','planar_ledger_sqlite_tests'],cwd=B,text=True).splitlines()[-1]
link=expand(shlex.split(line.split(' && ')[1]));libs=link[link.index('src/engine/libplanar_ledger_sqlite.a'):]
profileline=subprocess.check_output(['ninja','-t','commands','planar_capture_profile_tests'],cwd=B,text=True).splitlines()[-1]
profilelink=expand(shlex.split(profileline.split(' && ')[1]));extra=profilelink[profilelink.index('src/engine/libplanar_capture_profile.a'):]
libs += [a for a in extra if a not in libs]
before=compiled();members=dict(before)
for a in iface+impl+test:
 if a.startswith('-fmodule-file='):
  path=(B/a[len('-fmodule-file='):].split('=',1)[1]).resolve();members[str(path)]=entry(path)
for p in Path('/opt/homebrew/opt/llvm/lib/c++').glob('*.dylib'):members[str(p.resolve())]=entry(p.resolve())
write(P/'compiled-before.json',members)
def dest(p):return D/str(Path(p).resolve()).lstrip('/')
for k,v in members.items():
 p=dest(k);p.parent.mkdir(parents=True,exist_ok=True)
 if not p.exists() or entry(p)!=v:shutil.copy2(k,p)
 assert entry(p)==v
commands=[]
def run(label,args,expected=0):
 start=time.monotonic();p=subprocess.run(list(map(str,args)),cwd=B,capture_output=True,text=True)
 (P/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=list(map(str,args)),cwd=str(B),exit=p.returncode,seconds=time.monotonic()-start));write(P/'commands.json',commands);print(label,p.returncode,flush=True)
 assert p.returncode==expected,(p.stdout+p.stderr)[-4000:]
def redirect(args,source,out,custom=False):
 a=list(args)
 for i,v in enumerate(a):
  if v.startswith('-fmodule-file='):
   k,p=v[len('-fmodule-file='):].split('=',1);a[i]='-fmodule-file='+k+'='+str(P/'sqlite.pcm' if custom and k=='planar.engine.ledger.sqlite' else dest(B/p))
  elif v.startswith('-fmodule-output='):a[i]='-fmodule-output='+str(P/'sqlite.pcm')
 for flag,extension in [('-o','.o'),('-MF','.d'),('-MT','.o')]:
  if flag in a:a[a.index(flag)+1]=str(P/(out+extension))
 a[a.index('-c')+1]=str(source);a+=['-fno-implicit-modules','-fno-implicit-module-maps','-I'+str(S)]
 return a
interface=(R/'src/engine/ledger/sqlite.cppm').read_text().replace('  /// @brief Inspect first gate status', '  void* private_handle();\n  /// @brief Inspect first gate status',1)
interface=interface.replace('} // namespace planar::engine::ledger','std::size_t private_context_size();\nint private_backup_finishes();\nbool private_backup_live();\nvoid private_backup_cleanup();\n} // namespace planar::engine::ledger')
(P/'sqlite.cppm').write_text(interface);run('interface',redirect(iface,P/'sqlite.cppm','interface'))
original=(R/'src/engine/ledger/sqlite.cpp').read_text()
helpers='''
static sqlite3_backup* c4_live = nullptr;
static int c4_finished = 0;
static sqlite3_backup* c4_init(sqlite3* a,const char* b,sqlite3* c,const char* d) { return c4_live=sqlite3_backup_init(a,b,c,d); }
static int c4_finish(sqlite3_backup* p) { ++c4_finished;c4_live=nullptr;return sqlite3_backup_finish(p); }
'''
for label in ['red','green']:
 s=original
 if label=='red':
  start=s.index('  struct backup_owner {');end=s.index('  const int stepped =',start)
  s=s[:start]+s[end:];s=s.replace('const int finished = owner.finish();','const int finished = sqlite3_backup_finish(backup);')
 if label=='red':
  start=s.index('std::expected<void, sqlite_error> connection::online_backup_to');end=s.index('std::expected<transaction, sqlite_error> connection::begin',start)
  prior=(E/'preedit-sqlite.cpp').read_text();a=prior.index('std::expected<void, sqlite_error> connection::online_backup_to');b=prior.index('std::expected<transaction, sqlite_error> connection::begin',a)
  assert s[start:end]==prior[a:b], 'red backup body must exactly equal original preimage'
 s=s.replace('sqlite3_backup_init(', 'c4_init(').replace('sqlite3_backup_finish(', 'c4_finish(')
 s=s.replace('module planar.engine.ledger.sqlite;',helpers+'\nmodule planar.engine.ledger.sqlite;',1)
 s+='''\nnamespace planar::engine::ledger {
void* connection::private_handle(){return _state->handle;}
std::size_t private_context_size(){return sizeof(bound_context);}
int private_backup_finishes(){return c4_finished;}
bool private_backup_live(){return c4_live != nullptr;}
void private_backup_cleanup(){if(c4_live)c4_finish(c4_live);}
}\n'''
 (P/(label+'.cpp')).write_text(s);run(label+'-implementation',redirect(impl,P/(label+'.cpp'),label,True))
run('probes-compile',redirect(test,S/'private-probe.cpp','probe',True))
libs=[str(dest(B/a)) if a.endswith(('.o','.a','.pcm','.dylib')) else a for a in libs if a!='src/engine/libplanar_ledger_sqlite.a']
common=[link[0],'-stdlib=libc++','-arch','arm64','-L'+str(dest('/opt/homebrew/opt/llvm/lib/c++/libc++.1.dylib').parent),'-Wl,-rpath,'+str(dest('/opt/homebrew/opt/llvm/lib/c++/libc++.1.dylib').parent),'-Wl,-rpath,'+str(dest(B/'src/engine/libplanar_admission_syscalls.dylib').parent)]
for label in ['red','green']:
 run(label+'-link',common+[P/'probe.o',P/'interface.o',P/(label+'.o'),*libs,'-o',P/label])
 run(label+'-backup',[P/label,'[c4b1-backup]','--reporter','junit','--out',P/(label+'-backup.xml')],42 if label=='red' else 0)
for label in ['direct','receipt','new-alloc','open-alloc']:
 run('green-'+label,[P/'green','[c4b1-'+label+']','--reporter','junit','--out',P/('green-'+label+'.xml')])
assert compiled()==before
assert all(entry(Path(k))==v and entry(dest(k))==v for k,v in members.items())
assert {str(p) for p in D.rglob('*') if p.is_file()}=={str(dest(k)) for k in members}
write(P/'dependency-final.json',dict(members=len(members),shared_and_copied_unchanged=True,copied_membership_exact=True))
