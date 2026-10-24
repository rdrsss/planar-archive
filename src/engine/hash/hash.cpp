module;
#include <xxhash.h>
module planar.engine.hash;
import std;
namespace planar::engine {
namespace {
value_ptr text(std::string_view bytes) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = bytes;
  return v;
}
std::expected<std::string, diagnostic> allocation_failure() {
  return std::unexpected(diagnostic{"resource_limit", "Hash allocation failed", "canonical_bytes", 268435456});
}
} // namespace
std::expected<std::string, diagnostic> hash_value(std::string_view domain, const value_ptr& input) {
  if (domain.size() > 268435456 - 55)
    return allocation_failure();
  if (!valid_text(domain))
    return std::unexpected(diagnostic{"schema", "Invalid hash domain"});
  try {
    auto envelope   = std::make_shared<value>();
    envelope->kind  = value_kind::list;
    envelope->items = {text("planar"), text("canonical_v1"), text(domain), input};
    auto bytes      = canonical_encode(envelope);
    if (!bytes)
      return std::unexpected(bytes.error());
    const auto         hash = XXH3_128bits(bytes->data(), bytes->size());
    XXH128_canonical_t canonical;
    XXH128_canonicalFromHash(&canonical, hash);
    std::string                result = "xxh3_128:";
    constexpr std::string_view hex    = "0123456789abcdef";
    for (unsigned char b : canonical.digest) {
      result += hex[b >> 4];
      result += hex[b & 15];
    }
    return result;
  } catch (const std::bad_alloc&) {
    return allocation_failure();
  } catch (const std::length_error&) {
    return allocation_failure();
  }
}
std::expected<std::string, diagnostic> file_digest(std::string_view bytes) {
  if (bytes.size() > 268435456 - 76)
    return allocation_failure();
  try {
    return hash_value("file_bytes_v1", text(bytes));
  } catch (const std::bad_alloc&) {
    return allocation_failure();
  } catch (const std::length_error&) {
    return allocation_failure();
  }
}
std::expected<std::string, diagnostic> fragment_digest(std::string_view bytes) {
  if (bytes.size() > 268435456 - 74)
    return allocation_failure();
  if (!valid_text(bytes))
    return std::unexpected(diagnostic{"schema", "Invalid fragment text"});
  try {
    return hash_value("fragment_v1", text(bytes));
  } catch (const std::bad_alloc&) {
    return allocation_failure();
  } catch (const std::length_error&) {
    return allocation_failure();
  }
}
} // namespace planar::engine
