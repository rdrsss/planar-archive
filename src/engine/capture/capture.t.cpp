// Native captured filesystem boundary tests; each test owns its temporary roots.
#include <catch2/catch_test_macros.hpp>
#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.engine.capture;
using namespace planar::engine;
namespace {
struct counting_sink : budget_sink {
  std::size_t                     native = 0, instructions = 0, live = 0, peak = 0;
  std::size_t                     native_ceiling = 0, instruction_ceiling = 0, lua_ceiling = 0; ///< Zero means none.
  std::expected<void, diagnostic> charge_native(std::size_t bytes) override {
    if (native_ceiling && bytes > native_ceiling - std::min(native, native_ceiling))
      return std::unexpected(
          diagnostic{"resource_limit", "Native conversion allocation exceeded", "native_bytes", native_ceiling});
    native += bytes;
    return {};
  }
  std::expected<void, diagnostic> charge_instructions(std::size_t count) override {
    if (instruction_ceiling && count > instruction_ceiling - std::min(instructions, instruction_ceiling))
      return std::unexpected(diagnostic{"resource_limit", "Lua instruction limit exceeded", "instructions", instruction_ceiling});
    instructions += count;
    return {};
  }
  std::expected<void, diagnostic> acquire_lua(std::size_t bytes) override {
    if (lua_ceiling && bytes > lua_ceiling - std::min(live, lua_ceiling))
      return std::unexpected(diagnostic{"resource_limit", "Lua allocation exceeded", "lua_bytes", lua_ceiling});
    live += bytes;
    peak = std::max(peak, live);
    return {};
  }
  void release_lua(std::size_t bytes) noexcept override {
    live -= std::min(live, bytes);
  }
};
struct temporary {
  std::filesystem::path path;
  temporary() {
    char pattern[] = "/tmp/planar-capture-XXXXXX";
    path           = std::filesystem::canonical(::mkdtemp(pattern));
  }
  ~temporary() {
    std::filesystem::remove_all(path);
  }
  void put(std::string name, std::string bytes) {
    auto p = path / name;
    std::filesystem::create_directories(p.parent_path());
    std::ofstream(p, std::ios::binary) << bytes;
  }
};
/// Whole text of a product source file, read by the source-guard tests.
std::string read_source(const std::filesystem::path& source) {
  std::ifstream in(source, std::ios::binary);
  return std::string{std::istreambuf_iterator<char>(in), {}};
}
/// Whitespace-stripped text: the spelling every source guard compares against.
std::string squash_space(std::string_view text) {
  std::string out;
  for (const char c : text)
    if (!std::isspace(static_cast<unsigned char>(c)))
      out += c;
  return out;
}
/// Number of non-overlapping occurrences of needle in text.
std::size_t occurrences(std::string_view text, std::string_view needle) {
  std::size_t n = 0;
  for (auto at = text.find(needle); at != std::string_view::npos; at = text.find(needle, at + needle.size()))
    ++n;
  return n;
}
bool has_text(std::string_view text, std::string_view needle) {
  return text.find(needle) != std::string_view::npos;
}
/// Body of a helper from its squashed signature to the earliest following helper signature or capture_limit.
std::string_view helper_body(std::string_view text, std::string_view signature) {
  const auto start = text.find(signature);
  if (start == std::string_view::npos)
    return {};
  auto end = text.size();
  for (const std::string_view next :
       {"diagnosticpost_gate_failure(diagnosticerror,std::size_tmaximum){",
        "diagnosticseal_failure(diagnosticerror,std::size_tmaximum){",
        "diagnosticdigest_failure(diagnosticerror,std::size_tmaximum){", "diagnosticcapture_limit("}) {
    const auto at = text.find(next, start + signature.size());
    if (at != std::string_view::npos && at < end)
      end = at;
  }
  return text.substr(start, end - start);
}
/// Restores a path's permission bits on scope exit so the temporary tree can be removed.
struct mode_guard {
  std::filesystem::path  path;
  std::filesystem::perms restore;
  ~mode_guard() {
    std::error_code ignored;
    std::filesystem::permissions(path, restore, std::filesystem::perm_options::replace, ignored);
  }
};
} // namespace
TEST_CASE("capture retains sorted bytes and discovered references", "[capture]") {
  temporary t;
  t.put("b", "two");
  t.put("nested/a", "one\r\n");
  t.put(".git/config", "metadata");
  capture_session c;
  REQUIRE(c.add_root("workspace", t.path));
  REQUIRE(c.read("workspace", "nested/a"));
  REQUIRE(c.tree("workspace"));
  auto result = c.finish();
  REQUIRE(result);
  REQUIRE((*result)->size() == 2);
  CHECK((**result)[0].path == "b");
  CHECK((**result)[1].bytes == "one\r\n");
  t.put("b", "changed");
  CHECK((**result)[0].bytes == "two");
  CHECK_FALSE(c.read("workspace", "b"));
}
TEST_CASE("capture refuses portable paths aliases and unsafe file types", "[capture]") {
  for (auto path : {"../x", "/x", "a//b", "a/./b", "a/../b", "a\\b", "a/*", ".git/config", "a/", "a/.GIT/file"})
    CHECK_FALSE(valid_path(path));
  CHECK(valid_path("a-b/C_1.txt"));
  CHECK(valid_path("a/", true));
  SECTION("symlink") {
    temporary t;
    t.put("a", "x");
    std::filesystem::create_symlink("a", t.path / "link");
    capture_session c;
    REQUIRE(c.add_root("r", t.path));
    CHECK(c.read("r", "link").error().code == "path_denied");
  }
  SECTION("directory symlink") {
    temporary t;
    t.put("a/x", "x");
    std::filesystem::create_directory_symlink("a", t.path / "link");
    capture_session c;
    REQUIRE(c.add_root("r", t.path));
    CHECK_FALSE(c.read("r", "link/x"));
  }
  SECTION("hardlink") {
    temporary t;
    t.put("a", "x");
    std::filesystem::create_hard_link(t.path / "a", t.path / "b");
    capture_session c;
    REQUIRE(c.add_root("r", t.path));
    CHECK(c.tree("r").error().code == "path_denied");
  }
  SECTION("case alias") {
    temporary t;
    t.put("A/x", "x");
    t.put("a/y", "y");
    capture_session c;
    REQUIRE(c.add_root("r", t.path));
    REQUIRE(c.read("r", "A/x"));
    auto alias = c.read("r", "a/y");
    REQUIRE_FALSE(alias);
    CHECK(alias.error().code == "path_denied");
  }
  SECTION("fifo") {
    temporary t;
    REQUIRE(::mkfifo((t.path / "fifo").c_str(), 0600) == 0);
    capture_session c;
    REQUIRE(c.add_root("r", t.path));
    CHECK(c.tree("r").error().code == "path_denied");
  }
  SECTION("git file") {
    temporary t;
    t.put(".git", "gitdir: elsewhere");
    capture_session c;
    REQUIRE(c.add_root("r", t.path));
    CHECK_FALSE(c.tree("r"));
  }
}
TEST_CASE("capture rejects overlapping roots and cross-root hardlinks", "[capture]") {
  temporary a, b;
  a.put("nested/a", "a");
  b.put("b", "b");
  capture_session c;
  REQUIRE(c.add_root("a", a.path));
  CHECK_FALSE(c.add_root("alias", a.path));
  CHECK_FALSE(c.add_root("nested", a.path / "nested"));
  REQUIRE(c.add_root("b", b.path));
  std::filesystem::create_hard_link(a.path / "nested/a", b.path / "hard");
  REQUIRE(c.tree("a"));
  CHECK_FALSE(c.tree("b"));
}
TEST_CASE("capture deterministic seam detects bytes mode identity and membership changes", "[capture]") {
  for (int mutation = 0; mutation < 5; ++mutation) {
    temporary t;
    t.put("a", "old");
    capture_session c;
    REQUIRE(c.add_root("r", t.path));
    REQUIRE(c.tree("r"));
    c.before_verification([&] {
      if (mutation == 0)
        t.put("a", "new");
      if (mutation == 1)
        t.put("b", "add");
      if (mutation == 2)
        std::filesystem::remove(t.path / "a");
      if (mutation == 3)
        ::chmod((t.path / "a").c_str(), 0755);
      if (mutation == 4) {
        std::filesystem::rename(t.path / "a", t.path / "old");
        t.put("a", "old");
      }
    });
    auto result = c.finish();
    INFO(mutation);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "capture_changed");
  }
}
TEST_CASE("capture bounds text raw files totals and exclusions", "[capture]") {
  temporary t;
  t.put("large", std::string(1048577, 'x'));
  t.put("binary", std::string("x\0y", 3));
  t.put("generated/out", "ignore");
  capture_session c;
  REQUIRE(c.add_root("r", t.path));
  auto oversized_text = c.read("r", "large");
  REQUIRE_FALSE(oversized_text);
  CHECK(oversized_text.error().code == "resource_limit");
  CHECK(oversized_text.error().limit == "file_bytes");
  CHECK(c.read("r", "binary").error().code == "schema");
  REQUIRE(c.tree("r", {"generated/"}));
  auto result = c.finish();
  REQUIRE(result);
  CHECK((*result)->size() == 2);
  temporary big;
  big.put("large", std::string(16777217, 'x'));
  capture_session b;
  REQUIRE(b.add_root("r", big.path));
  auto oversized_binary = b.tree("r");
  REQUIRE_FALSE(oversized_binary);
  CHECK(oversized_binary.error().code == "resource_limit");
  CHECK(oversized_binary.error().limit == "file_bytes");
}

