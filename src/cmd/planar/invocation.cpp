module planar.cmd.invocation;
import std;
import planar.engine.capture;
import planar.engine.acceptance.load;
import planar.engine.capture.host_path;
import planar.engine.ledger.format;
import planar.engine.ledger.acceptance;
import planar.engine.history.read;
import planar.engine.source_export;
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
      auto metadata = engine::ledger::initialize_new(options.ledger, options.workspace_id);
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
    auto prepared = prepare_invocation_paths(
        options,
        []() -> std::expected<std::string, engine::diagnostic> {
          try {
            return std::filesystem::current_path().lexically_normal().string();
          } catch (const std::filesystem::filesystem_error&) {
            return std::unexpected(engine::diagnostic{"io", "Cannot obtain invocation cwd"});
          }
        },
        []() -> std::expected<std::string, engine::diagnostic> {
          const auto* value = std::getenv("PLANAR_WORKSPACE_DIR");
          return engine::snapshot_workspace_directory(value ? std::optional<std::string_view>(value) : std::nullopt);
        });
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
        auto saved                     = ledger->read_request(load.request_id);
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
} // namespace planar::cmd
