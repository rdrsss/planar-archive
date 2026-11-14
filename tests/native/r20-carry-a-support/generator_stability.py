# R20-CARRY-A: generators must be write-only-if-different (I4-I11, I24, I24b).
"""argv: <check>_<generator>, repository root.

unchanged_mtime_*: rerun over an unchanged scratch root leaves every output's st_mtime_ns and bytes untouched.
content_change_rewrites_*: a stale or altered output is rewritten to the exact first-run bytes with a newer mtime, and a
changed scratch input rewrites its derived outputs. mtimes are observed by os.utime backdating, never by sleeping.
Output sets are hard-coded and compared by equality and count. A set mismatch is a test bug, not a red.
"""
from pathlib import Path
import json,sys
sys.dont_write_bytecode=True
from common import OLD_NS,MARKER,expect,scratch,run_ok,copy_inputs,append_marker,listing,snapshot,backdate,dispatch
repo=Path(sys.argv[2]).resolve()
T='tests/native/'
RT=['src/engine/runtime/runtime.cpp','src/engine/runtime/runtime.cppm','src/stdlib/planar/commands.lua']
FAMILIES=['render','init','history','export','load','extension','activate','status','preview','claim','takeover','refresh',
 'checkpoint','release','recover','check','check-recover','verdict']
CLI_PATHS=['bootstrap.cpp','bootstrap.cppm','invocation.cpp','invocation.cppm']+[f'handlers/{f}/command.{e}' for f in FAMILIES for e in ('cpp','cppm')]
CLI_INPUTS=[f'src/cmd/planar/{p}' for p in CLI_PATHS]+['src/cmd/planar/main.cpp']

def triple(n,lo=0):return {f'{k}_{i}.{e}' for i in range(lo,n) for k,e in (('commands','lua'),('runtime','cpp'),('runtime','cppm'))}

# name -> dict(script, inputs copied into the scratch root, argv builder, exact output set, stale kinds, input -> derived outputs)
GENS={
 'b1':dict(script=T+'v5-02-b1-support/generate.py',inputs=RT+[T+'v5-02-b1-support/altered.lua'],out=triple(21),count=63,
  stale=['commands_0.lua','runtime_0.cpp','runtime_0.cppm'],
  derived={'src/stdlib/planar/commands.lua':[f'commands_{i}.lua' for i in range(21)],
   'src/engine/runtime/runtime.cpp':[f'runtime_{i}.cpp' for i in range(21)],'src/engine/runtime/runtime.cppm':[f'runtime_{i}.cppm' for i in range(21)]}),
 'd6':dict(script=T+'r14-d6-support/generate.py',inputs=RT+[T+'r14-d6-support/load.lua'],out=triple(6),count=18,
  stale=['commands_0.lua','runtime_0.cpp','runtime_0.cppm'],
  # variant 4 truncates the appended tail (rsplit on the last 'end'), so it cannot carry an appended marker.
  derived={'src/stdlib/planar/commands.lua':[f'commands_{i}.lua' for i in (0,1,2,3,5)],
   'src/engine/runtime/runtime.cpp':[f'runtime_{i}.cpp' for i in range(6)],'src/engine/runtime/runtime.cppm':[f'runtime_{i}.cppm' for i in range(6)]}),
 'd6_c01':dict(script=T+'r14-d6-c01-support/generate.py',inputs=RT+[T+'r14-d6-support/load.lua',T+'r14-d6-support/generate.py'],
  out={'commands_0.lua','runtime_0.cpp','runtime_0.cppm'},count=3,stale=['commands_0.lua','runtime_0.cpp','runtime_0.cppm'],
  derived={'src/stdlib/planar/commands.lua':['commands_0.lua'],'src/engine/runtime/runtime.cpp':['runtime_0.cpp'],
   'src/engine/runtime/runtime.cppm':['runtime_0.cppm']}),
 'x05_a0':dict(script=T+'v5-02-x05-a0-support/generate.py',inputs=RT,
  out={'x05_a0_0_runtime.cpp','x05_a0_0_runtime.cppm','x05_a0_1_runtime.cpp','x05_a0_1_runtime.cppm','commands_0.lua','commands_1.lua','generation.json'},count=7,
  stale=['commands_1.lua','x05_a0_1_runtime.cpp','x05_a0_0_runtime.cppm','generation.json'],
  derived={'src/stdlib/planar/commands.lua':['commands_0.lua','commands_1.lua'],
   'src/engine/runtime/runtime.cpp':['x05_a0_0_runtime.cpp','x05_a0_1_runtime.cpp'],
   'src/engine/runtime/runtime.cppm':['x05_a0_0_runtime.cppm','x05_a0_1_runtime.cppm']}),
}
LABEL={'b1':'B1 generator','d6':'D6 generator','d6_c01':'D6-C01 generator','x05_a0':'X05-A0 generator'}