TEST_CASE("capture classifies retained text growth beyond quota as changed", "[capture]") {
  temporary t;
  t.put("a", "old");
  capture_session c;
  REQUIRE(c.add_root("r", t.path));
  REQUIRE(c.read("r", "a"));
  c.before_verification([&] { std::filesystem::resize_file(t.path / "a", 1048577); });
  auto result = c.finish();
  REQUIRE_FALSE(result);
  INFO(result.error().code << '/' << result.error().limit);
  CHECK(result.error().code == "capture_changed");
}

TEST_CASE("capture classifies retained binary growth beyond quota as changed", "[capture]") {
  temporary t;
  t.put("a", std::string("x\0y", 3));
  capture_session c;
  REQUIRE(c.add_root("r", t.path));
  REQUIRE(c.tree("r"));
  c.before_verification([&] { std::filesystem::resize_file(t.path / "a", 16777217); });
  auto result = c.finish();
  REQUIRE_FALSE(result);
  INFO(result.error().code << '/' << result.error().limit);
  CHECK(result.error().code == "capture_changed");
}

TEST_CASE("capture declaration references join final retained snapshot", "[capture]") {
  temporary t;
  t.put("decl/spec.lua", "return planar.spec {goal=planar.file('text.md')}");
  t.put("decl/text.md", "exact\r\n");
  capture_session c;
  REQUIRE(c.add_root("workspace", t.path));
  auto d = c.load("decl/spec.lua");
  REQUIRE(d);
  c.before_verification([&] { t.put("decl/text.md", "changed"); });
  auto result = c.finish();
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "capture_changed");
  capture_session stable;
  REQUIRE(stable.add_root("workspace", t.path));
  REQUIRE(stable.load("decl/spec.lua"));
  auto captured = stable.finish();
  REQUIRE(captured);
  REQUIRE((*captured)->size() == 2);
  CHECK((**captured)[1].path == "decl/text.md");
  t.put("decl/bad.lua", "return planar.spec {goal=planar.file('../escape')}");
  capture_session bad;
  REQUIRE(bad.add_root("workspace", t.path));
  auto invalid = bad.load("decl/bad.lua");
  REQUIRE_FALSE(invalid);
  CHECK(invalid.error().code == "path_denied");
}
TEST_CASE("capture enforces aggregate bytes and count before retention", "[capture]") {
  SECTION("bytes") {
    temporary t;
    for (int i = 0; i < 5; ++i)
      t.put(std::to_string(i), std::string(16777216, 'x'));
    capture_session c;
    REQUIRE(c.add_root("r", t.path));
    auto result = c.tree("r");
    REQUIRE_FALSE(result);
    CHECK(result.error().limit == "retained_bytes");
  }
  SECTION("count") {
    temporary t;
    for (int i = 0; i < 10001; ++i)
      t.put(std::to_string(i), "");
    capture_session c;
    REQUIRE(c.add_root("r", t.path));
    auto result = c.tree("r");
    REQUIRE_FALSE(result);
    CHECK(result.error().limit == "files");
  }
}

