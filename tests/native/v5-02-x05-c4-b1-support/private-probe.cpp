// Private reversible production copies expose only raw verification accessors, never shipped APIs.
#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <fcntl.h>
#include <new>
#include <sqlite3.h>
#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.engine.ledger.sqlite;
import planar.engine.capture.profile;
using namespace planar::engine;
using namespace planar::engine::ledger;
static bool        c4_fail_allocation = false;
static int         c4_new_after       = -1;
static std::size_t c4_failed_request  = 0;
void*              operator new(std::size_t n) {
  if (std::exchange(c4_fail_allocation, false) || (c4_new_after >= 0 && c4_new_after-- == 0)) {
    c4_failed_request = n;
    throw std::bad_alloc();
  }
  if (auto* p = std::malloc(n ? n : 1))
    return p;
  throw std::bad_alloc();
}
void operator delete(void* p) noexcept {
  std::free(p);
}
void* operator new[](std::size_t n) {
  return ::operator new(n);
}
void operator delete[](void* p) noexcept {
  ::operator delete(p);
}
namespace {
struct scratch {
  std::filesystem::path root;
  scratch() {
    char pattern[] = "/tmp/planar-c4b1-private-XXXXXX";
    REQUIRE(::mkdtemp(pattern));
    root = std::filesystem::canonical(pattern);
  }
  ~scratch() {
    std::filesystem::remove_all(root);
  }
  connection open(std::string_view name) {
    auto c = connection::open(root / name, open_mode::scratch_create, 0);
    REQUIRE(c);
    return std::move(*c);
  }
};
void sql(connection& db, std::string_view s) {
  auto q = db.prepare(s);
  REQUIRE(q);
  REQUIRE(q->step() == step_result::done);
}
#include "observer.inc"
struct guard : read_guard {
  read_gate_status status = read_gate_status::ready;
  read_gate_status verify() override {
    return status;
  }
};
} // namespace
TEST_CASE("C4B1 actual diagnostic allocation exception finishes backup", "[c4b1-backup]") {
  scratch      f;
  observed_vfs observer;
  auto         source = f.open("source"), target = f.open("target");
  sql(source, "CREATE TABLE items(n)");
  sql(source, "INSERT INTO items VALUES(1)");
  auto stale = target.prepare("SELECT * FROM deliberately_missing_table_with_a_long_native_sqlite_diagnostic");
  REQUIRE_FALSE(stale);
  REQUIRE(stale.error().message.size() > 22);
  bool threw         = false;
  c4_fail_allocation = true;
  try {
    source.online_backup_to(target);
  } catch (const std::bad_alloc&) {
    threw = true;
  }
  c4_fail_allocation = false;
  REQUIRE(threw);
  if (private_backup_live()) {
    CHECK(private_backup_finishes() == 0);
    CHECK(sqlite3_close(static_cast<sqlite3*>(source.private_handle())) == SQLITE_BUSY);
    private_backup_cleanup(); // Reconstructed-red cleanup only; prove the leaked real handle, then release it.
    FAIL_CHECK("original backup leaked through actual std::string allocation failure");
  } else
    CHECK(private_backup_finishes() == 1);
  REQUIRE(source.online_backup_to(target));
  CHECK(private_backup_finishes() == 2);
  auto q = target.prepare("SELECT count(*) FROM items");
  REQUIRE(q);
  REQUIRE(q->step() == step_result::row);
  CHECK(q->integer(0).value().value() == 1);
}
TEST_CASE("C4B1 direct production callbacks gate read fetch sidecar shm and retain temp IO", "[c4b1-direct]") {
  scratch f;
  {
    auto source = f.open("source");
    sql(source, "CREATE TABLE items(n)");
    sql(source, "INSERT INTO items VALUES(1)");
  }
  observed_vfs observer;
  auto         g  = std::make_shared<guard>();
  auto         db = connection::open_gated(f.root / "source", g);
  REQUIRE(db);
  auto*         raw  = static_cast<sqlite3*>(db->private_handle());
  sqlite3_file* main = nullptr;
  REQUIRE(sqlite3_file_control(raw, "main", SQLITE_FCNTL_FILE_POINTER, &main) == SQLITE_OK);
  // FILE_POINTER is handled by SQLite above the wrapper and returns the actual production outer file.
  char header[100]{};
  REQUIRE(main->pMethods->xRead(main, header, sizeof(header), 0) == SQLITE_OK);
  CHECK(std::string_view(header, 15) == "SQLite format 3");
  int reads = observer.reads, fetches = observer.fetches;
  g->status = read_gate_status::refused;
  std::fill(std::begin(header), std::end(header), '!');
  CHECK(main->pMethods->xRead(main, header, sizeof(header), 0) != SQLITE_OK);
  CHECK(std::ranges::all_of(header, [](char c) { return c == '!'; }));
  CHECK(observer.reads == reads);
  void* mapping = reinterpret_cast<void*>(1);
  REQUIRE(main->pMethods->iVersion >= 3);
  REQUIRE(main->pMethods->xFetch);
  CHECK(main->pMethods->xFetch(main, 0, 100, &mapping) != SQLITE_OK);
  CHECK(mapping == nullptr);
  CHECK(observer.fetches == fetches);
  void volatile* shm = reinterpret_cast<void*>(1);
  CHECK(main->pMethods->xShmMap(main, 0, 32768, 0, &shm) != SQLITE_OK);
  CHECK(shm == nullptr);
  CHECK(db->gate_status() == read_gate_status::refused);
  sqlite3_vfs* v = nullptr;
  for (auto* p = sqlite3_vfs_find(nullptr); p; p = p->pNext)
    if (std::string_view(p->zName).starts_with("planar-read-"))
      v = p;
  REQUIRE(v);
  std::vector<std::max_align_t> storage((v->szOsFile + sizeof(std::max_align_t) - 1) / sizeof(std::max_align_t));
  auto*                         file  = reinterpret_cast<sqlite3_file*>(storage.data());
  int                           flags = 0;
  int                           opens = observer.opens;
  for (auto type : {SQLITE_OPEN_MAIN_JOURNAL, SQLITE_OPEN_WAL}) {
    CHECK(v->xOpen(v, (f.root / "source-journal").c_str(), file, type | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, &flags) !=
          SQLITE_OK);
    CHECK(observer.opens == opens);
    CHECK(file->pMethods == nullptr);
  }
  REQUIRE(v->xOpen(v, nullptr, file, SQLITE_OPEN_TEMP_DB | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_DELETEONCLOSE,
                   &flags) == SQLITE_OK);
  REQUIRE(file->pMethods->xWrite(file, "temp", 4, 0) == SQLITE_OK);
  char temp[4]{};
  REQUIRE(file->pMethods->xRead(file, temp, 4, 0) == SQLITE_OK);
  CHECK(std::string_view(temp, 4) == "temp");
  REQUIRE(file->pMethods->xTruncate(file, 0) == SQLITE_OK);
  CHECK(file->pMethods->xClose(file) == SQLITE_OK);
}
namespace {
struct receipt_guard : read_guard {
  profile_capture              capture;
  int                          fd;
  std::optional<profile_error> original;
  receipt_guard(profile_capture c, int f) : capture(std::move(c)), fd(f) {
  }
  ~receipt_guard() {
    ::close(fd);
  }
  read_gate_status verify() override {
    auto result = capture.verify_file("extension.ledger", fd);
    if (result)
      return read_gate_status::ready;
    original.emplace(std::move(result.error()));
    return read_gate_status::refused;
  }
};
std::shared_ptr<receipt_guard> capture_ledger(scratch& f) {
  std::filesystem::create_directory(f.root / "trusted");
  std::ofstream(f.root / "trusted/profile.json") << R"({"version":1,"modules":[]})";
  auto c = profile_capture::open((f.root / "trusted/profile.json").string());
  REQUIRE(c);
  REQUIRE(c->admit_file("extension.ledger", (f.root / "source").string(), admission_file_role::ledger));
  int fd = ::open((f.root / "source").c_str(), O_RDONLY | O_NOFOLLOW);
  REQUIRE(fd >= 0);
  return std::make_shared<receipt_guard>(std::move(*c), fd);
}
} // namespace
TEST_CASE("C4B1 original profile receipt and held descriptor bind actual gated reads", "[c4b1-receipt]") {
  for (int kind = 0; kind < 4; ++kind) {
    scratch f;
    {
      auto source = f.open("source");
      sql(source, "CREATE TABLE items(n)");
      sql(source, "INSERT INTO items VALUES(1)");
    }
    auto         g = capture_ledger(f);
    observed_vfs observer;
    if (kind == 1)
      std::ofstream(f.root / "trusted/profile.json", std::ios::app) << " ";
    if (kind == 2) {
      std::filesystem::copy_file(f.root / "source", f.root / "other");
      ::close(g->fd);
      g->fd = ::open((f.root / "other").c_str(), O_RDONLY | O_NOFOLLOW);
      REQUIRE(g->fd >= 0);
    }
    if (kind == 3) {
      std::filesystem::rename(f.root / "trusted", f.root / "old-trusted");
      std::filesystem::create_directory(f.root / "trusted");
      std::filesystem::copy_file(f.root / "old-trusted/profile.json", f.root / "trusted/profile.json");
    }
    auto db = connection::open_gated(f.root / "source", g);
    if (kind == 0) {
      REQUIRE(db);
      auto q = db->prepare("SELECT n FROM items");
      REQUIRE(q);
      REQUIRE(q->step() == step_result::row);
      CHECK(q->integer(0).value().value() == 1);
      CHECK(observer.reads > 0);
      CHECK_FALSE(g->original);
    } else {
      REQUIRE_FALSE(db);
      CHECK(db.error().gate_status == read_gate_status::refused);
      CHECK(observer.reads == 0);
      REQUIRE(g->original);
      CHECK(g->original->detail.code == "capture_changed");
    }
  }
}
TEST_CASE("C4B1 actual native owner allocation failures release all VFS state", "[c4b1-new-alloc]") {
  scratch f;
  {
    auto source = f.open("source");
    sql(source, "CREATE TABLE items(n)");
  }
  observed_vfs observer;
  auto         g              = std::make_shared<guard>();
  bool         context_failed = false;
  int          thrown         = 0;
  for (int n = 0; n < 16; ++n) {
    bool threw   = false;
    c4_new_after = n;
    try {
      auto c = connection::open_gated(f.root / "source", g);
    } catch (const std::bad_alloc&) {
      threw = true;
    }
    c4_new_after = -1;
    if (threw) {
      ++thrown;
      if (c4_failed_request == private_context_size())
        context_failed = true;
    }
    CHECK(g.use_count() == 1);
    for (auto* p = sqlite3_vfs_find(nullptr); p; p = p->pNext)
      CHECK_FALSE(std::string_view(p->zName).starts_with("planar-read-"));
  }
  CHECK(thrown > 0);
  CHECK(context_failed);
  auto c = connection::open_gated(f.root / "source", g);
  REQUIRE(c);
}
namespace {
sqlite3_mem_methods c4_sqlite_allocator{};
int                 c4_sqlite_after = -1, c4_sqlite_failures = 0;
void*               c4_malloc(int n) {
  if (c4_sqlite_after == 0) {
    ++c4_sqlite_failures;
    return nullptr;
  }
  if (c4_sqlite_after > 0)
    --c4_sqlite_after;
  return c4_sqlite_allocator.xMalloc(n);
}
void* c4_realloc(void* p, int n) {
  if (c4_sqlite_after == 0) {
    ++c4_sqlite_failures;
    return nullptr;
  }
  if (c4_sqlite_after > 0)
    --c4_sqlite_after;
  return c4_sqlite_allocator.xRealloc(p, n);
}
} // namespace
TEST_CASE("C4B1 actual SQLite allocation failures close partial opens before VFS removal", "[c4b1-open-alloc]") {
  REQUIRE(sqlite3_config(SQLITE_CONFIG_GETMALLOC, &c4_sqlite_allocator) == SQLITE_OK);
  auto methods     = c4_sqlite_allocator;
  methods.xMalloc  = c4_malloc;
  methods.xRealloc = c4_realloc;
  REQUIRE(sqlite3_config(SQLITE_CONFIG_MALLOC, &methods) == SQLITE_OK);
  scratch f;
  {
    auto source = f.open("source");
    sql(source, "CREATE TABLE items(n)");
  }
  observed_vfs observer;
  auto         g        = std::make_shared<guard>();
  int          failures = 0;
  for (int n = 0; n < 24; ++n) {
    c4_sqlite_after = n;
    auto c          = connection::open_gated(f.root / "source", g);
    c4_sqlite_after = -1;
    if (!c)
      ++failures;
    c = std::unexpected(sqlite_error{0, 0, {}});
    CHECK(g.use_count() == 1);
    for (auto* p = sqlite3_vfs_find(nullptr); p; p = p->pNext)
      CHECK_FALSE(std::string_view(p->zName).starts_with("planar-read-"));
  }
  CHECK(failures > 0);
  CHECK(c4_sqlite_failures > 0);
  auto c = connection::open_gated(f.root / "source", g);
  REQUIRE(c);
}
TEST_CASE("C4B1 actual WAL sidecar open refuses before base access", "[c4b1-direct]") {
  scratch f;
  {
    auto source = f.open("source");
    sql(source, "CREATE TABLE items(n)");
    sql(source, "INSERT INTO items VALUES(1)");
  }
  sqlite3* writer = nullptr;
  REQUIRE(sqlite3_open((f.root / "source").c_str(), &writer) == SQLITE_OK);
  REQUIRE(sqlite3_exec(writer, "PRAGMA journal_mode=WAL; INSERT INTO items VALUES(2)", nullptr, nullptr, nullptr) == SQLITE_OK);
  REQUIRE(std::filesystem::exists(f.root / "source-wal"));
  {
    observed_vfs observer;
    auto         c = connection::open_gated(f.root / "source", std::make_shared<guard>());
    REQUIRE(c);
    auto q = c->prepare("SELECT count(*) FROM items");
    if (q)
      CHECK_FALSE(q->step());
    else
      CHECK(q.error().gate_status == read_gate_status::sidecar);
    CHECK(c->gate_status() == read_gate_status::sidecar);
    CHECK(observer.sidecars == 0);
  }
  {
    observed_vfs observer;
    auto         c = connection::open(f.root / "source", open_mode::read_only);
    REQUIRE(c);
    auto q = c->prepare("SELECT count(*) FROM items");
    REQUIRE(q);
    REQUIRE(q->step() == step_result::row);
    CHECK(q->integer(0).value().value() == 2);
    CHECK(observer.sidecars > 0);
  }
  REQUIRE(sqlite3_close(writer) == SQLITE_OK);
}
