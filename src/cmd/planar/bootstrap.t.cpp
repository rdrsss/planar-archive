// Replacement invocation boundary tests; no legacy test harness or runtime.
#include <catch2/catch_test_macros.hpp>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
import std;
import cli11;
import planar.cmd.envelope;
import planar.engine.ledger.format;
import planar.engine.ledger.acceptance;
import planar.engine.acceptance.intent;
import planar.cmd.planar.handlers.render;
import planar.cmd.bootstrap;
import planar.cmd.invocation;
import planar.engine.capture.host_path;
import planar.cmd.planar.handlers.init;
import planar.cmd.planar.handlers.history;
import planar.cmd.planar.handlers.source_export;
import planar.cmd.planar.handlers.load;
import planar.engine.ledger.types;

TEST_CASE("help is explicit and buffered", "[bootstrap]") {
  const std::array<std::string_view, 1> args   = {"--help"};
  const auto                            result = planar::cmd::invoke(args);
  CHECK(result.status == 0);
  CHECK(result.error.empty());
  CHECK(result.output.find("--capture-outputs") != std::string::npos);
}
TEST_CASE("missing explicit files refuse without partial output", "[bootstrap]") {
  const std::array<std::string_view, 11> args   = {"render",     "--workspace", "workspace.lua", "--registry",
                                                   "tools.json", "--root",      "source=src",    "--job",
                                                   "combined",   "--role",      "builder"};
  const auto                             result = planar::cmd::invoke(args);
  CHECK(result.status == 4);
  CHECK(result.output.empty());
  CHECK(result.error.find("io") != std::string::npos);
}
TEST_CASE("legacy commands and implicit discovery are refused", "[bootstrap]") {
  CHECK(planar::cmd::invoke({}).status == 2);
  for (const std::string_view command : {"status", "task", "claim", "render"}) {
    const std::array args = {command};
    CHECK(planar::cmd::invoke(args).status == 2);
  }
}

TEST_CASE("render handler attaches CLI11 bindings without executing capture", "[bootstrap][cli11]") {
  CLI::App root{"", "planar"};
  root.set_help_flag();
  planar::cmd::handlers::render_options options;
  auto&                                 command = planar::cmd::handlers::attach_render(root, options);
  std::vector<std::string> arguments = {"builder",      "--role",     "combined",    "--job",       "source=missing", "--root",
                                        "missing.json", "--registry", "missing.lua", "--workspace", "render"};
  REQUIRE_NOTHROW(root.parse(arguments));
  CHECK(command.parsed());
  CHECK(options.values[0] == "missing.lua");
  CHECK(options.values[3] == "combined");
  CHECK_FALSE(options.capture_outputs);
  // Parsing does not access the host. Only explicit dispatch reaches capture.
  CHECK(planar::cmd::handlers::render_command(options).status == 4);
}

TEST_CASE("CLI11 rejects repeated handler bindings before execution", "[bootstrap][cli11]") {
  for (const auto& duplicate :
       {std::vector<std::string>{"again", "--workspace"}, std::vector<std::string>{"--capture-outputs", "--capture-outputs"}}) {
    CLI::App root{"", "planar"};
    root.set_help_flag();
    planar::cmd::handlers::render_options options;
    planar::cmd::handlers::attach_render(root, options);
    auto                           arguments = duplicate;
    const std::vector<std::string> valid     = {"builder",        "--role",      "combined",     "--job",
                                                "source=missing", "--root",      "missing.json", "--registry",
                                                "missing.lua",    "--workspace", "render"};
    arguments.insert(arguments.end(), valid.begin(), valid.end());
    CHECK_THROWS_AS(root.parse(arguments), CLI::ParseError);
  }
}

TEST_CASE("init handler binds explicit options without filesystem access", "[bootstrap][cli11][init]") {
  CLI::App root{"", "planar"};
  root.set_help_flag();
  planar::cmd::handlers::init_options options;
  auto&                               command   = planar::cmd::handlers::attach_init(root, options);
  std::vector<std::string>            arguments = {"sample", "--workspace-id", "missing/target.db", "--ledger", "init"};
  REQUIRE_NOTHROW(root.parse(arguments));
  CHECK(command.count() == 1);
  CHECK(options.ledger == "missing/target.db");
  CHECK(options.workspace_id == "sample");
}

TEST_CASE("ledger error adapter pins every typed refusal without native messages", "[bootstrap][init]") {
  using enum planar::engine::ledger::types::format_error_code;
  using code_type = planar::engine::ledger::types::format_error_code;
  struct mapping {
    code_type        native;
    std::string_view code;
    int              status;
  };
  const std::array cases = {mapping{invalid_argument, "schema", 3},
                            mapping{not_found, "ledger_not_found", 3},
                            mapping{exists, "ledger_exists", 3},
                            mapping{unsafe_path, "path_denied", 3},
                            mapping{foreign_ledger, "foreign_ledger", 3},
                            mapping{unsupported_version, "unsupported_ledger_version", 3},
                            mapping{unsupported_state, "unsupported_ledger_version", 3},
                            mapping{corrupt, "ledger_corrupt", 4},
                            mapping{busy, "ledger_busy", 4},
                            mapping{storage_full, "storage_full", 4},
                            mapping{io, "io", 4},
                            mapping{publication_uncertain, "io", 4},
                            mapping{recovery_required, "io", 4},
                            mapping{hash_collision, "hash_collision", 3},
                            mapping{request_conflict, "request_conflict", 3},
                            mapping{revision_conflict, "revision_conflict", 3},
                            mapping{capture_changed, "capture_changed", 3}};
  const std::array messages{"Invalid ledger arguments",
                            "Ledger was not found",
                            "Ledger target already exists",
                            "Ledger path requires existing real parent directories and a valid target",
                            "File is not a Planar ledger",
                            "Ledger version or state is unsupported",
                            "Ledger version or state is unsupported",
                            "Ledger integrity verification failed",
                            "Ledger is busy",
                            "Ledger storage is full",
                            "Ledger operation failed",
                            "Ledger may be published; inspect the explicit target before retrying",
                            "Explicit ledger recovery is required",
                            "Ledger record identity collision",
                            "Ledger request conflicts with its saved intent",
                            "Ledger revision does not match the expected revision",
                            "Captured declaration inputs changed"};
  std::size_t      index = 0;
  for (const auto& entry : cases) {
    CAPTURE(entry.code);
    const auto error = planar::cmd::detail::ledger_error({entry.native, "/private/marker captured bytes"});
    CHECK(error->fields.at("code")->string == entry.code);
    CHECK(error->fields.at("field")->string.empty());
    CHECK(error->fields.at("message")->string.find("private") == std::string::npos);
    if (entry.code == "io") {
      CHECK(error->fields.at("details")->fields.at("operation")->string == "ledger");
      CHECK(error->fields.size() == 4);
    } else {
      CHECK(error->fields.size() == 3);
    }
    if (entry.native == publication_uncertain)
      CHECK(error->fields.at("message")->string == "Ledger may be published; inspect the explicit target before retrying");
    if (entry.native == recovery_required)
      CHECK(error->fields.at("message")->string == "Explicit ledger recovery is required");
    const auto response = planar::cmd::detail::failed(error);
    CHECK(response.status == entry.status);
    CHECK(response.output.empty());
    CHECK(response.error.ends_with("\n"));
    CHECK(response.error.find("private") == std::string::npos);
    const auto details  = entry.code == "io" ? R"(,"details":{"operation":"ledger"})" : "";
    const auto expected = std::string(R"({"error":{"code":")") + std::string(entry.code) + "\"" + details +
                          R"(,"field":"","message":")" + messages[index++] + R"("},"kind":"error","version":1})" + "\n";
    CHECK(response.error == expected);
    const auto decoded = planar::engine::codec_decode(response.error);
    REQUIRE(decoded);
    CHECK((*decoded)->fields.at("kind")->string == "error");
    CHECK((*decoded)->fields.at("version")->integer == 1);
  }
}

TEST_CASE("history CLI11 bindings validate selectors before any ledger access", "[bootstrap][cli11]") {
  CLI::App root{"", "planar"};
  root.set_help_flag();
  planar::cmd::handlers::history_options options;
  auto&                                  command = planar::cmd::handlers::attach_history(root, options);
  std::vector<std::string> arguments{"100", "--limit", "9223372036854775807", "--after-event", "missing", "--ledger", "history"};
  REQUIRE_NOTHROW(root.parse(arguments));
  CHECK(command.parsed());
  CHECK(options.limit == "100");
  CHECK(options.after_event == "9223372036854775807");
  CHECK(options.after_event_number == std::numeric_limits<std::int64_t>::max());
  CHECK(options.limit_number == 100);
  CHECK(planar::cmd::handlers::history_command(options).status == 3);
}

TEST_CASE("export binds strict explicit options before native access", "[bootstrap][cli11][export]") {
  CLI::App root{"", "planar"};
  root.set_help_flag();
  planar::cmd::handlers::export_options options;
  auto&                                 command = planar::cmd::handlers::attach_export(root, options);
  std::vector<std::string>              args{"missing/../target", "--target", "9223372036854775807", "--revision", "missing",
                                             "--ledger",          "export"};
  REQUIRE_NOTHROW(root.parse(args));
  CHECK(command.count() == 1);
  CHECK(options.target == "missing/../target");
  CHECK(options.revision_number == std::numeric_limits<std::int64_t>::max());
  const auto result = planar::cmd::handlers::export_command(options);
  CHECK(result.status == 3);
  CHECK(result.output.empty());
  CHECK(result.stdout_error.empty());
}

TEST_CASE("export adapters redact refusals and validate generated stage tokens", "[bootstrap][export]") {
  using namespace planar::engine::source_export;
  using planar::cmd::handlers::export_failure;
  struct mapping {
    error_code       native;
    std::string_view code;
    int              status;
  };
  for (auto m :
       {mapping{error_code::revision_not_found, "revision_not_found", 3}, mapping{error_code::invalid_argument, "schema", 3},
        mapping{error_code::path_denied, "path_denied", 3}, mapping{error_code::target_exists, "target_exists", 3},
        mapping{error_code::io, "io", 4}, mapping{error_code::publication_uncertain, "io", 4}}) {
    auto result = export_failure({m.native, {}, "/PRIVATE/native bytes"});
    CHECK(result.status == m.status);
    CHECK(result.output.empty());
    CHECK(result.stdout_error.empty());
    CHECK(result.error.find("PRIVATE") == std::string::npos);
    auto decoded = planar::engine::codec_decode(result.error);
    REQUIRE(decoded);
    CHECK((*decoded)->fields.at("error")->fields.at("code")->string == m.code);
  }
  const auto token = ".planar-export-0123456789abcdef0123456789abcdef";
  auto       valid = export_failure({error_code::cleanup_incomplete, {}, token});
  CHECK(valid.status == 4);
  CHECK(valid.error.find(token) != std::string::npos);
  CHECK(valid.error.find("inspect the sibling") != std::string::npos);
  for (std::string invalid : {"", "../PRIVATE", ".planar-export-0123456789abcdef0123456789abcdeF",
                              ".planar-export-0123456789abcdef0123456789abcdef/PRIVATE"}) {
    auto result = export_failure({error_code::cleanup_incomplete, {}, invalid});
    CHECK(result.status == 4);
    CHECK(result.error.find("PRIVATE") == std::string::npos);
    CHECK(result.error.find("location") != std::string::npos);
    auto decoded = planar::engine::codec_decode(result.error);
    REQUIRE(decoded);
    CHECK((*decoded)->fields.at("error")->fields.at("code")->string == "internal");
  }
  auto storage = export_failure(
      {error_code::storage,
       planar::engine::ledger::types::format_error{planar::engine::ledger::types::format_error_code::not_found, "PRIVATE"},
       "PRIVATE"});
  CHECK(storage.status == 3);
  CHECK(storage.error.find("ledger_not_found") != std::string::npos);
  CHECK(storage.error.find("PRIVATE") == std::string::npos);
}

TEST_CASE("render mapping refusal precedes missing routed inputs", "[bootstrap][render]") {
  struct temporary_root {
    std::filesystem::path path;
    temporary_root()
        : path(std::filesystem::temp_directory_path() /
               std::format("planar-b2-route-{}", std::chrono::steady_clock::now().time_since_epoch().count())) {
      std::filesystem::create_directory(path);
    }
    ~temporary_root() {
      std::error_code ec;
      std::filesystem::remove_all(path, ec);
    }
  } temporary;
  auto file = temporary.path / "workspace.lua";
  std::filesystem::copy_file(PLANAR_B2_WORKSPACE_FIXTURE, file);
  const auto                             workspace = file.string();
  const std::array<std::string_view, 11> args   = {"render", "--workspace",          workspace, "--registry", "missing-registry",
                                                   "--root", "other=missing-source", "--job",   "combined",   "--role",
                                                   "builder"};
  auto                                   result = planar::cmd::invoke(args);
  CHECK(result.status == 3);
  CHECK(result.output.empty());
  auto error = planar::engine::codec_decode(result.error);
  REQUIRE(error);
  CHECK((*error)->fields.at("error")->fields.at("code")->string == "reference");
  CHECK((*error)->fields.at("error")->fields.at("field")->string == "root");
}
TEST_CASE("invocation paths snapshot cwd and workspace environment only when needed", "[bootstrap][workspace_selector]") {
  using namespace planar::engine;
  int  cwd_reads = 0, env_reads = 0;
  auto cwd = [&]() -> std::expected<std::string, diagnostic> {
    ++cwd_reads;
    return cwd_reads == 1 ? "/snapshot" : "/changed";
  };
  auto env = [&]() -> std::expected<std::string, diagnostic> {
    ++env_reads;
    return env_reads == 1 ? "/named/../workspaces" : "/changed";
  };
  command_options options = command_render_options{
      "demo", "tools.json", "source=src/../source", "job", "role", "source=base", "observations.json", {}, true};
  REQUIRE(planar::cmd::prepare_invocation_paths(options, cwd, env));
  const auto& render = std::get<command_render_options>(options);
  CHECK(render.workspace == "/workspaces/demo/workspace.lua");
  CHECK(render.registry == "/snapshot/tools.json");
  CHECK(render.root == "source=/snapshot/src/../source");
  CHECK(render.base_root == "source=/snapshot/base");
  CHECK(render.observations == "/snapshot/observations.json");
  CHECK(cwd_reads == 1);
  CHECK(env_reads == 1);
  options = command_render_options{"/abs/workspace.lua", "/tools.json", "source=/source", "job", "role", {}, {}, {}, false};
  REQUIRE(planar::cmd::prepare_invocation_paths(options, cwd, env));
  CHECK(cwd_reads == 1);
  CHECK(env_reads == 1);
  options = command_export_options{"relative.db", "target/../new", 1};
  REQUIRE(planar::cmd::prepare_invocation_paths(options, cwd, env));
  CHECK(std::get<command_export_options>(options).ledger == "/changed/relative.db");
  CHECK(std::get<command_export_options>(options).target == "/changed/target/../new");
  CHECK(cwd_reads == 2);
  CHECK(env_reads == 1);
  for (command_options ledger : {command_options(command_init_options{"relative.db", "demo"}),
                                 command_options(command_history_options{"relative.db", {}, {}, {}, {}, {}})}) {
    REQUIRE(planar::cmd::prepare_invocation_paths(ledger, cwd, env));
  }
  CHECK(cwd_reads == 4);
  CHECK(env_reads == 1);
}
TEST_CASE("invocation path refusal precedes cwd reads and native services", "[bootstrap][workspace_selector]") {
  using namespace planar::engine;
  int  cwd_reads = 0, env_reads = 0;
  auto cwd = [&]() -> std::expected<std::string, diagnostic> {
    ++cwd_reads;
    return std::unexpected(diagnostic{"io", "private cwd failure"});
  };
  auto env = [&]() -> std::expected<std::string, diagnostic> {
    ++env_reads;
    return snapshot_workspace_directory({});
  };
  for (const auto selector : {"bad.name", "bad.LUA", "..", "a b"}) {
    command_options options = command_render_options{selector, "tools.json", "source=src", "job", "role", {}, {}, {}, false};
    const auto      result  = planar::cmd::prepare_invocation_paths(options, cwd, env);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "usage");
  }
  CHECK(cwd_reads == 0);
  CHECK(env_reads == 0);
  command_options name = command_render_options{"demo", "tools.json", "source=src", "job", "role", {}, {}, {}, false};
  CHECK_FALSE(planar::cmd::prepare_invocation_paths(name, cwd, env));
  CHECK(env_reads == 1);
  CHECK(cwd_reads == 0);
  command_options invalid        = command_export_options{std::string("/bad\0private", 12), "relative", 1};
  const auto      invalid_result = planar::cmd::prepare_invocation_paths(invalid, cwd, env);
  REQUIRE_FALSE(invalid_result);
  CHECK(invalid_result.error().code == "path_denied");
  CHECK(cwd_reads == 0);
  command_options relative    = command_export_options{"relative.db", "target", 1};
  const auto      missing_cwd = planar::cmd::prepare_invocation_paths(relative, cwd, env);
  REQUIRE_FALSE(missing_cwd);
  CHECK(missing_cwd.error().code == "io");
  CHECK(missing_cwd.error().message == "Cannot obtain invocation cwd");
  CHECK(cwd_reads == 1);
  auto       malformed     = []() -> std::expected<std::string, diagnostic> { return "/a/../private"; };
  const auto malformed_cwd = planar::cmd::prepare_invocation_paths(relative, malformed, env);
  REQUIRE_FALSE(malformed_cwd);
  CHECK(malformed_cwd.error().code == "path_denied");
}
TEST_CASE("invocation refuses environment views before snapshot ownership and cwd", "[bootstrap][workspace_selector]") {
  using namespace planar::engine;
  constexpr std::size_t maximum = 64 * 1024 * 1024;
  std::string           directory(maximum + 1, 'x');
  directory.front() = '/';
  int  cwd_reads = 0, env_reads = 0;
  auto cwd = [&]() -> std::expected<std::string, diagnostic> {
    ++cwd_reads;
    return "/cwd";
  };
  auto env = [&]() -> std::expected<std::string, diagnostic> {
    ++env_reads;
    // This is a borrowed native view, just like getenv: no owned snapshot is made on refusal.
    return snapshot_workspace_directory(std::string_view(directory));
  };
  command_options options = command_render_options{"demo", "relative.json", "source=relative", "job", "role", {}, {}, {}, false};
  auto            oversized = planar::cmd::prepare_invocation_paths(options, cwd, env);
  REQUIRE_FALSE(oversized);
  CHECK(oversized.error().code == "resource_limit");
  CHECK(oversized.error().limit == "path_bytes");
  CHECK(oversized.error().maximum == maximum);
  CHECK(std::get<command_render_options>(options).workspace == "demo");
  CHECK(cwd_reads == 0);
  CHECK(env_reads == 1);
  directory.back() = '\0';
  auto invalid     = planar::cmd::prepare_invocation_paths(options, cwd, env);
  REQUIRE_FALSE(invalid);
  CHECK(invalid.error().code == "usage"); // Invalid text wins over the same excessive size.
  CHECK(cwd_reads == 0);
  CHECK(env_reads == 2);
  CHECK(snapshot_workspace_directory("/original/../base") == "/original/../base");
}