TEST_CASE("capture standalone inputs share physical workspace without tree authority", "[capture]") {
  temporary t;
  t.put("registry.json", "{\"version\":1}");
  capture_session c;
  REQUIRE(c.add_root("workspace", t.path));
  REQUIRE(c.add_input("registry", t.path / "registry.json"));
  auto input = c.read("registry", "input.json");
  REQUIRE(input);
  CHECK(input->root == "registry");
  CHECK(input->path == "input.json");
  CHECK_FALSE(c.tree("registry"));
  CHECK_FALSE(c.read("registry", "registry.json"));
  auto result = c.finish();
  REQUIRE(result);
  CHECK((*result)->size() == 1);
}

TEST_CASE("capture detects empty directory membership and identity changes", "[capture]") {
  for (int mutation = 0; mutation < 3; ++mutation) {
    temporary t;
    std::filesystem::create_directories(t.path / "empty");
    capture_session c;
    REQUIRE(c.add_root("r", t.path));
    REQUIRE(c.tree("r"));
    c.before_verification([&] {
      if (mutation == 0)
        std::filesystem::create_directory(t.path / "added");
      if (mutation == 1)
        std::filesystem::remove(t.path / "empty");
      if (mutation == 2) {
        std::filesystem::rename(t.path / "empty", t.path / "old");
        std::filesystem::create_directory(t.path / "empty");
        std::filesystem::remove(t.path / "old");
      }
    });
    auto result = c.finish();
    INFO(mutation);
    CHECK_FALSE(result);
    if (!result)
      CHECK(result.error().code == "capture_changed");
  }
}

TEST_CASE("capture exposes sealed logical topology independently of file snapshots", "[capture]") {
  temporary a, b;
  a.put("z/file", "bytes");
  std::filesystem::create_directories(a.path / "a/empty");
  std::filesystem::create_directories(a.path / "excluded/empty");
  std::filesystem::create_directories(a.path / ".git/objects");
  std::filesystem::create_directories(b.path / "only");
  capture_topology saved;
  capture_snapshot files;
  {
    capture_session c;
    CHECK_FALSE(c.directories());
    REQUIRE(c.add_root("zroot", a.path));
    REQUIRE(c.add_root("aroot", b.path));
    REQUIRE(c.tree("zroot", {"excluded/"}));
    REQUIRE(c.tree("aroot"));
    c.before_verification([&] {
      a.put("excluded/new/file", "ignored");
      a.put(".git/new/file", "ignored");
      std::filesystem::remove_all(a.path / "excluded/empty");
    });
    auto result = c.finish();
    REQUIRE(result);
    files = *result;
    REQUIRE(files->size() == 1);
    CHECK((*files)[0].path == "z/file");
    auto topology = c.directories();
    REQUIRE(topology);
    saved = *topology;
    REQUIRE(saved->size() == 4);
    CHECK((*saved)[0].root == "aroot");
    CHECK((*saved)[0].path == "only");
    CHECK((*saved)[1].path == "a");
    CHECK((*saved)[2].path == "a/empty");
    CHECK((*saved)[3].path == "z");
    std::filesystem::remove_all(a.path);
    std::filesystem::remove_all(b.path);
    // The roots no longer exist: repeated getters must return the same retained object.
    REQUIRE(c.directories());
    CHECK(*c.directories() == saved);
  }
  CHECK(saved->size() == 4);
  CHECK((*files)[0].bytes == "bytes");
}

TEST_CASE("capture withholds topology after unsuccessful verification", "[capture]") {
  temporary t;
  std::filesystem::create_directory(t.path / "empty");
  capture_session c;
  REQUIRE(c.add_root("r", t.path));
  REQUIRE(c.tree("r"));
  c.before_verification([&] { std::filesystem::remove(t.path / "empty"); });
  REQUIRE_FALSE(c.finish());
  CHECK_FALSE(c.directories());
}

TEST_CASE("capture bounds name staging across trees and both passes", "[capture][enumeration]") {
  // A directory-only fixture bypasses the independent regular-file quota.
  temporary a, b;
  for (int i = 0; i < 50000; ++i) {
    std::filesystem::create_directory(a.path / std::to_string(i));
    std::filesystem::create_directory(b.path / std::to_string(i));
  }
  auto overflow = b.path / "extra";
  std::filesystem::create_directory(overflow);
  {
    capture_session c;
    REQUIRE(c.add_root("a", a.path));
    REQUIRE(c.add_root("b", b.path));
    REQUIRE(c.tree("a"));
    auto result = c.tree("b");
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "resource_limit");
    CHECK(result.error().limit == "enumeration_entries");
    CHECK(result.error().maximum == 100000);
  }
  std::filesystem::remove(overflow);
  capture_session c;
  REQUIRE(c.add_root("a", a.path));
  REQUIRE(c.add_root("b", b.path));
  REQUIRE(c.tree("a"));
  REQUIRE(c.tree("b"));
  // Excluded names still consume staging budget; their addition changes no retained topology.
  // To exclude the one extra entry, use a fresh session with that declared boundary.
  capture_session verified;
  REQUIRE(verified.add_root("a", a.path));
  REQUIRE(verified.add_root("b", b.path));
  REQUIRE(verified.tree("a"));
  REQUIRE(verified.tree("b", {"extra/"}));
  REQUIRE(c.finish());
  verified.before_verification([&] { std::filesystem::create_directory(overflow); });
  auto result = verified.finish();
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "resource_limit");
  CHECK(result.error().limit == "enumeration_entries");
  CHECK(result.error().maximum == 100000);
}

