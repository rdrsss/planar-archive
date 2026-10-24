// Replacement invocation boundary tests; no legacy test harness or runtime.
#include <catch2/catch_test_macros.hpp>
import std;
import cli11;
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
