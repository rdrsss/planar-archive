module;
#include <sqlite3.h>
module planar.engine.ledger.sqlite;
import std;
namespace planar::engine::ledger {
namespace {
struct bound_context;
struct file {
  sqlite3_file       base{};
  sqlite3_file*      inner   = nullptr;
  bound_context*     context = nullptr;
  bool               main    = false;
  sqlite3_io_methods methods{};
};
// All storage is fixed-size except the shared ownership block supplied by the caller.
// SQLite allocates each file wrapper together with its aligned inner file in szOsFile.
struct bound_context {
  std::shared_ptr<read_guard> guard;
  sqlite3_vfs                 vfs{};
  sqlite3_vfs*                inner = nullptr;
  char                        name[64]{};
  read_gate_status            status      = read_gate_status::ready;
  int                         first       = SQLITE_OK;
  bool                        registered  = false;
  bool                        main_opened = false;
  explicit bound_context(std::shared_ptr<read_guard> g) : guard(std::move(g)) {
  }
  int initialize() noexcept {
    inner = sqlite3_vfs_find(nullptr);
    if (!inner || inner->szOsFile < 0 ||
        inner->szOsFile > std::numeric_limits<int>::max() - static_cast<int>(sizeof(file) + alignof(std::max_align_t)))
      return SQLITE_CANTOPEN;
    constexpr std::string_view prefix = "planar-read-";
    std::copy(prefix.begin(), prefix.end(), name);
    auto result = std::to_chars(name + prefix.size(), name + sizeof(name) - 1, reinterpret_cast<std::uintptr_t>(this), 16);
    if (result.ec != std::errc{})
      return SQLITE_CANTOPEN;
    *result.ptr           = '\0';
    vfs                   = *inner;
    vfs.iVersion          = std::min(inner->iVersion, 3);
    vfs.pNext             = nullptr;
    vfs.zName             = name;
    vfs.pAppData          = this;
    vfs.szOsFile          = static_cast<int>(sizeof(file) + alignof(std::max_align_t)) + inner->szOsFile;
    vfs.xOpen             = open;
    vfs.xDelete           = del;
    vfs.xAccess           = access;
    vfs.xFullPathname     = full;
    vfs.xDlOpen           = inner->xDlOpen ? dlopen : nullptr;
    vfs.xDlError          = inner->xDlError ? dlerror : nullptr;
    vfs.xDlSym            = inner->xDlSym ? dlsym : nullptr;
    vfs.xDlClose          = inner->xDlClose ? dlclose : nullptr;
    vfs.xRandomness       = random;
    vfs.xSleep            = sleep;
    vfs.xCurrentTime      = time;
    vfs.xGetLastError     = inner->xGetLastError ? error : nullptr;
    vfs.xCurrentTimeInt64 = vfs.iVersion >= 2 && inner->xCurrentTimeInt64 ? time64 : nullptr;
    vfs.xSetSystemCall    = vfs.iVersion >= 3 && inner->xSetSystemCall ? set_system : nullptr;
    vfs.xGetSystemCall    = vfs.iVersion >= 3 && inner->xGetSystemCall ? get_system : nullptr;
    vfs.xNextSystemCall   = vfs.iVersion >= 3 && inner->xNextSystemCall ? next_system : nullptr;
    int rc                = sqlite3_vfs_register(&vfs, 0);
    registered            = rc == SQLITE_OK;
    return rc;
  }
  ~bound_context() {
    // Pinned SQLite unregister cannot fail with this live initialized owner. Concurrent shutdown is unsupported.
    if (registered && sqlite3_vfs_unregister(&vfs) != SQLITE_OK)
      std::terminate();
  }
  int reject(read_gate_status failure, int code = SQLITE_IOERR_READ) noexcept {
    if (status == read_gate_status::ready) {
      status = failure;
      first  = code;
    }
    return first;
  }
  bool gate(file* f) noexcept {
    if (first != SQLITE_OK)
      return false;
    try {
      switch (guard->verify()) {
      case read_gate_status::ready:
        break;
      case read_gate_status::refused:
        reject(read_gate_status::refused);
        return false;
      case read_gate_status::resource:
        reject(read_gate_status::resource, SQLITE_NOMEM);
        return false;
      default:
        reject(read_gate_status::invalid_status);
        return false;
      }
    } catch (const std::bad_alloc&) {
      reject(read_gate_status::resource, SQLITE_NOMEM);
      return false;
    } catch (const std::length_error&) {
      reject(read_gate_status::resource, SQLITE_NOMEM);
      return false;
    } catch (...) {
      reject(read_gate_status::verifier_exception);
      return false;
    }
    int         moved = 0;
    const auto* m     = f->inner->pMethods;
    int         rc    = m->xFileControl ? m->xFileControl(f->inner, SQLITE_FCNTL_HAS_MOVED, &moved) : SQLITE_NOTFOUND;
    if (rc != SQLITE_OK || moved) {
      reject(rc == SQLITE_NOTFOUND ? read_gate_status::moved_unsupported
             : rc != SQLITE_OK     ? read_gate_status::moved_error
                                   : read_gate_status::moved);
      return false;
    }
    return true;
  }
  static bound_context& ctx(sqlite3_vfs* v) {
    return *static_cast<bound_context*>(v->pAppData);
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
      return o.reject(read_gate_status::sidecar, SQLITE_CANTOPEN);
    }
    if (f->main) {
      if (o.main_opened || (flags & (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE)))
        return SQLITE_CANTOPEN;
      o.main_opened = true;
    }
    int rc = o.inner->xOpen(o.inner, z, f->inner, flags, out);
    if (rc != SQLITE_OK) {
      if (f->inner->pMethods)
        f->inner->pMethods->xClose(f->inner);
      return rc;
    }
    f->methods          = *f->inner->pMethods;
    f->methods.iVersion = std::min(f->methods.iVersion, 3);
    install_methods(*f);
    f->base.pMethods = &f->methods;
    return SQLITE_OK;
  }
  static int close(sqlite3_file* p) noexcept {
    auto* f          = cast(p);
    int   rc         = f->inner->pMethods->xClose(f->inner);
    f->base.pMethods = nullptr;
    return rc;
  }
  static int read(sqlite3_file* p, void* b, int n, sqlite3_int64 off) noexcept {
    auto* f = cast(p);
    if (f->main && !f->context->gate(f))
      return f->context->first;
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
    return f->inner->pMethods->xFileControl ? f->inner->pMethods->xFileControl(f->inner, n, a) : SQLITE_NOTFOUND;
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
      *a = nullptr;
      return f->context->reject(read_gate_status::sidecar, SQLITE_IOERR_SHMMAP);
    }
    return f->inner->pMethods->xShmMap(f->inner, i, s, w, a);
  }
  static int shmlock(sqlite3_file* p, int o, int n, int flags) noexcept {
    auto* f = cast(p);
    if (f->main)
      return f->context->reject(read_gate_status::sidecar, SQLITE_IOERR_SHMLOCK);
    return f->inner->pMethods->xShmLock(f->inner, o, n, flags);
  }
  static void barrier(sqlite3_file* p) noexcept {
    auto* f = cast(p);
    if (!f->main)
      f->inner->pMethods->xShmBarrier(f->inner);
  }
  static int unmap(sqlite3_file* p, int d) noexcept {
    auto* f = cast(p);
    return f->main ? SQLITE_OK : f->inner->pMethods->xShmUnmap(f->inner, d);
  }
  static int fetch(sqlite3_file* p, sqlite3_int64 off, int n, void** a) noexcept {
    auto* f = cast(p);
    *a      = nullptr;
    if (f->main && !f->context->gate(f))
      return f->context->first;
    auto* m = f->inner->pMethods;
    if (m->iVersion < 3 || !m->xFetch)
      return SQLITE_OK;
    int rc = m->xFetch(f->inner, off, n, a);
    return rc;
  }
  static int unfetch(sqlite3_file* p, sqlite3_int64 off, void* a) noexcept {
    auto* f = cast(p);
    auto* m = f->inner->pMethods;
    return m->iVersion >= 3 && m->xUnfetch ? m->xUnfetch(f->inner, off, a) : SQLITE_OK;
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
  static int set_system(sqlite3_vfs* v, const char* z, sqlite3_syscall_ptr p) noexcept {
    auto& o = ctx(v);
    return o.inner->xSetSystemCall(o.inner, z, p);
  }
  static sqlite3_syscall_ptr get_system(sqlite3_vfs* v, const char* z) noexcept {
    auto& o = ctx(v);
    return o.inner->xGetSystemCall(o.inner, z);
  }
  static const char* next_system(sqlite3_vfs* v, const char* z) noexcept {
    auto& o = ctx(v);
    return o.inner->xNextSystemCall(o.inner, z);
  }
  static void install_methods(file& f) noexcept {
    auto& m                  = f.methods;
    m.xClose                 = close;
    m.xRead                  = read;
    m.xWrite                 = write;
    m.xTruncate              = truncate;
    m.xSync                  = sync;
    m.xFileSize              = size;
    m.xLock                  = lock;
    m.xUnlock                = unlock;
    m.xCheckReservedLock     = reserved;
    m.xFileControl           = control;
    m.xSectorSize            = m.xSectorSize ? sector : nullptr;
    m.xDeviceCharacteristics = m.xDeviceCharacteristics ? device : nullptr;
    if (m.iVersion >= 2) {
      m.xShmMap     = f.main || m.xShmMap ? shmmap : nullptr;
      m.xShmLock    = m.xShmLock ? shmlock : nullptr;
      m.xShmBarrier = m.xShmBarrier ? barrier : nullptr;
      m.xShmUnmap   = m.xShmUnmap ? unmap : nullptr;
    } else {
      m.xShmMap     = nullptr;
      m.xShmLock    = nullptr;
      m.xShmBarrier = nullptr;
      m.xShmUnmap   = nullptr;
    }
    if (m.iVersion >= 3) {
      m.xFetch   = m.xFetch ? fetch : nullptr;
      m.xUnfetch = m.xUnfetch ? unfetch : nullptr;
    } else {
      m.xFetch   = nullptr;
      m.xUnfetch = nullptr;
    }
  }
};
} // namespace

