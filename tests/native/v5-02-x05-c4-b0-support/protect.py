from pathlib import Path
import hashlib,json,os,stat,sys
R=Path(__file__).resolve().parents[3]; O=R/'tests/native/v5-02-x05-c4-b0-evidence'
def entry(p):
 d={'mode':stat.S_IMODE(p.lstat().st_mode)}
 if p.is_symlink():d['link']=os.readlink(p)
 elif p.is_file():d['sha256']=hashlib.sha256(p.read_bytes()).hexdigest()
 else:d['directory']=True
 return d
def snap():
 d={}
 for top in ['src','vendor','cmake','scripts','fixtures','docs','tests','build/rewrite-release','build/install-release','CLAUDE.md','AGENTS.md','CMakeLists.txt','CMakePresets.json','Makefile']:
  p=R/top
  if not p.exists():d[top]={'absent':True};continue
  for q in [p,*p.rglob('*')] if p.is_dir() else [p]:
   k=str(q.relative_to(R))
   if k.startswith('tests/native/v5-02-x05-c4-b0-') or k=='docs/vernacular/rounds/V5-02-X05-C4-B.md':continue
   d[k]=entry(q)
 return d
def write(p,d):p.write_text(json.dumps(d,sort_keys=True,indent=2)+'\n')
if __name__=='__main__':
 if sys.argv[1]=='before':write(O/'protected-before.json',snap())
 else:
  a=json.loads((O/'protected-before.json').read_text());b=snap();delta=[k for k in a.keys()|b.keys() if a.get(k)!=b.get(k)];write(O/'protection-final.json',{'entries':len(a),'changed':delta});assert not delta,delta
