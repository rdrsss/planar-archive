-- Scoped material context over a globally validated bundle and sealed byte inventory.
-- Native hashing is the only digest implementation; no capabilities are acquired here.
local function selection_copy(value)
  local out={}
  for key,item in pairs(value) do out[key]=item end
  return engine.object(out)
end
local function selection_filter(items,predicate)
  local out={}
  for _,item in ipairs(items) do if predicate(item) then out[#out+1]=item end end
  return engine.array(out)
end
local function selection_intersection(items,selected)
  return selection_filter(items,function(id)return selected[id] end)
end
local function selection_hex(bytes)
  local digits="0123456789abcdef"
  local chunks={}
  -- Metered byte loops; concatenate bounded chunks to avoid quadratic copies.
  for start=1,#bytes,4096 do
    local chars={}
    for i=start,math.min(start+4095,#bytes) do
      local byte=string.byte(bytes,i)
      chars[#chars+1]=string.sub(digits,(byte//16)+1,(byte//16)+1)
      chars[#chars+1]=string.sub(digits,(byte%16)+1,(byte%16)+1)
    end
    chunks[#chunks+1]=table.concat(chars)
  end
  return table.concat(chunks)
end
local function selection_material(ctx,request,model)
  if #request.files>10000 then
    ctx.issue("","selection.files","Too many retained files","resource_limit",{limit="files",maximum=10000})
    return ctx.finish({})
  end
  local b=model.bundle
  local w,s=b.workspace,b.spec
  local source=w.root.id
  local captured=model.capture.status=="captured"
  local by_file,expected,topology={},{},{}
  local count,total=0,0
  local function issue(field,message,code,details) ctx.issue("",field,message,code,details) end
  for _,f in ipairs(request.bundle.files) do expected["workspace\0"..f.path]=f.bytes end
  expected["registry\0input.json"]=request.bundle.registry.bytes
  -- Required declarations must themselves be present in the retained inventory.
  for _,path in ipairs({b.workspace_file,b.spec_file}) do
    if expected["workspace\0"..path]==nil then issue("selection.files","Declaration bytes absent from bundle inventory","reference") end
  end
  for _,entry in ipairs(b.jobs) do
    if expected["workspace\0"..entry.file]==nil then issue("selection.files","Job bytes absent from bundle inventory","reference") end
  end
  if captured then
    for _,entry in ipairs(model.capture.files) do topology[entry.root.."\0"..entry.path]=true end
  end
  for i,f in ipairs(request.files) do
    local field="selection.files["..i.."]"
    local key=f.root.."\0"..f.path
    local tree=f.root=="current:"..source or f.root=="base:"..source
    if not (tree or f.root=="workspace" or f.root=="registry" or f.root=="observations") then issue(field..".root","Unknown retained root","reference") end
    if (f.root=="registry" or f.root=="observations") and f.path~="input.json" then issue(field..".path","Reserved input root requires input.json","path_denied") end
    if by_file[key] then issue(field,"Duplicate retained file","reference") end
    by_file[key]=f
    if type(f.bytes)~="string" then issue(field..".bytes","Expected retained bytes")
    else
      count=count+1; total=total+#f.bytes
      local maximum=tree and 16777216 or 1048576
      if #f.bytes>maximum then issue(field..".bytes","Retained file exceeds byte limit","resource_limit",{limit="file_bytes",maximum=maximum}) end
    end
    if tree then
      if not captured or not topology[key] then issue(field,"Retained source file absent from captured topology","reference") end
      for _,boundary in ipairs(w.root.excluded) do if semantic_contains(boundary,f.path) then issue(field,"Excluded file in retained capture","ownership") end end
    elseif f.root~="observations" then
      if type(f.bytes)=="string" and (expected[key]==nil or expected[key]~=f.bytes) then issue(field,"Retained bytes disagree with declaration inventory","reference") end
    end
  end
  if count>10000 then issue("selection.files","Too many retained files","resource_limit",{limit="files",maximum=10000}) end
  if total>67108864 then issue("selection.files","Too many retained bytes","resource_limit",{limit="input_bytes",maximum=67108864}) end
  for key in pairs(expected) do if not by_file[key] then issue("selection.files","Missing retained declaration input","reference") end end
  for key in pairs(topology) do if not by_file[key] then issue("selection.files","Captured topology file has no retained bytes","reference") end end
  -- Global portable spelling, including additional workspace/provenance files.
  local spellings={}
  for _,f in ipairs(request.files) do
    for i=1,#f.path+1 do
      if i>#f.path or string.byte(f.path,i)==47 then
        local prefix=string.sub(f.path,1,i-1)
        local key=f.root.."\0"..string.lower(prefix)
        if spellings[key] and spellings[key]~=prefix then issue("selection.files","Portable retained path alias","ownership") end
        spellings[key]=prefix
      end
    end
  end
  if #ctx.errors>0 then return ctx.finish({}) end
  local job,job_model,role
  for i,entry in ipairs(b.jobs) do if entry.value.id==request.job then job=entry.value; job_model=model.jobs[i] end end
  if not job then issue("selection.job","Unknown selected job","reference"); return ctx.finish({}) end
  for _,r in ipairs(job.roles) do if r.id==request.role then role=r end end
  if not role then issue("selection.role","Unknown selected role","reference"); return ctx.finish({}) end
  local ids=role.kind=="worker" and role.work or role.targets
  local selected=semantic_set(ids)
  local units=selection_filter(s.work,function(u)return selected[u.id] end)
  local files,current,base_entries={}, {}, {}
  for _,f in ipairs(sorted(request.files,function(f)return f.root.."\0"..f.path end)) do
    local digest=engine.file_digest(f.bytes)
    files[#files+1]=engine.object({root=f.root,path=f.path,kind="file",mode=f.mode,size=#f.bytes,bytes_hex=selection_hex(f.bytes),digest=digest})
    if f.root=="current:"..source or f.root=="base:"..source then
      local entry=engine.object({root=source,path=f.path,kind="file",mode=f.mode,size=#f.bytes,content=digest})
      local tree=f.root=="current:"..source and current or base_entries
      tree[#tree+1]=entry
    end
  end
  files=engine.array(files); current=engine.array(current); base_entries=engine.array(base_entries)
  local base,material_base,observation
  if captured then
    local digest=engine.hash_value("base_v1",{root=source,label=w.root.base,entries=base_entries,excluded=w.root.excluded})
    base=engine.object({status="captured",digest=digest,entries=base_entries})
    material_base=engine.object({status="captured",digest=digest})
    observation=engine.object({status="captured",digest=engine.hash_value("observation_v1",{root=source,base=digest,entries=current,excluded=w.root.excluded}),entries=current})
  else
    base=engine.object({status="not_captured",root=source,label=w.root.base})
    material_base=selection_copy(base); material_base.authority="declaration_assertion_unverified"
    observation=engine.object({status="not_captured"})
  end
  local outputs,output_map={},{}
  for _,u in ipairs(s.work) do
    local output=engine.object({work=u.id,status=captured and "captured" or "not_captured"})
    if captured then
      local entries=selection_filter(current,function(entry)
        for _,path in ipairs(u.paths) do if semantic_contains(path,entry.path) then return true end end
        return false
      end)
      output.output=engine.hash_value("unit_output_v1",{work=u.id,root=source,base=base.digest,paths=u.paths,entries=entries})
      output.present=#entries>0
    end
    outputs[#outputs+1]=output; output_map[u.id]=output
  end
  local role_output,targets
  targets=engine.array({})
  if captured then
    if role.kind=="worker" then
      local vector={}
      for _,id in ipairs(ids) do vector[#vector+1]=engine.object({work=id,output=output_map[id].output}) end
      role_output=engine.hash_value("role_output_v1",{role=role.id,units=engine.array(vector)})
    else
      for _,id in ipairs(ids) do targets[#targets+1]=engine.object({work=id,output=output_map[id].output}) end
    end
  end
  local facet_values,facet_by_id={},{}
  for _,f in ipairs(s.facets) do
    local value=engine.object({id=f.id,status=captured and "unavailable" or "not_captured"})
    if captured then
      local paths=semantic_set(f.paths)
      local entries=selection_filter(current,function(entry)return paths[entry.path] end)
      if #entries==#f.paths then
        value.status="available"
        value.semantic=engine.hash_value("facet_v1",{kind=f.kind,normalizer=f.normalizer,root=source,paths=f.paths,entries=entries})
        value.entries=entries
      end
    end
    facet_values[#facet_values+1]=value; facet_by_id[f.id]=value
  end
  local contracts,contract_ids={},{}
  for _,c in ipairs(s.contracts) do
    local semantic=engine.hash_value("contract_v1",{kind="text",normalizer="exact_bytes_v1",expected=c.expected})
    contract_ids[c.id]=semantic
    local applicable=selected[c.producer]
    for _,consumer in ipairs(c.consumers) do if selected[consumer] then applicable=true end end
    if applicable then contracts[#contracts+1]=engine.object({declaration=c,semantic=semantic}) end
  end
  local deps,facets_needed,start_facets,prerequisites,mapping_needed={},{},{},{},{}
  for id in pairs(selected) do mapping_needed[id]=true end
  for _,d in ipairs(model.dependencies) do if selected[d.consumer] then
    local entry=selection_copy(d)
    mapping_needed[d.producer]=true
    local status
    if d.source=="contract" then entry.semantic=contract_ids[d.ref]; status="available"
    else
      facets_needed[d.ref]=true
      local value=facet_by_id[d.ref]
      status=d.quality=="available" and value.status or "unverified"
      if d.phase=="start" and not selected[d.producer] then
        local start=engine.object({dependency=d.id})
        if value.status=="available" then start.semantic=value.semantic
        elseif captured then start.unavailable=true else start.not_captured=true end
        start_facets[#start_facets+1]=start
      end
    end
    deps[#deps+1]=entry
    prerequisites[#prerequisites+1]=engine.object({id=d.id,phase=d.phase,status=status,source=d.source})
  end end
  local pipeline=selection_filter(job_model.pipeline_predecessors,function(p)return selected[p.consumer] end)
  for _,p in ipairs(pipeline) do
    mapping_needed[p.producer]=true
    local prerequisite=selection_copy(p); prerequisite.status="unverified"
    prerequisites[#prerequisites+1]=prerequisite
  end
  local function scoped(items,field)
    local out={}
    for _,item in ipairs(items) do
      if item[field]=="global" then out[#out+1]=item
      else
        local intersection=selection_intersection(item[field],selected)
        if #intersection>0 then local copy=selection_copy(item); copy[field]=intersection; out[#out+1]=copy end
      end
    end
    return engine.array(out)
  end
  local questions=scoped(s.questions,"scope")
  for _,q in ipairs(questions) do if q.blocks~="none" then
    prerequisites[#prerequisites+1]=engine.object({id="question:"..q.id,phase=q.blocks,status=q.answer and "answered" or "unavailable",source="question"})
  end end
  local effective=selection_filter(job_model.effective_checks,function(e)return selected[e.work] end)
  local checks_needed,tools_needed={},{}
  for _,e in ipairs(effective) do for _,id in ipairs(e.checks) do checks_needed[id]=true end end
  local checks=selection_filter(b.registry.checks,function(c)return checks_needed[c.id] end)
  for _,c in ipairs(checks) do tools_needed[c.toolchain]=true end
  local ownership={}
  for _,u in ipairs(units) do ownership[#ownership+1]=engine.object({work=u.id,paths=u.paths}) end
  local W=engine.object({versions=engine.array({"declarations_v1","selection_v1","dependencies_v1","exact_bytes_v1","canonical_v1","xxh3_128_v1"}),workspace=w.id,spec=s.id,job=job.id,role=role.id,root=source,base=material_base,goal=s.goal,units=units,milestones=scoped(s.milestones,"work"),constraints=scoped(s.constraints,"scope"),prescriptions=scoped(s.prescriptions,"scope"),contracts=engine.array(contracts),dependencies=engine.array(deps),facets=selection_filter(s.facets,function(f)return facets_needed[f.id] end),pipeline_predecessors=pipeline,questions=questions,ownership=engine.object({work=engine.array(ownership),frozen=s.frozen,excluded=w.root.excluded}),effective_checks=effective,checks=checks,toolchains=selection_filter(b.registry.toolchains,function(t)return tools_needed[t.id] end),shape=engine.object({kind=job.shape,max_rounds=job.max_rounds,mapping=selection_filter(job_model.assignments,function(a)return mapping_needed[a.work] end),reviewer=job_model.reviewer}),start_facets=engine.array(start_facets)})
  local domain="work_v1"
  if job_model.selected_shape then
    local shape=job_model.selected_shape
    W.versions[#W.versions+1]="shape_selection_v1"
    W.shape.definition=shape.definition
    W.shape.revision=shape.revision
    local obligations={}
    for _,o in ipairs(shape.obligations) do
      if selected[o.producer] or selected[o.consumer] then obligations[#obligations+1]=o end
    end
    W.shape.obligations=sorted(obligations,function(o)return o.id end)
    domain="work_external_shape_v1"
  end
  local result=engine.object({context=engine.object({workspace=w.id,spec=s.id,job=job.id,role=role.id,kind=role.kind,work=ids,budget_bytes=role.budget_bytes,registry=engine.object({id=b.registry.id,revision=b.registry.revision,authority="driver_selected_trusted_configuration"}),optional=engine.object({workspace_notes=w.notes,rationale=s.rationale,job_notes=job.notes,preferences=role.preferences})}),work=engine.object({value=W,digest=engine.hash_value(domain,W)}),base=base,observation=observation,outputs=engine.array(outputs),targets=targets,files=files,facets=engine.array(facet_values),prerequisites=sorted(prerequisites,function(p)return p.id end)})
  if role_output then result.role_output=role_output end
  return ctx.finish(result)
end

local function normalize_selection_request(ctx,input)
  return ctx.normalize(input,record({bundle=req(primitive("any")),capture=req(primitive("any")),files=req(list(record({root=req(STR),path=req(PATH),mode=req(enum("0644","0755")),bytes=req(primitive("raw"))}))),job=req(ID),role=req(ID)}),"","selection")
end

local function select_role(input)
  local ctx=schema_context({},false)
  local request=normalize_selection_request(ctx,input)
  if #ctx.errors>0 then return ctx.finish({}) end
  -- Deliberately validate every job before looking up the selected job or role.
  local validated=validate_bundle({bundle=request.bundle,capture=request.capture})
  if validated.error then return validated end
  local model=validated.value
  return selection_material(ctx,request,model)
end
-- Private retained native phase: model is the same-call finalized semantic result.
local function select_profile_material(input)
  local ctx=schema_context({},false)
  local request=normalize_selection_request(ctx,input.request)
  if #ctx.errors>0 then return ctx.finish({}) end
  return selection_material(ctx,request,input.model)
end
