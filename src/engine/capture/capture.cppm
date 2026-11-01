/// @file
/// @brief Descriptor-confined discovery and final verification of immutable captured bytes.
export module planar.engine.capture;
import std;
export import planar.engine.capture.native;
export import planar.engine.lua;
export namespace planar::engine {
/// Discovery session. Not thread-safe; completed snapshots are independently shareable.
class capture_session {
public:
  /// @brief Create an empty session; no ambient roots are discovered.
  /// @param budget Optional factory-owned cumulative budget forwarded to native reads and declaration loading.
  explicit capture_session(std::shared_ptr<budget_sink> budget = {});
  /// Close held root descriptors.
  ~capture_session();
  capture_session(const capture_session&)            = delete;
  capture_session& operator=(const capture_session&) = delete;
  /// @brief Add a driver-selected root, refusing links and overlapping physical trees.
  /// @param logical Logical provenance name.
  /// @param directory Absolute host directory.
  /// @return Success or typed capture refusal.
  std::expected<void, diagnostic> add_root(std::string logical, std::string directory);
  /// @brief Forward original root authority to native discovery for declarations and references.
  /// @param logical Logical provenance root.
  /// @param directory Absolute host directory.
  /// @param receipt Original supplied-root capability; no profile/runtime dependency is added.
  /// @return Success or terminal capture refusal.
  std::expected<void, diagnostic> add_root(std::string logical, std::string directory, retained_root_receipt receipt);
  /// @brief Register a standalone driver input, captured logically as input.json.
  /// @param logical Logical input provenance root, distinct from registered roots.
  /// @param file Absolute canonical host file; its directory may contain other roots.
  /// @return Success or typed refusal; no tree traversal is exposed for this input.
  std::expected<void, diagnostic> add_input(std::string logical, std::string file);
  /// @brief Bind an original standalone supplied-file authority before content access.
  /// @param logical Logical input root.
  /// @param file Original physical host spelling.
  /// @param receipt Original supplied-file capability.
  /// @return Success or terminal refusal.
  std::expected<void, diagnostic> add_input(std::string logical, std::string file, retained_file_receipt receipt);
  /// @brief Read one explicitly selected original file under a bound root.
  /// @param root Logical root.
  /// @param path Relative selected path.
  /// @param receipt Original supplied-file selection.
  /// @param text Require bounded text.
  /// @return Original bytes or terminal refusal.
  std::expected<captured_file, diagnostic> read(std::string_view root, std::string_view path, retained_file_receipt receipt,
                                                bool text = true);
  /// @brief Retain one required file during discovery; repeated reads reuse retained bytes.
  /// @param root Registered logical root.
  /// @param path Portable relative file path.
  /// @param text Require non-NUL UTF-8 and the 1 MiB text-input limit.
  /// @return Retained entry or typed capture refusal.
  std::expected<captured_file, diagnostic> read(std::string_view root, std::string_view path, bool text = true);
  /// @brief Load a retained workspace declaration and join its text references to this session.
  /// @param path Workspace-relative declaration path; references resolve from its directory.
  /// @return Immutable structural declaration, or a typed loading/capture error.
  std::expected<declaration, diagnostic> load(std::string_view path);
  /// @brief Capture all regular files under a root, excluding declared boundaries and .git directories.
  /// @param root Registered logical root.
  /// @param exclusions Portable file or directory boundaries supplied by trusted validation.
  /// @return Success or typed capture refusal.
  std::expected<void, diagnostic> tree(std::string_view root, std::vector<std::string> exclusions = {});
  /// @brief Re-enumerate and reread all discovered inputs once, then seal the session.
  /// @return Sorted immutable retained bytes, or capture_changed on any detected change.
  std::expected<capture_snapshot, diagnostic> finish();
  /// @brief Seal and transfer receipts and roots to an acceptance-only capability.
  /// @return Owned immutable capture and native verifier, or the initial sealing diagnostic.
  std::expected<acceptance_capture, diagnostic> finish_for_acceptance();
  /// @brief Retrieve sealed directory topology without filesystem access.
  /// @return Sorted immutable directories, or internal unless finish succeeded.
  std::expected<capture_topology, diagnostic> directories() const;
  /// @brief Set a deterministic verification seam, called immediately before the final reread.
  /// @param callback Test/driver instrumentation; exceptions are not swallowed.
  void before_verification(std::function<void()> callback);

private:
  native_capture_session       _native;
  std::shared_ptr<budget_sink> _budget;
};
/// @brief Build the shared capture resource refusal for a named bound.
/// @param limit Bound name reported in the details, such as `diagnostic_bytes` or `prompt_bytes`.
/// @param maximum Bound value reported in the details.
/// @return `resource_limit` with the existing message `Capture limit exceeded`; no new message exists.
diagnostic capture_limit(std::string_view limit, std::size_t maximum);
/// Facts of one bounded diagnostic file read through a capture session.
struct captured_diagnostic {
  std::string   bytes;  ///< Exact retained file bytes.
  std::uint64_t size;   ///< Byte count of the retained file.
  std::string   digest; ///< `file_bytes_v1` digest of the retained bytes.
};
/// @brief Read one already path-stage-resolved diagnostic file under the session's no-follow confinement and recheck.
/// @param session Discovery session that registers the file and verifies it at finish.
/// @param file Absolute canonical host file path.
/// @param maximum Largest accepted size in bytes, checked from the descriptor before allocation.
/// @return Bytes, size and digest, or a typed capture refusal with existing messages only.
/// @note A size over `maximum`, the post-read bound and digest or allocation failures (`bad_alloc`, `Hash allocation
/// failed`, `Canonical allocation failed`) are `resource_limit` `diagnostic_bytes`. After the gate passes, `add_input` and
/// `read` failures are `capture_changed` `File changed during read`; a `finish` result of `capture_changed` stays
/// `capture_changed`, a detail-less `finish` `resource_limit` is `diagnostic_bytes`, and any other `finish` or digest
/// result passes through unchanged (unreachable from this function). A symbolic link is `path_denied`; a failed open or a
/// non-regular file is `io`.
/// @note Not thread-safe with respect to `session`.
std::expected<captured_diagnostic, diagnostic> capture_diagnostic(capture_session& session, std::string_view file,
                                                                  std::size_t maximum);
} // namespace planar::engine
