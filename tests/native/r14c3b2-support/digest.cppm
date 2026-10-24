/// @file
/// @brief Test-only selective digest collisions retaining real hashes for every other record kind.
export module planar.test.atomic_digest;
import std;
import planar.engine.history.record;
import planar.engine.history.request_v2;
export namespace planar::test::atomic_digest {
/// Target kind for one isolated thread-confined test; zero retains all real digests.
inline thread_local int target_kind = 0;
/// @brief Compute the actual codec hash except for the one deliberately selected collision kind.
/// @param value Locally validated original record.
/// @return Real digest, forced kind digest or unchanged codec failure.
std::expected<engine::history::record_digest, engine::history::record_error> digest(const engine::history::record& value) {
  auto actual = engine::history::digest_record(value);
  if (!actual || static_cast<int>(engine::history::kind_of(value)) != target_kind)
    return actual;
  engine::history::record_digest forced{};
  forced[0] = static_cast<std::uint8_t>(target_kind);
  return forced;
}
/// @brief Apply the same selected request collision to both codec2 read and write paths.
/// @param value Independently framed exact request.
/// @return Real complete-frame digest unless request kind is selected.
std::expected<engine::history::record_digest, engine::history::record_error> digest_v2(const engine::history::request_v2& value) {
  auto actual = engine::history::digest_request_v2(value);
  if (!actual || target_kind != 4)
    return actual;
  engine::history::record_digest forced{};
  forced[0] = 4;
  return forced;
}
} // namespace planar::test::atomic_digest
