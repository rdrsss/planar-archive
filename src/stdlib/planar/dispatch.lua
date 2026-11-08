-- Trusted execution projection (execution contract EX-5, P2-2, P2-3): work_exec_v1 W, role_structure_v1 and the
-- dispatch packet with dispatch_manifest_v1. Pure over recorded interpretation values and ledger facts supplied by
-- native code: no capture, no checkout, no reads, no clock and no secrets. Only three names leave this block.
local dispatch_internal,work_exec,render_dispatch
do
  local WORK_EXEC_VERSIONS={"declarations_v1","selection_v1","dependencies_v1","exact_bytes_v1","canonical_v1","xxh3_128_v1","work_exec_v1"}
  -- D147: byte-exact purposes of the legal next actions, in E1 table order per role kind.
  local PURPOSES={
    refresh="Re-render this work and acknowledge the current packet.",
    checkpoint="Record observations of progress on this run.",
    check="Run the checks of the job against the output of this role.",
    release="End this run as released or interrupted.",
    verdict="Record the verdict of the reviewer for this run.",
  }
  local ACTIONS={worker={"refresh","checkpoint","check","release"},reviewer={"refresh","checkpoint","verdict","release"}}

  local function copy(value)
    local out={}
    for key,item in pairs(value) do out[key]=item end
    return engine.object(out)
  end
  local function filter(items,predicate)
    local out={}
    for _,item in ipairs(items or {}) do if predicate(item) then out[#out+1]=item end end
    return engine.array(out)
  end
  -- sorted() tags its argument when it has at most one item, so always sort a fresh plain copy.
  local function sort_copy(items,key)
    local out={}
    for i,item in ipairs(items or {}) do out[i]=item end
    return sorted(out,key)
  end
  local function plain_list(items)
    local out={}
    for i,item in ipairs(items or {}) do out[i]=item end
    return engine.array(out)
  end
  local function decimal(v)
    if math.type(v)=="integer" then return ""..v end
    return v
  end

  -- Locate a job (and optionally a role) in an interpretation {bundle,semantic}; the recorded envelope has both.
  local function locate(interpretation,job_id,role_id)
    if engine.kind(interpretation)~="object" or engine.kind(interpretation.bundle)~="object"
      or engine.kind(interpretation.semantic)~="object" then return nil,"interpretation" end
    local b,s=interpretation.bundle,interpretation.semantic
    local job,job_model,role
    for _,entry in ipairs(b.jobs or {}) do if entry.value and entry.value.id==job_id then job=entry.value end end
    for _,model in ipairs(s.jobs or {}) do if model.id==job_id then job_model=model end end
    if not job or not job_model then return nil,"job" end
    if role_id~=nil then
      for _,r in ipairs(job.roles) do if r.id==role_id then role=r end end
      if not role then return nil,"role" end
    end
    return {bundle=b,workspace=b.workspace,spec=b.spec,registry=b.registry,job=job,job_model=job_model,role=role,
      dependencies=s.dependencies or {},jobs=s.jobs}
  end

  local function role_ids(role)
    if role.kind=="worker" then return role.work or {} end
    return role.targets or {}
  end

  -- role_structure_v1 = {job,role,kind,work,targets,ownership,shape} (EX-5); lists sorted, empty lists typed.
  local function role_structure(location)
    local role,job,s=location.role,location.job,location.spec
    local ids=role_ids(role)
    local selected=semantic_set(ids)
    local ownership={}
    for _,unit in ipairs(s.work) do
      if selected[unit.id] then ownership[#ownership+1]=engine.object({work=unit.id,paths=plain_list(unit.paths)}) end
    end
    local function identity(x) return x end
    local structure=engine.object({
      job=job.id,role=role.id,kind=role.kind,
      work=role.kind=="worker" and sort_copy(ids,identity) or engine.array({}),
      targets=role.kind=="reviewer" and sort_copy(ids,identity) or engine.array({}),
      ownership=sort_copy(ownership,function(entry)return entry.work end),
      shape=job.shape,
    })
    return structure,engine.hash_value("role_structure_v1",structure)
  end

  -- Prerequisite status from ledger qualities (EX-4): contracts are present in the revision; a facet needs a
  -- recorded publication; done and accepted qualities need completion or acceptance facts (none exist in R15,
  -- except that a producer role whose ledger state is done counts for the done quality); questions are
  -- declaration facts. role_states maps role ID to the ledger state of the same job, when known.
  local function facet_status(dependency,publication,producer_role,role_states)
    if dependency.quality=="available" then return publication and "available" or "unavailable" end
    if dependency.quality=="done" then
      return (producer_role and role_states and role_states[producer_role]=="done") and "available" or "unavailable"
    end
    return "unavailable"
  end

  -- Complete per-role material: W, its digest, the manifest prerequisites, facet statuses for the dependency
  -- section and whether a start prerequisite is absent (EX-4 precedence rule 3).
  local function role_material(location,publications,repair,role_states)
    local b,w,s,job,job_model,role=location.bundle,location.workspace,location.spec,location.job,location.job_model,location.role
    local source=w.root.id
    local ids=role_ids(role)
    local selected=semantic_set(ids)
    local assignments={}
    for _,entry in ipairs(job_model.assignments or {}) do assignments[entry.work]=entry.role end
    local published={}
    for _,entry in ipairs(publications or {}) do published[entry.facet]=entry end
    local units=filter(s.work,function(u)return selected[u.id] end)
    local contracts,contract_ids={},{}
    for _,c in ipairs(s.contracts or {}) do
      local semantic=engine.hash_value("contract_v1",{kind="text",normalizer="exact_bytes_v1",expected=c.expected})
      contract_ids[c.id]=semantic
      local applicable=selected[c.producer]
      for _,consumer in ipairs(c.consumers) do if selected[consumer] then applicable=true end end
      if applicable then contracts[#contracts+1]=engine.object({declaration=c,semantic=semantic}) end
    end
    local deps,facets_needed,start_facets,prerequisites,mapping_needed={}, {}, {}, {}, {}
    local blocked=false
    for id in pairs(selected) do mapping_needed[id]=true end
    for _,d in ipairs(location.dependencies) do if selected[d.consumer] then
      local entry=copy(d)
      mapping_needed[d.producer]=true
      local status
      if d.source=="contract" then entry.semantic=contract_ids[d.ref]; status="available"
      else
        facets_needed[d.ref]=true
        local publication=published[d.ref]
        status=facet_status(d,publication,assignments[d.producer],role_states)
        if d.phase=="start" and not selected[d.producer] then
          local start=engine.object({dependency=d.id})
          if publication then start.semantic=publication.semantic else start.unavailable=true end
          start_facets[#start_facets+1]=start
        end
      end
      -- Internal obligations (producer owned by this role) never block this role's start.
      if d.phase=="start" and status~="available" and not selected[d.producer] then blocked=true end
      deps[#deps+1]=entry
      prerequisites[#prerequisites+1]=engine.object({id=d.id,phase=d.phase,status=status,source=d.source})
    end end
    local pipeline=filter(job_model.pipeline_predecessors,function(p)return selected[p.consumer] end)
    for _,p in ipairs(pipeline) do
      mapping_needed[p.producer]=true
      local prerequisite=copy(p)
      prerequisite.status=(role_states and role_states[p.producer_role]=="done") and "available" or "unavailable"
      if p.phase=="start" and prerequisite.status~="available" then blocked=true end
      prerequisites[#prerequisites+1]=prerequisite
    end
    local function scoped(items,field)
      local out={}
      for _,item in ipairs(items or {}) do
        if item[field]=="global" then out[#out+1]=item
        else
          local intersection=filter(item[field],function(id)return selected[id] end)
          if #intersection>0 then local c=copy(item); c[field]=intersection; out[#out+1]=c end
        end
      end
      return engine.array(out)
    end
    local questions=scoped(s.questions,"scope")
    for _,q in ipairs(questions) do if q.blocks~="none" then
      local status=q.answer and "answered" or "unavailable"
      if q.blocks=="start" and status=="unavailable" then blocked=true end
      prerequisites[#prerequisites+1]=engine.object({id="question:"..q.id,phase=q.blocks,status=status,source="question"})
    end end
    -- Graph edge role:<worker>:complete -> role:<reviewer>:start: a reviewer starts after every worker completes.
    if role.kind=="reviewer" then
      for _,r in ipairs(job.roles) do
        if r.kind=="worker" and not (role_states and role_states[r.id]=="done") then blocked=true end
      end
    end
    local effective=filter(job_model.effective_checks,function(e)return selected[e.work] end)
    local checks_needed,tools_needed={},{}
    for _,e in ipairs(effective) do for _,id in ipairs(e.checks) do checks_needed[id]=true end end
    local checks=filter(b.registry.checks,function(c)return checks_needed[c.id] end)
    for _,c in ipairs(checks) do tools_needed[c.toolchain]=true end
    local ownership={}
    for _,u in ipairs(units) do ownership[#ownership+1]=engine.object({work=u.id,paths=u.paths}) end
    local shape=engine.object({kind=job.shape,max_rounds=job.max_rounds,
      mapping=filter(job_model.assignments,function(a)return mapping_needed[a.work] end),reviewer=job_model.reviewer})
    if job_model.selected_shape then
      local selected_shape=job_model.selected_shape
      shape.definition=selected_shape.definition
      shape.revision=selected_shape.revision
      local obligations={}
      for _,o in ipairs(selected_shape.obligations or {}) do
        if selected[o.producer] or selected[o.consumer] then obligations[#obligations+1]=o end
      end
      shape.obligations=sort_copy(obligations,function(o)return o.id end)
    end
    local W=engine.object({
      versions=plain_list(WORK_EXEC_VERSIONS),workspace=w.id,spec=s.id,job=job.id,role=role.id,root=source,
      base=engine.object({status="declared",root=source,label=w.root.base}),
      goal=s.goal,units=units,milestones=scoped(s.milestones,"work"),constraints=scoped(s.constraints,"scope"),
      prescriptions=scoped(s.prescriptions,"scope"),contracts=engine.array(contracts),dependencies=engine.array(deps),
      facets=filter(s.facets,function(f)return facets_needed[f.id] end),pipeline_predecessors=pipeline,
      questions=questions,ownership=engine.object({work=engine.array(ownership),frozen=s.frozen or engine.array({}),excluded=w.root.excluded or engine.array({})}),
      effective_checks=effective,checks=checks,
      toolchains=filter(b.registry.toolchains,function(t)return tools_needed[t.id] end),
      shape=shape,start_facets=engine.array(start_facets),repair=decimal(repair or "0"),
    })
    local facet_values={}
    for _,f in ipairs(W.facets) do
      facet_values[#facet_values+1]=engine.object({id=f.id,status=published[f.id] and "available" or "unavailable"})
    end
    return {work=W,digest=engine.hash_value("work_exec_v1",W),ids=ids,blocked=blocked,
      prerequisites=sort_copy(prerequisites,function(p)return p.id end),facets=engine.array(facet_values)}
  end

  local function selection_issue(ctx,missing)
    if missing=="role" then ctx.issue("","selection.role","Unknown selected role","reference")
    elseif missing=="job" then ctx.issue("","selection.job","Unknown selected job","reference")
    else ctx.issue("","interpretation","Invalid recorded interpretation","schema") end
    return ctx.finish({})
  end

  -- Fixed export: {interpretation,job,role,publications?,repair?} -> {work,digest,structure,structure_digest}.
  work_exec=function(input)
    local ctx=schema_context({},false)
    if engine.kind(input)~="object" then ctx.issue("","work_exec","Invalid work_exec request","schema"); return ctx.finish({}) end
    local location,missing=locate(input.interpretation,input.job,input.role)
    if not location then return selection_issue(ctx,missing) end
    local material=role_material(location,input.publications,input.repair,nil)
    local structure,structure_digest=role_structure(location)
    return ctx.finish(engine.object({work=material.work,digest=material.digest,structure=structure,structure_digest=structure_digest}))
  end

  local function authority_text(context,generation,revision)
    local run=context.run
    if run.kind=="preview" then
      return "This packet is a preview of a dispatch for generation "..generation.." of role "..context.role..", under revision "..revision.." of job "..context.job..". No run exists and nothing was retained. Checks have not run in this dispatch. Upstream facet content comes from recorded publications, and prerequisite status comes from ledger qualities, not from a checkout."
    end
    return "This packet was dispatched by Planar for run request "..run.request..", generation "..generation.." of role "..context.role..", under revision "..revision.." of job "..context.job..". Checks have not run in this dispatch. Upstream facet content comes from recorded publications, and prerequisite status comes from ledger qualities, not from a checkout."
  end

  local function run_text(run,kind)
    local lines={run.kind=="preview" and "run request: (preview)" or "run request: "..run.request}
    for _,verb in ipairs(ACTIONS[kind]) do lines[#lines+1]="- "..verb..": "..PURPOSES[verb] end
    lines[#lines+1]="Actions not shipped in this build are unavailable."
    return table.concat(lines,"\n").."\n"
  end

  -- dispatch_context_v1 (DD3): {revision:{id,source,interpretation},job,role,generation,run,snapshot_role?,
  -- interpretation,publications?,repair?,budget_bytes?}; run is {request,kind[,previous]} or {kind:"preview"}.
  local function check_context(ctx,context)
    local function bad(field) ctx.issue("","dispatch."..field,"Invalid dispatch context","schema") end
    if engine.kind(context)~="object" then bad("context"); return false end
    local rev=context.revision
    if engine.kind(rev)~="object" then bad("revision") else
      for _,key in ipairs({"id","source","interpretation"}) do if type(decimal(rev[key]))~="string" then bad("revision."..key) end end
    end
    if not id_valid(context.job) then bad("job") end
    if not id_valid(context.role) then bad("role") end
    if type(decimal(context.generation))~="string" then bad("generation") end
    local run=context.run
    if engine.kind(run)~="object" then bad("run")
    elseif run.kind=="preview" then
      if run.request~=nil or run.previous~=nil then bad("run") end
    elseif run.kind=="claim" or run.kind=="takeover" then
      if type(run.request)~="string" or #run.request==0 then bad("run.request") end
      if (run.kind=="takeover")~=(run.previous~=nil) then bad("run.previous") end
    else bad("run.kind") end
    if context.budget_bytes~=nil and (math.type(context.budget_bytes)~="integer" or context.budget_bytes<1 or context.budget_bytes>1048576) then bad("budget_bytes") end
    return #ctx.errors==0
  end

  -- Fixed export: dispatch_context_v1 -> {version,kind:"dispatch",packet,manifest,work}. Packet mode "dispatch",
  -- sections ordered as the shipped projections with the mandatory run section second (P2-2).
  render_dispatch=function(context)
    local ctx=schema_context({},false)
    if not check_context(ctx,context) then return ctx.finish({}) end
    local location,missing=locate(context.interpretation,context.job,context.role)
    if not location then return selection_issue(ctx,missing) end
    local role_states=nil
    local material=role_material(location,context.publications,context.repair,role_states)
    local role,job,w,s,b=location.role,location.job,location.workspace,location.spec,location.bundle
    local budget=context.budget_bytes or role.budget_bytes or job.budget_bytes
    local registry=engine.object({id=b.registry.id,revision=b.registry.revision,authority="driver_selected_trusted_configuration"})
    local C=engine.object({workspace=w.id,spec=s.id,job=job.id,role=role.id,kind=role.kind,work=plain_list(material.ids),
      budget_bytes=budget,registry=registry,
      optional=engine.object({workspace_notes=w.notes,rationale=s.rationale,job_notes=job.notes,preferences=role.preferences})})
    local selected={work={value=material.work,digest=material.digest},context=C,prerequisites=material.prerequisites,
      facets=material.facets,observation={status="not_captured"},outputs=engine.array({}),targets=engine.array({})}
    local model={bundle=b,jobs=location.jobs,dependencies=location.dependencies}
    local request={files=engine.array({}),job=job.id}
    local prepared=role_base_material(ctx,request,selected,model,location.job_model.selected_shape)
    if prepared.error then return prepared end
    local generation,revision=decimal(context.generation),decimal(context.revision.id)
    local sections={}
    for i,entry in ipairs(prepared.value.base) do
      local text=entry.text
      if i==1 then text="## authority\n\n"..authority_text(context,generation,revision).."\n" end
      sections[#sections+1]={id=entry.id,text=text,mandatory=entry.mandatory,included=true}
      if i==1 then sections[#sections+1]={id="run",text="## run\n\n"..run_text(context.run,role.kind),mandatory=true,included=true} end
    end
    local assembled=assemble_sections(ctx,sections,{},budget)
    if not assembled then return ctx.finish({}) end
    local markdown=assembled.markdown
    local packet=engine.object({version=1,mode="dispatch",workspace=w.id,spec=s.id,job=job.id,role=role.id,kind=role.kind,
      work=plain_list(material.ids),markdown=markdown})
    local versions=plain_list(WORK_EXEC_VERSIONS)
    versions[#versions+1]="dispatch_v1"
    local run
    if context.run.kind=="preview" then run=engine.object({kind="preview"})
    else
      run=engine.object({request=context.run.request,kind=context.run.kind})
      if context.run.kind=="takeover" then run.previous=decimal(context.run.previous) end
    end
    local publications={}
    for _,entry in ipairs(context.publications or {}) do
      publications[#publications+1]=engine.object({work=entry.work,facet=entry.facet,publication=decimal(entry.publication),semantic=entry.semantic})
    end
    local manifest=engine.object({
      version=1,revision=revision,job=job.id,role=role.id,generation=generation,run=run,versions=versions,
      work=engine.object({digest=material.digest}),
      render=engine.object({digest=engine.fragment_digest(markdown),bytes=#markdown,budget_bytes=budget}),
      sections=assembled.sections,omitted=assembled.omitted,prerequisites=material.prerequisites,registry=registry,
      inputs=engine.object({source=decimal(context.revision.source),interpretation=decimal(context.revision.interpretation),
        publications=sort_copy(publications,function(p)return p.work.."\0"..p.facet end),repair=decimal(context.repair or "0")}),
    })
    return ctx.finish(engine.object({version=1,kind="dispatch",packet=packet,manifest=manifest,work=material.work}))
  end

  dispatch_internal={locate=locate,role_structure=role_structure,role_material=role_material,decimal=decimal,
    sort_copy=sort_copy,selection_issue=selection_issue}
end
