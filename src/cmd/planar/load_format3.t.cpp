// Format3 load reconciliation through the real CLI entry (R15-C P3): early replay, noop, structural change, removed and
// re-added roles, absent active job (D94), and the unchanged format1 and format2 paths. Every ledger and workspace copy
// lives under a per-test mkdtemp root; ledgers are created with initialize_format3 directly (the init cutover is P4).
#include <catch2/catch_test_macros.hpp>
#include <stdlib.h>
#include <unistd.h>
import std;
import planar.cmd.bootstrap;
import planar.engine.json_codec;
import planar.engine.ledger.execution;
import planar.engine.ledger.execution_schema;
import planar.engine.ledger.format;

namespace {
namespace fs = std::filesystem;
using planar::engine::value_kind;
using planar::engine::value_ptr;

struct scratch {
  fs::path root;
  scratch() {
    char  pattern[] = "/tmp/planar-load-format3-XXXXXX";
    char* made      = ::mkdtemp(pattern);
    REQUIRE(made != nullptr);
    root = fs::canonical(made);
    fs::copy(fs::path(PLANAR_B2_WORKSPACE_FIXTURE).parent_path().parent_path(), root / "quota", fs::copy_options::recursive);
  }
  ~scratch() {
    std::error_code ignored;
    fs::remove_all(root, ignored);
  }
  fs::path ledger() const {
    return root / "ledger.planar";
  }
  std::string workspace() const {
    return (root / "quota/workspace/workspace.lua").string();
  }
  std::string registry() const {
    return (root / "quota/registry/quota-tools.json").string();
  }
  void write(std::string_view relative, std::string_view text) const {
    std::ofstream out(root / "quota/workspace" / relative, std::ios::binary | std::ios::trunc);
    out << text;
  }
};

constexpr std::string_view split_v1 = R"(local p = planar
local L = p.list
return p.job {
  version = 1, id = "split", shape = "fanout_review", budget_bytes = 131072,
  roles = L {
    { id = "library_worker", kind = "worker", work = L { "library" } },
    { id = "cli_worker", kind = "worker", work = L { "command" } },
    { id = "reviewer", kind = "reviewer", targets = L { "library", "command" } }
  }
}
)";

// library_worker takes both units (structural change) and cli_worker is removed (retired).
constexpr std::string_view split_v2 = R"(local p = planar
local L = p.list
return p.job {
  version = 1, id = "split", shape = "fanout_review", budget_bytes = 131072,
  roles = L {
    { id = "library_worker", kind = "worker", work = L { "library", "command" } },
    { id = "reviewer", kind = "reviewer", targets = L { "library", "command" } }
  }
}
)";

constexpr std::string_view workspace_split_only = R"(local p = planar
local L = p.list
return p.workspace {
  version = 1, id = "quota",
  root = { id = "source", base = "initial", excluded = L { "generated/" } },
  spec = "spec.lua",
  jobs = L { { id = "solo", file = "solo.lua" }, { id = "split", file = "split.lua" } },
  registry = { id = "quota_tools", revision = "v1" },
  toolchains = L { "python" }, checks = L { "contract", "integration" },
  notes = p.immaterial(p.md "The driver supplies stable sibling source snapshots.")
}
)";

// A second job keeps the job list at two entries: a one-job workspace currently trips the pre-existing acceptance.lua
// sorted() retagging of a single-entry list (reported in the round note, not fixed here).
constexpr std::string_view solo_job = R"(local p = planar
local L = p.list
return p.job {
  version = 1, id = "solo", shape = "fanout_review", budget_bytes = 131072,
  roles = L {
    { id = "builder", kind = "worker", work = L { "library", "command" } },
    { id = "reviewer", kind = "reviewer", targets = L { "library", "command" } }
  }
}
)";

struct outcome {
  int         status = 0;
  std::string output;
  std::string error;
  value_ptr   value;
};

