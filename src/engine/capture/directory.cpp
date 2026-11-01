module;
#include <cerrno>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
module planar.engine.capture.directory;
import std;
import :recheck;
namespace planar::engine {
namespace {
constexpr std::size_t max_names      = 1024;
constexpr std::size_t max_file_bytes = 65536;
constexpr std::size_t max_total      = 4194304;

/// Owns one descriptor and closes it on every path.
struct descriptor {
  int fd       = -1;
  descriptor() = default;
  explicit descriptor(int value) : fd(value) {
  }
  descriptor(const descriptor&)            = delete;
  descriptor& operator=(const descriptor&) = delete;
  descriptor(descriptor&& other) noexcept : fd(std::exchange(other.fd, -1)) {
  }
  descriptor& operator=(descriptor&& other) noexcept {
    if (this != &other) {
      reset();
      fd = std::exchange(other.fd, -1);
    }
    return *this;
  }
  ~descriptor() {
    reset();
  }
  void reset() noexcept {
    if (fd >= 0)
      ::close(fd);
    fd = -1;
  }
};

diagnostic denied_kind() {
  return diagnostic{"path_denied", "Links and special files forbidden"};
}
diagnostic denied_name() {
  return diagnostic{"path_denied", "Invalid captured path"};
}
diagnostic cannot_inspect() {
  return diagnostic{"io", "Cannot inspect confined input"};
}
diagnostic limit(std::string_view name, std::size_t maximum) {
  return diagnostic{"resource_limit", "Capture limit exceeded", std::string(name), maximum};
}

/// Portable bytes for ASCII, any valid UTF-8 above it; never a control byte or a C1 control.
bool name_valid(std::string_view name) {
  if (name.empty() || name.size() > 255 || !valid_text(name))
    return false;
  for (std::size_t i = 0; i < name.size(); ++i) {
    const auto c = static_cast<unsigned char>(name[i]);
    if (c < 128) {
      if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '.' || c == '-'))
        return false;
    } else if (c == 0xc2 && i + 1 < name.size() && static_cast<unsigned char>(name[i + 1]) < 0xa0) {
      return false;
    }
  }
  return true;
}
bool json_name(std::string_view name) {
  return name.size() > 5 && name.ends_with(".json");
}

/// One directory listing as observed: every name (dot names included) plus the identity of the directory itself.
struct listing {
  std::vector<std::string> names;
  std::uint64_t            device = 0;
  std::uint64_t            inode  = 0;
};
/// Everything one scan observed, compared field by field by the recheck.
struct snapshot {
  std::vector<listing>           listings;
  std::vector<observation_entry> entries;
};

class scanner {
public:
  explicit scanner(int root) : _root(root) {
  }

  std::expected<snapshot, diagnostic> run() {
    snapshot out;
    if (auto done = scan_root(out); !done)
      return std::unexpected(std::move(done.error()));
    std::ranges::stable_sort(out.entries, [](const observation_entry& a, const observation_entry& b) {
      const auto rank = [](const std::string& c) { return c == "checkpoint" ? 0 : c == "check" ? 1 : 2; };
      if (rank(a.category) != rank(b.category))
        return rank(a.category) < rank(b.category);
      return a.name < b.name;
    });
    return out;
  }

private:
  int                                               _root;
  std::size_t                                       _counted = 0;
  std::size_t                                       _total   = 0;
  std::set<std::pair<std::uint64_t, std::uint64_t>> _seen;

