/// @file
/// @brief Pure catalog-v2 structural validation and bounded canonical sealing without Lua or host access.
export module planar.engine.history.effective_catalog;
import std;
export import planar.engine.value;
export namespace planar::engine::history {
/// Closed historical structural refusal; adapters map identity/resource/protocol to reference/resource_limit/internal.
enum class catalog_error_code { invalid_catalog, identity_conflict, resource_limit };
/// Redacted refusal retaining only catalog or a one-based catalog list location.
struct catalog_error {
  catalog_error_code code;  ///< Closed category, with no caller bytes.
  std::string        field; ///< Logical catalog location only.
};
/// Sealed owned canonical catalog. No caller-mutable aliases or shared mutable state are retained.
class validated_catalog {
  value_ptr   _value;
  std::string _bytes;
  validated_catalog(value_ptr value, std::string bytes);
  friend std::expected<validated_catalog, catalog_error> validate_catalog(const value_ptr&, std::span<const std::string_view>,
                                                                          std::span<const std::string_view>);
  friend std::expected<validated_catalog, catalog_error> decode_catalog(std::string_view, std::span<const std::string_view>,
                                                                        std::span<const std::string_view>);

public:
  /// @return Owned immutable decoded catalog-v2 tree.
  const value_ptr& data() const;
  /// @return Exact canonical_v1 bytes, bounded to 1 MiB (aggregate effective manifest remains a separate boundary).
  std::string_view bytes() const;
};
/// @brief Admit exactly the current shipped schema, then seal through bounded owned decoding to sever input aliases.
/// @param input Catalog-v2 value; no mutation may occur during this call.
/// @param shipped Ordered original unprefixed embedded paths, defining shipped ordinals.
/// @param external Ordered unprefixed profile module paths, defining external ordinals.
/// @return Sealed structural catalog or logical refusal; no callability or semantic honesty proof is made.
std::expected<validated_catalog, catalog_error>
validate_catalog(const value_ptr& input, std::span<const std::string_view> shipped, std::span<const std::string_view> external);
/// @brief Decode a complete supported historical schema with its epoch-specific pairings, signatures and namespaces.
/// Recognized shipped cuts contain exactly 5, 10, 14, 16 or 17 documented commands; arbitrary subsets refuse.
/// @param bytes Complete canonical_v1 catalog; more than 1 MiB refuses before decoding.
/// @param shipped Ordered original unprefixed embedded paths.
/// @param external Ordered unprefixed profile module paths.
/// @return Sealed owned catalog or logical refusal; no filesystem or runtime lookup occurs.
std::expected<validated_catalog, catalog_error> decode_catalog(std::string_view bytes, std::span<const std::string_view> shipped,
                                                               std::span<const std::string_view> external);
} // namespace planar::engine::history
