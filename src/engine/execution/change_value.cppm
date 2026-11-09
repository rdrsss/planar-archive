/// @file
/// @brief Closed-shape decoding of trusted Lua's `execution_change_v1` value into `ledger::execution_change`, with the
/// request framing built natively from typed options (execution-contract P2-1, EX-2, EX-3).
///
/// Entry points are `build_request_v3` (the `request_v3` framing of an execution verb with its P2-1 parameter map),
/// `build_framing` (`request_v2` for load, `request_v3` otherwise) and `decode_execution_change`. The decoder refuses any
/// unknown, missing or mistyped member, a `snapshot_seq` that is not the snapshot's `last_seq` (L16), and a load or
/// activate change that writes a first row for a role without asserting its no-row `role_state` member (L20). Every
/// refusal is a handler response mismatch: the caller maps `change_error` to the redacted `internal` code (D151). No
/// framing byte, identifier, job or role of the request ever comes from Lua. Pure functions; no shared state.
export module planar.engine.execution.change_value;

import std;
export import planar.engine.value;
export import planar.engine.ledger.execution;

export namespace planar::engine::execution {

/// @brief Typed options of a format3 load: the natively built `request_v2` framing and the candidate revision.
struct load_options {
  history::request_v2                                request;  ///< Load framing built by the load prelude.
  ledger::execution_revision_input                   revision; ///< Revision identifiers, or the noop flag and active ids.
  std::optional<ledger::execution_prepared_revision> prepared; ///< G2 records to intern for an accepted load.
};

/// @brief Typed options of `activate` (P2-1 option map).
struct activate_options {
  std::string job;             ///< `--job`.
  std::string expect_revision; ///< `--expect-revision`: canonical decimal or `none`.
  std::string expect_job;      ///< `--expect-job`: `NONE` or a job identifier.
};

/// @brief Typed options of `claim`.
struct claim_options {
  std::string                job;             ///< `--job`.
  std::string                role;            ///< `--role`.
  std::optional<std::string> provenance_path; ///< Normalized absolute private `--provenance` path, when supplied.
};

/// @brief Typed options of `takeover`.
struct takeover_options {
  std::string                job;             ///< `--job`.
  std::string                role;            ///< `--role`.
  std::uint64_t              run_id = 0;      ///< `--run-id`.
  std::optional<std::string> provenance_path; ///< Normalized absolute private `--provenance` path, when supplied.
};

/// @brief Typed recovery mutation input; the token remains native and is never framed.
struct recovery_options {
  std::string                operation;         ///< refresh, checkpoint or release.
  std::string                job;               ///< Authorized run job identifier.
  std::string                role;              ///< Authorized run role identifier.
  std::string                run_token;         ///< Ephemeral run authorization.
  std::string                expect_fp;         ///< Refresh acknowledged work digest.
  std::string                reason;            ///< Release reason.
  std::optional<std::string> observations_path; ///< Native checkpoint observation file slot.
  std::optional<std::string> provenance_path;   ///< Native driver provenance file slot.
};
/// @brief Options of one execution operation; the alternative selects the operation.
using operation_options = std::variant<load_options, activate_options, claim_options, takeover_options, recovery_options>;

/// @brief Everything native knows about a request before Lua builds its change.
struct change_request {
  std::string                request_id;       ///< `--request-id`.
  std::string                actor;            ///< `--actor`.
  std::optional<std::string> profile_argument; ///< `request_v3` selector 01 argument; absent for the embedded selector.
  operation_options          options;          ///< Typed operation options.
  value_ptr                  provenance;       ///< Normalized `read_provenance` document (claim, takeover); else null.
};

/// @brief A handler response mismatch: the dotted member of the change value that failed (D151, redacted `internal`).
struct change_error {
  std::string field; ///< Dotted member path such as `change.expected.3.kind`; never shown to a driver.
};

/// @brief Name of the operation an options alternative selects.
/// @param options Typed options.
/// @return `load`, `activate`, `claim` or `takeover`.
std::string_view operation_name(const operation_options& options);

/// @brief Build the `request_v3` framing of a non-load execution verb with its exact P2-1 parameter map.
/// @param actor Request actor.
/// @param options Activate, claim or takeover options.
/// @param profile_argument Selector 01 argument, or absent for selector 00.
/// @return Framing whose `parameters` are the canonical_v1 bytes of the parameter map, or a mismatch for load options.
std::expected<history::request_v3, change_error> build_request_v3(std::string_view actor, const operation_options& options,
                                                                  const std::optional<std::string>& profile_argument);

/// @brief Build the framing of a request: the load prelude's `request_v2`, or a native `request_v3`.
/// @param request Native request description.
/// @return Framing, or a mismatch when the load framing names another actor or operation.
std::expected<ledger::execution_framing, change_error> build_framing(const change_request& request);

/// @brief Decode a closed-shape `execution_change_v1` value into a complete ledger change.
/// @param change Value returned by trusted Lua (`reconcile_execution` or a claim handler).
/// @param snapshot Snapshot the change was built from; `last_seq` must equal `snapshot_seq` (L16).
/// @param request Native request description; it supplies the framing, request id, actor, job, role and previous run.
/// @return The ledger change, or the first failing member.
std::expected<ledger::execution_change, change_error>
decode_execution_change(const value_ptr& change, const ledger::execution_snapshot& snapshot, const change_request& request);

/// @brief Decode a verdict change against its native document and request framing.
/// @param change Trusted Lua verdict change value.
/// @param snapshot Snapshot used by the policy evaluation.
/// @param request_id Idempotency key.
/// @param request Native framed request with private normalized verdict path.
/// @param run Target reviewer run identifier authorized by the caller.
/// @param document Native normalized input document.
/// @return Complete verdict ledger change, or a structural mismatch.
std::expected<ledger::execution_change, change_error>
decode_verdict_change(const value_ptr& change, const ledger::execution_snapshot& snapshot, std::string_view request_id,
                      const history::request_v3& request, std::uint64_t run, const value_ptr& document);

} // namespace planar::engine::execution
