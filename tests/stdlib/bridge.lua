-- Compiled only into the separate test runtime library.
local calls = 0
-- Each doubling is metered Lua work; allocations remain subject to lua_bytes.
local function repeat_byte(byte, count)
  local bytes = byte
  while #bytes < count do bytes = bytes .. bytes end
  return bytes:sub(1,count)
end
return {
  json_roundtrip = function(input)
    local value, decode_error = engine.json_decode(input)
    if decode_error then return {error=decode_error} end
    local encoded, encode_error = engine.json_encode(value)
    if encode_error then return {error=encode_error} end
    return {encoded=encoded, value=value}
  end,
  json_error_recovery = function()
    local _, old = pcall(engine.fragment_digest, string.char(255))
    local value, refusal = engine.json_decode('{"a":1,}')
    local valid, absent = engine.json_decode('false')
    local encoded, encode_error = engine.json_encode(engine.array {"\n\t\r\b\f"..string.char(31),true})
    return {ordinary=type(refusal)=='table',code=refusal.code,offset=refusal.details.offset,
      recovered=valid==false and absent==nil, encoded=encoded, encode_ok=encode_error==nil,
      value_absent=value==nil, old_token=type(old)=='userdata'}
  end,
  json_encode_refusal = function(input)
    local value = input == 'cycle' and {} or engine.array {[2]=true}
    if input == 'cycle' then value.self=value end
    local encoded, refusal = engine.json_encode(value)
    return {absent=encoded==nil,error=refusal}
  end,
  copy = function(input) return input end,
  mutate = function(input) input.changed = true; input.nested.value = 'new'; return input end,
  fresh = function() calls = calls + 1; return calls end,
  capabilities = function()
    return {missing = io == nil and os == nil and package == nil and require == nil
      and load == nil and loadfile == nil and dofile == nil and debug == nil
      and print == nil and collectgarbage == nil and coroutine == nil
      and math.random == nil and math.randomseed == nil and string.dump == nil
      and string.format == nil and string.find == nil and string.match == nil
      and string.gmatch == nil and string.gsub == nil
      and string.rep == nil and ('').rep == nil and table.move == nil
      and ('').find == nil and ('').match == nil and ('').gmatch == nil and ('').gsub == nil
      and tostring == nil and setmetatable == nil and xpcall == nil,
      useful = string.upper('abc') .. table.concat({'x','y'}) .. utf8.char(65),
      integer = math.max(1,2)}
  end,
  native_pattern = function(path)
    -- Retain the reviewer's pathological expressions, including the tiny find input.
    local pattern = 'a*a*a*a*a*b'
    if path == 'find' then return string.find(repeat_byte('a',100),pattern) ~= nil end
    if path == 'method_find' then return (repeat_byte('a',100)):find(pattern) ~= nil end
    if path == 'match' then return string.match(repeat_byte('a',50000),pattern) ~= nil end
    if path == 'method_match' then return (repeat_byte('a',50000)):match(pattern) ~= nil end
    if path == 'gmatch' then
      for _ in string.gmatch(repeat_byte('a',50000),pattern) do end
      return true
    end
    if path == 'method_gmatch' then
      for _ in (repeat_byte('a',50000)):gmatch(pattern) do end
      return true
    end
    if path == 'gsub' then return string.gsub(repeat_byte('a',50000),pattern,'x') end
    if path == 'method_gsub' then return (repeat_byte('a',50000)):gsub(pattern,'x') end
    error('unknown pattern probe')
  end,
  native_count = function(path)
    -- Empty input/output does not bound these native numeric-range loops.
    local count = 1152921504606846976
    if path == 'rep' then return string.rep('',count,'') end
    if path == 'method_rep' then return (''):rep(count,'') end
    if path == 'move' then return table.move({},1,count,1) end
    error('unknown count probe')
  end,
  native_table = function(path)
    -- Tiny ordinary inputs prove capability exposure without a large native loop.
    local values = {'b','a'}
    if path == 'insert' then table.insert(values,1,'c'); return true end
    if path == 'remove' then table.remove(values,1); return true end
    if path == 'sort' then table.sort(values); return true end
    error('unknown table probe')
  end,
  tables = function()
    local allowed = {concat=true,create=true,pack=true,unpack=true}
    local count = 0
    local valid = true
    for name, helper in pairs(table) do
      if not allowed[name] or type(helper) ~= 'function' then valid = false end
      count = count + 1
    end
    local created = table.create(2,1)
    created[1], created[2], created.key = 'a', 'b', true
    local packed = table.pack('a',nil,'c')
    local a,b,c = table.unpack(packed,1,3)
    return {
      allowlist = valid and count == 4,
      absent = table.insert == nil and table.remove == nil and table.sort == nil and table.move == nil,
      concat = table.concat(created,',') == 'a,b',
      create = created.key and #created == 2,
      pack = packed.n == 3 and packed[1] == 'a' and packed[2] == nil and packed[3] == 'c',
      unpack = a == 'a' and b == nil and c == 'c',
    }
  end,
  strings = function()
    local packed = string.pack('>I4', 16909060)
    local unpacked, next_byte = string.unpack('>I4', packed)
    local repeated = ''
    for i=1,3 do
      if i > 1 then repeated = repeated .. '-' end
      repeated = repeated .. 'ab'
    end
    return {
      bytes = string.char(string.byte('abc',2)) == 'b',
      slice = ('abcdef'):sub(2,4) == 'bcd',
      repeat_bytes = repeated == 'ab-ab-ab',
      case = ('AbC'):lower() == 'abc' and ('AbC'):upper() == 'ABC',
      reverse = ('abc'):reverse() == 'cba',
      length = string.len('a\0b') == 3,
      binary = packed == '\1\2\3\4' and unpacked == 16909060
        and next_byte == 5 and string.packsize('>I4') == 4,
    }
  end,
  hash_value = function(input) return engine.hash_value(input.domain, input.value) end,
  file_digest = function(input) return engine.file_digest(input) end,
  fragment_digest = function(input) return engine.fragment_digest(input) end,
  containers = function() return engine.array {engine.object {}, engine.array {}, {list='ordinary user key'}} end,
  kinds = function(input) return engine.kind(input) end,
  malformed = function(input)
    if input == 'function' then return function() end end
    if input == 'float' then return 0.5 end
    if input == 'nil' then return nil end
    if input == 'many' then return 1, 2 end
    if input == 'cycle' then local x={}; x.x=x; return x end
    if input == 'sparse' then return engine.array {[2]=true} end
    if input == 'mixed' then return engine.array {true,key=true} end
    if input == 'key' then return {[1]=true} end
    if input == 'meta' then return setmetatable({}, {}) end
  end,
  finalizer = function()
    if setmetatable == nil then return true end
    local x = setmetatable({}, {__gc=function() while true do end end})
    return true
  end,
  caught_quota = function()
    local ok = pcall(function() while true do end end)
    return ok
  end,
  repeated_quota = function()
    while true do pcall(function() while true do end end) end
  end,
  nested_quota = function()
    return pcall(function()
      return pcall(function() while true do end end)
    end)
  end,
  handler_quota = function()
    xpcall(function() while true do end end, function() while true do end end)
    return true
  end,
  protected_calls = function()
    local checks = {}
    local none = table.pack(pcall(function() end))
    checks.none = (none.n == 1 and none[1] == true)
    local many = table.pack(pcall(function(a,b) return a,nil,b end, 'a', 'b'))
    checks.many = (many.n == 4 and many[1] and many[2] == 'a' and many[3] == nil and many[4] == 'b')
    local marker = {}
    local failed = table.pack(pcall(function() error(marker) end))
    checks.failed = (failed.n == 2 and failed[1] == false and failed[2] == marker)
    local nil_error = table.pack(pcall(error, nil))
    checks.nil_error = (nil_error.n == 2 and nil_error[1] == false and nil_error[2] == '<no error object>')
    local bad = table.pack(pcall(false))
    checks.bad = (bad.n == 2 and bad[1] == false and type(bad[2]) == 'string')
    local missing = table.pack(pcall(pcall))
    checks.missing = (missing.n == 2 and missing[1] == false)
    local nested = table.pack(pcall(function()
      return pcall(function() return pcall(function() error(marker) end) end)
    end))
    checks.nested = (nested.n == 4 and nested[1] and nested[2] and nested[3] == false and nested[4] == marker)
    return checks
  end,
  repair = function()
    local x = {bad=function() end}
    local ok = pcall(engine.hash_value, 'work_v1', x)
    if ok then error('expected refusal') end
    x.bad = nil
    engine.hash_value('work_v1', x)
    return x
  end,
  escaping = function(mode)
    local _, old = pcall(engine.fragment_digest, string.char(255))
    if mode == 'loop' then while true do end end
    if mode == 'memory' then return repeat_byte('x', 600 * 1024 * 1024) end
    if mode == 'other' then error('later error') end
    pcall(engine.hash_value, 'work_v1', function() end)
    error(old)
  end,
  invalid_wrapper = function(input) input.value = true; return input end,
  caught_many = function()
    for i=1,100000 do pcall(engine.fragment_digest, string.char(255)) end
    return true
  end,
  cumulative = function()
    local x = {a=repeat_byte('x', 1024*1024), bad=function() end}
    for i=1,600 do pcall(engine.hash_value, 'work_v1', x) end
    x.bad = nil
    return engine.hash_value('work_v1', x)
  end,
  depth = function() local x={}; for i=1,260 do x={child=x} end; return x end,
  loop = function() while true do end end,
  allocate = function() return repeat_byte('x', 600 * 1024 * 1024) end,
  amplify = function() local x=repeat_byte('x',1024*1024); local t=engine.array {}; for i=1,600 do t[i]=x end; return t end,
  stack = function() local function f() return 1 + f() end; return f() end,
  diagnostic = function() return {error={code='schema',field='job.roles',file='job.lua',details={expected='list'}}} end,
}
