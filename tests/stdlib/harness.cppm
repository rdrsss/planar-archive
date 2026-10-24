/// @file
/// @brief Test-only embedded exports exercising the trusted bridge without production script injection.
export module planar.engine.runtime_test;
import std;
export import planar.engine.value;
export namespace planar::engine::runtime_test {
/// @brief Run a fixed test export in a fresh bounded state.
/// @param entry Embedded test export name.
/// @param input Structural test data.
/// @return Converted result or sanitized failure.
std::expected<value_ptr, diagnostic> run_trusted(std::string_view entry, const value_ptr& input);
} // namespace planar::engine::runtime_test
