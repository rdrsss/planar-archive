module;
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
module planar.test.ledger_posix;
import std;
namespace planar::test::ledger_posix {
namespace {
fault                 selected = fault::none;
std::filesystem::path target;
int                   triggered = 0;
int                   syncs     = 0;
bool                  inject(fault wanted) {
  if (selected != wanted || triggered)
    return false;
  ++triggered;
  std::println("POSIX fault={} sync_count={}", static_cast<int>(wanted), syncs);
  errno = EIO;
  return true;
}
} // namespace
void reset(fault next, const std::filesystem::path& path) {
  selected  = next;
  target    = path;
  triggered = 0;
  syncs     = 0;
}
int hits() {
  return triggered;
}
int fsync(int fd) {
  ++syncs;
  if ((syncs == 1 && inject(fault::file_sync)) || (syncs == 3 && inject(fault::parent_sync)))
    return -1;
  if (syncs == 2 && inject(fault::sidecar)) {
    std::ofstream out(target.string() + "-journal", std::ios::binary);
    out << "late-sidecar-sentinel";
    if (!out)
      return -1;
  }
  return ::fsync(fd);
}
int linkat(int from_dir, const char* from, int to_dir, const char* to, int flags) {
  if (inject(fault::link))
    return -1;
  if (inject(fault::target_arrival)) {
    const int fd = ::openat(to_dir, to, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    if (fd < 0)
      return -1;
    const char sentinel[] = "arriving-target-sentinel";
    const auto written    = ::write(fd, sentinel, sizeof(sentinel) - 1);
    ::close(fd);
    if (written != sizeof(sentinel) - 1)
      return -1;
  }
  return ::linkat(from_dir, from, to_dir, to, flags);
}
int unlinkat(int dir, const char* name, int flags) {
  if ((flags == 0 && inject(fault::cleanup_file)) || ((flags & AT_REMOVEDIR) && inject(fault::cleanup_directory)))
    return -1;
  return ::unlinkat(dir, name, flags);
}
} // namespace planar::test::ledger_posix
