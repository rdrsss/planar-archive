// planar activate end to end through the real CLI entry (R15-C P3): first activation, job switch, re-activation, the
// P2-1 refusal order (format, replay, interpretation, expectations, reference), the D149 activate pin and the root help
// line. Every ledger lives under a per-test mkdtemp root and is created with initialize_format3 directly.
#include <catch2/catch_test_macros.hpp>
#include <stdlib.h>
#include <unistd.h>
import std;
import planar.cmd.bootstrap;
import planar.engine.canonical;
import planar.engine.json_codec;
import planar.engine.value;
import planar.engine.history.request_v2;
import planar.engine.history.tree_core;
import planar.engine.ledger.execution;
import planar.engine.ledger.execution_schema;
import planar.engine.ledger.format;

namespace {
namespace fs   = std::filesystem;
namespace hist = planar::engine::history;
using planar::engine::value;
using planar::engine::value_kind;
using planar::engine::value_ptr;

struct scratch {
  fs::path root;
  scratch() {
    char  pattern[] = "/tmp/planar-activate-XXXXXX";
    char* made      = ::mkdtemp(pattern);
    REQUIRE(made != nullptr);
    root = fs::canonical(made);
  }
  ~scratch() {
    std::error_code ignored;
    fs::remove_all(root, ignored);
  }
  fs::path ledger() const {
    return root / "ledger.planar";
  }
};

fs::path fixture_root() {
  return fs::path(PLANAR_B2_WORKSPACE_FIXTURE).parent_path().parent_path();
}

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

const value_ptr& member(const value_ptr& node, std::string_view key) {
  REQUIRE(node);
  REQUIRE(node->kind == value_kind::record);
  auto found = node->fields.find(std::string(key));
  REQUIRE(found != node->fields.end());
  return found->second;
}

std::string field(const value_ptr& node, std::string_view key) {
  const auto& item = member(node, key);
  REQUIRE(item->kind == value_kind::string);
  return item->string;
}

const value_ptr& error_of(const outcome& result) {
  REQUIRE(result.value);
  return member(result.value, "error");
}

outcome load(const scratch& s, std::string_view head, std::string_view request) {
  return run({"load", "--ledger", s.ledger().string(), "--workspace", (fixture_root() / "workspace/workspace.lua").string(),
              "--registry", (fixture_root() / "registry/quota-tools.json").string(), "--expect-revision", std::string(head),
              "--request-id", std::string(request), "--actor", "tester"});
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

void seeded(const scratch& s) {
  REQUIRE(planar::engine::ledger::initialize_format3(s.ledger(), "quota"));
  const auto first = load(s, "none", "seed");
  INFO(first.error);
  REQUIRE(first.status == 0);
}

/// `{role, generation, state}` triples of an activate_v1 response, in response order.
std::vector<std::array<std::string, 3>> roles_of(const value_ptr& response) {
  std::vector<std::array<std::string, 3>> out;
  const auto&                             roles = member(response, "roles");
  REQUIRE(roles->kind == value_kind::list);
  for (const auto& item : roles->items)
    out.push_back({field(item, "role"), field(item, "generation"), field(item, "state")});
  return out;
}

value_ptr vstr(std::string text) {
  value out{};
  out.kind   = value_kind::string;
  out.string = std::move(text);
  return std::make_shared<const value>(std::move(out));
}

value_ptr vrec(std::map<std::string, value_ptr> fields) {
  value out{};
  out.kind   = value_kind::record;
  out.fields = std::move(fields);
  return std::make_shared<const value>(std::move(out));
}

std::string hex(const hist::record_digest& digest) {
  std::string out;
  for (auto byte : digest)
    out += std::format("{:02x}", byte);
  return out;
}

/// Commit one load revision whose recorded stdlib is a one-file tree that no binary embeds, through the ledger API.
void seed_foreign_stdlib(const scratch& s) {
  namespace ledger = planar::engine::ledger;
  REQUIRE(ledger::initialize_format3(s.ledger(), "quota"));
  const std::vector<hist::tree_file> stdlib_files{{"stdlib.lua", "foreign stdlib body", 420}};
  std::vector<hist::tree_file_view>  views{{stdlib_files[0].path, stdlib_files[0].payload, stdlib_files[0].mode}};
  auto                               stdlib_digest = hist::digest_tree(views, hist::tree_profile::provenance);
  REQUIRE(stdlib_digest);
  const auto versions = vrec({{"stdlib_digest", vstr(hex(*stdlib_digest))}});
  const auto interpretation =
      vrec({{"tag", vstr("foreign")}, {"bundle", vrec({{"workspace", vrec({{"id", vstr("quota")}})}})}, {"versions", versions}});
  auto interpretation_bytes = planar::engine::canonical_encode(interpretation);
  auto versions_bytes       = planar::engine::canonical_encode(versions);
  REQUIRE(interpretation_bytes);
  REQUIRE(versions_bytes);
  auto state = ledger::read_execution_snapshot(s.ledger());
  REQUIRE(state);
  ledger::execution_change change;
  change.operation  = "load";
  change.request_id = "foreign";
  change.actor      = "tester";
  hist::request_v2 request;
  request.operation          = "load";
  request.actor              = "tester";
  request.expected_head      = "none";
  request.workspace_argument = "quota";
  request.registry_argument  = "registry";
  change.request             = request;
  change.revision            = ledger::execution_revision_input{};
  change.prepared            = ledger::execution_prepared_revision{
      {{{"source.txt", "source foreign", 420}}, hist::tree_profile::source},
      {{{"input.json", "{\"tag\":\"foreign\"}", 420}}, hist::tree_profile::provenance},
      {stdlib_files, hist::tree_profile::provenance},
      *interpretation_bytes,
      *versions_bytes};
  using kind_t = ledger::execution_expectation::kind_t;
  ledger::execution_expectation revision, job, unused;
  revision.kind = kind_t::active_revision;
  job.kind      = kind_t::active_job;
  unused.kind   = kind_t::request_unused;
  change.expected = {revision, job, unused};
  auto committed  = ledger::commit_execution(s.ledger(), *state, change);
  INFO("seed commit failed with code " << (committed ? -1 : static_cast<int>(committed.error().code)));
  REQUIRE(committed);
}

} // namespace

TEST_CASE("activate first time opens activated generations and replays exactly", "[activate]") {
  scratch s;
  seeded(s);
  const auto revision = head(s);
  const auto first    = activate(s, "combined", revision, "NONE", "act");
  INFO(first.error);
  REQUIRE(first.status == 0);
  CHECK(field(first.value, "kind") == "activate");
  CHECK(field(first.value, "job") == "combined");
  CHECK(field(first.value, "previous_job").empty());
  CHECK(field(first.value, "revision") == revision);
  CHECK(field(first.value, "request_id") == "act");
  const std::vector<std::array<std::string, 3>> expected{{"builder", "1", "ready"}, {"reviewer", "1", "blocked"}};
  CHECK(roles_of(first.value) == expected);
  const auto state = snapshot(s);
  CHECK(state.active_job == "combined");
  CHECK(state.activation);
  CHECK(field(first.value, "activation") == std::to_string(*state.activation));
  CHECK(field(first.value, "event") == std::to_string(state.last_seq));

  // Exact replay returns the saved bytes, even though the expectations no longer hold.
  const auto replay = activate(s, "combined", revision, "NONE", "act");
  REQUIRE(replay.status == 0);
  CHECK(replay.output == first.output);
  CHECK(snapshot(s).last_seq == state.last_seq);
  // Another body under the same ID conflicts before any expectation.
  const auto conflict = activate(s, "split", revision, "NONE", "act");
  CHECK(conflict.status == 3);
  CHECK(field(error_of(conflict), "code") == "request_conflict");
}

TEST_CASE("activate of another job retires the previous job and re-activation opens new generations", "[activate]") {
  scratch s;
  seeded(s);
  const auto revision = head(s);
  REQUIRE(activate(s, "combined", revision, "NONE", "one").status == 0);
  const auto switched = activate(s, "split", revision, "combined", "two");
  INFO(switched.error);
  REQUIRE(switched.status == 0);
  CHECK(field(switched.value, "previous_job") == "combined");
  const std::vector<std::array<std::string, 3>> split_first{
      {"cli_worker", "1", "ready"}, {"library_worker", "1", "ready"}, {"reviewer", "1", "blocked"}};
  CHECK(roles_of(switched.value) == split_first);
  auto state = snapshot(s);
  CHECK(state.active_job == "split");
  CHECK(role(state, "combined", "builder")->state == "retired");
  CHECK(role(state, "combined", "reviewer")->state == "retired");

  // Re-activation of the active job: every role gets a new generation, nothing retires.
  const auto again = activate(s, "split", revision, "split", "three");
  INFO(again.error);
  REQUIRE(again.status == 0);
  CHECK(field(again.value, "previous_job") == "split");
  const std::vector<std::array<std::string, 3>> split_second{
      {"cli_worker", "2", "ready"}, {"library_worker", "2", "ready"}, {"reviewer", "2", "blocked"}};
  CHECK(roles_of(again.value) == split_second);
  state = snapshot(s);
  for (const auto& row : state.roles)
    if (row.job == "split")
      CHECK(row.state != "retired");
}

TEST_CASE("activate refuses in the P2-1 order with the closed codes", "[activate]") {
  SECTION("format2 ledger is unsupported_ledger_version") {
    scratch s;
    REQUIRE(planar::engine::ledger::initialize_new_v2(s.ledger(), "quota"));
    const auto refused = activate(s, "combined", "none", "NONE", "k");
    CHECK(refused.status == 3);
    CHECK(field(error_of(refused), "code") == "unsupported_ledger_version");
    CHECK(field(error_of(refused), "message") == "Ledger version or state is unsupported");
  }
  SECTION("no active revision: the job is not in any revision") {
    scratch s;
    REQUIRE(planar::engine::ledger::initialize_format3(s.ledger(), "quota"));
    const auto refused = activate(s, "combined", "none", "NONE", "k");
    CHECK(refused.status == 3);
    CHECK(field(error_of(refused), "code") == "reference");
    CHECK(field(error_of(refused), "field") == "selection.job");
  }
  SECTION("expect-revision mismatch is revision_conflict with details") {
    scratch s;
    seeded(s);
    const auto refused = activate(s, "combined", "none", "NONE", "k");
    CHECK(refused.status == 3);
    const auto& error = error_of(refused);
    CHECK(field(error, "code") == "revision_conflict");
    CHECK(field(error, "message") == "Ledger revision does not match the expected revision");
    CHECK(field(member(error, "details"), "expected") == "none");
    CHECK(field(member(error, "details"), "actual") == head(s));
  }
  SECTION("expect-job naming a job while none is active is no_active_job") {
    scratch s;
    seeded(s);
    const auto refused = activate(s, "combined", head(s), "combined", "k");
    CHECK(refused.status == 3);
    CHECK(field(error_of(refused), "code") == "no_active_job");
    CHECK(field(error_of(refused), "message") == "No active job matches the request");
  }
  SECTION("unknown job is reference Unknown selected job") {
    scratch s;
    seeded(s);
    const auto refused = activate(s, "ghost", head(s), "NONE", "k");
    CHECK(refused.status == 3);
    CHECK(field(error_of(refused), "code") == "reference");
    CHECK(field(error_of(refused), "message") == "Unknown selected job");
    CHECK(field(error_of(refused), "field") == "selection.job");
  }
  SECTION("malformed expect-job is usage") {
    scratch s;
    seeded(s);
    CHECK(activate(s, "combined", head(s), "Combined", "k").status == 2);
    CHECK(activate(s, "combined", head(s), "none!", "k").status == 2);
  }
}

TEST_CASE("D149 activate pin: expect-job NONE while a job is active is state_conflict and a same-ID retry refuses",
          "[activate][d149]") {
  scratch s;
  seeded(s);
  const auto revision = head(s);
  REQUIRE(activate(s, "combined", revision, "NONE", "first").status == 0);
  const auto before = snapshot(s).last_seq;
  for (int attempt = 0; attempt < 2; ++attempt) {
    const auto refused = activate(s, "split", revision, "NONE", "pinned");
    CHECK(refused.status == 3);
    CHECK(field(error_of(refused), "code") == "state_conflict");
    CHECK(field(error_of(refused), "message") == "Execution state changed since the request was prepared");
  }
  CHECK(snapshot(s).last_seq == before);
  // A job expectation that names another job than the active one is no_active_job, not a conflict.
  const auto other = activate(s, "split", revision, "split", "other");
  CHECK(field(error_of(other), "code") == "no_active_job");
}

TEST_CASE("activate refuses a revision recorded under another stdlib with interpretation_mismatch", "[activate][ex6]") {
  scratch s;
  seed_foreign_stdlib(s);
  const auto refused = activate(s, "combined", head(s), "NONE", "k");
  CHECK(refused.status == 3);
  CHECK(field(error_of(refused), "code") == "interpretation_mismatch");
  CHECK(field(error_of(refused), "message") == "Interpretation does not match the active revision");
}

TEST_CASE("root help lists activate before the final help line", "[activate][help]") {
  const std::array<std::string_view, 1> args{"--help"};
  const auto                            result = planar::cmd::invoke(args);
  REQUIRE(result.status == 0);
  const std::string line = "planar activate --ledger PATH --job ID --expect-revision ID_OR_NONE --expect-job JOB_OR_NONE "
                           "--request-id ID --actor TEXT\n";
  const auto at = result.output.find(line);
  REQUIRE(at != std::string::npos);
  CHECK(result.output.find("planar --help\n") > at);
}
