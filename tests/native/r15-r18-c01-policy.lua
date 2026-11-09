-- C01 actual checks/review policy regression tests; run from repository root.
local function read(p)local f=assert(io.open(p));local s=f:read('*a');f:close();return s end
local checks,review=C01_CHECKS,C01_REVIEW
if not checks then checks,review=assert(load(read('src/stdlib/planar/checks.lua')..'\n'..read('src/stdlib/planar/review.lua')..'\nreturn checks,review'))()end
local function H(x)return 'xxh3_128:'..string.rep(x,32)end
local function base()
 local S={active_job='j',active_revision='1',activation='1',last_seq='20',roles={
 {job='j',role='worker',state='done',drifted=false,fact_version='1',generation={id='1',lineage={'1'},lineage_opened='1',structure={kind='worker',work={'a'}}},run='none'},
 {job='j',role='reviewer',state='done',drifted=false,fact_version='1',generation={id='2',lineage={'2'},lineage_opened='1',structure={kind='reviewer',work={},targets={'a'}}},run='none'}},
 completions={{id='1',job='j',role='worker',generation='1',seq='5',work_fp=H('a'),units={{work='a',output=H('b')}},vectors={}}},observed={{work='a',output=H('b')}},review={count='0',repairs={},blockers={},verdicts={{id='1',run='2',generation='2',seq='10',applicability='current',decision='approve',body={targets={{work='a',output=H('b'),work_fp=H('a')}},policy=H('c'),decision='approve',findings={},repair={}}}}}}
 return S
end
checks.hooks.current_work=function(S,r)return {digest=r.role=='worker' and H('a') or H('c'),work={shape={max_rounds=2},units=r.role=='worker' and {{id='a'}} or {},dependencies={}},prerequisites={},blocked=false}end

local failures,total=0,0
local function check(name,fn)
 total=total+1;local ok,err=pcall(fn);print((ok and "PASS " or "FAIL ")..name..(ok and "" or ": "..tostring(err)));if not ok then failures=failures+1 end
