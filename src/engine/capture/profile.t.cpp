// Strict native external profile capture, pre-read bounds and retained staged receipts.
#include <catch2/catch_test_macros.hpp>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.engine.capture.profile;
using namespace planar::engine;
extern "C" void          planar_admission_watch(std::uint64_t device, std::uint64_t inode);
extern "C" std::uint64_t planar_admission_read_attempts();
namespace {
struct temporary {
  std::filesystem::path path;
  temporary() {
    char pattern[] = "/tmp/planar-profile-XXXXXX";
    path           = std::filesystem::canonical(::mkdtemp(pattern));
    std::filesystem::create_directories(path / "trusted");
    std::filesystem::create_directories(path / "workspace");
    put("trusted/profile é space.json", R"({"version":1,"modules":[]})");
    put("trusted/a.lua", "return 'private module bytes'\n");
    put("trusted/b.lua", "return 2\n");
    put("workspace/workspace.lua", "return {}\n");
    put("observations.json", "{}");
  }
  ~temporary() {
    std::filesystem::remove_all(path);
  }
  void put(std::string_view name, std::string_view bytes) {
    auto file = path / name;
    std::filesystem::create_directories(file.parent_path());
    int directory = ::open(file.parent_path().c_str(), O_RDONLY | O_DIRECTORY);
    REQUIRE(directory >= 0);
    int fd = ::openat(directory, file.filename().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    REQUIRE(::close(directory) == 0);
    REQUIRE(fd >= 0);
    REQUIRE(::write(fd, bytes.data(), bytes.size()) == static_cast<ssize_t>(bytes.size()));
    REQUIRE(::close(fd) == 0);
  }
  std::string selector() const {
    return (path / "trusted/profile é space.json").string();
  }
  void profile(std::string_view bytes) {
    put("trusted/profile é space.json", bytes);
  }
  void modules(std::string_view list) {
    profile(std::format(R"({{"version":1,"modules":[{}]}})", list));
  }
  profile_capture captured() {
    auto result = profile_capture::open(selector());
    REQUIRE(result);
    return std::move(*result);
  }
  void watch(std::string_view name) {
    struct stat s{};
    REQUIRE(::stat((path / name).c_str(), &s) == 0);
    planar_admission_watch(s.st_dev, s.st_ino);
  }
};
template <class T>
void refuses(const std::expected<T, profile_error>& result, std::string_view code, std::string_view field = "profile") {
  REQUIRE_FALSE(result);
  CHECK(result.error().detail.code == code);
  CHECK(result.error().field == field);
  CHECK(result.error().detail.message.find("/tmp/") == std::string::npos);
  CHECK(result.error().detail.message.find("private") == std::string::npos);
  if (code == "schema")
    CHECK(result.error().detail.message == "Invalid external profile structure");
  if (code == "resource_limit")
    CHECK(result.error().detail.message == "External profile resource limit exceeded");
  if (code == "path")
    CHECK(result.error().detail.message == "Invalid external module path");
  if (code == "unsupported_version")
    CHECK(result.error().detail.message == "Unsupported external profile version");
  if (code == "capture_changed")
    CHECK(result.error().detail.message == "Captured external profile changed");
}
std::string long_path(std::size_t bytes) {
  std::string result;
  while (bytes > 128) {
    result += std::string(119, 'a') + '/';
    bytes -= 120;
  }
  result += std::string(bytes - 4, 'b') + ".lua";
  return result;
}
} // namespace
static_assert(!std::is_default_constructible_v<profile_capture>);
static_assert(!std::is_copy_constructible_v<profile_capture>);
TEST_CASE("profile capture empty closure retains Unicode host filename privately", "[profile][capture]") {
  temporary t;
  auto      owner    = t.captured();
  auto      snapshot = owner.snapshot();
  REQUIRE(snapshot);
  CHECK((*snapshot)->profile.path == "profile.json");
  CHECK((*snapshot)->profile.bytes == R"({"version":1,"modules":[]})");
  CHECK((*snapshot)->modules.empty());
  CHECK(owner.verify());
  CHECK(owner.verify());
}
TEST_CASE("profile capture retains exact whitespace modes and module list order", "[profile][capture]") {
  temporary         t;
  const std::string raw = " \n {\"modules\":[\"b.lua\",\"a.lua\"],\"version\":1}\t\r\n";
  t.profile(raw);
  REQUIRE(::chmod((t.path / "trusted/b.lua").c_str(), 0711) == 0);
  REQUIRE(::chmod(t.selector().c_str(), 0600) == 0);
  auto owner    = t.captured();
  auto snapshot = *owner.snapshot();
  CHECK(snapshot->profile.bytes == raw);
  CHECK(snapshot->profile.mode == "0644");
  REQUIRE(snapshot->modules.size() == 2);
  CHECK(snapshot->modules[0].path == "b.lua");
  CHECK(snapshot->modules[0].mode == "0755");
  CHECK(snapshot->modules[1].bytes == "return 'private module bytes'\n");
  CHECK(owner.verify());
}
TEST_CASE("profile capture strict schema refuses malformed and ambiguous JSON", "[profile][capture][schema]") {
  temporary t;
  for (std::string bytes : {"",
                            "[]",
                            "null",
                            "{}",
                            "true",
                            R"({"version":1})",
                            R"({"modules":[]})",
                            R"({"version":1,"modules":[],"extra":0})",
                            R"({"version":1,"version":1,"modules":[]})",
                            R"({"version":1,"\u0076ersion":1,"modules":[]})",
                            R"({"version":1,"modules":[],"modules":[]})",
                            R"({"version":1,"modules":null})",
                            R"({"version":null,"modules":[]})",
                            R"({"version":true,"modules":[]})",
                            R"({"version":"1","modules":[]})",
                            R"({"version":1.0,"modules":[]})",
                            R"({"version":1e0,"modules":[]})",
                            R"({"version":01,"modules":[]})",
                            R"({"version":+1,"modules":[]})",
                            R"({"version":1,"modules":{}})",
                            R"({"version":1,"modules":[null]})",
                            R"({"version":1,"modules":[1]})",
                            R"({"version":1,"modules":[]}x)",
                            R"({"version":1 "modules":[]})",
                            R"({"version":1,"modules":[],})",
                            R"({/* comment */"version":1,"modules":[]})",
                            R"({"version":1,"modules":["\q"]})",
                            R"({"version":1,"modules":["\ud800"]})",
                            R"({"version":1,"modules":["\u0000"]})"}) {
    INFO(bytes);
    t.profile(bytes);
    refuses(profile_capture::open(t.selector()), "schema",
            bytes == R"({"version":1,"modules":[null]})" || bytes == R"({"version":1,"modules":[1]})" ? "profile.modules[1]"
                                                                                                      : "profile");
  }
}
TEST_CASE("profile capture integer versions have no machine integer classification ceiling", "[profile][capture][schema]") {
  temporary t;
  for (auto version :
       {"0", "-0", "-1", "2147483648", "-2147483649", "9223372036854775808", "-9223372036854775809", "18446744073709551616"}) {
    t.profile(std::format(R"({{"version":{},"modules":[]}})", version));
    refuses(profile_capture::open(t.selector()), "unsupported_version");
  }
  t.profile(std::format(R"({{"version":{},"modules":[]}})", std::string(65000, '9')));
  refuses(profile_capture::open(t.selector()), "unsupported_version");
}
TEST_CASE("profile capture UTF8 NUL BOM and escaped scalar validation is strict", "[profile][capture][schema]") {
  temporary                t;
  std::vector<std::string> values = {std::string("\xef\xbb\xbf") + R"({"version":1,"modules":[]})",
                                     std::string("{\"version\":1,\"modules\":[\"") + char(0xff) + "\"]}",
                                     std::string("{\"version\":1,\"modules\":[]}") + '\0',
                                     R"({"version":1,"modules":["\ud83d\ude00.lua"]})"};
  for (std::size_t i = 0; i < values.size(); ++i) {
    t.profile(values[i]);
    refuses(profile_capture::open(t.selector()), i == 3 ? "path" : "schema", i == 3 ? "profile.modules[1]" : "profile");
  }
}
TEST_CASE("profile capture escaped keys and portable paths use decoded library strings", "[profile][capture]") {
  temporary t;
  t.profile(R"({"\u0076ersion":1,"mod\u0075les":["\u0061.lua"]})");
  auto owner = t.captured();
  CHECK((*owner.snapshot())->modules[0].path == "a.lua");
  CHECK((*owner.snapshot())->profile.bytes == R"({"\u0076ersion":1,"mod\u0075les":["\u0061.lua"]})");
}
TEST_CASE("profile capture portable suffix segment namespace and alias policy", "[profile][capture][path]") {
  temporary t;
  for (std::string list :
       {R"("../a.lua")", R"("/a.lua")", R"("a.LUA")", R"("a.lua/")", R"("a//b.lua")", R"("a/./b.lua")", R"("a/../b.lua")",
        R"(".Git/a.lua")", R"("a/.git/b.lua")", R"("planar/a.lua")", R"("PLANAR/a.lua")", R"("__planar__/a.lua")", R"("a b.lua")",
        R"("a\\b.lua")", R"("a.lua","a.lua")", R"("Dir/a.lua","dir/b.lua")", R"("A.lua","a.lua")"}) {
    INFO(list);
    t.modules(list);
    auto result = profile_capture::open(t.selector());
    REQUIRE_FALSE(result);
    CHECK(result.error().detail.code == "path");
    CHECK(result.error().field.starts_with("profile.modules["));
  }
}
TEST_CASE("profile capture validates full portable list before any module contents", "[profile][capture][syscalls]") {
  temporary t;
  t.modules(R"("a.lua","../bad.lua")");
  t.watch("trusted/a.lua");
  refuses(profile_capture::open(t.selector()), "path", "profile.modules[2]");
  CHECK(planar_admission_read_attempts() == 0);
}
TEST_CASE("profile capture selects all module metadata before any module contents", "[profile][capture][syscalls]") {
  temporary t;
  t.modules(R"("a.lua")");
  t.watch("trusted/a.lua");
  REQUIRE(profile_capture::open(t.selector()));
  REQUIRE(planar_admission_read_attempts() > 0);
  t.modules(R"("a.lua","missing.lua")");
  t.watch("trusted/a.lua");
  auto result = profile_capture::open(t.selector());
  REQUIRE_FALSE(result);
  CHECK(result.error().detail.code == "io");
  CHECK(result.error().field == "profile.modules[2]");
  CHECK(planar_admission_read_attempts() == 0);
}
TEST_CASE("profile capture selected hard link aliases refuse before first module read", "[profile][capture][syscalls]") {
  temporary t;
  t.modules(R"("a.lua","alias.lua")");
  std::filesystem::create_hard_link(t.path / "trusted/a.lua", t.path / "trusted/alias.lua");
  t.watch("trusted/a.lua");
  refuses(profile_capture::open(t.selector()), "unsafe_path", "profile.modules[2]");
  CHECK(planar_admission_read_attempts() == 0);
}
TEST_CASE("profile capture profile module hard link alias refuses", "[profile][capture][path]") {
  temporary t;
  t.modules(R"("alias.lua")");
  std::filesystem::create_hard_link(t.selector(), t.path / "trusted/alias.lua");
  refuses(profile_capture::open(t.selector()), "unsafe_path", "profile.modules[1]");
}
TEST_CASE("profile capture final intermediate and root symlinks refuse", "[profile][capture][path]") {
  temporary t;
  std::filesystem::create_symlink(t.path / "trusted/a.lua", t.path / "trusted/link.lua");
  std::filesystem::create_directory_symlink(t.path / "trusted", t.path / "trusted/link");
  for (auto list : {R"("link.lua")", R"("link/a.lua")"}) {
    t.modules(list);
    auto result = profile_capture::open(t.selector());
    REQUIRE_FALSE(result);
    CHECK(result.error().detail.code == "path_denied");
  }
  std::filesystem::create_directory_symlink(t.path / "trusted", t.path / "root-link");
  CHECK_FALSE(profile_capture::open((t.path / "root-link/profile é space.json").string()));
  std::filesystem::create_symlink(t.selector(), t.path / "profile-link");
  CHECK_FALSE(profile_capture::open((t.path / "profile-link").string()));
}
TEST_CASE("profile capture allows explicit stdlib directory and excludes unlisted siblings", "[profile][capture]") {
  temporary t;
  t.put("trusted/stdlib.d/a.lua", "return 1");
  t.modules(R"("stdlib.d/a.lua")");
  auto owner = t.captured();
  t.put("trusted/unlisted.lua", "invalid bytecode sibling");
  t.put("trusted/stdlib.d/unlisted.lua", "changed sibling");
  std::filesystem::remove(t.path / "trusted/b.lua");
  CHECK(owner.verify());
  CHECK((*owner.snapshot())->modules.size() == 1);
}
TEST_CASE("profile capture module count zero sixtyfour and sixtyfive", "[profile][capture][limits]") {
  temporary   t;
  std::string list;
  for (std::size_t i = 0; i < 65; ++i) {
    auto name = std::format("m{}.lua", i);
    t.put("trusted/" + name, "return 1");
    if (i)
      list += ',';
    list += '"' + name + '"';
    if (i == 63) {
      t.modules(list);
      auto owner = t.captured();
      CHECK((*owner.snapshot())->modules.size() == 64);
    }
  }
  t.modules(list);
  refuses(profile_capture::open(t.selector()), "resource_limit");
}
TEST_CASE("profile capture path byte and segment bounds include escaped spellings", "[profile][capture][limits]") {
  temporary t;
  auto      path = long_path(1024);
  t.put("trusted/" + path, "return 1");
  t.modules('"' + path + '"');
  CHECK(profile_capture::open(t.selector()));
  std::string escaped;
  for (char c : path)
    escaped += std::format("\\u{:04x}", static_cast<unsigned char>(c));
  t.modules('"' + escaped + '"');
  CHECK(profile_capture::open(t.selector()));
  t.modules('"' + long_path(1025) + '"');
  refuses(profile_capture::open(t.selector()), "resource_limit");
  t.modules('"' + std::string(125, 'a') + ".lua\"");
  refuses(profile_capture::open(t.selector()), "path", "profile.modules[1]");
  t.modules('"' + std::string(124, 'a') + ".lua\"");
  t.put("trusted/" + std::string(124, 'a') + ".lua", "return 1");
  CHECK(profile_capture::open(t.selector()));
}
TEST_CASE("profile capture oversized raw escaped string and key refuse bounded scratch", "[profile][capture][limits]") {
  temporary   t;
  std::string raw;
  for (int i = 0; i < 1025; ++i)
    raw += "\\u0061";
  t.modules('"' + raw + '"');
  refuses(profile_capture::open(t.selector()), "resource_limit");
  t.profile(std::format(R"({{"{}":1,"modules":[]}})", raw));
  refuses(profile_capture::open(t.selector()), "resource_limit");
}
TEST_CASE("profile capture input bound checked before observed profile read", "[profile][capture][limits][syscalls]") {
  temporary   t;
  std::string bytes = R"({"version":1,"modules":[]})";
  bytes.resize(65536, ' ');
  t.profile(bytes);
  t.watch("trusted/profile é space.json");
  CHECK(profile_capture::open(t.selector()));
  CHECK(planar_admission_read_attempts() > 0);
  bytes += ' ';
  t.profile(bytes);
  t.watch("trusted/profile é space.json");
  refuses(profile_capture::open(t.selector()), "resource_limit");
  CHECK(planar_admission_read_attempts() == 0);
}
TEST_CASE("profile capture node and depth ceilings refuse resource exhaustion", "[profile][capture][limits]") {
  temporary t;
  t.profile(R"({"version":1,"modules":[[[[[]]]]]})");
  refuses(profile_capture::open(t.selector()), "resource_limit");
  std::string list;
  for (int i = 0; i < 133; ++i) {
    if (i)
      list += ',';
    list += "false";
  }
  t.modules(list);
  refuses(profile_capture::open(t.selector()), "resource_limit");
}
TEST_CASE("profile capture source only modules reject bytecode NUL invalid UTF8", "[profile][capture]") {
  temporary t;
  t.modules(R"("a.lua")");
  for (auto bytes : {std::string("\x1bLua"), std::string("return 1\0", 9), std::string(1, char(0xff))}) {
    t.put("trusted/a.lua", bytes);
    refuses(profile_capture::open(t.selector()), "schema", "profile.modules[1]");
  }
  t.put("trusted/a.lua", "");
  CHECK(profile_capture::open(t.selector()));
}
TEST_CASE("profile capture module ceiling checked before observed module read", "[profile][capture][limits][syscalls]") {
  temporary t;
  t.modules(R"("a.lua")");
  t.put("trusted/a.lua", std::string(1048576, ' '));
  CHECK(profile_capture::open(t.selector()));
  t.put("trusted/a.lua", std::string(1048577, ' '));
  t.watch("trusted/a.lua");
  refuses(profile_capture::open(t.selector()), "resource_limit", "profile.modules[1]");
  CHECK(planar_admission_read_attempts() == 0);
}
TEST_CASE("profile capture aggregate remaining module budget precedes final read", "[profile][capture][limits][syscalls]") {
  temporary   t;
  std::string list;
  for (int i = 0; i < 8; ++i) {
    auto name = std::format("m{}.lua", i);
    t.put("trusted/" + name, std::string(1048576, ' '));
    if (i)
      list += ',';
    list += '"' + name + '"';
  }
  t.modules(list);
  CHECK(profile_capture::open(t.selector()));
  t.put("trusted/last.lua", " ");
  t.modules(list + R"(,"last.lua")");
  t.watch("trusted/last.lua");
  refuses(profile_capture::open(t.selector()), "resource_limit", "profile.modules[9]");
  CHECK(planar_admission_read_attempts() == 0);
  t.put("trusted/last.lua", "");
  CHECK(profile_capture::open(t.selector()));
}
TEST_CASE("profile capture repeated seals retain selected original bytes after failure", "[profile][capture]") {
  temporary t;
  t.modules(R"("a.lua")");
  auto owner    = t.captured();
  auto retained = *owner.snapshot();
  CHECK(owner.verify());
  t.put("trusted/a.lua", "new private source");
  refuses(owner.verify(), "capture_changed");
  refuses(owner.snapshot(), "capture_changed");
  refuses(owner.verify(), "capture_changed");
  CHECK(retained->modules[0].bytes == "return 'private module bytes'\n");
}
TEST_CASE("profile capture receipt modes include normalized equal file mode changes", "[profile][capture]") {
  temporary t;
  t.modules(R"("a.lua")");
  auto owner = t.captured();
  REQUIRE(::chmod((t.path / "trusted/a.lua").c_str(), 0600) == 0);
  refuses(owner.verify(), "capture_changed");
}
TEST_CASE("profile capture selected directory ancestor replacement refuses", "[profile][capture][path]") {
  temporary t;
  t.put("trusted/d/a.lua", "return 1");
  t.modules(R"("d/a.lua")");
  auto owner = t.captured();
  std::filesystem::rename(t.path / "trusted/d", t.path / "trusted/old");
  std::filesystem::create_directory(t.path / "trusted/d");
  std::filesystem::create_hard_link(t.path / "trusted/old/a.lua", t.path / "trusted/d/a.lua");
  refuses(owner.verify(), "capture_changed");
}
TEST_CASE("profile capture staged root file and target checks retain initial receipts", "[profile][capture]") {
  temporary t;
  t.modules(R"("a.lua")");
  t.put("trusted/registry.json", "{}");
  auto owner = t.captured();
  REQUIRE(owner.admit_root("workspace", (t.path / "workspace").string()));
  REQUIRE(owner.admit_file("registry", (t.path / "trusted/registry.json").string(), admission_file_role::registry));
  REQUIRE(owner.admit_file("observations", (t.path / "observations.json").string(), admission_file_role::observation));
  REQUIRE(owner.admit_target("export", (t.path / "new-export").string()));
  CHECK(owner.verify());
  t.put("observations.json", "new");
  refuses(owner.verify(), "capture_changed");
}
TEST_CASE("profile capture staged overlap missing and aliases fail terminally", "[profile][capture][path]") {
  temporary t;
  for (auto path : {t.path, t.path / "trusted"}) {
    auto owner = t.captured();
    refuses(owner.admit_root("workspace", path.string()), "unsafe_path");
    refuses(owner.snapshot(), "capture_changed");
  }
  auto missing = t.captured();
  auto result  = missing.admit_file("missing", (t.path / "missing").string(), admission_file_role::registry);
  REQUIRE_FALSE(result);
  CHECK(result.error().detail.code == "io");
  refuses(missing.verify(), "capture_changed");
  std::filesystem::create_hard_link(t.selector(), t.path / "alias");
  auto alias = t.captured();
  refuses(alias.admit_file("registry", (t.path / "alias").string(), admission_file_role::registry), "unsafe_path");
}
TEST_CASE("profile capture staged admission consumes no supplied file content", "[profile][capture][syscalls]") {
  temporary t;
  auto      owner = t.captured();
  t.watch("observations.json");
  int fd = ::open((t.path / "observations.json").c_str(), O_RDONLY);
  REQUIRE(fd >= 0);
  char byte;
  REQUIRE(::read(fd, &byte, 1) == 1);
  REQUIRE(::close(fd) == 0);
  CHECK(planar_admission_read_attempts() > 0);
  t.watch("observations.json");
  CHECK(owner.admit_file("observation", (t.path / "observations.json").string(), admission_file_role::observation));
  CHECK(owner.verify());
  CHECK(owner.verify());
  CHECK(planar_admission_read_attempts() == 0);
}
TEST_CASE("profile capture staged module aliases refuse without reads and poison owner", "[profile][capture][syscalls]") {
  for (auto role : {admission_file_role::registry, admission_file_role::observation}) {
    CAPTURE(role == admission_file_role::registry ? "registry" : "observation");
    temporary t;
    t.modules(R"("a.lua")");
    // Link before capture so admission tests alias policy, not a changed ctime receipt.
    std::filesystem::create_hard_link(t.path / "trusted/a.lua", t.path / "supplied.json");
    auto owner    = t.captured();
    auto retained = *owner.snapshot();
    REQUIRE(owner.verify());
    t.watch("supplied.json");
    int fd = ::open((t.path / "supplied.json").c_str(), O_RDONLY);
    REQUIRE(fd >= 0);
    char byte;
    REQUIRE(::read(fd, &byte, 1) == 1);
    REQUIRE(::close(fd) == 0);
    REQUIRE(planar_admission_read_attempts() > 0);
    t.watch("supplied.json");
    auto result = owner.admit_file("supplied", (t.path / "supplied.json").string(), role);
    refuses(result, "unsafe_path");
    CHECK(result.error().detail.message == "External profile overlaps a supplied input");
    CHECK(planar_admission_read_attempts() == 0);
    refuses(owner.verify(), "capture_changed");
    refuses(owner.snapshot(), "capture_changed");
    CHECK(planar_admission_read_attempts() == 0);
    CHECK(retained->modules[0].bytes == "return 'private module bytes'\n");
  }
}
TEST_CASE("profile capture move assignment and self move transfer retained receipts", "[profile][capture]") {
  temporary t;
  auto      owner    = t.captured();
  auto      original = *owner.snapshot();
  auto      moved    = std::move(owner);
  refuses(owner.snapshot(), "capture_changed");
  refuses(owner.verify(), "capture_changed");
  auto* same = &moved;
  moved      = std::move(*same);
  CHECK(moved.verify());
  auto replaced = t.captured();
  replaced      = std::move(moved);
  refuses(moved.snapshot(), "capture_changed");
  CHECK(*replaced.snapshot() == original);
  CHECK(replaced.verify());
}
TEST_CASE("profile bound native seam checks original descriptor before ceiling and read", "[profile][capture][syscalls]") {
  temporary t;
  auto      admission = capture_admission::create((t.path / "trusted").string());
  REQUIRE(admission);
  REQUIRE(admission->select_trusted("module", (t.path / "trusted/a.lua").string()));
  native_capture_session session;
  REQUIRE(session.add_root("modules", (t.path / "trusted").string()));
  std::filesystem::rename(t.path / "trusted/a.lua", t.path / "trusted/old.lua");
  t.put("trusted/a.lua", "replacement");
  t.watch("trusted/a.lua");
  auto result = session.read_bounded("modules", "a.lua", *admission, "module", 0, 0);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "capture_changed");
  CHECK(planar_admission_read_attempts() == 0);
}
TEST_CASE("profile bounded native sealing reread refuses growth before observed content read", "[profile][capture][syscalls]") {
  temporary t;
  auto      admission = capture_admission::create((t.path / "trusted").string());
  REQUIRE(admission);
  REQUIRE(admission->select_trusted("module", (t.path / "trusted/a.lua").string()));
  native_capture_session session;
  REQUIRE(session.add_root("modules", (t.path / "trusted").string()));
  REQUIRE(session.read_bounded("modules", "a.lua", *admission, "module", 1048576, 1048576, false));
  session.before_verification([&] {
    t.put("trusted/a.lua", std::string(1048577, ' '));
    t.watch("trusted/a.lua");
  });
  auto result = session.finish_for_acceptance();
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "capture_changed");
  CHECK(planar_admission_read_attempts() == 0);
}
TEST_CASE("profile bounded acceptance rereads preserve original charged byte ceiling", "[profile][capture][syscalls]") {
  temporary t;
  auto      admission = capture_admission::create((t.path / "trusted").string());
  REQUIRE(admission);
  REQUIRE(admission->select_trusted("module", (t.path / "trusted/a.lua").string()));
  native_capture_session session;
  REQUIRE(session.add_root("modules", (t.path / "trusted").string()));
  // Exercise upgrading an existing old-overload cached read to a bounded receipt.
  REQUIRE(session.read("modules", "a.lua", false));
  REQUIRE(session.read_bounded("modules", "a.lua", *admission, "module", 1048576, 1048576, false));
  auto sealed = session.finish_for_acceptance();
  REQUIRE(sealed);
  REQUIRE(sealed->verify());
  t.put("trusted/a.lua", std::string(1048577, ' '));
  t.watch("trusted/a.lua");
  auto result = sealed->verify();
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "capture_changed");
  CHECK(planar_admission_read_attempts() == 0);
}
TEST_CASE("profile bounded native cached read honors newly tighter caller ceilings", "[profile][capture][syscalls]") {
  temporary t;
  auto      admission = capture_admission::create((t.path / "trusted").string());
  REQUIRE(admission);
  REQUIRE(admission->select_trusted("module", (t.path / "trusted/a.lua").string()));
  native_capture_session session;
  REQUIRE(session.add_root("modules", (t.path / "trusted").string()));
  REQUIRE(session.read_bounded("modules", "a.lua", *admission, "module", 1048576, 1048576));
  t.watch("trusted/a.lua");
  auto result = session.read_bounded("modules", "a.lua", *admission, "module", 0, 0);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "resource_limit");
  CHECK(planar_admission_read_attempts() == 0);
  CHECK(session.read("modules", "a.lua"));
}
TEST_CASE("profile capture staged descriptor binding is metadata only and terminal on mismatch", "[profile][capture][syscalls]") {
  temporary t;
  auto      owner = t.captured();
  REQUIRE(owner.admit_file("observation", (t.path / "observations.json").string(), admission_file_role::observation));
  int original = ::open((t.path / "observations.json").c_str(), O_RDONLY | O_NOFOLLOW);
  REQUIRE(original >= 0);
  t.watch("observations.json");
  CHECK(owner.verify_file("observation", original));
  CHECK(owner.verify_file("observation", original));
  CHECK(planar_admission_read_attempts() == 0);
  int wrong = ::open((t.path / "workspace/workspace.lua").c_str(), O_RDONLY | O_NOFOLLOW);
  REQUIRE(wrong >= 0);
  refuses(owner.verify_file("observation", wrong), "capture_changed");
  refuses(owner.verify_file("observation", original), "capture_changed");
  refuses(owner.snapshot(), "capture_changed");
  REQUIRE(::close(original) == 0);
  REQUIRE(::close(wrong) == 0);
}
TEST_CASE("profile capture staged binding refuses replaced inode before content", "[profile][capture][syscalls]") {
  temporary t;
  auto      owner = t.captured();
  REQUIRE(owner.admit_file("observation", (t.path / "observations.json").string(), admission_file_role::observation));
  std::filesystem::rename(t.path / "observations.json", t.path / "old-observations.json");
  t.put("observations.json", "replacement");
  int fd = ::open((t.path / "observations.json").c_str(), O_RDONLY | O_NOFOLLOW);
  REQUIRE(fd >= 0);
  t.watch("observations.json");
  refuses(owner.verify_file("observation", fd), "capture_changed");
  CHECK(planar_admission_read_attempts() == 0);
  REQUIRE(::close(fd) == 0);
}
TEST_CASE("profile capture exact decoded key limit and surrogate escape boundaries", "[profile][capture][limits]") {
  temporary t;
  t.profile(std::format(R"({{"{}":1,"modules":[]}})", std::string(1024, 'k')));
  refuses(profile_capture::open(t.selector()), "schema");
  t.profile(std::format(R"({{"{}":1,"modules":[]}})", std::string(1025, 'k')));
  refuses(profile_capture::open(t.selector()), "resource_limit");
  std::string escaped;
  for (int i = 0; i < 256; ++i)
    escaped += "\\ud83d\\ude00";
  t.modules('"' + escaped + '"');
  refuses(profile_capture::open(t.selector()), "path", "profile.modules[1]");
  escaped += "\\ud83d\\ude00";
  t.modules('"' + escaped + '"');
  refuses(profile_capture::open(t.selector()), "resource_limit");
}
TEST_CASE("profile capture exact structural node ceiling before container growth", "[profile][capture][limits]") {
  temporary t;
  // root/version/modules =3,64 nested arrays =64,65 scalars =65:132.
  std::string list;
  for (int i = 0; i < 64; ++i) {
    if (i)
      list += ',';
    list += i == 63 ? "[true,true]" : "[true]";
  }
  t.modules(list);
  refuses(profile_capture::open(t.selector()), "schema", "profile.modules[1]");
  list.replace(list.size() - 1, 1, ",true]");
  t.modules(list);
  refuses(profile_capture::open(t.selector()), "resource_limit");
  t.profile(R"({"version":1,"modules":[[[]]]})");
  refuses(profile_capture::open(t.selector()), "schema", "profile.modules[1]");
}
TEST_CASE("profile capture directory modes and trusted root replacement retain receipts", "[profile][capture]") {
  temporary t;
  t.put("trusted/d/a.lua", "return 1");
  t.modules(R"("d/a.lua")");
  auto mode = t.captured();
  REQUIRE(::chmod((t.path / "trusted/d").c_str(), 0700) == 0);
  refuses(mode.verify(), "capture_changed");
  auto root = t.captured();
  std::filesystem::rename(t.path / "trusted", t.path / "old-trusted");
  std::filesystem::create_directory(t.path / "trusted");
  refuses(root.verify(), "capture_changed");
}
TEST_CASE("profile capture parent admitted slots retain original owner and traversed receipts", "[profile][capture][parent]") {
  temporary t;
  t.modules(R"("a.lua")");
  auto owner    = t.captured();
  auto snapshot = owner.snapshot();
  REQUIRE(snapshot);
  auto bytes = (*snapshot)->profile.bytes;
  REQUIRE(owner.admit_root("workspace", (t.path / "trusted/../workspace").string()));
  REQUIRE(owner.admit_file("declaration", (t.path / "trusted/../workspace/workspace.lua").string(),
                           admission_file_role::declaration));
  REQUIRE(owner.admit_file("observation", (t.path / "trusted/../observations.json").string(), admission_file_role::observation));
  REQUIRE(owner.verify());
  t.profile(R"({"version":1,"modules":[]})");
  refuses(owner.verify(), "capture_changed");
  CHECK((*snapshot)->profile.bytes == bytes);
}
TEST_CASE("profile capture exited late slot ancestor replacement poisons original owner", "[profile][capture][parent]") {
  temporary t;
  std::filesystem::create_directory(t.path / "visited");
  auto owner = t.captured();
  REQUIRE(owner.admit_root("workspace", (t.path / "visited/../workspace").string()));
  REQUIRE(owner.verify());
  std::filesystem::rename(t.path / "visited", t.path / "old-visited");
  std::filesystem::create_directory(t.path / "visited");
  refuses(owner.verify(), "capture_changed");
  refuses(owner.admit_file("later", (t.path / "observations.json").string(), admission_file_role::registry), "capture_changed");
}
TEST_CASE("profile capture binds roots through the moved original admission and preserves closure", "[capture][profile][d1]") {
  temporary t;
  auto      capture  = t.captured();
  auto      original = *capture.snapshot();
  REQUIRE(capture.admit_root("workspace", (t.path / "workspace").string()));
  auto                   moved = std::move(capture);
  native_capture_session session;
  refuses(capture.bind_root(session, "failed", (t.path / "workspace").string(), "workspace"), "capture_changed");
  int fd = ::open((t.path / "workspace").c_str(), O_RDONLY | O_DIRECTORY);
  REQUIRE(fd >= 0);
  REQUIRE(moved.verify_root("workspace", fd));
  CHECK(::close(fd) == 0);
  REQUIRE(moved.bind_root(session, "workspace", t.path.string() + "/workspace/../workspace", "workspace"));
  CHECK(*moved.snapshot() == original);
  CHECK(moved.verify());
  CHECK(session.read("workspace", "workspace.lua"));
  REQUIRE(session.finish());
}
TEST_CASE("profile capture bound root failures close owner and leave original snapshot intact", "[capture][profile][d1]") {
  for (int failure : {0, 1, 2, 3}) {
    temporary t;
    auto      capture  = t.captured();
    auto      original = *capture.snapshot();
    auto      bytes    = original->profile.bytes;
    REQUIRE(capture.admit_root("workspace", (t.path / "workspace").string()));
    native_capture_session session;
    if (failure == 0) {
      REQUIRE(session.add_root("existing", (t.path / "workspace").string()));
      refuses(capture.bind_root(session, "workspace", (t.path / "workspace").string(), "workspace"), "path_denied");
    } else if (failure == 1) {
      refuses(capture.bind_root(session, "workspace", (t.path / "workspace").string(), "unknown"), "capture_changed");
    } else if (failure == 2) {
      refuses(capture.bind_root(session, "workspace", t.path.string() + "/missing/../workspace", "workspace"), "io");
    } else {
      refuses(capture.verify_root("workspace", -1), "capture_changed");
    }
    refuses(capture.snapshot(), "capture_changed");
    refuses(capture.verify(), "capture_changed");
    refuses(capture.bind_root(session, "retry", (t.path / "workspace").string(), "workspace"), "capture_changed");
    CHECK(original->profile.bytes == bytes);
    CHECK_FALSE(session.read("workspace", "workspace.lua"));
  }
}
TEST_CASE("profile capture bound roots still verify original trusted content receipts", "[capture][profile][d1]") {
  temporary t;
  auto      capture  = t.captured();
  auto      original = *capture.snapshot();
  REQUIRE(capture.admit_root("workspace", (t.path / "workspace").string()));
  native_capture_session session;
  REQUIRE(capture.bind_root(session, "workspace", (t.path / "workspace").string(), "workspace"));
  t.profile(R"({"version":1,"modules":["a.lua"]})");
  refuses(capture.verify(), "capture_changed");
  CHECK(original->modules.empty());
  CHECK(original->profile.bytes == R"({"version":1,"modules":[]})");
}
TEST_CASE("D2 A profile forwards retained supplied reads after owner move", "[capture][profile][d2-a]") {
  temporary t;
  auto      owner    = t.captured();
  auto      original = *owner.snapshot();
  REQUIRE(owner.admit_root("workspace", (t.path / "workspace").string()));
  REQUIRE(owner.admit_file("file", (t.path / "workspace/workspace.lua").string(), admission_file_role::declaration));
  native_capture_session session;
  REQUIRE(owner.bind_root(session, "workspace", (t.path / "workspace").string(), "workspace"));
  auto moved = std::move(owner);
  CHECK_FALSE(owner.snapshot());
  REQUIRE(moved.read(session, "workspace", "workspace.lua", "file"));
  REQUIRE(moved.read_bounded(session, "workspace", "workspace.lua", "file", 64, 64));
  CHECK(*moved.snapshot() == original);
  REQUIRE(session.finish());
  CHECK(moved.verify());
}
TEST_CASE("D2 A native late poison is visible in profile snapshot without refresh", "[capture][profile][d2-a]") {
  temporary t;
  auto      owner    = t.captured();
  auto      original = *owner.snapshot();
  REQUIRE(owner.admit_file("file", (t.path / "workspace/workspace.lua").string(), admission_file_role::declaration));
  native_capture_session session;
  REQUIRE(session.add_root("workspace", (t.path / "workspace").string()));
  REQUIRE(owner.read(session, "workspace", "workspace.lua", "file"));
  t.put("workspace/workspace.lua", "changed bytes\n");
  CHECK_FALSE(session.read("workspace", "workspace.lua"));
  CHECK_FALSE(owner.snapshot());
  CHECK_FALSE(owner.verify());
  CHECK(original->modules.empty());
}
TEST_CASE("D2 A profile retained read failures are terminal and publish no file", "[capture][profile][d2-a]") {
  for (std::string slot : {"profile", "unknown", "file"}) {
    temporary t;
    auto      owner = t.captured();
    REQUIRE(owner.admit_file("file", (t.path / "workspace/workspace.lua").string(), admission_file_role::declaration));
    native_capture_session session;
    REQUIRE(session.add_root("workspace", (t.path / "workspace").string()));
    CHECK_FALSE(owner.read_bounded(session, "workspace", "workspace.lua", slot, 1, 1));
    CHECK_FALSE(owner.snapshot());
    CHECK_FALSE(owner.read(session, "workspace", "workspace.lua", "file"));
    auto seal = session.finish();
    REQUIRE(seal);
    CHECK((*seal)->empty());
  }
  temporary t;
  auto      owner = t.captured();
  REQUIRE(owner.admit_file("file", (t.path / "workspace/workspace.lua").string(), admission_file_role::declaration));
  native_capture_session session;
  REQUIRE(session.add_root("workspace", (t.path / "workspace").string()));
  REQUIRE(owner.read(session, "workspace", "workspace.lua", "file"));
  native_capture_session rejected;
  CHECK_FALSE(owner.bind_root(rejected, "workspace", (t.path / "workspace").string(), "unknown"));
  CHECK_FALSE(session.read("workspace", "workspace.lua"));
}
TEST_CASE("D2 A native retained receipt survives original profile destruction", "[capture][profile][d2-a]") {
  temporary              t;
  native_capture_session session;
  REQUIRE(session.add_root("workspace", (t.path / "workspace").string()));
  {
    auto owner = t.captured();
    REQUIRE(owner.admit_file("file", (t.path / "workspace/workspace.lua").string(), admission_file_role::declaration));
    REQUIRE(owner.read(session, "workspace", "workspace.lua", "file"));
  }
  REQUIRE(session.read("workspace", "workspace.lua"));
  auto seal = session.finish_for_acceptance();
  REQUIRE(seal);
  CHECK(seal->verify());
}
TEST_CASE("D2 B1 profile binds colocated registry without supplied parent root", "[capture][profile][d2-b1]") {
  temporary t;
  t.put("trusted/registry.json", "{}");
  auto owner = t.captured();
  REQUIRE(owner.admit_file("registry", (t.path / "trusted/registry.json").string(), admission_file_role::registry));
  native_capture_session session;
  REQUIRE(owner.bind_input(session, "registry", (t.path / "trusted/registry.json").string(), "registry"));
  REQUIRE(session.read("registry", "input.json"));
  CHECK(owner.read(session, "registry", "input.json", "registry"));
  auto sealed = session.finish_for_acceptance();
  REQUIRE(sealed);
  CHECK(sealed->verify());
  CHECK(owner.verify());
  auto observation = t.captured();
  CHECK_FALSE(
      observation.admit_file("observation", (t.path / "trusted/registry.json").string(), admission_file_role::observation));
  auto alias = t.captured();
  CHECK_FALSE(alias.admit_file("alias", t.selector(), admission_file_role::registry));
}
TEST_CASE("D2 B1 pending input survives profile moves and destruction", "[capture][profile][d2-b1]") {
  temporary              t;
  native_capture_session session;
  {
    auto owner = t.captured();
    REQUIRE(owner.admit_file("file", (t.path / "observations.json").string(), admission_file_role::observation));
    auto moved = std::move(owner);
    CHECK_FALSE(owner.bind_input(session, "bad", (t.path / "observations.json").string(), "file"));
    REQUIRE(moved.bind_input(session, "input", t.path.string() + "/workspace/../observations.json", "file"));
    CHECK(moved.snapshot());
  }
  CHECK(session.read("input", "input.json")->bytes == "{}");
  auto sealed = session.finish_for_acceptance();
  REQUIRE(sealed);
  CHECK(sealed->verify());
}
TEST_CASE("D2 B1 input binding failures close profile owner and preserve immutable closure", "[capture][profile][d2-b1]") {
  for (int route : {0, 1, 2, 3}) {
    temporary t;
    auto      owner    = t.captured();
    auto      original = *owner.snapshot();
    REQUIRE(owner.admit_file("file", (t.path / "observations.json").string(), admission_file_role::observation));
    native_capture_session session;
    if (route == 0)
      CHECK_FALSE(owner.bind_input(session, "input", (t.path / "observations.json").string(), "unknown"));
    if (route == 1)
      CHECK_FALSE(owner.bind_input(session, "input", (t.path / "workspace/workspace.lua").string(), "file"));
    if (route == 2)
      CHECK_FALSE(owner.bind_input(session, "input", t.path.string() + "/missing/../observations.json", "file"));
    if (route == 3) {
      REQUIRE(session.add_input("input", (t.path / "observations.json").string()));
      CHECK_FALSE(owner.bind_input(session, "input", (t.path / "observations.json").string(), "file"));
    }
    CHECK_FALSE(owner.snapshot());
    CHECK_FALSE(owner.verify());
    CHECK_FALSE(owner.bind_input(session, "retry", (t.path / "observations.json").string(), "file"));
    CHECK(original->modules.empty());
  }
}
TEST_CASE("D2 B1 unread native input change is visible to original profile", "[capture][profile][d2-b1]") {
  temporary t;
  auto      owner = t.captured();
  REQUIRE(owner.admit_file("file", (t.path / "observations.json").string(), admission_file_role::observation));
  native_capture_session session;
  REQUIRE(owner.bind_input(session, "input", (t.path / "observations.json").string(), "file"));
  t.put("observations.json", "changed");
  CHECK_FALSE(session.finish());
  CHECK_FALSE(owner.snapshot());
  CHECK_FALSE(owner.verify());
}
TEST_CASE("D2 B2 profile bound roots discover ordinary files and propagate poison", "[capture][profile][d2-b2]") {
  temporary t;
  auto      owner = t.captured();
  REQUIRE(owner.admit_root("root", (t.path / "workspace").string()));
  native_capture_session session;
  REQUIRE(owner.bind_root(session, "workspace", (t.path / "workspace").string(), "root"));
  REQUIRE(session.read("workspace", "workspace.lua"));
  REQUIRE(session.tree("workspace"));
  CHECK(owner.verify());
  t.put("workspace/workspace.lua", "changed original");
  CHECK_FALSE(session.read("workspace", "workspace.lua"));
  CHECK_FALSE(owner.snapshot());
}
TEST_CASE("D2 B2 empty and unread roots retain metadata through profile destruction", "[capture][profile][d2-b2]") {
  temporary              t;
  native_capture_session session;
  {
    auto owner = t.captured();
    REQUIRE(owner.admit_root("root", (t.path / "workspace").string()));
    auto token = owner.retain_root("root");
    REQUIRE(token);
    REQUIRE(session.add_root("workspace", (t.path / "workspace").string(), std::move(*token)));
  }
  REQUIRE(::chmod((t.path / "workspace").c_str(), 0700) == 0);
  CHECK_FALSE(session.finish());
}
TEST_CASE("X06 B3 B note_registration is a pure counter that never affects authority", "[capture][profile][x06b3b]") {
  temporary t;
  auto      owner    = t.captured();
  auto      original = *owner.snapshot();
  REQUIRE(owner.registration_count() == 0);
  for (std::size_t i = 1; i <= 5; ++i) {
    owner.note_registration();
    CHECK(owner.registration_count() == i);
  }
  auto after = owner.snapshot();
  REQUIRE(after);
  CHECK(*after == original);
  CHECK((*after)->profile.bytes == original->profile.bytes);
  CHECK((*after)->profile.mode == original->profile.mode);
  CHECK(owner.verify());
  REQUIRE(owner.admit_root("workspace", (t.path / "workspace").string()));
  REQUIRE(owner.admit_file("file", (t.path / "workspace/workspace.lua").string(), admission_file_role::declaration));
  owner.note_registration();
  CHECK(owner.retain_root("workspace"));
  CHECK(owner.retain_file("file"));
  CHECK(owner.verify());
  CHECK(owner.registration_count() == 6);
  // A poisoned owner still reports its count, keeps refusing, and noting changes nothing else.
  REQUIRE_FALSE(owner.admit_root("relative", "relative/path"));
  refuses(owner.snapshot(), "capture_changed");
  const auto before_failed = owner.registration_count();
  owner.note_registration();
  CHECK(owner.registration_count() == before_failed + 1);
  refuses(owner.snapshot(), "capture_changed");
  refuses(owner.verify(), "capture_changed");
}
TEST_CASE("X06 B3 B note_registration is a no-op on moved-from owners and the count moves with the capture",
          "[capture][profile][x06b3b]") {
  temporary t;
  auto      owner = t.captured();
  owner.note_registration();
  owner.note_registration();
  auto moved = std::move(owner);
  CHECK(owner.registration_count() == 0);
  owner.note_registration();
  CHECK(owner.registration_count() == 0);
  CHECK(moved.registration_count() == 2);
  moved.note_registration();
  CHECK(moved.registration_count() == 3);
  auto replaced = t.captured();
  replaced.note_registration();
  replaced = std::move(moved);
  CHECK(moved.registration_count() == 0);
  CHECK(replaced.registration_count() == 3);
  CHECK(replaced.verify());
}
