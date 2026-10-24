-- Fixed build-composed helpers. No runtime module loading or source selection.
local function byte_order(a,b)
  for i=1,math.min(#a,#b) do
    local x,y=string.byte(a,i),string.byte(b,i)
    if x~=y then return x<y end
  end
  return #a<=#b
end
local function sorted(values, key)
  -- Bottom-up merge sort: all work is metered Lua, including sparse-table refusal
  -- at input conversion. Never call native table.sort/insert/remove/move.
  local n, width = #values, 1
  while width < n do
    local out = {}
    for lo = 1, n, width * 2 do
      local mid, hi = math.min(lo + width, n + 1), math.min(lo + width * 2, n + 1)
      local a, b = lo, mid
      for i = lo, hi - 1 do
        if b >= hi or (a < mid and byte_order(key(values[a]), key(values[b]))) then
          out[i], a = values[a], a + 1
        else out[i], b = values[b], b + 1 end
      end
    end
    values, width = out, width * 2
  end
  return engine.array(values)
end
local function utf8_text(s)
  if type(s) ~= "string" then return false end
  local i, n = 1, #s
  while i <= n do
    local c = string.byte(s, i)
    local count, low, high = 0, 128, 191
    if c == 0 then return false
    elseif c < 128 then count = 0
    elseif c >= 194 and c <= 223 then count = 1
    elseif c >= 224 and c <= 239 then
      count = 2
      if c == 224 then low = 160 elseif c == 237 then high = 159 end
    elseif c >= 240 and c <= 244 then
      count = 3
      if c == 240 then low = 144 elseif c == 244 then high = 143 end
    else return false end
    if i + count > n then return false end
    for j = 1, count do
      local b = string.byte(s, i + j)
      if b < (j == 1 and low or 128) or b > (j == 1 and high or 191) then return false end
    end
    i = i + count + 1
  end
  return true
end
-- Grammar, Unicode unescaping and numeric conversion belong to native Glaze.
local function json_parse(bytes, file)
  if type(bytes) ~= "string" then
    error({code="schema",message="Invalid JSON",file=file,field="$",details={reason="expected string",offset=1}},0)
  end
  local result, refusal = engine.json_decode(bytes)
  if refusal then
    refusal.file, refusal.field = file, "$"
    error(refusal, 0)
  end
  return result
end