end
local function state(C,role)for _,r in ipairs(C.vector)do if r.role==role then return r.state end end end
local function result(output,failed)return {recapture='matched',observation={units={{work='a',output=output}}},facets={},evidence=failed and {{work='a',check='unit',outcome='fail'}} or {}}end
local plan={job='j',role='worker',run='3',generation='1',units={{work='a'}}}
local function running(S)S.roles[1].state='running';S.roles[1].run={id='3',dispatch={id='3',work_fp=H('a')}}end
check('reviewer immaterial identity remains applicable',function()local S=base();assert(checks.role_completion(S,'reviewer'));assert(state(checks.rederive(S,{facts={},transitions={}}),'reviewer')=='done')end)
check('reviewer lineage rejects recreated approval',function()local S=base();S.roles[2].generation.lineage={'3'};S.roles[2].generation.id='3';assert(not review.acceptance(S)[1].accepted)end)
check('reviewer missing generation fails closed',function()local S=base();S.review.verdicts[1].generation=nil;assert(not review.acceptance(S)[1].accepted)end)
check('changed target stales reviewer on completion',function()local S=base();running(S);local C=checks.evaluate(S,plan,result(H('d')),{snapshot_seq='20'});assert(state(C,'reviewer')=='stale')end)
check('restored target restores stale reviewer',function()local S=base();S.roles[2].state='stale';running(S);local C=checks.evaluate(S,plan,result(H('b')),{snapshot_seq='20'});assert(state(C,'reviewer')=='done')end)
for _,failed in ipairs({false,true})do
 check('post-result clears rejection '..tostring(failed),function()local S=base();running(S);S.review.verdicts[1].decision='reject';S.review.verdicts[1].body.decision='reject';S.review.blockers={{id='1',job='j',kind='unowned_rejection',verdict='1'}};local C=checks.evaluate(S,plan,result(H('d'),failed),{snapshot_seq='20'});assert(#C.facts.blockers_cleared==1);assert(S.observed[1].output==H('b'));if failed then assert(not C.facts.completion);assert(state(C,'worker')=='running');assert(state(C,'reviewer')=='blocked')end end)
end
check('unblocked obsolete worker completion is stale',function()local S=base();S.roles[1].state='blocked';S.observed[1].output=H('d');assert(state(checks.rederive(S,{facts={},transitions={}}),'worker')=='stale')end)
check('unblocked obsolete reviewer verdict is stale',function()local S=base();S.roles[2].state='blocked';S.completions[1].units[1].output=H('d');S.observed[1].output=H('d');assert(state(checks.rederive(S,{facts={},transitions={}}),'reviewer')=='stale')end)
check('unblocked worker without completion is ready',function()local S=base();S.roles[1].state='blocked';S.completions={};assert(state(checks.rederive(S,{facts={},transitions={}}),'worker')=='ready')end)
check('unblocked reviewer without verdict is ready',function()local S=base();S.roles[2].state='blocked';S.review.verdicts={};assert(state(checks.rederive(S,{facts={},transitions={}}),'reviewer')=='ready')end)
check('stale captured producer vector withholds and retains run',function()
 local S=base();local consumer={job='j',role='consumer',state='running',drifted=false,generation={id='4',lineage={'4'},structure={kind='worker',work={'z'}}},run={id='4',dispatch={id='4',work_fp=H('e')}}};S.roles[#S.roles+1]=consumer
 local saved=checks.hooks.current_work
 checks.hooks.current_work=function(s,r)if r.role~='consumer'then return saved(s,r)end;return {digest=H('e'),work={units={{id='z'}},dependencies={{id='dep',producer='a',consumer='z',source='contract',phase='complete',quality='done'}}},prerequisites={}}end
 S.completions[1].units[1].output=H('d');S.observed[1].output=H('d')
 local P={job='j',role='consumer',run='4',generation='4',units={{work='z'}}}
 local R={recapture='matched',observation={units={{work='z',output=H('f')}}},facets={},evidence={{work='z',check='integration',outcome='pass',vector={targets={{work='a',output=H('b')},{work='z',output=H('f')}}}}}}
 local C=checks.evaluate(S,P,R,{snapshot_seq='20'});checks.hooks.current_work=saved
 assert(not C.facts.completion);assert(state(C,'consumer')=='running');assert(C.withheld[1]=='input_changed')
end)
check('active worker retained completion blocks reviewer',function()
 local S=base();running(S);S.roles[1].drifted=true
 assert(checks.applicable_completion(S,'worker'))
 assert(checks.material(S,S.roles[2]).blocked)
 assert(state(checks.rederive(S,{facts={},transitions={}}),'reviewer')=='blocked')
 local C=checks.evaluate(S,plan,result(H('b'),true),{snapshot_seq='20'})
 assert(state(C,'worker')=='running');assert(state(C,'reviewer')=='blocked')
end)
check('restored inactive worker releases reviewer prerequisite',function()
 local S=base();S.roles[1].state='stale';S.roles[2].state='blocked'
 local C=checks.rederive(S,{facts={},transitions={}})
 assert(state(C,'worker')=='done');assert(state(C,'reviewer')=='done')
end)
check('done dependency quality remains completion applicability',function()
 local S=base();running(S)
 local consumer={job='j',role='consumer',state='ready',generation={structure={kind='worker',work={'z'}}},run='none'}
 S.roles[#S.roles+1]=consumer
 local saved=checks.hooks.current_work
 checks.hooks.current_work=function(snapshot,r)
  if r.role~='consumer'then return saved(snapshot,r)end
  return {digest=H('e'),work={units={{id='z'}},dependencies={{id='dep',producer='a',consumer='z',source='contract',phase='start',quality='done'}}},prerequisites={{id='dep',phase='start',status='unavailable'}}}
 end
 local material=checks.material(S,consumer);checks.hooks.current_work=saved
 assert(not material.blocked);assert(material.prerequisites[1].status=='available')
end)
print('SUMMARY tests='..total..' failures='..failures..' skips=0');assert(failures==0,"C01 policy regressions failed")
