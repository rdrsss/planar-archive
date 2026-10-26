/// @file acceptance_seam.cppm
/// @brief Internal partition: the native candidate abstraction and the one shared acceptance transaction body.
///
/// This is a non-exported interface partition of the acceptance module; the primary interface imports it without
/// re-exporting. Its non-exported entities (candidate_values, native_candidate, complete_native_candidate) sit in the nested
/// namespace seam_detail, which holds no exported or reachable entity, so argument-dependent lookup from the exported
/// acceptance_result cannot find complete_native_candidate. The guarantee is language visibility against misuse by trusted
/// in-tree code that imports the module. Known limits: ODR violations, defining another module's entities, bit_cast of
/// internal types, conversion-operator or template tricks that obtain unnamed internal types, sources outside src, a new
/// unit declared part of the module, and generated sources. The source scan in acceptance.t.cpp is the second check. The two
/// adapters are the only production callers. Errors use the shared format_error vocabulary; there is no shared state
/// (thread-confined per call).
export module planar.engine.ledger.acceptance:seam;
import std;
export import planar.engine.ledger.storage_types;
export import planar.engine.ledger.completion;
export import planar.engine.acceptance.prepared;
namespace planar::engine::ledger {
/// Exact committed response bytes, buffered until commit succeeds.
export struct acceptance_result {
  std::string result;   ///< Original canonical saved JSON bytes.
  bool        replayed; ///< True only for an exact previously committed request.
};
namespace seam_detail {
/// Internal seam: borrowed values a native candidate stores.
struct candidate_values {
  std::string_view                                      workspace_id; ///< Workspace identity compared with the ledger.
  std::span<const ::planar::engine::history::tree_file> source;       ///< Source tree files.
  std::span<const ::planar::engine::history::tree_file> registry;     ///< Registry tree files.
  std::span<const ::planar::engine::history::tree_file> stdlib;       ///< Stdlib tree files.
  bool                        stdlib_effective = false; ///< True interns the stdlib tree with the effective provenance limits.
  std::string_view            interpretation;           ///< Exact interpretation value bytes.
  std::string_view            versions;                 ///< Exact versions value bytes.
  std::shared_ptr<const void> keepalive;                ///< Owner keeping every borrowed view valid for the call.
};
/// Internal seam: abstract completion candidate.
class native_candidate {
public:
  virtual ~native_candidate() = default;
  /// @return True when the candidate completes a profile request.
  virtual bool selects_profile() const noexcept = 0;
  /// @brief Inspect the candidate after replay and CAS.
  /// @param request Bound request data to compare against.
  /// @return Borrowed values or a typed refusal.
  virtual std::expected<candidate_values, format_error>
  values(const ::planar::engine::acceptance::request_data& request) const = 0;
  /// @return True while every native capture receipt still matches.
  virtual bool verify() const = 0;
};
/// @brief Complete a native candidate through the shared acceptance transaction body (internal seam).
/// @param path Explicit existing ledger path.
/// @param intent Immutable validated request and expected revision.
/// @param candidate Native candidate inspected only after replay and CAS.
/// @param expected_ledger_identity Native prelude identity; empty selects the embedded no-identity path (no receipt evidence).
/// @param receipt Initially empty thread-confined output.
/// @return Exact saved result or typed refusal.
std::expected<acceptance_result, format_error>
complete_native_candidate(const std::filesystem::path& path, const ::planar::engine::acceptance::request_intent& intent,
                          const native_candidate& candidate, std::optional<std::string_view> expected_ledger_identity,
                          ::planar::engine::ledger::completion_receipt& receipt);
} // namespace seam_detail
} // namespace planar::engine::ledger