/// @brief Retained thread-confined connection and transaction generation; no global state.
struct sqlite_state {
  /// @brief Stable optional read-gate context, destroyed only after actual connection close.
  std::unique_ptr<bound_context> gate;
  /// @brief SQLite connection released after the last retained owner.
  sqlite3* handle = nullptr;
  /// @brief Permit transaction authorization only during internal control calls.
  bool internal = false;
  /// @brief Refuse further operations after a failed rollback.
  bool poisoned = false;
  /// @brief Whether the wrapper owns an active transaction.
  bool active = false;
  /// @brief Monotonic identity distinguishing abandoned and subsequent guards.
  std::uint64_t generation = 0;
  ~sqlite_state() {
    // Public statements and transactions retain this state; no raw handle or BLOB escapes.
    // An unfinished resource here would violate that ownership invariant, so never free a live VFS.
    if (handle && sqlite3_close(handle) != SQLITE_OK)
      std::terminate();
  }
};
namespace {
sqlite_error misuse(std::string message) {
  return {SQLITE_MISUSE, SQLITE_MISUSE, std::move(message)};
}
sqlite_error error(sqlite_state& state, int code) {
  int extended = sqlite3_extended_errcode(state.handle);
  if ((extended & 255) != (code & 255))
    extended = code;
  return {code & 255, extended, sqlite3_errmsg(state.handle), state.gate ? state.gate->status : read_gate_status::ready};
}
int authorize(void* data, int action, const char*, const char*, const char*, const char*) {
  auto& state = *static_cast<sqlite_state*>(data);
  if (state.gate && (action == SQLITE_ATTACH || action == SQLITE_DETACH))
    return SQLITE_DENY;
  if ((action == SQLITE_TRANSACTION || action == SQLITE_SAVEPOINT) && !state.internal)
    return SQLITE_DENY;
  return SQLITE_OK;
}
int control(sqlite_state& state, const char* sql) {
  state.internal = true;
  const int code = sqlite3_exec(state.handle, sql, nullptr, nullptr, nullptr);
  state.internal = false;
  return code;
}
int rollback_state(sqlite_state& state) noexcept {
  const int code = sqlite3_get_autocommit(state.handle) ? SQLITE_OK : control(state, "ROLLBACK");
  state.active   = false;
  if (code != SQLITE_OK)
    state.poisoned = true;
  return code;
}
std::unexpected<sqlite_error> failed(sqlite_state& state, int code) {
  auto failure = error(state, code);
  if (state.active)
    rollback_state(state);
  return std::unexpected(std::move(failure));
}
bool usable(const std::shared_ptr<sqlite_state>& state) {
  return state && !state->poisoned;
}
sqlite3_stmt* handle(void* pointer) {
  return static_cast<sqlite3_stmt*>(pointer);
}
} // namespace
connection::connection(std::shared_ptr<sqlite_state> state) : _state(std::move(state)) {
}
std::expected<connection, sqlite_error> connection::open(const std::filesystem::path& path, open_mode mode, int busy_ms) {
  return open_impl(path, mode, busy_ms, {});
}
std::expected<connection, sqlite_error> connection::open_gated(const std::filesystem::path& path,
                                                               std::shared_ptr<read_guard> guard, int busy_ms) {
  if (!guard)
    return std::unexpected(misuse("A native read guard is required"));
  return open_impl(path, open_mode::read_only, busy_ms, std::move(guard));
}
std::expected<connection, sqlite_error> connection::open_impl(const std::filesystem::path& path, open_mode mode, int busy_ms,
                                                              std::shared_ptr<read_guard> guard) {
  const auto supplied = path.string();
  if (supplied.empty() || supplied.find('\0') != std::string::npos || busy_ms < 0 || busy_ms > 5000)
    return std::unexpected(misuse("Invalid explicit path or busy timeout"));
  // Make native filenames unambiguous to SQLite without resolving links or normalizing components.
  std::error_code path_error;
  const auto      absolute = std::filesystem::absolute(path, path_error);
  if (path_error)
    return std::unexpected(sqlite_error{SQLITE_CANTOPEN, SQLITE_CANTOPEN,
                                        std::format("Cannot make explicit path absolute: {}", path_error.message())});
  // SQLite can collapse missing/.. itself; require the OS to traverse the original parent first.
  if (!std::filesystem::is_directory(absolute.parent_path(), path_error))
    return std::unexpected(sqlite_error{SQLITE_CANTOPEN, SQLITE_CANTOPEN, "Explicit path parent is not an accessible directory"});
  const auto native = absolute.string();
  auto       state  = std::make_shared<sqlite_state>();
  int        flags  = SQLITE_OPEN_NOFOLLOW | SQLITE_OPEN_EXRESCODE;
  switch (mode) {
  case open_mode::read_only:
    flags |= SQLITE_OPEN_READONLY;
    break;
  case open_mode::read_write:
    flags |= SQLITE_OPEN_READWRITE;
    break;
  case open_mode::scratch_create:
    flags |= SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    break;
  default:
    return std::unexpected(misuse("Invalid opening mode"));
  }
  if (guard) {
    state->gate = std::make_unique<bound_context>(std::move(guard));
    int rc      = state->gate->initialize();
    if (rc != SQLITE_OK)
      return std::unexpected(sqlite_error{rc & 255, rc, "Cannot register native read VFS"});
  }
  int code = sqlite3_open_v2(native.c_str(), &state->handle, flags, state->gate ? state->gate->name : nullptr);
  if (code != SQLITE_OK) {
    if (!state->handle)
      return std::unexpected(sqlite_error{code & 255, code, "SQLite could not allocate a connection",
                                          state->gate ? state->gate->status : read_gate_status::ready});
    return std::unexpected(error(*state, code));
  }
  sqlite3_extended_result_codes(state->handle, 1);
  code = sqlite3_busy_timeout(state->handle, busy_ms);
  if (code != SQLITE_OK)
    return std::unexpected(error(*state, code));
  code = sqlite3_set_authorizer(state->handle, authorize, state.get());
  if (code != SQLITE_OK)
    return std::unexpected(error(*state, code));
  return connection(std::move(state));
}
read_gate_status connection::gate_status() const noexcept {
  return _state && _state->gate ? _state->gate->status : read_gate_status::ready;
}
std::expected<statement, sqlite_error> connection::prepare(std::string_view sql) {
  if (!usable(_state))
    return std::unexpected(misuse("Connection is moved-from or poisoned"));
  if (sql.empty() || sql.find('\0') != std::string_view::npos ||
      sql.size() >= static_cast<std::size_t>(std::numeric_limits<int>::max())) {
    if (_state->active)
      rollback_state(*_state);
    return std::unexpected(misuse("SQL must be one nonempty statement without NUL"));
  }
  // The terminating NUL belongs to this copy; SQLite reports the exact tail.
  const std::string owned(sql);
  sqlite3_stmt*     prepared = nullptr;
  const char*       tail     = nullptr;
  int code = sqlite3_prepare_v3(_state->handle, owned.c_str(), static_cast<int>(owned.size() + 1), 0, &prepared, &tail);
  if (code != SQLITE_OK)
    return failed(*_state, code);
  // Let SQLite parse the tail as well, allowing comments/whitespace but never another statement.
  sqlite3_stmt* extra = nullptr;
  if (prepared)
    code = sqlite3_prepare_v3(_state->handle, tail, -1, 0, &extra, nullptr);
  if (code != SQLITE_OK || !prepared || extra) {
    sqlite3_finalize(prepared);
    sqlite3_finalize(extra);
    if (code != SQLITE_OK)
      return failed(*_state, code);
    if (_state->active)
      rollback_state(*_state);
    return std::unexpected(misuse("Expected exactly one SQL statement"));
  }
  return statement(_state, prepared);
}
std::expected<void, sqlite_error> connection::online_backup_to(connection& destination) {
  if (!usable(_state) || !usable(destination._state) || _state == destination._state || destination._state->active)
    return std::unexpected(misuse("Backup requires distinct usable owners and an idle new destination"));
  auto* backup = sqlite3_backup_init(destination._state->handle, "main", _state->handle, "main");
  if (!backup)
    return std::unexpected(error(*destination._state, sqlite3_errcode(destination._state->handle)));
  struct backup_owner {
    sqlite3_backup* pointer;
    ~backup_owner() {
      if (pointer)
        sqlite3_backup_finish(pointer);
    }
    int finish() noexcept {
      return sqlite3_backup_finish(std::exchange(pointer, nullptr));
    }
  } owner{backup};
  const int stepped = sqlite3_backup_step(backup, -1);
  // Capture the step diagnostic before finish changes the destination error state; always release the backup handle.
  auto      failure  = error(*destination._state, stepped);
  const int finished = owner.finish();
  if (stepped != SQLITE_DONE)
    return std::unexpected(std::move(failure));
  if (finished != SQLITE_OK)
    return std::unexpected(error(*destination._state, finished));
  return {};
}
std::expected<transaction, sqlite_error> connection::begin(bool write) {
  if (!usable(_state) || _state->active)
    return std::unexpected(misuse("Connection unavailable or transaction already active"));
  const int code = control(*_state, write ? "BEGIN IMMEDIATE" : "BEGIN");
  if (code != SQLITE_OK)
    return failed(*_state, code);
  _state->active = true;
  return transaction(_state, ++_state->generation);
}
statement::statement(std::shared_ptr<sqlite_state> state, void* pointer) : _state(std::move(state)), _handle(pointer) {
}
statement::statement(statement&& other) noexcept
    : _state(std::move(other._state)), _handle(std::exchange(other._handle, nullptr)), _row(std::exchange(other._row, false)) {
}
statement& statement::operator=(statement&& other) noexcept {
  if (this != &other) {
    sqlite3_finalize(handle(_handle));
    _state  = std::move(other._state);
    _handle = std::exchange(other._handle, nullptr);
    _row    = std::exchange(other._row, false);
  }
  return *this;
}
statement::~statement() {
  sqlite3_finalize(handle(_handle));
}
std::expected<void, sqlite_error> statement::bound(int code) {
  if (code != SQLITE_OK)
    return failed(*_state, code);
  return {};
}
std::expected<void, sqlite_error> statement::bind_null(int index) {
  if (!_handle || !usable(_state))
    return std::unexpected(misuse("Statement unavailable"));
  return bound(sqlite3_bind_null(handle(_handle), index));
}
std::expected<void, sqlite_error> statement::bind_integer(int index, std::int64_t value) {
  if (!_handle || !usable(_state))
    return std::unexpected(misuse("Statement unavailable"));
  return bound(sqlite3_bind_int64(handle(_handle), index, value));
}
std::expected<void, sqlite_error> statement::bind_text(int index, std::string_view bytes) {
  if (!_handle || !usable(_state))
    return std::unexpected(misuse("Statement unavailable"));
  return bound(sqlite3_bind_text64(handle(_handle), index, bytes.empty() ? "" : bytes.data(), bytes.size(), SQLITE_TRANSIENT,
                                   SQLITE_UTF8));
}
std::expected<void, sqlite_error> statement::bind_blob(int index, std::string_view bytes) {
  if (!_handle || !usable(_state))
    return std::unexpected(misuse("Statement unavailable"));
  return bound(sqlite3_bind_blob64(handle(_handle), index, bytes.empty() ? "" : bytes.data(), bytes.size(), SQLITE_TRANSIENT));
}
std::expected<step_result, sqlite_error> statement::step() {
  _row = false;
  if (!_handle || !usable(_state))
    return std::unexpected(misuse("Statement unavailable"));
  const int code = sqlite3_step(handle(_handle));
  if (code == SQLITE_ROW) {
    _row = true;
    return step_result::row;
  }
  if (code == SQLITE_DONE)
    return step_result::done;
  return failed(*_state, code);
}
std::expected<void, sqlite_error> statement::reset() {
  _row = false;
  if (!_handle || !usable(_state))
    return std::unexpected(misuse("Statement unavailable"));
  const int code = sqlite3_reset(handle(_handle));
  sqlite3_clear_bindings(handle(_handle));
  return bound(code);
}
std::expected<int, sqlite_error> statement::column_type(int column) const {
  if (!_handle || !usable(_state) || !_row || column < 0 || column >= sqlite3_column_count(handle(_handle)))
    return std::unexpected(misuse("Column requires a current row and valid index"));
  return sqlite3_column_type(handle(_handle), column);
}
std::expected<std::optional<std::int64_t>, sqlite_error> statement::integer(int column) const {
  auto type = column_type(column);
  if (!type)
    return std::unexpected(type.error());
  if (*type == SQLITE_NULL)
    return std::nullopt;
  if (*type != SQLITE_INTEGER)
    return std::unexpected(misuse("Column is not an integer"));
  return sqlite3_column_int64(handle(_handle), column);
}
std::expected<std::optional<std::string>, sqlite_error> statement::bytes(int column, int wanted) const {
  auto type = column_type(column);
  if (!type)
    return std::unexpected(type.error());
  if (*type == SQLITE_NULL)
    return std::nullopt;
  if (*type != wanted)
    return std::unexpected(misuse("Column has the wrong byte type"));
  const auto* data =
      wanted == SQLITE_BLOB ? sqlite3_column_blob(handle(_handle), column) : sqlite3_column_text(handle(_handle), column);
  const int size = sqlite3_column_bytes(handle(_handle), column);
  if (!data && sqlite3_errcode(_state->handle) == SQLITE_NOMEM)
    return std::unexpected(error(*_state, SQLITE_NOMEM));
  return std::string(size ? static_cast<const char*>(data) : "", size);
}
std::expected<std::optional<std::string>, sqlite_error> statement::text(int column) const {
  return bytes(column, SQLITE_TEXT);
}
std::expected<std::optional<std::string>, sqlite_error> statement::blob(int column) const {
  return bytes(column, SQLITE_BLOB);
}
transaction::transaction(std::shared_ptr<sqlite_state> state, std::uint64_t generation)
    : _state(std::move(state)), _generation(generation) {
}
transaction::transaction(transaction&& other) noexcept
    : _state(std::move(other._state)), _generation(std::exchange(other._generation, 0)) {
}
transaction& transaction::operator=(transaction&& other) noexcept {
  if (this != &other) {
    abandon();
    _state      = std::move(other._state);
    _generation = std::exchange(other._generation, 0);
  }
  return *this;
}
transaction::~transaction() {
  abandon();
}
void transaction::abandon() noexcept {
  if (_state && _state->active && _state->generation == _generation)
    rollback_state(*_state);
  _generation = 0;
}
std::expected<void, sqlite_error> transaction::commit() {
  bool confirmed_noncommit = false;
  return commit(confirmed_noncommit);
}
std::expected<void, sqlite_error> transaction::commit(bool& confirmed_noncommit) {
  confirmed_noncommit = true;
  if (!usable(_state) || !_generation || !_state->active || _state->generation != _generation)
    return std::unexpected(misuse("Transaction is no longer active"));
  // SQLite owns diagnostic storage independently of the connection error slot. Retain it across rollback,
  // then publish the observed outcome before allocating the C++ error. SQLite allocation failure is itself
  // an ordinary COMMIT failure; the fallback diagnostic comes from SQLite's static result-code vocabulary.
  struct diagnostic_owner {
    char* bytes = nullptr;
    ~diagnostic_owner() {
      sqlite3_free(bytes);
    }
  } diagnostic;
  confirmed_noncommit = false;
  _state->internal    = true;
  const int code      = sqlite3_exec(_state->handle, "COMMIT", nullptr, nullptr, &diagnostic.bytes);
  _state->internal    = false;
  _generation         = 0;
  if (code != SQLITE_OK) {
    int extended = sqlite3_extended_errcode(_state->handle);
    if ((extended & 255) != (code & 255))
      extended = code;
    const bool was_active  = !sqlite3_get_autocommit(_state->handle);
    const int  rolled_back = rollback_state(*_state);
    confirmed_noncommit    = was_active && rolled_back == SQLITE_OK;
    return std::unexpected(sqlite_error{code & 255, extended, diagnostic.bytes ? diagnostic.bytes : sqlite3_errstr(code)});
  }
  _state->active = false;
  return {};
}
std::expected<void, sqlite_error> transaction::rollback() {
  if (!usable(_state) || !_generation || !_state->active || _state->generation != _generation)
    return std::unexpected(misuse("Transaction is no longer active"));
  _generation    = 0;
  const int code = rollback_state(*_state);
  if (code != SQLITE_OK)
    return std::unexpected(error(*_state, code));
  return {};
}
} // namespace planar::engine::ledger
