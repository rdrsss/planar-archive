-- Standalone draft live orchestrator projection check; no ledger or native process is opened.
local root=arg[1] or "."
engine={object=function(x)return x end,array=function(x)return x end,fragment_digest=function()return "xxh3_128:"..string.rep("0",32)end}
byte_order=function(a,b)return a<b end
sorted=function(values,key)table.sort(values,function(a,b)return key(a)<key(b)end);return values end
presentation_data=function(_,text)return text end
local file=assert(io.open(root.."/src/stdlib/planar/orchestrator.lua"));local source=file:read("*a");file:close()
local command=assert(load(source.."\nreturn orchestrator_command_impl","orchestrator"))()
local bundle={workspace={id="workspace"},spec={id="spec",prescriptions={}},registry={id="tools",revision="v1"},jobs={{value={id="job",shape="fanout_review",max_rounds=2,roles={{id="worker",kind="worker",work={"unit"}}}}}}}
local snapshot={
 active_job="job",active_revision="1",activation="1",
 revision={interpretation={bundle=bundle,semantic={jobs={{graph={vertices={},edges={}}}}}}},
 roles={{job="job",role="worker",state="running",drifted=false,
  generation={ordinal="1",structure={kind="worker"}},
  run={id="1",request_id="claim",secret="DO_NOT_PRINT",token="DO_NOT_PRINT"}}},
 review={blockers={}}
}
local out=command({present={ledger=true},options={ledger="opaque"}},{read_execution=function()return {ok=true,value={ledger="identity",last_seq="2",commands={"check","claim","orchestrator","status"},snapshot=snapshot}}end})
assert(out.ok)
local text=out.value.prompt.markdown
assert(text:find("## Orchestrator state",1,true))
assert(text:find("worker: running",1,true))
assert(text:find("check (available)",1,true))
assert(text:find("escaping descendants are not tracked",1,true))
assert(not text:find("DO_NOT_PRINT",1,true))
print("Live orchestrator projection passed: recorded state, available check and no token disclosure")
snapshot.reservations={{run="1",id="7"}}
snapshot.review={count="2",blockers={{id="8",kind="max_rounds",verdict="6"}}}
local held=command({present={ledger=true},options={ledger="opaque"}},{read_execution=function()return {ok=true,value={ledger="identity",last_seq="9",commands={"check","check-recover","claim","orchestrator","status"},snapshot=snapshot}}end})
assert(held.ok)
local held_text=held.value.prompt.markdown
assert(held_text:find("check-recover (only after confirming the check process stopped)",1,true))
assert(held_text:find("blocker 8: max_rounds (verdict 6); clears when a later revision raises max_rounds above 2",1,true))
assert(held_text:find("run 1: role worker, generation 1, request claim",1,true))
print("Live orchestrator reservation, run identity and blocker guidance passed")