TEST_CASE("capture bounds path metadata before staging on both passes", "[capture][enumeration]") {
  temporary         t;
  const std::string parent    = std::string(128, 'p') + "/" + std::string(128, 'q');
  const auto        directory = t.path / parent;
  std::filesystem::create_directories(directory);
  // Two 128-byte parent components charge 1544 + 2318. Children have 128-byte
  // basenames, 386-byte relative paths, and charge 512 + 6*387 + 2*129 = 3092.
  constexpr std::size_t maximum = 67108864, parent_charge = 3862, child_charge = 3092;
  constexpr std::size_t fits = (maximum - parent_charge) / child_charge;
  auto                  name = [](std::size_t n) {
    auto suffix = std::to_string(n);
    return std::string(128 - suffix.size(), 'n') + suffix;
  };
  for (std::size_t i = 0; i <= fits; ++i)
    std::filesystem::create_directory(directory / name(i));
  {
    capture_session c;
    REQUIRE(c.add_root("r", t.path));
    auto result = c.tree("r");
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "resource_limit");
    CHECK(result.error().limit == "enumeration_bytes");
    CHECK(result.error().maximum == maximum);
  }
  auto extra = directory / name(fits);
  std::filesystem::remove(extra);
  capture_session c;
  REQUIRE(c.add_root("r", t.path));
  REQUIRE(c.tree("r", {parent + "/" + name(fits) + "/"}));
  REQUIRE(c.finish());
  capture_session verified;
  REQUIRE(verified.add_root("r", t.path));
  REQUIRE(verified.tree("r", {parent + "/" + name(fits) + "/"}));
  verified.before_verification([&] { std::filesystem::create_directory(extra); });
  auto result = verified.finish();
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "resource_limit");
  CHECK(result.error().limit == "enumeration_bytes");
  CHECK(result.error().maximum == maximum);
}

TEST_CASE("capture empty directories leave file provenance unchanged", "[capture]") {
  temporary t;
  t.put("file", "exact bytes");
  capture_session first;
  REQUIRE(first.add_root("r", t.path));
  REQUIRE(first.tree("r"));
  auto before = first.finish();
  REQUIRE(before);
  std::filesystem::create_directories(t.path / "empty/nested");
  capture_session second;
  REQUIRE(second.add_root("r", t.path));
  REQUIRE(second.tree("r"));
  auto after = second.finish();
  REQUIRE(after);
  REQUIRE((*before)->size() == 1);
  REQUIRE((*after)->size() == 1);
  CHECK((**before)[0].root == (**after)[0].root);
  CHECK((**before)[0].path == (**after)[0].path);
  CHECK((**before)[0].mode == (**after)[0].mode);
  CHECK((**before)[0].bytes == (**after)[0].bytes);
  REQUIRE(first.directories());
  REQUIRE(second.directories());
  CHECK((*first.directories())->empty());
  CHECK((*second.directories())->size() == 2);
}

static_assert(!std::is_default_constructible_v<acceptance_capture>);
static_assert(!std::is_copy_constructible_v<acceptance_capture>);
static_assert(!std::is_copy_assignable_v<acceptance_capture>);
static_assert(std::is_nothrow_move_constructible_v<acceptance_capture>);
static_assert(std::is_nothrow_move_assignable_v<acceptance_capture>);

TEST_CASE("capture acceptance detects changes after sealing without changing published values", "[capture][acceptance]") {
  for (auto mutation : {"bytes", "mode", "identity", "file-added", "file-removed", "empty-added", "empty-removed",
                        "directory-identity", "symlink", "directory-link", "special", "root", "root-link"}) {
    CAPTURE(mutation);
    temporary t;
    t.put("root/file", "original");
    std::filesystem::create_directories(t.path / "root/empty");
    REQUIRE(::chmod((t.path / "root/file").c_str(), 0644) == 0);
    capture_session session;
    REQUIRE(session.add_root("workspace", t.path / "root"));
    REQUIRE(session.tree("workspace"));
    auto sealed = session.finish_for_acceptance();
    REQUIRE(sealed);
    const auto& capture     = *sealed;
    auto        files       = capture.snapshot();
    auto        directories = capture.directories();
    REQUIRE(files);
    REQUIRE(directories);
    REQUIRE((*files)->size() == 1);
    REQUIRE((*directories)->size() == 1);
    REQUIRE(capture.verify());
    REQUIRE(capture.verify());
    std::string_view kind(mutation);
    if (kind == "bytes")
      t.put("root/file", "modified");
    else if (kind == "mode")
      REQUIRE(::chmod((t.path / "root/file").c_str(), 0755) == 0);
    else if (kind == "identity") {
      std::filesystem::rename(t.path / "root/file", t.path / "old-file");
      t.put("root/file", "original");
    } else if (kind == "file-added")
      t.put("root/new", "new");
    else if (kind == "file-removed")
      std::filesystem::remove(t.path / "root/file");
    else if (kind == "empty-added")
      std::filesystem::create_directory(t.path / "root/new");
    else if (kind == "empty-removed")
      std::filesystem::remove(t.path / "root/empty");
    else if (kind == "directory-identity") {
      std::filesystem::rename(t.path / "root/empty", t.path / "old-directory");
      std::filesystem::create_directory(t.path / "root/empty");
    } else if (kind == "symlink") {
      std::filesystem::remove(t.path / "root/file");
      std::filesystem::create_symlink("../old-file", t.path / "root/file");
    } else if (kind == "directory-link") {
      std::filesystem::remove(t.path / "root/empty");
      std::filesystem::create_directory_symlink("..", t.path / "root/empty");
    } else if (kind == "special")
      REQUIRE(::mkfifo((t.path / "root/pipe").c_str(), 0600) == 0);
    else {
      std::filesystem::rename(t.path / "root", t.path / "old-root");
      if (kind == "root-link")
        std::filesystem::create_directory_symlink("old-root", t.path / "root");
      else {
        t.put("root/file", "original");
        std::filesystem::create_directory(t.path / "root/empty");
      }
    }
    for (int repeat = 0; repeat != 2; ++repeat) {
      auto result = capture.verify();
      REQUIRE_FALSE(result);
      CHECK(result.error().code == "capture_changed");
      CHECK(result.error().message.find(t.path.string()) == std::string::npos);
      CHECK(*capture.snapshot() == *files);
      CHECK(*capture.directories() == *directories);
      CHECK((**files)[0].bytes == "original");
      CHECK((**files)[0].mode == "0644");
      CHECK((**directories)[0].path == "empty");
    }
  }
}

