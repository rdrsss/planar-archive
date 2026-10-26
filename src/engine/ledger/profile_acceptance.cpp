module planar.engine.ledger.acceptance;
import std;
import planar.engine.runtime;
import :seam;
namespace planar::engine::ledger {
namespace {
/// Adapter over an opaque prepared profile: reads only immutable derived data and repeats native receipt checks; runs no Lua.
class profile_adapter final : public seam_detail::native_candidate {
public:
  /// @param profile Prepared profile borrowed for the call.
  explicit profile_adapter(const ::planar::engine::prepared_profile& profile) : _profile(profile) {
  }
  bool selects_profile() const noexcept override {
    return true;
  }
  std::expected<seam_detail::candidate_values, format_error>
  values(const ::planar::engine::acceptance::request_data& request) const override {
    auto derived = _profile.data();
    if (!derived || !*derived)
      return std::unexpected<format_error>(
          format_error{format_error_code::invalid_argument, "Unavailable prepared acceptance", std::nullopt});
    const auto& x = **derived;
    if (!x.bound_intent || x.bound_intent->request_id != request.request_id || x.bound_intent->encoded != request.encoded)
      return std::unexpected<format_error>(
          format_error{format_error_code::invalid_argument, "Prepared profile differs from request intent", std::nullopt});
    return seam_detail::candidate_values{x.workspace_id,
                                         x.source,
                                         x.registry.files,
                                         x.effective.files,
                                         true,
                                         x.interpretation.encoded,
                                         x.interpretation.versions_encoded,
                                         std::shared_ptr<const void>(*derived)};
  }
  bool verify() const override {
    return _profile.verify_native_receipts().has_value();
  }

private:
  const ::planar::engine::prepared_profile& _profile;
};
} // namespace
std::expected<acceptance_result, format_error> complete_acceptance(const std::filesystem::path&                        path,
                                                                   const ::planar::engine::acceptance::request_intent& intent,
                                                                   const ::planar::engine::prepared_profile&           profile,
                                                                   std::string_view expected_ledger_identity,
                                                                   ::planar::engine::ledger::completion_receipt& receipt) {
  const profile_adapter adapter{profile};
  return seam_detail::complete_native_candidate(path, intent, adapter, expected_ledger_identity, receipt);
}
} // namespace planar::engine::ledger
