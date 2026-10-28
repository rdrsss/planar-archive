// Pure native capture receipts; no declaration loader, Lua runtime or ledger linkage.
#include <catch2/catch_test_macros.hpp>
#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.engine.capture.native;
using namespace planar::engine;
namespace {
struct temporary {
  std::filesystem::path path;
  temporary() {
    char pattern[] = "/tmp/planar-native-capture-XXXXXX";
    path           = std::filesystem::canonical(::mkdtemp(pattern));
  }
  ~temporary() {
    std::filesystem::remove_all(path);
  }
  void put(std::string name, std::string bytes) {
    auto file = path / name;
    std::filesystem::create_directories(file.parent_path());
    std::ofstream(file, std::ios::binary) << bytes;
  }
};
struct counting_sink : budget_sink {
  std::size_t                     native = 0, native_ceiling = 0; ///< Zero ceiling means none.
  std::vector<std::size_t>        native_requests;
  std::expected<void, diagnostic> charge_native(std::size_t bytes) override {
    native_requests.push_back(bytes);
    if (native_ceiling && bytes > native_ceiling - std::min(native, native_ceiling))
      return std::unexpected(
          diagnostic{"resource_limit", "Native conversion allocation exceeded", "native_bytes", native_ceiling});
    native += bytes;
    return {};
  }
  std::expected<void, diagnostic> charge_instructions(std::size_t) override {
    return {};
  }
  std::expected<void, diagnostic> acquire_lua(std::size_t) override {
    return {};
  }
  void release_lua(std::size_t) noexcept override {
  }
};
// Sink total after reading each file individually, without tree discovery.
std::size_t read_total(temporary& t, const std::vector<std::string>& paths) {
  auto                   sink = std::make_shared<counting_sink>();
  native_capture_session session;
  session.set_budget(sink);
  REQUIRE(session.add_root("ws", t.path / "ws"));
  for (auto& p : paths)
    REQUIRE(session.read("ws", p, false));
  return sink->native;
}
std::size_t enumeration_charge(std::size_t prefix, std::size_t name) {
  return 512 + 6 * (prefix + name + 1) + 2 * (name + 1);
}
} // namespace
static_assert(!std::is_default_constructible_v<acceptance_capture>);
static_assert(!std::is_copy_constructible_v<acceptance_capture>);
TEST_CASE("capture native seals workspace and registry without Lua", "[capture][native]") {
  temporary t;
  t.put("workspace/dir/file", std::string("x\0y\r\n", 5));
  t.put("workspace/excluded/file", "ignored");
  t.put("registry.json", "{}");
  std::filesystem::create_directory(t.path / "workspace/empty");
  REQUIRE(::chmod((t.path / "workspace/dir/file").c_str(), 0755) == 0);
  native_capture_session session;
  REQUIRE(session.add_root("workspace", t.path / "workspace"));
  REQUIRE(session.add_input("registry", t.path / "registry.json"));
  REQUIRE(session.read("registry", "input.json"));
  CHECK_FALSE(session.tree("registry"));
  CHECK_FALSE(session.read("registry", "registry.json"));
  REQUIRE(session.tree("workspace", {"excluded/"}));
  int calls = 0;
  session.before_verification([&] { ++calls; });
  auto receipt = session.finish_for_acceptance();
  REQUIRE(receipt);
  auto files    = *receipt->snapshot();
  auto topology = *receipt->directories();
  REQUIRE(files->size() == 2);
  CHECK((*files)[0].root == "registry");
  CHECK((*files)[1].bytes == std::string("x\0y\r\n", 5));
  CHECK((*files)[1].mode == "0755");
  REQUIRE(topology->size() == 2);
  CHECK((*topology)[0].path == "dir");
  CHECK((*topology)[1].path == "empty");
  t.put("unrelated", "allowed");
  t.put("workspace/excluded/added", "allowed");
  CHECK(receipt->verify());
  CHECK(receipt->verify());
  CHECK(calls == 1);
  CHECK_FALSE(session.read("workspace", "dir/file"));
  CHECK_FALSE(session.finish());
  CHECK_FALSE(session.finish_for_acceptance());
  t.put("workspace/dir/file", "changed");
  CHECK(receipt->verify().error().code == "capture_changed");
  CHECK(receipt->verify().error().code == "capture_changed");
  CHECK(*receipt->snapshot() == files);
  CHECK(*receipt->directories() == topology);
  CHECK((*files)[1].bytes == std::string("x\0y\r\n", 5));
}
TEST_CASE("capture native seal performs full verification before transfer", "[capture][native]") {
  temporary t;
  t.put("file", "original");
  native_capture_session session;
  REQUIRE(session.add_root("workspace", t.path));
  REQUIRE(session.tree("workspace"));
  session.before_verification([&] { t.put("added", "new membership"); });
  auto receipt = session.finish_for_acceptance();
  REQUIRE_FALSE(receipt);
  CHECK(receipt.error().code == "capture_changed");
  CHECK_FALSE(session.directories());
  CHECK_FALSE(session.finish_for_acceptance());
}
TEST_CASE("capture native registry receipt refuses replacement after seal", "[capture][native]") {
  temporary t;
  t.put("registry/input.json", "{}");
  native_capture_session session;
  REQUIRE(session.add_input("registry", t.path / "registry/input.json"));
  REQUIRE(session.read("registry", "input.json"));
  auto receipt = session.finish_for_acceptance();
  REQUIRE(receipt);
  std::filesystem::rename(t.path / "registry", t.path / "old");
  t.put("registry/input.json", "{}");
  CHECK(receipt->verify().error().code == "capture_changed");
  CHECK((**receipt->snapshot())[0].bytes == "{}");
}
TEST_CASE("capture native receipt owns state through session destruction and moves", "[capture][native]") {
  temporary t;
  t.put("file", "original");
  std::optional<acceptance_capture> held;
  {
    native_capture_session session;
    REQUIRE(session.add_root("workspace", t.path));
    REQUIRE(session.tree("workspace"));
    auto receipt = session.finish_for_acceptance();
    REQUIRE(receipt);
    held.emplace(std::move(*receipt));
    CHECK_FALSE(receipt->snapshot());
    CHECK_FALSE(receipt->directories());
    CHECK_FALSE(receipt->verify());
  }
  REQUIRE(held->verify());
  auto files = *held->snapshot();
  auto moved = std::move(*held);
  CHECK_FALSE(held->verify());
  *held = std::move(moved);
  CHECK_FALSE(moved.verify());
  auto* same = &*held;
  *held      = std::move(*same);
  CHECK(held->verify());
  CHECK(*held->snapshot() == files);
  t.put("file", "modified");
  CHECK_FALSE(held->verify());
  CHECK((*files)[0].bytes == "original");
}
TEST_CASE("capture native selected ancestor replacement preserves file inode but refuses receipt",
          "[capture][native][ancestor]") {
  temporary t;
  t.put("selected/parent/file", "original");
  native_capture_session session;
  REQUIRE(session.add_root("workspace", t.path));
  REQUIRE(session.read("workspace", "selected/parent/file"));
  auto receipt = session.finish_for_acceptance();
  REQUIRE(receipt);
  std::filesystem::rename(t.path / "selected/parent", t.path / "old");
  std::filesystem::create_directory(t.path / "selected/parent");
  std::filesystem::create_hard_link(t.path / "old/file", t.path / "selected/parent/file");
  auto verified = receipt->verify();
  REQUIRE_FALSE(verified);
  CHECK(verified.error().code == "capture_changed");
}
TEST_CASE("capture native selected ancestor changes refuse initial sealing and discovery", "[capture][native][ancestor]") {
  for (auto phase : {"finish", "acceptance", "discovery"}) {
    CAPTURE(phase);
    temporary t;
    t.put("a/b/file", "original");
    t.put("a/b/second", "second");
    native_capture_session session;
    REQUIRE(session.add_root("workspace", t.path));
    REQUIRE(session.read("workspace", "a/b/file"));
    std::filesystem::rename(t.path / "a", t.path / "old");
    std::filesystem::create_directories(t.path / "a/b");
    std::filesystem::create_hard_link(t.path / "old/b/file", t.path / "a/b/file");
    std::filesystem::create_hard_link(t.path / "old/b/second", t.path / "a/b/second");
    if (std::string_view(phase) == "discovery") {
      auto result = session.read("workspace", "a/b/second");
      REQUIRE_FALSE(result);
      CHECK(result.error().code == "capture_changed");
    } else if (std::string_view(phase) == "finish") {
      auto result = session.finish();
      REQUIRE_FALSE(result);
      CHECK(result.error().code == "capture_changed");
    } else {
      auto result = session.finish_for_acceptance();
      REQUIRE_FALSE(result);
      CHECK(result.error().code == "capture_changed");
    }
    CHECK_FALSE(session.directories());
  }
}
TEST_CASE("capture native selected closure ignores unrelated siblings and keeps topology private",
          "[capture][native][ancestor]") {
  temporary t;
  t.put("a/b/file", "original");
  t.put("a/b/second", "second");
  t.put("a/unrelated/remove", "removed");
  native_capture_session session;
  REQUIRE(session.add_root("workspace", t.path));
  REQUIRE(session.read("workspace", "a/b/file"));
  for (int i = 0; i < 100; ++i)
    REQUIRE(session.read("workspace", "a/b/file"));
  REQUIRE(session.read("workspace", "a/b/second"));
  auto receipt = session.finish_for_acceptance();
  REQUIRE(receipt);
  auto files    = *receipt->snapshot();
  auto topology = *receipt->directories();
  REQUIRE(topology->empty());
  REQUIRE(files->size() == 2);
  t.put("a/b/added", "new");
  t.put("a/unrelated/remove", "different");
  std::filesystem::rename(t.path / "a/unrelated", t.path / "a/renamed");
  std::filesystem::remove_all(t.path / "a/renamed");
  t.put("a/b/large", "");
  std::filesystem::resize_file(t.path / "a/b/large", 16777217);
  t.put("a/b/unreadable", "private");
  REQUIRE(::chmod((t.path / "a/b/unreadable").c_str(), 0000) == 0);
  std::filesystem::create_symlink("missing", t.path / "a/b/link");
  REQUIRE(::mkfifo((t.path / "a/b/pipe").c_str(), 0600) == 0);
  REQUIRE(::chmod((t.path / "a/b/file").c_str(), 0600) == 0);
  REQUIRE(::chmod((t.path / "a/b").c_str(), 0700) == 0);
  CHECK(receipt->verify());
  CHECK(receipt->verify());
  CHECK(*receipt->snapshot() == files);
  CHECK(*receipt->directories() == topology);
  auto moved = std::move(*receipt);
  CHECK(receipt->verify().error().code == "capture_changed");
  CHECK(moved.verify());
  REQUIRE(::chmod((t.path / "a/b/file").c_str(), 0700) == 0);
  CHECK(moved.verify().error().code == "capture_changed");
  REQUIRE(::chmod((t.path / "a/b/file").c_str(), 0644) == 0);
  CHECK(moved.verify());
  t.put("a/b/file", "modified");
  CHECK(moved.verify().error().code == "capture_changed");
  CHECK((*files)[0].bytes == "original");
}
TEST_CASE("capture native selected ancestor removal and links refuse repeat verification", "[capture][native][ancestor]") {
  for (bool link : {false, true}) {
    temporary t;
    t.put("a/b/file", "original");
    native_capture_session session;
    REQUIRE(session.add_root("workspace", t.path));
    REQUIRE(session.read("workspace", "a/b/file"));
    auto receipt = session.finish_for_acceptance();
    REQUIRE(receipt);
    std::filesystem::rename(t.path / "a/b", t.path / "old");
    if (link)
      std::filesystem::create_directory_symlink(t.path / "old", t.path / "a/b");
    auto result = receipt->verify();
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "capture_changed");
    CHECK(receipt->verify().error().code == "capture_changed");
    REQUIRE((**receipt->directories()).empty());
  }
}
TEST_CASE("capture native selected receipts preserve alias and file identity refusal", "[capture][native][ancestor]") {
  temporary t;
  t.put("a/b/file", "original");
  native_capture_session session;
  REQUIRE(session.add_root("workspace", t.path));
  REQUIRE(session.read("workspace", "a/b/file"));
  std::filesystem::create_hard_link(t.path / "a/b/file", t.path / "a/b/alias");
  auto alias = session.read("workspace", "a/b/alias");
  REQUIRE_FALSE(alias);
  CHECK(alias.error().code == "path_denied");
  auto casing = session.read("workspace", "A/b/other");
  REQUIRE_FALSE(casing);
  CHECK(casing.error().code == "path_denied");
  auto receipt = session.finish_for_acceptance();
  REQUIRE(receipt);
  CHECK(receipt->verify());
  std::filesystem::rename(t.path / "a/b/file", t.path / "original");
  t.put("a/b/file", "original");
  CHECK(receipt->verify().error().code == "capture_changed");
}
TEST_CASE("capture native failed selected reads do not retain ancestor receipts", "[capture][native][ancestor]") {
  temporary t;
  t.put("a/file", std::string("invalid\0text", 12));
  native_capture_session session;
  REQUIRE(session.add_root("workspace", t.path));
  auto invalid = session.read("workspace", "a/file");
  REQUIRE_FALSE(invalid);
  CHECK(invalid.error().code == "schema");
  std::filesystem::rename(t.path / "a", t.path / "old");
  t.put("a/file", "valid");
  REQUIRE(session.read("workspace", "a/file"));
  auto missing = session.read("workspace", "missing/file");
  REQUIRE_FALSE(missing);
  CHECK(missing.error().code == "io");
  std::filesystem::create_directory_symlink(t.path / "a", t.path / "link");
  auto linked = session.read("workspace", "link/file");
  REQUIRE_FALSE(linked);
  CHECK(linked.error().code == "path_denied");
  auto receipt = session.finish_for_acceptance();
  REQUIRE(receipt);
  REQUIRE((**receipt->snapshot()).size() == 1);
  CHECK((**receipt->directories()).empty());
  CHECK(receipt->verify());
}
TEST_CASE("capture native bound roots support physical parent spelling through final verification", "[capture][native][d1]") {
  temporary t;
  t.put("trusted/profile", "{}");
  t.put("workspace/file", "bytes");
  std::filesystem::create_directories(t.path / "branch/deep");
  for (auto suffix : {"/branch/deep/../../workspace", "/workspace/./", "/workspace/../workspace//", "/workspace/child/../"}) {
    std::filesystem::create_directory(t.path / "workspace/child");
    auto admission = capture_admission::create((t.path / "trusted").string());
    REQUIRE(admission);
    auto spelling = "/../../" + t.path.string() + suffix;
    REQUIRE(admission->admit_root("root", spelling));
    native_capture_session session;
    REQUIRE(session.add_root("source", spelling, *admission, "root"));
    REQUIRE(session.read("source", "file"));
    auto captured = session.finish_for_acceptance();
    REQUIRE(captured);
    CHECK(captured->verify());
    CHECK(admission->verify());
  }
  native_capture_session old;
  CHECK_FALSE(old.add_root("source", t.path.string() + "/workspace/../workspace"));
}
TEST_CASE("capture native bound roots never cancel missing link or file parents", "[capture][native][d1]") {
  temporary t;
  t.put("trusted/profile", "{}");
  t.put("workspace/file", "bytes");
  std::filesystem::create_directory_symlink(t.path / "workspace", t.path / "link");
  t.put("regular", "regular");
  for (auto suffix : {"/missing/../workspace", "/link/../workspace", "/regular/../workspace", "/regular/./../workspace"}) {
    auto admission = capture_admission::create((t.path / "trusted").string());
    REQUIRE(admission);
    REQUIRE(admission->admit_root("root", (t.path / "workspace").string()));
    native_capture_session session;
    CHECK_FALSE(session.add_root("source", t.path.string() + suffix, *admission, "root"));
    REQUIRE(session.add_root("source", (t.path / "workspace").string(), *admission, "root"));
    CHECK(session.read("source", "file"));
  }
}
TEST_CASE("capture native bound root mismatch verifies before overlap and publishes nothing", "[capture][native][d1]") {
  temporary t;
  t.put("trusted/profile", "{}");
  t.put("workspace/file", "bytes");
  t.put("other/file", "other");
  auto admission = capture_admission::create((t.path / "trusted").string());
  REQUIRE(admission);
  REQUIRE(admission->admit_root("root", (t.path / "workspace").string()));
  native_capture_session session;
  REQUIRE(session.add_root("existing", (t.path / "other").string()));
  auto refused = session.add_root("existing", (t.path / "other").string(), *admission, "root");
  REQUIRE_FALSE(refused);
  CHECK(refused.error().code == "capture_changed");
  CHECK_FALSE(session.read("candidate", "file"));
  REQUIRE(session.add_root("candidate", (t.path / "workspace").string()));
  CHECK(session.read("candidate", "file"));
  CHECK(session.read("existing", "file"));
}
TEST_CASE("capture native bound roots preserve duplicate and physical overlap policy", "[capture][native][d1]") {
  temporary t;
  t.put("trusted/profile", "{}");
  t.put("workspace/child/file", "bytes");
  auto admission = capture_admission::create((t.path / "trusted").string());
  REQUIRE(admission);
  REQUIRE(admission->admit_root("root", (t.path / "workspace").string()));
  REQUIRE(admission->admit_root("child", (t.path / "workspace/child").string()));
  native_capture_session session;
  REQUIRE(session.add_root("source", (t.path / "workspace").string(), *admission, "root"));
  auto duplicate = session.add_root("source", (t.path / "workspace").string(), *admission, "root");
  REQUIRE_FALSE(duplicate);
  CHECK(duplicate.error().code == "path_denied");
  auto nested = session.add_root("child", (t.path / "workspace/child").string(), *admission, "child");
  REQUIRE_FALSE(nested);
  CHECK(nested.error().code == "path_denied");
  CHECK(admission->verify());
  CHECK_FALSE(session.read("child", "file"));
  CHECK(session.read("source", "child/file"));
}
TEST_CASE("capture native bound roots validate absolute host text without publishing", "[capture][native][d1]") {
  temporary t;
  t.put("trusted/profile", "{}");
  t.put("workspace/file", "bytes");
  auto admission = capture_admission::create((t.path / "trusted").string());
  REQUIRE(admission);
  REQUIRE(admission->admit_root("root", (t.path / "workspace").string()));
  for (auto spelling : {std::string(""), std::string("relative"), (t.path / "workspace").string() + std::string("\0suffix", 7)}) {
    native_capture_session session;
    auto                   result = session.add_root("root", spelling, *admission, "root");
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "path_denied");
    REQUIRE(session.add_root("root", (t.path / "workspace").string(), *admission, "root"));
    CHECK(session.read("root", "file"));
  }
}
TEST_CASE("D2 A native retained read uses same selection on cache and acceptance", "[capture][native][d2-a]") {
  temporary t;
  t.put("trusted/profile", "{}");
  t.put("workspace/file", "original");
  auto owner = capture_admission::create((t.path / "trusted").string());
  REQUIRE(owner);
  REQUIRE(owner->admit_file("file", (t.path / "workspace/file").string(), admission_file_role::registry));
  native_capture_session session;
  REQUIRE(session.add_root("workspace", (t.path / "workspace").string()));
  auto token = owner->retain_file("file");
  REQUIRE(token);
  REQUIRE(session.read("workspace", "file", std::move(*token)));
  auto same = owner->retain_file("file");
  REQUIRE(same);
  REQUIRE(session.read_bounded("workspace", "file", std::move(*same), 8, 8));
  CHECK(session.read("workspace", "file")->bytes == "original");
  REQUIRE(session.tree("workspace"));
  auto sealed = session.finish_for_acceptance();
  REQUIRE(sealed);
  auto moved = std::move(*sealed);
  CHECK_FALSE(sealed->verify());
  REQUIRE(moved.verify());
  t.put("workspace/file", "changed!");
  CHECK_FALSE(moved.verify());
  CHECK_FALSE(owner->healthy());
  CHECK((*moved.snapshot())->front().bytes == "original");
}
TEST_CASE("D2 A owned caches reject borrowed authority through both overloads", "[capture][native][d2-a]") {
  for (bool bounded : {false, true}) {
    temporary t;
    t.put("trusted/profile", "{}");
    t.put("workspace/file", "original");
    auto owner = capture_admission::create((t.path / "trusted").string());
    REQUIRE(owner);
    REQUIRE(owner->admit_file("file", (t.path / "workspace/file").string(), admission_file_role::registry));
    native_capture_session session;
    REQUIRE(session.add_root("workspace", (t.path / "workspace").string()));
    auto token = owner->retain_file("file");
    REQUIRE(token);
    REQUIRE(session.read("workspace", "file", std::move(*token)));
    if (bounded)
      CHECK_FALSE(session.read_bounded("workspace", "file", *owner, "file", 8, 8));
    else
      CHECK_FALSE(session.read("workspace", "file", *owner, "file"));
    CHECK_FALSE(owner->healthy());
    CHECK_FALSE(session.finish());
  }
}
TEST_CASE("D2 A unbound caches cannot be upgraded to retained authority", "[capture][native][d2-a]") {
  temporary t;
  t.put("trusted/profile", "{}");
  t.put("workspace/file", "original");
  auto owner = capture_admission::create((t.path / "trusted").string());
  REQUIRE(owner);
  REQUIRE(owner->admit_file("file", (t.path / "workspace/file").string(), admission_file_role::registry));
  native_capture_session session;
  REQUIRE(session.add_root("workspace", (t.path / "workspace").string()));
  REQUIRE(session.read("workspace", "file"));
  auto token = owner->retain_file("file");
  REQUIRE(token);
  CHECK_FALSE(session.read("workspace", "file", std::move(*token)));
  CHECK_FALSE(owner->healthy());
}
TEST_CASE("D2 A owned cache refuses different original state or selection", "[capture][native][d2-a]") {
  for (bool other_state : {false, true}) {
    temporary t;
    t.put("trusted/profile", "{}");
    t.put("workspace/file", "original");
    auto owner = capture_admission::create((t.path / "trusted").string());
    REQUIRE(owner);
    auto other = capture_admission::create((t.path / "trusted").string());
    REQUIRE(other);
    REQUIRE(owner->admit_file("file", (t.path / "workspace/file").string(), admission_file_role::registry));
    REQUIRE(owner->admit_file("alias", (t.path / "workspace/file").string(), admission_file_role::registry));
    REQUIRE(other->admit_file("file", (t.path / "workspace/file").string(), admission_file_role::registry));
    native_capture_session session;
    REQUIRE(session.add_root("workspace", (t.path / "workspace").string()));
    auto token = owner->retain_file("file");
    REQUIRE(token);
    REQUIRE(session.read("workspace", "file", std::move(*token)));
    auto replacement = other_state ? other->retain_file("file") : owner->retain_file("alias");
    REQUIRE(replacement);
    CHECK_FALSE(session.read("workspace", "file", std::move(*replacement)));
    CHECK_FALSE(owner->healthy());
    if (other_state)
      CHECK_FALSE(other->healthy());
  }
}
TEST_CASE("D2 A changed retained file refuses every later content path", "[capture][native][d2-a]") {
  for (int route : {0, 1, 2, 3}) {
    temporary t;
    t.put("trusted/profile", "{}");
    t.put("workspace/file", "original");
    auto owner = capture_admission::create((t.path / "trusted").string());
    REQUIRE(owner);
    REQUIRE(owner->admit_file("file", (t.path / "workspace/file").string(), admission_file_role::registry));
    native_capture_session session;
    REQUIRE(session.add_root("workspace", (t.path / "workspace").string()));
    auto token = owner->retain_file("file");
    REQUIRE(token);
    REQUIRE(session.read("workspace", "file", std::move(*token)));
    auto repeat = owner->retain_file("file");
    REQUIRE(repeat);
    t.put("workspace/file", "changed!");
    if (route == 0)
      CHECK_FALSE(session.read("workspace", "file"));
    if (route == 1)
      CHECK_FALSE(session.read("workspace", "file", std::move(*repeat)));
    if (route == 2)
      CHECK_FALSE(session.tree("workspace"));
    if (route == 3)
      CHECK_FALSE(session.finish());
    CHECK_FALSE(owner->healthy());
  }
}
TEST_CASE("D2 A retained file survives admission wrapper destruction until sealing", "[capture][native][d2-a]") {
  temporary t;
  t.put("trusted/profile", "{}");
  t.put("workspace/file", "original");
  native_capture_session session;
  REQUIRE(session.add_root("workspace", (t.path / "workspace").string()));
  {
    auto owner = capture_admission::create((t.path / "trusted").string());
    REQUIRE(owner);
    REQUIRE(owner->admit_file("file", (t.path / "workspace/file").string(), admission_file_role::registry));
    auto token = owner->retain_file("file");
    REQUIRE(token);
    REQUIRE(session.read("workspace", "file", std::move(*token)));
  }
  REQUIRE(session.read("workspace", "file"));
  auto seal = session.finish_for_acceptance();
  REQUIRE(seal);
  CHECK(seal->verify());
}
TEST_CASE("D2 B1 bound standalone input moves original pending authority into first plain read", "[capture][native][d2-b1]") {
  temporary t;
  t.put("trusted/profile", "{}");
  t.put("workspace/file", "original");
  native_capture_session session;
  {
    auto owner = capture_admission::create((t.path / "trusted").string());
    REQUIRE(owner);
    REQUIRE(owner->admit_file("file", (t.path / "workspace/file").string(), admission_file_role::registry));
    auto token = owner->retain_file("file");
    REQUIRE(token);
    REQUIRE(session.add_input("input", (t.path / "workspace/file").string(), std::move(*token)));
    // Force input-root vector relocation while the original authority is pending.
    for (int i = 0; i < 20; ++i)
      REQUIRE(session.add_input(std::format("other{}", i), (t.path / "workspace/file").string()));
  }
  REQUIRE(session.read("input", "input.json"));
  CHECK(session.read("input", "input.json")->bytes == "original");
  auto sealed = session.finish_for_acceptance();
  REQUIRE(sealed);
  REQUIRE(sealed->verify());
  t.put("workspace/file", "changed!");
  CHECK_FALSE(sealed->verify());
}
TEST_CASE("D2 B1 unread bound input retains metadata authority through acceptance", "[capture][native][d2-b1]") {
  for (bool mutate_before_seal : {false, true}) {
    temporary t;
    t.put("trusted/profile", "{}");
    // Invalid text and bytes above the content limit must remain unread.
    t.put("workspace/file", std::string(1048577, '\0'));
    auto owner = capture_admission::create((t.path / "trusted").string());
    REQUIRE(owner);
    REQUIRE(owner->admit_file("file", (t.path / "workspace/file").string(), admission_file_role::registry));
    auto token = owner->retain_file("file");
    REQUIRE(token);
    native_capture_session session;
    REQUIRE(session.add_input("input", (t.path / "workspace/file").string(), std::move(*token)));
    if (mutate_before_seal) {
      t.put("workspace/file", "changed");
      CHECK_FALSE(session.finish());
      CHECK_FALSE(owner->healthy());
    } else {
      auto sealed = session.finish_for_acceptance();
      REQUIRE(sealed);
      CHECK((**sealed->snapshot()).empty());
      REQUIRE(sealed->verify());
      t.put("workspace/file", "changed");
      CHECK_FALSE(sealed->verify());
      CHECK_FALSE(owner->healthy());
    }
  }
}
TEST_CASE("D2 B1 input binding opens physical parents without cancellation", "[capture][native][d2-b1]") {
  temporary t;
  t.put("trusted/profile", "{}");
  t.put("workspace/file", "original");
  t.put("regular", "regular");
  std::filesystem::create_directories(t.path / "branch/deep");
  std::filesystem::create_directory_symlink(t.path / "workspace", t.path / "link");
  for (auto suffix : {"/branch/deep/../../workspace/file", "/workspace/./file", "/missing/../workspace/file",
                      "/link/../workspace/file", "/regular/../workspace/file"}) {
    auto owner = capture_admission::create((t.path / "trusted").string());
    REQUIRE(owner);
    REQUIRE(owner->admit_file("file", (t.path / "workspace/file").string(), admission_file_role::registry));
    auto token = owner->retain_file("file");
    REQUIRE(token);
    native_capture_session session;
    auto                   bound = session.add_input("input", t.path.string() + suffix, std::move(*token));
    if (std::string_view(suffix).starts_with("/branch") || std::string_view(suffix).starts_with("/workspace")) {
      REQUIRE(bound);
      REQUIRE(session.read("input", "input.json"));
      REQUIRE(session.finish());
    } else {
      CHECK_FALSE(bound);
      CHECK_FALSE(owner->healthy());
    }
  }
  native_capture_session legacy;
  CHECK_FALSE(legacy.add_input("input", t.path.string() + "/branch/../workspace/file"));
}
TEST_CASE("D2 B1 binding wrong selected file poisons before publication", "[capture][native][d2-b1]") {
  temporary t;
  t.put("trusted/profile", "{}");
  t.put("workspace/file", "original");
  t.put("workspace/other", "original");
  auto owner = capture_admission::create((t.path / "trusted").string());
  REQUIRE(owner);
  REQUIRE(owner->admit_file("file", (t.path / "workspace/file").string(), admission_file_role::registry));
  auto token = owner->retain_file("file");
  REQUIRE(token);
  native_capture_session session;
  auto                   result = session.add_input("input", (t.path / "workspace/other").string(), std::move(*token));
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "capture_changed");
  CHECK_FALSE(owner->healthy());
  CHECK_FALSE(session.read("input", "input.json"));
}
TEST_CASE("D2 B1 pending and captured input refuse borrowed and different retained modes", "[capture][native][d2-b1]") {
  for (bool captured : {false, true})
    for (int route : {0, 1, 2, 3}) {
      temporary t;
      t.put("trusted/profile", "{}");
      t.put("workspace/file", "original");
      auto owner = capture_admission::create((t.path / "trusted").string());
      REQUIRE(owner);
      REQUIRE(owner->admit_file("file", (t.path / "workspace/file").string(), admission_file_role::registry));
      REQUIRE(owner->admit_file("other", (t.path / "workspace/file").string(), admission_file_role::registry));
      auto alternate = capture_admission::create((t.path / "trusted").string());
      REQUIRE(alternate);
      REQUIRE(alternate->admit_file("file", (t.path / "workspace/file").string(), admission_file_role::registry));
      auto token = owner->retain_file("file");
      REQUIRE(token);
      native_capture_session session;
      REQUIRE(session.add_input("input", (t.path / "workspace/file").string(), std::move(*token)));
      if (captured)
        REQUIRE(session.read("input", "input.json"));
      if (route == 0)
        CHECK_FALSE(session.read("input", "input.json", *owner, "file"));
      if (route == 1)
        CHECK_FALSE(session.read_bounded("input", "input.json", *owner, "file", 8, 8));
      if (route >= 2) {
        auto replacement = route == 2 ? owner->retain_file("other") : alternate->retain_file("file");
        REQUIRE(replacement);
        CHECK_FALSE(session.read("input", "input.json", std::move(*replacement)));
        if (route == 3)
          CHECK_FALSE(alternate->healthy());
      }
      CHECK_FALSE(owner->healthy());
      CHECK_FALSE(session.read("input", "input.json"));
      CHECK_FALSE(session.finish());
    }
}
TEST_CASE("D2 B1 same selection retained read preserves binding before and after transfer", "[capture][native][d2-b1]") {
  temporary t;
  t.put("trusted/profile", "{}");
  t.put("workspace/file", "original");
  auto owner = capture_admission::create((t.path / "trusted").string());
  REQUIRE(owner);
  REQUIRE(owner->admit_file("file", (t.path / "workspace/file").string(), admission_file_role::registry));
  auto token = owner->retain_file("file");
  REQUIRE(token);
  native_capture_session session;
  REQUIRE(session.add_input("input", (t.path / "workspace/file").string(), std::move(*token)));
  for (int i = 0; i < 2; ++i) {
    auto repeat = owner->retain_file("file");
    REQUIRE(repeat);
    CHECK(session.read_bounded("input", "input.json", std::move(*repeat), 8, 8));
  }
  CHECK(session.read("input", "input.json"));
  CHECK(session.finish());
}
TEST_CASE("D2 B1 bound input restrictions and initial ceiling failures are terminal", "[capture][native][d2-b1]") {
  for (int route : {0, 1, 2, 3}) {
    temporary t;
    t.put("trusted/profile", "{}");
    t.put("workspace/file", "original");
    auto owner = capture_admission::create((t.path / "trusted").string());
    REQUIRE(owner);
    REQUIRE(owner->admit_file("file", (t.path / "workspace/file").string(), admission_file_role::registry));
    auto token = owner->retain_file("file");
    REQUIRE(token);
    native_capture_session session;
    REQUIRE(session.add_input("input", (t.path / "workspace/file").string(), std::move(*token)));
    if (route == 0)
      CHECK_FALSE(session.tree("input"));
    if (route == 1)
      CHECK_FALSE(session.read("input", "file"));
    if (route == 2)
      CHECK_FALSE(session.read("input", "../file"));
    if (route == 3) {
      auto repeat = owner->retain_file("file");
      REQUIRE(repeat);
      CHECK_FALSE(session.read_bounded("input", "input.json", std::move(*repeat), 7, 7));
    }
    CHECK_FALSE(owner->healthy());
    CHECK_FALSE(session.read("input", "input.json"));
    CHECK_FALSE(session.finish());
  }
}
TEST_CASE("D2 B1 refused unpublished binding cannot be revived by unbound registration", "[capture][native][d2-b1]") {
  temporary t;
  t.put("trusted/profile", "{}");
  t.put("workspace/file", "original");
  t.put("workspace/other", "other");
  auto owner = capture_admission::create((t.path / "trusted").string());
  REQUIRE(owner);
  REQUIRE(owner->admit_file("file", (t.path / "workspace/file").string(), admission_file_role::registry));
  auto token = owner->retain_file("file");
  REQUIRE(token);
  native_capture_session session;
  CHECK_FALSE(session.add_input("input", (t.path / "workspace/other").string(), std::move(*token)));
  CHECK_FALSE(owner->healthy());
  CHECK_FALSE(session.add_input("input", (t.path / "workspace/other").string()));
  CHECK_FALSE(session.add_root("root", (t.path / "workspace").string()));
  CHECK_FALSE(session.read("input", "input.json"));
  CHECK_FALSE(session.finish());
  // Never-bound unbound sessions preserve their ordinary recoverable failures.
  native_capture_session legacy;
  CHECK_FALSE(legacy.add_input("input", "relative"));
  CHECK(legacy.add_input("input", (t.path / "workspace/other").string()));
  CHECK(legacy.read("input", "input.json"));
}
TEST_CASE("D2 B2 rooted capture survives admission destruction and verifies acceptance", "[capture][native][d2-b2]") {
  temporary t;
  t.put("trusted/profile", "trusted");
  t.put("workspace/dir/file", "original");
  native_capture_session session;
  {
    auto admission = capture_admission::create((t.path / "trusted").string());
    REQUIRE(admission);
    REQUIRE(admission->admit_root("root", (t.path / "workspace").string()));
    auto root = admission->retain_root("root");
    REQUIRE(root);
    REQUIRE(session.add_root("r", (t.path / "workspace").string(), std::move(*root)));
  }
  CHECK(session.read("r", "dir/file")->bytes == "original");
  REQUIRE(session.tree("r"));
  auto acceptance = session.finish_for_acceptance();
  REQUIRE(acceptance);
  auto moved = std::move(*acceptance);
  REQUIRE(moved.verify());
  t.put("workspace/dir/file", "different");
  CHECK_FALSE(moved.verify());
}
TEST_CASE("D2 B2 rooted caches refuse mutation without recapture", "[capture][native][d2-b2]") {
  temporary t;
  t.put("trusted/profile", "trusted");
  t.put("workspace/file", "original");
  auto admission = capture_admission::create((t.path / "trusted").string());
  REQUIRE(admission);
  REQUIRE(admission->admit_root("root", (t.path / "workspace").string()));
  auto root = admission->retain_root("root");
  REQUIRE(root);
  native_capture_session session;
  REQUIRE(session.add_root("r", (t.path / "workspace").string(), std::move(*root)));
  auto original = session.read("r", "file");
  REQUIRE(original);
  t.put("workspace/file", "different");
  CHECK_FALSE(session.read("r", "file"));
  CHECK_FALSE(session.finish());
  CHECK_FALSE(admission->healthy());
  CHECK(original->bytes == "original");
}
TEST_CASE("D2 B2 final rooted tree membership refuses additions and removals", "[capture][native][d2-b2]") {
  for (bool added : {false, true}) {
    temporary t;
    t.put("trusted/profile", "trusted");
    t.put("workspace/file", "original");
    auto admission = capture_admission::create((t.path / "trusted").string());
    REQUIRE(admission);
    REQUIRE(admission->admit_root("root", (t.path / "workspace").string()));
    auto root = admission->retain_root("root");
    REQUIRE(root);
    native_capture_session session;
    REQUIRE(session.add_root("r", (t.path / "workspace").string(), std::move(*root)));
    REQUIRE(session.tree("r"));
    if (added)
      t.put("workspace/new", "new content must not be read");
    else
      std::filesystem::remove(t.path / "workspace/file");
    CHECK_FALSE(session.finish());
    CHECK_FALSE(admission->healthy());
  }
}
TEST_CASE("D2 B2 root registration failure is terminal with no unbound retry", "[capture][native][d2-b2]") {
  temporary t;
  t.put("trusted/profile", "trusted");
  std::filesystem::create_directories(t.path / "workspace");
  auto admission = capture_admission::create((t.path / "trusted").string());
  REQUIRE(admission);
  REQUIRE(admission->admit_root("root", (t.path / "workspace").string()));
  auto root = admission->retain_root("root");
  REQUIRE(root);
  native_capture_session session;
  CHECK_FALSE(session.add_root("r", (t.path / "trusted").string(), std::move(*root)));
  CHECK_FALSE(session.add_root("r", (t.path / "workspace").string()));
  CHECK_FALSE(session.finish());
  CHECK_FALSE(admission->healthy());
}
TEST_CASE("D2 B2 C01 first retained reads require original root selection", "[capture][native][d2-b2-c01]") {
  for (bool bounded : {false, true})
    for (int route : {0, 1, 2, 3}) {
      temporary t;
      t.put("trusted/profile", "trusted");
      t.put("other-trusted/profile", "other trusted");
      t.put("workspace/file", "original");
      if (route == 3)
        std::filesystem::create_hard_link(t.path / "workspace/file", t.path / "workspace/other");
      else
        t.put("workspace/other", "other");
      auto owner   = capture_admission::create((t.path / "trusted").string());
      auto foreign = capture_admission::create((t.path / "other-trusted").string());
      REQUIRE(owner);
      REQUIRE(foreign);
      REQUIRE(owner->admit_root("root", (t.path / "workspace").string()));
      auto& incoming      = route < 2 ? *foreign : *owner;
      auto  selected_path = route == 0 ? "workspace/file" : "workspace/other";
      REQUIRE(incoming.admit_file("file", (t.path / selected_path).string(), admission_file_role::declaration));
      auto token = incoming.retain_file("file");
      REQUIRE(token);
      auto root = owner->retain_root("root");
      REQUIRE(root);
      native_capture_session session;
      REQUIRE(session.add_root("r", (t.path / "workspace").string(), std::move(*root)));
      auto result = bounded ? session.read_bounded("r", "file", std::move(*token), 1024, 1024)
                            : session.read("r", "file", std::move(*token));
      CHECK_FALSE(result);
      CHECK_FALSE(owner->healthy());
      CHECK_FALSE(incoming.healthy());
      CHECK_FALSE(session.read("r", "file"));
      CHECK_FALSE(session.add_root("retry", (t.path / "workspace").string()));
      CHECK_FALSE(session.finish_for_acceptance());
    }
}
TEST_CASE("D2 B2 C01 exact original pre-cache receipt survives all owned rereads", "[capture][native][d2-b2-c01]") {
  for (bool bounded : {false, true}) {
    temporary t;
    t.put("trusted/profile", "trusted");
    t.put("workspace/file", "original");
    auto owner = capture_admission::create((t.path / "trusted").string());
    REQUIRE(owner);
    REQUIRE(owner->admit_root("root", (t.path / "workspace").string()));
    REQUIRE(owner->admit_file("file", (t.path / "workspace/file").string(), admission_file_role::declaration));
    auto token  = owner->retain_file("file");
    auto repeat = owner->retain_file("file");
    auto root   = owner->retain_root("root");
    REQUIRE(token);
    REQUIRE(repeat);
    REQUIRE(root);
    native_capture_session session;
    REQUIRE(session.add_root("r", (t.path / "workspace").string(), std::move(*root)));
    auto first =
        bounded ? session.read_bounded("r", "file", std::move(*token), 8, 8) : session.read("r", "file", std::move(*token));
    REQUIRE(first);
    CHECK(first->bytes == "original");
    CHECK(session.read_bounded("r", "file", std::move(*repeat), 8, 8));
    CHECK(session.read("r", "file"));
    REQUIRE(session.tree("r"));
    auto receipt = session.finish_for_acceptance();
    REQUIRE(receipt);
    CHECK(receipt->verify());
    CHECK(receipt->verify());
    CHECK(owner->healthy());
  }
}
TEST_CASE("D2 B2 C01 initial borrowed and cached replacement modes refuse", "[capture][native][d2-b2-c01]") {
  for (int route : {0, 1, 2, 3}) {
    temporary t;
    t.put("trusted/profile", "trusted");
    t.put("workspace/file", "original");
    auto owner = capture_admission::create((t.path / "trusted").string());
    REQUIRE(owner);
    REQUIRE(owner->admit_root("root", (t.path / "workspace").string()));
    REQUIRE(owner->admit_file("file", (t.path / "workspace/file").string(), admission_file_role::declaration));
    REQUIRE(owner->admit_file("alternate", (t.path / "workspace/file").string(), admission_file_role::declaration));
    auto root = owner->retain_root("root");
    REQUIRE(root);
    native_capture_session session;
    REQUIRE(session.add_root("r", (t.path / "workspace").string(), std::move(*root)));
    if (route >= 2) {
      auto token = owner->retain_file("file");
      REQUIRE(token);
      REQUIRE(session.read("r", "file", std::move(*token)));
    }
    if (route == 0 || route == 2)
      CHECK_FALSE(session.read("r", "file", *owner, "file"));
    else if (route == 1)
      CHECK_FALSE(session.read_bounded("r", "file", *owner, "file", 8, 8));
    else {
      auto alternate = owner->retain_file("alternate");
      REQUIRE(alternate);
      CHECK_FALSE(session.read("r", "file", std::move(*alternate)));
    }
    CHECK_FALSE(owner->healthy());
    CHECK_FALSE(session.finish());
  }
}