TEST_CASE("capture acceptance preserves exclusions and normalized modes", "[capture][acceptance]") {
  temporary t;
  t.put("file", "original");
  t.put("excluded/a", "before");
  t.put("ignored", "before");
  t.put(".git/config", "before");
  REQUIRE(::chmod((t.path / "file").c_str(), 0644) == 0);
  capture_session session;
  REQUIRE(session.add_root("workspace", t.path));
  REQUIRE(session.tree("workspace", {"excluded/", "ignored"}));
  auto sealed = session.finish_for_acceptance();
  REQUIRE(sealed);
  t.put("excluded/a", "after");
  t.put("excluded/new/deep", "new");
  t.put("ignored", "after");
  t.put(".git/new/config", "after");
  REQUIRE(::chmod((t.path / "file").c_str(), 0600) == 0);
  CHECK(sealed->verify());
  REQUIRE((*sealed->snapshot())->size() == 1);
  CHECK((**sealed->snapshot())[0].mode == "0644");
  // Failure does not poison receipts: a cooperative change-and-revert is outside
  // the guarantee, and another commit attempt must perform another native check.
  t.put("file", "changed");
  CHECK_FALSE(sealed->verify());
  t.put("file", "original");
  CHECK(sealed->verify());
}

TEST_CASE("capture acceptance retains standalone registry file and parent receipts", "[capture][acceptance]") {
  for (auto mutation : {"unrelated", "bytes", "mode", "identity", "link", "parent"}) {
    CAPTURE(mutation);
    temporary t;
    t.put("registry/input.json", "{}");
    REQUIRE(::chmod((t.path / "registry/input.json").c_str(), 0644) == 0);
    capture_session session;
    REQUIRE(session.add_input("registry", t.path / "registry/input.json"));
    REQUIRE(session.read("registry", "input.json"));
    auto sealed = session.finish_for_acceptance();
    REQUIRE(sealed);
    std::string_view kind(mutation);
    if (kind == "unrelated")
      t.put("registry/other/new", "unrelated parent membership");
    else if (kind == "bytes")
      t.put("registry/input.json", "[]");
    else if (kind == "mode")
      REQUIRE(::chmod((t.path / "registry/input.json").c_str(), 0755) == 0);
    else if (kind == "parent") {
      std::filesystem::rename(t.path / "registry", t.path / "old-registry");
      t.put("registry/input.json", "{}");
    } else {
      std::filesystem::rename(t.path / "registry/input.json", t.path / "old-input.json");
      if (kind == "link")
        std::filesystem::create_symlink("../old-input.json", t.path / "registry/input.json");
      else
        t.put("registry/input.json", "{}");
    }
    auto result = sealed->verify();
    if (kind == "unrelated")
      CHECK(result);
    else {
      REQUIRE_FALSE(result);
      CHECK(result.error().code == "capture_changed");
    }
    REQUIRE((*sealed->snapshot())->size() == 1);
    CHECK((**sealed->snapshot())[0].bytes == "{}");
    CHECK((*sealed->directories())->empty());
  }
}

TEST_CASE("capture acceptance owns receipts after moves and session destruction", "[capture][acceptance]") {
  temporary t;
  t.put("file", "original");
  int                               seam_calls = 0;
  std::optional<acceptance_capture> retained;
  {
    capture_session session;
    REQUIRE(session.add_root("workspace", t.path));
    REQUIRE(session.tree("workspace"));
    auto               token = std::make_shared<int>(42);
    std::weak_ptr<int> weak  = token;
    session.before_verification([token, &seam_calls] { ++seam_calls; });
    token.reset();
    auto sealed = session.finish_for_acceptance();
    REQUIRE(sealed);
    CHECK(weak.expired());
    retained.emplace(std::move(*sealed));
    CHECK_FALSE(sealed->verify());
    CHECK_FALSE(sealed->snapshot());
    CHECK_FALSE(sealed->directories());
    CHECK_FALSE(session.read("workspace", "file"));
    CHECK_FALSE(session.tree("workspace"));
    CHECK_FALSE(session.add_root("other", t.path));
    CHECK_FALSE(session.add_input("registry", t.path / "file"));
    CHECK_FALSE(session.finish());
    CHECK_FALSE(session.finish_for_acceptance());
    CHECK_FALSE(session.directories());
    session.before_verification([&seam_calls] { ++seam_calls; });
  }
  CHECK(retained->verify());
  CHECK(retained->verify());
  CHECK(seam_calls == 1);
  auto original = *retained->snapshot();
  auto moved    = std::move(*retained);
  CHECK_FALSE(retained->verify());
  *retained = std::move(moved);
  CHECK_FALSE(moved.verify());
  CHECK(retained->verify());
  // Self-move is explicitly supported, without releasing the owned roots.
  auto* same = &*retained;
  *retained  = std::move(*same);
  CHECK(retained->verify());
  CHECK(*retained->snapshot() == original);
  t.put("file", "changed");
  CHECK_FALSE(retained->verify());
  CHECK((*original)[0].bytes == "original");
}

