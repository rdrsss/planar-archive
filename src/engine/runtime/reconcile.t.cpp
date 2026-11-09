// Trusted reconcile pass (execution contract EX-3, EX-4, EX-5) through the fixed export reconcile_execution, driven
// by handcrafted read_execution snapshot values over the quota fixture's real interpretation.
#include <catch2/catch_test_macros.hpp>
#include <unistd.h>
import std;
import planar.engine.runtime;
import planar.engine.json_codec;
import planar.engine.capture;
import planar.engine.hash;
using namespace planar::engine;
namespace {
value_ptr str(std::string bytes) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = std::move(bytes);
  return v;
}
value_ptr rec(std::map<std::string, value_ptr> fields = {}) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::record;
  v->fields = std::move(fields);
  return v;
}
value_ptr arr(std::vector<value_ptr> items = {}) {
  auto v   = std::make_shared<value>();
  v->kind  = value_kind::list;
  v->items = std::move(items);
  return v;
}
value_ptr flag(bool b) {
  auto v     = std::make_shared<value>();
  v->kind    = value_kind::boolean;
  v->boolean = b;
  return v;
}
value_ptr with(value_ptr original, std::string key, value_ptr replacement) {
  auto v                    = std::make_shared<value>(*original);
  v->fields[std::move(key)] = std::move(replacement);
  return v;
}
const value_ptr& at(const value_ptr& v, const std::string& key) {
  REQUIRE(v);
  INFO(key);
  REQUIRE(v->fields.contains(key));
  return v->fields.at(key);
}
std::string text(const value_ptr& v, const std::string& key) {
  return at(v, key)->string;
}
std::string read_bytes(const std::filesystem::path& file) {
  std::ifstream stream(file, std::ios::binary);
  REQUIRE(stream.good());
  return {std::istreambuf_iterator<char>(stream), {}};
}
value_ptr success(std::string_view entry, value_ptr input) {
  auto result = run_trusted(entry, input);
  INFO(entry);
  INFO((result ? "native success" : result.error().message));
  REQUIRE(result);
  if ((*result)->fields.contains("error")) {
    auto e = (*result)->fields.at("error");
    INFO(e->fields.at("field")->string);
    INFO(e->fields.at("message")->string);
    FAIL("Unexpected trusted refusal");
  }
  return (*result)->fields.at("value");
}
std::string digest(const value_ptr& v) {
  auto d = hash_value("test_equality", v);
  REQUIRE(d);
  return *d;
}
// The quota fixture interpreted by the shipped interpret_bundle: {bundle, semantic} as a revision records it.
struct quota {
  std::filesystem::path root;
  value_ptr             interpretation;
  quota() {
    std::string pattern = (std::filesystem::temp_directory_path() / "planar-reconcile-XXXXXX").string();
    REQUIRE(mkdtemp(pattern.data()));
    root = std::filesystem::canonical(pattern);
    std::filesystem::copy(std::filesystem::path(PLANAR_SCHEMA_FIXTURE) / "workspace", root / "workspace",
                          std::filesystem::copy_options::recursive);
    capture_session capture;
    REQUIRE(capture.add_root("workspace", (root / "workspace").string()));
    auto workspace = capture.load("workspace.lua");
    REQUIRE(workspace);
    auto route = success("route_workspace", rec({{"workspace", workspace->root}, {"file", str("workspace.lua")}}));
    auto spec  = capture.load(route->fields.at("spec")->string);
    REQUIRE(spec);
    std::vector<value_ptr> jobs;
    for (auto& j : route->fields.at("jobs")->items) {
      auto file = j->fields.at("file");
      auto job  = capture.load(file->string);
      REQUIRE(job);
      jobs.push_back(rec({{"file", file}, {"value", job->root}}));
    }
    auto snapshot = capture.finish();
    REQUIRE(snapshot);
    std::vector<value_ptr> files;
    for (const auto& f : **snapshot)
      files.push_back(rec({{"path", str(f.path)}, {"bytes", str(f.bytes)}}));
    auto bundle =
        rec({{"workspace", rec({{"file", str("workspace.lua")}, {"value", workspace->root}})},
             {"spec", rec({{"file", route->fields.at("spec")}, {"value", spec->root}})},
             {"jobs", arr(jobs)},
             {"registry",
              rec({{"file", str("input.json")},
                   {"bytes", str(read_bytes(std::filesystem::path(PLANAR_SCHEMA_FIXTURE) / "registry/quota-tools.json"))}})},
             {"files", arr(files)}});
    interpretation = success("interpret_bundle", bundle);
  }
  ~quota() {
    std::filesystem::remove_all(root);
  }
  value_ptr work(const std::string& job, const std::string& role, value_ptr interp = nullptr) const {
    return success("work_exec",
                   rec({{"interpretation", interp ? interp : interpretation}, {"job", str(job)}, {"role", str(role)}}));
  }
  // One snapshot role row; run is "none" or {id, generation, dispatch{..., work}} with the acknowledged W.
  value_ptr row(const std::string& job, const std::string& role, const std::string& state, bool drifted = false,
                std::optional<std::string> run = std::nullopt, value_ptr acknowledged = nullptr,
                value_ptr structure = nullptr) const {
    auto w = work(job, role);
    if (!structure)
      structure = at(w, "structure");
    value_ptr run_value = str("none");
    if (run)
      run_value = rec({{"id", str(*run)},
                       {"generation", str("1")},
                       {"dispatch", rec({{"id", str("1")},
                                         {"work_fp", str("")},
                                         {"render_fp", str("")},
                                         {"revision", str("1")},
                                         {"work", acknowledged ? acknowledged : at(w, "work")}})}});
    return rec({{"job", str(job)},
                {"role", str(role)},
                {"kind", at(structure, "kind")},
                {"generation", rec({{"id", str("1")}, {"ordinal", str("1")}, {"structure", structure}})},
                {"state", str(state)},
                {"drifted", flag(drifted)},
                {"fact_version", str("3")},
                {"run", run_value}});
  }
  value_ptr snapshot(const std::string& active_job, std::vector<value_ptr> rows, value_ptr recorded = nullptr) const {
    return rec({{"version", str("1")},
                {"ledger", str(std::string(32, 'a'))},
                {"last_seq", str("7")},
                {"snapshot", rec({{"active_revision", str("1")},
                                  {"active_job", str(active_job)},
                                  {"activation", str(active_job.empty() ? "none" : "1")},
                                  {"revision", rec({{"id", str("1")},
                                                    {"interpretation", recorded ? recorded : interpretation},
                                                    {"versions", rec()},
                                                    {"stdlib_digest", str(std::string(32, 'b'))},
                                                    {"registry", rec()},
                                                    {"source", str("1")}})},
                                  {"roles", arr(std::move(rows))},
                                  {"publications", arr()}})}});
  }
  // Completion-backed policy fixture: each finished worker has exact recorded W and observed own outputs.
  value_ptr completed(value_ptr outer) const {
    auto                   state = at(outer, "snapshot");
    std::vector<value_ptr> completions, observed;
    for (const auto& role : at(state, "roles")->items) {
      if (text(role, "kind") != "worker")
        continue;
      const auto             material = work(text(role, "job"), text(role, "role"));
      std::vector<value_ptr> units;
      for (const auto& unit : at(at(material, "structure"), "work")->items) {
        const auto output = hash_value("test_output", unit);
        REQUIRE(output);
        units.push_back(rec({{"work", unit}, {"output", str(*output)}}));
        observed.push_back(rec({{"work", unit}, {"output", str(*output)}, {"seq", str("5")}}));
      }
      completions.push_back(rec({{"id", str(std::to_string(completions.size() + 1))},
                                 {"job", at(role, "job")},
                                 {"role", at(role, "role")},
                                 {"generation", at(at(role, "generation"), "id")},
                                 {"seq", str("5")},
                                 {"work_fp", at(material, "digest")},
                                 {"units", arr(std::move(units))},
                                 {"vectors", arr()}}));
    }
    state = with(with(state, "completions", arr(std::move(completions))), "observed", arr(std::move(observed)));
    return with(outer, "snapshot", state);
  }
  value_ptr activate(value_ptr snapshot, const std::string& job) const {
    return success("reconcile_execution", rec({{"operation", str("activate")},
                                               {"snapshot", std::move(snapshot)},
                                               {"job", str(job)},
                                               {"request_id", str("act-1")}}));
  }
  value_ptr load(value_ptr snapshot, value_ptr candidate = nullptr) const {
    return success("reconcile_execution", rec({{"operation", str("load")},
                                               {"snapshot", std::move(snapshot)},
                                               {"candidate", candidate ? candidate : interpretation},
                                               {"request_id", str("load-1")}}));
  }
};
// Interpretation with the spec goal replaced: every role's W moves, no structure changes.
value_ptr moved_goal(const value_ptr& interpretation) {
  auto bundle = at(interpretation, "bundle");
  auto spec   = with(at(bundle, "spec"), "goal", str("A different goal."));
  return with(interpretation, "bundle", with(bundle, "spec", spec));
}
struct transition_view {
  std::string                job, role, after, reason;
  std::optional<std::string> before;
  bool                       before_drifted = false, after_drifted = false;
};
std::vector<transition_view> transitions(const value_ptr& change) {
  std::vector<transition_view> out;
  for (const auto& t : at(change, "transitions")->items) {
    transition_view v;
    v.job           = text(t, "job");
    v.role          = text(t, "role");
    v.after         = text(t, "after_state");
    v.reason        = text(t, "reason");
    v.after_drifted = at(t, "after_drifted")->boolean;
    if (t->fields.contains("before_state")) {
      v.before         = text(t, "before_state");
      v.before_drifted = at(t, "before_drifted")->boolean;
    } else {
      CHECK_FALSE(t->fields.contains("before_drifted"));
    }
    out.push_back(v);
  }
  return out;
}
std::vector<std::string> kinds(const value_ptr& change) {
  std::vector<std::string> out;
  for (const auto& e : at(change, "expected")->items)
    out.push_back(text(e, "kind"));
  return out;
}
std::vector<std::string> fenced(const value_ptr& change) {
  std::vector<std::string> out;
  for (const auto& r : at(at(change, "facts"), "fenced_runs")->items)
    out.push_back(r->string);
  return out;
}
} // namespace

