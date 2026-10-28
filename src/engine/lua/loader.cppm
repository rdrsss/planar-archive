/// @file
/// @brief Bounded text-only untrusted Lua loading into immutable structural values.
export module planar.engine.lua;
import std;
export import planar.engine.value;
import planar.engine.budget;
export namespace planar::engine {
/// Evaluated declaration plus unresolved references; no filesystem capability.
struct declaration {
  value_ptr                root;       ///< Private workspace, spec or job node.
  std::vector<std::string> references; ///< File reference paths in byte order.
};
/// @brief Evaluate a fresh restricted chunk with fixed contract resource limits.
/// @param source UTF-8 text, at most 1 MiB.
/// @param budget Optional factory-owned cumulative budget; null leaves behavior unchanged.
/// @return Immutable declaration or typed diagnostic. Independent calls share no state.
std::expected<declaration, diagnostic> load_declaration(std::string_view source, std::shared_ptr<budget_sink> budget = {});
} // namespace planar::engine
