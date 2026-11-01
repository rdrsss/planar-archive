-- Exact declaration v1 schema. B2 owns cross-entity policy and selection.
local function path_valid(path, boundary)
  if type(path) ~= "string" or #path == 0 or #path > 1024 then return false end
  local n=#path
  if string.byte(path,n)==47 then
    if not boundary then return false end
    n=n-1
  end
  local start=1
  for i=1,n+1 do
    local b=i<=n and string.byte(path,i) or 47
    if b==47 then
      local segment=string.sub(path,start,i-1)
      if #segment==0 or #segment>128 or segment=="." or segment==".." or string.lower(segment)==".git" then return false end
      start=i+1
    elseif not ((b>=65 and b<=90) or (b>=97 and b<=122) or (b>=48 and b<=57) or b==95 or b==46 or b==45) then return false end
  end
  return true
end
local function relative(file,path)
  local parent=""
  for i=1,#file do if string.byte(file,i)==47 then parent=string.sub(file,1,i) end end
  return parent..path
end
local function id_valid(s)
  if type(s)~="string" or #s<1 or #s>64 then return false end
  for i=1,#s do
    local b=string.byte(s,i)
    if not ((b>=97 and b<=122) or (i>1 and ((b>=48 and b<=57) or b==95))) then return false end
  end
  return true
