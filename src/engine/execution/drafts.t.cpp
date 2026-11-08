// Draft comparison of status --workspace --registry (R15-D P1, DD-D6): changed, absent and added paths, sorted by
// unsigned bytes, with mode differences counted as changes. Pure function; no temporary roots are needed.
#include <catch2/catch_test_macros.hpp>
import std;
import planar.engine.execution.drafts;

namespace {
using planar::engine::history::tree_file;
using planar::engine::execution::changed_paths;
} // namespace

TEST_CASE("R15-D drafts identical trees have no changed paths", "[drafts]") {
  const std::vector<tree_file> recorded{{"spec.lua", "a", 420}, {"text/shared.md", "b", 420}};
  const std::vector<tree_file> captured{{"text/shared.md", "b", 420}, {"spec.lua", "a", 420}};
  CHECK(changed_paths(captured, recorded).empty());
}

TEST_CASE("R15-D drafts report differing, absent and added paths sorted", "[drafts]") {
  const std::vector<tree_file> recorded{
      {"workspace.lua", "w", 420}, {"spec.lua", "a", 420}, {"text/old.md", "gone", 420}, {"tool.sh", "x", 493}};
  const std::vector<tree_file> captured{
      {"workspace.lua", "w", 420}, {"spec.lua", "A", 420}, {"text/new.md", "new", 420}, {"tool.sh", "x", 420}};
  const std::vector<std::string> expected{"spec.lua", "text/new.md", "text/old.md", "tool.sh"};
  CHECK(changed_paths(captured, recorded) == expected);
}

TEST_CASE("R15-D drafts order paths by unsigned bytes", "[drafts]") {
  const std::vector<tree_file> recorded{};
  const std::vector<tree_file> captured{{"\xc3\xa9.md", "e", 420}, {"Z.md", "z", 420}, {"a.md", "a", 420}};
  const std::vector<std::string> expected{"Z.md", "a.md", "\xc3\xa9.md"};
  CHECK(changed_paths(captured, recorded) == expected);
}

TEST_CASE("R15-D drafts empty capture lists every recorded path once", "[drafts]") {
  const std::vector<tree_file> recorded{{"b", "1", 420}, {"a", "2", 420}};
  const std::vector<std::string> expected{"a", "b"};
  CHECK(changed_paths({}, recorded) == expected);
}
