// execution_change_v1 decoding tests (execution-contract P2-1, EX-3, L16, L20, D151). Change values are built as Lua
// would return them; snapshots are hand-built except in the end-to-end load, which commits on a fresh format3 ledger
// under a per-test mkdtemp root.
#include <catch2/catch_test_macros.hpp>
#include <stdlib.h>
#include <unistd.h>
import std;
import planar.engine.canonical;
import planar.engine.value;
import planar.engine.ledger.execution;
import planar.engine.ledger.execution_schema;
import planar.engine.execution.change_value;

using namespace planar::engine;
using namespace planar::engine::execution;
namespace ledger = planar::engine::ledger;
namespace hist   = planar::engine::history;
using kind_t     = ledger::execution_expectation::kind_t;

namespace {

struct scratch {
  std::filesystem::path root;
  scratch() {
    char  pattern[] = "/tmp/planar-change-value-XXXXXX";
    char* made      = ::mkdtemp(pattern);
    REQUIRE(made != nullptr);
    root = std::filesystem::canonical(made);
  }
  ~scratch() {
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
  }
};

value_ptr vstr(std::string text) {
  value out{};
  out.kind   = value_kind::string;
  out.string = std::move(text);
  return std::make_shared<const value>(std::move(out));
}

value_ptr vint(std::int64_t number) {
  value out{};
  out.kind    = value_kind::integer;
  out.integer = number;
  return std::make_shared<const value>(std::move(out));
}

value_ptr vbool(bool flag) {
  value out{};
  out.kind    = value_kind::boolean;
  out.boolean = flag;
  return std::make_shared<const value>(std::move(out));
}

value_ptr vrec(const std::vector<std::pair<std::string, value_ptr>>& entries) {
  value out{};
  out.kind = value_kind::record;
  for (const auto& [key, item] : entries)
    out.fields[key] = item;
  return std::make_shared<const value>(std::move(out));
}

value_ptr vlist(const std::vector<value_ptr>& items) {
  value out{};
  out.kind  = value_kind::list;
  out.items = items;
  return std::make_shared<const value>(std::move(out));
}

value_ptr with(const value_ptr& node, const std::string& key, const value_ptr& replacement) {
  auto copy = std::make_shared<value>(*node);
  if (replacement)
    copy->fields[key] = replacement;
  else
    copy->fields.erase(key);
  return copy;
}

std::string bytes_of(const value_ptr& node) {
  auto bytes = canonical_encode(node);
  REQUIRE(bytes.has_value());
  return *bytes;
}

value_ptr structure_value(std::string job, std::string role) {
  return vrec({{"job", vstr(job)},
               {"role", vstr(role)},
               {"kind", vstr("worker")},
               {"work", vlist({vstr("unit_one")})},
               {"targets", vlist({})},
               {"ownership", vlist({})},
               {"shape", vstr("linear")}});
}

hist::role_structure_v1 structure(std::string job, std::string role) {
  auto decoded = hist::decode_role_structure_v1(bytes_of(structure_value(job, role)));
  REQUIRE(decoded.has_value());
  return *decoded;
}

/// Snapshot at seq 9, revision 2, job alpha active with builder ready (row) and runner running on run 5.
ledger::execution_snapshot base_snapshot() {
  ledger::execution_snapshot snapshot;
  snapshot.identity   = std::string(32, 'a');
  snapshot.last_seq   = 9;
  snapshot.active_job = "alpha";
  snapshot.activation = 1;
  ledger::execution_revision revision;
  revision.id       = 2;
  snapshot.revision = revision;
  ledger::execution_role builder;
  builder.job                   = "alpha";
  builder.role                  = "builder";
  builder.generation_id         = 3;
  builder.generation_ordinal    = 1;
  builder.structure             = structure("alpha", "builder");
  builder.state                 = "ready";
  builder.fact_version          = 1;
  ledger::execution_role runner = builder;
  runner.role                   = "runner";
  runner.generation_id          = 4;
  runner.structure              = structure("alpha", "runner");
  runner.state                  = "running";
  runner.run_id                 = 5;
  snapshot.roles                = {builder, runner};
  return snapshot;
}

value_ptr role_state_member(std::string job, std::string role, std::string state, bool drifted, std::string run) {
  return vrec({{"kind", vstr("role_state")},
               {"job", vstr(job)},
               {"role", vstr(role)},
               {"state", vstr(state)},
               {"drifted", vbool(drifted)},
               {"run", vstr(run)}});
}

value_ptr no_row_member(std::string job, std::string role) {
  return vrec({{"kind", vstr("role_state")}, {"job", vstr(job)}, {"role", vstr(role)}, {"no_row", vbool(true)}});
}

/// Activation of job beta (never active before, so its two roles have no rows): the shape reconcile.lua returns.
value_ptr activate_value(const std::vector<value_ptr>& extra_expected = {}, bool with_no_rows = true) {
  std::vector<value_ptr> expected{
      vrec({{"kind", vstr("request_unused")}, {"request_id", vstr("act-1")}}),
      vrec({{"kind", vstr("active_revision")}, {"revision", vstr("2")}}),
      vrec({{"kind", vstr("active_job")}, {"job", vstr("alpha")}}),
      vrec({{"kind", vstr("fact_version")}, {"job", vstr("alpha")}, {"role", vstr("builder")}, {"fact_version", vstr("1")}}),
      role_state_member("alpha", "builder", "ready", false, "none")};
  if (with_no_rows) {
    expected.push_back(no_row_member("beta", "lead"));
    expected.push_back(no_row_member("beta", "second"));
  }
  for (const auto& item : extra_expected)
    expected.push_back(item);
  auto generation = [](std::string role) {
    return vrec({{"job", vstr("beta")},
                 {"role", vstr(role)},
                 {"reason", vstr("activated")},
                 {"structure", structure_value("beta", role)}});
  };
  auto first = [](std::string role) {
    return vrec({{"job", vstr("beta")},
                 {"role", vstr(role)},
                 {"after_state", vstr("ready")},
                 {"after_drifted", vbool(false)},
                 {"reason", vstr("activated")}});
  };
  return vrec({{"version", vint(1)},
               {"operation", vstr("activate")},
               {"snapshot_seq", vstr("9")},
               {"facts", vrec({{"job", vstr("beta")},
                               {"generations", vlist({generation("lead"), generation("second")})},
                               {"fenced_runs", vlist({vstr("5")})}})},
               {"vector", vlist({vrec({{"job", vstr("beta")},
                                       {"role", vstr("lead")},
                                       {"state", vstr("ready")},
                                       {"drifted", vbool(false)},
                                       {"run", vstr("none")}})})},
               {"transitions", vlist({vrec({{"job", vstr("alpha")},
                                            {"role", vstr("builder")},
                                            {"before_state", vstr("ready")},
                                            {"before_drifted", vbool(false)},
                                            {"after_state", vstr("retired")},
                                            {"after_drifted", vbool(false)},
                                            {"reason", vstr("retired")}}),
                                      first("lead"), first("second")})},
               {"expected", vlist(expected)}});
}

change_request activate_request() {
  return change_request{.request_id       = "act-1",
                        .actor            = "tester",
                        .profile_argument = std::nullopt,
                        .options          = activate_options{.job = "beta", .expect_revision = "2", .expect_job = "alpha"},
                        .provenance       = nullptr};
}

void require_field(const std::expected<ledger::execution_change, change_error>& result, std::string_view field) {
  REQUIRE_FALSE(result.has_value());
  INFO("failing field " << result.error().field);
  REQUIRE(result.error().field == field);
}

} // namespace

