-- Trusted reconcile pass for format3 load and activate (execution contract EX-3, EX-4, EX-5). Input is the
-- read_execution snapshot value plus the operation; output is one execution_change_v1 value. Pure: native code
-- validates the change, allocates every identifier and enforces every precondition under the lock.
local reconcile_execution
do
  local locate,role_structure,role_material=dispatch_internal.locate,dispatch_internal.role_structure,dispatch_internal.role_material
  local decimal,sort_copy=dispatch_internal.decimal,dispatch_internal.sort_copy

  -- Decimal strings sort numerically after left padding to the 19 digits of a positive int64.
  local function numeric_key(text)
    local pad={}
    for _=1,19-#text do pad[#pad+1]="0" end
    return table.concat(pad)..text
  end
  local function run_id(row)
    if engine.kind(row.run)=="object" then return decimal(row.run.id) end
    return nil
  end
  local function row_key(job,role) return job.."\0"..role end

  -- Project captured evidence into a hypothetical post-operation owning lineage.
  -- Persistent history remains in the ledger; only current facts feed readiness.
  local function current_facts(view,prior)
    local owners,old_owners={},{}
    local function collect(snapshot,out)
      for _,r in ipairs(snapshot.roles or {})do
        if r.job==snapshot.active_job and r.state~="retired" and r.generation.structure.kind=="worker"then
          for _,w in ipairs(r.generation.structure.work or {})do out[w]=r end
        end
      end
    end
    collect(view,owners);collect(prior,old_owners)
    local function contains(r,g)
      for _,id in ipairs(r.generation.lineage or {r.generation.id})do if id==g then return true end end
      return false
    end
    local completions,latest={},{}
    for _,c in ipairs(prior.completions or {})do
      local valid=false
      for _,u in ipairs(c.units or {})do local r=owners[u.work]
        if r and r.job==c.job and r.role==c.role and contains(r,c.generation)then valid=true end
      end
      if valid then
        completions[#completions+1]=c
        local key=row_key(c.job,c.role)
        if not latest[key] or tonumber(c.seq)>tonumber(latest[key].seq)then latest[key]=c end
      end
    end
    view.completions=engine.array(completions)
    local publications={}
    for _,p in ipairs(prior.publications or {})do local r=owners[p.work]
      local c=r and latest[row_key(r.job,r.role)]
      if c and p.job==r.job and p.role==r.role and p.completion==c.id then publications[#publications+1]=p end
    end
    view.publications=engine.array(publications)
    local function carried(work)
      local r,old=owners[work],old_owners[work]
      return r and old and r.job==old.job and r.role==old.role and contains(r,old.generation.id)
    end
    for _,name in ipairs({"observed","evidence"})do
      local kept={};for _,v in ipairs(prior[name] or {})do if carried(v.work)then kept[#kept+1]=v end end
      view[name]=engine.array(kept)
    end
    return view
  end

  local function transition(job,role,row,after_state,after_drifted,reason)
    local t={job=job,role=role,after_state=after_state,after_drifted=after_drifted,reason=reason}
    -- NULL-first: a first transition of a (job, role) carries no before keys (EX-4 Derivation, D19).
    if row then t.before_state=row.state; t.before_drifted=row.drifted==true end
    return engine.object(t)
  end

  local function check_snapshot(ctx,input)
    local function bad(field) ctx.issue("","reconcile."..field,"Invalid reconcile request","schema") end
    if engine.kind(input)~="object" then bad("request"); return nil end
    if input.operation~="load" and input.operation~="activate" then bad("operation") end
    local outer=input.snapshot
    if engine.kind(outer)~="object" or engine.kind(outer.snapshot)~="object" then bad("snapshot"); return nil end
    local S=outer.snapshot
    if type(decimal(outer.last_seq))~="string" then bad("snapshot.last_seq") end
    if type(decimal(S.active_revision))~="string" then bad("snapshot.active_revision") end
    if type(S.active_job)~="string" then bad("snapshot.active_job") end
    if engine.kind(S.roles)~="array" then bad("snapshot.roles") end
    if input.operation=="load" and engine.kind(input.candidate)~="object" then bad("candidate") end
    if input.operation=="activate" and not id_valid(input.job) then bad("job") end
    if input.request_id~=nil and type(input.request_id)~="string" then bad("request_id") end
    if #ctx.errors>0 then return nil end
    return S,decimal(outer.last_seq)
  end

  -- Fixed export: {operation:"load",snapshot,candidate:{bundle,semantic},request_id?} or
  -- {operation:"activate",snapshot,job,request_id?} -> execution_change_v1 without packet.
  reconcile_execution=function(input)
    local ctx=schema_context({},false)
    local S,last_seq=check_snapshot(ctx,input)
    if not S then return ctx.finish({}) end
    local operation=input.operation
    local active_revision,active_job=decimal(S.active_revision),S.active_job
    local recorded=engine.kind(S.revision)=="object" and S.revision.interpretation or nil
    local rows,rows_by_job={}, {}
    for _,row in ipairs(S.roles) do
      rows[row_key(row.job,row.role)]=row
      rows_by_job[row.job]=rows_by_job[row.job] or {}
      rows_by_job[row.job][#rows_by_job[row.job]+1]=row
    end
    local function states_of(job)
      local states={}
      for _,row in ipairs(rows_by_job[job] or {}) do states[row.role]=row.state end
      return states
    end
    local publications=S.publications or {}
    local view={};for k,v in pairs(S)do view[k]=v end
    local transitions,generations,fenced,after={}, {}, {}, {}
    local function fence(row)
      local id=row.state=="running" and run_id(row) or nil
      if id then fenced[#fenced+1]=id end
    end
    local function record(job,role,row,state,drifted,reason)
      transitions[#transitions+1]=transition(job,role,row,state,drifted,reason)
    end
    local function open(job,role,reason,structure)
      generations[#generations+1]=engine.object({job=job,role=role,reason=reason,structure=structure})
    end
    -- Retire every non-retired row of a job (M5/M6 on activation, M13/M14 on load).
    local function retire_all(job,keep)
      for _,row in ipairs(rows_by_job[job] or {}) do
        if not (keep and keep[row.role]) and row.state~="retired" then
          record(job,row.role,row,"retired",false,row.state=="running" and "fenced" or "retired")
          fence(row)
          after[row_key(job,row.role)]={job=job,role=row.role,state="retired",drifted=false}
        end
      end
    end
    local function fresh(material) return material.blocked and "blocked" or "ready" end

    local after_job=active_job
    local roles_of_interest={} -- job -> set of role IDs whose expected members are asserted
    local function interest(job,role)
      roles_of_interest[job]=roles_of_interest[job] or {}
      roles_of_interest[job][role]=true
    end
    for _,row in ipairs(rows_by_job[active_job] or {}) do interest(active_job,row.role) end
    if active_job~="" and recorded then
      local location=locate(recorded,active_job)
      if location then for _,r in ipairs(location.job.roles) do interest(active_job,r.id) end end
    end

    if operation=="activate" then
      local job_id=input.job
      local location=recorded and locate(recorded,job_id)
      if active_revision=="none" or not location then
        ctx.issue("","selection.job","Unknown selected job","reference")
        return ctx.finish({})
      end
      for _,row in ipairs(rows_by_job[job_id] or {}) do interest(job_id,row.role) end
      for _,r in ipairs(location.job.roles) do interest(job_id,r.id) end
      if active_job~="" and active_job~=job_id then retire_all(active_job) end
      view.active_job=job_id
      view.roles={}
      for _,r in ipairs(location.job.roles)do
        view.roles[#view.roles+1]={job=job_id,role=r.id,state="ready",drifted=false,run="none",
          generation={structure=role_structure(locate(recorded,job_id,r.id)),lineage={}}}
      end
      current_facts(view,S)
      publications=view.publications
      view.review={repairs={}}
      local states={} -- every generation of the job starts empty: no producer completion applies
      for _,r in ipairs(location.job.roles) do
        local role_location=locate(recorded,job_id,r.id)
        local structure=role_structure(role_location)
        local material=role_material(role_location,publications,"0",states)
        local row=rows[row_key(job_id,r.id)]
        local state=fresh(material)
        open(job_id,r.id,"activated",structure)
        if row then fence(row) end
        record(job_id,r.id,row,state,false,"activated")
        after[row_key(job_id,r.id)]={job=job_id,role=r.id,state=state,drifted=false}
        for _,v in ipairs(view.roles)do if v.role==r.id then v.state=state end end
      end
      after_job=job_id
    else
      local candidate=input.candidate
      view.revision={interpretation=candidate}
      -- Structural replacement closes old completion lineage before qualities are derived.
      view.roles={}
      local reset={}
      for _,old in ipairs(S.roles)do
        local copy={};for k,v in pairs(old)do copy[k]=v end
        if old.job==active_job then
          local next_location=locate(candidate,active_job,old.role)
          if next_location then
            local next_structure,next_digest=role_structure(next_location)
            if old.state=="retired" or engine.hash_value("role_structure_v1",old.generation.structure)~=next_digest then
              copy.generation={id=old.generation.id,structure=next_structure,lineage={}}
              copy.state="ready";copy.run="none";reset[old.role]=true
            end
          else copy.state="retired" end
        end
        view.roles[#view.roles+1]=copy
      end
      local next_job=locate(candidate,active_job)
      if next_job then
        for _,r in ipairs(next_job.job.roles)do
          local found=false;for _,v in ipairs(view.roles)do if v.job==active_job and v.role==r.id then found=true end end
          if not found then view.roles[#view.roles+1]={job=active_job,role=r.id,state="ready",drifted=false,run="none",
            generation={structure=role_structure(locate(candidate,active_job,r.id)),lineage={}}}end
        end
      end
      current_facts(view,S)
      publications=view.publications
      if S.review then
        view.review={};for k,v in pairs(S.review)do view.review[k]=v end
        view.review.repairs={};for _,repair in ipairs(S.review.repairs or {})do if not reset[repair.role]then view.review.repairs[#view.review.repairs+1]=repair end end
      end
      local function policy_material(role,fallback)
        return dispatch_internal.material and dispatch_internal.material(view,role) or fallback
      end
      if active_job~="" then
        local location=locate(candidate,active_job)
        if not location then
          -- D94: the active job is absent from the new revision; retire its roles and clear the active job.
          retire_all(active_job)
          after_job=""
        else
          local states=states_of(active_job)
          local declared={}
          -- Review readiness consumes the committed worker states, including restored
          -- completions and active-run precedence, independently of declaration order.
          local ordered={}
          for _,kind in ipairs({"worker","reviewer"})do
            for _,r in ipairs(location.job.roles)do if r.kind==kind then ordered[#ordered+1]=r end end
          end
          for _,r in ipairs(ordered) do
            declared[r.id]=true
            interest(active_job,r.id)
            local role_location=locate(candidate,active_job,r.id)
            local structure,digest=role_structure(role_location)
            local material=policy_material(r.id,role_material(role_location,publications,"0",states))
            local key=row_key(active_job,r.id)
            local row=rows[key]
            if not row then
              -- A role added to the active job: first-ever row, new empty generation (NULL before state); the
              -- role table has no load row with a NULL before state, so the draft uses structural_change.
              local state=fresh(material)
              open(active_job,r.id,"structural_change",structure)
              record(active_job,r.id,nil,state,false,"structural_change")
              after[key]={job=active_job,role=r.id,state=state,drifted=false}
            elseif row.state=="retired" then
              local state=fresh(material)
              open(active_job,r.id,"recreated",structure)
              record(active_job,r.id,row,state,false,"recreated")
              after[key]={job=active_job,role=r.id,state=state,drifted=false}
            elseif engine.hash_value("role_structure_v1",row.generation.structure)~=digest then
              local state=fresh(material)
              open(active_job,r.id,"structural_change",structure)
              fence(row)
              record(active_job,r.id,row,state,false,"structural_change")
              after[key]={job=active_job,role=r.id,state=state,drifted=false}
            elseif row.state=="running" then
              local acknowledged=engine.kind(row.run)=="object" and row.run.dispatch and row.run.dispatch.work
              local drifted=acknowledged~=nil and acknowledged~=false and engine.hash_value("work_exec_v1",acknowledged)~=material.digest
              if drifted~=(row.drifted==true) then
                record(active_job,r.id,row,"running",drifted,drifted and "work_changed" or "drift_cleared")
              end
              after[key]={job=active_job,role=r.id,state="running",drifted=drifted,run=run_id(row)}
            elseif material.blocked then
              if row.state~="blocked" then record(active_job,r.id,row,"blocked",false,"prerequisite_unavailable") end
              after[key]={job=active_job,role=r.id,state="blocked",drifted=false}
            elseif dispatch_internal.applicable and dispatch_internal.applicable(view,r.id) then
              if row.state~="done" then record(active_job,r.id,row,"done",false,"prerequisite_available") end
              after[key]={job=active_job,role=r.id,state="done",drifted=false}
            elseif dispatch_internal.applicable and dispatch_internal.applicable(view,r.id,true) then
              if row.state~="stale" then record(active_job,r.id,row,"stale",false,row.state=="blocked" and "prerequisite_available" or "work_changed") end
              after[key]={job=active_job,role=r.id,state="stale",drifted=false}
            elseif row.state=="blocked" then
              record(active_job,r.id,row,"ready",false,"prerequisite_available")
              after[key]={job=active_job,role=r.id,state="ready",drifted=false}
            else
              after[key]={job=active_job,role=r.id,state=row.state,drifted=false}
            end
            local projected=after[key]
            states[r.id]=projected.state
            local found=false
            for _,v in ipairs(view.roles)do if v.job==active_job and v.role==r.id then
              v.state=projected.state;v.drifted=projected.drifted
              if projected.state~="running"then v.run="none"end
              found=true
            end end
            if not found then view.roles[#view.roles+1]={job=active_job,role=r.id,state=projected.state,
              drifted=projected.drifted,run="none",generation={structure=structure,lineage={}}}end
          end
          -- Roles removed from the active job retire (M13/M14 cause (a)).
          retire_all(active_job,declared)
        end
      end
    end

    -- Complete next vector of the active job after the commit: every role_state row it will have.
    local vector={}
    if after_job~="" then
      local seen={}
      for _,entry in pairs(after) do
        if entry.job==after_job then
          seen[entry.role]=true
          vector[#vector+1]=engine.object({job=after_job,role=entry.role,state=entry.state,drifted=entry.drifted,run=entry.run or "none"})
        end
      end
      for _,row in ipairs(rows_by_job[after_job] or {}) do
        if not seen[row.role] then
          vector[#vector+1]=engine.object({job=after_job,role=row.role,state=row.state,drifted=row.drifted==true,run=run_id(row) or "none"})
        end
      end
    end

    -- Expected set exactly per EX-3 "Format3 load and activate preconditions" (L20: one no-row member per role
    -- without a row). Ordered: request_unused, active_revision, active_job, then per (job, role) fact_version and
    -- role_state.
    local expected={}
    local unused=engine.object({kind="request_unused"})
    if input.request_id~=nil then unused.request_id=input.request_id end
    expected[1]=unused
    expected[2]=engine.object({kind="active_revision",revision=active_revision})
    expected[3]=engine.object({kind="active_job",job=active_job})
    local members={}
    for job,set in pairs(roles_of_interest) do
      for role in pairs(set) do members[#members+1]={job=job,role=role} end
    end
    for _,member in ipairs(sort_copy(members,function(m)return row_key(m.job,m.role) end)) do
      local row=rows[row_key(member.job,member.role)]
      if row then
        expected[#expected+1]=engine.object({kind="fact_version",job=member.job,role=member.role,fact_version=decimal(row.fact_version)})
        expected[#expected+1]=engine.object({kind="role_state",job=member.job,role=member.role,state=row.state,
          drifted=row.drifted==true,run=run_id(row) or "none"})
      elseif member.job==active_job then
        -- EX-3: a role of the active job without a row asserts that no row exists; roles of the job being activated
        -- assert members only when they already have a row (draft decision DD29).
        expected[#expected+1]=engine.object({kind="role_state",job=member.job,role=member.role,no_row=true})
      end
    end

    local function by_role(items) return sort_copy(items,function(t)return row_key(t.job,t.role) end) end
    local fenced_runs={}
    for _,id in ipairs(sort_copy(fenced,numeric_key)) do fenced_runs[#fenced_runs+1]=id end
    -- Draft: both operations carry {job,generations,fenced_runs}; job is the active job after the commit.
    local facts=engine.object({job=after_job,generations=by_role(generations),fenced_runs=engine.array(fenced_runs)})
    if dispatch_internal.clear_blockers then facts.blockers_cleared=dispatch_internal.clear_blockers(S,input,after_job,view) end
    return ctx.finish(engine.object({version=1,operation=operation,snapshot_seq=last_seq,facts=facts,
      vector=by_role(vector),transitions=by_role(transitions),expected=engine.array(expected)}))
  end
end