namespace {
std::size_t root_handles(const std::filesystem::path& path) {
  struct stat root{};
  if (::stat(path.c_str(), &root))
    throw std::runtime_error("Cannot inspect test root");
  std::size_t count = 0;
  for (const auto& entry : std::filesystem::directory_iterator("/dev/fd")) {
    auto        name   = entry.path().filename().string();
    int         fd     = -1;
    auto        parsed = std::from_chars(name.data(), name.data() + name.size(), fd);
    struct stat opened{};
    if (parsed.ec == std::errc{} && parsed.ptr == name.data() + name.size() && ::fstat(fd, &opened) == 0 &&
        opened.st_dev == root.st_dev && opened.st_ino == root.st_ino)
      ++count;
  }
  return count;
}
} // namespace
TEST_CASE("capture acceptance releases root descriptors exactly once", "[capture][acceptance]") {
  temporary first, second;
  first.put("file", "first");
  second.put("file", "second");
  auto             first_before  = root_handles(first.path);
  auto             second_before = root_handles(second.path);
  capture_snapshot saved;
  {
    std::optional<acceptance_capture> held_first, held_second;
    {
      capture_session session;
      REQUIRE(session.add_root("workspace", first.path));
      REQUIRE(session.tree("workspace"));
      auto result = session.finish_for_acceptance();
      REQUIRE(result);
      CHECK(root_handles(first.path) == first_before + 1);
      held_first.emplace(std::move(*result));
    }
    CHECK(root_handles(first.path) == first_before + 1);
    {
      capture_session session;
      REQUIRE(session.add_root("workspace", second.path));
      REQUIRE(session.tree("workspace"));
      auto result = session.finish_for_acceptance();
      REQUIRE(result);
      held_second.emplace(std::move(*result));
    }
    CHECK(root_handles(second.path) == second_before + 1);
    saved        = *held_first->snapshot();
    *held_second = std::move(*held_first);
    CHECK(root_handles(first.path) == first_before + 1);
    CHECK(root_handles(second.path) == second_before);
    CHECK_FALSE(held_first->verify());
    held_first.reset();
    CHECK(root_handles(first.path) == first_before + 1);
    for (int attempt = 0; attempt != 4; ++attempt) {
      CHECK(held_second->verify());
      CHECK(root_handles(first.path) == first_before + 1);
      first.put("file", "changed");
      CHECK_FALSE(held_second->verify());
      CHECK(root_handles(first.path) == first_before + 1);
      first.put("file", "first");
    }
  }
  CHECK(root_handles(first.path) == first_before);
  CHECK(root_handles(second.path) == second_before);
  REQUIRE(saved->size() == 1);
  CHECK((*saved)[0].bytes == "first");
  // An unsuccessful initial seal also owns and releases its original root.
  {
    capture_session session;
    REQUIRE(session.add_root("workspace", second.path));
    REQUIRE(session.tree("workspace"));
    session.before_verification([&] { second.put("new", "membership change"); });
    auto result = session.finish_for_acceptance();
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "capture_changed");
    CHECK_FALSE(session.directories());
    CHECK(root_handles(second.path) == second_before + 1);
  }
  CHECK(root_handles(second.path) == second_before);
}
TEST_CASE("D2 B2 original root authority loads real declarations and text references", "[capture][d2-b2]") {
  temporary t;
  t.put("trusted/profile", "trusted");
  t.put("workspace/decl/spec.lua", "return planar.spec {goal=planar.file('text.md')}");
  t.put("workspace/decl/text.md", "exact reference\r\n");
  capture_session session;
  {
    auto admission = capture_admission::create((t.path / "trusted").string());
    REQUIRE(admission);
    REQUIRE(admission->admit_root("workspace", (t.path / "workspace").string()));
    auto root = admission->retain_root("workspace");
    REQUIRE(root);
    REQUIRE(session.add_root("workspace", (t.path / "workspace").string(), std::move(*root)));
  }
  REQUIRE(session.load("decl/spec.lua"));
  auto seal = session.finish_for_acceptance();
  REQUIRE(seal);
  REQUIRE(seal->verify());
  REQUIRE((*seal->snapshot())->size() == 2);
  t.put("workspace/decl/text.md", "changed reference");
  CHECK_FALSE(seal->verify());
}
// Mutant: capture.cpp capture_session::load calls load_declaration without the sink (or the native session lacks the budget).
TEST_CASE("X06 D C01 capture session threads the factory budget through declarations and reads", "[capture][x06d]") {
  temporary         t;
  const std::string reference(2000, 'r');
  t.put("t.md", reference);
  const std::string declaration_source = "for i=1,5000 do end\nreturn planar.spec {version=1, ref=planar.file('t.md')}";
  t.put("d.lua", declaration_source);
  auto            sink = std::make_shared<counting_sink>();
  capture_session c(sink);
  REQUIRE(c.add_root("workspace", t.path));
  REQUIRE(c.load("d.lua"));
  // Native reads alone account for the declaration and its reference bytes.
  CHECK(sink->native >= reference.size() + declaration_source.size());
  // The 5000-iteration loop executes inside the declaration VM and reaches the same sink.
  CHECK(sink->instructions >= 4000);
}
TEST_CASE("RA01-B2 capture_diagnostic refuses a file changed before verification", "[capture][ra01]") {
  // Mutants: capture_diagnostic never calls session.finish() (no recheck: every replacement case is accepted);
  // the file is opened without no-follow (the symlink case reads the target); regular-file check dropped (FIFO case).
  // C01-14: the symlink and FIFO sections assert the Messages pairs of authoring-contract.md:492-494 (symlink ->
  // path_denied / Explicit input file is a symbolic link; FIFO -> io / Explicit input must be a regular file); restoring
  // the old pair (path_denied / Only regular files can be captured in capture.cpp:89 or :96) fails both sections.
  SECTION("unchanged control reads bytes, size and digest") {
    temporary t;
    t.put("diag.json", "{\"code\":\"schema\"}\n");
    capture_session c;
    int             hook = 0;
    c.before_verification([&] { ++hook; });
    const auto result = capture_diagnostic(c, (t.path / "diag.json").string(), 65536);
    REQUIRE(result);
    CHECK(hook == 1);
    CHECK(result->bytes == "{\"code\":\"schema\"}\n");
    CHECK(result->size == 18);
    CHECK(result->digest.starts_with("xxh3_128:"));
    CHECK(result->digest.size() == 9 + 32);
  }
  SECTION("bytes replaced in place") {
    temporary t;
    t.put("diag.json", "old");
    capture_session c;
    c.before_verification([&] { t.put("diag.json", "new"); });
    const auto result = capture_diagnostic(c, (t.path / "diag.json").string(), 65536);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "capture_changed");
    CHECK(result.error().message == "File changed during read");
  }
  SECTION("file replaced by a new inode") {
    temporary t;
    t.put("diag.json", "old");
    capture_session c;
    c.before_verification([&] {
      t.put("other.json", "old");
      std::filesystem::rename(t.path / "other.json", t.path / "diag.json");
    });
    const auto result = capture_diagnostic(c, (t.path / "diag.json").string(), 65536);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "capture_changed");
  }
  SECTION("file removed") {
    temporary t;
    t.put("diag.json", "old");
    capture_session c;
    c.before_verification([&] { std::filesystem::remove(t.path / "diag.json"); });
    const auto result = capture_diagnostic(c, (t.path / "diag.json").string(), 65536);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "capture_changed");
  }
  SECTION("symlink leaf is refused and not followed") {
    temporary t;
    t.put("target.json", "secret");
    std::filesystem::create_symlink("target.json", t.path / "diag.json");
    capture_session c;
    const auto      result = capture_diagnostic(c, (t.path / "diag.json").string(), 65536);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "path_denied");
    CHECK(result.error().message == "Explicit input file is a symbolic link");
  }
  SECTION("special file is refused without blocking") {
    temporary t;
    REQUIRE(::mkfifo((t.path / "diag.json").c_str(), 0600) == 0);
    capture_session c;
    const auto      result = capture_diagnostic(c, (t.path / "diag.json").string(), 65536);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "io");
    CHECK(result.error().message == "Explicit input must be a regular file");
  }
}

