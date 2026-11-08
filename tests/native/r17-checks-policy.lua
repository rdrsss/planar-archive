-- Focused draft policy checks. Run: lua tests/native/r17-checks-policy.lua <repository-root>
local root=arg[1] or "."
local file=assert(io.open(root.."/src/stdlib/planar/checks.lua"));local source=file:read("*a");file:close()
local checks=assert(load(source.."\nreturn checks", "checks"))()
local digest="xxh3_128:"..string.rep("a",32)
local function role(id,work,state)
 return {job="job",role=id,state=state or "done",drifted=false,generation={id="1",structure={kind="worker",work={work}}},run="none"}
end
local S={active_job="job",roles={role("producer","p"),role("consumer","c")},completions={
 {id="1",job="job",role="producer",generation="1",seq="2",work_fp=digest,units={{work="p",output="p1"}},vectors={}},
 {id="2",job="job",role="consumer",generation="1",seq="3",work_fp=digest,units={{work="c",output="c1"}},vectors={{work="c",targets={{work="p",output="p1"}}}}}
},observed={{work="p",output="p1"},{work="c",output="c1"}}}
checks.hooks.current_work=function()return {digest=digest,work={dependencies={}},blocked=false}end
assert(checks.applicable_completion(S,"consumer").id=="2")
S.completions[1].units[1].output="p2"
assert(checks.applicable_completion(S,"consumer")==nil)
assert(checks.completion_mismatches(S,S.completions[2])[1].current=="p2")
local change=checks.rederive(S,{facts={},transitions={}})
assert(#change.transitions==2) -- Both producer observation and consumer vector no longer match.
assert(change.transitions[1].reason=="evidence_inapplicable")
S.completions[1].units[1].output="p1"
assert(checks.applicable_completion(S,"consumer").id=="2")
S.roles[2].generation.id="2"
assert(checks.current_completion(S,"consumer")==nil)
S.roles[2].generation.lineage={"1","2"}
assert(checks.current_completion(S,"consumer").id=="2")
print("R17 policy: applicability, vector staleness, identity restoration and lineage passed")
-- Readiness follows applicable completion and approval, not a stale done state.
local quality="done"
checks.hooks.current_work=function(_,r)
 return {digest=digest,work={units={{id=r.role=="producer" and "p" or "c"}},dependencies=r.role=="consumer" and {{id="dep",producer="p",consumer="c",source="contract",phase="start",quality=quality}} or {}},prerequisites=r.role=="consumer" and {{id="dep",phase="start",status="available"}} or {},blocked=false}
end
assert(not checks.material(S,S.roles[2]).blocked)
S.observed[1].output="changed"
assert(checks.material(S,S.roles[2]).blocked)
S.observed[1].output="p1"
quality="accepted"
checks.hooks.acceptance=function()return {{work="p",accepted=false}}end
assert(checks.material(S,S.roles[2]).blocked)
checks.hooks.acceptance=function()return {{work="p",accepted=true}}end
assert(not checks.material(S,S.roles[2]).blocked)
assert(checks.material(S,S.roles[2]).digest==digest)
print("R17 readiness: completion applicability and accepted quality passed")
-- An approval must be visible while deriving downstream transitions in the same commit.
S.roles[2].state="blocked"
S.review={count="0",verdicts={},repairs={},blockers={}}
checks.hooks.acceptance=function(snapshot)
 return {{work="p",accepted=#(snapshot.review.verdicts or {})>0}}
end
local approved=checks.rederive(S,{operation="verdict",snapshot_seq="10",facts={run="4",case="approve",verdict={decision="approve"},repairs={},blockers_opened={},blockers_cleared={}},transitions={}})
assert(#approved.transitions==1 and approved.transitions[1].role=="consumer" and approved.transitions[1].after_state=="done")
assert(#S.review.verdicts==0 and S.roles[2].state=="blocked")
print("R18 readiness: same-commit approval derivation preserves input snapshot")
