/// @file
/// @brief Metadata-only external trust admission and retained native identity checks.
export module planar.engine.capture.admission;
import std;
export import planar.engine.value;
export namespace planar::engine {
/// Explicit standalone input policy; none authorizes content access.
enum class admission_file_role {
  observation, ///< Must remain outside the trusted tree and distinct from trusted files.
  registry,    ///< Directory colocation is allowed; selected trusted aliases are forbidden.
  ledger,      ///< Native data; selected trusted aliases are forbidden.
  declaration  ///< Discovered declaration; checked before its first content read.
};
class retained_file_receipt;
class retained_root_receipt;
/// Move-only, thread-confined metadata capability. No content reads, discovery,
/// callbacks or Lua. Additions retain original receipts; any failure poisons the
/// capability. Directory timestamps are deliberately excluded: siblings are not
/// selected. Physical parent traversal retains every visited edge while only
/// final ancestry governs containment; intermediate links never cancel away.
/// Cooperative checks cannot detect change-and-revert or prevent a
/// write after verification. File receipts include identity, mode, size and times.
class capture_admission {
public:
  /// @brief Admit an explicit trusted directory and its no-follow ancestors.
  /// @param directory Absolute host directory, never exposed in diagnostics.
  /// @return Capability or typed native refusal.
  static std::expected<capture_admission, diagnostic> create(std::string_view directory);
  /// Release all retained descriptors.
  ~capture_admission();
  /// @brief Transfer receipts; the source thereafter refuses operations.
  /// @param other Capability to transfer.
  capture_admission(capture_admission&& other) noexcept;
  /// @brief Transfer receipts; self-move preserves the capability.
  /// @param other Capability to transfer.
  /// @return This capability.
  capture_admission& operator=(capture_admission&& other) noexcept;
  capture_admission(const capture_admission&)            = delete;
  capture_admission& operator=(const capture_admission&) = delete;
  /// @brief Select a trusted file physically within the trusted root, without reading it.
  /// @param slot Unique private logical receipt key, at most 1024 bytes.
  /// @param file Absolute host selector; profile filenames need not be portable module paths.
  /// @return Success or typed refusal. At most 65 trusted selected files.
  std::expected<void, diagnostic> select_trusted(std::string_view slot, std::string_view file);
  /// @brief Admit a supplied workspace/source/base directory, disjoint from the trusted tree.
  /// @param slot Unique private logical receipt key.
  /// @param directory Explicit absolute supplied directory.
  /// @return Success or overlap/native refusal; supplied roots may overlap each other.
  std::expected<void, diagnostic> admit_root(std::string_view slot, std::string_view directory);
  /// @brief Admit an explicitly supplied or later-discovered standalone regular file.
  /// @param slot Unique private logical receipt key.
  /// @param file Explicit absolute supplied file.
  /// @param role Policy governing tree colocation and trusted aliases.
  /// @return Success or typed refusal before any content access.
  std::expected<void, diagnostic> admit_file(std::string_view slot, std::string_view file, admission_file_role role);
  /// @brief Admit a proposed export tree; an absent final component needs an existing parent.
  /// @param slot Unique private logical receipt key.
  /// @param directory Absolute proposed target directory.
  /// @return Success or refusal; existing and absent targets retain original receipts.
  std::expected<void, diagnostic> admit_target(std::string_view slot, std::string_view directory);
  /// @brief Compare all original relationships, names, ancestors, file metadata and aliases.
  /// @return Success or capture_changed. A failed check permanently poisons this capability.
  std::expected<void, diagnostic> verify() const;
  /// @brief Bind a future read to the original admitted file before consuming bytes.
  /// @param slot Previously selected/admitted file key.
  /// @param descriptor Already-open no-follow descriptor about to be read.
  /// @return Success only for the retained original metadata and current receipts.
  std::expected<void, diagnostic> verify_file(std::string_view slot, int descriptor) const;

  /// @brief Bind an opened supplied directory to its original admission without reading content.
  /// @param slot Original supplied-root key; trusted, file and target keys refuse.
  /// @param descriptor Already-open no-follow directory descriptor.
  /// @return Success only for original identity, mode and current global receipts; failure poisons.
  std::expected<void, diagnostic> verify_root(std::string_view slot, int descriptor) const;

