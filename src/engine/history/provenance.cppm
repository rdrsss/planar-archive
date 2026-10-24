/// @file provenance.cppm
/// @brief Embedded stdlib preparation and compatibility re-exports above the pure provenance codec.
export module planar.engine.history.provenance;
import std;
export import planar.engine.history.provenance_codec;
export import planar.engine.history.tree;
export namespace planar::engine::history {
/// @brief Prepare product embedded sources without runtime filesystem access.
/// @return The verified generated ordered originals, separator and chunk.
std::expected<prepared_provenance, provenance_error> prepare_embedded_stdlib();
} // namespace planar::engine::history
