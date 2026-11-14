  local function load_command(invocation,context)
    local mode=invocation.options["request-id"]
    if mode=="forged" then return context.open_workspace({workspace={}}) end
    if mode=="wrongslot" then return context.open_workspace({workspace=invocation.options.registry}) end
    if mode=="cross" then return context.read_history({}) end
    if mode=="order" then return context.capture_load({route={}}) end
    if mode=="skipped" then return result({version=1,kind="load"}) end
    if mode=="memory" then pcall(function() local s="x";while true do s=s..s end end) end
    if mode=="instructions" then pcall(function() while true do end end) end
    if mode=="cumulative" then
      local bytes="x";for i=1,15 do bytes=bytes..bytes end
      pcall(function() for i=1,20000 do engine.json_encode({x=bytes}) end end)
    end
    local opened=context.open_workspace({workspace=invocation.options.workspace})
    if mode=="double" then pcall(context.open_workspace,{workspace=invocation.options.workspace}) end
    if not opened.ok then
      if mode=="poison" then pcall(context.capture_load,{route={}});return failed({code="schema",message="replacement",field=""}) end
      return opened
    end
    local routed=route_workspace(opened.value)
    if routed.error then return failed(routed.error) end
    local captured=context.capture_load({route=routed.value})
    if not captured.ok then return captured end
    local interpreted=interpret_bundle(captured.value.bundle)
    if interpreted.error then return failed(interpreted.error) end
    local normalized=interpreted.value
    -- Verification-only native room: padbase-N/padaccept-N charge N string bytes through json_encode's input conversion
    -- and again through its result (two native bytes per pad byte), then send malformed or exact acceptance arguments.
    local pad_kind=string.sub(mode,1,8)=="padbase-" and "base" or string.sub(mode,1,10)=="padaccept-" and "accept" or nil
    if pad_kind then
      local n=0
      for i=#pad_kind+5,#mode do n=n*10+string.byte(mode,i)-48 end
      local chunk="x";for i=1,20 do chunk=chunk..chunk end
      while n>=#chunk do engine.json_encode({x=chunk});n=n-#chunk end
      local rest,piece="","x"
      while n>0 do if n%2==1 then rest=rest..piece end;piece=piece..piece;n=n//2 end
      engine.json_encode({x=rest})
      if pad_kind=="base" then return context.accept_load({bundle=normalized.bundle,semantic=normalized.semantic,extra=true}) end
    end
    if mode=="badbundle" then normalized.bundle={} end
    if mode=="badsemantic" then normalized.semantic={} end
    if mode=="badargs" then return context.accept_load({bundle=normalized.bundle,semantic=normalized.semantic,extra=true}) end
    local accepted=context.accept_load({bundle=normalized.bundle,semantic=normalized.semantic})
    if mode=="postcumulative" then
      local bytes="x";for i=1,15 do bytes=bytes..bytes end
      pcall(function() for i=1,20000 do engine.json_encode({x=bytes}) end end)
    end
    if mode=="posterror" then error("after native completion") end
    if mode=="postinstructions" then pcall(function() while true do end end) end
    if mode=="postmemory" then pcall(function() local s="x";while true do s=s..s end end) end
    if mode=="postdouble" then pcall(context.accept_load,{bundle=normalized.bundle,semantic=normalized.semantic}) end
    if mode=="postchanged" then accepted.value.event="99" end
    if mode=="postfalse" then return failed({code="schema",message="after completion",field=""}) end
    if mode=="postmalformed" then return nil end
    return accepted
  end
