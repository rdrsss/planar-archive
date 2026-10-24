// Independent verification of read-snapshot lifetime after a real SQLite read failure.
#include <catch2/catch_test_macros.hpp>
#include <sqlite3.h>
#include <unistd.h>
import std;
import planar.engine.ledger.format;
using namespace planar::engine::ledger;
using namespace planar::engine::history;
namespace {
struct fault_vfs {
  struct file {
    sqlite3_file  base;
    sqlite3_file* real;
    fault_vfs*    owner;
    bool          journal;
  };
  static inline fault_vfs* active   = nullptr;
  sqlite3_vfs*             original = sqlite3_vfs_find(nullptr);
  sqlite3_vfs              wrapper  = *original;
  sqlite3_io_methods       methods{};
  bool                     armed = false;
  int                      hits  = 0;
  fault_vfs() {
    REQUIRE_FALSE(active);
    active           = this;
    methods.iVersion = 1;
    methods.xClose   = [](sqlite3_file* p) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xClose(f.real);
    };
    methods.xRead = [](sqlite3_file* p, void* b, int n, sqlite3_int64 o) {
      auto& f = *reinterpret_cast<file*>(p);
      if (f.owner->armed && !f.journal) {
        f.owner->armed = false;
        ++f.owner->hits;
        std::println("Independent injected SQLITE_IOERR_READ offset={} bytes={}", o, n);
        return SQLITE_IOERR_READ;
      }
      return f.real->pMethods->xRead(f.real, b, n, o);
    };
    methods.xWrite = [](sqlite3_file* p, const void* b, int n, sqlite3_int64 o) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xWrite(f.real, b, n, o);
    };
    methods.xTruncate = [](sqlite3_file* p, sqlite3_int64 n) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xTruncate(f.real, n);
    };
    methods.xSync = [](sqlite3_file* p, int flags) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xSync(f.real, flags);
    };
    methods.xFileSize = [](sqlite3_file* p, sqlite3_int64* n) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xFileSize(f.real, n);
    };
    methods.xLock = [](sqlite3_file* p, int n) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xLock(f.real, n);
    };
    methods.xUnlock = [](sqlite3_file* p, int n) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xUnlock(f.real, n);
    };
    methods.xCheckReservedLock = [](sqlite3_file* p, int* n) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xCheckReservedLock(f.real, n);
    };
    methods.xFileControl = [](sqlite3_file* p, int op, void* arg) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xFileControl(f.real, op, arg);
    };
    methods.xSectorSize = [](sqlite3_file* p) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xSectorSize(f.real);
    };
    methods.xDeviceCharacteristics = [](sqlite3_file* p) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xDeviceCharacteristics(f.real);
    };
    wrapper.zName    = "planar-r13b1c01-snapshot-test";
    wrapper.szOsFile = static_cast<int>(sizeof(file) + alignof(std::max_align_t) + original->szOsFile);
    wrapper.xOpen    = [](sqlite3_vfs*, const char* name, sqlite3_file* p, int flags, int* out) {
      auto& f       = *reinterpret_cast<file*>(p);
      auto  address = reinterpret_cast<std::uintptr_t>(p) + sizeof(file);
      address       = (address + alignof(std::max_align_t) - 1) & ~(alignof(std::max_align_t) - 1);
      f.real        = reinterpret_cast<sqlite3_file*>(address);
      f.owner       = active;
      f.journal     = (flags & SQLITE_OPEN_MAIN_JOURNAL) != 0;
      const int rc  = active->original->xOpen(active->original, name, f.real, flags, out);
      if (rc == SQLITE_OK)
        f.base.pMethods = &active->methods;
      return rc;
    };
    REQUIRE(sqlite3_vfs_register(&wrapper, 1) == SQLITE_OK);
  }
  ~fault_vfs() {
    sqlite3_vfs_register(original, 1);
    sqlite3_vfs_unregister(&wrapper);
    active = nullptr;
  }
};

