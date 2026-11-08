/// @file
/// @brief Recorded-history projections over one verified snapshot; typed misses and storage failures without live policy.
///
/// Entry points are `read_history`, `event_summary` and `revision_metadata`. Format1 and format2 views are unchanged; on a
/// format3 snapshot the event summary carries the typed reference keys, `--at-event` replays the execution state from rows
/// and `--request-id` returns load and activate results (P2-5). No Lua runs; errors are `history_read_error` values.
export module planar.engine.history.read;
import std;
export import planar.engine.ledger.format;
export import planar.engine.value;
export namespace planar::engine::history {
/// Mutually exclusive recorded-history views.
enum class history_selector { page, revision, request, state };
/// Validated invocation; native IDs are never transported as JSON numbers.
struct history_query {
  history_selector selector = history_selector::page; ///< Selected view.
  std::uint64_t    id       = 0;                      ///< Revision, event or exclusive page cursor.
  std::size_t      limit    = 50;                     ///< Page count, one through 100.
  std::string      request_id;                        ///< Validated ASCII replay key.
};
/// Projection-owned failures, separate from native storage refusal.
enum class history_read_code {
  storage,
  revision_not_found,
  request_not_found,
  event_not_found,
  unsupported_interpretation,
  corrupt
};
/// Redacted failure retaining native storage category only when applicable.
struct history_read_error {
  history_read_code    code; ///< Stable projection category.
  ledger::format_error storage = {ledger::format_error_code::corrupt,
                                  ""}; ///< Original native category without forwarding messages.
};
/// Convert owned revision metadata without I/O or live policy.
/// @param row Verified native row.
/// @return Exact public metadata; all IDs use decimal strings.
value_ptr revision_metadata(const ledger::revision_row& row);
/// Convert a verified event without I/O, validating its public payload through the codec of its ledger format.
/// @param row Verified native row.
/// @param format Ledger format of the row; format3 adds the typed `job`, `role`, `generation_id` and `run_id` keys and
/// validates payload version 2 (P2-5), format1 and format2 keep the eight-key summary.
/// @return Public event summary or corruption refusal.
std::expected<value_ptr, history_read_error> event_summary(const ledger::event_row& row,
                                                           ledger::ledger_format format = ledger::ledger_format::v1);
/// Read one public history view using only this owner's pinned snapshot.
/// @param file Thread-confined verified owner, never reopened or mutated by this function.
/// @param query Validated selector arguments; no host arguments enter the projection.
/// @return Complete public envelope or typed refusal. No Lua, capture or current policy is invoked.
std::expected<value_ptr, history_read_error> read_history(const ledger::format_file& file, const history_query& query);
} // namespace planar::engine::history
