// Native host-path extraction and filesystem-free request intent normalization.
#include <catch2/catch_test_macros.hpp>
#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.engine.capture.host_path;
using namespace planar::engine;
namespace {
struct temporary {
  std::filesystem::path path;
  temporary() {
    char pattern[] = "/tmp/planar-host-path-XXXXXX";
    path           = std::filesystem::canonical(::mkdtemp(pattern));
    std::filesystem::create_directories(path / "physical/child");
    std::filesystem::create_directories(path / "lexical");
    std::ofstream(path / "physical/file") << "physical";
    std::ofstream(path / "lexical/file") << "lexical";
    std::filesystem::create_directory_symlink(path / "physical/child", path / "lexical/hop");
    std::filesystem::create_directory_symlink("physical", path / "alias");
    std::filesystem::create_symlink("physical/file", path / "file-link");
    std::filesystem::create_symlink("loop", path / "loop");
  }
  ~temporary() {
    std::filesystem::remove_all(path);
  }
};
void refused(const std::expected<std::string, diagnostic>& result, std::string_view code, std::string_view message) {
  REQUIRE_FALSE(result);
  CHECK(result.error().code == code);
  CHECK(result.error().message == message);
  CHECK(result.error().limit.empty());
  CHECK(result.error().maximum == 0);
}
} // namespace
TEST_CASE("host path preserves physical parent traversal", "[host_path]") {
  temporary t;
  CHECK(resolve_explicit_file((t.path / "lexical/hop/../file").string()) == (t.path / "physical/file").string());
  CHECK(resolve_explicit_file((t.path / "alias/file").string()) == (t.path / "physical/file").string());
  CHECK(resolve_explicit_directory((t.path / "lexical/hop/..").string()) == (t.path / "physical").string());
  refused(resolve_explicit_directory((t.path / "lexical/hop").string()), "path_denied",
          "Explicit root is a symbolic link target");
  CHECK(resolve_explicit_file((t.path / "physical/file").lexically_relative(std::filesystem::current_path()).string()) ==
        (t.path / "physical/file").string());
  CHECK(resolve_explicit_directory((t.path / "physical").lexically_relative(std::filesystem::current_path()).string()) ==
        (t.path / "physical").string());
  // A direct final alias is refused; a link to a different intermediate directory is allowed.
  refused(resolve_explicit_directory((t.path / "alias").string()), "path_denied", "Explicit root is a symbolic link target");
}
TEST_CASE("host path retains ordinary directory spellings", "[host_path]") {
  temporary t;
  for (const auto& suffix : {"", "/", "/.", "/child/..", "/child/../."})
    CHECK(resolve_explicit_directory((t.path / "physical").string() + suffix) == (t.path / "physical").string());
  CHECK(resolve_explicit_file((t.path / "physical/./child/../file").string()) == (t.path / "physical/file").string());
}
TEST_CASE("host path refuses final root aliases with all suffixes", "[host_path]") {
  temporary t;
  for (const auto& suffix : {"", "/", "/.", "/child/..", "/child/../."})
    refused(resolve_explicit_directory((t.path / "alias").string() + suffix), "path_denied",
            "Explicit root is a symbolic link target");
  std::filesystem::create_directory_symlink("alias", t.path / "nested");
  refused(resolve_explicit_directory((t.path / "nested/child/..").string()), "path_denied",
          "Explicit root is a symbolic link target");
}
TEST_CASE("host path refuses file leaf links and nonregular inputs", "[host_path]") {
  temporary t;
  refused(resolve_explicit_file((t.path / "file-link").string()), "path_denied", "Explicit input file is a symbolic link");
  refused(resolve_explicit_file((t.path / "physical").string()), "io", "Explicit input must be a regular file");
  REQUIRE(::mkfifo((t.path / "pipe").c_str(), 0600) == 0);
  refused(resolve_explicit_file((t.path / "pipe").string()), "io", "Explicit input must be a regular file");
  refused(resolve_explicit_directory((t.path / "pipe").string()), "io", "Explicit root must be a directory");
  refused(resolve_explicit_directory((t.path / "physical/file").string()), "io", "Explicit root must be a directory");
}
TEST_CASE("host path sanitizes invalid file traversal and missing parents", "[host_path]") {
  temporary t;
  for (const auto& name : {"physical/file/", "physical/file/.", "physical/file/../file", "absent/../physical/file"})
    refused(resolve_explicit_file((t.path / name).string()), "io", "Explicit input must be a regular file");
  refused(resolve_explicit_file((t.path / "absent").string()), "io", "Explicit input must be a regular file");
  refused(resolve_explicit_file((t.path / "loop").string()), "path_denied", "Explicit input file is a symbolic link");
  refused(resolve_explicit_file((t.path / "loop/file").string()), "io", "Cannot resolve explicit input path");
  refused(resolve_explicit_directory((t.path / "physical/file/..").string()), "io", "Cannot resolve explicit root directory");
  for (const auto& name : {"absent", "absent/../physical", "loop"})
    refused(resolve_explicit_directory((t.path / name).string()), "io", "Cannot resolve explicit input path");
}
TEST_CASE("host path maps a regular file followed by parent traversal to the root directory error", "[host_path]") {
  temporary t;
  for (const auto& name : {"physical/file/..", "file-link/..", "alias/file/..", "physical/file/../file", "lexical/hop/../file/..",
                           "physical/file/./.."})
    refused(resolve_explicit_directory((t.path / name).string()), "io", "Cannot resolve explicit root directory");
}
TEST_CASE("host path keeps missing and unresolvable directory spellings as input path errors", "[host_path]") {
  temporary t;
  for (const auto& name : {"absent/../physical", "physical/file/x/.."})
    refused(resolve_explicit_directory((t.path / name).string()), "io", "Cannot resolve explicit input path");
}
TEST_CASE("intent path normalizes nonexistent lexical equivalents against explicit cwd", "[host_path]") {
  // No root here is required to exist; the cwd is data, not ambient process state.
  CHECK(normalize_intent_path("../input/./job/../workspace.lua", "/uncreated/cwd") == "/uncreated/input/workspace.lua");
  CHECK(normalize_intent_path("/uncreated/input/workspace.lua", "/different") == "/uncreated/input/workspace.lua");
  CHECK(normalize_intent_path("../input/./workspace.lua", "/uncreated/cwd") == "/uncreated/input/workspace.lua");
  CHECK(normalize_intent_path("/x/../../y", "/uncreated/cwd") == "/y");
  CHECK(normalize_intent_path(".", "/uncreated/cwd") == "/uncreated/cwd/");
  CHECK(normalize_intent_path("../", "/uncreated/cwd") == "/uncreated/");
  CHECK(normalize_intent_path("résumé/文件", "/uncreated") == "/uncreated/résumé/文件");
  CHECK(normalize_intent_path("relative", "/") == "/relative");
}
TEST_CASE("intent path refuses invalid text and cwd shape without leaking bytes", "[host_path]") {
  for (const auto& text :
       {std::string(), std::string("x\0private", 9), std::string("\xc0\xaf", 2), std::string("\xed\xa0\x80", 3)}) {
    refused(normalize_intent_path(text, "/valid"), "path_denied", "Invalid explicit input path");
    refused(normalize_intent_path("/absolute", text), "path_denied", "Invalid explicit input cwd");
  }
  for (const auto& cwd : {"relative", "/a/../b", "/a//b", "/a/./b"})
    refused(normalize_intent_path("/absolute", cwd), "path_denied", "Invalid explicit input cwd");
}
TEST_CASE("intent path bounds input and joined output before concatenation", "[host_path]") {
  constexpr std::size_t maximum = 64 * 1024 * 1024;
  auto                  limited = [](const auto& result) {
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "resource_limit");
    CHECK(result.error().message == "Explicit input path limit exceeded");
    CHECK(result.error().limit == "path_bytes");
    CHECK(result.error().maximum == maximum);
  };
  std::string large(maximum + 1, 'x');
  limited(normalize_intent_path(large, "/"));
  limited(normalize_intent_path("/x", large));
  large.resize(maximum);
  large[0]   = '/';
  auto exact = normalize_intent_path(large, "/");
  REQUIRE(exact);
  CHECK(exact->size() == maximum);
  auto exact_join = normalize_intent_path(std::string_view(large).substr(1), "/");
  REQUIRE(exact_join);
  CHECK(exact_join->size() == maximum);
  limited(normalize_intent_path("x", large));
  limited(normalize_intent_path(".", large));
  limited(normalize_intent_path(std::string_view(large).substr(1), "/cwd"));
}
TEST_CASE("workspace selector classifies exact names and explicit file grammar", "[host_path][workspace_selector]") {
  for (const auto& name : {std::string("a"), std::string("9_A-z"), std::string(64, 'x')}) {
    CHECK(workspace_is_name(name) == true);
    CHECK(expand_workspace(name, "/unused/../workspaces/.") == "/workspaces/" + name + "/workspace.lua");
  }
  for (const auto& name : {std::string(), std::string("."), std::string(".."), std::string("a.LUA"), std::string("a b"),
                           std::string("a\\b"), std::string("a:b"), std::string("-a"), std::string("_a"), std::string("é"),
                           std::string(65, 'x'), std::string(67108865, 'x')}) {
    const auto classified = workspace_is_name(name);
    REQUIRE_FALSE(classified);
    CHECK(classified.error().code == "usage");
  }
  for (const auto path : {"/absolute", "a/b", "./a", "a.lua", ".lua", "a b.lua", "a\\b.lua", "a/b.LUA"}) {
    CHECK(workspace_is_name(path) == false);
    CHECK(expand_workspace(path, "invalid-relative-env") == path);
    CHECK(expand_workspace(path, std::string_view("\xff", 1)) == path);
    CHECK(expand_workspace(path, {}) == path);
  }
}
TEST_CASE("workspace names require only explicit absolute valid environment", "[host_path][workspace_selector]") {
  for (const auto& directory :
       {std::optional<std::string_view>(), std::optional<std::string_view>(""), std::optional<std::string_view>("relative"),
        std::optional<std::string_view>("~/workspaces"), std::optional<std::string_view>(std::string_view("/\xff", 2)),
        std::optional<std::string_view>(std::string_view("/x\0secret", 9))}) {
    const auto result = expand_workspace("demo", directory);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "usage");
    CHECK(result.error().message.find("secret") == std::string::npos);
  }
  CHECK(expand_workspace("demo", "/") == "/demo/workspace.lua");
  CHECK(expand_workspace("demo", "/a/../b//") == "/b/demo/workspace.lua");
}
TEST_CASE("prepared native paths retain render spelling and normalize future intent", "[host_path][workspace_selector]") {
  temporary  t;
  const auto spelling = (t.path / "lexical/hop/../file").string();
  const auto prepared = prepare_host_path(spelling);
  REQUIRE(prepared);
  CHECK(prepared->spelling == spelling);
  CHECK(prepared->intent == (t.path / "lexical/file").string());
  CHECK(resolve_explicit_file(prepared->spelling) == (t.path / "physical/file").string());
  CHECK(resolve_explicit_file(prepared->intent) == (t.path / "lexical/file").string());
  const auto relative = prepare_host_path("hop/../file", (t.path / "lexical").string());
  REQUIRE(relative);
  CHECK(relative->spelling == spelling);
  CHECK(relative->intent == prepared->intent);
  CHECK(prepare_host_path("/abs/../file", "malformed-cwd")->intent == "/file");
  CHECK_FALSE(prepare_host_path("relative"));
  CHECK_FALSE(prepare_host_path("relative", "/a/../b"));
}
TEST_CASE("workspace preparation validates text and checks all join bounds", "[host_path][workspace_selector]") {
  constexpr std::size_t maximum = 64 * 1024 * 1024;
  for (const auto& text : {std::string("/x\0secret", 9), std::string("/\xff", 2)}) {
    refused(expand_workspace(text, {}), "path_denied", "Invalid explicit input path");
    const auto result = prepare_host_path(text);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "path_denied");
  }
  std::string base(maximum, 'x');
  base[0]          = '/';
  auto check_limit = [&](const auto& result) {
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "resource_limit");
    CHECK(result.error().limit == "path_bytes");
    CHECK(result.error().maximum == maximum);
  };
  check_limit(expand_workspace("demo", base));
  check_limit(prepare_host_path("x", base));
  check_limit(prepare_host_path(base + "x"));
  const auto absolute = prepare_host_path(base);
  REQUIRE(absolute);
  CHECK(absolute->spelling.size() == maximum);
  CHECK(absolute->intent.size() == maximum);
  base.resize(maximum - std::string_view("/demo/workspace.lua").size());
  const auto exact = expand_workspace("demo", base);
  REQUIRE(exact);
  CHECK(exact->size() == maximum);
  base += 'x';
  check_limit(expand_workspace("demo", base));
}
TEST_CASE("host path pure preflight preserves relevant text size and ignored snapshot errors", "[host_path][preflight]") {
  for (auto cwd : {std::optional<std::string_view>{}, std::optional<std::string_view>{""},
                   std::optional<std::string_view>{"relative"}, std::optional<std::string_view>{"/bad\xff"}}) {
    auto checked  = validate_host_path_inputs("relative", cwd);
    auto prepared = prepare_host_path("relative", cwd);
    REQUIRE_FALSE(checked);
    REQUIRE_FALSE(prepared);
    CHECK(checked.error().code == prepared.error().code);
    CHECK(checked.error().message == prepared.error().message);
    REQUIRE(validate_host_path_inputs("/absolute", cwd));
    REQUIRE(prepare_host_path("/absolute", cwd));
  }
  for (auto directory : {std::optional<std::string_view>{}, std::optional<std::string_view>{""},
                         std::optional<std::string_view>{"relative"}, std::optional<std::string_view>{"/bad\xff"}}) {
    auto checked  = validate_workspace_directory(directory);
    auto copied   = snapshot_workspace_directory(directory);
    auto expanded = expand_workspace("demo", directory);
    REQUIRE_FALSE(checked);
    REQUIRE_FALSE(copied);
    REQUIRE_FALSE(expanded);
    CHECK(checked.error().code == "usage");
    CHECK(checked.error().message == copied.error().message);
    CHECK(checked.error().message == expanded.error().message);
    REQUIRE(expand_workspace("/explicit.lua", directory));
  }
}
TEST_CASE("host path pure preflight checks malformed oversized text before size", "[host_path][preflight]") {
  constexpr std::size_t maximum = 64 * 1024 * 1024;
  std::string           snapshot(maximum + 1, 'x');
  snapshot[0]      = '/';
  snapshot[1]      = '\0';
  auto cwd         = validate_host_path_inputs("relative", snapshot);
  auto environment = validate_workspace_directory(snapshot);
  REQUIRE_FALSE(cwd);
  REQUIRE_FALSE(environment);
  CHECK(cwd.error().code == "path_denied");
  CHECK(environment.error().code == "usage");
  snapshot[1] = 'x';
  for (auto checked : {validate_host_path_inputs("relative", snapshot), validate_workspace_directory(snapshot)}) {
    REQUIRE_FALSE(checked);
    CHECK(checked.error().code == "resource_limit");
    CHECK(checked.error().limit == "path_bytes");
    CHECK(checked.error().maximum == maximum);
  }
  REQUIRE(validate_host_path_inputs("/absolute", snapshot));
}
TEST_CASE("host path lexical cwd validity still precedes joined length overflow", "[host_path][preflight]") {
  constexpr std::size_t maximum = 64 * 1024 * 1024;
  std::string           cwd(maximum, 'a');
  cwd[0] = '/';
  cwd.replace(maximum - 3, 3, "/..");
  REQUIRE(validate_host_path_inputs("relative", cwd));
  auto result = prepare_host_path("relative", cwd);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "path_denied");
  CHECK(result.error().message == "Invalid explicit input cwd");
  cwd.replace(maximum - 3, 3, "aaa");
  REQUIRE(validate_host_path_inputs("relative", cwd));
  result = prepare_host_path("relative", cwd);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "resource_limit");
  CHECK(result.error().limit == "path_bytes");
}
