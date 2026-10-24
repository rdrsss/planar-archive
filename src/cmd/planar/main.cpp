/// @file
/// @brief Thin render process entry; writes buffered results and detects closed or failed streams.
#include <cerrno>
#include <csignal>
#include <unistd.h>
import std;
import planar.cmd.bootstrap;

namespace {
bool write_all(int descriptor, std::string_view bytes) {
  while (!bytes.empty()) {
    const auto count = ::write(descriptor, bytes.data(), bytes.size());
    if (count < 0 && errno == EINTR)
      continue;
    if (count <= 0 || static_cast<std::size_t>(count) != bytes.size())
      return false;
    bytes.remove_prefix(static_cast<std::size_t>(count));
  }
  return true;
}
} // namespace
/// Write the complete command response using unbuffered, checked process writes.
/// @param argc Number of arguments including executable name.
/// @param argv Argument vector supplied by the host.
/// @return Command status, or 4 when writing output fails.
int main(int argc, char** argv) {
  struct sigaction action{};
  action.sa_handler = SIG_IGN;
  sigemptyset(&action.sa_mask);
  if (::sigaction(SIGPIPE, &action, nullptr) != 0)
    return 4;
  std::vector<std::string_view> arguments;
  for (int index = 1; index < argc; ++index)
    arguments.emplace_back(argv[index]);
  const auto result = planar::cmd::invoke(arguments);
  if (!write_all(STDOUT_FILENO, result.output)) {
    if (!result.stdout_error.empty())
      write_all(STDERR_FILENO, result.stdout_error);
    else
      write_all(STDERR_FILENO, planar::cmd::stdout_failure().error);
    return 4;
  }
  return write_all(STDERR_FILENO, result.error) ? result.status : 4;
}