TEST_CASE("execution change request_v3 parameter maps are built natively per verb") {
  auto activate =
      build_request_v3("tester", activate_options{.job = "beta", .expect_revision = "none", .expect_job = "NONE"}, std::nullopt);
  REQUIRE(activate.has_value());
  REQUIRE(activate->operation == "activate");
  REQUIRE(activate->actor == "tester");
  REQUIRE_FALSE(activate->profile_argument.has_value());
  REQUIRE(activate->parameters ==
          bytes_of(vrec({{"expect_job", vstr("NONE")}, {"expect_revision", vstr("none")}, {"job", vstr("beta")}})));

  auto claim =
      build_request_v3("tester", claim_options{.job = "alpha", .role = "builder", .provenance_path = std::nullopt}, std::nullopt);
  REQUIRE(claim.has_value());
  REQUIRE(claim->parameters == bytes_of(vrec({{"job", vstr("alpha")}, {"role", vstr("builder")}})));
  auto claim_with = build_request_v3(
      "tester", claim_options{.job = "alpha", .role = "builder", .provenance_path = "/private/p.json"}, "/private/profile");
  REQUIRE(claim_with.has_value());
  REQUIRE(claim_with->profile_argument == std::optional<std::string>("/private/profile"));
  REQUIRE(claim_with->parameters ==
          bytes_of(vrec({{"job", vstr("alpha")}, {"provenance_path", vstr("/private/p.json")}, {"role", vstr("builder")}})));

  auto takeover = build_request_v3(
      "tester", takeover_options{.job = "alpha", .role = "runner", .run_id = 5, .provenance_path = std::nullopt}, std::nullopt);
  REQUIRE(takeover.has_value());
  REQUIRE(takeover->operation == "takeover");
  REQUIRE(takeover->parameters == bytes_of(vrec({{"job", vstr("alpha")}, {"role", vstr("runner")}, {"run_id", vstr("5")}})));

  REQUIRE_FALSE(build_request_v3("tester", load_options{}, std::nullopt).has_value());
}

