module planar.engine.acceptance.intent;
namespace planar::engine::acceptance {
namespace {
auto refuse(preparation_error_code code) {
  return std::unexpected(preparation_error{code});
}
bool absolute_normal(std::string_view argument) {
  if (argument.empty() || argument.front() != '/' || !valid_text(argument))
    return false;
  std::filesystem::path path(argument);
  return path.is_absolute() && path.lexically_normal().native() == argument;
}
} // namespace
request_intent::request_intent(std::shared_ptr<const request_data> data) : _data(std::move(data)) {
}
std::expected<request_intent, preparation_error> request_intent::create(std::string_view request_id, std::string_view actor,
                                                                        std::optional<std::int64_t> expected_head,
                                                                        std::string_view            workspace_argument,
                                                                        std::string_view            registry_argument) {
  // Bound every caller view before allocating paths or request strings. The shared
  // record encoder enforces the aggregate 64 MiB framing ceiling as well.
  if (request_id.size() > 128 || actor.size() > 256 || workspace_argument.size() > 64 * 1024 * 1024 ||
      registry_argument.size() > 64 * 1024 * 1024)
    return refuse(preparation_error_code::resource_limit);
  if (expected_head && *expected_head <= 0)
    return refuse(preparation_error_code::invalid_request);
  auto                  head                  = expected_head ? std::to_string(*expected_head) : "none";
  constexpr std::size_t framing_and_operation = 14 + 1 + 5 * 8 + 4;
  if (workspace_argument.size() + registry_argument.size() + actor.size() + head.size() >
      64 * 1024 * 1024 - framing_and_operation)
    return refuse(preparation_error_code::resource_limit);
  if (request_id.empty() ||
      !std::ranges::all_of(request_id,
                           [](unsigned char c) {
                             return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' ||
                                    c == '.' || c == ':' || c == '-';
                           }) ||
      actor.empty() || !valid_text(actor) || !absolute_normal(workspace_argument) || !absolute_normal(registry_argument))
    return refuse(preparation_error_code::invalid_request);
  history::request_record fields{"load", std::string(actor), std::move(head), std::string(workspace_argument),
                                 std::string(registry_argument)};
  auto                    encoded = history::encode_record(fields);
  if (!encoded)
    return refuse(encoded.error().code == history::record_error_code::resource_limit ? preparation_error_code::resource_limit
                                                                                     : preparation_error_code::invalid_request);
  return request_intent(std::make_shared<const request_data>(
      request_data{std::string(request_id), std::move(fields), expected_head, std::move(*encoded)}));
}
std::expected<request_intent, preparation_error> request_intent::create_v2(std::string_view request_id, std::string_view actor,
                                                                           std::optional<std::int64_t>     expected_head,
                                                                           std::string_view                workspace_argument,
                                                                           std::string_view                registry_argument,
                                                                           std::optional<std::string_view> profile_argument) {
  if (profile_argument && profile_argument->size() > 64 * 1024 * 1024)
    return refuse(preparation_error_code::resource_limit);
  if (profile_argument && !absolute_normal(*profile_argument))
    return refuse(preparation_error_code::invalid_request);
  auto legacy = create(request_id, actor, expected_head, workspace_argument, registry_argument);
  if (!legacy)
    return std::unexpected(legacy.error());
  auto data     = *legacy->_data;
  data.version2 = history::request_v2{
      data.fields.operation,         data.fields.actor,
      data.fields.expected_head,     data.fields.workspace_argument,
      data.fields.registry_argument, profile_argument ? std::optional{std::string(*profile_argument)} : std::nullopt};
  auto encoded = history::encode_request_v2(*data.version2);
  if (!encoded)
    return refuse(encoded.error().code == history::record_error_code::resource_limit ? preparation_error_code::resource_limit
                                                                                     : preparation_error_code::invalid_request);
  data.encoded = std::move(*encoded);
  return request_intent(std::make_shared<const request_data>(std::move(data)));
}
std::expected<std::shared_ptr<const request_data>, preparation_error> request_intent::data() const {
  if (!_data)
    return refuse(preparation_error_code::moved_from);
  return _data;
}
} // namespace planar::engine::acceptance
