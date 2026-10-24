/// @file
/// @brief Pinned-Lua retained closure guard; typed refusals never execute inspected code.
module;
extern "C" {
#include "lua.h"
}
export module planar.engine.runtime.closure_guard;
import std;
export namespace planar::engine::closure_guard {
/// Fixed relative frame capacity: graph top 2040 plus two traversal slots and six spare slots.
inline constexpr int audit_stack_slots = 2048;
/// Closed failure categories for this retained closure guard.
enum class failure { sealed_write, unknown_native, c_root, resource, depth, invalid_metadata, unsupported_value };
/// Refusal with the zero-based original root position, absent before root selection.
struct root_failure {
  failure                    code; ///< Existing conservative refusal category.
  std::optional<std::size_t> root; ///< Selected original root; no inferred graph ownership.
};
/// Actual completed inspection counts, including compiler payload words.
struct report {
  std::size_t edges         = 0; ///< Graph edge visits including aliases and primitive leaves.
  std::size_t prototypes    = 0; ///< Completed or entered prototype inspections.
  std::size_t instructions  = 0; ///< Inspected instruction and payload words.
  std::size_t lua_closures  = 0; ///< Distinct reachable actual Lua closures.
  std::size_t cells         = 0; ///< Distinct retained upvalue cell identities.
  std::size_t loadkx        = 0; ///< Inspected LOADKX opcodes.
  std::size_t newtable      = 0; ///< Inspected NEWTABLE opcodes, including zero-k payloads.
  std::size_t setlist_extra = 0; ///< Inspected SETLIST opcodes with k set.
};
/// Capture exact curated C identities before module evaluation into a strong registry table.
/// @param l Protected Lua call's VM. No C++ destructor-bearing objects may cross this allocating call.
/// @param slot Private registry integer slot, distinct from runtime-owned slots.
void snapshot_curated_natives(lua_State* l, int slot);
/// Record a native callback immediately at its reviewed native installation site, never from module data.
/// @param l Protected Lua call's VM; the function at index must be native-installed.
/// @param slot Registry catalog created by snapshot_curated_natives.
/// @param index Stable absolute function stack index.
void remember_installed_callback(lua_State* l, int slot, int index);
/// Inspect retained tables/functions before proxy freezing; C roots always refuse.
/// No Lua allocation/metamethod/call occurs. The active C frame must permit relative top 2048;
/// at top zero, reserve audit_stack_slots before pushing the root. Insufficient capacity refuses
/// before native allocation or any Lua push; backing storage reserved by another frame is insufficient.
/// Native temporary allocation uses the caller cumulative resource; failure restores the original Lua top.
/// Calls are sequential on one VM. Pinned Lua private metadata is source-compiled only;
/// dependency upgrades must revalidate opcode payload and frame invariants.
/// @param l VM containing source-compiled closures and the private native catalog.
/// @param root Absolute index of retained Lua callable or graph table.
/// @param slot Private registry catalog slot.
/// @param resource Cumulative checked native allocation resource; never reset between roots.
/// @return Completed counts or a conservative refusal; no mutation of retained state.
std::expected<report, failure> audit_retained(lua_State* l, int root, int slot, std::pmr::memory_resource& resource);
/// Inspect ordered original roots with one identity/prototype memo before any table freezing.
/// Global roots must precede callable roots, preserving existing shared graph depth semantics.
/// The active-frame and source-compiled metadata requirements of audit_retained apply.
/// @param l VM containing each root at a stable absolute stack index.
/// @param roots Ordered global table and retained callable indices; C callable roots refuse.
/// @param slot Strong native identity catalog's private registry slot.
/// @param resource Shared cumulative allocation resource; allocations are never refunded.
/// @param sealed_slot Optional native-owned strong successful-seal set; zero disables atom shortcuts.
/// Entries may be published only after complete audit and freeze success, never by Lua code.
/// @return Combined checked inspection counts or a typed refusal, restoring the original top.
std::expected<report, failure> audit_roots(lua_State* l, std::span<const int> roots, int slot,
                                           std::pmr::memory_resource& resource, int sealed_slot = 0);

/// Inspect ordered roots with the same shared traversal as audit_roots and retain failure location.
/// @param l VM containing stable absolute root indices; active-frame requirements above apply.
/// @param roots Ordered roots, traversed once with one shared identity/prototype memo.
/// @param slot Strong native identity catalog slot.
/// @param resource Shared cumulative checked allocation resource.
/// @param sealed_slot Optional strong successful-seal identity set.
/// @return Inspection counts or refusal with the selected zero-based root, restoring stack top.
std::expected<report, root_failure> audit_roots_detailed(lua_State* l, std::span<const int> roots, int slot,
                                                         std::pmr::memory_resource& resource, int sealed_slot = 0);

} // namespace planar::engine::closure_guard