TEST_CASE("ledger authoritative diagnostics include stable revision and collision detail", "[bootstrap][ledger_response]") {
  using namespace planar::engine::ledger::types;
  using namespace planar::engine::history;
  using enum format_error_code;
  auto check = [](format_error error, std::string_view expected) {
    const auto result = planar::cmd::detail::failed(planar::cmd::detail::ledger_error(error));
    CHECK(result.status == 3);
    CHECK(result.output.empty());
    CHECK(result.error == expected);
  };
  check(
      {revision_conflict, "private request/path/body", revision_conflict_detail{1, 2}},
      R"({"error":{"code":"revision_conflict","details":{"actual":"2","expected":"1"},"field":"","message":"Ledger revision does not match the expected revision"},"kind":"error","version":1})"
      "\n");
  check(
      {revision_conflict, "private", revision_conflict_detail{std::nullopt, 9223372036854775807LL}},
      R"({"error":{"code":"revision_conflict","details":{"actual":"9223372036854775807","expected":"none"},"field":"","message":"Ledger revision does not match the expected revision"},"kind":"error","version":1})"
      "\n");
  check(
      {revision_conflict, "private", revision_conflict_detail{1, std::nullopt}},
      R"({"error":{"code":"revision_conflict","details":{"actual":"none","expected":"1"},"field":"","message":"Ledger revision does not match the expected revision"},"kind":"error","version":1})"
      "\n");
  record_digest digest{0x00, 0x01, 0x02, 0x03, 0x10, 0x20, 0x30, 0x40, 0x80, 0x90, 0xa0, 0xb0, 0xfc, 0xfd, 0xfe, 0xff};
  for (auto [kind, name] :
       {std::pair{record_kind::file, "file"}, {record_kind::directory, "directory"}, {record_kind::value, "value"}}) {
    auto expected =
        std::string(R"({"error":{"code":"hash_collision","details":{"digest":"00010203102030408090a0b0fcfdfeff","kind":")") +
        name +
        R"("},"field":"","message":"Ledger record identity collision"},"kind":"error","version":1})"
        "\n";
    check({hash_collision, "private payload/path", record_collision_detail{kind, digest}}, expected);
  }
  check(
      {hash_collision, "private request key/body/path/digest", record_collision_detail{record_kind::request, std::nullopt}},
      R"({"error":{"code":"hash_collision","details":{"kind":"request"},"field":"","message":"Ledger record identity collision"},"kind":"error","version":1})"
      "\n");
  check(
      {request_conflict, "private request key/body/path/digest"},
      R"({"error":{"code":"request_conflict","field":"","message":"Ledger request conflicts with its saved intent"},"kind":"error","version":1})"
      "\n");
}
TEST_CASE("ledger diagnostic malformed typed payloads refuse with exact redacted internal envelope",
          "[bootstrap][ledger_response]") {
  using namespace planar::engine::ledger::types;
  using namespace planar::engine::history;
  using enum format_error_code;
  const record_digest       digest{};
  std::vector<format_error> invalid{
      {revision_conflict, "secret", revision_conflict_detail{0, 1}},
      {revision_conflict, "secret", revision_conflict_detail{1, -1}},
      {revision_conflict, "secret", record_collision_detail{record_kind::file, digest}},
      {hash_collision, "secret", revision_conflict_detail{1, 2}},
      {hash_collision, "secret", record_collision_detail{static_cast<record_kind>(0), digest}},
      {hash_collision, "secret", record_collision_detail{static_cast<record_kind>(255), std::nullopt}},
      {hash_collision, "secret", record_collision_detail{record_kind::request, digest}}};
  for (auto kind : {record_kind::file, record_kind::directory, record_kind::value})
    invalid.push_back({hash_collision, "secret", record_collision_detail{kind, std::nullopt}});
  for (auto code :
       {invalid_argument, not_found, exists, unsafe_path, foreign_ledger, unsupported_version, corrupt, unsupported_state,
        recovery_required, busy, storage_full, io, publication_uncertain, request_conflict, capture_changed}) {
    invalid.push_back({code, "secret", revision_conflict_detail{1, 2}});
    invalid.push_back({code, "secret", record_collision_detail{record_kind::request, std::nullopt}});
  }
  for (const auto& error : invalid) {
    const auto result = planar::cmd::detail::failed(planar::cmd::detail::ledger_error(error));
    CHECK(result.status == 4);
    CHECK(result.output.empty());
    CHECK(result.error ==
          R"({"error":{"code":"internal","field":"","message":"Invalid embedded command protocol"},"kind":"error","version":1})"
          "\n");
  }
}

TEST_CASE("load handler attaches six required text bindings", "[bootstrap][cli11][load]") {
  CLI::App root{"", "planar"};
  root.set_help_flag();
  planar::cmd::handlers::load_options options;
  auto&                               command = planar::cmd::handlers::attach_load(root, options);
  REQUIRE(command.get_options().size() == 6);
  for (const auto* option : command.get_options()) {
    CHECK(option->get_required());
    CHECK(option->get_type_name().starts_with("TEXT"));
    CHECK(option->get_expected_min() == 1);
    CHECK(option->get_expected_max() == 1);
    CHECK(option->get_multi_option_policy() == CLI::MultiOptionPolicy::Throw);
  }
}

TEST_CASE("load invocation snapshots one cwd and environment with lexical targets", "[bootstrap][load]") {
  using namespace planar;
  engine::command_options options =
      engine::command_load_options{"ledger.db", "quota", "registry.json", "none", "key", "actor", {}};
  int  cwd_reads = 0, environment_reads = 0;
  auto result = cmd::prepare_invocation_paths(
      options,
      [&]() -> std::expected<std::string, engine::diagnostic> {
        ++cwd_reads;
        return "/scratch";
      },
      [&]() -> std::expected<std::string, engine::diagnostic> {
        ++environment_reads;
        return "/workspaces";
      });
  REQUIRE(result);
  const auto& load = std::get<engine::command_load_options>(options);
  CHECK(cwd_reads == 1);
  CHECK(environment_reads == 1);
  CHECK(load.workspace == "/workspaces/quota/workspace.lua");
  CHECK(load.ledger == "/scratch/ledger.db");
  CHECK(load.registry == "/scratch/registry.json");
  options = engine::command_load_options{
      "/missing/../ledger.db", "/missing/../workspace.lua", "/missing/../registry.json", "none", "key", "actor", {}};
  REQUIRE(cmd::prepare_invocation_paths(
      options,
      [&]() -> std::expected<std::string, engine::diagnostic> {
        ++cwd_reads;
        return std::unexpected(engine::diagnostic{"io", "unused"});
      },
      [&]() -> std::expected<std::string, engine::diagnostic> {
        ++environment_reads;
        return std::unexpected(engine::diagnostic{"usage", "unused"});
      }));
  CHECK(cwd_reads == 1);
  CHECK(environment_reads == 1);
  CHECK(std::get<engine::command_load_options>(options).ledger == "/missing/../ledger.db");
  CHECK(std::get<engine::command_load_options>(options).workspace == "/workspace.lua");
  CHECK(std::get<engine::command_load_options>(options).registry == "/registry.json");
}

TEST_CASE("load intent aggregate record bound remains a resource refusal before ledger access", "[bootstrap][load]") {
  constexpr std::size_t half_record = 32 * 1024 * 1024;
  std::string           workspace(half_record, 'w'), registry(half_record, 'r');
  workspace.front()                       = '/';
  registry.front()                        = '/';
  planar::engine::command_options options = planar::engine::command_load_options{
      "/nonexistent-planar-intent-bound/ledger.db", std::move(workspace), std::move(registry), "none", "bounded", "driver", {}};
  auto result = planar::cmd::execute(std::move(options));
  REQUIRE(result.status == 3);
  CHECK(result.output.empty());
  auto decoded = planar::engine::codec_decode(result.error);
  REQUIRE(decoded);
  const auto& error = (*decoded)->fields.at("error");
  CHECK(error->fields.at("code")->string == "resource_limit");
  CHECK(error->fields.at("field")->string.empty());
  CHECK(result.error.ends_with("\n"));
  CHECK(result.error.find("nonexistent-planar-intent-bound") == std::string::npos);
  CHECK(result.error.size() < 1024);
  CHECK(
      result.error ==
      R"({"error":{"code":"resource_limit","field":"","message":"Load request exceeds native resource limits"},"kind":"error","version":1})"
      "\n");
}

namespace {
// Verification-only catalogs. No VM, file capture or command dispatch occurs.
struct envelope_spike_catalog {
  CLI::App                               root{"", "planar"};
  planar::cmd::handlers::render_options  render;
  planar::cmd::handlers::init_options    init;
  planar::cmd::handlers::history_options history;
  planar::cmd::handlers::export_options  source_export;
  planar::cmd::handlers::load_options    load;
  std::string                            text;
  bool                                   flag = false;
  envelope_spike_catalog(bool prefix, bool extension) {
    root.set_help_flag();
    root.allow_extras(false);
    root.fallthrough(false);
    root.require_subcommand(0, 1);
    planar::cmd::handlers::attach_render(root, render);
    planar::cmd::handlers::attach_init(root, init);
    planar::cmd::handlers::attach_history(root, history);
    planar::cmd::handlers::attach_export(root, source_export);
    planar::cmd::handlers::attach_load(root, load);
    if (extension) {
      auto* command = root.add_subcommand("inspect");
      command->set_help_flag();
      command->add_option("--text", text)->required()->multi_option_policy(CLI::MultiOptionPolicy::Throw);
      command->add_flag("--flag", flag)->multi_option_policy(CLI::MultiOptionPolicy::Throw);
    }
    root.prefix_command(prefix);
  }
};

std::size_t envelope_spike_consumed(const CLI::App& root) {
  std::size_t consumed = 0;
  for (const auto* command : root.get_subcommands()) {
    consumed += command->count();
    for (const auto* option : command->get_options())
      consumed += option->count() * (option->get_expected_max() == 0 ? 1 : 2);
  }
  return consumed;
}

struct envelope_spike_result {
  std::string              outcome;
  std::string              profile;
  std::vector<std::string> tail;
  std::size_t              prefix_consumed = 0;
  std::size_t              probe_consumed  = 0;
  std::size_t              final_consumed  = 0;
  bool                     final_attempted = false;
};

envelope_spike_result envelope_spike_parse(const std::vector<std::string>& argv) {
  envelope_spike_result result;
  // Exact existing whole-invocation separate-token guard; no option discovery.
  if (std::ranges::any_of(argv, [](std::string_view token) { return token.starts_with("--") && token.contains('='); })) {
    result.outcome = "syntax";
    return result;
  }
  envelope_spike_catalog initial{false, false};
  auto*                  envelope = initial.root.add_subcommand("with-profile");
  envelope->set_help_flag();
  envelope->prefix_command();
  auto* selector =
      envelope->add_option("--stdlib-profile", result.profile)->required()->multi_option_policy(CLI::MultiOptionPolicy::Throw);
  bool  help            = false;
  auto* selected_help   = envelope->add_flag("--help", help)->multi_option_policy(CLI::MultiOptionPolicy::Throw);
  auto  snapshot_prefix = [&] {
    auto passthrough = envelope->remaining_for_passthrough();
    result.tail.assign(passthrough.rbegin(), passthrough.rend());
    result.prefix_consumed = envelope->count() + 2 * selector->count() + selected_help->count();
  };
  auto arguments = std::vector<std::string>(argv.rbegin(), argv.rend());
  try {
    initial.root.parse(arguments);
  } catch (const CLI::ParseError& error) {
    snapshot_prefix();
    result.outcome = "prefix:" + error.get_name();
    return result;
  }
  snapshot_prefix();
  auto passthrough = envelope->remaining_for_passthrough();
  if (envelope->count() != 1 || selector->count() != 1 || result.profile.empty() ||
      result.prefix_consumed + result.tail.size() != argv.size()) {
    result.outcome = "prefix:metadata";
    return result;
  }
  if (help) {
    result.outcome = result.tail.empty() ? "selected-help" : "prefix:help-tail";
    return result;
  }
  if (result.tail.empty()) {
    result.outcome = "prefix:empty-tail";
    return result;
  }
  envelope_spike_catalog probe{true, false};
  auto                   probe_arguments = passthrough;
  try {
    probe.root.parse(probe_arguments);
  } catch (const CLI::ParseError& error) {
    result.probe_consumed = envelope_spike_consumed(probe.root);
    result.outcome        = "probe:" + error.get_name();
    return result;
  }
  result.probe_consumed = envelope_spike_consumed(probe.root);
  if (!probe.root.get_subcommands().empty() &&
      (result.probe_consumed != result.tail.size() || !probe.root.remaining(true).empty())) {
    result.outcome = "probe:metadata";
    return result;
  }
  envelope_spike_catalog final{false, true};
  auto                   final_arguments = passthrough;
  result.final_attempted                 = true;
  try {
    final.root.parse(final_arguments);
  } catch (const CLI::ParseError& error) {
    result.final_consumed = envelope_spike_consumed(final.root);
    result.outcome        = "final:" + error.get_name();
    return result;
  }
  result.final_consumed = envelope_spike_consumed(final.root);
  result.outcome        = final.root.get_subcommands().size() == 1 && result.final_consumed == result.tail.size() &&
                                  final.root.remaining(true).empty()
                              ? "accepted"
                              : "final:metadata";
  return result;
}
} // namespace

TEST_CASE("X01 CLI11 envelope and sealed catalog matrix", "[bootstrap][cli11][x01]") {
  struct example {
    std::string              id;
    std::vector<std::string> args;
  };
  const std::vector<example> examples = {
      {"ordered-extension", {"with-profile", "--stdlib-profile", "p.json", "inspect", "--text", "value", "--flag"}},
      {"shipped-history", {"with-profile", "--stdlib-profile", "p.json", "history", "--ledger", "missing"}},
      {"no-selector", {"with-profile", "inspect", "--text", "value"}},
      {"no-value", {"with-profile", "--stdlib-profile"}},
      {"empty-value", {"with-profile", "--stdlib-profile", "", "inspect", "--text", "value"}},
      {"verb-as-value", {"with-profile", "--stdlib-profile", "inspect", "--text", "value"}},
      {"duplicate-selector", {"with-profile", "--stdlib-profile", "p", "--stdlib-profile", "q", "inspect", "--text", "v"}},
      {"unknown-option-value", {"with-profile", "--stdlib-profile", "--bogus", "inspect", "--text", "v"}},
      {"known-option-value", {"with-profile", "--stdlib-profile", "--help", "inspect", "--text", "v"}},
      {"missing-help-value", {"with-profile", "--stdlib-profile", "--help"}},
      {"extra-prefix-option", {"with-profile", "--stdlib-profile", "p", "--bogus", "inspect", "--text", "v"}},
      {"option-before-selector", {"with-profile", "--bogus", "--stdlib-profile", "p", "inspect", "--text", "v"}},
      {"unknown-verb", {"with-profile", "--stdlib-profile", "p", "absent", "--text", "v"}},
      {"profile-as-tail-value", {"with-profile", "--stdlib-profile", "p", "inspect", "--text", "--stdlib-profile"}},
      {"tail-selector", {"with-profile", "--stdlib-profile", "p", "inspect", "--stdlib-profile", "q", "--text", "v"}},
      {"tail-option-value", {"with-profile", "--stdlib-profile", "p", "inspect", "--text", "--bogus"}},
      {"tail-known-option-value", {"with-profile", "--stdlib-profile", "p", "inspect", "--text", "--flag"}},
      {"no-tail", {"with-profile", "--stdlib-profile", "p"}},
      {"prefix-terminator", {"with-profile", "--stdlib-profile", "p", "--", "inspect", "--text", "v"}},
      {"tail-terminator", {"with-profile", "--stdlib-profile", "p", "inspect", "--text", "v", "--"}},
      {"terminator-value", {"with-profile", "--stdlib-profile", "--", "inspect", "--text", "v"}},
      {"selected-help", {"with-profile", "--stdlib-profile", "p", "--help"}},
      {"help-before-selector", {"with-profile", "--help", "--stdlib-profile", "p"}},
      {"help-with-tail", {"with-profile", "--stdlib-profile", "p", "--help", "inspect", "--text", "v"}},
      {"help-after-verb", {"with-profile", "--stdlib-profile", "p", "inspect", "--help"}},
      {"duplicate-help", {"with-profile", "--stdlib-profile", "p", "--help", "--help"}},
      {"shipped-missing", {"with-profile", "--stdlib-profile", "p", "history"}},
      {"shipped-duplicate", {"with-profile", "--stdlib-profile", "p", "history", "--ledger", "a", "--ledger", "b"}},
      {"shipped-extra", {"with-profile", "--stdlib-profile", "p", "history", "--ledger", "a", "--bogus", "b"}},
      {"shipped-terminator", {"with-profile", "--stdlib-profile", "p", "history", "--ledger", "a", "--"}},
      {"name-equals", {"with-profile", "--stdlib-profile=p", "inspect", "--text", "v"}},
      {"tail-name-equals", {"with-profile", "--stdlib-profile", "p", "inspect", "--text=v"}},
      {"root-terminator", {"--", "with-profile", "--stdlib-profile", "p", "inspect", "--text", "v"}},
      {"shipped-init", {"with-profile", "--stdlib-profile", "p", "init", "--ledger", "a", "--workspace-id", "w"}},
      {"shipped-export",
       {"with-profile", "--stdlib-profile", "p", "export", "--ledger", "a", "--revision", "1", "--target", "out"}},
      {"shipped-load",
       {"with-profile", "--stdlib-profile", "p", "load", "--ledger", "a", "--workspace", "w", "--registry", "r",
        "--expect-revision", "none", "--request-id", "k", "--actor", "a"}},
      {"shipped-render",
       {"with-profile", "--stdlib-profile", "p", "render", "--workspace", "w", "--registry", "r", "--root", "s=src", "--job", "j",
        "--role", "r"}},
      {"init-missing", {"with-profile", "--stdlib-profile", "p", "init", "--ledger", "a"}},
      {"export-malformed",
       {"with-profile", "--stdlib-profile", "p", "export", "--ledger", "a", "--revision", "bad", "--target", "out"}},
      {"load-missing", {"with-profile", "--stdlib-profile", "p", "load", "--ledger", "a"}},
      {"render-missing", {"with-profile", "--stdlib-profile", "p", "render", "--workspace", "w"}},
      {"help-terminator", {"with-profile", "--stdlib-profile", "p", "--help", "--"}},
      {"empty-tail-value", {"with-profile", "--stdlib-profile", "p", "inspect", "--text", ""}},
      {"second-command", {"with-profile", "--stdlib-profile", "p", "history", "--ledger", "a", "inspect", "--text", "v"}},
  };
  // Characterization expectations were frozen from the exploratory pinned-library run.
  const std::vector<envelope_spike_result> expected = {
      {"accepted", "p.json", {"inspect", "--text", "value", "--flag"}, 3, 0, 4, true},
      {"accepted", "p.json", {"history", "--ledger", "missing"}, 3, 3, 3, true},
      {"prefix:RequiredError", "", {"inspect", "--text", "value"}, 1, 0, 0, false},
      {"prefix:ArgumentMismatch", "", {}, 1, 0, 0, false},
      {"prefix:metadata", "", {"inspect", "--text", "value"}, 3, 0, 0, false},
      {"final:ExtrasError", "inspect", {"--text", "value"}, 3, 0, 0, true},
      {"prefix:ArgumentMismatch", "", {"inspect", "--text", "v"}, 5, 0, 0, false},
      {"accepted", "--bogus", {"inspect", "--text", "v"}, 3, 0, 3, true},
      {"accepted", "--help", {"inspect", "--text", "v"}, 3, 0, 3, true},
      {"prefix:empty-tail", "--help", {}, 3, 0, 0, false},
      {"final:ExtrasError", "p", {"--bogus", "inspect", "--text", "v"}, 3, 0, 3, true},
      {"prefix:RequiredError", "", {"--bogus", "--stdlib-profile", "p", "inspect", "--text", "v"}, 1, 0, 0, false},
      {"final:ExtrasError", "p", {"absent", "--text", "v"}, 3, 0, 0, true},
      {"accepted", "p", {"inspect", "--text", "--stdlib-profile"}, 3, 0, 3, true},
      {"final:ExtrasError", "p", {"inspect", "--stdlib-profile", "q", "--text", "v"}, 3, 0, 3, true},
      {"accepted", "p", {"inspect", "--text", "--bogus"}, 3, 0, 3, true},
      {"accepted", "p", {"inspect", "--text", "--flag"}, 3, 0, 3, true},
      {"prefix:empty-tail", "p", {}, 3, 0, 0, false},
      {"final:metadata", "p", {"--", "inspect", "--text", "v"}, 3, 0, 0, true},
      {"final:metadata", "p", {"inspect", "--text", "v", "--"}, 3, 0, 3, true},
      {"accepted", "--", {"inspect", "--text", "v"}, 3, 0, 3, true},
      {"selected-help", "p", {}, 4, 0, 0, false},
      {"selected-help", "p", {}, 4, 0, 0, false},
      {"prefix:help-tail", "p", {"inspect", "--text", "v"}, 4, 0, 0, false},
      {"final:RequiredError", "p", {"inspect", "--help"}, 3, 0, 1, true},
      {"prefix:ArgumentMismatch", "p", {}, 5, 0, 0, false},
      {"probe:RequiredError", "p", {"history"}, 3, 1, 0, false},
      {"probe:ArgumentMismatch", "p", {"history", "--ledger", "a", "--ledger", "b"}, 3, 5, 0, false},
      {"probe:ExtrasError", "p", {"history", "--ledger", "a", "--bogus", "b"}, 3, 3, 0, false},
      {"probe:metadata", "p", {"history", "--ledger", "a", "--"}, 3, 3, 0, false},
      {"syntax", "", {}, 0, 0, 0, false},
      {"syntax", "", {}, 0, 0, 0, false},
      {"prefix:metadata", "p", {"inspect", "--text", "v"}, 3, 0, 0, false},
      {"accepted", "p", {"init", "--ledger", "a", "--workspace-id", "w"}, 3, 5, 5, true},
      {"accepted", "p", {"export", "--ledger", "a", "--revision", "1", "--target", "out"}, 3, 7, 7, true},
      {"accepted",
       "p",
       {"load", "--ledger", "a", "--workspace", "w", "--registry", "r", "--expect-revision", "none", "--request-id", "k",
        "--actor", "a"},
       3,
       13,
       13,
       true},
      {"accepted",
       "p",
       {"render", "--workspace", "w", "--registry", "r", "--root", "s=src", "--job", "j", "--role", "r"},
       3,
       11,
       11,
       true},
      {"probe:RequiredError", "p", {"init", "--ledger", "a"}, 3, 3, 0, false},
      {"probe:ValidationError", "p", {"export", "--ledger", "a", "--revision", "bad", "--target", "out"}, 3, 7, 0, false},
      {"probe:RequiredError", "p", {"load", "--ledger", "a"}, 3, 3, 0, false},
      {"probe:RequiredError", "p", {"render", "--workspace", "w"}, 3, 3, 0, false},
      {"prefix:help-tail", "p", {"--"}, 4, 0, 0, false},
      {"accepted", "p", {"inspect", "--text", ""}, 3, 0, 3, true},
      {"probe:ExtrasError", "p", {"history", "--ledger", "a", "inspect", "--text", "v"}, 3, 3, 0, false},
  };
  REQUIRE(examples.size() == expected.size());
  std::size_t index = 0;
  for (const auto& example : examples) {
    const auto  result   = envelope_spike_parse(example.args);
    const auto& observed = expected[index++];
    std::cout << "X01 " << std::quoted(example.id) << " argv=";
    for (const auto& arg : example.args)
      std::cout << std::quoted(arg) << ' ';
    std::cout << "outcome=" << result.outcome << " profile=" << std::quoted(result.profile)
              << " consumed=" << result.prefix_consumed << '/' << result.probe_consumed << '/' << result.final_consumed
              << " final=" << result.final_attempted << " tail=";
    for (const auto& arg : result.tail)
      std::cout << std::quoted(arg) << ' ';
    std::cout << '\n';
    CAPTURE(example.id, result.outcome);
    CHECK(result.outcome == observed.outcome);
    CHECK(result.profile == observed.profile);
    CHECK(result.tail == observed.tail);
    CHECK(result.prefix_consumed == observed.prefix_consumed);
    CHECK(result.probe_consumed == observed.probe_consumed);
    CHECK(result.final_consumed == observed.final_consumed);
    CHECK(result.final_attempted == observed.final_attempted);
  }
}

