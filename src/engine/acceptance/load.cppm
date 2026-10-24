/// @file
/// @brief Bounded declaration-only load capture and pure preparation, with terminal ownership and typed failures.
export module planar.engine.acceptance.load;
import std;
export import planar.engine.acceptance.prepared;
export namespace planar::engine::acceptance {
/// Native load boundary categories; trusted semantic diagnostics stay with the caller.
enum class load_error_code {
  invalid_state,
  invalid_path,
  invalid_route,
  invalid_interpretation,
  capture,
  preparation,
  resource_limit
};
/// Owned redacted boundary refusal, preserving existing lower-layer diagnostic vocabularies.
struct load_error {
  load_error_code                  code;             ///< Load boundary category.
  diagnostic                       detail      = {}; ///< Original native capture failure, if applicable.
  std::optional<preparation_error> preparation = {}; ///< Original preparation refusal, if applicable.
};
/// Move-only declaration capture owner. Not thread-safe; returned structural values are immutable.
/// Every operation after opening is one-shot. Any failure terminates the owner,
/// releases native roots and makes later calls invalid_state. Moves transfer the
/// entire state; moved-from and consumed owners refuse. Self-move preserves state.
/// Trusted callers must route and interpret the returned values in their retained
/// VM, supplying that VM's exact provenance. Native checks certify structural
/// correspondence and complete captured membership, not semantic execution.
class load_capture {
public:
  /// Release retained native roots and structural values.
  ~load_capture();
  /// @brief Transfer the complete owner, leaving the source unusable.
  /// @param other Owner whose state is transferred.
  load_capture(load_capture&& other) noexcept;
  /// @brief Release current state and transfer ownership; self-move preserves it.
  /// @param other Owner whose state is transferred.
  /// @return This owner.
  load_capture& operator=(load_capture&& other) noexcept;
  load_capture(const load_capture&)            = delete;
  load_capture& operator=(const load_capture&) = delete;
  /// @brief Open and load only the workspace declaration and its text references.
  /// @param workspace Absolute lexically normalized workspace declaration intent path.
  /// @param registry Absolute lexically normalized registry intent path, retained without opening until capture.
  /// @return New owner or bounded typed refusal. No cwd, environment or name lookup occurs.
  static std::expected<load_capture, load_error> open_workspace(std::string_view workspace, std::string_view registry);
  /// @brief Read the original opened workspace route input, once, for trusted routing.
  /// @return Exact {file,workspace} value or invalid_state; no filesystem access.
  std::expected<value_ptr, load_error> workspace();
  /// @brief Check the fixed trusted route before capturing all routed declarations and explicit registry.
  /// @param route Exact successful route_workspace value for the opened workspace.
  /// @return Complete captured bundle for trusted interpretation, with sealed full workspace closure.
  std::expected<value_ptr, load_error> capture(const value_ptr& route);
  /// @brief Consume the capture into pure immutable preparation without executing Lua or reading files.
  /// @param bundle Complete trusted normalized B from interpret_bundle.
  /// @param semantic Complete trusted semantic M from interpret_bundle.
  /// @param stdlib_files Exact trusted-native original/synthetic provenance from the retained VM; reverified and copied.
  /// @return Prepared owner with every sealed workspace file, or terminal typed refusal.
  std::expected<prepared_acceptance, load_error> prepare(const value_ptr& bundle, const value_ptr& semantic,
                                                         std::span<const history::tree_file> stdlib_files);

private:
  struct implementation;
  explicit load_capture(std::unique_ptr<implementation> state);
  std::unique_ptr<implementation> _impl;
};
} // namespace planar::engine::acceptance
