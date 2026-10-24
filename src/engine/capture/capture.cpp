module planar.engine.capture;
import std;
namespace planar::engine {
capture_session::capture_session()  = default;
capture_session::~capture_session() = default;
std::expected<void, diagnostic> capture_session::add_root(std::string logical, std::string directory) {
  return _native.add_root(std::move(logical), std::move(directory));
}
std::expected<void, diagnostic> capture_session::add_root(std::string logical, std::string directory,
                                                          retained_root_receipt receipt) {
  return _native.add_root(std::move(logical), std::move(directory), std::move(receipt));
}
std::expected<void, diagnostic> capture_session::add_input(std::string logical, std::string file) {
  return _native.add_input(std::move(logical), std::move(file));
}
std::expected<void, diagnostic> capture_session::add_input(std::string logical, std::string file, retained_file_receipt receipt) {
  return _native.add_input(std::move(logical), std::move(file), std::move(receipt));
}
std::expected<captured_file, diagnostic> capture_session::read(std::string_view root, std::string_view path,
                                                               retained_file_receipt receipt, bool text) {
  return _native.read(root, path, std::move(receipt), text);
}
std::expected<captured_file, diagnostic> capture_session::read(std::string_view root, std::string_view path, bool text) {
  return _native.read(root, path, text);
}
std::expected<void, diagnostic> capture_session::tree(std::string_view root, std::vector<std::string> exclusions) {
  return _native.tree(root, std::move(exclusions));
}
std::expected<capture_snapshot, diagnostic> capture_session::finish() {
  return _native.finish();
}
std::expected<acceptance_capture, diagnostic> capture_session::finish_for_acceptance() {
  return _native.finish_for_acceptance();
}
std::expected<capture_topology, diagnostic> capture_session::directories() const {
  return _native.directories();
}
void capture_session::before_verification(std::function<void()> callback) {
  _native.before_verification(std::move(callback));
}
std::expected<declaration, diagnostic> capture_session::load(std::string_view path) {
  auto input = read("workspace", path, true);
  if (!input)
    return std::unexpected(input.error());
  auto result = load_declaration(input->bytes);
  if (!result)
    return std::unexpected(result.error());
  auto slash  = path.rfind('/');
  auto parent = slash == std::string_view::npos ? std::string{} : std::string(path.substr(0, slash + 1));
  for (const auto& reference : result->references) {
    if (!valid_path(reference))
      return std::unexpected(diagnostic{"path_denied", "Invalid text reference path"});
    auto captured = read("workspace", parent + reference, true);
    if (!captured)
      return std::unexpected(captured.error());
  }
  return result;
}
} // namespace planar::engine
