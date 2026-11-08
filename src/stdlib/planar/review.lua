-- R18 review and repair over one execution_snapshot_v1 (execution-contract-r18.md): verdict validation (R18-2 steps 5
-- to 7), the current target vector and policy identity (R18-1 items 3 and 4), applicability and admission (R18-3,
-- R18-5), blocker clearing, derived acceptance (R18-6), carried findings and reviewer packet lines (R18-1 item 5), the
-- status_v1.review value and the execution_change_v1 of operation verdict. Pure Lua over the snapshot: no I/O.
-- R17 applicability comes from the `checks` local of checks.lua (current_completion, applicable_completion,
-- unit_output). Optional hooks: review.hooks.current_work(snapshot, role_entry) -> {work, digest} supplies a role's
-- current work_exec_v1 W (R15-C reconcile); review.hooks.reconcile(snapshot, change) -> change adds the M17 to M22
-- prerequisite rederivation (identity when absent, DB-R4). Not embedded and not registered yet (DB-R4).
local review=(function()
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
  local prefix="xxh3_128:"
  local reviewer_authority="reviewer_role_v1"

  -- Scalars and grammars ------------------------------------------------------------------------------------------
  local function num(v)
    if type(v)=="number" then return v end
    if type(v)=="string" then return tonumber(v) end
    return nil
  end
  local function dec(v)
    local n=num(v)
    if n==nil then return scalar_text(v) end
    return ""..n
  end
  local function same_id(a,b)
    if a==nil or b==nil then return false end
    return dec(a)==dec(b)
  end
  local function is_integer(v)
    if type(v)~="number" then return false end
    if math and math.type then return math.type(v)=="integer" end
    return v==v and v%1==0
  end
  local function digits(s,first,hex)
    if type(s)~="string" or #s<(first or 1) then return false end
    for i=first or 1,#s do local c=string.byte(s,i);if not(c>=48 and c<=57) and not(hex and c>=97 and c<=102)then return false end end;return true
  end
  local function portable(s,colon)
    for i=1,#s do local c=string.byte(s,i);if not((c>=65 and c<=90)or(c>=97 and c<=122)or(c>=48 and c<=57)or c==95 or c==46 or c==45 or(colon and c==58))then return false end end;return true
  end
  local function identifier(s)
    if type(s)~="string"or #s<1 or #s>64 then return false end
    for i=1,#s do local c=string.byte(s,i);if not(c>=97 and c<=122)and not(i>1 and ((c>=48 and c<=57)or c==95))then return false end end;return true
  end
  local function finding_identifier(s)return type(s)=="string"and #s>=1 and #s<=128 and portable(s,true)end
  local function digest_text(s)return type(s)=="string"and #s==41 and string.sub(s,1,9)==prefix and digits(s,10,true)end
  local function fp(s)
    if type(s)=="string" and string.sub(s,1,9)~=prefix then return prefix..s end
    return s
  end
  -- UTF-8 without NUL, as the native valid_text.
  local function valid_text(s)
    if type(s)~="string" then return false end
    local i,n=1,#s
    while i<=n do
      local b=string.byte(s,i)
      local extra,low,high
      if b==0 then return false
      elseif b<0x80 then extra=0
      elseif b>=0xC2 and b<=0xDF then extra=1;low=0x80;high=0xBF
      elseif b==0xE0 then extra=2;low=0xA0;high=0xBF
      elseif (b>=0xE1 and b<=0xEC) or b==0xEE or b==0xEF then extra=2;low=0x80;high=0xBF
      elseif b==0xED then extra=2;low=0x80;high=0x9F
      elseif b==0xF0 then extra=3;low=0x90;high=0xBF
      elseif b>=0xF1 and b<=0xF3 then extra=3;low=0x80;high=0xBF
      elseif b==0xF4 then extra=3;low=0x80;high=0x8F
      else return false end
      for k=1,extra do
        local c=string.byte(s,i+k)
        if c==nil then return false end
        if k==1 then
          if c<low or c>high then return false end
        elseif c<0x80 or c>0xBF then return false end
      end
      i=i+extra+1
    end
    return true
  end
  -- Portable source path: declaration segment rules with 1 to 4096 bytes (DB-R14, as the native codec).
  local function source_path(p)
    if type(p)~="string" or #p==0 or #p>4096 or string.sub(p,-1)=="/" then return false end
    local begin=1
    for i=1,#p+1 do if i>#p or string.byte(p,i)==47 then
      local part=string.sub(p,begin,i-1);begin=i+1
      if #part==0 or #part>128 or part=="."or part==".."or string.lower(part)==".git"or not portable(part,false)then return false end
    end end
    return true
  end
  local function is_list(t)
    if type(t)~="table" then return false end
    local n,count=#t,0
    for _ in pairs(t) do count=count+1 end
    return count==n
  end
  local function is_object(t)
    if type(t)~="table" then return false end
    for k in pairs(t) do if type(k)~="string" then return false end end
    return true
  end
  local function sorted_copy(list,less)
    local out={}
    for i,v in ipairs(list or {}) do out[i]=v end
    metered_sort(out,less)
    return out
  end
  local function set_of(list)
    local out={}
    for _,v in ipairs(list or {}) do out[v]=true end
    return out
  end

  -- Snapshot helpers ----------------------------------------------------------------------------------------------
  local function roles_of(snapshot,job)
    local out={}
    for _,r in ipairs(snapshot.roles or {}) do if r.job==job then out[#out+1]=r end end
    return out
  end
  local function structure(r)
    return (type(r.generation)=="table" and r.generation.structure) or {}
  end
  local function role_kind(r)
    return r.kind or structure(r).kind
  end
  local function role_entry(snapshot,job,role)
    for _,r in ipairs(snapshot.roles or {}) do if r.job==job and r.role==role then return r end end
    return nil
  end
  -- The reviewer of a job: the first non-retired reviewer role in snapshot order (one reviewer per job shape).
  local function reviewer_of(snapshot,job)
    for _,r in ipairs(roles_of(snapshot,job)) do
      if role_kind(r)=="reviewer" and r.state~="retired" then return r end
    end
    return nil
  end
  local function workers_of(snapshot,job)
    local out={}
    for _,r in ipairs(roles_of(snapshot,job)) do
      if role_kind(r)=="worker" and r.state~="retired" then out[#out+1]=r end
    end
    return out
  end
  local function owner_of(snapshot,job,work)
    for _,r in ipairs(workers_of(snapshot,job)) do
      for _,w in ipairs(structure(r).work or {}) do if w==work then return r end end
    end
    return nil
  end
  local function has_run(r)
    return type(r.run)=="table"
  end
  -- Current structural lineage generation ids (draft P4-D2: roles[].generation.lineage, else the generation itself).
  local function lineage_set(r)
    local out={}
    local g=r.generation or {}
    if type(g.lineage)=="table" then
      for _,id in ipairs(g.lineage) do out[dec(id)]=true end
    end
    if g.id~=nil then out[dec(g.id)]=true end
    return out
  end
  -- opened_seq of the first generation of the current lineage (draft P4-D2), 0 when the snapshot does not carry it.
  local function lineage_opened(r)
    local g=r.generation or {}
    return num(g.lineage_opened) or 0
  end
  local function review_part(snapshot)
    return snapshot.review or {count="0",verdicts={},blockers={},repairs={}}
  end
  -- A verdict lies in the reviewer's current lineage: its run generation (draft P4-D2) is in the lineage, or its run is
  -- the reviewer's active run; a verdict without a generation is trusted to the native snapshot filter.
  local function in_reviewer_lineage(v,reviewer)
    if v.generation==nil then return true end
    if has_run(reviewer) and same_id(reviewer.run.id,v.run) then return true end
    return lineage_set(reviewer)[dec(v.generation)]==true
  end
  -- A verdict of the current activation: its activation (draft P4-D3) equals snapshot.activation, or it has none.
  local function current_activation(snapshot,v)
    if v.activation==nil then return true end
    return same_id(v.activation,snapshot.activation)
  end
  local function by_seq_desc(list)
    return sorted_copy(list,function(a,b) return num(a.seq)>num(b.seq) end)
  end
  local function open_blockers(snapshot,job,cleared)
    local out={}
    for _,b in ipairs(review_part(snapshot).blockers or {}) do
      if b.job==job and not (cleared and cleared[dec(b.id)]) then out[#out+1]=b end
    end
    metered_sort(out,function(a,b) return num(a.id)<num(b.id) end)
    return out
  end

  -- R18-1 item 4 and DB-R17 ----------------------------------------------------------------------------------------
  -- Current W of a role: the reconcile hook when installed, else the acknowledged dispatch W of a non-drifted running
  -- role (DB-R5), else nil.
  local function current_work(snapshot,r)
    if r==nil then return nil end
    if checks and checks.current_work then return checks.current_work(snapshot,r) end
    if M.hooks.current_work then
      local cw=M.hooks.current_work(snapshot,r)
      if cw then return {work=cw.work,digest=fp(cw.digest)} end
      return nil
    end
    if has_run(r) and not r.drifted and type(r.run.dispatch)=="table" then
      return {work=r.run.dispatch.work,digest=fp(r.run.dispatch.work_fp)}
    end
    return nil
  end
  M.current_work=current_work

  function M.policy(snapshot,reviewer)
    local r=role_entry(snapshot,snapshot.active_job,reviewer)
    return current_work(snapshot,r)
  end

  -- shape.max_rounds of the reviewer's current W (integer or canonical decimal string), default 2 (declarations.md:275).
  function M.max_rounds(snapshot)
    local r=reviewer_of(snapshot,snapshot.active_job)
    local cw=current_work(snapshot,r)
    local shape=cw and type(cw.work)=="table" and cw.work.shape
    if type(shape)=="table" then
      local value=shape.max_rounds
      if is_integer(value) and value>=0 then return value end
      if type(value)=="string" and digits(value) and (value=="0" or string.sub(value,1,1)~="0") then
        return tonumber(value)
      end
    end
    return 2
  end

  -- R18-1 item 3 --------------------------------------------------------------------------------------------------
  -- {complete, targets, units, reviewer}: targets holds the complete entries sorted by work (as the native facts),
  -- units every reviewer target with its owner and entry (nil when the owner has no applicable completion).
  function M.target_vector(snapshot,job)
    job=job or snapshot.active_job
    local reviewer=reviewer_of(snapshot,job)
    local out={complete=false,targets={},units={},reviewer=reviewer and reviewer.role}
    if reviewer==nil then return out end
    local works=sorted_copy(structure(reviewer).targets or {})
    local complete=#works>0
    for _,work in ipairs(works) do
      local owner=owner_of(snapshot,job,work)
      local completion=owner and checks.applicable_completion(snapshot,owner.role)
      local output=completion and checks.unit_output(completion,work)
      local entry=nil
      if output then
        entry={work=work,output=output,work_fp=fp(completion.work_fp)}
        out.targets[#out.targets+1]=entry
      else
        complete=false
      end
      out.units[#out.units+1]={work=work,role=owner and owner.role,entry=entry}
    end
    out.complete=complete
    return out
  end

  local function targets_equal(a,b)
    if #a~=#b then return false end
    for i=1,#a do
      if a[i].work~=b[i].work or a[i].output~=b[i].output or a[i].work_fp~=b[i].work_fp then return false end
    end
    return true
  end
  -- R18-3 identity: the vector is complete, targets equal it exactly and policy equals the current policy identity.
  local function identity_matches(body,vector,policy)
    return vector.complete and policy~=nil and targets_equal(body.targets or {},vector.targets)
      and body.policy==policy.digest
  end

  -- R18-2 steps 5 to 7 --------------------------------------------------------------------------------------------
  local function fail(field,message,code)
    error({planar_review_failure=true,code=code or "schema",message=message,field=field},0)
  end
  local function keys(v,field,required,optional)
    local known=set_of(required)
    for _,k in ipairs(optional or {}) do known[k]=true end
    local unknown={}
    for k in pairs(v) do if not known[k] then unknown[#unknown+1]=k end end
    metered_sort(unknown)
    if #unknown>0 then fail(field.."."..unknown[1],"Unknown field") end
    for _,k in ipairs(required) do if v[k]==nil then fail(field.."."..k,"Required field") end end
  end
  local function record(v,field)
    if not is_object(v) then fail(field,"Expected object") end
    return v
  end
  local function list(v,field)
    if not is_list(v) then fail(field,"Expected list") end
    return v
  end
  local function id_field(v,field)
    if not identifier(v) then fail(field,"Invalid ID") end
    return v
  end
  local function digest_field(v,field)
    if not digest_text(v) then fail(field,"Malformed output digest") end
    return v
  end
  local function string_field(v,field,minimum,maximum)
    if type(v)~="string" or #v<minimum or #v>maximum or not valid_text(v) then fail(field,"Invalid string") end
    return v
  end
  -- Step 5, DB-R10/DB-R11 order; array indexes are 0-based canonical decimals as the native field paths.
  local function shape(doc)
    local root="verdict"
    local v=record(doc,root)
    keys(v,root,{"version","targets","policy","decision","findings","repair","summary"},{"authority"})
    if not is_integer(v.version) then fail(root..".version","Expected version integer") end
    if v.version~=1 then fail(root..".version","Unsupported schema version") end
    local out={version=1,targets={},findings={},repair={}}
    local targets=list(v.targets,root..".targets")
    if #targets==0 then fail(root..".targets","List is too short") end
    if #targets>256 then fail(root..".targets","Expected list") end
    local works={}
    for i,item in ipairs(targets) do
      local field=root..".targets."..(i-1)
      local t=record(item,field)
      keys(t,field,{"work","output","work_fp"})
      local entry={work=id_field(t.work,field..".work"),output=digest_field(t.output,field..".output"),
        work_fp=digest_field(t.work_fp,field..".work_fp")}
      if i>1 and not (out.targets[i-1].work<entry.work) then fail(field..".work","Duplicate value") end
      works[entry.work]=true
      out.targets[i]=entry
    end
    out.policy=digest_field(v.policy,root..".policy")
    if v.decision~="approve" and v.decision~="reject" then fail(root..".decision","Invalid enum value") end
    out.decision=v.decision
    local ids={}
    for i,item in ipairs(list(v.findings,root..".findings")) do
      local field=root..".findings."..(i-1)
      local f=record(item,field)
      keys(f,field,{"id","severity","work","text"},{"path","line","prescription"})
      if not finding_identifier(f.id) then fail(field..".id","Invalid ID") end
      if f.severity~="must" and f.severity~="should" and f.severity~="note" then
        fail(field..".severity","Invalid enum value")
      end
      local entry={id=f.id,severity=f.severity,work=id_field(f.work,field..".work")}
      if not works[entry.work] then fail(field..".work","Invalid enum value") end
      if f.path~=nil then
        if not source_path(f.path) then fail(field..".path","Invalid portable path") end
        entry.path=f.path
      end
      if f.line~=nil then
        if not is_integer(f.line) or f.line<1 or f.line>2147483647 then fail(field..".line","Integer out of range") end
        if entry.path==nil then fail(field..".path","Required field") end
        entry.line=f.line
      end
      if f.prescription~=nil then entry.prescription=id_field(f.prescription,field..".prescription") end
      entry.text=string_field(f.text,field..".text",1,4096)
      if ids[entry.id] then fail(field..".id","Duplicate value") end
      ids[entry.id]=true
      out.findings[i]=entry
    end
    for i,role in ipairs(list(v.repair,root..".repair")) do
      local field=root..".repair."..(i-1)
      id_field(role,field)
      if i>1 and not (out.repair[i-1]<role) then fail(field,"Duplicate value") end
      out.repair[i]=role
    end
    out.summary=string_field(v.summary,root..".summary",0,16384)
    if v.authority~=nil then
      if type(v.authority)~="string" or not valid_text(v.authority) then fail(root..".authority","Invalid string") end
      out.authority=v.authority
    end
    return out
  end

  -- {value=normalized verdict} or {error={code,message,field}}. The normalized verdict carries authority
  -- reviewer_role_v1 (R18-2 normalization).
  function M.validate_verdict(doc,snapshot)
    local ok,result=pcall(function()
      local out=shape(doc)
      -- Step 6 (R18-7).
      if out.authority~=nil and out.authority~=reviewer_authority then
        fail("","Execution policy is not supported","unsupported_policy")
      end
      out.authority=reviewer_authority
      -- Step 7: decision, then repair ownership.
      if out.decision=="approve" then
        local must=false
        for _,f in ipairs(out.findings) do if f.severity=="must" then must=true end end
        if must or #out.repair>0 then fail("verdict.decision","Invalid enum value") end
      end
      local job=snapshot.active_job
      for i,role in ipairs(out.repair) do
        local r=role_entry(snapshot,job,role)
        local owns=false
        if r and role_kind(r)=="worker" and r.state~="retired" then
          local own=set_of(structure(r).work)
          for _,t in ipairs(out.targets) do if own[t.work] then owns=true end end
        end
        if not owns then fail("verdict.repair."..(i-1),"Unknown selected role","reference") end
      end
      return out
    end)
    if ok then return {value=result} end
    if type(result)=="table" and result.planar_review_failure then
      return {error={code=result.code,message=result.message,field=result.field}}
    end
    error(result,0)
  end

  -- R18-3 and R18-5 -----------------------------------------------------------------------------------------------
  -- {applicability, case, repairs, blockers_opened}; cleared names blocker ids cleared by the same commit, which are not
  -- open for the "unless one is open" rule (DB-R19).
  function M.classify(verdict,snapshot,max_rounds,cleared)
    local job=snapshot.active_job
    local reviewer=reviewer_of(snapshot,job)
    local vector=M.target_vector(snapshot,job)
    local policy=reviewer and current_work(snapshot,reviewer)
    local out={applicability="historical",case="historical",repairs={},blockers_opened={}}
    if not identity_matches(verdict,vector,policy) then return out end
    out.applicability="current"
    if verdict.decision=="approve" then
      out.case="approve"
      return out
    end
    local clear_set={}
    for _,id in ipairs(cleared or {}) do clear_set[dec(id)]=true end
    local open={}
    for _,b in ipairs(open_blockers(snapshot,job,clear_set)) do open[b.kind]=true end
    local count=num(review_part(snapshot).count) or 0
    max_rounds=max_rounds or M.max_rounds(snapshot)
    if #(verdict.repair or {})==0 then
      out.case="unowned"
      if not open.unowned_rejection then out.blockers_opened={"unowned_rejection"} end
    elseif count<max_rounds then
      out.case="admitted"
      out.repairs=sorted_copy(verdict.repair)
    else
      out.case="capped"
      if not open.max_rounds then out.blockers_opened={"max_rounds"} end
    end
    return out
  end

  -- R18-5 clearing. next = {operation, job (active job after the commit, "" when cleared), max_rounds (after the
  -- commit), snapshot (the post-commit view for the vector and policy; defaults to snapshot)}. Ids ascending.
  function M.blockers_to_clear(snapshot,next)
    next=next or {}
    local job=snapshot.active_job
    local blockers=open_blockers(snapshot,job)
    local out={}
    local all=next.operation=="activate" or (next.job~=nil and next.job~=job)
    local view=next.snapshot or snapshot
    local count=num(review_part(snapshot).count) or 0
    for _,b in ipairs(blockers) do
      local clear=all
      if not clear and b.kind=="max_rounds" then
        clear=next.operation=="load" and next.max_rounds~=nil and next.max_rounds>count
      elseif not clear and b.kind=="unowned_rejection" then
        local opening=nil
        for _,v in ipairs(review_part(snapshot).verdicts or {}) do
          if same_id(v.id,b.verdict) then opening=v end
        end
        if opening then
          local reviewer=reviewer_of(view,view.active_job)
          local vector=M.target_vector(view,view.active_job)
          local policy=reviewer and current_work(view,reviewer)
          clear=not identity_matches(opening.body or opening,vector,policy)
        end
      end
      if clear then out[#out+1]=b.id end
    end
    return out
  end

  -- R18-6 ---------------------------------------------------------------------------------------------------------
  local function approvals_and_rejection(snapshot,job,vector,policy)
    local reviewer=reviewer_of(snapshot,job)
    local floor=0
    for _,unit in ipairs(vector.units) do
      local owner=unit.role and role_entry(snapshot,job,unit.role)
      if owner and lineage_opened(owner)>floor then floor=lineage_opened(owner) end
    end
    local approved,rejected=false,false
    local latest=nil
    for _,v in ipairs(review_part(snapshot).verdicts or {}) do
      local body=v.body or {}
      if v.applicability=="current" and reviewer and in_reviewer_lineage(v,reviewer)
        and identity_matches(body,vector,policy) and num(v.seq)>floor then
        if v.decision=="approve" then approved=true end
        if latest==nil or num(v.seq)>num(latest.seq) then latest=v end
      end
    end
    if latest and latest.decision=="reject" then rejected=true end
    return approved,rejected
  end

  -- Array sorted by work of {work, role, accepted, why} over every unit of the active job's non-retired workers.
  function M.acceptance(snapshot)
    local job=snapshot.active_job
    if job==nil or job=="" then return {} end
    local reviewer=reviewer_of(snapshot,job)
    local vector=M.target_vector(snapshot,job)
    local policy=reviewer and current_work(snapshot,reviewer)
    local approved,rejected=approvals_and_rejection(snapshot,job,vector,policy)
    local blocked=#open_blockers(snapshot,job)>0
    local units={}
    for _,r in ipairs(workers_of(snapshot,job)) do
      for _,w in ipairs(structure(r).work or {}) do units[#units+1]={work=w,role=r} end
    end
    metered_sort(units,function(a,b) return a.work<b.work end)
    local out={}
    for _,u in ipairs(units) do
      local why={}
      if u.role.state~="done" then why[#why+1]="worker_not_done" end
      if checks.applicable_completion(snapshot,u.role.role)==nil then why[#why+1]="no_applicable_completion" end
      if not approved then why[#why+1]="no_current_approval" end
      if rejected then why[#why+1]="rejected" end
      if blocked then why[#why+1]="blocker_open" end
      out[#out+1]={work=u.work,role=u.role.role,accepted=#why==0,why=why}
    end
    return out
  end

  -- R18-1 item 5 --------------------------------------------------------------------------------------------------
  local function finding_less(a,b)
    if a.work~=b.work then return a.work<b.work end
    return a.id<b.id
  end
  -- Lines of one finding block: header line, then each LF-separated line of text indented by four spaces.
  function M.finding_lines(f)
    local head="- ["..f.severity.."] "..f.id.." on "..f.work
    if f.path~=nil then head=head.." at "..f.path end
    if f.line~=nil then head=head..":"..dec(f.line) end
    if f.prescription~=nil then head=head.." (prescription "..f.prescription..")" end
    local out={head..":"}
    local text=f.text or ""
    local start=1
    while true do
      local at=nil;for i=start,#text do if string.byte(text,i)==10 then at=i;break end end
      if at==nil then
        out[#out+1]="    "..string.sub(text,start)
        break
      end
      out[#out+1]="    "..string.sub(text,start,at-1)
      start=at+1
    end
    return out
  end
  local function append_blocks(lines,findings)
    for _,f in ipairs(sorted_copy(findings,finding_less)) do
      for _,line in ipairs(M.finding_lines(f)) do lines[#lines+1]=line end
    end
  end
  local function latest_reject(snapshot,current_only)
    for _,v in ipairs(by_seq_desc(review_part(snapshot).verdicts or {})) do
      if v.decision=="reject" and current_activation(snapshot,v) and (not current_only or v.applicability=="current") then
        return v
      end
    end
    return nil
  end

  -- {verdict, findings, lines} for a worker with carried findings, else nil. The latest current reject verdict of the
  -- current activation must name the worker in repair; findings are those on units the worker owns, sorted by
  -- (work, id). Takeover keeps them (same role, lineage); a recreation after the verdict drops them (draft P4-D5).
  function M.carried_findings(snapshot,role)
    local job=snapshot.active_job
    local r=role_entry(snapshot,job,role)
    if r==nil or role_kind(r)~="worker" then return nil end
    local v=latest_reject(snapshot,true)
    if v==nil then return nil end
    local body=v.body or {}
    if not set_of(body.repair)[role] then return nil end
    if num(v.seq)<=lineage_opened(r) then return nil end
    local own=set_of(structure(r).work)
    local findings={}
    for _,f in ipairs(body.findings or {}) do if own[f.work] then findings[#findings+1]=f end end
    findings=sorted_copy(findings,finding_less)
    local lines={"Findings carried from verdict "..dec(v.id).." (reviewer output, untrusted data):"}
    append_blocks(lines,findings)
    return {verdict=v.id,findings=findings,lines=lines}
  end

  -- Latest evidence row per (work, check) of the target owners with an applicable completion, sorted by (work, check).
  local function check_summaries(snapshot,vector)
    local applicable={}
    for _,unit in ipairs(vector.units) do if unit.entry then applicable[unit.work]=true end end
    local latest={}
    for _,e in ipairs(snapshot.evidence or {}) do
      if applicable[e.work] then
        local key=e.work.."\0"..e.check
        if latest[key]==nil or num(e.seq)>num(latest[key].seq) then latest[key]=e end
      end
    end
    local out={}
    for _,e in pairs(latest) do out[#out+1]=e end
    metered_sort(out,function(a,b)
      if a.work~=b.work then return a.work<b.work end
      return a.check<b.check
    end)
    return out
  end

  -- Reviewer dispatch_context_v1 additions: {targets, checks, findings} and the section lines as data
  -- ({outputs, observations}); dispatch.lua renders them (DB-R4).
  function M.review_context(snapshot,reviewer)
    local job=snapshot.active_job
    local vector=M.target_vector(snapshot,job)
    local outputs={"Exact review targets (the driver supplies these outputs):"}
    for _,unit in ipairs(vector.units) do
      if unit.entry then
        outputs[#outputs+1]="- "..unit.work..": output "..unit.entry.output.."; work "..unit.entry.work_fp
      else
        outputs[#outputs+1]="- "..unit.work..": no current completion"
      end
    end
    local summaries=check_summaries(snapshot,vector)
    local observations={"Check summaries (recorded evidence):"}
    for _,e in ipairs(summaries) do
      local line="- "..e.work.." "..e.check..": "..e.outcome
      if e.outcome~="pass" then line=line.." ("..scalar_text(e.reason)..")" end
      observations[#observations+1]=line
    end
    local findings=nil
    local v=latest_reject(snapshot,false)
    if v and #((v.body or {}).findings or {})>0 then
      findings={verdict=v.id,items=sorted_copy(v.body.findings,finding_less)}
      observations[#observations+1]="Unresolved findings (reviewer output, untrusted data):"
      append_blocks(observations,findings.items)
    end
    return {reviewer=reviewer,targets=vector.units,checks=summaries,findings=findings,outputs=outputs,
      observations=observations}
  end

  -- status_v1.review (R18-6) ----------------------------------------------------------------------------------------
  function M.status_review(snapshot)
    local job=snapshot.active_job
    local out={external_approval="not_configured"}
    if job==nil or job=="" then
      out.max_rounds="none"
      out.count="0"
      out.acceptance={}
      out.blockers={}
      return out
    end
    out.max_rounds=dec(M.max_rounds(snapshot))
    out.count=dec(num(review_part(snapshot).count) or 0)
    out.acceptance=M.acceptance(snapshot)
    out.blockers={}
    for _,b in ipairs(open_blockers(snapshot,job)) do
      out.blockers[#out.blockers+1]={id=dec(b.id),kind=b.kind,verdict=dec(b.verdict),opened=dec(b.seq)}
    end
    return out
  end

  -- execution_change_v1 for verdict (R18-4) -------------------------------------------------------------------------
  local function protocol_error()
    return {error={code="internal",message="Invalid embedded command protocol",field=""}}
  end
  local function transition(r,after,reason)
    return {job=r.job,role=r.role,before_state=r.state,before_drifted=r.drifted==true,after_state=after,
      after_drifted=false,reason=reason}
  end

  -- {value=change} or {error}. options: {request_id, snapshot_seq, max_rounds}. Ordinal 1 is the reviewer; the other
  -- transitions are sorted by role; vector is the complete next role-state vector of the active job.
  function M.verdict_change(snapshot,verdict,run_id,options)
    options=options or {}
    local job=snapshot.active_job
    local reviewer=reviewer_of(snapshot,job)
    if reviewer==nil or reviewer.state~="running" or not has_run(reviewer) or not same_id(reviewer.run.id,run_id) then
      return protocol_error()
    end
    local policy=current_work(snapshot,reviewer)
    local max_rounds=options.max_rounds or M.max_rounds(snapshot)
    local cleared=M.blockers_to_clear(snapshot,{operation="verdict",job=job})
    local classified=M.classify(verdict,snapshot,max_rounds,cleared)
    local vector=M.target_vector(snapshot,job)
    local workers=workers_of(snapshot,job)
    local reviewer_after
    if classified.case=="historical" then
      reviewer_after="stale"
      for _,w in ipairs(workers) do if w.state~="done" then reviewer_after="blocked" end end
    elseif classified.case=="admitted" then
      reviewer_after="blocked"
    else
      reviewer_after="done"
    end
    local transitions={transition(reviewer,reviewer_after,"completed")}
    for _,role in ipairs(classified.repairs) do
      local w=role_entry(snapshot,job,role)
      if w==nil then return protocol_error() end
      if w.state=="done" then
        transitions[#transitions+1]=transition(w,"stale","repair_admitted")
      elseif w.state=="blocked" then
        transitions[#transitions+1]=transition(w,"blocked","repair_admitted")
      else
        return protocol_error()
      end
    end
    local blockers_opened={}
    for i,k in ipairs(classified.blockers_opened) do blockers_opened[i]=k end
    local expected={{kind="request_unused",request_id=options.request_id},
      {kind="active_revision",revision=snapshot.active_revision},
      {kind="active_job",job=job}}
    for _,r in ipairs(roles_of(snapshot,job)) do
      expected[#expected+1]={kind="fact_version",job=r.job,role=r.role,fact_version=dec(r.fact_version)}
    end
    expected[#expected+1]={kind="role_state",job=job,role=reviewer.role,state="running",drifted=false,run=reviewer.run.id}
    expected[#expected+1]={kind="generation_current",job=job,role=reviewer.role,generation=reviewer.generation.id}
    expected[#expected+1]={kind="run_active",job=job,role=reviewer.role,run=reviewer.run.id,
      generation=reviewer.generation.id}
    local change={version=1,operation="verdict",snapshot_seq=options.snapshot_seq,
      facts={role=reviewer.role,run=reviewer.run.id,verdict=verdict,
        current={targets=vector.targets,policy=policy and policy.digest,work=policy and policy.work},
        max_rounds=max_rounds,case=classified.case,repairs=classified.repairs,blockers_opened=blockers_opened,
        blockers_cleared=cleared},
      transitions=transitions,expected=expected}
    if checks and checks.rederive then change=checks.rederive(snapshot,change) end
    if M.hooks.reconcile then change=M.hooks.reconcile(snapshot,change) or change end
    -- Ordinal 1 stays the reviewer; the rest ascend by role (DB-R29).
    local first=change.transitions[1]
    local rest={}
    for i=2,#change.transitions do rest[#rest+1]=change.transitions[i] end
    metered_sort(rest,function(a,b) return a.role<b.role end)
    change.transitions={first}
    for _,t in ipairs(rest) do change.transitions[#change.transitions+1]=t end
    local after={}
    for _,t in ipairs(change.transitions) do after[t.role]=t end
    change.vector={}
    for _,r in ipairs(roles_of(snapshot,job)) do
      local t=after[r.role]
      local entry={job=r.job,role=r.role,state=r.state,drifted=r.drifted==true,run="none"}
      if has_run(r) then entry.run=r.run.id end
      if t then
        entry.state=t.after_state
        entry.drifted=t.after_drifted
        if r.role==reviewer.role then entry.run="none" end
      end
      change.vector[#change.vector+1]=entry
    end
    return {value=change}
  end

  return M
end)()

-- Bind the shipped review policy to check completion policy after both modules exist.
if checks then
 checks.hooks.acceptance=review.acceptance
 checks.hooks.blockers_to_clear=review.blockers_to_clear
end

-- Late-bound dispatch hooks keep the earlier renderer independent of review module order.
if dispatch_internal then
 dispatch_internal.material=function(S,role)
  for _,r in ipairs(S.roles or {})do if r.job==S.active_job and r.role==role then return checks.material(S,r)end end
 end
 dispatch_internal.section=function(S,role,kind,id,text)
  if kind=="reviewer"then
   local context=review.review_context(S,role)
   if id=="outputs"then return "## outputs\n\n"..table.concat(context.outputs,"\n").."\n"end
   if id=="observations"then return "## observations\n\n"..table.concat(context.observations,"\n").."\n"end
  elseif id=="observations"then
   local carried=review.carried_findings(S,role)
   if carried then return text.."\n"..table.concat(carried.lines,"\n").."\n"end
  end
  return text
 end
end

if dispatch_internal then
 dispatch_internal.applicable=checks.applicable_completion
 dispatch_internal.clear_blockers=function(S,input,job)
  local view={};for k,v in pairs(S)do view[k]=v end
  if input.operation=="load"then view.revision={interpretation=input.candidate}end
  view.active_job=job
  local location=view.revision and dispatch_internal.locate(view.revision.interpretation,job)
  return engine.array(review.blockers_to_clear(S,{operation=input.operation,job=job,snapshot=view,max_rounds=location and location.job.max_rounds}))
 end
end
