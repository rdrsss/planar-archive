"""X08 adversarial binary acceptance: hard links, extension ordering, job mixes, registry aliases and resource limits.

argv: ctest case, production planar. Every case runs real processes in its own scratch tree with HOME and PLANAR_DB
redirected; the forbidden legacy database path is asserted never created. Expected refusals are pinned to exact closed
bytes taken from the external profile contract, bootstrap.t.cpp, response.cpp, envelope.cpp or the shipped Lua
semantic and schema modules. A failing guard is a product defect candidate: never weaken a pin to make it pass.
"""
from pathlib import Path
import json,os,shutil,sqlite3,subprocess,sys,tempfile
case,planar=sys.argv[1],str(Path(sys.argv[2]).resolve())
repo=Path(__file__).resolve().parents[2]
worlds=[]
def canonical(value):return json.dumps(value,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()+b'\n'
def closed(**fields):return canonical({'error':fields,'kind':'error','version':1})
def expect(got,status,stderr=None):
 """Exit status, empty stdout and exact stderr bytes; a refusal never prints a host path or traceback."""
 assert got.returncode==status and got.stdout==b'' and (stderr is None or got.stderr==stderr),(got.returncode,got.stdout[:200],got.stderr[:600])
 assert b'traceback' not in got.stderr and b'Traceback' not in got.stderr
class World:
 """One scratch tree: root/{work, anc/trusted/modules, fx}; fx is a private copy of the quota fixture."""
 selector='../anc/trusted/p.json'
 def __init__(self,root):
  self.root=Path(root).resolve();self.work=self.root/'work';self.trusted=self.root/'anc/trusted';self.fx=self.root/'fx'
  self.work.mkdir(parents=True);(self.trusted/'modules').mkdir(parents=True)
  shutil.copytree(repo/'fixtures/render/quota',self.fx)
  (self.trusted/'p.json').write_text('{"version":1,"modules":["modules/p.lua"]}');(self.trusted/'modules/p.lua').write_text('-- profile v1\n')
  self.env={**os.environ,'HOME':str(self.root),'PLANAR_DB':str(self.root/'forbidden-legacy.db')}
  self.env.pop('PLANAR_WORKSPACE_DIR',None)
  worlds.append(self)
 def ws(self):return self.fx/'workspace/workspace.lua'
 def reg(self):return self.fx/'registry/quota-tools.json'
 def module(self,text):
  data=text if isinstance(text,bytes) else text.encode()
  (self.trusted/'modules/p.lua').write_bytes(data)
 def run(self,args):
  return subprocess.run([planar,*args],capture_output=True,env=self.env,cwd=self.work,timeout=60)
 def init(self):
  made=self.run(['init','--ledger',str(self.work/'led.db'),'--workspace-id','quota']);assert made.returncode==0,made
  return self.work/'led.db'
def rows(ledger):
 with sqlite3.connect(ledger) as db:return tuple(db.execute('SELECT COUNT(*) FROM '+n).fetchone()[0] for n in ['revisions','requests'])
def with_profile(args):return ['with-profile','--stdlib-profile',World.selector,*args]
def render(w,ws=None,reg=None,root=None,job='combined',profile=True):
 args=['render','--workspace',str(ws or w.ws()),'--registry',str(reg or w.reg()),'--root','source='+str(root or w.fx/'current'),'--job',job,'--role','builder']
 return with_profile(args) if profile else args
def load(w,key,ws=None,reg=None,head='none',profile=True):
 args=['load','--ledger',str(w.work/'led.db'),'--workspace',str(ws or w.ws()),'--registry',str(reg or w.reg()),'--expect-revision',head,'--request-id',key,'--actor','driver']
 return with_profile(args) if profile else args
def add_jobs(w,jobs,custom=()):
 """Route the workspace to jobs [(id, file)]; fixture files keep their bytes, derived files copy combined with a new ID."""
 ws=w.ws();text=ws.read_text()
 old='jobs = L { { id = "combined", file = "combined.lua" }, { id = "split", file = "split.lua" } },'
 assert text.count(old)==1
 ws.write_text(text.replace(old,'jobs = L { %s },'%', '.join('{ id = "%s", file = "%s" }'%(i,f) for i,f in jobs)))
 base=(repo/'fixtures/render/quota/workspace/combined.lua').read_text()
 for identifier,name in jobs:
  if name in ('combined.lua','split.lua'):body=(repo/'fixtures/render/quota/workspace'/name).read_text()
  else:body=base.replace('id = "combined"','id = "%s"'%identifier)
  if identifier in custom:body=body.replace('shape = "fanout_review"','shape = "custom"')
  (w.fx/'workspace'/name).write_text(body)
def replace_once(path,old,new):
 text=path.read_text();assert text.count(old)==1,(str(path.name),old)
 path.write_text(text.replace(old,new))
def shape(body):
 return ("local D={id='custom',abi='shape_v1',revision='final',stages='optional',help='shape'}\n"
         "registration.shape(D,function(input) "+body+" end)\n")
OK_SHAPE=shape("return {ok=true,value={obligations=engine.array{},guidance='custom workflow guidance'}}")
def obligations(count,producer='library',consumer='command'):
 return shape("local o={} for i=1,"+str(count)+" do o[i]={id='o'..i,producer='"+producer+"',consumer='"+consumer+
              "',phase='start',quality='done'} end return {ok=true,value={obligations=engine.array(o),guidance='g'}}")
def opt(name,kind,required=False):
 return "{name='--%s',type='%s',required=%s,help='o'}"%(name,kind,'true' if required else 'false')
def command(name,options,capabilities,body):
 return ("registration.add({name='%s',family='extension_read_v1',handler_id='custom.%s',options=engine.array{%s},help='h',"
         "response_kind='extension_v1',capabilities=engine.array{%s}},function(inv,ctx) %s end)\n")%(
         name,name,','.join(sorted(options)),capabilities,body)
ROLE=[opt('workspace','text',True),opt('registry','text',True),opt('root','text',True),opt('job','text',True),opt('role','text',True),
      opt('base-root','text'),opt('observations','text'),opt('budget-bytes','decimal'),opt('capture-outputs','flag')]
# The late-slot command raises a marker when its handler runs; the marker must never surface.
LATE_COMMAND=command('lateread',ROLE,"'role_context_read_v1'","error('X08-HANDLER-RAN')")
CHUNK_RAN='error("X08-CHUNK-RAN")\n'
INTERNAL=closed(code='internal',field='',message='Invalid external profile protocol')
UNSAFE=closed(code='unsafe_path',field='',message='External profile overlaps a supplied input')
def adv_hardlink_module_refuses_before_chunk(temp):
 # Matrix row 3 sub-clause: the selected module is a hard link of the selected declaration (disjoint directories).
 # The declaration bytes raise the marker if they ever run as a module, so the initial admission must refuse first.
 w=World(temp/'main');w.ws().write_text(CHUNK_RAN)
 (w.trusted/'modules/p.lua').unlink();os.link(w.ws(),w.trusted/'modules/p.lua')
 ledger=w.init()
 expect(w.run(render(w)),3,UNSAFE)
 expect(w.run(load(w,'hardlink')),3,UNSAFE)
 assert rows(ledger)==(0,0),rows(ledger)
 # Control: an ordinary module with the same marker reaches the chunk and surfaces as the internal protocol refusal.
 control=World(temp/'control');control.module(CHUNK_RAN)
 expect(control.run(render(control)),4,INTERNAL)
def adv_missing_declaration_initial_admission(temp):
 # Matrix row 3 sub-clause: an absent declaration (existing directory, missing file) refuses before registration.
 # Pin: X06 B3 A "missing declaration refuses with the declaration adapter" maps to io / Cannot inspect confined input.
 w=World(temp/'main');w.module(CHUNK_RAN);absent=w.fx/'workspace/absent.lua';assert not absent.exists()
 ledger=w.init()
 io=closed(code='io',details={'operation':'capture'},field='',message='Cannot inspect confined input')
 expect(w.run(render(w,ws=absent)),4,io)
 expect(w.run(load(w,'missing',ws=absent)),4,io)
 assert rows(ledger)==(0,0),rows(ledger)
 # Control: the same marker module with the declaration present reaches registration, so the chunk refusal appears.
 control=World(temp/'control');control.module(CHUNK_RAN)
 expect(control.run(render(control)),4,INTERNAL)
def adv_opaque_ordering_binary(temp):
 # Matrix row 3 opaque ordering. Mapping row 3 "opaque owner cannot transition to built-in load" has no binary observable:
 # the public with-profile path never hands an opaque owner to load. It is mapped to the native names in
 # bootstrap.t.cpp (X07 cmd sources open the profile owner through one factory call per branch) and recorded as a limit.
 w=World(temp);w.module(LATE_COMMAND)
 def late(root,extra=()):
  return with_profile(['lateread','--workspace',str(w.ws()),'--registry',str(w.reg()),'--root','source='+str(root),
                       '--job','combined','--role','builder',*extra])
 # (a) final syntax error and an overlapping late root: the CLI11 final parse precedes every late check.
 expect(w.run(late(w.trusted,['--bogus','1'])),2,
        closed(code='usage',field='',message='Expected command options or --help; see --help'))
 # (b) valid syntax and the same overlapping late root: unsafe_path before the handler, whose marker never surfaces.
 got=w.run(late(w.trusted))
 expect(got,3,UNSAFE);assert b'X08-HANDLER-RAN' not in got.stderr
 # (c) control: a non-overlapping root reaches the handler, whose error surfaces as the internal protocol refusal.
 expect(w.run(late(w.fx/'current')),4,INTERNAL)
def adv_invalid_unselected_job_refuses_all(temp):
 # An extra job that is invalid but never selected by the role still refuses the whole load with the same bytes with
 # and without the profile, and publishes no revision or request row.
 w=World(temp);w.module('-- profile v1\n')
 add_jobs(w,[('combined','combined.lua'),('split','split.lua'),('bad','bad.lua')])
 replace_once(w.fx/'workspace/bad.lua','version = 1, id = "bad"','version = 2, id = "bad"')
 ledger=w.init()
 refusal=closed(code='unsupported_version',details={'received':2,'supported':[1]},field='job.version',file='bad.lua',
                message='Unsupported schema version')
 expect(w.run(load(w,'refuse')),3,refusal)
 expect(w.run(load(w,'refuse',profile=False)),3,refusal)
 assert rows(ledger)==(0,0),rows(ledger)
def adv_job_mix_and_phase_failures(temp):
 failures=[]
 def check(name,fn):
  try:fn()
  except AssertionError as error:failures.append((name,str(error)[:800]))
 def accepted(w,key,expected_rows):
  got=w.run(load(w,key));assert got.returncode==0 and not got.stderr,(got.returncode,got.stdout[:200],got.stderr[:600])
  value=json.loads(got.stdout);assert got.stdout==canonical(value) and value['outcome']=='accepted' and value['revision']=='1',value
  assert rows(w.work/'led.db')==expected_rows,rows(w.work/'led.db')
 def refused_empty(w,key,stderr):
  expect(w.run(load(w,key)),3,stderr);assert rows(w.work/'led.db')==(0,0),rows(w.work/'led.db')
 # (a) one custom shape on five jobs loads and publishes one revision.
 five=[('combined','combined.lua'),('split','split.lua'),('j3','j3.lua'),('j4','j4.lua'),('j5','j5.lua')]
 w=World(temp/'five');w.module(OK_SHAPE);add_jobs(w,five,custom={i for i,_ in five});w.init()
 check('five custom jobs load',lambda:accepted(w,'five',(1,1)))
 # (b) a built-in and custom mix loads: combined is custom, split stays fanout_review.
 w=World(temp/'mix');w.module(OK_SHAPE);add_jobs(w,[('combined','combined.lua'),('split','split.lua')],custom={'combined'});w.init()
 check('built-in and custom mix loads',lambda:accepted(w,'mix',(1,1)))
 # (c) phase 0, route: a duplicate toolchain in the declaration refuses before any job is validated.
 w=World(temp/'route');w.module(OK_SHAPE);add_jobs(w,[('combined','combined.lua'),('split','split.lua')],custom={'combined'})
 replace_once(w.ws(),'toolchains = L { "python" }','toolchains = L { "python", "python" }');w.init()
 check('phase 0 route refusal',lambda:refused_empty(w,'route',closed(code='reference',details={},field='workspace.toolchains[2]',
                                                                       file='workspace.lua',message='Duplicate value')))
 # (c) phase 1, all-job validation: an invalid extra job refuses with the semantic code and publishes nothing.
 w=World(temp/'validate');w.module(OK_SHAPE);add_jobs(w,[('combined','combined.lua'),('split','split.lua'),('bad','bad.lua')],custom={'combined'})
 replace_once(w.fx/'workspace/bad.lua','targets = L { "library", "command" }','targets = L { "library", "nosuch" }');w.init()
 check('phase 1 validation refusal',lambda:refused_empty(w,'validate',closed(code='reference',details={},field='job.roles[2].targets',
                                                                              file='bad.lua',message='Unknown reviewer target: nosuch')))
 # (c) phase 2, shape check: a custom shape refusal for split after combined dispatched publishes nothing.
 w=World(temp/'shape');w.module(shape("if input.job.id=='split' then return {ok=false,error={code='reference',message='Policy requires a declaration',"
                                      "field='job.roles',file='job.lua',details={wanted='worker'}}} end "
                                      "return {ok=true,value={obligations=engine.array{},guidance='g'}}"))
 add_jobs(w,[('combined','combined.lua'),('split','split.lua')],custom={'combined','split'});w.init()
 check('phase 2 shape refusal',lambda:refused_empty(w,'shape',closed(code='reference',details={'wanted':'worker'},field='job.roles',
                                                                      file='job.lua',message='Policy requires a declaration')))
 # (d) obligation quota: 10001 obligations refuse with the existing per-job limit; a grouped cycle refuses with dependency_cycle.
 w=World(temp/'over');w.module(obligations(10001));add_jobs(w,[('combined','combined.lua')],custom={'combined'});w.init()
 check('obligation 10001 refusal',lambda:refused_empty(w,'over',closed(code='resource_limit',details={'limit':'shape_obligations_per_job','maximum':10000},
                                                                      field='job.shape',file='combined.lua',message='Shape obligation limit exceeded')))
 w=World(temp/'cycle');w.module(obligations(1,producer='command',consumer='library'));add_jobs(w,[('combined','combined.lua')],custom={'combined'});w.init()
 def cycle():
  got=w.run(load(w,'cycle'));expect(got,3);value=json.loads(got.stderr);assert got.stderr==canonical(value),got.stderr[:200]
  error=value['error'];assert error['code']=='dependency_cycle' and error['message']=='Unsatisfiable dependency phases',value
  cycle_path=error['details']['cycle'];assert len(cycle_path)>2 and cycle_path[0]==cycle_path[-1],cycle_path
  assert rows(w.work/'led.db')==(0,0)
 check('grouped dependency cycle refusal',cycle)
 # (d) obligation load at a large count. Repair note (X08 repair round): the exact 10000 boundary cannot load through the binary,
 # because the whole-invocation native allocation budget (native_bytes, 536870912) is exhausted at about 5500 obligations
 # (render of 10000 refuses resource_limit native_bytes; load of 10000 refuses capture_changed). The 10000/10001 quota boundary
 # is pinned natively by `replacement.X05 B1 obligation quota accepts 10000 and refuses 10001`; the binary pin keeps a large
 # count that fits the native budget and must publish exactly one revision.
 w=World(temp/'large');w.module(obligations(4000));add_jobs(w,[('combined','combined.lua')],custom={'combined'});w.init()
 check('obligation 4000 loads',lambda:accepted(w,'large',(1,1)))
 assert not failures,failures
def adv_registry_colocation_binary(temp):
 # A registry colocated in the trusted configuration directory but distinct from every selected file is allowed.
 w=World(temp);w.module('-- profile v1\n');registry=w.trusted/'registry.json';shutil.copyfile(w.reg(),registry)
 baseline=w.run(render(w,profile=False));assert baseline.returncode==0 and not baseline.stderr,baseline
 got=w.run(render(w,reg=registry));assert got.returncode==0 and got.stdout==baseline.stdout and not got.stderr,got
 ledger=w.init();got=w.run(load(w,'colocated',reg=registry));assert got.returncode==0 and not got.stderr,got
 assert rows(ledger)==(1,1),rows(ledger)
 # A registry that is the same inode as a selected module refuses as an alias.
 # Leaf replacement between registration and first read is native-only and mapped by name, not observed here.
 os.unlink(registry);os.link(w.trusted/'modules/p.lua',registry)
 expect(w.run(render(w,reg=registry)),3,UNSAFE)
def adv_limits_cli(temp):
 # An unbounded registration loop traps at the existing registration instruction budget. The message is the existing
 # propagated native message (response.cpp:211, X-LIMIT: existing propagated native errors keep their exact messages).
 w=World(temp/'spin');w.module('while true do end\n')
 expect(w.run(render(w)),3,closed(code='resource_limit',details={'limit':'instructions','maximum':10000000},field='',
                                  message='Lua instruction limit exceeded'))
 # A module one byte over the 1 MiB capture bound refuses with the closed resource_limit row (envelope.cpp:46, response.cpp:208).
 oversize=World(temp/'oversize');oversize.module(b'-- '+b'x'*(1048577-4)+b'\n')
 expect(oversize.run(render(oversize)),3,closed(code='resource_limit',field='',message='External profile resource limit exceeded'))
 # X08 C01 survivor I16b: the per-module byte bound in profile.cpp must refuse before the module is ever registered. A module
 # one byte over the bound that would otherwise trap the registration loop must still refuse with the capture resource_limit row,
 # never with the later Lua instruction limit (a raised bound would let the module run and report 'instructions').
 head=b'while true do end\n-- '
 spin=World(temp/'oversize-spin');spin.module(head+b'x'*(1048577-len(head)-1)+b'\n')
 assert (spin.trusted/'modules/p.lua').stat().st_size==1048577
 expect(spin.run(render(spin)),3,closed(code='resource_limit',field='',message='External profile resource limit exceeded'))
 # Control at exactly the bound: the same module shape at 1048576 bytes is accepted.
 exact=World(temp/'exact');exact.module(b'-- '+b'x'*(1048576-4)+b'\n')
 got=exact.run(with_profile(['--help']));assert got.returncode==0 and got.stdout and not got.stderr,got
CASES={'adv_hardlink_module_refuses_before_chunk':adv_hardlink_module_refuses_before_chunk,
       'adv_missing_declaration_initial_admission':adv_missing_declaration_initial_admission,
       'adv_opaque_ordering_binary':adv_opaque_ordering_binary,
       'adv_invalid_unselected_job_refuses_all':adv_invalid_unselected_job_refuses_all,
       'adv_job_mix_and_phase_failures':adv_job_mix_and_phase_failures,
       'adv_registry_colocation_binary':adv_registry_colocation_binary,
       'adv_limits_cli':adv_limits_cli}
if case not in CASES:raise SystemExit('unknown case '+case)
with tempfile.TemporaryDirectory(prefix='planar-x08-') as temp:
 CASES[case](Path(temp).resolve())
 for world in worlds:assert not (world.root/'forbidden-legacy.db').exists()
print(case,'passed')
