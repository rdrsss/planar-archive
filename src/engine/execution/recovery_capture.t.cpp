// Recovery output capture observes exact role boundaries without writing ledger state.
#include <catch2/catch_test_macros.hpp>
#include <stdlib.h>
import std;
import planar.engine.execution.recovery_capture;
namespace {
using namespace planar::engine;
value_ptr str(std::string s) {
  value v{};
  v.kind   = value_kind::string;
  v.string = std::move(s);
  return std::make_shared<const value>(std::move(v));
}
value_ptr list(std::vector<value_ptr> x) {
  value v{};
  v.kind  = value_kind::list;
  v.items = std::move(x);
  return std::make_shared<const value>(std::move(v));
}
value_ptr obj(std::map<std::string, value_ptr> x) {
  value v{};
  v.kind   = value_kind::record;
  v.fields = std::move(x);
  return std::make_shared<const value>(std::move(v));
}
struct scratch {
  std::filesystem::path root;
  scratch() {
    char  p[]  = "/tmp/planar-recovery-capture-XXXXXX";
    auto* made = mkdtemp(p);
    REQUIRE(made);
    root = std::filesystem::canonical(made);
    std::filesystem::create_directories(root / "current/src");
    std::filesystem::create_directories(root / "base/src");
  }
  ~scratch() {
    std::error_code e;
    std::filesystem::remove_all(root, e);
  }
  void write(std::string name, std::string bytes) {
    std::ofstream f(root / name, std::ios::binary);
    f << bytes;
    REQUIRE(f.good());
  }
};
ledger::execution_snapshot state() {
  ledger::execution_snapshot s;
  s.active_job = "job";
  s.revision.emplace();
  s.revision->interpretation_value =
      obj({{"bundle",
            obj({{"workspace", obj({{"root", obj({{"id", str("source")}, {"base", str("baseline")}, {"excluded", list({})}})}})},
                 {"spec", obj({{"work", list({})}})}})}});
  ledger::execution_role r;
  r.job                 = "job";
  r.role                = "worker";
  r.structure.kind      = "worker";
  r.structure.work      = {"unit"};
  r.structure.ownership = {{"unit", {"src/"}}};
  s.roles.push_back(r);
  return s;
}
std::string output(const value_ptr& v) {
  return v->fields.at("units")->items.at(0)->fields.at("output")->string;
}
} // namespace
TEST_CASE("R16 recovery capture separates owned output from unrelated current files") {
  scratch t;
  t.write("base/src/a", "before");
  t.write("current/src/a", "after");
  auto s     = state();
  auto first = execution::capture_recovery_outputs(s, "worker", "source=" + (t.root / "current").string(),
                                                   "source=" + (t.root / "base").string());
  REQUIRE(first);
  t.write("current/unrelated", "outside");
  auto unrelated = execution::capture_recovery_outputs(s, "worker", "source=" + (t.root / "current").string(),
                                                       "source=" + (t.root / "base").string());
  REQUIRE(unrelated);
  CHECK(output(*first) == output(*unrelated));
  t.write("current/src/a", "new output");
  auto changed = execution::capture_recovery_outputs(s, "worker", "source=" + (t.root / "current").string(),
                                                     "source=" + (t.root / "base").string());
  REQUIRE(changed);
  CHECK(output(*changed) != output(*first));
  CHECK((*changed)->fields.at("units")->items.at(0)->fields.at("present")->boolean);
}
TEST_CASE("R16 recovery capture refuses wrong logical roots and observes absent outputs") {
  scratch t;
  auto    s   = state();
  auto    bad = execution::capture_recovery_outputs(s, "worker", "wrong=" + (t.root / "current").string(),
                                                    "source=" + (t.root / "base").string());
  REQUIRE_FALSE(bad);
  CHECK(bad.error().code == "reference");
  auto empty = execution::capture_recovery_outputs(s, "worker", "source=" + (t.root / "current").string(),
                                                   "source=" + (t.root / "base").string());
  REQUIRE(empty);
  CHECK_FALSE((*empty)->fields.at("units")->items.at(0)->fields.at("present")->boolean);
}
