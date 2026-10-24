    local mode=invocation.options["workspace-id"]
    if mode=="projection" then return {ok=false,error={code="schema",message="Altered retained handler",field="",details={}}} end
    if mode=="skip" then return {ok=true,value={version=1,kind="init"}} end
    if mode=="cross" then return context.read_history({}) end
    if mode=="badargs" then return context.initialize_ledger({workspace_id="different"}) end
    if mode=="unknownargs" then return context.initialize_ledger({workspace_id=mode,unknown=true}) end
    if string.sub(mode,1,8)=="combined" then
      local bytes="x";for i=1,15 do bytes=bytes..bytes end
      local count=tonumber(string.sub(mode,9))
      pcall(function() for i=1,count do engine.json_encode({x=bytes}) end end)
      return {ok=false,error={code="schema",message="completed transfer sequence",field=""}}
    end
    if mode=="cumulative" then
      local bytes="x";for i=1,15 do bytes=bytes..bytes end
      pcall(function() for i=1,20000 do engine.json_encode({x=bytes}) end end)
      return {ok=false,error={code="schema",message="hidden",field=""}}
    end
    if mode=="catch_instruction" then pcall(function() while true do end end); return {ok=false,error={code="schema",message="hidden",field=""}} end
    if mode=="catch_allocation" then pcall(function() local t={}; while true do t[#t+1]=string.sub("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",1,31)..#t end end); return {ok=false,error={code="schema",message="hidden",field=""}} end
    if mode=="multi_stack" then local bytes="x";for i=1,12 do bytes=bytes..bytes end;pcall(string.byte,bytes,1,#bytes);return {ok=false,error={code="schema",message="hidden",field=""}} end
    if mode=="stack" then local function recurse(n) local a,b,c,d,e,f,g,h=1,2,3,4,5,6,7,8;if n==0 then return 1 end;return recurse(n-1)+a+b+c+d+e+f+g+h end;pcall(recurse,10000);return {ok=false,error={code="schema",message="hidden",field=""}} end
    if mode=="depth" then local v={};for i=1,258 do v={nested=v} end;pcall(context.initialize_ledger,v);return {ok=false,error={code="schema",message="hidden",field=""}} end
    if mode=="freeze" then
      if rawset or rawget or getmetatable or registration then error("exposed mutation") end
      if pcall(function() engine.kind=nil end) then error("mutable engine") end
      if pcall(function() math.floor=nil end) then error("mutable library") end
      if pcall(function() injected=true end) then error("mutable globals") end
    end
    local first=context.initialize_ledger({workspace_id=mode})
    if mode=="repeat" or mode=="poison" then pcall(context.initialize_ledger,{workspace_id=mode}) end
    if mode=="replace" and first.ok then first.value.workspace="forged" end
    if mode=="suppress" or mode=="poison" then return {ok=true,value={version=1,kind="init"}} end
    if not first.ok then return first end
    local facts=first.value
    return {ok=true,value={version=1,kind="init",ledger=facts.ledger,workspace=facts.workspace,active_revision=facts.active_revision,event=facts.event}}
