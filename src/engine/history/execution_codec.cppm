/// @file execution_codec.cppm
/// @brief Pure execution codecs for payload v2, transition rows, role structures, dispatch manifests, work fingerprints,
/// driver provenance, value-record framing, saved results and the packet value; no storage or host access.
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
/// Owned value record: a codec identifier and its encoded typed bytes.
struct execution_value_record {
  std::string codec;   ///< One of the four execution codec identifiers.
  std::string encoded; ///< Canonical_v1 bytes of the typed value.
};
/// @brief Frame canonical value bytes as a value record for one of the four execution codecs.
/// @param codec Codec identifier; canonical_v1 and unknown identifiers are refused.
/// @param canonical_bytes Canonical_v1 bytes of the typed value.
/// @return Exact PLANAR-RECORD value frame, or an invalid refusal. Thread-safe, without shared state.
std::expected<std::string, execution_codec_error> encode_execution_value_record(std::string_view codec,
                                                                                std::string_view canonical_bytes);
/// @brief Decode one exact value frame for one of the four execution codecs.
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
  std::string      operation; ///< One of load, activate, claim or takeover.
  payload_position before;    ///< Position before the event.
  payload_position after;     ///< Position after the event.
  std::string      reason;    ///< Sixteen-reason vocabulary value of the ordinal one transition, or empty without transitions.
  std::map<std::string, std::string> references; ///< Canonical positive u64 decimal references keyed per event type.
};
/// @brief Encode payload version two as canonical JSON with the event type's exact reference key set.
/// @param event_type One of declaration_accepted, load_noop, job_activated, run_claimed or run_taken_over.
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
/// @brief Encode a normalized checkpoint with its exact shape and 8 KiB bound.
/// @return Canonical bytes or a typed codec failure.
std::expected<std::string, execution_codec_error> encode_checkpoint_v1(const value_ptr& checkpoint);
/// @brief Decode a retained checkpoint with the same validation as the encoder.
/// @return Normalized value or a typed codec failure.
std::expected<value_ptr, execution_codec_error> decode_checkpoint_v1(std::string_view bytes);
/// @brief Common saved recovery result identity; no run token is retained.
struct recovery_result_identity {
  std::string ledger, request_id, job, role, run_id, generation;
};
/// @brief Saved refresh result, without assembled packet and manifest.
struct refresh_result_v1 : recovery_result_identity {
  std::string                revision, dispatch_id, work_fp, render_digest;
  std::uint64_t              bytes = 0;
  std::optional<std::string> checkpoint;
  bool                       drift_cleared = false;
};
/// @brief Saved checkpoint result.
struct checkpoint_result_v1 : recovery_result_identity {
  std::string checkpoint, event;
};
/// @brief Saved release result.
struct release_result_v1 : recovery_result_identity {
  std::string                state, role_state, event;
  std::optional<std::string> checkpoint;
};
/// @brief Encode the closed saved refresh shape with native grammar validation.
/// @return Canonical JSON or a typed refusal.
std::expected<std::string, execution_codec_error> encode_refresh_result_v1(const refresh_result_v1& result);
/// @brief Decode a closed saved refresh value.
/// @return Typed fields or a codec refusal.
std::expected<refresh_result_v1, execution_codec_error> decode_refresh_result_v1(std::string_view bytes);
/// @brief Encode the closed saved checkpoint shape.
/// @return Canonical JSON or a typed refusal.
std::expected<std::string, execution_codec_error> encode_checkpoint_result_v1(const checkpoint_result_v1& result);
/// @brief Decode a closed saved checkpoint value.
/// @return Typed fields or a codec refusal.
std::expected<checkpoint_result_v1, execution_codec_error> decode_checkpoint_result_v1(std::string_view bytes);
/// @brief Encode the closed saved release shape.
/// @return Canonical JSON or a typed refusal.
std::expected<std::string, execution_codec_error> encode_release_result_v1(const release_result_v1& result);
/// @brief Decode a closed saved release value.
/// @return Typed fields or a codec refusal.
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
  std::uint64_t              bytes = 0;     ///< Packet Markdown byte length.
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
} // namespace planar::engine::history