outcome run(const std::vector<std::string>& args) {
  std::vector<std::string_view> views(args.begin(), args.end());
  auto                          result = planar::cmd::invoke(views);
  outcome                       out{result.status, result.output, result.error, nullptr};
  const auto&                   text = result.status == 0 ? result.output : result.error;
  if (!text.empty()) {
    auto decoded = planar::engine::codec_decode(std::string_view(text).substr(0, text.size() - 1));
    if (decoded)
      out.value = *decoded;
  }
  return out;
}

std::string field(const value_ptr& node, std::string_view key) {
  REQUIRE(node);
  REQUIRE(node->kind == value_kind::record);
  auto found = node->fields.find(std::string(key));
  REQUIRE(found != node->fields.end());
  REQUIRE(found->second->kind == value_kind::string);
  return found->second->string;
}

std::string error_code(const outcome& result) {
  REQUIRE(result.value);
  return field(result.value->fields.at("error"), "code");
}

outcome load(const scratch& s, std::string_view head, std::string_view request) {
  return run({"load", "--ledger", s.ledger().string(), "--workspace", s.workspace(), "--registry", s.registry(),
              "--expect-revision", std::string(head), "--request-id", std::string(request), "--actor", "tester"});
}

outcome activate(const scratch& s, std::string_view job, std::string_view head, std::string_view expect_job,
                 std::string_view request) {
  return run({"activate", "--ledger", s.ledger().string(), "--job", std::string(job), "--expect-revision", std::string(head),
              "--expect-job", std::string(expect_job), "--request-id", std::string(request), "--actor", "tester"});
}

planar::engine::ledger::execution_snapshot snapshot(const scratch& s) {
  auto read = planar::engine::ledger::read_execution_snapshot(s.ledger());
  INFO("snapshot read failed with code " << (read ? -1 : static_cast<int>(read.error().code)));
  REQUIRE(read);
  return *read;
}

std::string head(const scratch& s) {
  auto read = snapshot(s);
  return read.revision ? std::to_string(read.revision->id) : std::string("none");
}

const planar::engine::ledger::execution_role* role(const planar::engine::ledger::execution_snapshot& state,
                                                   std::string_view job, std::string_view name) {
  for (const auto& row : state.roles)
    if (row.job == job && row.role == name)
      return &row;
  return nullptr;
}

/// A format3 ledger with one accepted revision of the shipped fixture.
void seeded(const scratch& s) {
  REQUIRE(planar::engine::ledger::initialize_format3(s.ledger(), "quota"));
  const auto first = load(s, "none", "seed");
  INFO(first.error);
  REQUIRE(first.status == 0);
}

} // namespace

TEST_CASE("format3 load without an active job accepts, replays and conflicts", "[load_format3]") {
  scratch s;
  REQUIRE(planar::engine::ledger::initialize_format3(s.ledger(), "quota"));
  const auto first = load(s, "none", "first");
  INFO(first.error);
  REQUIRE(first.status == 0);
  CHECK(field(first.value, "kind") == "load");
  CHECK(field(first.value, "outcome") == "accepted");
  CHECK(field(first.value, "parent") == "none");
  const auto state = snapshot(s);
  REQUIRE(state.revision);
  CHECK(std::to_string(state.revision->id) == field(first.value, "revision"));
  CHECK(state.active_job.empty());
  CHECK(state.roles.empty());

  // Same ID and body: the saved bytes, with no new event.
  const auto replay = load(s, "none", "first");
  REQUIRE(replay.status == 0);
  CHECK(replay.output == first.output);
  CHECK(snapshot(s).last_seq == state.last_seq);

  // Same ID with another body (expected head) is request_conflict before capture.
  const auto conflict = load(s, head(s), "first");
  CHECK(conflict.status == 3);
  CHECK(error_code(conflict) == "request_conflict");
}

