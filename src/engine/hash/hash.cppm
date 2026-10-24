/// @file
/// @brief Domain-separated canonical_v1 XXH3-128 identities with typed encoding failures.
export module planar.engine.hash;
import std;
export import planar.engine.canonical;
export namespace planar::engine {
/// @brief Hash C(["planar", "canonical_v1", domain, input]) using pinned seed-zero XXH3_128bits.
/// @param domain Valid non-NUL UTF-8 domain selected by trusted callers.
/// @param input Normalized data; declaration schema and identity selection remain trusted Lua policy.
/// @return xxh3_128: plus lowercase canonical high64/low64 bytes, or a typed diagnostic.
/// @note Pure and thread-safe. Equal digests never replace available exact-byte comparison.
std::expected<std::string, diagnostic> hash_value(std::string_view domain, const value_ptr& input);
/// @brief Hash exact file bytes with the file_bytes_v1 domain.
/// @param bytes Arbitrary captured bytes, including NUL and non-UTF-8.
/// @return Versioned canonical digest or a typed resource diagnostic.
std::expected<std::string, diagnostic> file_digest(std::string_view bytes);
/// @brief Hash non-NUL UTF-8 Markdown bytes with the fragment_v1 domain.
/// @param bytes Exact fragment bytes, excluding the transport envelope newline.
/// @return Versioned canonical digest or a typed text/resource diagnostic.
std::expected<std::string, diagnostic> fragment_digest(std::string_view bytes);
} // namespace planar::engine
