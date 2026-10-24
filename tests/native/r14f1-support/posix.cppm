/// @file posix.cppm
/// @brief Test-only serial export syscall faults and process termination, with explicit reset.
export module planar.test.export_posix;
import std;
export namespace planar::test::export_posix {
/// Selected scratch-only boundary.
enum class fault {
  none,
  mkdir_stage,
  mkdir_child,
  open_stage,
  open_file,
  write,
  zero_write,
  short_write,
  eintr,
  chmod_file,
  file_sync,
  directory_sync,
  rename,
  unsupported,
  parent_sync,
  cleanup_file,
  cleanup_directory,
  replace_file,
  replace_directory,
  replace_root,
  unowned,
  close_file,
  close_root_after,
  close_parent_after,
  allocation,
  arrival,
  crash_before,
  crash_after
};
/// @brief Reset process-local fault state.
/// @param selected Next boundary.
/// @param target Explicit scratch target.
void reset(fault selected, const std::filesystem::path& target);
/// @brief Count injected boundaries.
/// @return Zero or one.
int hits();
/// @brief Redirect mkdirat.
/// @param fd Parent descriptor.
/// @param name Basename.
/// @param mode Requested mode.
/// @return Syscall result.
int mkdirat(int fd, const char* name, unsigned mode);
/// @brief Redirect openat without creation.
/// @param fd Parent descriptor.
/// @param name Basename.
/// @param flags Open flags.
/// @return Syscall result.
int openat(int fd, const char* name, int flags);
/// @brief Redirect exclusive file creation.
/// @param fd Parent descriptor.
/// @param name Basename.
/// @param flags Open flags.
/// @param mode Requested mode.
/// @return Syscall result.
int openat(int fd, const char* name, int flags, unsigned mode);
/// @brief Redirect exact writes.
/// @param fd File descriptor.
/// @param data Bytes.
/// @param size Byte count.
/// @return Syscall result.
std::ptrdiff_t write(int fd, const void* data, std::size_t size);
/// @brief Redirect fchmod.
/// @param fd Descriptor.
/// @param mode Mode.
/// @return Syscall result.
int fchmod(int fd, unsigned mode);
/// @brief Redirect fsync.
/// @param fd Descriptor.
/// @return Syscall result.
int fsync(int fd);
/// @brief Redirect close, closing the real descriptor even when injecting refusal.
/// @param fd Descriptor.
/// @return Syscall result.
int close(int fd);
/// @brief Redirect ownership-checked cleanup.
/// @param fd Parent.
/// @param name Basename.
/// @param flags Unlink flags.
/// @return Syscall result.
int unlinkat(int fd, const char* name, int flags);
/// @brief Intercept publication immediately before the real platform primitive.
/// @param parent Parent descriptor.
/// @param stage Stage basename.
/// @param target Target basename.
/// @return Injected result or absence to execute the original syscall.
std::optional<int> before_publish(int parent, const char* stage, const char* target);
} // namespace planar::test::export_posix
