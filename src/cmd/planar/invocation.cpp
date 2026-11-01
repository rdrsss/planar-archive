module planar.cmd.invocation;
import std;
import planar.engine.capture;
import planar.engine.acceptance.load;
import planar.engine.capture.host_path;
import planar.engine.ledger.format;
import planar.engine.ledger.acceptance;
import planar.engine.history.read;
import planar.engine.source_export;
import planar.engine.runtime; // X07-REAL-RUNTIME
#ifdef PLANAR_AUTHORING_BUNDLE
import planar.engine.capture.directory;
import planar.engine.hash;
#endif
namespace planar::cmd {
namespace {
using namespace detail;
using namespace engine;
struct refusal {
  value_ptr error;
};
template <class T> T required(std::expected<T, diagnostic> result) {
  if (!result)
    throw result.error();
  return std::move(*result);
}
void required(std::expected<void, diagnostic> result) {
  if (!result)
    throw result.error();
}
value_ptr uncertainty_error(std::string operation) {
  using namespace detail;
  return object({{"code", text("io")},
                 {"message", text("Export may be complete; inspect the explicit target before retrying")},
                 {"field", text("")},
                 {"details", object({{"operation", text(std::move(operation))}})}});
}
value_ptr load_uncertainty_error(std::string operation) {
  return object({{"code", text("io")},
                 {"message", text("Load may be complete; retry the same request ID or inspect request history")},
                 {"field", text("")},
                 {"details", object({{"operation", text(std::move(operation))}})}});
}
value_ptr load_error(const acceptance::load_error& error) {
  if (!error.detail.code.empty())
    return native_error(error.detail);
  if (error.code == acceptance::load_error_code::invalid_path)
    return native_error({"path_denied", "Cannot resolve explicit input path"});
  if (error.code == acceptance::load_error_code::resource_limit)
    return native_error({"resource_limit", "Load exceeds native resource limits"});
  return native_error({"internal", "Invalid native load preparation"});
}
bool stage_token(std::string_view s) {
  constexpr std::string_view prefix = ".planar-export-";
  return s.starts_with(prefix) && s.size() == prefix.size() + 32 &&
         std::ranges::all_of(s.substr(prefix.size()), [](char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); });
}
value_ptr export_error(const engine::source_export::export_error& e) {
  using namespace detail;
  using enum engine::source_export::error_code;
  if (e.code == storage)
    return ledger_error(
        e.storage.value_or(engine::ledger::types::format_error{engine::ledger::types::format_error_code::io, ""}));
  if (e.code == publication_uncertain)
    return uncertainty_error("export");
  std::string code, message;
  switch (e.code) {
  case revision_not_found:
    code    = "revision_not_found";
    message = "Recorded revision was not found";
    break;
  case invalid_argument:
    code    = "schema";
    message = "Invalid export arguments";
    break;
  case path_denied:
    code    = "path_denied";
    message = "Export path requires existing real parent directories and a valid target";
    break;
  case target_exists:
    code    = "target_exists";
    message = "Export target already exists";
    break;
  case cleanup_incomplete:
    code = "export_incomplete";
    message =
        "Export staging cleanup is incomplete; inspect the sibling location in the explicit target parent before taking action";
    break;
  default:
    code    = "io";
    message = "Export operation failed";
    break;
  }
  auto fields = std::map<std::string, engine::value_ptr>{{"code", text(code)}, {"message", text(message)}, {"field", text("")}};
  if (e.code == cleanup_incomplete) {
    if (!stage_token(e.location))
      return native_error({"internal", "Invalid export cleanup location"});
    fields["details"] = object({{"operation", text("export")}, {"cleanup", text("incomplete")}, {"location", text(e.location)}});
  } else if (code == "io")
    fields["details"] = object({{"operation", text("export")}});
  return object(std::move(fields));
}

std::expected<std::string, diagnostic> process_cwd() {
  try {
    return std::filesystem::current_path().lexically_normal().string();
  } catch (const std::filesystem::filesystem_error&) {
    return std::unexpected(diagnostic{"io", "Cannot obtain invocation cwd"});
  }
}
std::optional<std::string_view> process_workspace_environment() {
  const auto* value = std::getenv("PLANAR_WORKSPACE_DIR");
  return value ? std::optional<std::string_view>(value) : std::nullopt;
}
std::expected<std::string, diagnostic> process_workspace_directory() {
  return snapshot_workspace_directory(process_workspace_environment());
}
using snapshot_reader = std::function<std::expected<std::string, diagnostic>()>;
#ifdef PLANAR_AUTHORING_BUNDLE
/// Maps both digest allocation failures (Hash and Canonical allocation failed) of an observation entry to the observation
/// total bound (D-C01-32); other errors pass.
diagnostic observation_digest_failure(diagnostic error) {
  if (error.code == "resource_limit" &&
      (error.message == "Hash allocation failed" || error.message == "Canonical allocation failed"))
    return capture_limit("observation_total_bytes", 4194304);
  return error;
}
#endif

class native_services final : public command_services {
  capture_session                                  _session;
  std::string                                      _workspace_path;
  value_ptr                                        _workspace;
  template <class Function> command_service_result capture_boundary(Function&& function) {
    try {
      return function();
    } catch (const refusal& e) {
      return {{}, e.error};
    } catch (const diagnostic& e) {
      return {{}, native_error(e)};
    } catch (const std::filesystem::filesystem_error&) {
      return {{}, native_error({"io", "Cannot resolve explicit input path"})};
    }
  }

public:
  bool                                      export_published = false;
  std::optional<acceptance::request_intent> load_intent;
  std::string                               load_ledger_identity;
  std::optional<acceptance::load_capture>   load_capture;
  // Borrow the stable VM receipt before entering completion. It remains authoritative
  // even if constructing the first service value throws after COMMIT.
  ledger::completion_receipt* load_receipt = nullptr;
  command_service_result      open_workspace(const command_load_options& options) override {
    auto opened = acceptance::load_capture::open_workspace(options.workspace, options.registry);
    if (!opened)
      return {{}, load_error(opened.error())};
    load_capture.emplace(std::move(*opened));
    auto value = load_capture->workspace();
    if (!value)
      return {{}, load_error(value.error())};
    return {*value, {}};
  }
  command_service_result capture_load(const command_load_options&, const value_ptr& route) override {
    if (!load_capture)
      return {{}, native_error({"internal", "Missing native load capture"})};
    auto captured = load_capture->capture(route);
    if (!captured)
      return {{}, load_error(captured.error())};
    return {object({{"bundle", *captured}}), {}};
  }
  command_service_result accept_load(const command_load_options& options, const value_ptr& bundle, const value_ptr& semantic,
                                     std::span<const history::tree_file> provenance,
                                     ledger::completion_receipt&         completion) override {
    load_receipt = &completion;
    if (!load_capture || !load_intent)
      return {{}, native_error({"internal", "Missing native load intent"})};
    auto prepared = load_capture->prepare(bundle, semantic, provenance);
    if (!prepared)
      return {{}, load_error(prepared.error())};
    auto accepted = ledger::complete_acceptance(options.ledger, *load_intent, *prepared, load_ledger_identity, completion);
    if (!accepted)
      return {{}, ledger_error(accepted.error())};
    auto decoded = codec_decode(completion.saved_result);
    if (!decoded)
      return {{}, native_error({"internal", "Invalid saved load response"})};
    return {*decoded, {}};
  }
  command_service_result open_workspace(const command_render_options& options) override {
    return capture_boundary([&]() -> command_service_result {
      const std::filesystem::path file(required(resolve_explicit_file(options.workspace)));
      required(_session.add_root("workspace", file.parent_path().string()));
      _workspace_path = file.filename().string();
      _workspace      = required(_session.load(_workspace_path)).root;
      return {object({{"file", text(_workspace_path)}, {"workspace", _workspace}}), {}};
    });
  }
  command_service_result capture_render(const command_render_options& options, const value_ptr& route) override {
    return capture_boundary([&]() -> command_service_result {
      const auto source_id  = route->fields.at("root")->fields.at("id")->string;
      auto       mapping_id = options.root.substr(0, options.root.find('='));
      if (mapping_id != source_id)
        throw refusal{object({{"code", text("reference")},
                              {"field", text("root")},
                              {"message", text("Root mapping must match workspace source ID")}})};
      auto                   spec_path = route->fields.at("spec");
      auto                   spec      = required(_session.load(spec_path->string));
      std::vector<value_ptr> jobs;
      for (const auto& job : route->fields.at("jobs")->items) {
        auto path   = job->fields.at("file");
        auto loaded = required(_session.load(path->string));
        jobs.push_back(object({{"file", path}, {"value", loaded.root}}));
      }
      required(_session.add_input("registry", required(resolve_explicit_file(options.registry))));
      auto registry = required(_session.read("registry", "input.json"));
      if (!options.observations.value_or("").empty()) {
        required(_session.add_input("observations", required(resolve_explicit_file(options.observations.value_or("")))));
        required(_session.read("observations", "input.json"));
      }
      required(_session.add_root("current:" + source_id,
                                 required(resolve_explicit_directory(options.root.substr(options.root.find('=') + 1)))));
      if (options.capture_outputs) {
        required(_session.add_root("base:" + source_id, required(resolve_explicit_directory(options.base_root.value_or("").substr(
                                                            options.base_root.value_or("").find('=') + 1)))));
        std::vector<std::string> exclusions;
        for (const auto& entry : route->fields.at("root")->fields.at("excluded")->items)
          exclusions.push_back(entry->string);
        required(_session.tree("current:" + source_id, exclusions));
        required(_session.tree("base:" + source_id, exclusions));
      }
      auto                   snapshot    = required(_session.finish());
      auto                   directories = required(_session.directories());
      std::vector<value_ptr> retained, workspace_files, source_files, source_directories;
      for (const auto& file : *snapshot) {
        retained.push_back(object(
            {{"root", text(file.root)}, {"path", text(file.path)}, {"mode", text(file.mode)}, {"bytes", text(file.bytes)}}));
        if (file.root == "workspace")
          workspace_files.push_back(object({{"path", text(file.path)}, {"bytes", text(file.bytes)}}));
        if (file.root.starts_with("current:") || file.root.starts_with("base:"))
          source_files.push_back(object({{"root", text(file.root)}, {"path", text(file.path)}}));
      }
      for (const auto& directory : *directories)
        source_directories.push_back(object({{"root", text(directory.root)}, {"path", text(directory.path)}}));
      auto bundle = object({{"workspace", object({{"file", text(_workspace_path)}, {"value", _workspace}})},
                            {"spec", object({{"file", spec_path}, {"value", spec.root}})},
                            {"jobs", array(jobs)},
                            {"registry", object({{"file", text("input.json")}, {"bytes", text(registry.bytes)}})},
                            {"files", array(workspace_files)}});
      auto capture =
          options.capture_outputs
              ? object({{"status", text("captured")}, {"files", array(source_files)}, {"directories", array(source_directories)}})
              : object({{"status", text("not_captured")}});
      std::map<std::string, value_ptr> request{{"bundle", bundle},
                                               {"capture", capture},
                                               {"files", array(retained)},
                                               {"job", text(std::string(options.job))},
                                               {"role", text(std::string(options.role))}};
      if (options.budget_bytes)
        request["budget_bytes"] = integer(*options.budget_bytes);
      return {object(std::move(request)), {}};
    });
  }
  command_service_result initialize_ledger(const command_init_options& options) override {
    try {
      auto metadata = engine::ledger::initialize_new_v2(options.ledger, options.workspace_id);
      if (!metadata)
        return {{}, ledger_error(metadata.error())};
      return {object({{"ledger", text(metadata->identity)},
                      {"workspace", text(metadata->workspace_id)},
                      {"active_revision", text("none")},
                      {"event", text("1")}}),
              {}};
    } catch (const std::bad_alloc&) {
      throw;
    } catch (...) {
      return {{}, ledger_error({engine::ledger::types::format_error_code::io, ""})};
    }
  }
  command_service_result read_history(const command_history_options& options, const command_history_selector& selector) override {
    using namespace engine::history;
    try {
      history_query q;
      if (selector.selector == "revision")
        q.selector = history_selector::revision;
      else if (selector.selector == "request")
        q.selector = history_selector::request;
      else if (selector.selector == "state")
        q.selector = history_selector::state;
      // The retained VM has already checked exact agreement with this typed intent.
      q.id = static_cast<std::uint64_t>(options.revision.value_or(options.at_event.value_or(options.after_event.value_or(0))));
      q.request_id = selector.request_id;
      q.limit      = selector.limit;
      auto file    = engine::ledger::open_existing(options.ledger);
      if (!file)
        return {{}, ledger_error(file.error())};
      auto result = engine::history::read_history(*file, q);
      if (result)
        return {*result, {}};
      const auto& e = result.error();
      if (e.code == history_read_code::storage)
        return {{}, ledger_error(e.storage)};
      if (e.code == history_read_code::corrupt)
        return {{}, ledger_error({engine::ledger::format_error_code::corrupt, ""})};
      const auto code = e.code == history_read_code::revision_not_found  ? "revision_not_found"
                        : e.code == history_read_code::request_not_found ? "request_not_found"
                        : e.code == history_read_code::event_not_found   ? "event_not_found"
                                                                         : "unsupported_interpretation";
      return {{}, native_error({code, "Recorded history view is unavailable"})};
    } catch (const std::bad_alloc&) {
      throw;
    } catch (...) {
      return {{}, ledger_error({engine::ledger::types::format_error_code::io, ""})};
    }
  }
  command_service_result export_source(const command_export_options& options) override {
    auto result = engine::source_export::export_source(options.ledger, options.revision, options.target);
    if (!result)
      return {{}, export_error(result.error())};
    export_published = true;
    return {object({{"ledger", text(result->ledger)},
                    {"revision", text(std::to_string(result->revision))},
                    {"source", text(std::to_string(result->source))}}),
            {}};
  }
#ifdef PLANAR_AUTHORING_BUNDLE
  /// Open the authoring workspace exactly like render: resolve the explicit file, root its directory, load the declaration.
  command_service_result open_workspace(const command_authoring_options& options) override {
    return capture_boundary([&]() -> command_service_result {
      const std::filesystem::path file(required(resolve_explicit_file(options.workspace)));
      required(_session.add_root("workspace", file.parent_path().string()));
      _workspace_path = file.filename().string();
      _workspace      = required(_session.load(_workspace_path)).root;
      return {object({{"file", text(_workspace_path)}, {"workspace", _workspace}}), {}};
    });
  }
  /// Capture the workspace, optionally the spec, and the registry. Job files are never read, listed or required.
  command_service_result capture_authoring(const command_authoring_options& options, const value_ptr& route,
                                           std::string_view include) override {
    return capture_boundary([&]() -> command_service_result {
      std::map<std::string, value_ptr> bundle_fields;
      if (include == "spec") {
        auto spec_path = route->fields.at("spec");
        auto spec      = required(_session.load(spec_path->string));
        bundle_fields.emplace("spec", object({{"file", spec_path}, {"value", spec.root}}));
      }
      auto captured = finish_authoring(options.registry, std::move(bundle_fields), {});
      return {object({{"bundle", captured.bundle}, {"capture", captured.capture}, {"files", captured.files}}), {}};
    });
  }
  /// Capture the spec, the registry and exactly the selected job; other listed jobs are neither read nor required.
  command_service_result capture_orchestrator(const command_authoring_options& options, const value_ptr& route,
                                              std::string_view job) override {
    return capture_boundary([&]() -> command_service_result {
      auto      spec_path = route->fields.at("spec");
      auto      spec      = required(_session.load(spec_path->string));
      value_ptr job_path;
      for (const auto& entry : route->fields.at("jobs")->items)
        if (entry->fields.at("id")->string == job)
          job_path = entry->fields.at("file");
      if (!job_path)
        throw refusal{unknown_selected_job()};
      auto                             loaded = required(_session.load(job_path->string));
      std::map<std::string, value_ptr> bundle_fields;
      bundle_fields.emplace("spec", object({{"file", spec_path}, {"value", spec.root}}));
      auto captured =
          finish_authoring(options.registry, std::move(bundle_fields), {object({{"file", job_path}, {"value", loaded.root}})});
      return {object({{"bundle", captured.bundle},
                      {"capture", captured.capture},
                      {"files", captured.files},
                      {"job", text(std::string(job))}}),
              {}};
    });
  }
  /// Read the diagnostic file through the bounded no-follow capture of the engine.
  command_service_result read_diagnostic(const command_authoring_options& options) override {
    return capture_boundary([&]() -> command_service_result {
      capture_session session;
      auto            facts = required(capture_diagnostic(session, options.diagnostic, 65536));
      return {object({{"bytes", text(facts.bytes)},
                      {"size", integer(static_cast<std::int64_t>(facts.size))},
                      {"digest", text(facts.digest)}}),
              {}};
    });
  }
  /// Read the observation directory through the directory module.
  command_service_result read_observation_directory(const command_authoring_options& options) override {
    return capture_boundary([&]() -> command_service_result {
      auto                   directory = required(engine::read_observation_directory(options.from));
      std::vector<value_ptr> entries;
      for (const auto& entry : directory.entries) {
        auto digest = engine::file_digest(entry.bytes);
        if (!digest)
          throw observation_digest_failure(std::move(digest.error()));
        entries.push_back(object({{"category", text(entry.category)},
                                  {"name", text(entry.name)},
                                  {"size", integer(static_cast<std::int64_t>(entry.size))},
                                  {"digest", text(*digest)},
                                  {"bytes", text(entry.bytes)}}));
      }
      return {object({{"entries", array(entries)}}), {}};
    });
  }

private:
  /// The value render's selection stage gives for a job the workspace does not list, byte for byte.
  static value_ptr unknown_selected_job() {
    // Render's selection stage owns this existing message; kept out of a literal pair so no source adds a message to the table.
    static const std::string unknown_job = "Unknown selected job";
    return object({{"code", text("reference")},
                   {"message", text(unknown_job)},
                   {"file", text("")},
                   {"field", text("selection.job")},
                   {"details", object({})}});
  }
  struct authoring_capture {
    value_ptr bundle, capture, files;
  };
  /// Register the registry, seal the session and build the render-shaped bundle, capture and files values.
  authoring_capture finish_authoring(const std::string& registry_path, std::map<std::string, value_ptr> bundle_fields,
                                     std::vector<value_ptr> jobs) {
    required(_session.add_input("registry", required(resolve_explicit_file(registry_path))));
    auto                   registry = required(_session.read("registry", "input.json"));
    auto                   snapshot = required(_session.finish());
    std::vector<value_ptr> retained, workspace_files;
    for (const auto& file : *snapshot) {
      retained.push_back(
          object({{"root", text(file.root)}, {"path", text(file.path)}, {"mode", text(file.mode)}, {"bytes", text(file.bytes)}}));
      if (file.root == "workspace")
        workspace_files.push_back(object({{"path", text(file.path)}, {"bytes", text(file.bytes)}}));
    }
    bundle_fields.emplace("workspace", object({{"file", text(_workspace_path)}, {"value", _workspace}}));
    bundle_fields.emplace("jobs", array(jobs));
    bundle_fields.emplace("registry", object({{"file", text("input.json")}, {"bytes", text(registry.bytes)}}));
    bundle_fields.emplace("files", array(workspace_files));
    return {object(std::move(bundle_fields)), object({{"status", text("not_captured")}}), array(retained)};
  }
#endif
};
} // namespace
std::expected<void, engine::diagnostic>
prepare_invocation_paths(engine::command_options&                                               options,
                         const std::function<std::expected<std::string, engine::diagnostic>()>& cwd_reader,
                         const std::function<std::expected<std::string, engine::diagnostic>()>& workspace_directory_reader) {
  struct selected_path {
    std::string* target;
    std::size_t  prefix;
    bool         lexical = false;
  };
  std::vector<selected_path> selected;
  if (auto* render = std::get_if<engine::command_render_options>(&options)) {
    const auto named = engine::workspace_is_name(render->workspace);
    if (!named)
      return std::unexpected(named.error());
    std::optional<std::string> directory;
    if (*named) {
      auto captured = workspace_directory_reader();
      if (!captured)
        return std::unexpected(captured.error());
      directory = std::move(*captured);
    }
    auto workspace =
        engine::expand_workspace(render->workspace, directory ? std::optional<std::string_view>(*directory) : std::nullopt);
    if (!workspace)
      return std::unexpected(workspace.error());
    render->workspace = std::move(*workspace);
    selected.push_back({&render->workspace, 0});
    selected.push_back({&render->registry, 0});
    selected.push_back({&render->root, render->root.find('=') + 1});
    if (render->base_root)
      selected.push_back({&*render->base_root, render->base_root->find('=') + 1});
    if (render->observations)
      selected.push_back({&*render->observations, 0});
  } else if (auto* load = std::get_if<engine::command_load_options>(&options)) {
    const auto named = engine::workspace_is_name(load->workspace);
    if (!named)
      return std::unexpected(named.error());
    std::optional<std::string> directory;
    if (*named) {
      auto captured = workspace_directory_reader();
      if (!captured)
        return std::unexpected(captured.error());
      directory = std::move(*captured);
    }
    auto workspace =
        engine::expand_workspace(load->workspace, directory ? std::optional<std::string_view>(*directory) : std::nullopt);
    if (!workspace)
      return std::unexpected(workspace.error());
    load->workspace = std::move(*workspace);
    selected.push_back({&load->workspace, 0, true});
    selected.push_back({&load->registry, 0, true});
    selected.push_back({&load->ledger, 0});
  } else if (auto* exporting = std::get_if<engine::command_export_options>(&options)) {
    selected.push_back({&exporting->ledger, 0});
    selected.push_back({&exporting->target, 0});
  } else if (auto* initializing = std::get_if<engine::command_init_options>(&options)) {
    selected.push_back({&initializing->ledger, 0});
  } else {
    selected.push_back({&std::get<engine::command_history_options>(options).ledger, 0});
  }
  bool needs_cwd = false;
  for (const auto& path : selected) {
    const auto argument = std::string_view(*path.target).substr(path.prefix);
    if (auto checked = engine::validate_host_path(argument); !checked)
      return std::unexpected(checked.error());
    needs_cwd = needs_cwd || !argument.starts_with('/');
  }
  std::optional<std::string> cwd;
  if (needs_cwd) {
    auto captured = cwd_reader();
    if (!captured)
      return std::unexpected(engine::diagnostic{"io", "Cannot obtain invocation cwd"});
    cwd = std::move(*captured);
  }
  for (const auto& path : selected) {
    auto prepared = engine::prepare_host_path(std::string_view(*path.target).substr(path.prefix),
                                              cwd ? std::optional<std::string_view>(*cwd) : std::nullopt);
    if (!prepared)
      return std::unexpected(prepared.error());
    // Only load workspace/registry use lexical targets for intent and capture.
    // Ledger traversal retains its original parent spelling like all ledger commands.
    path.target->replace(path.prefix, std::string::npos, path.lexical ? prepared->intent : prepared->spelling);
  }
  return {};
}
namespace {
// Normalize the profile selector lexically against the invocation's one cwd snapshot, without any existence check.
std::expected<std::string, engine::diagnostic>
normalize_profile_selector(std::string_view selector, invocation_snapshot& snapshot, const snapshot_reader& cwd_reader) {
  if (auto checked = engine::validate_host_path(selector); !checked)
    return std::unexpected(checked.error());
  std::optional<std::string_view> cwd;
  if (!selector.starts_with('/')) {
    if (!snapshot.cwd) {
      auto captured = cwd_reader();
      if (!captured)
        return std::unexpected(engine::diagnostic{"io", "Cannot obtain invocation cwd"});
      snapshot.cwd = std::move(*captured);
    }
    cwd = *snapshot.cwd;
  }
  auto prepared = engine::prepare_host_path(selector, cwd);
  if (!prepared)
    return std::unexpected(prepared.error());
  return std::move(prepared->intent);
}
} // namespace
std::expected<std::string, engine::diagnostic>
prepare_profile_invocation(std::string_view selector, engine::command_options& options, invocation_snapshot& snapshot,
                           const std::function<std::expected<std::string, engine::diagnostic>()>& cwd_reader,
                           const std::function<std::expected<std::string, engine::diagnostic>()>& workspace_directory_reader) {
  if (auto checked = engine::validate_host_path(selector); !checked)
    return std::unexpected(checked.error());
  // Each reader runs at most once; later slots, including the selector, reuse the snapshot.
  const snapshot_reader cwd_once = [&]() -> std::expected<std::string, engine::diagnostic> {
    if (snapshot.cwd)
      return *snapshot.cwd;
    auto captured = cwd_reader();
    if (!captured)
      return std::unexpected(captured.error());
    snapshot.cwd = *captured;
    return std::move(*captured);
  };
  const snapshot_reader workspace_once = [&]() -> std::expected<std::string, engine::diagnostic> {
    if (snapshot.workspace_directory)
      return *snapshot.workspace_directory;
    auto captured = workspace_directory_reader();
    if (!captured)
      return std::unexpected(captured.error());
    snapshot.workspace_directory = *captured;
    return std::move(*captured);
  };
  if (auto prepared = prepare_invocation_paths(options, cwd_once, workspace_once); !prepared)
    return std::unexpected(prepared.error());
  return normalize_profile_selector(selector, snapshot, cwd_once);
}
std::expected<std::string, engine::diagnostic> prepare_process_profile_selector(std::string_view     selector,
                                                                                invocation_snapshot& snapshot) {
  return normalize_profile_selector(selector, snapshot, process_cwd);
}
std::expected<void, engine::diagnostic> complete_process_snapshot(invocation_snapshot& snapshot) {
  if (!snapshot.cwd) {
    auto captured = process_cwd();
    if (!captured)
      return std::unexpected(captured.error());
    snapshot.cwd = std::move(*captured);
  }
  if (!snapshot.workspace_directory && process_workspace_environment()) {
    // An unusable environment value is left absent: a later bare workspace name then refuses as usage.
    if (auto captured = process_workspace_directory())
      snapshot.workspace_directory = std::move(*captured);
  }
  return {};
}
namespace {
// Profile load always runs on the one real runtime that owns the ledger's prepared_profile type, even in generated variants.
namespace rt_api = ::planar::engine;
// X07-ANCHOR:response-ceiling
constexpr std::size_t response_ceiling = 16777215;
} // namespace
response execute_profile_load(const engine::acceptance::request_intent& intent, std::string_view ledger_path) {
  using namespace detail;
  response                           fallback{4, "", ""};
  std::string                        stdout_error;
  engine::ledger::completion_receipt completion;
  const auto committed = [&]() { return completion.phase != engine::ledger::completion_phase::not_completed; };
  try {
    fallback     = failed(load_uncertainty_error("load"));
    stdout_error = failed(load_uncertainty_error("stdout")).error;
    if (fallback.error.empty() || stdout_error.empty())
      return {4, "", ""};
    auto data = intent.data();
    if (!data)
      return failed(native_error({"internal", "Missing native load intent"}));
    const auto& request = **data;
    std::string ledger_identity;
    {
      auto ledger = engine::ledger::open_existing(std::filesystem::path(std::string(ledger_path)),
                                                  engine::ledger::ledger_access::read_only);
      if (!ledger)
        return failed(ledger_error(ledger.error()));
      ledger_identity = ledger->metadata().identity;
      auto saved      = ledger->read_request(request.request_id);
      if (!saved)
        return failed(ledger_error(saved.error()));
      if (*saved) {
        if ((*saved)->body_encoded != request.encoded)
          return failed(ledger_error({engine::ledger::format_error_code::request_conflict, ""}));
        return {0, (*saved)->result + "\n", "", std::move(stdout_error)};
      }
      // Only a format2 ledger can record a profile request; this is decided before the profile is read.
      if (ledger->metadata().format != engine::ledger::ledger_format::v2)
        return failed(ledger_error({engine::ledger::format_error_code::unsupported_version, ""}));
    } // The verified read transaction is released before any profile capture or writable acceptance.
    // X07-ANCHOR:after-early-miss
    auto opened = rt_api::profile_invocation::open_with_profile(intent, ledger_path);
    if (!opened)
      return registration_failed(opened.error().detail);
    // X07-ANCHOR:after-factory-open
    // The step-4 intent object is passed unchanged: preparation refuses any other intent.
    auto prepared = std::move(*opened).prepare_load(intent);
    if (!prepared) {
      if (prepared.error().semantic)
        return semantic_failed(prepared.error().semantic);
      return registration_failed(prepared.error().failure.detail);
    }
    // X07-ANCHOR:after-prepare-load
    // Exact relookup under the lock, the stale-head check and the final receipt verification run inside the commit.
    auto sealed = engine::ledger::complete_acceptance(std::filesystem::path(std::string(ledger_path)), intent, *prepared,
                                                      ledger_identity, completion);
    // X07-ANCHOR:after-commit
    if (!sealed) {
      if (committed())
        return fallback;
      return failed(ledger_error(sealed.error()));
    }
    if (completion.phase != engine::ledger::completion_phase::committed &&
        completion.phase != engine::ledger::completion_phase::replayed)
      return committed() ? std::move(fallback) : failed(native_error({"internal", "Missing load completion"}));
    // Encoding the saved response is a post-commit step: an oversize or malformed record keeps the completion receipt.
    if (completion.saved_result.size() > response_ceiling)
      return fallback;
    auto saved_value = engine::codec_decode(completion.saved_result);
    if (!saved_value)
      return fallback;
    return {0, completion.saved_result + "\n", "", std::move(stdout_error)};
  } catch (...) {
    if (committed())
      return fallback;
    return {4, "", ""};
  }
}
#ifdef PLANAR_AUTHORING_BUNDLE
response execute_authoring(engine::command_authoring_options options, engine::command_vm* registered) {
  using namespace detail;
  // Both ceilings of RA-3 and RA-6: checked before the generic 16 MiB output ceiling, each with its own limit name.
  constexpr std::size_t prompt_ceiling = 1048576;
  try {
    std::optional<engine::command_vm> local;
    engine::command_vm*               vm = registered;
    if (!vm) {
      auto created = engine::command_vm::create();
      if (!created)
        return failed(native_error(created.error()));
      local.emplace(std::move(*created));
      vm = &*local;
    }
    const bool observing = options.verb == engine::command_authoring_verb::observe;
    auto       services  = std::make_unique<native_services>();
    auto       result    = vm->invoke_authoring(std::move(options), *services);
    if (!result)
      return failed(native_error(result.error()));
    if (!(*result)->fields.at("ok")->boolean)
      return failed((*result)->fields.at("error"));
    auto envelope = (*result)->fields.at("value");
    if (!observing && envelope->fields.at("prompt")->fields.at("markdown")->string.size() > prompt_ceiling)
      return failed(native_error(capture_limit("prompt_bytes", prompt_ceiling)));
    auto encoded = engine::codec_encode(envelope, {response_ceiling, 256, 4000000});
    if (!encoded) {
      auto error = encoded.error().detail;
      if (error.code == "resource_limit" && error.limit == "output_bytes")
        error.maximum = response_ceiling + 1;
      return failed(native_error(error));
    }
    if (observing && encoded->size() > prompt_ceiling)
      return failed(native_error(capture_limit("observation_bytes", prompt_ceiling)));
    return {0, *encoded + "\n", ""};
  } catch (...) {
    return {4, "", ""};
  }
}
#endif
response native_export_failure(const engine::source_export::export_error& error) {
  auto result = detail::failed(export_error(error));
  if (error.code == engine::source_export::error_code::cleanup_incomplete)
    result.status = 4;
  return result;
}
response execute(engine::command_options options, engine::command_vm* registered) {
  using namespace detail;
  response                           fallback{4, "", ""};
  std::string                        stdout_error;
  std::optional<engine::command_vm>  local;
  engine::command_vm*                vm = registered;
  std::unique_ptr<native_services>   services;
  engine::ledger::completion_receipt early_completion;
  auto                               published = [&]() {
    return (services &&
            (services->export_published ||
             (services->load_receipt && services->load_receipt->phase != engine::ledger::completion_phase::not_completed))) ||
           (vm && (vm->export_may_be_complete() || vm->load_may_be_complete())) ||
           early_completion.phase != engine::ledger::completion_phase::not_completed;
  };
  try {
    if (!vm) {
      auto created = engine::command_vm::create();
      if (!created)
        return failed(native_error(created.error()));
      local.emplace(std::move(*created));
      vm = &*local;
    }
    auto prepared = prepare_invocation_paths(options, process_cwd, process_workspace_directory);
    if (!prepared)
      return failed(native_error(prepared.error()));
    const bool exporting = std::holds_alternative<engine::command_export_options>(options);
    if (exporting) {
      fallback     = failed(uncertainty_error("export"));
      stdout_error = failed(uncertainty_error("stdout")).error;
      if (fallback.error.empty() || stdout_error.empty())
        return {4, "", ""};
    }
    const bool loading = std::holds_alternative<engine::command_load_options>(options);
    if (loading) {
      fallback     = failed(load_uncertainty_error("load"));
      stdout_error = failed(load_uncertainty_error("stdout")).error;
      if (fallback.error.empty() || stdout_error.empty())
        return {4, "", ""};
    }
    services = std::make_unique<native_services>();
    if (loading) {
      const auto& load = std::get<engine::command_load_options>(options);
      auto intent = engine::acceptance::request_intent::create(load.request_id, load.actor, load.expected_head, load.workspace,
                                                               load.registry);
      if (!intent) {
        if (intent.error().code == engine::acceptance::preparation_error_code::resource_limit)
          return failed(native_error({"resource_limit", "Load request exceeds native resource limits"}));
        return failed(native_error({"schema", "Invalid load arguments"}));
      }
      services->load_intent.emplace(std::move(*intent));
      {
        auto ledger = engine::ledger::open_existing(load.ledger, engine::ledger::ledger_access::read_only);
        if (!ledger)
          return failed(ledger_error(ledger.error()));
        services->load_ledger_identity = ledger->metadata().identity;
        if (ledger->metadata().format == engine::ledger::ledger_format::v2) {
          // The ledger format, not the command, selects the request codec; the embedded profile stays absent.
          auto framed = engine::acceptance::request_intent::create_v2(load.request_id, load.actor, load.expected_head,
                                                                      load.workspace, load.registry);
          if (!framed) {
            if (framed.error().code == engine::acceptance::preparation_error_code::resource_limit)
              return failed(native_error({"resource_limit", "Load request exceeds native resource limits"}));
            return failed(native_error({"schema", "Invalid load arguments"}));
          }
          services->load_intent.emplace(std::move(*framed));
        }
        auto saved = ledger->read_request(load.request_id);
        if (!saved)
          return failed(ledger_error(saved.error()));
        if (*saved) {
          auto data = services->load_intent->data();
          if ((*saved)->body_encoded != (*data)->encoded)
            return failed(ledger_error({engine::ledger::format_error_code::request_conflict, ""}));
          early_completion.saved_result = std::move((*saved)->result);
          early_completion.phase        = engine::ledger::completion_phase::replayed;
        }
      } // Release the verified read transaction before capture or writable acceptance.
      if (early_completion.phase == engine::ledger::completion_phase::replayed)
        return {0, early_completion.saved_result + "\n", "", std::move(stdout_error)};
    }
    auto result = vm->invoke(std::move(options), *services);
    if (!result) {
      if (published())
        return fallback;
      return failed(native_error(result.error()));
    }
    if (!(*result)->fields.at("ok")->boolean) {
      if (published())
        return fallback;
      auto error    = (*result)->fields.at("error");
      auto response = failed(error);
      if (error->fields.at("code")->string == "export_incomplete")
        response.status = 4;
      return response;
    }
    if (loading) {
      const auto* completion = vm->load_completion();
      if (!completion || (completion->phase != engine::ledger::completion_phase::committed &&
                          completion->phase != engine::ledger::completion_phase::replayed))
        return published() ? std::move(fallback) : failed(native_error({"internal", "Missing load completion"}));
      return {0, completion->saved_result + "\n", "", std::move(stdout_error)};
    }
    auto envelope = (*result)->fields.at("value");
    auto encoded  = engine::codec_encode(envelope, {16777215, 256, 4000000});
    if (!encoded) {
      if (published())
        return fallback;
      auto error = encoded.error().detail;
      if (error.code == "resource_limit" && error.limit == "output_bytes")
        error.maximum = 16777216;
      return failed(native_error(error));
    }
    return {0, *encoded + "\n", "", std::move(stdout_error)};
  } catch (...) {
    if (published())
      return fallback;
    return {4, "", ""};
  }
}
response execute_with_profile(engine::command_options options, std::string_view profile_selector) {
  using namespace detail;
  response                         fallback{4, "", ""};
  std::string                      stdout_error;
  std::unique_ptr<native_services> services;
  const auto                       published = [&]() { return services && services->export_published; };
  try {
    invocation_snapshot snapshot;
    auto selector = prepare_profile_invocation(profile_selector, options, snapshot, process_cwd, process_workspace_directory);
    if (!selector)
      return failed(native_error(selector.error()));
    if (const auto* load = std::get_if<engine::command_load_options>(&options)) {
      // The one request_v2 intent framed here is the intent every later load step uses.
      auto intent = engine::acceptance::request_intent::create_v2(load->request_id, load->actor, load->expected_head,
                                                                  load->workspace, load->registry, *selector);
      if (!intent) {
        if (intent.error().code == engine::acceptance::preparation_error_code::resource_limit)
          return failed(native_error({"resource_limit", "Load request exceeds native resource limits"}));
        return failed(native_error({"schema", "Invalid load arguments"}));
      }
      return execute_profile_load(*intent, load->ledger);
    }
    if (std::holds_alternative<engine::command_export_options>(options)) {
      fallback     = failed(uncertainty_error("export"));
      stdout_error = failed(uncertainty_error("stdout")).error;
      if (fallback.error.empty() || stdout_error.empty())
        return {4, "", ""};
    }
    services    = std::make_unique<native_services>();
    auto opened = engine::profile_invocation::open_with_profile(*selector, options);
    if (!opened)
      return registration_failed(opened.error().detail);
    // The owner dispatches exactly the options it admitted and is consumed by the call.
    auto result = std::move(*opened).run_known(options, *services);
    if (!result) {
      if (published())
        return fallback;
      return failed(native_error(result.error()));
    }
    if (!(*result)->fields.at("ok")->boolean) {
      if (published())
        return fallback;
      auto error    = (*result)->fields.at("error");
      auto response = failed(error);
      if (error->fields.at("code")->string == "export_incomplete")
        response.status = 4;
      return response;
    }
    auto encoded = engine::codec_encode((*result)->fields.at("value"), {response_ceiling, 256, 4000000});
    if (!encoded) {
      if (published())
        return fallback;
      auto error = encoded.error().detail;
      if (error.code == "resource_limit" && error.limit == "output_bytes")
        error.maximum = response_ceiling + 1;
      return failed(native_error(error));
    }
    return {0, *encoded + "\n", "", std::move(stdout_error)};
  } catch (...) {
    if (published())
      return fallback;
    return {4, "", ""};
  }
}
} // namespace planar::cmd