TEST_CASE("X01 CLI11 opaque positional probe rejects prefix flags before catalog selection", "[bootstrap][cli11][x01]") {
  struct example {
    std::vector<std::string> tail;
    bool                     admitted;
    bool                     shipped;
  };
  const std::vector<example> examples = {
      {{"inspect", "--text", "v", "--flag"}, true, false},
      {{"inspect", "history", "--text", "--stdlib-profile"}, true, false},
      {{"absent", "--unknown", "--", "history"}, true, false},
      {{"--bogus", "inspect", "--text", "v"}, false, false},
      {{"--help"}, false, false},
      {{"--", "inspect", "--text", "v"}, false, false},
      {{"history", "--ledger", "a"}, true, true},
      {{"history"}, false, true},
      {{"history", "--ledger", "a", "--bogus", "b"}, false, true},
      {{"history", "--ledger", "a", "--"}, false, true},
      {{"history", "--ledger", "a", "inspect", "--text", "v"}, false, true},
      {{}, false, false},
  };
  for (const auto& example : examples) {
    envelope_spike_catalog   probe{true, false};
    std::vector<std::string> opaque;
    auto*                    positional = probe.root.add_option("opaque-tail", opaque)->expected(-1);
    probe.root.positionals_at_end();
    auto        arguments = std::vector<std::string>(example.tail.rbegin(), example.tail.rend());
    bool        parsed    = true;
    std::string error;
    try {
      probe.root.parse(arguments);
    } catch (const CLI::ParseError& failure) {
      parsed = false;
      error  = failure.get_name();
    }
    const bool shipped  = !probe.root.get_subcommands().empty();
    const auto consumed = envelope_spike_consumed(probe.root) + positional->count();
    const bool admitted = parsed && consumed == example.tail.size() && probe.root.remaining(true).empty() &&
                          (shipped || (!opaque.empty() && opaque == example.tail));
    std::cout << "X01-PROBE argv=";
    for (const auto& arg : example.tail)
      std::cout << std::quoted(arg) << ' ';
    std::cout << "admitted=" << admitted << " shipped=" << shipped << " consumed=" << consumed << " opaque=";
    for (const auto& arg : opaque)
      std::cout << std::quoted(arg) << ' ';
    std::cout << " error=" << error << '\n';
    CAPTURE(example.tail, error);
    CHECK(admitted == example.admitted);
    CHECK(shipped == example.shipped);
  }
}

TEST_CASE("X01 CLI11 parsed profile scalar supports existing explicit value policy", "[bootstrap][cli11][x01]") {
  for (const std::string value : {"p.json", "./--help", "inspect", "", "--help", "--bogus", "--"}) {
    CLI::App root;
    root.set_help_flag();
    std::string profile;
    auto*       option =
        root.add_option("--stdlib-profile", profile)
            ->required()
            ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
            ->check(CLI::Validator(
                [](std::string& parsed) { return parsed.empty() || parsed.starts_with("--") ? "Expected explicit value" : ""; },
                "VALUE"));
    std::vector<std::string> arguments{value, "--stdlib-profile"};
    const bool               accepted = !value.empty() && !value.starts_with("--");
    CAPTURE(value);
    if (accepted) {
      REQUIRE_NOTHROW(root.parse(arguments));
      CHECK(profile == value);
    } else {
      CHECK_THROWS_AS(root.parse(arguments), CLI::ValidationError);
    }
    CHECK(option->count() == 1);
  }
}

// X07 production envelope tests. Every row runs the real exported invoke in a scratch tree: trusted/ holds the profiles and
// work/ is the invocation cwd. Three trees give a hook-free access oracle: TA is a valid profile, TB a malformed profile
// (accessed exactly when its bytes differ from TA) and TC a valid profile whose module spins at chunk level (registered
// exactly when its bytes differ from TA). Limit: rows that never access the profile are indistinguishable among themselves.
namespace {
namespace x07 {
namespace fs         = std::filesystem;
namespace cmd_detail = planar::cmd::detail;
using planar::cmd::response;
using planar::engine::profile_invocation;

std::string slurp(const fs::path& path) {
  std::ifstream in(path, std::ios::binary);
  return std::string{std::istreambuf_iterator<char>(in), {}};
}
void put(const fs::path& path, std::string_view bytes) {
  fs::create_directories(path.parent_path());
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  out << bytes;
  REQUIRE(out.good());
}
bool contains(std::string_view text, std::string_view needle) {
  return text.find(needle) != std::string_view::npos;
}
std::string squash(std::string_view text) {
  std::string out;
  for (const char c : text)
    if (!std::isspace(static_cast<unsigned char>(c)))
      out += c;
  return out;
}
/// Split a table cell on single spaces; the token `""` is the empty string and an empty cell is an empty argument list.
std::vector<std::string> tok(std::string_view line) {
  std::vector<std::string> out;
  if (line.empty())
    return out;
  std::size_t at = 0;
  while (true) {
    const auto next  = line.find(' ', at);
    const auto piece = line.substr(at, next == std::string_view::npos ? next : next - at);
    out.push_back(piece == "\"\"" ? std::string{} : std::string(piece));
    if (next == std::string_view::npos)
      break;
    at = next + 1;
  }
  return out;
}
std::string inspect_module(std::string_view name) {
  const std::string options = name == "inspect" ? "{name='--text',type='text',required=true,help='text'},"
                                                  "{name='--flag',type='flag',required=false,help='flag'}"
                                                : "";
  return "local D={name='" + std::string(name) + "',family='extension_read_v1',handler_id='custom." + std::string(name) +
         "',options=engine.array{" + options + "},help='" + std::string(name) +
         "',response_kind='extension_v1',capabilities=engine.array{}}\nregistration.add(D,function(inv) return "
         "{ok=true,value={version=1,kind='extension',command=inv.command,data={type='summary',text=D.help}}} end)\n";
}
enum class kind { valid, malformed, spinning };
/// Scratch tree: <root>/trusted/{p.json,q.json,modules/} and <root>/work/ (the invocation cwd).
struct tree {
  fs::path root, trusted, work;
  explicit tree(kind k) {
    char pattern[] = "/tmp/planar-x07-XXXXXX";
    REQUIRE(::mkdtemp(pattern));
    root    = fs::canonical(pattern);
    trusted = root / "trusted";
    work    = root / "work";
    fs::create_directories(trusted / "modules");
    fs::create_directories(work);
    put(trusted / "p.json",
        k == kind::malformed ? "{\"version\":1,\"modules\":" : "{\"version\":1,\"modules\":[\"modules/p.lua\"]}");
    put(trusted / "modules/p.lua", (k == kind::spinning ? "while true do end\n" : "") + inspect_module("inspect"));
    put(trusted / "q.json", "{\"version\":1,\"modules\":[\"modules/q.lua\"]}");
    put(trusted / "modules/q.lua", inspect_module("other"));
  }
  tree(const tree&)            = delete;
  tree& operator=(const tree&) = delete;
  ~tree() {
    std::error_code ignored;
    fs::remove_all(root, ignored);
  }
  std::string profile() const {
    return (trusted / "p.json").string();
  }
};
/// Scoped cwd and workspace-environment guard: restores the cwd and unsets PLANAR_WORKSPACE_DIR on exit.
struct invocation_guard {
  fs::path saved;
  invocation_guard(const fs::path& to, const std::optional<std::string>& workspace_env) : saved(fs::current_path()) {
    fs::current_path(to);
    ::unsetenv("PLANAR_WORKSPACE_DIR");
    if (workspace_env)
      ::setenv("PLANAR_WORKSPACE_DIR", workspace_env->c_str(), 1);
  }
  invocation_guard(const invocation_guard&)            = delete;
  invocation_guard& operator=(const invocation_guard&) = delete;
  ~invocation_guard() {
    std::error_code ignored;
    fs::current_path(saved, ignored);
    ::unsetenv("PLANAR_WORKSPACE_DIR");
  }
};
response run_in(const fs::path& cwd, const std::vector<std::string>& args, const std::optional<std::string>& workspace_env = {}) {
  invocation_guard              guard(cwd, workspace_env);
  std::vector<std::string_view> views(args.begin(), args.end());
  return planar::cmd::invoke(views);
}
response run(const tree& t, const std::vector<std::string>& args, const std::optional<std::string>& workspace_env = {}) {
  return run_in(t.work, args, workspace_env);
}
/// Exit status plus exact stdout and stderr bytes.
struct bytes {
  int         status = -1;
  std::string out, err;
  bool        operator==(const bytes&) const = default;
};
bytes of(const response& r) {
  return {r.status, r.output, r.error};
}
void same(const bytes& got, const bytes& want) {
  CHECK(got.status == want.status);
  CHECK(got.out == want.out);
  CHECK(got.err == want.err);
}
bytes usage_bytes() {
  return of(cmd_detail::usage());
}
bytes native_failure(const planar::engine::diagnostic& d) {
  return of(cmd_detail::failed(cmd_detail::native_error(d)));
}
bytes ledger_failure(planar::engine::ledger::types::format_error_code code) {
  return of(cmd_detail::failed(cmd_detail::ledger_error({code, ""})));
}
/// Hand-built closed error JSON (sorted keys, no details), independent of the product encoder.
std::string error_json(std::string_view code, std::string_view message) {
  return std::format("{{\"error\":{{\"code\":\"{}\",\"field\":\"\",\"message\":\"{}\"}},\"kind\":\"error\",\"version\":1}}\n",
                     code, message);
}
std::string escape_help(std::string_view text) {
  std::string out;
  for (const char c : text) {
    const auto u = static_cast<unsigned char>(c);
    if (c == '\\')
      out += "\\\\";
    else if (c == '\n')
      out += "\\n";
    else if (c == '\t')
      out += "\\t";
    else if (u < 0x20 || u == 0x7f)
      out += std::format("\\u{:04x}", static_cast<unsigned>(u));
    else
      out += c;
  }
  return out;
}
/// Expected selected help computed from the native catalog by an independent test-side formatter (contract X-CLI help).
std::string expected_help(const planar::engine::value_ptr& catalog) {
  struct option {
    std::string name, type, help;
    bool        required;
  };
  struct command {
    std::string         name, help;
    std::vector<option> options;
  };
  std::vector<command> commands;
  for (const auto& item : catalog->fields.at("commands")->items) {
    const auto& descriptor = item->fields.contains("descriptor") ? item->fields.at("descriptor") : item;
    command     c{descriptor->fields.at("name")->string, descriptor->fields.at("help")->string, {}};
    for (const auto& o : descriptor->fields.at("options")->items)
      c.options.push_back({o->fields.at("name")->string, o->fields.at("type")->string, o->fields.at("help")->string,
                           o->fields.at("required")->boolean});
    std::ranges::sort(c.options, {}, &option::name);
    commands.push_back(std::move(c));
  }
  std::ranges::sort(commands, {}, &command::name);
  std::string out = "Usage: planar with-profile --stdlib-profile FILE COMMAND [OPTIONS]\n\nCommands:\n";
  for (const auto& c : commands) {
    out += "  " + c.name + "\t" + escape_help(c.help) + "\n";
    for (const auto& o : c.options)
      out += "    " + o.name + " " + o.type + " " + (o.required ? "required" : "optional") + "\t" + escape_help(o.help) + "\n";
  }
  return out;
}
/// Access oracle: three trees and the exact bytes every classified outcome must produce.
struct oracle {
  tree  a{kind::valid}, b{kind::malformed}, c{kind::spinning};
  bytes bad, spin, usage, not_found, help;
  oracle() {
    auto valid = profile_invocation::open_opaque(a.profile());
    REQUIRE(valid);
    help           = {0, expected_help(valid->catalog()), ""};
    auto malformed = profile_invocation::open_opaque(b.profile());
    REQUIRE_FALSE(malformed);
    bad = native_failure(malformed.error().detail);
    REQUIRE(bad.status == 3);
    auto spinning = profile_invocation::open_opaque(c.profile());
    REQUIRE_FALSE(spinning);
    spin = native_failure(spinning.error().detail);
    REQUIRE(spin.status == 3);
    CHECK(spinning.error().detail.code == "resource_limit");
    usage     = usage_bytes();
    not_found = ledger_failure(planar::engine::ledger::types::format_error_code::not_found);
    REQUIRE_FALSE(bad == spin);
    REQUIRE_FALSE(bad == usage);
    REQUIRE_FALSE(spin == usage);
  }
};
const std::string prefix = "with-profile --stdlib-profile ../trusted/p.json";
/// The oracle must be live: a row that must access the profile differs under TB (E13 gives exit 3 schema, not usage).
void require_live_oracle(oracle& o) {
  const auto args = tok(prefix + " absent --text v");
  const auto a    = of(run(o.a, args));
  const auto b    = of(run(o.b, args));
  CHECK(a == o.usage);
  CHECK(b == o.bad);
  CHECK_FALSE(a == b);
}
enum class cls { syn, pfx, prb, nrm, ksa, fin, hlp, hnd };
struct row {
  std::string id, argv;
  cls         klass;
  std::string tag; ///< Matrix test that owns the row: M1..M7 (M6 is wave B).
};
const std::vector<row>& rows() {
  static const std::vector<row> table = [] {
    const auto& P = prefix;
    return std::vector<row>{
        {"E01", P + " inspect --text value --flag", cls::hnd, "M6"},
        {"E02", P + " history --ledger missing", cls::ksa, "M4"},
        {"E03", "with-profile inspect --text value", cls::pfx, "M1"},
        {"E04", "with-profile --stdlib-profile", cls::pfx, "M1"},
        {"E05", "with-profile --stdlib-profile \"\" inspect --text value", cls::pfx, "M1"},
        {"E06", "with-profile --stdlib-profile inspect --text value", cls::prb, "M2"},
        {"E07", P + " --stdlib-profile ../trusted/q.json inspect --text v", cls::pfx, "M1"},
        {"E08", "with-profile --stdlib-profile --bogus inspect --text v", cls::pfx, "M1"},
        {"E09", "with-profile --stdlib-profile --help inspect --text v", cls::pfx, "M1"},
        {"E10", "with-profile --stdlib-profile --help", cls::pfx, "M1"},
        {"E11", P + " --bogus inspect --text v", cls::prb, "M2"},
        {"E12", "with-profile --bogus --stdlib-profile ../trusted/p.json inspect --text v", cls::pfx, "M1"},
        {"E13", P + " absent --text v", cls::fin, "M3"},
        {"E14", P + " inspect --text --stdlib-profile", cls::fin, "M3"},
        {"E15", P + " inspect --stdlib-profile ../trusted/q.json --text v", cls::fin, "M3"},
        {"E16", P + " inspect --text --bogus", cls::fin, "M3"},
        {"E17", P + " inspect --text --flag", cls::fin, "M3"},
        {"E18", P, cls::pfx, "M1"},
        {"E19", P + " -- inspect --text v", cls::prb, "M2"},
        {"E20", P + " inspect --text v --", cls::fin, "M3"},
        {"E21", "with-profile --stdlib-profile -- inspect --text v", cls::pfx, "M1"},
        {"E22", P + " --help", cls::hlp, "M5"},
        {"E23", "with-profile --help --stdlib-profile ../trusted/p.json", cls::hlp, "M5"},
        {"E24", P + " --help inspect --text v", cls::pfx, "M5"},
        {"E25", P + " inspect --help", cls::fin, "M3"},
        {"E26", P + " --help --help", cls::pfx, "M5"},
        {"E27", P + " history", cls::prb, "M2"},
        {"E28", P + " history --ledger a --ledger b", cls::prb, "M2"},
        {"E29", P + " history --ledger a --bogus b", cls::prb, "M2"},
        {"E30", P + " history --ledger a --", cls::prb, "M2"},
        {"E31", "with-profile --stdlib-profile=../trusted/p.json inspect --text v", cls::syn, "M1"},
        {"E32", P + " inspect --text=v", cls::syn, "M1"},
        {"E33", "-- " + P + " inspect --text v", cls::pfx, "M1"},
        {"E34", P + " init --ledger a --workspace-id w", cls::hnd, "M6"},
        {"E35", P + " export --ledger a --revision 1 --target out", cls::ksa, "M4"},
        {"E36", P + " load --ledger a --workspace w --registry r --expect-revision none --request-id k --actor a", cls::nrm,
         "M4"},
        {"E37", P + " render --workspace w --registry r --root s=src --job j --role r", cls::nrm, "M4"},
        {"E38", P + " init --ledger a", cls::prb, "M2"},
        {"E39", P + " export --ledger a --revision bad --target out", cls::prb, "M2"},
        {"E40", P + " load --ledger a", cls::prb, "M2"},
        {"E41", P + " render --workspace w", cls::prb, "M2"},
        {"E42", P + " --help --", cls::pfx, "M5"},
        {"E43", P + " inspect --text \"\"", cls::fin, "M3"},
        {"E44", P + " history --ledger a inspect --text v", cls::prb, "M2"},
        {"O01", P + " inspect --text v --flag", cls::hnd, "M6"},
        {"O02", P + " inspect history --text --stdlib-profile", cls::fin, "M7"},
        {"O03", P + " absent --unknown -- history", cls::fin, "M7"},
        {"O04", P + " --bogus inspect --text v", cls::prb, "M7"},
        {"O05", P + " --help", cls::hlp, "M7"},
        {"O06", P + " -- inspect --text v", cls::prb, "M7"},
        {"O07", P + " history --ledger a", cls::ksa, "M7"},
        {"O08", P + " history", cls::prb, "M7"},
        {"O09", P + " history --ledger a --bogus b", cls::prb, "M7"},
        {"O10", P + " history --ledger a --", cls::prb, "M7"},
        {"O11", P + " history --ledger a inspect --text v", cls::prb, "M7"},
        {"O12", P, cls::pfx, "M7"},
    };
  }();
  return table;
}
bytes expect_a(const oracle& o, const row& r) {
  if (r.klass == cls::ksa)
    return o.not_found;
  if (r.klass == cls::hlp)
    return o.help;
  return o.usage;
}
/// Run every row owned by the tag against TA, TB and TC and assert exit, exact bytes and the tri-state outcome.
void check_rows(oracle& o, std::string_view tag) {
  std::size_t checked = 0;
  for (const auto& r : rows()) {
    if (r.tag != tag)
      continue;
    ++checked;
    CAPTURE(r.id, r.argv);
    const auto args                = tok(r.argv);
    const auto a                   = of(run(o.a, args));
    const auto b                   = of(run(o.b, args));
    const auto c                   = of(run(o.c, args));
    const auto want                = expect_a(o, r);
    const bool accessed_expected   = r.klass == cls::ksa || r.klass == cls::fin || r.klass == cls::hlp || r.klass == cls::hnd;
    const bool registered_expected = r.klass == cls::fin || r.klass == cls::hlp || r.klass == cls::hnd;
    same(a, want);
    same(b, accessed_expected ? o.bad : want);
    same(c, registered_expected ? o.spin : want);
    CHECK((b == a) == !accessed_expected);
    CHECK((c == a) == !registered_expected);
  }
  CHECK(checked > 0);
}
} // namespace x07
} // namespace

