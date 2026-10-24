"""Isolated reversible observations of real semantic phases and production codec hooks."""
from pathlib import Path
import hashlib,json,shlex,subprocess,sys,os,time
root=Path(__file__).resolve().parents[3];build=root/'build/rewrite-debug';support=Path(__file__).resolve().parent
out=root/'tests/native/v5-02-x05-b4-b-evidence'/sys.argv[1];out.mkdir()
entries=json.loads((build/'compile_commands.json').read_text());commands=[]
def run(label,args,mode=None,expected=0):
 env=dict(os.environ,R01_MODE=mode or 'success');t=time.time()
 p=subprocess.run(list(map(str,args)),cwd=build,capture_output=True,text=True,env=env)
 (out/(label+'.log')).write_text(p.stdout+p.stderr);commands.append(dict(label=label,argv=list(map(str,args)),mode=mode,exit=p.returncode,seconds=time.time()-t));(out/'commands.json').write_text(json.dumps(commands,indent=2)+'\n');print(label,p.returncode,flush=True)
 assert p.returncode==expected,(label,(p.stdout+p.stderr)[-3000:])
def snapshot():
 return {str(p.relative_to(build)):hashlib.sha256(p.read_bytes()).hexdigest() for p in build.rglob('*') if p.is_file() and p.suffix in {'.pcm','.o','.a'}}
def compile(label,suffix,path):
 cmd=shlex.split(next(e['command'] for e in entries if e['file'].endswith(suffix)))
 def expand(args):
  result=[]
  for a in args:result.extend(expand(shlex.split((build/a[1:]).read_text()))) if a.startswith('@') else result.append(a)
  return result
 cmd=expand(cmd);cmd[cmd.index('-c')+1]=str(path);cmd[cmd.index('-o')+1]=str(out/(label+'.o'))
 cmd=[('-fmodule-output='+str(out/(label+'.pcm'))) if a.startswith('-fmodule-output=') else a for a in cmd]
 if '-MF' in cmd:cmd[cmd.index('-MF')+1]=str(out/(label+'.d'))
 assert all(not a.startswith('@') for a in cmd)
 before=snapshot();run(label+'-compile',cmd);after=snapshot();assert before==after
 (out/(label+'-shared-stability.json')).write_text(json.dumps(dict(entries=len(before),membership_and_hashes_unchanged=True,hashes=before),indent=2)+'\n');return out/(label+'.o')