  std::expected<std::vector<std::string>, diagnostic> list(int directory, snapshot& out) {
    descriptor fresh(::openat(directory, ".", O_RDONLY | O_DIRECTORY | O_CLOEXEC));
    if (fresh.fd < 0)
      return std::unexpected(cannot_inspect());
    struct stat self{};
    if (::fstat(fresh.fd, &self))
      return std::unexpected(cannot_inspect());
    DIR* raw = ::fdopendir(fresh.fd);
    if (!raw)
      return std::unexpected(cannot_inspect());
    fresh.fd = -1;
    std::unique_ptr<DIR, decltype(&::closedir)> guard(raw, ::closedir);
    std::vector<std::string>                    names;
    errno = 0;
    while (auto e = ::readdir(raw)) {
      const std::string_view name(e->d_name);
      if (name != "." && name != "..") {
        if (++_counted > max_names)
          return std::unexpected(limit("observation_entries", max_names));
        names.emplace_back(name);
      }
      errno = 0;
    }
    if (errno)
      return std::unexpected(cannot_inspect());
    std::ranges::sort(names);
    out.listings.push_back({names, static_cast<std::uint64_t>(self.st_dev), static_cast<std::uint64_t>(self.st_ino)});
    return names;
  }

  std::expected<void, diagnostic> scan_root(snapshot& out) {
    auto names = list(_root, out);
    if (!names)
      return std::unexpected(std::move(names.error()));
    for (const auto& name : *names) {
      if (name.starts_with('.'))
        continue;
      if (!name_valid(name) || (name != "checkpoint.json" && name != "checks" && name != "findings"))
        return std::unexpected(denied_name());
      struct stat status{};
      if (::fstatat(_root, name.c_str(), &status, AT_SYMLINK_NOFOLLOW))
        return std::unexpected(cannot_inspect());
      if (name == "checkpoint.json") {
        if (S_ISDIR(status.st_mode))
          return std::unexpected(denied_name());
        if (auto added = read_file(_root, name, name, "checkpoint", status, out); !added)
          return std::unexpected(std::move(added.error()));
      } else {
        if (S_ISLNK(status.st_mode) || (!S_ISDIR(status.st_mode) && !S_ISREG(status.st_mode)))
          return std::unexpected(denied_kind());
        if (!S_ISDIR(status.st_mode))
          return std::unexpected(denied_name());
        if (auto added = scan_directory(name, name == "checks" ? "check" : "finding", status, out); !added)
          return std::unexpected(std::move(added.error()));
      }
    }
    return {};
  }

  std::expected<void, diagnostic> scan_directory(const std::string& directory, const char* category, const struct stat& expected,
                                                 snapshot& out) {
    descriptor child(::openat(_root, directory.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC));
    if (child.fd < 0)
      return std::unexpected(errno == ELOOP ? denied_kind() : cannot_inspect());
    struct stat opened{};
    if (::fstat(child.fd, &opened) || opened.st_dev != expected.st_dev || opened.st_ino != expected.st_ino)
      return std::unexpected(diagnostic{"capture_changed", "Directory membership or identity changed"});
    auto names = list(child.fd, out);
    if (!names)
      return std::unexpected(std::move(names.error()));
    for (const auto& name : *names) {
      if (name.starts_with('.'))
        continue;
      if (!name_valid(name) || !json_name(name))
        return std::unexpected(denied_name());
      struct stat status{};
      if (::fstatat(child.fd, name.c_str(), &status, AT_SYMLINK_NOFOLLOW))
        return std::unexpected(cannot_inspect());
      if (S_ISDIR(status.st_mode))
        return std::unexpected(denied_name());
      if (auto added = read_file(child.fd, name, directory + "/" + name, category, status, out); !added)
        return std::unexpected(std::move(added.error()));
    }
    return {};
  }

