module planar.engine.acceptance.load;
import planar.engine.capture;
import planar.engine.capture.host_path;
import planar.engine.acceptance.source_capture;
import planar.engine.canonical;
namespace planar::engine::acceptance {
namespace {
auto refuse(load_error_code code) {
  return std::unexpected(load_error{code});
}
auto captured_error(const diagnostic& e) {
  return std::unexpected(load_error{e.code == "resource_limit" ? load_error_code::resource_limit : load_error_code::capture, e});
}
load_error mapped_error(source_error e) {
  load_error_code code = load_error_code::capture;
  switch (e.code) {
  case source_error_code::invalid_state:
    code = load_error_code::invalid_state;
    break;
  case source_error_code::invalid_path:
    code = load_error_code::invalid_path;
    break;
  case source_error_code::invalid_route:
    code = load_error_code::invalid_route;
    break;
  case source_error_code::capture:
    code = load_error_code::capture;
    break;
  case source_error_code::resource_limit:
    code = load_error_code::resource_limit;
    break;
  }
  return load_error{code, std::move(e.detail)};
}
} // namespace
/// Sole owner of the source capture; never shared between threads.
struct load_capture::implementation {
  source_capture source; ///< Terminal source capture owner, moved in from the factory.
  /// @brief Take ownership of an opened source capture without reading any content.
  /// @param state Successfully opened owner, moved into this implementation.
  explicit implementation(source_capture state) : source(std::move(state)) {
  }
};
load_capture::load_capture(std::unique_ptr<implementation> state) : _impl(std::move(state)) {
}
load_capture::~load_capture()                             = default;
load_capture::load_capture(load_capture&& other) noexcept = default;
load_capture& load_capture::operator=(load_capture&& other) noexcept {
  if (this != &other)
    _impl = std::move(other._impl);
  return *this;
}
std::expected<load_capture, load_error> load_capture::open_workspace(std::string_view workspace, std::string_view registry) {
  auto source = source_capture::open_embedded(workspace, registry);
  if (!source)
    return std::unexpected(mapped_error(std::move(source.error())));
  return load_capture(std::make_unique<implementation>(std::move(*source)));
}
std::expected<value_ptr, load_error> load_capture::workspace() {
  if (!_impl) {
    return refuse(load_error_code::invalid_state);
  }
  auto result = _impl->source.workspace();
  if (!result) {
    _impl.reset();
    return std::unexpected(mapped_error(std::move(result.error())));
  }
  return *result;
}
std::expected<value_ptr, load_error> load_capture::capture(const value_ptr& route) {
  auto state = std::move(_impl);
  if (!state)
    return refuse(load_error_code::invalid_state);
  auto result = state->source.capture(route);
  if (!result)
    return std::unexpected(mapped_error(std::move(result.error())));
  _impl = std::move(state);
  return *result;
}
std::expected<prepared_acceptance, load_error> load_capture::prepare(const value_ptr& bundle, const value_ptr& semantic,
                                                                     std::span<const history::tree_file> stdlib_files) {
  auto state = std::move(_impl);
  if (!state || !state->source.sealed_ready())
    return refuse(load_error_code::invalid_state);
  auto& source = state->source;
  auto  stdlib = history::verify_stdlib(stdlib_files);
  if (!stdlib)
    return refuse(stdlib.error().code == history::provenance_error_code::resource_limit
                      ? load_error_code::resource_limit
                      : load_error_code::invalid_interpretation);
  std::string digest;
  for (auto b : stdlib->digest)
    digest += std::format("{:02x}", b);
  auto encoded = history::encode_interpretation(bundle, semantic, digest);
  if (!encoded)
    return refuse(encoded.error().code == history::interpretation_error_code::resource_limit
                      ? load_error_code::resource_limit
                      : load_error_code::invalid_interpretation);
  auto paths = verify_load_correspondence(bundle, source);
  if (!paths)
    return paths.error().code == correspondence_error_code::capture ? captured_error(paths.error().detail)
                                                                    : refuse(load_error_code::invalid_interpretation);
  std::vector<std::string_view> views(paths->begin(), paths->end());
  auto                          sealed = source.release_sealed();
  auto                          prepared =
      prepared_acceptance::create(std::move(*sealed), views, stdlib_files, encoded->encoded, encoded->versions_encoded);
  if (!prepared)
    return std::unexpected(load_error{prepared.error().code == preparation_error_code::resource_limit
                                          ? load_error_code::resource_limit
                                          : load_error_code::preparation,
                                      {},
                                      prepared.error()});
  return std::move(*prepared);
}
} // namespace planar::engine::acceptance
