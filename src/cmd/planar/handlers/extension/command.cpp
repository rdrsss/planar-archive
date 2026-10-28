module planar.cmd.planar.handlers.extension;
import std;
import planar.cmd.invocation;
namespace planar::cmd::handlers {
namespace {
// X07-ANCHOR:response-ceiling
constexpr std::size_t response_ceiling = 16777215;

response history_failed(const engine::history::history_read_error& error) {
  using engine::history::history_read_code;
  if (error.code == history_read_code::storage)
    return detail::failed(detail::ledger_error(error.storage));
  if (error.code == history_read_code::corrupt)
    return detail::failed(detail::ledger_error({engine::ledger::format_error_code::corrupt, ""}));
  const auto code = error.code == history_read_code::revision_not_found  ? "revision_not_found"
                    : error.code == history_read_code::request_not_found ? "request_not_found"
                    : error.code == history_read_code::event_not_found   ? "event_not_found"
                                                                         : "unsupported_interpretation";
  return detail::failed(detail::native_error({code, "Recorded history view is unavailable"}));
}
response registration_refused(const engine::registration_error& error) {
  if (error.detail.code == "usage")
    return detail::extension_usage_failed();
  if (error.detail.code == "resource_limit")
    return detail::extension_resource_failed(error.detail);
  return detail::registration_failed(error.detail);
}
} // namespace
response extension_command(engine::profile_invocation&& owner, std::string_view command,
                           std::span<const engine::extension_parsed_option> options, const invocation_snapshot& snapshot) {
  try {
    auto extension = std::move(owner).into_extension();
    if (!extension)
      return registration_refused(extension.error());
    std::optional<std::string_view> cwd;
    std::optional<std::string_view> workspace;
    if (snapshot.cwd)
      cwd = std::string_view(*snapshot.cwd);
    if (snapshot.workspace_directory)
      workspace = std::string_view(*snapshot.workspace_directory);
    if (auto admitted = extension->admit(command, options, cwd, workspace); !admitted)
      return registration_refused(admitted.error());
    if (auto verified = extension->verify(); !verified)
      return registration_refused(verified.error());
    // History is prepared lazily by the retained read service from the same admitted owner, exactly once.
    auto executed = extension->execute();
    if (!executed) {
      return std::visit(
          [](const auto& error) -> response {
            using T = std::decay_t<decltype(error)>;
            if constexpr (std::is_same_v<T, engine::registration_error>)
              return registration_refused(error);
            else if constexpr (std::is_same_v<T, engine::ledger::format_error>)
              return detail::failed(detail::ledger_error(error));
            else if constexpr (std::is_same_v<T, engine::history::history_read_error>)
              return history_failed(error);
            else
              return detail::semantic_failed(error.error);
          },
          executed.error());
    }
    const auto& result = *executed;
    if (!result || result->kind != engine::value_kind::record)
      return detail::extension_protocol_failed();
    const auto ok = result->fields.find("ok");
    if (ok == result->fields.end() || !ok->second || ok->second->kind != engine::value_kind::boolean)
      return detail::extension_protocol_failed();
    if (!ok->second->boolean) {
      const auto error = result->fields.find("error");
      return detail::semantic_failed(error == result->fields.end() ? nullptr : error->second);
    }
    const auto value = result->fields.find("value");
    if (value == result->fields.end())
      return detail::extension_protocol_failed();
    // X07-ANCHOR:before-result-seal
    // Late-slot receipts are rechecked once more before the response is released.
    if (auto verified = extension->verify(); !verified)
      return registration_refused(verified.error());
    // X07-ANCHOR:after-final-verify
    auto encoded = engine::codec_encode(value->second, {response_ceiling, 256, 4000000});
    if (!encoded) {
      auto error = encoded.error().detail;
      if (error.code == "resource_limit" && error.limit == "output_bytes")
        error.maximum = 16777216;
      return detail::failed(detail::native_error(error));
    }
    return {0, *encoded + "\n", ""};
  } catch (...) {
    return {4, "", ""};
  }
}
} // namespace planar::cmd::handlers