  std::expected<void, diagnostic> read_file(int directory, const std::string& name, const std::string& logical,
                                            const char* category, const struct stat& expected, snapshot& out) {
    if (!S_ISREG(expected.st_mode))
      return std::unexpected(denied_kind());
    const auto identity = std::pair(static_cast<std::uint64_t>(expected.st_dev), static_cast<std::uint64_t>(expected.st_ino));
    if (!_seen.insert(identity).second)
      return std::unexpected(diagnostic{"path_denied", "Hard-link input alias"});
    descriptor file(::openat(directory, name.c_str(), O_RDONLY | O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC));
    if (file.fd < 0)
      return std::unexpected(errno == ELOOP ? denied_kind() : cannot_inspect());
    struct stat before{};
    if (::fstat(file.fd, &before))
      return std::unexpected(cannot_inspect());
    if (!S_ISREG(before.st_mode))
      return std::unexpected(denied_kind());
    if (before.st_dev != expected.st_dev || before.st_ino != expected.st_ino)
      return std::unexpected(diagnostic{"capture_changed", "File changed during read"});
    if (before.st_size < 0 || static_cast<std::uint64_t>(before.st_size) > max_file_bytes)
      return std::unexpected(limit("observation_file_bytes", max_file_bytes));
    const auto size = static_cast<std::size_t>(before.st_size);
    if (size > max_total - _total)
      return std::unexpected(limit("observation_total_bytes", max_total));
    std::string bytes;
    bytes.resize(size);
    std::size_t got = 0;
    while (got < size) {
      const auto n = ::read(file.fd, bytes.data() + got, size - got);
      if (n < 0) {
        if (errno == EINTR)
          continue;
        return std::unexpected(cannot_inspect());
      }
      if (n == 0)
        break;
      got += static_cast<std::size_t>(n);
    }
    char        extra = 0;
    struct stat after{};
    if (got != size || ::read(file.fd, &extra, 1) != 0 || ::fstat(file.fd, &after) || after.st_size != before.st_size)
      return std::unexpected(diagnostic{"capture_changed", "File changed during read"});
    _total += size;
    out.entries.push_back(observation_entry{category, logical, std::move(bytes), static_cast<std::uint64_t>(size),
                                            static_cast<std::uint64_t>(before.st_dev),
                                            static_cast<std::uint64_t>(before.st_ino)});
    return {};
  }
};

/// One failed scan: its diagnostic, and whether it came from a bad_alloc (the private recheck carve-out of D-C01-28).
struct scan_error {
  diagnostic error;
  bool       allocation = false;
};

std::expected<snapshot, scan_error> scan_once(int root) {
  try {
    auto scanned = scanner(root).run();
    if (!scanned)
      return std::unexpected(scan_error{std::move(scanned.error())});
    return std::move(*scanned);
  } catch (const std::bad_alloc&) {
    return std::unexpected(scan_error{limit("observation_total_bytes", max_total), true});
  }
}
} // namespace

namespace recheck_detail {
std::expected<observation_directory, diagnostic> read_observation_directory_impl(std::string_view      path,
                                                                                 std::function<void()> before_recheck) {
  const std::string host(path);
  descriptor        root(::open(host.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC));
  if (root.fd < 0)
    return std::unexpected(cannot_inspect());
  auto first = scan_once(root.fd);
  if (!first)
    return std::unexpected(std::move(first.error().error));
  if (before_recheck)
    before_recheck();
  auto             second = scan_once(root.fd);
  const diagnostic membership{"capture_changed", "Directory membership or identity changed"};
  if (!second) {
    if (second.error().allocation)
      return std::unexpected(std::move(second.error().error));
    return std::unexpected(membership);
  }
  if (first->listings.size() != second->listings.size() || first->entries.size() != second->entries.size())
    return std::unexpected(membership);
  for (std::size_t i = 0; i < first->listings.size(); ++i) {
    const auto &a = first->listings[i], &b = second->listings[i];
    if (a.names != b.names || a.device != b.device || a.inode != b.inode)
      return std::unexpected(membership);
  }
  for (std::size_t i = 0; i < first->entries.size(); ++i) {
    const auto &a = first->entries[i], &b = second->entries[i];
    if (a.name != b.name || a.device != b.device || a.inode != b.inode)
      return std::unexpected(membership);
  }
  for (std::size_t i = 0; i < first->entries.size(); ++i)
    if (first->entries[i].bytes != second->entries[i].bytes)
      return std::unexpected(diagnostic{"capture_changed", "File changed during read"});
  return observation_directory{std::move(first->entries)};
}
} // namespace recheck_detail
std::expected<observation_directory, diagnostic> read_observation_directory(std::string_view path) {
  return recheck_detail::read_observation_directory_impl(path, {});
}
} // namespace planar::engine
