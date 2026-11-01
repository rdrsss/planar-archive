-- Authoring orchestrator handler (RA-7). Embedded only in the authoring stack.
-- Reads the sealed command names from the capture result, validates the selected job only, builds the
-- orchestrator_context_v1 value and renders the five mandatory sections from that context alone.
local orchestrator_command_impl
do
  local function refusal() return {ok=false,error={code='internal',message='Invalid embedded command protocol',field=''}} end
  local purposes={
    ["activate"]="Start the driven run for the selected job after its graph has been reviewed.",
    ["check"]="Execute the declared checks for one work unit and record the outcome.",
    ["check-recover"]="Recover a check execution that was interrupted before it recorded an outcome.",
    ["checkpoint"]="Record a durable checkpoint of the driven run for later recovery.",
    ["claim"]="Claim a work unit for a worker session of the driven run.",
    ["critique"]="Print the mechanical critique of a specification for the author to review.",
    ["explain-rejection"]="Print a prompt that explains a rejected declaration diagnostic to its author.",
    ["export"]="Export the accepted source of one ledger revision.",
    ["history"]="Read the ledger history by page, revision, state or request.",
    ["ingest"]="Print a prompt that guides the author in writing a job declaration.",
    ["init"]="Create a new ledger for a workspace identity.",
    ["load"]="Validate a workspace and accept it into the ledger as a new revision.",
    ["observe"]="Read an observation directory and print the canonical observations.",
    ["orchestrator"]="Print this orchestrator prompt for the selected job.",
    ["preview"]="Preview the next step of the driven run without changing its state.",
    ["recover"]="Recover the driven run after an interruption.",
    ["refresh"]="Refresh the captured inputs of the driven run.",
    ["release"]="Release a claimed work unit without completing it.",
    ["render"]="Render the prompt of one role for one job.",
    ["spec-prompt"]="Print a prompt that guides the author in writing a specification.",
    ["status"]="Report the recorded state of the driven run.",
    ["takeover"]="Take over a work unit claimed by another session.",
    ["verdict"]="Record the reviewer verdict for a work unit."
  }
  -- sorted() types its result, and refuses an input that is already typed when it has at most one element; copy first.
  local function sort_copy(values,key)
    local copy={}
    for i,v in ipairs(values) do copy[i]=v end
    return sorted(copy,key)
  end
  local future={"activate","status","preview","claim","takeover","refresh","checkpoint","release","recover","check","check-recover","verdict"}
  local function names_valid(commands)
    if type(commands)~="table" then return false end
    local count=0
    for key in pairs(commands) do
      if type(key)~="number" or key%1~=0 or key<1 then return false end
      count=count+1
    end
    if count~=#commands then return false end
    for i,name in ipairs(commands) do
      if type(name)~="string" or name=="" then return false end
      if i>1 and not (byte_order(commands[i-1],name) and commands[i-1]~=name) then return false end
    end
    return true
  end
  local function build_actions(commands)
    local available,all={},{}
    for _,name in ipairs(commands) do available[name]=true; all[name]=true end
    for _,name in ipairs(future) do all[name]=true end
    local names={}
    for name in pairs(all) do names[#names+1]=name end
    names=sort_copy(names,function(x)return x end)
    local actions={}
    for _,name in ipairs(names) do
      actions[#actions+1]=engine.object({command=name,status=available[name] and "available" or "future",purpose=purposes[name] or "Run the sealed command "..name.."."})
    end
    return engine.array(actions)
  end
  local function list_text(items) return table.concat(items,", ") end
  local function intersects(scope,units)
    for _,id in ipairs(scope) do if units[id] then return true end end
    return false
  end
  local function build_context(validated,captured,actions)
    local b=validated.bundle
    local job_id=captured.job
    local index
    for i,entry in ipairs(b.jobs) do if entry.value.id==job_id then index=i end end
    if not index then return nil end
    local j,model=b.jobs[index].value,validated.jobs[index]
    if not model then return nil end
    local roles,units={},{}
    for _,r in ipairs(sort_copy(j.roles,function(x)return x.id end)) do
      if r.kind=="worker" then
        for _,id in ipairs(r.work) do units[id]=true end
        roles[#roles+1]=engine.object({id=r.id,kind=r.kind,work=sort_copy(r.work,function(x)return x end)})
      else
        roles[#roles+1]=engine.object({id=r.id,kind=r.kind,targets=sort_copy(r.targets,function(x)return x end)})
      end
    end
    local stages={}
    if j.shape=="pipeline" then
      for _,stage in ipairs(j.stages or {}) do stages[#stages+1]=stage end
    end
    local prescriptions={}
    for _,p in ipairs(sort_copy(b.spec.prescriptions,function(x)return x.id end)) do
      if p.scope=="global" or intersects(p.scope,units) then
        local scope=p.scope
        if scope~="global" then scope=sort_copy(scope,function(x)return x end) end
        prescriptions[#prescriptions+1]=engine.object({id=p.id,scope=scope,text=p.text.bytes})
      end
    end
    return engine.object({
      workspace=engine.object({id=b.workspace.id}),
      spec=engine.object({id=b.spec.id}),
      job=engine.object({id=j.id}),
      shape=engine.object({kind=j.shape,max_rounds=j.max_rounds,stages=engine.array(stages)}),
      roles=engine.array(roles),
      graph=engine.object({vertices=model.graph.vertices,edges=model.graph.edges}),
      prescriptions=engine.array(prescriptions),
      actions=actions
    })
  end
  -- The projection sees only the context value.
  local function project(c)
    local sections={}
    local function section(id,heading,body) sections[#sections+1]={id=id,text="## "..heading.."\n"..body,mandatory=true} end
    section("orchestrator_authority","Orchestrator authority",
      "This prompt describes the graph of job "..c.job.id.." in workspace "..c.workspace.id.." and spec "..c.spec.id..".\n"..
      "The driver schedules sessions; Planar does not. Planar prints this prompt and states no execution state.\n"..
      "Use only the actions marked as available below; actions marked as future are unavailable.\n")
    local body={"- shape: "..c.shape.kind.." max_rounds "..c.shape.max_rounds.."\n"}
    for i,stage in ipairs(c.shape.stages) do body[#body+1]="- stage "..i..": "..list_text(stage).."\n" end
    for _,v in ipairs(c.graph.vertices) do body[#body+1]="- vertex "..v.."\n" end
    for _,e in ipairs(c.graph.edges) do body[#body+1]="- edge "..e.from.." -> "..e.to.."\n" end
    section("orchestrator_graph","Orchestrator graph",table.concat(body))
    body={}
    for _,r in ipairs(c.roles) do
      if r.kind=="worker" then body[#body+1]="- "..r.id.." (worker) work: "..list_text(r.work).."\n"
      else body[#body+1]="- "..r.id.." (reviewer) targets: "..list_text(r.targets).."\n" end
    end
    section("orchestrator_roles","Orchestrator roles",table.concat(body))
    body={}
    for _,p in ipairs(c.prescriptions) do
      body[#body+1]="- "..p.id.." ("..(p.scope=="global" and "global" or list_text(p.scope))..")\n"
      body[#body+1]=presentation_data("text",p.text)
    end
    if #body==0 then body[1]="No prescriptions drive this job.\n" end
    section("orchestrator_prescriptions","Orchestrator prescriptions",table.concat(body))
    body={}
    for _,a in ipairs(c.actions) do body[#body+1]="- "..a.command.." ("..a.status.."): "..a.purpose.."\n" end
    section("orchestrator_actions","Orchestrator actions",table.concat(body))
    return {version=1,sections=sections}
  end
  orchestrator_command_impl=function(invocation,context)
    local opened=context.open_workspace({workspace=invocation.options.workspace})
    if not opened.ok then return opened end
    local routed=route_workspace(opened.value)
    if routed.error then return {ok=false,error=routed.error} end
    local captured=context.capture_orchestrator({route=routed.value,job=invocation.options.job})
    if not captured.ok then return captured end
    local value=captured.value
    if type(value)~="table" or not names_valid(value.commands) then return refusal() end
    local validated=validate_bundle(engine.object({bundle=value.bundle,capture=value.capture}),nil,"selected_job")
    if validated.error then return {ok=false,error=validated.error} end
    local c=build_context(validated.value,value,build_actions(value.commands))
    if not c then return refusal() end
    local parts={"# Planar orchestrator prompt\n"}
    local result=project(c)
    for _,s in ipairs(result.sections) do parts[#parts+1]=s.text end
    local markdown=table.concat(parts,"\n")
    local digest=engine.fragment_digest(markdown)
    if type(digest)~="string" then return refusal() end
    local files={}
    for _,f in ipairs(sort_copy(value.files,function(x)return x.root.."\0"..x.path end)) do
      files[#files+1]=engine.object({root=f.root,path=f.path,kind="file",mode=f.mode,size=#f.bytes,digest=engine.file_digest(f.bytes)})
    end
    local registry=validated.value.bundle.registry
    return {ok=true,value=engine.object({version=1,kind="prompt",prompt=engine.object({
      kind="orchestrator",markdown=markdown,bytes=#markdown,digest=digest,
      inputs=engine.object({files=engine.array(files),registry=engine.object({id=registry.id,revision=registry.revision})})
    })})}
  end
end
