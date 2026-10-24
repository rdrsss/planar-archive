/// @file
/// @brief Strict external profile closure capture, staged admission and original receipt verification.
export module planar.engine.capture.profile;
import std;
export import planar.engine.capture.native;
export import planar.engine.capture.profile_codec;
export namespace planar::engine {
/// Exact selected source bytes; modules retain profile order independently of sorted snapshots.
struct profile_closure {
  captured_file              profile; ///< Original bytes/mode, logically named profile.json.
  std::vector<captured_file> modules; ///< Exact original modules in declared list order.
};
/// Immutable closure safe to share independently of its thread-confined native owner.
using profile_snapshot = std::shared_ptr<const profile_closure>;
/// Move-only thread-confined trusted closure and staged supplied-input capability.
/// No Lua or discovery occurs here. Supplied reads forward original private authority. Failures are terminal.
/// Verification compares original admission and content receipts without replacing bytes;
/// unrelated siblings are ignored. Cooperative checks cannot prevent later hostile writes.
class profile_capture {
public:
  /// @brief Capture the entire trusted closure before exposing supplied-input admission.
  /// @param selector Explicit absolute host profile file; Unicode/spaced filenames are allowed.
  /// @return Complete owner or logical refusal, with no partially published closure.
  static std::expected<profile_capture, profile_error> open(std::string_view selector);
  /// Close retained descriptors.
  ~profile_capture();
  /// @brief Transfer all original receipts and bytes; the source thereafter refuses operations.
  /// @param other Owner to transfer.
  profile_capture(profile_capture&& other) noexcept;
  /// @brief Transfer ownership; self-move preserves the owner.
  /// @param other Owner to transfer.
  /// @return This owner.
  profile_capture& operator=(profile_capture&& other) noexcept;
  profile_capture(const profile_capture&)            = delete;
  profile_capture& operator=(const profile_capture&) = delete;
  /// @brief Retrieve immutable original profile bytes and ordered modules without filesystem access.
  /// @return Original closure, or capture_changed for failed/moved owners.
  std::expected<profile_snapshot, profile_error> snapshot() const;
  /// @brief Admit a supplied root after trusted closure capture, without reading content.
  /// @param slot Unique private receipt key.
  /// @param directory Explicit absolute supplied directory.
  /// @return Success or terminal admission refusal.
  std::expected<void, profile_error> admit_root(std::string_view slot, std::string_view directory);
  /// @brief Admit a supplied or later-discovered file before its first content read.
  /// @param slot Unique private receipt key.
  /// @param file Explicit absolute supplied file.
  /// @param role Supplied-file separation policy.
  /// @return Success or terminal admission refusal.
  std::expected<void, profile_error> admit_file(std::string_view slot, std::string_view file, admission_file_role role);
  /// @brief Admit a proposed export target before publication.
  /// @param slot Unique private receipt key.
  /// @param directory Explicit absolute proposed target directory.
  /// @return Success or terminal admission refusal.
  std::expected<void, profile_error> admit_target(std::string_view slot, std::string_view directory);
  /// @brief Bind a later read's opened descriptor to its original staged file receipt.
  /// @param slot Original admitted file key.
  /// @param descriptor Already-open no-follow file descriptor, never read by this method.
  /// @return Success or terminal metadata refusal before the caller consumes content.
  std::expected<void, profile_error> verify_file(std::string_view slot, int descriptor) const;
  /// @brief Compare an opened supplied root with the original private admission.
  /// @param slot Original supplied-root key.
  /// @param descriptor Open directory descriptor; no enumeration or content reads occur.
  /// @return Success or terminal logical metadata refusal.
  std::expected<void, profile_error> verify_root(std::string_view slot, int descriptor) const;
  /// @brief Issue original root authority for high-level declaration capture without a profile dependency.
  /// @param slot Original supplied-root key.
  /// @return Retained original capability or terminal logical refusal.
  std::expected<retained_root_receipt, profile_error> retain_root(std::string_view slot);
  /// @brief Issue the original supplied-file authority without exposing mutable admission.
  /// @param slot Original supplied-file key.
  /// @return Original capability or terminal logical refusal.
  std::expected<retained_file_receipt, profile_error> retain_file(std::string_view slot);
  /// @brief Bind a session root using this owner's original private admission.
  /// @param session Native session receiving the root only after verification and overlap checks.
  /// @param logical Logical provenance name.
  /// @param directory Absolute physical spelling of the admitted directory.
  /// @param slot Original supplied-root receipt key.
  /// @return Success or terminal logical refusal, including native overlap/resource failures.
  std::expected<void, profile_error> bind_root(native_capture_session& session, std::string logical, std::string directory,
                                               std::string_view slot);
  /// @brief Bind one standalone input through the private original supplied-file admission.
  /// @param session Native session receiving only selected input.json authority.
  /// @param logical Logical provenance name.
  /// @param file Absolute physical spelling of the already admitted file.
  /// @param slot Original supplied-file receipt key; no parent root is admitted.
  /// @return Success or terminal logical refusal; binding reads no content or membership.
  std::expected<void, profile_error> bind_input(native_capture_session& session, std::string logical, std::string file,
                                                std::string_view slot);
  /// @brief Read an explicitly admitted supplied file with retained original authority.
  /// @param session Native session already containing the logical root/input.
  /// @param root Logical provenance root.
  /// @param path Portable path or input.json.
  /// @param slot Original supplied-file key.
  /// @param text Require bounded UTF-8 text.
  /// @return Captured original bytes or terminal logical profile refusal.
  std::expected<captured_file, profile_error> read(native_capture_session& session, std::string_view root, std::string_view path,
                                                   std::string_view slot, bool text = true);
  /// @brief Forward a retained read with caller content ceilings through the original admission.
  /// @param session Native receiving session.
  /// @param root Logical provenance root.
  /// @param path Portable file path or input.json.
  /// @param slot Original supplied-file key.
  /// @param byte_ceiling Per-file content ceiling.
  /// @param remaining_bytes Remaining caller aggregate content budget.
  /// @param text Require bounded UTF-8 text.
  /// @return Original bytes or terminal logical refusal.
  std::expected<captured_file, profile_error> read_bounded(native_capture_session& session, std::string_view root,
                                                           std::string_view path, std::string_view slot, std::size_t byte_ceiling,
                                                           std::size_t remaining_bytes, bool text = true);
  /// @brief Repeat original metadata, relationship and content receipt checks at a required seal.
  /// @return Success or terminal capture_changed; sealed original bytes never change.
  std::expected<void, profile_error> verify() const;

private:
  struct implementation;
  explicit profile_capture(std::unique_ptr<implementation> state);
  std::unique_ptr<implementation> _impl;
};
} // namespace planar::engine
