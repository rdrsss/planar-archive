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
value_ptr at(const value_ptr& v, std::string_view key) {
  if (!v)
    return {};
  auto it = v->fields.find(std::string(key));
  return it == v->fields.end() ? value_ptr{} : it->second;
}
bool kind(const value_ptr& v, value_kind k) {
  return v && v->kind == k;
}
bool keys(const value_ptr& v, std::initializer_list<std::string_view> ks) {
  return kind(v, value_kind::record) && v->fields.size() == ks.size() &&
         std::ranges::all_of(ks, [&](auto k) { return bool(at(v, k)); });
}
bool same(const value_ptr& a, const value_ptr& b) {
  if (!a || !b)
    return false;
  auto aa = canonical_encode(a), bb = canonical_encode(b);
  return aa && bb && *aa == *bb;
}
bool same_field(const value_ptr& a, const value_ptr& b, std::string_view key) {
  return same(at(a, key), at(b, key));
}
std::string parent(std::string_view path) {
  auto slash = path.rfind('/');
  return slash == std::string_view::npos ? std::string{} : std::string(path.substr(0, slash + 1));
}
bool path(const value_ptr& v) {
  return kind(v, value_kind::string) && v->string.size() <= 1024 && valid_path(v->string);
}
bool identity(const value_ptr& raw, const value_ptr& normalized) {
  return same_field(raw, normalized, "id") && same_field(raw, normalized, "version");
}
bool routes(const value_ptr& jobs, std::map<std::string, std::string>& out, std::string_view prefix, bool sorted) {
  if (!kind(jobs, value_kind::list) || jobs->items.empty() || jobs->items.size() > 4096)
    return false;
  std::string           previous;
  std::set<std::string> files;
  for (auto& j : jobs->items) {
    auto id = at(j, "id"), file = at(j, "file");
    if (!keys(j, {"id", "file"}) || !kind(id, value_kind::string) || id->string.empty() || id->string.size() > 64 ||
        !path(file) || (sorted && id->string <= previous))
      return false;
    auto resolved = std::string(prefix) + file->string;
    if (resolved.size() > 1024 || !valid_path(resolved) || !files.insert(resolved).second ||
        !out.emplace(id->string, resolved).second)
      return false;
    previous = id->string;
  }
  return true;
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
namespace {
bool references(const value_ptr& v, const std::map<std::string, std::string_view>& files, std::set<std::string>& found,
                std::size_t depth = 0) {
  if (!v || depth > 256)
    return false;
  if (auto f = at(v, "file"); kind(f, value_kind::record) && at(v, "kind") && kind(at(v, "kind"), value_kind::string) &&
                              at(v, "kind")->string == "text") {
    auto root = at(f, "root"), p = at(f, "path"), bytes = at(v, "bytes");
    if (!keys(f, {"root", "path"}) || !kind(root, value_kind::string) || root->string != "workspace" || !path(p) ||
        !kind(bytes, value_kind::string))
      return false;
    auto it = files.find(p->string);
    if (it == files.end() || it->second != bytes->string)
      return false;
    found.insert(p->string);
  }
  for (auto& [k, c] : v->fields)
    if (!references(c, files, found, depth + 1))
      return false;
  for (auto& c : v->items)
    if (!references(c, files, found, depth + 1))
      return false;
  return true;
}
} // namespace
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
  const auto& workspace_root = source.workspace_declaration().root;
  const auto& declarations   = source.declarations();
  auto        w = at(bundle, "workspace"), spec = at(bundle, "spec");
  if (at(bundle, "workspace_file")->string != source.file() || at(bundle, "spec_file")->string != source.spec_path() ||
      at(bundle, "registry_file")->string != "input.json" || !identity(workspace_root, w) ||
      !same_field(workspace_root, w, "spec") || !same_field(workspace_root, w, "registry") ||
      !same_field(at(workspace_root, "root"), at(w, "root"), "id") ||
      !same_field(at(workspace_root, "root"), at(w, "root"), "base") ||
      !kind(declarations.at(source.spec_path()).root, value_kind::spec) ||
      !identity(declarations.at(source.spec_path()).root, spec))
    return refuse(load_error_code::invalid_interpretation);
  std::map<std::string, std::string> normalized_jobs;
  if (!routes(at(w, "jobs"), normalized_jobs, parent(source.file()), true) || normalized_jobs != source.jobs() ||
      at(bundle, "jobs")->items.size() != source.jobs().size())
    return refuse(load_error_code::invalid_interpretation);
  for (auto& entry : at(bundle, "jobs")->items) {
    auto file = at(entry, "file")->string;
    auto j    = at(entry, "value");
    auto it   = declarations.find(file);
    if (it == declarations.end() || !kind(it->second.root, value_kind::job) || !identity(it->second.root, j))
      return refuse(load_error_code::invalid_interpretation);
  }
  auto registry = at(bundle, "registry"), header = at(w, "registry");
  if (!same_field(registry, header, "id") || !same_field(registry, header, "revision"))
    return refuse(load_error_code::invalid_interpretation);
  auto snapshot = source.sealed()->snapshot();
  if (!snapshot)
    return captured_error(snapshot.error());
  std::map<std::string, std::string_view> files;
  std::vector<std::string_view>           paths;
  for (auto& f : **snapshot)
    if (f.root == "workspace") {
      files.emplace(f.path, f.bytes);
      paths.push_back(f.path);
    }
  std::set<std::string> expected_refs, found;
  auto                  collect = [&](std::string_view file, const declaration& d) {
    for (auto& r : d.references)
      expected_refs.insert(parent(file) + r);
  };
  collect(source.file(), source.workspace_declaration());
  for (auto& [file, d] : declarations)
    collect(file, d);
  if (!references(bundle, files, found) || found != expected_refs)
    return refuse(load_error_code::invalid_interpretation);
  std::ranges::sort(paths);
  auto sealed = source.release_sealed();
  auto prepared =
      prepared_acceptance::create(std::move(*sealed), paths, stdlib_files, encoded->encoded, encoded->versions_encoded);
  if (!prepared)
    return std::unexpected(load_error{prepared.error().code == preparation_error_code::resource_limit
                                          ? load_error_code::resource_limit
                                          : load_error_code::preparation,
                                      {},
                                      prepared.error()});
  return std::move(*prepared);
}
} // namespace planar::engine::acceptance
