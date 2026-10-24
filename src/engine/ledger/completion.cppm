/// @file
/// @brief Dependency-free native load completion receipt shared by VM, adapter and transaction boundaries.
export module planar.engine.ledger.completion;
import std;
export namespace planar::engine::ledger {
/// Authoritative native phase; uncertain completion must never imply rollback.
enum class completion_phase { not_completed, committed, replayed, commit_uncertain };
/// Thread-confined native output, never exposed to Lua or accepted from callers.
/// Allocate saved_result before COMMIT, then change phase without allocation.
/// Moving owned bytes with the standard allocator and assigning phase are noexcept;
/// keep this owner alive through all conversion, handler and transport failures.
struct completion_receipt {
  completion_phase phase = completion_phase::not_completed; ///< Native completion phase.
  std::string      saved_result;                            ///< Exact saved canonical JSON, with no terminal LF.
};
static_assert(std::is_nothrow_move_assignable_v<completion_receipt>);
} // namespace planar::engine::ledger
