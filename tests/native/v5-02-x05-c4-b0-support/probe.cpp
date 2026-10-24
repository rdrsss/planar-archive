// Test-only cooperative SQLite original-receipt gate; never linked into Planar.
#include <catch2/catch_test_macros.hpp>
#include <fcntl.h>
#include <sqlite3.h>
#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.engine.capture.profile;
using namespace planar::engine;
extern "C" void          planar_admission_watch(std::uint64_t, std::uint64_t);
extern "C" std::uint64_t planar_admission_read_attempts();
namespace {
struct fixture {
  std::filesystem::path root, db;
  fixture() {
    char p[] = "/tmp/planar-c4b0-XXXXXX";
    root     = std::filesystem::canonical(::mkdtemp(p));
    std::filesystem::create_directory(root / "trusted");
    std::ofstream(root / "trusted/profile.json") << R"({"version":1,"modules":[]})";
    db         = root / "ledger.db";
    sqlite3* c = nullptr;
    REQUIRE(sqlite3_open(db.c_str(), &c) == SQLITE_OK);
    REQUIRE(sqlite3_exec(c,
                         "CREATE TABLE t(x); WITH RECURSIVE n(x) AS (VALUES(1) UNION ALL SELECT x+1 FROM n WHERE x<3000) INSERT "
                         "INTO t SELECT printf('%01000d',x) FROM n",
                         nullptr, nullptr, nullptr) == SQLITE_OK);
    REQUIRE(sqlite3_close(c) == SQLITE_OK);
  }
  ~fixture() {
    std::filesystem::remove_all(root);
  }
  profile_capture capture() {
    auto p = profile_capture::open((root / "trusted/profile.json").string());
    REQUIRE(p);
    REQUIRE(p->admit_file("extension.ledger", db.string(), admission_file_role::ledger));
    return std::move(*p);
  }
  void replace() {
    std::filesystem::rename(db, root / "original.db");
    std::filesystem::copy_file(root / "original.db", db);
  }
};
struct owner;
struct file {
  sqlite3_file  base{};
  sqlite3_file* inner   = nullptr;
  owner*        context = nullptr;
  bool          main    = false;
};
struct owner {
  profile_capture              capture;
  std::optional<profile_error> original_failure;
  static_assert(std::is_nothrow_move_constructible_v<profile_error>);
  std::string           path, name;
  sqlite3_vfs           vfs{};
  sqlite3_vfs*          inner = nullptr;
  sqlite3*              db    = nullptr;
  int                   fd    = -1;
  int                   reads = 0, fetches = 0, mapped = 0, raw_reads = 0, open_main = 0, open_aux = 0, closed = 0, shm = 0;
  int                   first = 0, moved_override = -1;
  bool                  bypass = false, throw_gate = false, fail_open = false, registered = false;
  int                   fetch_attempts = 0;
  std::function<void()> after_open;
  std::function<void()> before_fetch;
  explicit owner(fixture& f) : capture(f.capture()), path(f.db.string()) {
    name  = std::format("c4b0-{}", reinterpret_cast<std::uintptr_t>(this));
    inner = sqlite3_vfs_find(nullptr);
    REQUIRE(inner);
    vfs               = *inner;
    vfs.zName         = name.c_str();
    vfs.pAppData      = this;
    vfs.pNext         = nullptr;
    vfs.szOsFile      = sizeof(file) + alignof(std::max_align_t) + inner->szOsFile;
    vfs.xOpen         = open;
    vfs.xDelete       = del;
    vfs.xAccess       = access;
    vfs.xFullPathname = full;
    // The remaining VFS callbacks forward using the real VFS argument below.
    vfs.xDlOpen           = dlopen;
    vfs.xDlError          = dlerror;
    vfs.xDlSym            = dlsym;
    vfs.xDlClose          = dlclose;
    vfs.xRandomness       = random;
    vfs.xSleep            = sleep;
    vfs.xCurrentTime      = time;
    vfs.xGetLastError     = error;
    vfs.xCurrentTimeInt64 = time64;
    vfs.xSetSystemCall    = nullptr;
    vfs.xGetSystemCall    = nullptr;
    vfs.xNextSystemCall   = nullptr;
    REQUIRE(sqlite3_vfs_register(&vfs, 0) == SQLITE_OK);
    registered = true;
    fd         = ::open(path.c_str(), O_RDONLY | O_NOFOLLOW);
    REQUIRE(fd >= 0);
  }
  int finish(bool controlled_unregister_failure = false) noexcept {
    if (db) {
      int rc = sqlite3_close(db);
      if (rc != SQLITE_OK)
        return rc;
      db = nullptr;
    }
    if (registered) {
      if (controlled_unregister_failure)
        return SQLITE_ERROR;
      int rc = sqlite3_vfs_unregister(&vfs);
      if (rc != SQLITE_OK)
        return rc;
      registered = false;
    }
    return SQLITE_OK;
  }
  ~owner() {
    REQUIRE(finish() == SQLITE_OK);
    if (fd >= 0)
      ::close(fd);
  }
  int reject(int rc) {
    if (!first)
      first = rc;
    return rc;
  }
  bool receipt() noexcept {
    try {
      if (first)
        return false;
      if (throw_gate)
        throw std::bad_alloc();
      auto verified = capture.verify_file("extension.ledger", fd);
      if (!verified) {
        original_failure.emplace(std::move(verified.error()));
        reject(SQLITE_IOERR_READ);
        return false;
      }
      return true;
    } catch (...) {
      reject(SQLITE_NOMEM);
      return false;
    }
  }
  bool gate(file* f) noexcept {
    if (bypass)
      return true;
    if (!receipt())
      return false;
    int moved = 0;
    int rc    = moved_override < 0 ? f->inner->pMethods->xFileControl(f->inner, SQLITE_FCNTL_HAS_MOVED, &moved) : moved_override;
    if (moved_override == 999) {
      rc    = SQLITE_OK;
      moved = 1;
    }
    if (rc != SQLITE_OK || moved) {
      reject(SQLITE_IOERR_READ);
      return false;
    }
    return true;
  }
  int raw() {
    if (!sidecars())
      return reject(SQLITE_CANTOPEN);
    if (!receipt())
      return first;
    char header[100];
    ++raw_reads;
    return ::pread(fd, header, sizeof header, 0) == sizeof header ? SQLITE_OK : SQLITE_IOERR_READ;
  }
  bool sidecars() {
    for (auto suffix : {"-journal", "-wal", "-shm"}) {
      struct stat st{};
      if (::lstat((path + suffix).c_str(), &st) == 0)
        return false;
    }
    return true;
  }
  int connect() {
    if (!sidecars())
      return reject(SQLITE_CANTOPEN);
    return sqlite3_open_v2(path.c_str(), &db, SQLITE_OPEN_READONLY, name.c_str());
  }
  static owner& ctx(sqlite3_vfs* v) {
    return *static_cast<owner*>(v->pAppData);
  }
  static file* cast(sqlite3_file* f) {
    return reinterpret_cast<file*>(f);
  }
  static int open(sqlite3_vfs* v, const char* z, sqlite3_file* p, int flags, int* out) noexcept {
    auto& o      = ctx(v);
    auto* f      = new (p) file;
    f->context   = &o;
    f->main      = flags & SQLITE_OPEN_MAIN_DB;
    auto address = reinterpret_cast<std::uintptr_t>(f + 1);
    address      = (address + alignof(std::max_align_t) - 1) & ~(alignof(std::max_align_t) - 1);
    f->inner     = reinterpret_cast<sqlite3_file*>(address);
    if (flags & (SQLITE_OPEN_MAIN_JOURNAL | SQLITE_OPEN_WAL)) {
      ++o.open_aux;
      return o.reject(SQLITE_CANTOPEN);
    }
    if (f->main)
      ++o.open_main;
    if (o.fail_open)
      return o.reject(SQLITE_NOMEM);
    int rc = o.inner->xOpen(o.inner, z, f->inner, flags, out);
    if (rc != SQLITE_OK)
      return rc;
    f->base.pMethods = &methods;
    try {
      if (f->main && o.after_open)
        o.after_open();
    } catch (...) {
      return o.reject(SQLITE_NOMEM);
    }
    return SQLITE_OK;
  }
  static int close(sqlite3_file* p) noexcept {
    auto* f = cast(p);
    ++f->context->closed;
    int rc           = f->inner->pMethods->xClose(f->inner);
    f->base.pMethods = nullptr;
    return rc;
  }
  static int read(sqlite3_file* p, void* b, int n, sqlite3_int64 off) noexcept {
    auto* f = cast(p);
    if (f->main && !f->context->gate(f))
      return f->context->first;
    ++f->context->reads;
    return f->inner->pMethods->xRead(f->inner, b, n, off);
  }
  static int write(sqlite3_file* p, const void* b, int n, sqlite3_int64 off) noexcept {
    auto* f = cast(p);
    if (f->main)
      return SQLITE_READONLY;
    return f->inner->pMethods->xWrite(f->inner, b, n, off);
  }
  static int truncate(sqlite3_file* p, sqlite3_int64 n) noexcept {
    auto* f = cast(p);
    return f->main ? SQLITE_READONLY : f->inner->pMethods->xTruncate(f->inner, n);
  }
  static int sync(sqlite3_file* p, int n) noexcept {
    auto* f = cast(p);
    return f->inner->pMethods->xSync(f->inner, n);
  }
  static int size(sqlite3_file* p, sqlite3_int64* n) noexcept {
    auto* f = cast(p);
    return f->inner->pMethods->xFileSize(f->inner, n);
  }
  static int lock(sqlite3_file* p, int n) noexcept {
    auto* f = cast(p);
    return f->inner->pMethods->xLock(f->inner, n);
  }
  static int unlock(sqlite3_file* p, int n) noexcept {
    auto* f = cast(p);
    return f->inner->pMethods->xUnlock(f->inner, n);
  }
  static int reserved(sqlite3_file* p, int* n) noexcept {
    auto* f = cast(p);
    return f->inner->pMethods->xCheckReservedLock(f->inner, n);
  }
  static int control(sqlite3_file* p, int n, void* a) noexcept {
    auto* f = cast(p);
    return f->inner->pMethods->xFileControl(f->inner, n, a);
  }
  static int sector(sqlite3_file* p) noexcept {
    auto* f = cast(p);
    return f->inner->pMethods->xSectorSize(f->inner);
  }
  static int device(sqlite3_file* p) noexcept {
    auto* f = cast(p);
    return f->inner->pMethods->xDeviceCharacteristics(f->inner);
  }
  static int shmmap(sqlite3_file* p, int i, int s, int w, void volatile** a) noexcept {
    auto* f = cast(p);
    if (f->main) {
      ++f->context->shm;
      return f->context->reject(SQLITE_IOERR_SHMMAP);
    }
    return f->inner->pMethods->xShmMap(f->inner, i, s, w, a);
  }
  static int shmlock(sqlite3_file* p, int o, int n, int flags) noexcept {
    auto* f = cast(p);
    return f->inner->pMethods->xShmLock(f->inner, o, n, flags);
  }
  static void barrier(sqlite3_file* p) noexcept {
    auto* f = cast(p);
    f->inner->pMethods->xShmBarrier(f->inner);
  }
  static int unmap(sqlite3_file* p, int d) noexcept {
    auto* f = cast(p);
    return f->inner->pMethods->xShmUnmap(f->inner, d);
  }
  static int fetch(sqlite3_file* p, sqlite3_int64 off, int n, void** a) noexcept {
    auto* f = cast(p);
    ++f->context->fetch_attempts;
    try {
      if (f->main && f->context->before_fetch)
        f->context->before_fetch();
    } catch (...) {
      *a = nullptr;
      return f->context->reject(SQLITE_NOMEM);
    }
    *a = nullptr;
    if (f->main && !f->context->gate(f))
      return f->context->first;
    ++f->context->fetches;
    auto* m = f->inner->pMethods;
    if (m->iVersion < 3 || !m->xFetch)
      return SQLITE_OK;
    int rc = m->xFetch(f->inner, off, n, a);
    if (*a)
      ++f->context->mapped;
    return rc;
  }
  static int unfetch(sqlite3_file* p, sqlite3_int64 off, void* a) noexcept {
    auto* f = cast(p);
    auto* m = f->inner->pMethods;
    return m->iVersion >= 3 && m->xUnfetch ? m->xUnfetch(f->inner, off, a) : SQLITE_OK;
  }
  static int del(sqlite3_vfs* v, const char* z, int n) {
    auto& o = ctx(v);
    return o.inner->xDelete(o.inner, z, n);
  }
  static int access(sqlite3_vfs* v, const char* z, int n, int* a) {
    auto& o = ctx(v);
    return o.inner->xAccess(o.inner, z, n, a);
  }
  static int full(sqlite3_vfs* v, const char* z, int n, char* a) {
    auto& o = ctx(v);
    return o.inner->xFullPathname(o.inner, z, n, a);
  }
  static void* dlopen(sqlite3_vfs* v, const char* z) {
    auto& o = ctx(v);
    return o.inner->xDlOpen(o.inner, z);
  }
  static void dlerror(sqlite3_vfs* v, int n, char* z) {
    auto& o = ctx(v);
    o.inner->xDlError(o.inner, n, z);
  }
  static void (*dlsym(sqlite3_vfs* v, void* h, const char* z))(void) {
    auto& o = ctx(v);
    return o.inner->xDlSym(o.inner, h, z);
  }
  static void dlclose(sqlite3_vfs* v, void* h) {
    auto& o = ctx(v);
    o.inner->xDlClose(o.inner, h);
  }
  static int random(sqlite3_vfs* v, int n, char* z) {
    auto& o = ctx(v);
    return o.inner->xRandomness(o.inner, n, z);
  }
  static int sleep(sqlite3_vfs* v, int n) {
    auto& o = ctx(v);
    return o.inner->xSleep(o.inner, n);
  }
  static int time(sqlite3_vfs* v, double* n) {
    auto& o = ctx(v);
    return o.inner->xCurrentTime(o.inner, n);
  }
  static int error(sqlite3_vfs* v, int n, char* z) {
    auto& o = ctx(v);
    return o.inner->xGetLastError(o.inner, n, z);
  }
  static int time64(sqlite3_vfs* v, sqlite3_int64* n) {
    auto& o = ctx(v);
    return o.inner->xCurrentTimeInt64(o.inner, n);
  }
  static const sqlite3_io_methods methods;
};
const sqlite3_io_methods owner::methods = {3,       close,  read,   write,  truncate, sync,    size,  lock,  unlock, reserved,
                                           control, sector, device, shmmap, shmlock,  barrier, unmap, fetch, unfetch};
int                      query(owner& o) {
  return sqlite3_exec(o.db, "SELECT sum(length(x)) FROM t", nullptr, nullptr, nullptr);
}
file* mainfile(owner& o) {
  sqlite3_file* f = nullptr;
  REQUIRE(sqlite3_file_control(o.db, "main", SQLITE_FCNTL_FILE_POINTER, &f) == SQLITE_OK);
  return reinterpret_cast<file*>(f);
}
} // namespace
TEST_CASE("C4B0 actual healthy open and unguarded open consume header before return", "[c4b0]") {
  fixture f;
  auto*   default_vfs = sqlite3_vfs_find(nullptr);
  {
    owner o(f);
    o.bypass = true;
    REQUIRE(o.connect() == SQLITE_OK);
    CHECK(o.reads > 0);
    REQUIRE(query(o) == SQLITE_OK);
  }
  {
    owner o(f);
    REQUIRE(o.raw() == SQLITE_OK);
    CHECK(o.raw_reads == 1);
    REQUIRE(o.connect() == SQLITE_OK);
    CHECK(o.reads > 0);
    REQUIRE(query(o) == SQLITE_OK);
    CHECK(o.first == 0);
  }
  CHECK(sqlite3_vfs_find(nullptr) == default_vfs);
}
TEST_CASE("C4B0 original held descriptor gates raw header and SQLite first read", "[c4b0]") {
  for (int stage = 0; stage < 3; ++stage) {
    fixture f;
    owner   o(f);
    if (stage < 2)
      f.replace();
    else
      o.after_open = [&] { f.replace(); };
    if (stage == 0) {
      CHECK(o.raw() != SQLITE_OK);
      CHECK(o.raw_reads == 0);
    } else {
      CHECK(o.connect() != SQLITE_OK);
      CHECK(o.reads == 0);
      CHECK(o.first != 0);
      REQUIRE(o.original_failure);
      CHECK(o.original_failure->detail.code == "capture_changed");
      auto retained = o.original_failure->field;
      o.throw_gate  = true;
      CHECK_FALSE(o.receipt());
      CHECK(o.original_failure->field == retained);
    }
  }
}
TEST_CASE("C4B0 afteropen move and metadata mutation refuse next actual read", "[c4b0]") {
  for (int stage = 0; stage < 2; ++stage) {
    fixture f;
    owner   o(f);
    REQUIRE(o.connect() == SQLITE_OK);
    auto* fptr   = mainfile(o);
    int   before = o.reads;
    if (stage == 0)
      std::filesystem::rename(f.db, f.root / "moved.db");
    else
      REQUIRE(::chmod(f.db.c_str(), 0600) == 0);
    char buf[100];
    CHECK(owner::read(&fptr->base, buf, 100, 0) != SQLITE_OK);
    CHECK(o.reads == before);
    CHECK_FALSE(o.capture.verify());
  }
}
TEST_CASE("C4B0 HAS_MOVED unavailable error moved all fail closed", "[c4b0]") {
  for (int rc : {SQLITE_NOTFOUND, SQLITE_IOERR, 999}) {
    fixture f;
    owner   o(f);
    o.moved_override = rc;
    CHECK(o.connect() != SQLITE_OK);
    CHECK(o.reads == 0);
    CHECK(o.first != 0);
  }
}
TEST_CASE("C4B0 actual mmap positive then original receipt refusal gates xFetch", "[c4b0]") {
  fixture f;
  owner   o(f);
  REQUIRE(o.connect() == SQLITE_OK);
  REQUIRE(sqlite3_exec(o.db, "PRAGMA mmap_size=67108864", nullptr, nullptr, nullptr) == SQLITE_OK);
  REQUIRE(query(o) == SQLITE_OK);
  std::cout << "actual mmap fetch delegates=" << o.fetches << " nonnull=" << o.mapped << "\n";
  REQUIRE(o.mapped > 0);
  auto* fp       = mainfile(o);
  int   before   = o.fetches;
  bool  fired    = false;
  o.before_fetch = [&] {
    if (!fired) {
      fired = true;
      f.replace();
    }
  };
  int attempts = o.fetch_attempts;
  CHECK(query(o) != SQLITE_OK);
  CHECK(o.fetch_attempts > attempts);
  CHECK(o.fetches == before);
  std::cout << "actual refused mmap callbacks=" << o.fetch_attempts - attempts << " delegated delta=" << o.fetches - before
            << "\n";
  void* p = reinterpret_cast<void*>(1);
  CHECK(owner::fetch(&fp->base, 4096, 4096, &p) != SQLITE_OK);
  CHECK(p == nullptr);
  CHECK(o.fetches == before);
}
TEST_CASE("C4B0 main journal WAL shm refuse but unrelated temp delegates", "[c4b0]") {
  fixture f;
  owner   o(f);
  REQUIRE(o.connect() == SQLITE_OK);
  auto*          fp = mainfile(o);
  void volatile* p  = nullptr;
  CHECK(owner::shmmap(&fp->base, 0, 32768, 1, &p) == SQLITE_IOERR_SHMMAP);
  CHECK(o.shm == 1);
  for (int flag : {SQLITE_OPEN_MAIN_JOURNAL, SQLITE_OPEN_WAL}) {
    std::vector<std::max_align_t> a((o.vfs.szOsFile + sizeof(std::max_align_t) - 1) / sizeof(std::max_align_t));
    CHECK(owner::open(&o.vfs, (o.path + "-journal").c_str(), reinterpret_cast<sqlite3_file*>(a.data()),
                      flag | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) == SQLITE_CANTOPEN);
  }
  std::vector<std::max_align_t> a((o.vfs.szOsFile + sizeof(std::max_align_t) - 1) / sizeof(std::max_align_t));
  auto*                         t = reinterpret_cast<sqlite3_file*>(a.data());
  REQUIRE(owner::open(&o.vfs, nullptr, t,
                      SQLITE_OPEN_TEMP_DB | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_DELETEONCLOSE,
                      nullptr) == SQLITE_OK);
  CHECK(t->pMethods->xClose(t) == SQLITE_OK);
  CHECK(o.open_aux == 2);
}
TEST_CASE("C4B0 multiple owners partial failure and callback allocation cleanup", "[c4b0]") {
  fixture a, b;
  auto*   def = sqlite3_vfs_find(nullptr);
  {
    owner x(a);
    owner y(b);
    REQUIRE(x.name != y.name);
    REQUIRE(x.connect() == SQLITE_OK);
    REQUIRE(y.connect() == SQLITE_OK);
    REQUIRE(query(y) == SQLITE_OK);
    a.replace();
    CHECK(query(x) != SQLITE_OK);
    CHECK(query(y) == SQLITE_OK);
  }
  for (int stage = 0; stage < 3; ++stage) {
    fixture f;
    owner   o(f);
    if (stage == 0)
      o.fail_open = true;
    else if (stage == 1)
      o.throw_gate = true;
    else
      o.after_open = [] { throw std::bad_alloc(); };
    CHECK(o.connect() != SQLITE_OK);
    CHECK(o.reads == 0);
    CHECK(o.first == SQLITE_NOMEM);
    REQUIRE(sqlite3_close(o.db) == SQLITE_OK);
    o.db = nullptr;
    CHECK(o.closed == (stage == 0 ? 0 : 1));
  }
  CHECK(sqlite3_vfs_find(nullptr) == def);
}