TEST_CASE("reconcile activate first time opens activated generations with NULL before states", "[reconcile]") {
  quota q;
  auto  change = q.activate(q.snapshot("", {}), "split");
  CHECK(text(change, "operation") == "activate");
  CHECK(text(change, "snapshot_seq") == "7"); // L16: snapshot_seq is the snapshot's last_seq.
  CHECK(at(change, "version")->integer == 1);
  auto t = transitions(change);
  REQUIRE(t.size() == 3);
  for (const auto& row : t) {
    CHECK_FALSE(row.before.has_value());
    CHECK(row.reason == "activated");
    CHECK(row.job == "split");
  }
  CHECK(t[0].role == "cli_worker");
  CHECK(t[0].after == "ready");
  CHECK(t[1].role == "library_worker");
  CHECK(t[1].after == "ready");
  CHECK(t[2].role == "reviewer");
  CHECK(t[2].after == "blocked"); // A reviewer starts after every worker completes.
  auto facts = at(change, "facts");
  CHECK(text(facts, "job") == "split");
  REQUIRE(at(facts, "generations")->items.size() == 3);
  for (const auto& g : at(facts, "generations")->items) {
    CHECK(text(g, "reason") == "activated");
    CHECK(text(at(g, "structure"), "job") == "split");
  }
  CHECK(fenced(change).empty());
  CHECK(at(change, "vector")->items.size() == 3);
  // EX-3 set (DD29): request_unused, active_revision and active_job; roles of the job being activated assert members only
  // when they already have a row, so a first activation carries no role members.
  CHECK(kinds(change) == std::vector<std::string>{"request_unused", "active_revision", "active_job"});
  auto expected = at(change, "expected")->items;
  CHECK(text(expected[0], "request_id") == "act-1");
  CHECK(text(expected[1], "revision") == "1");
  CHECK(text(expected[2], "job") == "");
  for (std::size_t i = 3; i < expected.size(); ++i) {
    CHECK(at(expected[i], "no_row")->boolean);
    CHECK_FALSE(expected[i]->fields.contains("state"));
  }
}

