/// @file
/// @brief Version-one typed binary encoding; malformed values and resource exhaustion return diagnostics.
export module planar.engine.canonical;
import std;
export import planar.engine.value;
export namespace planar::engine {
/// Fixed native safety bounds, large enough for maximum captured-file inventories.
struct canonical_limits {
  std::size_t depth = 256;       ///< Maximum nested value depth, counting the root as one.
  std::size_t nodes = 4000000;   ///< Maximum visited values, including object keys and repeated shared nodes.
  std::size_t bytes = 268435456; ///< Maximum encoded bytes (256 MiB).
};
/// @brief Encode normalized data with canonical_v1 tags, unsigned-byte key sorting and exact string bytes.
/// @param input Immutable tree; private declaration tags, nulls, malformed nodes and cycles are refused.
/// @param limits Optional tighter bounds; every field is clamped to its native safety maximum.
/// @return Exact binary bytes or schema/resource_limit diagnostic. No shared mutable state is used.
std::expected<std::string, diagnostic> canonical_encode(const value_ptr& input, canonical_limits limits = {});
/// @brief Decode exactly one canonical_v1 value, preserving binary strings and container kinds.
/// @param bytes Exact canonical encoding; trailing bytes, malformed keys and noncanonical integers are refused.
/// @param limits Optional tighter bounds, clamped to the encoding safety maxima before allocation.
/// @return Immutable value or schema/resource_limit diagnostic. No shared mutable state is used.
std::expected<value_ptr, diagnostic> canonical_decode(std::string_view bytes, canonical_limits limits = {});
} // namespace planar::engine