TEST_CASE("X07 production envelope rows refused by syntax and prefix usage never access the profile", "[bootstrap][x07]") {
  x07::oracle o;
  x07::require_live_oracle(o);
  x07::check_rows(o, "M1");
}
TEST_CASE("X07 production envelope rows refused by the probe never access the profile", "[bootstrap][x07]") {
  x07::oracle o;
  x07::require_live_oracle(o);
  x07::check_rows(o, "M2");
}
TEST_CASE("X07 production envelope rows refused after capture and registration are final usage", "[bootstrap][x07]") {
  x07::oracle o;
  x07::check_rows(o, "M3");
  // A different profile q lacks `inspect`: the same final tail is unknown there and refused as usage.
  same(x07::of(x07::run(o.a, x07::tok("with-profile --stdlib-profile ../trusted/q.json inspect --text v"))), o.usage);
  // q registers `other`, which takes no options: its own final parse accepts only the bare name, so an option is usage.
  same(x07::of(x07::run(o.a, x07::tok("with-profile --stdlib-profile ../trusted/q.json other --text v"))), o.usage);
}
TEST_CASE("X07 production normalization and known slot rows refuse at their step", "[bootstrap][x07]") {
  x07::oracle o;
  x07::check_rows(o, "M4");
  // E36 with the workspace environment set to the scratch work directory reaches the early lookup (ERL): the ledger `a`
  // is missing, so the refusal is ledger_not_found and the profile is still not accessed.
  const auto args = x07::tok(x07::rows()[35].argv);
  REQUIRE(x07::rows()[35].id == "E36");
  const auto env = o.a.work.string();
  const auto a   = x07::of(x07::run(o.a, args, env));
  const auto b   = x07::of(x07::run(o.b, args, env));
  const auto c   = x07::of(x07::run(o.c, args, env));
  x07::same(a, o.not_found);
  CHECK(b == a);
  CHECK(c == a);
}
TEST_CASE("X07 production selected help rows capture and register the profile", "[bootstrap][x07]") {
  x07::oracle o;
  x07::check_rows(o, "M5");
  // Both option orders give identical bytes and empty stderr (E22 and E23).
  const auto first  = x07::of(x07::run(o.a, x07::tok(x07::prefix + " --help")));
  const auto second = x07::of(x07::run(o.a, x07::tok("with-profile --help --stdlib-profile ../trusted/p.json")));
  CHECK(first == second);
  CHECK(first.status == 0);
  CHECK(first.err.empty());
  CHECK(first.out == o.help.out);
  CHECK(first.out.starts_with("Usage: planar with-profile --stdlib-profile FILE COMMAND [OPTIONS]\n\nCommands:\n"));
}
TEST_CASE("X07 production opaque probe rows refuse or admit at their step", "[bootstrap][x07]") {
  x07::oracle o;
  x07::require_live_oracle(o);
  x07::check_rows(o, "M7");
}
TEST_CASE("X07 production handler rows succeed", "[bootstrap][x07]") {
  // Wave B rows: the extension handler and format2 init arrive later; the stub handler makes this red until then.
  x07::tree a(x07::kind::valid);
  for (const auto& r : x07::rows()) {
    if (r.tag != "M6")
      continue;
    CAPTURE(r.id);
    const auto got = x07::run(a, x07::tok(r.argv));
    CHECK(got.status == 0);
    CHECK(got.error.empty());
    CHECK_FALSE(got.output.empty());
    if (r.id == "E01" || r.id == "O01")
      CHECK(got.output == "{\"command\":\"inspect\",\"data\":{\"text\":\"inspect\",\"type\":\"summary\"},\"kind\":\"extension\","
                          "\"version\":1}\n");
    if (r.id == "E34") {
      auto made = planar::engine::ledger::open_existing(a.work / "a", planar::engine::ledger::ledger_access::read_only);
      REQUIRE(made);
      CHECK(made->metadata().format == planar::engine::ledger::ledger_format::v2);
      CHECK(x07::fs::exists(a.work / "a"));
      // Response bytes equal the public init response, apart from the random ledger identity.
      auto mask = [](std::string text) {
        const auto at = text.find("\"ledger\":\"");
        if (at != std::string::npos && text.size() >= at + 10 + 32)
          text.replace(at + 10, 32, std::string(32, 'x'));
        return text;
      };
      const auto control = x07::run(a, {"init", "--ledger", "plain", "--workspace-id", "w"});
      CHECK(mask(got.output) == mask(control.output));
    }
  }
}
TEST_CASE("X07 production matrices enumerate every X01 row", "[bootstrap][x07]") {
  std::set<std::string> ids;
  for (const auto& r : x07::rows())
    CHECK(ids.insert(r.id).second);
  CHECK(x07::rows().size() == 56);
  for (int n = 1; n <= 44; ++n)
    CHECK(ids.contains(std::format("E{:02d}", n)));
  for (int n = 1; n <= 12; ++n)
    CHECK(ids.contains(std::format("O{:02d}", n)));
  const std::set<std::string> tags{"M1", "M2", "M3", "M4", "M5", "M6", "M7"};
  for (const auto& r : x07::rows()) {
    CHECK(tags.contains(r.tag));
    CHECK_FALSE(r.argv.empty());
  }
  // Every row is also present in the frozen X01 spike matrix by position count: 44 spike cases plus 12 opaque cases.
  std::size_t e = 0, opaque = 0;
  for (const auto& r : x07::rows())
    (r.id.starts_with('E') ? e : opaque)++;
  CHECK(e == 44);
  CHECK(opaque == 12);
}
TEST_CASE("X07 envelope and bootstrap source has no manual argv scan", "[bootstrap][x07]") {
  const auto dir = std::filesystem::path(__FILE__).parent_path();
  for (const char* name : {"envelope.cpp", "bootstrap.cpp"}) {
    const auto text = x07::squash(x07::slurp(dir / name));
    REQUIRE_FALSE(text.empty());
    INFO(name);
    for (const char* forbidden : {"==\"--stdlib-profile\"", "==\"with-profile\"", "arguments[0]", "arguments.front()",
                                  "arguments.subspan(", "arguments.first(", "arguments.last("})
      CHECK_FALSE(x07::contains(text, forbidden));
  }
}
TEST_CASE("X07 selector with invalid UTF-8 or NUL is prefix usage before profile access", "[bootstrap][x07]") {
  x07::oracle o;
  x07::require_live_oracle(o);
  const std::vector<std::string> selectors{std::string("../trusted/p\xff.json"), std::string("../trusted/p\0.json", 17),
                                           std::string("\xc3\x28"), std::string("../trusted/\xed\xa0\x80.json")};
  for (const auto& selector : selectors) {
    CAPTURE(selector.size());
    const std::vector<std::string> args{"with-profile", "--stdlib-profile", selector, "inspect", "--text", "v"};
    const auto                     a = x07::of(x07::run(o.a, args));
    x07::same(a, o.usage);
    CHECK(x07::of(x07::run(o.b, args)) == a);
    CHECK(x07::of(x07::run(o.c, args)) == a);
  }
  // Control: a valid non-ASCII selector is accepted and runs selected help.
  x07::put(o.a.trusted / "\xc3\xa9.json", x07::slurp(o.a.trusted / "p.json"));
  const auto help = x07::of(x07::run(o.a, {"with-profile", "--stdlib-profile", "../trusted/\xc3\xa9.json", "--help"}));
  x07::same(help, o.help);
}
TEST_CASE("X07 prefix count is three or four and dot slash help is a valid selector", "[bootstrap][x07]") {
  x07::oracle o;
  // Extra prefix tokens refuse (E24 shape) without profile access.
  for (const auto& line : {std::string(x07::prefix) + " --help inspect", std::string(x07::prefix) + " --help --help"}) {
    const auto args = x07::tok(line);
    x07::same(x07::of(x07::run(o.a, args)), o.usage);
    CHECK(x07::of(x07::run(o.b, args)) == o.usage);
    CHECK(x07::of(x07::run(o.c, args)) == o.usage);
  }
  // A selector spelled ./--help is a valid explicit filename, here a copy of p.json with its module beside it.
  x07::put(o.a.work / "--help", x07::slurp(o.a.trusted / "p.json"));
  x07::put(o.a.work / "modules/p.lua", x07::slurp(o.a.trusted / "modules/p.lua"));
  const auto dotted = x07::of(x07::run(o.a, {"with-profile", "--stdlib-profile", "./--help", "--help"}));
  x07::same(dotted, o.help);
  const auto reversed = x07::of(x07::run(o.a, {"with-profile", "--help", "--stdlib-profile", "./--help"}));
  x07::same(reversed, o.help);
}
TEST_CASE("X07 root level stdlib-profile flag is usage", "[bootstrap][x07]") {
  x07::oracle o;
  const auto  args = x07::tok("--stdlib-profile ../trusted/p.json history --ledger L");
  x07::same(x07::of(x07::run(o.a, args)), o.usage);
  CHECK(x07::of(x07::run(o.b, args)) == o.usage);
  CHECK(x07::of(x07::run(o.c, args)) == o.usage);
}
TEST_CASE("X07 root help bytes are unchanged and omit with-profile", "[bootstrap][x07]") {
  const std::array<std::string_view, 1> args   = {"--help"};
  const auto                            result = planar::cmd::invoke(args);
  CHECK(result.status == 0);
  CHECK(result.error.empty());
  CHECK(result.output ==
        "Usage: planar render --workspace FILE --registry FILE --root ID=DIRECTORY --job ID --role ID\n"
        "Optional: --capture-outputs --base-root ID=DIRECTORY --observations FILE --budget-bytes N\n"
        "planar init --ledger FILE --workspace-id ID\n"
        "planar history --ledger FILE [--after-event ID] [--limit N]\n"
        "  or --revision ID | --request-id ID | --at-event ID\n"
        "planar export --ledger PATH --revision ID --target NEW_DIRECTORY\n"
        "planar load --ledger PATH --workspace FILE_OR_NAME --registry FILE --expect-revision ID_OR_NONE --request-id ID "
        "--actor TEXT\n"
        "planar --help\n");
  CHECK_FALSE(x07::contains(result.output, "with-profile"));
}

namespace {
namespace x07 {
using planar::engine::command_options;
fs::path fixture_root() {
  return fs::path(PLANAR_B2_WORKSPACE_FIXTURE).parent_path().parent_path();
}
std::string quota_workspace() {
  return (fixture_root() / "workspace/workspace.lua").string();
}
std::string quota_registry() {
  return (fixture_root() / "registry/quota-tools.json").string();
}
std::vector<std::string> load_args(const std::string& ledger, const std::string& request, const std::string& actor = "a") {
  return {"load",
          "--ledger",
          ledger,
          "--workspace",
          quota_workspace(),
          "--registry",
          quota_registry(),
          "--expect-revision",
          "none",
          "--request-id",
          request,
          "--actor",
          actor};
}
std::vector<std::string> with_profile(std::vector<std::string> tail, std::string selector = "../trusted/p.json") {
  std::vector<std::string> args{"with-profile", "--stdlib-profile", std::move(selector)};
  args.insert(args.end(), tail.begin(), tail.end());
  return args;
}
/// Format1 ledger with one accepted embedded load (revision 1) at work/led.db.
fs::path seed_v1(const tree& t) {
  const auto ledger = t.work / "led.db";
  REQUIRE(planar::engine::ledger::initialize_new(ledger, "quota"));
  const auto loaded = run(t, load_args(ledger.string(), "seed"));
  REQUIRE(loaded.status == 0);
  return ledger;
}
fs::path make_v2(const tree& t, std::string_view name = "led2.db") {
  const auto ledger = t.work / name;
  REQUIRE(planar::engine::ledger::initialize_new_v2(ledger, "quota"));
  return ledger;
}
/// A ledger selection that is a hard link of the profile's only module. A real ledger is binary and the capture refuses it
/// as a module before any alias check, so the alias is built the other way round; the refusal precedes opening the ledger.
fs::path alias_ledger_to_module(const tree& t, std::string_view name = "alias-ledger.db") {
  const auto ledger = t.work / name;
  fs::create_hard_link(t.trusted / "modules/p.lua", ledger);
  return ledger;
}
std::string ledger_identity(const fs::path& ledger) {
  auto file = planar::engine::ledger::open_existing(ledger, planar::engine::ledger::ledger_access::read_only);
  REQUIRE(file);
  return file->metadata().identity;
}
/// Natively accepted profile load of request `key` into a format2 ledger; returns the saved result bytes.
std::string save_profile_load(const tree& t, const fs::path& ledger, std::string_view key, std::string_view actor) {
  using namespace planar::engine;
  auto intent = acceptance::request_intent::create_v2(key, actor, std::nullopt, quota_workspace(), quota_registry(), t.profile());
  REQUIRE(intent);
  auto opened = profile_invocation::open_known_load(*intent, ledger.string());
  REQUIRE(opened);
  auto prepared = std::move(*opened).prepare_load(*intent);
  REQUIRE(prepared);
  ledger::completion_receipt receipt;
  auto                       done = ledger::complete_acceptance(ledger, *intent, *prepared, ledger_identity(ledger), receipt);
  REQUIRE(done);
  REQUIRE_FALSE(receipt.saved_result.empty());
  return receipt.saved_result;
}
std::vector<std::string> listing(const fs::path& root) {
  std::vector<std::string> out;
  if (!fs::exists(root))
    return {"<missing>"};
  for (const auto& entry : fs::recursive_directory_iterator(root))
    out.push_back(fs::relative(entry.path(), root).generic_string() + (entry.is_directory() ? "/" : ""));
  std::ranges::sort(out);
  return out;
}
const bytes unsafe_overlap() {
  return native_failure({"unsafe_path", "External profile overlaps a supplied input"});
}
} // namespace x07
} // namespace

