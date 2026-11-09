/// @file
/// @brief Native process runner for check commands (R17-2 rules 2 to 8).
///
/// Entry point: run. It validates a spawn_spec, spawns the absolute executable with posix_spawn as the leader of
/// a new process group, with stdin from /dev/null, stdout and stderr merged into one pipe, the working directory
/// taken from an open directory descriptor and exactly the supplied environment. It reads the merged output with
/// poll and read, retains a prefix, reaps the leader, kills what remains of the group and drains the pipe for a
/// bounded time. Spawn errors that mean "nothing could be executed" (ENOENT, EACCES, ENOEXEC, ENOTDIR) are a
/// recorded termination::not_spawned outcome; spec violations and other system failures are process_error values.
/// Checks run trusted registry commands with bounded time, output, cwd and environment and no OS sandbox;
/// escaping descendants are not tracked (Q15 A). The module holds no state and changes no signal
/// disposition; each call is confined to its calling thread and owns every descriptor it opens.
export module planar.engine.process.runner;
import std;
export namespace planar::engine::process {
/// Failure class of run; the system errno, when one applies, travels in process_error::sys_errno.
enum class process_error_code { spawn_io, pipe_io, wait_io, resource_limit };
/// A run failure: invalid spec, or a system call failure that is not a recorded outcome.
struct process_error {
  process_error_code code;          ///< Process failure category.
  int                sys_errno = 0; ///< Captured system error number; zero when not applicable.
};

/// Everything run needs to start one check process.
struct spawn_spec {
  std::string                                      executable;              ///< Absolute path from resolve_executable.
  std::vector<std::string>                         argv;                    ///< Arguments, argv[0] included.
  std::vector<std::pair<std::string, std::string>> environment;             ///< Exact, sorted by name, unique.
  int                                              cwd_fd           = -1;   ///< Open directory of the bound root.
  std::uint64_t                                    timeout_ms       = 0;    ///< 1..3600000.
  std::uint64_t                                    max_output_bytes = 0;    ///< 1..16777216.
  std::uint64_t                                    retain_bytes     = 0;    ///< At most max_output_bytes.
  std::uint64_t                                    kill_grace_ms    = 2000; ///< Product 2000; tests may lower it.
};

/// How the leader process ended, or that it never started.
enum class termination { exited, signaled, timed_out, output_limit, not_spawned };

/// Recorded outcome of one run.
struct run_result {
  termination   how;                  ///< Leader termination category.
  int           exit_code   = 0;      ///< exited only.
  int           signal      = 0;      ///< signaled, timed_out, output_limit: the signal that ended the leader.
  int           spawn_errno = 0;      ///< not_spawned only.
  std::string   output;               ///< First retained bytes of merged stdout and stderr.
  std::uint64_t output_bytes = 0;     ///< Total bytes read, saturating at max_output_bytes + 1.
  bool          truncated    = false; ///< output.size() < output_bytes.
  std::uint64_t duration_ms  = 0;     ///< Steady clock, spawn to leader reap.
};

/// Run one check process to completion.
/// @param spec Validated before any descriptor is opened; cwd_fd is borrowed and never closed.
/// @return The recorded outcome (including not_spawned), or a process_error for invalid specs (resource_limit for
///         bound violations, spawn_io with EINVAL for structural ones) and system failures.
std::expected<run_result, process_error> run(const spawn_spec& spec);
} // namespace planar::engine::process