TEST_CASE("reconcile activate another job retires the previous job and fences its runs", "[reconcile]") {
  quota q;
  auto  snapshot =
      q.snapshot("combined", {q.row("combined", "builder", "running", false, "4"), q.row("combined", "reviewer", "blocked")});
  auto change = q.activate(snapshot, "split");
  auto t      = transitions(change);
  REQUIRE(t.size() == 5);
  CHECK(t[0].job == "combined");
  CHECK(t[0].role == "builder");
  CHECK(t[0].before == "running");
  CHECK(t[0].after == "retired");
  CHECK(t[0].reason == "fenced"); // M6
  CHECK(t[1].role == "reviewer");
  CHECK(t[1].before == "blocked");
  CHECK(t[1].after == "retired");
  CHECK(t[1].reason == "retired"); // M5
  for (std::size_t i = 2; i < t.size(); ++i) {
    CHECK(t[i].job == "split");
    CHECK_FALSE(t[i].before.has_value());
  }
  CHECK(fenced(change) == std::vector<std::string>{"4"});
  CHECK(text(at(change, "facts"), "job") == "split");
  for (const auto& v : at(change, "vector")->items)
    CHECK(text(v, "job") == "split");
  // fact_version and role_state for both rows of the active job; the split roles have no rows and assert nothing (DD29).
  CHECK(kinds(change) == std::vector<std::string>{"request_unused", "active_revision", "active_job", "fact_version", "role_state",
                                                  "fact_version", "role_state"});
  auto expected = at(change, "expected")->items;
  CHECK(text(expected[2], "job") == "combined");
  CHECK(text(expected[3], "fact_version") == "3");
  CHECK(text(expected[4], "state") == "running");
  CHECK(text(expected[4], "run") == "4");
  CHECK(text(expected[6], "run") == "none");
}

