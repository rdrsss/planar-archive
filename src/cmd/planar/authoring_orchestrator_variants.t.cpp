// RA01-B2 orchestrator tests over the generated variant runtimes (variant 0 and variant 20) with test-owned command services.
// The provider answers capture_orchestrator with exactly the four keys {bundle,capture,files,job}; the value is a real capture
// of the quota fixture built through the same capture session the stack's native services use (the render capture value
// restricted to the selected job). Native appends the fifth key, commands, from the sealed catalog.
#include <catch2/catch_test_macros.hpp>
import std;
import planar.engine.runtime;
import planar.cmd.response;
import planar.engine.authoring_variant_0;
import planar.engine.authoring_variant_20;
import planar.engine.capture;

namespace {
namespace fs = std::filesystem;
using planar::engine::value_ptr;

/// The quota fixture workspace file, found by walking up from this source file (and the cwd) to the repository root.
fs::path quota_workspace_file() {
  const fs::path relative = "fixtures/render/quota/workspace/workspace.lua";
  for (fs::path start : {fs::path(__FILE__).parent_path(), fs::current_path()})
    for (auto at = start; !at.empty(); at = at.parent_path()) {
      if (fs::exists(at / relative))
        return fs::canonical(at / relative);
      if (at == at.parent_path())
        break;
    }
  return {};
}

/// What a provider adds to its four-key result.
enum class decoy { none, commands, other_key };

struct drive_result {
  int                      status = -1; ///< Exit status the command layer would give: 0, or the failed() status.
  value_ptr                document;    ///< The successful response value, empty on failure.
  std::string              code, message, field, file;
  int                      opens = 0;
  std::vector<std::string> jobs; ///< Job IDs the provider was asked to capture.
  std::string              markdown;
};

/// Test-owned services: a real capture of the quota fixture for the selected job; every other service refuses.
template <class Services, class Render, class Init, class History, class Selector, class Export, class Auth>
class orchestrator_services final : public Services {
  using result_type = decltype(std::declval<Services&>().capture_orchestrator(
      std::declval<const Auth&>(), std::declval<const value_ptr&>(), std::string_view{}));
  planar::engine::capture_session _session;
  std::string                     _file;
  value_ptr                       _workspace;
  decoy                           _decoy;
  std::string                     _job_file; ///< Replacement spelling of the captured job file; empty keeps the listed one.
  static result_type              refusal() {
    return {{}, planar::cmd::detail::native_error({"internal", "Invalid embedded command protocol"})};
  }
  template <class T> static T unwrap(std::expected<T, planar::engine::diagnostic> result) {
    if (!result)
      throw result.error();
    return std::move(*result);
  }
  static void unwrap(std::expected<void, planar::engine::diagnostic> result) {
    if (!result)
      throw result.error();
  }

public:
  int                      opens = 0;
  std::vector<std::string> jobs;
  explicit orchestrator_services(decoy kind = decoy::none, std::string job_file = {})
      : _decoy(kind), _job_file(std::move(job_file)) {
  }
  using Services::capture_orchestrator;
  using Services::open_workspace;
  result_type open_workspace(const Render&) override {
    return refusal();
  }
  result_type capture_render(const Render&, const value_ptr&) override {
    return refusal();
  }
  result_type initialize_ledger(const Init&) override {
    return refusal();
  }
  result_type read_history(const History&, const Selector&) override {
    return refusal();
  }
  result_type export_source(const Export&) override {
    return refusal();
  }
  result_type open_workspace(const Auth& options) override {
    using namespace planar::cmd::detail;
    try {
      ++opens;
      const fs::path file(options.workspace);
      unwrap(_session.add_root("workspace", file.parent_path().string()));
      _file      = file.filename().string();
      _workspace = unwrap(_session.load(_file)).root;
      return {object({{"file", text(_file)}, {"workspace", _workspace}}), {}};
    } catch (const planar::engine::diagnostic& e) {
      return {{}, native_error(e)};
    }
  }
  result_type capture_orchestrator(const Auth& options, const value_ptr& route, std::string_view job) override {
    using namespace planar::cmd::detail;
    jobs.emplace_back(job);
    try {
      auto      spec_path = route->fields.at("spec");
      auto      spec      = unwrap(_session.load(spec_path->string));
      value_ptr job_path;
      for (const auto& entry : route->fields.at("jobs")->items)
        if (entry->fields.at("id")->string == job)
          job_path = entry->fields.at("file");
      if (!job_path)
        return refusal();
      auto loaded = unwrap(_session.load(job_path->string));
      unwrap(_session.add_input("registry", options.registry));
      auto                   registry = unwrap(_session.read("registry", "input.json"));
      auto                   snapshot = unwrap(_session.finish());
      std::vector<value_ptr> retained, workspace_files;
      for (const auto& file : *snapshot) {
        retained.push_back(object(
            {{"root", text(file.root)}, {"path", text(file.path)}, {"mode", text(file.mode)}, {"bytes", text(file.bytes)}}));
        if (file.root == "workspace")
          workspace_files.push_back(object({{"path", text(file.path)}, {"bytes", text(file.bytes)}}));
      }
      auto bundle =
          object({{"workspace", object({{"file", text(_file)}, {"value", _workspace}})},
                  {"spec", object({{"file", spec_path}, {"value", spec.root}})},
                  {"jobs", array({object({{"file", _job_file.empty() ? job_path : text(_job_file)}, {"value", loaded.root}})})},
                  {"registry", object({{"file", text("input.json")}, {"bytes", text(registry.bytes)}})},
                  {"files", array(workspace_files)}});
      std::map<std::string, value_ptr> result{{"bundle", bundle},
                                              {"capture", object({{"status", text("not_captured")}})},
                                              {"files", array(retained)},
                                              {"job", text(std::string(job))}};
      if (_decoy == decoy::commands)
        result["commands"] = array({text("zzz")});
      if (_decoy == decoy::other_key)
        result["extra"] = text("extra");
      return {object(std::move(result)), {}};
    } catch (const planar::engine::diagnostic& e) {
      return {{}, native_error(e)};
    }
  }
};

/// Run orchestrator through one generated variant and map the result as the command layer does.
template <class Vm, class Services, class Render, class Init, class History, class Selector, class Export, class Auth, class Verb>
drive_result drive_orchestrator(const fs::path& workspace_file, std::string job, decoy kind = decoy::none,
                                std::string job_file = {}) {
  drive_result out;
  auto         vm = Vm::create();
  REQUIRE(vm);
  orchestrator_services<Services, Render, Init, History, Selector, Export, Auth> services(kind, std::move(job_file));
  Auth                                                                           options;
  options.verb      = Verb::orchestrator;
  options.workspace = workspace_file.string();
  options.registry  = (workspace_file.parent_path().parent_path() / "registry" / "quota-tools.json").string();
  options.job       = std::move(job);
  auto result       = vm->invoke_authoring(options, services);
  out.opens         = services.opens;
  out.jobs          = services.jobs;
  if (!result) {
    const auto response = planar::cmd::detail::failed(planar::cmd::detail::native_error(result.error()));
    out.status          = response.status;
    out.code            = result.error().code;
    out.message         = result.error().message;
    return out;
  }
  if (!(*result)->fields.at("ok")->boolean) {
    const auto error    = (*result)->fields.at("error");
    const auto response = planar::cmd::detail::failed(error);
    out.status          = response.status;
    out.code            = error->fields.at("code")->string;
    out.message         = error->fields.at("message")->string;
    if (error->fields.contains("field"))
      out.field = error->fields.at("field")->string;
    if (error->fields.contains("file"))
      out.file = error->fields.at("file")->string;
    return out;
  }
  out.status   = 0;
  out.document = (*result)->fields.at("value");
  if (out.document->fields.contains("prompt"))
    out.markdown = out.document->fields.at("prompt")->fields.at("markdown")->string;
  return out;
}
#define RA01_DRIVE_ORCHESTRATOR(ns, ...)                                                                                         \
  drive_orchestrator<planar::engine::ns::command_vm, planar::engine::ns::command_services,                                       \
                     planar::engine::ns::command_render_options, planar::engine::ns::command_init_options,                       \
                     planar::engine::ns::command_history_options, planar::engine::ns::command_history_selector,                  \
                     planar::engine::ns::command_export_options, planar::engine::ns::command_authoring_options,                  \
                     planar::engine::ns::command_authoring_verb>(__VA_ARGS__)

/// Action lines of the rendered Markdown: command to status, in document order.
std::vector<std::pair<std::string, std::string>> actions_of(const std::string& markdown) {
  std::vector<std::pair<std::string, std::string>> out;
  std::size_t                                      begin = 0;
  while (begin < markdown.size()) {
    auto end = markdown.find('\n', begin);
    if (end == std::string::npos)
      end = markdown.size();
    const auto line = markdown.substr(begin, end - begin);
    begin           = end + 1;
    const auto open = line.find(" (");
    const auto shut = line.find("): ");
    if (!line.starts_with("- ") || open == std::string::npos || shut == std::string::npos || open > shut)
      continue;
    const auto status = line.substr(open + 2, shut - open - 2);
    if (status == "available" || status == "future")
      out.emplace_back(line.substr(2, open - 2), status);
  }
  return out;
}
std::vector<std::string> names_with(const std::vector<std::pair<std::string, std::string>>& actions, std::string_view status) {
  std::vector<std::string> out;
  for (const auto& [command, label] : actions)
    if (label == status)
      out.push_back(command);
  return out;
}

const std::vector<std::string> catalog_eleven = {"critique", "explain-rejection", "export",       "history", "ingest",     "init",
                                                 "load",     "observe",           "orchestrator", "render",  "spec-prompt"};
/// The twelve future names of RA-7, sorted.
const std::vector<std::string> future_twelve = {"activate", "check",   "check-recover", "checkpoint", "claim",    "preview",
                                                "recover",  "refresh", "release",       "status",     "takeover", "verdict"};
std::vector<std::string>       without(std::vector<std::string> names, std::string_view removed) {
  std::erase(names, std::string(removed));
  return names;
}
std::vector<std::string> sorted_union(std::vector<std::string> a, const std::vector<std::string>& b) {
  a.insert(a.end(), b.begin(), b.end());
  std::ranges::sort(a);
  a.erase(std::unique(a.begin(), a.end()), a.end());
  return a;
}
} // namespace