def comment_for(rel):return '--' if rel.endswith('.lua') else '//'

def generate(g,root,out):
 run_ok([sys.executable,repo/g['script'],root,out],f"{g['label']} run")

def setup(key,tmp):
 g=dict(GENS[key]);g['label']=LABEL[key]
 root=tmp/'root';out=tmp/'out';copy_inputs(repo,root,g['inputs'])
 return g,root,out

def first_run(key,tmp):
 g,root,out=setup(key,tmp)
 generate(g,root,out)
 names=set(listing(out))
 expect(names==g['out'] and len(names)==g['count'],f"{g['label']}: output set mismatch (expected {g['count']} files): missing {sorted(g['out']-names)[:5]} extra {sorted(names-g['out'])[:5]} count {len(names)}")
 snap=snapshot(out)
 expect(all(len(b)>0 for b,_ in snap.values()),f"{g['label']}: empty output")
 return g,root,out,snap

def unchanged(key):
 with scratch('r20-gen-') as tmp:
  g,root,out,first=first_run(key,tmp)
  backdate(out);generate(g,root,out)
  after=snapshot(out)
  expect(set(after)==set(first),f"{g['label']}: output set changed on rerun")
  moved=sorted(n for n,(b,m) in after.items() if m!=OLD_NS)
  expect(not moved,f"{g['label']} rewrote {len(moved)} of {len(after)} unchanged outputs (mtime moved), first: {moved[:3]}")
  changed=sorted(n for n,(b,m) in after.items() if b!=first[n][0])
  expect(not changed,f"{g['label']}: bytes changed on rerun: {changed[:3]}")

def content_change(key):
 with scratch('r20-gen-') as tmp:
  g,root,out,first=first_run(key,tmp)
  label=g['label']
  # (i) stale outputs of each kind are restored to the exact first-run bytes with a newer mtime. Untouched outputs are not
  # asserted here: the whole-set mtime stability is the unchanged_mtime check.
  for name in g['stale']:(out/name).write_bytes(b'stale r20 bytes\n')
  backdate(out);generate(g,root,out)
  after=snapshot(out)
  expect(set(after)==set(first),f'{label}: output set changed after stale rewrite')
  for name in g['stale']:
   expect(after[name][0]==first[name][0],f'{label}: stale {name} not rewritten to the exact first-run bytes')
   expect(after[name][1]>OLD_NS,f'{label}: stale {name} rewritten without a newer mtime')
  # (ii) a changed input rewrites every derived output.
  for rel,derived in g['derived'].items():
   backdate(out);append_marker(root/rel,comment_for(rel));before=snapshot(out)
   generate(g,root,out)
   now=snapshot(out)
   expect(set(now)==g['out'],f'{label}: output set changed after input change {rel}')
   for name in derived:
    expect(now[name][0]!=before[name][0],f'{label}: {name} did not change after input change {rel}')
    expect(MARKER in now[name][0].decode(),f'{label}: {name} lacks the changed input text after {rel}')
    expect(now[name][1]>OLD_NS,f'{label}: {name} not rewritten with a newer mtime after {rel}')
   restored=(repo/rel).read_bytes()
   (root/rel).write_bytes(restored)

# Manifest generators: each writes generation.json; the first argument is a source directory or a repository root.
def per_variant(n):
 base=['commands.lua','runtime.cpp','runtime.cppm','main.cpp']+CLI_PATHS
 return {f'{i}/{b}' for i in range(n) for b in base}
MANIFESTS={
 'r12c2':dict(script=T+'r12c2-support/generate.py',kind='dir',src='src/engine/ledger',files=['format.cppm','format.cpp'],
  out={'format.cppm','format.cpp','generation.json'},count=3,change='format.cpp',derived=['format.cpp']),
 'r14f1':dict(script=T+'r14f1-support/generate.py',kind='dir',src='src/engine/export',files=['source_export.cppm','source_export.cpp'],
  out={'source_export.cppm','source_export.cpp','generation.json'},count=3,change='source_export.cpp',derived=['source_export.cpp']),
 'r14f2':dict(script=T+'r14f2-support/generate.py',kind='dir',src='src/cmd/planar',files=['invocation.cppm','invocation.cpp'],
  out={'invocation.cppm','invocation.cpp','generation.json'},count=3,change='invocation.cpp',derived=['invocation.cpp']),
 'v5-02-b2':dict(script=T+'v5-02-b2-support/generate.py',kind='root',files=RT+CLI_INPUTS,
  out=per_variant(6)|{'generation.json'},count=265,change='src/engine/runtime/runtime.cpp',derived=[f'{i}/runtime.cpp' for i in range(6)]),
 'r14-d7-b':dict(script=T+'r14-d7-b-support/generate.py',kind='root',files=RT+CLI_INPUTS,
  out={b for b in per_variant(1) if True}|{'generation.json'},count=45,change='src/engine/runtime/runtime.cpp',derived=['runtime.cpp']),
 'v5-02-x07':dict(script=T+'v5-02-x07-support/generate.py',kind='root',files=RT+CLI_INPUTS,
  out=per_variant(5)|{'generation.json'},count=221,change='src/engine/runtime/runtime.cpp',derived=[f'{i}/runtime.cpp' for i in range(5)]),
}
# d7b writes flat names (no variant directory): strip the '0/' prefix of the one-variant template.
MANIFESTS['r14-d7-b']['out']={n[2:] if n.startswith('0/') else n for n in MANIFESTS['r14-d7-b']['out']}

