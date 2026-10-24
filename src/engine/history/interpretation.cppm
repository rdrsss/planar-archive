/// @file interpretation.cppm
/// @brief Live trusted interpretation preparation above pure recorded codecs and exact embedded provenance.
export module planar.engine.history.interpretation;
import std;
export import planar.engine.history.provenance;
export import planar.engine.history.interpretation_codec;
export namespace planar::engine::history {
/// Pure preparation result ready for later transactional storage; no write lock is acquired here.
struct prepared_interpretation {
  recorded_interpretation recorded; ///< Complete interpreted state and metadata.
  prepared_provenance     stdlib;   ///< Exact verified embedded code used by the fixed trusted export.
};
/// @brief Normalize and validate every job with source topology not_captured in the fixed trusted export.
/// @param captured_bundle Sealed captured bundle accepted by the existing normalization boundary.
/// @return Owned complete state with internally prepared embedded provenance, or typed live refusal.
/// @note Thread-safe; no caller digest, database, filesystem reads or ambient capabilities are accepted.
std::expected<prepared_interpretation, interpretation_error> prepare_interpretation(const value_ptr& captured_bundle);
} // namespace planar::engine::history
