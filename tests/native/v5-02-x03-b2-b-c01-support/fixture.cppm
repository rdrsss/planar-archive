/// @file fixture.cppm
/// @brief Thread-confined test control supplying one verified metadata format difference without altering SQL verification.
export module planar.test.metadata_fixture;
import std;
export namespace planar::test::metadata_fixture {
/// The three production metadata comparison boundaries, or the unchanged control.
enum class boundary { none, begin, writer_commit, acceptance_commit };
/// Thread-confined selected boundary. Tests run serially within this executable.
inline boundary selected = boundary::none;
/// Number of supplied differences; reset by each test scenario.
inline unsigned supplied = 0;
/// @brief Preserve verification failure and all metadata except the selected supported format.
/// @param result Actual SQL verification result, owned by the caller.
/// @param point Exact boundary about to compare this result.
/// @return Original verified result, with only format toggled at the selected boundary.
template <class Result> Result supply(Result result, boundary point) {
  if (result && selected == point) {
    result->format = static_cast<decltype(result->format)>(static_cast<int>(result->format) == 1 ? 2 : 1);
    ++supplied;
  }
  return result;
}
} // namespace planar::test::metadata_fixture