  /// @brief Retain an already admitted supplied file, without copying metadata or selectors.
  /// @param slot Original supplied-file key; all other kinds refuse and poison.
  /// @return Move-only authority sharing the original append-only admission state.
  std::expected<retained_file_receipt, diagnostic> retain_file(std::string_view slot);
  /// @brief Retain original supplied-root authority for confined metadata discovery.
  /// @param slot Original supplied-root receipt key.
  /// @return Move-only original authority, or terminal refusal.
  std::expected<retained_root_receipt, diagnostic> retain_root(std::string_view slot);
  /// @brief Observe terminal state without filesystem access.
  /// @return True only while the original state remains usable.
  bool healthy() const noexcept;
  /// @brief Permanently refuse further use, without allocation or filesystem access.
  void poison() const noexcept;

private:
  friend class retained_file_receipt;
  friend class retained_root_receipt;
  struct implementation;
  explicit capture_admission(std::shared_ptr<implementation> state);
  std::shared_ptr<implementation> _impl;
};
/// Original supplied-file authority. Thread-confined; survives admission wrapper lifetime.
/// No host selector or mutable admission state is exposed. Handles are move-only.
class retained_file_receipt {
public:
  /// Release this handle; other handles retain the same original metadata descriptors.
  ~retained_file_receipt();
  /// @brief Transfer authority; the moved-from handle refuses verification.
  /// @param other Original handle.
  retained_file_receipt(retained_file_receipt&& other) noexcept;
  /// @brief Transfer authority; self-move preserves the handle.
  /// @param other Original handle.
  /// @return This handle.
  retained_file_receipt& operator=(retained_file_receipt&& other) noexcept;
  retained_file_receipt(const retained_file_receipt&)            = delete;
  retained_file_receipt& operator=(const retained_file_receipt&) = delete;
  /// @brief Verify all original relationships and the actual opened file before content access.
  /// @param descriptor Open no-follow regular file descriptor.
  /// @return Success or terminal capture_changed.
  std::expected<void, diagnostic> verify(int descriptor) const;
  /// @brief Compare original state and immutable selection identity without filesystem access.
  /// @param other Authority proposed for a retained cache entry.
  /// @return True only for the same original supplied-file selection.
  bool same_selection(const retained_file_receipt& other) const noexcept;
  /// @brief Permanently poison the shared original state without allocation.
  void poison() const noexcept;

private:
  friend class capture_admission;
  friend class retained_root_receipt;
  retained_file_receipt(std::shared_ptr<capture_admission::implementation> state, std::size_t index);
  std::shared_ptr<capture_admission::implementation> _state;
  std::size_t                                        _index;
};
/// Original supplied-root authority with portable relative metadata discovery.
/// Thread-confined, move-only; retains original admission after wrapper destruction.
class retained_root_receipt {
public:
  /// Release the handle; issued file handles retain the same original state.
  ~retained_root_receipt();
  /// @brief Transfer authority.
  /// @param other Original handle; its moved-from value refuses.
  retained_root_receipt(retained_root_receipt&& other) noexcept;
  /// @brief Transfer authority, preserving self-move.
  /// @param other Original handle.
  /// @return This handle.
  retained_root_receipt& operator=(retained_root_receipt&& other) noexcept;
  retained_root_receipt(const retained_root_receipt&)            = delete;
  retained_root_receipt& operator=(const retained_root_receipt&) = delete;
  /// @brief Verify all original relationships and an actual opened root before use.
  /// @param descriptor Open no-follow directory descriptor.
  /// @return Success or terminal capture_changed.
  std::expected<void, diagnostic> verify(int descriptor) const;
  /// @brief Metadata-admit a portable relative file, reusing its original selection.
  /// @param path Portable root-relative file path; no host selector is accepted.
  /// @return Original owned file authority, without content reads or enumeration.
  std::expected<retained_file_receipt, diagnostic> discover(std::string_view path) const;
  /// @brief Permanently poison shared authority without allocation.
  void poison() const noexcept;

private:
  friend class capture_admission;
  retained_root_receipt(std::shared_ptr<capture_admission::implementation> state, std::size_t index);
  std::shared_ptr<capture_admission::implementation> _state;
  std::size_t                                        _index;
};
} // namespace planar::engine
