module;
#include <cerrno>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
module planar.engine.capture.native;
import std;
namespace planar::engine {
namespace {
struct descriptor {
  int fd = -1;
  explicit descriptor(int n = -1) : fd(n) {
  }
  ~descriptor() {
    if (fd >= 0)
      ::close(fd);
  }
  descriptor(descriptor&& o) noexcept : fd(std::exchange(o.fd, -1)) {
  }
  descriptor(const descriptor&) = delete;
  descriptor& operator=(descriptor&& o) noexcept {
    if (fd >= 0)
      ::close(fd);
    fd = std::exchange(o.fd, -1);
    return *this;
  }
};
using identity = std::pair<std::uint64_t, std::uint64_t>;
identity id(const struct stat& s) {
  return {s.st_dev, s.st_ino};
}
[[noreturn]] void fail(std::string code, std::string message) {
  throw diagnostic{std::move(code), std::move(message)};
}
[[noreturn]] void limit(std::string name, std::size_t maximum) {
  throw diagnostic{"resource_limit", "Capture limit exceeded", std::move(name), maximum};
}
std::string folded(std::string s) {
  for (auto& c : s)
    if (c >= 'A' && c <= 'Z')
      c += 'a' - 'A';
  return s;
}
bool stable(const struct stat& a, const struct stat& b) {
#ifdef __APPLE__
  auto mt_a = a.st_mtimespec, mt_b = b.st_mtimespec, ct_a = a.st_ctimespec, ct_b = b.st_ctimespec;
#else
  auto mt_a = a.st_mtim, mt_b = b.st_mtim, ct_a = a.st_ctim, ct_b = b.st_ctim;
#endif
  return id(a) == id(b) && a.st_size == b.st_size && a.st_mode == b.st_mode && mt_a.tv_sec == mt_b.tv_sec &&
         mt_a.tv_nsec == mt_b.tv_nsec && ct_a.tv_sec == ct_b.tv_sec && ct_a.tv_nsec == ct_b.tv_nsec;
}
struct stat status(int fd) {
  struct stat s{};
  if (::fstat(fd, &s))
    fail("io", "Cannot inspect input");
  return s;
}
descriptor child(int parent, const std::string& path, bool directory) {
  int fd = ::openat(parent, path.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_NONBLOCK | (directory ? O_DIRECTORY : 0));
  if (fd < 0)
    fail(errno == ELOOP || errno == ENOTDIR ? "path_denied" : "io", "Cannot open confined input");
  return descriptor(fd);
}
descriptor open_root(const std::string& path, bool physical_parents = false) {
  if (path.empty() || path[0] != '/')
    fail("path_denied", "Root must be absolute");
  if (physical_parents && !valid_text(path))
    fail("path_denied", "Invalid capture root");
  descriptor d(::open("/", O_RDONLY | O_DIRECTORY | O_CLOEXEC));
  if (physical_parents && d.fd < 0)
    fail("io", "Cannot open confined input");
  std::size_t start = 1;
  while (start < path.size()) {
    auto end = path.find('/', start);
    if (end == std::string::npos)
      end = path.size();
    auto segment = path.substr(start, end - start);
    if (segment == ".." && !physical_parents)
      fail("path_denied", "Parent root traversal forbidden");
    if (!segment.empty() && segment != ".")
      d = child(d.fd, segment, true);
    start = end + 1;
  }
  return d;
}
bool ancestor(identity target, int fd) {
  descriptor d(::dup(fd));
  for (;;) {
    auto current = id(status(d.fd));
    if (current == target)
      return true;
    auto parent = child(d.fd, "..", true);
    if (id(status(parent.fd)) == current)
      return false;
    d = std::move(parent);
  }
}
} // namespace
/// @cond
struct native_capture_session::implementation {
  struct root {
    std::string                          name;
    descriptor                           fd;
    identity                             physical;
    std::string                          host;
    std::string                          input_name;
    bool                                 physical_parents = false;
    bool                                 bound_input      = false;
    std::optional<retained_file_receipt> pending          = std::nullopt;
    std::optional<retained_root_receipt> authority        = std::nullopt;
  };
  using directory_map = std::map<std::pair<std::string, std::string>, identity>;
  struct retained {
    captured_file                        file;
    identity                             physical;
    bool                                 text;
    bool                                 bounded;
    directory_map                        ancestors;
    std::optional<retained_file_receipt> authority;
  };
  std::vector<root>                                          roots;
  std::map<std::pair<std::string, std::string>, retained>    files;
  std::map<std::string, std::vector<std::string>>            trees;
  std::map<std::pair<std::string, std::string>, std::string> spelling;
  std::map<identity, std::pair<std::string, std::string>>    identities;
  directory_map                                              directories;
  directory_map                                              ancestors;
  capture_topology                                           topology;
  struct enumeration_budget {
    std::size_t  entries = 0;
    std::size_t  bytes   = 0;
    budget_sink* sink    = nullptr;
    void         charge(std::size_t prefix, std::size_t name) {
      if (entries == 100000)
        limit("enumeration_entries", 100000);
      // Cover staging, spelling, retained/verified topology, and the sealed logical copy.
      auto charge = 512 + 6 * (prefix + name + 1) + 2 * (name + 1);
      if (charge > 67108864 - bytes)
        limit("enumeration_bytes", 67108864);
      if (sink)
        if (auto charged = sink->charge_native(charge); !charged)
          throw std::move(charged.error());
      ++entries;
      bytes += charge;
    }
  } discovery_budget, ancestor_budget;
  std::shared_ptr<budget_sink> sink;
  std::size_t                  bytes  = 0;
  bool                         sealed = false;
  bool                         failed = false;
  std::function<void()>        seam;
  root&                        get(std::string_view name, bool verifying = false) {
    if (failed)
      fail("capture_changed", "Bound input capture already failed");
    if (sealed && !verifying)
      fail("internal", "Capture session already sealed");
    for (auto& r : roots)
      if (r.name == name)
        return r;
    fail("path_denied", "Unknown capture root");
  }
  void alias(const std::string& root_name, const std::string& path) {
    std::size_t at = 0;
    for (;;) {
      at                  = path.find('/', at);
      auto prefix         = path.substr(0, at);
      auto [it, inserted] = spelling.emplace(std::pair(root_name, folded(prefix)), prefix);
      if (!inserted && it->second != prefix)
        fail("path_denied", "Portable case alias");
      if (at == std::string::npos)
        break;
      ++at;
    }
  }
  void poison() noexcept {
    for (const auto& r : roots) {
      if (r.authority) {
        failed = true;
        r.authority->poison();
      }
      if (r.bound_input) {
        failed = true;
        if (r.pending)
          r.pending->poison();
      }
    }
    for (const auto& [key, entry] : files)
      if (entry.authority)
        entry.authority->poison();
  }
  struct operation {
    implementation*              state;
    const retained_file_receipt* incoming = nullptr;
    bool                         complete = false;
    ~operation() {
      if (!complete) {
        if (state)
          state->poison();
        if (incoming)
          incoming->poison();
      }
    }
  };
  retained read_file(root& r, const std::string& path, bool text, bool verifying = false,
                     const capture_admission* admission = nullptr, std::string_view slot = {},
                     std::optional<std::size_t> byte_ceiling = {}, std::size_t remaining_bytes = 0,
                     const retained_file_receipt* authority = nullptr) {
    // A pending standalone input and a captured entry impose the same original
    // authority. Repeated retained handles may identify it, never replace it.
    auto                         known    = files.find({r.name, path});
    const retained_file_receipt* original = nullptr;
    if (known != files.end() && known->second.authority)
      original = &*known->second.authority;
    else if (r.bound_input) {
      if (!r.pending)
        fail("capture_changed", "Bound input authority unavailable");
      original = &*r.pending;
    }
    if (original) {
      if (admission || (authority && !original->same_selection(*authority))) {
        original->poison();
        if (authority)
          authority->poison();
        if (admission)
          admission->poison();
        fail("capture_changed", "Retained file authority cannot be replaced");
      }
      authority = original;
      if (known != files.end()) {
        remaining_bytes =
            byte_ceiling ? std::min(remaining_bytes, known->second.file.bytes.size()) : known->second.file.bytes.size();
        if (!byte_ceiling || *byte_ceiling > known->second.file.bytes.size())
          byte_ceiling = known->second.file.bytes.size();
      }
    }
    operation                            guard{this, authority};
    std::optional<retained_file_receipt> discovered;
    if (r.authority) {
      auto verified = r.authority->verify(r.fd.fd);
      if (!verified)
        throw std::move(verified.error());
      if (admission || (known != files.end() && !known->second.authority))
        fail("capture_changed", "Root authority cannot upgrade an unbound entry");
      if (!authority || known == files.end()) {
        if (verifying || sealed)
          fail("capture_changed", "Discovery after initial capture");
        // First capture must select through the original root even when a file
        // receipt was supplied. The temporary comparison issuance and staging
        // retain discover's cumulative charges; cached authority needs no issue.
        auto selected = r.authority->discover(path);
        if (!selected)
          throw std::move(selected.error());
        if (authority) {
          if (!selected->same_selection(*authority))
            fail("capture_changed", "Retained file authority cannot be replaced");
        } else {
          discovered.emplace(std::move(*selected));
          authority = &*discovered;
        }
      }
    }
    if (!valid_path(path))
      fail("path_denied", "Invalid portable file path");
    if (!sealed && !verifying)
      alias(r.name, path);
    if (!r.input_name.empty() && path != "input.json")
      fail("path_denied", "Standalone input path must be input.json");
    directory_map traversed;
    auto          slash = path.find('/');
    descriptor    directory(::dup(r.fd.fd));
    std::size_t   begin = 0;
    while (slash != std::string::npos) {
      directory     = child(directory.fd, path.substr(begin, slash - begin), true);
      auto key      = std::pair(r.name, path.substr(0, slash));
      auto physical = id(status(directory.fd));
      auto known    = ancestors.find(key);
      if ((known != ancestors.end() && known->second != physical) || (verifying && known == ancestors.end()))
        fail("capture_changed", "Selected ancestor identity changed");
      if (!verifying)
        traversed.emplace(std::move(key), physical);
      begin = slash + 1;
      slash = path.find('/', begin);
    }
    auto        name = r.input_name.empty() ? path.substr(begin) : r.input_name;
    struct stat named{};
    if (::fstatat(directory.fd, name.c_str(), &named, AT_SYMLINK_NOFOLLOW))
      fail("io", "Cannot inspect confined input");
    if (!S_ISREG(named.st_mode))
      fail("path_denied", "Only regular files can be captured");
    auto f      = child(directory.fd, name, false);
    auto before = status(f.fd);
    if (id(named) != id(before))
      fail("capture_changed", "File identity changed before read");
    if (!S_ISREG(before.st_mode))
      fail("path_denied", "Only regular files can be captured");
    if (authority) {
      auto verified = authority->verify(f.fd);
      if (!verified)
        throw std::move(verified.error());
    }
    if (admission) {
      auto admitted = admission->verify_file(slot, f.fd);
      if (!admitted)
        throw admitted.error();
    }
    if (byte_ceiling && (before.st_size < 0 || static_cast<std::uint64_t>(before.st_size) > *byte_ceiling ||
                         static_cast<std::uint64_t>(before.st_size) > remaining_bytes))
      fail(verifying ? "capture_changed" : "resource_limit",
           verifying ? "Captured external profile changed" : "External profile resource limit exceeded");
    auto maximum = text ? 1048576u : 16777216u;
    if (before.st_size < 0 || static_cast<std::uint64_t>(before.st_size) > maximum) {
      // A retained file already met its quota; reject changed size before allocating or reading.
      if (verifying)
        fail("capture_changed", "Captured file size changed beyond quota");
      limit("file_bytes", maximum);
    }
    // Budget before allocation, also during the final reread.
    if (static_cast<std::uint64_t>(before.st_size) > 67108864 - bytes && !files.contains({r.name, path}))
      limit("retained_bytes", 67108864);
    // Composed factory ceiling: charged before any read or allocation of a new entry.
    if (sink && !verifying && !files.contains({r.name, path}))
      if (auto charged = sink->charge_native(static_cast<std::size_t>(before.st_size) + path.size() + r.name.size() + 4 +
                                             sizeof(captured_file));
          !charged)
        throw std::move(charged.error());
    std::string data(static_cast<std::size_t>(before.st_size), '\0');
    std::size_t offset = 0;
    while (offset < data.size()) {
      auto count = ::read(f.fd, data.data() + offset, data.size() - offset);
      if (count < 0 && errno == EINTR)
        continue;
      if (count <= 0)
        fail("capture_changed", "File changed during read");
      offset += count;
    }
    char extra;
    auto n = ::read(f.fd, &extra, 1);
    if (n != 0 || !stable(before, status(f.fd)))
      fail("capture_changed", "File changed during read");
    struct stat linked{};
    if (::fstatat(directory.fd, name.c_str(), &linked, AT_SYMLINK_NOFOLLOW) || id(linked) != id(before))
      fail("capture_changed", "File identity changed during read");
    if (text && !valid_text(data))
      fail("schema", "Captured text is invalid UTF-8 or contains NUL");
    guard.complete = true;
    return {{r.name, path, (before.st_mode & 0111) ? "0755" : "0644", std::move(data)},
            id(before),
            text,
            byte_ceiling.has_value(),
            std::move(traversed),
            std::move(discovered)};
  }
  void retain(retained entry) {
    auto& r = get(entry.file.root);
    // Move the issued binding handle itself only after successful content capture.
    // Leave the pending optional engaged until publication succeeds: any failed
    // publication is terminal and can never expose an unbound fallback.
    if (r.bound_input && r.pending)
      entry.authority.emplace(std::move(*r.pending));
    operation guard{this, entry.authority ? &*entry.authority : nullptr};
    auto      key = std::pair(entry.file.root, entry.file.path);
    if (files.contains(key)) {
      guard.complete = true;
      return;
    }
    if (files.size() >= 10000)
      limit("files", 10000);
    if (entry.file.bytes.size() > 67108864 - bytes)
      limit("retained_bytes", 67108864);
    // Stage only ancestors of a successful retained read. Unique receipts share the
    // finite tree-entry/byte ceiling, independently of unrelated tree enumeration.
    auto budget = ancestor_budget;
    for (const auto& [ancestor_key, physical] : entry.ancestors)
      if (!ancestors.contains(ancestor_key))
        budget.charge(ancestor_key.first.size(), ancestor_key.second.size());
    auto [it, inserted] = identities.emplace(entry.physical, key);
    if (!inserted && it->second != key)
      fail("path_denied", "Hard-link input alias");
    ancestors.merge(entry.ancestors);
    entry.ancestors.clear();
    ancestor_budget = budget;
    auto size       = entry.file.bytes.size();
    files.emplace(std::move(key), std::move(entry));
    bytes += size;
    if (r.bound_input)
      r.pending.reset();
    guard.complete = true;
  }
  bool excluded(const std::string& p, const std::vector<std::string>& exclusions) {
    for (auto& x : exclusions)
      if (p == x || (x.back() == '/' && p.starts_with(x)))
        return true;
    return false;
  }
  void enumerate(root& r, int fd, const std::string& prefix, const std::vector<std::string>& exclusions,
                 std::set<std::string>& found, directory_map& found_directories, enumeration_budget& budget, bool verifying) {
    if (r.authority) {
      auto verified = r.authority->verify(r.fd.fd);
      if (!verified)
        throw std::move(verified.error());
    }
    auto before = status(fd);
    auto fresh  = child(fd, ".", true);
    DIR* raw    = ::fdopendir(fresh.fd);
    if (!raw)
      fail("io", "Cannot enumerate input");
    fresh.fd                       = -1;
    auto                     close = std::unique_ptr<DIR, decltype(&::closedir)>(raw, ::closedir);
    std::vector<std::string> names;
    errno = 0;
    while (auto e = ::readdir(raw)) {
      std::string_view name(e->d_name);
      if (name != "." && name != "..") {
        budget.charge(prefix.size(), name.size());
        names.emplace_back(name);
      }
      errno = 0;
    }
    if (errno)
      fail("io", "Cannot enumerate input");
    std::ranges::sort(names);
    for (auto& name : names) {
      auto        path = prefix + name;
      struct stat s{};
      if (::fstatat(fd, name.c_str(), &s, AT_SYMLINK_NOFOLLOW))
        fail("capture_changed", "Tree membership changed");
      if (name == ".git" && S_ISDIR(s.st_mode))
        continue;
      if (!valid_path(path))
        fail("path_denied", "Invalid captured path");
      if (!sealed && !verifying)
        alias(r.name, path);
      if (!r.input_name.empty() && path != "input.json")
        fail("path_denied", "Standalone input path must be input.json");
      if (excluded(path, exclusions) || excluded(path + "/", exclusions))
        continue;
      if (S_ISDIR(s.st_mode)) {
        auto d = child(fd, name, true);
        if (id(status(d.fd)) != id(s))
          fail("capture_changed", "Directory identity changed");
        auto key = std::pair(r.name, path);
        if (verifying) {
          auto known = directories.find(key);
          if (known == directories.end() || known->second != id(s))
            fail("capture_changed", "Directory membership or identity changed");
        }
        found_directories.emplace(std::move(key), id(s));
        enumerate(r, d.fd, path + "/", exclusions, found, found_directories, budget, verifying);
        struct stat after{};
        if (::fstatat(fd, name.c_str(), &after, AT_SYMLINK_NOFOLLOW) || id(after) != id(s))
          fail("capture_changed", "Directory identity changed");
      } else {
        if (!S_ISREG(s.st_mode))
          fail("path_denied", "Links and special files forbidden");
        found.insert(path);
        if (!verifying)
          retain(read_file(r, path, false));
      }
    }
    if (!stable(before, status(fd)))
      fail("capture_changed", "Directory changed during traversal");
  }
  void verify() {
    operation guard{this};
    if (failed)
      fail("capture_changed", "Bound input capture already failed");
    enumeration_budget verification_budget;
    verification_budget.sink = sink.get();
    directory_map verified_directories;
    for (auto& r : roots) {
      auto reopened = open_root(r.host, r.physical_parents);
      if (id(status(reopened.fd)) != r.physical || id(status(r.fd.fd)) != r.physical)
        fail("capture_changed", "Root identity changed");
      if (r.authority) {
        auto original = r.authority->verify(reopened.fd);
        if (!original)
          throw std::move(original.error());
      }
      if (r.bound_input && r.pending) {
        auto leaf     = child(reopened.fd, r.input_name, false);
        auto verified = r.pending->verify(leaf.fd);
        if (!verified)
          throw std::move(verified.error());
      }
      auto tree = trees.find(r.name);
      if (tree != trees.end()) {
        std::set<std::string> found, expected;
        enumerate(r, r.fd.fd, "", tree->second, found, verified_directories, verification_budget, true);
        for (auto& [key, f] : files)
          if (key.first == r.name && !excluded(key.second, tree->second))
            expected.insert(key.second);
        if (found != expected)
          fail("capture_changed", "Tree membership changed");
      }
    }
    if (verified_directories != directories)
      fail("capture_changed", "Directory membership or identity changed");
    for (auto& [key, f] : files) {
      // Bounded captures may never allocate/read more than their original charge
      // at any reread. A grown file refuses before content access; old callers
      // retain the original inherited verification behavior.
      auto ceiling = f.bounded ? std::optional(f.file.bytes.size()) : std::nullopt;
      auto fresh   = read_file(get(key.first, true), key.second, f.text, true, nullptr, {}, ceiling, f.file.bytes.size());
      if (fresh.physical != f.physical || fresh.file.mode != f.file.mode || fresh.file.bytes != f.file.bytes)
        fail("capture_changed", "Captured file changed");
    }
    guard.complete = true;
  }
};
/// @endcond
native_capture_session::native_capture_session() : _impl(std::make_unique<implementation>()) {
}
native_capture_session::~native_capture_session() = default;
std::expected<void, diagnostic> native_capture_session::add_root(std::string logical, std::string directory) {
  try {
    if (!_impl || _impl->sealed || _impl->failed || logical.empty())
      fail("path_denied", "Invalid capture root");
    auto fd       = open_root(directory);
    auto physical = id(status(fd.fd));
    for (auto& r : _impl->roots)
      if (r.name == logical || (r.input_name.empty() && (ancestor(physical, r.fd.fd) || ancestor(r.physical, fd.fd))))
        fail("path_denied", "Capture roots overlap");
    _impl->roots.push_back({std::move(logical), std::move(fd), physical, std::move(directory), {}});
    return {};
  } catch (diagnostic& e) {
    return std::unexpected(std::move(e));
  }
}
std::expected<void, diagnostic> native_capture_session::add_root(std::string logical, std::string directory,
                                                                 const capture_admission& admission, std::string_view slot) {
  try {
    if (!_impl || _impl->sealed || _impl->failed || logical.empty())
      fail("path_denied", "Invalid capture root");
    auto fd       = open_root(directory, true);
    auto verified = admission.verify_root(slot, fd.fd);
    if (!verified)
      return std::unexpected(std::move(verified.error()));
    auto physical = id(status(fd.fd));
    for (const auto& r : _impl->roots)
      if (r.name == logical || (r.input_name.empty() && (ancestor(physical, r.fd.fd) || ancestor(r.physical, fd.fd))))
        fail("path_denied", "Capture roots overlap");
    _impl->roots.push_back({std::move(logical), std::move(fd), physical, std::move(directory), {}, true});
    return {};
  } catch (diagnostic& e) {
    return std::unexpected(std::move(e));
  } catch (const std::bad_alloc&) {
    return std::unexpected(diagnostic{"resource_limit", "Capture limit exceeded"});
  }
}
std::expected<void, diagnostic> native_capture_session::add_root(std::string logical, std::string directory,
                                                                 retained_root_receipt receipt) {
  try {
    if (!_impl || _impl->sealed || _impl->failed || logical.empty())
      fail("path_denied", "Invalid capture root");
    auto fd       = open_root(directory, true);
    auto verified = receipt.verify(fd.fd);
    if (!verified)
      throw std::move(verified.error());
    auto physical = id(status(fd.fd));
    for (const auto& r : _impl->roots)
      if (r.name == logical || (r.input_name.empty() && (ancestor(physical, r.fd.fd) || ancestor(r.physical, fd.fd))))
        fail("path_denied", "Capture roots overlap");
    _impl->roots.reserve(_impl->roots.size() + 1);
    implementation::root value{std::move(logical), std::move(fd), physical, std::move(directory), {}, true};
    value.authority.emplace(std::move(receipt));
    _impl->roots.push_back(std::move(value));
    return {};
  } catch (diagnostic& e) {
    receipt.poison();
    if (_impl) {
      _impl->failed = true;
      _impl->poison();
    }
    return std::unexpected(std::move(e));
  } catch (const std::bad_alloc&) {
    receipt.poison();
    if (_impl) {
      _impl->failed = true;
      _impl->poison();
    }
    return std::unexpected(diagnostic{"resource_limit", "Capture limit exceeded"});
  }
}
std::expected<void, diagnostic> native_capture_session::add_input(std::string logical, std::string file) {
  try {
    if (!_impl || _impl->sealed || _impl->failed || logical.empty())
      fail("path_denied", "Invalid input root");
    for (const auto& r : _impl->roots)
      if (r.name == logical)
        fail("path_denied", "Duplicate input root");
    auto slash = file.rfind('/');
    if (slash == std::string::npos || slash + 1 == file.size())
      fail("path_denied", "Input must be an absolute file path");
    auto parent = file.substr(0, slash);
    if (parent.empty())
      parent = "/";
    auto name = file.substr(slash + 1);
    if (name == "." || name == "..")
      fail("path_denied", "Invalid input file");
    auto fd       = open_root(parent);
    auto physical = id(status(fd.fd));
    _impl->roots.push_back({std::move(logical), std::move(fd), physical, std::move(parent), std::move(name)});
    return {};
  } catch (diagnostic& e) {
    return std::unexpected(std::move(e));
  }
}
std::expected<void, diagnostic> native_capture_session::add_input(std::string logical, std::string file,
                                                                  retained_file_receipt receipt) {
  implementation::operation guard{_impl.get(), &receipt};
  try {
    if (!_impl || _impl->sealed || _impl->failed || logical.empty() || !valid_text(file))
      fail("path_denied", "Invalid input root");
    auto slash = file.rfind('/');
    if (slash == std::string::npos || slash + 1 == file.size())
      fail("path_denied", "Input must be an absolute file path");
    auto parent = file.substr(0, slash);
    if (parent.empty())
      parent = "/";
    auto name = file.substr(slash + 1);
    if (name == "." || name == "..")
      fail("path_denied", "Invalid input file");
    auto fd       = open_root(parent, true);
    auto leaf     = child(fd.fd, name, false);
    auto verified = receipt.verify(leaf.fd);
    if (!verified)
      throw std::move(verified.error());
    for (const auto& r : _impl->roots)
      if (r.name == logical)
        fail("path_denied", "Duplicate input root");
    auto physical = id(status(fd.fd));
    // Vector allocation happens before the noexcept move of the token/root.
    _impl->roots.reserve(_impl->roots.size() + 1);
    _impl->roots.push_back(
        {std::move(logical), std::move(fd), physical, std::move(parent), std::move(name), true, true, std::move(receipt)});
    guard.complete = true;
    return {};
  } catch (diagnostic& e) {
    if (_impl) {
      _impl->failed = true;
      _impl->poison();
    }
    receipt.poison();
    return std::unexpected(std::move(e));
  } catch (const std::bad_alloc&) {
    if (_impl) {
      _impl->failed = true;
      _impl->poison();
    }
    receipt.poison();
    return std::unexpected(diagnostic{"resource_limit", "Capture limit exceeded"});
  }
}
std::expected<captured_file, diagnostic> native_capture_session::read(std::string_view root, std::string_view path, bool text) {
  implementation::operation guard{_impl.get()};
  try {
    if (!_impl)
      fail("internal", "Capture session already transferred");
    auto& r   = _impl->get(root);
    auto  key = std::pair(std::string(root), std::string(path));
    auto  it  = _impl->files.find(key);
    if (it == _impl->files.end()) {
      if (_impl->files.size() >= 10000)
        limit("files", 10000);
      _impl->retain(_impl->read_file(r, key.second, text));
      it = _impl->files.find(key);
    }
    if (it->second.authority) {
      auto fresh =
          _impl->read_file(r, key.second, text, true, nullptr, {}, it->second.file.bytes.size(), it->second.file.bytes.size());
      if (fresh.file.bytes != it->second.file.bytes || fresh.file.mode != it->second.file.mode ||
          fresh.physical != it->second.physical)
        fail("capture_changed", "Captured input changed");
    }
    if (text && it->second.file.bytes.size() > 1048576)
      limit("file_bytes", 1048576);
    if (text && !valid_text(it->second.file.bytes))
      fail("schema", "Invalid text reference");
    if (text)
      it->second.text = true;
    auto result    = it->second.file;
    guard.complete = true;
    return result;
  } catch (diagnostic& e) {
    if (_impl)
      _impl->poison();
    return std::unexpected(std::move(e));
  } catch (const std::bad_alloc&) {
    if (_impl)
      _impl->poison();
    return std::unexpected(diagnostic{"resource_limit", "Capture limit exceeded"});
  }
}
std::expected<captured_file, diagnostic> native_capture_session::read(std::string_view root, std::string_view path,
                                                                      const capture_admission& admission, std::string_view slot,
                                                                      bool text) {
  implementation::operation guard{_impl.get()};
  try {
    if (!_impl)
      fail("internal", "Capture session already transferred");
    auto& r   = _impl->get(root);
    auto  key = std::pair(std::string(root), std::string(path));
    if (!_impl->files.contains(key) && _impl->files.size() >= 10000)
      limit("files", 10000);
    auto fresh    = _impl->read_file(r, key.second, text, false, &admission, slot);
    auto existing = _impl->files.find(key);
    if (existing != _impl->files.end()) {
      if (existing->second.physical != fresh.physical || existing->second.file.bytes != fresh.file.bytes ||
          existing->second.file.mode != fresh.file.mode)
        fail("capture_changed", "Captured external profile changed");
      if (text)
        existing->second.text = true;
      auto result    = existing->second.file;
      guard.complete = true;
      return result;
    }
    _impl->retain(std::move(fresh));
    auto result    = _impl->files.at(key).file;
    guard.complete = true;
    return result;
  } catch (diagnostic& e) {
    if (_impl)
      _impl->poison();
    return std::unexpected(std::move(e));
  } catch (const std::bad_alloc&) {
    if (_impl)
      _impl->poison();
    return std::unexpected(diagnostic{"resource_limit", "Capture limit exceeded"});
  }
}
std::expected<captured_file, diagnostic> native_capture_session::read_bounded(std::string_view root, std::string_view path,
                                                                              const capture_admission& admission,
                                                                              std::string_view slot, std::size_t byte_ceiling,
                                                                              std::size_t remaining_bytes, bool text) {
  implementation::operation guard{_impl.get()};
  try {
    if (!_impl)
      fail("internal", "Capture session already transferred");
    auto& r   = _impl->get(root);
    auto  key = std::pair(std::string(root), std::string(path));
    if (!_impl->files.contains(key) && _impl->files.size() >= 10000)
      limit("files", 10000);
    auto fresh    = _impl->read_file(r, key.second, text, false, &admission, slot, byte_ceiling, remaining_bytes);
    auto existing = _impl->files.find(key);
    if (existing != _impl->files.end()) {
      if (existing->second.physical != fresh.physical || existing->second.file.bytes != fresh.file.bytes ||
          existing->second.file.mode != fresh.file.mode)
        fail("capture_changed", "Captured external profile changed");
      existing->second.bounded = true;
      if (text)
        existing->second.text = true;
      auto result    = existing->second.file;
      guard.complete = true;
      return result;
    }
    _impl->retain(std::move(fresh));
    auto result    = _impl->files.at(key).file;
    guard.complete = true;
    return result;
  } catch (diagnostic& e) {
    if (_impl)
      _impl->poison();
    return std::unexpected(std::move(e));
  } catch (const std::bad_alloc&) {
    if (_impl)
      _impl->poison();
    return std::unexpected(diagnostic{"resource_limit", "Capture limit exceeded"});
  }
}
std::expected<captured_file, diagnostic> native_capture_session::read(std::string_view root, std::string_view path,
                                                                      retained_file_receipt receipt, bool text) {
  return read_bounded(root, path, std::move(receipt), text ? 1048576 : 16777216, 67108864, text);
}
std::expected<captured_file, diagnostic> native_capture_session::read_bounded(std::string_view root, std::string_view path,
                                                                              retained_file_receipt receipt,
                                                                              std::size_t           byte_ceiling,
                                                                              std::size_t remaining_bytes, bool text) {
  implementation::operation guard{_impl.get(), &receipt};
  try {
    if (!_impl)
      fail("internal", "Capture session already transferred");
    auto& r        = _impl->get(root);
    auto  key      = std::pair(std::string(root), std::string(path));
    auto  existing = _impl->files.find(key);
    if (existing != _impl->files.end() && (!existing->second.authority || !existing->second.authority->same_selection(receipt)))
      fail("capture_changed", "Retained file authority cannot be replaced");
    if (existing == _impl->files.end() && _impl->files.size() >= 10000)
      limit("files", 10000);
    auto fresh = _impl->read_file(r, key.second, text, false, nullptr, {}, byte_ceiling, remaining_bytes, &receipt);
    if (existing != _impl->files.end()) {
      if (existing->second.physical != fresh.physical || existing->second.file.bytes != fresh.file.bytes ||
          existing->second.file.mode != fresh.file.mode)
        fail("capture_changed", "Captured input changed");
      auto result = existing->second.file;
      if (text)
        existing->second.text = true;
      guard.complete = true;
      return result;
    }
    auto result = fresh.file;
    if (!r.bound_input)
      fresh.authority.emplace(std::move(receipt));
    _impl->retain(std::move(fresh));
    guard.complete = true;
    return result;
  } catch (diagnostic& e) {
    if (_impl)
      _impl->poison();
    receipt.poison();
    return std::unexpected(std::move(e));
  } catch (const std::bad_alloc&) {
    if (_impl)
      _impl->poison();
    receipt.poison();
    return std::unexpected(diagnostic{"resource_limit", "Capture limit exceeded"});
  }
}
std::expected<void, diagnostic> native_capture_session::tree(std::string_view root, std::vector<std::string> exclusions) {
  implementation::operation guard{_impl.get()};
  try {
    if (!_impl)
      fail("internal", "Capture session already transferred");
    auto& r = _impl->get(root);
    if (!r.input_name.empty())
      fail("path_denied", "Standalone input has no tree capability");
    for (auto& x : exclusions)
      if (!valid_path(x, x.ends_with('/')))
        fail("path_denied", "Invalid excluded boundary");
    if (_impl->trees.contains(r.name))
      fail("internal", "Tree already discovered");
    std::set<std::string> found;
    _impl->enumerate(r, r.fd.fd, "", exclusions, found, _impl->directories, _impl->discovery_budget, false);
    _impl->trees.emplace(r.name, std::move(exclusions));
    guard.complete = true;
    return {};
  } catch (diagnostic& e) {
    if (_impl)
      _impl->poison();
    return std::unexpected(std::move(e));
  } catch (const std::bad_alloc&) {
    if (_impl)
      _impl->poison();
    return std::unexpected(diagnostic{"resource_limit", "Capture limit exceeded"});
  }
}
void native_capture_session::set_budget(std::shared_ptr<budget_sink> budget) {
  if (!_impl)
    return;
  _impl->sink                  = std::move(budget);
  _impl->discovery_budget.sink = _impl->sink.get();
  _impl->ancestor_budget.sink  = _impl->sink.get();
}
void native_capture_session::before_verification(std::function<void()> callback) {
  if (_impl && !_impl->sealed)
    _impl->seam = std::move(callback);
}
std::expected<capture_snapshot, diagnostic> native_capture_session::finish() {
  implementation::operation guard{_impl.get()};
  try {
    if (!_impl || _impl->sealed)
      fail("internal", "Capture session already sealed");
    if (_impl->seam)
      _impl->seam();
    _impl->verify();
    auto topology = std::make_shared<std::vector<captured_directory>>();
    for (auto& [key, physical] : _impl->directories)
      topology->push_back({key.first, key.second});
    auto result = std::make_shared<std::vector<captured_file>>();
    for (auto& [key, f] : _impl->files)
      result->push_back(f.file);
    _impl->topology = std::move(topology);
    _impl->sealed   = true;
    guard.complete  = true;
    return capture_snapshot(std::move(result));
  } catch (diagnostic& e) {
    if (_impl)
      _impl->poison();
    if (_impl)
      _impl->sealed = true;
    if (e.code == "io" || e.code == "path_denied" || e.code == "schema")
      e = {"capture_changed", "Captured inputs changed during verification"};
    return std::unexpected(std::move(e));
  } catch (const std::bad_alloc&) {
    if (_impl)
      _impl->poison();
    return std::unexpected(diagnostic{"resource_limit", "Capture limit exceeded"});
  }
}
std::expected<capture_topology, diagnostic> native_capture_session::directories() const {
  if (!_impl || !_impl->topology)
    return std::unexpected(diagnostic{"internal", "Capture topology requires successful sealing"});
  return _impl->topology;
}
/// @cond
struct acceptance_capture::implementation {
  std::unique_ptr<native_capture_session::implementation> receipts;
  capture_snapshot                                        snapshot;
};
/// @endcond
acceptance_capture::acceptance_capture(std::unique_ptr<implementation> state) : _impl(std::move(state)) {
}
acceptance_capture::~acceptance_capture()                             = default;
acceptance_capture::acceptance_capture(acceptance_capture&&) noexcept = default;
acceptance_capture& acceptance_capture::operator=(acceptance_capture&& other) noexcept {
  if (this != &other)
    _impl = std::move(other._impl);
  return *this;
}
std::expected<acceptance_capture, diagnostic> native_capture_session::finish_for_acceptance() {
  implementation::operation guard{_impl.get()};
  try {
    auto snapshot = finish();
    if (!snapshot)
      return std::unexpected(std::move(snapshot.error()));
    auto state      = std::make_unique<acceptance_capture::implementation>();
    _impl->seam     = {};
    state->snapshot = std::move(*snapshot);
    state->receipts = std::move(_impl);
    guard.complete  = true;
    return acceptance_capture(std::move(state));
  } catch (const std::bad_alloc&) {
    if (_impl)
      _impl->poison();
    return std::unexpected(diagnostic{"resource_limit", "Capture limit exceeded"});
  }
}
std::expected<capture_snapshot, diagnostic> acceptance_capture::snapshot() const {
  if (!_impl)
    return std::unexpected(diagnostic{"capture_changed", "Capture capability has been transferred"});
  return _impl->snapshot;
}
std::expected<capture_topology, diagnostic> acceptance_capture::directories() const {
  if (!_impl)
    return std::unexpected(diagnostic{"capture_changed", "Capture capability has been transferred"});
  return _impl->receipts->topology;
}
std::expected<void, diagnostic> acceptance_capture::verify() const {
  if (!_impl)
    return std::unexpected(diagnostic{"capture_changed", "Capture capability has been transferred"});
  try {
    _impl->receipts->verify();
    return {};
  } catch (...) {
    _impl->receipts->poison();
    return std::unexpected(diagnostic{"capture_changed", "Captured inputs changed during verification"});
  }
}
} // namespace planar::engine
