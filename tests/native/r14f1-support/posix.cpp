module;
#include <cerrno>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
module planar.test.export_posix;
import std;
namespace planar::test::export_posix {
namespace {
fault selected  = fault::none;
int   triggered = 0, mkdirs = 0, post_closes = 0;

bool publishing = false;
bool inject(fault wanted) {
  if (selected != wanted || triggered)
    return false;
  ++triggered;
  errno = EIO;
  return true;
}
bool is_file(int fd) {
  struct stat st{};
  return ::fstat(fd, &st) == 0 && S_ISREG(st.st_mode);
}
void sentinel(int parent, const char* name) {
  int fd = ::openat(parent, name, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
  if (fd < 0)
    throw std::runtime_error("scratch sentinel");
  if (::write(fd, "unowned", 7) != 7) {
    ::close(fd);
    throw std::runtime_error("scratch sentinel write");
  }
  ::close(fd);
}
} // namespace
void reset(fault f, const std::filesystem::path& target) {
  (void)target;
  selected    = f;
  triggered   = 0;
  mkdirs      = 0;
  post_closes = 0;
  publishing  = false;
}
int hits() {
  return triggered;
}
int mkdirat(int fd, const char* name, unsigned mode) {
  ++mkdirs;
  if ((mkdirs == 1 && inject(fault::mkdir_stage)) || (mkdirs == 2 && inject(fault::mkdir_child)))
    return -1;
  return ::mkdirat(fd, name, static_cast<mode_t>(mode));
}
int openat(int fd, const char* name, int flags) {
  if (std::string_view(name).starts_with(".planar-export-") && inject(fault::open_stage))
    return -1;
  return ::openat(fd, name, flags);
}
int openat(int fd, const char* name, int flags, unsigned mode) {
  if (inject(fault::open_file))
    return -1;
  return ::openat(fd, name, flags, static_cast<mode_t>(mode));
}
std::ptrdiff_t write(int fd, const void* data, std::size_t size) {
  if (inject(fault::write))
    return -1;
  if (inject(fault::allocation))
    throw std::bad_alloc();
  if (inject(fault::zero_write))
    return 0;
  if (inject(fault::eintr)) {
    errno = EINTR;
    return -1;
  }
  if (inject(fault::short_write))
    return ::write(fd, data, std::max<std::size_t>(1, size / 2));
  return ::write(fd, data, size);
}
int fchmod(int fd, unsigned mode) {
  if (is_file(fd) && inject(fault::chmod_file))
    return -1;
  return ::fchmod(fd, static_cast<mode_t>(mode));
}
int fsync(int fd) {
  if (publishing && selected == fault::crash_after)
    ::_exit(77);
  if ((publishing && inject(fault::parent_sync)) ||
      (!publishing && (is_file(fd) ? inject(fault::file_sync) : inject(fault::directory_sync))))
    return -1;
  return ::fsync(fd);
}
int close(int fd) {
  if (publishing)
    ++post_closes;
  bool fail =
      (is_file(fd) && inject(fault::close_file)) || (publishing && ((post_closes == 1 && inject(fault::close_root_after)) ||
                                                                    (post_closes == 2 && inject(fault::close_parent_after))));
  int result = ::close(fd);
  return fail ? -1 : result;
}
int unlinkat(int fd, const char* name, int flags) {
  if ((flags == 0 && inject(fault::cleanup_file)) || (flags == AT_REMOVEDIR && inject(fault::cleanup_directory)))
    return -1;
  return ::unlinkat(fd, name, flags);
}
std::optional<int> before_publish(int parent, const char* stage, const char* target) {
  if (selected == fault::crash_before)
    ::_exit(77);
  if (inject(fault::rename))
    return -1;
  if (inject(fault::unsupported)) {
    errno = ENOTSUP;
    return -1;
  }
  if (selected == fault::cleanup_file || selected == fault::cleanup_directory) {
    errno = EIO;
    return -1;
  }
  if (inject(fault::arrival)) {
    if (::mkdirat(parent, target, 0700) != 0)
      return -1;
  }
  if (inject(fault::replace_root)) {
    if (::renameat(parent, stage, parent, "moved-owned-stage") != 0 || ::mkdirat(parent, stage, 0700) != 0)
      return -1;
    int replacement = ::openat(parent, stage, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    sentinel(replacement, "sentinel");
    ::close(replacement);
    errno = EIO;
    return -1;
  }
  if (selected == fault::replace_file || selected == fault::replace_directory || selected == fault::unowned) {
    int d = ::openat(parent, stage, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (d < 0)
      return -1;
    if (inject(fault::replace_file)) {
      // Rename the owned inode aside before creating the replacement: inode reuse cannot mask this test.
      if (::renameat(d, "a.lua", d, "owned-file") != 0) {
        ::close(d);
        return -1;
      }
      sentinel(d, "a.lua");
    } else if (inject(fault::replace_directory)) {
      if (::renameat(d, "nested", d, "owned-directory") != 0 || ::mkdirat(d, "nested", 0700) != 0) {
        ::close(d);
        return -1;
      }
      int sub = ::openat(d, "nested", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
      sentinel(sub, "sentinel");
      ::close(sub);
    } else if (inject(fault::unowned))
      sentinel(d, "unexpected");
    ::close(d);
    errno = EIO;
    return -1;
  }
  publishing = true;
  return {};
}
} // namespace planar::test::export_posix
