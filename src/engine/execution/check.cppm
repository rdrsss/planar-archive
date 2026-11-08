/// @file
/// @brief Trusted check preparation, bounded execution, observations and boundary evidence; diagnostics redact host details.
export module planar.engine.execution.check;
import std;
export import planar.engine.value;
export import planar.engine.capture.native;
export import planar.engine.capture.tree_digest;
export import planar.engine.process.runner;
export namespace planar::engine::execution {
struct check_evidence {
  std::string                  work, check, outcome, reason;
  std::optional<std::uint64_t> matched;
  std::optional<int>           exit_status;
  std::uint64_t                duration_ms = 0, output_bytes = 0;
  bool                         output_truncated = false;
  std::string                  output;
  value_ptr                    definition, vector;
};
struct check_publication {
  std::string                work, facet, semantic;
  std::vector<captured_file> files;
};
struct check_run_result {
  std::vector<check_evidence>    evidence;
  bool                           recapture_matches = false;
  value_ptr                      observation;
  std::vector<check_publication> publications;
};
/// Native prepared authority. Move-only, thread-confined, owns descriptors through execution.
class prepared_check {
public:
  struct impl;
  std::unique_ptr<impl> state;
  prepared_check();
  ~prepared_check();
  prepared_check(prepared_check&&) noexcept;
  prepared_check& operator=(prepared_check&&) noexcept;
  prepared_check(const prepared_check&) = delete;
};
/// @brief Validate plan, capture roots and check delivered contracts and producer vectors before reservation.
/// @param plan Trusted check_plan_v1; additionally root, base (label), units entries {work,paths}, contracts bytes.
/// @param root Working root, base_root immutable comparison root, scratch disjoint existing scratch directory.
/// @return Owned captured authority, or a safe refusal. Callers validate plan against accepted registry first.
std::expected<prepared_check, diagnostic> prepare_check(const value_ptr& plan, const std::filesystem::path& root,
                                                        const std::filesystem::path& base_root,
                                                        const std::filesystem::path& scratch);
/// @brief Execute every required check, boundary gate and recapture using prepared descriptors.
/// @param prepared Owned preparation retained until call returns.
/// @param reservation Positive committed reservation identifier, used in scratch names.
/// @return Native facts for ledger commit, or I/O refusal leaving reservation open.
std::expected<check_run_result, diagnostic> run_checks(prepared_check& prepared, std::uint64_t reservation);
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
