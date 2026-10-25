/// @file
/// @brief Declaration-only source capture owner with original receipt binding, terminal ownership and typed refusals.
export module planar.engine.acceptance.source_capture;
import std;
export import planar.engine.capture;

export namespace planar::engine::acceptance {
/// Source capture boundary categories; load maps them onto its public vocabulary.
enum class source_error_code {
  invalid_state, ///< Consumed, terminal or out-of-order owner, or a moved-from receipt supplied to open_bound.
  invalid_path,  ///< Host path or declaration file name is not an absolute normalized intent path.
  invalid_route, ///< Trusted route does not match the opened declaration.
  capture,       ///< Native discovery, admission or sealing refused; detail carries the diagnostic.
  resource_limit ///< Native resource ceiling refused; detail carries the diagnostic.
};

/// Owned redacted source refusal preserving the native diagnostic when one exists.
struct source_error {
  source_error_code code;        ///< Source boundary category.
  diagnostic        detail = {}; ///< Original native diagnostic, if applicable.
};

/// @brief Adapt a selected-declaration admission refusal to the existing load-family capture diagnostic.
/// Intended for the future known-load factory, which admits the declaration before registration and
/// passes profile_error::detail together with profile_capture::last_admission_refusal_was_absent().
/// Absence is taken only from that internal signal (ENOENT of the selected final component), never from message
/// text: when it is set and the diagnostic is io, "Cannot open confined input", the result is the embedded open's
/// capture diagnostic (io, "Cannot inspect confined input", no field). Permission refusals such as EACCES share
/// that message but are never reported as missing. Platform note: on macOS the metadata open (O_EVTONLY) refuses
/// an unreadable present file at admission; on Linux O_PATH admits it and the content read fails later.
/// Every other diagnostic maps exactly as the source boundary maps native diagnostics.
/// @param admission Raw admission diagnostic for the selected declaration.
/// @param absent Internal absence signal from the failed admission.
/// @return Owned source refusal with the existing capture or resource_limit category.
source_error known_declaration_error(const diagnostic& admission, bool absent);

/// Move-only declaration capture owner. Not thread-safe; returned structural values are immutable.
/// Every operation after opening is one-shot. Any failure terminates the owner and
/// releases native roots. Moves transfer the entire state; moved-from and consumed
/// owners refuse. Self-move preserves state. Bound openings consume the supplied
/// original receipts; any refusal poisons the shared original admission so it cannot
/// be used again. No content is read before the bound declaration is admitted.
class source_capture {
public:
  /// Release retained native roots and structural values.
  ~source_capture();
  /// @brief Transfer the complete owner, leaving the source unusable.
  /// @param other Owner whose state is transferred.
  source_capture(source_capture&& other) noexcept;
  /// @brief Release current state and transfer ownership; self-move preserves it.
  /// @param other Owner whose state is transferred.
  /// @return This owner.
  source_capture& operator=(source_capture&& other) noexcept;
  source_capture(const source_capture&)            = delete;
  source_capture& operator=(const source_capture&) = delete;

  /// @brief Open and load the workspace declaration through discovery with no original authority.
  /// @param workspace Absolute lexically normalized workspace declaration intent path.
  /// @param registry Absolute lexically normalized registry intent path, retained without opening until capture.
  /// @return New owner or typed refusal. No cwd, environment or name lookup occurs.
  static std::expected<source_capture, source_error> open_embedded(std::string_view workspace, std::string_view registry);

  /// @brief Open the workspace declaration through original supplied-root and declaration authority.
  /// Construction first refuses any moved-from receipt as invalid_state, then poisoned, foreign-admission or
  /// wrong-role receipts (declaration slot must carry role declaration, registry slot role registry) as
  /// capture/capture_changed, before any consumption or content read. The root receipt needs no role check:
  /// retain_root only issues supplied-root slots. Every refusal, including after consumption, poisons the
  /// single shared original admission, reached through the registry receipt after consumption.
  /// @param workspace Absolute lexically normalized workspace declaration intent path.
  /// @param registry Absolute lexically normalized registry intent path, bound as standalone input at capture.
  /// @param root Original supplied-root receipt for the declaration's directory; consumed.
  /// @param declaration Original supplied-file receipt for the workspace declaration; consumed.
  /// @param registry_file Original supplied-file receipt for the registry; consumed at capture.
  /// @return New owner or typed refusal.
  static std::expected<source_capture, source_error> open_bound(std::string_view workspace, std::string_view registry,
                                                                retained_root_receipt root, retained_file_receipt declaration,
                                                                retained_file_receipt registry_file);

  /// @brief Read the original opened workspace route input, once, for trusted routing.
  /// @return Exact {file,workspace} value or invalid_state; no filesystem access.
  std::expected<value_ptr, source_error> workspace();

  /// @brief Check the fixed trusted route before capturing all routed declarations and the explicit registry.
  /// @param route Exact successful route_workspace value for the opened workspace.
  /// @return Complete captured bundle for trusted interpretation, with sealed full workspace closure.
  std::expected<value_ptr, source_error> capture(const value_ptr& route);

  /// @brief Observe whether the one-shot capture has sealed the owner.
  /// @return True only after a successful capture that has not been released.
  bool sealed_ready() const noexcept;

  /// @brief Return the portable declaration filename; valid only when sealed_ready() is true.
  /// @return Stable reference owned by this owner.
  const std::string& file() const noexcept;

  /// @brief Return the validated portable specification route; valid only when sealed_ready() is true.
  /// @return Stable reference owned by this owner.
  const std::string& spec_path() const noexcept;

  /// @brief Return sorted unique routed job IDs and portable paths; valid only when sealed_ready() is true.
  /// @return Stable reference owned by this owner.
  const std::map<std::string, std::string>& jobs() const noexcept;

  /// @brief Return exact original specification and job declarations; valid only when sealed_ready() is true.
  /// @return Stable reference owned by this owner.
  const std::map<std::string, declaration>& declarations() const noexcept;

  /// @brief Return the exact original opened workspace declaration; valid only when sealed_ready() is true.
  /// @return Stable reference owned by this owner.
  const declaration& workspace_declaration() const noexcept;

  /// @brief Access the sealed native capture for snapshot reads; valid only when sealed_ready() is true.
  /// @return Mutable optional reference owned by this owner; callers must not move it out.
  std::optional<acceptance_capture>& sealed() noexcept;

  /// @brief Transfer the sealed native capture out of this owner for consumption by preparation.
  /// @return The sealed capture, or an empty optional when none was sealed.
  std::optional<acceptance_capture> release_sealed() noexcept;

  /// @brief Repeat the original native receipt checks at the precommit boundary.
  /// @return Success or a typed capture refusal; a failed check terminates native receipts.
  std::expected<void, source_error> verify() const;

private:
  struct implementation;
  explicit source_capture(std::unique_ptr<implementation> state);
  std::unique_ptr<implementation> _impl;
};
} // namespace planar::engine::acceptance