struct scratch {
  std::filesystem::path root;
  scratch() {
    char p[] = "/tmp/planar-r13b1c01-XXXXXX";
    REQUIRE(::mkdtemp(p));
    root = std::filesystem::canonical(p);
  }
  ~scratch() {
    std::filesystem::remove_all(root);
  }
};
} // namespace
TEST_CASE("ledger verified reader must not silently switch snapshots after storage read failure", "[ledger][records][snapshot]") {
  scratch f;
  auto    path = f.root / "ledger";
  REQUIRE(initialize_new(path, "review"));
  {
    auto writer = begin_records(path);
    REQUIRE(writer);
    for (int i = 0; i < 8; ++i)
      REQUIRE(writer->intern(file_record{420, std::string(1024 * 1024, char('A' + i))}));
    REQUIRE(writer->commit());
  }
  fault_vfs vfs;
  auto      reader = open_existing(path);
  REQUIRE(reader);
  const auto info  = reader->metadata();
  auto       valid = std::move(*reader);
  *reader          = std::move(valid);
  vfs.armed        = true;
  auto failed      = reader->read_record(1);
  REQUIRE(vfs.hits == 1);
  REQUIRE_FALSE(failed);
  CHECK(failed.error().code == format_error_code::io);
  std::println("First read refused as io; injecting fault is now disabled");
  {
    auto writer = begin_records(path);
    REQUIRE(writer);
    auto ref = writer->intern(file_record{420, "new after reader snapshot"});
    REQUIRE(ref);
    REQUIRE(ref->id == 9);
    auto committed = writer->commit();
    REQUIRE(committed);
    std::println("Independent writer committed new record 9 while original reader remains alive");
  }
  auto observed = reader->read_record(9);
  if (observed)
    std::println("BUG: original verified reader exposed newly committed record 9: {}",
                 std::get<file_record>(observed->value).payload);
  // The invalidated owner must refuse without starting another snapshot.
  REQUIRE_FALSE(observed);
  CHECK(observed.error().code == format_error_code::invalid_argument);
  CHECK(reader->metadata().identity == info.identity);
  CHECK(reader->metadata().workspace_id == info.workspace_id);
  CHECK(reader->metadata().created_at == info.created_at);
  auto moved = std::move(*reader);
  CHECK_FALSE(reader->read_record(1));
  auto destination = open_existing(path);
  REQUIRE(destination);
  *destination = std::move(moved);
  CHECK_FALSE(destination->read_record(1));
  CHECK_FALSE(destination->read_record(9));
  CHECK(destination->metadata().identity == info.identity);
  auto fresh = open_existing(path);
  REQUIRE(fresh);
  auto new_record = fresh->read_record(9);
  REQUIRE(new_record);
  CHECK(std::get<file_record>(new_record->value).payload == "new after reader snapshot");
}
TEST_CASE("ledger writer read failure invalidates and rolls back uncommitted prefix", "[ledger][records][snapshot]") {
  scratch f;
  auto    path = f.root / "ledger";
  REQUIRE(initialize_new(path, "review"));
  {
    auto writer = begin_records(path);
    REQUIRE(writer);
    for (int i = 0; i < 8; ++i)
      REQUIRE(writer->intern(file_record{420, std::string(1024 * 1024, char('A' + i))}));
    REQUIRE(writer->commit());
  }
  fault_vfs vfs;
  auto      writer = begin_records(path);
  REQUIRE(writer);
  auto prefix = writer->intern(file_record{420, "prefix"});
  REQUIRE(prefix);
  REQUIRE(prefix->id == 9);
  vfs.armed   = true;
  bool got_io = false;
  for (std::uint64_t id = 1; id <= 8 && vfs.hits == 0; ++id) {
    auto result = writer->read_record(id);
    if (!result)
      got_io = result.error().code == format_error_code::io;
  }
  REQUIRE(vfs.hits == 1);
  REQUIRE(got_io);
  CHECK_FALSE(writer->intern(file_record{420, "must refuse"}));
  CHECK_FALSE(writer->commit());
  auto reader = open_existing(path);
  REQUIRE(reader);
  CHECK(reader->read_record(1));
  CHECK_FALSE(reader->read_record(9));
}
