/// @file
/// @brief Test-only pinned-Lua lexical write audit; typed refusals never execute inspected code.
module;
extern "C" {
#include "lua.h"
}
export module planar.engine.x05_closure_audit;
import std;
export namespace planar::engine::x05_closure_audit {
/// Independent feasibility ceilings; each audit owns all state and is not concurrently reentrant on one VM.
struct limits {
  std::size_t edges        = 1000000;         ///< Traversed graph edges including aliases and primitive leaves.
  std::size_t prototypes   = 100000;          ///< Prototype inspections, including repeats under different root masks.
  std::size_t instructions = 10000000;        ///< Inspected words, including EXTRAARG payloads.
  std::size_t depth        = 256;             ///< Separate graph and prototype recursion ceilings.
  std::size_t native_bytes = 4 * 1024 * 1024; ///< Fixed native arena bytes, with no fallback allocation.
};
/// Fixed relative frame capacity: graph top 2040 plus two traversal slots and six spare slots.
inline constexpr int audit_stack_slots = 2048;
/// Closed failure categories for this verification-only experiment.
enum class failure { sealed_write, unknown_native, c_root, resource, invalid_metadata, unsupported_value };
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
/// Native temporary storage is bounded by a fixed arena; failure restores the original Lua top.
/// @param l VM containing source-compiled closures and the private native catalog.
/// @param root Absolute index of retained Lua callable or graph table.
/// @param slot Private registry catalog slot.
/// @param bound Checked native work and arena ceilings.
/// @return Completed counts or a conservative refusal; no mutation of retained state.
std::expected<report, failure> audit_retained(lua_State* l, int root, int slot, limits bound = {});
} // namespace planar::engine::x05_closure_audit
