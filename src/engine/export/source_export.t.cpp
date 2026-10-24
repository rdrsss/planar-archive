// Exact source publication using scratch native acceptance and real filesystem boundaries.
#include <catch2/catch_test_macros.hpp>
#include <fcntl.h>
#include <sqlite3.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
import std;
import planar.engine.ledger.format;
import planar.engine.history.provenance_codec;
import planar.engine.history.interpretation_codec;
import planar.engine.canonical;
#if defined(PLANAR_EXPORT_FAULT_TEST)
import planar.test.source_export;
import planar.test.export_posix;
namespace exporting = planar::engine::source_export_fault;
#else
import planar.engine.source_export;
namespace exporting = planar::engine::source_export;
#endif
using namespace planar::engine;
using namespace planar::engine::history;
using namespace planar::engine::ledger;
using namespace planar::engine::acceptance;
namespace {
template <class T> auto take(T&& result) {
  if (!result)
    throw std::runtime_error("scratch fixture refused");
  if constexpr (!std::is_void_v<typename std::remove_cvref_t<T>::value_type>)
    return std::move(*result);
}
std::string bytes(const std::filesystem::path& p) {
  std::ifstream in(p, std::ios::binary);
  return {std::istreambuf_iterator<char>(in), {}};
}
void put(const std::filesystem::path& p, std::string_view s) {
  std::ofstream out(p, std::ios::binary);
  out.write(s.data(), s.size());
  if (!out)
    throw std::runtime_error("scratch write");
}
struct scratch {
  std::filesystem::path root;
  scratch() {
    char p[] = "/tmp/planar-export-XXXXXX";
    if (!::mkdtemp(p))
      throw std::runtime_error("scratch root");
    root = std::filesystem::canonical(p);
  }
  ~scratch() {
    std::error_code ec;
    std::filesystem::remove_all(root, ec);
  }
};
std::string hex(const record_digest& d) {
  std::string s;
  for (auto b : d)
    s += std::format("{:02x}", b);
  return s;
}
void accept(const std::filesystem::path& root, const std::vector<std::string>& paths, std::string key = "first",
            std::optional<std::int64_t> head = {}) {
  const std::array originals{original_file_view{"part.lua", "return {}"}};
  auto             stdlib = take(prepare_stdlib(originals, "\n", "return {}\n"));
  auto             gold   = std::filesystem::path(PLANAR_PREPARED_GOLDENS);
  auto saved       = take(decode_interpretation(bytes(gold / "interpretation.canonical"), bytes(gold / "versions.canonical")));
  auto interpreted = take(encode_interpretation(saved.interpretation->fields.at("bundle"),
                                                saved.interpretation->fields.at("semantic"), hex(stdlib.digest)));
  native_capture_session session;
  take(session.add_root("workspace", root / "workspace"));
  take(session.tree("workspace"));
  take(session.add_input("registry", root / "registry.json"));
  take(session.read("registry", "input.json", false));
  auto                          capture = take(session.finish_for_acceptance());
  std::vector<std::string_view> views;
  for (auto& p : paths)
    views.push_back(p);
  auto candidate = take(
      prepared_acceptance::create(std::move(capture), views, stdlib.files, interpreted.encoded, interpreted.versions_encoded));
  auto intent = take(request_intent::create(key, "fixture:actor", head, (root / "PRIVATE_WORKSPACE.lua").string(),
                                            (root / "PRIVATE_REGISTRY.json").string()));
  take(complete_acceptance(root / "ledger", intent, candidate));
}
void fixture(const std::filesystem::path& root) {
  take(initialize_new(root / "ledger", "quota"));
  std::filesystem::create_directories(root / "workspace/nested");
  put(root / "workspace/a.lua", std::string_view("binary\0\xff\r\n", 10));
  put(root / "workspace/nested/run", "#!/bin/sh\r\nexit 0\r\n");
  put(root / "workspace/empty", "");
  put(root / "workspace/excluded", "unselected source");
  put(root / "registry.json", "private registry");
  REQUIRE(::chmod((root / "workspace/nested/run").c_str(), 0755) == 0);
  accept(root, {"a.lua", "empty", "nested/run"});
}
std::vector<std::filesystem::path> stages(const std::filesystem::path& root) {
  std::vector<std::filesystem::path> out;
  for (auto& e : std::filesystem::directory_iterator(root))
    if (e.path().filename().string().starts_with(".planar-export-"))
      out.push_back(e.path());
  return out;
}
void exact(const std::filesystem::path& target, const std::filesystem::path& workspace) {
  for (auto p : {"a.lua", "empty", "nested/run"})
    CHECK(bytes(target / p) == bytes(workspace / p));
  std::size_t count = 0;
  for (auto& e : std::filesystem::recursive_directory_iterator(target)) {
    (void)e;
    ++count;
  }
  CHECK(count == 4);
  for (auto p : {"", "nested", "nested/run", "a.lua", "empty"}) {
    struct stat st{};
    REQUIRE(::stat((target / p).c_str(), &st) == 0);
    CHECK((st.st_mode & 0777) == (std::string_view(p) == "a.lua" || std::string_view(p) == "empty" ? 0644 : 0755));
  }
}
} // namespace
#if !defined(PLANAR_EXPORT_FAULT_TEST)
TEST_CASE("source export preserves exact bytes modes and source-only closure", "[source_export]") {
  scratch s;
  fixture(s.root);
  auto before    = bytes(s.root / "ledger");
  auto old_umask = ::umask(0077);
  auto result    = exporting::export_source(s.root / "ledger", 1, s.root / "target");
  ::umask(old_umask);
  REQUIRE(result);
  CHECK(result->revision == 1);
  CHECK(result->source > 0);
  CHECK(result->ledger.size() == 32);
  exact(s.root / "target", s.root / "workspace");
  CHECK(bytes(s.root / "ledger") == before);
  CHECK(stages(s.root).empty());
}
TEST_CASE("source export refuses existing entries and unsafe original paths", "[source_export]") {
  scratch s;
  fixture(s.root);
  std::filesystem::create_directory(s.root / "directory");
  put(s.root / "file", "sentinel");
  std::filesystem::create_directory_symlink(s.root / "workspace", s.root / "link");
  std::filesystem::create_symlink("absent", s.root / "dangling");
  for (auto p : {"directory", "file", "link", "dangling"}) {
    auto r = exporting::export_source(s.root / "ledger", 1, s.root / p);
    REQUIRE_FALSE(r);
    CHECK(r.error().code == exporting::error_code::target_exists);
  }
  for (auto p : {"missing/../target", "link/target", "missing/target"}) {
    auto r = exporting::export_source(s.root / "ledger", 1, s.root / p);
    REQUIRE_FALSE(r);
    CHECK(r.error().code == exporting::error_code::path_denied);
  }
  for (auto p : {"target/", "target/.", "target/.."}) {
    auto r = exporting::export_source(s.root / "ledger", 1, s.root / p);
    REQUIRE_FALSE(r);
    CHECK(r.error().code == exporting::error_code::invalid_argument);
  }
  CHECK(bytes(s.root / "file") == "sentinel");
  CHECK(stages(s.root).empty());
  auto missing = exporting::export_source(s.root / "ledger", 99, s.root / "target");
  REQUIRE_FALSE(missing);
  CHECK(missing.error().code == exporting::error_code::revision_not_found);
  auto invalid = exporting::export_source(s.root / "ledger", 0, s.root / "target");
  REQUIRE_FALSE(invalid);
  CHECK(invalid.error().code == exporting::error_code::invalid_argument);
  auto bad = exporting::export_source(s.root / "absent", 1, s.root / "target");
  REQUIRE_FALSE(bad);
  CHECK(bad.error().code == exporting::error_code::storage);
  REQUIRE(bad.error().storage);
  CHECK(bad.error().storage->code == format_error_code::not_found);
}
TEST_CASE("source export accepts an unsupported interpretation and an empty stored root", "[source_export]") {
  scratch unknown;
  auto    pid = ::fork();
  REQUIRE(pid >= 0);
  if (pid == 0) {
    ::execl(PLANAR_EXPORT_FIXTURE, PLANAR_EXPORT_FIXTURE, unknown.root.c_str(), "unknown", static_cast<char*>(nullptr));
    ::_exit(91);
  }
  int status = 0;
  REQUIRE(::waitpid(pid, &status, 0) == pid);
  REQUIRE(WIFEXITED(status));
  REQUIRE(WEXITSTATUS(status) == 0);
  REQUIRE(exporting::export_source(unknown.root / "ledger", 1, unknown.root / "target"));
  CHECK(bytes(unknown.root / "target/a.lua") == "original");
  scratch empty;
  fixture(empty.root);
  auto source_id = [&] {
    auto reader = take(open_existing(empty.root / "ledger"));
    return take(reader.read_revision(1))->references.source;
  }();
  // Scratch-only fixture adaptation: the supported tree format permits an empty root;
  // native acceptance deliberately requires a nonempty declaration source selection.
  sqlite3* db = nullptr;
  REQUIRE(sqlite3_open_v2((empty.root / "ledger").c_str(), &db, SQLITE_OPEN_READWRITE, nullptr) == SQLITE_OK);
  auto sql = [&](const std::string& statement) {
    REQUIRE(sqlite3_exec(db, statement.c_str(), nullptr, nullptr, nullptr) == SQLITE_OK);
  };
  sql("BEGIN IMMEDIATE");
  sql("DROP TRIGGER records_no_update");
  sql("DROP TRIGGER tree_edges_no_delete");
  sql(std::format("DELETE FROM tree_edges WHERE parent_id={}", source_id));
  record        r       = directory_record{};
  auto          encoded = take(encode_record(r));
  auto          digest  = take(digest_record(r));
  sqlite3_stmt* stmt    = nullptr;
  REQUIRE(sqlite3_prepare_v2(db, "UPDATE records SET encoded=?1,digest=?2 WHERE id=?3", -1, &stmt, nullptr) == SQLITE_OK);
  REQUIRE(sqlite3_bind_blob(stmt, 1, encoded.data(), static_cast<int>(encoded.size()), SQLITE_TRANSIENT) == SQLITE_OK);
  REQUIRE(sqlite3_bind_blob(stmt, 2, digest.data(), static_cast<int>(digest.size()), SQLITE_TRANSIENT) == SQLITE_OK);
  REQUIRE(sqlite3_bind_int64(stmt, 3, source_id) == SQLITE_OK);
  REQUIRE(sqlite3_step(stmt) == SQLITE_DONE);
  REQUIRE(sqlite3_finalize(stmt) == SQLITE_OK);
  sql("CREATE TRIGGER records_no_update BEFORE UPDATE ON records BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END");
  sql("CREATE TRIGGER tree_edges_no_delete BEFORE DELETE ON tree_edges BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END");
  sql("COMMIT");
  REQUIRE(sqlite3_close(db) == SQLITE_OK);
  REQUIRE(open_existing(empty.root / "ledger"));
  REQUIRE(exporting::export_source(empty.root / "ledger", 1, empty.root / "target"));
  CHECK(std::filesystem::is_empty(empty.root / "target"));
}
TEST_CASE("source export accepts relative target and old reverted revisions", "[source_export]") {
  scratch s;
  fixture(s.root);
  auto original = bytes(s.root / "workspace/a.lua");
  put(s.root / "workspace/a.lua", "changed");
  accept(s.root, {"a.lua", "empty", "nested/run"}, "second", 1);
  put(s.root / "workspace/a.lua", original);
  accept(s.root, {"a.lua", "empty", "nested/run"}, "third", 2);
  auto relative = std::filesystem::relative(s.root / "old", std::filesystem::current_path());
  REQUIRE(exporting::export_source(s.root / "ledger", 1, relative));
  REQUIRE(exporting::export_source(s.root / "ledger", 2, s.root / "changed"));
  REQUIRE(exporting::export_source(s.root / "ledger", 3, s.root / "revert"));
  CHECK(bytes(s.root / "old/a.lua") == original);
  CHECK(bytes(s.root / "changed/a.lua") == "changed");
  CHECK(bytes(s.root / "revert/a.lua") == original);
}
TEST_CASE("source export supports bounded-depth traversal under a low descriptor ceiling", "[source_export]") {
  scratch s;
  take(initialize_new(s.root / "ledger", "quota"));
  std::filesystem::create_directory(s.root / "workspace");
  put(s.root / "registry.json", "registry");
  std::vector<std::string> paths;
  std::string              deep;
  for (int i = 0; i < 63; ++i)
    deep += "d/";
  paths.push_back(deep + "leaf");
  std::filesystem::create_directories((s.root / "workspace" / paths.back()).parent_path());
  put(s.root / "workspace" / paths.back(), "depth64");
  for (int i = 0; i < 256; ++i) {
    paths.push_back(std::format("file{:04}", i));
    put(s.root / "workspace" / paths.back(), "x");
  }
  accept(s.root, paths);
  auto pid = ::fork();
  REQUIRE(pid >= 0);
  if (pid == 0) {
    struct rlimit limit{128, 128};
    if (::setrlimit(RLIMIT_NOFILE, &limit) != 0)
      ::_exit(20);
    auto r = exporting::export_source(s.root / "ledger", 1, s.root / "target");
    ::_exit(r ? 0 : 21);
  }
  int status = 0;
  REQUIRE(::waitpid(pid, &status, 0) == pid);
  REQUIRE(WIFEXITED(status));
  CHECK(WEXITSTATUS(status) == 0);
  CHECK(bytes(s.root / "target" / paths.front()) == "depth64");
  CHECK(stages(s.root).empty());
}
TEST_CASE("source export handles the source file ceiling and thousands of directories with bounded descriptors",
          "[source_export]") {
  scratch s;
  take(initialize_new(s.root / "ledger", "quota"));
  std::filesystem::create_directory(s.root / "workspace");
  put(s.root / "registry.json", "registry");
  std::vector<std::string> paths;
  for (int i = 0; i < 4095; ++i) {
    paths.push_back(std::format("d{:04}/child/file", i));
    std::filesystem::create_directories((s.root / "workspace" / paths.back()).parent_path());
    put(s.root / "workspace" / paths.back(), "x");
  }
  paths.push_back("root-file");
  put(s.root / "workspace/root-file", "last");
  accept(s.root, paths);
  auto pid = ::fork();
  REQUIRE(pid >= 0);
  if (pid == 0) {
    struct rlimit limit{128, 128};
    if (::setrlimit(RLIMIT_NOFILE, &limit) != 0)
      ::_exit(20);
    auto r = exporting::export_source(s.root / "ledger", 1, s.root / "target");
    ::_exit(r ? 0 : 21);
  }
  int status = 0;
  REQUIRE(::waitpid(pid, &status, 0) == pid);
  REQUIRE(WIFEXITED(status));
  REQUIRE(WEXITSTATUS(status) == 0);
  std::size_t files = 0, dirs = 1;
  for (auto& e : std::filesystem::recursive_directory_iterator(s.root / "target")) {
    if (e.is_regular_file())
      ++files;
    else if (e.is_directory())
      ++dirs;
  }
  CHECK(files == 4096);
  CHECK(dirs == 8191);
  CHECK(stages(s.root).empty());
}

