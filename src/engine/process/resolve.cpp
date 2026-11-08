module;
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
module planar.engine.process.resolve;
import std;
namespace planar::engine::process {
namespace {
// Follows symlinks (DB-5): the target must be a regular file executable by the effective identity.
bool regular_executable(const std::string& path) {
  if (path.find('\0') != std::string::npos)
    return false;
  struct stat info{};
  if (::stat(path.c_str(), &info) != 0 || !S_ISREG(info.st_mode))
    return false;
  return ::faccessat(AT_FDCWD, path.c_str(), X_OK, AT_EACCESS) == 0;
}
} // namespace

std::expected<std::string, resolve_outcome>
resolve_executable(std::string_view command0, const std::vector<std::pair<std::string, std::string>>& environment) {
  if (command0.empty() || command0.find('\0') != std::string_view::npos)
    return std::unexpected(resolve_outcome::missing);
  if (command0.find('/') != std::string_view::npos) {
    std::string path(command0);
    if (path.front() != '/' || !regular_executable(path))
      return std::unexpected(resolve_outcome::missing);
    return path;
  }
  const auto entry = std::ranges::find_if(environment, [](const auto& pair) { return pair.first == "PATH"; });
  if (entry == environment.end())
    return std::unexpected(resolve_outcome::missing);
  for (const auto part : std::views::split(std::string_view(entry->second), ':')) {
    const std::string_view directory(part.begin(), part.end());
    if (directory.empty() || directory.front() != '/')
      continue;
    std::string candidate(directory);
    if (candidate.back() != '/')
      candidate += '/';
    candidate += command0;
    if (regular_executable(candidate))
      return candidate;
  }
  return std::unexpected(resolve_outcome::missing);
}
} // namespace planar::engine::process
