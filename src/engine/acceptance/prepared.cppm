/// @file
/// @brief Native-only immutable acceptance preparation, with redacted boundary failures; re-exports request intent from planar.engine.acceptance.intent.
export module planar.engine.acceptance.prepared;
import std;
export import planar.engine.capture.native;
export import planar.engine.history.provenance_codec;
export import planar.engine.history.interpretation_codec;
export import planar.engine.history.request_v2;
export import planar.engine.acceptance.intent;
export namespace planar::engine::acceptance {
/// Owned exact prepared values, exposed through immutable shared ownership and independent of input lifetimes.
struct acceptance_data {
  std::vector<history::tree_file>  source;         ///< Complete selected workspace files, sorted by portable path.
  history::record_digest           source_digest;  ///< Pure source tree identity.
  history::prepared_provenance     registry;       ///< Exactly captured registry/input.json.
  history::prepared_provenance     stdlib;         ///< Reverified external originals, assembly and manifest.
  history::recorded_interpretation interpretation; ///< Freshly decoded exact bytes and immutable values.
  std::string                      workspace_id;   ///< Workspace identity from the validated fixed interpretation structure.
};
/// Opaque move-only prepared input retaining the precise native capture. Not thread-safe; data is independently shareable.
class prepared_acceptance {
public:
  /// Release the retained native roots and owned prepared data.
  ~prepared_acceptance();
  /// @brief Transfer ownership; subsequent operations on the source refuse.
  /// @param other Prepared input whose data and receipts are transferred.
  prepared_acceptance(prepared_acceptance&& other) noexcept;
  /// @brief Release old roots and transfer ownership; self-move preserves the value.
  /// @param other Prepared input whose data and receipts are transferred.
  /// @return This owner after transfer.
  prepared_acceptance& operator=(prepared_acceptance&& other) noexcept;
  prepared_acceptance(const prepared_acceptance&)            = delete;
  prepared_acceptance& operator=(const prepared_acceptance&) = delete;
  /// @brief Prepare solely from sealed snapshot and explicit externally prepared immutable values; no file reads occur.
  /// @param capture Consumed native capability; preparation does not verify current filesystem receipts.
  /// @param source_paths Nonempty unique portable paths selected only in the fixed workspace root.
  /// @param stdlib_files Complete external provenance files, reverified before copying.
  /// @param interpretation Exact canonical interpretation bytes.
  /// @param versions Exact canonical separate versions bytes.
  /// @return Owned prepared input or redacted bounded structural refusal.
  static std::expected<prepared_acceptance, preparation_error> create(acceptance_capture                  capture,
                                                                      std::span<const std::string_view>   source_paths,
                                                                      std::span<const history::tree_file> stdlib_files,
                                                                      std::string_view interpretation, std::string_view versions);
  /// @brief Retrieve immutable owned prepared values without filesystem access.
  /// @return Stable independently retained values, or moved_from.
  std::expected<std::shared_ptr<const acceptance_data>, preparation_error> data() const;
  /// @brief Delegate directly to the retained native receipt check, without changing prepared values.
  /// @return Native success/capture_changed; moved-from owners refuse as capture_changed.
  std::expected<void, diagnostic> verify() const;

private:
  struct implementation;
  explicit prepared_acceptance(std::unique_ptr<implementation> state);
  std::unique_ptr<implementation> _impl;
};
} // namespace planar::engine::acceptance
