module planar.engine.acceptance.prepared;
namespace planar::engine::acceptance {
namespace {
constexpr std::size_t maximum = 16 * 1024 * 1024;
auto                  refuse(preparation_error_code code) {
  return std::unexpected(preparation_error{code});
}
preparation_error_code provenance_code(const history::provenance_error& error, preparation_error_code fallback) {
  return error.code == history::provenance_error_code::resource_limit ? preparation_error_code::resource_limit : fallback;
}
bool absolute_normal(std::string_view argument) {
  if (argument.empty() || argument.front() != '/' || !valid_text(argument))
    return false;
  std::filesystem::path path(argument);
  return path.is_absolute() && path.lexically_normal().native() == argument;
}
std::optional<std::uint32_t> mode(std::string_view spelling) {
  if (spelling == "0644")
    return 420;
  if (spelling == "0755")
    return 493;
  return {};
}
std::string hex(const history::record_digest& digest) {
  constexpr std::string_view digits = "0123456789abcdef";
  std::string                result;
  for (auto byte : digest) {
    result += digits[byte >> 4];
    result += digits[byte & 15];
  }
  return result;
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
/// Single owner binding immutable prepared values to their exact retained native receipts.
struct prepared_acceptance::implementation {
  acceptance_capture                     capture; ///< Original sealed capability; never substituted during verification.
  std::shared_ptr<const acceptance_data> data;    ///< Independently shareable immutable owned data.
};
prepared_acceptance::prepared_acceptance(std::unique_ptr<implementation> state) : _impl(std::move(state)) {
}
prepared_acceptance::~prepared_acceptance()                                    = default;
prepared_acceptance::prepared_acceptance(prepared_acceptance&& other) noexcept = default;
prepared_acceptance& prepared_acceptance::operator=(prepared_acceptance&& other) noexcept {
  if (this != &other)
    _impl = std::move(other._impl);
  return *this;
}
std::expected<prepared_acceptance, preparation_error>
prepared_acceptance::create(acceptance_capture capture, std::span<const std::string_view> source_paths,
                            std::span<const history::tree_file> stdlib_files, std::string_view interpretation,
                            std::string_view versions) {
  using namespace history;
  if (source_paths.size() > 4096 || stdlib_files.size() > 4096 || interpretation.size() > maximum || versions.size() > maximum)
    return refuse(preparation_error_code::resource_limit);
  if (source_paths.empty())
    return refuse(preparation_error_code::invalid_source);
  auto snapshot = capture.snapshot();
  if (!snapshot)
    return refuse(preparation_error_code::moved_from);
  std::set<std::string_view>  selected;
  std::vector<tree_file_view> views;
  for (auto path : source_paths) {
    if (path.size() > 1024)
      return refuse(preparation_error_code::resource_limit);
    if (!valid_path(path) || !selected.insert(path).second)
      return refuse(preparation_error_code::invalid_source);
    auto file = std::ranges::find_if(**snapshot, [&](const auto& f) { return f.root == "workspace" && f.path == path; });
    if (file == (*snapshot)->end() || !mode(file->mode))
      return refuse(preparation_error_code::invalid_source);
    views.push_back({file->path, file->bytes, *mode(file->mode)});
  }
  auto source_digest = digest_tree(views, tree_profile::source);
  if (!source_digest)
    return refuse(source_digest.error().code == tree_error_code::resource_limit ? preparation_error_code::resource_limit
                                                                                : preparation_error_code::invalid_source);
  const captured_file* registry_file = nullptr;
  for (const auto& file : **snapshot)
    if (file.root == "registry") {
      if (registry_file || file.path != "input.json")
        return refuse(preparation_error_code::invalid_registry);
      registry_file = &file;
    }
  if (!registry_file || !mode(registry_file->mode))
    return refuse(preparation_error_code::invalid_registry);
  auto registry = prepare_registry(registry_file->bytes, *mode(registry_file->mode));
  if (!registry)
    return refuse(provenance_code(registry.error(), preparation_error_code::invalid_registry));
  // The shared verifier bounds the complete set before owning payloads and
  // reconstructs digest/manifest relationships rather than trusting caller fields.
  auto stdlib = verify_stdlib(stdlib_files);
  if (!stdlib)
    return refuse(provenance_code(stdlib.error(), preparation_error_code::invalid_stdlib));
  auto decoded = decode_interpretation(interpretation, versions);
  if (!decoded)
    return refuse(decoded.error().code == interpretation_error_code::resource_limit ? preparation_error_code::resource_limit
                  : decoded.error().code == interpretation_error_code::unsupported_version
                      ? preparation_error_code::unsupported_version
                      : preparation_error_code::invalid_interpretation);
  if (decoded->versions->fields.at("stdlib_digest")->string != hex(stdlib->digest))
    return refuse(preparation_error_code::invalid_stdlib);
  auto workspace_id = decoded->interpretation->fields.at("bundle")->fields.at("workspace")->fields.at("id")->string;
  std::vector<tree_file> source;
  source.reserve(views.size());
  for (const auto& file : views)
    source.push_back({std::string(file.path), std::string(file.payload), file.mode});
  std::ranges::sort(source, {}, &tree_file::path);
  auto data = std::make_shared<const acceptance_data>(acceptance_data{
      std::move(source), *source_digest, std::move(*registry), std::move(*stdlib), std::move(*decoded), std::move(workspace_id)});
  return prepared_acceptance(std::make_unique<implementation>(implementation{std::move(capture), std::move(data)}));
}
std::expected<std::shared_ptr<const acceptance_data>, preparation_error> prepared_acceptance::data() const {
  if (!_impl)
    return refuse(preparation_error_code::moved_from);
  return _impl->data;
}
std::expected<void, diagnostic> prepared_acceptance::verify() const {
  if (!_impl)
    return std::unexpected(diagnostic{"capture_changed", "Captured inputs changed."});
  return _impl->capture.verify();
}
} // namespace planar::engine::acceptance
