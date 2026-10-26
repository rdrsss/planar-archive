/// @file acceptance.cppm
/// @brief Atomic completion of one prepared embedded acceptance over native storage, with exact replay resolved before candidate inspection.
///
/// Entry points are the two `complete_acceptance` overloads, plus the prepared-profile overload re-exported from the
/// profile partition. Failures use the shared redacted `format_error` vocabulary; the receipt overload retains
/// authoritative completion evidence. The shared transaction body lives in an internal partition that this interface
/// imports but never re-exports; shared state is none (thread-confined per call).
export module planar.engine.ledger.acceptance;
import std;
export import planar.engine.ledger.storage_types;
export import planar.engine.ledger.completion;
export import planar.engine.acceptance.prepared;
import planar.engine.ledger.storage;
import :seam;
export import :profile;
export namespace planar::engine::ledger {
/// Exact committed response bytes (defined by the internal partition, re-exported here).
using ::planar::engine::ledger::acceptance_result;
/// @brief Complete one prepared acceptance atomically, resolving exact replay before inspecting the candidate.
/// @param path Explicit existing ledger path.
/// @param intent Immutable validated request and expected revision.
/// @param candidate Immutable prepared input retaining native capture receipts.
/// @return Exact saved result after commit, or a redacted refusal with all tentative writes rolled back.
/// @note Thread-confined per call; native receipts are verified immediately before SQLite commit.
std::expected<acceptance_result, format_error>
complete_acceptance(const std::filesystem::path& path, const ::planar::engine::acceptance::request_intent& intent,
                    const ::planar::engine::acceptance::prepared_acceptance& candidate);
/// @brief Complete against the ledger identity retained by a native prelude, retaining authoritative completion evidence.
/// @param path Explicit existing ledger path.
/// @param intent Immutable validated request and expected revision.
/// @param candidate Immutable prepared input; exact replay does not inspect it.
/// @param expected_ledger_identity Native prelude identity, checked before locked request lookup.
/// @param receipt Initially empty thread-confined output; saved bytes and phase survive errors and exceptions.
/// @return Exact saved result or typed refusal; an uncertain receipt never promises rollback.
std::expected<acceptance_result, format_error>
complete_acceptance(const std::filesystem::path& path, const ::planar::engine::acceptance::request_intent& intent,
                    const ::planar::engine::acceptance::prepared_acceptance& candidate, std::string_view expected_ledger_identity,
                    ::planar::engine::ledger::completion_receipt& receipt);
} // namespace planar::engine::ledger