// W2-70. Mutants killed: orchestrator.lua labels availability from a literal eleven-name list instead of the commands
// names (status stays future in variant 20), or the runtime appends a literal eleven-name list. The provider returns only the
// four provider keys, so the names can only come from the native derivation.
TEST_CASE("RA01-B2 orchestrator label flips to available for a catalog stub verb", "[authoring-catalog]") {
  const auto workspace = quota_workspace_file();
  REQUIRE_FALSE(workspace.empty());

  const auto control = RA01_DRIVE_ORCHESTRATOR(authoring_variant_0, workspace, "combined");
  INFO(control.code << ": " << control.message);
  REQUIRE(control.status == 0);
  CHECK(control.opens == 1);
  CHECK(control.jobs == std::vector<std::string>{"combined"});
  REQUIRE(control.document);
  CHECK(control.document->fields.at("prompt")->fields.at("kind")->string == "orchestrator");
  const auto base = actions_of(control.markdown);
  CHECK(base.size() == 23);
  CHECK(names_with(base, "available") == catalog_eleven);
  CHECK(names_with(base, "future") == future_twelve);

  const auto flipped = RA01_DRIVE_ORCHESTRATOR(authoring_variant_20, workspace, "combined");
  INFO(flipped.code << ": " << flipped.message);
  REQUIRE(flipped.status == 0);
  CHECK(flipped.jobs == std::vector<std::string>{"combined"});
  const auto labels = actions_of(flipped.markdown);
  CHECK(labels.size() == 23);
  // status is a sealed catalog name in variant 20, so it is available and the other eleven future labels do not change.
  CHECK(names_with(labels, "available") == sorted_union(catalog_eleven, {"status"}));
  CHECK(names_with(labels, "future") == without(future_twelve, "status"));
}