TEST_CASE("RA01-B2 capture_diagnostic bounds the file at 65536 bytes from the descriptor", "[capture][ra01]") {
  // Mutants: comparison against 65537 or >= (the boundary pair flips); size read before the check is not observable
  // here; the grown-by-hook case is refused by the recheck, not by the size gate.
  SECTION("exactly 65536 bytes is accepted") {
    temporary t;
    t.put("diag.json", std::string(65536, 'x'));
    capture_session c;
    const auto      result = capture_diagnostic(c, (t.path / "diag.json").string(), 65536);
    REQUIRE(result);
    CHECK(result->size == 65536);
    CHECK(result->bytes.size() == 65536);
  }
  SECTION("65537 bytes is refused from the descriptor size") {
    temporary t;
    t.put("diag.json", std::string(65537, 'x'));
    capture_session c;
    const auto      result = capture_diagnostic(c, (t.path / "diag.json").string(), 65536);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "resource_limit");
    CHECK(result.error().limit == "diagnostic_bytes");
    CHECK(result.error().maximum == 65536);
  }
  SECTION("the limit is the caller's maximum") {
    temporary t;
    t.put("diag.json", std::string(11, 'x'));
    capture_session c;
    const auto      result = capture_diagnostic(c, (t.path / "diag.json").string(), 10);
    REQUIRE_FALSE(result);
    CHECK(result.error().limit == "diagnostic_bytes");
    CHECK(result.error().maximum == 10);
  }
  SECTION("a file grown by the verification hook is refused by the recheck") {
    temporary t;
    t.put("diag.json", std::string(65536, 'x'));
    capture_session c;
    c.before_verification([&] { t.put("diag.json", std::string(65537, 'x')); });
    const auto result = capture_diagnostic(c, (t.path / "diag.json").string(), 65536);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "capture_changed");
  }
}

TEST_CASE("RA01-B2 C01 capture_diagnostic refuses a symlink as a symbolic link", "[capture][ra01]") {
  // Kills C01-09: dropping O_NOFOLLOW from the gate open. Without it the dangling link gives io / Cannot resolve explicit
  // input path, the 70000-byte link gives resource_limit diagnostic_bytes, and the small-file link passes the gate and then
  // the native read's fstatat(AT_SYMLINK_NOFOLLOW) fails path_denied, which post_gate_failure remaps to capture_changed /
  // File changed during read (not success); each outcome differs from the required pair. Kills C01-10: the ELOOP branch
  // restored to path_denied / Only regular files can be captured.
  SECTION("dangling symlink") {
    temporary t;
    std::filesystem::create_symlink("missing.json", t.path / "diag.json");
    capture_session c;
    const auto      result = capture_diagnostic(c, (t.path / "diag.json").string(), 65536);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "path_denied");
    CHECK(result.error().message == "Explicit input file is a symbolic link");
  }
  SECTION("symlink to a 70000-byte regular file") {
    temporary t;
    t.put("target.json", std::string(70000, 'x'));
    std::filesystem::create_symlink("target.json", t.path / "diag.json");
    capture_session c;
    const auto      result = capture_diagnostic(c, (t.path / "diag.json").string(), 65536);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "path_denied");
    CHECK(result.error().message == "Explicit input file is a symbolic link");
  }
  SECTION("symlink to a small regular file") {
    temporary t;
    t.put("target.json", "{}");
    std::filesystem::create_symlink("target.json", t.path / "diag.json");
    capture_session c;
    const auto      result = capture_diagnostic(c, (t.path / "diag.json").string(), 65536);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "path_denied");
    CHECK(result.error().message == "Explicit input file is a symbolic link");
  }
}

TEST_CASE("RA01-B2 C01 capture_diagnostic refuses a non-regular file as io", "[capture][ra01]") {
  // Kills C01-11: restoring path_denied / Only regular files can be captured at capture.cpp:96, or deleting the S_ISREG
  // refusal. The FIFO is opened O_NONBLOCK by the gate, so this case cannot block; the directory is opened read-only and
  // refused by the same check.
  SECTION("FIFO") {
    temporary t;
    REQUIRE(::mkfifo((t.path / "diag.json").c_str(), 0600) == 0);
    capture_session c;
    const auto      result = capture_diagnostic(c, (t.path / "diag.json").string(), 65536);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "io");
    CHECK(result.error().message == "Explicit input must be a regular file");
  }
  SECTION("directory") {
    temporary t;
    std::filesystem::create_directory(t.path / "diag.json");
    capture_session c;
    const auto      result = capture_diagnostic(c, (t.path / "diag.json").string(), 65536);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "io");
    CHECK(result.error().message == "Explicit input must be a regular file");
  }
}

