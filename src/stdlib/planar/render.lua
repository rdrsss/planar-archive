-- Private pure assembly over authentic prepared base sections and separately
-- admitted projection records. Native owns response schema/limits/reserved IDs;
-- budget is already validated. No admission, callbacks, reads or input mutation.
local function assemble_sections(ctx,base,custom,budget)
  local sections={}
  for _,entry in ipairs(base) do
    sections[#sections+1]={id=entry.id,text=entry.text,mandatory=entry.mandatory,included=true}
  end
  for _,entry in ipairs(custom) do
    sections[#sections+1]={id=entry.id,text="## "..entry.id.."\n\n"..entry.text,mandatory=entry.mandatory,included=true}
  end
  local omitted={}
  local function notice()
    if #omitted==0 then return nil end
    return {id="omissions",text="## omissions\n\nOptional sections omitted: "..table.concat(omitted,", ")..".\n",mandatory=true,included=true}
  end
  local function assemble()
    local chunks,metadata={},{}
    for _,entry in ipairs(sections) do
      local size=#entry.text
      if entry.included then
        if #chunks>0 then size=size+1 end
        chunks[#chunks+1]=entry.text
      end
      metadata[#metadata+1]=engine.object({id=entry.id,mandatory=entry.mandatory,included=entry.included,bytes=size})
    end
    local extra=notice()
    if extra then
      metadata[#metadata+1]=engine.object({id=extra.id,mandatory=true,included=true,bytes=#extra.text+(#chunks>0 and 1 or 0)})
      chunks[#chunks+1]=extra.text
    end
    return table.concat(chunks,"\n"),engine.array(metadata)
  end
  local markdown,metadata=assemble()
  local function remove(entry)
    if #markdown>budget and not entry.mandatory then
      entry.included=false
      omitted[#omitted+1]=entry.id
      markdown,metadata=assemble()
    end
  end
  for i=#sections,#base+1,-1 do
    if #markdown<=budget then break end
    remove(sections[i])
  end
  for _,id in ipairs({"checkpoint","job_notes","workspace_notes","rationale"}) do
    if #markdown<=budget then break end
    for i=1,#base do if sections[i].id==id then remove(sections[i]) end end
  end
  if #markdown>budget then
    local required={}
    for _,entry in ipairs(metadata) do if entry.included then required[#required+1]=engine.object({id=entry.id,bytes=entry.bytes}) end end
    ctx.issue("","render.budget_bytes","Mandatory Markdown exceeds budget","budget_exceeded",{budget_bytes=budget,mandatory_bytes=#markdown,sections=engine.array(required)})
    return nil
  end
  return {markdown=markdown,sections=metadata,omitted=engine.array(omitted)}
end

-- Internal pure ABI only; these tables must come from native preparation and
-- admission. This export does not authenticate caller-supplied base/custom data.
local function assemble_projection_material(input)
  local ctx=schema_context({},false)
  local result=assemble_sections(ctx,input.base,input.custom,input.budget_bytes)
  if not result then return ctx.finish({}) end
  local metadata={}
  for _,entry in ipairs(result.sections) do
    metadata[#metadata+1]=engine.object({id=entry.id,bytes=entry.bytes,included=entry.included})
  end
  return ctx.finish(engine.object({text=result.markdown,sections=engine.array(metadata),omitted=result.omitted}))
end

-- Shared projection over finalized selection and retained bytes. No fresh reads.
local function role_base_material(ctx,request,selected,model,shape)
  local observations=render_observations(request,selected,model)
  if observations.error then return observations end
  local assertions=observations.value
  local W,C=selected.work.value,selected.context
  local sections={}
  local function section(id,body,mandatory)
    -- Body producers supply one final structural LF; captured trailing bytes stay fenced.
    sections[#sections+1]={id=id,text="## "..id.."\n\n"..body,mandatory=mandatory,included=true}
  end
  local function descriptor(label,value) return presentation_data(label.."; canonical JSON (string controls escaped)",presentation_json(value)) end
  local function text(label,value)
    if value.file then label=label.."; "..value.file.root..":"..value.file.path end
    return presentation_data(label,value.bytes)
  end
  local function absent(body,message) if #body==0 then return message.."\n" end; return table.concat(body) end
  section("authority","Render-only preview. No run is active. Checks have not been executed by this\nrender; no completion or approval is established.\n\nJob: "..C.job.."; role: "..C.role.."; kind: "..C.kind.."; work: "..table.concat(C.work,", ")..".\nWork fingerprint: "..selected.work.digest..". Trusted registry: "..C.registry.id.."/"..C.registry.revision..", driver_selected_trusted_configuration.\n",true)
  local body={text("goal; declaration data",W.goal)}
  for _,unit in ipairs(W.units) do body[#body+1]=text("work:"..unit.id.."; declaration data",unit.summary) end
  section("goal",table.concat(body),true)
  body={}
  for _,milestone in ipairs(W.milestones) do for _,criterion in ipairs(milestone.acceptance) do body[#body+1]=text(milestone.id..":"..criterion.id.."; declaration data",criterion.text) end end
  section("acceptance",absent(body,"No applicable acceptance criteria."),true)
  body={}
  for _,kind in ipairs({"constraints","prescriptions"}) do
    for _,requirement in ipairs(W[kind]) do body[#body+1]=text(requirement.id.."; scope:"..(type(requirement.scope)=="string" and requirement.scope or table.concat(requirement.scope,",")).."; mandatory declaration data",requirement.text) end
  end
  for _,question in ipairs(W.questions) do
    body[#body+1]=text("question:"..question.id.."; prompt; blocks:"..question.blocks.."; declaration data",question.prompt)
    if question.answer then body[#body+1]=text("question:"..question.id.."; answer; declaration data",question.answer)
    else body[#body+1]="Question "..question.id.." is unanswered.\n" end
  end
  if #W.questions==0 then body[#body+1]="Questions: none. Question blockers: none.\n" end
  section("requirements",absent(body,"No applicable requirements or questions."),true)
  body={}
  for _,contract in ipairs(W.contracts) do
    local declaration=contract.declaration
    body[#body+1]=text(declaration.id.."; expected; mandatory declaration data",declaration.expected)
    body[#body+1]=text(declaration.id.."; review_note; declaration assertion, unverified",declaration.review_note)
    body[#body+1]="Producer: "..declaration.producer..". Consumers: "..table.concat(declaration.consumers,", ")..". Conformance check: "..declaration.conformance..". Semantic identity: "..contract.semantic..".\n"
  end
  section("contracts",absent(body,"No applicable contracts."),true)
  body={}
  for _,entry in ipairs(W.ownership.work) do body[#body+1]="work:"..entry.work..": "..table.concat(entry.paths,", ")..(C.kind=="worker" and ". Exclusive worker grant.\n" or ". Review target boundary.\n") end
  body[#body+1]="Frozen source boundaries: "..(#W.ownership.frozen>0 and table.concat(W.ownership.frozen,", ") or "none")..".\nExcluded source boundaries: "..(#W.ownership.excluded>0 and table.concat(W.ownership.excluded,", ") or "none")..".\nAll other source paths are read-only.\n"
  section("boundary",table.concat(body),true)
  local assignments,prerequisites,facets={},{},{}
  for _,entry in ipairs(W.shape.mapping) do assignments[entry.work]=entry.role end
  for _,entry in ipairs(selected.prerequisites) do prerequisites[entry.id]=entry end
  for _,entry in ipairs(selected.facets) do facets[entry.id]=entry end
  local own=semantic_set(C.work)
  body={}
  for _,dep in ipairs(W.dependencies) do
    body[#body+1]="Dependency "..dep.id.."; source: "..dep.source.."; reference: "..dep.ref..".\nProducer: "..dep.producer.." (role "..assignments[dep.producer].."); consumer: "..dep.consumer.." (role "..assignments[dep.consumer]..").\nPhase: "..dep.phase.."; quality: "..dep.quality.."; "..(assignments[dep.producer]==assignments[dep.consumer] and "internal" or "cross-role").." obligation; status: "..prerequisites[dep.id].status..".\n"
  end
  if #W.dependencies==0 then body[#body+1]="Explicit dependencies: none.\n" end
  for _,facet in ipairs(W.facets) do
    local available=facets[facet.id]
    body[#body+1]="Facet "..facet.id.."; producer: "..facet.producer.."; kind: "..facet.kind.."; normalizer: "..facet.normalizer.."; status: "..available.status..".\n"
    body[#body+1]=descriptor("Facet "..facet.id.." paths",facet.paths)
    for _,entry in ipairs(available.entries or {}) do
      body[#body+1]=presentation_data("Facet "..facet.id.." captured file; mode "..entry.mode.."; "..entry.size.." bytes",entry.root..":"..entry.path)
    end
  end
  for _,predecessor in ipairs(W.pipeline_predecessors) do
    body[#body+1]="Pipeline predecessor "..predecessor.id.."; source: "..predecessor.source.."; phase: "..predecessor.phase.."; quality: "..predecessor.quality.."; status: unverified.\nProducer "..predecessor.producer.." (role "..predecessor.producer_role..") must complete before consumer "..predecessor.consumer.." (role "..predecessor.consumer_role..") starts.\n"
  end
  if #W.pipeline_predecessors==0 then body[#body+1]="Pipeline predecessors: none ("..W.shape.kind..").\n" end
  for _,entry in ipairs(selected.prerequisites) do if entry.source=="question" then
    body[#body+1]="Prerequisite "..entry.id.."; source: question; phase: "..entry.phase.."; status: "..entry.status..".\n"
  end end
  body[#body+1]="Render establishes no completion or acceptance; quality obligations remain unverified.\n"
  section("dependencies",table.concat(body),true)
  if shape then
    section("shape_policy",presentation_data("shape guidance; trusted policy",shape.guidance)..presentation_data("shape obligations; canonical JSON",presentation_json(W.shape.obligations)),true)
  end
  body={}
  for _,entry in ipairs(W.effective_checks) do body[#body+1]="work:"..entry.work..": "..table.concat(entry.checks,", ")..".\n" end
  for _,check in ipairs(W.checks) do
    body[#body+1]="Check "..check.id.."; toolchain: "..check.toolchain..".\nTimeout: "..check.timeout_ms.." ms; output cap: "..check.max_output_bytes.." bytes; selection: "..check.selection..".\n"
    body[#body+1]=descriptor("Check "..check.id.." argv; exact argument boundaries",check.argv)
    body[#body+1]=descriptor("Check "..check.id.." inputs",check.inputs)
    body[#body+1]=presentation_data("Check "..check.id.." coverage; trusted driver configuration",check.coverage)
  end
  for _,toolchain in ipairs(W.toolchains) do
    body[#body+1]="Toolchain "..toolchain.id.."; trusted driver configuration.\n"
    body[#body+1]=descriptor("Toolchain "..toolchain.id.." command argv; exact argument boundaries",toolchain.command)
    body[#body+1]=descriptor("Toolchain "..toolchain.id.." environment",toolchain.environment)
    body[#body+1]=presentation_data("Toolchain "..toolchain.id.." description",toolchain.description)
  end
  body[#body+1]="Not executed by render. Commands describe checks; coverage is a driver assertion, unverified. No check receipt exists.\n"
  section("checks",table.concat(body),true)
  body={}
  if selected.observation.status~="captured" then body[1]="Outputs and base not captured; exact review targets unavailable.\n"
  else
    -- The captured W base excludes the driver label; display its declaration separately.
    body[1]="Current checkout and base captured.\n"..presentation_data("Base label; driver declaration data",model.bundle.workspace.root.base)
    local visible={}
    for id in pairs(own) do visible[id]=true end
    if C.kind=="worker" then for _,dep in ipairs(W.dependencies) do visible[dep.producer]=true end end
    for _,output in ipairs(selected.outputs) do if visible[output.work] then
      if C.kind~="reviewer" then body[#body+1]="Unit output "..output.work..": "..output.output..".\n" end
      if not output.present then body[#body+1]="work:"..output.work..": no files captured in this unit; output absent.\n" end
    end end
    if selected.role_output then body[#body+1]="Role output: "..selected.role_output..".\n" end
    if C.kind=="reviewer" then
      body[#body+1]="Exact review target vector; current captured outputs (ordered):\n"
      for _,target in ipairs(selected.targets) do body[#body+1]="work:"..target.work.." → "..target.output.."\n" end
    end
    local paths={}
    for _,unit in ipairs(W.units) do for _,path in ipairs(unit.paths) do paths[#paths+1]=path end end
    if C.kind=="worker" then for _,facet in ipairs(W.facets) do for _,path in ipairs(facet.paths) do paths[#paths+1]=path end end end
    local current,base,all={},{},{}
    for _,file in ipairs(request.files) do
      if file.root=="current:"..W.root or file.root=="base:"..W.root then
        local include=false
        for _,path in ipairs(paths) do if semantic_contains(path,file.path) then include=true end end
        if include then
          local map=file.root=="current:"..W.root and current or base
          map[file.path]=file; all[file.path]=true
        end
      end
    end
    local names={}
    for path in pairs(all) do names[#names+1]=path end
    local function content(file,label)
      local bytes=file.bytes
      local description=file.root..":"..file.path.."; "..label.."; mode "..file.mode.."; "..#bytes.." bytes"
      if not utf8_text(bytes) then return presentation_data(description.."; binary lowercase hex",selection_hex(bytes)) end
      return presentation_data(description.."; complete captured data",bytes)
    end
    local deleted=false
    for _,path in ipairs(sorted(names,function(x)return x end)) do
      local old,new=base[path],current[path]
      if not old then body[#body+1]="Added "..path.."; old absent.\n"; body[#body+1]=content(new,"new")
      elseif not new then deleted=true; body[#body+1]="Deleted "..path.."; new absent.\n"; body[#body+1]=content(old,"old")
      elseif old.mode~=new.mode or old.bytes~=new.bytes then body[#body+1]="Modified "..path..".\n"; body[#body+1]=content(old,"old"); body[#body+1]=content(new,"new")
      else body[#body+1]="Unchanged "..path..".\n"; body[#body+1]=content(new,"current") end
    end
    if #names==0 then body[#body+1]="No selected current or base files.\n" end
    if not deleted then body[#body+1]="No selected deleted paths.\n" end
  end
  section("outputs",table.concat(body),true)
  body={}
  for _,kind in ipairs({"checks","findings"}) do for _,assertion in ipairs(assertions[kind]) do body[#body+1]=descriptor(kind..":"..assertion.id.."; driver assertion, unverified",assertion) end end
  section("observations",absent(body,"No applicable check or finding driver assertions supplied. No check receipt or approval exists."),true)
  if C.optional.preferences then section("preferences",descriptor("Requested preferences; driver preference; not executed",C.optional.preferences),true) end
  for _,id in ipairs({"rationale","workspace_notes","job_notes"}) do if C.optional[id] then section(id,text(id.."; optional immaterial declaration data",C.optional[id]),false) end end
  if assertions.checkpoint then
    local checkout=selected.observation.status=="captured" and "captured" or "not captured"
    section("checkpoint","Session note supplied for this preview; history unavailable; no run is active.\nCurrent checkout: "..checkout..".\n"..descriptor("checkpoint; session note, unverified",assertions.checkpoint),false)
  end
  local budget=request.budget_bytes or C.budget_bytes
  return {value={base=engine.array(sections),budget_bytes=budget,assertions=assertions}}
end

local function render_material(ctx,request,selected,model,shape)
  local prepared=role_base_material(ctx,request,selected,model,shape)
  if prepared.error then return prepared end
  local sections,budget,assertions=prepared.value.base,prepared.value.budget_bytes,prepared.value.assertions
  local W,C=selected.work.value,selected.context
  local assembled=assemble_sections(ctx,sections,{},budget)
  if not assembled then return ctx.finish({}) end
  local markdown,metadata,omitted=assembled.markdown,assembled.sections,assembled.omitted
  local packet=engine.object({version=1,mode="preview",workspace=C.workspace,spec=C.spec,job=C.job,role=C.role,kind=C.kind,work=C.work,markdown=markdown})
  local versions={}
  for _,version in ipairs(W.versions) do versions[#versions+1]=version end
  versions[#versions+1]="projection_v2"
  local rendered_prerequisites=selected.prerequisites
  if shape then
    local merged={}
    for _,entry in ipairs(selected.prerequisites) do merged[#merged+1]=entry end
    for _,obligation in ipairs(W.shape.obligations) do
      local entry={}
      for key,value in pairs(obligation) do entry[key]=value end
      entry.status="unverified"
      merged[#merged+1]=engine.object(entry)
    end
    rendered_prerequisites=sorted(merged,function(entry)return entry.id end)
  end
  local manifest=engine.object({version=1,versions=engine.array(versions),work=selected.work,render=engine.object({digest=engine.fragment_digest(markdown),bytes=#markdown,budget_bytes=budget}),base=selected.base,observation=selected.observation,outputs=selected.outputs,targets=selected.targets,files=selected.files,assertions=assertions,sections=metadata,omitted=omitted,prerequisites=rendered_prerequisites,registry=C.registry})
  if selected.role_output then manifest.role_output=selected.role_output end
  if shape then
    local identity=engine.object({id=shape.id,revision=shape.revision,definition=shape.definition})
    manifest.selected_shape=identity
    return ctx.finish(engine.object({version=1,kind="render_external",packet=packet,manifest=manifest,shape=identity}))
  end
  return ctx.finish(engine.object({version=1,kind="render",packet=packet,manifest=manifest}))
end

local function normalize_render_request(ctx,input)
  return ctx.normalize(input,record({bundle=req(primitive("any")),capture=req(primitive("any")),files=req(primitive("any")),job=req(ID),role=req(ID),budget_bytes=opt(BUDGET)}),"","render")
end

-- Normalize both original schemas before native semantic/F dispatch.
local function prepare_profile_render(input)
  local ctx=schema_context({},false)
  local request=normalize_render_request(ctx,input)
  if #ctx.errors>0 then return ctx.finish({}) end
  local selection=normalize_selection_request(ctx,engine.object({bundle=request.bundle,capture=request.capture,files=request.files,job=request.job,role=request.role}))
  if #ctx.errors>0 then return ctx.finish({}) end
  return ctx.finish(engine.object({request=request,selection=selection}))
end

-- Private trusted role ABI. Native supplies authentic same-call finalized material.
-- Reuse observation validation even though role transport contains no assertion field.
local function role_profile_material(input)
  local checked=render_observations(input.request,input.selected,input.model)
  if checked.error then return checked end
  local selected=input.selected
  local result={}
  for _,key in ipairs({"context","work","base","observation","outputs","targets","files","prerequisites","facets","role_output"}) do
    if selected[key]~=nil then result[key]=selected[key] end
  end
  local guidance=""
  for _,job in ipairs(input.model.jobs) do
    if job.id==input.request.job and job.selected_shape then guidance=job.selected_shape.guidance end
  end
  result.shape_guidance=guidance
  return {value=engine.object(result)}
end

local function render(input)
  local ctx=schema_context({},false)
  local request=normalize_render_request(ctx,input)
  if #ctx.errors>0 then return ctx.finish({}) end
  local selection_input=engine.object({bundle=request.bundle,capture=request.capture,files=request.files,job=request.job,role=request.role})
  local selection=select_role(selection_input)
  if selection.error then return selection end
  local selected=selection.value
  local model=validate_bundle({bundle=request.bundle,capture=request.capture}).value
  return render_material(ctx,request,selected,model)
end

-- Private product ABI: {request,model,selected}. Native composition must supply
-- authentic same-call finalized results. These logical checks do not authenticate
-- caller-supplied tables. No validation, selection, F dispatch or input mutation.
local function profile_material(input,generate)
  local ctx=schema_context({},false)
  local request=normalize_render_request(ctx,input.request)
  if #ctx.errors>0 then return ctx.finish({}) end
  local model,selected=input.model,input.selected
  local function mismatch()
    ctx.issue("","render.selected","Finalized render material does not match request","schema")
    return ctx.finish({})
  end
  -- Semantic generation and selection preserve normalized bundle job order.
  -- Check the complete one-to-one mapping before using any retained job shape.
  if engine.kind(model)~="object" or engine.kind(model.bundle)~="object"
    or engine.kind(model.bundle.jobs)~="array" or engine.kind(model.jobs)~="array"
    or #model.bundle.jobs~=#model.jobs then return mismatch() end
  local seen={}
  local job,job_model
  for i,entry in ipairs(model.bundle.jobs) do
    local semantic=model.jobs[i]
    if engine.kind(entry)~="object" or engine.kind(entry.value)~="object"
      or engine.kind(semantic)~="object" then return mismatch() end
    local id=entry.value.id
    if not id_valid(id) or not id_valid(semantic.id) or semantic.id~=id or seen[id] then return mismatch() end
    seen[id]=true
    if id==request.job then job=entry.value; job_model=semantic end
  end
  local W,C=selected.work.value,selected.context
  local shape=job_model and job_model.selected_shape
  local matches=job and C.job==request.job and W.job==request.job and C.role==request.role and W.role==request.role
    and C.workspace==model.bundle.workspace.id and W.workspace==C.workspace
    and C.spec==model.bundle.spec.id and W.spec==C.spec and W.shape.kind==job.shape
  if shape then
    matches=matches and shape.id==job.shape and W.shape.revision==shape.revision and W.shape.definition==shape.definition
  else
    matches=matches and W.shape.definition==nil and W.shape.revision==nil and W.shape.obligations==nil
  end
  if not matches then return mismatch() end
  return generate(ctx,request,selected,model,shape)
end

local function render_profile_material(input)
  return profile_material(input,render_material)
end

-- Private authentic role projection ABI; preserve the same consistency guards
-- and regenerate observations solely from immutable retained bytes.
local function role_projection_base_material(input)
  local prepared=profile_material(input,role_base_material)
  if prepared.error then return prepared end
  local base={}
  for _,entry in ipairs(prepared.value.base) do
    base[#base+1]=engine.object({id=entry.id,text=entry.text,mandatory=entry.mandatory})
  end
  return {value=engine.object({base=engine.array(base),budget_bytes=prepared.value.budget_bytes})}
end
