-- Product policy over B1-normalized declarations and sealed, generic topology.
-- This pass never selects a role, reads a file, executes a check or establishes quality.
local function semantic_map(items)
  local result={}
  for _,item in ipairs(items) do result[item.id]=item end
  return result
end
local function semantic_has(items,wanted)
  for _,item in ipairs(items) do if item==wanted then return true end end
  return false
end
local function semantic_set(items)
  local result={}
  for _,item in ipairs(items) do result[item]=true end
  return result
end
local function semantic_keys(map)
  local keys={}
  for key in pairs(map) do keys[#keys+1]=key end
  return sorted(keys,function(x)return x end)
end
local function semantic_stem(path)
  return string.sub(path,-1)=="/" and string.sub(path,1,-2) or path
end
local function semantic_ancestor(a,b)
  return #b>#a and string.sub(b,1,#a+1)==a.."/"
end
local function semantic_contains(boundary,path)
  if string.sub(boundary,-1)=="/" then return semantic_ancestor(semantic_stem(boundary),path) end
  return boundary==path
end
-- Deterministic iterative DFS: adjacency and roots are byte ordered; cycle is
-- the actual stack suffix closed by an existing back edge, not a set of nodes.
local function semantic_cycle(vertices,adjacency)
  local color,stack,positions={},{},{}
  for _,root in ipairs(vertices) do
    if not color[root] then
      stack[1]={vertex=root,index=1}; color[root]=1; positions[root]=1
      while #stack>0 do
        local frame=stack[#stack]
        local next_vertex=adjacency[frame.vertex][frame.index]
        if next_vertex then
          frame.index=frame.index+1
          if color[next_vertex]==1 then
            local cycle={}
            for i=positions[next_vertex],#stack do cycle[#cycle+1]=stack[i].vertex end
            cycle[#cycle+1]=next_vertex
            return engine.array(cycle)
          elseif not color[next_vertex] then
            stack[#stack+1]={vertex=next_vertex,index=1}; color[next_vertex]=1; positions[next_vertex]=#stack
          end
        else color[frame.vertex]=2; positions[frame.vertex]=nil; stack[#stack]=nil end
      end
    end
  end
end
local function prepare_shapes(input,context,mode)
  local ctx=schema_context({},false)
  local envelope=ctx.normalize(input,record({bundle=req(primitive("any")),capture=req(primitive("any"))}),"","validation")
  if #ctx.errors>0 then return ctx.finish({}) end
  local normalized=normalize_bundle(envelope.bundle,context,mode)
  if normalized.error then return normalized end
  local b=normalized.value
  local capture=envelope.capture
  local status=type(capture)=="table" and capture.status or nil
  local topology=record({root=req(STR),path=req(PATH)})
  if status=="not_captured" then
    capture=ctx.normalize(capture,record({status=req(enum("not_captured"))}),"","capture")
  elseif status=="captured" then
    capture=ctx.normalize(capture,record({status=req(enum("captured")),files=req(list(topology)),directories=req(list(topology))}),"","capture")
  else ctx.issue("","capture.status","Expected captured or not_captured") end
  if #ctx.errors>0 then return ctx.finish({}) end
  local w,s=b.workspace,b.spec
  local raw_spec
  if mode=="workspace" then
    -- No spec is normalized in this mode; empty collections make every spec rule vacuous.
    s={work={},contracts={},facets={},milestones={},constraints={},prescriptions={},questions={},frozen={},dependencies={}}
    raw_spec={}
  else raw_spec=envelope.bundle.spec.value end
  local raw_registry=json_parse(envelope.bundle.registry.bytes,b.registry_file)
  local work,contracts,facets=semantic_map(s.work),semantic_map(s.contracts),semantic_map(s.facets)
  local checks,toolchains=semantic_map(b.registry.checks),semantic_map(b.registry.toolchains)
  local allowed_checks,allowed_tools=semantic_set(w.checks),semantic_set(w.toolchains)
  -- Normalization sorts sets/records. Resolve diagnostic indexes against the
  -- raw declarations, so a refusal points to the author's original list.
  local function original_index(raw,value)
    for i,item in ipairs(raw or {}) do
      if item==value or (type(item)=="table" and item.id==value) then return i end
    end
    return 1
  end
  local function sf(collection,id,suffix)
    return "spec."..collection.."["..original_index(raw_spec[collection],id).."]"..(suffix or "")
  end
  local function problem(file,field,message,code,details)
    ctx.issue(file,field,message,code or "reference",details)
  end
  local function spec_problem(collection,id,suffix,message,code,details)
    problem(b.spec_file,sf(collection,id,suffix),message,code,details)
  end
  local function work_ref(id,collection,owner,suffix)
    if not work[id] then spec_problem(collection,owner,suffix,"Unknown work ID: "..id); return false end
    return true
  end
  local function check_ref(id,file,field)
    local c=checks[id]
    if not c or not allowed_checks[id] then problem(file,field,"Check is unknown or not allowed: "..id); return nil end
    if not toolchains[c.toolchain] or not allowed_tools[c.toolchain] then problem(file,field,"Check toolchain is unknown or not allowed: "..c.toolchain); return nil end
    return c
  end
  for _,id in ipairs(w.toolchains) do
    if not toolchains[id] then problem(b.workspace_file,"workspace.toolchains["..original_index(envelope.bundle.workspace.value.toolchains,id).."]","Unknown allowed toolchain: "..id) end
  end
  for _,id in ipairs(w.checks) do check_ref(id,b.workspace_file,"workspace.checks["..original_index(envelope.bundle.workspace.value.checks,id).."]") end
  -- Even unselected registry descriptors must resolve their declared toolchain.
  for _,c in ipairs(b.registry.checks) do
    if not toolchains[c.toolchain] then problem(b.registry_file,"registry.checks["..original_index(raw_registry.checks,c.id).."].toolchain","Unknown registry toolchain: "..c.toolchain) end
  end
  local membership={}
  for _,m in ipairs(s.milestones) do
    for _,id in ipairs(m.work) do work_ref(id,"milestones",m.id,".work"); membership[id]=true end
  end
  for _,u in ipairs(s.work) do
    if not membership[u.id] then spec_problem("work",u.id,".id","Work must belong to a milestone") end
    for _,id in ipairs(u.checks) do check_ref(id,b.spec_file,sf("work",u.id,".checks")) end
  end
  local requirement_ids={}
  for _,collection in ipairs({"constraints","prescriptions","questions"}) do
    for _,r in ipairs(s[collection]) do
      if collection~="questions" then
        if requirement_ids[r.id] then spec_problem(collection,r.id,".id","Constraint and prescription IDs share a namespace") end
        requirement_ids[r.id]=true
      end
      if r.scope~="global" then for _,id in ipairs(r.scope) do work_ref(id,collection,r.id,".scope") end end
    end
  end
  local mandatory={}
  for _,u in ipairs(s.work) do mandatory[u.id]=semantic_set(u.checks) end
  for _,c in ipairs(s.contracts) do
    work_ref(c.producer,"contracts",c.id,".producer")
    local definition=check_ref(c.conformance,b.spec_file,sf("contracts",c.id,".conformance"))
    if definition and (not semantic_has(definition.inputs,"expected_contracts") or not semantic_has(definition.inputs,"own_output")) then
      spec_problem("contracts",c.id,".conformance","Conformance requires expected_contracts and own_output inputs","schema")
    end
    if mandatory[c.producer] then mandatory[c.producer][c.conformance]=true end
    for _,id in ipairs(c.consumers) do
      work_ref(id,"contracts",c.id,".consumers")
      if id==c.producer then spec_problem("contracts",c.id,".consumers","Contract producer cannot consume itself") end
      if mandatory[id] then mandatory[id][c.conformance]=true end
    end
  end
  -- Check every boundary pair, including pairs within one unit/list/class.
  local boundaries={}
  local function add_boundary(path,file,field)
    boundaries[#boundaries+1]={path=path,file=file,field=field}
  end
  for _,u in ipairs(s.work) do for _,p in ipairs(u.paths) do add_boundary(p,b.spec_file,sf("work",u.id,".paths["..original_index(raw_spec.work[original_index(raw_spec.work,u.id)].paths,p).."]")) end end
  for _,p in ipairs(s.frozen) do add_boundary(p,b.spec_file,"spec.frozen["..original_index(raw_spec.frozen,p).."]") end
  for _,p in ipairs(w.root.excluded) do add_boundary(p,b.workspace_file,"workspace.root.excluded["..original_index(envelope.bundle.workspace.value.root.excluded,p).."]") end
  local function ownership(a,other,message)
    problem(a.file,a.field,message,"ownership",{paths=sorted({a.path,other},function(x)return x end)})
  end
  for i,a in ipairs(boundaries) do
    local stem=string.lower(semantic_stem(a.path))
    for j=i+1,#boundaries do
      local other=string.lower(semantic_stem(boundaries[j].path))
      if stem==other or semantic_ancestor(stem,other) or semantic_ancestor(other,stem) then ownership(a,boundaries[j].path,"Conflicting ownership boundaries") end
    end
  end
  local paths={}
  for _,a in ipairs(boundaries) do paths[#paths+1]=a end
  for _,f in ipairs(s.facets) do
    local valid=work_ref(f.producer,"facets",f.id,".producer")
    for _,p in ipairs(f.paths) do
      local a={path=p,file=b.spec_file,field=sf("facets",f.id,".paths["..original_index(raw_spec.facets[original_index(raw_spec.facets,f.id)].paths,p).."]")}
      paths[#paths+1]=a
      if valid then
        local contained=false
        for _,boundary in ipairs(work[f.producer].paths) do if semantic_contains(boundary,p) then contained=true end end
        if not contained then ownership(a,p,"Facet is outside its producer boundary") end
      end
    end
  end
  -- Shared segments also alias: Src/a and src/b conflict portably even though
  -- their complete paths are neither ancestors nor case-fold equivalent.
  local spelling={}
  local function aliases(a)
    local path=semantic_stem(a.path)
    for i=1,#path+1 do
      if i>#path or string.byte(path,i)==47 then
        local prefix=string.sub(path,1,i-1)
        local key=string.lower(prefix)
        local previous=spelling[key]
        if previous and previous.prefix~=prefix then ownership(a,previous.path,"Portable path segment alias")
        elseif not previous then spelling[key]={prefix=prefix,path=a.path} end
      end
    end
  end
  for _,a in ipairs(paths) do aliases(a) end
  if status=="captured" then
    local seen={}
    for _,kind in ipairs({"files","directories"}) do
      for i,entry in ipairs(capture[kind]) do
        local field="capture."..kind.."["..i.."]"
        if entry.root~="current:"..w.root.id and entry.root~="base:"..w.root.id then
          problem("",field..".root","Unknown captured source root")
        end
        local key=entry.root.."\0"..entry.path
        if seen[key] then problem("",field,"Duplicate captured topology entry") end
        seen[key]=true
        aliases({path=entry.path,file="",field=field..".path"})
        for _,a in ipairs(boundaries) do
          local stem=semantic_stem(a.path)
          local is_directory=string.sub(a.path,-1)=="/"
          if (kind=="directories" and not is_directory and entry.path==stem) or
             (kind=="files" and ((is_directory and entry.path==stem) or semantic_ancestor(entry.path,stem) or (not is_directory and semantic_ancestor(stem,entry.path)))) or
             (kind=="directories" and not is_directory and semantic_ancestor(stem,entry.path)) then
            ownership(a,entry.path,"Captured topology conflicts with boundary kind")
          end
        end
        -- A facet always denotes a file, including an absent one.
        for _,f in ipairs(s.facets) do for _,p in ipairs(f.paths) do
          if (kind=="directories" and (entry.path==p or semantic_ancestor(p,entry.path))) or
             (kind=="files" and (semantic_ancestor(entry.path,p) or semantic_ancestor(p,entry.path))) then
            ownership({path=p,file=b.spec_file,field=sf("facets",f.id,".paths")},entry.path,"Captured topology conflicts with file facet")
          end
        end end
      end
    end
  end
  local complete_dependencies={}
  for _,u in ipairs(s.work) do complete_dependencies[u.id]=false end
  local resolved_dependencies=engine.array({})
  for _,d in ipairs(s.dependencies) do
    work_ref(d.consumer,"dependencies",d.id,".consumer")
    local source
    if d.source=="contract" then source=contracts[d.ref] else source=facets[d.ref] end
    if not source then spec_problem("dependencies",d.id,".ref","Unknown dependency reference: "..d.ref)
    else
      if d.source=="contract" then
        if d.quality~="available" then spec_problem("dependencies",d.id,".quality","Contract dependencies require available quality","schema") end
        if not semantic_has(source.consumers,d.consumer) then spec_problem("dependencies",d.id,".consumer","Consumer is absent from contract consumers") end
      elseif d.consumer==source.producer then spec_problem("dependencies",d.id,".consumer","Facet producer cannot depend on itself") end
      resolved_dependencies[#resolved_dependencies+1]=engine.object({id=d.id,producer=source.producer,consumer=d.consumer,source=d.source,ref=d.ref,phase=d.phase,quality=d.quality})
    end
    if d.source=="facet" and d.phase=="complete" then complete_dependencies[d.consumer]=true end
  end
  -- Missing subscriptions and check applicability depend on valid references.
  -- Do not replace an actionable reference diagnostic with a cascading policy error.
  if #ctx.errors==0 then
    for _,c in ipairs(s.contracts) do for _,id in ipairs(c.consumers) do
      local found=false
      for _,d in ipairs(s.dependencies) do
        if d.consumer==id and d.source=="contract" and d.ref==c.id and d.phase=="start" and d.quality=="available" then found=true end
      end
      if not found then spec_problem("contracts",c.id,".consumers","Consumer requires a start/contract/available dependency: "..id,"schema") end
    end end
  end
  local global_valid=#ctx.errors==0
  local models=engine.array({})
  for _,entry in ipairs(b.jobs) do
    local j,file=entry.value,entry.file
    local raw
    for _,candidate in ipairs(envelope.bundle.jobs) do if candidate.file==file then raw=candidate.value end end
    local function jf(id,suffix) return "job.roles["..original_index(raw.roles,id).."]"..(suffix or "") end
    local assignments,workers,reviewers={},{},{}
    local role_map=semantic_map(j.roles)
    local valid=true
    local function job_problem(field,message,code,details)
      valid=false; problem(file,field,message,code,details)
    end
    for _,r in ipairs(j.roles) do
      if r.kind=="worker" then
        workers[#workers+1]=r
        for _,id in ipairs(r.work) do
          if not work[id] then job_problem(jf(r.id,".work"),"Unknown assigned work: "..id)
          elseif assignments[id] then job_problem(jf(r.id,".work"),"Work has multiple workers: "..id)
          else assignments[id]=r.id end
        end
      else
        reviewers[#reviewers+1]=r
        if #r.checks>0 then job_problem(jf(r.id,".checks"),"Reviewer checks must be empty","schema") end
        for _,id in ipairs(r.targets) do if not work[id] then job_problem(jf(r.id,".targets"),"Unknown reviewer target: "..id) end end
        for _,u in ipairs(s.work) do if not semantic_has(r.targets,u.id) then job_problem(jf(r.id,".targets"),"Reviewer must target every work unit") end end
      end
      for _,id in ipairs(r.checks) do check_ref(id,file,jf(r.id,".checks")) end
    end
    if #reviewers~=1 then job_problem("job.roles","Exactly one independent reviewer is required","schema") end
    if #workers==0 then job_problem("job.roles","At least one worker is required","schema") end
    for _,u in ipairs(s.work) do if #workers>0 and not assignments[u.id] then job_problem("job.roles","Work has no worker: "..u.id) end end
    if j.shape=="pipeline" or (context and j.stages) then
      local staged={}
      for i,stage in ipairs(j.stages) do for _,id in ipairs(stage) do
        if not role_map[id] or role_map[id].kind~="worker" then job_problem("job.stages["..i.."]","Stage must reference a worker: "..id)
        elseif staged[id] then job_problem("job.stages["..i.."]","Worker occurs in multiple stages: "..id)
        else staged[id]=true end
      end end
      for _,r in ipairs(workers) do if not staged[r.id] then job_problem("job.stages","Worker missing from stages: "..r.id) end end
    end
    local effective=engine.array({})
    for _,u in ipairs(s.work) do
      local required={}
      for id in pairs(mandatory[u.id]) do required[id]=true end
      if assignments[u.id] then for _,id in ipairs(role_map[assignments[u.id]].checks) do required[id]=true end end
      local ids=semantic_keys(required)
      local vector=false
      for _,id in ipairs(ids) do
        local c=checks[id]
        if c and semantic_has(c.inputs,"output_vector") then vector=true end
      end
      if global_valid and vector and not complete_dependencies[u.id] then job_problem("job.roles","output_vector check has no complete facet dependency for work: "..u.id,"schema") end
      if global_valid and complete_dependencies[u.id] and not vector then job_problem("job.roles","Complete facet dependency requires an output_vector check for work: "..u.id,"schema") end
      effective[#effective+1]=engine.object({work=u.id,checks=ids})
    end
    if valid then
      local mapping=engine.array({})
      for _,u in ipairs(s.work) do mapping[#mapping+1]=engine.object({work=u.id,role=assignments[u.id]}) end
      local predecessors={}
      if j.shape=="pipeline" then
        for i=1,#j.stages-1 do
          for _,a in ipairs(j.stages[i]) do for _,z in ipairs(j.stages[i+1]) do
            for _,producer in ipairs(role_map[a].work) do for _,consumer in ipairs(role_map[z].work) do
              predecessors[#predecessors+1]=engine.object({id="pipeline:"..producer..":"..consumer,source="pipeline",producer=producer,consumer=consumer,producer_role=a,consumer_role=z,phase="start",quality="done"})
            end end
          end end
        end
      end
      predecessors=sorted(predecessors,function(x)return x.id end)
      local adjacency={}
      local function vertex(label) if not adjacency[label] then adjacency[label]={} end end
      local function edge(a,z) vertex(a); vertex(z); adjacency[a][z]=true end
      local reviewer=reviewers[1].id
      edge("role:"..reviewer..":start","role:"..reviewer..":complete")
      for _,u in ipairs(s.work) do
        edge("work:"..u.id..":start","work:"..u.id..":complete")
        edge("work:"..u.id..":complete","work:"..u.id..":accepted")
        edge("work:"..u.id..":complete","role:"..assignments[u.id]..":complete")
        edge("role:"..reviewer..":complete","work:"..u.id..":accepted")
      end
      for _,r in ipairs(workers) do edge("role:"..r.id..":complete","role:"..reviewer..":start") end
      for _,d in ipairs(resolved_dependencies) do
        if work[d.producer] and work[d.consumer] and d.source=="facet" and d.quality~="available" then
          edge("work:"..d.producer..":"..(d.quality=="done" and "complete" or "accepted"),"work:"..d.consumer..":"..d.phase)
        end
      end
      for _,p in ipairs(predecessors) do edge("work:"..p.producer..":complete","work:"..p.consumer..":start") end
      local vertices=semantic_keys(adjacency)
      local edges=engine.array({})
      for _,v in ipairs(vertices) do
        adjacency[v]=semantic_keys(adjacency[v])
        for _,to in ipairs(adjacency[v]) do edges[#edges+1]=engine.object({from=v,to=to}) end
      end
      local cycle=semantic_cycle(vertices,adjacency)
      if cycle then problem(file,"job","Unsatisfiable dependency phases","dependency_cycle",{cycle=cycle}) end
      models[#models+1]=engine.object({id=j.id,file=file,assignments=mapping,reviewer=reviewer,effective_checks=effective,pipeline_predecessors=predecessors,graph=engine.object({vertices=vertices,edges=edges})})
    end
  end
  local base=engine.object({bundle=b,capture=capture,dependencies=resolved_dependencies,jobs=models})
  if #ctx.errors>0 then return ctx.finish(base) end
  local requests=engine.array({})
  if context then
    for index,entry in ipairs(b.jobs) do
      local j,model=entry.value,models[index]
      if j.shape~="pipeline" and j.shape~="fanout_review" then
        requests[#requests+1]=engine.object({index=index,id=j.id,shape=j.shape,file=entry.file,
          input=engine.object({version=1,job=j,spec=s,assignments=model.assignments,reviewer=model.reviewer})})
      end
    end
  end
  return ctx.finish(engine.object({base=base,requests=requests}))
end

-- Each phase accepts/returns structural values. Native owns their lifetime,
-- immutability and cumulative conversion accounting between protected calls.
local function check_shape(input)
  local request,response,total=input.request,input.response,input.total
  local work=semantic_map(request.input.spec.work)
  local function invalid()
    return engine.object({error=engine.object({code="internal",message="Invalid trusted shape result",file=request.file,field="job.shape",details=engine.object({})})})
  end
  local function exact(value,keys)
    if engine.kind(value)~="object" then return false end
    local allowed={}
    for _,key in ipairs(keys) do allowed[key]=true; if value[key]==nil then return false end end
    for key in pairs(value) do if not allowed[key] then return false end end
    return true
  end
  if type(response)~="table" then return invalid() end
  if response.ok==false then
    if not exact(response,{"ok","error"}) then return invalid() end
    local e=response.error
    if engine.kind(e)~="object" then return invalid() end
    for key,value in pairs(e) do
      if key=="details" then
        if engine.kind(value)~="object" then return invalid() end
      elseif key=="code" or key=="message" or key=="field" or key=="file" then
        if not utf8_text(value) then return invalid() end
      else return invalid() end
    end
    if type(e.code)~="string" or e.code=="" or type(e.message)~="string" or type(e.field)~="string" or
       (e.file~=nil and e.file~="" and not path_valid(e.file,false)) then return invalid() end
    -- Native Glaze owns the existing finite value/JSON structural bounds.
    local encoded,refusal=engine.json_encode(e)
    if refusal then return invalid() end
    return engine.object({error=e})
  end
  if not exact(response,{"ok","value"}) or response.ok~=true or
     not exact(response.value,{"obligations","guidance"}) then return invalid() end
  local value=response.value
  if not utf8_text(value.guidance) or #value.guidance>65536 or engine.kind(value.obligations)~="array" then return invalid() end
  local count=0
  for key in pairs(value.obligations) do
    if type(key)~="number" or key%1~=0 or key<1 or key>#value.obligations then return invalid() end
    count=count+1
  end
  if count~=#value.obligations then return invalid() end
  -- Check counts before expanding/retaining any edge or normalized record.
  if #value.obligations>10000 or total+#value.obligations>100000 then
    return engine.object({error=engine.object({code="resource_limit",message="Shape obligation limit exceeded",file=request.file,field="job.shape",
      details=engine.object({limit=#value.obligations>10000 and "shape_obligations_per_job" or "shape_obligations",maximum=#value.obligations>10000 and 10000 or 100000})})})
  end
  total=total+#value.obligations
  local obligations,by_id={},{}
  local mapping={}
  for _,row in ipairs(request.input.assignments) do mapping[row.work]=row.role end
  for _,o in ipairs(value.obligations) do
    if not exact(o,{"id","producer","consumer","phase","quality"}) or not id_valid(o.id) or by_id[o.id] or
       not work[o.producer] or not work[o.consumer] or o.producer==o.consumer or
       (o.phase~="start" and o.phase~="complete") or o.quality~="done" then return invalid() end
    by_id[o.id]=engine.object({id="shape:"..request.shape..":"..o.id,source="shape",producer=o.producer,consumer=o.consumer,
      producer_role=mapping[o.producer],consumer_role=mapping[o.consumer],phase=o.phase,quality=o.quality})
  end
  -- The synthetic prefix is identical for this selected shape. Sort the
  -- original IDs once, avoiding repeated comparisons of that long prefix.
  for _,id in ipairs(semantic_keys(by_id)) do obligations[#obligations+1]=by_id[id] end
  return engine.object({value=engine.object({index=request.index,id=request.id,shape=request.shape,
    obligations=engine.array(obligations),guidance=value.guidance,total=total})})
end

local function finish_shapes(input,legacy)
  local prepared,results=input.prepared,input.results
  if engine.kind(results)~="array" or #results~=#prepared.requests or
     (not legacy and (engine.kind(input.definitions)~="array" or #input.definitions~=#prepared.requests)) then
    return engine.object({error=engine.object({code="internal",message="Invalid trusted shape correspondence",file="",field="job.shape",details=engine.object({})})})
  end
  local base=engine.object({})
  for key,value in pairs(prepared.base) do base[key]=value end
  base.jobs=engine.array({})
  for _,model in ipairs(prepared.base.jobs) do
    local copy=engine.object({})
    for key,value in pairs(model) do copy[key]=value end
    base.jobs[#base.jobs+1]=copy
  end
  local ctx=schema_context({},false)
  local shape_results=engine.array({})
  for n,request in ipairs(prepared.requests) do
    local result=results[n]
    local model=base.jobs[request.index]
    -- Results and definitions are trusted native-owned correspondence, never
    -- fields supplied by declarations or by F. Refuse mismatched orchestration.
    if not result or result.index~=request.index or result.id~=request.id or result.shape~=request.shape then
      return engine.object({error=engine.object({code="internal",message="Invalid trusted shape result",file=request.file,field="job.shape",details=engine.object({})})})
    end
    local adjacency={}
    for _,vertex in ipairs(model.graph.vertices) do adjacency[vertex]={} end
    for _,edge in ipairs(model.graph.edges) do adjacency[edge.from][edge.to]=true end
    for _,o in ipairs(result.obligations) do
      adjacency["work:"..o.producer..":complete"]["work:"..o.consumer..":"..o.phase]=true
    end
    local vertices=model.graph.vertices
    local edges=engine.array({})
    for _,vertex in ipairs(vertices) do
      adjacency[vertex]=semantic_keys(adjacency[vertex])
      for _,to in ipairs(adjacency[vertex]) do edges[#edges+1]=engine.object({from=vertex,to=to}) end
    end
    local cycle=semantic_cycle(vertices,adjacency)
    if cycle then ctx.issue(request.file,"job","Unsatisfiable dependency phases","dependency_cycle",{cycle=cycle}) end
    model.graph=engine.object({vertices=vertices,edges=edges})
    if legacy then
      shape_results[#shape_results+1]=engine.object({id=request.id,obligations=result.obligations,guidance=result.guidance})
    else
      local definition=input.definitions[n]
      if not definition or definition.index~=request.index or definition.job~=request.id or definition.id~=request.shape or type(definition.revision)~="string" or type(definition.definition)~="string" then
        return engine.object({error=engine.object({code="internal",message="Invalid trusted shape definition",file=request.file,field="job.shape",details=engine.object({})})})
      end
      model.selected_shape=engine.object({id=definition.id,revision=definition.revision,definition=definition.definition,
        obligations=result.obligations,guidance=result.guidance})
    end
  end
  if legacy then base.shape_results=shape_results end
  return ctx.finish(base)
end

local function validate_bundle(input,context,mode)
  local prepared=prepare_shapes(input,context,mode)
  if prepared.error then return prepared end
  if not context then return engine.object({value=prepared.value.base}) end
  local results=engine.array({})
  local total=0
  for _,request in ipairs(prepared.value.requests) do
    local checked=check_shape({request=request,response=context.call(request.shape,request.input),total=total})
    if checked.error then return checked end
    results[#results+1]=checked.value
    total=checked.value.total
  end
  return finish_shapes({prepared=prepared.value,results=results},true)
end
