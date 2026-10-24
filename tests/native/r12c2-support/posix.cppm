/// @file
/// @brief Test-only deterministic publication syscall faults; serial, process-local state with explicit reset.
export module planar.test.ledger_posix;
import std;
export namespace planar::test::ledger_posix {
/// @brief Single publication boundary selected by a test.
enum class fault { none, file_sync, link, parent_sync, cleanup_file, cleanup_directory, sidecar, target_arrival };
/// @brief Reset counters and choose one scratch-only publication fault.
/// @param selected Boundary to exercise.
/// @param target Explicit test-owned target pathname.
void reset(fault selected, const std::filesystem::path& target);
/// @brief Return the number of triggered deterministic faults.
/// @return Trigger count, expected to equal one.
int hits();
/// @brief Redirect a format module fsync call, preserving all others.
/// @param fd Original descriptor.
/// @return Native syscall result or injected EIO.
int fsync(int fd);
/// @brief Redirect no-replace publication, preserving original arguments.
/// @param from_dir Source directory descriptor.
/// @param from Source basename.
/// @param to_dir Target directory descriptor.
/// @param to Target basename.
/// @param flags Original link flags.
/// @return Native syscall result or injected EIO.
int linkat(int from_dir, const char* from, int to_dir, const char* to, int flags);
/// @brief Redirect staging cleanup, preserving original arguments.
/// @param dir Directory descriptor.
/// @param name Entry basename.
/// @param flags Original unlink flags.
/// @return Native syscall result or injected EIO.
int unlinkat(int dir, const char* name, int flags);
} // namespace planar::test::ledger_posix
