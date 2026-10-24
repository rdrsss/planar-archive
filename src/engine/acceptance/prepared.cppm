/// @file
/// @brief Native-only immutable acceptance preparation and explicit request intent, with redacted boundary failures.
export module planar.engine.acceptance.prepared;
import std;
export import planar.engine.capture.native;
export import planar.engine.history.provenance_codec;
export import planar.engine.history.interpretation_codec;
export import planar.engine.history.request_v2;
export namespace planar::engine::acceptance {
/// Fixed preparation failure categories; no caller bytes or host locations are retained.
enum class preparation_error_code {
  invalid_request,
  invalid_source,
  invalid_registry,
  invalid_stdlib,
  invalid_interpretation,
  unsupported_version,
  resource_limit,
  moved_from
};
/// Redacted preparation failure, distinct from the native final receipt diagnostic.
struct preparation_error {
  preparation_error_code code; ///< Owning boundary refusal category.
};
/// Owned exact intent fields, exposed only through immutable shared ownership.
struct request_data {
  std::string                        request_id;    ///< Caller-selected idempotency key, excluded from body equality.
  history::request_record            fields;        ///< Exact load parameters and canonical expected-head token.
  std::optional<std::int64_t>        expected_head; ///< Positive signed-64 revision, or initial empty head.
  std::string                        encoded;       ///< Complete selected request codec bytes, without captured content.
  std::optional<history::request_v2> version2{};    ///< Exact selector and codec2 fields; absent for legacy intent.
};
/// Validated intent, independent of filesystem, capture and current ledger state. Immutable and thread-safe.
class request_intent {
public:
  /// @brief Validate explicit already-normalized absolute arguments without consulting cwd or filesystem.
  /// @param request_id One to 128 ASCII identifier bytes.
  /// @param actor One to 256 non-NUL UTF-8 bytes.
  /// @param expected_head Empty or positive signed-64 head.
  /// @param workspace_argument Exact absolute lexically normalized UTF-8 argument.
  /// @param registry_argument Exact absolute lexically normalized UTF-8 argument.
  /// @return Owned immutable load intent or redacted validation/bounds failure.
  static std::expected<request_intent, preparation_error> create(std::string_view request_id, std::string_view actor,
                                                                 std::optional<std::int64_t> expected_head,
                                                                 std::string_view            workspace_argument,
                                                                 std::string_view            registry_argument);
  /// @brief Construct an immutable codec2 intent from normalized explicit selectors without host access.
  /// @param request_id Bounded ASCII idempotency key, excluded from body bytes.
  /// @param actor Bounded nonempty UTF-8 assertion.
  /// @param expected_head Empty or positive signed-64 revision.
  /// @param workspace_argument Normalized absolute workspace selector.
  /// @param registry_argument Normalized absolute registry selector.
  /// @param profile_argument Absent for embedded, otherwise normalized absolute profile selector.
  /// @return Owned exact codec2 intent or redacted validation/bounds failure.
  static std::expected<request_intent, preparation_error> create_v2(std::string_view request_id, std::string_view actor,
                                                                    std::optional<std::int64_t>     expected_head,
                                                                    std::string_view                workspace_argument,
                                                                    std::string_view                registry_argument,
                                                                    std::optional<std::string_view> profile_argument = {});
  /// @brief Retrieve stable immutable fields; copies retain their data independently.
  /// @return Owned fields or moved_from after ownership transfer.
  std::expected<std::shared_ptr<const request_data>, preparation_error> data() const;

private:
  explicit request_intent(std::shared_ptr<const request_data> data);
  std::shared_ptr<const request_data> _data;
};
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
