-- Authoring observe handler: structural validation and assembly of one observation directory.
-- Embedded only in the authoring stack. Applies render's structural rules to each file and none of its semantic ones.
local function observe_directory_command(invocation,context)
  if type(invocation.options.from) ~= "userdata" then return { ok = false, error = { code = "internal", message = "Invalid embedded command protocol", field = "", details = {} } } end
  local read=context.read_observation_directory({})
  if not read.ok then return read end
  local function failed(err) return {ok=false,error=err} end
  local function relative_field(field)
    if field=="" then return "$" end
    if string.sub(field,1,1)=="." then return string.sub(field,2) end
    return field
  end
  local function digest_valid(digest)
    local valid=#digest==41 and string.sub(digest,1,9)=="xxh3_128:"
    for i=10,#digest do local b=string.byte(digest,i); if not ((b>=48 and b<=57) or (b>=97 and b<=102)) then valid=false end end
    return valid
  end
  -- Returns the normalized value, the raw parsed value and nil, or nil, nil and the typed error.
  local function validate(entry,schema,targets)
    local ok,parsed=pcall(json_parse,entry.bytes,entry.name)
    if not ok then
      if type(parsed)=="table" then return nil,nil,parsed end
      error(parsed,0)
    end
    local ctx=schema_context({},false)
    local value=ctx.normalize(parsed,schema,entry.name,"")
    if #ctx.errors==0 and targets then
      if value.target then
        if not digest_valid(value.target.output) then ctx.issue(entry.name,".target.output","Malformed output digest","schema") end
      end
      for j,target in ipairs(value.vector or {}) do
        if not digest_valid(target.output) then ctx.issue(entry.name,".vector["..ctx.index(target,j).."].output","Malformed output digest","schema") end
      end
    end
    if #ctx.errors>0 then
      local err=ctx.finish({}).error
      err.field=relative_field(err.field)
      return nil,nil,err
    end
    return value,parsed,nil
  end
  local checks,findings,checkpoint={},{},nil
  local seen_checks,seen_findings={},{}
  for _,category in ipairs({"check","finding","checkpoint"}) do
    for _,entry in ipairs(read.value.entries) do
      if entry.category==category then
        if category=="check" then
          local value,_,err=validate(entry,observation_check,true)
          if err then return failed(err) end
          if seen_checks[value.id] then return failed({code="reference",message="Duplicate value",file=entry.name,field="id",details={}}) end
          seen_checks[value.id]=true
          checks[#checks+1]=value
        elseif category=="finding" then
          local value,_,err=validate(entry,observation_finding,true)
          if err then return failed(err) end
          if seen_findings[value.id] then return failed({code="reference",message="Duplicate value",file=entry.name,field="id",details={}}) end
          seen_findings[value.id]=true
          findings[#findings+1]=value
        else
          local value,parsed,err=validate(entry,observation_checkpoint,false)
          if err then return failed(err) end
          if #value.files>32 then return failed({code="resource_limit",message="Too many checkpoint paths",file=entry.name,field="",details={limit="checkpoint_files",maximum=32}}) end
          if #presentation_json(value)>8192 then return failed({code="resource_limit",message="Checkpoint canonical JSON too large",file=entry.name,field="",details={limit="checkpoint_bytes",maximum=8192}}) end
          local files={}
          for i,path in ipairs(parsed.files) do files[i]=path end
          value.files=engine.array(files)
          checkpoint=value
        end
      end
    end
  end
  local result=engine.object({version=1,checks=sorted(checks,function(x)return x.id end),findings=sorted(findings,function(x)return x.id end)})
  if checkpoint then result.checkpoint=checkpoint end
  return {ok=true,value=result}
end
