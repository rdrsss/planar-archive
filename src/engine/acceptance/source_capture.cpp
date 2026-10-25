module planar.engine.acceptance.source_capture;
import std;
import planar.engine.capture.host_path;
import planar.engine.canonical;
namespace planar::engine::acceptance {
namespace {
std::unexpected<source_error> refuse(source_error_code code) {
  return std::unexpected(source_error{code});
}
source_error to_source_error(const diagnostic& e) {
  return source_error{e.code == "resource_limit" ? source_error_code::resource_limit : source_error_code::capture, e};
}
std::unexpected<source_error> captured_error(const diagnostic& e) {
  return std::unexpected(to_source_error(e));
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
// Lexical host-path checks shared by the embedded and bound openings.
std::optional<source_error> validate_paths(std::string_view workspace, std::string_view registry) {
  for (auto p : {workspace, registry}) {
    auto checked = validate_host_path(p);
    if (!checked)
      return source_error{checked.error().code == "resource_limit" ? source_error_code::resource_limit
                                                                   : source_error_code::capture,
                          checked.error()};
    auto normalized = normalize_intent_path(p, "/");
    if (!normalized)
      return source_error{normalized.error().code == "resource_limit" ? source_error_code::resource_limit
                                                                      : source_error_code::capture,
                          normalized.error()};
    if (p.empty() || p.front() != '/' || *normalized != p)
      return source_error{source_error_code::invalid_path};
  }
  return std::nullopt;
}
} // namespace
source_error known_declaration_error(const diagnostic& admission) {
  if (admission.code == "io" && admission.message == "Cannot open confined input")
    return source_error{source_error_code::capture, diagnostic{"io", "Cannot inspect confined input"}};
  return to_source_error(admission);
}
/// Sole owner of discovery, sealed receipts and identity correspondence; never shared between threads.
struct source_capture::implementation {
  capture_session session;                               ///< Confined declaration discovery; no tree authority is exercised.
  declaration     workspace;                             ///< Exact original opened declaration and its original references.
  std::string     file,                                  ///< Portable original workspace filename.
      registry_path,                                     ///< Private normalized explicit registry target, never exported.
      spec_path;                                         ///< Validated portable route to the specification.
  std::map<std::string, std::string>   jobs;             ///< Sorted unique routed job IDs and portable paths.
  std::map<std::string, declaration>   declarations;     ///< Exact original specification and all jobs.
  std::optional<acceptance_capture>    sealed;           ///< Original selected receipts transferred at the first seal.
  std::optional<retained_file_receipt> registry_receipt; ///< Original registry authority, bound at capture when present.
  bool                                 workspace_returned = false; ///< One-shot routing input state, retained across moves.
};
source_capture::source_capture(std::unique_ptr<implementation> state) : _impl(std::move(state)) {
}
source_capture::~source_capture()                               = default;
source_capture::source_capture(source_capture&& other) noexcept = default;
source_capture& source_capture::operator=(source_capture&& other) noexcept {
  if (this != &other)
    _impl = std::move(other._impl);
  return *this;
}
std::expected<source_capture, source_error> source_capture::open_embedded(std::string_view workspace, std::string_view registry) {
  if (auto refused = validate_paths(workspace, registry))
    return std::unexpected(*refused);
  auto state  = std::make_unique<implementation>();
  auto host   = std::filesystem::path(workspace);
  state->file = host.filename().string();
  if (!valid_path(state->file))
    return refuse(source_error_code::invalid_path);
  state->registry_path = registry;
  auto root            = state->session.add_root("workspace", host.parent_path().string());
  if (!root)
    return captured_error(root.error());
  auto loaded = state->session.load(state->file);
  if (!loaded)
    return captured_error(loaded.error());
  state->workspace = std::move(*loaded);
  return source_capture(std::move(state));
}
std::expected<source_capture, source_error> source_capture::open_bound(std::string_view workspace, std::string_view registry,
                                                                       retained_root_receipt root,
                                                                       retained_file_receipt declaration_receipt,
                                                                       retained_file_receipt registry_file) {
  // Construction refuses moved-from, poisoned or foreign receipts before any consumption or read.
  // Every refusal poisons each supplied receipt; poison on a moved-from handle is a no-op.
  auto refuse_unconsumed = [&](source_error e) {
    root.poison();
    declaration_receipt.poison();
    registry_file.poison();
    return std::unexpected(std::move(e));
  };
  if (!root.live() || !declaration_receipt.live() || !registry_file.live() || !root.same_admission(declaration_receipt) ||
      !root.same_admission(registry_file))
    return refuse_unconsumed(
        source_error{source_error_code::capture, diagnostic{"capture_changed", "Captured external profile changed"}});
  if (auto refused = validate_paths(workspace, registry))
    return refuse_unconsumed(std::move(*refused));
  auto host = std::filesystem::path(workspace);
  auto file = host.filename().string();
  if (!valid_path(file))
    return refuse_unconsumed(source_error{source_error_code::invalid_path});
  auto state           = std::make_unique<implementation>();
  state->file          = std::move(file);
  state->registry_path = registry;
  // After consumption every refusal poisons the single shared admission through the registry receipt.
  // Construction guaranteed that root, declaration and registry share one admission.
  state->registry_receipt.emplace(std::move(registry_file));
  auto refuse_bound = [&](source_error e) {
    state->registry_receipt->poison();
    return std::unexpected(std::move(e));
  };
  auto added = state->session.add_root("workspace", host.parent_path().string(), std::move(root));
  if (!added)
    return refuse_bound(to_source_error(added.error()));
  auto declared = state->session.read("workspace", state->file, std::move(declaration_receipt));
  if (!declared)
    return refuse_bound(to_source_error(declared.error()));
  auto loaded = state->session.load(state->file);
  if (!loaded)
    return refuse_bound(to_source_error(loaded.error()));
  state->workspace = std::move(*loaded);
  return source_capture(std::move(state));
}
std::expected<value_ptr, source_error> source_capture::workspace() {
  if (!_impl || _impl->workspace_returned || _impl->sealed) {
    _impl.reset();
    return refuse(source_error_code::invalid_state);
  }
  _impl->workspace_returned = true;
  return object({{"file", text(_impl->file)}, {"workspace", _impl->workspace.root}});
}
std::expected<value_ptr, source_error> source_capture::capture(const value_ptr& route) {
  auto state = std::move(_impl);
  if (!state || !state->workspace_returned || state->sealed)
    return refuse(source_error_code::invalid_state);
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
    return refuse(source_error_code::invalid_route);
  state->spec_path = routed_spec->string;
  state->jobs      = std::move(routed_jobs);
  std::set<std::string> declarations{state->file, state->spec_path};
  if (declarations.size() != 2)
    return refuse(source_error_code::invalid_route);
  for (auto& [id, file] : state->jobs)
    if (!declarations.insert(file).second)
      return refuse(source_error_code::invalid_route);
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
  auto registered = state->registry_receipt
                        ? state->session.add_input("registry", state->registry_path, std::move(*state->registry_receipt))
                        : state->session.add_input("registry", state->registry_path);
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
bool source_capture::sealed_ready() const noexcept {
  return _impl && _impl->sealed.has_value();
}
const std::string& source_capture::file() const noexcept {
  return _impl->file;
}
const std::string& source_capture::spec_path() const noexcept {
  return _impl->spec_path;
}
const std::map<std::string, std::string>& source_capture::jobs() const noexcept {
  return _impl->jobs;
}
const std::map<std::string, declaration>& source_capture::declarations() const noexcept {
  return _impl->declarations;
}
const declaration& source_capture::workspace_declaration() const noexcept {
  return _impl->workspace;
}
std::optional<acceptance_capture>& source_capture::sealed() noexcept {
  return _impl->sealed;
}
std::optional<acceptance_capture> source_capture::release_sealed() noexcept {
  auto out = std::move(_impl->sealed);
  _impl->sealed.reset();
  return out;
}
std::expected<void, source_error> source_capture::verify() const {
  if (!sealed_ready())
    return refuse(source_error_code::invalid_state);
  auto verified = _impl->sealed->verify();
  if (!verified)
    return captured_error(verified.error());
  return {};
}
} // namespace planar::engine::acceptance
