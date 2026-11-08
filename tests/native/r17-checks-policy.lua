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