TEST_CASE("execution change decodes an activate change with native framing and no-row members") {
  const auto snapshot = base_snapshot();
  auto       result   = decode_execution_change(activate_value(), snapshot, activate_request());
  INFO("failing field " << (result.has_value() ? std::string{} : result.error().field));
  REQUIRE(result.has_value());
  const auto& change = *result;
  REQUIRE(change.operation == "activate");
  REQUIRE(change.request_id == "act-1");
  REQUIRE(change.actor == "tester");
  REQUIRE(change.job == "beta");
  REQUIRE(change.role.empty());
  const auto* framed = std::get_if<hist::request_v3>(&change.request);
  REQUIRE(framed != nullptr);
  REQUIRE(framed->parameters ==
          bytes_of(vrec({{"expect_job", vstr("alpha")}, {"expect_revision", vstr("2")}, {"job", vstr("beta")}})));
  REQUIRE(change.generations.size() == 2);
  REQUIRE(change.generations[0].structure.role == "lead");
  REQUIRE(change.fenced_runs == std::vector<std::uint64_t>{5});
  REQUIRE(change.transitions.size() == 3);
  REQUIRE(change.transitions[0].before_state == std::optional<std::string>("ready"));
  REQUIRE_FALSE(change.transitions[1].before_state.has_value());
  REQUIRE_FALSE(change.transitions[1].before_drifted.has_value());
  REQUIRE(change.vector.size() == 1);
  REQUIRE_FALSE(change.vector[0].run.has_value());
  REQUIRE(change.expected.size() == 7);
  REQUIRE(change.expected[0].kind == kind_t::request_unused);
  REQUIRE(change.expected[1].revision == std::optional<std::uint64_t>(2));
  REQUIRE(change.expected[2].text == "alpha");
  REQUIRE(change.expected[3].number == 1);
  REQUIRE(change.expected[4].state == "ready");
  REQUIRE(change.expected[5].no_row);
  REQUIRE(change.expected[5].role == "lead");
  REQUIRE_FALSE(change.packet.has_value());
}

TEST_CASE("execution change refuses a snapshot_seq other than the snapshot last_seq") {
  const auto snapshot = base_snapshot();
  require_field(decode_execution_change(with(activate_value(), "snapshot_seq", vstr("8")), snapshot, activate_request()),
                "change.snapshot_seq");
  require_field(decode_execution_change(with(activate_value(), "snapshot_seq", vint(9)), snapshot, activate_request()),
                "change.snapshot_seq");
  auto later     = snapshot;
  later.last_seq = 10;
  require_field(decode_execution_change(activate_value(), later, activate_request()), "change.snapshot_seq");
}

TEST_CASE("execution change refuses a load or activate change missing a no-row member") {
  const auto snapshot = base_snapshot();
  // DD29: beta is not the active job, so its first-row transitions need no no-row member (EX-3 asserts members for the
  // activated job only where rows exist); the decoder accepts the change without them.
  CHECK(decode_execution_change(activate_value({}, false), snapshot, activate_request()).has_value());
  // A no-row member for a role that has a row is a mismatch, not a weaker assertion.
  require_field(decode_execution_change(activate_value({no_row_member("alpha", "builder")}), snapshot, activate_request()),
                "change.expected.7.no_row");
}

