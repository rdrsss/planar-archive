/// @file
/// @brief Read-only checkout observations for recovery, bounded by render capture rules and accepted role ownership.
export module planar.engine.execution.recovery_capture;
import std;
export import planar.engine.value;
export import planar.engine.ledger.execution;
export namespace planar::engine::execution {
/// Capture current and base roots without retaining records or acknowledging work.
/// @param snapshot Verified accepted interpretation and role structure.
/// @param role Role of the active job whose owned or target units are observed.
/// @param root_mapping Driver mapping logical_root=directory.
/// @param base_mapping Driver mapping logical_root=directory for the immutable baseline.
/// @return Captured unit identities, or a typed render capture refusal. Thread-confined; no shared state.
std::expected<value_ptr, diagnostic> capture_recovery_outputs(const ledger::execution_snapshot& snapshot, std::string_view role,
                                                              std::string_view root_mapping, std::string_view base_mapping);
} // namespace planar::engine::execution
