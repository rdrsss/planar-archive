module planar.engine.acceptance.load;
import planar.engine.capture;
import planar.engine.capture.host_path;
import planar.engine.canonical;
namespace planar::engine::acceptance {
namespace {
auto refuse(load_error_code code) {
  return std::unexpected(load_error{code});
}
auto captured_error(const diagnostic& e) {
  return std::unexpected(load_error{e.code == "resource_limit" ? load_error_code::resource_limit : load_error_code::capture, e});
}
value_ptr object(std::map<std::string, value_ptr> fields) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::record;
  v->fields = std::move(fields);
  return v;
}
value_ptr text(std::string bytes) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = std::move(bytes);
  return v;
}
value_ptr list(std::vector<value_ptr> items) {
  auto v   = std::make_shared<value>();
  v->kind  = value_kind::list;
  v->items = std::move(items);
  return v;
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
/// Sole owner of discovery, sealed receipts and identity correspondence; never shared between threads.
struct load_capture::implementation {
  capture_session session;                         ///< Confined declaration discovery; no tree authority is exercised.
  declaration     workspace;                       ///< Exact original opened declaration and its original references.
  std::string     file,                            ///< Portable original workspace filename.
      registry_path,                               ///< Private normalized explicit registry target, never exported.
      spec_path;                                   ///< Validated portable route to the specification.
  std::map<std::string, std::string> jobs;         ///< Sorted unique routed job IDs and portable paths.
  std::map<std::string, declaration> declarations; ///< Exact original specification and all jobs.
  std::optional<acceptance_capture>  sealed;       ///< Original selected receipts transferred at the first seal.
  bool                               workspace_returned = false; ///< One-shot routing input state, retained across moves.
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
  for (auto p : {workspace, registry}) {
    auto checked = validate_host_path(p);
    if (!checked)
      return captured_error(checked.error());
    auto normalized = normalize_intent_path(p, "/");
    if (!normalized)
      return captured_error(normalized.error());
    if (p.empty() || p.front() != '/' || *normalized != p)
      return refuse(load_error_code::invalid_path);
  }
  auto state  = std::make_unique<implementation>();
  auto host   = std::filesystem::path(workspace);
  state->file = host.filename().string();
  if (!valid_path(state->file))
    return refuse(load_error_code::invalid_path);
  state->registry_path = registry;
  auto root            = state->session.add_root("workspace", host.parent_path().string());
  if (!root)
    return captured_error(root.error());
  auto loaded = state->session.load(state->file);
  if (!loaded)
    return captured_error(loaded.error());
  state->workspace = std::move(*loaded);
  return load_capture(std::move(state));
}
std::expected<value_ptr, load_error> load_capture::workspace() {
  if (!_impl || _impl->workspace_returned || _impl->sealed) {
    _impl.reset();
    return refuse(load_error_code::invalid_state);
  }
  _impl->workspace_returned = true;
  return object({{"file", text(_impl->file)}, {"workspace", _impl->workspace.root}});
}
std::expected<value_ptr, load_error> load_capture::capture(const value_ptr& route) {
  auto state = std::move(_impl);
  if (!state || !state->workspace_returned || state->sealed)
    return refuse(load_error_code::invalid_state);
  auto                               raw = state->workspace.root, w = at(route, "workspace");
  std::map<std::string, std::string> opened_jobs, routed_jobs, normalized_jobs;
  auto                               spec = at(raw, "spec"), routed_spec = at(route, "spec");
  if (!keys(route, {"workspace", "file", "spec", "jobs", "root", "registry"}) || !kind(raw, value_kind::workspace) ||
      !(at(w, "notes") ? keys(w, {"version", "id", "root", "spec", "jobs", "registry", "toolchains", "checks", "notes"})
                       : keys(w, {"version", "id", "root", "spec", "jobs", "registry", "toolchains", "checks"})) ||
      !kind(at(w, "toolchains"), value_kind::list) || !kind(at(w, "checks"), value_kind::list) ||
      (at(w, "notes") && !kind(at(w, "notes"), value_kind::record)) || !kind(at(w, "id"), value_kind::string) ||
      !kind(at(w, "version"), value_kind::integer) || !keys(at(w, "root"), {"id", "base", "excluded"}) ||
      !kind(at(at(w, "root"), "id"), value_kind::string) || !kind(at(at(w, "root"), "base"), value_kind::string) ||
      !kind(at(at(w, "root"), "excluded"), value_kind::list) || !keys(at(w, "registry"), {"id", "revision"}) ||
      !kind(at(at(w, "registry"), "id"), value_kind::string) || !kind(at(at(w, "registry"), "revision"), value_kind::string) ||
      !identity(raw, w) || !path(at(route, "file")) || at(route, "file")->string != state->file || !path(spec) ||
      !path(routed_spec) || routed_spec->string != parent(state->file) + spec->string || !same_field(raw, w, "spec") ||
      !same_field(raw, w, "registry") || !same(at(route, "registry"), at(w, "registry")) ||
      !same_field(at(raw, "root"), at(w, "root"), "id") || !same_field(at(raw, "root"), at(w, "root"), "base") ||
      !same(at(route, "root"), at(w, "root")) || !routes(at(raw, "jobs"), opened_jobs, parent(state->file), false) ||
      !routes(at(route, "jobs"), routed_jobs, "", true) || !routes(at(w, "jobs"), normalized_jobs, parent(state->file), true) ||
      opened_jobs != routed_jobs || opened_jobs != normalized_jobs)
    return refuse(load_error_code::invalid_route);
  state->spec_path = routed_spec->string;
  state->jobs      = std::move(routed_jobs);
  std::set<std::string> declarations{state->file, state->spec_path};
  if (declarations.size() != 2)
    return refuse(load_error_code::invalid_route);
  for (auto& [id, file] : state->jobs)
    if (!declarations.insert(file).second)
      return refuse(load_error_code::invalid_route);
  auto loaded_spec = state->session.load(state->spec_path);
  if (!loaded_spec)
    return captured_error(loaded_spec.error());
  state->declarations.emplace(state->spec_path, std::move(*loaded_spec));
  std::vector<value_ptr> jobs;
  for (auto& [id, file] : state->jobs) {
    auto j = state->session.load(file);
    if (!j)
      return captured_error(j.error());
    jobs.push_back(object({{"file", text(file)}, {"value", j->root}}));
    state->declarations.emplace(file, std::move(*j));
  }
  auto registered = state->session.add_input("registry", state->registry_path);
  if (!registered)
    return captured_error(registered.error());
  auto registry = state->session.read("registry", "input.json");
  if (!registry)
    return captured_error(registry.error());
  auto sealed = state->session.finish_for_acceptance();
  if (!sealed)
    return captured_error(sealed.error());
  state->sealed.emplace(std::move(*sealed));
  auto snapshot = state->sealed->snapshot();
  if (!snapshot)
    return captured_error(snapshot.error());
  std::vector<value_ptr> files;
  for (auto& f : **snapshot)
    if (f.root == "workspace")
      files.push_back(object({{"path", text(f.path)}, {"bytes", text(f.bytes)}}));
  auto result =
      object({{"workspace", object({{"file", text(state->file)}, {"value", raw}})},
              {"spec", object({{"file", text(state->spec_path)}, {"value", state->declarations.at(state->spec_path).root}})},
              {"jobs", list(std::move(jobs))},
              {"registry", object({{"file", text("input.json")}, {"bytes", text(registry->bytes)}})},
              {"files", list(std::move(files))}});
  _impl = std::move(state);
  return result;
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
  if (!state || !state->sealed)
    return refuse(load_error_code::invalid_state);
  auto stdlib = history::verify_stdlib(stdlib_files);
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
  auto w = at(bundle, "workspace"), spec = at(bundle, "spec");
  if (at(bundle, "workspace_file")->string != state->file || at(bundle, "spec_file")->string != state->spec_path ||
      at(bundle, "registry_file")->string != "input.json" || !identity(state->workspace.root, w) ||
      !same_field(state->workspace.root, w, "spec") || !same_field(state->workspace.root, w, "registry") ||
      !same_field(at(state->workspace.root, "root"), at(w, "root"), "id") ||
      !same_field(at(state->workspace.root, "root"), at(w, "root"), "base") ||
      !kind(state->declarations.at(state->spec_path).root, value_kind::spec) ||
      !identity(state->declarations.at(state->spec_path).root, spec))
    return refuse(load_error_code::invalid_interpretation);
  std::map<std::string, std::string> normalized_jobs;
  if (!routes(at(w, "jobs"), normalized_jobs, parent(state->file), true) || normalized_jobs != state->jobs ||
      at(bundle, "jobs")->items.size() != state->jobs.size())
    return refuse(load_error_code::invalid_interpretation);
  for (auto& entry : at(bundle, "jobs")->items) {
    auto file = at(entry, "file")->string;
    auto j    = at(entry, "value");
    auto it   = state->declarations.find(file);
    if (it == state->declarations.end() || !kind(it->second.root, value_kind::job) || !identity(it->second.root, j))
      return refuse(load_error_code::invalid_interpretation);
  }
  auto registry = at(bundle, "registry"), header = at(w, "registry");
  if (!same_field(registry, header, "id") || !same_field(registry, header, "revision"))
    return refuse(load_error_code::invalid_interpretation);
  auto snapshot = state->sealed->snapshot();
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
  collect(state->file, state->workspace);
  for (auto& [file, d] : state->declarations)
    collect(file, d);
  if (!references(bundle, files, found) || found != expected_refs)
    return refuse(load_error_code::invalid_interpretation);
  std::ranges::sort(paths);
  auto prepared =
      prepared_acceptance::create(std::move(*state->sealed), paths, stdlib_files, encoded->encoded, encoded->versions_encoded);
  if (!prepared)
    return std::unexpected(load_error{prepared.error().code == preparation_error_code::resource_limit
                                          ? load_error_code::resource_limit
                                          : load_error_code::preparation,
                                      {},
                                      prepared.error()});
  return std::move(*prepared);
}
} // namespace planar::engine::acceptance
