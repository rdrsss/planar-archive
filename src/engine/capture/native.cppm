/// @file
/// @brief Descriptor-confined discovery and final verification of immutable captured bytes.
export module planar.engine.capture.native;
import std;
export import planar.engine.value;
export import planar.engine.capture.admission;
export namespace planar::engine {
/// Retained file provenance; host paths and metadata never escape this boundary.
struct captured_file {
  std::string root;  ///< Logical root name.
  std::string path;  ///< Portable root-relative path.
  std::string mode;  ///< Normalized 0644 or 0755.
  std::string bytes; ///< Exact retained bytes.
};
/// Immutable snapshot with sorted unique logical file entries.
using capture_snapshot = std::shared_ptr<const std::vector<captured_file>>;
/// Logical directory in a captured tree; excludes roots themselves and excluded subtrees.
struct captured_directory {
  std::string root; ///< Logical root name.
  std::string path; ///< Portable relative directory path without a trailing slash.
};
/// Immutable sorted unique directory topology, separate from file provenance and identities.
using capture_topology = std::shared_ptr<const std::vector<captured_directory>>;
class acceptance_capture;
/// Discovery session. Not thread-safe; completed snapshots are independently shareable.
class native_capture_session {
public:
  /// Create an empty session; no ambient roots are discovered.
  native_capture_session();
  /// Close held root descriptors.
  ~native_capture_session();
  native_capture_session(const native_capture_session&)            = delete;
  native_capture_session& operator=(const native_capture_session&) = delete;
  /// @brief Add a driver-selected root, refusing links and overlapping physical trees.
  /// @param logical Logical provenance name.
  /// @param directory Absolute host directory.
  /// @return Success or typed capture refusal.
  std::expected<void, diagnostic> add_root(std::string logical, std::string directory);
  /// @brief Open and bind an original admitted supplied root before overlap checks/publication.
  /// @param logical Logical provenance name.
  /// @param directory Absolute physical spelling; parent edges are walked without lexical cancellation.
  /// @param admission Original metadata capability, never replaced or exposed.
  /// @param slot Original supplied-root receipt key.
  /// @return Success or typed refusal; failure publishes no root and closes temporary descriptors.
  std::expected<void, diagnostic> add_root(std::string logical, std::string directory, const capture_admission& admission,
                                           std::string_view slot);
  /// @brief Bind retained original root discovery authority before publication.
  /// @param logical Logical provenance name.
  /// @param directory Physical absolute spelling of the admitted root.
  /// @param receipt Original supplied-root capability; every read and tree uses it.
  /// @return Success or terminal refusal, with no unbound fallback.
  std::expected<void, diagnostic> add_root(std::string logical, std::string directory, retained_root_receipt receipt);
  /// @brief Register a standalone driver input, captured logically as input.json.
  /// @param logical Logical input provenance root, distinct from registered roots.
  /// @param file Absolute canonical host file; its directory may contain other roots.
  /// @return Success or typed refusal; no tree traversal is exposed for this input.
  std::expected<void, diagnostic> add_input(std::string logical, std::string file);
  /// @brief Bind one admitted standalone file without content reads or parent-tree authority.
  /// @param logical Logical input name, exposing only input.json.
  /// @param file Absolute physical spelling; parent components are opened without lexical cancellation.
  /// @param receipt Original supplied-file authority, retained until sealing or transferred to its first read.
  /// @return Success or terminal refusal; metadata verification precedes publication.
  std::expected<void, diagnostic> add_input(std::string logical, std::string file, retained_file_receipt receipt);
  /// @brief Retain one required file during discovery; repeated reads reuse retained bytes.
  /// @param root Registered logical root.
  /// @param path Portable relative file path.
  /// @param text Require non-NUL UTF-8 and the 1 MiB text-input limit.
  /// @return Retained entry or typed capture refusal.
  std::expected<captured_file, diagnostic> read(std::string_view root, std::string_view path, bool text = true);
  /// @brief Read only after the opened descriptor matches its original admission receipt.
  /// @param root Registered logical capture root.
  /// @param path Portable relative file path, or input.json for a standalone input.
  /// @param admission Retained thread-confined metadata capability.
  /// @param slot Original admitted file key.
  /// @param text Require bounded UTF-8 text.
  /// @return Retained entry or typed refusal before content access on a mismatched identity.
  std::expected<captured_file, diagnostic> read(std::string_view root, std::string_view path, const capture_admission& admission,
                                                std::string_view slot, bool text = true);
  /// @brief Bind and read with additional per-file and remaining aggregate byte ceilings.
  /// @param root Registered logical root.
  /// @param path Portable relative path or input.json for a standalone input.
  /// @param admission Original metadata capability, checked before any allocation or read.
  /// @param slot Original admitted file key.
  /// @param byte_ceiling Maximum bytes for this read, in addition to inherited limits.
  /// @param remaining_bytes Remaining caller-owned aggregate content budget.
  /// @param text Require non-NUL UTF-8 text under the inherited text limit.
  /// @return Retained bytes or typed refusal; bounds use the opened descriptor's size.
  std::expected<captured_file, diagnostic> read_bounded(std::string_view root, std::string_view path,
                                                        const capture_admission& admission, std::string_view slot,
                                                        std::size_t byte_ceiling, std::size_t remaining_bytes, bool text = true);
  /// @brief Capture with owned original supplied-file authority, retained across every reread.
  /// @param root Registered logical root.
  /// @param path Portable file path or input.json.
  /// @param receipt Move-only original receipt; existing entries require the same selection.
  /// @param text Require bounded UTF-8 text.
  /// @return Original retained bytes or terminal refusal; unbound caches cannot be upgraded.
  std::expected<captured_file, diagnostic> read(std::string_view root, std::string_view path, retained_file_receipt receipt,
                                                bool text = true);
  /// @brief Retain original authority with additional caller content ceilings.
  /// @param root Registered logical root.
  /// @param path Portable file path or input.json.
  /// @param receipt Original supplied-file authority, never replaced on cache reuse.
  /// @param byte_ceiling Maximum file content bytes.
  /// @param remaining_bytes Remaining caller content budget.
  /// @param text Require bounded UTF-8 text.
  /// @return Retained bytes or terminal refusal before content on a changed receipt.
  std::expected<captured_file, diagnostic> read_bounded(std::string_view root, std::string_view path,
                                                        retained_file_receipt receipt, std::size_t byte_ceiling,
                                                        std::size_t remaining_bytes, bool text = true);
  /// @brief Capture all regular files under a root, excluding declared boundaries and .git directories.
  /// @param root Registered logical root.
  /// @param exclusions Portable file or directory boundaries supplied by trusted validation.
  /// @return Success or typed capture refusal.
  std::expected<void, diagnostic> tree(std::string_view root, std::vector<std::string> exclusions = {});
  /// @brief Verify retained roots, selected file ancestors and files, plus registered tree membership, then seal.
  /// @return Sorted immutable retained bytes, or capture_changed on any detected change.
  std::expected<capture_snapshot, diagnostic> finish();
  /// @brief Seal and transfer receipts and roots to an acceptance-only capability.
  /// @return Owned immutable capture and native verifier, or the initial sealing diagnostic.
  std::expected<acceptance_capture, diagnostic> finish_for_acceptance();
  /// @brief Retrieve sealed directory topology without filesystem access.
  /// @return Sorted immutable directories, or internal unless finish succeeded.
  std::expected<capture_topology, diagnostic> directories() const;
  /// @brief Attach a factory-owned cumulative budget charged by reads and enumeration.
  /// @param budget Budget to charge, or null for none.
  void set_budget(std::shared_ptr<budget_sink> budget);
  /// @brief Set a deterministic verification seam, called immediately before the final reread.
  /// @param callback Test/driver instrumentation; exceptions are not swallowed.
  void before_verification(std::function<void()> callback);

private:
  friend class acceptance_capture;
  struct implementation;
  std::unique_ptr<implementation> _impl;
};
/// Move-only sealed capture and retained native receipts. Not thread-safe.
/// Snapshot/topology values are immutable and independently shareable. Verification
/// compares the original receipts, including selected file ancestor identities,
/// without publishing new values or invoking Lua. Ancestors do not add public topology.
class acceptance_capture {
public:
  /// Close retained root descriptors.
  ~acceptance_capture();
  /// @brief Transfer ownership; the source thereafter refuses all operations.
  /// @param other Capability whose retained roots and immutable values are transferred.
  acceptance_capture(acceptance_capture&& other) noexcept;
  /// @brief Release current roots and transfer ownership; self-move preserves the value.
  /// @param other Capability whose retained roots and immutable values are transferred.
  /// @return This capability after the ownership transfer.
  acceptance_capture& operator=(acceptance_capture&& other) noexcept;
  acceptance_capture(const acceptance_capture&)            = delete;
  acceptance_capture& operator=(const acceptance_capture&) = delete;
  /// @brief Retrieve the precise original sealed bytes without filesystem access.
  /// @return Immutable snapshot, or capture_changed for a moved-from capability.
  std::expected<capture_snapshot, diagnostic> snapshot() const;
  /// @brief Retrieve the precise original sealed logical directories.
  /// @return Immutable topology, or capture_changed for a moved-from capability.
  std::expected<capture_topology, diagnostic> directories() const;
  /// @brief Repeat native receipt comparison immediately before each commit attempt.
  /// @return Success or capture_changed; never replaces sealed bytes or topology.
  /// Cooperative changes are detected; writes after this check remain possible.
  std::expected<void, diagnostic> verify() const;

private:
  friend class native_capture_session;
  struct implementation;
  explicit acceptance_capture(std::unique_ptr<implementation> state);
  std::unique_ptr<implementation> _impl;
};
} // namespace planar::engine
