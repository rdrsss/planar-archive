// read_execution snapshot value tests (execution-contract P2-1, R15-B L18, advisory A1). Hand-built snapshots exercise
// the pure mapping; one fresh format3 ledger under a per-test mkdtemp root checks the value of a real read.
#include <catch2/catch_test_macros.hpp>
#include <stdlib.h>
#include <unistd.h>
import std;
import planar.engine.canonical;
import planar.engine.value;
import planar.engine.ledger.execution;
import planar.engine.ledger.execution_schema;
import planar.engine.execution.snapshot_value;

using namespace planar::engine;
using planar::engine::execution::snapshot_value;
namespace ledger = planar::engine::ledger;
namespace hist   = planar::engine::history;

namespace {

struct scratch {
  std::filesystem::path root;
  scratch() {
    char  pattern[] = "/tmp/planar-snapshot-value-XXXXXX";
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

value_ptr vrec(const std::vector<std::pair<std::string, value_ptr>>& entries) {
  value out{};
  out.kind = value_kind::record;
  for (const auto& [key, item] : entries)
    out.fields[key] = item;
  return std::make_shared<const value>(std::move(out));
}

const value& at(const value_ptr& node, std::string_view key) {
  REQUIRE(node != nullptr);
  REQUIRE(node->kind == value_kind::record);
  const auto found = node->fields.find(std::string(key));
  REQUIRE(found != node->fields.end());
  return *found->second;
}

value_ptr field(const value_ptr& node, std::string_view key) {
  REQUIRE(node != nullptr);
  const auto found = node->fields.find(std::string(key));
  REQUIRE(found != node->fields.end());
  return found->second;
}

std::set<std::string> keys_of(const value_ptr& node) {
  std::set<std::string> out;
  for (const auto& [key, unused] : node->fields)
    out.insert(key);
  return out;
}

void collect_keys(const value_ptr& node, std::set<std::string>& out) {
  if (!node)
    return;
  for (const auto& [key, child] : node->fields) {
    out.insert(key);
    collect_keys(child, out);
  }
  for (const auto& item : node->items)
    collect_keys(item, out);
}

hist::role_structure_v1 worker(std::string job, std::string role) {
  hist::role_structure_v1 out;
  out.job   = std::move(job);
  out.role  = std::move(role);
  out.kind  = "worker";
  out.work  = {"unit_one"};
  out.shape = "linear";
  return out;
}

/// A snapshot at revision 10 with a ready role and a running role whose dispatch was rendered at revision 9.
ledger::execution_snapshot running_snapshot() {
  ledger::execution_snapshot snapshot;
  snapshot.identity   = std::string(32, 'a');
  snapshot.last_seq   = 42;
  snapshot.active_job = "alpha";
  snapshot.activation = 3;
  ledger::execution_revision revision;
  revision.id                   = 10;
  revision.source               = 11;
  revision.registry             = 12;
  revision.stdlib               = 13;
  revision.interpretation       = 14;
  revision.versions             = 15;
  revision.source_digest        = std::string(32, 'b');
  revision.stdlib_digest        = std::string(32, 'c');
  revision.interpretation_value = vrec({{"tag", vstr("one")}});
  revision.versions_value       = vrec({{"stdlib_digest", vstr(std::string(32, 'c'))}});
  revision.registry_value       = vrec({{"id", vstr("12")}, {"digest", vstr(std::string(32, 'd'))}});
  snapshot.revision             = revision;

  ledger::execution_role ready;
  ready.job                = "alpha";
  ready.role               = "builder";
  ready.generation_id      = 5;
  ready.generation_ordinal = 1;
  ready.structure          = worker("alpha", "builder");
  ready.state              = "ready";
  ready.fact_version       = 2;
  ready.last_seq           = 40;

  ledger::execution_role running;
  running.job                = "alpha";
  running.role               = "tester";
  running.generation_id      = 7;
  running.generation_ordinal = 3;
  running.structure          = worker("alpha", "tester");
  running.state              = "running";
  running.drifted            = true;
  running.run_id             = 21;
  running.fact_version       = 4;
  running.last_seq           = 41;
  ledger::execution_dispatch dispatch;
  dispatch.id        = 30;
  dispatch.kind      = "claim";
  dispatch.work_fp   = std::string(32, 'e');
  dispatch.render_fp = std::string(32, 'f');
  dispatch.revision  = 9;
  dispatch.work      = vrec({{"job", vstr("alpha")}, {"role", vstr("tester")}});
  running.dispatch   = dispatch;

  snapshot.roles = {ready, running};
  return snapshot;
}

} // namespace

TEST_CASE("execution snapshot value of a fresh format3 ledger") {
  scratch    dir;
  const auto path = dir.root / "ledger.planar";
  REQUIRE(ledger::initialize_format3(path, "alpha_workspace").has_value());
  auto snapshot = ledger::read_execution_snapshot(path);
  REQUIRE(snapshot.has_value());
  auto result = snapshot_value(*snapshot);
  REQUIRE(result.has_value());
  REQUIRE(keys_of(*result) == std::set<std::string>{"version", "ledger", "last_seq", "snapshot"});
  REQUIRE(at(*result, "version").kind == value_kind::integer);
  REQUIRE(at(*result, "version").integer == 1);
  REQUIRE(at(*result, "ledger").string == snapshot->identity);
  REQUIRE(at(*result, "last_seq").string == "1");
  const auto inner = field(*result, "snapshot");
  REQUIRE(keys_of(inner) ==
          std::set<std::string>{"active_revision", "active_job", "activation", "revision", "roles", "publications"});
  REQUIRE(at(inner, "active_revision").string == "none");
  REQUIRE(at(inner, "active_job").string.empty());
  REQUIRE(at(inner, "activation").string == "none");
  REQUIRE(at(inner, "revision").string == "none");
  REQUIRE(at(inner, "roles").kind == value_kind::list);
  REQUIRE(at(inner, "roles").items.empty());
  REQUIRE(at(inner, "publications").kind == value_kind::list);
  REQUIRE(at(inner, "publications").items.empty());
}

TEST_CASE("execution snapshot value spells roles, runs and dispatches with the L18 spellings") {
  const auto snapshot = running_snapshot();
  auto       result   = snapshot_value(snapshot);
  REQUIRE(result.has_value());
  const auto inner = field(*result, "snapshot");
  REQUIRE(at(inner, "active_revision").string == "10");
  REQUIRE(at(inner, "active_job").string == "alpha");
  REQUIRE(at(inner, "activation").string == "3");
  const auto& roles = at(inner, "roles").items;
  REQUIRE(roles.size() == 2);

  const auto& ready = roles[0];
  REQUIRE(keys_of(ready) ==
          std::set<std::string>{"job", "role", "kind", "generation", "state", "drifted", "fact_version", "run"});
  REQUIRE(at(ready, "role").string == "builder");
  REQUIRE(at(ready, "kind").string == "worker");
  REQUIRE(at(ready, "run").string == "none");
  REQUIRE(at(ready, "fact_version").string == "2");
  REQUIRE(at(ready, "drifted").kind == value_kind::boolean);
  const auto generation = field(ready, "generation");
  REQUIRE(keys_of(generation) == std::set<std::string>{"id", "ordinal", "structure"});
  REQUIRE(at(generation, "id").string == "5");
  REQUIRE(at(generation, "ordinal").string == "1");
  auto structure_bytes = canonical_encode(field(generation, "structure"));
  auto codec_bytes     = hist::encode_role_structure_v1(snapshot.roles[0].structure);
  REQUIRE(structure_bytes.has_value());
  REQUIRE(codec_bytes.has_value());
  REQUIRE(*structure_bytes == *codec_bytes);

  const auto& running = roles[1];
  REQUIRE(at(running, "state").string == "running");
  REQUIRE(at(running, "drifted").boolean);
  const auto run = field(running, "run");
  REQUIRE(keys_of(run) == std::set<std::string>{"id", "generation", "dispatch"});
  REQUIRE(at(run, "id").string == "21");
  // L18: run.generation is the generation ordinal (3), never the generation identifier (7).
  REQUIRE(at(run, "generation").string == "3");
  const auto dispatch = field(run, "dispatch");
  REQUIRE(keys_of(dispatch) == std::set<std::string>{"id", "work_fp", "render_fp", "revision", "work"});
  REQUIRE(at(dispatch, "id").string == "30");
  // A1: the dispatch names its own revision (9), not the active revision (10).
  REQUIRE(at(dispatch, "revision").string == "9");
  REQUIRE(at(dispatch, "work_fp").string == std::string(32, 'e'));
  REQUIRE(field(dispatch, "work") == snapshot.roles[1].dispatch->work);
}

TEST_CASE("execution snapshot value carries the decoded recorded revision values") {
  const auto snapshot = running_snapshot();
  auto       result   = snapshot_value(snapshot);
  REQUIRE(result.has_value());
  const auto revision = field(field(*result, "snapshot"), "revision");
  REQUIRE(keys_of(revision) == std::set<std::string>{"id", "interpretation", "versions", "stdlib_digest", "registry", "source"});
  REQUIRE(at(revision, "id").string == "10");
  REQUIRE(field(revision, "interpretation") == snapshot.revision->interpretation_value);
  REQUIRE(field(revision, "versions") == snapshot.revision->versions_value);
  REQUIRE(field(revision, "registry") == snapshot.revision->registry_value);
  REQUIRE(at(revision, "stdlib_digest").string == std::string(32, 'c'));
  const auto source = field(revision, "source");
  REQUIRE(at(source, "id").string == "11");
  REQUIRE(at(source, "digest").string == std::string(32, 'b'));
}

TEST_CASE("execution snapshot value never carries a secret or token and agrees with the D24 measure") {
  const auto snapshot = running_snapshot();
  auto       result   = snapshot_value(snapshot);
  REQUIRE(result.has_value());
  std::set<std::string> keys;
  collect_keys(*result, keys);
  REQUIRE_FALSE(keys.contains("secret"));
  REQUIRE_FALSE(keys.contains("token"));
  // The D24 value S is this snapshot value with the revision reduced to {id}; its byte length is the native measure.
  auto inner                = std::make_shared<value>(*field(*result, "snapshot"));
  inner->fields["revision"] = vrec({{"id", vstr("10")}});
  auto bytes                = canonical_encode(inner);
  REQUIRE(bytes.has_value());
  REQUIRE(bytes->size() == ledger::execution_state_bytes(snapshot));
}

TEST_CASE("execution snapshot value is byte stable across builds") {
  const auto snapshot = running_snapshot();
  auto       first    = snapshot_value(snapshot);
  auto       second   = snapshot_value(snapshot);
  REQUIRE(first.has_value());
  REQUIRE(second.has_value());
  REQUIRE(canonical_encode(*first).value() == canonical_encode(*second).value());
}