base=(root/'src/engine/runtime/runtime.cpp').read_text()
helper=r'''
void r01_observe(lua_State* l,bool final) {
 int top=lua_gettop(l);
 for(auto& e:commands(l).entries) {lua_rawgeti(l,LUA_REGISTRYINDEX,e.reference);r01_reference(e.reference,lua_type(l,-1),lua_topointer(l,-1));lua_pop(l,1);}
 lua_rawgeti(l,LUA_REGISTRYINDEX,external_environment_index);r01_reference(external_environment_index,lua_type(l,-1),lua_topointer(l,-1));lua_pop(l,1);
 for(auto ref:commands(l).shape_phases) {lua_rawgeti(l,LUA_REGISTRYINDEX,ref);r01_reference(ref,lua_type(l,-1),lua_topointer(l,-1));lua_pop(l,1);}
 for(auto& e:commands(l).external_entries)for(int ref:{e.descriptor_reference,e.function_reference}){lua_rawgeti(l,LUA_REGISTRYINDEX,ref);r01_reference(ref,lua_type(l,-1),lua_topointer(l,-1));lua_pop(l,1);}
 for(int slot:{tags_index,backing_index,sealed_index}) {lua_rawgeti(l,LUA_REGISTRYINDEX,slot);lua_pushnil(l);while(lua_next(l,-2)){r01_member(slot==tags_index?0:slot==backing_index?1:2,lua_topointer(l,-2),lua_type(l,-1),lua_topointer(l,-1),lua_isinteger(l,-1)?lua_tointeger(l,-1):0);lua_pop(l,1);}lua_pop(l,1);}
 lua_settop(l,top);lua_gc(l,LUA_GCCOLLECT);
 std::size_t retained_capacity=0;
 for(int slot:{tags_index,backing_index,sealed_index}) {
   lua_rawgeti(l,LUA_REGISTRYINDEX,slot);auto table=static_cast<const Table*>(lua_topointer(l,-1));
   auto bytes=sizenode(table)*sizeof(Node);retained_capacity+=bytes;
   std::println("capacity slot={} hash={} array={} node_bytes={}",slot,sizenode(table),table->asize,bytes);
   lua_pop(l,1);
 }
 auto& s=context(l);r01_boundary(final,s.semantic_active,commands(l).shape_baseline_ready,s.memory,s.instructions,s.nodes,s.native_bytes,retained_capacity);
}
'''
subs=[('      auto phase = [&](std::size_t index, value_ptr input) -> value_ptr {', '      auto phase = [&](std::size_t index, value_ptr input) -> value_ptr {\n        auto mode=std::string_view(std::getenv("R01_MODE"));\n        if(mode=="nodes" || (index==3 && mode=="selection-nodes"))s.nodes=node_max;\n        if(index==3 && mode=="selection-native")s.native_bytes=memory_max;\n        if(index==5 && mode=="render-native")s.native_bytes=memory_max;\n        if(index==5 && mode=="render-nodes")s.nodes=node_max;'),('#include <cstdlib>','#include <cstdlib>\nextern "C" void r01_watch(int,std::size_t,bool);\nextern "C" void r01_dispatch(int);\nextern "C" void r01_member(int,const void*,int,const void*,long long);\nextern "C" void r01_reference(int,int,const void*);\nextern "C" void r01_boundary(bool,bool,bool,std::size_t,std::size_t,std::size_t,std::size_t,std::size_t);'),
('int execute_shape(lua_State* l) {',helper+'\nint execute_shape(lua_State* l) {\n  int phase=6;for(int i=0;i<6;++i)if(commands(l).shape_function==commands(l).shape_phases[i])phase=i; r01_dispatch(phase);'),
('    return command_vm(std::move(owner));','    r01_observe(owner->lua,false);\n    return command_vm(std::move(owner));'),
('\n            release_shape_temporaries(owner.lua);','\n            release_shape_temporaries(owner.lua);\n            r01_observe(owner.lua,false);'),
('\n          release_shape_temporaries(owner.lua);','\n          release_shape_temporaries(owner.lua);\n          r01_observe(owner.lua,false);'),
('        state& owner;\n        ~semantic_scope() {\n          owner.semantic_active = false;','        state& owner;\n        lua_State* lua;\n        ~semantic_scope() {\n          owner.semantic_active = false;\n          r01_observe(lua,true);'),
('      } semantic_release{s};','      } semantic_release{s,_owner->lua};'),
('\n                                      charge(*static_cast<state*>(owner), bytes);','\n                                      std::println("parser-hook bytes={}",bytes);\n                                 if(std::string_view(std::getenv("R01_MODE"))=="parser")static_cast<state*>(owner)->native_bytes=memory_max;\n                                      charge(*static_cast<state*>(owner), bytes);'),
('[](void* owner, const codec_decode_reservation& capacity) -> std::expected<void, codec_error> {\n                                    try {\n                                      charge(*static_cast<state*>(owner), capacity.total_bytes);','[](void* owner, const codec_decode_reservation& capacity) -> std::expected<void, codec_error> {\n                                    try {\n                                      std::println("conversion-hook bytes={}",capacity.total_bytes);\n                                 if(std::string_view(std::getenv("R01_MODE"))=="conversion")static_cast<state*>(owner)->native_bytes=memory_max;\n                                 charge(*static_cast<state*>(owner), capacity.total_bytes);'),
('      lua_rawsetp(l, LUA_REGISTRYINDEX, slot == tags_index ? &shape_tags_key : &shape_backings_key);','      lua_rawsetp(l, LUA_REGISTRYINDEX, slot == tags_index ? &shape_tags_key : &shape_backings_key);\n      if(slot==tags_index && std::string_view(std::getenv("R01_MODE"))=="snapshot")context(l).memory_limit=context(l).memory;'),
('  auto result = std::realloc(p, size);','  if(s.semantic_active && std::string_view(std::getenv("R01_MODE"))=="lua" && size>old){s.exhausted=true;return nullptr;}\n  if(s.command && s.instruction_limit==instruction_max && s.memory_limit!=memory_max && std::string_view(std::getenv("R01_MODE"))=="snapshot" && size>old){s.exhausted=true;return nullptr;}\n  auto result = std::realloc(p, size);'),
('    if (lua)\n      lua_close(lua);','    if (lua) {lua_close(lua);if(runtime.memory!=0)std::abort();}')]
subs.extend([
('  charge(s, sizeof(std::__shared_ptr_emplace<value, std::allocator<value>>));\n  auto result  = std::make_shared<value>();', '''  constexpr auto bytes=sizeof(std::__shared_ptr_emplace<value,std::allocator<value>>);
  auto mode=std::string_view(std::getenv("R01_MODE"));
  if(mode=="error-node")s.native_bytes=memory_max-bytes+1;
  if(mode=="error-node-exact" && s.native_bytes<memory_max)s.native_bytes=memory_max-bytes;
  charge(s, bytes);
  r01_watch(1,bytes,true);
  auto result  = std::make_shared<value>();
  r01_watch(1,bytes,false);'''),
('  charge(s, sizeof(std::__tree_node<std::__value_type<std::string, value_ptr>, void*>));\n  object.fields.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(std::move(child)));', '''  constexpr auto bytes=sizeof(std::__tree_node<std::__value_type<std::string,value_ptr>,void*>);
  if(std::string_view(std::getenv("R01_MODE"))=="error-map")s.native_bytes=memory_max-bytes+1;
  charge(s, bytes);r01_watch(2,bytes,true);
  object.fields.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(std::move(child)));
  r01_watch(2,bytes,false);'''),
('  charge(s, size);\n  const auto padding = alignment - size % alignment;', '''  const auto padding = alignment - size % alignment;
  if(std::string_view(std::getenv("R01_MODE"))=="error-string")s.native_bytes=memory_max-(size+padding)+1;
  charge(s, size);'''),
('  result->string.reserve(text.size());','''  auto bytes=text.size()<=std::string{}.capacity()?0:text.size()+16-text.size()%16;
  r01_watch(3,bytes,true);result->string.reserve(text.size());r01_watch(3,bytes,false);'''),
('    if (s.semantic_active)\n      charge(s, sizeof(std::__shared_ptr_emplace<value, std::allocator<value>>));\n    s.codec_result = codec_text(std::move(*result));', '''    if (s.semantic_active) {
      constexpr auto bytes=sizeof(std::__shared_ptr_emplace<value,std::allocator<value>>);
      if(std::string_view(std::getenv("R01_MODE"))=="wrapper")s.native_bytes=memory_max-bytes+1;
      charge(s, bytes);r01_watch(4,bytes,true);
    }
    s.codec_result = codec_text(std::move(*result));
    if(s.semantic_active)r01_watch(4,sizeof(std::__shared_ptr_emplace<value,std::allocator<value>>),false);''')])
