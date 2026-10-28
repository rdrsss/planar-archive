/// @file
/// @brief Factory-owned cumulative resource budget interface shared by capture and Lua loading.
export module planar.engine.budget;
import std;
export import planar.engine.value;
export namespace planar::engine {
/// @brief Single-thread-confined, factory-owned cumulative resource budget.
///
/// Each charge is a checked addition made BEFORE the allocation or copy it
/// accounts for. A refusal returns the owner's resource_limit diagnostic.
/// Charges are never refunded; acquire/release tracks live Lua memory only.
/// Not thread-safe: all calls come from the owning thread.
class budget_sink {
public:
  virtual ~budget_sink() = default;
  /// @brief Charge retained native bytes.
  /// @param bytes Bytes about to be allocated or copied.
  /// @return Success or the owner's resource_limit diagnostic.
  virtual std::expected<void, diagnostic> charge_native(std::size_t bytes) = 0;
  /// @brief Charge Lua VM instructions.
  /// @param count Instructions executed since the previous charge.
  /// @return Success or the owner's resource_limit diagnostic.
  virtual std::expected<void, diagnostic> charge_instructions(std::size_t count) = 0;
  /// @brief Acquire live Lua allocator bytes.
  /// @param bytes Bytes about to be allocated.
  /// @return Success or the owner's resource_limit diagnostic.
  virtual std::expected<void, diagnostic> acquire_lua(std::size_t bytes) = 0;
  /// @brief Release live Lua allocator bytes previously acquired.
  /// @param bytes Bytes freed or shrunk.
  virtual void release_lua(std::size_t bytes) noexcept = 0;
};
} // namespace planar::engine
