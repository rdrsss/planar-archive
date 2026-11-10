/// @file
/// @brief Runtime-independent CLI option state shared by embedded and closed-profile execution handlers.
///
/// Each invocation owns these plain values through CLI11 parsing and dispatch. No VM, native service, filesystem access
/// or mutable shared state is retained here. Command families translate the values into their owning runtime intents.
export module planar.cmd.execution_options;
import std;
export namespace planar::cmd::execution_options {
/// Invocation-owned activate CLI state, retained unchanged through parsing and dispatch.
struct activate_options {
  std::string ledger;          ///< Explicit ledger path.
  std::string job;             ///< Job ID to activate.
  std::string expect_revision; ///< Canonical positive decimal or none.
  std::string expect_job;      ///< Uppercase NONE or the expected active job ID.
  std::string request_id;      ///< ASCII idempotency key.
  std::string actor;           ///< UTF-8 driver assertion.
};
/// Invocation-owned status CLI state, retained unchanged through parsing and dispatch.
struct status_options {
  std::string                ledger;     ///< Explicit ledger path; never inferred from home configuration.
  std::optional<std::string> role;       ///< Selected role identifier.
  std::optional<std::string> request_id; ///< Caller-provided idempotency key.
  std::optional<std::string> workspace;  ///< Optional declaration workspace for a draft report.
  std::optional<std::string> registry;   ///< Trusted registry file paired with the workspace.
};
/// Invocation-owned preview CLI state, retained unchanged through parsing and dispatch.
struct preview_options {
  std::string                ledger; ///< Explicit ledger path; never inferred from home configuration.
  std::string                role;   ///< Selected role identifier.
  std::optional<std::string> since;  ///< Optional retained render digest for packet comparison.
};
/// Invocation-owned claim CLI state, retained unchanged through parsing and dispatch.
struct claim_options {
  std::string                ledger;     ///< Explicit normalized ledger path.
  std::string                job;        ///< Selected job identifier.
  std::string                role;       ///< Selected role identifier.
  std::string                request_id; ///< Driver supplied idempotency key.
  std::string                actor;      ///< Recorded actor text.
  std::optional<std::string> run_id;     ///< Decimal run identifier for takeover or recovery.
  std::optional<std::string> provenance; ///< Optional provenance document path.
};
/// Invocation-owned takeover CLI state, retained unchanged through parsing and dispatch.
struct takeover_options {
  std::string                ledger;     ///< Explicit normalized ledger path.
  std::string                job;        ///< Selected job identifier.
  std::string                role;       ///< Selected role identifier.
  std::string                request_id; ///< Driver supplied idempotency key.
  std::string                actor;      ///< Recorded actor text.
  std::optional<std::string> run_id;     ///< Decimal run identifier for takeover or recovery.
  std::optional<std::string> provenance; ///< Optional provenance document path.
};
/// Invocation-owned refresh CLI state, retained unchanged through parsing and dispatch.
struct refresh_options {
  std::string                operation;    ///< Selected execution operation.
  std::string                ledger;       ///< Explicit normalized ledger path.
  std::string                run;          ///< Native run token; never exposed to Lua.
  std::string                request_id;   ///< Driver supplied idempotency key.
  std::string                actor;        ///< Recorded actor text.
  std::string                expect_fp;    ///< Expected work fingerprint for refresh.
  std::string                reason;       ///< Release reason: released or interrupted.
  std::optional<std::string> observations; ///< Optional observation document path.
  std::optional<std::string> provenance;   ///< Optional provenance document path.
};
/// Invocation-owned checkpoint CLI state, retained unchanged through parsing and dispatch.
struct checkpoint_options {
  std::string                operation;    ///< Selected execution operation.
  std::string                ledger;       ///< Explicit normalized ledger path.
  std::string                run;          ///< Native run token; never exposed to Lua.
  std::string                request_id;   ///< Driver supplied idempotency key.
  std::string                actor;        ///< Recorded actor text.
  std::string                expect_fp;    ///< Expected work fingerprint for refresh.
  std::string                reason;       ///< Release reason: released or interrupted.
  std::optional<std::string> observations; ///< Optional observation document path.
  std::optional<std::string> provenance;   ///< Optional provenance document path.
};
/// Invocation-owned release CLI state, retained unchanged through parsing and dispatch.
struct release_options {
  std::string                operation;    ///< Selected execution operation.
  std::string                ledger;       ///< Explicit normalized ledger path.
  std::string                run;          ///< Native run token; never exposed to Lua.
  std::string                request_id;   ///< Driver supplied idempotency key.
  std::string                actor;        ///< Recorded actor text.
  std::string                expect_fp;    ///< Expected work fingerprint for refresh.
  std::string                reason;       ///< Release reason: released or interrupted.
  std::optional<std::string> observations; ///< Optional observation document path.
  std::optional<std::string> provenance;   ///< Optional provenance document path.
};
/// Invocation-owned recover CLI state, retained unchanged through parsing and dispatch.
struct recover_options {
  std::string                ledger;    ///< Explicit normalized ledger path.
  std::string                role;      ///< Selected role identifier.
  std::optional<std::string> root;      ///< Current output root path.
  std::optional<std::string> base_root; ///< Baseline output root path.
};
/// Invocation-owned check CLI state, retained unchanged through parsing and dispatch.
struct check_options {
  std::string                ledger;     ///< Explicit ledger path; never inferred from home configuration.
  std::string                run;        ///< Secret run token supplied by the driver.
  std::string                root;       ///< Working checkout directory supplied for this invocation.
  std::string                base_root;  ///< Immutable comparison checkout directory.
  std::string                scratch;    ///< Existing disjoint directory for bounded check execution.
  std::string                request_id; ///< Caller-provided idempotency key.
  std::string                actor;      ///< Driver actor recorded with the event.
  std::optional<std::string> provenance; ///< Optional driver provenance file.
};
/// Invocation-owned check-recover CLI state, retained unchanged through parsing and dispatch.
struct check_recover_options {
  std::string ledger;          ///< Explicit ledger path; never inferred from home configuration.
  std::string run_id;          ///< Canonical positive run identifier; contains no secret.
  std::string reservation;     ///< Canonical positive check reservation identifier.
  std::string confirm_stopped; ///< Explicit true acknowledgement that the prior check process stopped.
  std::string request_id;      ///< Caller-provided idempotency key.
  std::string actor;           ///< Driver actor recorded with the event.
};
/// Invocation-owned verdict CLI state, retained unchanged through parsing and dispatch.
struct verdict_options {
  std::string ledger;     ///< Explicit ledger path; never inferred from home configuration.
  std::string run;        ///< Secret run token supplied by the driver.
  std::string verdict;    ///< Reviewer verdict JSON file path.
  std::string request_id; ///< Caller-provided idempotency key.
  std::string actor;      ///< Driver actor recorded with the event.
};
} // namespace planar::cmd::execution_options
