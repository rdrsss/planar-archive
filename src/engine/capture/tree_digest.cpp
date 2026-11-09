module;
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <xxhash.h>
module planar.engine.capture.tree_digest;
import std;
import planar.engine.canonical;
namespace planar::engine::capture {
namespace {
struct fd_owner {
  int fd;
  ~fd_owner() {
    if (fd >= 0)
      ::close(fd);
  }
};
auto fail(std::string code, std::string message) {
  return std::unexpected(capture_error{std::move(code), std::move(message)});
}
auto bound(std::string name, std::uint64_t maximum) {
  return std::unexpected(capture_error{"resource_limit", "Execution resource limit exceeded", std::move(name), maximum});
}
bool inside(std::string_view path, std::string_view boundary) {
  return path == boundary || (boundary.ends_with('/') && path.starts_with(boundary));
}
bool same(const struct stat& a, const struct stat& b) {
#if defined(__APPLE__)
  const bool time = a.st_mtimespec.tv_sec == b.st_mtimespec.tv_sec && a.st_mtimespec.tv_nsec == b.st_mtimespec.tv_nsec;
#else
  const bool time = a.st_mtim.tv_sec == b.st_mtim.tv_sec && a.st_mtim.tv_nsec == b.st_mtim.tv_nsec;
#endif
  return a.st_dev == b.st_dev && a.st_ino == b.st_ino && a.st_size == b.st_size && a.st_mode == b.st_mode && time;
}
value_ptr text(std::string s) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = std::move(s);
  return v;
}
struct walker {
  const tree_digest_spec&                           spec;
  tree_digest                                       result;
  std::uint64_t                                     bytes = 0;
  std::set<std::pair<std::uint64_t, std::uint64_t>> inodes;
  std::map<std::string, std::string>                spellings;
  std::expected<void, capture_error>                walk(int parent, std::string prefix, std::uint64_t depth) {
    if (depth > spec.max_depth)
      return bound("check_observation_depth", spec.max_depth);
    struct stat dir_before{};
    if (::fstat(parent, &dir_before))
      return fail("io", "Cannot read captured directory");
    int copy = ::openat(parent, ".", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (copy < 0)
      return fail("io", "Cannot read captured directory");
    DIR* raw = ::fdopendir(copy);
    if (!raw) {
      ::close(copy);
      return fail("io", "Cannot read captured directory");
    }
    std::unique_ptr<DIR, decltype(&::closedir)> dir(raw, ::closedir);
    std::vector<std::string>                    names;
    errno = 0;
    while (auto* e = ::readdir(raw)) {
      std::string n = e->d_name;
      if (n != "." && n != "..")
        names.push_back(std::move(n));
      errno = 0;
    }
    if (errno)
      return fail("io", "Cannot read captured directory");
    std::ranges::sort(names);
    for (const auto& name : names) {
      auto path = prefix + name;
      if (std::ranges::any_of(spec.excluded, [&](const auto& x) { return inside(path, x) || x == path + "/"; }))
        continue;
      struct stat before{};
      if (::fstatat(parent, name.c_str(), &before, AT_SYMLINK_NOFOLLOW))
        return fail("io", "Cannot read captured file");
      if (name == ".git" && S_ISDIR(before.st_mode))
        continue;
      if (!valid_path(path))
        return fail("path_denied", "Invalid captured path");
      auto lower = path;
      for (char& c : lower)
        if (c >= 'A' && c <= 'Z')
          c = static_cast<char>(c + 32);
      auto [it, added] = spellings.emplace(lower, path);
      if (!added && it->second != path)
        return fail("path_denied", "Portable retained path alias");
      if (S_ISLNK(before.st_mode) || (!S_ISREG(before.st_mode) && !S_ISDIR(before.st_mode)))
        return fail("path_denied", "Unsupported captured file type");
      fd_owner fd{
          ::openat(parent, name.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC | (S_ISDIR(before.st_mode) ? O_DIRECTORY : 0))};
      if (fd.fd < 0)
        return fail("io", "Cannot read captured file");
      struct stat opened{};
      if (::fstat(fd.fd, &opened) || !same(before, opened))
        return fail("capture_changed", "File changed during read");
      if (S_ISDIR(before.st_mode)) {
        auto nested = walk(fd.fd, path + "/", depth + 1);
        if (!nested)
          return nested;
        continue;
      }
      if (!inodes.emplace(before.st_dev, before.st_ino).second)
        return fail("path_denied", "Captured hardlink alias");
      if (result.entries.size() >= spec.max_files)
        return bound("check_observation_files", spec.max_files);
      if (before.st_size < 0 || static_cast<std::uint64_t>(before.st_size) > spec.max_hashed_bytes - bytes)
        return bound("check_observation_bytes", spec.max_hashed_bytes);
      const auto size = static_cast<std::uint64_t>(before.st_size);
      bytes += size;
      bool retain = std::ranges::any_of(spec.retain_paths, [&](const auto& p) { return inside(path, p); });
      if (retain && size > spec.max_retained_file_bytes)
        return bound("facet_file_bytes", spec.max_retained_file_bytes);
      auto envelope   = std::make_shared<value>();
      envelope->kind  = value_kind::list;
      envelope->items = {text("planar"), text("canonical_v1"), text("file_bytes_v1"), text("")};
      auto header     = canonical_encode(envelope);
      if (!header)
        return std::unexpected(header.error());
      // The final canonical string length is known from the opened descriptor; stream its bytes without retaining them.
      for (int i = 0; i < 8; ++i)
        (*header)[header->size() - 8 + i] = static_cast<char>((size >> (56 - 8 * i)) & 255);
      std::unique_ptr<XXH3_state_t, decltype(&XXH3_freeState)> hash(XXH3_createState(), XXH3_freeState);
      if (!hash)
        return bound("check_observation_bytes", spec.max_hashed_bytes);
      XXH3_128bits_reset(hash.get());
      XXH3_128bits_update(hash.get(), header->data(), header->size());
      std::string content;
      if (retain)
        content.reserve(size);
      std::array<char, 65536> buffer;
      std::uint64_t           read_bytes = 0;
      while (true) {
        auto n = ::read(fd.fd, buffer.data(), buffer.size());
        if (n < 0 && errno == EINTR)
          continue;
        if (n < 0)
          return fail("io", "Cannot read captured file");
        if (n == 0)
          break;
        read_bytes += static_cast<std::uint64_t>(n);
        if (read_bytes > size)
          return fail("capture_changed", "File changed during read");
        XXH3_128bits_update(hash.get(), buffer.data(), n);
        if (retain)
          content.append(buffer.data(), n);
      }
      struct stat after{};
      if (::fstat(fd.fd, &after) || !same(before, after) || read_bytes != size)
        return fail("capture_changed", "File changed during read");
      XXH128_canonical_t digest;
      XXH128_canonicalFromHash(&digest, XXH3_128bits_digest(hash.get()));
      tree_digest_entry entry{path, (before.st_mode & 0111) ? "0755" : "0644", size, {}};
      for (int i = 0; i < 16; ++i)
        entry.digest[i] = static_cast<std::byte>(digest.digest[i]);
      result.entries.push_back(std::move(entry));
      if (retain)
        result.retained.emplace(path, std::move(content));
    }
    struct stat dir_after{};
    if (::fstat(parent, &dir_after) || !same(dir_before, dir_after))
      return fail("capture_changed", "Directory changed during read");
    return {};
  }
};
} // namespace
std::expected<tree_digest, capture_error> capture_tree_digest(int dir_fd, const tree_digest_spec& spec) {
  for (const auto& p : spec.excluded)
    if (!valid_path(p, p.ends_with('/')))
      return fail("path_denied", "Invalid captured path");
  walker w{spec, {}, 0, {}, {}};
  auto   r = w.walk(dir_fd, "", 0);
  if (!r)
    return std::unexpected(r.error());
  std::ranges::sort(w.result.entries, {}, &tree_digest_entry::path);
  return std::move(w.result);
}
std::vector<tree_change> diff_trees(const tree_digest& base, const tree_digest& current) {
  std::vector<tree_change> out;
  std::size_t              a = 0, b = 0;
  while (a < base.entries.size() || b < current.entries.size()) {
    if (b == current.entries.size() || (a < base.entries.size() && base.entries[a].path < current.entries[b].path)) {
      out.push_back({base.entries[a++].path, tree_change::kind::deleted});
      continue;
    }
    if (a == base.entries.size() || current.entries[b].path < base.entries[a].path) {
      out.push_back({current.entries[b++].path, tree_change::kind::added});
      continue;
    }
    const auto& x = base.entries[a++];
    const auto& y = current.entries[b++];
    if (x.digest != y.digest || x.size != y.size)
      out.push_back({x.path, tree_change::kind::modified});
    else if (x.mode != y.mode)
      out.push_back({x.path, tree_change::kind::mode});
  }
  return out;
}
std::string digest_text(const std::array<std::byte, 16>& digest) {
  std::string    out = "xxh3_128:";
  constexpr auto hex = "0123456789abcdef";
  for (auto b : digest) {
    auto n = std::to_integer<unsigned>(b);
    out += hex[n >> 4];
    out += hex[n & 15];
  }
  return out;
}
} // namespace planar::engine::capture