end
local function primitive(name,extra) return {type=name,extra=extra} end
local ID, STR, TEXT, NOTE = primitive("id"),primitive("string"),primitive("text"),primitive("note")
local PATH, BOUNDARY = primitive("path"),primitive("boundary")
local function enum(...) return {type="enum",values={...}} end
local function list(item,minimum,set,key,sort_records) return {type="list",item=item,minimum=minimum or 0,set=set,key=key,sort_records=sort_records} end
local function keyed(item,minimum) return list(item,minimum,false,"id",true) end
local function set(item,minimum) return list(item,minimum,true) end
local function record(fields,kind) return {type="record",fields=fields,kind=kind or "object"} end
local function req(schema) return {schema=schema,required=true} end
local function opt(schema,default) return {schema=schema,default=default} end
local function lists(schema) return opt(schema,"empty") end
local function number(maximum,minimum) return primitive("number",{maximum=maximum,minimum=minimum or 0}) end
local UINT=number(2147483647)
local BUDGET=number(1048576,1)
local scope=primitive("scope")
local requirement=record({id=req(ID),scope=req(scope),text=req(TEXT)})
local acceptance=record({id=req(ID),text=req(TEXT)})
local work=record({id=req(ID),summary=req(TEXT),paths=req(set(BOUNDARY,1)),checks=req(set(ID,1))})
local milestone=record({id=req(ID),work=req(set(ID,1)),acceptance=req(list(acceptance,1,false,"id"))})
local contract=record({id=req(ID),producer=req(ID),consumers=req(set(ID,1)),expected=req(TEXT),conformance=req(ID),review_note=req(TEXT)})
local facet=record({id=req(ID),producer=req(ID),paths=req(set(PATH,1))})
local dependency=record({id=req(ID),consumer=req(ID),source=req(enum("contract","facet")),ref=req(ID),phase=req(enum("start","complete")),quality=req(enum("available","done","accepted"))})
local question=record({id=req(ID),scope=req(scope),prompt=req(TEXT),blocks=opt(enum("none","start","complete"),"none"),answer=opt(TEXT)})
local role=record({id=req(ID),kind=req(enum("worker","reviewer")),work=opt(set(ID,1)),targets=opt(set(ID,1)),checks=lists(set(ID)),budget_bytes=opt(BUDGET),preferences=opt(record({model=opt(primitive("string",256)),session=opt(primitive("string",256))}))})
local workspace_schema=record({version=req(primitive("version")),id=req(ID),root=req(record({id=req(ID),base=req(ID),excluded=lists(set(BOUNDARY))})),spec=req(PATH),jobs=req(keyed(record({id=req(ID),file=req(PATH)}),1)),registry=req(record({id=req(ID),revision=req(ID)})),toolchains=req(set(ID,1)),checks=req(set(ID,1)),notes=opt(NOTE)},"workspace")
local spec_schema=record({version=req(primitive("version")),id=req(ID),goal=req(TEXT),work=req(keyed(work,1)),milestones=req(keyed(milestone,1)),constraints=lists(keyed(requirement,0)),prescriptions=lists(keyed(requirement,0)),contracts=lists(keyed(contract,0)),facets=lists(keyed(facet,0)),dependencies=lists(keyed(dependency,0)),questions=lists(keyed(question,0)),frozen=lists(set(BOUNDARY)),rationale=opt(NOTE)},"spec")
local job_schema=record({version=req(primitive("version")),id=req(ID),shape=req(enum("pipeline","fanout_review")),roles=req(keyed(role,1)),stages=opt(list(set(ID,1))),max_rounds=opt(UINT,2),budget_bytes=opt(BUDGET,32768),notes=opt(NOTE)},"job")
local environment=record({name=req(primitive("environment")),value=req(STR)})
local toolchain=record({id=req(ID),description=req(STR),command=req(list(STR,1)),environment=req(list(environment,0,false,"name"))})
local check=record({id=req(ID),toolchain=req(ID),argv=req(list(STR)),coverage=req(primitive("nonempty_string")),inputs=req(set(enum("expected_contracts","own_output","output_vector"),1)),timeout_ms=req(number(3600000,1)),max_output_bytes=req(number(16777216,1)),selection=req(enum("nonempty","not_applicable"))})
local registry_schema=record({version=req(primitive("version")),id=req(ID),revision=req(ID),toolchains=req(keyed(toolchain,0)),checks=req(keyed(check,0))})
local function schema_context(files,route)
  local errors,origins={},{}
  local function issue(file,field,reason,code,details)
    errors[#errors+1]={code=code or "schema",message=reason,file=file,field=field,details=details or {}}
  end
  local normalize
  normalize=function(value,schema,file,field)
    local t,k=schema.type,engine.kind(value)
    local function bad(reason,code,details) issue(file,field,reason,code,details); return nil end
    if t=="record" then
      if k~=schema.kind then return bad("Expected "..schema.kind) end
      local out=engine.object({})
      local keys={}
      for key in pairs(value) do keys[#keys+1]=key end
      keys=sorted(keys,function(x)return x end)
      for _,key in ipairs(keys) do if not schema.fields[key] then issue(file,field.."."..key,"Unknown field") end end
      keys={}
      for key in pairs(schema.fields) do keys[#keys+1]=key end
      keys=sorted(keys,function(x)return x end)
      for _,key in ipairs(keys) do
        local f=schema.fields[key]
        if value[key]==nil then
          if f.required then issue(file,field.."."..key,"Required field")
          elseif f.default=="empty" then out[key]=engine.array({})
          elseif f.default~=nil then out[key]=f.default end
        else out[key]=normalize(value[key],f.schema,file,field.."."..key) end
      end
      return out
    elseif t=="list" then
      if k~="array" then return bad("Expected list") end
      if #value<schema.minimum then issue(file,field,"List is too short") end
      local out,seen={},{}
      for i,item in ipairs(value) do
        local v=normalize(item,schema.item,file,field.."["..i.."]")
        if v~=nil then
          out[#out+1]=v
          if type(v)=="table" then origins[v]=i end
          local key=schema.key and v[schema.key] or (schema.set and v or nil)
          if key~=nil then
            if seen[key] then issue(file,field.."["..i.."]","Duplicate value","reference") end
            seen[key]=true
          end
        end
      end
      if schema.set then return sorted(out,function(x)return x end) end
      if schema.sort_records then return sorted(out,function(x)return x[schema.key] or "" end) end
      return engine.array(out)
    elseif t=="scope" then
      if value=="global" then return value end
      return normalize(value,set(ID,1),file,field)
    elseif t=="text" or t=="note" then
      if k=="immaterial" then
        if t~="note" then return bad("Immaterial wrapper forbidden") end
        value=value.value; k=engine.kind(value)
      end
      if k~="md" and k~="file" then return bad("Expected text constructor") end
      local bytes=value.value
      if k=="file" then
        if not path_valid(bytes,false) then return bad("Invalid text path","path_denied") end
        local path=relative(file,bytes)
        if not path_valid(path,false) then return bad("Invalid resolved text path","path_denied") end
        if route then return engine.object({kind="file",path=path}) end
        bytes=files[path]
        if bytes==nil then return bad("Required text file not captured","io",{operation="read"}) end
        if type(bytes)~="string" or #bytes>1048576 or not utf8_text(bytes) then return bad("Invalid captured text") end
        return engine.object({kind="text",bytes=bytes,file=engine.object({root="workspace",path=path})})
      end
      if type(bytes)~="string" or #bytes>65536 or not utf8_text(bytes) then return bad("Invalid Markdown text") end
      return engine.object({kind="text",bytes=bytes})
    elseif t=="id" then
      if not id_valid(value) then return bad("Invalid ID") end
    elseif t=="path" or t=="boundary" then
      if not path_valid(value,t=="boundary") then return bad("Invalid portable path","path_denied") end
    elseif t=="string" or t=="nonempty_string" then
      if type(value)~="string" or #value>(schema.extra or 65536) or not utf8_text(value) or (t=="nonempty_string" and #value==0) then return bad("Invalid string") end
    elseif t=="enum" then
      local found=false
      for _,v in ipairs(schema.values) do if value==v then found=true end end
      if not found then return bad("Invalid enum value") end
    elseif t=="number" then
      if k~="integer" or value<schema.extra.minimum or value>schema.extra.maximum then return bad("Integer out of range") end
    elseif t=="version" then
      if k~="integer" then return bad("Expected version integer") end
      if value~=1 then return bad("Unsupported schema version","unsupported_version",{supported=engine.array({1}),received=value}) end
    elseif t=="environment" then
      if type(value)~="string" or #value==0 or #value>65536 then return bad("Invalid environment name") end
      for i=1,#value do
        local b=string.byte(value,i)
        if not ((b>=65 and b<=90) or (i>1 and ((b>=48 and b<=57) or b==95))) then return bad("Invalid environment name") end
      end
    end
    return value
  end
  local function finish(value)
    if #errors>0 then
      local ordered=sorted(errors,function(e)return e.file.."\0"..e.field.."\0"..e.code end)
      return engine.object({error=ordered[1]})
    end
    return engine.object({value=value})
  end
  return {normalize=normalize,issue=issue,finish=finish,errors=errors,index=function(value,fallback)return origins[value] or fallback end}
end
local function logical_file(file)
  return path_valid(file,false) and file or ""
end
local function workspace_rules(ctx,w,file)
  if not w then return end
  if w.root and (w.root.id=="workspace" or w.root.id=="registry" or w.root.id=="observations") then ctx.issue(file,"workspace.root.id","Reserved root ID") end
  local seen={}
  for i,j in ipairs(w.jobs or {}) do
    if j.file then
      local index=ctx.index(j,i)
      if seen[j.file] then ctx.issue(file,"workspace.jobs["..math.max(index,seen[j.file]).."].file","Duplicate job file","reference") end
      seen[j.file]=math.min(index,seen[j.file] or index)
    end
  end
end
-- Only a native-owned caller may provide this context; no declaration/envelope
-- field selects a dialect. Shipped schemas remain the literal v1 schemas above.
local function profile_shape(context,id)
  if not context then return nil end
  if context.dialect~="declarations_shapes_v1" or type(context.shapes)~="table" then
    error("Invalid trusted shape context",0)
  end
  return context.shapes[id]
end
local function job_rules(ctx,j,file,context)
  if not j then return end
  local descriptor=context and profile_shape(context,j.shape)
  if context and j.shape~="pipeline" and j.shape~="fanout_review" then
    if not descriptor then ctx.issue(file,"job.shape","Unknown shape ID","reference")
    elseif descriptor.stages=="required" and not j.stages then ctx.issue(file,"job.stages","Shape requires stages")
    elseif descriptor.stages=="forbidden" and j.stages then ctx.issue(file,"job.stages","Stages forbidden for shape") end
  end
  if j.shape=="pipeline" and not j.stages then ctx.issue(file,"job.stages","Pipeline requires stages") end
  if j.shape=="fanout_review" and j.stages then ctx.issue(file,"job.stages","Stages forbidden for fanout_review") end
  for i,r in ipairs(j.roles or {}) do
    local prefix="job.roles["..ctx.index(r,i).."]"
    if r.kind=="worker" then
      if not r.work then ctx.issue(file,prefix..".work","Worker requires work") end
      if r.targets then ctx.issue(file,prefix..".targets","Worker targets forbidden") end
    elseif r.kind=="reviewer" then
      if not r.targets then ctx.issue(file,prefix..".targets","Reviewer requires targets") end
      if r.work then ctx.issue(file,prefix..".work","Reviewer work forbidden") end
    end
    if not r.budget_bytes then r.budget_bytes=j.budget_bytes end
  end
end
local function route_workspace(input)
  local ctx=schema_context({},true)
  local request=ctx.normalize(input,record({file=req(PATH),workspace=req(primitive("any"))}),"","route")
  if #ctx.errors>0 then return ctx.finish({}) end
  input=request
  local file=logical_file(input.file)
  if file=="" then ctx.issue("","workspace.file","Invalid logical declaration path","path_denied") end
  local w=ctx.normalize(input.workspace,workspace_schema,file,"workspace")
  workspace_rules(ctx,w,file)
  local out={workspace=w,file=file,jobs=engine.array({})}
  if w then
    if w.spec then out.spec=relative(file,w.spec); if not path_valid(out.spec,false) then ctx.issue(file,"workspace.spec","Invalid resolved path","path_denied") end end
    for i,j in ipairs(w.jobs or {}) do
      if j.file and j.id then
        local path=relative(file,j.file)
        if not path_valid(path,false) then ctx.issue(file,"workspace.jobs["..ctx.index(j,i).."].file","Invalid resolved path","path_denied") end
        out.jobs[#out.jobs+1]=engine.object({id=j.id,file=path})
      end
    end
    out.root=w.root; out.registry=w.registry
  end
  return ctx.finish(engine.object(out))
end
local function normalize_bundle(input,context,mode)
  local ctx=schema_context({},false)
  -- Envelope is driver data, but reject malformed provenance rather than echoing it.
  local envelope=record({workspace=req(record({file=req(PATH),value=req(primitive("any"))})),spec=(mode=="workspace" and opt(primitive("any")) or req(record({file=req(PATH),value=req(primitive("any"))}))),jobs=req(list(record({file=req(PATH),value=req(primitive("any"))}))),registry=req(record({file=req(PATH),bytes=req(primitive("raw"))})),files=req(list(record({path=req(PATH),bytes=req(primitive("raw"))})))})
  local safe=ctx.normalize(input,envelope,"","bundle")
  if #ctx.errors>0 then return ctx.finish({}) end
  local files={}
  for i,f in ipairs(safe.files) do
    if type(f.bytes)~="string" then ctx.issue("","bundle.files["..i.."].bytes","Expected retained bytes")
    elseif files[f.path] then ctx.issue(f.path,"bundle.files","Duplicate retained file","reference")
    else files[f.path]=f.bytes end
  end
  local file_errors=ctx.errors
  ctx=schema_context(files,false)
  for _,e in ipairs(file_errors) do ctx.errors[#ctx.errors+1]=e end
  -- Repeat envelope checks above separately from schema fields; all declaration
  -- diagnostics use only validated logical paths.
  local w=ctx.normalize(safe.workspace.value,workspace_schema,safe.workspace.file,"workspace")
  workspace_rules(ctx,w,safe.workspace.file)
  local s=mode~="workspace" and ctx.normalize(safe.spec.value,spec_schema,safe.spec.file,"spec") or nil
  for _,f in ipairs(s and s.facets or {}) do f.kind="files"; f.normalizer="exact_bytes_v1" end
  if mode~="workspace" and s and s.goal and s.goal.bytes=="" then ctx.issue(safe.spec.file,"spec.goal","Goal must be nonempty") end
  local selected_job_schema=job_schema
  if context then
    profile_shape(context,"")
    local fields={}
    for key,value in pairs(job_schema.fields) do fields[key]=value end
    fields.shape=req(ID)
    selected_job_schema=record(fields,"job")
  end
  local jobs=engine.array({})
  local supplied,seen_jobs={},{}
  for _,entry in ipairs(mode~="workspace" and safe.jobs or {}) do
    if seen_jobs[entry.file] then ctx.issue(entry.file,"job","Duplicate job declaration","reference") end
    local j=ctx.normalize(entry.value,selected_job_schema,entry.file,"job")
    job_rules(ctx,j,entry.file,context)
    seen_jobs[entry.file]=true
    supplied[entry.file]=j
  end
  if w then
    -- Resolve and validate the entire route before matching declarations. Missing
    -- captures are dependent diagnostics and must not use invalid joined paths.
    local spec_path=w.spec and relative(safe.workspace.file,w.spec)
    if spec_path and not path_valid(spec_path,false) then ctx.issue(safe.workspace.file,"workspace.spec","Invalid resolved path","path_denied") end
    local paths={}
    for i,entry in ipairs(w.jobs or {}) do
      if entry.file then
        local path=relative(safe.workspace.file,entry.file)
        if not path_valid(path,false) then ctx.issue(safe.workspace.file,"workspace.jobs["..ctx.index(entry,i).."].file","Invalid resolved path","path_denied")
        else paths[entry]=path end
      end
    end
    local valid=true
    for _,e in ipairs(ctx.errors) do if e.code=="path_denied" then valid=false end end
    if valid then
      if mode~="workspace" and spec_path and safe.spec.file~=spec_path then ctx.issue(safe.spec.file,"spec","Spec path does not match workspace","reference") end
      local wanted={}
      local match_jobs=mode~="workspace" and mode~="spec_without_jobs"
      for _,entry in ipairs(match_jobs and w.jobs or {}) do
        local path=paths[entry]
        if path then
          local j=supplied[path]
          wanted[path]=true
          if not seen_jobs[path] then
            if mode~="selected_job" then ctx.issue(path,"job","Listed job not captured","reference") end
          elseif j and j.id and j.id~=entry.id then ctx.issue(path,"job.id","Returned job ID does not match workspace","reference")
          elseif j then jobs[#jobs+1]=engine.object({file=path,value=j}) end
        end
      end
      for path in pairs(match_jobs and seen_jobs or {}) do if not wanted[path] then ctx.issue(path,"job","Unlisted job declaration","reference") end end
    end
  end
  local ok,r=pcall(json_parse,safe.registry.bytes,safe.registry.file)
  if not ok then
    if type(r)=="table" then
      if r.code=="resource_limit" then return engine.object({error=r}) end
      ctx.errors[#ctx.errors+1]=r
    else error(r,0) end
  else r=ctx.normalize(r,registry_schema,safe.registry.file,"registry") end
  if ok and r and r.id and r.revision and w and w.registry and (r.id~=w.registry.id or r.revision~=w.registry.revision) then ctx.issue(safe.registry.file,"registry","Registry header does not match workspace","reference") end
  -- Nonempty executable is an intrinsic descriptor schema rule.
  if ok and r then
    for i,t in ipairs(r.toolchains or {}) do if t.command and t.command[1]=="" then ctx.issue(safe.registry.file,"registry.toolchains["..ctx.index(t,i).."].command[1]","Executable must be nonempty") end end
  end
  return ctx.finish(engine.object({workspace=w,spec=s,jobs=jobs,registry=ok and r or nil,workspace_file=safe.workspace.file,spec_file=mode~="workspace" and safe.spec.file or nil,registry_file=safe.registry.file}))
end
