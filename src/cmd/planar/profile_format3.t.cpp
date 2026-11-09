// Format3 profile preparation retains one active frame and freezes reconciliation before cleanup.
#include <catch2/catch_test_macros.hpp>
#include <stdlib.h>
#include <unistd.h>
import std;
import planar.engine.runtime;
import planar.engine.acceptance.intent;
import planar.engine.ledger.execution;
import planar.engine.ledger.execution_schema;
import planar.engine.execution.snapshot_value;
import planar.engine.canonical;

namespace {
namespace fs = std::filesystem;
using namespace planar::engine;
struct profile_scratch {
  fs::path root;
  profile_scratch() {
    char pattern[] = "/tmp/planar-profile-format3-XXXXXX";
    auto made      = ::mkdtemp(pattern);
    REQUIRE(made);
    root = fs::canonical(made);
    fs::copy(fs::path(PLANAR_B2_WORKSPACE_FIXTURE).parent_path().parent_path(), root / "quota", fs::copy_options::recursive);
    fs::create_directory(root / "trusted");
    std::ofstream(root / "trusted/profile.json") << R"({"version":1,"modules":[]})";
    REQUIRE(ledger::initialize_format3(root / "ledger.db", "quota"));
  }
  ~profile_scratch() {
    std::error_code ignored;
    fs::remove_all(root, ignored);
  }
  acceptance::request_intent intent() const {
    auto value = acceptance::request_intent::create_v2(
        "first", "test", std::nullopt, (root / "quota/workspace/workspace.lua").string(),
        (root / "quota/registry/quota-tools.json").string(), (root / "trusted/profile.json").string());
    REQUIRE(value);
    return std::move(*value);
  }
};
} // namespace

TEST_CASE("C04 profile preparation freezes copied snapshot and reconcile in one frame", "[profile][c04]") {
  profile_scratch fixture;
  auto            intent   = fixture.intent();
  auto            snapshot = ledger::read_execution_snapshot(fixture.root / "ledger.db");
  REQUIRE(snapshot);
  auto encoded = execution::snapshot_value(*snapshot);
  REQUIRE(encoded);
  auto mutable_input                = std::make_shared<value>(**encoded);
  auto mutable_snapshot             = std::make_shared<value>(*mutable_input->fields.at("snapshot"));
  mutable_input->fields["snapshot"] = mutable_snapshot;
  auto original                     = canonical_encode(mutable_input);
  REQUIRE(original);
  auto owner = profile_invocation::open_with_profile(intent, (fixture.root / "ledger.db").string());
  REQUIRE(owner);
  const auto before   = owner->observe();
  auto       prepared = std::move(*owner).prepare_execution_load(intent, mutable_input);
  INFO((prepared ? "prepared" : prepared.error().failure.detail.message));
  REQUIRE(prepared);
  auto data = prepared->data();
  REQUIRE(data);
  REQUIRE((*data)->execution_change);
  REQUIRE((*data)->execution_snapshot);
  CHECK((*data)->execution_snapshot != mutable_input);
  mutable_snapshot->fields.clear();
  mutable_input->fields.clear();
  auto frozen = canonical_encode((*data)->execution_snapshot);
  REQUIRE(frozen);
  CHECK(*frozen == *original);
  auto observation = prepared->observe();
  CHECK(observation.registrations == 1);
  CHECK(observation.frame_begun);
  CHECK(observation.cleanup_ran);
  CHECK(observation.temporary_entries == 0);
  CHECK(observation.native_bytes >= before.native_bytes);
  CHECK(observation.instructions >= before.instructions);
  CHECK(prepared->verify_native_receipts());
  CHECK_FALSE(std::move(*owner).prepare_load(intent));
  // Native receipt verification never enters Lua or mutates the cumulative counters.
  const auto after = prepared->observe();
  CHECK(after.native_bytes == observation.native_bytes);
  CHECK(after.instructions == observation.instructions);
}

TEST_CASE("C04 profile preparation without execution snapshot retains its format2 shape", "[profile][c04]") {
  profile_scratch fixture;
  auto            intent = fixture.intent();
  auto            owner  = profile_invocation::open_with_profile(intent, (fixture.root / "ledger.db").string());
  REQUIRE(owner);
  auto prepared = std::move(*owner).prepare_load(intent);
  REQUIRE(prepared);
  auto data = prepared->data();
  REQUIRE(data);
  CHECK_FALSE((*data)->execution_snapshot);
  CHECK_FALSE((*data)->execution_change);
  CHECK(prepared->observe().registrations == 1);
  CHECK(prepared->observe().cleanup_ran);
}

TEST_CASE("C05 execution profile preparation refuses absent and malformed snapshots", "[profile][c05]") {
  profile_scratch fixture;
  auto            intent = fixture.intent();
  value_ptr       input;
  SECTION("null is not format2") {
  }
  SECTION("empty value is not an execution snapshot") {
    input = std::make_shared<value>();
  }
  SECTION("missing snapshot fields refuse") {
    auto snapshot = ledger::read_execution_snapshot(fixture.root / "ledger.db");
    REQUIRE(snapshot);
    auto encoded = execution::snapshot_value(*snapshot);
    REQUIRE(encoded);
    auto malformed = std::make_shared<value>(**encoded);
    malformed->fields.clear();
    input = malformed;
  }
  auto owner = profile_invocation::open_with_profile(intent, (fixture.root / "ledger.db").string());
  REQUIRE(owner);
  auto prepared = std::move(*owner).prepare_execution_load(intent, input);
  REQUIRE_FALSE(prepared);
  CHECK_FALSE(owner->observe().live);
  CHECK_FALSE(std::move(*owner).prepare_load(intent));
}
