-- Parse retained observations and validate every assertion before role filtering.
local observation_target=record({work=req(ID),output=req(STR)})
local observation_check=record({id=req(ID),check=req(ID),target=req(observation_target),reported=req(enum("pass","fail","inconclusive")),summary=req(primitive("string",4096)),vector=req(list(observation_target,0,false,"work",true))})
local observation_finding=record({id=req(ID),target=req(observation_target),severity=req(enum("note","required")),text=req(primitive("string",4096))})
local observation_checkpoint=record({progress=req(primitive("string",2048)),next=req(primitive("string",2048)),unresolved=req(primitive("string",2048)),files=req(set(PATH))})
local observation_schema=record({version=req(primitive("version")),checks=lists(keyed(observation_check)),findings=lists(keyed(observation_finding)),checkpoint=opt(observation_checkpoint)})
local function render_observations(request,selected,model)
  local file="input.json"
  local bytes='{"version":1,"checks":[],"findings":[]}'
  for _,entry in ipairs(request.files) do if entry.root=="observations" then bytes=entry.bytes end end
  local ok,parsed=pcall(json_parse,bytes,file)
  if not ok then if type(parsed)=="table" then return {error=parsed} end; error(parsed,0) end
  local ctx=schema_context({},false)
  local value=ctx.normalize(parsed,observation_schema,file,"observations")
  if #ctx.errors>0 then return ctx.finish({}) end
  local function issue(field,message,code,details) ctx.issue(file,"observations."..field,message,code,details) end
  local outputs,checks,effective,dependencies={},{},{},model.dependencies
  for _,output in ipairs(selected.outputs) do outputs[output.work]=output end
  for _,check in ipairs(model.bundle.registry.checks) do checks[check.id]=check end
  -- Model job records are position aligned with normalized declaration jobs.
  for i,job in ipairs(model.bundle.jobs) do if job.value.id==request.job then effective=model.jobs[i].effective_checks end end
  local policy={}
  for _,entry in ipairs(effective) do policy[entry.work]=semantic_set(entry.checks) end
  local function target_valid(target,field)
    local digest=target.output
    local valid=#digest==41 and string.sub(digest,1,9)=="xxh3_128:"
    for i=10,#digest do local b=string.byte(digest,i); if not ((b>=48 and b<=57) or (b>=97 and b<=102)) then valid=false end end
    if not valid then issue(field..".output","Malformed output digest","schema"); return false end
    local output=outputs[target.work]
    if not output or output.status~="captured" or not output.present or output.output~=digest then
      issue(field,"Assertion does not match a present captured output","observation_target_mismatch",{work=target.work}); return false
    end
    return true
  end
  for i,assertion in ipairs(value.checks) do
    local field="checks["..ctx.index(assertion,i).."]"
    target_valid(assertion.target,field..".target")
    local check=checks[assertion.check]
    if not check or (policy[assertion.target.work] and not policy[assertion.target.work][assertion.check]) then issue(field..".check","Check is not required for target","reference") end
    local expected={[assertion.target.work]=true}
    if check and semantic_set(check.inputs).output_vector then
      for _,dep in ipairs(dependencies) do if dep.consumer==assertion.target.work and dep.phase=="complete" then expected[dep.producer]=true end end
    end
    local count=0
    for _ in pairs(expected) do count=count+1 end
    local mismatch=#assertion.vector~=count
    for j,target in ipairs(assertion.vector) do
      target_valid(target,field..".vector["..ctx.index(target,j).."]")
      if not expected[target.work] then mismatch=true end
    end
    if mismatch then issue(field..".vector","Wrong output vector","observation_target_mismatch",{work=assertion.target.work}) end
  end
  for i,assertion in ipairs(value.findings) do target_valid(assertion.target,"findings["..ctx.index(assertion,i).."].target") end
  local own=semantic_set(selected.context.work)
  if value.checkpoint then
    local note=value.checkpoint
    if #note.files>32 then issue("checkpoint.files","Too many checkpoint paths","resource_limit",{limit="checkpoint_files",maximum=32}) end
    if #presentation_json(note)>8192 then issue("checkpoint","Checkpoint canonical JSON too large","resource_limit",{limit="checkpoint_bytes",maximum=8192}) end
    -- Schema uniqueness has already succeeded, so primitive path origins are
    -- unambiguous even though normalization sorts their successful output.
    local indexes={}
    for i,path in ipairs(parsed.checkpoint.files) do indexes[path]=i end
    local spelling={}
    local function aliases(path,field)
      local stem=semantic_stem(path)
      local conflict=false
      for i=1,#stem+1 do
        if i>#stem or string.byte(stem,i)==47 then
          local prefix=string.sub(stem,1,i-1)
          local key=string.lower(prefix)
          local previous=spelling[key]
          if previous and previous.prefix~=prefix then
            if field then issue(field,"Portable path segment alias","ownership",{paths=sorted({path,previous.path},function(x)return x end)}) end
            conflict=true
          elseif not previous then spelling[key]={prefix=prefix,path=path} end
        end
      end
      return conflict
    end
    -- Reuse the source namespace already validated by selection, including
    -- captured directories with no selected file and unselected declarations.
    for _,unit in ipairs(model.bundle.spec.work) do for _,path in ipairs(unit.paths) do aliases(path) end end
    for _,path in ipairs(model.bundle.spec.frozen) do aliases(path) end
    for _,path in ipairs(model.bundle.workspace.root.excluded) do aliases(path) end
    for _,facet in ipairs(model.bundle.spec.facets) do for _,path in ipairs(facet.paths) do aliases(path) end end
    if model.capture.status=="captured" then
      for _,kind in ipairs({"files","directories"}) do for _,entry in ipairs(model.capture[kind]) do aliases(entry.path) end end
    end
    for _,path in ipairs(note.files) do
      local field="checkpoint.files["..indexes[path].."]"
      local alias=aliases(path,field)
      local allowed=false
      for _,unit in ipairs(selected.work.value.units) do for _,boundary in ipairs(unit.paths) do if semantic_contains(boundary,path) then allowed=true end end end
      if not allowed and not alias then issue(field,"Checkpoint path outside role boundaries","ownership",{paths=engine.array({path})}) end
    end
  end
  if #ctx.errors>0 then return ctx.finish({}) end
  local visible={}
  for id in pairs(own) do visible[id]=true end
  if selected.context.kind=="worker" then for _,dep in ipairs(selected.work.value.dependencies) do visible[dep.producer]=true end end
  local result=engine.object({version=1,checks=engine.array({}),findings=engine.array({})})
  for _,kind in ipairs({"checks","findings"}) do
    for _,assertion in ipairs(value[kind]) do if visible[assertion.target.work] then assertion.authority="driver_assertion_unverified"; result[kind][#result[kind]+1]=assertion end end
  end
  if value.checkpoint then value.checkpoint.authority="session_note_unverified"; result.checkpoint=value.checkpoint end
  return ctx.finish(result)
end