TEST_CASE("reconcile re-activation fences every run and reopens every role without retiring", "[reconcile]") {
  quota q;
  auto  snapshot =
      q.snapshot("combined", {q.row("combined", "builder", "running", true, "9"), q.row("combined", "reviewer", "blocked")});
  auto change = q.activate(snapshot, "combined");
  auto t      = transitions(change);
  REQUIRE(t.size() == 2);
  CHECK(t[0].before == "running");
  CHECK(t[0].before_drifted);
  CHECK(t[0].after == "ready");
  CHECK_FALSE(t[0].after_drifted);
  CHECK(t[0].reason == "activated"); // M8 with run N5
  CHECK(t[1].before == "blocked");
  CHECK(t[1].after == "blocked");
  CHECK(t[1].reason == "activated"); // M7
  CHECK(fenced(change) == std::vector<std::string>{"9"});
  CHECK(at(at(change, "facts"), "generations")->items.size() == 2);
  for (const auto& v : at(change, "vector")->items)
    CHECK(text(v, "run") == "none");
}

TEST_CASE("reconcile activate refuses a job the active revision does not declare", "[reconcile]") {
  quota q;
  auto  result = run_trusted("reconcile_execution",
                             rec({{"operation", str("activate")}, {"snapshot", q.snapshot("", {})}, {"job", str("absent")}}));
  REQUIRE(result);
  REQUIRE((*result)->fields.contains("error"));
  auto e = at(*result, "error");
  CHECK(text(e, "code") == "reference");
  CHECK(text(e, "message") == "Unknown selected job");
  CHECK(text(e, "field") == "selection.job");
}

TEST_CASE("reconcile load sets and clears drift from the acknowledged W in place", "[reconcile]") {
  quota q;
  auto  current = at(q.work("split", "cli_worker"), "work");
  auto  old     = with(current, "goal", str("An earlier goal."));
  // Unchanged W: no transition at all.
  auto same = q.load(q.snapshot("split", {q.row("split", "cli_worker", "running", false, "5"),
                                          q.row("split", "library_worker", "ready"), q.row("split", "reviewer", "blocked")}));
  CHECK(transitions(same).empty());
  CHECK(at(at(same, "facts"), "generations")->items.empty());
  // Acknowledged W differs from the current W: running -> running+drift (M23).
  auto drift = q.load(q.snapshot("split", {q.row("split", "cli_worker", "running", false, "5", old),
                                           q.row("split", "library_worker", "ready"), q.row("split", "reviewer", "blocked")}));
  auto t     = transitions(drift);
  REQUIRE(t.size() == 1);
  CHECK(t[0].role == "cli_worker");
  CHECK(t[0].after == "running");
  CHECK(t[0].after_drifted);
  CHECK(t[0].reason == "work_changed");
  CHECK(fenced(drift).empty());
  bool found = false;
  for (const auto& v : at(drift, "vector")->items)
    if (text(v, "role") == "cli_worker") {
      found = true;
      CHECK(text(v, "run") == "5");
      CHECK(at(v, "drifted")->boolean);
    }
  CHECK(found);
  // A drifted role whose acknowledged W is current again: drift_cleared (M25).
  auto cleared = q.load(q.snapshot("split", {q.row("split", "cli_worker", "running", true, "5"),
                                             q.row("split", "library_worker", "ready"), q.row("split", "reviewer", "blocked")}));
  t            = transitions(cleared);
  REQUIRE(t.size() == 1);
  CHECK(t[0].before_drifted);
  CHECK_FALSE(t[0].after_drifted);
  CHECK(t[0].reason == "drift_cleared");
}

