/// @file profile_acceptance.cppm
/// @brief Atomic completion of one prepared_profile through the shared ledger acceptance transaction.
///
/// The single entry point is the `complete_acceptance` overload taking an opaque prepared_profile. Failures use the
/// shared redacted `format_error` vocabulary and the receipt retains authoritative completion evidence. This module
/// is an interface partition of the acceptance module, re-exported by its primary interface; it imports runtime and the
/// internal partition without re-exporting either.
export module planar.engine.ledger.acceptance:profile;
import std;
import planar.engine.runtime;
import :seam;
export namespace planar::engine::ledger {
/// @brief Complete one prepared profile atomically, resolving exact replay before inspecting the profile.
/// @param path Explicit existing ledger path.
/// @param intent Immutable validated codec2 request and expected revision; must equal the intent bound at open.
/// @param profile Opaque prepared profile retaining native capture receipts; exact replay does not inspect it.
/// @param expected_ledger_identity Native prelude identity, checked before locked request lookup.
/// @param receipt Initially empty thread-confined output; saved bytes and phase survive errors and exceptions.
/// @return Exact saved result or typed refusal; an uncertain receipt never promises rollback.
/// @note Thread-confined per call; native receipts are verified immediately before SQLite commit and no Lua runs.
std::expected<acceptance_result, format_error> complete_acceptance(const std::filesystem::path&                        path,
                                                                   const ::planar::engine::acceptance::request_intent& intent,
                                                                   const ::planar::engine::prepared_profile&           profile,
                                                                   std::string_view expected_ledger_identity,
                                                                   ::planar::engine::ledger::completion_receipt& receipt);
} // namespace planar::engine::ledger