TEST_CASE("execution change refuses any member outside the closed shape") {
  const auto snapshot = base_snapshot();
  const auto request  = activate_request();
  require_field(decode_execution_change(with(activate_value(), "extra", vstr("x")), snapshot, request), "change.extra");
  require_field(decode_execution_change(with(activate_value(), "vector", nullptr), snapshot, request), "change.vector");
  require_field(decode_execution_change(with(activate_value(), "version", vint(2)), snapshot, request), "change.version");
  require_field(decode_execution_change(with(activate_value(), "operation", vstr("claim")), snapshot, request),
                "change.operation");
  // An activate change cannot carry a packet.
  require_field(decode_execution_change(with(activate_value(), "packet", vrec({})), snapshot, request), "change.packet");
  // run_active and reservation_open are outside the R15 vocabulary.
  require_field(decode_execution_change(activate_value({vrec({{"kind", vstr("run_active")}})}), snapshot, request),
                "change.expected.7.kind");
  // A request_unused member naming another request.
  auto other = activate_value();
  auto items = other->fields.at("expected")->items;
  items[0]   = vrec({{"kind", vstr("request_unused")}, {"request_id", vstr("other")}});
  require_field(decode_execution_change(with(other, "expected", vlist(items)), snapshot, request),
                "change.expected.0.request_id");
  // facts.job must be the activated job of the typed options.
  auto facts           = std::make_shared<value>(*activate_value()->fields.at("facts"));
  facts->fields["job"] = vstr("gamma");
  require_field(decode_execution_change(with(activate_value(), "facts", facts), snapshot, request), "change.facts.job");
  // Half a before state is not NULL-first.
  auto transitions = activate_value()->fields.at("transitions")->items;
  transitions[0]   = with(transitions[0], "before_drifted", nullptr);
  require_field(decode_execution_change(with(activate_value(), "transitions", vlist(transitions)), snapshot, request),
                "change.transitions.0.before_state");
}

TEST_CASE("execution change accepts an empty Lua table as an empty list") {
  const auto snapshot = base_snapshot();
  auto       value    = with(activate_value(), "vector", vrec({}));
  auto       result   = decode_execution_change(value, snapshot, activate_request());
  REQUIRE(result.has_value());
  REQUIRE(result->vector.empty());
}