TEST_CASE("C4B0 interposed syscall controls and original alias refusal", "[c4b0]") {
  fixture     f;
  owner       o(f);
  struct stat st{};
  REQUIRE(::fstat(o.fd, &st) == 0);
  planar_admission_watch(st.st_dev, st.st_ino);
  REQUIRE(o.capture.verify_file("extension.ledger", o.fd));
  CHECK(planar_admission_read_attempts() == 0);
  REQUIRE(o.raw() == SQLITE_OK);
  CHECK(planar_admission_read_attempts() == 1);
  f.replace();
  planar_admission_watch(st.st_dev, st.st_ino);
  CHECK(o.raw() != SQLITE_OK);
  CHECK(planar_admission_read_attempts() == 0);
  planar_admission_watch(0, 0);
  fixture g;
  std::filesystem::remove(g.db);
  std::filesystem::create_hard_link(g.root / "trusted/profile.json", g.db);
  auto capture = profile_capture::open((g.root / "trusted/profile.json").string());
  REQUIRE(capture);
  CHECK_FALSE(capture->admit_file("extension.ledger", g.db.string(), admission_file_role::ledger));
}
TEST_CASE("C4B0 existing sidecars refuse before any main content", "[c4b0]") {
  for (auto suffix : {"-journal", "-wal", "-shm"}) {
    fixture f;
    owner   o(f);
    std::ofstream(o.path + suffix) << "unrelated sentinel";
    CHECK(o.raw() != SQLITE_OK);
    CHECK(o.connect() != SQLITE_OK);
    CHECK(o.raw_reads == 0);
    CHECK(o.reads == 0);
    CHECK(o.open_main == 0);
  }
}
TEST_CASE("C4B0 journal arriving after precheck uses actual SQLite refused open", "[c4b0]") {
  fixture f;
  owner   o(f);
  o.after_open = [&] { std::ofstream(o.path + "-journal") << std::string(1024, 'x'); };
  int rc       = o.connect();
  if (rc == SQLITE_OK)
    rc = query(o);
  std::cout << "late journal SQLite rc=" << rc << " refused opens=" << o.open_aux << "\n";
  CHECK(rc != SQLITE_OK);
  CHECK(o.open_aux > 0);
}