def m_setup(key,tmp):
 m=MANIFESTS[key];out=tmp/'out'
 if m['kind']=='dir':
  src=tmp/'src';copy_inputs(repo/m['src'],src,m['files']);first_arg=src;changed=src/m['change']
 else:
  src=tmp/'root';copy_inputs(repo,src,m['files']);first_arg=src;changed=src/m['change']
 return m,first_arg,out,changed

def m_generate(key,m,first_arg,out):
 run_ok([sys.executable,repo/m['script'],first_arg,out],f'manifest generator {key} run')

def m_first(key,tmp):
 m,first_arg,out,changed=m_setup(key,tmp);m_generate(key,m,first_arg,out)
 names=set(listing(out))
 expect(names==m['out'] and len(names)==m['count'],f"manifest generator {key}: output set mismatch (expected {m['count']}): missing {sorted(m['out']-names)[:5]} extra {sorted(names-m['out'])[:5]} count {len(names)}")
 expect(all(len(b)>0 for b,_ in snapshot(out).values()),f'manifest generator {key}: empty output')
 return m,first_arg,out,changed,snapshot(out)

def manifests_unchanged():
 failures=[]
 for key in MANIFESTS:
  with scratch('r20-man-') as tmp:
   m,first_arg,out,changed,first=m_first(key,tmp)
   backdate(out);m_generate(key,m,first_arg,out);after=snapshot(out)
   moved=sorted(n for n,(b,t) in after.items() if t!=OLD_NS)
   altered=sorted(n for n,(b,t) in after.items() if b!=first[n][0])
   if set(after)!=set(first):failures.append(f'{key}: output set changed on rerun')
   if moved:failures.append(f'{key}: rewrote {moved[:3]} (mtime moved) of {len(after)} outputs')
   if altered:failures.append(f'{key}: bytes changed {altered[:3]}')
 expect(not failures,'manifest generators not write-if-different: '+'; '.join(failures))

def manifests_content_change():
 failures=[]
 for key in MANIFESTS:
  with scratch('r20-man-') as tmp:
   m,first_arg,out,changed,first=m_first(key,tmp)
   # (i) altered generation.json goes back to the first-run bytes with a newer mtime; every other output keeps the backdate.
   (out/'generation.json').write_text('{"stale": true}\n')
   backdate(out);m_generate(key,m,first_arg,out);after=snapshot(out)
   if after['generation.json'][0]!=first['generation.json'][0]:failures.append(f'{key}: generation.json not restored to first-run bytes')
   if after['generation.json'][1]<=OLD_NS:failures.append(f'{key}: stale generation.json rewritten without newer mtime')
   others=sorted(n for n,(b,t) in after.items() if n!='generation.json' and t!=OLD_NS)
   if others:failures.append(f'{key}: untouched outputs rewritten {others[:3]}')
   # (ii) a changed input rewrites generation.json and the derived outputs.
   backdate(out);append_marker(changed,'//');before=snapshot(out);m_generate(key,m,first_arg,out);now=snapshot(out)
   if now['generation.json'][0]==before['generation.json'][0]:failures.append(f'{key}: generation.json unchanged after input change')
   if now['generation.json'][1]<=OLD_NS:failures.append(f'{key}: generation.json not rewritten after input change')
   for name in m['derived']:
    if MARKER not in now[name][0].decode() or now[name][1]<=OLD_NS:failures.append(f'{key}: {name} not rewritten after input change')
 expect(not failures,'manifest generators: '+'; '.join(failures))

CASES={}
for k in GENS:
 CASES[f'unchanged_mtime_{k}']=lambda k=k:unchanged(k)
 CASES[f'content_change_rewrites_{k}']=lambda k=k:content_change(k)
CASES['unchanged_mtime_manifests']=manifests_unchanged
CASES['content_change_rewrites_manifests']=manifests_content_change
dispatch(CASES,sys.argv)