TEST_CASE("execution change claim and takeover take identity and lineage from native options") {
  auto snapshot = base_snapshot();
  auto packet   = vrec(
      {{"markdown", vstr("# Packet\n")}, {"manifest", vrec({{"job", vstr("alpha")}})}, {"work", vrec({{"job", vstr("alpha")}})}});
  auto           claim = vrec({{"version", vint(1)},
                               {"operation", vstr("claim")},
                               {"snapshot_seq", vstr("9")},
                               {"facts", vrec({{"role", vstr("builder")}})},
                               {"vector", vlist({})},
                               {"transitions", vlist({})},
                               {"packet", packet},
                               {"expected", vlist({vrec({{"kind", vstr("generation_current")},
                                                         {"job", vstr("alpha")},
                                                         {"role", vstr("builder")},
                                                         {"generation", vstr("3")}})})}});
  change_request request{.request_id       = "claim-1",
                         .actor            = "tester",
                         .profile_argument = std::nullopt,
                         .options          = claim_options{.job = "alpha", .role = "builder", .provenance_path = std::nullopt},
                         .provenance       = nullptr};
  auto           decoded = decode_execution_change(claim, snapshot, request);
  INFO("failing field " << (decoded.has_value() ? std::string{} : decoded.error().field));
  REQUIRE(decoded.has_value());
  REQUIRE(decoded->job == "alpha");
  REQUIRE(decoded->role == "builder");
  REQUIRE(decoded->packet.has_value());
  REQUIRE(decoded->packet->markdown == "# Packet\n");
  REQUIRE(decoded->expected[0].kind == kind_t::generation_current);
  REQUIRE(decoded->expected[0].number == 3);
  REQUIRE(decoded->generations.empty());
  REQUIRE(decoded->fenced_runs.empty());
  require_field(decode_execution_change(with(claim, "facts", vrec({{"role", vstr("runner")}})), snapshot, request),
                "change.facts.role");
  require_field(decode_execution_change(with(claim, "packet", nullptr), snapshot, request), "change.packet");

  // Provenance in the facts must be exactly the native document.
  const auto document = vrec({{"version", vint(1)}});
  auto       with_doc = request;
  with_doc.provenance = document;
  require_field(decode_execution_change(claim, snapshot, with_doc), "change.facts.provenance");
  auto claimed = decode_execution_change(with(claim, "facts", vrec({{"role", vstr("builder")}, {"provenance", document}})),
                                         snapshot, with_doc);
  REQUIRE(claimed.has_value());
  REQUIRE(claimed->provenance == document);
  require_field(decode_execution_change(
                    with(claim, "facts", vrec({{"role", vstr("builder")}, {"provenance", vrec({{"version", vint(2)}})}})),
                    snapshot, with_doc),
                "change.facts.provenance");

  // Takeover: the generation and the fenced run are native; previous_run must be the --run-id.
  auto takeover =
      with(with(claim, "operation", vstr("takeover")), "facts", vrec({{"role", vstr("runner")}, {"previous_run", vstr("5")}}));
  change_request takeover_request{
      .request_id       = "take-1",
      .actor            = "tester",
      .profile_argument = std::nullopt,
      .options          = takeover_options{.job = "alpha", .role = "runner", .run_id = 5, .provenance_path = std::nullopt},
      .provenance       = nullptr};
  auto taken = decode_execution_change(takeover, snapshot, takeover_request);
  REQUIRE(taken.has_value());
  REQUIRE(taken->previous_run == std::optional<std::uint64_t>(5));
  REQUIRE(taken->fenced_runs == std::vector<std::uint64_t>{5});
  REQUIRE(taken->generations.size() == 1);
  REQUIRE(taken->generations[0].reason == "takeover");
  REQUIRE(bytes_of(vrec({})) != "");
  REQUIRE(hist::encode_role_structure_v1(taken->generations[0].structure).value() ==
          hist::encode_role_structure_v1(snapshot.roles[1].structure).value());
  require_field(decode_execution_change(with(takeover, "facts", vrec({{"role", vstr("runner")}, {"previous_run", vstr("6")}})),
                                        snapshot, takeover_request),
                "change.facts.previous_run");
}