TEST_CASE("C4B0 WAL arriving after precheck uses actual SQLite refused open", "[c4b0]") {
  fixture  f;
  sqlite3* c = nullptr;
  REQUIRE(sqlite3_open(f.db.c_str(), &c) == SQLITE_OK);
  REQUIRE(sqlite3_exec(c, "PRAGMA journal_mode=WAL", nullptr, nullptr, nullptr) == SQLITE_OK);
  REQUIRE(sqlite3_close(c) == SQLITE_OK);
  owner o(f);
  o.after_open = [&] { std::ofstream(o.path + "-wal") << std::string(1024, 'x'); };
  int rc       = o.connect();
  if (rc == SQLITE_OK)
    rc = query(o);
  std::cout << "late WAL SQLite rc=" << rc << " refused opens=" << o.open_aux << "\n";
  CHECK(rc != SQLITE_OK);
  CHECK(o.open_aux > 0);
}
TEST_CASE("C4B0 actual first header failure has no hidden content syscall", "[c4b0]") {
  fixture     f;
  owner       o(f);
  struct stat st{};
  REQUIRE(::fstat(o.fd, &st) == 0);
  o.after_open = [&] {
    f.replace();
    planar_admission_watch(st.st_dev, st.st_ino);
  };
  CHECK(o.connect() != SQLITE_OK);
  CHECK(o.open_main == 1);
  CHECK(o.reads == 0);
  CHECK(planar_admission_read_attempts() == 0);
  planar_admission_watch(0, 0);
}

TEST_CASE("C4B0 close BUSY and controlled unregister failure retain context until retry", "[c4b0]") {
  fixture f;
  owner   o(f);
  REQUIRE(o.connect() == SQLITE_OK);
  sqlite3_stmt* statement = nullptr;
  REQUIRE(sqlite3_prepare_v2(o.db, "SELECT x FROM t", -1, &statement, nullptr) == SQLITE_OK);
  CHECK(o.finish() == SQLITE_BUSY);
  CHECK(o.db != nullptr);
  CHECK(sqlite3_vfs_find(o.name.c_str()) == &o.vfs);
  CHECK(sqlite3_step(statement) == SQLITE_ROW);
  REQUIRE(sqlite3_finalize(statement) == SQLITE_OK);
  CHECK(o.finish(true) == SQLITE_ERROR);
  CHECK(o.db == nullptr);
  CHECK(o.closed == 1);
  CHECK(o.registered);
  CHECK(sqlite3_vfs_find(o.name.c_str()) == &o.vfs);
  CHECK(o.raw() == SQLITE_OK);
  CHECK(o.finish() == SQLITE_OK);
  CHECK_FALSE(o.registered);
  CHECK(sqlite3_vfs_find(o.name.c_str()) == nullptr);
}
