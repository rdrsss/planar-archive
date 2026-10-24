-- Test-only providers around exact product helpers, never a product export.
local function probe(request)
  local inputs=engine.array({})
  local context
  if request.profile then
    context={dialect="declarations_shapes_v1",shapes=request.descriptors,call=function(id,input)
      inputs[#inputs+1]=input
      if request.mode=="malformed" then
        if request.variant=="nil" then return nil end
        if request.variant=="extra" then return {ok=true,value={obligations=engine.array({extra=true}),guidance=""}} end
        if request.variant=="sparse" then return {ok=true,value={obligations=engine.array({[2]={}}),guidance=""}} end
        if request.variant=="utf8" then return {ok=true,value={obligations=engine.array({}),guidance=string.char(255)}} end
        if request.variant=="nul" then return {ok=true,value={obligations=engine.array({}),guidance="a\0b"}} end
        if request.variant=="callable" then return function() end end
      end
      if request.mode=="generated" then
        local obligations=engine.array({})
        for i=1,request.count do
          -- IDs use the declaration grammar, including beyond 10000.
          local n,tail=i,""
          while n>0 do tail=string.char(97+n%26)..tail; n=n//26 end
          obligations[#obligations+1]=engine.object({id="edge"..tail,producer="library",consumer="command",phase="complete",quality="done"})
        end
        return engine.object({ok=true,value=engine.object({obligations=obligations,guidance=""})})
      end
      return request.responses[input.job.id] or request.response
    end}
  end
  local result
  if request.normalize then result=normalize_bundle(request.bundle,context)
  else
    local ok,value=pcall(validate_bundle,engine.object({bundle=request.bundle,capture=engine.object({status="not_captured"})}),context)
    if not ok then return engine.object({failure=type(value)=="string" and value or "nonstring failure"}) end
    result=value
  end
  return engine.object({result=result,inputs=inputs,calls=#inputs})
end
return {shapes_probe=probe}
