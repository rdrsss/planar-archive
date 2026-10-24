// Serial test-only base VFS: counts real delegates and supplies controlled platform failures.
struct observed_vfs {
  struct file {
    sqlite3_file       base{};
    sqlite3_file*      inner = nullptr;
    observed_vfs*      owner = nullptr;
    sqlite3_io_methods methods{};
    bool               main = false;
  };
  sqlite3_vfs* inner = sqlite3_vfs_find(nullptr);
  sqlite3_vfs  vfs   = *inner;
  int          reads = 0, fetches = 0, mapped = 0, moves = 0, closes = 0, opens = 0, sidecars = 0;
  int          moved_code = SQLITE_OK, moved_value = 0;
  bool         partial_open  = false;
  int          io_version    = 3;
  bool         optional_null = false;
  observed_vfs() {
    vfs.zName    = "c4b1-observer";
    vfs.pAppData = this;
    vfs.pNext    = nullptr;
    vfs.szOsFile += sizeof(file) + alignof(std::max_align_t);
    vfs.xOpen             = open;
    vfs.xDelete           = del;
    vfs.xAccess           = access;
    vfs.xFullPathname     = full;
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
    REQUIRE(sqlite3_vfs_register(&vfs, 1) == SQLITE_OK);
  }
  ~observed_vfs() {
    sqlite3_vfs_register(inner, 1);
    sqlite3_vfs_unregister(&vfs);
  }
  static observed_vfs& ctx(sqlite3_vfs* v) {
    auto& o = *static_cast<observed_vfs*>(v->pAppData);
    REQUIRE(v == &o.vfs);
    return o;
  }
  static file& cast(sqlite3_file* p) {
    return *reinterpret_cast<file*>(p);
  }
  static int open(sqlite3_vfs* v, const char* z, sqlite3_file* p, int flags, int* out) {
    auto& o  = ctx(v);
    auto* f  = new (p) file;
    f->owner = &o;
    f->main  = flags & SQLITE_OPEN_MAIN_DB;
    auto a   = reinterpret_cast<std::uintptr_t>(f + 1);
    a        = (a + alignof(std::max_align_t) - 1) & ~(alignof(std::max_align_t) - 1);
    f->inner = reinterpret_cast<sqlite3_file*>(a);
    ++o.opens;
    if (flags & (SQLITE_OPEN_MAIN_JOURNAL | SQLITE_OPEN_WAL))
      ++o.sidecars;
    int rc = o.inner->xOpen(o.inner, z, f->inner, flags, out);
    if (rc != SQLITE_OK)
      return rc;
    f->methods                        = *f->inner->pMethods;
    f->methods.iVersion               = std::min(o.io_version, f->methods.iVersion);
    f->methods.xClose                 = close;
    f->methods.xRead                  = read;
    f->methods.xWrite                 = write;
    f->methods.xTruncate              = truncate;
    f->methods.xSync                  = sync;
    f->methods.xFileSize              = size;
    f->methods.xLock                  = lock;
    f->methods.xUnlock                = unlock;
    f->methods.xCheckReservedLock     = reserved;
    f->methods.xFileControl           = control;
    f->methods.xSectorSize            = sector;
    f->methods.xDeviceCharacteristics = device;
    f->methods.xShmMap                = shmmap;
    f->methods.xShmLock               = shmlock;
    f->methods.xShmBarrier            = barrier;
    f->methods.xShmUnmap              = unmap;
    f->methods.xFetch                 = o.optional_null ? nullptr : fetch;
    f->methods.xUnfetch               = o.optional_null ? nullptr : unfetch;
    f->base.pMethods                  = &f->methods;
    return o.partial_open ? SQLITE_IOERR : SQLITE_OK;
  }
  static int close(sqlite3_file* p) {
    auto& f = cast(p);
    ++f.owner->closes;
    return f.inner->pMethods->xClose(f.inner);
  }
  static int read(sqlite3_file* p, void* b, int n, sqlite3_int64 off) {
    auto& f = cast(p);
    ++f.owner->reads;
    return f.inner->pMethods->xRead(f.inner, b, n, off);
  }
  static int write(sqlite3_file* p, const void* b, int n, sqlite3_int64 off) {
    auto& f = cast(p);
    return f.inner->pMethods->xWrite(f.inner, b, n, off);
  }
  static int truncate(sqlite3_file* p, sqlite3_int64 n) {
    auto& f = cast(p);
    return f.inner->pMethods->xTruncate(f.inner, n);
  }
  static int sync(sqlite3_file* p, int n) {
    auto& f = cast(p);
    return f.inner->pMethods->xSync(f.inner, n);
  }
  static int size(sqlite3_file* p, sqlite3_int64* n) {
    auto& f = cast(p);
    return f.inner->pMethods->xFileSize(f.inner, n);
  }
  static int lock(sqlite3_file* p, int n) {
    auto& f = cast(p);
    return f.inner->pMethods->xLock(f.inner, n);
  }
  static int unlock(sqlite3_file* p, int n) {
    auto& f = cast(p);
    return f.inner->pMethods->xUnlock(f.inner, n);
  }
  static int reserved(sqlite3_file* p, int* n) {
    auto& f = cast(p);
    return f.inner->pMethods->xCheckReservedLock(f.inner, n);
  }
  static int control(sqlite3_file* p, int n, void* a) {
    auto& f = cast(p);
    if (n == SQLITE_FCNTL_HAS_MOVED) {
      ++f.owner->moves;
      *static_cast<int*>(a) = f.owner->moved_value;
      if (f.owner->moved_code != SQLITE_OK || f.owner->moved_value)
        return f.owner->moved_code;
    }
    return f.inner->pMethods->xFileControl(f.inner, n, a);
  }
  static int sector(sqlite3_file* p) {
    auto& f = cast(p);
    return f.inner->pMethods->xSectorSize(f.inner);
  }
  static int device(sqlite3_file* p) {
    auto& f = cast(p);
    return f.inner->pMethods->xDeviceCharacteristics(f.inner);
  }
  static int shmmap(sqlite3_file* p, int i, int s, int w, void volatile** a) {
    auto& f = cast(p);
    return f.inner->pMethods->xShmMap(f.inner, i, s, w, a);
  }
  static int shmlock(sqlite3_file* p, int o, int n, int flags) {
    auto& f = cast(p);
    return f.inner->pMethods->xShmLock(f.inner, o, n, flags);
  }
  static void barrier(sqlite3_file* p) {
    auto& f = cast(p);
    f.inner->pMethods->xShmBarrier(f.inner);
  }
  static int unmap(sqlite3_file* p, int d) {
    auto& f = cast(p);
    return f.inner->pMethods->xShmUnmap(f.inner, d);
  }
  static int fetch(sqlite3_file* p, sqlite3_int64 off, int n, void** a) {
    auto& f = cast(p);
    ++f.owner->fetches;
    int rc = f.inner->pMethods->xFetch(f.inner, off, n, a);
    if (*a)
      ++f.owner->mapped;
    return rc;
  }
  static int unfetch(sqlite3_file* p, sqlite3_int64 off, void* a) {
    auto& f = cast(p);
    return f.inner->pMethods->xUnfetch(f.inner, off, a);
  }
  static int del(sqlite3_vfs* v, const char* z, int n) noexcept {
    auto& o = ctx(v);
    return o.inner->xDelete(o.inner, z, n);
  }
  static int access(sqlite3_vfs* v, const char* z, int n, int* a) noexcept {
    auto& o = ctx(v);
    return o.inner->xAccess(o.inner, z, n, a);
  }
  static int full(sqlite3_vfs* v, const char* z, int n, char* a) noexcept {
    auto& o = ctx(v);
    return o.inner->xFullPathname(o.inner, z, n, a);
  }
  static void* dlopen(sqlite3_vfs* v, const char* z) noexcept {
    auto& o = ctx(v);
    return o.inner->xDlOpen(o.inner, z);
  }
  static void dlerror(sqlite3_vfs* v, int n, char* z) noexcept {
    auto& o = ctx(v);
    o.inner->xDlError(o.inner, n, z);
  }
  static void (*dlsym(sqlite3_vfs* v, void* h, const char* z) noexcept)(void) {
    auto& o = ctx(v);
    return o.inner->xDlSym(o.inner, h, z);
  }
  static void dlclose(sqlite3_vfs* v, void* h) noexcept {
    auto& o = ctx(v);
    o.inner->xDlClose(o.inner, h);
  }
  static int random(sqlite3_vfs* v, int n, char* z) noexcept {
    auto& o = ctx(v);
    return o.inner->xRandomness(o.inner, n, z);
  }
  static int sleep(sqlite3_vfs* v, int n) noexcept {
    auto& o = ctx(v);
    return o.inner->xSleep(o.inner, n);
  }
  static int time(sqlite3_vfs* v, double* n) noexcept {
    auto& o = ctx(v);
    return o.inner->xCurrentTime(o.inner, n);
  }
  static int error(sqlite3_vfs* v, int n, char* z) noexcept {
    auto& o = ctx(v);
    return o.inner->xGetLastError(o.inner, n, z);
  }
  static int time64(sqlite3_vfs* v, sqlite3_int64* n) noexcept {
    auto& o = ctx(v);
    return o.inner->xCurrentTimeInt64(o.inner, n);
  }
};
