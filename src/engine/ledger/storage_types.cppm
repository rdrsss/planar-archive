/// @file storage_types.cppm
/// @brief Inert verified-ledger metadata, access and owned row values shared by native storage, format and acceptance; no functions or ownership.
export module planar.engine.ledger.storage_types;
import std;
export import planar.engine.ledger.types;
export import planar.engine.ledger.history_codec;
export namespace planar::engine::ledger {
/// Exact shared ledger failure category.
using format_error_code = ::planar::engine::ledger::types::format_error_code;
/// Exact shared redacted ledger failure.
using format_error = ::planar::engine::ledger::types::format_error;
/// @brief Exact supported on-disk schema discriminator.
enum class ledger_format { v1 = 1, v2 = 2, v3 = 3 };
/// @brief Verified initialization metadata; identifiers remain strings at JSON boundaries.
struct ledger_metadata {
  std::string   identity;                   ///< Thirty-two lowercase hexadecimal digits from sixteen OS-random bytes.
  std::string   workspace_id;               ///< Declaration identifier grammar.
  std::string   created_at;                 ///< UTC RFC3339 timestamp with milliseconds.
  ledger_format format = ledger_format::v1; ///< Exact verified schema, retained independently of request codecs.
};
/// @brief Requested access after independent read-only verification.
enum class ledger_access { read_only, read_write };
/// Exact shared native immutable record identity.
using record_reference = ::planar::engine::ledger::types::record_reference;
/// Exact shared owned bounded record.
using stored_record = ::planar::engine::ledger::types::stored_record;
/// Owned current projection from the pinned snapshot.
struct history_head {
  std::optional<std::int64_t> revision;      ///< Active revision, absent before the first acceptance.
  std::int64_t                last_sequence; ///< Last committed event sequence.
};
/// Owned immutable revision metadata; all five references are database local.
struct revision_row {
  std::int64_t                               id;          ///< Positive revision identity.
  std::optional<std::int64_t>                parent;      ///< Older revision, absent for initial acceptance.
  ::planar::engine::ledger::event_references references;  ///< Complete captured roots.
  std::string                                actor;       ///< Bounded UTF-8 native assertion.
  std::string                                accepted_at; ///< Contextual UTC time, never a causal clock.
};
/// Owned event row retaining exact saved canonical payload bytes.
struct event_row {
  std::int64_t                sequence;        ///< Positive event sequence.
  std::string                 type;            ///< Fixed event type.
  std::int64_t                version;         ///< Event row version.
  std::optional<std::int64_t> revision;        ///< Absent only at initialization.
  std::optional<std::string>  request_id;      ///< Absent only at initialization.
  std::string                 actor;           ///< Bounded UTF-8 native assertion.
  std::string                 occurred_at;     ///< Contextual UTC time.
  std::string                 payload;         ///< Exact canonical event bytes; decode with the pure history codec.
  std::string                 job{};           ///< Format3 `events.job`; empty when NULL and on format1/format2.
  std::string                 role{};          ///< Format3 `events.role`; empty when NULL and on format1/format2.
  std::optional<std::int64_t> generation_id{}; ///< Format3 `events.generation_id`; absent when NULL.
  std::optional<std::int64_t> run_id{};        ///< Format3 `events.run_id`; absent when NULL.
};
/// Owned private replay data. Host argument bytes must never appear in public history output.
struct request_row {
  std::string                                       request_id;     ///< Bounded replay key.
  ::planar::engine::ledger::types::record_reference body_reference; ///< Validated request record identity.
  std::string                            body_encoded;   ///< Exact stored request codec bytes, including private host arguments.
  std::int64_t                           event_sequence; ///< Saved successful operation event.
  std::int64_t                           revision;       ///< Returned revision.
  ::planar::engine::ledger::load_outcome outcome;        ///< Accepted or unchanged result.
  std::string                            result;         ///< Exact saved canonical result bytes.
};
/// Owned format3 request row without its private body: any execution operation and its saved result bytes.
struct execution_request_row {
  std::string  request_id;     ///< Bounded replay key.
  std::string  operation;      ///< Stored operation, such as `load` or `activate`.
  std::string  outcome;        ///< Stored outcome, such as `accepted`, `noop` or `activated`.
  std::int64_t event_sequence; ///< Saved successful operation event.
  std::int64_t revision;       ///< Revision named by the request row.
  std::string  result;         ///< Exact saved canonical result bytes (at most 1 MiB).
};
/// @brief Owned transition fields for an event expansion, with no private run secret.
struct execution_transition_detail {
  std::int64_t               ordinal = 0, generation = 0;
  std::string                job, role, after, reason;
  std::optional<std::string> before;
  std::optional<bool>        before_drifted;
  bool                       after_drifted = false;
};
/// @brief Native metadata needed to expand a recorded execution event within its pinned snapshot.
struct execution_event_detail {
  std::vector<execution_transition_detail> transitions;
  std::optional<std::int64_t> run, generation, dispatch, packet, manifest, work, checkpoint, checkpoint_body, provenance,
      structure;
  std::string run_request, dispatch_kind, work_fp, render_fp;
};
/// Run of a replayed format3 role: its identifier and the ordinal of its generation.
struct state_run_reference {
  std::int64_t id;         ///< Positive run identifier.
  std::int64_t generation; ///< Positive generation ordinal of the run.
};
/// One role of the active job replayed at an event from its last transition row.
struct state_role_row {
  std::string                        role;       ///< Role identifier.
  std::int64_t                       generation; ///< Generation ordinal of the last transition.
  std::string                        state;      ///< After-state of the last transition.
  bool                               drifted;    ///< After-drift of the last transition.
  std::optional<state_run_reference> run;        ///< Run claimed at the last transition into running, while running.
};
/// One run active at a replayed event.
struct state_run_row {
  std::int64_t id;         ///< Positive run identifier.
  std::string  role;       ///< Role of the run.
  std::int64_t generation; ///< Generation ordinal of the run.
};
/// Format3 execution state replayed from rows with sequence at most one event, without Lua.
struct execution_state_at {
  std::optional<std::int64_t> activation; ///< Latest job activation with event_seq at most the event, absent when none.
  std::vector<state_role_row> roles;      ///< Roles of the given job at the event, sorted by role.
  std::vector<state_run_row>  runs;       ///< Runs active at the event, sorted by id.
};
/// One bounded ascending event page from the pinned snapshot.
struct event_page {
  std::vector<event_row>      events;     ///< At most the explicitly requested limit.
  std::optional<std::int64_t> next_after; ///< Last returned sequence only when this snapshot has more events.
};
} // namespace planar::engine::ledger
