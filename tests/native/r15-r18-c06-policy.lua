-- Actual shipped policy over current and historical owning-lineage controls.
local total,failures=0,0
local function check(name,fn)
 total=total+1;local ok,err=pcall(fn)
 print((ok and 'PASS ' or 'FAIL ')..name..(ok and '' or ': '..tostring(err)))
 if not ok then failures=failures+1 end
end
check('C06 observed fallback ignores retired owner completion',function()
 local s=base();s.observed={}
 s.roles[#s.roles+1]={job='j',role='old_alpha',state='retired',generation={id='9',lineage={'9'},structure={kind='worker',work={'a'}}}}
 s.completions[#s.completions+1]={id='9',job='j',role='old_alpha',generation='9',seq='99',work_fp=H('9'),units={{work='a',output=H('9')}},vectors={}}
 assert(checks.applicable_completion(s,'alpha'),'obsolete later output replaced active owner observation')
 assert(not checks.current_completion(s,'old_alpha'),'retirement did not end current completion lineage')
end)
check('C06 takeover retains completion from carried generation',function()
 local s=base();s.roles[1].generation.id='8';s.roles[1].generation.lineage={'1','8'}
 assert(checks.current_completion(s,'alpha').id=='1')
 assert(checks.applicable_completion(s,'alpha'))
end)
check('C06 blocker adapter uses post-operation structural lineage',function()
 local s=base();s.roles[3].state='done';s.roles[3].run='none'
 s.review.verdicts={verdict('1','30','reject',{decision='reject',repair={}})}
 s.review.blockers={{id='1',job='j',kind='unowned_rejection',verdict='1'}}
 local view=base();view.roles[1].generation.id='8';view.roles[1].generation.lineage={}
 view.completions={view.completions[2]};view.observed={view.observed[2]}
 view.revision={interpretation={}}
 assert(#review.blockers_to_clear(s,{operation='load',job='j',snapshot=s})==0,'unchanged control incorrectly clears')
 local cleared=dispatch_internal.clear_blockers(s,{operation='load',candidate={}},'j',view)
 assert(#cleared==1 and cleared[1]=='1','post-reset vector must invalidate old unowned rejection')
end)
print('SUMMARY tests='..total..' failures='..failures..' skips=0');assert(failures==0,'C06 policy regression')
