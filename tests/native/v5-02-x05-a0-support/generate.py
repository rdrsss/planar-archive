#!/usr/bin/env python3
"""Reversible X05-A0 verification variants, never product hooks or source changes."""
from pathlib import Path
import hashlib,json,sys
def put(path,text):
 if not path.exists() or path.read_text()!=text:path.write_text(text)
root,out=map(Path,sys.argv[1:]);out.mkdir(parents=True,exist_ok=True)
support=root/'tests/native/v5-02-x05-a0-support'
impl=(root/'src/engine/runtime/runtime.cpp').read_text()
interface=(root/'src/engine/runtime/runtime.cppm').read_text()
commands=(root/'src/stdlib/planar/commands.lua').read_text()
manifest={}
for variant in range(2):
 name=f'x05_a0_{variant}'
 injected_import='import planar.engine.x05_closure_audit;\n'
 verify='''  // Reserve in this active protected C frame, before pushes or native RAII.
  if (!lua_checkstack(l, x05_closure_audit::audit_stack_slots)) {
    context(l).stack_exhausted = true;
    return 0;
  }
  // X05-A0 only: inspect every original root before any proxy freezing.
  bool x05_ok = false;
  lua_pushglobaltable(l);
  {
    auto result = x05_closure_audit::audit_retained(l, lua_gettop(l), LUA_RIDX_LAST + 3);
    x05_ok = result.has_value();
  }
  lua_pop(l, 1);
  if (!x05_ok)
    return luaL_error(l, "x05_lexical_refusal");
  for (const auto& entry : commands(l).entries) {
    lua_rawgeti(l, LUA_REGISTRYINDEX, entry.reference);
    {
      auto result = x05_closure_audit::audit_retained(l, lua_gettop(l), LUA_RIDX_LAST + 3);
      x05_ok = result.has_value();
    }
    lua_pop(l, 1);
    if (!x05_ok)
      return luaL_error(l, "x05_lexical_refusal");
  }
'''
 substitutions=[
 ('module planar.engine.runtime;',f'module planar.engine.{name};',1),
 ('import planar.engine.runtime_source;',f'import planar.engine.{name}_source;\n'+injected_import,1),
 ('namespace planar::engine {',f'namespace planar::engine::{name} {{',1),
 ('int seal_environment(lua_State* l) {\n','int seal_environment(lua_State* l) {\n'+verify,1),
 ('  if (luaL_loadbufferx(l, embedded::source().data(), embedded::source().size(), "trusted", "t"))',
  '  x05_closure_audit::snapshot_curated_natives(l, LUA_RIDX_LAST + 3);\n  if (luaL_loadbufferx(l, embedded::source().data(), embedded::source().size(), "trusted", "t"))',1),
 ('    lua_pushcclosure(l, digest, 1);\n    lua_setfield(l, -2, i == 0 ?',
  '    lua_pushcclosure(l, digest, 1);\n    x05_closure_audit::remember_installed_callback(l, LUA_RIDX_LAST + 3, lua_gettop(l));\n    lua_setfield(l, -2, i == 0 ?',1),
 ('    lua_pushcclosure(l, json_codec, 1);\n    lua_setfield(l, -2, i ? "json_encode" : "json_decode");\n  }\n  lua_pop(l, 1);\n  return 0;\n}\nbool token',
  '    lua_pushcclosure(l, json_codec, 1);\n    x05_closure_audit::remember_installed_callback(l, LUA_RIDX_LAST + 3, lua_gettop(l));\n    lua_setfield(l, -2, i ? "json_encode" : "json_decode");\n  }\n  lua_pop(l, 1);\n  return 0;\n}\nbool token',1)]
 for filename,source,subs in [('runtime.cpp',impl,substitutions),('runtime.cppm',interface,[substitutions[0],substitutions[2]])]:
  generated=source
  for before,after,count in subs:
   assert generated.count(before)==count,(filename,before)
   generated=generated.replace(before,after)
  reverse=generated
  for before,after,count in reversed(subs):
   assert reverse.count(after)==count,(filename,after)
   reverse=reverse.replace(after,before)
  assert reverse==source
  target=out/f'{name}_{filename}';put(target,generated)
  manifest[target.name]=dict(source_sha256=hashlib.sha256(source.encode()).hexdigest(),generated_sha256=hashlib.sha256(generated.encode()).hexdigest(),reverse_identical=True,substitutions=[dict(before=a,after=b,count=c) for a,b,c in subs])
 source=commands
 if variant:
  needle='  local function init_command(invocation,context)'
  assert source.count(needle)==1
  source=source.replace(needle,'  local x05_retained_scalar=0\n'+needle+'\n    x05_retained_scalar=x05_retained_scalar+1')
 put(out/f'commands_{variant}.lua',source)
needle='  local function init_command(invocation,context)'
replacement='  local x05_retained_scalar=0\n'+needle+'\n    x05_retained_scalar=x05_retained_scalar+1'
altered=(out/'commands_1.lua').read_text()
assert altered.count(replacement)==1 and altered.replace(replacement,needle)==commands
manifest['lua']={'variant0_exact_shipped_commands':(out/'commands_0.lua').read_text()==commands,
 'variant1_retained_scalar_write_only':True,'source_sha256':hashlib.sha256(commands.encode()).hexdigest(),
 'altered_sha256':hashlib.sha256(altered.encode()).hexdigest(),'before':needle,'after':replacement,'count':1,'reverse_identical':True}
put(out/'generation.json',json.dumps(manifest,indent=2,sort_keys=True)+'\n')
print('X05-A0: 2 runtime/interface variants forward/reverse exact; variant0 shipped commands exact')