TEST_CASE("execution change decoded load commits on a fresh format3 ledger") {
  scratch    dir;
  const auto path = dir.root / "ledger.planar";
  REQUIRE(ledger::initialize_format3(path, "alpha_workspace").has_value());
  auto snapshot = ledger::read_execution_snapshot(path);
  REQUIRE(snapshot.has_value());

  ledger::execution_prepared_revision prepared;
  prepared.source.files = {hist::tree_file{"workspace.lua", "return {}\n", 420}};
  prepared.registry     = {{hist::tree_file{"input.json", "{}", 420}}, hist::tree_profile::provenance};
  prepared.stdlib       = {{hist::tree_file{"planar/stdlib.lua", "-- stdlib\n", 420}}, hist::tree_profile::provenance};
  std::vector<hist::tree_file_view> views{{"planar/stdlib.lua", "-- stdlib\n", 420}};
  auto                              digest = hist::digest_tree(views, hist::tree_profile::provenance);
  REQUIRE(digest.has_value());
  std::string hex;
  for (auto byte : *digest)
    hex += std::format("{:02x}", byte);
  const auto versions = vrec({{"stdlib_digest", vstr(hex)}});
  prepared.interpretation =
      bytes_of(vrec({{"bundle", vrec({{"workspace", vrec({{"id", vstr("alpha_workspace")}})}})}, {"versions", versions}}));
  prepared.versions = bytes_of(versions);

  hist::request_v2 framing;
  framing.operation          = "load";
  framing.actor              = "tester";
  framing.expected_head      = "none";
  framing.workspace_argument = "alpha_workspace";
  framing.registry_argument  = "/private/registry.json";
  change_request request{.request_id       = "load-1",
                         .actor            = "tester",
                         .profile_argument = std::nullopt,
                         .options          = load_options{.request = framing, .revision = {}, .prepared = prepared},
                         .provenance       = nullptr};
  auto           change  = vrec({{"version", vint(1)},
                                 {"operation", vstr("load")},
                                 {"snapshot_seq", vstr("1")},
                                 {"facts", vrec({{"job", vstr("")}, {"generations", vrec({})}, {"fenced_runs", vrec({})}})},
                                 {"vector", vrec({})},
                                 {"transitions", vrec({})},
                                 {"expected", vlist({vrec({{"kind", vstr("request_unused")}, {"request_id", vstr("load-1")}}),
                                                     vrec({{"kind", vstr("active_revision")}, {"revision", vstr("none")}}),
                                                     vrec({{"kind", vstr("active_job")}, {"job", vstr("")}})})}});
  auto           decoded = decode_execution_change(change, *snapshot, request);
  INFO("failing field " << (decoded.has_value() ? std::string{} : decoded.error().field));
  REQUIRE(decoded.has_value());
  REQUIRE(std::holds_alternative<hist::request_v2>(decoded->request));
  auto committed = ledger::commit_execution(path, *snapshot, *decoded);
  INFO("commit error " << (committed.has_value() ? -1 : static_cast<int>(committed.error().code)));
  REQUIRE(committed.has_value());
  REQUIRE(ledger::verify_execution_ledger(path).has_value());
  auto after = ledger::read_execution_snapshot(path);
  REQUIRE(after.has_value());
  REQUIRE(after->revision.has_value());
  REQUIRE(after->revision->id == 1);

  // A framing that names another actor than the request is a mismatch, never repaired.
  auto wrong                                          = request;
  std::get<load_options>(wrong.options).request.actor = "intruder";
  require_field(decode_execution_change(change, *snapshot, wrong), "request.framing");
}

TEST_CASE("R18 verdict change decoder preserves native document and refuses another run") {
  auto             snapshot = base_snapshot();
  auto             digest   = "xxh3_128:" + std::string(32, 'a');
  auto             targets  = vlist({vrec({{"work", vstr("alpha")}, {"output", vstr(digest)}, {"work_fp", vstr(digest)}})});
  auto             document = vrec({{"version", vint(1)},
                                    {"targets", targets},
                                    {"policy", vstr(digest)},
                                    {"decision", vstr("approve")},
                                    {"findings", vlist({})},
                                    {"repair", vlist({})},
                                    {"summary", vstr("checked")},
                                    {"authority", vstr("reviewer_role_v1")}});
  auto             facts    = vrec({{"role", vstr("reviewer")},
                                    {"run", vstr("4")},
                                    {"verdict", document},
                                    {"current", vrec({{"targets", targets}, {"policy", vstr(digest)}, {"work", vrec({})}})},
                                    {"max_rounds", vint(2)},
                                    {"case", vstr("approve")},
                                    {"repairs", vlist({})},
                                    {"blockers_opened", vlist({})},
                                    {"blockers_cleared", vlist({})}});
  auto             change   = vrec({{"version", vint(1)},
                                    {"operation", vstr("verdict")},
                                    {"snapshot_seq", vstr("9")},
                                    {"facts", facts},
                                    {"vector", vlist({})},
                                    {"transitions", vlist({})},
                                    {"expected", vlist({vrec({{"kind", vstr("run_active")},
                                                              {"job", vstr("alpha")},
                                                              {"role", vstr("reviewer")},
                                                              {"run", vstr("4")},
                                                              {"generation", vstr("3")}})})}});
  hist::request_v3 request{.operation        = "verdict",
                           .actor            = "tester",
                           .parameters       = bytes_of(vrec({{"run_id", vstr("4")}, {"verdict_path", vstr("/verdict.json")}})),
                           .profile_argument = std::nullopt};
  auto             decoded = decode_verdict_change(change, snapshot, "review-1", request, 4, document);
  REQUIRE(decoded);
  REQUIRE(decoded->verdict);
  REQUIRE(decoded->verdict->max_rounds == 2);
  REQUIRE(decoded->verdict->document == document);
  REQUIRE(decoded->expected.front().number == 3);
  REQUIRE_FALSE(decode_verdict_change(change, snapshot, "review-1", request, 5, document));
}
