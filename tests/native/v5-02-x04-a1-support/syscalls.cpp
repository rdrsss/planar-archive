// Test-only dynamic interposition: count data-read attempts for explicitly watched inodes.
#include <atomic>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <unistd.h>
namespace {
std::atomic<std::uint64_t> watched_device{0}, watched_inode{0}, attempts{0};
void                       observe(int fd) {
  struct stat value{};
  if (watched_inode.load() && ::fstat(fd, &value) == 0 && static_cast<std::uint64_t>(value.st_dev) == watched_device.load() &&
      value.st_ino == watched_inode.load())
    ++attempts;
}
} // namespace
extern "C" void planar_admission_watch(std::uint64_t device, std::uint64_t inode) {
  watched_device = device;
  watched_inode  = inode;
  attempts       = 0;
}
extern "C" std::uint64_t planar_admission_read_attempts() {
  return attempts.load();
}
extern "C" ssize_t admission_read(int fd, void* bytes, size_t count) {
  auto saved = errno;
  observe(fd);
  errno = saved;
#ifdef __APPLE__
  return ::read(fd, bytes, count);
#else
  return ::syscall(SYS_read, fd, bytes, count);
#endif
}
extern "C" ssize_t admission_pread(int fd, void* bytes, size_t count, off_t offset) {
  auto saved = errno;
  observe(fd);
  errno = saved;
#ifdef __APPLE__
  return ::pread(fd, bytes, count, offset);
#else
  return ::syscall(SYS_pread64, fd, bytes, count, offset);
#endif
}
extern "C" ssize_t admission_readv(int fd, const struct iovec* vectors, int count) {
  auto saved = errno;
  observe(fd);
  errno = saved;
#ifdef __APPLE__
  return ::readv(fd, vectors, count);
#else
  return ::syscall(SYS_readv, fd, vectors, count);
#endif
}
extern "C" void* admission_mmap(void* address, size_t length, int protection, int flags, int fd, off_t offset) {
  auto saved = errno;
  observe(fd);
  errno = saved;
#ifdef __APPLE__
  return ::mmap(address, length, protection, flags, fd, offset);
#else
  return reinterpret_cast<void*>(::syscall(SYS_mmap, address, length, protection, flags, fd, offset));
#endif
}
#ifdef __APPLE__
#define ADMISSION_INTERPOSE(replacement, original)                                                                               \
  __attribute__((used)) static const struct {                                                                                    \
    const void* replacement;                                                                                                     \
    const void* original;                                                                                                        \
  } interpose_##original __attribute__((section("__DATA,__interpose"))) = {reinterpret_cast<const void*>(&replacement),          \
                                                                           reinterpret_cast<const void*>(&original)};
ADMISSION_INTERPOSE(admission_read, read)
ADMISSION_INTERPOSE(admission_pread, pread)
ADMISSION_INTERPOSE(admission_readv, readv)
ADMISSION_INTERPOSE(admission_mmap, mmap)
#else
extern "C" ssize_t read(int fd, void* bytes, size_t count) {
  return admission_read(fd, bytes, count);
}
extern "C" ssize_t pread(int fd, void* bytes, size_t count, off_t offset) {
  return admission_pread(fd, bytes, count, offset);
}
extern "C" ssize_t readv(int fd, const struct iovec* vectors, int count) {
  return admission_readv(fd, vectors, count);
}
extern "C" void* mmap(void* address, size_t length, int protection, int flags, int fd, off_t offset) {
  return admission_mmap(address, length, protection, flags, fd, offset);
}
#endif