// W2-77. Mutants killed: call_service_native (a) appends a literal eleven-name list (variant 20 would not list status),
// (b) takes commands from the provider result (the decoy would be accepted), (c) appends in bundle_table order instead of
// sorted (orchestrator.lua refuses an unsorted commands array internal, so the control run would fail).
TEST_CASE("RA01-B2 native appends the sorted sealed command names to the orchestrator capture", "[authoring-catalog]") {
  const auto workspace = quota_workspace_file();
  REQUIRE_FALSE(workspace.empty());

  // Lua sees the sealed catalog sorted: eleven names in variant 0 and twelve in variant 20, exactly the available labels.
  const auto v0 = RA01_DRIVE_ORCHESTRATOR(authoring_variant_0, workspace, "split");
  INFO(v0.code << ": " << v0.message);
  REQUIRE(v0.status == 0);
  CHECK(names_with(actions_of(v0.markdown), "available") == catalog_eleven);
  CHECK(names_with(actions_of(v0.markdown), "available").size() == 11);
  const auto v20 = RA01_DRIVE_ORCHESTRATOR(authoring_variant_20, workspace, "split");
  INFO(v20.code << ": " << v20.message);
  REQUIRE(v20.status == 0);
  CHECK(names_with(actions_of(v20.markdown), "available") == sorted_union(catalog_eleven, {"status"}));
  CHECK(names_with(actions_of(v20.markdown), "available").size() == 12);

  // A provider result that already carries commands (decoy ["zzz"]) or any fifth key is refused internal exit 4 and the
  // decoy name never reaches the document.
  const auto decoys = {decoy::commands, decoy::other_key};
  for (const auto kind : decoys) {
    const auto a = RA01_DRIVE_ORCHESTRATOR(authoring_variant_0, workspace, "combined", kind);
    CAPTURE(static_cast<int>(kind));
    CHECK(a.status == 4);
    CHECK(a.code == "internal");
    CHECK(a.message == "Invalid embedded command protocol");
    CHECK_FALSE(a.document);
    CHECK(a.markdown.empty());
    const auto b = RA01_DRIVE_ORCHESTRATOR(authoring_variant_20, workspace, "combined", kind);
    CHECK(b.status == 4);
    CHECK(b.code == "internal");
    CHECK(b.message == "Invalid embedded command protocol");
    CHECK_FALSE(b.document);
  }
}

// W2-80, Unlisted branch. Mutant killed: schema.lua selected_job mode skips the whole job loop and listing checks. The
// provider returns the selected job under a file spelling the workspace does not list; native never does this, so it is only
// reachable through a test-owned provider. The file sorts before the listed one so it is the first error.
TEST_CASE("RA01-B2 orchestrator selected-job mode refuses an unlisted job declaration", "[authoring-catalog]") {
  const auto workspace = quota_workspace_file();
  REQUIRE_FALSE(workspace.empty());
  const auto control = RA01_DRIVE_ORCHESTRATOR(authoring_variant_0, workspace, "combined");
  INFO(control.code << ": " << control.message);
  REQUIRE(control.status == 0);
  const auto refused = RA01_DRIVE_ORCHESTRATOR(authoring_variant_0, workspace, "combined", decoy::none, "aaa.lua");
  CHECK(refused.status == 3);
  CHECK(refused.code == "reference");
  CHECK(refused.message == "Unlisted job declaration");
  CHECK(refused.field == "job");
  CHECK(refused.file == "aaa.lua");
  CHECK_FALSE(refused.document);
}