TEST_CASE("reconcile load structural change fences the run and opens an empty generation", "[reconcile]") {
  quota q;
  // The stored structure of the builder covered only library; the candidate's builder covers both units.
  auto stored = at(q.work("combined", "builder"), "structure");
  stored      = with(stored, "work", arr({str("library")}));
  stored      = with(stored, "ownership", arr({at(stored, "ownership")->items[1]}));
  auto change = q.load(q.snapshot("combined", {q.row("combined", "builder", "running", false, "6", nullptr, stored),
                                               q.row("combined", "reviewer", "blocked")}));
  auto t      = transitions(change);
  REQUIRE(t.size() == 1);
  CHECK(t[0].role == "builder");
  CHECK(t[0].before == "running");
  CHECK(t[0].after == "ready");
  CHECK(t[0].reason == "structural_change"); // M12, N6
  CHECK(fenced(change) == std::vector<std::string>{"6"});
  auto generations = at(at(change, "facts"), "generations")->items;
  REQUIRE(generations.size() == 1);
  CHECK(text(generations[0], "reason") == "structural_change");
  CHECK(at(at(generations[0], "structure"), "work")->items.size() == 2);
}

TEST_CASE("reconcile load retires a removed role and recreates a re-added one", "[reconcile]") {
  quota q;
  auto  old_worker = q.row("split", "library_worker", "running", false, "8");
  old_worker       = with(old_worker, "role", str("old_worker"));
  auto change = q.load(q.snapshot("split", {q.row("split", "cli_worker", "retired"), q.row("split", "library_worker", "ready"),
                                            old_worker, q.row("split", "reviewer", "blocked")}));
  auto t      = transitions(change);
  REQUIRE(t.size() == 2);
  CHECK(t[0].role == "cli_worker");
  CHECK(t[0].before == "retired");
  CHECK(t[0].after == "ready");
  CHECK(t[0].reason == "recreated"); // M16
  CHECK(t[1].role == "old_worker");
  CHECK(t[1].before == "running");
  CHECK(t[1].after == "retired");
  CHECK(t[1].reason == "fenced"); // M14 cause (a), N7
  CHECK(fenced(change) == std::vector<std::string>{"8"});
  auto generations = at(at(change, "facts"), "generations")->items;
  REQUIRE(generations.size() == 1);
  CHECK(text(generations[0], "reason") == "recreated");
  // The vector keeps every row of the job, the removed role as retired.
  CHECK(at(change, "vector")->items.size() == 4);
}

TEST_CASE("reconcile load with the active job absent retires it and clears the active job (D94)", "[reconcile]") {
  quota q;
  auto  gone_running = with(q.row("split", "library_worker", "running", false, "3"), "job", str("gone"));
  auto  gone_ready   = with(with(q.row("split", "cli_worker", "ready"), "job", str("gone")), "role", str("helper"));
  auto  gone_retired = with(with(q.row("split", "reviewer", "retired"), "job", str("gone")), "role", str("old"));
  auto  change       = q.load(q.snapshot("gone", {gone_running, gone_ready, gone_retired}));
  auto  t            = transitions(change);
  REQUIRE(t.size() == 2);
  CHECK(t[0].role == "helper");
  CHECK(t[0].reason == "retired"); // M13 cause (b)
  CHECK(t[1].role == "library_worker");
  CHECK(t[1].reason == "fenced"); // M14 cause (b)
  CHECK(text(at(change, "facts"), "job") == "");
  CHECK(at(change, "vector")->items.empty());
  CHECK(fenced(change) == std::vector<std::string>{"3"});
  // Every row of the formerly active job is asserted, the already retired one included.
  CHECK(kinds(change).size() == 3 + 2 * 3);
}

