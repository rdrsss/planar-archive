-- Invocation-local native registration is absent from fresh fixed-export VMs.
if registration then
  local function result(value) return {ok=true,value=value} end
  local function failed(error) return {ok=false,error=error} end
  local function render_command(invocation,context)
    local opened=context.open_workspace({workspace=invocation.options.workspace})
    if not opened.ok then return opened end
    local routed=route_workspace(opened.value)
    if routed.error then return failed(routed.error) end
    local captured=context.capture_render({route=routed.value})
    if not captured.ok then return captured end
    local rendered=render(captured.value)
    if rendered.error then return failed(rendered.error) end
    return result(rendered.value)
  end
  local function load_command(invocation,context)
    local opened=context.open_workspace({workspace=invocation.options.workspace})
    if not opened.ok then return opened end
    local routed=route_workspace(opened.value)
    if routed.error then return failed(routed.error) end
    local captured=context.capture_load({route=routed.value})
    if not captured.ok then return captured end
    local interpreted=interpret_bundle(captured.value.bundle)
    if interpreted.error then return failed(interpreted.error) end
    local normalized=interpreted.value
    return context.accept_load({bundle=normalized.bundle,semantic=normalized.semantic})
  end
  local function init_command(invocation,context)
    local id=invocation.options["workspace-id"]
    local valid=#id>0 and #id<=64
    for i=1,#id do
      local b=string.byte(id,i)
      if not ((b>=97 and b<=122) or (i>1 and ((b>=48 and b<=57) or b==95))) then valid=false end
    end
    if not valid then return failed({code="schema",message="Invalid ledger arguments",field=""}) end
    local initialized=context.initialize_ledger({workspace_id=invocation.options["workspace-id"]})
    if not initialized.ok then return initialized end
    local facts=initialized.value
    return result({version=1,kind="init",ledger=facts.ledger,workspace=facts.workspace,active_revision=facts.active_revision,event=facts.event})
  end
  local function history_command(invocation,context)
    local options,present=invocation.options,invocation.present
    local selector,id,key="page",options["after-event"],""
    if present.revision then selector,id="revision",options.revision
    elseif present["at-event"] then selector,id="state",options["at-event"]
    elseif present["request-id"] then selector,id,key="request","0",options["request-id"] end
    local count=present.limit and options.limit or 50
    return context.read_history({selector=selector,id=id,request_id=key,limit=count})
  end
  local function export_command(invocation,context)
    local exported=context.export_source({revision=invocation.options.revision})
    if not exported.ok then return exported end
    local facts=exported.value
    return result({version=1,kind="export",ledger=facts.ledger,revision=facts.revision,source=facts.source})
  end
  local function option(name,kind,required)
    return {name="--"..name,type=kind,required=required,help="Explicit "..name.." value"}
  end
  local function descriptor(name,capability,options)
    return {name=name,family=name,handler_id="shipped."..name..".v1",options=engine.array(options),help="Planar "..name,response_kind=name.."_v1",capabilities=engine.array({capability})}
  end
  registration.add(descriptor("render","render_capture_v1",{
    option("workspace","text",true),option("registry","text",true),option("root","text",true),
    option("job","text",true),option("role","text",true),option("base-root","text",false),
    option("observations","text",false),option("budget-bytes","decimal",false),option("capture-outputs","flag",false)
  }),render_command)
  registration.add(descriptor("init","initialize_ledger_v1",{
    option("ledger","text",true),option("workspace-id","text",true)
  }),init_command)
  registration.add(descriptor("history","read_history_v1",{
    option("ledger","text",true),option("request-id","text",false),option("revision","decimal",false),
    option("at-event","decimal",false),option("after-event","decimal",false),option("limit","decimal",false)
  }),history_command)
  registration.add(descriptor("export","export_source_v1",{
    option("ledger","text",true),option("target","text",true),option("revision","decimal",true)
  }),export_command)
  registration.add(descriptor("load","accept_load_v1",{
    option("ledger","text",true),option("workspace","text",true),option("registry","text",true),
    option("expect-revision","text",true),option("request-id","text",true),option("actor","text",true)
  }),load_command)
end
