module;
#include <cerrno>
#include <fcntl.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <unistd.h>
#if defined(__APPLE__)
#include <sys/stdio.h>
#elif defined(__linux__)
#include <linux/fs.h>
#include <sys/syscall.h>
#endif
module planar.engine.source_export;
import std;
import planar.engine.ledger.format;
import planar.engine.history.tree;
namespace planar::engine::source_export {
namespace {
using failure = std::unexpected<export_error>;
failure refuse(error_code code) {
  return failure(export_error{code, {}, {}});
}
failure storage(ledger::format_error error) {
  return failure(export_error{error_code::storage, std::move(error), {}});
}
struct descriptor {
  int fd       = -1;
  descriptor() = default;
  explicit descriptor(int n) : fd(n) {
  }
  descriptor(const descriptor&)            = delete;
  descriptor& operator=(const descriptor&) = delete;
  descriptor(descriptor&& other) noexcept : fd(std::exchange(other.fd, -1)) {
  }
  descriptor& operator=(descriptor&& other) noexcept {
    if (fd >= 0)
      ::close(fd);
    fd = std::exchange(other.fd, -1);
    return *this;
  }
  bool close() noexcept {
    return fd < 0 || ::close(std::exchange(fd, -1)) == 0;
  }
  ~descriptor() {
    if (fd >= 0)
      ::close(fd);
  }
};
bool same(const struct stat& a, const struct stat& b) noexcept {
  return a.st_dev == b.st_dev && a.st_ino == b.st_ino;
}
struct location {
  descriptor  parent;
  std::string name;
};
std::expected<location, export_error> locate(const std::filesystem::path& path) {
  const auto raw = path.string();
  if (raw.empty() || raw.find('\0') != std::string::npos || path.filename().empty() || path.filename() == "." ||
      path.filename() == "..")
    return refuse(error_code::invalid_argument);
  std::error_code ec;
  auto            absolute = std::filesystem::absolute(path, ec);
  if (ec)
    return refuse(error_code::path_denied);
  descriptor dir(::open("/", O_RDONLY | O_DIRECTORY | O_CLOEXEC));
  if (dir.fd < 0)
    return refuse(error_code::io);
  // Preserve original traversal: a missing/.. component never disappears through normalization.
  for (const auto& part : absolute.parent_path().relative_path()) {
    descriptor next(::openat(dir.fd, part.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
    if (next.fd < 0)
      return refuse(error_code::path_denied);
    dir = std::move(next);
  }
  return location{std::move(dir), absolute.filename().string()};
}
// Each receipt and basename is allocated before staging. Root has index zero.
struct entry {
  std::size_t                parent;
  std::string                name;
  std::optional<std::size_t> file;
  struct stat                identity{};
  bool                       created    = false;
  bool                       identified = false;
};
struct staging {
  int                parent;
  descriptor         root;
  std::string        name;
  std::vector<entry> entries;
  bool               published = false;
  bool               active    = false;
  bool               complete  = true;
  // Open from retained root, checking each recorded ancestor, with at most depth+2 temporary descriptors.
  descriptor directory(std::size_t n) const noexcept {
    if (n == 0) {
      descriptor  d(::openat(root.fd, ".", O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
      struct stat st{};
      if (d.fd < 0 || !entries[0].identified || ::fstat(d.fd, &st) != 0 || !same(st, entries[0].identity))
        return {};
      return d;
    }
    const auto& e = entries[n];
    auto        p = directory(e.parent);
    if (p.fd < 0 || !e.identified)
      return {};
    descriptor  d(::openat(p.fd, e.name.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
    struct stat st{};
    if (d.fd < 0 || ::fstat(d.fd, &st) != 0 || !same(st, e.identity))
      return {};
    return d;
  }
  bool named_root() const noexcept {
    struct stat st{};
    return entries[0].identified && ::fstatat(parent, name.c_str(), &st, AT_SYMLINK_NOFOLLOW) == 0 && S_ISDIR(st.st_mode) &&
           same(st, entries[0].identity);
  }
  bool cleanup() noexcept {
    if (!active || published)
      return complete;
    active = false;
    // An unverified/replaced root or ancestor prevents deletion beneath it.
    if (!named_root())
      return complete = false;
    if (root.fd < 0)
      root = descriptor(::openat(parent, name.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
    if (root.fd < 0)
      return complete = false;
    for (std::size_t i = entries.size(); i-- > 1;) {
      const auto& e = entries[i];
      if (!e.created)
        continue;
      auto        p = directory(e.parent);
      struct stat st{};
      if (p.fd < 0 || !e.identified || ::fstatat(p.fd, e.name.c_str(), &st, AT_SYMLINK_NOFOLLOW) != 0 || !same(st, e.identity) ||
          (e.file ? !S_ISREG(st.st_mode) : !S_ISDIR(st.st_mode)) ||
          ::unlinkat(p.fd, e.name.c_str(), e.file ? 0 : AT_REMOVEDIR) != 0)
        complete = false;
    }
    if (!root.close())
      complete = false;
    if (!named_root() || ::unlinkat(parent, name.c_str(), AT_REMOVEDIR) != 0)
      complete = false;
    return complete;
  }
  ~staging() {
    cleanup();
  }
};
std::vector<entry> plan(const std::vector<history::tree_file>& files) {
  std::vector<entry> entries;
  entries.reserve(8192 + files.size());
  entries.push_back({0, {}, {}});
  std::map<std::string, std::size_t> dirs{{"", 0}};
  for (std::size_t f = 0; f < files.size(); ++f) {
    std::size_t parent = 0, begin = 0;
    const auto& path = files[f].path;
    for (;;) {
      auto slash = path.find('/', begin);
      if (slash == std::string::npos) {
        entries.push_back({parent, path.substr(begin), f});
        break;
      }
      auto prefix = path.substr(0, slash);
      auto found  = dirs.find(prefix);
      if (found == dirs.end()) {
        auto n = entries.size();
        entries.push_back({parent, path.substr(begin, slash - begin), {}});
        dirs.emplace(std::move(prefix), n);
        parent = n;
      } else
        parent = found->second;
      begin = slash + 1;
    }
  }
  return entries;
}
int publish(int parent, const char* stage, const char* target) noexcept {
#if defined(__APPLE__) && defined(RENAME_EXCL)
  return ::renameatx_np(parent, stage, parent, target, RENAME_EXCL);
#elif defined(__linux__) && defined(SYS_renameat2) && defined(RENAME_NOREPLACE)
  return static_cast<int>(::syscall(SYS_renameat2, parent, stage, parent, target, RENAME_NOREPLACE));
#else
  (void)parent;
  (void)stage;
  (void)target;
  errno = ENOTSUP;
  return -1;
#endif
}
std::expected<void, export_error> materialize(staging& stage, const std::vector<history::tree_file>& files) {
  if (::mkdirat(stage.parent, stage.name.c_str(), 0700) != 0)
    return refuse(error_code::io);
  stage.active = true;
  auto& root   = stage.entries[0];
  root.created = true;
  if (::fstatat(stage.parent, stage.name.c_str(), &root.identity, AT_SYMLINK_NOFOLLOW) != 0 || !S_ISDIR(root.identity.st_mode))
    return refuse(error_code::io);
  root.identified = true;
  stage.root      = descriptor(::openat(stage.parent, stage.name.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
  struct stat check{};
  if (stage.root.fd < 0 || ::fstat(stage.root.fd, &check) != 0 || !same(root.identity, check) ||
      ::fchmod(stage.root.fd, 0700) != 0)
    return refuse(error_code::io);
  for (std::size_t i = 1; i < stage.entries.size(); ++i) {
    auto& e = stage.entries[i];
    auto  p = stage.directory(e.parent);
    if (p.fd < 0)
      return refuse(error_code::io);
    descriptor d;
    if (e.file)
      d = descriptor(::openat(p.fd, e.name.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600));
    else {
      if (::mkdirat(p.fd, e.name.c_str(), 0700) != 0)
        return refuse(error_code::io);
      e.created = true;
      // Record directory identity before its open, so open failures still permit safe cleanup.
      if (::fstatat(p.fd, e.name.c_str(), &e.identity, AT_SYMLINK_NOFOLLOW) != 0 || !S_ISDIR(e.identity.st_mode))
        return refuse(error_code::io);
      e.identified = true;
      d            = descriptor(::openat(p.fd, e.name.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
    }
    if (d.fd < 0)
      return refuse(error_code::io);
    e.created = true;
    if (::fstat(d.fd, &check) != 0 || (e.identified && !same(e.identity, check)))
      return refuse(error_code::io);
    e.identity   = check;
    e.identified = true;
    if (e.file) {
      const auto& f      = files[*e.file];
      std::size_t offset = 0;
      while (offset < f.payload.size()) {
        auto n = ::write(d.fd, f.payload.data() + offset, f.payload.size() - offset);
        if (n < 0 && errno == EINTR)
          continue;
        if (n <= 0)
          return refuse(error_code::io);
        offset += static_cast<std::size_t>(n);
      }
      if (::fchmod(d.fd, static_cast<mode_t>(f.mode)) != 0 || ::fsync(d.fd) != 0)
        return refuse(error_code::io);
    } else if (::fchmod(d.fd, 0700) != 0)
      return refuse(error_code::io);
    if (!d.close() || !p.close())
      return refuse(error_code::io);
  }
  // Reverse creation order is child before parent, even when file paths have unlike depths.
  for (std::size_t i = stage.entries.size(); i-- > 0;) {
    if (stage.entries[i].file)
      continue;
    auto d = stage.directory(i);
    if (d.fd < 0 || ::fchmod(d.fd, 0755) != 0 || ::fsync(d.fd) != 0 || !d.close())
      return refuse(error_code::io);
  }
  return {};
}
} // namespace
std::expected<export_result, export_error> export_source(const std::filesystem::path& ledger_path, std::int64_t revision,
                                                         const std::filesystem::path& target) {
  if (revision <= 0)
    return refuse(error_code::invalid_argument);
  auto reader = ledger::open_existing(ledger_path);
  if (!reader)
    return storage(reader.error());
  auto row = reader->read_revision(static_cast<std::uint64_t>(revision));
  if (!row)
    return storage(row.error());
  if (!*row)
    return refuse(error_code::revision_not_found);
  auto source = reader->read_record((**row).references.source);
  if (!source)
    return storage(source.error());
  auto files = history::read_tree(*reader, source->reference, history::tree_profile::source);
  if (!files) {
    if (files.error().storage)
      return storage(*files.error().storage);
    return storage({ledger::format_error_code::corrupt, "Recorded source tree is corrupt"});
  }
  export_result result{reader->metadata().identity, revision, (**row).references.source};
  auto          where = locate(target);
  if (!where)
    return std::unexpected(where.error());
  struct stat existing{};
  if (::fstatat(where->parent.fd, where->name.c_str(), &existing, AT_SYMLINK_NOFOLLOW) == 0)
    return refuse(error_code::target_exists);
  if (errno != ENOENT)
    return refuse(error_code::io);
  std::array<unsigned char, 16> random{};
  if (::getentropy(random.data(), random.size()) != 0)
    return refuse(error_code::io);
  std::string                name = ".planar-export-";
  constexpr std::string_view hex  = "0123456789abcdef";
  for (auto b : random) {
    name += hex[b >> 4];
    name += hex[b & 15];
  }
  export_error incomplete{error_code::cleanup_incomplete, {}, name};
  staging      stage{where->parent.fd, {}, std::move(name), plan(*files)};
  auto         failed = [&](export_error error) -> std::expected<export_result, export_error> {
    if (stage.published)
      return refuse(error_code::publication_uncertain);
    if (!stage.cleanup())
      return std::unexpected(std::move(incomplete));
    return std::unexpected(std::move(error));
  };
  try {
    auto written = materialize(stage, *files);
    if (!written)
      return failed(std::move(written.error()));
    if (!stage.named_root())
      return failed({error_code::io, {}, {}});
    if (publish(where->parent.fd, stage.name.c_str(), where->name.c_str()) != 0)
      return failed({errno == EEXIST || errno == ENOTEMPTY ? error_code::target_exists : error_code::io, {}, {}});
    // No potentially throwing operation may intervene before this flag.
    stage.published = true;
    if (::fsync(where->parent.fd) != 0 || !stage.root.close() || !where->parent.close())
      return refuse(error_code::publication_uncertain);
    return result;
  } catch (...) {
    // Includes allocation failures after creation; cleanup does not allocate or throw.
    return failed({error_code::io, {}, {}});
  }
}
} // namespace planar::engine::source_export
