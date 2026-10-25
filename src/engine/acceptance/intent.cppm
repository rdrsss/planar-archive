/// @file
/// @brief Pure explicit load request intent and redacted preparation errors; no filesystem, VM or ledger dependency.
/// Entry points are request_intent::create and create_v2; failures return a redacted preparation_error.
export module planar.engine.acceptance.intent;
import std;
import planar.engine.value;
export import planar.engine.history.record;
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
} // namespace planar::engine::acceptance
