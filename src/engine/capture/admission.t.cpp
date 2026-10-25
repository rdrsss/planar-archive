// Metadata-only admission and binding; no profile parser, Lua or CLI phase claims.
#include <catch2/catch_test_macros.hpp>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.engine.capture.admission;
import planar.engine.capture.native;
using namespace planar::engine;
namespace {
struct temporary {
  std::filesystem::path path;
  temporary() {
    char pattern[] = "/tmp/planar-admission-XXXXXX";
    path           = std::filesystem::canonical(::mkdtemp(pattern));
    std::filesystem::create_directories(path / "trusted/modules");
    std::filesystem::create_directories(path / "workspace");
    put("trusted/profile with é space.json", "{}");
    put("trusted/modules/a.lua", "return 1");
    put("workspace/workspace.lua", "return 2");
    put("observation.json", "{}");
  }
  ~temporary() {
    std::filesystem::remove_all(path);
  }
  void put(std::string_view name, std::string_view bytes) {
    auto file = path / name;
    std::filesystem::create_directories(file.parent_path());
    std::ofstream(file, std::ios::binary) << bytes;
  }
  capture_admission admitted() {
    auto result = capture_admission::create((path / "trusted").string());
    REQUIRE(result);
    REQUIRE(result->select_trusted("profile", (path / "trusted/profile with é space.json").string()));
    REQUIRE(result->select_trusted("module", (path / "trusted/modules/a.lua").string()));
    return std::move(*result);
  }
};
void unsafe(const std::expected<void, diagnostic>& result) {
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "unsafe_path");
  CHECK(result.error().message == "External profile overlaps a supplied input");
}
} // namespace
static_assert(!std::is_default_constructible_v<capture_admission>);
static_assert(!std::is_copy_constructible_v<capture_admission>);
TEST_CASE("capture admission roots reject equality and both ancestry directions", "[capture][admission][path]") {
  temporary t;
  for (auto path : {t.path / "trusted", t.path, t.path / "trusted/modules"}) {
    auto admission = t.admitted();
    unsafe(admission.admit_root("untrusted", path.string()));
    CHECK_FALSE(admission.verify());
  }
  auto admission = t.admitted();
  std::filesystem::create_directory(t.path / "trusted-other");
  REQUIRE(admission.admit_root("source", (t.path / "trusted-other").string()));
  REQUIRE(admission.admit_root("workspace", (t.path / "workspace").string()));
  REQUIRE(admission.admit_root("base", t.path.string() + "/workspace"));
  CHECK(admission.verify());
}
TEST_CASE("capture admission standalone roles preserve registry colocation policy", "[capture][admission]") {
  temporary t;
  t.put("trusted/registry.json", "{}");
  auto admission = t.admitted();
  REQUIRE(admission.admit_file("registry", (t.path / "trusted/registry.json").string(), admission_file_role::registry));
  REQUIRE(admission.admit_file("ledger", (t.path / "trusted/registry.json").string(), admission_file_role::ledger));
  REQUIRE(admission.admit_file("observations", (t.path / "observation.json").string(), admission_file_role::observation));
  CHECK(admission.verify());
  for (auto role : {admission_file_role::observation, admission_file_role::declaration}) {
    auto rejected = t.admitted();
    unsafe(rejected.admit_file("untrusted", (t.path / "trusted/registry.json").string(), role));
  }
}
TEST_CASE("capture admission selected hard links refuse every supplied file role", "[capture][admission][alias]") {
  temporary t;
  std::filesystem::create_hard_link(t.path / "trusted/modules/a.lua", t.path / "alias");
  for (auto role : {admission_file_role::observation, admission_file_role::registry, admission_file_role::ledger,
                    admission_file_role::declaration}) {
    auto admission = t.admitted();
    unsafe(admission.admit_file("alias", (t.path / "alias").string(), role));
  }
  std::filesystem::create_hard_link(t.path / "trusted/modules/a.lua", t.path / "trusted/alias.lua");
  auto admission = t.admitted();
  unsafe(admission.select_trusted("duplicate", (t.path / "trusted/alias.lua").string()));
}
TEST_CASE("capture admission receipts report same admission and liveness", "[capture][admission][receipts]") {
  temporary t;
  auto      admission = t.admitted();
  REQUIRE(admission.admit_root("workspace", (t.path / "workspace").string()));
  REQUIRE(admission.admit_file("late", (t.path / "workspace/workspace.lua").string(), admission_file_role::declaration));
  auto root   = admission.retain_root("workspace");
  auto file   = admission.retain_file("late");
  auto second = admission.retain_file("late");
  REQUIRE(root);
  REQUIRE(file);
  REQUIRE(second);
  // Root and file receipts, and repeated file receipts, share one original admission.
  CHECK(root->live());
  CHECK(file->live());
  CHECK(root->same_admission(*file));
  CHECK(file->same_admission(*root));
  CHECK(file->same_admission(*second));
  CHECK(second->same_admission(*file));
  CHECK(root->same_admission(*root));
  // Receipts from an independent admission of the same paths do not match.
  auto other = t.admitted();
  REQUIRE(other.admit_root("workspace", (t.path / "workspace").string()));
  REQUIRE(other.admit_file("late", (t.path / "workspace/workspace.lua").string(), admission_file_role::declaration));
  auto other_root = other.retain_root("workspace");
  auto other_file = other.retain_file("late");
  REQUIRE(other_root);
  REQUIRE(other_file);
  CHECK(other_root->live());
  CHECK(other_file->live());
  CHECK_FALSE(root->same_admission(*other_root));
  CHECK_FALSE(other_root->same_admission(*root));
  CHECK_FALSE(file->same_admission(*other_file));
  CHECK_FALSE(other_file->same_admission(*file));
  CHECK_FALSE(file->same_admission(*other_root));
  CHECK_FALSE(root->same_admission(*other_file));
  // A moved-from receipt refuses liveness and identity; poisoning it is a harmless no-op.
  auto sink = std::move(*file);
  CHECK_FALSE(file->live());
  CHECK_FALSE(file->same_admission(*root));
  CHECK_FALSE(root->same_admission(*file));
  CHECK_FALSE(file->same_admission(*second));
  file->poison();
  CHECK(sink.live());
  CHECK(sink.same_admission(*root));
  CHECK(sink.same_admission(*second));
  auto root_sink = std::move(*root);
  CHECK_FALSE(root->live());
  CHECK_FALSE(root->same_admission(*file));
  root->poison();
  CHECK(root_sink.live());
  // Admission or receipt poisoning makes every receipt of that admission non-live.
  auto poisoned_by_admission = t.admitted();
  REQUIRE(poisoned_by_admission.admit_root("workspace", (t.path / "workspace").string()));
  REQUIRE(
      poisoned_by_admission.admit_file("late", (t.path / "workspace/workspace.lua").string(), admission_file_role::declaration));
  auto admission_root = poisoned_by_admission.retain_root("workspace");
  auto admission_file = poisoned_by_admission.retain_file("late");
  REQUIRE(admission_root);
  REQUIRE(admission_file);
  poisoned_by_admission.poison();
  CHECK_FALSE(admission_root->live());
  CHECK_FALSE(admission_file->live());
  // Identity is the shared original state; poisoning alters liveness, not identity.
  CHECK(admission_root->same_admission(*admission_file));
  auto poisoned_by_receipt = t.admitted();
  REQUIRE(poisoned_by_receipt.admit_root("workspace", (t.path / "workspace").string()));
  REQUIRE(
      poisoned_by_receipt.admit_file("late", (t.path / "workspace/workspace.lua").string(), admission_file_role::declaration));
  auto receipt_root = poisoned_by_receipt.retain_root("workspace");
  auto receipt_file = poisoned_by_receipt.retain_file("late");
  REQUIRE(receipt_root);
  REQUIRE(receipt_file);
  receipt_file->poison();
  CHECK_FALSE(receipt_root->live());
  CHECK_FALSE(receipt_file->live());
  CHECK_FALSE(poisoned_by_receipt.healthy());
}
TEST_CASE("capture admission staged trusted selections check earlier supplied aliases", "[capture][admission][alias]") {
  temporary t;
  std::filesystem::create_hard_link(t.path / "trusted/modules/a.lua", t.path / "alias");
  auto admission = capture_admission::create((t.path / "trusted").string());
  REQUIRE(admission);
  REQUIRE(admission->admit_file("early", (t.path / "alias").string(), admission_file_role::registry));
  unsafe(admission->select_trusted("later-module", (t.path / "trusted/modules/a.lua").string()));
}
TEST_CASE("capture admission symlinks refuse root intermediate and final components", "[capture][admission][path]") {
  temporary t;
  std::filesystem::create_directory_symlink(t.path / "trusted", t.path / "root-link");
  CHECK_FALSE(capture_admission::create((t.path / "root-link").string()));
  std::filesystem::create_directory_symlink(t.path / "trusted/modules", t.path / "trusted/link");
  std::filesystem::create_symlink(t.path / "trusted/modules/a.lua", t.path / "trusted/file-link");
  for (auto file : {"trusted/link/a.lua", "trusted/file-link"}) {
    auto admission = t.admitted();
    auto result    = admission.select_trusted("linked", (t.path / file).string());
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "path_denied");
  }
  for (auto operation : {"file", "root", "target"}) {
    auto                            admission = t.admitted();
    std::expected<void, diagnostic> result;
    if (operation == std::string_view("file"))
      result = admission.admit_file("linked", (t.path / "root-link/modules/a.lua").string(), admission_file_role::registry);
    else if (operation == std::string_view("root"))
      result = admission.admit_root("linked", (t.path / "root-link/modules").string());
    else
      result = admission.admit_target("linked", (t.path / "root-link/absent").string());
    CHECK_FALSE(result);
  }
}
TEST_CASE("capture admission retained modes and file metadata reject changes", "[capture][admission]") {
  for (auto item : {"trusted", "trusted/modules", "trusted/modules/a.lua", "workspace"}) {
    temporary t;
    auto      admission = t.admitted();
    REQUIRE(admission.admit_root("workspace", (t.path / "workspace").string()));
    REQUIRE(::chmod((t.path / item).c_str(), 0700) == 0);
    auto result = admission.verify();
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "capture_changed");
    CHECK(result.error().message == "Captured external profile changed");
    CHECK_FALSE(admission.verify());
  }
  temporary t;
  auto      admission = t.admitted();
  t.put("trusted/modules/a.lua", "different");
  CHECK_FALSE(admission.verify());
}
TEST_CASE("capture admission ancestor replacement preserving selected inode refuses", "[capture][admission][ancestor]") {
  temporary t;
  auto      admission = t.admitted();
  std::filesystem::rename(t.path / "trusted/modules", t.path / "old");
  std::filesystem::create_directory(t.path / "trusted/modules");
  std::filesystem::create_hard_link(t.path / "old/a.lua", t.path / "trusted/modules/a.lua");
  CHECK_FALSE(admission.verify());
}
TEST_CASE("capture admission root relocation and late addition retain original receipts", "[capture][admission][ancestor]") {
  temporary t;
  auto      admission = t.admitted();
  REQUIRE(admission.admit_root("workspace", (t.path / "workspace").string()));
  REQUIRE(admission.admit_file("late", (t.path / "workspace/workspace.lua").string(), admission_file_role::declaration));
  REQUIRE(admission.verify());
  std::filesystem::rename(t.path / "workspace", t.path / "old-workspace");
  std::filesystem::create_directory(t.path / "workspace");
  std::filesystem::create_hard_link(t.path / "old-workspace/workspace.lua", t.path / "workspace/workspace.lua");
  CHECK_FALSE(admission.admit_file("later", (t.path / "observation.json").string(), admission_file_role::registry));
  CHECK_FALSE(admission.verify());
}
TEST_CASE("capture admission unrelated siblings and their modes do not invalidate selections", "[capture][admission]") {
  temporary t;
  auto      admission = t.admitted();
  REQUIRE(admission.admit_root("workspace", (t.path / "workspace").string()));
  t.put("trusted/unselected.lua", "unread");
  t.put("trusted/modules/unselected.lua", "unread");
  t.put("workspace/unselected.lua", "unread");
  REQUIRE(::chmod((t.path / "trusted/unselected.lua").c_str(), 0755) == 0);
  std::filesystem::remove(t.path / "trusted/modules/unselected.lua");
  CHECK(admission.verify());
}
TEST_CASE("capture admission export targets retain absence and reject trusted overlap", "[capture][admission][path]") {
  temporary t;
  for (auto target : {t.path / "trusted", t.path, t.path / "trusted/new"}) {
    auto admission = t.admitted();
    unsafe(admission.admit_target("target", target.string()));
  }
  auto admission = t.admitted();
  REQUIRE(admission.admit_target("target", (t.path / "new-export").string()));
  REQUIRE(admission.verify());
  t.put("unrelated", "okay");
  REQUIRE(admission.verify());
  std::filesystem::create_directory(t.path / "new-export");
  CHECK_FALSE(admission.verify());
  auto missing = t.admitted();
  CHECK_FALSE(missing.admit_target("target", (t.path / "missing/child").string()));
  auto existing = t.admitted();
  REQUIRE(existing.admit_target("target", (t.path / "workspace").string()));
  CHECK(existing.verify());
}
TEST_CASE("capture admission move failed and bounded states refuse safely", "[capture][admission]") {
  temporary t;
  auto      admission = t.admitted();
  auto      moved     = std::move(admission);
  CHECK_FALSE(admission.verify());
  CHECK_FALSE(admission.admit_root("root", (t.path / "workspace").string()));
  auto* self = &moved;
  moved      = std::move(*self);
  REQUIRE(moved.verify());
  auto bad = moved.admit_root(std::string(1025, 'x'), (t.path / "workspace").string());
  CHECK_FALSE(bad);
  CHECK_FALSE(moved.verify());
  auto bounded = capture_admission::create((t.path / "trusted").string());
  REQUIRE(bounded);
  for (int n = 0; n < 65; ++n) {
    auto file = std::format("trusted/m{}.lua", n);
    t.put(file, "return 0");
    REQUIRE(bounded->select_trusted(std::format("module{}", n), (t.path / file).string()));
  }
  t.put("trusted/m65.lua", "return 0");
  auto result = bounded->select_trusted("overflow", (t.path / "trusted/m65.lua").string());
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "resource_limit");
  CHECK(result.error().message == "External profile resource limit exceeded");
  CHECK_FALSE(bounded->verify());
}
TEST_CASE("capture admission bound standalone profile and module reads retain original identity",
          "[capture][admission][binding]") {
  temporary              t;
  auto                   admission = t.admitted();
  native_capture_session session;
  REQUIRE(session.add_input("profile", (t.path / "trusted/profile with é space.json").string()));
  auto profile = session.read("profile", "input.json", admission, "profile");
  REQUIRE(profile);
  CHECK(profile->bytes == "{}");
  REQUIRE(session.add_root("modules", (t.path / "trusted/modules").string()));
  auto module = session.read("modules", "a.lua", admission, "module");
  REQUIRE(module);
  CHECK(module->bytes == "return 1");
  CHECK(session.finish());
}
TEST_CASE("capture admission descriptor binding rejects replacement before native content read",
          "[capture][admission][binding]") {
  temporary              t;
  auto                   admission = t.admitted();
  native_capture_session session;
  REQUIRE(session.add_input("profile", (t.path / "trusted/profile with é space.json").string()));
  std::filesystem::rename(t.path / "trusted/profile with é space.json", t.path / "trusted/old-profile");
  // Invalid text would produce schema if the replacement bytes were consumed and validated.
  t.put("trusted/profile with é space.json", std::string("\0invalid", 8));
  auto result = session.read("profile", "input.json", admission, "profile");
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "capture_changed");
  CHECK_FALSE(admission.verify());
}
TEST_CASE("capture admission descriptor binding refuses a different live file and unknown slots",
          "[capture][admission][binding]") {
  temporary t;
  for (auto slot : {"profile", "missing"}) {
    auto admission = t.admitted();
    int  fd        = ::open((t.path / "observation.json").c_str(), O_RDONLY | O_NOFOLLOW);
    REQUIRE(fd >= 0);
    auto result = admission.verify_file(slot, fd);
    ::close(fd);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "capture_changed");
    CHECK_FALSE(admission.verify());
  }
}
extern "C" void          planar_admission_watch(std::uint64_t device, std::uint64_t inode);
extern "C" std::uint64_t planar_admission_read_attempts();
TEST_CASE("capture admission selected inputs perform zero observed data reads before binding", "[capture][admission][syscalls]") {
  temporary   t;
  auto        profile = t.path / "trusted/profile with é space.json";
  struct stat selected{};
  REQUIRE(::stat(profile.c_str(), &selected) == 0);
  planar_admission_watch(selected.st_dev, selected.st_ino);
  auto admission = t.admitted();
  REQUIRE(admission.admit_root("workspace", (t.path / "workspace").string()));
  REQUIRE(admission.admit_file("late", (t.path / "workspace/workspace.lua").string(), admission_file_role::declaration));
  REQUIRE(admission.verify());
  REQUIRE(admission.verify());
  CHECK(planar_admission_read_attempts() == 0);
  // Positive control proves dynamic interposition sees actual reads of this selected inode.
  int fd = ::open(profile.c_str(), O_RDONLY | O_NOFOLLOW);
  REQUIRE(fd >= 0);
  char byte = 0;
  CHECK(::read(fd, &byte, 1) == 1);
  ::close(fd);
  CHECK(planar_admission_read_attempts() == 1);
  planar_admission_watch(selected.st_dev, selected.st_ino);
  native_capture_session session;
  REQUIRE(session.add_input("profile", profile.string()));
  REQUIRE(session.read("profile", "input.json", admission, "profile"));
  CHECK(planar_admission_read_attempts() > 0);
  // Observe the replacement inode: a mismatch must refuse before even the first read attempt.
  std::filesystem::rename(profile, t.path / "trusted/old-profile");
  t.put("trusted/profile with é space.json", "replacement");
  REQUIRE(::stat(profile.c_str(), &selected) == 0);
  planar_admission_watch(selected.st_dev, selected.st_ino);
  native_capture_session changed_session;
  REQUIRE(changed_session.add_input("profile", profile.string()));
  auto refused = changed_session.read("profile", "input.json", admission, "profile");
  CHECK_FALSE(refused);
  if (!refused)
    CHECK(refused.error().code == "capture_changed");
  CHECK(planar_admission_read_attempts() == 0);
  planar_admission_watch(0, 0);
}
TEST_CASE("capture admission all selected file roles have zero observed metadata phase data reads",
          "[capture][admission][syscalls]") {
  temporary t;
  t.put("trusted/registry.json", "{}");
  for (auto file : {"trusted/profile with é space.json", "trusted/modules/a.lua", "trusted/registry.json", "observation.json",
                    "workspace/workspace.lua"}) {
    auto        path = t.path / file;
    struct stat selected{};
    REQUIRE(::stat(path.c_str(), &selected) == 0);
    planar_admission_watch(selected.st_dev, selected.st_ino);
    auto admission = t.admitted();
    REQUIRE(admission.admit_file("registry", (t.path / "trusted/registry.json").string(), admission_file_role::registry));
    REQUIRE(admission.admit_file("observation", (t.path / "observation.json").string(), admission_file_role::observation));
    REQUIRE(admission.admit_root("workspace", (t.path / "workspace").string()));
    REQUIRE(admission.admit_file("declaration", (t.path / "workspace/workspace.lua").string(), admission_file_role::declaration));
    REQUIRE(admission.verify());
    REQUIRE(admission.verify());
    CHECK(planar_admission_read_attempts() == 0);
    int fd = ::open(path.c_str(), O_RDONLY | O_NOFOLLOW);
    REQUIRE(fd >= 0);
    char byte = 0;
    CHECK(::read(fd, &byte, 1) == 1);
    ::close(fd);
    CHECK(planar_admission_read_attempts() == 1);
    planar_admission_watch(0, 0);
  }
}
TEST_CASE("capture admission host and private receipt key bounds reject before path opening", "[capture][admission][bounds]") {
  temporary t;
  auto      admission = t.admitted();
  auto      result    = admission.admit_root("large", std::string(67108865, '/'));
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "resource_limit");
  CHECK(result.error().message == "External profile resource limit exceeded");
  CHECK_FALSE(admission.verify());
  auto selected = t.admitted();
  CHECK_FALSE(selected.admit_file("profile", (t.path / "observation.json").string(), admission_file_role::registry));
  CHECK_FALSE(selected.verify());
}
TEST_CASE("capture admission selected ancestors above trusted root retain modes", "[capture][admission][ancestor]") {
  temporary t;
  auto      admission = t.admitted();
  REQUIRE(::chmod(t.path.c_str(), 0750) == 0);
  CHECK_FALSE(admission.verify());
}
TEST_CASE("capture admission final supplied symlinks and absent target replacements refuse", "[capture][admission][path]") {
  temporary t;
  std::filesystem::create_directory_symlink(t.path / "workspace", t.path / "directory-link");
  std::filesystem::create_symlink(t.path / "observation.json", t.path / "input-link");
  {
    auto admission = t.admitted();
    CHECK_FALSE(admission.admit_root("root", (t.path / "directory-link").string()));
  }
  {
    auto admission = t.admitted();
    CHECK_FALSE(admission.admit_target("target", (t.path / "directory-link").string()));
  }
  for (auto role : {admission_file_role::registry, admission_file_role::observation, admission_file_role::declaration,
                    admission_file_role::ledger}) {
    auto admission = t.admitted();
    CHECK_FALSE(admission.admit_file("file", (t.path / "input-link").string(), role));
  }
  auto admission = t.admitted();
  REQUIRE(admission.admit_target("target", (t.path / "absent").string()));
  std::filesystem::create_directory_symlink(t.path / "workspace", t.path / "absent");
  CHECK_FALSE(admission.verify());
}
TEST_CASE("capture admission charges a host component before its temporary copy", "[capture][admission][bounds]") {
  temporary t;
  auto      admission = t.admitted();
  // Host length meets the inherited64MiB ceiling, but copying it plus retained
  // metadata would exceed the cumulative native admission allocation charge.
  auto result = admission.admit_root("component", "/" + std::string(67108863, 'x'));
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "resource_limit");
  CHECK(result.error().message == "External profile resource limit exceeded");
  CHECK_FALSE(admission.verify());
}
TEST_CASE("capture admission export target selected file aliases use overlap refusal", "[capture][admission][alias]") {
  temporary t;
  std::filesystem::create_hard_link(t.path / "trusted/modules/a.lua", t.path / "target-alias");
  for (auto file : {"trusted/modules/a.lua", "target-alias"}) {
    auto admission = t.admitted();
    unsafe(admission.admit_target("target", (t.path / file).string()));
  }
  auto admission = t.admitted();
  auto unrelated = admission.admit_target("target", (t.path / "observation.json").string());
  REQUIRE_FALSE(unrelated);
  CHECK(unrelated.error().code == "path_denied");
}
TEST_CASE("capture admission physical parent traversal retains final ancestry", "[capture][admission][parent]") {
  temporary t;
  auto      admission = t.admitted();
  REQUIRE(admission.admit_root("workspace", (t.path / "trusted/../workspace").string()));
  REQUIRE(admission.admit_root("equivalent", (t.path / "workspace/../workspace/./").string()));
  REQUIRE(admission.admit_file("declaration", (t.path / "trusted/../workspace/workspace.lua").string(),
                               admission_file_role::declaration));
  CHECK(admission.verify());
}
TEST_CASE("capture admission parent traversal never cancels unsafe intermediate components", "[capture][admission][parent]") {
  temporary t;
  std::filesystem::create_directory_symlink(t.path / "workspace", t.path / "link");
  for (auto component : {"link", "missing", "observation.json"}) {
    for (auto operation : {"root", "file", "target"}) {
      auto                            admission = t.admitted();
      auto                            prefix    = t.path / component / "..";
      std::expected<void, diagnostic> result;
      if (operation == std::string_view("root"))
        result = admission.admit_root("root", (prefix / "workspace").string());
      else if (operation == std::string_view("file"))
        result = admission.admit_file("file", (prefix / "observation.json").string(), admission_file_role::registry);
      else
        result = admission.admit_target("target", (prefix / "absent").string());
      REQUIRE_FALSE(result);
      CHECK(result.error().code == (component == std::string_view("missing") ? "io" : "path_denied"));
      CHECK(result.error().message.find(t.path.string()) == std::string::npos);
      CHECK_FALSE(admission.verify());
    }
  }
}
TEST_CASE("capture admission parent traversal preserves physical overlaps and file role policy", "[capture][admission][parent]") {
  temporary t;
  for (auto relative : {"workspace/../trusted", "workspace/..", "workspace/../trusted/modules"}) {
    auto admission = t.admitted();
    unsafe(admission.admit_root("root", (t.path / relative).string()));
    auto target = t.admitted();
    unsafe(target.admit_target("target", (t.path / relative).string()));
  }
  t.put("trusted/registry.json", "{}");
  for (auto role : {admission_file_role::observation, admission_file_role::declaration}) {
    auto admission = t.admitted();
    unsafe(admission.admit_file("file", (t.path / "workspace/../trusted/registry.json").string(), role));
  }
  auto registry = t.admitted();
  REQUIRE(
      registry.admit_file("registry", (t.path / "workspace/../trusted/registry.json").string(), admission_file_role::registry));
  auto trusted = capture_admission::create((t.path / "workspace/../trusted").string());
  REQUIRE(trusted);
  REQUIRE(trusted->select_trusted("module", (t.path / "workspace/../trusted/modules/a.lua").string()));
  unsafe(trusted->admit_file("alias", (t.path / "trusted/modules/../modules/a.lua").string(), admission_file_role::ledger));
}
TEST_CASE("capture admission exited traversal remains verified without false final nesting", "[capture][admission][parent]") {
  temporary t;
  auto      admission = capture_admission::create((t.path / "workspace/../trusted").string());
  REQUIRE(admission);
  REQUIRE(admission->admit_root("workspace", (t.path / "workspace").string()));
  REQUIRE(admission->verify());
  std::filesystem::rename(t.path / "workspace", t.path / "old-workspace");
  std::filesystem::create_directory(t.path / "workspace");
  auto result = admission->verify();
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "capture_changed");
  // The exited directory is also retained when it is no final selected root.
  std::filesystem::create_directory(t.path / "trusted-other");
  auto exited = t.admitted();
  REQUIRE(exited.admit_root("root", (t.path / "workspace/../trusted-other").string()));
  std::filesystem::rename(t.path / "workspace", t.path / "visited-workspace");
  CHECK_FALSE(exited.verify());
}
TEST_CASE("capture admission root repeated parents and slash typing remain physical", "[capture][admission][parent]") {
  temporary t;
  auto      trusted = capture_admission::create("/../.././//");
  REQUIRE(trusted);
  REQUIRE(trusted->verify());
  auto admission = t.admitted();
  REQUIRE(admission.admit_root("root", (t.path / "workspace/../../" / t.path.filename() / "workspace///./").string()));
  REQUIRE(admission.admit_target("absent", (t.path / "workspace/../new-export///").string()));
  CHECK(admission.verify());
  std::filesystem::create_directory(t.path / "new-export");
  CHECK_FALSE(admission.verify());
  for (auto suffix : {"observation.json/", "observation.json/.", "workspace/..", "workspace/../"}) {
    auto file = t.admitted();
    CHECK_FALSE(file.admit_file("file", (t.path / suffix).string(), admission_file_role::registry));
  }
  auto missing = t.admitted();
  CHECK_FALSE(missing.admit_target("target", (t.path / "missing/../new-export").string()));
}
TEST_CASE("capture admission root binding retains original directory and ignores sibling timestamps",
          "[capture][admission][d1]") {
  temporary t;
  auto      admission = t.admitted();
  REQUIRE(admission.admit_root("workspace", (t.path / "workspace").string()));
  int fd = ::open((t.path / "workspace").c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW);
  REQUIRE(fd >= 0);
  t.put("workspace/sibling", "unselected");
  REQUIRE(admission.verify_root("workspace", fd));
  auto moved = std::move(admission);
  CHECK_FALSE(admission.verify_root("workspace", fd));
  REQUIRE(moved.verify_root("workspace", fd));
  CHECK(::close(fd) == 0);
}
TEST_CASE("capture admission root binding refuses wrong descriptor slot and invalid descriptor terminally",
          "[capture][admission][d1]") {
  temporary t;
  int       workspace = ::open((t.path / "workspace").c_str(), O_RDONLY | O_DIRECTORY);
  int       trusted   = ::open((t.path / "trusted").c_str(), O_RDONLY | O_DIRECTORY);
  int       file      = ::open((t.path / "observation.json").c_str(), O_RDONLY);
  REQUIRE(workspace >= 0);
  REQUIRE(trusted >= 0);
  REQUIRE(file >= 0);
  for (auto [slot, fd] : std::vector<std::pair<std::string, int>>{{"workspace", trusted},
                                                                  {"workspace", file},
                                                                  {"workspace", -1},
                                                                  {"unknown", workspace},
                                                                  {"trusted", trusted},
                                                                  {"profile", workspace},
                                                                  {"target", workspace},
                                                                  {"absent", workspace},
                                                                  {"file", workspace}}) {
    auto admission = t.admitted();
    REQUIRE(admission.admit_root("workspace", (t.path / "workspace").string()));
    REQUIRE(admission.admit_target("target", (t.path / "workspace").string()));
    REQUIRE(admission.admit_target("absent", (t.path / "absent").string()));
    REQUIRE(admission.admit_file("file", (t.path / "observation.json").string(), admission_file_role::registry));
    auto result = admission.verify_root(slot, fd);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "capture_changed");
    CHECK_FALSE(admission.verify_root("workspace", workspace));
    CHECK_FALSE(admission.verify());
  }
  CHECK(::close(workspace) == 0);
  CHECK(::close(trusted) == 0);
  CHECK(::close(file) == 0);
}
TEST_CASE("capture admission root binding checks original ancestors leaf names and modes", "[capture][admission][d1]") {
  for (int mutation : {0, 1, 2, 3}) {
    temporary t;
    std::filesystem::create_directories(t.path / "outside/parent/root");
    auto path      = t.path / "outside/parent/root";
    auto admission = t.admitted();
    REQUIRE(admission.admit_root("root", path.string()));
    int fd = ::open(path.c_str(), O_RDONLY | O_DIRECTORY);
    REQUIRE(fd >= 0);
    if (mutation == 0) {
      std::filesystem::rename(path, path.string() + "-old");
      std::filesystem::create_directory(path);
    } else if (mutation == 1) {
      std::filesystem::rename(path.parent_path(), path.parent_path().string() + "-old");
      std::filesystem::create_directories(path);
    } else if (mutation == 2) {
      REQUIRE(::chmod(path.c_str(), 0700) == 0);
    } else {
      std::filesystem::rename(t.path / "outside", t.path / "moved");
      std::filesystem::create_directory(t.path / "outside");
    }
    auto result = admission.verify_root("root", fd);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "capture_changed");
    CHECK_FALSE(admission.verify());
    CHECK(::close(fd) == 0);
  }
}
TEST_CASE("capture admission root binding retains exited physical parent branches", "[capture][admission][d1]") {
  temporary t;
  std::filesystem::create_directories(t.path / "exit/branch");
  auto admission = t.admitted();
  REQUIRE(admission.admit_root("root", t.path.string() + "/exit/branch/../../workspace"));
  int fd = ::open((t.path / "workspace").c_str(), O_RDONLY | O_DIRECTORY);
  REQUIRE(fd >= 0);
  REQUIRE(admission.verify_root("root", fd));
  std::filesystem::rename(t.path / "exit/branch", t.path / "exit/old");
  std::filesystem::create_directory(t.path / "exit/branch");
  CHECK_FALSE(admission.verify_root("root", fd));
  CHECK(::close(fd) == 0);
}
TEST_CASE("D2 A retained receipt survives original wrapper moves and destruction", "[capture][admission][d2-a]") {
  temporary t;
  auto      receipt = [&] {
    auto owner = t.admitted();
    REQUIRE(owner.admit_file("file", (t.path / "workspace/workspace.lua").string(), admission_file_role::declaration));
    auto moved = std::move(owner);
    CHECK_FALSE(owner.healthy());
    auto token = moved.retain_file("file");
    REQUIRE(token);
    return std::move(*token);
  }();
  int fd = ::open((t.path / "workspace/workspace.lua").c_str(), O_RDONLY | O_NOFOLLOW);
  REQUIRE(fd >= 0);
  REQUIRE(receipt.verify(fd));
  auto moved = std::move(receipt);
  CHECK_FALSE(receipt.verify(fd));
  REQUIRE(moved.verify(fd));
  auto& alias = moved;
  moved       = std::move(alias);
  REQUIRE(moved.verify(fd));
  CHECK(::close(fd) == 0);
}
TEST_CASE("D2 A retained receipt refuses every non supplied file slot", "[capture][admission][d2-a]") {
  for (std::string slot : {"profile", "module", "trusted", "root", "target", "unknown"}) {
    temporary t;
    auto      owner = t.admitted();
    REQUIRE(owner.admit_root("root", (t.path / "workspace").string()));
    REQUIRE(owner.admit_target("target", (t.path / "out").string()));
    CHECK_FALSE(owner.retain_file(slot));
    CHECK_FALSE(owner.healthy());
    CHECK_FALSE(owner.verify());
  }
}
TEST_CASE("D2 A retained receipt uses stable append only selection and shared poison", "[capture][admission][d2-a]") {
  temporary t;
  auto      owner = t.admitted();
  REQUIRE(owner.admit_file("file", (t.path / "workspace/workspace.lua").string(), admission_file_role::registry));
  auto token = owner.retain_file("file");
  REQUIRE(token);
  for (int i = 0; i < 40; ++i)
    REQUIRE(owner.admit_root(std::format("root{}", i), (t.path / "workspace").string()));
  auto same = owner.retain_file("file");
  REQUIRE(same);
  CHECK(token->same_selection(*same));
  int fd = ::open((t.path / "workspace/workspace.lua").c_str(), O_RDONLY);
  REQUIRE(fd >= 0);
  REQUIRE(token->verify(fd));
  CHECK_FALSE(token->verify(-1));
  CHECK_FALSE(owner.healthy());
  CHECK_FALSE(same->verify(fd));
  CHECK(::close(fd) == 0);
}
TEST_CASE("D2 A retained receipt rejects different actual opened file", "[capture][admission][d2-a]") {
  temporary t;
  auto      owner = t.admitted();
  REQUIRE(owner.admit_file("file", (t.path / "workspace/workspace.lua").string(), admission_file_role::registry));
  auto token = owner.retain_file("file");
  REQUIRE(token);
  int fd = ::open((t.path / "trusted/modules/a.lua").c_str(), O_RDONLY);
  REQUIRE(fd >= 0);
  CHECK_FALSE(token->verify(fd));
  CHECK_FALSE(owner.healthy());
  CHECK(::close(fd) == 0);
}
TEST_CASE("D2 B2 root discovery retains metadata only and original explicit selection", "[capture][admission][d2-b2]") {
  temporary t;
  auto      admission = t.admitted();
  REQUIRE(admission.admit_root("root", (t.path / "workspace").string()));
  REQUIRE(admission.admit_file("explicit", (t.path / "workspace/workspace.lua").string(), admission_file_role::declaration));
  auto explicit_file = admission.retain_file("explicit");
  auto root          = admission.retain_root("root");
  REQUIRE(root);
  auto first = root->discover("workspace.lua");
  REQUIRE(first);
  REQUIRE(explicit_file);
  CHECK(first->same_selection(*explicit_file));
  auto repeated = root->discover("workspace.lua");
  REQUIRE(repeated);
  CHECK(first->same_selection(*repeated));
  auto moved = std::move(*root);
  CHECK_FALSE(root->discover("workspace.lua"));
  CHECK(moved.discover("workspace.lua"));
  t.put("workspace/workspace.lua", "changed original");
  CHECK_FALSE(moved.discover("workspace.lua"));
  CHECK_FALSE(admission.healthy());
}
TEST_CASE("D2 B2 discovery rejects escapes grammar links and trusted aliases", "[capture][admission][d2-b2]") {
  for (std::string path : {"../x", "/x", "a//b", "a/./b", "a/../b", ".GIT/x", "a\\b", "link", "alias", "linkdir/x"}) {
    temporary t;
    std::filesystem::create_symlink("workspace.lua", t.path / "workspace/link");
    std::filesystem::create_directory_symlink("../trusted", t.path / "workspace/linkdir");
    std::filesystem::create_hard_link(t.path / "trusted/modules/a.lua", t.path / "workspace/alias");
    auto admission = t.admitted();
    REQUIRE(admission.admit_root("root", (t.path / "workspace").string()));
    auto root = admission.retain_root("root");
    REQUIRE(root);
    CHECK_FALSE(root->discover(path));
    CHECK_FALSE(admission.healthy());
    CHECK_FALSE(root->discover("workspace.lua"));
  }
}
TEST_CASE("D2 B2 discovery keeps private keys bounded and independent of caller keys", "[capture][admission][d2-b2]") {
  temporary   t;
  std::string longest;
  for (int i = 0; i < 7; ++i)
    longest += std::string(128, 'a') + "/";
  longest += std::string(121, 'b');
  REQUIRE(longest.size() == 1024);
  // Construct through descriptors: the absolute host spelling exceeds macOS PATH_MAX.
  int directory = ::open((t.path / "workspace").c_str(), O_RDONLY | O_DIRECTORY);
  REQUIRE(directory >= 0);
  for (int i = 0; i < 7; ++i) {
    std::string name(128, 'a');
    REQUIRE(::mkdirat(directory, name.c_str(), 0755) == 0);
    int next = ::openat(directory, name.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW);
    REQUIRE(next >= 0);
    REQUIRE(::close(directory) == 0);
    directory = next;
  }
  int leaf = ::openat(directory, std::string(121, 'b').c_str(), O_WRONLY | O_CREAT, 0644);
  REQUIRE(leaf >= 0);
  REQUIRE(::write(leaf, "longest", 7) == 7);
  REQUIRE(::close(leaf) == 0);
  REQUIRE(::close(directory) == 0);
  auto admission = t.admitted();
  REQUIRE(admission.admit_root("root", (t.path / "workspace").string()));
  REQUIRE(admission.admit_file("4", (t.path / "observation.json").string(), admission_file_role::observation));
  auto root = admission.retain_root("root");
  REQUIRE(root);
  auto long_file = root->discover(longest);
  REQUIRE(long_file);
  auto other = root->discover("workspace.lua");
  REQUIRE(other);
  CHECK_FALSE(long_file->same_selection(*other));
  REQUIRE(root->discover(longest));
  CHECK_FALSE(root->discover(longest + "b"));
}
TEST_CASE("D2 B2 root receipt requires supplied roots and actual original root descriptors", "[capture][admission][d2-b2]") {
  temporary t;
  for (std::string slot : {"trusted", "profile", "missing"}) {
    auto admission = t.admitted();
    CHECK_FALSE(admission.retain_root(slot));
    CHECK_FALSE(admission.healthy());
  }
  auto admission = t.admitted();
  REQUIRE(admission.admit_root("root", (t.path / "workspace").string()));
  auto root = admission.retain_root("root");
  REQUIRE(root);
  int fd = ::open((t.path / "workspace").c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW);
  REQUIRE(fd >= 0);
  CHECK(root->verify(fd));
  REQUIRE(::close(fd) == 0);
  CHECK_FALSE(root->verify(-1));
  CHECK_FALSE(admission.healthy());
}
TEST_CASE("D2 B2 metadata discovery refuses component case aliases", "[capture][admission][d2-b2]") {
  temporary t;
  t.put("workspace/A/one", "one");
  t.put("workspace/a/two", "two");
  auto admission = t.admitted();
  REQUIRE(admission.admit_root("root", (t.path / "workspace").string()));
  auto root = admission.retain_root("root");
  REQUIRE(root);
  REQUIRE(root->discover("A/one"));
  CHECK_FALSE(root->discover("a/two"));
  CHECK_FALSE(admission.healthy());
}
TEST_CASE("D2 B2 caller file slots exclude private ordinal namespace in either order", "[capture][admission][d2-b2]") {
  for (bool caller_first : {false, true}) {
    temporary t;
    auto      admission = t.admitted();
    REQUIRE(admission.admit_root("root", (t.path / "workspace").string()));
    // Initial selection count is four; inserting the caller first advances it to five.
    std::string key = caller_first ? "5" : "4";
    if (caller_first)
      REQUIRE(admission.admit_file(key, (t.path / "observation.json").string(), admission_file_role::observation));
    auto root = admission.retain_root("root");
    REQUIRE(root);
    auto discovered = root->discover("workspace.lua");
    REQUIRE(discovered);
    if (!caller_first)
      REQUIRE(admission.admit_file(key, (t.path / "observation.json").string(), admission_file_role::observation));
    auto caller = admission.retain_file(key);
    REQUIRE(caller);
    CHECK_FALSE(caller->same_selection(*discovered));
    int fd = ::open((t.path / "observation.json").c_str(), O_RDONLY | O_NOFOLLOW);
    REQUIRE(fd >= 0);
    CHECK(admission.verify_file(key, fd));
    REQUIRE(::close(fd) == 0);
  }
  temporary t;
  auto      admission = t.admitted();
  REQUIRE(admission.admit_root("root", (t.path / "workspace").string()));
  auto root = admission.retain_root("root");
  REQUIRE(root);
  REQUIRE(root->discover("workspace.lua"));
  int fd = ::open((t.path / "workspace/workspace.lua").c_str(), O_RDONLY | O_NOFOLLOW);
  REQUIRE(fd >= 0);
  CHECK_FALSE(admission.verify_file("4", fd));
  REQUIRE(::close(fd) == 0);
  CHECK_FALSE(admission.healthy());
}
namespace {
struct permission_guard {
  std::filesystem::path path;
  ~permission_guard() {
    std::error_code ec;
    std::filesystem::permissions(path, std::filesystem::perms::owner_all, std::filesystem::perm_options::replace, ec);
  }
};
} // namespace
TEST_CASE("capture admission reports only final-component ENOENT as absence", "[capture][admission][absent]") {
  temporary  t;
  const auto role = admission_file_role::declaration;
  SECTION("present file admits and reports no absence") {
    auto admission = t.admitted();
    CHECK_FALSE(admission.last_refusal_was_absent());
    REQUIRE(admission.admit_file("declaration", (t.path / "workspace/workspace.lua").string(), role));
    CHECK_FALSE(admission.last_refusal_was_absent());
  }
  SECTION("missing final component reports absence with the unchanged message and resets on the next admission") {
    auto admission = t.admitted();
    auto missing   = admission.admit_file("declaration", (t.path / "workspace/missing.lua").string(), role);
    REQUIRE_FALSE(missing);
    CHECK(missing.error().code == "io");
    CHECK(missing.error().message == "Cannot open confined input");
    CHECK(admission.last_refusal_was_absent());
    CHECK_FALSE(admission.healthy());
    auto next = admission.admit_file("other", (t.path / "workspace/workspace.lua").string(), role);
    REQUIRE_FALSE(next);
    CHECK_FALSE(admission.last_refusal_was_absent());
  }
  SECTION("missing intermediate directory is not absence of the selected file") {
    auto admission = t.admitted();
    auto missing   = admission.admit_file("declaration", (t.path / "no-such-dir/workspace.lua").string(), role);
    REQUIRE_FALSE(missing);
    CHECK(missing.error().message == "Cannot open confined input");
    CHECK_FALSE(admission.last_refusal_was_absent());
  }
  SECTION("hard-link refusal is not absence") {
    std::filesystem::create_hard_link(t.path / "trusted/modules/a.lua", t.path / "alias");
    auto admission = t.admitted();
    unsafe(admission.admit_file("alias", (t.path / "alias").string(), role));
    CHECK_FALSE(admission.last_refusal_was_absent());
  }
  SECTION("permission-denied directory is not absence") {
    if (::geteuid() == 0)
      SUCCEED("root bypasses directory permissions");
    else {
      auto locked = t.path / "locked";
      t.put("locked/workspace.lua", "return 3");
      permission_guard guard{locked};
      std::filesystem::permissions(
          locked, std::filesystem::perms::owner_read | std::filesystem::perms::group_read | std::filesystem::perms::others_read,
          std::filesystem::perm_options::replace);
      auto admission = t.admitted();
      auto denied    = admission.admit_file("declaration", (locked / "workspace.lua").string(), role);
      REQUIRE_FALSE(denied);
      CHECK(denied.error().code == "io");
      CHECK(denied.error().message == "Cannot open confined input");
      CHECK_FALSE(admission.last_refusal_was_absent());
    }
  }
  SECTION("moved-from admission reports no absence") {
    auto admission = t.admitted();
    auto moved     = std::move(admission);
    CHECK_FALSE(admission.last_refusal_was_absent()); // NOLINT(bugprone-use-after-move)
    CHECK_FALSE(moved.last_refusal_was_absent());
  }
}
