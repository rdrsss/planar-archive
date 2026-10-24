// Private dynamic interposition counts actual production data and enumeration operations.
#include <atomic>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <unistd.h>
namespace {
std::atomic<bool> active{false};
std::atomic<unsigned long> reads{0}, enumeration{0};
void data() { if(active) ++reads; }
void listing() { if(active) ++enumeration; }
}
extern "C" void d1_watch(bool enabled) { if(enabled) { reads=0; enumeration=0; } active=enabled; }
extern "C" unsigned long d1_reads() { return reads; }
extern "C" unsigned long d1_enumeration() { return enumeration; }
extern "C" ssize_t d1_read(int fd, void* b, size_t n) { data(); return ::read(fd,b,n); }
extern "C" ssize_t d1_pread(int fd, void* b, size_t n, off_t o) { data(); return ::pread(fd,b,n,o); }
extern "C" ssize_t d1_readv(int fd, const struct iovec* v, int n) { data(); return ::readv(fd,v,n); }
extern "C" void* d1_mmap(void* a,size_t n,int p,int f,int fd,off_t o) { data(); return ::mmap(a,n,p,f,fd,o); }
extern "C" struct dirent* d1_readdir(DIR* d) { listing(); return ::readdir(d); }
extern "C" DIR* d1_fdopendir(int fd) { listing(); return ::fdopendir(fd); }
extern "C" DIR* d1_opendir(const char* p) { listing(); return ::opendir(p); }
#define D1_INTERPOSE(replacement, original) \
__attribute__((used)) static const struct { const void* replacement; const void* original; } interpose_##original \
__attribute__((section("__DATA,__interpose"))) = {reinterpret_cast<const void*>(&replacement), reinterpret_cast<const void*>(&original)};
D1_INTERPOSE(d1_read, read)
D1_INTERPOSE(d1_pread, pread)
D1_INTERPOSE(d1_readv, readv)
D1_INTERPOSE(d1_mmap, mmap)
D1_INTERPOSE(d1_readdir, readdir)
D1_INTERPOSE(d1_fdopendir, fdopendir)
D1_INTERPOSE(d1_opendir, opendir)
#include <fcntl.h>
#include <sys/stat.h>
#include <cstdarg>
#include <cstring>
namespace { void (*opened_hook)(int) = nullptr; std::atomic<unsigned long> metadata{0}; }
extern "C" void d2_open_hook(void (*hook)(int)) { opened_hook = hook; }
extern "C" unsigned long d2_metadata() { return metadata; }
extern "C" void d2_metadata_reset() { metadata = 0; }
extern "C" int d2_openat(int parent, const char* path, int flags, ...) {
  mode_t mode = 0;
  if (flags & O_CREAT) { va_list args; va_start(args, flags); mode = va_arg(args, int); va_end(args); }
  if (active) ++metadata;
  int fd = ::openat(parent, path, flags, mode);
  if (fd >= 0 && opened_hook && std::strcmp(path, "file") == 0 && !(flags & O_EVTONLY)) {
    auto hook = opened_hook; opened_hook = nullptr; hook(fd);
  }
  return fd;
}
extern "C" int d2_fstat(int fd, struct stat* st) { if(active) ++metadata; return ::fstat(fd, st); }
extern "C" int d2_fstatat(int fd, const char* path, struct stat* st, int flags) { if(active) ++metadata; return ::fstatat(fd,path,st,flags); }
D1_INTERPOSE(d2_openat, openat)
D1_INTERPOSE(d2_fstat, fstat)
D1_INTERPOSE(d2_fstatat, fstatat)
