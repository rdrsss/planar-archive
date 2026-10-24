-- Glaze owns JSON presentation and transport. Display escaping below is prose.
local function presentation_hex_byte(b)
  local digits="0123456789abcdef"
  return string.sub(digits,b//16+1,b//16+1)..string.sub(digits,b%16+1,b%16+1)
end
local function presentation_json(value)
  local result, refusal = engine.json_encode(value)
  if refusal then error(refusal, 0) end
  return result
end
local function presentation_data(label,bytes)
  local out,escaped={},false
  local i=1
  while i<=#bytes do
    local b=string.byte(bytes,i)
    local next_byte=string.byte(bytes,i+1)
    if b==194 and next_byte and next_byte>=128 and next_byte<=159 then
      out[#out+1]="\\u00"..presentation_hex_byte(next_byte); escaped=true; i=i+2
    else
      if (b<32 and b~=9 and b~=10 and b~=13) or b==127 then
        out[#out+1]="\\u00"..presentation_hex_byte(b); escaped=true
      else out[#out+1]=string.char(b) end
      i=i+1
    end
  end
  local display=table.concat(out)
  local longest,run=0,0
  for i=1,#display do
    if string.byte(display,i)==96 then run=run+1; longest=math.max(longest,run) else run=0 end
  end
  local fence={}
  for _=1,math.max(3,longest+1) do fence[#fence+1]="`" end
  fence=table.concat(fence)
  if escaped then label=label.."; display controls escaped as \\u00xx" end
  return label.."\n"..fence.."\n"..display..(string.sub(display,-1)=="\n" and "" or "\n")..fence.."\n"
end
