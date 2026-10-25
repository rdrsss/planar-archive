module planar.engine.ledger.acceptance;
import std;
import planar.engine.ledger.storage;
import planar.engine.history.tree_core;
namespace planar::engine::ledger {
namespace {
using failure = std::unexpected<format_error>;
failure refuse(format_error_code code, std::string message, std::optional<types::format_error_detail> detail = {}) {
  return failure(format_error{code, std::move(message), std::move(detail)});
}
std::expected<acceptance_result, format_error>
complete_acceptance_impl(const std::filesystem::path& path, const ::planar::engine::acceptance::request_intent& intent,
                         const ::planar::engine::acceptance::prepared_acceptance& candidate,
                         std::optional<std::string_view>                          expected_ledger_identity,
                         ::planar::engine::ledger::completion_receipt&            receipt) {
  if (receipt.phase != ::planar::engine::ledger::completion_phase::not_completed || !receipt.saved_result.empty())
    return refuse(format_error_code::invalid_argument, "Completion receipt is already in use");
  auto request = intent.data();
  if (!request)
    return refuse(format_error_code::invalid_argument, "Unavailable request intent");
  // Retain only this local RAII owner: every return and exception destroys the active transaction.
  auto tx = begin_write(path);
  if (!tx)
    return std::unexpected(tx.error());
  const auto& data = **request;
  if (expected_ledger_identity && *expected_ledger_identity != tx->metadata().identity)
    return refuse(format_error_code::unsafe_path, "Ledger identity differs from prelude");
  {
    auto saved = tx->read_request(data.request_id);
    if (!saved)
      return std::unexpected(saved.error());
    if (*saved) {
      if ((*saved)->body_encoded != data.encoded)
        return refuse(format_error_code::request_conflict, "Request parameters differ from saved request");
      if (!expected_ledger_identity)
        return acceptance_result{std::move((*saved)->result), true};
      // Publish the saved owner and replay evidence before the independent API result copy can allocate.
      receipt.saved_result = std::move((*saved)->result);
      receipt.phase        = ::planar::engine::ledger::completion_phase::replayed;
      return acceptance_result{receipt.saved_result, true};
    }
  }
  const auto format = tx->metadata().format;
  if ((format == ledger_format::v1 && data.version2) || (format == ledger_format::v2 && !data.version2))
    return refuse(format_error_code::unsupported_version, "Unsupported ledger format version");
  if (data.version2 && data.version2->profile_argument)
    return refuse(format_error_code::unsupported_state, "Unsupported profile acceptance");
  auto head = tx->read_head();
  if (!head)
    return std::unexpected(head.error());
  if (head->revision != data.expected_head)
    return refuse(format_error_code::revision_conflict, "Expected revision differs from current head",
                  types::revision_conflict_detail{data.expected_head, head->revision});
  auto prepared = candidate.data();
  if (!prepared)
    return refuse(format_error_code::invalid_argument, "Unavailable prepared acceptance");
  const auto& input = **prepared;
  if (input.workspace_id != tx->metadata().workspace_id)
    return refuse(format_error_code::invalid_argument, "Prepared workspace differs from ledger");
  auto source = tx->intern_tree(input.source, history::tree_profile::source);
  if (!source)
    return std::unexpected(source.error());
  auto registry = tx->intern_tree(input.registry.files, history::tree_profile::provenance);
  if (!registry)
    return std::unexpected(registry.error());
  auto stdlib = tx->intern_tree(input.stdlib.files, history::tree_profile::provenance);
  if (!stdlib)
    return std::unexpected(stdlib.error());
  auto interpretation = tx->intern_value(history::value_record{"canonical_v1", input.interpretation.encoded});
  if (!interpretation)
    return std::unexpected(interpretation.error());
  auto versions = tx->intern_value(history::value_record{"canonical_v1", input.interpretation.versions_encoded});
  if (!versions)
    return std::unexpected(versions.error());
  auto body = data.version2 ? tx->intern_request(types::stored_value{*data.version2}) : tx->intern_value(data.fields);
  if (!body)
    return std::unexpected(body.error());
  const event_references      refs{std::int64_t(source->id), std::int64_t(registry->id), std::int64_t(stdlib->id),
                                   std::int64_t(interpretation->id), std::int64_t(versions->id)};
  std::optional<revision_row> previous;
  if (head->revision) {
    auto row = tx->read_revision(std::uint64_t(*head->revision));
    if (!row)
      return std::unexpected(row.error());
    if (!*row)
      return refuse(format_error_code::corrupt, "Accepted history is inconsistent");
    previous = std::move(**row);
  }
  bool noop = false;
  if (previous) {
    auto same = tx->identical_content(previous->references, refs);
    if (!same)
      return std::unexpected(same.error());
    noop = *same;
  }
  auto rows = tx->append_acceptance_rows(
      acceptance_rows_input{*head, std::move(previous), refs, noop, data.request_id, data.fields.actor, *body});
  if (!rows)
    return std::unexpected(rows.error());
  acceptance_result response{std::move(rows->result), false};
  auto              verified = tx->verify_before_commit(commit_site::acceptance);
  if (!verified)
    return std::unexpected(verified.error());
  if (!candidate.verify())
    return refuse(format_error_code::capture_changed, "Capture changed before acceptance commit");
  if (!expected_ledger_identity) {
    auto committed = tx->commit(commit_site::acceptance);
    if (!committed)
      return std::unexpected(committed.error());
    return response;
  }
  // Allocate the second API owner before issuing COMMIT. The receipt remains authoritative if later error
  // construction or result conversion throws; publishing phases and moving owned strings never allocate.
  receipt.saved_result     = response.result;
  bool confirmed_noncommit = false;
  struct commit_evidence_guard {
    ::planar::engine::ledger::completion_receipt& receipt;
    const bool&                                   confirmed_noncommit;
    ~commit_evidence_guard() noexcept {
      if (confirmed_noncommit)
        receipt.phase = ::planar::engine::ledger::completion_phase::not_completed;
    }
  } evidence{receipt, confirmed_noncommit};
  receipt.phase  = ::planar::engine::ledger::completion_phase::commit_uncertain;
  auto committed = tx->commit(commit_site::acceptance, confirmed_noncommit);
  if (!committed)
    return std::unexpected(committed.error());
  receipt.phase = ::planar::engine::ledger::completion_phase::committed;
  return response;
}
} // namespace
std::expected<acceptance_result, format_error>
complete_acceptance(const std::filesystem::path& path, const ::planar::engine::acceptance::request_intent& intent,
                    const ::planar::engine::acceptance::prepared_acceptance& candidate) {
  ::planar::engine::ledger::completion_receipt receipt;
  return complete_acceptance_impl(path, intent, candidate, {}, receipt);
}
std::expected<acceptance_result, format_error>
complete_acceptance(const std::filesystem::path& path, const ::planar::engine::acceptance::request_intent& intent,
                    const ::planar::engine::acceptance::prepared_acceptance& candidate, std::string_view expected_ledger_identity,
                    ::planar::engine::ledger::completion_receipt& receipt) {
  return complete_acceptance_impl(path, intent, candidate, expected_ledger_identity, receipt);
}
} // namespace planar::engine::ledger