TEST_CASE("RA01-B2 C01 capture_diagnostic maps a failed open to cannot resolve", "[capture][ra01]") {
  // Kills C01-12: restoring io / Cannot inspect confined input at capture.cpp:90 for a failed open that is not ELOOP.
  // The unreadable section is root-aware: as root, mode-000 files still open, so it records SUCCEED.
  SECTION("missing path") {
    temporary       t;
    capture_session c;
    const auto      result = capture_diagnostic(c, (t.path / "missing.json").string(), 65536);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "io");
    CHECK(result.error().message == "Cannot resolve explicit input path");
  }
  SECTION("unreadable file") {
    if (::geteuid() == 0) {
      SUCCEED("permission denial is not observable as root");
    } else {
      temporary t;
      t.put("diag.json", "{}");
      mode_guard restore{t.path / "diag.json", std::filesystem::perms::owner_read | std::filesystem::perms::owner_write};
      std::filesystem::permissions(t.path / "diag.json", std::filesystem::perms::none, std::filesystem::perm_options::replace);
      capture_session c;
      const auto      result = capture_diagnostic(c, (t.path / "diag.json").string(), 65536);
      REQUIRE_FALSE(result);
      CHECK(result.error().code == "io");
      CHECK(result.error().message == "Cannot resolve explicit input path");
    }
  }
}

TEST_CASE("RA01-B2 C01 capture_diagnostic maps a post-gate add_input failure to file changed", "[capture][ra01]") {
  // Kills C01-34: the add_input branch returning std::move(added.error()) unmapped. The gate path sub/../diag.json passes
  // the gate (sub exists and the last component is a regular file), then native add_input refuses path_denied /
  // Parent root traversal forbidden, which the remap must turn into capture_changed / File changed during read.
  // Deterministic and seam-free: no hook between the gate and add_input is needed.
  temporary t;
  t.put("diag.json", "{\"code\":\"schema\"}\n");
  std::filesystem::create_directories(t.path / "sub");
  capture_session c;
  const auto      result = capture_diagnostic(c, (t.path / "sub" / ".." / "diag.json").string(), 65536);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "capture_changed");
  CHECK(result.error().message == "File changed during read");
}

TEST_CASE("RA01-B2 C01 capture_diagnostic bad_alloc refusal carries diagnostic_bytes", "[capture][ra01]") {
  // Kills C01-13: the outer bad_alloc catch returning the two-argument diagnostic again (no limit name, no maximum).
  // SOURCE GUARD with a stated limit: no allocator fault hook and no seam exists, and none may be added, so the handler is
  // pinned by its anchored spelling and by the absence of the bare two-argument pair.
  const auto text = squash_space(read_source(std::filesystem::path(__FILE__).parent_path() / "capture.cpp"));
  REQUIRE_FALSE(text.empty());
  CHECK(occurrences(text, "catch(conststd::bad_alloc&){returnstd::unexpected(capture_limit(\"diagnostic_bytes\",maximum));}") ==
        1);
  CHECK_FALSE(has_text(text, "diagnostic{\"resource_limit\",\"Capturelimitexceeded\"}"));
}

TEST_CASE("RA01-B2 C01 capture_diagnostic post-gate failures are remapped at every native call", "[capture][ra01]") {
  // Kills C01-35 (the add_input, read and finish remaps deleted or the finish pass-through), C01-36 (the detail-less
  // resource_limit rule deleted from post_gate_failure or seal_failure) and C01-37 (the file_digest allocation remap
  // deleted). The digest_failure assertions also kill C02-01 (the Canonical allocation alternative dropped), C02-03 (the
  // || turned into &&, or a wrong limit name or maximum in the return) and C02-35 (a separate canonical_bytes branch
  // before the Hash branch). SOURCE GUARD with a stated limit: no allocator fault seam exists, and no seam exists between
  // the gate and add_input or read, so those failures are pinned by source spelling only.
  const auto text = squash_space(read_source(std::filesystem::path(__FILE__).parent_path() / "capture.cpp"));
  REQUIRE_FALSE(text.empty());
  SECTION("add_input site") {
    CHECK(has_text(text, "post_gate_failure(std::move(added.error()),maximum)"));
  }
  SECTION("read site") {
    CHECK(has_text(text, "post_gate_failure(std::move(read.error()),maximum)"));
  }
  SECTION("finish site") {
    CHECK(has_text(text, "seal_failure(std::move(sealed.error()),maximum)"));
  }
  SECTION("detail-less limit sites in the post_gate_failure and seal_failure bodies") {
    const auto post = helper_body(text, "diagnosticpost_gate_failure(diagnosticerror,std::size_tmaximum){");
    const auto seal = helper_body(text, "diagnosticseal_failure(diagnosticerror,std::size_tmaximum){");
    CHECK_FALSE(post.empty());
    CHECK_FALSE(seal.empty());
    for (const auto body : {post, seal}) {
      CHECK(has_text(body, "limit.empty()"));
      CHECK(has_text(body, "capture_limit(\"diagnostic_bytes\",maximum)"));
    }
  }
  SECTION("file_digest allocation site") {
    CHECK(has_text(text, "digest_failure(std::move(digest.error()),maximum)"));
    const auto digest = helper_body(text, "diagnosticdigest_failure(diagnosticerror,std::size_tmaximum){");
    CHECK_FALSE(digest.empty());
    CHECK(has_text(digest, "Hashallocationfailed"));
    CHECK(has_text(digest, "Canonicalallocationfailed"));
    CHECK(occurrences(digest, "if(error.code==\"resource_limit\"&&(error.message==\"Hashallocationfailed\"||error.message=="
                              "\"Canonicalallocationfailed\"))"
                              "returncapture_limit(\"diagnostic_bytes\",maximum);") == 1);
    CHECK(occurrences(digest, "capture_limit(") == 1);
  }
}
