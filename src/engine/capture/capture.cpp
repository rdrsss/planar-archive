module;
#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
module planar.engine.capture;
import std;
import planar.engine.hash;
namespace planar::engine {
capture_session::capture_session(std::shared_ptr<budget_sink> budget) : _budget(budget) {
  _native.set_budget(std::move(budget));
}
capture_session::~capture_session() = default;
std::expected<void, diagnostic> capture_session::add_root(std::string logical, std::string directory) {
  return _native.add_root(std::move(logical), std::move(directory));
}
std::expected<void, diagnostic> capture_session::add_root(std::string logical, std::string directory,
                                                          retained_root_receipt receipt) {
  return _native.add_root(std::move(logical), std::move(directory), std::move(receipt));
}
std::expected<void, diagnostic> capture_session::add_input(std::string logical, std::string file) {
  return _native.add_input(std::move(logical), std::move(file));
}
std::expected<void, diagnostic> capture_session::add_input(std::string logical, std::string file, retained_file_receipt receipt) {
  return _native.add_input(std::move(logical), std::move(file), std::move(receipt));
}
std::expected<captured_file, diagnostic> capture_session::read(std::string_view root, std::string_view path,
                                                               retained_file_receipt receipt, bool text) {
  return _native.read(root, path, std::move(receipt), text);
}
std::expected<captured_file, diagnostic> capture_session::read(std::string_view root, std::string_view path, bool text) {
  return _native.read(root, path, text);
}
std::expected<void, diagnostic> capture_session::tree(std::string_view root, std::vector<std::string> exclusions) {
  return _native.tree(root, std::move(exclusions));
}
std::expected<capture_snapshot, diagnostic> capture_session::finish() {
  return _native.finish();
}
std::expected<acceptance_capture, diagnostic> capture_session::finish_for_acceptance() {
  return _native.finish_for_acceptance();
}
std::expected<capture_topology, diagnostic> capture_session::directories() const {
  return _native.directories();
}
void capture_session::before_verification(std::function<void()> callback) {
  _native.before_verification(std::move(callback));
}
std::expected<declaration, diagnostic> capture_session::load(std::string_view path) {
  auto input = read("workspace", path, true);
  if (!input)
    return std::unexpected(input.error());
  auto result = load_declaration(input->bytes, _budget);
  if (!result)
    return std::unexpected(result.error());
  auto slash  = path.rfind('/');
  auto parent = slash == std::string_view::npos ? std::string{} : std::string(path.substr(0, slash + 1));
  for (const auto& reference : result->references) {
    if (!valid_path(reference))
      return std::unexpected(diagnostic{"path_denied", "Invalid text reference path"});
    auto captured = read("workspace", parent + reference, true);
    if (!captured)
      return std::unexpected(captured.error());
  }
  return result;
}
namespace {
/// Owns one descriptor for the size gate and closes it on every path.
struct gate_descriptor {
  int fd;
  ~gate_descriptor() {
    if (fd >= 0)
      ::close(fd);
  }
};
/// The gate proved the path openable, regular and bounded, so a later add_input or read failure means the file or its
/// ancestry changed (D-C01-25). A detail-less resource_limit is a native bad_alloc and keeps the diagnostic bound.
diagnostic post_gate_failure(diagnostic error, std::size_t maximum) {
  if (error.code == "resource_limit" && error.limit.empty())
    return capture_limit("diagnostic_bytes", maximum);
  return diagnostic{"capture_changed", "File changed during read"};
}
/// Finish keeps its existing capture_changed remap; a detail-less bad_alloc keeps the diagnostic bound; other refusals pass.
diagnostic seal_failure(diagnostic error, std::size_t maximum) {
  if (error.code == "capture_changed")
    return diagnostic{"capture_changed", "File changed during read"};
  if (error.code == "resource_limit" && error.limit.empty())
    return capture_limit("diagnostic_bytes", maximum);
  return error;
}
/// Maps both file_digest allocation failures (Hash and Canonical allocation failed) to the diagnostic bound; other digest
/// errors pass through unchanged.
diagnostic digest_failure(diagnostic error, std::size_t maximum) {
  if (error.code == "resource_limit" &&
      (error.message == "Hash allocation failed" || error.message == "Canonical allocation failed"))
    return capture_limit("diagnostic_bytes", maximum);
  return error;
}
} // namespace
diagnostic capture_limit(std::string_view limit, std::size_t maximum) {
  return diagnostic{"resource_limit", "Capture limit exceeded", std::string(limit), maximum};
}
std::expected<captured_diagnostic, diagnostic> capture_diagnostic(capture_session& session, std::string_view file,
                                                                  std::size_t maximum) {
  try {
    // Size gate from the no-follow descriptor before any allocation. O_NONBLOCK keeps a FIFO open from blocking.
    const std::string path(file);
    {
      gate_descriptor gate{::open(path.c_str(), O_RDONLY | O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC)};
      if (gate.fd < 0) {
        if (errno == ELOOP)
          return std::unexpected(diagnostic{"path_denied", "Explicit input file is a symbolic link"});
        return std::unexpected(diagnostic{"io", "Cannot resolve explicit input path"});
      }
      struct stat status{};
      if (::fstat(gate.fd, &status))
        return std::unexpected(diagnostic{"io", "Cannot resolve explicit input path"});
      if (!S_ISREG(status.st_mode))
        return std::unexpected(diagnostic{"io", "Explicit input must be a regular file"});
      if (status.st_size < 0 || static_cast<std::uint64_t>(status.st_size) > maximum)
        return std::unexpected(capture_limit("diagnostic_bytes", maximum));
    }
    if (auto added = session.add_input("diagnostic", path); !added)
      return std::unexpected(post_gate_failure(std::move(added.error()), maximum));
    auto read = session.read("diagnostic", "input.json", false);
    if (!read)
      return std::unexpected(post_gate_failure(std::move(read.error()), maximum));
    // The bytes read are bounded again: a file that grew after the gate never reaches the caller.
    if (read->bytes.size() > maximum)
      return std::unexpected(capture_limit("diagnostic_bytes", maximum));
    auto sealed = session.finish();
    if (!sealed)
      return std::unexpected(seal_failure(std::move(sealed.error()), maximum));
    auto digest = file_digest(read->bytes);
    if (!digest)
      return std::unexpected(digest_failure(std::move(digest.error()), maximum));
    const auto size = static_cast<std::uint64_t>(read->bytes.size());
    return captured_diagnostic{std::move(read->bytes), size, std::move(*digest)};
  } catch (const std::bad_alloc&) {
    return std::unexpected(capture_limit("diagnostic_bytes", maximum));
  }
}
} // namespace planar::engine
