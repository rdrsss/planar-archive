module;
#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
module planar.engine.capture.admission;
import std;
namespace planar::engine {
namespace {
// The inherited native ancestor budget is independent of content retention:
// <=100000 metadata receipts/slots, <=64 MiB charged storage; selected files <=10000.
constexpr std::size_t max_receipts = 100000, max_storage = 67108864, max_files = 10000;
struct descriptor {
  int fd = -1;
  explicit descriptor(int value = -1) : fd(value) {
  }
  ~descriptor() {
    if (fd >= 0)
      ::close(fd);
  }
  descriptor(descriptor&& other) noexcept : fd(std::exchange(other.fd, -1)) {
  }
  descriptor& operator=(descriptor&& other) noexcept {
    if (this != &other) {
      if (fd >= 0)
        ::close(fd);
      fd = std::exchange(other.fd, -1);
    }
    return *this;
  }
  descriptor(const descriptor&) = delete;
};
using identity = std::pair<std::uint64_t, std::uint64_t>;
identity id(const struct stat& s) {
  return {s.st_dev, s.st_ino};
}
[[noreturn]] void changed() {
  throw diagnostic{"capture_changed", "Captured external profile changed"};
}
[[noreturn]] void overlap() {
  throw diagnostic{"unsafe_path", "External profile overlaps a supplied input"};
}
[[noreturn]] void exhausted() {
  throw diagnostic{"resource_limit", "External profile resource limit exceeded"};
}
[[noreturn]] void invalid() {
  throw diagnostic{"path_denied", "Invalid input file"};
}
[[noreturn]] void io_error() {
  throw diagnostic{errno == ELOOP || errno == ENOTDIR ? "path_denied" : "io", "Cannot open confined input"};
}
struct stat status(int fd) {
  struct stat s{};
  if (::fstat(fd, &s))
    io_error();
  return s;
}
bool same_directory(const struct stat& a, const struct stat& b) {
  return id(a) == id(b) && a.st_mode == b.st_mode;
}
bool same_file(const struct stat& a, const struct stat& b) {
#ifdef __APPLE__
  auto am = a.st_mtimespec, bm = b.st_mtimespec, ac = a.st_ctimespec, bc = b.st_ctimespec;
#else
  auto am = a.st_mtim, bm = b.st_mtim, ac = a.st_ctim, bc = b.st_ctim;
#endif
  return same_directory(a, b) && a.st_size == b.st_size && am.tv_sec == bm.tv_sec && am.tv_nsec == bm.tv_nsec &&
         ac.tv_sec == bc.tv_sec && ac.tv_nsec == bc.tv_nsec;
}
int metadata_flags(bool directory) {
#ifdef __APPLE__
  // O_EVTONLY retains the vnode without granting file data reads.
  return O_CLOEXEC | O_NOFOLLOW | O_NONBLOCK | (directory ? O_RDONLY | O_DIRECTORY : O_EVTONLY);
#else
  // O_PATH descriptors cannot consume file data; directories still support openat.
  return O_PATH | O_CLOEXEC | O_NOFOLLOW | (directory ? O_DIRECTORY : 0);
#endif
}
} // namespace
/// @cond
struct capture_admission::implementation {
  enum class kind { trusted_root, trusted_file, supplied_root, supplied_file, discovered_file, target };
  struct receipt {
    descriptor  fd;
    struct stat metadata;
    std::size_t parent;
    std::string name;
    bool        directory;
  };
  struct selection {
    std::string              slot;
    kind                     type;
    admission_file_role      role = admission_file_role::registry;
    std::size_t              leaf;
    std::vector<std::size_t> chain;
    bool                     absent = false;
    std::string              absent_name;
  };
  std::vector<receipt>                                       receipts;
  std::vector<selection>                                     selections;
  std::map<std::pair<std::size_t, std::string>, std::size_t> discovered;
  std::size_t                                                storage = 0, trusted_files = 0, files = 0;
  mutable bool                                               failed = false;
  void                                                       charge(std::size_t bytes) {
    if (bytes > max_storage - storage)
      exhausted();
    storage += bytes;
  }
  void ready() const {
    if (failed)
      changed();
  }
  void key(std::string_view slot) {
    ready();
    if (slot.size() > 1024)
      exhausted();
    if (slot.empty() || !valid_text(slot))
      throw diagnostic{"internal", "Invalid external profile protocol"};
    if (selections.size() >= max_receipts)
      exhausted();
    for (const auto& s : selections)
      if (s.type != kind::discovered_file && s.slot == slot)
        throw diagnostic{"internal", "Invalid external profile protocol"};
    charge(512 + slot.size());
  }
  std::size_t retain(descriptor fd, struct stat metadata, std::size_t parent, std::string_view name, bool directory) {
    if (directory) {
      for (std::size_t index = 0; index < receipts.size(); ++index) {
        const auto& known = receipts[index];
        if (known.directory && known.parent == parent && known.name == name && same_directory(known.metadata, metadata))
          return index;
      }
    }
    if (receipts.size() == max_receipts)
      exhausted();
    charge(512 + name.size());
    receipts.push_back({std::move(fd), metadata, parent, std::string(name), directory});
    return receipts.size() - 1;
  }
  selection open(std::string_view slot, std::string_view host, kind type, bool directory, bool allow_absent = false) {
    key(slot);
    // Host selectors use the existing native host-text ceiling, not portable module grammar.
    if (host.size() > max_storage)
      exhausted();
    if (host.empty() || host.front() != '/' || !valid_text(host))
      invalid();
    selection  result{std::string(slot), type, admission_file_role::registry, 0, {}, false, {}};
    descriptor start(::open("/", metadata_flags(true)));
    if (start.fd < 0)
      io_error();
    auto root_metadata = status(start.fd);
    result.leaf        = retain(std::move(start), root_metadata, 0, "", true);
    charge(sizeof(std::size_t));
    result.chain.push_back(result.leaf);
    std::size_t at = 1;
    while (at < host.size()) {
      auto end = host.find('/', at);
      if (end == std::string_view::npos)
        end = host.size();
      auto name = host.substr(at, end - at);
      at        = end + 1;
      if (name.empty() || name == ".")
        continue;
      bool last         = end == host.size() || host.find_first_not_of('/', end) == std::string_view::npos;
      bool is_directory = !last || directory;
      if (last && end != host.size() && !directory) // trailing slash denotes a directory only
        invalid();
      if (name == "..")
        is_directory = true;
      // Reserve the temporary component and a potential new receipt before
      // copying the name or opening another descriptor. Directory deduplication
      // can consume less; the cumulative charge never refunds temporary work.
      if (receipts.size() == max_receipts || max_storage - storage < 640 || name.size() > (max_storage - storage - 640) / 2)
        exhausted();
      charge(128 + name.size());
      std::string component(name);
      auto        parent = result.leaf;
      struct stat named{};
      if (::fstatat(receipts[parent].fd.fd, component.c_str(), &named, AT_SYMLINK_NOFOLLOW)) {
        if (errno == ENOENT && last && allow_absent) {
          charge(128 + name.size());
          result.absent      = true;
          result.absent_name = std::move(component);
          return result;
        }
        io_error();
      }
      // A proposed target can itself alias a selected trusted file. Inspect
      // that regular-file identity before applying the directory-type refusal.
      if (last && type == kind::target && S_ISREG(named.st_mode))
        is_directory = false;
      if (S_ISLNK(named.st_mode) || (is_directory ? !S_ISDIR(named.st_mode) : !S_ISREG(named.st_mode)))
        throw diagnostic{"path_denied", is_directory ? "Cannot open confined input" : "Only regular files can be captured"};
      descriptor fd(::openat(receipts[parent].fd.fd, component.c_str(), metadata_flags(is_directory)));
      if (fd.fd < 0)
        io_error();
      auto metadata = status(fd.fd);
      if (!(is_directory ? same_directory(named, metadata) : same_file(named, metadata)))
        changed();
      auto retained = retain(std::move(fd), metadata, parent, name, is_directory);
      if (name == "..") {
        // Retain the physical upward edge for verification, independently of
        // the final ancestry. An exited directory remains selected metadata,
        // but must not count as containing the eventual leaf.
        auto ancestor = result.chain[result.chain.size() > 1 ? result.chain.size() - 2 : 0];
        if (!same_directory(receipts[ancestor].metadata, metadata))
          changed();
        if (result.chain.size() > 1)
          result.chain.pop_back();
        result.leaf = ancestor;
      } else {
        result.leaf = retained;
        charge(sizeof(std::size_t));
        result.chain.push_back(result.leaf);
      }
    }
    if (!directory && receipts[result.leaf].directory)
      invalid();
    return result;
  }
  bool contains(const selection& tree, const selection& value) const {
    auto target = id(receipts[tree.leaf].metadata);
    return std::ranges::any_of(value.chain,
                               [&](auto index) { return receipts[index].directory && id(receipts[index].metadata) == target; });
  }
  void relationships() const {
    const auto& trusted = selections.front();
    for (const auto& s : selections) {
      if (s.type == kind::trusted_file && !contains(trusted, s))
        overlap();
      if (s.type == kind::supplied_root || s.type == kind::target) {
        if (contains(trusted, s) || (!s.absent && contains(s, trusted)))
          overlap();
      }
      if ((s.type == kind::supplied_file || s.type == kind::discovered_file) &&
          (s.role == admission_file_role::observation || s.role == admission_file_role::declaration) && contains(trusted, s))
        overlap();
      if (s.type != kind::trusted_file)
        continue;
      for (const auto& other : selections) {
        if (&s == &other || receipts[other.leaf].directory || other.absent)
          continue;
        if (id(receipts[s.leaf].metadata) == id(receipts[other.leaf].metadata))
          overlap();
      }
    }
  }
  void check() const {
    ready();
    for (std::size_t index = 0; index < receipts.size(); ++index) {
      const auto& r       = receipts[index];
      auto        current = status(r.fd.fd);
      if (!(r.directory ? same_directory(r.metadata, current) : same_file(r.metadata, current)))
        changed();
      if (!r.name.empty()) {
        struct stat named{};
        if (::fstatat(receipts[r.parent].fd.fd, r.name.c_str(), &named, AT_SYMLINK_NOFOLLOW) ||
            !(r.directory ? same_directory(r.metadata, named) : same_file(r.metadata, named)))
          changed();
      }
    }
    for (const auto& s : selections)
      if (s.absent) {
        struct stat current{};
        if (::fstatat(receipts[s.leaf].fd.fd, s.absent_name.c_str(), &current, AT_SYMLINK_NOFOLLOW) == 0 || errno != ENOENT)
          changed();
      }
    relationships();
  }
  void append(selection value) {
    selections.push_back(std::move(value));
    relationships();
    check();
  }
};
/// @endcond
capture_admission::capture_admission(std::shared_ptr<implementation> state) : _impl(std::move(state)) {
}
capture_admission::~capture_admission()                            = default;
capture_admission::capture_admission(capture_admission&&) noexcept = default;
capture_admission& capture_admission::operator=(capture_admission&& other) noexcept {
  if (this != &other)
    _impl = std::move(other._impl);
  return *this;
}
std::expected<capture_admission, diagnostic> capture_admission::create(std::string_view directory) {
  try {
    auto state = std::make_shared<implementation>();
    // Conservative fixed allowance covers the shared control block and state.
    state->charge(sizeof(implementation) + 128);
    state->append(state->open("trusted", directory, implementation::kind::trusted_root, true));
    return capture_admission(std::move(state));
  } catch (const diagnostic& e) {
    return std::unexpected(e);
  } catch (const std::bad_alloc&) {
    return std::unexpected(diagnostic{"resource_limit", "External profile resource limit exceeded"});
  }
}
std::expected<void, diagnostic> capture_admission::select_trusted(std::string_view slot, std::string_view file) {
  try {
    if (!_impl)
      changed();
    _impl->check();
    if (_impl->trusted_files == 65)
      exhausted();
    if (_impl->files == max_files)
      exhausted();
    _impl->append(_impl->open(slot, file, implementation::kind::trusted_file, false));
    ++_impl->files;
    ++_impl->trusted_files;
    return {};
  } catch (const diagnostic& e) {
    if (_impl)
      _impl->failed = true;
    return std::unexpected(e);
  } catch (const std::bad_alloc&) {
    poison();
    return std::unexpected(diagnostic{"resource_limit", "External profile resource limit exceeded"});
  }
}
std::expected<void, diagnostic> capture_admission::admit_root(std::string_view slot, std::string_view directory) {
  try {
    if (!_impl)
      changed();
    _impl->check();
    _impl->append(_impl->open(slot, directory, implementation::kind::supplied_root, true));
    return {};
  } catch (const diagnostic& e) {
    if (_impl)
      _impl->failed = true;
    return std::unexpected(e);
  } catch (const std::bad_alloc&) {
    poison();
    return std::unexpected(diagnostic{"resource_limit", "External profile resource limit exceeded"});
  }
}
std::expected<void, diagnostic> capture_admission::admit_file(std::string_view slot, std::string_view file,
                                                              admission_file_role role) {
  try {
    if (!_impl)
      changed();
    _impl->check();
    if (role != admission_file_role::registry && role != admission_file_role::ledger &&
        role != admission_file_role::observation && role != admission_file_role::declaration)
      invalid();
    if (_impl->files == max_files)
      exhausted();
    auto value = _impl->open(slot, file, implementation::kind::supplied_file, false);
    value.role = role;
    _impl->append(std::move(value));
    ++_impl->files;
    return {};
  } catch (const diagnostic& e) {
    if (_impl)
      _impl->failed = true;
    return std::unexpected(e);
  } catch (const std::bad_alloc&) {
    poison();
    return std::unexpected(diagnostic{"resource_limit", "External profile resource limit exceeded"});
  }
}
std::expected<void, diagnostic> capture_admission::admit_target(std::string_view slot, std::string_view directory) {
  try {
    if (!_impl)
      changed();
    _impl->check();
    _impl->append(_impl->open(slot, directory, implementation::kind::target, true, true));
    if (!_impl->receipts[_impl->selections.back().leaf].directory)
      throw diagnostic{"path_denied", "Cannot open confined input"};
    return {};
  } catch (const diagnostic& e) {
    if (_impl)
      _impl->failed = true;
    return std::unexpected(e);
  } catch (const std::bad_alloc&) {
    poison();
    return std::unexpected(diagnostic{"resource_limit", "External profile resource limit exceeded"});
  }
}
std::expected<void, diagnostic> capture_admission::verify() const {
  try {
    if (!_impl)
      changed();
    _impl->check();
    return {};
  } catch (const diagnostic&) {
    if (_impl)
      _impl->failed = true;
    return std::unexpected(diagnostic{"capture_changed", "Captured external profile changed"});
  } catch (const std::bad_alloc&) {
    poison();
    return std::unexpected(diagnostic{"resource_limit", "External profile resource limit exceeded"});
  }
}
std::expected<void, diagnostic> capture_admission::verify_file(std::string_view slot, int descriptor) const {
  auto verified = verify();
  if (!verified)
    return verified;
  try {
    for (const auto& s : _impl->selections)
      if (s.type != implementation::kind::discovered_file && s.slot == slot && !s.absent && !_impl->receipts[s.leaf].directory) {
        if (!same_file(_impl->receipts[s.leaf].metadata, status(descriptor)))
          changed();
        return {};
      }
    changed();
  } catch (const diagnostic&) {
    _impl->failed = true;
    return std::unexpected(diagnostic{"capture_changed", "Captured external profile changed"});
  } catch (const std::bad_alloc&) {
    poison();
    return std::unexpected(diagnostic{"resource_limit", "External profile resource limit exceeded"});
  }
}
std::expected<void, diagnostic> capture_admission::verify_root(std::string_view slot, int descriptor) const {
  auto verified = verify();
  if (!verified)
    return verified;
  try {
    for (const auto& s : _impl->selections)
      if (s.slot == slot && s.type == implementation::kind::supplied_root && !s.absent && _impl->receipts[s.leaf].directory) {
        if (!same_directory(_impl->receipts[s.leaf].metadata, status(descriptor)))
          changed();
        return {};
      }
    changed();
  } catch (const diagnostic&) {
    _impl->failed = true;
    return std::unexpected(diagnostic{"capture_changed", "Captured external profile changed"});
  } catch (const std::bad_alloc&) {
    poison();
    return std::unexpected(diagnostic{"resource_limit", "External profile resource limit exceeded"});
  }
}

bool capture_admission::healthy() const noexcept {
  return _impl && !_impl->failed;
}
void capture_admission::poison() const noexcept {
  if (_impl)
    _impl->failed = true;
}
std::expected<retained_file_receipt, diagnostic> capture_admission::retain_file(std::string_view slot) {
  try {
    if (!_impl)
      changed();
    _impl->check();
    for (std::size_t i = 0; i < _impl->selections.size(); ++i) {
      const auto& selected = _impl->selections[i];
      if (selected.slot == slot && selected.type == implementation::kind::supplied_file && !selected.absent) {
        // Every issued handle is charged cumulatively; moves allocate nothing.
        _impl->charge(sizeof(retained_file_receipt) + sizeof(std::optional<retained_file_receipt>));
        return retained_file_receipt(_impl, i);
      }
    }
    changed();
  } catch (const diagnostic& e) {
    poison();
    return std::unexpected(e);
  } catch (const std::bad_alloc&) {
    poison();
    return std::unexpected(diagnostic{"resource_limit", "External profile resource limit exceeded"});
  }
}
retained_file_receipt::retained_file_receipt(std::shared_ptr<capture_admission::implementation> state, std::size_t index)
    : _state(std::move(state)), _index(index) {
}
retained_file_receipt::~retained_file_receipt()                                = default;
retained_file_receipt::retained_file_receipt(retained_file_receipt&&) noexcept = default;
retained_file_receipt& retained_file_receipt::operator=(retained_file_receipt&& other) noexcept {
  if (this != &other) {
    _state = std::move(other._state);
    _index = other._index;
  }
  return *this;
}
void retained_file_receipt::poison() const noexcept {
  if (_state)
    _state->failed = true;
}
bool retained_file_receipt::same_selection(const retained_file_receipt& other) const noexcept {
  return _state && _state == other._state && _index == other._index;
}
std::expected<void, diagnostic> retained_file_receipt::verify(int descriptor) const {
  try {
    if (!_state)
      changed();
    _state->check();
    const auto& selected = _state->selections[_index];
    if (!same_file(_state->receipts[selected.leaf].metadata, status(descriptor)))
      changed();
    return {};
  } catch (...) {
    poison();
    return std::unexpected(diagnostic{"capture_changed", "Captured external profile changed"});
  }
}
std::expected<retained_root_receipt, diagnostic> capture_admission::retain_root(std::string_view slot) {
  try {
    if (!_impl)
      changed();
    _impl->check();
    for (std::size_t i = 0; i < _impl->selections.size(); ++i)
      if (_impl->selections[i].slot == slot && _impl->selections[i].type == implementation::kind::supplied_root) {
        _impl->charge(sizeof(retained_root_receipt) + sizeof(std::optional<retained_root_receipt>));
        return retained_root_receipt(_impl, i);
      }
    changed();
  } catch (const diagnostic& e) {
    poison();
    return std::unexpected(e);
  } catch (const std::bad_alloc&) {
    poison();
    return std::unexpected(diagnostic{"resource_limit", "External profile resource limit exceeded"});
  }
}
retained_root_receipt::retained_root_receipt(std::shared_ptr<capture_admission::implementation> state, std::size_t index)
    : _state(std::move(state)), _index(index) {
}
retained_root_receipt::~retained_root_receipt()                                = default;
retained_root_receipt::retained_root_receipt(retained_root_receipt&&) noexcept = default;
retained_root_receipt& retained_root_receipt::operator=(retained_root_receipt&& other) noexcept {
  if (this != &other) {
    _state = std::move(other._state);
    _index = other._index;
  }
  return *this;
}
void retained_root_receipt::poison() const noexcept {
  if (_state)
    _state->failed = true;
}
std::expected<void, diagnostic> retained_root_receipt::verify(int descriptor) const {
  try {
    if (!_state)
      changed();
    _state->check();
    if (!same_directory(_state->receipts[_state->selections[_index].leaf].metadata, status(descriptor)))
      changed();
    return {};
  } catch (...) {
    poison();
    return std::unexpected(diagnostic{"capture_changed", "Captured external profile changed"});
  }
}
std::expected<retained_file_receipt, diagnostic> retained_root_receipt::discover(std::string_view path) const {
  try {
    if (!_state)
      changed();
    _state->check();
    if (!valid_path(path))
      invalid();
    // Charge issued handles and staging before allocation, including repeated issuance.
    _state->charge(sizeof(retained_file_receipt) + sizeof(std::optional<retained_file_receipt>) + 256 + 2 * path.size());
    auto key = std::pair(_index, std::string(path));
    if (auto known = _state->discovered.find(key); known != _state->discovered.end())
      return retained_file_receipt(_state, known->second);
    // Compare every component prefix so directory spelling aliases refuse too.
    auto equal_folded = [](std::string_view left, std::string_view right) {
      if (left.size() != right.size())
        return false;
      for (std::size_t i = 0; i < left.size(); ++i) {
        auto fold = [](char c) { return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c; };
        if (fold(left[i]) != fold(right[i]))
          return false;
      }
      return true;
    };
    for (const auto& [known, index] : _state->discovered) {
      if (known.first != _index)
        continue;
      std::size_t at = 0;
      for (;;) {
        auto end = path.find('/', at), old_end = known.second.find('/', at);
        auto prefix = path.substr(0, end), old_prefix = std::string_view(known.second).substr(0, old_end);
        if (!equal_folded(prefix, old_prefix))
          break;
        if (prefix != old_prefix)
          invalid();
        if (end == std::string_view::npos || old_end == std::string_view::npos)
          break;
        at = end + 1;
      }
    }
    const auto& root = _state->selections[_index];
    _state->charge(512 + root.chain.size() * sizeof(std::size_t));
    // Private kind gives a separate key namespace; ordinal strings are <= six bytes.
    capture_admission::implementation::selection value{std::to_string(_state->selections.size()),
                                                       capture_admission::implementation::kind::discovered_file,
                                                       admission_file_role::declaration,
                                                       root.leaf,
                                                       root.chain,
                                                       false,
                                                       {}};
    std::size_t                                  begin = 0;
    for (;;) {
      auto end       = path.find('/', begin);
      bool directory = end != std::string_view::npos;
      auto part      = path.substr(begin, directory ? end - begin : path.size() - begin);
      _state->charge(128 + part.size() + sizeof(std::size_t));
      std::string name(part);
      auto        parent = value.leaf;
      struct stat named{};
      if (::fstatat(_state->receipts[parent].fd.fd, name.c_str(), &named, AT_SYMLINK_NOFOLLOW))
        io_error();
      if (directory ? !S_ISDIR(named.st_mode) : !S_ISREG(named.st_mode))
        invalid();
      descriptor fd(::openat(_state->receipts[parent].fd.fd, name.c_str(), metadata_flags(directory)));
      if (fd.fd < 0)
        io_error();
      auto metadata = status(fd.fd);
      if (!(directory ? same_directory(named, metadata) : same_file(named, metadata)))
        changed();
      if (!directory) {
        // Explicitly admitted files keep their immutable selection; never replace it.
        for (std::size_t i = 0; i < _state->selections.size(); ++i) {
          const auto& selected = _state->selections[i];
          const auto& receipt  = _state->receipts[selected.leaf];
          if (selected.type == capture_admission::implementation::kind::supplied_file && receipt.name == name &&
              same_directory(_state->receipts[receipt.parent].metadata, _state->receipts[parent].metadata)) {
            if (!same_file(receipt.metadata, metadata))
              changed();
            _state->discovered.emplace(std::move(key), i);
            return retained_file_receipt(_state, i);
          }
        }
        // Reuse above consumes no file-selection slot. Only a genuinely new
        // discovered selection is subject to the selected-file ceiling.
        if (_state->files == max_files || _state->selections.size() >= max_receipts)
          exhausted();
      }
      value.leaf = _state->retain(std::move(fd), metadata, parent, part, directory);
      value.chain.push_back(value.leaf);
      if (!directory)
        break;
      begin = end + 1;
    }
    auto index = _state->selections.size();
    _state->append(std::move(value));
    ++_state->files;
    _state->discovered.emplace(std::move(key), index);
    return retained_file_receipt(_state, index);
  } catch (const diagnostic& e) {
    poison();
    return std::unexpected(e);
  } catch (const std::bad_alloc&) {
    poison();
    return std::unexpected(diagnostic{"resource_limit", "External profile resource limit exceeded"});
  }
}
} // namespace planar::engine
