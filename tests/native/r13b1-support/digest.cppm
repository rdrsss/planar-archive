/// @file
/// @brief Verification-only kind-specific forced record digests; no production override exists.
export module planar.test.record_digest;
import std;
import planar.engine.history.record;
export namespace planar::test {
/// @brief Validate using the approved codec then force a deterministic digest for each kind.
/// @param value Exact original record type, retaining the production encoder and validation.
/// @return Constant digest for this kind or the original typed validation refusal.
std::expected<engine::history::record_digest, engine::history::record_error> forced_digest(const engine::history::record& value) {
  auto encoded = engine::history::encode_record(value);
  if (!encoded)
    return std::unexpected(encoded.error());
  engine::history::record_digest result{};
  result[0] = static_cast<std::uint8_t>(engine::history::kind_of(value));
  return result;
}
} // namespace planar::test