#else
TEST_CASE("source export fault boundaries clean before publication and preserve after", "[source_export_fault]") {
  using namespace planar::test::export_posix;
  for (auto fault : {fault::mkdir_stage, fault::mkdir_child, fault::open_stage, fault::open_file, fault::write, fault::zero_write,
                     fault::chmod_file, fault::file_sync, fault::directory_sync, fault::rename, fault::unsupported,
                     fault::close_file, fault::allocation}) {
    CAPTURE(static_cast<int>(fault));
    scratch s;
    fixture(s.root);
    reset(fault, s.root / "target");
    auto result = exporting::export_source(s.root / "ledger", 1, s.root / "target");
    REQUIRE_FALSE(result);
    CHECK(result.error().code == exporting::error_code::io);
    CHECK(hits() == 1);
    CHECK_FALSE(std::filesystem::exists(s.root / "target"));
    CHECK(stages(s.root).empty());
    reset(fault::none, {});
  }
  for (auto fault : {fault::short_write, fault::eintr}) {
    scratch s;
    fixture(s.root);
    reset(fault, s.root / "target");
    REQUIRE(exporting::export_source(s.root / "ledger", 1, s.root / "target"));
    CHECK(hits() == 1);
    exact(s.root / "target", s.root / "workspace");
    reset(fault::none, {});
  }
  for (auto fault : {fault::parent_sync, fault::close_root_after, fault::close_parent_after}) {
    scratch s;
    fixture(s.root);
    reset(fault, s.root / "target");
    auto result = exporting::export_source(s.root / "ledger", 1, s.root / "target");
    REQUIRE_FALSE(result);
    CHECK(result.error().code == exporting::error_code::publication_uncertain);
    CHECK(hits() == 1);
    exact(s.root / "target", s.root / "workspace");
    CHECK(stages(s.root).empty());
    reset(fault::none, {});
  }
}
TEST_CASE("source export no-replace race preserves arriving target", "[source_export_fault]") {
  using namespace planar::test::export_posix;
  scratch s;
  fixture(s.root);
  reset(fault::arrival, s.root / "target");
  auto result = exporting::export_source(s.root / "ledger", 1, s.root / "target");
  REQUIRE_FALSE(result);
  CHECK(result.error().code == exporting::error_code::target_exists);
  CHECK(hits() == 1);
  CHECK(std::filesystem::is_directory(s.root / "target"));
  CHECK(std::filesystem::is_empty(s.root / "target"));
  CHECK(stages(s.root).empty());
  reset(fault::none, {});
}
TEST_CASE("source export refuses cleanup of replaced or unowned entries", "[source_export_fault]") {
  using namespace planar::test::export_posix;
  for (auto fault : {fault::cleanup_file, fault::cleanup_directory, fault::replace_file, fault::replace_directory,
                     fault::replace_root, fault::unowned}) {
    CAPTURE(static_cast<int>(fault));
    scratch s;
    fixture(s.root);
    reset(fault, s.root / "target");
    auto result = exporting::export_source(s.root / "ledger", 1, s.root / "target");
    REQUIRE_FALSE(result);
    CHECK(result.error().code == exporting::error_code::cleanup_incomplete);
    CHECK(hits() == 1);
    CHECK(result.error().location.starts_with(".planar-export-"));
    CHECK(result.error().location.size() == 47);
    CHECK(result.error().location.find('/') == std::string::npos);
    CHECK_FALSE(std::filesystem::exists(s.root / "target"));
    REQUIRE(stages(s.root).size() == 1);
    if (fault == fault::replace_file)
      CHECK(bytes(stages(s.root)[0] / "a.lua") == "unowned");
    if (fault == fault::unowned)
      CHECK(bytes(stages(s.root)[0] / "unexpected") == "unowned");
    if (fault == fault::replace_root) {
      CHECK(bytes(stages(s.root)[0] / "sentinel") == "unowned");
      CHECK(std::filesystem::is_directory(s.root / "moved-owned-stage"));
    }
    if (fault == fault::replace_directory)
      CHECK(bytes(stages(s.root)[0] / "nested/sentinel") == "unowned");
    reset(fault::none, {});
  }
}
TEST_CASE("source export termination leaves absent or complete target and never adopts leftovers", "[source_export_fault]") {
  using namespace planar::test::export_posix;
  for (auto fault : {fault::crash_before, fault::crash_after}) {
    scratch s;
    fixture(s.root);
    auto pid = ::fork();
    REQUIRE(pid >= 0);
    if (pid == 0) {
      reset(fault, s.root / "target");
      (void)exporting::export_source(s.root / "ledger", 1, s.root / "target");
      ::_exit(90);
    }
    int status = 0;
    REQUIRE(::waitpid(pid, &status, 0) == pid);
    REQUIRE(WIFEXITED(status));
    CHECK(WEXITSTATUS(status) == 77);
    if (fault == fault::crash_before) {
      CHECK_FALSE(std::filesystem::exists(s.root / "target"));
      REQUIRE(stages(s.root).size() == 1);
    } else {
      exact(s.root / "target", s.root / "workspace");
      CHECK(stages(s.root).empty());
    }
    auto before = stages(s.root);
    REQUIRE(exporting::export_source(s.root / "ledger", 1, s.root / "fresh"));
    CHECK(stages(s.root) == before);
  }
}
#endif
