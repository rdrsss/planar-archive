-- R17 trusted check planning and completion applicability. Pure over accepted snapshots; native owns all I/O.
-- Embed after dispatch.lua, before review.lua. Integration installs hooks.acceptance and hooks.blockers_to_clear.
local checks=(function()
local function metered_sort(values,less)
 less=less or function(a,b)return a<b end
 local n,width=#values,1
 while width<n do
  local out={}
  for lo=1,n,width*2 do local mid,hi=math.min(lo+width,n+1),math.min(lo+2*width,n+1);local a,b=lo,mid
   for i=lo,hi-1 do if b>=hi or (a<mid and not less(values[b],values[a]))then out[i]=values[a];a=a+1 else out[i]=values[b];b=b+1 end end
  end
  for i=1,n do values[i]=out[i]end;width=width*2
 end
end
local function scalar_text(v)if type(v)=="string"or type(v)=="number"then return ""..v end;return v==nil and "nil" or v==true and "true" or v==false and "false" or "unsupported"end
 local M={hooks={}}
 local function arr(t)return engine and engine.array and engine.array(t or {}) or (t or {}) end
 local function obj(t)return engine and engine.object and engine.object(t) or t end
 local function fp(v)if type(v)=="string" and #v==32 then return "xxh3_128:"..v end;return v end
 local function eq(a,b)return a~=nil and b~=nil and scalar_text(a)==scalar_text(b) end
 local function sorted(t,key)local out={};for _,v in ipairs(t or {})do out[#out+1]=v end;metered_sort(out,function(a,b)return key(a)<key(b)end);return arr(out)end
 local function row(S,role)for _,r in ipairs(S.roles or {})do if r.job==S.active_job and r.role==role then return r end end end
 local function structure(r)return r and type(r.generation)=="table" and r.generation.structure or {} end
 local function lineage(r,g)
  if not r or type(r.generation)~="table" then return false end
  for _,id in ipairs(r.generation.lineage or {r.generation.id})do if eq(id,g)then return true end end
  return false
 end
 local function owner(S,work)for _,r in ipairs(S.roles or {})do if r.job==S.active_job and r.state~="retired" then for _,w in ipairs(structure(r).work or {})do if w==work then return r end end end end end
 local function completion_list(S,role)
  local r=row(S,role);local out={}
  for _,c in ipairs(S.completions or {})do if c.job==S.active_job and c.role==role and lineage(r,c.generation)then out[#out+1]=c end end
  metered_sort(out,function(a,b)return tonumber(a.seq)>tonumber(b.seq)end);return out
 end
 function M.current_completion(S,role)return completion_list(S,role)[1]end
 function M.unit_output(c,work)for _,u in ipairs(c and c.units or {})do if u.work==work then return u.output end end end
 function M.current_work(S,r)
  if M.hooks.current_work then return M.hooks.current_work(S,r)end
  if dispatch_internal and S.revision and S.revision.interpretation then
   local location=dispatch_internal.locate(S.revision.interpretation,S.active_job,r.role)
   if location then
    local states={};for _,v in ipairs(S.roles or {})do if v.job==S.active_job then states[v.role]=v.state end end
    local repair="0";for _,v in ipairs(S.review and S.review.repairs or {})do if v.role==r.role then repair=v.revision end end
    return dispatch_internal.role_material(location,S.publications or {},repair,states)
   end
  end
  if type(r.run)=="table" and r.run.dispatch and not r.drifted then return {work=r.run.dispatch.work,digest=r.run.dispatch.work_fp}end
 end
 local function observed(S,work)
  for _,v in ipairs(S.observed or {})do if v.work==work then return v.output end end
  local latest
  for _,c in ipairs(S.completions or {})do local out=M.unit_output(c,work);if out and (not latest or tonumber(c.seq)>tonumber(latest.seq))then latest={seq=c.seq,output=out}end end
  return latest and latest.output
 end
 function M.completion_mismatches(S,c)
  local out={}
  for _,u in ipairs(c.units or {})do local o=owner(S,u.work);if o and o.role==c.role then local current=observed(S,u.work);if current~=u.output then out[u.work]={work=u.work,recorded=u.output,current=current or "none"}end end end
  for _,v in ipairs(c.vectors or {})do for _,t in ipairs(v.targets or {})do local o=owner(S,t.work);if o and o.role~=c.role then local current=M.unit_output(M.current_completion(S,o.role),t.work);if current~=t.output then out[t.work]={work=t.work,recorded=t.output,current=current or "none"}end end end end
  local values={};for _,v in pairs(out)do values[#values+1]=obj(v)end;return sorted(values,function(v)return v.work end)
 end
 function M.applicable_completion(S,role)
  local r=row(S,role);if not r then return nil end
  local current=M.current_work(S,r);if not current then return nil end
  for _,c in ipairs(completion_list(S,role))do if fp(c.work_fp)==fp(current.digest) and #M.completion_mismatches(S,c)==0 then return c end end
 end
 function M.plan(S,role)
  local r=row(S,role);if not r or type(r.run)~="table" then return nil,"fenced"end
  if structure(r).kind=="reviewer"then return nil,"reference"end
  local current=M.current_work(S,r);if not current then return nil,"internal"end
  local W=current.work;local owned,units,selected={},{},{}
  for _,u in ipairs(W.units or {})do selected[u.id]=true;units[#units+1]=obj({work=u.id,paths=u.paths});for _,p in ipairs(u.paths)do owned[#owned+1]=p end end
  local definitions,tools={},{};for _,c in ipairs(W.checks or {})do definitions[c.id]=c end;for _,t in ipairs(W.toolchains or {})do tools[t.id]=t end
  local producers,producer_ids={},{}
  for _,d in ipairs(W.dependencies or {})do if d.phase=="complete" or d.source=="facet"then if not selected[d.producer]then producer_ids[d.producer]=true end end end
  local location=dispatch_internal and dispatch_internal.locate(S.revision.interpretation,S.active_job,role)
  for _,u in ipairs(location and location.spec.work or {})do if producer_ids[u.id]then local own=owner(S,u.id);producers[#producers+1]=obj({work=u.id,paths=u.paths,completion_output=(own and M.unit_output(M.current_completion(S,own.role),u.id))or "none"})end end
  local plan_checks,contracts,need_contracts={},{},false
  for _,e in ipairs(W.effective_checks or {})do for _,id in ipairs(e.checks)do local c=definitions[id];local t=c and tools[c.toolchain];if not t then return nil,"internal"end
   local env={};for _,v in ipairs(t.environment or {})do env[v.name]=v.value end
   local vector={e.work};for _,input in ipairs(c.inputs or {})do if input=="output_vector"then for id in pairs(producer_ids)do vector[#vector+1]=id end elseif input=="expected_contracts"then need_contracts=true end end
   metered_sort(vector)
   plan_checks[#plan_checks+1]=obj({work=e.work,check=c.id,toolchain=c.toolchain,command=t.command,argv=c.argv,environment=obj(env),timeout_ms=c.timeout_ms,max_output_bytes=c.max_output_bytes,selection=c.selection,inputs=c.inputs,coverage=c.coverage,vector=arr(vector)})
  end end
  if need_contracts then for _,entry in ipairs(W.contracts or {})do
   local c=entry.declaration or entry;local expected=c.expected;local path,bytes
   if type(expected)=="table" then path=expected.path or (type(expected.file)=="table" and expected.file.path or expected.file);bytes=expected.text or expected.bytes or expected.content end
   -- Normalized interpretations retain file provenance in the expected value; inline text deliberately has no path.
   if path and not bytes and location then for _,file in ipairs(location.bundle.files or {})do if file.path==path then bytes=file.bytes end end end
   contracts[#contracts+1]=obj({id=c.id,work=c.producer,path=path or "",bytes=bytes or (type(expected)=="string" and expected or ""),bytes_digest=entry.semantic or ""})
  end end
  local facets={};for _,f in ipairs(location and location.spec.facets or W.facets or {})do if selected[f.producer or f.work]then facets[#facets+1]=obj({work=f.producer or f.work,facet=f.id,paths=f.paths})end end
  local registry=location and location.registry or {}
  return obj({version=1,job=S.active_job,role=role,run=scalar_text(r.run.id),generation=scalar_text(r.generation.id),root=W.root,base=W.base.label,registry=obj({id=registry.id or "",revision=registry.revision or ""}),units=sorted(units,function(v)return v.work end),checks=sorted(plan_checks,function(v)return v.work.."\0"..v.check end),contracts=sorted(contracts,function(v)return v.id end),facets=sorted(facets,function(v)return v.work.."\0"..v.facet end),owned=sorted(owned,function(v)return v end),frozen=W.ownership.frozen,excluded=W.ownership.excluded,producers=sorted(producers,function(v)return v.work end)})
 end
 local function add(set,key)set[key]=true end
 function M.evaluate(S,plan,result,options)
  options=options or {};local r=row(S,plan.role)
  if not r or type(r.run)~="table" or not eq(r.run.id,plan.run)then return nil,"fenced"end
  local withheld={}
  for _,e in ipairs(result.evidence or {})do if e.outcome~="pass"then add(withheld,e.reason=="input_changed" and "input_changed" or e.check=="planar.boundary_v1" and "boundary" or "check_failed")end end
  if result.recapture=="differs" or result.recapture_matches==false then add(withheld,"input_changed")end
  if r.drifted then add(withheld,"drifted")end
  local material=M.current_work(S,r)
  for _,p in ipairs(material and material.prerequisites or {})do if p.status~="available" and p.status~="answered"then add(withheld,p.phase=="start" and "start_prerequisite" or "complete_prerequisite")end end
  for _,d in ipairs(material and material.work.dependencies or {})do if d.phase=="complete" and (d.quality=="done" or d.quality=="accepted")then
   local o=owner(S,d.producer);local ok=o and (o.role==plan.role or M.applicable_completion(S,o.role)~=nil)
   if d.quality=="accepted"then ok=false;if M.hooks.acceptance then local accepted=M.hooks.acceptance(S);for _,u in ipairs(accepted.units or accepted or {})do if u.work==d.producer and u.accepted then ok=true end end end end
   if not ok then add(withheld,"complete_prerequisite")end
  end end
  if options.retry_exhausted then add(withheld,"retry_exhausted")end
  local reasons={};for reason in pairs(withheld)do reasons[#reasons+1]=reason end;metered_sort(reasons)
  local transitions,vector={},{}
  if #reasons==0 then transitions[1]=obj({job=plan.job,role=plan.role,before_state="running",before_drifted=false,after_state="done",after_drifted=false,reason="completed"})end
  for _,v in ipairs(S.roles or {})do if v.job==plan.job then vector[#vector+1]=obj({job=v.job,role=v.role,state=v.role==plan.role and #reasons==0 and "done" or v.state,drifted=v.drifted==true})end end
  local facts={reservation=result.reservation or options.reservation,evidence=result.evidence,observation=result.observation,blockers_cleared=arr({})}
  if #reasons==0 then facts.completion=obj({publications=result.facets or result.publications or arr({})})end
  if M.hooks.blockers_to_clear then facts.blockers_cleared=M.hooks.blockers_to_clear(S,{operation="check",job=plan.job})end
  local change=obj({version=1,operation="check",facts=obj(facts),vector=sorted(vector,function(v)return v.role end),transitions=arr(transitions),expected=arr({}),withheld=arr(reasons)})
  change=M.rederive(S,change,plan,result)
  if M.hooks.reconcile then change=M.hooks.reconcile(S,change,plan,result)end
  return change
 end
 -- Derive downstream roles against a hypothetical committed snapshot; never mutate the input snapshot.
 function M.rederive(S,change,plan,result)
  local next={};for k,v in pairs(S)do next[k]=v end
  next.roles={};local original,explicit={},{}
  for _,r in ipairs(S.roles or {})do local c={};for k,v in pairs(r)do c[k]=v end;next.roles[#next.roles+1]=c;original[r.role]=r end
  for _,t in ipairs(change.transitions or {})do explicit[t.role]=t;for _,r in ipairs(next.roles)do if r.job==t.job and r.role==t.role then r.state=t.after_state;r.drifted=t.after_drifted;if r.state~="running"then r.run="none"end end end end
  if plan and result and change.facts.completion then
   local r=original[plan.role];local current=M.current_work(S,r);local sequence=tonumber(S.last_seq or "0");for _,c in ipairs(S.completions or {})do sequence=math.max(sequence,tonumber(c.seq))end;sequence=sequence+1;local units=result.observation and result.observation.units or {}
   local vectors={};for _,e in ipairs(result.evidence or {})do if e.vector then vectors[#vectors+1]={work=e.work,targets=e.vector.targets}end end
   next.completions={};for _,c in ipairs(S.completions or {})do next.completions[#next.completions+1]=c end
   next.completions[#next.completions+1]={id="0",job=plan.job,role=plan.role,generation=r.generation.id,run=plan.run,dispatch=r.run.dispatch.id,work_fp=current.digest,seq=scalar_text(sequence),units=units,vectors=vectors}
   local own={};for _,u in ipairs(plan.units)do own[u.work]=true end
   next.observed={};for _,o in ipairs(S.observed or {})do if not own[o.work]then next.observed[#next.observed+1]=o end end
   for _,u in ipairs(units)do if own[u.work]then next.observed[#next.observed+1]={work=u.work,output=u.output,seq=scalar_text(sequence)}end end
   next.publications={};local new={};for _,f in ipairs(result.facets or result.publications or {})do new[f.facet]=true;next.publications[#next.publications+1]={job=plan.job,role=plan.role,work=f.work,facet=f.facet,semantic=f.semantic}end
   for _,f in ipairs(S.publications or {})do if not new[f.facet]then next.publications[#next.publications+1]=f end end
  end
  local derived={}
  for _=1,#next.roles+1 do
   local changed=false
   for _,r in ipairs(next.roles)do if r.job==S.active_job and r.state~="retired" and not explicit[r.role]then
    local before=original[r.role];local material=M.current_work(next,r)
    if material then
     local state,drift=r.state,r.drifted==true;local reason
     if r.state=="running"then
      local acknowledged=type(r.run)=="table" and r.run.dispatch and r.run.dispatch.work_fp
      drift=fp(acknowledged)~=fp(material.digest)
      if drift~=r.drifted then reason=drift and "work_changed" or "drift_cleared"end
     elseif material.blocked then state="blocked";reason="prerequisite_unavailable"
     elseif structure(r).kind=="worker"then
      local applicable=M.applicable_completion(next,r.role)
      if applicable then state="done";reason=r.state=="blocked" and "prerequisite_available" or "evidence_inapplicable"
      elseif r.state=="done"then state="stale";reason="evidence_inapplicable"
      elseif r.state=="blocked"then state="ready";reason="prerequisite_available"end
     elseif r.state=="blocked"then state="ready";reason="prerequisite_available"end
     if state~=r.state or drift~=(r.drifted==true)then
      r.state=state;r.drifted=drift;changed=true
      derived[r.role]=obj({job=r.job,role=r.role,before_state=before.state,before_drifted=before.drifted==true,after_state=state,after_drifted=drift,reason=reason})
     end
    end
   end end
   if not changed then break end
  end
  local tail={};for _,t in pairs(derived)do if t.before_state~=t.after_state or t.before_drifted~=t.after_drifted then tail[#tail+1]=t end end
  for _,t in ipairs(sorted(tail,function(t)return t.role end))do change.transitions[#change.transitions+1]=t end
  local vector={};for _,r in ipairs(next.roles)do if r.job==S.active_job then vector[#vector+1]=obj({job=r.job,role=r.role,state=r.state,drifted=r.drifted==true,run=type(r.run)=="table" and r.run.id or "none"})end end
  change.vector=sorted(vector,function(r)return r.role end)
  return change
 end
 function M.evidence_lines(S,role)
  local own={};for _,w in ipairs(structure(row(S,role)).work or {})do own[w]=true end
  local lines={"Check evidence (recorded, latest per work and check):"}
  for _,e in ipairs(sorted(S.evidence or {},function(e)return e.work.."\0"..e.check end))do if own[e.work]then lines[#lines+1]="- "..e.work.." "..e.check..": "..e.outcome..(e.outcome=="pass" and "" or " ("..e.reason..")")end end
  return lines
 end
 return M
end)()