TEST_CASE("reconcile load with no active job changes no role", "[reconcile]") {
  quota q;
  auto  change = q.load(q.snapshot("", {}));
  CHECK(transitions(change).empty());
  CHECK(at(change, "vector")->items.empty());
  CHECK(text(at(change, "facts"), "job") == "");
  CHECK(kinds(change) == std::vector<std::string>{"request_unused", "active_revision", "active_job"});
}

TEST_CASE("reconcile precedence orders retired, running, blocked, done, stale and ready", "[reconcile]") {
  quota q;
  SECTION("blocked wins over done and a running role with a new blocker stays running") {
    // Workers are not done, so the reviewer's start prerequisite is absent.
    auto change = q.load(q.snapshot("split", {q.row("split", "cli_worker", "ready"), q.row("split", "library_worker", "ready"),
                                              q.row("split", "reviewer", "done")}));
    auto t      = transitions(change);
    REQUIRE(t.size() == 1);
    CHECK(t[0].before == "done");
    CHECK(t[0].after == "blocked");
    CHECK(t[0].reason == "prerequisite_unavailable"); // M18
    auto running = q.load(q.snapshot("split", {q.row("split", "cli_worker", "ready"), q.row("split", "library_worker", "ready"),
                                               q.row("split", "reviewer", "running", false, "2")}));
    CHECK(transitions(running).empty());
  }
  SECTION("material load stales completed workers while reviewer remains blocked") {
    const auto candidate = moved_goal(q.interpretation);
    const auto initial =
        q.completed(q.snapshot("split", {q.row("split", "cli_worker", "done"), q.row("split", "library_worker", "done"),
                                         q.row("split", "reviewer", "blocked")}));
    auto t = transitions(q.load(initial, candidate));
    REQUIRE(t.size() == 2);
    CHECK(t[0].role == "cli_worker");
    CHECK(t[0].after == "stale");
    CHECK(t[0].reason == "work_changed");
    CHECK(t[1].role == "library_worker");
    CHECK(t[1].after == "stale");
    CHECK(t[1].reason == "work_changed");
    // Restoring the recorded material reuses the inactive completions and releases review in the same load.
    auto restored = q.completed(q.snapshot(
        "split",
        {q.row("split", "cli_worker", "stale"), q.row("split", "library_worker", "stale"), q.row("split", "reviewer", "blocked")},
        candidate));
    t             = transitions(q.load(restored));
    REQUIRE(t.size() == 3);
    CHECK(t[0].after == "done");
    CHECK(t[1].after == "done");
    CHECK(t[2].role == "reviewer");
    CHECK(t[2].after == "ready");
    CHECK(t[2].reason == "prerequisite_available");
  }
  SECTION("done stays done when W does not move") {
    auto change = q.load(q.snapshot("split", {q.row("split", "cli_worker", "done"), q.row("split", "library_worker", "done"),
                                              q.row("split", "reviewer", "ready")}));
    CHECK(transitions(change).empty());
  }
}

TEST_CASE("reconcile W is stable under takeover-only generation changes", "[reconcile]") {
  quota q;
  // A takeover opens a generation with the same structure: an acknowledged W from the earlier generation stays current.
  auto row = q.row("split", "cli_worker", "running", false, "5");
  auto run = with(at(row, "run"), "generation", str("2"));
  row      = with(row, "run", run);
  row      = with(row, "generation", with(at(row, "generation"), "ordinal", str("2")));
  auto change =
      q.load(q.snapshot("split", {row, q.row("split", "library_worker", "ready"), q.row("split", "reviewer", "blocked")}));
  CHECK(transitions(change).empty());
}

TEST_CASE("reconcile is byte-identical across fresh VMs", "[reconcile]") {
  quota q;
  auto  snapshot =
      q.snapshot("combined", {q.row("combined", "builder", "running", false, "4"), q.row("combined", "reviewer", "blocked")});
  auto first  = q.activate(snapshot, "split");
  auto second = q.activate(snapshot, "split");
  CHECK(digest(first) == digest(second));
  auto encoded_first  = codec_encode(first);
  auto encoded_second = codec_encode(second);
  REQUIRE(encoded_first);
  REQUIRE(encoded_second);
  CHECK(*encoded_first == *encoded_second);
}
