/// @file execution_codec.cppm
/// @brief Pure execution codecs for payload v2, transition rows, role structures, dispatch manifests, work fingerprints,
/// driver provenance, verdict documents (R18-2), value-record framing, saved results (claim, activate, verdict) and the
/// packet value; no storage or host access. Entry points return execution_codec_error; the verdict document checks
/// also export the field-path violation that the R18-2 refusals reuse.
export module planar.engine.history.execution_codec;
import std;
export import planar.engine.history.record;
export import planar.engine.canonical;
export namespace planar::engine::history {
/// Closed codec failure vocabulary. Every encoder violation is invalid; resource_limit names only an EX-8 bound.
enum class execution_codec_code { invalid, resource_limit };
/// Redacted codec refusal. limit and maximum are set only for resource_limit.
struct execution_codec_error {
  execution_codec_code code;        ///< Invalid encoding or an exceeded EX-8 bound.
  std::string          limit;       ///< Bound name, such as manifest_bytes or provenance_depth; empty otherwise.
  std::uint64_t        maximum = 0; ///< Maximum of the named bound; zero otherwise.
};
/// Codec identifier of a dispatch manifest value record.
inline constexpr std::string_view codec_dispatch_manifest_v1 = "dispatch_manifest_v1";
/// Codec identifier of a work fingerprint value record.
inline constexpr std::string_view codec_work_exec_v1 = "work_exec_v1";
/// Codec identifier of a driver provenance value record.
inline constexpr std::string_view codec_driver_provenance_v1 = "driver_provenance_v1";
/// Codec identifier of a role structure value record.
inline constexpr std::string_view codec_role_structure_v1 = "role_structure_v1";
/// Codec identifier of a normalized verdict document value record (R18-2).
inline constexpr std::string_view codec_verdict_v1 = "verdict_v1";
/// @brief Canonical R17 retained value codec.
inline constexpr std::string_view codec_check_definition_v1 = "check_definition_v1";
/// @brief Canonical R17 retained value codec.
inline constexpr std::string_view codec_output_vector_v1 = "output_vector_v1";
/// @brief Canonical R17 retained value codec.
inline constexpr std::string_view codec_check_observation_v1 = "check_observation_v1";
/// @brief Canonical R17 retained value codec.
inline constexpr std::string_view codec_boundary_diff_v1 = "boundary_diff_v1";

/// The only verdict authority this build supports (R18-7); normalization adds it when absent.
inline constexpr std::string_view verdict_authority_reviewer = "reviewer_role_v1";
/// Owned value record: a codec identifier and its encoded typed bytes.
struct execution_value_record {
  std::string codec;   ///< One of the five execution codec identifiers.
  std::string encoded; ///< Canonical_v1 bytes of the typed value.
};
/// @brief Frame canonical value bytes as a value record for one of the five execution codecs.
/// @param codec Codec identifier; canonical_v1 and unknown identifiers are refused.
/// @param canonical_bytes Canonical_v1 bytes of the typed value.
/// @return Exact PLANAR-RECORD value frame, or an invalid refusal. Thread-safe, without shared state.
std::expected<std::string, execution_codec_error> encode_execution_value_record(std::string_view codec,
                                                                                std::string_view canonical_bytes);
/// @brief Decode one exact value frame for one of the five execution codecs.
/// @param frame Complete frame; trailing bytes, other record kinds and unknown codecs are refused.
/// @return Owned codec identifier and encoded bytes, or an invalid refusal. Thread-safe, without shared state.
std::expected<execution_value_record, execution_codec_error> decode_execution_value_record(std::string_view frame);
/// @brief Hash the complete value frame with seed-zero XXH3-128.
/// @param frame Complete frame accepted by decode_execution_value_record.
/// @return Canonical sixteen-byte digest, or an invalid refusal. Thread-safe, without shared state.
std::expected<record_digest, execution_codec_error> digest_execution_value_record(std::string_view frame);
/// Active position of a payload: the revision decimal or none, and the job ID or the empty string.
struct payload_position {
  std::string active_revision; ///< Canonical positive u64 decimal or none.
  std::string active_job;      ///< Job identifier, or the empty string when no job is active.
};
/// Payload version two of a format3 event; the references key set depends on the event type.
struct event_payload_v2 {
  std::string      operation; ///< One of load, activate, claim, takeover or verdict.
  payload_position before;    ///< Position before the event.
  payload_position after;     ///< Position after the event.
  std::string      reason;    ///< Sixteen-reason vocabulary value of the ordinal one transition, or empty without transitions.
  std::map<std::string, std::string> references; ///< Canonical positive u64 decimal references keyed per event type.
};
/// @brief Encode payload version two as canonical JSON with the event type's exact reference key set.
/// @param event_type One of declaration_accepted, load_noop, job_activated, run_claimed, run_taken_over or
/// verdict_recorded (references exactly body, run and verdict).
/// @param payload Owned payload; every field is validated against its grammar before encoding.
/// @return Compact sorted-key JSON without a terminal LF, or an invalid refusal. Thread-safe, without shared state.
std::expected<std::string, execution_codec_error> encode_event_payload_v2(std::string_view        event_type,
                                                                          const event_payload_v2& payload);
/// @brief Decode exact payload version two bytes for one event type.
/// @param event_type Event type that selects the reference key set.
/// @param bytes Stored JSON; version must be 2, unknown, missing and duplicate keys and trailing bytes are refused.
/// @return Owned payload, or an invalid refusal. Thread-safe, without shared state.
std::expected<event_payload_v2, execution_codec_error> decode_event_payload_v2(std::string_view event_type,
                                                                               std::string_view bytes);
/// One transition row of a format3 event, as stored in the transitions table.
struct transition_row {
  std::uint64_t              ordinal = 0;       ///< Positive position within its event, contiguous from one.
  std::string                job;               ///< Job identifier of the role.
  std::string                role;              ///< Role identifier.
  std::uint64_t              generation_id = 0; ///< Positive generation record identifier.
  std::optional<std::string> before_state;      ///< Absent or one of the six role states.
  std::optional<bool> before_drifted; ///< Absent exactly when before_state is absent; false unless before_state is running.
  std::string         after_state;    ///< One of the six role states.
  bool                after_drifted = false; ///< True only when after_state is running.
  std::string         reason;                ///< One of the sixteen transition reasons.
};
/// @brief Validate one transition row against the closed state, reason and drift rules.
/// @param row Owned transition row.
/// @return Success or an invalid refusal. Thread-safe, without shared state.
std::expected<void, execution_codec_error> validate_transition_row(const transition_row& row);
/// @brief Validate every row and require ordinals contiguous from one in the given order.
/// @param rows Transition rows of one event; an empty sequence is valid.
/// @return Success or an invalid refusal. Thread-safe, without shared state.
std::expected<void, execution_codec_error> validate_transition_sequence(std::span<const transition_row> rows);
/// @brief Return the event-level reason, which is the reason of the ordinal one transition.
/// @param rows Validated transition rows of one event.
/// @return The ordinal one reason, or the empty string when the event has no transitions. Thread-safe.
std::string event_level_reason(std::span<const transition_row> rows);
/// Work units owned by one path list within a role structure.
struct ownership_entry {
  std::string              work;  ///< Unit identifier.
  std::vector<std::string> paths; ///< Owned paths of that unit.
};
/// Structure value of one role, whose digest drives generation changes.
struct role_structure_v1 {
  std::string                  job;       ///< Job identifier.
  std::string                  role;      ///< Role identifier.
  std::string                  kind;      ///< Either worker or reviewer.
  std::vector<std::string>     work;      ///< Sorted unique unit identifiers of a worker role.
  std::vector<std::string>     targets;   ///< Sorted unique unit identifiers of a reviewer role.
  std::vector<ownership_entry> ownership; ///< Sorted unique pairs by work identifier.
  std::string                  shape;     ///< Job shape kind.
};
/// @brief Encode a role structure as canonical_v1 bytes after sorting and uniqueness validation.
/// @param structure Owned structure; unsorted or duplicate lists and kinds other than worker or reviewer are refused.
/// @return Canonical bytes, or an invalid refusal. Thread-safe, without shared state.
std::expected<std::string, execution_codec_error> encode_role_structure_v1(const role_structure_v1& structure);
/// @brief Decode canonical role structure bytes with the same validation as the encoder.
/// @param canonical_bytes Exact canonical_v1 bytes.
/// @return Owned structure, or an invalid refusal. Thread-safe, without shared state.
std::expected<role_structure_v1, execution_codec_error> decode_role_structure_v1(std::string_view canonical_bytes);
/// @brief Compute the role structure digest over every field.
/// @param structure Validated structure.
/// @return The xxh3_128 digest text, or an invalid refusal. Thread-safe, without shared state.
std::expected<std::string, execution_codec_error> role_structure_digest(const role_structure_v1& structure);
/// @brief Encode the fourteen-key dispatch manifest as canonical_v1 bytes within the 16 MiB manifest bound.
/// @param manifest Owned manifest value with exactly the fourteen manifest keys.
/// @return Canonical bytes, invalid for shape violations, or resource_limit with manifest_bytes. Thread-safe.
std::expected<std::string, execution_codec_error> encode_dispatch_manifest_v1(const value_ptr& manifest);
/// @brief Decode canonical dispatch manifest bytes with the same key and bound rules.
/// @param bytes Exact canonical_v1 bytes.
/// @return Owned manifest value, or a refusal. Thread-safe, without shared state.
std::expected<value_ptr, execution_codec_error> decode_dispatch_manifest_v1(std::string_view bytes);
/// @brief Encode the twenty-four-key work fingerprint value as canonical_v1 bytes.
/// @param work Owned work value with exactly the work_exec_v1 keys.
/// @return Canonical bytes, or an invalid refusal. Thread-safe, without shared state.
std::expected<std::string, execution_codec_error> encode_work_exec_v1(const value_ptr& work);
/// @brief Decode canonical work fingerprint bytes with the same key rules.
/// @param bytes Exact canonical_v1 bytes.
/// @return Owned work value, or an invalid refusal. Thread-safe, without shared state.
std::expected<value_ptr, execution_codec_error> decode_work_exec_v1(std::string_view bytes);
/// @brief Hash the work value under the work_exec_v1 domain.
/// @param work Validated work value.
/// @return The xxh3_128 digest text of the work_exec_v1 domain, or an invalid refusal. Thread-safe.
std::expected<std::string, execution_codec_error> work_exec_digest(const value_ptr& work);
/// @brief Encode the stored driver provenance record for a normalized document.
/// @param document Owned document with the fixed provenance shape; bounds and leaf rules are enforced first.
/// @return Canonical bytes of {version,authority,document}, or a refusal: resource_limit for an EX-8 bound, otherwise invalid.
std::expected<std::string, execution_codec_error> encode_driver_provenance_v1(const value_ptr& document);
/// @brief Decode a stored driver provenance record and apply the same bounds and leaf rules.
/// @param bytes Exact canonical bytes of the stored record.
/// @return Owned normalized document, or a refusal. Thread-safe, without shared state.
std::expected<value_ptr, execution_codec_error> decode_driver_provenance_v1(std::string_view bytes);
// Verdict document (R18-2). The field table is checked on structural values so that the native reader and the codec
// share one rule set; violations carry the P2-4 field path (verdict, or verdict. plus keys and decimal indexes).
/// One finding of a verdict document.
struct verdict_finding {
  std::string                 id;           ///< Finding identifier, [A-Za-z0-9_.:-]{1,128}, unique within the document.
  std::string                 severity;     ///< One of must, should or note.
  std::string                 work;         ///< Work identifier equal to some target work.
  std::optional<std::string>  path;         ///< Portable source path of 1 to 4096 bytes.
  std::optional<std::int64_t> line;         ///< Line 1 to 2147483647, only with path.
  std::optional<std::string>  prescription; ///< Declaration identifier.
  std::string                 text;         ///< 1 to 4096 bytes without NUL.
};
/// One reviewed target: a work unit, its output identity and its work fingerprint.
struct verdict_target {
  std::string work;    ///< Work identifier.
  std::string output;  ///< Output digest text.
  std::string work_fp; ///< Work fingerprint digest text.
};
/// Typed verdict document. A normalized document always carries authority reviewer_role_v1.
struct verdict_v1 {
  std::vector<verdict_target>  targets;   ///< 1 to 256 entries sorted strictly by work.
  std::string                  policy;    ///< Policy identity digest text.
  std::string                  decision;  ///< approve or reject.
  std::vector<verdict_finding> findings;  ///< 0 to 256 findings in document order.
  std::vector<std::string>     repair;    ///< Sorted unique worker role identifiers.
  std::string                  summary;   ///< 0 to 16384 bytes without NUL.
  std::string                  authority; ///< Asserted authority; empty when the parsed document omitted it.
};
/// A failed verdict field rule: the P2-4 field path and an existing schema-normalizer message.
struct verdict_violation {
  std::string field;   ///< verdict, or verdict. followed by keys and decimal array indexes.
  std::string message; ///< Existing message such as Required field, Unknown field or Malformed output digest.
};
/// @brief Apply the R18-2 step 4 bounds to a parsed document, in the order findings count, finding text, summary.
/// @param document Parsed structural value; shapes the bounds do not apply to are left for the shape check.
/// @return Success, or resource_limit naming verdict_findings, verdict_finding_bytes or verdict_summary_bytes.
std::expected<void, execution_codec_error> check_verdict_bounds(const value_ptr& document);
/// @brief Apply the R18-2 step 5 shape and field table and extract the typed document.
/// @param document Parsed structural value within the step 4 bounds.
/// @return The typed document (authority empty when absent), or the first violation in the documented order.
std::expected<verdict_v1, verdict_violation> verdict_from_value(const value_ptr& document);
/// @brief Apply the R18-2 step 7 approve rules: approve with a must finding or a nonempty repair list is refused.
/// @param verdict Typed document that passed the field table.
/// @return Success, or the violation at field verdict.decision.
std::expected<void, verdict_violation> check_verdict_decision(const verdict_v1& verdict);
/// @brief Build the normalized structural value: version 1, optional finding keys omitted when absent.
/// @param verdict Typed document; authority must be reviewer_role_v1.
/// @return The structural value, or an invalid refusal when any R18-2 rule fails. Thread-safe.
std::expected<value_ptr, execution_codec_error> verdict_value(const verdict_v1& verdict);
/// @brief Encode the normalized verdict document as canonical_v1 bytes.
/// @param verdict Typed document; every R18-2 rule (bounds, field table, authority, approve rules) is re-checked.
/// @return Canonical bytes, resource_limit for a verdict bound, otherwise invalid. Thread-safe.
std::expected<std::string, execution_codec_error> encode_verdict_v1(const verdict_v1& verdict);
/// @brief Decode stored canonical verdict bytes with exact keys and the same rules as the encoder.
/// @param bytes Exact canonical_v1 bytes.
/// @return The typed normalized document, or a refusal. Thread-safe, without shared state.
std::expected<verdict_v1, execution_codec_error> decode_verdict_v1(std::string_view bytes);
/// @brief Encode a normalized checkpoint with its exact shape and 8 KiB bound.
/// @return Canonical bytes or a typed codec failure.
/// @param checkpoint Normalized checkpoint value.
std::expected<std::string, execution_codec_error> encode_checkpoint_v1(const value_ptr& checkpoint);
/// @brief Decode a retained checkpoint with the same validation as the encoder.
/// @return Normalized value or a typed codec failure.
/// @param bytes Exact canonical checkpoint bytes.
std::expected<value_ptr, execution_codec_error> decode_checkpoint_v1(std::string_view bytes);
/// @brief Common saved recovery result identity; no run token is retained.
struct recovery_result_identity {
  std::string ledger;     ///< Ledger identity.
  std::string request_id; ///< Idempotency key of the recorded request.
  std::string job;        ///< Active job identifier.
  std::string role;       ///< Target role identifier.
  std::string run_id;     ///< Canonical decimal run identifier, without a secret.
  std::string generation; ///< Canonical decimal generation ordinal.
};
/// @brief Saved refresh result, without assembled packet and manifest.
struct refresh_result_v1 : recovery_result_identity {
  std::string                revision;              ///< Accepted revision acknowledged by this refresh.
  std::string                dispatch_id;           ///< Retained dispatch identifier.
  std::string                work_fp;               ///< Acknowledged work_exec_v1 digest.
  std::string                render_digest;         ///< Retained packet Markdown digest.
  std::uint64_t              bytes = 0;             ///< Retained Markdown byte length.
  std::optional<std::string> checkpoint;            ///< Checkpoint identifier when a note was retained.
  bool                       drift_cleared = false; ///< Whether refresh cleared the run drift flag.
};
/// @brief Saved checkpoint result.
struct checkpoint_result_v1 : recovery_result_identity {
  std::string checkpoint; ///< Newly retained checkpoint identifier.
  std::string event;      ///< Commit event sequence.
};
/// @brief Saved release result.
struct release_result_v1 : recovery_result_identity {
  std::string                state;      ///< Terminal run state after release.
  std::string                role_state; ///< Derived role state after release.
  std::string                event;      ///< Commit event sequence.
  std::optional<std::string> checkpoint; ///< Checkpoint identifier when a note was retained.
};
/// @brief Encode the closed saved refresh shape with native grammar validation.
/// @return Canonical JSON or a typed refusal.
/// @param result Typed saved result.
std::expected<std::string, execution_codec_error> encode_refresh_result_v1(const refresh_result_v1& result);
/// @brief Decode a closed saved refresh value.
/// @return Typed fields or a codec refusal.
/// @param bytes Exact saved JSON result bytes.
std::expected<refresh_result_v1, execution_codec_error> decode_refresh_result_v1(std::string_view bytes);
/// @brief Encode the closed saved checkpoint shape.
/// @return Canonical JSON or a typed refusal.
/// @param result Typed saved result.
std::expected<std::string, execution_codec_error> encode_checkpoint_result_v1(const checkpoint_result_v1& result);
/// @brief Decode a closed saved checkpoint value.
/// @return Typed fields or a codec refusal.
/// @param bytes Exact saved JSON result bytes.
std::expected<checkpoint_result_v1, execution_codec_error> decode_checkpoint_result_v1(std::string_view bytes);
/// @brief Encode the closed saved release shape.
/// @return Canonical JSON or a typed refusal.
/// @param result Typed saved result.
std::expected<std::string, execution_codec_error> encode_release_result_v1(const release_result_v1& result);
/// @brief Decode a closed saved release value.
/// @return Typed fields or a codec refusal.
/// @param bytes Exact saved JSON result bytes.
std::expected<release_result_v1, execution_codec_error> decode_release_result_v1(std::string_view bytes);
// Saved results (D25): Glaze JSON via planar.engine.json_codec, sorted keys, no terminal LF.
/// Saved result of a claim or takeover request, without the packet and manifest.
struct claim_result_v1 {
  std::string                ledger;        ///< Ledger identity as 32 lowercase hex.
  std::string                request_id;    ///< Bounded request token.
  std::string                job;           ///< Job identifier.
  std::string                role;          ///< Role identifier.
  std::string                revision;      ///< Revision decimal.
  std::string                run_id;        ///< Run decimal.
  std::string                generation;    ///< Generation ordinal decimal.
  std::string                token;         ///< Run token: run id, generation ordinal and 32 lowercase hex.
  std::string                dispatch_id;   ///< Dispatch decimal.
  std::string                work_fp;       ///< Work fingerprint digest text.
  std::string                render_digest; ///< Render digest text.
  std::uint64_t              bytes = 0;     ///< Retained Markdown byte length.     ///< Packet Markdown byte length.
  std::optional<std::string> previous_run;  ///< Present only for takeover.
};
/// One role entry of an activation result.
struct activate_role_v1 {
  std::string role;       ///< Role identifier.
  std::string generation; ///< New generation ordinal decimal.
  std::string state;      ///< After-state, blocked or ready.
};
/// Saved result of an activate request.
struct activate_result_v1 {
  std::string                   ledger;       ///< Ledger identity as 32 lowercase hex.
  std::string                   request_id;   ///< Bounded request token.
  std::string                   revision;     ///< Revision decimal.
  std::string                   job;          ///< Activated job identifier.
  std::string                   previous_job; ///< Previous job identifier or the empty string.
  std::string                   activation;   ///< Activation decimal.
  std::string                   event;        ///< Event decimal.
  std::vector<activate_role_v1> roles;        ///< Sorted unique by role identifier.
};
/// @brief Encode the claim saved result as Glaze JSON with sorted keys and no terminal LF.
/// @param result Owned result; takeover is represented by a present previous_run.
/// @return Exact JSON bytes, or an invalid refusal. Thread-safe, without shared state.
std::expected<std::string, execution_codec_error> encode_claim_result_v1(const claim_result_v1& result);
/// @brief Decode the claim saved result with the same grammar checks as the encoder.
/// @param bytes Exact saved JSON bytes.
/// @return Owned result, or an invalid refusal. Thread-safe, without shared state.
std::expected<claim_result_v1, execution_codec_error> decode_claim_result_v1(std::string_view bytes);
/// @brief Encode the activate saved result as Glaze JSON with roles sorted by role and no terminal LF.
/// @param result Owned result; roles must be sorted unique by role identifier.
/// @return Exact JSON bytes, or an invalid refusal. Thread-safe, without shared state.
std::expected<std::string, execution_codec_error> encode_activate_result_v1(const activate_result_v1& result);
/// @brief Decode the activate saved result with the same grammar checks as the encoder.
/// @param bytes Exact saved JSON bytes.
/// @return Owned result, or an invalid refusal. Thread-safe, without shared state.
std::expected<activate_result_v1, execution_codec_error> decode_activate_result_v1(std::string_view bytes);
/// One repaired worker in a verdict saved result.
struct verdict_result_repair {
  std::string role;       ///< Repaired worker role identifier.
  std::string generation; ///< Worker generation ordinal decimal.
  std::string revision;   ///< New repair revision decimal.
};
/// One blocker opened by a verdict commit.
struct verdict_result_blocker {
  std::string id;   ///< Blocker decimal.
  std::string kind; ///< max_rounds or unowned_rejection.
};
/// After-state of one role with a transition in the verdict commit.
struct verdict_result_role {
  std::string role;            ///< Role identifier.
  std::string state;           ///< One of the six role states.
  bool        drifted = false; ///< True only when state is running.
};
/// Saved result of a verdict request (verdict_response_v1); it never carries a token.
struct verdict_result_v1 {
  std::string                         ledger;           ///< Ledger identity as 32 lowercase hex.
  std::string                         request_id;       ///< Bounded request token.
  std::string                         job;              ///< Active job identifier.
  std::string                         role;             ///< Reviewer role identifier.
  std::string                         revision;         ///< Active revision decimal.
  std::string                         run;              ///< Reviewer run decimal.
  std::string                         event;            ///< verdict_recorded event decimal.
  std::string                         verdict_id;       ///< Verdict row decimal.
  std::string                         applicability;    ///< current or historical.
  std::string                         decision;         ///< approve or reject.
  bool                                admitted = false; ///< True exactly for an admitted rejection.
  std::vector<verdict_result_repair>  repairs;          ///< Sorted unique by role; nonempty exactly when admitted.
  std::string                         count;            ///< Review count after the commit, nonnegative decimal.
  std::string                         max_rounds;       ///< max_rounds of the active job, nonnegative decimal.
  std::vector<verdict_result_blocker> opened;           ///< Opened blockers ascending by id.
  std::vector<std::string>            cleared;          ///< Cleared blocker decimals ascending.
  std::vector<verdict_result_role>    roles;            ///< Sorted unique by role identifier.
};
/// @brief Encode the verdict saved result as Glaze JSON with sorted keys and no terminal LF.
/// @param result Owned result; grammar, ordering and admission consistency are validated first.
/// @return Exact JSON bytes, or an invalid refusal. Thread-safe, without shared state.
std::expected<std::string, execution_codec_error> encode_verdict_result_v1(const verdict_result_v1& result);
/// @brief Decode the verdict saved result with the same grammar checks as the encoder.
/// @param bytes Exact saved JSON bytes.
/// @return Owned result, or an invalid refusal. Thread-safe, without shared state.
std::expected<verdict_result_v1, execution_codec_error> decode_verdict_result_v1(std::string_view bytes);
/// Fields of the dispatch packet value; work holds work identifiers that must be sorted and unique.
struct packet_fields {
  std::string              workspace; ///< Workspace identifier.
  std::string              spec;      ///< Spec identifier.
  std::string              job;       ///< Job identifier.
  std::string              role;      ///< Role identifier.
  std::string              kind;      ///< Worker or reviewer.
  std::vector<std::string> work;      ///< Sorted unique work identifiers (render-contract 420-422).
  std::string              markdown;  ///< Packet Markdown text.
};
/// @brief Assemble the dispatch packet value with exactly the packet keys and mode dispatch.
/// @param fields Owned packet fields; unsorted or duplicate work identifiers are refused.
/// @return Packet value with keys version, mode, workspace, spec, job, role, kind, work and markdown, or an invalid refusal.
std::expected<value_ptr, execution_codec_error> assemble_packet_value(const packet_fields& fields);
/// @brief Encode a structurally validated R17 retained value in canonical form.
/// @param codec One of the four R17 value codec identifiers.
/// @param document Value to validate and encode.
/// @return Canonical bytes, or invalid/resource_limit.
std::expected<std::string, execution_codec_error> encode_check_value(std::string_view codec, const value_ptr& document);
/// @brief Decode and validate an R17 canonical value.
/// @param codec Expected codec identifier.
/// @param bytes Canonical encoded value.
/// @return Validated value, or invalid/resource_limit.
std::expected<value_ptr, execution_codec_error> decode_check_value(std::string_view codec, std::string_view bytes);

} // namespace planar::engine::history