subs.extend([
('    if (s.semantic_active) {\n      reserve_hash_payload(s, input);', '    auto hash_before=s.native_bytes;\n    if (s.semantic_active) {\n      if(std::string_view(std::getenv("R01_MODE"))=="hash-remaining")s.native_bytes=memory_max;\n      reserve_hash_payload(s, input);'),
('    result = hash_value({bytes, n}, input);', '    if(s.semantic_active)r01_hash(0,hash_before,s.native_bytes,input.get(),n);\n    result = hash_value({bytes, n}, input);'),
('  } else {\n    if (s.semantic_active) {\n      // file_digest', '  } else {\n    auto hash_before=s.native_bytes;\n    if (s.semantic_active) {\n      if(std::string_view(std::getenv("R01_MODE"))=="hash-remaining")s.native_bytes=memory_max;\n      // file_digest'),
('    result = operation == 1 ? file_digest({bytes, n}) : fragment_digest({bytes, n});', '    if(s.semantic_active)r01_hash(static_cast<int>(operation),hash_before,s.native_bytes,nullptr,n);\n    result = operation == 1 ? file_digest({bytes, n}) : fragment_digest({bytes, n});')
])
subs[1]=(subs[1][0],subs[1][1]+'\nextern "C" void r01_hash(int,std::size_t,std::size_t,const void*,std::size_t);\nextern "C" void r01_hash_end();')
subs.extend([
('    result = hash_value({bytes, n}, input);', '    result = hash_value({bytes, n}, input);\n    if(s.semantic_active)r01_hash_end();'),
('    result = operation == 1 ? file_digest({bytes, n}) : fragment_digest({bytes, n});', '    result = operation == 1 ? file_digest({bytes, n}) : fragment_digest({bytes, n});\n    if(s.semantic_active)r01_hash_end();'),
('void digest_native(lua_State* l) {', 'extern "C" bool r01_direct_hash(const value_ptr* input) {state s;reserve_hash_payload(s,*input);reserve_hash_frame(s,9);r01_hash(0,0,s.native_bytes,input->get(),9);auto result=hash_value("adversary",*input);r01_hash_end();return result.has_value();}\nvoid digest_native(lua_State* l) {')
])
subs.append(('                         charge(*static_cast<state*>(owner), reservation.total_bytes);', '                         if(std::string_view(std::getenv("R01_MODE"))=="render-codec")static_cast<state*>(owner)->native_bytes=memory_max;\n                         charge(*static_cast<state*>(owner), reservation.total_bytes);'))
subs.append(('        auto normalized = phase(4, render);', '        auto normalized = std::string_view(std::getenv("R01_MODE"))=="preflight-red" ? record({{"value",record({{"request",render},{"selection",render}})}}) : phase(4, render);'))
# Reconstruct only the old inline metadata layout, never an old implementation.
start=base.index('struct command_state {');end=base.index('\n};',start)+3
old_layout=base[start:end].replace('struct command_state {','struct command_state_before {').replace('std::array<int, 6>','std::array<int, 4>').replace('LUA_NOREF, LUA_NOREF, LUA_NOREF, LUA_NOREF, LUA_NOREF, LUA_NOREF','LUA_NOREF, LUA_NOREF, LUA_NOREF, LUA_NOREF')
subs.append((base[start:end],base[start:end]+'\n'+old_layout))
subs.append(('namespace {\nvoid check_profile(const profile_closure& profile) {','namespace {\nstruct implementation_before {state runtime;command_state_before command;lua_State* lua;};\nstatic_assert(sizeof(command_state)-sizeof(command_state_before)==2*sizeof(int));\nstatic_assert(sizeof(command_vm::implementation)-sizeof(implementation_before)==2*sizeof(int));\nvoid check_profile(const profile_closure& profile) {\n std::println("layout command_before={} command_after={} owner_before={} owner_after={} charged_delta={}",sizeof(command_state_before),sizeof(command_state),sizeof(implementation_before),sizeof(command_vm::implementation),2*sizeof(int));'))
subs.append(('      // Bound the complete render response, including manifest and envelope, using','      if(std::string_view(std::getenv("R01_MODE"))=="render-output-limit")rendered=command_record({{"value",command_text(std::string(16*1024*1024,\'x\'))}});\n      // Bound the complete render response, including manifest and envelope, using'))
text=base
for a,b in subs:
 n=text.count(a);assert n==1,(a,n);text=text.replace(a,b)