// X06 D C01 rows N1-N6: composed factory budget in native capture.
TEST_CASE("X06 D C01 capture charges the composed ceiling before reading or allocating a file", "[capture][native][x06dc01]") {
  temporary t;
  t.put("ws/big", std::string(2000, static_cast<char>(0xff)));
  auto sink            = std::make_shared<counting_sink>();
  sink->native_ceiling = 1000;
  native_capture_session session;
  session.set_budget(sink);
  REQUIRE(session.add_root("ws", t.path / "ws"));
  auto result = session.read("ws", "big", true);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "resource_limit");
  CHECK(result.error().maximum == 1000);
  REQUIRE(sink->native_requests.size() == 1);
  CHECK(sink->native_requests[0] >= 2000);
}
TEST_CASE("X06 D C01 native read charges each retained file exactly once", "[capture][native][x06dc01]") {
  temporary t;
  t.put("ws/f.txt", "0123456789");
  auto                   sink = std::make_shared<counting_sink>();
  native_capture_session session;
  session.set_budget(sink);
  REQUIRE(session.add_root("ws", t.path / "ws"));
  REQUIRE(session.read("ws", "f.txt", true));
  auto expected =
      10 + std::string("f.txt").size() + std::string("ws").size() + std::string("0644").size() + sizeof(captured_file);
  CHECK(sink->native == expected);
  REQUIRE(session.read("ws", "f.txt", true));
  CHECK(sink->native == expected);
}
TEST_CASE("X06 D C01 discovery staging is charged to the factory budget and never refunded", "[capture][native][x06dc01]") {
  temporary t;
  t.put("ws/a", "x");
  t.put("ws/bb", "yy");
  auto                   control = read_total(t, {"a", "bb"});
  auto                   sink    = std::make_shared<counting_sink>();
  native_capture_session session;
  session.set_budget(sink);
  REQUIRE(session.add_root("ws", t.path / "ws"));
  REQUIRE(session.tree("ws"));
  CHECK(sink->native - control == enumeration_charge(0, 1) + enumeration_charge(0, 2));
  auto after = sink->native;
  REQUIRE(session.finish());
  CHECK(sink->native >= after);
}
TEST_CASE("X06 D C01 discovery staging refuses at the composed ceiling", "[capture][native][x06dc01]") {
  temporary t;
  t.put("ws/a", "x");
  t.put("ws/bb", "yy");
  auto sink            = std::make_shared<counting_sink>();
  sink->native_ceiling = 600;
  native_capture_session session;
  session.set_budget(sink);
  REQUIRE(session.add_root("ws", t.path / "ws"));
  auto result = session.tree("ws");
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "resource_limit");
  CHECK(result.error().maximum == 600);
  CHECK_FALSE(session.read("ws", "a", false));
}
TEST_CASE("X06 D C01 verification staging is charged on every verify and never refunded", "[capture][native][x06dc01]") {
  temporary t;
  t.put("ws/a", "x");
  t.put("ws/bb", "yy");
  auto                   sink = std::make_shared<counting_sink>();
  native_capture_session session;
  session.set_budget(sink);
  REQUIRE(session.add_root("ws", t.path / "ws"));
  REQUIRE(session.tree("ws"));
  auto receipt = session.finish_for_acceptance();
  REQUIRE(receipt);
  auto before = sink->native;
  REQUIRE(receipt->verify());
  auto first = sink->native - before;
  REQUIRE(receipt->verify());
  auto second = sink->native - before - first;
  CHECK(first >= enumeration_charge(0, 1) + enumeration_charge(0, 2));
  CHECK(second == first);
}
TEST_CASE("X06 D C01 ancestor staging is charged with each retained file", "[capture][native][x06dc01]") {
  temporary t;
  t.put("ws/c.txt", "same");
  t.put("ws/a/b/c.txt", "same");
  auto flat   = read_total(t, {"c.txt"});
  auto nested = read_total(t, {"a/b/c.txt"});
  // Ancestor receipts are keyed by (root, ancestor path): 'a' and 'a/b'.
  CHECK(nested - flat >= enumeration_charge(2, 1) + enumeration_charge(2, 3));
}
// Allocation probe for the before-allocation tests below: records the largest request made while armed.
namespace {
std::atomic<bool>        probe_armed{false};
std::atomic<std::size_t> probe_largest{0};
} // namespace
void* operator new(std::size_t size) {
  if (probe_armed.load(std::memory_order_relaxed)) {
    auto seen = probe_largest.load(std::memory_order_relaxed);
    while (size > seen && !probe_largest.compare_exchange_weak(seen, size, std::memory_order_relaxed)) {
    }
  }
  if (void* memory = std::malloc(size ? size : 1))
    return memory;
  throw std::bad_alloc();
}
void operator delete(void* memory) noexcept {
  std::free(memory);
}
void operator delete(void* memory, std::size_t) noexcept {
  std::free(memory);
}
// Mutants N1b/N1c: the composed charge moves after the read loop or after the std::string allocation. A refused read of a
// 100000-byte file must not request any allocation near the file size.
TEST_CASE("X06 D C01 refused capture allocates nothing near the file size", "[capture][native][x06dc01]") {
  temporary t;
  t.put("ws/big.txt", std::string(100000, 'a'));
  auto sink            = std::make_shared<counting_sink>();
  sink->native_ceiling = 1000;
  native_capture_session session;
  session.set_budget(sink);
  REQUIRE(session.add_root("ws", t.path / "ws"));
  probe_largest = 0;
  probe_armed   = true;
  auto result   = session.read("ws", "big.txt", true);
  probe_armed   = false;
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "resource_limit");
  CHECK(result.error().maximum == 1000);
  CHECK(probe_largest.load() < 50000);
  // Control: an unrefused read of the same file does allocate the content.
  auto                   accepted = std::make_shared<counting_sink>();
  native_capture_session control;
  control.set_budget(accepted);
  REQUIRE(control.add_root("ws", t.path / "ws"));
  probe_largest = 0;
  probe_armed   = true;
  auto read     = control.read("ws", "big.txt", true);
  probe_armed   = false;
  REQUIRE(read);
  CHECK(probe_largest.load() >= 100000);
}
// Mutant N4: enumeration_budget::charge ignores the sink refusal. One entry means no later charge can mask it.
TEST_CASE("X06 D C01 a single discovery entry refuses at the composed ceiling", "[capture][native][x06dc01]") {
  temporary t;
  t.put("ws/a", "x");
  auto sink            = std::make_shared<counting_sink>();
  sink->native_ceiling = enumeration_charge(0, 1) - 1;
  native_capture_session session;
  session.set_budget(sink);
  REQUIRE(session.add_root("ws", t.path / "ws"));
  auto result = session.tree("ws");
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "resource_limit");
  CHECK(result.error().maximum == enumeration_charge(0, 1) - 1);
  CHECK(sink->native == 0);
}
// Mutants N2 (both cache guards removed) and N2d: re-reading retained files during verification adds no read charge, so one
// verify costs exactly the enumeration staging.
TEST_CASE("X06 D C01 verification charges only the enumeration staging", "[capture][native][x06dc01]") {
  temporary t;
  t.put("ws/a", "x");
  t.put("ws/bb", "yy");
  auto                   sink = std::make_shared<counting_sink>();
  native_capture_session session;
  session.set_budget(sink);
  REQUIRE(session.add_root("ws", t.path / "ws"));
  REQUIRE(session.tree("ws"));
  REQUIRE(session.read("ws", "a", false));
  REQUIRE(session.read("ws", "bb", false));
  auto receipt = session.finish_for_acceptance();
  REQUIRE(receipt);
  auto before = sink->native;
  REQUIRE(receipt->verify());
  CHECK(sink->native - before == enumeration_charge(0, 1) + enumeration_charge(0, 2));
}
