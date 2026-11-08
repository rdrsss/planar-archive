/// @file execution.cppm
/// @brief Format3 execution ledger reader, exact request lookup, run-token authorization and the locked commit protocol.
///
/// Entry points are `read_execution_snapshot` and `verify_execution_ledger` (reader), `lookup_execution_request` (exact
/// replay or conflict), `authorize_run_token` (run token secret and state), and `stage_execution` with `commit_execution`
/// (the EX-3 commit protocol). Every entry point refuses format1 and format2 ledgers with `unsupported_ledger_version`
/// before any other work. Failures use the closed `execution_error_code` vocabulary of `execution_schema`. The module
/// imports no runtime, command or acceptance code; the saved load result is built natively from allocated identifiers.
export module planar.engine.ledger.execution;

import std;
export import planar.engine.ledger.execution_schema;
export import planar.engine.history.request_v2;
export import planar.engine.history.request_v3;
export import planar.engine.history.execution_codec;
import planar.engine.ledger.sqlite; // R18: non-exported review helpers take the open connection.

export namespace planar::engine::ledger {

/// @brief One dispatch of a running role: its packet, manifest, W value and fingerprints as stored.
struct execution_dispatch {
  std::uint64_t id = 0;          ///< dispatches.id.
  std::string   kind;            ///< claim, takeover or refresh.
  std::string   work_fp;         ///< 32 lowercase hex digest of the work_exec_v1 value W.
  std::string   render_fp;       ///< 32 lowercase hex digest of the packet Markdown.
  std::uint64_t packet_id   = 0; ///< Packet file record identifier.
  std::uint64_t manifest_id = 0; ///< Manifest value record identifier.
  std::uint64_t work_id     = 0; ///< W value record identifier.
  value_ptr     work;            ///< Decoded W value.
};

/// @brief Snapshot of one (job, role) row with its current generation and, when running, its active run and dispatch.
struct execution_role {
  std::string                       job;                    ///< Job identifier.
  std::string                       role;                   ///< Role identifier.
  std::uint64_t                     generation_id      = 0; ///< Current generation record identifier.
  std::uint64_t                     generation_ordinal = 0; ///< Current generation ordinal.
  history::role_structure_v1        structure;              ///< Decoded role structure of the current generation.
  std::string                       state;                  ///< One of the six role states.
  bool                              drifted = false;        ///< Drift flag, true only when running.
  std::optional<std::uint64_t>      run_id;                 ///< Active run, present exactly when running.
  std::uint64_t                     fact_version = 0;       ///< role_state.fact_version.
  std::uint64_t                     last_seq     = 0;       ///< role_state.last_seq.
  std::optional<execution_dispatch> dispatch;               ///< Acknowledged dispatch of the active run.
};

/// @brief Recorded revision values of the active revision, decoded from its immutable records.
struct execution_revision {
  std::uint64_t id             = 0;   ///< Revision identifier.
  std::uint64_t source         = 0;   ///< Source tree root record identifier.
  std::uint64_t registry       = 0;   ///< Registry tree root record identifier.
  std::uint64_t stdlib         = 0;   ///< Standard-library tree root record identifier.
  std::uint64_t interpretation = 0;   ///< Interpretation value record identifier.
  std::uint64_t versions       = 0;   ///< Versions value record identifier.
  std::string   source_digest;        ///< 32 lowercase hex digest of the source root record, without prefix.
  std::string   stdlib_digest;        ///< 32 lowercase hex digest of the stdlib root record, without prefix.
  value_ptr     interpretation_value; ///< Decoded canonical_v1 interpretation value.
  value_ptr     versions_value;       ///< Decoded canonical_v1 versions value.
  value_ptr     registry_value;       ///< Registry tree identity {id, digest} of the registry root.
};

/// @brief One unit output of a completion: the work identifier and its output identity text.
struct execution_completion_unit {
  std::string work;   ///< Work identifier.
  std::string output; ///< Output identity text as recorded in the observation value.
};

/// @brief One completion of a role in its current lineage (DRAFT-STUB R17-C: minimal R17-7 snapshot shape).
struct execution_completion {
  std::uint64_t                          id = 0;         ///< completions.id.
  std::string                            job;            ///< Job of the completing run.
  std::string                            role;           ///< Role of the completing run.
  std::uint64_t                          generation = 0; ///< Generation record identifier of the completing run.
  std::uint64_t                          run        = 0; ///< Completing run identifier.
  std::uint64_t                          dispatch   = 0; ///< Acknowledged dispatch identifier.
  std::string                            work_fp;        ///< 32 lowercase hex work fingerprint of that dispatch.
  std::uint64_t                          seq = 0;        ///< Completing event sequence.
  std::vector<execution_completion_unit> units;          ///< Unit outputs sorted by work.
};

/// @brief One stored verdict with its decoded normalized document (R18-T, DR18-8).
struct execution_verdict {
  std::uint64_t       id = 0;        ///< verdicts.id.
  std::string         request_id;    ///< Verdict request identifier.
  std::uint64_t       run = 0;       ///< Reviewer run identifier.
  std::uint64_t       seq = 0;       ///< verdict_recorded event sequence.
  std::string         applicability; ///< current or historical.
  std::string         decision;      ///< approve or reject.
  history::verdict_v1 body;          ///< Decoded normalized verdict_v1 document.
};

/// @brief One blocker row (R18-T); the snapshot lists open blockers only.
struct execution_blocker {
  std::uint64_t id  = 0;     ///< blockers.id.
  std::uint64_t seq = 0;     ///< Opening verdict_recorded event sequence.
  std::string   job;         ///< Job identifier.
  std::string   kind;        ///< max_rounds or unowned_rejection.
  std::uint64_t verdict = 0; ///< Opening verdict identifier.
};

/// @brief Current-lineage repair revision of one worker role of the active job; only revisions above zero are listed.
struct execution_repair {
  std::string   job;          ///< Job identifier.
  std::string   role;         ///< Worker role identifier.
  std::uint64_t revision = 0; ///< Repair revision in the role's current structural lineage.
};

/// @brief The snapshot review value of DR18-8.
struct execution_review {
  std::uint64_t count = 0; ///< Review count of the active job in the current activation (E17 A).
  std::vector<execution_verdict>
      verdicts; ///< Active-job verdicts of the current activation plus the latest reviewer-lineage verdict, by id.
  std::vector<execution_blocker> blockers; ///< Open blockers sorted by id.
  std::vector<execution_repair>  repairs;  ///< Current-lineage repair revisions of the active job, sorted by role.
};

/// @brief Verified execution snapshot read in one read transaction at one last_seq.
struct execution_snapshot {
  std::string                       identity;        ///< Ledger identity, 32 lowercase hex.
  std::uint64_t                     last_seq = 0;    ///< Last committed event sequence at the read.
  std::optional<execution_revision> revision;        ///< Active revision, absent when none.
  std::string                       active_job;      ///< Active job identifier, or the empty string.
  std::optional<std::uint64_t>      activation;      ///< Latest activation identifier, absent when none.
  std::vector<execution_role>       roles;           ///< Every role_state row, sorted by job then role.
  std::vector<execution_completion> completions;     ///< R18 (DRAFT-STUB R17-C): current-lineage completions of the active job.
  execution_review                  review;          ///< R18: the DR18-8 review value.
  std::uint64_t                     state_bytes = 0; ///< Canonical byte length of the D24 snapshot value S.
};

/// @brief Read and verify the whole format3 ledger, then return the consistent execution snapshot.
/// @param path Explicit existing ledger path; never created.
/// @return Snapshot, or the closed failure category (unsupported_ledger_version for format1 and format2).
std::expected<execution_snapshot, execution_error> read_execution_snapshot(const std::filesystem::path& path);

/// @brief Verify every format3 invariant of the ledger without returning state.
/// @param path Explicit existing ledger path; never created.
/// @return Success, or the closed failure category, ledger_corrupt for any violated invariant.
std::expected<void, execution_error> verify_execution_ledger(const std::filesystem::path& path);

/// @brief Measure the D24 canonical byte length of the snapshot value S with the revision's recorded values excluded.
/// @param snapshot Snapshot whose roles, revision reference and active job form S.
/// @return Canonical_v1 byte length of execution_snapshot_v1 S, the same measure the reader and writer enforce.
/// @note Pure function: no I/O and no shared state; R15-C reuses this single definition.
std::uint64_t execution_state_bytes(const execution_snapshot& snapshot);

/// @brief Framing of an execution request: request_v2 for load, request_v3 for every other execution operation.
using execution_framing = std::variant<history::request_v2, history::request_v3>;

/// @brief Result of an exact request lookup: unused, a replay of saved bytes, or a conflict.
enum class request_state { unused, replay, conflict };

/// @brief Exact lookup outcome with the saved result bytes when the request replays.
struct request_lookup {
  request_state state = request_state::unused; ///< Lookup state.
  std::string   result;                        ///< Saved result bytes of a replay; empty otherwise.
};

/// @brief Look up a request id against the stored framing of an earlier request.
/// @param path Existing format3 ledger, read only.
/// @param request_id Request identifier.
/// @param framing Framing of the new request, compared byte for byte with the stored body.
/// @return unused, replay with saved bytes, or conflict (request_conflict is the caller's mapping), or a failure category.
std::expected<request_lookup, execution_error>
lookup_execution_request(const std::filesystem::path& path, std::string_view request_id, const execution_framing& framing);

/// @brief Run token components: run id, generation ordinal and the 16-byte secret.
struct run_token {
  std::uint64_t                run_id             = 0; ///< runs.id.
  std::uint64_t                generation_ordinal = 0; ///< Ordinal of the run's generation.
  std::array<std::uint8_t, 16> secret{};               ///< Secret stored in runs.secret.
};

/// @brief Render a run token as "<run id>.<generation ordinal>.<32 lowercase hex>".
/// @param token Token components.
/// @return Token text.
std::string format_run_token(const run_token& token);

/// @brief Parse a run token text; a text that does not follow the grammar yields no token.
/// @param text Token text.
/// @return Components, or nullopt for any text outside the grammar.
std::optional<run_token> parse_run_token(std::string_view text);

/// @brief Authorize a run token against the current run state.
/// @param path Existing format3 ledger, read only.
/// @param token_text Token text.
/// @return Success for the active run with the matching secret; fenced{run_state} for a wrong secret or a run that is
/// not current; unknown_run for a token that parses but names no run, or does not parse.
std::expected<void, execution_error> authorize_run_token(const std::filesystem::path& path, std::string_view token_text);

/// @brief One precondition member of the closed EX-3 vocabulary, checked under the lock.
struct execution_expectation {
  /// @brief Member kind, in the EX-3 vocabulary order.
  enum class kind_t {
    active_revision,
    active_job,
    fact_version,
    role_state,
    generation_current,
    run_active,
    reservation_open,
    request_unused
  };
  kind_t                       kind = kind_t::request_unused; ///< Member kind.
  std::string                  job;                           ///< Job of fact_version, role_state and generation_current.
  std::string                  role;                          ///< Role of fact_version, role_state and generation_current.
  std::optional<std::uint64_t> revision;                      ///< active_revision value; absent for none.
  std::string                  text;                          ///< active_job value: the job, or the empty string for none.
  std::uint64_t                number = 0;      ///< fact_version value, or the generation identifier of generation_current.
  std::string                  state;           ///< role_state state.
  bool                         drifted = false; ///< role_state drifted flag.
  std::optional<std::uint64_t> run;             ///< role_state run identifier.
  bool no_row = false; ///< role_state only: asserts that NO role_state row exists for (job, role) (D37 a).
};

/// @brief One transition the change asks the commit to write, without generation identifiers (native allocates them).
struct execution_transition_input {
  std::string                job;                   ///< Job of the role.
  std::string                role;                  ///< Role identifier.
  std::optional<std::string> before_state;          ///< Locked and snapshot before state; absent for a first transition.
  std::optional<bool>        before_drifted;        ///< Before drift flag; absent exactly when before_state is absent.
  std::string                after_state;           ///< After state.
  bool                       after_drifted = false; ///< After drift flag, true only when after_state is running.
  std::string                reason;                ///< One of the sixteen transition reasons.
};

/// @brief One generation the change opens for a role, without a generation identifier (native allocates it).
struct execution_generation_input {
  std::string                job;       ///< Job identifier.
  std::string                role;      ///< Role identifier.
  std::string                reason;    ///< activated, structural_change, takeover or recreated.
  history::role_structure_v1 structure; ///< Role structure of the new generation.
};

/// @brief One complete next role-state vector entry of the active job after the commit.
struct execution_vector_entry {
  std::string                  job;             ///< Job identifier.
  std::string                  role;            ///< Role identifier.
  std::string                  state;           ///< Role state after the commit.
  bool                         drifted = false; ///< Drift flag after the commit.
  std::optional<std::uint64_t> run;             ///< Carried run, absent for a run created by this change or no run.
};

/// @brief Packet of a claim or takeover: Markdown, the dispatch manifest and the W value.
struct execution_packet {
  std::string markdown; ///< Packet Markdown, stored as a mode 420 file record.
  value_ptr   manifest; ///< dispatch_manifest_v1 value whose work and render digests equal the native fingerprints.
  value_ptr   work;     ///< work_exec_v1 value W with the 24 keys.
};

/// @brief Revision identifiers of a load, each the id of an existing immutable record.
struct execution_revision_input {
  std::uint64_t source         = 0;     ///< Source tree root record identifier.
  std::uint64_t registry       = 0;     ///< Registry tree root record identifier.
  std::uint64_t stdlib         = 0;     ///< Standard-library tree root record identifier.
  std::uint64_t interpretation = 0;     ///< Interpretation value record identifier.
  std::uint64_t versions       = 0;     ///< Versions value record identifier.
  bool          noop           = false; ///< Whether the load answers the active revision unchanged.
};

/// @brief Target vector and policy a verdict change was evaluated against (R18-1 items 3 and 4).
struct execution_verdict_current {
  std::vector<history::verdict_target>
              targets; ///< Complete entries of the current target vector sorted by work; fewer than the reviewer targets when incomplete.
  std::string policy;  ///< xxh3_128 text of the reviewer's current W (E9 A).
  value_ptr   work;    ///< The reviewer's current W, equal to its acknowledged dispatch W (DB-R5).
};

/// @brief Facts of a verdict change (R18 execution_change_v1 for verdict); every member is re-derived natively.
struct execution_verdict_facts {
  std::string         role;     ///< Reviewer role of the active job.
  std::uint64_t       run = 0;  ///< Reviewer run named by the token.
  value_ptr           document; ///< Parsed verdict document read by this invocation (R18-2 steps 1 to 3).
  history::verdict_v1 verdict;  ///< Normalized document; must equal the native normalization of document.
  execution_verdict_current current;        ///< Current target vector, policy and W.
  std::uint64_t             max_rounds = 0; ///< shape.max_rounds of the reviewer W (DB-R5).
  std::string               admission;      ///< R18-5 case: approve, admitted, capped, unowned or historical.
  std::vector<std::string>  repairs;        ///< Repaired worker roles, sorted; nonempty exactly when admitted.
  std::vector<std::string>  blockers_opened; ///< Blocker kinds this verdict opens (R18-5).
};

/// @brief Complete execution change built by a handler from one snapshot (D12, D13, D37).
struct execution_change {
  std::string       operation;  ///< load, activate, claim, takeover or verdict.
  std::string       request_id; ///< Request identifier.
  std::string       actor;      ///< Actor of the request.
  execution_framing request;    ///< Framing whose operation and actor match this change.
  std::string
      job; ///< Load and activate: the active job after the commit, or the empty string; claim and takeover: the target job.
  std::string                             role;         ///< Claim and takeover: the target role; otherwise empty.
  std::optional<std::uint64_t>            previous_run; ///< Takeover only: the run that the takeover fences.
  std::optional<execution_revision_input> revision;     ///< Load only.
  std::vector<execution_generation_input> generations;  ///< Generations this change opens.
  std::vector<std::uint64_t>              fenced_runs;  ///< Runs this change fences.
  std::vector<execution_transition_input>
      transitions; ///< Ordinal is position plus one; ordinal 1 is the target for claim and takeover (D20).
  std::vector<execution_vector_entry> vector; ///< Complete next role-state vector of the active job after the commit.
  std::optional<execution_packet>     packet; ///< Claim and takeover only.
  value_ptr provenance; ///< Normalized driver_provenance_v1 document, claim and takeover only; absent when not supplied.
  std::vector<execution_expectation> expected; ///< Precondition set checked under the lock.
  std::optional<execution_verdict_facts> verdict; ///< R18: verdict facts, operation verdict only.
  std::vector<std::uint64_t> blockers_cleared; ///< R18: blocker ids this commit clears (load, activate, verdict; DR18-8).
};

/// @brief Result of a committed or replayed execution change.
struct execution_commit_result {
  bool                       replayed = false; ///< Whether the saved result was replayed without a write.
  std::string                saved_result;     ///< Saved result bytes of the request.
  std::uint64_t              event_seq = 0;    ///< Event sequence of the change; zero for a replay.
  std::optional<std::string> token;            ///< Run token text of a claim or takeover commit, once.
};

/// @brief One staged execution transaction: rows written and not yet committed; move-only and thread-confined (D13, D42).
class execution_write {
public:
  /// @brief Transfer the staged transaction, leaving the source unusable.
  execution_write(execution_write&&) noexcept;
  /// @brief Release this staged transaction and transfer the other one.
  /// @return This staged transaction.
  execution_write& operator=(execution_write&&) noexcept;
  /// @brief Roll back a staged transaction that was neither committed nor aborted.
  ~execution_write();
  /// @brief Get the result of the staged change.
  /// @return Result, with the saved result bytes and the allocated event sequence.
  const execution_commit_result& result() const;
  /// @brief Issue COMMIT once; a replayed result commits nothing and succeeds.
  /// @return Success, or the closed failure category; a failure leaves nothing written.
  std::expected<void, execution_error> commit();
  /// @brief Roll back the staged transaction without writing anything.
  void abort() noexcept;

private:
  struct impl; // defined in execution.cpp (D42)
  explicit execution_write(std::unique_ptr<impl>) noexcept;
  std::unique_ptr<impl>                                  _impl;
  friend std::expected<execution_write, execution_error> stage_execution(const std::filesystem::path&, const execution_snapshot&,
                                                                         const execution_change&);
};

/// @brief Validate a change against its snapshot, then write every row inside a write transaction without committing.
/// @param path Existing format3 ledger.
/// @param snapshot Snapshot the change was built from. The snapshot checks run before the lock unless the request id is
/// already used, in which case the request is answered under the lock. Under the lock the snapshot identity is compared
/// with the stored ledger identity first (a mismatch, meaning the ledger was replaced at the path since the snapshot, is
/// refused with path_denied), and then the preconditions are checked against the stored state.
/// @param change Complete change.
/// @return Staged transaction, or invalid_change, resource_limit, request_conflict, revision_conflict, state_conflict,
/// claim_conflict, fenced, hash_collision, path_denied (the ledger was replaced since the snapshot, or a path check at
/// open refused the ledger: bad parent, non-regular file, or a file-identity change between descriptor and open) or a
/// storage category.
std::expected<execution_write, execution_error>
stage_execution(const std::filesystem::path& path, const execution_snapshot& snapshot, const execution_change& change);

/// @brief Stage and commit one execution change in one call.
/// @param path Existing format3 ledger.
/// @param snapshot Snapshot the change was built from.
/// @param change Complete change.
/// @return Commit result, or the closed failure category with nothing written.
std::expected<execution_commit_result, execution_error>
commit_execution(const std::filesystem::path& path, const execution_snapshot& snapshot, const execution_change& change);

/// @brief The R18-8 history expansion of one verdict_recorded event, decoded natively without Lua.
/// @param path Existing format3 ledger, read only; the whole ledger is verified first.
/// @param event Event sequence.
/// @return The execution object {verdict, repairs, blockers, provenance}, nullopt when the event is absent or is not a
/// verdict_recorded event, or the closed failure category (ledger_corrupt for a missing referenced row).
std::expected<std::optional<value_ptr>, execution_error> read_verdict_expansion(const std::filesystem::path& path,
                                                                                std::uint64_t                event);

/// @brief Saved result bytes of one request (history --request-id; for a verdict the verdict_response_v1 bytes).
/// @param path Existing format3 ledger, read only; the whole ledger is verified first.
/// @param request_id Request identifier.
/// @return The saved bytes, nullopt for an unknown request, or the closed failure category.
std::expected<std::optional<std::string>, execution_error> read_saved_result(const std::filesystem::path& path,
                                                                             std::string_view             request_id);

} // namespace planar::engine::ledger

