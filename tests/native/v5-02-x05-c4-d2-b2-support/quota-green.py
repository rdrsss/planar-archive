"""Recompile only the corrected copied admission under the same controlled file ceiling."""
from pathlib import Path
import json,subprocess,time,hashlib,stat
R=Path(__file__).resolve().parents[3];B=R/'build/rewrite-debug';E=R/'tests/native/v5-02-x05-c4-d2-b2-evidence';P=E/'quota-regression';D=E/'quota-dependencies'
def entry(p):return dict(mode=stat.S_IMODE(p.stat().st_mode),sha256=hashlib.file_digest(p.open('rb'),'sha256').hexdigest(),bytes=p.stat().st_size,mtime_ns=p.stat().st_mtime_ns)
def write(p,x):p.write_text(json.dumps(x,sort_keys=True,indent=2)+'\n')
members=json.loads((P/'compiled-before.json').read_text());dest=lambda p:D/str(Path(p).resolve()).lstrip('/')
assert all(entry(Path(k))==v and entry(dest(k))==v for k,v in members.items())
assert {str(p) for p in D.rglob('*') if p.is_file()}=={str(dest(k)) for k in members}
journals=json.loads((P/'journals-before.json').read_text());assert {n:entry(B/n) for n in journals}==journals
commands=[]
def run(label,args):
 start=time.monotonic();p=subprocess.run(list(map(str,args)),cwd=B,capture_output=True,text=True)
 (P/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=list(map(str,args)),cwd=str(B),exit=p.returncode,seconds=time.monotonic()-start));write(P/'green-commands.json',commands);print(label,p.returncode,flush=True);assert p.returncode==0,(p.stdout+p.stderr)[-4000:]
a=json.loads((P/'admission-command.json').read_text());source=(R/'src/engine/capture/admission.cpp').read_text()
needle='return retained_root_receipt(_impl, i);';assert source.count(needle)==1
(P/'controlled-after.cpp').write_text(source.replace(needle,'if (slot == "quota") _impl->files = max_files;\n        '+needle))
a[a.index('-c')+1]=str(P/'controlled-after.cpp')
for flag,ext in [('-o','.o'),('-MF','.d'),('-MT','.o')]:
 if flag in a:a[a.index(flag)+1]=str(P/('green-admission'+ext))
run('controlled-after-compile',a)
old=json.loads((P/'commands.json').read_text());link=next(x['argv'] for x in old if x['label']=='controlled-before-link')
link=[str(P/'green-admission.o') if x==str(P/'quota-admission.o') else str(P/'after') if x==str(P/'before') else x for x in link]
run('controlled-after-link',link)
run('controlled-after',[P/'after','[d2-b2-quota]','--reporter','junit','--out',P/'after.xml'])
assert all(entry(Path(k))==v and entry(dest(k))==v for k,v in members.items())
assert {n:entry(B/n) for n in journals}==journals
write(P/'green-dependency-final.json',dict(members=len(members),shared_and_copied_unchanged=True,ninja_journals_unchanged=True,controlled_limit='files=max_files only when issuing root slot quota',actual_10000_fixture_claimed=False))