TEST_CASE("X07 profile invocation reads cwd and environment once", "[bootstrap][x07]") {
  using namespace planar::engine;
  int                                                           cwd_calls = 0, env_calls = 0;
  const std::function<std::expected<std::string, diagnostic>()> cwd_reader = [&]() -> std::expected<std::string, diagnostic> {
    ++cwd_calls;
    return std::string("/scratch/x07/work");
  };
  const std::function<std::expected<std::string, diagnostic>()> env_reader = [&]() -> std::expected<std::string, diagnostic> {
    ++env_calls;
    return std::string("/scratch/x07/ws");
  };
  command_options                  options = command_load_options{"led.db", "w", "r.json", "none", "k", "a", std::nullopt};
  planar::cmd::invocation_snapshot snapshot;
  auto selector = planar::cmd::prepare_profile_invocation("../trusted/p.json", options, snapshot, cwd_reader, env_reader);
  REQUIRE(selector);
  CHECK(*selector == "/scratch/x07/trusted/p.json");
  CHECK(cwd_calls == 1);
  CHECK(env_calls == 1);
  CHECK(snapshot.cwd == std::optional<std::string>("/scratch/x07/work"));
  CHECK(snapshot.workspace_directory == std::optional<std::string>("/scratch/x07/ws"));
  const auto& load = std::get<command_load_options>(options);
  CHECK(load.registry == "/scratch/x07/work/r.json");
  CHECK(load.ledger == "/scratch/x07/work/led.db");
  CHECK(load.workspace.starts_with("/scratch/x07/ws"));
  // History needs no environment: the cwd is still read once for the relative ledger and the relative selector.
  cwd_calls = env_calls = 0;
  command_options history =
      command_history_options{"rel.db", std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt};
  planar::cmd::invocation_snapshot history_snapshot;
  REQUIRE(planar::cmd::prepare_profile_invocation("p.json", history, history_snapshot, cwd_reader, env_reader));
  CHECK(cwd_calls == 1);
  CHECK(env_calls == 0);
  CHECK_FALSE(history_snapshot.workspace_directory.has_value());
  CHECK(history_snapshot.cwd == std::optional<std::string>("/scratch/x07/work"));
  // All-absolute inputs read neither the cwd nor the environment.
  cwd_calls = env_calls                     = 0;
  command_options                  absolute = command_export_options{"/abs/l.db", "/abs/out", 1};
  planar::cmd::invocation_snapshot absolute_snapshot;
  auto                             absolute_selector =
      planar::cmd::prepare_profile_invocation("/abs/p.json", absolute, absolute_snapshot, cwd_reader, env_reader);
  REQUIRE(absolute_selector);
  CHECK(*absolute_selector == "/abs/p.json");
  CHECK(cwd_calls == 0);
  CHECK(env_calls == 0);
}
TEST_CASE("X07 profile selector normalizes lexically without existence", "[bootstrap][x07]") {
  using namespace planar::engine;
  // A symlinked cwd separates lexical from filesystem normalization: lexically ../marker.json leaves the link's parent.
  x07::tree t(x07::kind::valid);
  x07::fs::create_directories(t.work / "deep");
  x07::fs::create_directory_symlink(t.work / "deep", t.root / "link");
  const std::string                                             cwd        = (t.root / "link").string();
  const std::function<std::expected<std::string, diagnostic>()> cwd_reader = [&]() -> std::expected<std::string, diagnostic> {
    return cwd;
  };
  const std::function<std::expected<std::string, diagnostic>()> env_reader = []() -> std::expected<std::string, diagnostic> {
    return std::unexpected(diagnostic{"internal", "unused"});
  };
  command_options options = command_history_options{"l.db", std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt};
  planar::cmd::invocation_snapshot snapshot;
  auto selector = planar::cmd::prepare_profile_invocation("../marker.json", options, snapshot, cwd_reader, env_reader);
  REQUIRE(selector);
  CHECK(*selector == (t.root / "marker.json").string());
  // Absent intermediate directories are accepted: ../trusted/sub/../p.json is E01-like and reaches selected help.
  x07::oracle o;
  const auto  help = x07::of(x07::run(o.a, {"with-profile", "--stdlib-profile", "../trusted/sub/../p.json", "--help"}));
  x07::same(help, o.help);
  // A nonexistent selector fails at capture (a native capture refusal), never as normalization or usage.
  const auto absent = o.a.trusted / "absent.json";
  auto       native = profile_invocation::open_opaque(absent.string());
  REQUIRE_FALSE(native);
  const auto got = x07::of(x07::run(o.a, {"with-profile", "--stdlib-profile", "../trusted/absent.json", "--help"}));
  x07::same(got, x07::native_failure(native.error().detail));
  CHECK_FALSE(got == o.usage);
}
TEST_CASE("X07 profile selector honours the path ceiling and the request ceiling wins", "[bootstrap][x07]") {
  using namespace planar::engine;
  constexpr std::size_t                                         ceiling    = 64 * 1024 * 1024;
  const std::function<std::expected<std::string, diagnostic>()> cwd_reader = []() -> std::expected<std::string, diagnostic> {
    return std::string("/scratch/x07/work");
  };
  const std::function<std::expected<std::string, diagnostic>()> env_reader = []() -> std::expected<std::string, diagnostic> {
    return std::string("/scratch/x07/ws");
  };
  {
    command_options                  options = command_init_options{"/abs/l.db", "w"};
    planar::cmd::invocation_snapshot snapshot;
    const std::string                over = "/" + std::string(ceiling, 'a');
    auto refused = planar::cmd::prepare_profile_invocation(over, options, snapshot, cwd_reader, env_reader);
    REQUIRE_FALSE(refused);
    CHECK(refused.error().code == "resource_limit");
    CHECK(refused.error().message == "Explicit input path limit exceeded");
    const std::string exact    = "/" + std::string(ceiling - 1, 'a');
    auto              accepted = planar::cmd::prepare_profile_invocation(exact, options, snapshot, cwd_reader, env_reader);
    REQUIRE(accepted);
    CHECK(accepted->size() == ceiling);
  }
  // Through the production CLI: a load whose selector, workspace and registry each pass the 64 MiB path ceiling but together
  // exceed the 64 MiB request ceiling is the request refusal G17, before the ledger or the profile is touched.
  x07::oracle                    o;
  const std::string              selector  = "/" + std::string(60 * 1024 * 1024, 's');
  const std::string              workspace = "/" + std::string(8 * 1024 * 1024, 'w') + "/workspace.lua";
  const std::vector<std::string> args{"with-profile",
                                      "--stdlib-profile",
                                      selector,
                                      "load",
                                      "--ledger",
                                      "a",
                                      "--workspace",
                                      workspace,
                                      "--registry",
                                      x07::quota_registry(),
                                      "--expect-revision",
                                      "none",
                                      "--request-id",
                                      "k",
                                      "--actor",
                                      "a"};
  const auto                     got = x07::of(x07::run(o.a, args));
  x07::same(got, x07::native_failure({"resource_limit", "Load request exceeds native resource limits"}));
}
TEST_CASE("X07 profile load replays a saved request before touching the profile", "[bootstrap][x07]") {
  x07::tree  t(x07::kind::valid);
  const auto ledger = x07::make_v2(t);
  const auto saved  = x07::save_profile_load(t, ledger, "k", "a");
  // The profile is now unreadable: an early equal replay must not touch it.
  x07::put(t.trusted / "p.json", "{\"version\":1,\"modules\":");
  const auto got = x07::run(t, x07::with_profile(x07::load_args(ledger.string(), "k")));
  CHECK(got.status == 0);
  CHECK(got.output == saved + "\n");
  CHECK(got.error.empty());
}
TEST_CASE("X07 profile load with a different body conflicts before profile access", "[bootstrap][x07]") {
  x07::tree  t(x07::kind::valid);
  const auto ledger = x07::make_v2(t);
  x07::save_profile_load(t, ledger, "k", "a");
  x07::put(t.trusted / "p.json", "{\"version\":1,\"modules\":");
  const auto conflict = x07::ledger_failure(planar::engine::ledger::types::format_error_code::request_conflict);
  REQUIRE(conflict.status == 3);
  // Same ID with a different actor is a different body; the early lookup refuses it without reading the profile.
  x07::same(x07::of(x07::run(t, x07::with_profile(x07::load_args(ledger.string(), "k", "b")))), conflict);
  // A different profile selector spelling is also a different body.
  x07::put(t.trusted / "q2.json", "{\"version\":1,\"modules\":[]}");
  x07::same(x07::of(x07::run(t, x07::with_profile(x07::load_args(ledger.string(), "k"), "../trusted/q2.json"))), conflict);
}
TEST_CASE("X07 profile load on a format1 ledger refuses before profile access", "[bootstrap][x07]") {
  x07::tree  t(x07::kind::malformed);
  const auto ledger = t.work / "v1.db";
  REQUIRE(planar::engine::ledger::initialize_new(ledger, "quota"));
  // The saved body is the embedded v1 request: the same ID under with-profile is a v2 body and conflicts.
  REQUIRE(x07::run(t, x07::load_args(ledger.string(), "k")).status == 0);
  const auto conflict = x07::ledger_failure(planar::engine::ledger::types::format_error_code::request_conflict);
  x07::same(x07::of(x07::run(t, x07::with_profile(x07::load_args(ledger.string(), "k")))), conflict);
  // An unknown ID on a format1 ledger cannot be a profile load: unsupported_ledger_version, still before the profile.
  const auto unsupported = x07::ledger_failure(planar::engine::ledger::types::format_error_code::unsupported_version);
  REQUIRE(unsupported.status == 3);
  x07::same(x07::of(x07::run(t, x07::with_profile(x07::load_args(ledger.string(), "fresh")))), unsupported);
}
TEST_CASE("X07 early lookup scope closes before profile capture", "[bootstrap][x07]") {
  // Weak structural scan (limit: no clean observer of the released read transaction). The scan becomes active once a closed
  // factory call site exists in the command sources: the load intent overload must come after the scope that holds
  // read_request has closed.
  const auto dir = std::filesystem::path(__FILE__).parent_path();
  for (const char* name : {"invocation.cpp", "bootstrap.cpp"}) {
    const auto text = x07::slurp(dir / name);
    const auto call = text.find("open_with_profile(");
    const auto read = text.find("read_request(");
    if (call == std::string::npos || read == std::string::npos)
      continue;
    // Depth just before read_request, then the first later position where the depth falls below it.
    int depth = 0;
    for (std::size_t i = 0; i < read; ++i)
      depth += text[i] == '{' ? 1 : text[i] == '}' ? -1 : 0;
    std::size_t close = read;
    for (int d = depth; close < text.size(); ++close) {
      d += text[close] == '{' ? 1 : text[close] == '}' ? -1 : 0;
      if (d < depth)
        break;
    }
    INFO(name);
    CHECK(close < call);
  }
}

namespace {
namespace x07 {
/// Known-slot overlap row: A and C (registration would spin) both give unsafe_path, B gives the capture error.
void check_known_slot_row(oracle& o, const std::string& id, const std::function<std::vector<std::string>(const tree&)>& build,
                          const std::function<void(const tree&)>& prepare = {}) {
  CAPTURE(id);
  for (auto* t : {&o.a, &o.b, &o.c}) {
    if (prepare)
      prepare(*t);
  }
  same(of(run(o.a, build(o.a))), unsafe_overlap());
  same(of(run(o.c, build(o.c))), unsafe_overlap());
  same(of(run(o.b, build(o.b))), o.bad);
}
std::vector<std::string> render_args(const tree& t, std::string workspace, std::string source,
                                     std::vector<std::string> extra = {}) {
  std::vector<std::string> args{"render",
                                "--workspace",
                                std::move(workspace),
                                "--registry",
                                quota_registry(),
                                "--root",
                                "s=" + std::move(source),
                                "--job",
                                "combined",
                                "--role",
                                "builder"};
  args.insert(args.end(), extra.begin(), extra.end());
  return with_profile(std::move(args), "../trusted/p.json");
  (void)t;
}
} // namespace x07
} // namespace

TEST_CASE("X07 render known slots refuse overlap before registration", "[bootstrap][x07]") {
  x07::oracle o;
  for (auto* t : {&o.a, &o.b, &o.c}) {
    x07::fs::create_directories(t->work / "src");
    x07::put(t->trusted / "workspace.lua", "-- placeholder");
    x07::put(t->trusted / "obs.json", "{}");
    x07::put(t->root / "ws.lua", "-- placeholder");
    x07::fs::create_hard_link(t->trusted / "modules/p.lua", t->work / "alias-obs.json");
    x07::fs::create_hard_link(t->trusted / "p.json", t->work / "alias-reg.json");
    x07::fs::create_hard_link(t->trusted / "p.json", t->work / "alias-ws.lua");
  }
  using x07::render_args;
  const auto work_src = [](const x07::tree& t) { return (t.work / "src").string(); };
  x07::check_known_slot_row(o, "source root equals trusted root",
                            [&](const x07::tree& t) { return render_args(t, x07::quota_workspace(), t.trusted.string()); });
  x07::check_known_slot_row(o, "source root contains trusted root",
                            [&](const x07::tree& t) { return render_args(t, x07::quota_workspace(), t.root.string()); });
  x07::check_known_slot_row(o, "base root equals trusted root", [&](const x07::tree& t) {
    return render_args(t, x07::quota_workspace(), work_src(t), {"--capture-outputs", "--base-root", "s=" + t.trusted.string()});
  });
  x07::check_known_slot_row(o, "declaration root inside trusted root", [&](const x07::tree& t) {
    return render_args(t, (t.trusted / "workspace.lua").string(), work_src(t));
  });
  x07::check_known_slot_row(o, "declaration root contains trusted root",
                            [&](const x07::tree& t) { return render_args(t, (t.root / "ws.lua").string(), work_src(t)); });
  x07::check_known_slot_row(o, "observations file inside trusted root", [&](const x07::tree& t) {
    return render_args(t, x07::quota_workspace(), work_src(t), {"--observations", (t.trusted / "obs.json").string()});
  });
  x07::check_known_slot_row(o, "observations file aliases a trusted file", [&](const x07::tree& t) {
    return render_args(t, x07::quota_workspace(), work_src(t), {"--observations", (t.work / "alias-obs.json").string()});
  });
  x07::check_known_slot_row(o, "registry aliases a trusted file", [&](const x07::tree& t) {
    auto args = render_args(t, x07::quota_workspace(), work_src(t));
    for (auto& a : args)
      if (a == x07::quota_registry())
        a = (t.work / "alias-reg.json").string();
    return args;
  });
  x07::check_known_slot_row(o, "selected declaration is a hard link of a trusted file",
                            [&](const x07::tree& t) { return render_args(t, (t.work / "alias-ws.lua").string(), work_src(t)); });
}
TEST_CASE("X07 init known slot is alias only and runs before registration", "[bootstrap][x07]") {
  x07::oracle o;
  // Q-INIT A: a new target inside the trusted root is not refused by containment, so admission passes and registration runs.
  const auto inside = [](const x07::tree& t) {
    return x07::with_profile({"init", "--ledger", (t.trusted / "new.db").string(), "--workspace-id", "w"});
  };
  x07::same(x07::of(x07::run(o.c, inside(o.c))), o.spin);
  x07::same(x07::of(x07::run(o.b, inside(o.b))), o.bad);
  CHECK_FALSE(x07::fs::exists(o.c.trusted / "new.db"));
  CHECK_FALSE(x07::fs::exists(o.b.trusted / "new.db"));
  // An existing target that is a hard link of a trusted file is the alias refusal, before registration.
  for (auto* t : {&o.a, &o.b, &o.c})
    x07::fs::create_hard_link(t->trusted / "modules/p.lua", t->work / "alias.db");
  const auto alias = [](const x07::tree& t) {
    return x07::with_profile({"init", "--ledger", (t.work / "alias.db").string(), "--workspace-id", "w"});
  };
  x07::same(x07::of(x07::run(o.a, alias(o.a))), x07::unsafe_overlap());
  x07::same(x07::of(x07::run(o.c, alias(o.c))), x07::unsafe_overlap());
  x07::same(x07::of(x07::run(o.b, alias(o.b))), o.bad);
}
TEST_CASE("X07 history known slot refuses a ledger alias before registration", "[bootstrap][x07]") {
  x07::tree t(x07::kind::valid);
  x07::seed_v1(t);
  const auto ledger = x07::alias_ledger_to_module(t);
  const auto got    = x07::of(x07::run(t, x07::with_profile({"history", "--ledger", ledger.string()})));
  x07::same(got, x07::unsafe_overlap());
  // The same ledger used with a plain profile does not alias anything and is not refused as unsafe.
  x07::oracle o;
  const auto  plain = x07::seed_v1(o.a);
  CHECK_FALSE(x07::of(x07::run(o.a, x07::with_profile({"history", "--ledger", plain.string()}))) == x07::unsafe_overlap());
}
TEST_CASE("X07 export known slots refuse before registration", "[bootstrap][x07]") {
  {
    x07::tree t(x07::kind::valid);
    x07::seed_v1(t);
    const auto ledger = x07::alias_ledger_to_module(t);
    x07::same(
        x07::of(x07::run(t, x07::with_profile({"export", "--ledger", ledger.string(), "--revision", "1", "--target", "out"}))),
        x07::unsafe_overlap());
    CHECK_FALSE(x07::fs::exists(t.work / "out"));
  }
  x07::oracle o;
  for (auto* t : {&o.a, &o.b, &o.c})
    x07::seed_v1(*t);
  const auto before = x07::listing(o.c.trusted);
  const auto target = [](const x07::tree& t) {
    return x07::with_profile(
        {"export", "--ledger", (t.work / "led.db").string(), "--revision", "1", "--target", (t.trusted / "exported").string()});
  };
  x07::same(x07::of(x07::run(o.a, target(o.a))), x07::unsafe_overlap());
  x07::same(x07::of(x07::run(o.c, target(o.c))), x07::unsafe_overlap());
  x07::same(x07::of(x07::run(o.b, target(o.b))), o.bad);
  CHECK(x07::listing(o.c.trusted) == before);
}
TEST_CASE("X07 load known slots refuse before registration", "[bootstrap][x07]") {
  x07::oracle o;
  for (auto* t : {&o.a, &o.b, &o.c}) {
    x07::make_v2(*t);
    x07::put(t->trusted / "workspace.lua", "-- placeholder");
    x07::put(t->root / "ws.lua", "-- placeholder");
    x07::fs::create_hard_link(t->trusted / "p.json", t->work / "alias-ws.lua");
    x07::fs::create_hard_link(t->trusted / "modules/p.lua", t->work / "alias-reg.json");
  }
  const auto args = [](const x07::tree& t, const std::string& workspace, const std::string& registry, std::string key) {
    return x07::with_profile({"load", "--ledger", (t.work / "led2.db").string(), "--workspace", workspace, "--registry", registry,
                              "--expect-revision", "none", "--request-id", std::move(key), "--actor", "a"});
  };
  x07::check_known_slot_row(o, "workspace root inside trusted root", [&](const x07::tree& t) {
    return args(t, (t.trusted / "workspace.lua").string(), x07::quota_registry(), "k1");
  });
  x07::check_known_slot_row(o, "workspace root contains trusted root", [&](const x07::tree& t) {
    return args(t, (t.root / "ws.lua").string(), x07::quota_registry(), "k2");
  });
  x07::check_known_slot_row(o, "declaration aliases a trusted file", [&](const x07::tree& t) {
    return args(t, (t.work / "alias-ws.lua").string(), x07::quota_registry(), "k3");
  });
  x07::check_known_slot_row(o, "registry aliases a trusted file", [&](const x07::tree& t) {
    return args(t, x07::quota_workspace(), (t.work / "alias-reg.json").string(), "k4");
  });
  // Ledger alias: the ledger selection is also the only module; registration would fail differently from unsafe_path.
  x07::tree linked(x07::kind::valid);
  x07::make_v2(linked);
  const auto ledger = x07::alias_ledger_to_module(linked);
  // The load handler reads the ledger before any profile access, so a module alias is not a ledger and is refused natively;
  // a real ledger is binary and the capture refuses it as a module, so the alias check itself is unreachable for load.
  const auto aliased =
      x07::with_profile({"load", "--ledger", ledger.string(), "--workspace", x07::quota_workspace(), "--registry",
                         x07::quota_registry(), "--expect-revision", "none", "--request-id", "k5", "--actor", "a"});
  x07::same(x07::of(x07::run(linked, aliased)),
            x07::ledger_failure(planar::engine::ledger::types::format_error_code::foreign_ledger));
}
TEST_CASE("X07 shipped tail under a profile keeps probe option state and scalar command words stay values", "[bootstrap][x07]") {
  x07::oracle o;
  // A token spelling another command in a scalar position is a value: the ledger named after a command is simply missing.
  for (const auto* word : {"export", "history", "init", "load", "render", "inspect"}) {
    CAPTURE(word);
    x07::same(x07::of(x07::run(o.a, x07::with_profile({"history", "--ledger", word}))), o.not_found);
  }
  x07::same(x07::of(x07::run(o.a, x07::with_profile({"export", "--ledger", "history", "--revision", "1", "--target", "load"}))),
            o.not_found);
  // The accepted probe state is the final state: a leading option value cannot be re-read as a verb or selector.
  x07::same(x07::of(x07::run(o.a, x07::with_profile({"history", "--ledger", "--stdlib-profile"}))), o.usage);
}

namespace {
namespace x07 {
struct closed_row {
  const char* id;
  int         status;
  const char* code;
  const char* message;
  const char* limit;
  std::size_t maximum;
};
/// Registration rows of the closed table (addendum 2.5, G23 to G30 and G32 to G34) with their exit statuses.
const std::vector<closed_row>& registration_rows() {
  static const std::vector<closed_row> table{
      {"G23", 3, "schema", "Invalid external profile structure", "", 0},
      {"G24", 3, "unsupported_version", "Unsupported external profile version", "", 0},
      {"G25", 3, "path", "Invalid external module path", "", 0},
      {"G26", 3, "unsafe_path", "External profile overlaps a supplied input", "", 0},
      {"G27", 3, "reference", "External registration identity conflict", "", 0},
      {"G28", 3, "capture_changed", "Captured external profile changed", "", 0},
      {"G29", 3, "resource_limit", "External profile resource limit exceeded", "", 0},
      {"G30", 4, "internal", "Invalid external profile protocol", "", 0},
      {"G32", 3, "resource_limit", "Lua stack exceeded", "stack", 2048},
      {"G33", 3, "resource_limit", "Lua instruction limit exceeded", "instructions", 20000000},
      {"G34", 3, "resource_limit", "Lua allocation exceeded", "lua_bytes", 33554432},
  };
  return table;
}
std::string closed_json(const closed_row& r) {
  if (std::string_view(r.limit).empty())
    return error_json(r.code, r.message);
  return std::format(
      "{{\"error\":{{\"code\":\"{}\",\"details\":{{\"limit\":\"{}\",\"maximum\":{}}},\"field\":\"\",\"message\":\"{}\"}},"
      "\"kind\":\"error\",\"version\":1}}\n",
      r.code, r.limit, r.maximum, r.message);
}
planar::engine::diagnostic as_diagnostic(const closed_row& r) {
  return {r.code, r.message, r.limit, r.maximum};
}
const std::vector<std::string_view>& closed_messages() {
  static const std::vector<std::string_view> table{
      "Expected command options or --help; see --help",
      "Invalid explicit input path",
      "Invalid explicit input cwd",
      "Explicit input path limit exceeded",
      "Cannot resolve explicit input path",
      "Cannot obtain invocation cwd",
      "Explicit input file is a symbolic link",
      "Explicit input must be a regular file",
      "Explicit root must be a directory",
      "Too many symbolic links in explicit root",
      "Explicit root is a symbolic link target",
      "Cannot resolve explicit root directory",
      "Load exceeds native resource limits",
      "Invalid native load preparation",
      "Load request exceeds native resource limits",
      "Invalid load arguments",
      "Missing native load capture",
      "Missing native load intent",
      "Invalid saved load response",
      "Missing load completion",
      "Invalid external profile structure",
      "Unsupported external profile version",
      "Invalid external module path",
      "External profile overlaps a supplied input",
      "External registration identity conflict",
      "Captured external profile changed",
      "External profile resource limit exceeded",
      "Invalid external profile protocol",
      "Invalid embedded command protocol",
      "Lua stack exceeded",
      "Lua instruction limit exceeded",
      "Lua allocation exceeded",
      "Invalid ledger arguments",
      "Ledger path requires existing real parent directories and a valid target",
      "Ledger target already exists",
      "Ledger was not found",
      "File is not a Planar ledger",
      "Ledger version or state is unsupported",
      "Ledger integrity verification failed",
      "Ledger is busy",
      "Ledger storage is full",
      "Ledger operation failed",
      "Ledger may be published; inspect the explicit target before retrying",
      "Explicit ledger recovery is required",
      "Ledger record identity collision",
      "Ledger request conflicts with its saved intent",
      "Ledger revision does not match the expected revision",
      "Captured declaration inputs changed",
      "Recorded history view is unavailable",
      "Recorded revision was not found",
      "Invalid export arguments",
      "Export path requires existing real parent directories and a valid target",
      "Export target already exists",
      "Export staging cleanup is incomplete; inspect the sibling location in the explicit target parent before taking action",
      "Export operation failed",
      "Invalid export cleanup location",
      "Export may be complete; inspect the explicit target before retrying",
      "Load may be complete; retry the same request ID or inspect request history",
      "Cannot write stdout",
      "JSON codec refused input",
      // Present at the starting revision outside the G table and unchanged by X07.
      "Invalid embedded command catalog",
      "Root mapping must match workspace source ID",
  };
  return table;
}
const std::set<std::string>& closed_codes() {
  static const std::set<std::string> codes{"usage",
                                           "path_denied",
                                           "resource_limit",
                                           "io",
                                           "schema",
                                           "unsupported_version",
                                           "path",
                                           "unsafe_path",
                                           "reference",
                                           "capture_changed",
                                           "internal",
                                           "ledger_not_found",
                                           "foreign_ledger",
                                           "unsupported_ledger_version",
                                           "ledger_corrupt",
                                           "ledger_busy",
                                           "storage_full",
                                           "ledger_exists",
                                           "hash_collision",
                                           "request_conflict",
                                           "revision_conflict",
                                           "revision_not_found",
                                           "request_not_found",
                                           "event_not_found",
                                           "unsupported_interpretation",
                                           "target_exists",
                                           "export_incomplete"};
  return codes;
}
/// Every non-test source under src/cmd/planar, recursively, as (relative path, text).
std::vector<std::pair<std::string, std::string>> command_sources() {
  const auto                                       dir = fs::path(__FILE__).parent_path();
  std::vector<std::pair<std::string, std::string>> out;
  for (const auto& entry : fs::recursive_directory_iterator(dir)) {
    const auto name = entry.path().filename().string();
    if (!entry.is_regular_file() || (entry.path().extension() != ".cpp" && entry.path().extension() != ".cppm") ||
        name.ends_with(".t.cpp"))
      continue;
    out.emplace_back(fs::relative(entry.path(), dir).generic_string(), slurp(entry.path()));
  }
  std::ranges::sort(out);
  return out;
}
std::size_t count_of(std::string_view text, std::string_view needle) {
  std::size_t n = 0;
  for (auto at = text.find(needle); at != std::string_view::npos; at = text.find(needle, at + needle.size()))
    ++n;
  return n;
}
} // namespace x07
} // namespace

