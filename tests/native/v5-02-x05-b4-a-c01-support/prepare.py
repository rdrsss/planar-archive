from pathlib import Path
import hashlib,json
root=Path(__file__).resolve().parents[3];out=root/'tests/native/v5-02-x05-b4-a-c01-evidence'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
# Reversible test-only native seam: the real freezer freezes supplied model/selection.
# Run_trusted's converter consults the same private backing map for this probe only.
base=(root/'src/engine/runtime/runtime.cpp').read_text()
subs=[('int setup(lua_State* l) {\n  setup_environment(l);','extern "C" const char* review_source(std::size_t*);\nvoid review_read_setup(lua_State*);\nvoid review_freeze(lua_State*);\nint setup(lua_State* l) {\n  setup_environment(l);\n  review_read_setup(l);'),('  auto source = embedded::source();\n  if (luaL_loadbufferx(l, source.data(), source.size(), "trusted", "t"))','  std::size_t size; auto bytes=review_source(&size); auto source=std::string_view(bytes,size);\n  if (luaL_loadbufferx(l, source.data(), source.size(), "trusted", "t"))'),('  push(l, context(l).input.get());\n  lua_call(l, 1, LUA_MULTRET);','  push(l, context(l).input.get());\n  if(context(l).entry=="render_profile_material")review_freeze(l);\n  lua_call(l, 1, LUA_MULTRET);'),('    if (s.command) {\n      lua_rawgeti(l, LUA_REGISTRYINDEX, backing_index);','    if (true) {\n      lua_rawgeti(l, LUA_REGISTRYINDEX, backing_index);')]
helper=r'''
void review_read_setup(lua_State* l) {
  lua_newtable(l);lua_pushvalue(l,-1);lua_rawseti(l,LUA_REGISTRYINDEX,backing_index);
  for(auto item:{std::pair{"rawget",frozen_rawget},{"next",frozen_next}}){lua_pushvalue(l,-1);lua_pushcclosure(l,item.second,1);lua_setglobal(l,item.first);}
  lua_pop(l,1);lua_getglobal(l,"next");lua_pushcclosure(l,frozen_pairs,1);lua_setglobal(l,"pairs");
}
int review_write_control(lua_State* l) {lua_pushboolean(l,true);lua_setfield(l,1,"id");return 0;}
void review_freeze(lua_State* l) {
 int top=lua_gettop(l),input=top;
 lua_newtable(l);int memo=lua_gettop(l);lua_rawgeti(l,LUA_REGISTRYINDEX,backing_index);int backings=lua_gettop(l);lua_newtable(l);int sealed=lua_gettop(l);
 for(auto field:{"model","selected"}){lua_getfield(l,input,field);freeze_graph(l,lua_gettop(l),memo,backings,sealed);lua_pushcfunction(l,review_write_control);lua_pushvalue(l,-2);int status=lua_pcall(l,1,0,0);if(status!=LUA_ERRRUN)luaL_error(l,"frozen_control_failed");lua_pop(l,2);}
 lua_settop(l,top);
}
'''
subs.append(('// Module sealing maps root pairs to the same original registration token used',helper+'\n// Module sealing maps root pairs to the same original registration token used'))
text=base
for a,b in subs:assert text.count(a)==1,a;text=text.replace(a,b)
reverse=text
for a,b in reversed(subs):reverse=reverse.replace(b,a)
assert reverse==base
(out/'runtime.cpp').write_text(text);(out/'transformations.json').write_text(json.dumps(dict(original=sha(root/'src/engine/runtime/runtime.cpp'),copy=hashlib.sha256(text.encode()).hexdigest(),reverse_identical=True,substitutions=[dict(before=a,after=b)for a,b in subs]),indent=2)+'\n')
# Old render bytes are approved B3-C baseline bytes, no replacement implementations.
order=['json','schema','semantic','selection','presentation','observations','render','acceptance','commands']
source='\n'.join((root/('src/stdlib/planar/'+p+'.lua')).read_text()for p in order)
old=(root/'tests/native/v5-02-x05-b4-a-evidence/baseline/src/stdlib/planar/render.lua').read_text();bootstrap=(root/'src/stdlib/planar/bootstrap.lua').read_text()
source+='\nlocal review_current_render=render\n'+old+'\nlocal review_old_render=render\nlocal render=review_current_render\n'+bootstrap.replace('  render = render,','  review_old_render = review_old_render,\n  render = render,')
(out/'review-source.lua').write_text(source);assert ')REVIEW"'not in source
(out/'review-source.inc').write_text('extern "C" const char* review_source(std::size_t* n) { static constexpr char s[]=R"REVIEW('+source+')REVIEW"; *n=sizeof(s)-1;return s;}\n')
