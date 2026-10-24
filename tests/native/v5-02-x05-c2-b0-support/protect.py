"""Freeze standalone support; compare source, historical evidence and both builds."""
from pathlib import Path
import hashlib,json,os,sys
R=Path(__file__).resolve().parents[3];E=R/'tests/native/v5-02-x05-c2-b0-evidence';S=Path(__file__).parent

def sha(p):
 h=hashlib.sha256()
 with p.open('rb')as f:
  for b in iter(lambda:f.read(1024*1024),b''):h.update(b)
 return h.hexdigest()
def write(p,v):p.write_text(json.dumps(v,indent=2,sort_keys=True)+'\n')
def support():return {str(p.relative_to(R)):sha(p)for p in S.rglob('*')if p.is_file()}
def check_support():assert support()==json.loads((E/'frozen-support.json').read_text()),'frozen support changed'
if __name__=='__main__':
 if sys.argv[1]=='freeze':write(E/'frozen-support.json',support());print('frozen',len(support()));sys.exit()
 check_support();counts={}
 for baseline in ['source','tests','compiled','rewrite-debug','rewrite-release']:
  old=json.loads((E/(baseline+'-before.json')).read_text());now={}
  if baseline=='source':
   for top in ['src','vendor','cmake','scripts','fixtures','docs']:
    now.update({str(p.relative_to(R)):sha(p)for p in (R/top).rglob('*')if p.is_file()})
   now.update({p.name:sha(p)for p in R.iterdir()if p.is_file()})
  elif baseline=='tests':now={str(p.relative_to(R)):sha(p)for p in (R/'tests').rglob('*')if p.is_file()and not any(s.startswith('v5-02-x05-c2-b0-')for s in p.parts)}
  else:
   build=R/'build'/('rewrite-debug'if baseline=='compiled'else baseline)
   now={str(p.relative_to(R)):sha(p)for p in build.rglob('*')if p.is_file()and (baseline=='rewrite-release'or p.suffix in ({'.o','.pcm','.a'}if baseline=='compiled'else{'.o','.pcm'})or (baseline=='rewrite-debug'and p.name in ['compile_commands.json','build.ninja']))}
  assert now==old,(baseline,sorted(k for k in set(now)|set(old)if now.get(k)!=old.get(k)))
  counts[baseline]=len(now);print(baseline,len(now),'unchanged',flush=True)
 assert (R/'AGENTS.md').is_symlink()and os.readlink(R/'AGENTS.md')=='CLAUDE.md'
 write(E/'protected-final.json',dict(counts=counts,bytes_and_membership_unchanged=True,agents_symlink_preserved=True,support_frozen=True))