TEST_CASE("X07 adapter maps every registration refusal to its closed code message and exit", "[bootstrap][x07]") {
  CHECK(x07::registration_rows().size() == 11);
  for (const auto& row : x07::registration_rows()) {
    CAPTURE(row.id);
    const auto got = planar::cmd::detail::registration_failed(x07::as_diagnostic(row));
    CHECK(got.status == row.status);
    CHECK(got.output.empty());
    CHECK(got.error == x07::closed_json(row));
  }
}
TEST_CASE("X07 adapter never prints fields host paths or tracebacks", "[bootstrap][x07]") {
  x07::tree t(x07::kind::valid);
  // A module path escaping the trusted root is a registration refusal whose logical field and scratch path stay private.
  x07::put(t.trusted / "p.json", "{\"version\":1,\"modules\":[\"../escape.lua\"]}");
  auto native = planar::engine::profile_invocation::open_opaque(t.profile());
  REQUIRE_FALSE(native);
  REQUIRE_FALSE(native.error().field.empty());
  const auto got = x07::run(t, x07::with_profile({"--help"}));
  x07::same(x07::of(got), x07::native_failure(native.error().detail));
  CHECK_FALSE(x07::contains(got.error, native.error().field));
  CHECK_FALSE(x07::contains(got.error, t.root.string()));
  CHECK_FALSE(x07::contains(got.error, "modules["));
  CHECK_FALSE(x07::contains(got.error, "traceback"));
  CHECK_FALSE(x07::contains(got.error, ".lua"));
  // A spinning module exposes no Lua text or chunk names either.
  x07::tree  spin(x07::kind::spinning);
  const auto spun = x07::run(spin, x07::with_profile({"--help"}));
  CHECK(spun.status == 3);
  CHECK_FALSE(x07::contains(spun.error, "p.lua"));
  CHECK_FALSE(x07::contains(spun.error, spin.root.string()));
  CHECK_FALSE(x07::contains(spun.error, "while true"));
}
TEST_CASE("X07 extension preparation usage prints the public usage bytes", "[bootstrap][x07]") {
  const auto usage = x07::usage_bytes();
  const auto got   = x07::of(planar::cmd::detail::extension_usage_failed());
  x07::same(got, usage);
  CHECK(got.status == 2);
  CHECK_FALSE(x07::contains(got.err, "Invalid extension invocation"));
  const auto protocol = x07::of(planar::cmd::detail::extension_protocol_failed());
  CHECK(protocol.status == 4);
  CHECK(protocol.out.empty());
  CHECK(protocol.err == x07::error_json("internal", "Invalid external profile protocol"));
  for (const auto& row : x07::registration_rows()) {
    if (std::string_view(row.code) != "resource_limit")
      continue;
    CAPTURE(row.id);
    const auto resource = planar::cmd::detail::extension_resource_failed(x07::as_diagnostic(row));
    CHECK(resource.status == 3);
    CHECK(resource.output.empty());
    CHECK(resource.error == x07::closed_json(row));
  }
}
TEST_CASE("X07 profile preparation semantic error exits by code", "[bootstrap][x07]") {
  using planar::cmd::detail::semantic_failed;
  using planar::cmd::detail::text;
  struct mapping {
    const char* code;
    int         status;
  };
  const std::array table{mapping{"usage", 2},
                         mapping{"io", 4},
                         mapping{"internal", 4},
                         mapping{"ledger_corrupt", 4},
                         mapping{"ledger_busy", 4},
                         mapping{"storage_full", 4},
                         mapping{"schema", 3},
                         mapping{"reference", 3},
                         mapping{"custom_shape_refusal", 3},
                         mapping{"resource_limit", 3},
                         mapping{"ledger_not_found", 3}};
  for (const auto& entry : table) {
    CAPTURE(entry.code);
    const std::string message = std::string("Inner message for ") + entry.code;
    auto       inner = planar::cmd::detail::object({{"code", text(entry.code)}, {"message", text(message)}, {"field", text("")}});
    const auto got   = semantic_failed(inner);
    CHECK(got.status == entry.status);
    CHECK(got.output.empty());
    CHECK(got.error == x07::error_json(entry.code, message));
  }
}
TEST_CASE("X07 adapter redacts malformed trusted payloads as internal protocol", "[bootstrap][x07]") {
  using planar::cmd::detail::object;
  using planar::cmd::detail::text;
  const std::string                            protocol = x07::error_json("internal", "Invalid external profile protocol");
  const std::vector<planar::engine::value_ptr> malformed{
      nullptr,
      text("not a record"),
      object({}),
      object({{"code", text("schema")}}),
      object({{"message", text("Only a message")}}),
      object({{"code", planar::cmd::detail::integer(3)}, {"message", text("m")}, {"field", text("")}}),
      object({{"code", text("schema")}, {"message", planar::cmd::detail::integer(3)}, {"field", text("")}}),
      object({{"code", text("")}, {"message", text("empty code")}, {"field", text("")}}),
  };
  std::size_t index = 0;
  for (const auto& payload : malformed) {
    CAPTURE(index++);
    const auto got = planar::cmd::detail::semantic_failed(payload);
    CHECK(got.status == 4);
    CHECK(got.output.empty());
    CHECK(got.error == protocol);
  }
  // An unknown code, or a known code with a message outside the closed table, is redacted instead of passed through.
  for (const auto& d :
       {planar::engine::diagnostic{"weird_code", "Weird message"},
        planar::engine::diagnostic{"schema", "A message outside the closed table"}, planar::engine::diagnostic{}}) {
    CAPTURE(d.code, d.message);
    const auto got = planar::cmd::detail::registration_failed(d);
    CHECK(got.status == 4);
    CHECK(got.error == protocol);
  }
}
TEST_CASE("X07 command sources add no diagnostic outside the closed table", "[bootstrap][x07]") {
  std::set<std::string_view> allowed(x07::closed_messages().begin(), x07::closed_messages().end());
  const std::regex           pair(R"re(\{\s*"([a-z_]+)"\s*,\s*"((?:[^"\\]|\\.)*)")re");
  const std::regex           field(R"re("message"\s*,\s*text\(\s*"((?:[^"\\]|\\.)*)")re");
  const std::regex           assigned(R"re(\bmessage\s*=\s*"((?:[^"\\]|\\.)*)")re");
  const auto                 sources = x07::command_sources();
  CHECK(sources.size() >= 18);
  std::size_t literals = 0;
  for (const auto& [name, text] : sources) {
    for (std::sregex_iterator it(text.begin(), text.end(), pair), end; it != end; ++it) {
      CAPTURE(name, (*it)[1].str(), (*it)[2].str());
      ++literals;
      CHECK(x07::closed_codes().contains((*it)[1].str()));
      CHECK(allowed.contains((*it)[2].str()));
    }
    for (const auto* expression : {&field, &assigned})
      for (std::sregex_iterator it(text.begin(), text.end(), *expression), end; it != end; ++it) {
        CAPTURE(name, (*it)[1].str());
        ++literals;
        CHECK(allowed.contains((*it)[1].str()));
      }
  }
  CHECK(literals > 25);
  // The adapter golden table equals the closed table: the registration rows are exactly G23 to G30 and G32 to G34.
  CHECK(x07::registration_rows().size() == 11);
  for (const auto& row : x07::registration_rows()) {
    CAPTURE(row.id);
    CHECK(allowed.contains(row.message));
    const auto got = planar::cmd::detail::registration_failed(x07::as_diagnostic(row));
    CHECK(got.status == row.status);
    CHECK(got.error == x07::closed_json(row));
  }
  CHECK(x07::of(planar::cmd::detail::extension_usage_failed()) == x07::usage_bytes());
}

TEST_CASE("X07 selected help bytes are exact sorted and escaped", "[bootstrap][x07]") {
  namespace envelope = planar::cmd::envelope;
  const std::vector<envelope::help_command> commands{
      {"zeta", "last", {}},
      {"\xc3\xa9", "hi", {}},
      {"alpha",
       std::string("a\tb\nc\\d\x01", 9),
       {{"--zz", "text", false, "z\ty"}, {"--aa", "flag", true, "line\nbreak"}, {"--mm", "decimal", false, "caf\xc3\xa9"}}},
  };
  const auto got = envelope::format_selected_help(commands);
  REQUIRE(got);
  const std::string expected = "Usage: planar with-profile --stdlib-profile FILE COMMAND [OPTIONS]\n\nCommands:\n"
                               "  alpha\ta\\tb\\nc\\\\d\\u0001\\u0000\n"
                               "    --aa flag required\tline\\nbreak\n"
                               "    --mm decimal optional\tcaf\xc3\xa9\n"
                               "    --zz text optional\tz\\ty\n"
                               "  zeta\tlast\n"
                               "  \xc3\xa9\thi\n";
  CHECK(*got == expected);
  // The input order never changes the output.
  auto reversed = commands;
  std::ranges::reverse(reversed);
  const auto again = envelope::format_selected_help(reversed);
  REQUIRE(again);
  CHECK(*again == expected);
}
TEST_CASE("X07 selected help refuses above the help ceiling", "[bootstrap][x07]") {
  namespace envelope = planar::cmd::envelope;
  const std::vector<envelope::help_command> commands{{"alpha", "some help text", {{"--a", "text", true, "option help"}}}};
  const auto full = envelope::format_selected_help(commands, std::numeric_limits<std::size_t>::max());
  REQUIRE(full);
  REQUIRE_FALSE(full->empty());
  const auto exact = envelope::format_selected_help(commands, full->size());
  REQUIRE(exact);
  CHECK(*exact == *full);
  for (const std::size_t ceiling : {full->size() - 1, std::size_t{10}, std::size_t{0}}) {
    CAPTURE(ceiling);
    const auto refused = envelope::format_selected_help(commands, ceiling);
    REQUIRE_FALSE(refused);
    CHECK(refused.error().code == "resource_limit");
    CHECK(refused.error().message == "External profile resource limit exceeded");
  }
}
TEST_CASE("X07 src cmd contains no profile VM creation site", "[bootstrap][x07]") {
  std::size_t scanned = 0;
  for (const auto& [name, text] : x07::command_sources()) {
    ++scanned;
    const auto code = x07::squash(text);
    INFO(name);
    CHECK_FALSE(x07::contains(code, "extension_invocation::create"));
    // Only the argument-free embedded creation is allowed.
    for (auto at = code.find("command_vm::create("); at != std::string::npos; at = code.find("command_vm::create(", at + 1))
      CHECK(code.compare(at, std::string_view("command_vm::create()").size(), "command_vm::create()") == 0);
  }
  CHECK(scanned >= 18);
}
TEST_CASE("X07 history and export under a profile validate the profile before dispatch", "[bootstrap][x07]") {
  x07::oracle o;
  for (auto* t : {&o.a, &o.b, &o.c})
    x07::seed_v1(*t);
  const auto history = [](const x07::tree& t) {
    return x07::with_profile({"history", "--ledger", (t.work / "led.db").string()});
  };
  const auto exporting = [](const x07::tree& t, const char* target) {
    return x07::with_profile({"export", "--ledger", (t.work / "led.db").string(), "--revision", "1", "--target", target});
  };
  const auto ha = x07::of(x07::run(o.a, history(o.a)));
  CHECK(ha.status == 0);
  // A malformed profile is validated before the handler: the history result is never produced.
  x07::same(x07::of(x07::run(o.b, history(o.b))), o.bad);
  x07::same(x07::of(x07::run(o.b, exporting(o.b, "out"))), o.bad);
  CHECK_FALSE(x07::fs::exists(o.b.work / "out"));
  // A registered spinning profile also precedes the handler.
  x07::same(x07::of(x07::run(o.c, history(o.c))), o.spin);
  x07::same(x07::of(x07::run(o.c, exporting(o.c, "out"))), o.spin);
  CHECK_FALSE(x07::fs::exists(o.c.work / "out"));
}
TEST_CASE("X07 history and export responses are identical with and without a profile", "[bootstrap][x07]") {
  x07::tree  t(x07::kind::valid);
  const auto ledger = x07::seed_v1(t);
  const auto plain  = x07::run(t, {"history", "--ledger", ledger.string()});
  REQUIRE(plain.status == 0);
  const auto profiled = x07::run(t, x07::with_profile({"history", "--ledger", ledger.string()}));
  CHECK(profiled.status == 0);
  CHECK(profiled.output == plain.output);
  CHECK(profiled.error == plain.error);
  CHECK_FALSE(x07::contains(profiled.output, "extension_v1"));
  const auto plain_export = x07::run(t, {"export", "--ledger", ledger.string(), "--revision", "1", "--target", "plain-out"});
  REQUIRE(plain_export.status == 0);
  const auto profiled_export =
      x07::run(t, x07::with_profile({"export", "--ledger", ledger.string(), "--revision", "1", "--target", "profile-out"}));
  CHECK(profiled_export.status == 0);
  CHECK(profiled_export.output == plain_export.output);
  CHECK(profiled_export.error == plain_export.error);
  CHECK(x07::listing(t.work / "profile-out") == x07::listing(t.work / "plain-out"));
  for (const auto& relative : x07::listing(t.work / "plain-out")) {
    if (relative.ends_with('/'))
      continue;
    CHECK(x07::slurp(t.work / "profile-out" / relative) == x07::slurp(t.work / "plain-out" / relative));
  }
}
TEST_CASE("X07 export refuses a target overlapping the trusted root before publication", "[bootstrap][x07]") {
  x07::tree  t(x07::kind::valid);
  const auto ledger = x07::seed_v1(t);
  const auto before = x07::listing(t.root);
  const auto got    = x07::of(x07::run(t, x07::with_profile({"export", "--ledger", ledger.string(), "--revision", "1", "--target",
                                                             (t.trusted / "exported").string()})));
  x07::same(got, x07::unsafe_overlap());
  CHECK(x07::listing(t.root) == before);
  CHECK_FALSE(x07::fs::exists(t.trusted / "exported"));
  // A target in a sibling of the trusted root is not an overlap and is not refused as one.
  const auto sibling = x07::of(x07::run(t, x07::with_profile({"export", "--ledger", ledger.string(), "--revision", "1",
                                                              "--target", (t.work / "outside").string()})));
  CHECK_FALSE(sibling == x07::unsafe_overlap());
}
TEST_CASE("X07 ledger aliasing the profile or a module refuses for history export and load", "[bootstrap][x07]") {
  x07::tree t(x07::kind::valid);
  x07::seed_v1(t);
  const auto ledger = x07::alias_ledger_to_module(t);
  x07::same(x07::of(x07::run(t, x07::with_profile({"history", "--ledger", ledger.string()}))), x07::unsafe_overlap());
  x07::same(
      x07::of(x07::run(t, x07::with_profile({"export", "--ledger", ledger.string(), "--revision", "1", "--target", "out"}))),
      x07::unsafe_overlap());
  CHECK_FALSE(x07::fs::exists(t.work / "out"));
  // Load reads the ledger before the profile, so the aliased non-ledger is refused natively as foreign_ledger.
  x07::tree loaded(x07::kind::valid);
  x07::make_v2(loaded);
  const auto v2 = x07::alias_ledger_to_module(loaded);
  x07::same(x07::of(x07::run(loaded, x07::with_profile(x07::load_args(v2.string(), "alias-key")))),
            x07::ledger_failure(planar::engine::ledger::types::format_error_code::foreign_ledger));
}
TEST_CASE("X07 cmd sources open the profile owner through one factory call per branch", "[bootstrap][x07]") {
  // Weak source scan: help, catalog and dispatch share one capture and one registration, so the command sources hold exactly
  // one opaque open, one extension adoption and the two closed-factory overloads.
  std::size_t opaque = 0, adopt = 0, with_profile = 0;
  for (const auto& [name, text] : x07::command_sources()) {
    opaque += x07::count_of(text, "open_opaque(");
    adopt += x07::count_of(text, "into_extension(");
    with_profile += x07::count_of(text, "open_with_profile(");
  }
  CHECK(opaque == 1);
  CHECK(adopt == 1);
  CHECK(with_profile == 2);
}
TEST_CASE("X07 profile environment variables are ignored without with-profile", "[bootstrap][x07]") {
  x07::oracle o;
  const auto  args  = std::vector<std::string>{"history", "--ledger", "missing"};
  const auto  unset = x07::of(x07::run(o.a, args));
  for (const char* name : {"PLANAR_STDLIB_PROFILE", "PLANAR_PROFILE", "PLANAR_WITH_PROFILE", "STDLIB_PROFILE"})
    ::setenv(name, o.c.profile().c_str(), 1);
  const auto set = x07::of(x07::run(o.a, args));
  for (const char* name : {"PLANAR_STDLIB_PROFILE", "PLANAR_PROFILE", "PLANAR_WITH_PROFILE", "STDLIB_PROFILE"})
    ::unsetenv(name);
  CHECK(set == unset);
  CHECK(set.status == 3);
  // The only environment name the command sources read is the workspace directory.
  const std::regex      getenv_name(R"re(getenv\(\s*"([^"]+)"\s*\))re");
  std::set<std::string> names;
  for (const auto& [name, text] : x07::command_sources())
    for (std::sregex_iterator it(text.begin(), text.end(), getenv_name), end; it != end; ++it)
      names.insert((*it)[1].str());
  CHECK(names == std::set<std::string>{"PLANAR_WORKSPACE_DIR"});
}
TEST_CASE("X07 selected help ceiling is one mebibyte by default and bootstrap passes none", "[bootstrap][x07]") {
  namespace envelope             = planar::cmd::envelope;
  constexpr std::size_t mebibyte = 1048576;
  const auto            sized    = [](std::size_t total) {
    std::vector<envelope::help_command> commands{{"c", "", {}}};
    const auto                          probe = envelope::format_selected_help(commands, std::numeric_limits<std::size_t>::max());
    REQUIRE(probe);
    REQUIRE(total > probe->size());
    commands[0].help = std::string(total - probe->size(), 'h');
    return commands;
  };
  const auto under = envelope::format_selected_help(sized(mebibyte - 1));
  REQUIRE(under);
  CHECK(under->size() == mebibyte - 1);
  const auto over = envelope::format_selected_help(sized(mebibyte + 1));
  REQUIRE_FALSE(over);
  CHECK(over.error().code == "resource_limit");
  CHECK(over.error().message == "External profile resource limit exceeded");
  // Production wiring: every bootstrap call of the formatter passes the help alone and so inherits the default ceiling.
  const auto  text  = x07::squash(x07::slurp(std::filesystem::path(__FILE__).parent_path() / "bootstrap.cpp"));
  std::size_t calls = 0;
  for (auto at = text.find("format_selected_help("); at != std::string::npos; at = text.find("format_selected_help(", at + 1)) {
    ++calls;
    int depth = 0;
    for (auto i = at + std::string_view("format_selected_help").size(); i < text.size(); ++i) {
      if (text[i] == '(')
        ++depth;
      else if (text[i] == ')' && --depth == 0)
        break;
      else if (text[i] == ',' && depth == 1)
        FAIL("bootstrap passes an explicit help ceiling");
    }
  }
  CHECK(calls >= 1);
}

// ---- Wave B (X07 B-TEST): extension command contract through the production CLI, format2 cutover and load binding. ----
// Handlers report what they observe through a semantic `schema` refusal (exit 3, message passes through) because a command
// declaring a read capability cannot return a `summary`. Pure commands return a summary. Every row asserts exact bytes.
namespace {
namespace x07 {
std::string lua_opt(std::string_view name, std::string_view type, bool required = false) {
  return std::format("{{name='--{}',type='{}',required={},help='o'}}", name, type, required ? "true" : "false");
}
// One extension command; options are sorted by name (they all start with the same prefix, so string order is name order).
std::string lua_command(std::string_view name, std::vector<std::string> options, std::string_view capabilities,
                        std::string_view body) {
  std::ranges::sort(options);
  std::string joined;
  for (const auto& option : options)
    joined += (joined.empty() ? "" : ",") + option;
  return std::format("registration.add({{name='{}',family='extension_read_v1',handler_id='custom.{}',options=engine.array{{{}}},"
                     "help='h',response_kind='extension_v1',capabilities=engine.array{{{}}}}},function(inv,ctx) {} end)\n",
                     name, name, joined, capabilities, body);
}
std::vector<std::string> history_option_list() {
  return {lua_opt("after-event", "history_id"), lua_opt("at-event", "history_id"), lua_opt("ledger", "text", true),
          lua_opt("limit", "decimal"),          lua_opt("request-id", "text"),     lua_opt("revision", "history_id")};
}
// Reports type, value and presence of the three history IDs and the limit.
std::string history_reporter_module() {
  return lua_command("hist", history_option_list(), "'history_read_v1'",
                     "local o,p=inv.options,inv.present local names={'revision','at-event','after-event'} local t={} "
                     "for i=1,3 do local n=names[i] t[#t+1]=n..'='..type(o[n])..':'..o[n]..':'..(p[n] and 'true' or 'false') end "
                     "t[#t+1]='limit='..math.type(o.limit)..':'..o.limit..':'..(p.limit and 'true' or 'false') "
                     "local s='' for i=1,#t do s=s..(i>1 and ';' or '')..t[i] end "
                     "return {ok=false,error={code='schema',message=s,field=''}}");
}
void install(const tree& t, const std::string& module_source) {
  put(t.trusted / "modules/p.lua", module_source);
}
fs::path plain_ledger(const tree& t) {
  const auto ledger = t.work / "led.db";
  REQUIRE(planar::engine::ledger::initialize_new_v2(ledger, "quota"));
  return ledger;
}
// The exact message the reporter emits for the supplied history values and limit (absent values are "0"/false).
std::string reporter_message(std::map<std::string, std::string> ids, std::optional<std::string> limit) {
  std::string out;
  for (const char* name : {"revision", "at-event", "after-event"}) {
    const bool present = ids.contains(name);
    out += std::format("{}=string:{}:{};", name, present ? ids[name] : "0", present ? "true" : "false");
  }
  out += std::format("limit=integer:{}:{}", limit.value_or("0"), limit ? "true" : "false");
  return out;
}
bytes semantic_schema(std::string_view message) {
  return {3, "", error_json("schema", message)};
}
std::vector<std::string> hist_tail(std::vector<std::string> extra) {
  std::vector<std::string> tail{"hist", "--ledger", "led.db"};
  tail.insert(tail.end(), extra.begin(), extra.end());
  return tail;
}
bytes run_hist(const tree& t, std::vector<std::string> extra) {
  return of(run(t, with_profile(hist_tail(std::move(extra)))));
}
const std::array<const char*, 3> history_id_names{"revision", "at-event", "after-event"};
std::string                      summary_json(std::string_view command, std::string_view text) {
  return std::format(
      "{{\"command\":\"{}\",\"data\":{{\"text\":\"{}\",\"type\":\"summary\"}},\"kind\":\"extension\",\"version\":1}}\n", command,
      text);
}
} // namespace x07
} // namespace

TEST_CASE("X07 history_id vectors report Lua type and presence through the real CLI", "[bootstrap][x07]") {
  x07::tree t(x07::kind::valid);
  x07::install(t, x07::history_reporter_module());
  x07::plain_ledger(t);
  // Control: the handler is live and absent IDs arrive as "0" with present false.
  x07::same(x07::run_hist(t, {}), x07::semantic_schema(x07::reporter_message({}, std::nullopt)));
  for (const char* name : x07::history_id_names) {
    for (const char* token : {"1", "2147483647", "2147483648", "9223372036854775807"}) {
      CAPTURE(name, token);
      // Selectors are exclusive with each other but a lone revision or at-event is fine; after-event pages.
      x07::same(x07::run_hist(t, {std::string("--") + name, token}),
                x07::semantic_schema(x07::reporter_message({{name, token}}, std::nullopt)));
    }
  }
}
TEST_CASE("X07 history_id invalid spellings refuse before the handler", "[bootstrap][x07]") {
  x07::tree t(x07::kind::valid);
  x07::install(t, x07::history_reporter_module());
  x07::plain_ledger(t);
  // Contract lists ten tokens; whitespace expands to three spellings, giving the twelve of the X05 native list.
  const std::vector<std::string> tokens{"0", "00", "01", "+1", "-1", "1.0", "1e0", "", " ", " 1", "1 ", "9223372036854775808"};
  REQUIRE(tokens.size() == 12);
  const auto usage = x07::usage_bytes();
  // Control: the handler is reachable with valid input, so usage below is a refusal and not an absent command.
  x07::same(x07::run_hist(t, {"--revision", "1"}),
            x07::semantic_schema(x07::reporter_message({{"revision", "1"}}, std::nullopt)));
  for (const char* name : x07::history_id_names)
    for (const auto& token : tokens) {
      CAPTURE(name, token);
      const auto got = x07::run_hist(t, {std::string("--") + name, token});
      x07::same(got, usage);
      CHECK(got.out.empty());
    }
}
TEST_CASE("X07 history_id selector conflicts are usage regardless of magnitude", "[bootstrap][x07]") {
  x07::tree t(x07::kind::valid);
  x07::install(t, x07::history_reporter_module());
  x07::plain_ledger(t);
  const auto usage = x07::usage_bytes();
  for (const char* magnitude : {"1", "9223372036854775807"}) {
    const std::vector<std::vector<std::string>> conflicts{{"--revision", magnitude, "--at-event", magnitude},
                                                          {"--revision", magnitude, "--request-id", "k"},
                                                          {"--at-event", magnitude, "--request-id", "k"},
                                                          {"--revision", magnitude, "--after-event", magnitude},
                                                          {"--at-event", magnitude, "--after-event", magnitude},
                                                          {"--revision", magnitude, "--limit", "5"},
                                                          {"--at-event", magnitude, "--limit", "5"},
                                                          {"--request-id", "k", "--after-event", magnitude},
                                                          {"--request-id", "k", "--limit", "5"}};
    for (const auto& extra : conflicts) {
      CAPTURE(magnitude, extra);
      x07::same(x07::run_hist(t, extra), usage);
    }
  }
  // The mirror images are valid when only the page pair is combined.
  x07::same(x07::run_hist(t, {"--after-event", "9223372036854775807", "--limit", "5"}),
            x07::semantic_schema(x07::reporter_message({{"after-event", "9223372036854775807"}}, "5")));
}
TEST_CASE("X07 history_id after-event with limit pages through the real CLI", "[bootstrap][x07]") {
  x07::tree t(x07::kind::valid);
  x07::install(t, x07::history_reporter_module());
  x07::plain_ledger(t);
  // limit is a Lua number (integer) with present true, never a string; after-event keeps its exact decimal string.
  x07::same(x07::run_hist(t, {"--after-event", "1", "--limit", "5"}),
            x07::semantic_schema(x07::reporter_message({{"after-event", "1"}}, "5")));
  x07::same(x07::run_hist(t, {"--limit", "100", "--after-event", "2147483648"}),
            x07::semantic_schema(x07::reporter_message({{"after-event", "2147483648"}}, "100")));
}
TEST_CASE("X07 decimal and limit boundary vectors through the real CLI", "[bootstrap][x07]") {
  x07::tree t(x07::kind::valid);
  x07::install(t,
               x07::history_reporter_module() +
                   x07::lua_command("dec", {x07::lua_opt("count", "decimal")}, "",
                                    "return {ok=true,value={version=1,kind='extension',command=inv.command,data={type='summary',"
                                    "text=math.type(inv.options.count)..':'..inv.options.count}}}"));
  x07::plain_ledger(t);
  const auto usage = x07::usage_bytes();
  x07::same(x07::of(x07::run(t, x07::with_profile({"dec", "--count", "2147483647"}))),
            {0, x07::summary_json("dec", "integer:2147483647"), ""});
  x07::same(x07::of(x07::run(t, x07::with_profile({"dec", "--count", "1"}))), {0, x07::summary_json("dec", "integer:1"), ""});
  for (const char* refused : {"2147483648", "9223372036854775807", "0", "-1", "1.0"}) {
    CAPTURE(refused);
    x07::same(x07::of(x07::run(t, x07::with_profile({"dec", "--count", refused}))), usage);
  }
  for (const char* accepted : {"1", "100"})
    x07::same(x07::run_hist(t, {"--limit", accepted}), x07::semantic_schema(x07::reporter_message({}, accepted)));
  for (const char* refused : {"0", "101", "2147483648", "9223372036854775807"}) {
    CAPTURE(refused);
    x07::same(x07::run_hist(t, {"--limit", refused}), usage);
  }
}
TEST_CASE("X07 history_id misuse is refused at registration through the real CLI", "[bootstrap][x07]") {
  const auto protocol = x07::bytes{4, "", x07::error_json("internal", "Invalid external profile protocol")};
  // (a) history_id on another option name; (b) history_id on a command without history_read_v1; (c) a required history_id.
  const std::string body =
      "return {ok=true,value={version=1,kind='extension',command=inv.command,data={type='summary',text='x'}}}";
  const std::vector<std::string> modules{
      x07::lua_command("hist", {x07::lua_opt("ledger", "text", true), x07::lua_opt("count", "history_id")}, "'history_read_v1'",
                       body),
      x07::lua_command("hist", {x07::lua_opt("ledger", "text", true), x07::lua_opt("revision", "history_id")}, "", body),
      x07::lua_command("hist", {x07::lua_opt("ledger", "text", true), x07::lua_opt("revision", "history_id", true)},
                       "'history_read_v1'", body)};
  for (std::size_t i = 0; i < modules.size(); ++i) {
    CAPTURE(i);
    x07::tree t(x07::kind::valid);
    x07::install(t, modules[i]);
    x07::plain_ledger(t);
    x07::same(x07::run_hist(t, {}), protocol);
  }
}

namespace {
namespace x07 {
std::string role_module() {
  return lua_command("role",
                     {lua_opt("workspace", "text", true), lua_opt("registry", "text", true), lua_opt("root", "text", true),
                      lua_opt("job", "text", true), lua_opt("role", "text", true), lua_opt("base-root", "text"),
                      lua_opt("observations", "text"), lua_opt("budget-bytes", "decimal"), lua_opt("capture-outputs", "flag"),
                      lua_opt("text", "text")},
                     "'role_context_read_v1'", "return {ok=false,error={code='schema',message='handler ran',field=''}}");
}
std::vector<std::string> role_tail(const std::string& workspace, const std::string& registry, const std::string& root,
                                   std::vector<std::string> extra = {}) {
  std::vector<std::string> tail{"role", "--workspace", workspace,  "--registry", registry, "--root",
                                root,   "--job",       "combined", "--role",     "builder"};
  tail.insert(tail.end(), extra.begin(), extra.end());
  return tail;
}
} // namespace x07
} // namespace

TEST_CASE("X07 role read overlaps and aliases refuse before the handler runs", "[bootstrap][x07]") {
  x07::tree control(x07::kind::valid);
  x07::install(control, x07::role_module());
  x07::fs::create_directories(control.work / "src");
  // Control: valid slots reach the handler, proving the module is live.
  const auto live = x07::of(x07::run(control, x07::with_profile(x07::role_tail(x07::quota_workspace(), x07::quota_registry(),
                                                                               "source=" + (control.work / "src").string()))));
  CHECK(live.status == 3);
  CHECK(x07::contains(live.err, "handler ran"));
  struct row {
    const char*                                               name;
    std::function<std::vector<std::string>(const x07::tree&)> build;
  };
  const auto src  = [](const x07::tree& t) { return "source=" + (t.work / "src").string(); };
  const auto base = [&](const x07::tree& t, const std::string& root) {
    return x07::role_tail(x07::quota_workspace(), x07::quota_registry(), src(t),
                          {"--capture-outputs", "--base-root", "source=" + root});
  };
  const std::vector<row> rows{
      {"workspace file inside the trusted root",
       [&](const x07::tree& t) { return x07::role_tail((t.trusted / "workspace.lua").string(), x07::quota_registry(), src(t)); }},
      {"workspace file aliases a trusted file",
       [&](const x07::tree& t) { return x07::role_tail((t.work / "alias-ws.lua").string(), x07::quota_registry(), src(t)); }},
      {"observations file inside the trusted root",
       [&](const x07::tree& t) {
         return x07::role_tail(x07::quota_workspace(), x07::quota_registry(), src(t),
                               {"--observations", (t.trusted / "obs.json").string()});
       }},
      {"observations file aliases a trusted file",
       [&](const x07::tree& t) {
         return x07::role_tail(x07::quota_workspace(), x07::quota_registry(), src(t),
                               {"--observations", (t.work / "alias-obs.json").string()});
       }},
      {"source root equals the trusted root",
       [&](const x07::tree& t) {
         return x07::role_tail(x07::quota_workspace(), x07::quota_registry(), "source=" + t.trusted.string());
       }},
      {"source root contains the trusted root",
       [&](const x07::tree& t) {
         return x07::role_tail(x07::quota_workspace(), x07::quota_registry(), "source=" + t.root.string());
       }},
      {"base root equals the trusted root", [&](const x07::tree& t) { return base(t, t.trusted.string()); }},
      {"base root contains the trusted root", [&](const x07::tree& t) { return base(t, t.root.string()); }},
  };
  for (const auto& r : rows) {
    CAPTURE(r.name);
    x07::tree t(x07::kind::valid);
    x07::install(t, x07::role_module());
    x07::fs::create_directories(t.work / "src");
    x07::put(t.trusted / "workspace.lua", "-- placeholder");
    x07::put(t.trusted / "obs.json", "{}");
    x07::fs::create_hard_link(t.trusted / "p.json", t.work / "alias-ws.lua");
    x07::fs::create_hard_link(t.trusted / "modules/p.lua", t.work / "alias-obs.json");
    const auto got = x07::of(x07::run(t, x07::with_profile(r.build(t))));
    x07::same(got, x07::unsafe_overlap());
    CHECK_FALSE(x07::contains(got.err, "handler ran"));
  }
}
TEST_CASE("X07 final syntax error in an extension tail precedes late slot overlap", "[bootstrap][x07]") {
  x07::tree t(x07::kind::valid);
  x07::install(t, x07::role_module());
  x07::fs::create_directories(t.work / "src");
  x07::put(t.trusted / "workspace.lua", "-- placeholder");
  const auto overlapping = [&](std::vector<std::string> extra) {
    return x07::with_profile(x07::role_tail((t.trusted / "workspace.lua").string(), x07::quota_registry(),
                                            "source=" + (t.work / "src").string(), std::move(extra)));
  };
  // The overlapping slot alone is unsafe_path (the handler never runs).
  x07::same(x07::of(x07::run(t, overlapping({}))), x07::unsafe_overlap());
  // The same slot with a final-usage error is public usage and never unsafe_path.
  const auto mixed = x07::of(x07::run(t, overlapping({"--text", "--bogus"})));
  x07::same(mixed, x07::usage_bytes());
  CHECK_FALSE(x07::contains(mixed.err, "unsafe_path"));
  const auto unknown = x07::of(x07::run(t, overlapping({"--bogus"})));
  x07::same(unknown, x07::usage_bytes());
}

namespace {
namespace x07 {
std::string closed_module() {
  return lua_command("note", {lua_opt("text", "text", true)}, "",
                     "return {ok=true,value={version=1,kind='extension',command=inv.command,data={type='summary',text='n'}}}") +
         lua_command("hist2", history_option_list(), "'history_read_v1'",
                     "local h=ctx.read_history({}) return {ok=true,value={version=1,kind='extension',command=inv.command,"
                     "data={type='history',value=h}}}") +
         role_module() +
         lua_command("roleread",
                     {lua_opt("workspace", "text", true), lua_opt("registry", "text", true), lua_opt("root", "text", true),
                      lua_opt("job", "text", true), lua_opt("role", "text", true), lua_opt("base-root", "text"),
                      lua_opt("observations", "text"), lua_opt("budget-bytes", "decimal"), lua_opt("capture-outputs", "flag")},
                     "'role_context_read_v1'",
                     "local r=ctx.read_role({}) return {ok=true,value={version=1,kind='extension',command=inv.command,"
                     "data={type='role',value=r}}}");
}
// Lua statements binding local `name` to `count` copies of the one-byte string `unit` without the string library.
std::string lua_repeat(std::string_view name, char unit, std::size_t count) {
  return std::format("local {0},n_{0},u_{0}='',{1},'{2}' while n_{0}>0 do if n_{0}%2==1 then {0}={0}..u_{0} end n_{0}=n_{0}//2 "
                     "if n_{0}>0 then u_{0}=u_{0}..u_{0} end end ",
                     name, count, unit);
}
// History projection command; `sections` is a Lua expression for the list of custom sections.
std::string projection_module(std::string_view sections, std::string_view prelude = {}) {
  return lua_command("hproj", history_option_list(), "'history_read_v1','projection_apply_v1'",
                     "ctx.read_history({}) local p=ctx.project({id='p'}) return {ok=true,value={version=1,kind='extension',"
                     "command=inv.command,data=p}}") +
         std::format(
             "registration.projection({{id='p',abi='projection_v1',revision='final',input='history_snapshot_v1',help='p'}},"
             "function(input) {} return {{version=1,sections=engine.array{{{}}}}} end)\n",
             prelude, sections);
}
planar::engine::value_ptr decode(std::string_view line) {
  REQUIRE_FALSE(line.empty());
  REQUIRE(line.back() == '\n');
  auto value = planar::engine::codec_decode(line.substr(0, line.size() - 1), {16777216, 256, 4000000});
  REQUIRE(value);
  return *value;
}
std::set<std::string> keys(const planar::engine::value_ptr& record) {
  std::set<std::string> out;
  for (const auto& [name, ignored] : record->fields)
    out.insert(name);
  return out;
}
} // namespace x07
} // namespace

TEST_CASE("X07 extension responses are the closed extension_v1 and match retained services", "[bootstrap][x07]") {
  x07::tree t(x07::kind::valid);
  x07::install(t, x07::closed_module());
  x07::fs::create_directories(t.work / "src");
  const auto ledger = x07::seed_v1(t);
  // summary
  const auto summary = x07::of(x07::run(t, x07::with_profile({"note", "--text", "v"})));
  x07::same(summary, {0, x07::summary_json("note", "n"), ""});
  REQUIRE(summary.status == 0);
  const auto summary_record = x07::decode(summary.out);
  CHECK((x07::keys(summary_record) == std::set<std::string>{"command", "data", "kind", "version"}));
  // history equals the shipped history response for the same ledger, wrapped in the closed envelope
  const auto shipped = x07::run(t, {"history", "--ledger", ledger.string()});
  REQUIRE(shipped.status == 0);
  auto inner = shipped.output;
  inner.pop_back();
  const auto history = x07::of(x07::run(t, x07::with_profile({"hist2", "--ledger", ledger.string()})));
  x07::same(history, {0,
                      "{\"command\":\"hist2\",\"data\":{\"type\":\"history\",\"value\":" + inner +
                          "},\"kind\":\"extension\",\"version\":1}\n",
                      ""});
  // role: exact closed envelope, value is the retained role context
  const auto role = x07::of(x07::run(
      t, x07::with_profile({"roleread", "--workspace", x07::quota_workspace(), "--registry", x07::quota_registry(), "--root",
                            "source=" + (x07::fixture_root() / "current").string(), "--job", "combined", "--role", "builder"})));
  REQUIRE(role.status == 0);
  CHECK(role.err.empty());
  const auto decoded = x07::decode(role.out);
  CHECK((x07::keys(decoded) == std::set<std::string>{"command", "data", "kind", "version"}));
  CHECK(decoded->fields.at("kind")->string == "extension");
  CHECK(decoded->fields.at("command")->string == "roleread");
  const auto& data = decoded->fields.at("data");
  CHECK((x07::keys(data) == std::set<std::string>{"type", "value"}));
  CHECK(data->fields.at("type")->string == "role");
  for (const char* key : {"context", "work", "base", "observation", "outputs", "targets", "files", "prerequisites", "facets"})
    CHECK(data->fields.at("value")->fields.contains(key));
}
TEST_CASE("X07 projection omission vector reproduces 87 94 and 59 bytes through the CLI", "[bootstrap][x07]") {
  // The history projection budget is exactly 1 MiB and its first mandatory section is native, so the contract's 60-byte
  // vector cannot be fed literally. The same algorithm is driven at the real budget: optional a nearly fills the budget and
  // optional b is small, so removing b alone still exceeds the budget once the notice joins, and then a goes too.
  constexpr std::size_t budget = 1048576;
  const auto            probe  = [&](std::string_view sections, std::string_view prelude = {}) {
    x07::tree t(x07::kind::valid);
    x07::install(t, x07::projection_module(sections, prelude));
    x07::plain_ledger(t);
    return x07::of(x07::run(t, x07::with_profile({"hproj", "--ledger", "led.db"})));
  };
  const auto base = probe("");
  REQUIRE(base.status == 0);
  const auto first = x07::decode(base.out)->fields.at("data")->fields.at("sections");
  REQUIRE(!first->items.empty());
  CHECK(first->items[0]->fields.at("id")->string == "recorded_history");
  const std::size_t base_bytes = static_cast<std::size_t>(first->items[0]->fields.at("bytes")->integer);
  const std::size_t a_bytes    = budget - base_bytes - 20;
  const std::size_t b_bytes    = 37;
  const auto        a_text     = "## a\n\n" + std::string(a_bytes - 7, 'A') + "\n";
  const auto        b_text     = "## b\n\n" + std::string(b_bytes - 7, 'B') + "\n";
  REQUIRE(a_text.size() == a_bytes);
  REQUIRE(b_text.size() == b_bytes);
  const auto got = probe("{id='a',text=a..'\\n',mandatory=false},{id='b',text=b..'\\n',mandatory=false}",
                         x07::lua_repeat("a", 'A', a_bytes - 7) + x07::lua_repeat("b", 'B', b_bytes - 7));
  REQUIRE(got.status == 0);
  const auto data = x07::decode(got.out)->fields.at("data");
  const auto ids  = [](const planar::engine::value_ptr& list) {
    std::vector<std::string> out;
    for (const auto& item : list->items)
      out.push_back(item->string);
    return out;
  };
  CHECK((ids(data->fields.at("omitted")) == std::vector<std::string>{"b", "a"}));
  const std::string notice = "## omissions\n\nOptional sections omitted: b, a.\n";
  CHECK(data->fields.at("text")->string.ends_with("\n" + notice));
  CHECK(data->fields.at("text")->string.size() == base_bytes + 1 + notice.size());
  const auto& sections = data->fields.at("sections")->items;
  REQUIRE(sections.size() == 4);
  const std::array<std::pair<const char*, bool>, 4> expected{
      {{"recorded_history", true}, {"a", false}, {"b", false}, {"omissions", true}}};
  for (std::size_t i = 0; i < 4; ++i) {
    CHECK(sections[i]->fields.at("id")->string == expected[i].first);
    CHECK(sections[i]->fields.at("included")->boolean == expected[i].second);
  }
  CHECK(sections[1]->fields.at("bytes")->integer == static_cast<std::int64_t>(a_bytes));
  CHECK(sections[2]->fields.at("bytes")->integer == 37);
  CHECK(sections[3]->fields.at("bytes")->integer == static_cast<std::int64_t>(notice.size() + 1));
}
TEST_CASE("X07 extension and profile rows print one JSON line and nothing during registration", "[bootstrap][x07]") {
  x07::tree t(x07::kind::valid);
  x07::install(t, x07::closed_module());
  x07::plain_ledger(t);
  const auto ok = x07::run(t, x07::with_profile({"note", "--text", "v"}));
  CHECK(ok.status == 0);
  CHECK(ok.error.empty());
  REQUIRE_FALSE(ok.output.empty());
  CHECK(std::ranges::count(ok.output, '\n') == 1);
  CHECK(ok.output.back() == '\n');
  CHECK_FALSE(ok.output.ends_with("\n\n"));
  // A refused invocation prints exactly one error line on stderr and nothing on stdout.
  const auto refused = x07::run(
      t, x07::with_profile({"role", "--workspace", "w", "--registry", "r", "--root", "source=src", "--job", "j", "--role", "r"}));
  CHECK(refused.output.empty());
  CHECK(std::ranges::count(refused.error, '\n') == 1);
  CHECK(refused.error.back() == '\n');
  // Help prints one block and nothing on stderr; registration itself prints nothing in any outcome.
  const auto help = x07::run(t, {"with-profile", "--stdlib-profile", "../trusted/p.json", "--help"});
  CHECK(help.status == 0);
  CHECK(help.error.empty());
}
TEST_CASE("X07 public init creates a format2 ledger with unchanged response", "[bootstrap][x07]") {
  x07::tree  t(x07::kind::valid);
  const auto got = x07::run(t, {"init", "--ledger", "pub.db", "--workspace-id", "quota"});
  REQUIRE(got.status == 0);
  CHECK(got.error.empty());
  auto opened = planar::engine::ledger::open_existing(t.work / "pub.db", planar::engine::ledger::ledger_access::read_only);
  REQUIRE(opened);
  CHECK(opened->metadata().format == planar::engine::ledger::ledger_format::v2);
  const auto id = opened->metadata().identity;
  CHECK(got.output ==
        std::format("{{\"active_revision\":\"none\",\"event\":\"1\",\"kind\":\"init\",\"ledger\":\"{}\",\"version\":1,"
                    "\"workspace\":\"quota\"}}\n",
                    id));
}
TEST_CASE("X07 init refuses a format selector option", "[bootstrap][x07]") {
  x07::tree t(x07::kind::valid);
  for (const char* spelling : {"--format", "--ledger-format"}) {
    const auto got = x07::of(x07::run(t, {"init", "--ledger", "f.db", "--workspace-id", "quota", spelling, "2"}));
    x07::same(got, x07::usage_bytes());
  }
  CHECK_FALSE(x07::fs::exists(t.work / "f.db"));
}
TEST_CASE("X07 native format1 initializer remains available to fixtures", "[bootstrap][x07]") {
  x07::tree  t(x07::kind::valid);
  const auto made = planar::engine::ledger::initialize_new(t.work / "one.db", "quota");
  REQUIRE(made);
  CHECK(made->format == planar::engine::ledger::ledger_format::v1);
  auto opened = planar::engine::ledger::open_existing(t.work / "one.db", planar::engine::ledger::ledger_access::read_only);
  REQUIRE(opened);
  CHECK(opened->metadata().format == planar::engine::ledger::ledger_format::v1);
}

namespace {
namespace x07 {
std::string stored_body(const fs::path& ledger, std::string_view key) {
  auto file = planar::engine::ledger::open_existing(ledger, planar::engine::ledger::ledger_access::read_only);
  REQUIRE(file);
  auto row = file->read_request(key);
  REQUIRE(row);
  REQUIRE(*row);
  return (*row)->body_encoded;
}
std::string intent_bytes(std::string_view key, std::string_view actor, std::optional<std::string_view> profile) {
  auto intent = planar::engine::acceptance::request_intent::create_v2(key, actor, std::nullopt, quota_workspace(),
                                                                      quota_registry(), profile);
  REQUIRE(intent);
  return (*(*intent).data())->encoded;
}
planar::engine::value_ptr revision_refs(const fs::path& ledger) {
  const auto got = run_in(ledger.parent_path(), {"history", "--ledger", ledger.string(), "--revision", "1"});
  REQUIRE(got.status == 0);
  return decode(got.output)->fields.at("data")->fields.at("revision");
}
} // namespace x07
} // namespace

TEST_CASE("X07 embedded load on format2 frames request_v2 with embedded selector", "[bootstrap][x07]") {
  x07::tree  t(x07::kind::valid);
  const auto ledger = x07::make_v2(t);
  REQUIRE(x07::run(t, x07::load_args(ledger.string(), "k", "a")).status == 0);
  const auto body = x07::stored_body(ledger, "k");
  CHECK(body == x07::intent_bytes("k", "a", std::nullopt));
  CHECK(body.starts_with(std::string("PLANAR-REQUEST") + '\x02'));
}
TEST_CASE("X07 embedded load on format1 keeps request_v1", "[bootstrap][x07]") {
  x07::tree  t(x07::kind::valid);
  const auto ledger = t.work / "v1.db";
  REQUIRE(planar::engine::ledger::initialize_new(ledger, "quota"));
  REQUIRE(x07::run(t, x07::load_args(ledger.string(), "k", "a")).status == 0);
  auto intent =
      planar::engine::acceptance::request_intent::create("k", "a", std::nullopt, x07::quota_workspace(), x07::quota_registry());
  REQUIRE(intent);
  CHECK(x07::stored_body(ledger, "k") == (*(*intent).data())->encoded);
  // The format, not the command, selects the framing: the same embedded load on a format2 ledger is request_v2.
  const auto v2 = x07::make_v2(t);
  REQUIRE(x07::run(t, x07::load_args(v2.string(), "k", "a")).status == 0);
  CHECK(x07::stored_body(v2, "k") != x07::stored_body(ledger, "k"));
  CHECK(x07::stored_body(v2, "k") == x07::intent_bytes("k", "a", std::nullopt));
}
TEST_CASE("X07 embedded format2 load keeps v1 interpretation and versions", "[bootstrap][x07]") {
  x07::tree  t(x07::kind::valid);
  const auto v1 = t.work / "v1.db";
  REQUIRE(planar::engine::ledger::initialize_new(v1, "quota"));
  const auto v2 = x07::make_v2(t);
  REQUIRE(x07::run(t, x07::load_args(v1.string(), "k", "a")).status == 0);
  REQUIRE(x07::run(t, x07::load_args(v2.string(), "k", "a")).status == 0);
  // The request framing follows the format; the recorded provenance of an embedded load does not.
  CHECK(x07::stored_body(v2, "k").starts_with(std::string("PLANAR-REQUEST") + '\x02'));
  const auto old_refs = x07::revision_refs(v1);
  const auto new_refs = x07::revision_refs(v2);
  CHECK((x07::keys(old_refs) == x07::keys(new_refs)));
  for (const auto& [name, value] : old_refs->fields) {
    if (name == "accepted_at")
      continue;
    CAPTURE(name);
    CHECK(*planar::engine::codec_encode(value) == *planar::engine::codec_encode(new_refs->fields.at(name)));
  }
}
TEST_CASE("X07 profile load records request_v2 selector effective stdlib and v2 interpretation", "[bootstrap][x07]") {
  x07::tree  t(x07::kind::valid);
  const auto embedded = x07::make_v2(t, "embedded.db");
  const auto profiled = x07::make_v2(t, "profiled.db");
  const auto native   = x07::make_v2(t, "native.db");
  REQUIRE(x07::run(t, x07::load_args(embedded.string(), "k", "a")).status == 0);
  const auto got = x07::run(t, x07::with_profile(x07::load_args(profiled.string(), "k", "a")));
  REQUIRE(got.status == 0);
  CHECK(got.error.empty());
  CHECK(x07::stored_body(profiled, "k") == x07::intent_bytes("k", "a", t.profile()));
  // Same profile and inputs through the native seam record identical provenance; the embedded load records different one.
  x07::save_profile_load(t, native, "k", "a");
  const auto cli_refs    = x07::revision_refs(profiled);
  const auto native_refs = x07::revision_refs(native);
  const auto plain_refs  = x07::revision_refs(embedded);
  for (const char* name : {"stdlib", "interpretation", "versions", "source", "registry"}) {
    CAPTURE(name);
    if (!cli_refs->fields.contains(name))
      continue;
    CHECK(*planar::engine::codec_encode(cli_refs->fields.at(name)) ==
          *planar::engine::codec_encode(native_refs->fields.at(name)));
  }
  CHECK(*planar::engine::codec_encode(cli_refs->fields.at("stdlib")) !=
        *planar::engine::codec_encode(plain_refs->fields.at("stdlib")));
}
TEST_CASE("X07 profile load binds the early lookup intent through prepare_load", "[bootstrap][x07]") {
  x07::tree  t(x07::kind::valid);
  const auto ledger = x07::make_v2(t);
  const auto first  = x07::run(t, x07::with_profile(x07::load_args(ledger.string(), "bind", "carol")));
  REQUIRE(first.status == 0);
  // The committed request body is exactly create_v2 of the CLI-derived intent: actor and selector unchanged.
  CHECK(x07::stored_body(ledger, "bind") == x07::intent_bytes("bind", "carol", t.profile()));
  // An identical rerun replays the saved bytes through the early lookup, before the (now unreadable) profile is touched.
  x07::put(t.trusted / "p.json", "{\"version\":1,\"modules\":");
  const auto again = x07::run(t, x07::with_profile(x07::load_args(ledger.string(), "bind", "carol")));
  CHECK(again.status == 0);
  CHECK(again.output == first.output);
  CHECK(again.error.empty());
}

TEST_CASE("X07 wave B fixture modules register natively and misuse modules are refused", "[bootstrap][x07]") {
  // Guard: the Lua fixtures used by the wave B rows are valid protocol, so the rows are red only for want of the CLI path.
  const std::vector<std::pair<const char*, std::string>> valid{
      {"reporter", x07::history_reporter_module()},
      {"roles", x07::role_module()},
      {"closed", x07::closed_module()},
      {"projection", x07::projection_module("{id='a',text='## a\\n\\nA\\n',mandatory=false}")}};
  for (const auto& [name, source] : valid) {
    CAPTURE(name);
    x07::tree t(x07::kind::valid);
    x07::install(t, source);
    auto opened = planar::engine::profile_invocation::open_opaque(t.profile());
    INFO((opened ? std::string("opened")
                 : opened.error().detail.code + " " + opened.error().detail.message + " " + opened.error().field));
    CHECK(opened);
  }
  const std::string body =
      "return {ok=true,value={version=1,kind='extension',command=inv.command,data={type='summary',text='x'}}}";
  const std::vector<std::string> misuse{
      x07::lua_command("hist", {x07::lua_opt("ledger", "text", true), x07::lua_opt("count", "history_id")}, "'history_read_v1'",
                       body),
      x07::lua_command("hist", {x07::lua_opt("ledger", "text", true), x07::lua_opt("revision", "history_id")}, "", body),
      x07::lua_command("hist", {x07::lua_opt("ledger", "text", true), x07::lua_opt("revision", "history_id", true)},
                       "'history_read_v1'", body)};
  for (const auto& source : misuse) {
    x07::tree t(x07::kind::valid);
    x07::install(t, source);
    auto opened = planar::engine::profile_invocation::open_opaque(t.profile());
    REQUIRE_FALSE(opened);
    CHECK(opened.error().detail.code == "internal");
    CHECK(opened.error().detail.message == "Invalid external profile protocol");
  }
}

namespace {
namespace x07 {
// Native-seam run of one extension command from a profile module: capture, register, admit and execute without the CLI.
std::expected<planar::engine::value_ptr, planar::engine::extension_execution_error>
native_execute(const tree& t, std::string_view command, const std::vector<planar::engine::extension_parsed_option>& options,
               std::string* why = nullptr) {
  auto opened = planar::engine::profile_invocation::open_opaque(t.profile());
  REQUIRE(opened);
  auto owner = std::move(*opened).into_extension();
  REQUIRE(owner);
  auto admitted = owner->admit(command, options, t.work.string());
  if (!admitted && why)
    *why =
        "admit refused: " + admitted.error().detail.code + " " + admitted.error().detail.message + " " + admitted.error().field;
  if (!admitted)
    return std::unexpected(planar::engine::extension_execution_error{admitted.error()});
  auto result = owner->execute();
  if (!result && why) {
    if (const auto* registration = std::get_if<planar::engine::registration_error>(&result.error()))
      *why = registration->detail.code + " " + registration->detail.message + " " + registration->detail.limit + " " +
             registration->field;
    else if (const auto* assembly = std::get_if<planar::engine::assembly_error>(&result.error()))
      *why = "assembly refused: " + *planar::engine::codec_encode(assembly->error);
    else if (const auto* ledger = std::get_if<planar::engine::ledger::format_error>(&result.error()))
      *why = "ledger refused: " + ledger->message;
    else if (const auto* history = std::get_if<planar::engine::history::history_read_error>(&result.error()))
      *why = "history refused: " + std::to_string(static_cast<int>(history->code));
  }
  return result;
}
} // namespace x07
} // namespace

TEST_CASE("X07 wave B reporter and projection modules behave natively as the rows expect", "[bootstrap][x07]") {
  // Guard: the same fixtures run through the native extension seam, so the expected observation strings are not guesses.
  using planar::engine::extension_parsed_option;
  {
    x07::tree t(x07::kind::valid);
    x07::install(t, x07::history_reporter_module());
    x07::plain_ledger(t);
    const std::vector<extension_parsed_option> options{
        {"ledger", true, "led.db"}, {"after-event", true, "2147483648"}, {"limit", true, "5"}};
    std::string why;
    auto        result = x07::native_execute(t, "hist", options, &why);
    INFO(why);
    REQUIRE(result);
    const auto& error = (*result)->fields.at("error");
    CHECK(error->fields.at("code")->string == "schema");
    CHECK(error->fields.at("message")->string == x07::reporter_message({{"after-event", "2147483648"}}, "5"));
  }
  {
    // Sizes of the vector rows are those the native projection assembly really produces.
    constexpr std::size_t budget = 1048576;
    x07::tree             probe(x07::kind::valid);
    x07::install(probe, x07::projection_module(""));
    x07::plain_ledger(probe);
    auto base = x07::native_execute(probe, "hproj", {{"ledger", true, "led.db"}});
    REQUIRE(base);
    const auto base_bytes =
        (*(*base)->fields.at("value")->fields.at("data")->fields.at("sections")->items.begin())->fields.at("bytes")->integer;
    CHECK(base_bytes > 100);
    CHECK(base_bytes < 400);
    x07::tree  t(x07::kind::valid);
    const auto a_bytes = budget - static_cast<std::size_t>(base_bytes) - 20;
    x07::install(t, x07::projection_module("{id='a',text=a..'\\n',mandatory=false},{id='b',text=b..'\\n',mandatory=false}",
                                           x07::lua_repeat("a", 'A', a_bytes - 7) + x07::lua_repeat("b", 'B', 30)));
    x07::plain_ledger(t);
    std::string why;
    auto        result = x07::native_execute(t, "hproj", {{"ledger", true, "led.db"}}, &why);
    INFO(why);
    REQUIRE(result);
    const auto& data = (*result)->fields.at("value")->fields.at("data");
    REQUIRE(data->fields.at("omitted")->items.size() == 2);
    CHECK(data->fields.at("omitted")->items[0]->string == "b");
    CHECK(data->fields.at("omitted")->items[1]->string == "a");
  }
  {
    // The role row's arguments really read a role context natively.
    x07::tree t(x07::kind::valid);
    x07::install(t, x07::closed_module());
    const std::string                          root      = "source=" + (x07::fixture_root() / "current").string();
    const std::string                          workspace = x07::quota_workspace();
    const std::string                          registry  = x07::quota_registry();
    const std::vector<extension_parsed_option> options{{"workspace", true, workspace},
                                                       {"registry", true, registry},
                                                       {"root", true, root},
                                                       {"job", true, "combined"},
                                                       {"role", true, "builder"}};
    std::string                                why;
    auto                                       result = x07::native_execute(t, "roleread", options, &why);
    INFO(why);
    REQUIRE(result);
    const auto& data = (*result)->fields.at("value")->fields.at("data");
    CHECK(data->fields.at("type")->string == "role");
    for (const char* key : {"context", "work", "base", "observation", "outputs", "targets", "files", "prerequisites", "facets"})
      CHECK(data->fields.at("value")->fields.contains(key));
  }
}
