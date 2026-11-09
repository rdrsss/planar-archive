/// @file
/// @brief Trusted check preparation, bounded execution, observations and boundary evidence; diagnostics redact host details.
export module planar.engine.execution.check;
import std;
export import planar.engine.value;
export import planar.engine.capture.native;
export import planar.engine.capture.tree_digest;
export import planar.engine.process.runner;
export namespace planar::engine::execution {
/// Check-owned refusal context; generic capture/resource diagnostics remain unchanged.
struct check_error : diagnostic {
  std::string field;                     ///< Approved semantic field path, or empty.
  std::string work;                      ///< First mismatching producer work, only for observation_target_mismatch.
  bool        contract_delivery = false; ///< Delivery conflict; command adapter supplies the verified active revision.
  /// Wrap a lower-level refusal without changing its code, message or resource details.
  /// @param error Safe diagnostic from the owning capture/process boundary.
  check_error(diagnostic error) : diagnostic(std::move(error)) {
  }
};
/// Recorded outcome of one required check or boundary gate; contains no run secret.
struct check_evidence {
  std::string                  work;                     ///< Work unit identifier.
  std::string                  check;                    ///< Required check or built-in gate identifier.
  std::string                  outcome;                  ///< pass, fail or inconclusive.
  std::string                  reason;                   ///< Closed reason corresponding to the outcome.
  std::optional<std::uint64_t> matched;                  ///< Selected case count when supplied by a valid result.
  std::optional<int>           exit_status;              ///< Child exit status when available.
  std::uint64_t                duration_ms      = 0;     ///< Observed process duration.
  std::uint64_t                output_bytes     = 0;     ///< Total captured output length, including any unretained suffix.
  bool                         output_truncated = false; ///< Whether retained output omits a suffix.
  std::string                  output;                   ///< Retained merged process output bytes.
  value_ptr                    definition;               ///< Canonical check definition used for execution.
  value_ptr                    vector;                   ///< Captured output-vector identities used by the check.
};
/// Captured facet payload eligible for publication only with a completion.
struct check_publication {
  std::string                work;     ///< Owning work unit.
  std::string                facet;    ///< Declared facet identifier.
  std::string                semantic; ///< Captured facet semantic digest.
  std::vector<captured_file> files;    ///< Exact file payload and modes.
};
/// Immutable facts returned after executing checks and recapturing inputs.
struct check_run_result {
  std::vector<check_evidence>    evidence;                  ///< Ordered outcomes for checks and boundary gates.
  bool                           recapture_matches = false; ///< Whether all checked input identities remained stable.
  value_ptr                      observation;               ///< check_observation_v1 captured before process execution.
  std::vector<check_publication> publications;              ///< Facet payloads for a possible completion.
};
/// Native prepared authority. Move-only, thread-confined, owns descriptors through execution.
class prepared_check {
public:
  /// Descriptor and capture ownership retained through execution.
  struct impl;
  std::unique_ptr<impl> state; ///< Invocation-local owned preparation; not shared between threads.
  /// Construct empty preparation; prepare_check populates the state.
  prepared_check();
  /// Close all retained descriptors and release capture memory.
  ~prepared_check();
  /// Transfer preparation ownership.
  /// @param other Preparation relinquishing its resources.
  prepared_check(prepared_check&& other) noexcept;
  /// Replace preparation by moving resources.
  /// @param other Preparation relinquishing its resources.
  /// @return This preparation.
  prepared_check& operator=(prepared_check&& other) noexcept;
  /// Copying descriptor ownership is forbidden.
  prepared_check(const prepared_check&) = delete;
};
/// @brief Validate plan, capture roots and check delivered contracts and producer vectors before reservation.
/// @param plan Trusted check_plan_v1; additionally root, base (label), units entries {work,paths}, contracts bytes.
/// @param root Working checkout directory.
/// @param base_root Immutable comparison checkout directory.
/// @param scratch Disjoint existing scratch directory.
/// @return Owned captured authority, or a safe refusal. Callers validate plan against accepted registry first.
std::expected<prepared_check, check_error> prepare_check(const value_ptr& plan, const std::filesystem::path& root,
                                                         const std::filesystem::path& base_root,
                                                         const std::filesystem::path& scratch);
/// @brief Execute every required check, boundary gate and recapture using prepared descriptors.
/// @param prepared Owned preparation retained until call returns.
/// @param reservation Positive committed reservation identifier, used in scratch names.
/// @return Native facts for ledger commit, or I/O refusal leaving reservation open.
std::expected<check_run_result, check_error> run_checks(prepared_check& prepared, std::uint64_t reservation);
/// @brief Return the prepared check_observation_v1 before reservation.
/// @param prepared Successfully prepared authority.
/// @return Immutable observation with own and producer unit outputs.
value_ptr check_observation(const prepared_check& prepared);
/// @brief Classify a completed child and strict result JSON according to R17-3 precedence.
/// @param result Process outcome.
/// @param selection nonempty or not_applicable.
/// @param result_json Captured result bytes, absent when unreadable.
/// @return Evidence outcome, reason and process metrics, without work/check identities.
check_evidence classify(const process::run_result& result, std::string_view selection,
                        std::optional<std::string_view> result_json);
/// @brief Boundary changes, classified frozen, own, producer or outside, and gate pass flag.
/// @param plan Prepared policy, with producer output/completion_output equality established.
/// @param base Original digest tree.
/// @param current Working digest tree.
/// @return Pair of pass flag and boundary_diff_v1 value.
std::pair<bool, value_ptr> boundary_gate(const value_ptr& plan, const capture::tree_digest& base,
                                         const capture::tree_digest& current);
} // namespace planar::engine::execution
