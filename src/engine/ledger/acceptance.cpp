module planar.engine.ledger.acceptance;
import std;
import planar.engine.ledger.storage;
import :seam;
namespace planar::engine::ledger {
namespace {
/// Embedded adapter over a prepared acceptance.
class embedded_adapter final : public seam_detail::native_candidate {
public:
  /// @param candidate Prepared acceptance borrowed for the call.
  explicit embedded_adapter(const ::planar::engine::acceptance::prepared_acceptance& candidate) : _candidate(candidate) {
  }
  bool selects_profile() const noexcept override {
    return false;
  }
  std::expected<seam_detail::candidate_values, format_error>
  values(const ::planar::engine::acceptance::request_data&) const override {
    auto prepared = _candidate.data();
    if (!prepared)
      return std::unexpected<format_error>(
          format_error{format_error_code::invalid_argument, "Unavailable prepared acceptance", std::nullopt});
    const auto& input = **prepared;
    return seam_detail::candidate_values{input.workspace_id,
                                         input.source,
                                         input.registry.files,
                                         input.stdlib.files,
                                         false,
                                         input.interpretation.encoded,
                                         input.interpretation.versions_encoded,
                                         std::shared_ptr<const void>(*prepared)};
  }
  bool verify() const override {
    return _candidate.verify().has_value();
  }

private:
  const ::planar::engine::acceptance::prepared_acceptance& _candidate;
};
} // namespace
std::expected<acceptance_result, format_error>
complete_acceptance(const std::filesystem::path& path, const ::planar::engine::acceptance::request_intent& intent,
                    const ::planar::engine::acceptance::prepared_acceptance& candidate) {
  ::planar::engine::ledger::completion_receipt receipt;
  const embedded_adapter                       adapter{candidate};
  return seam_detail::complete_native_candidate(path, intent, adapter, std::nullopt, receipt);
}
std::expected<acceptance_result, format_error>
complete_acceptance(const std::filesystem::path& path, const ::planar::engine::acceptance::request_intent& intent,
                    const ::planar::engine::acceptance::prepared_acceptance& candidate, std::string_view expected_ledger_identity,
                    ::planar::engine::ledger::completion_receipt& receipt) {
  const embedded_adapter adapter{candidate};
  return seam_detail::complete_native_candidate(path, intent, adapter, expected_ledger_identity, receipt);
}
} // namespace planar::engine::ledger
