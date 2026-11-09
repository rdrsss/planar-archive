-- Exercise the shipped rederivation against producer replacement and job-scoped identity.
local total,failures=0,0
local current_work=checks.hooks.current_work
local blockers=checks.hooks.blockers_to_clear
local function test(name,fn)
 total=total+1;local ok,err=pcall(fn)
 checks.hooks.current_work=current_work;checks.hooks.blockers_to_clear=blockers
 print((ok and 'PASS ' or 'FAIL ')..name..(ok and '' or ': '..tostring(err)))
 if not ok then failures=failures+1 end
end
local function setup(completing,facets)
 local s=base();s.roles[1].state='running';s.roles[1].run={id='8',dispatch={id='9',work_fp=H('1')}}
 s.roles[1].generation.structure.work={'a','x'}
 s.publications={{job='j',role='alpha',work='a',facet='fa',semantic=H('a')},{job='j',role='alpha',work='x',facet='fx',semantic=H('c')},{job='j',role='beta',work='b',facet='fb',semantic=H('b')}}
 local change={operation='check',snapshot_seq='30',facts={completion=completing or nil},transitions={{job='j',role='alpha',before_state='running',after_state=completing and 'done' or 'running',after_drifted=false}}}
 local plan={job='j',role='alpha',run='8',units={{work='a'},{work='x'}}}
 local result={observation={units={{work='a',output=H('a')},{work='x',output=H('c')}}},facets=facets or {},evidence={}}
 return s,change,plan,result
end
local function derive(s,c,p,r)
 local seen
 checks.hooks.blockers_to_clear=function(_,event)seen=event.snapshot;return {}end
 checks.rederive(s,c,p,r)
 assert(seen,'blocker policy did not receive hypothetical snapshot')
 return seen
end
test('C07 completing producer withdraws absent publications',function()
 local s,c,p,r=setup(true);local before=enc(s)
 local next=derive(s,c,p,r)
 assert(#next.publications==1 and next.publications[1].facet=='fb','withdrawn facet survived or unrelated facet lost')
 assert(enc(s)==before,'input snapshot mutated')
end)
test('C07 grouped partial publication replacement preserves unrelated producer',function()
 local s,c,p,r=setup(true,{{work='x',facet='fx',semantic=H('d')}})
 local next=derive(s,c,p,r);local found={};for _,f in ipairs(next.publications)do found[f.facet]=f.semantic end
 assert(#next.publications==2 and found.fa==nil and found.fx==H('d') and found.fb==H('b'))
end)
test('C07 evidence-only check preserves current publications',function()
 local s,c,p,r=setup(false);local next=derive(s,c,p,r)
 assert(enc(next.publications)==enc(s.publications))
end)
test('C07 completion uses active job despite retired same-name role',function()
 local s,c,p,r=setup(true)
 s.roles[#s.roles+1]={job='z',role='alpha',state='retired',generation={id='99',structure={kind='worker',work={'a'}}},run='none'}
 local before=enc(s.roles[#s.roles]);local next=derive(s,c,p,r)
 local completion=next.completions[#next.completions]
 assert(completion.generation=='1' and completion.run=='8' and completion.dispatch=='9')
 assert(enc(next.roles[#next.roles])==before)
end)
test('C07 downstream transition before state comes from active job',function()
 local s,c,p,r=setup(false)
 s.roles[2].state='blocked';s.completions={};s.roles[#s.roles+1]={job='z',role='beta',state='retired',generation={id='99',structure={kind='worker',work={'b'}}},run='none'}
 derive(s,c,p,r)
 local t;for _,v in ipairs(c.transitions)do if v.role=='beta' then t=v end end
 assert(t and t.job=='j' and t.before_state=='blocked' and t.after_state=='ready')
end)
test('C07 repair identity and increment are active-job scoped',function()
 local s=base();s.review.repairs={{job='z',role='alpha',revision='9'},{job='j',role='alpha',revision='2'}}
 assert(checks.repair_revision(s,'alpha')=='2','inactive repair selected')
 local seen
 checks.hooks.current_work=function(snapshot)seen=snapshot;return nil end
 local c={operation='verdict',snapshot_seq='30',facts={role='rev',run='10',case='current',verdict={decision='reject'},repairs={'alpha'},blockers_opened={},blockers_cleared={}},transitions={}}
 checks.rederive(s,c)
 local byjob={};for _,v in ipairs(seen.review.repairs)do byjob[v.job]=v.revision end
 assert(byjob.j=='3' and byjob.z=='9','repair increment crossed job boundary')
end)
print('SUMMARY tests='..total..' failures='..failures..' skips=0');assert(failures==0,'C07 policy regression')
