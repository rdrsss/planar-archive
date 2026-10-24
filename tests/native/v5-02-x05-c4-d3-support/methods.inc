namespace planar::engine {
std::array<std::size_t, 23> extension_invocation::private_probe() const {
  auto& vm=*_owner->vm._owner; auto& s=vm.runtime; auto& c=vm.command;
  return {sizeof(command_state),sizeof(command_vm::implementation),sizeof(implementation),sizeof(projection_call_frame),
    s.native_bytes,s.nodes,s.instructions,s.memory,static_cast<std::size_t>(s.push_memo),
    reinterpret_cast<std::size_t>(s.input.get()),static_cast<std::size_t>(s.preserving_aliases),
    static_cast<std::size_t>(c.shape_function),static_cast<std::size_t>(c.extension_execution),
    reinterpret_cast<std::size_t>(c.extension_history.context_identity),reinterpret_cast<std::size_t>(c.extension_history.owner),
    static_cast<std::size_t>(lua_gettop(vm.lua)),static_cast<std::size_t>(c.shape_baseline_ready),
    reinterpret_cast<std::size_t>(c.extension_history.result),static_cast<std::size_t>(s.lua_memory_failed),
    c.shape_phases.size(),reinterpret_cast<std::size_t>(_owner->role_basis.get()),
    reinterpret_cast<std::size_t>(_owner->authenticated_role.get()),static_cast<std::size_t>(_owner->role_capture.has_value())};
}
void extension_invocation::private_limit(int metric) {
 auto& s=_owner->vm._owner->runtime;
 if(metric==0)s.native_bytes=memory_max-1;
 if(metric==1)s.instruction_limit=s.instructions+1;
 if(metric==2)s.nodes=node_max;
}
} // namespace planar::engine
