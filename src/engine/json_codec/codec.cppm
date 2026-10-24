/// @file
/// @brief Glaze structural JSON codec with bounded native conversion and safe diagnostics.
export module planar.engine.json_codec;
import std;
export import planar.engine.value;
export namespace planar::engine {
/// Explicit codec ceilings; callers may tighten the defaults only; container depth includes empty containers.
struct codec_limits {
  std::size_t bytes; ///< Maximum raw input or encoded output bytes.
  std::size_t depth; ///< Maximum nested containers, including empty ones.
  std::size_t nodes; ///< Maximum value visits, including repeated references.
};
/// Safe JSON refusal. Offset is a zero-based byte offset in the supplied input.
struct codec_error {
  diagnostic  detail;     ///< Stable error category and optional resource ceiling.
  std::size_t offset = 0; ///< Input byte offset; zero for encoding failures.
};
/// Logical capacity for one validated encode; no allocator overhead or RSS guarantee.
/// Container slots include their embedded node/string objects, without adding those objects twice.
struct codec_reservation {
  std::size_t visits          = 0; ///< Value visits, counting repeated aliases.
  std::size_t array_entries   = 0; ///< Copied array slots.
  std::size_t object_entries  = 0; ///< Copied key/node pairs.
  std::size_t value_strings   = 0; ///< Copied string values.
  std::size_t key_bytes       = 0; ///< Copied key payload bytes.
  std::size_t value_bytes     = 0; ///< Copied value string payload bytes.
  std::size_t minimum_bytes   = 0; ///< Unescaped JSON size lower bound.
  std::size_t graph_bytes     = 0; ///< Root node plus exact reserved container slots.
  std::size_t string_bytes    = 0; ///< Payload plus pinned library terminator/rounding allowance per key/string, including SSO.
  std::size_t workspace_bytes = 0; ///< Encoder plus 257 result/return-node and container slots; not a stack-size claim.
  std::size_t output_capacity = 0; ///< Fixed writer buffer: six times minimum bytes plus 1024.
  std::size_t output_storage_bytes = 0; ///< Writer buffer including pinned library terminator/rounding allowance.
  std::size_t total_bytes          = 0; ///< Checked sum; moved output storage counted once.
};
/// Synchronous trusted native charge adapter; does not own its context.
struct codec_reserver {
  void* context = nullptr; ///< Native charge owner, alive through the call.
  std::expected<void, diagnostic> (*reserve)(void*, const codec_reservation&) = nullptr; ///< Null means no charge hook.
};
/// @brief Encode with one capacity reservation after validation, before graph/output allocation.
/// @param input Stable ordinary graph for the entire synchronous call, including the callback.
/// @param reserver Trusted native hook; must not mutate input through aliases or reenter with mutable aliases.
/// @param limits Tightened codec ceilings, identical to ordinary encode.
/// @return JSON or typed refusal; callback diagnostics are preserved. Exceptions become safe refusals.
/// Thread-safe without shared state. The hook is not called for invalid input; no detached plan is retained.
std::expected<std::string, codec_error> codec_encode_reserved(const value_ptr& input, codec_reserver reserver,
                                                              codec_limits limits = {16777216, 256, 4000000});
/// Borrowed fixed projection record; strings remain caller-owned for the complete synchronous call.
struct projection_section_view {
  std::string_view id;   ///< Declaration ID: lowercase letter followed by lowercase letters, digits or underscores, 1..64 bytes.
  std::string_view text; ///< UTF-8 without NUL, at most 1 MiB per section.
  bool             mandatory; ///< Fixed native boolean; no coercion is performed.
};
/// Complete logical capacity of the size-only projection writer; no allocator overhead or RSS claim.
struct codec_projection_reservation {
  std::size_t scratch_bytes;   ///< One fixed 6,325,248-byte allocation, destroyed before return.
  std::size_t workspace_bytes; ///< Source-derived sizeof fixed workspace including root, sink, buffer and context; not an exact
                               ///< native stack-frame measure.
  std::size_t total_bytes;     ///< Checked sum of scratch and fixed workspace; no graph or retained output.
};
/// Synchronous nonowning cumulative charge hook; no refunds or resets.
struct codec_projection_reserver {
  void* context = nullptr; ///< Owner alive through the complete call.
  std::expected<void, diagnostic> (*reserve)(void*, const codec_projection_reservation&) = nullptr; ///< Null skips charging.
};
/// Distinguishes native validation, encoded ceiling, charging/allocation and internal writer failures.
enum class codec_projection_failure { invalid_input, output_limit, resource, scratch_defect, internal };
/// Owning codec refusal; future runtime adapters own external-profile protocol mapping and locations.
struct codec_projection_error {
  codec_projection_failure kind;   ///< Size-only boundary classification.
  codec_error              detail; ///< Existing codec vocabulary, or unchanged moved charge diagnostic.
};
/// @brief Count exact Glaze bytes of version1 projection output without copying strings or retaining JSON.
/// @param sections Stable borrowed views: at most 64 unique declaration IDs, text <=1 MiB each and <=16 MiB total.
/// @param reserver Trusted synchronous hook called once after full validation and before scratch allocation.
/// @param maximum_bytes Encoded ceiling; may tighten but never relax 16 MiB.
/// @return Exact size, or typed owning refusal; callback diagnostics move unchanged. Allocation/length
/// exceptions become resource refusals, other exceptions internal. Diagnostic adaptation is guarded;
/// final fallback may allocate, so persistent host heap exhaustion is not recoverable by contract.
/// Views, bytes and context must remain stable throughout the call, including its callback: do not mutate
/// input through aliases. Independent nested/concurrent owners are safe; no detached views/context survive.
/// Reserved section names are semantic policy owned by the later native scanner, not this codec boundary.
/// Sticky encoded overflow does not cancel writing: admitted input bounds continued work to <=100,669,888
/// logical bytes. Fixed scratch and workspace are cumulatively charged once, with no refund/reset.
std::expected<std::size_t, codec_projection_error> codec_projection_count(std::span<const projection_section_view> sections,
                                                                          codec_projection_reserver                reserver = {},
                                                                          std::size_t maximum_bytes = 16777216);
/// Checked immutable output capacity; temporary parser requests are charged separately.
struct codec_decode_reservation {
  std::size_t visits          = 0; ///< Persistent value/control blocks.
  std::size_t list_slots      = 0; ///< Exactly reserved shared-pointer slots.
  std::size_t map_entries     = 0; ///< Complete pinned tree nodes, including inline keys.
  std::size_t node_bytes      = 0; ///< Complete pinned make_shared control blocks.
  std::size_t list_bytes      = 0; ///< Exact list storage requests.
  std::size_t map_bytes       = 0; ///< Exact tree node storage requests.
  std::size_t string_bytes    = 0; ///< Long string/key buffers including capacity and terminator.
  std::size_t workspace_bytes = 0; ///< Actual bounded conversion workspace; logical stack charge.
  std::size_t total_bytes     = 0; ///< Checked sum of the preceding byte components.
};
/// Synchronous, nonowning cumulative decode hooks; no refunds or resets.
struct codec_decode_reserver {
  void* context = nullptr; ///< Owner remains alive until the complete call returns.
  std::expected<void, codec_error> (*allocate)(void*, std::size_t)             = nullptr; ///< Before each parser/checker request.
  std::expected<void, codec_error> (*convert)(void*,
                                              const codec_decode_reservation&) = nullptr; ///< Before persistent allocation.
};
/// @brief Decode with actual parser charges and a checked immutable conversion reservation.
/// @param input Stable captured bytes throughout this synchronous call and its callbacks.
/// @param reserver Trusted hooks; input/context must remain stable, without reentrant input mutation.
/// @param limits Ordinary decode ceilings, which callers may only tighten.
/// @return Ordinary immutable ownership, or a moved supplied refusal (including its offset).
/// Independent nested/concurrent calls are supported without global routing. Callbacks must
/// return/throw through C++; allocation/length exceptions become resource refusals, others internal.
/// Returned values and errors retain no borrowed allocator/context. Final fallback may allocate;
/// this boundary does not promise recovery under persistent host heap exhaustion.
std::expected<value_ptr, codec_error> codec_decode_reserved(std::string_view input, codec_decode_reserver reserver,
                                                            codec_limits limits = {1048576, 128, 100000});
/// @brief Decode JSON into ordinary immutable values, without applying product schemas.
/// @param input Captured UTF-8 JSON bytes; null and non-int32 numbers are refused.
/// @param limits Tightened input bytes, nested containers and total value ceilings.
/// @return Complete value or a safe refusal. Thread-safe and free of shared state.
std::expected<value_ptr, codec_error> codec_decode(std::string_view input, codec_limits limits = {1048576, 128, 100000});
/// @brief Encode ordinary values through Glaze with sorted UTF-8 keys.
/// @param input Acyclic ordinary value tree; integers must fit int32, NUL strings are escaped.
/// @param limits Tightened payload bytes, nested containers and visited value ceilings.
/// @return Complete JSON or a safe refusal. Thread-safe and free of shared state.
std::expected<std::string, codec_error> codec_encode(const value_ptr& input, codec_limits limits = {16777216, 256, 4000000});
} // namespace planar::engine
