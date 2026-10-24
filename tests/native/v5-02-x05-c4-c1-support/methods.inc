namespace planar::engine {
std::expected<projection_response, registration_error> command_vm::private_project(std::string_view id, const value_ptr& input) {
  auto&                 s        = _owner->runtime;
  auto&                 c        = _owner->command;
  std::size_t           ordinal  = 0;
  const external_entry* selected = nullptr;
  for (auto& entry : c.external_entries) {
    if (entry.kind != 2)
      continue;
    ++ordinal;
    if (text_field(entry.descriptor, "id") == id) {
      selected = &entry;
      break;
    }
  }
  const std::string field = selected ? std::format("catalog.projections[{}]", ordinal) : "catalog.projections";
  if (c.shape_failure)
    return std::unexpected(*c.shape_failure);
  if (!selected)
    return std::unexpected(registration_error{external_protocol(), field});
  if (text_field(selected->descriptor, "input") != "history_snapshot_v1" || !input || input.get() != c.extension_history.result)
    return std::unexpected(registration_error{external_protocol(), field});
  // Existing exact resolver ran before handler admission. C1 does not authorize
  // call-time service resolution; C2 must own that separate production boundary.
  auto retained = c.pending_descriptor;
  if (!retained || text_field(retained, "id") != id)
    return std::unexpected(registration_error{external_protocol(), field});
  projection_definition definition{text_field(retained, "id"), text_field(retained, "revision"), text_field(retained, "digest")};
  try {
    validate(s, input, 1);
    auto result = call_retained_projection(_owner->lua, selected->function_reference, input);
    if (!result)
      return std::unexpected(registration_error{std::move(result.error()), field});
    return projection_response{std::move(definition), std::move(*result)};
  } catch (const std::bad_alloc&) {
    s.native_memory_failed = true;
    return std::unexpected(registration_error{external_resource(), field});
  } catch (const std::length_error&) {
    s.native_memory_failed = true;
    return std::unexpected(registration_error{external_resource(), field});
  } catch (diagnostic& e) {
    return std::unexpected(registration_error{std::move(e), field});
  }
}
std::expected<projection_response, registration_error> extension_invocation::private_project(std::string_view id) {
  return _owner->vm.private_project(id, _owner->authenticated_history);
}
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
