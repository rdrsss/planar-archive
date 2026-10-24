/// @file
/// @brief Pure bounded external-profile JSON decoding with ordered paths and logical typed refusals.
export module planar.engine.capture.profile_codec;
import std;
export import planar.engine.value;
export namespace planar::engine {
/// Logical, host-private refusal for one profile capture or admission operation.
struct profile_error {
  diagnostic  detail; ///< Existing typed native diagnostic or closed external-profile diagnostic.
  std::string field;  ///< Only profile or a one-based profile.modules[N] location.
};
/// @brief Decode strict profile JSON without filesystem access or shared mutable state.
/// @param bytes Complete original JSON, bounded to 65536 bytes before parsing.
/// @return Ordered module paths or a logical schema, version, path or resource refusal.
std::expected<std::vector<std::string>, profile_error> decode_profile(std::string_view bytes);
} // namespace planar::engine
