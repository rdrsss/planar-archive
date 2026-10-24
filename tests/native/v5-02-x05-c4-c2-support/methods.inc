namespace planar::engine {
std::array<std::size_t, 19> extension_invocation::private_probe() const {
  auto& vm = *_owner->vm._owner;
  auto& s  = vm.runtime;
  auto& c  = vm.command;
  return {sizeof(command_state),
          sizeof(command_vm::implementation),
          sizeof(implementation),
          sizeof(projection_call_frame),
          s.native_bytes,
          s.nodes,
          s.instructions,
          s.memory,
          static_cast<std::size_t>(s.push_memo),
          reinterpret_cast<std::size_t>(s.input.get()),
          static_cast<std::size_t>(s.preserving_aliases),
          static_cast<std::size_t>(c.shape_function),
          static_cast<std::size_t>(c.extension_execution),
          reinterpret_cast<std::size_t>(c.extension_history.context_identity),
          reinterpret_cast<std::size_t>(c.extension_history.owner),
          static_cast<std::size_t>(lua_gettop(vm.lua)),
          static_cast<std::size_t>(c.shape_baseline_ready),
          reinterpret_cast<std::size_t>(c.extension_history.result),
          static_cast<std::size_t>(s.lua_memory_failed)};
}
} // namespace planar::engine
