/// @file
/// @brief The `read_execution({})` service value: a verified format3 execution snapshot as `{version:1,ledger,last_seq,
/// snapshot}` with `snapshot` the `execution_snapshot_v1` value of execution-contract P2-1.
///
/// Entry points are `snapshot_value` (whole service result) and `role_structure_value` (one decoded structure). The
/// value carries the L18 spellings of R15-B: every identifier, ordinal and counter is a canonical decimal string,
/// `run.generation` is the generation ordinal, and a dispatch names its own recorded revision. Run secrets and tokens
/// never appear, because `ledger::execution_snapshot` holds none. Failures are `snapshot_value_error`, raised only for a
/// structure the canonical codec refuses, which a verified snapshot never contains. Pure functions; no shared state.
export module planar.engine.execution.snapshot_value;

import std;
export import planar.engine.value;
export import planar.engine.ledger.execution;

export namespace planar::engine::execution {

/// @brief Failure of a snapshot value build: the dotted field of the value that could not be encoded.
struct snapshot_value_error {
  std::string field; ///< Dotted path of the failing member, for diagnostics only.
};

/// @brief Decode a role structure into its `role_structure_v1` canonical value.
/// @param structure Decoded role structure of a generation.
/// @return The value whose canonical bytes are the `role_structure_v1` codec bytes, or the failing field.
std::expected<value_ptr, snapshot_value_error> role_structure_value(const history::role_structure_v1& structure);

/// @brief Build the `read_execution({})` result for a verified snapshot.
/// @param snapshot Snapshot read by `ledger::read_execution_snapshot` in one read transaction.
/// @return `{version:1,ledger,last_seq,snapshot}`; `snapshot` has exactly `active_revision`, `active_job`, `activation`,
/// `revision`, `roles` and `publications` (`[]` in R15), or the failing field.
std::expected<value_ptr, snapshot_value_error> snapshot_value(const ledger::execution_snapshot& snapshot);

} // namespace planar::engine::execution