reverse=text
for a,b in reversed(subs):reverse=reverse.replace(b,a)
assert reverse==base
(out/'runtime.cpp').write_text(text);(out/'transformations.json').write_text(json.dumps(dict(original_sha256=hashlib.sha256(base.encode()).hexdigest(),copy_sha256=hashlib.sha256(text.encode()).hexdigest(),reverse_identical=True,substitutions=[dict(before=a,after=b)for a,b in subs]),indent=2)+'\n')
runtime=compile('runtime','/runtime/runtime.cpp',out/'runtime.cpp');main=compile('semantic','/runtime/external_registration.t.cpp',support/'semantic.cpp')
link=shlex.split(subprocess.check_output(['ninja','-t','commands','planar_runtime_tests'],cwd=build,text=True).splitlines()[-1].split(' && ')[1]);libs=link[link.index('src/engine/libplanar_runtime.a'):]
run('semantic-link',[link[0],'-stdlib=libc++','-arch','arm64','-L/opt/homebrew/opt/llvm/lib/c++','-Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++',main,runtime,*libs,'-o',out/'semantic'])
for mode in ['success','semantic','execution','parser','conversion','instructions','snapshot','lua','nodes','native','error','error-node','error-node-exact','error-map','error-string','wrapper','selection-nodes','selection-native','render-native','render-nodes','render-codec','render-output-limit','hash-remaining']:
 run('semantic-'+mode,[out/'semantic','[r01-semantic]','--reporter','junit','--out',out/(mode+'.xml')],mode)

run('preflight',[out/'semantic','[b4b-preflight]','--reporter','junit','--out',out/'preflight.xml'],'preflight')
run('preflight-red',[out/'semantic','[b4b-preflight]','--reporter','junit','--out',out/'preflight-red.xml'],'preflight-red',expected=42)