// R18: module-linkage helpers shared by execution.cpp and execution_review.cpp (DB-R3). They are not exported; every
// one runs on an open connection inside the caller's transaction and reports storage failures in the closed vocabulary.
namespace planar::engine::ledger {

/// @brief Generation identifiers of the structural lineage ending at one generation (EX-5 Lineage), in ordinal order.
/// @param db Open connection inside a transaction.
/// @param generation Generation record identifier; the lineage is the chain of takeover generations back to the last
/// generation with reason activated, structural_change or recreated.
/// @return Identifiers from the lineage start to the generation, or ledger_corrupt for an unknown generation.
std::expected<std::vector<std::uint64_t>, execution_error> lineage_generations(connection& db, std::uint64_t generation);

/// @brief Repair revision of a worker in the lineage of one generation (R18-T derived value), or 0.
/// @param db Open connection inside a transaction.
/// @param job Job identifier.
/// @param role Role identifier.
/// @param generation The role's current generation record identifier.
/// @return Revision of the latest repairs row whose generation lies in the lineage, or 0.
std::expected<std::uint64_t, execution_error> repair_revision(connection& db, std::string_view job, std::string_view role,
                                                              std::uint64_t generation);

/// @brief Activation of a verdict: the latest job_activations row of its job with event_seq below the verdict seq.
/// @param db Open connection inside a transaction.
/// @param verdict verdicts.id.
/// @return Activation identifier, nullopt when none precedes the verdict, or ledger_corrupt for an unknown verdict.
std::expected<std::optional<std::uint64_t>, execution_error> verdict_activation(connection& db, std::uint64_t verdict);

/// @brief Review count of a job in one activation: its verdicts of that activation with at least one repairs row.
/// @param db Open connection inside a transaction.
/// @param job Job identifier.
/// @param activation Activation identifier.
/// @return Count of admitted rejections (E17 A).
std::expected<std::uint64_t, execution_error> review_count(connection& db, std::string_view job, std::uint64_t activation);

/// @brief Open blockers of one job, sorted by id.
/// @param db Open connection inside a transaction.
/// @param job Job identifier.
/// @return Open blocker rows.
std::expected<std::vector<execution_blocker>, execution_error> open_blockers(connection& db, std::string_view job);

/// @brief Verify invariants 21 to 27 and the DRAFT-STUB completion rows, then fill snapshot completions and review.
/// @param db Open connection inside the verifier's transaction; the EX-1 verifier has already accepted every other table.
/// @param snapshot Snapshot built from the verified rows; its completions and review members are replaced.
/// @return Success, ledger_corrupt for any violated invariant, or a storage category.
std::expected<void, execution_error> read_review(connection& db, execution_snapshot& snapshot);

/// @brief Members the R18 review adds to the D24 state value S: completions and review, each only when nonempty.
/// @param snapshot Snapshot whose completions and review are measured.
/// @return Key and value pairs to insert into S; empty for a ledger without completions or review rows (DB-R16).
std::vector<std::pair<std::string, value_ptr>> review_state_entries(const execution_snapshot& snapshot);

/// @brief Structural rules of a verdict change (operation, identifiers, absent claim members, expectation kinds).
/// @param change Change with operation verdict.
/// @return Success or invalid_change.
std::expected<void, execution_error> check_verdict_shape(const execution_change& change);

/// @brief Native step-5 checks of a verdict change against its snapshot (R18 native interface table).
/// @param snapshot Snapshot the change was built from.
/// @param change Change with operation verdict.
/// @return Success or invalid_change for any mismatch (the native spelling of internal, D151).
std::expected<void, execution_error> check_verdict_snapshot(const execution_snapshot& snapshot, const execution_change& change);

/// @brief Snapshot rules of blockers_cleared for load and activate (DB-R7); claim and takeover may clear nothing.
/// @param snapshot Snapshot the change was built from.
/// @param change Any non-verdict change.
/// @return Success or invalid_change.
std::expected<void, execution_error> check_blockers_cleared(const execution_snapshot& snapshot, const execution_change& change);

/// @brief Lock-time preconditions of a verdict (R18-4): fenced with the locked run state, otherwise state_conflict.
/// @param db Open connection inside the write transaction.
/// @param change Change with operation verdict.
/// @return Success, fenced, state_conflict or a storage category.
std::expected<void, execution_error> check_verdict_locked(connection& db, const execution_change& change);

/// @brief Write every row of a verdict commit inside the caller's transaction (R18-4); records are interned already.
/// @param db Open connection inside the write transaction.
/// @param identity Ledger identity for the saved result.
/// @param change Change with operation verdict.
/// @param request_body Interned request record identifier.
/// @param verdict_body Interned verdict_v1 value record identifier.
/// @param now Commit timestamp.
/// @return Saved verdict_response_v1 bytes and the event sequence, or the closed failure category.
std::expected<execution_commit_result, execution_error> write_verdict(connection& db, std::string_view identity,
                                                                      const execution_change& change,
                                                                      std::uint64_t request_body, std::uint64_t verdict_body,
                                                                      std::string_view now);

/// @brief Clear blockers inside the caller's transaction: each must be open under the lock (else state_conflict).
/// @param db Open connection inside the write transaction.
/// @param ids Blocker identifiers.
/// @param seq Clearing event sequence.
/// @param job Job whose blockers a load may clear; empty to accept any job (activate, job-clearing load).
/// @return Success, state_conflict or a storage category.
std::expected<void, execution_error> clear_blockers(connection& db, const std::vector<std::uint64_t>& ids, std::uint64_t seq,
                                                    std::string_view job);

} // namespace planar::engine::ledger
