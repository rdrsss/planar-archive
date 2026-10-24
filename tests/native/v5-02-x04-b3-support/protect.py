"""Root-only filesystem protection and owned candidate snapshots for X04-B3."""
from pathlib import Path
import hashlib,json,os,shutil,stat,subprocess,sys
ROOT=Path(__file__).resolve().parents[3]
OUT=ROOT/'tests/native/v5-02-x04-b3-evidence'
OWNED=['src/engine/history/effective_provenance.'+e for e in ['cppm','cpp','t.cpp']]+['src/engine/CMakeLists.txt','CLAUDE.md','docs/architecture.md']+['docs/vernacular/'+n+'.md' for n in ['implementation','roadmap','v5-alignment','acceptance-matrix','external-profile-implementation']]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(n,v):(OUT/n).write_text(json.dumps(v,indent=2,sort_keys=True)+'\n')
def filesystem():
 rows={}
 for current,dirs,files in os.walk(ROOT,followlinks=False):
  base=Path(current)
  dirs[:]=[n for n in dirs if not (base==ROOT and n in ['.git','build']) and base/n!=OUT]
  for n in files+list(dirs):
   p=base/n; key=str(p.relative_to(ROOT)); s=p.lstat()
   if stat.S_ISLNK(s.st_mode):rows[key]={'symlink':os.readlink(p),'mode':stat.S_IMODE(s.st_mode)}
   elif stat.S_ISREG(s.st_mode):rows[key]={'sha256':sha(p),'mode':stat.S_IMODE(s.st_mode)}
 return rows
if __name__=='__main__':
 stage=sys.argv[1]
 write(stage+'-filesystem.json',filesystem())
 for name,args in [('head',['rev-parse','HEAD']),('branch',['branch','--show-current']),('status',['status','--short']),('diff',['diff','--binary'])]:
  (OUT/(stage+'-'+name+('.patch' if name=='diff' else '.txt'))).write_bytes(subprocess.check_output(['git',*args],cwd=ROOT))
 dest=OUT/(stage+'-owned'); dest.mkdir(exist_ok=False)
 for n in OWNED:
  p=ROOT/n
  if p.exists():
   target=dest/n;target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,target)
 write(stage+'-exclusions.json',{'root_only':['.git','build'],'evidence':str(OUT.relative_to(ROOT))})