TEST_CASE("format3 noop load writes load_noop without transitions", "[load_format3]") {
  scratch s;
  seeded(s);
  REQUIRE(activate(s, "combined", head(s), "NONE", "act").status == 0);
  const auto before = snapshot(s);
  const auto noop   = load(s, head(s), "again");
  INFO(noop.error);
  REQUIRE(noop.status == 0);
  CHECK(field(noop.value, "outcome") == "noop");
  const auto after = snapshot(s);
  CHECK(after.last_seq == before.last_seq + 1);
  CHECK(after.revision->id == before.revision->id);
  REQUIRE(after.roles.size() == before.roles.size());
  for (std::size_t i = 0; i < after.roles.size(); ++i) {
    CHECK(after.roles[i].state == before.roles[i].state);
    CHECK(after.roles[i].fact_version == before.roles[i].fact_version);
    CHECK(after.roles[i].generation_ordinal == before.roles[i].generation_ordinal);
  }
}

TEST_CASE("format3 load reconciles structural change, removal and re-addition in the active job", "[load_format3]") {
  scratch s;
  s.write("split.lua", split_v1);
  seeded(s);
  const auto activated = activate(s, "split", head(s), "NONE", "act");
  INFO(activated.error);
  REQUIRE(activated.status == 0);
  const auto before = snapshot(s);
  REQUIRE(role(before, "split", "library_worker"));
  CHECK(role(before, "split", "library_worker")->state == "ready");
  CHECK(role(before, "split", "cli_worker")->state == "ready");
  CHECK(role(before, "split", "reviewer")->state == "blocked");

  // library_worker changes structure; cli_worker is removed.
  s.write("split.lua", split_v2);
  const auto changed = load(s, head(s), "changed");
  INFO(changed.error);
  REQUIRE(changed.status == 0);
  CHECK(field(changed.value, "outcome") == "accepted");
  const auto middle = snapshot(s);
  CHECK(middle.active_job == "split");
  const auto* library = role(middle, "split", "library_worker");
  REQUIRE(library);
  CHECK(library->generation_ordinal == 2);
  CHECK(library->state == "ready");
  CHECK(library->fact_version == role(before, "split", "library_worker")->fact_version + 1);
  const auto* cli = role(middle, "split", "cli_worker");
  REQUIRE(cli);
  CHECK(cli->state == "retired");
  const auto* reviewer = role(middle, "split", "reviewer");
  REQUIRE(reviewer);
  CHECK(reviewer->generation_ordinal == 1);
  CHECK(reviewer->fact_version == role(before, "split", "reviewer")->fact_version);

  // The removed role comes back under its old name: a recreated generation that inherits nothing.
  s.write("split.lua", split_v1);
  const auto restored = load(s, head(s), "restored");
  INFO(restored.error);
  REQUIRE(restored.status == 0);
  const auto after = snapshot(s);
  const auto* back = role(after, "split", "cli_worker");
  REQUIRE(back);
  CHECK(back->state == "ready");
  CHECK(back->generation_ordinal == 2);
  CHECK(role(after, "split", "library_worker")->generation_ordinal == 3);
}

TEST_CASE("format3 load clears an active job absent from the new revision", "[load_format3]") {
  scratch s;
  seeded(s);
  REQUIRE(activate(s, "combined", head(s), "NONE", "act").status == 0);
  s.write("solo.lua", solo_job);
  s.write("workspace.lua", workspace_split_only);
  const auto absent = load(s, head(s), "absent");
  INFO(absent.error);
  REQUIRE(absent.status == 0);
  const auto state = snapshot(s);
  CHECK(state.active_job.empty());
  REQUIRE(state.roles.size() == 2);
  for (const auto& row : state.roles) {
    CHECK(row.job == "combined");
    CHECK(row.state == "retired");
  }
}

TEST_CASE("format1 and format2 loads keep the three-service path", "[load_format3]") {
  for (int format : {1, 2}) {
    scratch s;
    REQUIRE((format == 1 ? planar::engine::ledger::initialize_new : planar::engine::ledger::initialize_new_v2)(s.ledger(),
                                                                                                             "quota"));
    const auto first = load(s, "none", "first");
    INFO(first.error);
    REQUIRE(first.status == 0);
    CHECK(field(first.value, "kind") == "load");
    CHECK(field(first.value, "outcome") == "accepted");
    // The execution reader refuses legacy formats; the load path never needed it.
    CHECK_FALSE(planar::engine::ledger::read_execution_snapshot(s.ledger()));
  }
}
