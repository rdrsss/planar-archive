// Format3 execution tests: snapshot reads, full verification, exact request replay, run tokens and the commit protocol.
// Every ledger lives under a per-test mkdtemp root. Oracles read rows through a raw planar.engine.ledger.sqlite
// connection and never through product readers. Corrupting tests drop every trigger, apply the corruption and recreate
// each trigger with its stored text (D43). Commit fixtures are driven through commit_execution only. No test sleeps,
// forks, uses threads or signals, or sets resource limits.
//
// V3 has no test by design: the run secret never enters the snapshot is guaranteed by the struct having no such member
// (DECISIONS L5). The row set is V1, V2, V4 to V47 and K1 to K58, so this file held 104 test cases before the repair round, which
// adds 15 (119).
#include <catch2/catch_test_macros.hpp>
#include <stdlib.h>
#include <unistd.h>
import std;
import planar.engine.canonical;
import planar.engine.hash;
import planar.engine.json_codec;
import planar.engine.value;
import planar.engine.history.execution_codec;
import planar.engine.history.read;
import planar.engine.history.record;
import planar.engine.history.request_v2;
import planar.engine.history.request_v3;
import planar.engine.ledger.execution;
import planar.engine.ledger.execution_schema;
import planar.engine.ledger.format;
import planar.engine.ledger.history_codec;
import planar.engine.ledger.sqlite;

using namespace planar::engine::ledger;
using planar::engine::canonical_decode;
using planar::engine::fragment_digest;
using planar::engine::value;
using planar::engine::value_kind;
using planar::engine::value_ptr;
namespace hist = planar::engine::history;

namespace {

using kind_t = execution_expectation::kind_t;

constexpr std::string_view workspace_name = "alpha_workspace";
constexpr std::uint64_t    manifest_limit = 16777216;
constexpr std::uint64_t    state_limit    = 16777216;

/// Per-test scratch root, created with mkdtemp and removed afterwards.
struct scratch {
  std::filesystem::path root;
  scratch() {
    char  pattern[] = "/tmp/planar-execution-test-XXXXXX";
    char* made      = ::mkdtemp(pattern);
    REQUIRE(made != nullptr);
    root = std::filesystem::canonical(made);
  }
  ~scratch() {
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
  }
  std::filesystem::path ledger() const {
    return root / "ledger.planar";
  }
};

// ---------------------------------------------------------------------------------------------------------------------
// Raw SQL access (independent oracles and corruption).

connection open_raw(const std::filesystem::path& path) {
  auto db = connection::open(path, open_mode::read_write);
  REQUIRE(db.has_value());
  return std::move(*db);
}

void run_sql(connection& db, std::string_view sql) {
  auto stmt = db.prepare(sql);
  REQUIRE(stmt.has_value());
  auto step = stmt->step();
  REQUIRE(step.has_value());
  REQUIRE(*step == step_result::done);
}

std::int64_t int_of(connection& db, std::string_view sql) {
  auto stmt = db.prepare(sql);
  REQUIRE(stmt.has_value());
  auto step = stmt->step();
  REQUIRE(step.has_value());
  REQUIRE(*step == step_result::row);
  auto value_column = stmt->integer(0);
  REQUIRE(value_column.has_value());
  REQUIRE(value_column->has_value());
  return **value_column;
}

std::string text_of(connection& db, std::string_view sql) {
  auto stmt = db.prepare(sql);
  REQUIRE(stmt.has_value());
  auto step = stmt->step();
  REQUIRE(step.has_value());
  REQUIRE(*step == step_result::row);
  auto value_column = stmt->text(0);
  REQUIRE(value_column.has_value());
  REQUIRE(value_column->has_value());
  return **value_column;
}

std::string blob_of(connection& db, std::string_view sql) {
  auto stmt = db.prepare(sql);
  REQUIRE(stmt.has_value());
  auto step = stmt->step();
  REQUIRE(step.has_value());
  REQUIRE(*step == step_result::row);
  auto value_column = stmt->blob(0);
  REQUIRE(value_column.has_value());
  REQUIRE(value_column->has_value());
  return **value_column;
}

std::int64_t count_rows(connection& db, std::string_view table) {
  return int_of(db, std::format("SELECT COUNT(*) FROM {}", table));
}

std::vector<std::pair<std::string, std::string>> trigger_texts(connection& db) {
  std::vector<std::pair<std::string, std::string>> out;
  auto stmt = db.prepare("SELECT name, sql FROM sqlite_master WHERE type='trigger' ORDER BY name");
  REQUIRE(stmt.has_value());
  while (true) {
    auto step = stmt->step();
    REQUIRE(step.has_value());
    if (*step == step_result::done)
      break;
    auto name = stmt->text(0);
    auto sql  = stmt->text(1);
    REQUIRE(name.has_value());
    REQUIRE(sql.has_value());
    REQUIRE(name->has_value());
    REQUIRE(sql->has_value());
    out.emplace_back(**name, **sql);
  }
  return out;
}

/// Apply corrupting SQL with every trigger dropped, then recreate each trigger with its stored text (D43). The
/// corrupting statements update every dependent column they need, so the named invariant is the only one they break.
void corrupt(const std::filesystem::path& path, const std::vector<std::string>& statements) {
  auto db       = open_raw(path);
  auto triggers = trigger_texts(db);
  run_sql(db, "PRAGMA foreign_keys=OFF");
  run_sql(db, "PRAGMA ignore_check_constraints=ON");
  for (const auto& [name, sql] : triggers) {
    (void)sql;
    run_sql(db, std::format("DROP TRIGGER \"{}\"", name));
  }
  for (const auto& statement : statements)
    run_sql(db, statement);
  for (const auto& [name, sql] : triggers) {
    (void)name;
    run_sql(db, sql);
  }
}

// ---------------------------------------------------------------------------------------------------------------------
// Values and canonical bytes.

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

value_ptr vstrs(const std::vector<std::string>& items) {
  std::vector<value_ptr> out;
  for (const auto& item : items)
    out.push_back(vstr(item));
  return vlist(out);
}

std::string bytes_of(const value_ptr& input) {
  auto encoded = canonical_encode(input);
  REQUIRE(encoded.has_value());
  return *encoded;
}

std::string params_bytes(const std::vector<std::pair<std::string, std::string>>& fields) {
  std::vector<std::pair<std::string, value_ptr>> entries;
  for (const auto& [key, text] : fields)
    entries.emplace_back(key, vstr(text));
  return bytes_of(vrec(entries));
}

std::string decimal(std::uint64_t number) {
  return std::to_string(number);
}

// ---------------------------------------------------------------------------------------------------------------------
// Records and revision seeding (format2-shaped revision records, D23 and D43).

struct stored {
  std::uint64_t       id = 0;
  hist::record_digest digest{};
};

struct child {
  std::string       name;
  hist::record_kind kind;
  stored            node;
};

std::string digest_text(const hist::record_digest& digest) {
  return std::string(reinterpret_cast<const char*>(digest.data()), digest.size());
}

std::string digest_hex(const hist::record_digest& digest) {
  std::string out;
  for (auto byte : digest)
    out += std::format("{:02x}", byte);
  return out;
}

stored store_record(connection& db, const hist::record& rec) {
  auto encoded = hist::encode_record(rec);
  REQUIRE(encoded.has_value());
  auto digest = hist::digest_record(rec);
  REQUIRE(digest.has_value());
  const auto kind = static_cast<std::int64_t>(hist::kind_of(rec));
  auto       stmt = db.prepare("INSERT INTO records(kind,codec,digest,encoded) VALUES(?1,1,?2,?3)");
  REQUIRE(stmt.has_value());
  REQUIRE(stmt->bind_integer(1, kind).has_value());
  REQUIRE(stmt->bind_blob(2, digest_text(*digest)).has_value());
  REQUIRE(stmt->bind_blob(3, *encoded).has_value());
  auto step = stmt->step();
  REQUIRE(step.has_value());
  REQUIRE(*step == step_result::done);
  return stored{static_cast<std::uint64_t>(int_of(db, "SELECT last_insert_rowid()")), *digest};
}

stored store_directory(connection& db, const std::vector<child>& children) {
  hist::directory_record directory;
  for (const auto& entry : children)
    directory.children.push_back(hist::record_child{entry.name, entry.kind, entry.node.id, entry.node.digest});
  const auto   node    = store_record(db, hist::record{directory});
  std::int64_t ordinal = 0;
  for (const auto& entry : children) {
    auto stmt = db.prepare("INSERT INTO tree_edges(parent_id,ordinal,name,child_id) VALUES(?1,?2,?3,?4)");
    REQUIRE(stmt.has_value());
    REQUIRE(stmt->bind_integer(1, static_cast<std::int64_t>(node.id)).has_value());
    REQUIRE(stmt->bind_integer(2, ordinal).has_value());
    REQUIRE(stmt->bind_text(3, entry.name).has_value());
    REQUIRE(stmt->bind_integer(4, static_cast<std::int64_t>(entry.node.id)).has_value());
    auto step = stmt->step();
    REQUIRE(step.has_value());
    REQUIRE(*step == step_result::done);
    ++ordinal;
  }
  return node;
}

stored store_file(connection& db, std::string_view payload) {
  return store_record(db, hist::record{hist::file_record{.mode = 420, .payload = std::string(payload)}});
}

stored store_value(connection& db, const value_ptr& input) {
  return store_record(db, hist::record{hist::value_record{.codec = "canonical_v1", .encoded = bytes_of(input)}});
}

/// Interpretation value of a seeded revision: the tag, the embedded versions and the bundle workspace (L2 residuals).
value_ptr seeded_interpretation(std::string_view tag, const value_ptr& versions, std::string_view workspace = workspace_name) {
  return vrec({{"tag", vstr(std::string(tag))},
               {"bundle", vrec({{"workspace", vrec({{"id", vstr(std::string(workspace))}})}})},
               {"versions", versions}});
}

/// Identifiers of the five immutable records one revision names.
struct revision_ids {
  std::uint64_t source         = 0;
  std::uint64_t registry       = 0;
  std::uint64_t stdlib         = 0;
  std::uint64_t interpretation = 0;
  std::uint64_t versions       = 0;
};

/// Insert the five format2-shaped records of one revision. Stdlib content is constant across seeds, so every seed
/// carries the same stdlib digest; the source and interpretation carry the tag so distinct seeds differ.
revision_ids seed_revision(const std::filesystem::path& path, std::string_view tag, bool with_facet = false) {
  auto db          = open_raw(path);
  auto lib_file    = store_file(db, "stdlib body v1");
  auto stdlib      = store_directory(db, {child{"stdlib.lua", hist::record_kind::file, lib_file}});
  auto source_file = store_file(db, std::format("source {}", tag));
  auto source      = store_directory(db, {child{"source.txt", hist::record_kind::file, source_file}});
  auto input_file  = store_file(db, std::format("{{\"tag\":\"{}\"}}", tag));
  auto registry    = store_directory(db, {child{"input.json", hist::record_kind::file, input_file}});
  // L2 residuals: the interpretation embeds the versions value and names the ledger workspace.
  const auto versions_value = vrec({{"stdlib_digest", vstr(digest_hex(stdlib.digest))}});
  auto       interpreted    = seeded_interpretation(tag, versions_value);
  if (with_facet) {
    auto value             = std::make_shared<planar::engine::value>(*interpreted);
    auto bundle            = std::make_shared<planar::engine::value>(*value->fields.at("bundle"));
    bundle->fields["spec"] = vrec(
        {{"id", vstr("spec_one")},
         {"facets", vlist({vrec({{"id", vstr("api")}, {"producer", vstr("unit_one")}, {"paths", vstrs({"include/api.h"})}})})}});
    value->fields["bundle"] = bundle;
    interpreted             = value;
  }
  auto interpretation = store_value(db, interpreted);
  auto versions       = store_value(db, versions_value);
  return revision_ids{source.id, registry.id, stdlib.id, interpretation.id, versions.id};
}

hist::role_structure_v1 structure_of(std::string_view job, std::string_view role, std::string_view kind,
                                     std::string_view unit = "unit_one") {
  hist::role_structure_v1 out;
  out.job   = std::string(job);
  out.role  = std::string(role);
  out.kind  = std::string(kind);
  out.shape = "linear";
  if (kind == "worker")
    out.work.push_back(std::string(unit));
  else
    out.targets.push_back(std::string(unit));
  return out;
}

execution_revision_input revision_input(const revision_ids& ids, bool noop = false) {
  return execution_revision_input{.source         = ids.source,
                                  .registry       = ids.registry,
                                  .stdlib         = ids.stdlib,
                                  .interpretation = ids.interpretation,
                                  .versions       = ids.versions,
                                  .noop           = noop};
}

// ---------------------------------------------------------------------------------------------------------------------
// Snapshot access and result helpers.

execution_snapshot snapshot_of(const std::filesystem::path& path) {
  auto snapshot = read_execution_snapshot(path);
  REQUIRE(snapshot.has_value());
  return *snapshot;
}

std::string head_text(const execution_snapshot& snapshot) {
  return snapshot.revision ? decimal(snapshot.revision->id) : "none";
}

const execution_role* find_role(const execution_snapshot& snapshot, std::string_view job, std::string_view role) {
  for (const auto& row : snapshot.roles)
    if (row.job == job && row.role == role)
      return &row;
  return nullptr;
}

template <typename T> void require_code(const std::expected<T, execution_error>& result, execution_error_code code) {
  REQUIRE_FALSE(result.has_value());
  REQUIRE(result.error().code == code);
}

/// The ledger must verify and read through the product reader before any corruption is applied (D43).
void require_verifies(const std::filesystem::path& path) {
  REQUIRE(verify_execution_ledger(path).has_value());
  REQUIRE(read_execution_snapshot(path).has_value());
}

/// After a corruption, both the verifier and the snapshot reader must refuse with ledger_corrupt.
void expect_corrupt(const std::filesystem::path& path) {
  require_code(verify_execution_ledger(path), execution_error_code::ledger_corrupt);
  require_code(read_execution_snapshot(path), execution_error_code::ledger_corrupt);
}

// ---------------------------------------------------------------------------------------------------------------------
// Precondition members (the closed EX-3 vocabulary as the snapshot spells it).

execution_expectation member(kind_t kind) {
  execution_expectation out;
  out.kind = kind;
  return out;
}

execution_expectation revision_member(const execution_snapshot& snapshot) {
  auto out = member(kind_t::active_revision);
  if (snapshot.revision)
    out.revision = snapshot.revision->id;
  return out;
}

execution_expectation active_job_member(const execution_snapshot& snapshot) {
  auto out = member(kind_t::active_job);
  out.text = snapshot.active_job;
  return out;
}

execution_expectation fact_member(const execution_role& row) {
  auto out   = member(kind_t::fact_version);
  out.job    = row.job;
  out.role   = row.role;
  out.number = row.fact_version;
  return out;
}

execution_expectation state_member(const execution_role& row) {
  auto out    = member(kind_t::role_state);
  out.job     = row.job;
  out.role    = row.role;
  out.state   = row.state;
  out.drifted = row.drifted;
  out.run     = row.run_id;
  return out;
}

execution_expectation generation_member(const execution_role& row) {
  auto out   = member(kind_t::generation_current);
  out.job    = row.job;
  out.role   = row.role;
  out.number = row.generation_id;
  return out;
}

/// The fixed set of a load or activate: the revision, the active job, and for every role of the listed jobs with a row
/// its fact version and role state, then request_unused.
std::vector<execution_expectation> lifecycle_expected(const execution_snapshot& snapshot, const std::vector<std::string>& jobs) {
  auto selected = [&](const execution_role& row) { return std::find(jobs.begin(), jobs.end(), row.job) != jobs.end(); };
  std::vector<execution_expectation> out{revision_member(snapshot), active_job_member(snapshot)};
  for (const auto& row : snapshot.roles)
    if (selected(row))
      out.push_back(fact_member(row));
  for (const auto& row : snapshot.roles)
    if (selected(row))
      out.push_back(state_member(row));
  out.push_back(member(kind_t::request_unused));
  return out;
}

/// The fixed set of a claim or takeover: fact versions of the active job, then role_state and generation of the target.
std::vector<execution_expectation> targeted_expected(const execution_snapshot& snapshot, const execution_role& target) {
  std::vector<execution_expectation> out{revision_member(snapshot), active_job_member(snapshot)};
  for (const auto& row : snapshot.roles)
    if (!snapshot.active_job.empty() && row.job == snapshot.active_job)
      out.push_back(fact_member(row));
  out.push_back(state_member(target));
  out.push_back(generation_member(target));
  out.push_back(member(kind_t::request_unused));
  return out;
}

std::vector<execution_vector_entry> base_vector(const execution_snapshot& snapshot, std::string_view job) {
  std::vector<execution_vector_entry> out;
  if (job.empty())
    return out;
  for (const auto& row : snapshot.roles)
    if (row.job == job)
      out.push_back(execution_vector_entry{
          .job = row.job, .role = row.role, .state = row.state, .drifted = row.drifted, .run = row.run_id});
  return out;
}

// ---------------------------------------------------------------------------------------------------------------------
// Packet fixtures (W, manifest, fingerprints, provenance document).

value_ptr make_work(std::string_view job, std::string_view role, std::size_t goal_pad = 0) {
  return vrec({{"versions", vstrs({"1", "1", "1", "1", "1", "1", "1"})},
               {"workspace", vstr(std::string(workspace_name))},
               {"spec", vstr("spec_one")},
               {"job", vstr(std::string(job))},
               {"role", vstr(std::string(role))},
               {"root", vstr("root")},
               {"base", vstr("base")},
               {"goal", vstr("goal" + std::string(goal_pad, 'g'))},
               {"units", vstrs({"unit_one"})},
               {"milestones", vstrs({})},
               {"constraints", vstrs({})},
               {"prescriptions", vstrs({})},
               {"contracts", vstrs({})},
               {"dependencies", vstrs({})},
               {"facets", vstrs({})},
               {"pipeline_predecessors", vstrs({})},
               {"questions", vstrs({})},
               {"ownership", vstrs({})},
               {"effective_checks", vstrs({})},
               {"checks", vstrs({})},
               {"toolchains", vstrs({})},
               {"shape", vstr("linear")},
               {"start_facets", vstrs({})},
               {"repair", vstr("0")}});
}

/// Packet of a claim or takeover on the snapshot row: the manifest digests equal the native fingerprints.
execution_packet make_packet(const execution_snapshot& snapshot, const execution_role& row, std::string_view request_id,
                             std::string_view kind, std::optional<std::uint64_t> previous, std::size_t goal_pad = 0) {
  REQUIRE(snapshot.revision.has_value());
  execution_packet packet;
  packet.markdown = std::format("# Packet for {}/{}\n\nDispatch packet.\n", row.job, row.role);
  packet.work     = make_work(row.job, row.role, goal_pad);
  auto work_fp    = hash_value("work_exec_v1", packet.work);
  REQUIRE(work_fp.has_value());
  auto render_fp = fragment_digest(packet.markdown);
  REQUIRE(render_fp.has_value());
  std::vector<std::pair<std::string, value_ptr>> run{{"request", vstr(std::string(request_id))},
                                                     {"kind", vstr(std::string(kind))}};
  if (previous)
    run.emplace_back("previous", vstr(decimal(*previous)));
  packet.manifest = vrec({
      {"version", vint(1)},
      {"revision", vstr(decimal(snapshot.revision->id))},
      {"job", vstr(row.job)},
      {"role", vstr(row.role)},
      {"generation", vstr(decimal(row.generation_ordinal))},
      {"run", vrec(run)},
      {"versions", vstrs({"1", "1", "1", "1", "1", "1", "1", "1"})},
      {"work", vrec({{"digest", vstr(*work_fp)}})},
      {"render",
       vrec({{"digest", vstr(*render_fp)}, {"bytes", vstr(decimal(packet.markdown.size()))}, {"budget_bytes", vstr("65536")}})},
      {"sections",
       vlist({vrec({{"id", vstr("run")}, {"mandatory", vbool(true)}, {"included", vbool(true)}, {"bytes", vstr("0")}})})},
      {"omitted", vstrs({})},
      {"prerequisites", vlist({})},
      {"registry", vrec({{"id", vstr(decimal(snapshot.revision->registry))},
                         {"revision", vstr("1")},
                         {"authority", vstr("driver_selected_trusted_configuration")}})},
      {"inputs", vrec({{"source", vstr(decimal(snapshot.revision->source))},
                       {"interpretation", vstr(decimal(snapshot.revision->interpretation))},
                       {"publications", vlist({})},
                       {"repair", vstr("0")}})},
  });
  return packet;
}

/// Normalized driver_provenance_v1 document of the closed shape of P2-4 with unknown leaves and one setting.
value_ptr provenance_document() {
  return vrec({{"version", vint(1)},
               {"driver", vrec({{"name", vstr("planar-test-driver")}, {"version", vstr("1.0.0")}})},
               {"trial", vrec({{"id", vstr("trial_one")}})},
               {"view", vrec({{"id", vstr("view_one")}})},
               {"model", vrec({{"requested", vstr("model_a")}, {"actual", vstr("model_a")}})},
               {"session", vrec({{"requested", vstr("session_a")}, {"actual", vstr("session_a")}})},
               {"settings", vlist({vrec({{"name", vstr("temperature")}, {"value", vstr("0")}})})}});
}

// ---------------------------------------------------------------------------------------------------------------------
// Change builders. Every builder derives the expected set, the transitions, the complete next vector and the fenced run
// set from one snapshot, so a test that commits with an older snapshot produces a stale change, not an invalid one.

struct role_plan {
  std::string role;
  std::string kind;
  std::string after;
};

std::vector<role_plan> standard_roles() {
  return {role_plan{.role = "builder", .kind = "worker", .after = "ready"},
          role_plan{.role = "reviewer", .kind = "reviewer", .after = "blocked"}};
}

/// Fence every snapshot active run whose role leaves running in this change.
void derive_fences(const execution_snapshot& snapshot, execution_change& change) {
  for (const auto& row : snapshot.roles) {
    if (row.state != "running" || !row.run_id)
      continue;
    for (const auto& transition : change.transitions)
      if (transition.job == row.job && transition.role == row.role && transition.after_state != "running")
        change.fenced_runs.push_back(*row.run_id);
  }
}

execution_change load_change(const execution_snapshot& snapshot, std::string_view request_id, std::string_view expected_head,
                             const execution_revision_input& input) {
  execution_change change;
  change.operation  = "load";
  change.request_id = request_id;
  change.actor      = "tester";
  hist::request_v2 request;
  request.operation          = "load";
  request.actor              = "tester";
  request.expected_head      = expected_head;
  request.workspace_argument = std::string(workspace_name);
  request.registry_argument  = "registry";
  change.request             = request;
  change.job                 = snapshot.active_job;
  change.revision            = input;
  change.vector              = base_vector(snapshot, snapshot.active_job);
  change.expected = lifecycle_expected(snapshot, snapshot.active_job.empty() ? std::vector<std::string>{}
                                                                             : std::vector<std::string>{snapshot.active_job});
  return change;
}

execution_change activate_change(const execution_snapshot& snapshot, std::string_view request_id, std::string_view job,
                                 const std::vector<role_plan>& roles) {
  execution_change change;
  change.operation  = "activate";
  change.request_id = request_id;
  change.actor      = "tester";
  change.job        = job;
  change.request =
      hist::request_v3{.operation  = "activate",
                       .actor      = "tester",
                       .parameters = params_bytes({{"expect_job", snapshot.active_job.empty() ? "NONE" : snapshot.active_job},
                                                   {"expect_revision", head_text(snapshot)},
                                                   {"job", std::string(job)}}),
                       .profile_argument = std::nullopt};
  std::vector<std::string> jobs;
  if (!snapshot.active_job.empty())
    jobs.push_back(snapshot.active_job);
  if (std::find(jobs.begin(), jobs.end(), std::string(job)) == jobs.end())
    jobs.push_back(std::string(job));
  // Previous active job: every non-retired role retires (rows M5 and M6).
  if (!snapshot.active_job.empty() && snapshot.active_job != job) {
    for (const auto& row : snapshot.roles) {
      if (row.job != snapshot.active_job || row.state == "retired")
        continue;
      change.transitions.push_back(execution_transition_input{.job            = row.job,
                                                              .role           = row.role,
                                                              .before_state   = row.state,
                                                              .before_drifted = row.drifted,
                                                              .after_state    = "retired",
                                                              .after_drifted  = false,
                                                              .reason         = "retired"});
    }
  }
  for (const auto& plan : roles) {
    change.generations.push_back(execution_generation_input{
        .job = std::string(job), .role = plan.role, .reason = "activated", .structure = structure_of(job, plan.role, plan.kind)});
    const auto* row = find_role(snapshot, job, plan.role);
    change.transitions.push_back(
        execution_transition_input{.job            = std::string(job),
                                   .role           = plan.role,
                                   .before_state   = row ? std::optional<std::string>(row->state) : std::nullopt,
                                   .before_drifted = row ? std::optional<bool>(row->drifted) : std::nullopt,
                                   .after_state    = plan.after,
                                   .after_drifted  = false,
                                   .reason         = "activated"});
    change.vector.push_back(execution_vector_entry{
        .job = std::string(job), .role = plan.role, .state = plan.after, .drifted = false, .run = std::nullopt});
  }
  derive_fences(snapshot, change);
  change.expected = lifecycle_expected(snapshot, jobs);
  return change;
}

execution_change claim_change(const execution_snapshot& snapshot, std::string_view request_id, std::string_view job,
                              std::string_view role, bool with_provenance = false, std::size_t goal_pad = 0) {
  const auto* row = find_role(snapshot, job, role);
  REQUIRE(row != nullptr);
  execution_change change;
  change.operation  = "claim";
  change.request_id = request_id;
  change.actor      = "tester";
  std::vector<std::pair<std::string, std::string>> params{{"job", std::string(job)}, {"role", std::string(role)}};
  if (with_provenance)
    params.emplace_back("provenance_path", "/private/provenance.json");
  change.request = hist::request_v3{
      .operation = "claim", .actor = "tester", .parameters = params_bytes(params), .profile_argument = std::nullopt};
  change.job  = std::string(job);
  change.role = std::string(role);
  change.transitions.push_back(execution_transition_input{.job            = row->job,
                                                          .role           = row->role,
                                                          .before_state   = row->state,
                                                          .before_drifted = false,
                                                          .after_state    = "running",
                                                          .after_drifted  = false,
                                                          .reason         = "claimed"});
  change.vector = base_vector(snapshot, job);
  for (auto& entry : change.vector)
    if (entry.role == role)
      entry =
          execution_vector_entry{.job = entry.job, .role = entry.role, .state = "running", .drifted = false, .run = std::nullopt};
  change.packet = make_packet(snapshot, *row, request_id, "claim", std::nullopt, goal_pad);
  if (with_provenance)
    change.provenance = provenance_document();
  change.expected = targeted_expected(snapshot, *row);
  return change;
}

execution_change takeover_change(const execution_snapshot& snapshot, std::string_view request_id, std::string_view job,
                                 std::string_view role, std::uint64_t previous) {
  const auto* row = find_role(snapshot, job, role);
  REQUIRE(row != nullptr);
  execution_change change;
  change.operation  = "takeover";
  change.request_id = request_id;
  change.actor      = "tester";
  change.request    = hist::request_v3{
      .operation        = "takeover",
      .actor            = "tester",
      .parameters       = params_bytes({{"job", std::string(job)}, {"role", std::string(role)}, {"run_id", decimal(previous)}}),
      .profile_argument = std::nullopt};
  change.job          = std::string(job);
  change.role         = std::string(role);
  change.previous_run = previous;
  change.generations.push_back(execution_generation_input{
      .job = std::string(job), .role = std::string(role), .reason = "takeover", .structure = row->structure});
  change.transitions.push_back(execution_transition_input{.job            = row->job,
                                                          .role           = row->role,
                                                          .before_state   = std::string("running"),
                                                          .before_drifted = row->drifted,
                                                          .after_state    = "running",
                                                          .after_drifted  = row->drifted,
                                                          .reason         = "taken_over"});
  change.fenced_runs.push_back(previous);
  change.vector = base_vector(snapshot, job);
  for (auto& entry : change.vector)
    if (entry.role == role)
      entry = execution_vector_entry{
          .job = entry.job, .role = entry.role, .state = "running", .drifted = row->drifted, .run = std::nullopt};
  change.packet   = make_packet(snapshot, *row, request_id, "takeover", previous);
  change.expected = targeted_expected(snapshot, *row);
  return change;
}

// ---------------------------------------------------------------------------------------------------------------------
// Commit shortcuts. Each takes a fresh snapshot, builds the change from it and commits.

execution_commit_result commit_load(const std::filesystem::path& path, std::string_view request_id, const revision_ids& ids) {
  auto snapshot = snapshot_of(path);
  auto result   = commit_execution(path, snapshot, load_change(snapshot, request_id, head_text(snapshot), revision_input(ids)));
  INFO("commit failed with error code " << (result.has_value() ? -1 : static_cast<int>(result.error().code)));
  REQUIRE(result.has_value());
  return *result;
}

execution_commit_result commit_activate(const std::filesystem::path& path, std::string_view request_id, std::string_view job,
                                        const std::vector<role_plan>& roles) {
  auto snapshot = snapshot_of(path);
  auto result   = commit_execution(path, snapshot, activate_change(snapshot, request_id, job, roles));
  INFO("commit failed with error code " << (result.has_value() ? -1 : static_cast<int>(result.error().code)));
  REQUIRE(result.has_value());
  return *result;
}

execution_commit_result commit_claim(const std::filesystem::path& path, std::string_view request_id, std::string_view job,
                                     std::string_view role) {
  auto snapshot = snapshot_of(path);
  auto result   = commit_execution(path, snapshot, claim_change(snapshot, request_id, job, role));
  INFO("commit failed with error code " << (result.has_value() ? -1 : static_cast<int>(result.error().code)));
  REQUIRE(result.has_value());
  return *result;
}

/// A fresh ledger with one accepted revision (seeded "one") and job alpha activated: builder ready, reviewer blocked.
std::filesystem::path activated_ledger(const scratch& dir, bool with_facet = false) {
  const auto path = dir.ledger();
  REQUIRE(initialize_format3(path, workspace_name).has_value());
  const auto ids = seed_revision(path, "one", with_facet);
  commit_load(path, "load-1", ids);
  commit_activate(path, "activate-1", "alpha", standard_roles());
  return path;
}

} // namespace

namespace {

std::string blob_literal(std::string_view bytes) {
  std::string out = "X'";
  for (auto byte : bytes)
    out += std::format("{:02x}", static_cast<unsigned char>(byte));
  out += "'";
  return out;
}

const char* zero_secret_literal = "X'00000000000000000000000000000000'";

std::uint64_t insert_frame(connection& db, std::int64_t kind, std::int64_t codec, std::string_view digest,
                           std::string_view encoded) {
  auto stmt = db.prepare("INSERT INTO records(kind,codec,digest,encoded) VALUES(?1,?2,?3,?4)");
  REQUIRE(stmt.has_value());
  REQUIRE(stmt->bind_integer(1, kind).has_value());
  REQUIRE(stmt->bind_integer(2, codec).has_value());
  REQUIRE(stmt->bind_blob(3, digest).has_value());
  REQUIRE(stmt->bind_blob(4, encoded).has_value());
  auto step = stmt->step();
  REQUIRE(step.has_value());
  REQUIRE(*step == step_result::done);
  return static_cast<std::uint64_t>(int_of(db, "SELECT last_insert_rowid()"));
}

struct frame_node {
  std::uint64_t id = 0;
  std::string   digest;
  std::string   encoded;
};

/// Store an execution value under one of the four codec identifiers (D5): kind 3, codec 1, the framed bytes.
frame_node store_execution_value(connection& db, std::string_view codec_id, std::string_view canonical) {
  auto frame = hist::encode_execution_value_record(codec_id, canonical);
  if (!frame) {
    // The encoder refuses canonical bytes above the 16 MiB value ceiling, so the corrupt record is framed by hand: the
    // magic, the value tag, then the codec and the bytes with 8-byte big-endian lengths. Its digest cannot be computed.
    REQUIRE(canonical.size() > 16777216);
    std::string forged = std::string("PLANAR-RECORD") + '\x01' + '\x03';
    for (const std::string_view part : {codec_id, canonical}) {
      for (int shift = 56; shift >= 0; shift -= 8)
        forged += static_cast<char>((static_cast<std::uint64_t>(part.size()) >> shift) & 255);
      forged.append(part);
    }
    const std::string forged_digest(16, '\x7f');
    return frame_node{insert_frame(db, 3, 1, forged_digest, forged), forged_digest, forged};
  }
  auto digest = hist::digest_execution_value_record(*frame);
  REQUIRE(digest.has_value());
  const auto digest_bytes = digest_text(*digest);
  return frame_node{insert_frame(db, 3, 1, digest_bytes, *frame), digest_bytes, *frame};
}

/// SQL that rewrites one event payload v2 after decoding it, leaving every other field as stored.
std::string payload_update_sql(const std::filesystem::path& path, std::uint64_t seq,
                               const std::function<void(hist::event_payload_v2&)>& mutate) {
  auto       db      = open_raw(path);
  const auto type    = text_of(db, std::format("SELECT type FROM events WHERE seq = {}", seq));
  const auto bytes   = blob_of(db, std::format("SELECT payload FROM events WHERE seq = {}", seq));
  auto       decoded = hist::decode_event_payload_v2(type, bytes);
  REQUIRE(decoded.has_value());
  mutate(*decoded);
  auto encoded = hist::encode_event_payload_v2(type, *decoded);
  REQUIRE(encoded.has_value());
  return std::format("UPDATE events SET payload = {} WHERE seq = {}", blob_literal(*encoded), seq);
}

std::uint64_t seq_of_type(const std::filesystem::path& path, std::string_view type) {
  auto db = open_raw(path);
  return static_cast<std::uint64_t>(
      int_of(db, std::format("SELECT seq FROM events WHERE type = '{}' ORDER BY seq DESC LIMIT 1", type)));
}

/// Commit a change built from the snapshot that is read immediately before the commit.
execution_commit_result commit_with(const std::filesystem::path&                                      path,
                                    const std::function<execution_change(const execution_snapshot&)>& build) {
  const auto snapshot = snapshot_of(path);
  auto       result   = commit_execution(path, snapshot, build(snapshot));
  INFO("commit failed with error code " << (result.has_value() ? -1 : static_cast<int>(result.error().code)));
  REQUIRE(result.has_value());
  return *result;
}

/// Claim-then-takeover: the target role's current run is fenced by a takeover that names it.
execution_commit_result commit_takeover_current(const std::filesystem::path& path, std::string_view request_id,
                                                std::string_view job, std::string_view role) {
  const auto  snapshot = snapshot_of(path);
  const auto* row      = find_role(snapshot, job, role);
  REQUIRE(row != nullptr);
  REQUIRE(row->run_id.has_value());
  auto result = commit_execution(path, snapshot, takeover_change(snapshot, request_id, job, role, *row->run_id));
  INFO("commit failed with error code " << (result.has_value() ? -1 : static_cast<int>(result.error().code)));
  REQUIRE(result.has_value());
  return *result;
}

/// Replace the transition, generation and vector of a derived role change after a builder has filled them.
void settle_vector(execution_change& change, const execution_snapshot& snapshot) {
  for (const auto& transition : change.transitions) {
    if (transition.job != change.job)
      continue;
    auto entry = execution_vector_entry{.job     = transition.job,
                                        .role    = transition.role,
                                        .state   = transition.after_state,
                                        .drifted = transition.after_drifted,
                                        .run     = std::nullopt};
    auto found = std::find_if(change.vector.begin(), change.vector.end(),
                              [&](const execution_vector_entry& candidate) { return candidate.role == transition.role; });
    if (found != change.vector.end())
      *found = entry;
    else
      change.vector.push_back(entry);
  }
  derive_fences(snapshot, change);
}

/// A load that changes one role of the active job. `structural` opens a structural_change generation, `remove` retires
/// the role, and `readd` opens a recreated generation after the retired row.
execution_change role_load(const execution_snapshot& snapshot, std::string_view request_id, const revision_ids& ids,
                           std::string_view role, std::string_view mode) {
  auto        change = load_change(snapshot, request_id, head_text(snapshot), revision_input(ids));
  const auto* row    = find_role(snapshot, snapshot.active_job, role);
  REQUIRE(row != nullptr);
  const auto job = snapshot.active_job;
  if (mode == "structural") {
    change.generations.push_back(
        execution_generation_input{.job       = job,
                                   .role      = std::string(role),
                                   .reason    = "structural_change",
                                   .structure = structure_of(job, role, row->structure.kind, "unit_two")});
    change.transitions.push_back(execution_transition_input{.job            = job,
                                                            .role           = std::string(role),
                                                            .before_state   = row->state,
                                                            .before_drifted = row->drifted,
                                                            .after_state    = row->state,
                                                            .after_drifted  = row->drifted,
                                                            .reason         = "structural_change"});
  } else if (mode == "remove") {
    change.transitions.push_back(execution_transition_input{.job            = job,
                                                            .role           = std::string(role),
                                                            .before_state   = row->state,
                                                            .before_drifted = row->drifted,
                                                            .after_state    = "retired",
                                                            .after_drifted  = false,
                                                            .reason         = "retired"});
  } else {
    change.generations.push_back(execution_generation_input{
        .job = job, .role = std::string(role), .reason = "recreated", .structure = structure_of(job, role, row->structure.kind)});
    change.transitions.push_back(execution_transition_input{.job            = job,
                                                            .role           = std::string(role),
                                                            .before_state   = std::string("retired"),
                                                            .before_drifted = false,
                                                            .after_state    = "blocked",
                                                            .after_drifted  = false,
                                                            .reason         = "recreated"});
  }
  settle_vector(change, snapshot);
  return change;
}

/// A load that adds the never-seen role `role` to the active job: a structural_change generation and a NULL-first
/// transition to ready. A claim cannot open a generation (D44), so a competing NULL-first row is produced by a load.
execution_change add_role_load(const execution_snapshot& snapshot, std::string_view request_id, const revision_ids& ids,
                               std::string_view role) {
  auto       change = load_change(snapshot, request_id, head_text(snapshot), revision_input(ids));
  const auto job    = snapshot.active_job;
  change.generations.push_back(execution_generation_input{
      .job = job, .role = std::string(role), .reason = "structural_change", .structure = structure_of(job, role, "worker")});
  change.transitions.push_back(execution_transition_input{.job            = job,
                                                          .role           = std::string(role),
                                                          .before_state   = std::nullopt,
                                                          .before_drifted = std::nullopt,
                                                          .after_state    = "ready",
                                                          .after_drifted  = false,
                                                          .reason         = "prerequisite_available"});
  settle_vector(change, snapshot);
  return change;
}

/// Fresh ledger with a claimed builder run (claim-1) on the activated alpha job.
std::filesystem::path claimed_ledger(const scratch& dir) {
  auto path = activated_ledger(dir);
  commit_claim(path, "claim-1", "alpha", "builder");
  return path;
}

/// Claimed ledger whose builder run was taken over (takeover-1); run 1 is fenced and run 2 is active.
std::filesystem::path taken_over_ledger(const scratch& dir) {
  auto path = claimed_ledger(dir);
  commit_takeover_current(path, "takeover-1", "alpha", "builder");
  return path;
}

/// Two accepted revisions: the second is a load of a second seed with parent 1.
std::filesystem::path two_revision_ledger(const scratch& dir) {
  auto       path   = activated_ledger(dir);
  const auto second = seed_revision(path, "two");
  commit_load(path, "load-2", second);
  return path;
}

std::map<std::string, std::string> file_contents(const std::filesystem::path& root) {
  std::map<std::string, std::string> out;
  for (const auto& entry : std::filesystem::directory_iterator(root)) {
    std::ifstream      in(entry.path(), std::ios::binary);
    std::ostringstream buffer;
    buffer << in.rdbuf();
    out[entry.path().filename().string()] = buffer.str();
  }
  return out;
}

/// Independent D24 value S of a snapshot, built from fixture values and the raw dispatch revision (never the product
/// measure). Roles are in snapshot order, which is sorted by job then role.
value_ptr independent_state(connection& db, const execution_snapshot& snapshot) {
  std::vector<value_ptr> roles;
  for (const auto& row : snapshot.roles) {
    std::vector<value_ptr> work_units;
    std::vector<value_ptr> target_units;
    for (const auto& unit : row.structure.work)
      work_units.push_back(vstr(unit));
    for (const auto& unit : row.structure.targets)
      target_units.push_back(vstr(unit));
    auto      structure = vrec({{"job", vstr(row.structure.job)},
                                {"role", vstr(row.structure.role)},
                                {"kind", vstr(row.structure.kind)},
                                {"work", vlist(work_units)},
                                {"targets", vlist(target_units)},
                                {"ownership", vlist({})},
                                {"shape", vstr(row.structure.shape)}});
    value_ptr run       = vstr("none");
    if (row.run_id) {
      const auto revision = int_of(db, std::format("SELECT revision_id FROM dispatches WHERE id = {}", row.dispatch->id));
      run                 = vrec({{"id", vstr(decimal(*row.run_id))},
                                  {"generation", vstr(decimal(row.generation_ordinal))},
                                  {"dispatch", vrec({{"id", vstr(decimal(row.dispatch->id))},
                                                     {"work_fp", vstr(row.dispatch->work_fp)},
                                                     {"render_fp", vstr(row.dispatch->render_fp)},
                                                     {"revision", vstr(decimal(static_cast<std::uint64_t>(revision)))},
                                                     {"work", row.dispatch->work}})}});
    }
    roles.push_back(vrec({{"job", vstr(row.job)},
                          {"role", vstr(row.role)},
                          {"kind", vstr(row.structure.kind)},
                          {"generation", vrec({{"id", vstr(decimal(row.generation_id))},
                                               {"ordinal", vstr(decimal(row.generation_ordinal))},
                                               {"structure", structure}})},
                          {"state", vstr(row.state)},
                          {"drifted", vbool(row.drifted)},
                          {"fact_version", vstr(decimal(row.fact_version))},
                          {"run", run}}));
  }
  value_ptr revision = vstr("none");
  if (snapshot.revision)
    revision = vrec({{"id", vstr(decimal(snapshot.revision->id))}});
  value_ptr activation = vstr("none");
  if (snapshot.activation)
    activation = vstr(decimal(*snapshot.activation));
  return vrec({{"active_revision", vstr(head_text(snapshot))},
               {"active_job", vstr(snapshot.active_job)},
               {"activation", activation},
               {"revision", revision},
               {"roles", vlist(roles)},
               {"publications", vlist({})}});
}

/// Claimed ledger whose builder W carries `goal_pad` extra bytes, used to size the execution state exactly.
std::filesystem::path padded_claim_ledger(const scratch& dir, std::size_t goal_pad) {
  auto       path     = activated_ledger(dir);
  const auto snapshot = snapshot_of(path);
  auto       result   = commit_execution(path, snapshot, claim_change(snapshot, "claim-1", "alpha", "builder", false, goal_pad));
  REQUIRE(result.has_value());
  return path;
}

} // namespace

TEST_CASE("format3 snapshot of a target with no ledger refuses and creates nothing") {
  scratch    dir;
  const auto path = dir.ledger();
  require_code(read_execution_snapshot(path), execution_error_code::ledger_not_found);
  REQUIRE_FALSE(std::filesystem::exists(path));
  std::size_t entries = 0;
  for (const auto& entry : std::filesystem::directory_iterator(dir.root)) {
    (void)entry;
    ++entries;
  }
  REQUIRE(entries == 0);
}

TEST_CASE("format3 snapshot of a fresh ledger") {
  scratch    dir;
  const auto path     = dir.ledger();
  auto       metadata = initialize_format3(path, workspace_name);
  REQUIRE(metadata.has_value());
  const auto snapshot = snapshot_of(path);
  REQUIRE(snapshot.identity == metadata->identity);
  REQUIRE(snapshot.last_seq == 1);
  REQUIRE_FALSE(snapshot.revision.has_value());
  REQUIRE(snapshot.active_job.empty());
  REQUIRE_FALSE(snapshot.activation.has_value());
  REQUIRE(snapshot.roles.empty());
  // The contract value of a fresh ledger: the revision and activation are "none", the job is the empty string.
  auto       independent = vrec({{"active_revision", vstr("none")},
                                 {"active_job", vstr("")},
                                 {"activation", vstr("none")},
                                 {"revision", vstr("none")},
                                 {"roles", vlist({})},
                                 {"publications", vlist({})}});
  const auto size        = bytes_of(independent).size();
  REQUIRE(snapshot.state_bytes == size);
  REQUIRE(execution_state_bytes(snapshot) == size);
}

TEST_CASE("format3 snapshot execution state bound at limit and limit plus one") {
  scratch    base_dir;
  const auto base      = padded_claim_ledger(base_dir, 0);
  const auto base_size = [&] {
    auto db = open_raw(base);
    return bytes_of(independent_state(db, snapshot_of(base))).size();
  }();
  REQUIRE(base_size < state_limit);
  const auto pad = state_limit - base_size;

  scratch    at_dir;
  const auto at = padded_claim_ledger(at_dir, pad);
  {
    auto       db       = open_raw(at);
    const auto snapshot = snapshot_of(at);
    const auto size     = bytes_of(independent_state(db, snapshot)).size();
    REQUIRE(size == state_limit);
    REQUIRE(snapshot.state_bytes == size);
    REQUIRE(execution_state_bytes(snapshot) == size);
  }
  REQUIRE(verify_execution_ledger(at).has_value());

  scratch    over_dir;
  const auto over = padded_claim_ledger(over_dir, pad + 1);
  auto       read = read_execution_snapshot(over);
  require_code(read, execution_error_code::resource_limit);
  REQUIRE(read.error().limit == "execution_snapshot_bytes");
  REQUIRE(read.error().maximum == state_limit);
}

TEST_CASE("format1 and format2 refuse every execution entry point") {
  for (int version : {1, 2}) {
    scratch    dir;
    const auto path = dir.ledger();
    if (version == 1)
      REQUIRE(initialize_new(path, workspace_name).has_value());
    else
      REQUIRE(initialize_new_v2(path, workspace_name).has_value());
    const auto        before  = file_contents(dir.root);
    execution_framing framing = hist::request_v3{.operation        = "claim",
                                                 .actor            = "tester",
                                                 .parameters       = params_bytes({{"job", "alpha"}, {"role", "builder"}}),
                                                 .profile_argument = std::nullopt};
    require_code(read_execution_snapshot(path), execution_error_code::unsupported_ledger_version);
    require_code(verify_execution_ledger(path), execution_error_code::unsupported_ledger_version);
    require_code(lookup_execution_request(path, "claim-1", framing), execution_error_code::unsupported_ledger_version);
    require_code(authorize_run_token(path, "1.1.00000000000000000000000000000000"),
                 execution_error_code::unsupported_ledger_version);
    execution_snapshot snapshot;
    execution_change   change;
    require_code(stage_execution(path, snapshot, change), execution_error_code::unsupported_ledger_version);
    require_code(commit_execution(path, snapshot, change), execution_error_code::unsupported_ledger_version);
    REQUIRE(file_contents(dir.root) == before);
  }
}

TEST_CASE("format3 verification accepts ledgers written by the commit path") {
  scratch    dir;
  const auto path = dir.ledger();
  REQUIRE(initialize_format3(path, workspace_name).has_value());
  require_verifies(path);
  commit_load(path, "load-1", seed_revision(path, "one"));
  require_verifies(path);
  commit_activate(path, "activate-1", "alpha", standard_roles());
  require_verifies(path);
  commit_claim(path, "claim-1", "alpha", "builder");
  require_verifies(path);
  commit_takeover_current(path, "takeover-1", "alpha", "builder");
  require_verifies(path);
  commit_with(path, [&](const execution_snapshot& s) {
    return role_load(s, "load-3", seed_revision(path, "three"), "reviewer", "structural");
  });
  require_verifies(path);
  commit_with(path, [&](const execution_snapshot& s) {
    return role_load(s, "load-4", seed_revision(path, "four"), "reviewer", "remove");
  });
  require_verifies(path);
  commit_with(path, [&](const execution_snapshot& s) {
    return role_load(s, "load-5", seed_revision(path, "five"), "reviewer", "readd");
  });
  require_verifies(path);
}

TEST_CASE("format3 verification rejects a broken request and event bijection") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  corrupt(path, {"UPDATE events SET request_id = NULL WHERE request_id = 'claim-1'"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects an active run of a non-current generation") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  // The run moves to the reviewer generation with its run_state and claim event columns; the builder stays current.
  corrupt(path, {"UPDATE runs SET generation_id = (SELECT id FROM generations WHERE job = 'alpha' AND role = 'reviewer') WHERE "
                 "request_id = 'claim-1'",
                 "UPDATE run_state SET generation_id = (SELECT id FROM generations WHERE job = 'alpha' AND role = 'reviewer') "
                 "WHERE run_id = (SELECT id FROM runs WHERE request_id = 'claim-1')",
                 "UPDATE events SET generation_id = (SELECT id FROM generations WHERE job = 'alpha' AND role = 'reviewer') WHERE "
                 "request_id = 'claim-1'"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a dispatch of a run that was not active") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  // The dispatch sequence 1 precedes the run's claim event, so the run is not active when the dispatch is written.
  corrupt(path, {"UPDATE dispatches SET seq = 1"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a broken transition chain") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  corrupt(
      path,
      {"UPDATE transitions SET before_state = 'blocked' WHERE event_seq = (SELECT seq FROM events WHERE type = 'run_claimed')"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a role state that differs from the replay") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  // Only the drift flag differs from the replayed claim transition, which carries after_drifted 0.
  corrupt(path, {"UPDATE role_state SET drifted = 1 WHERE job = 'alpha' AND role = 'builder'"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a stale current state") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  corrupt(path, {"UPDATE current_state SET last_seq = 3"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects non contiguous generation ordinals") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  corrupt(path, {"UPDATE generations SET ordinal = 2 WHERE job = 'alpha' AND role = 'builder'"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a wrong active activation") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  corrupt(path, {"UPDATE current_state SET active_activation_id = NULL"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a broken run coupling") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  // An active run of the blocked reviewer, with no running role state: the non-running direction of invariant 12.
  corrupt(path, {"INSERT INTO runs(id,generation_id,request_id,claimed_seq,secret,provenance_id) VALUES(9,(SELECT id FROM "
                 "generations WHERE job = 'alpha' AND role = 'reviewer'),'phantom-claim',4," +
                     std::string(zero_secret_literal) + ",NULL)",
                 "INSERT INTO run_state(run_id,job,role,generation_id,state,last_seq) VALUES(9,'alpha','reviewer',(SELECT id "
                 "FROM generations WHERE job = 'alpha' AND role = 'reviewer'),'active',4)"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a wrong outcome pairing") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  corrupt(path, {"UPDATE requests SET outcome = 'claimed' WHERE request_id = 'activate-1'"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects dangling reader verified columns") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  // A generation id that names no generations row, carried by the run, its run_state and the claim event.
  corrupt(path, {"UPDATE runs SET generation_id = 999 WHERE request_id = 'claim-1'",
                 "UPDATE run_state SET generation_id = 999 WHERE run_id = (SELECT id FROM runs WHERE request_id = 'claim-1')",
                 "UPDATE events SET generation_id = 999 WHERE request_id = 'claim-1'"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a run state that differs from the replay") {
  scratch    dir;
  const auto path = taken_over_ledger(dir);
  require_verifies(path);
  // The fenced run of the takeover must replay as fenced; released is a legal stored value that differs.
  corrupt(path, {"UPDATE run_state SET state = 'released' WHERE run_id = (SELECT id FROM runs WHERE request_id = 'claim-1')"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a broken NULL first transition") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  // The builder's first transition, written by the activation, gets a non-NULL before state.
  corrupt(path, {"UPDATE transitions SET before_state = 'blocked', before_drifted = 0 WHERE event_seq = (SELECT seq FROM events "
                 "WHERE type = 'job_activated') AND role = 'builder'"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a non retired row outside the active job") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  commit_activate(path, "activate-2", "beta", {role_plan{.role = "worker", .kind = "worker", .after = "ready"}});
  require_verifies(path);
  // alpha is no longer active, so its retired rows must stay retired; both the row and its last transition change.
  corrupt(path, {"UPDATE role_state SET state = 'blocked' WHERE job = 'alpha' AND role = 'builder'",
                 "UPDATE transitions SET after_state = 'blocked', reason = 'prerequisite_unavailable' WHERE job = 'alpha' AND "
                 "role = 'builder' AND after_state = 'retired'"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a wrong fact version") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  corrupt(path, {"UPDATE role_state SET fact_version = fact_version + 5 WHERE job = 'alpha' AND role = 'builder'"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects non contiguous transition ordinals") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  // The activation event carries two transitions, ordinals 1 and 2; moving the second to 3 leaves a gap.
  corrupt(path, {"UPDATE transitions SET ordinal = 3 WHERE event_seq = (SELECT seq FROM events WHERE type = 'job_activated') AND "
                 "ordinal = 2"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects wrong event version or typed columns") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  corrupt(path, {"UPDATE events SET version = 1 WHERE type = 'run_claimed'"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects an inconsistent event payload") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  // The payload reason names taken_over while the ordinal one transition reason is claimed; every other field agrees.
  const auto claim_seq = seq_of_type(path, "run_claimed");
  const auto sql = payload_update_sql(path, claim_seq, [](hist::event_payload_v2& payload) { payload.reason = "taken_over"; });
  corrupt(path, {sql});
  expect_corrupt(path);
}

namespace {

std::string to_hex(std::string_view bytes) {
  std::string out;
  for (auto byte : bytes)
    out += std::format("{:02x}", static_cast<unsigned char>(byte));
  return out;
}

std::uint64_t nth_latest_seq(const std::filesystem::path& path, std::string_view type, int offset) {
  auto db = open_raw(path);
  return static_cast<std::uint64_t>(
      int_of(db, std::format("SELECT seq FROM events WHERE type = '{}' ORDER BY seq DESC LIMIT 1 OFFSET {}", type, offset)));
}

revision_ids ids_of(const execution_snapshot& snapshot) {
  REQUIRE(snapshot.revision.has_value());
  return revision_ids{snapshot.revision->source, snapshot.revision->registry, snapshot.revision->stdlib,
                      snapshot.revision->interpretation, snapshot.revision->versions};
}

/// Copy one stored record row and return the copy's identifier (the copy has equal kind, codec, digest and bytes).
std::uint64_t copy_record(connection& db, std::uint64_t id) {
  run_sql(db,
          std::format(
              "INSERT INTO records(kind,codec,digest,encoded) SELECT kind,codec,digest,encoded FROM records WHERE id = {}", id));
  return static_cast<std::uint64_t>(int_of(db, "SELECT last_insert_rowid()"));
}

/// The canonical bytes of the load result the native commit builds for one request, from raw rows (history_codec).
std::string expected_load_result(const std::filesystem::path& path, std::string_view request_id,
                                 const std::function<void(load_result&)>& mutate) {
  auto        db = open_raw(path);
  load_result result{};
  result.ledger      = text_of(db, "SELECT identity FROM ledger_meta");
  result.request_id  = std::string(request_id);
  const auto outcome = text_of(db, std::format("SELECT outcome FROM requests WHERE request_id = '{}'", request_id));
  result.outcome     = outcome == "noop" ? load_outcome::noop : load_outcome::accepted;
  result.revision    = int_of(db, std::format("SELECT revision_id FROM requests WHERE request_id = '{}'", request_id));
  const auto parent  = int_of(db, std::format("SELECT COALESCE(parent_id, 0) FROM revisions WHERE id = {}", result.revision));
  if (parent != 0)
    result.parent = parent;
  result.event = int_of(db, std::format("SELECT event_seq FROM requests WHERE request_id = '{}'", request_id));
  mutate(result);
  auto encoded = encode_load_result(result);
  REQUIRE(encoded.has_value());
  return *encoded;
}

/// Provenance document of the closed P2-4 shape. Leaves are the eight scalar leaves and one name and value per setting,
/// in a fixed order, so the caller can pad individual leaves to reach an exact compact JSON size.
value_ptr provenance_from_leaves(const std::vector<std::string>& leaves) {
  std::vector<value_ptr> settings;
  for (std::size_t index = 0; index < 32; ++index)
    settings.push_back(vrec({{"name", vstr(leaves[8 + 2 * index])}, {"value", vstr(leaves[9 + 2 * index])}}));
  return vrec({{"version", vint(1)},
               {"driver", vrec({{"name", vstr(leaves[0])}, {"version", vstr(leaves[1])}})},
               {"trial", vrec({{"id", vstr(leaves[2])}})},
               {"view", vrec({{"id", vstr(leaves[3])}})},
               {"model", vrec({{"requested", vstr(leaves[4])}, {"actual", vstr(leaves[5])}})},
               {"session", vrec({{"requested", vstr(leaves[6])}, {"actual", vstr(leaves[7])}})},
               {"settings", vlist(settings)}});
}

std::vector<std::string> base_leaves() {
  std::vector<std::string> leaves = {"planar-test-driver", "1.0.0",    "trial_one", "view_one", "model_a", "model_a",
                                     "session_a",          "session_a"};
  for (std::size_t index = 0; index < 32; ++index) {
    leaves.push_back(std::format("n{:02}", index));
    leaves.push_back("v");
  }
  return leaves;
}

/// Pad leaves, each so its string stays within the 1024-byte bound, until the compact JSON of
/// the document reaches `target_json` bytes exactly. Names keep their unique sorted prefix.
value_ptr provenance_with_json_size(std::size_t target_json) {
  auto leaves = base_leaves();
  auto size   = planar::engine::codec_encode(provenance_from_leaves(leaves));
  REQUIRE(size.has_value());
  REQUIRE(size->size() <= target_json);
  std::size_t extra = target_json - size->size();
  for (auto& leaf : leaves) {
    const auto add = std::min<std::size_t>(extra, 1024 - leaf.size());
    leaf += std::string(add, 'x');
    extra -= add;
    if (extra == 0)
      break;
  }
  REQUIRE(extra == 0);
  auto document = provenance_from_leaves(leaves);
  auto measured = planar::engine::codec_encode(document);
  REQUIRE(measured.has_value());
  REQUIRE(measured->size() == target_json);
  return document;
}

/// A manifest value whose canonical bytes are exactly `target` bytes: the base manifest plus an omitted list of short
/// identifiers whose lengths are chosen so the total is exact. Identifiers are 1 to 64 bytes.
value_ptr manifest_of_size(const value_ptr& base, std::size_t target) {
  value manifest             = *base;
  manifest.fields["omitted"] = vlist({});
  auto empty_size            = canonical_encode(std::make_shared<const value>(manifest));
  REQUIRE(empty_size.has_value());
  REQUIRE(empty_size->size() <= target);
  std::size_t remaining = target - empty_size->size();
  // An entry costs 9 bytes of header plus its identifier, so one entry carries 10 to 73 bytes.
  const std::size_t      entries = (remaining + 72) / 73;
  std::vector<value_ptr> omitted;
  for (std::size_t index = 0; index < entries; ++index) {
    const std::size_t left = entries - index;
    std::size_t       cost = std::max<std::size_t>(10, remaining - 73 * (left - 1));
    if (remaining < cost + 10 * (left - 1))
      cost = remaining - 10 * (left - 1);
    // The identifier is cost minus the 9-byte header; the hexadecimal index keeps identifiers distinct when it fits.
    const auto digits = std::format("{:x}", index);
    const auto length = cost - 9;
    omitted.push_back(
        vstr(length >= digits.size() ? std::string(length - digits.size(), 'a') + digits : std::string(length, 'a')));
    remaining -= cost;
  }
  REQUIRE(remaining == 0);
  manifest.fields["omitted"] = vlist(omitted);
  auto encoded               = canonical_encode(std::make_shared<const value>(manifest));
  REQUIRE(encoded.has_value());
  REQUIRE(encoded->size() == target);
  return std::make_shared<const value>(manifest);
}

} // namespace

TEST_CASE("format3 verification rejects reserved events and rows") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  corrupt(path,
          {"INSERT INTO events(seq,type,version,revision_id,request_id,actor,occurred_at,payload,job,role,generation_id,run_id) "
           "SELECT 5,'run_refreshed',2,NULL,NULL,'tester','2026-11-07T00:00:00.000Z',payload,NULL,NULL,NULL,NULL FROM events "
           "WHERE seq = 4",
           "UPDATE current_state SET last_seq = 5"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a corrupt request record") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  corrupt(path, {"UPDATE records SET digest = " + std::string(zero_secret_literal) +
                 " WHERE id = (SELECT body_id FROM requests WHERE request_id = 'claim-1')"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a dispatch digest mismatch") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  // The stored work fingerprint no longer equals the digest of the stored W value.
  corrupt(path, {"UPDATE dispatches SET work_fp = " + std::string(zero_secret_literal) + " WHERE id = 1"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a wrong generation structure") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  // The builder generation names the reviewer structure: a different job and role than the generation's row.
  corrupt(path, {"UPDATE generations SET structure_id = (SELECT structure_id FROM generations WHERE job = 'alpha' AND role = "
                 "'reviewer') WHERE job = 'alpha' AND role = 'builder'"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a run that disagrees with its event") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  corrupt(path, {"UPDATE runs SET claimed_seq = 3 WHERE request_id = 'claim-1'"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects an oversize stored manifest") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  auto       db      = open_raw(path);
  const auto frame   = blob_of(db, "SELECT encoded FROM records WHERE id = (SELECT manifest_id FROM dispatches WHERE id = 1)");
  auto       decoded = hist::decode_execution_value_record(frame);
  REQUIRE(decoded.has_value());
  auto manifest = canonical_decode(decoded->encoded);
  REQUIRE(manifest.has_value());
  const auto at_limit = manifest_of_size(*manifest, manifest_limit);
  const auto over     = manifest_of_size(*manifest, manifest_limit + 1);
  // Exactly at the limit the stored manifest verifies; one byte more is ledger_corrupt.
  auto       stored_at       = store_execution_value(db, hist::codec_dispatch_manifest_v1, bytes_of(at_limit));
  const auto manifest_at_sql = std::format("UPDATE dispatches SET manifest_id = {} WHERE id = 1", stored_at.id);
  corrupt(path,
          {manifest_at_sql, payload_update_sql(path, seq_of_type(path, "run_claimed"), [&](hist::event_payload_v2& payload) {
             payload.references["manifest"] = std::to_string(stored_at.id);
           })});
  REQUIRE(verify_execution_ledger(path).has_value());
  auto over_db     = open_raw(path);
  auto stored_over = store_execution_value(over_db, hist::codec_dispatch_manifest_v1, bytes_of(over));
  corrupt(path, {std::format("UPDATE dispatches SET manifest_id = {} WHERE id = 1", stored_over.id),
                 payload_update_sql(path, seq_of_type(path, "run_claimed"), [&](hist::event_payload_v2& payload) {
                   payload.references["manifest"] = std::to_string(stored_over.id);
                 })});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects an oversize stored provenance") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  // The stored record is the wrapper {version,authority,document}; its document is measured as compact JSON bytes.
  auto attach = [&](std::size_t json_size) {
    auto        db       = open_raw(path);
    auto        document = provenance_with_json_size(json_size);
    std::string wrapper;
    if (auto encoded = hist::encode_driver_provenance_v1(document)) {
      wrapper = *encoded;
    } else {
      // The encoder refuses an oversize document, so the corrupt record is framed by hand: the closed wrapper value.
      REQUIRE(json_size > 65536);
      auto forged = canonical_encode(
          vrec({{"version", vint(1)}, {"authority", vstr("driver_assertion_unverified")}, {"document", document}}));
      REQUIRE(forged.has_value());
      wrapper = *forged;
    }
    auto stored_record = store_execution_value(db, hist::codec_driver_provenance_v1, wrapper);
    return std::vector<std::string>{
        std::format("UPDATE runs SET provenance_id = {} WHERE request_id = 'claim-1'", stored_record.id),
        payload_update_sql(path, seq_of_type(path, "run_claimed"), [&](hist::event_payload_v2& payload) {
          payload.references["provenance"] = std::to_string(stored_record.id);
        })};
  };
  corrupt(path, attach(65536));
  REQUIRE(verify_execution_ledger(path).has_value());
  // G5: a document whose compact JSON is at most 65536 bytes is accepted even when its canonical_v1 size is larger.
  const auto canonical_big = bytes_of(provenance_with_json_size(65000));
  REQUIRE(canonical_big.size() > 65536);
  corrupt(path, attach(65000));
  REQUIRE(verify_execution_ledger(path).has_value());
  corrupt(path, attach(65537));
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a broken revision parent chain") {
  scratch    dir;
  const auto path = two_revision_ledger(dir);
  require_verifies(path);
  corrupt(path, {"UPDATE revisions SET parent_id = NULL WHERE id = 2"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a revision without exactly one accepting event") {
  scratch    dir;
  const auto path = two_revision_ledger(dir);
  require_verifies(path);
  // The accepting event of revision 2 no longer names it; the request row still names the revision.
  corrupt(path, {"UPDATE events SET revision_id = NULL WHERE revision_id = 2"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects disagreeing request and event links") {
  scratch    dir;
  const auto path = two_revision_ledger(dir);
  require_verifies(path);
  corrupt(path, {"UPDATE requests SET revision_id = 2 WHERE request_id = 'load-1'"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a request or revision on initialization") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  require_verifies(path);
  // A request row names the initialization event 1 and the event names that request.
  corrupt(path,
          {"INSERT INTO requests(request_id,operation,body_id,event_seq,revision_id,outcome,result,attachment_id) "
           "VALUES('init-request','load',(SELECT body_id FROM requests WHERE request_id = 'load-1'),1,1,'accepted',X'',NULL)",
           "UPDATE events SET request_id = 'init-request' WHERE seq = 1"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a gap in a committed sequence") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  // Event 4 moves to sequence 9 with every row that names it, so the committed sequence has a gap at 4.
  corrupt(path, {"UPDATE events SET seq = 9 WHERE seq = 4", "UPDATE transitions SET event_seq = 9 WHERE event_seq = 4",
                 "UPDATE requests SET event_seq = 9 WHERE event_seq = 4", "UPDATE runs SET claimed_seq = 9 WHERE id = 1",
                 "UPDATE dispatches SET seq = 9 WHERE id = 1", "UPDATE run_state SET last_seq = 9 WHERE run_id = 1",
                 "UPDATE role_state SET last_seq = 9 WHERE job = 'alpha' AND role = 'builder'",
                 "UPDATE current_state SET last_seq = 9"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a generation reason that disagrees with its event") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  // The activation opened the builder generation with reason activated; structural_change belongs to declaration_accepted.
  corrupt(path, {"UPDATE generations SET reason = 'structural_change' WHERE job = 'alpha' AND role = 'builder'"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a generation opened by an event without its transition") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  // Sequence 2 is the load, which carries no transition for alpha builder.
  corrupt(path, {"UPDATE generations SET opened_seq = 2 WHERE job = 'alpha' AND role = 'builder'"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a takeover generation that changes the structure") {
  scratch    dir;
  const auto path = taken_over_ledger(dir);
  require_verifies(path);
  auto       db      = open_raw(path);
  const auto variant = hist::encode_role_structure_v1(structure_of("alpha", "builder", "worker", "unit_two"));
  REQUIRE(variant.has_value());
  auto stored_variant = store_execution_value(db, hist::codec_role_structure_v1, *variant);
  // The takeover keeps its predecessor's structure; the variant has the same job and role and a different unit.
  corrupt(path, {std::format("UPDATE generations SET structure_id = {} WHERE reason = 'takeover'", stored_variant.id)});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects recreated without a retired predecessor") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  commit_with(path, [&](const execution_snapshot& s) {
    return role_load(s, "load-3", seed_revision(path, "three"), "reviewer", "remove");
  });
  commit_with(path, [&](const execution_snapshot& s) {
    return role_load(s, "load-4", seed_revision(path, "four"), "reviewer", "readd");
  });
  require_verifies(path);
  const auto remove_seq = nth_latest_seq(path, "declaration_accepted", 1);
  const auto readd_seq  = nth_latest_seq(path, "declaration_accepted", 0);
  // The removal now leaves the reviewer ready, so the recreated generation does not follow a retired row.
  corrupt(path,
          {std::format("UPDATE transitions SET after_state = 'ready', reason = 'prerequisite_available' WHERE event_seq = {} AND "
                       "role = 'reviewer'",
                       remove_seq),
           std::format("UPDATE transitions SET before_state = 'ready' WHERE event_seq = {} AND role = 'reviewer'", readd_seq)});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects structural change with an unchanged structure") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  commit_with(path, [&](const execution_snapshot& s) {
    return role_load(s, "load-3", seed_revision(path, "three"), "reviewer", "structural");
  });
  require_verifies(path);
  // The structural generation keeps the activated structure: the digest does not differ from its predecessor.
  corrupt(path, {"UPDATE generations SET structure_id = (SELECT structure_id FROM generations WHERE job = 'alpha' AND role = "
                 "'reviewer' AND reason = 'activated') "
                 "WHERE reason = 'structural_change'"});
  expect_corrupt(path);
}

TEST_CASE("format3 snapshot decodes the recorded revision values") {
  scratch    dir;
  const auto path     = activated_ledger(dir);
  const auto snapshot = snapshot_of(path);
  REQUIRE(snapshot.revision.has_value());
  const auto& revision = *snapshot.revision;
  auto        db       = open_raw(path);
  REQUIRE(revision.interpretation_value != nullptr);
  REQUIRE(revision.versions_value != nullptr);
  const auto stdlib_hex = to_hex(blob_of(db, std::format("SELECT digest FROM records WHERE id = {}", revision.stdlib)));
  REQUIRE(bytes_of(revision.interpretation_value) ==
          bytes_of(seeded_interpretation("one", vrec({{"stdlib_digest", vstr(stdlib_hex)}}))));
  REQUIRE(bytes_of(revision.versions_value) == bytes_of(vrec({{"stdlib_digest", vstr(stdlib_hex)}})));
  REQUIRE(revision.stdlib_digest == stdlib_hex);
  REQUIRE(revision.source_digest ==
          to_hex(blob_of(db, std::format("SELECT digest FROM records WHERE id = {}", revision.source))));
  REQUIRE(revision.registry_value != nullptr);
  const auto registry_hex = to_hex(blob_of(db, std::format("SELECT digest FROM records WHERE id = {}", revision.registry)));
  REQUIRE(bytes_of(revision.registry_value) ==
          bytes_of(vrec({{"id", vstr(decimal(revision.registry))}, {"digest", vstr(registry_hex)}})));
  // An interpretation root that is a file record, not a canonical_v1 value, is ledger_corrupt.
  const auto file_root = int_of(db, "SELECT child_id FROM tree_edges WHERE name = 'stdlib.lua'");
  auto       rewrite = payload_update_sql(path, seq_of_type(path, "declaration_accepted"), [&](hist::event_payload_v2& payload) {
    payload.references["interpretation"] = std::to_string(file_root);
  });
  corrupt(path, {std::format("UPDATE revisions SET interpretation_id = {} WHERE id = 1", file_root), rewrite});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a revision whose registry shape or stdlib digest disagrees") {
  // Sub-case a: the registry root is a directory without the single input.json child (the source tree).
  {
    scratch    dir;
    const auto path = activated_ledger(dir);
    require_verifies(path);
    auto       db     = open_raw(path);
    const auto source = int_of(db, "SELECT source_id FROM revisions WHERE id = 1");
    corrupt(path, {std::format("UPDATE revisions SET registry_id = {} WHERE id = 1", source),
                   payload_update_sql(path, seq_of_type(path, "declaration_accepted"), [&](hist::event_payload_v2& payload) {
                     payload.references["registry"] = std::to_string(source);
                   })});
    expect_corrupt(path);
  }
  // Sub-case b: the versions record names a stdlib digest that is not the stdlib record digest.
  {
    scratch    dir;
    const auto path = activated_ledger(dir);
    require_verifies(path);
    auto       db       = open_raw(path);
    const auto versions = store_value(db, vrec({{"stdlib_digest", vstr(std::string(32, '0'))}})).id;
    corrupt(path, {std::format("UPDATE revisions SET versions_id = {} WHERE id = 1", versions),
                   payload_update_sql(path, seq_of_type(path, "declaration_accepted"), [&](hist::event_payload_v2& payload) {
                     payload.references["versions"] = std::to_string(versions);
                   })});
    expect_corrupt(path);
  }
  // Sub-case c: the source root is a file record, not a directory.
  {
    scratch    dir;
    const auto path = activated_ledger(dir);
    require_verifies(path);
    auto       db        = open_raw(path);
    const auto file_root = int_of(db, "SELECT child_id FROM tree_edges WHERE name = 'source.txt'");
    corrupt(path, {std::format("UPDATE revisions SET source_id = {} WHERE id = 1", file_root),
                   payload_update_sql(path, seq_of_type(path, "declaration_accepted"), [&](hist::event_payload_v2& payload) {
                     payload.references["source"] = std::to_string(file_root);
                   })});
    expect_corrupt(path);
  }
}

namespace {

/// Row counts of every table the execution writer touches; equal counts before and after a refusal mean nothing was written.
std::map<std::string, std::int64_t> row_counts(const std::filesystem::path& path) {
  auto                                db = open_raw(path);
  std::map<std::string, std::int64_t> out;
  for (const auto* table : {"records", "revisions", "events", "requests", "job_activations", "generations", "runs", "dispatches",
                            "transitions", "role_state", "run_state", "current_state", "tree_edges"})
    out[table] = count_rows(db, table);
  return out;
}

/// A claim whose change also carries a derived transition of another role of the active job (EX-4 carrying events).
execution_change carried_claim(const execution_snapshot& snapshot, std::string_view request_id, std::string_view job,
                               std::string_view role, std::string_view carried_role, std::string_view after,
                               std::string_view reason) {
  auto        change  = claim_change(snapshot, request_id, job, role);
  const auto* carried = find_role(snapshot, job, carried_role);
  REQUIRE(carried != nullptr);
  change.transitions.push_back(execution_transition_input{.job            = std::string(job),
                                                          .role           = std::string(carried_role),
                                                          .before_state   = carried->state,
                                                          .before_drifted = false,
                                                          .after_state    = std::string(after),
                                                          .after_drifted  = false,
                                                          .reason         = std::string(reason)});
  for (auto& entry : change.vector)
    if (entry.role == carried_role)
      entry = execution_vector_entry{
          .job = entry.job, .role = entry.role, .state = std::string(after), .drifted = false, .run = std::nullopt};
  return change;
}

/// Three worker roles, all ready after activation, so claims of distinct roles can run on one snapshot.
std::vector<role_plan> three_workers() {
  return {role_plan{.role = "builder", .kind = "worker", .after = "ready"},
          role_plan{.role = "checker", .kind = "worker", .after = "ready"},
          role_plan{.role = "linter", .kind = "worker", .after = "ready"}};
}

/// Replace one field of a value record, keeping every other field.
value_ptr with_field(const value_ptr& input, const std::string& key, const value_ptr& replacement) {
  value copy       = *input;
  copy.fields[key] = replacement;
  return std::make_shared<const value>(std::move(copy));
}

/// A fresh ledger with one accepted revision (seeded "one") and job alpha activated with the given roles.
std::filesystem::path activated_ledger_with(const scratch& dir, const std::vector<role_plan>& roles) {
  const auto path = dir.ledger();
  REQUIRE(initialize_format3(path, workspace_name).has_value());
  commit_load(path, "load-1", seed_revision(path, "one"));
  commit_activate(path, "activate-1", "alpha", roles);
  return path;
}

/// Claim-first ledger with three ready workers, claim builder as run 1 and checker as run 2.
std::filesystem::path two_claims_ledger(const scratch& dir) {
  auto path = activated_ledger_with(dir, three_workers());
  commit_claim(path, "claim-builder", "alpha", "builder");
  commit_claim(path, "claim-checker", "alpha", "checker");
  return path;
}

} // namespace

TEST_CASE("format3 verification rejects a load request body that disagrees with its event") {
  // Three sub-cases, each a fresh ledger: the body operation, the actor and the expected head disagree with the event.
  struct sub_case {
    std::string                                       name;
    std::function<hist::request_v2(hist::request_v2)> mutate;
  };
  const std::vector<sub_case> cases = {
      {"operation",
       [](hist::request_v2 body) {
         body.operation = "activate";
         return body;
       }},
      {"actor",
       [](hist::request_v2 body) {
         body.actor = "intruder";
         return body;
       }},
      {"expected head",
       [](hist::request_v2 body) {
         body.expected_head = "none";
         return body;
       }},
  };
  for (const auto& entry : cases) {
    scratch    dir;
    const auto path = two_revision_ledger(dir);
    require_verifies(path);
    auto             db = open_raw(path);
    hist::request_v2 body;
    body.operation                                                 = "load";
    body.actor                                                     = "tester";
    body.expected_head                                             = "1";
    body.workspace_argument                                        = std::string(workspace_name);
    body.registry_argument                                         = "registry";
    body                                                           = entry.mutate(body);
    std::expected<std::string, hist::record_error>         encoded = std::unexpected(hist::record_error{});
    std::expected<hist::record_digest, hist::record_error> digest  = std::unexpected(hist::record_error{});
    std::int64_t                                           codec   = 2;
    if (entry.name == "operation") {
      // The version 2 frame admits only the load operation, so a body naming another operation is a version 3 frame.
      const hist::request_v3 other{.operation = "activate",
                                   .actor     = "tester",
                                   .parameters =
                                       params_bytes({{"expect_job", "NONE"}, {"expect_revision", "1"}, {"job", "alpha"}}),
                                   .profile_argument = std::nullopt};
      encoded = hist::encode_request_v3(other);
      digest  = hist::digest_request_v3(other);
      codec   = 3;
    } else {
      encoded = hist::encode_request_v2(body);
      digest  = hist::digest_request_v2(body);
    }
    REQUIRE(encoded.has_value());
    REQUIRE(digest.has_value());
    const auto body_id = insert_frame(db, 4, codec, digest_text(*digest), *encoded);
    corrupt(path, {std::format("UPDATE requests SET body_id = {} WHERE request_id = 'load-2'", body_id)});
    expect_corrupt(path);
  }
}

TEST_CASE("format3 verification rejects a saved load result that differs from the computed result") {
  {
    scratch    dir;
    const auto path = two_revision_ledger(dir);
    require_verifies(path);
    // The stored result equals the one computed from the rows: the positive side of invariant 20 (storage.cpp 911-918).
    auto db = open_raw(path);
    REQUIRE(blob_of(db, "SELECT result FROM requests WHERE request_id = 'load-2'") ==
            expected_load_result(path, "load-2", [](load_result&) {}));
  }
  struct sub_case {
    std::function<void(load_result&)> mutate;
  };
  const std::vector<sub_case> cases = {
      {[](load_result& result) { result.event = result.event + 1; }},
      {[](load_result& result) { result.parent = std::nullopt; }},
      {[](load_result& result) { result.outcome = load_outcome::noop; }},
      {[](load_result& result) { result.request_id = "load-9"; }},
  };
  for (const auto& entry : cases) {
    scratch    dir;
    const auto path = two_revision_ledger(dir);
    require_verifies(path);
    corrupt(path, {std::format("UPDATE requests SET result = {} WHERE request_id = 'load-2'",
                               blob_literal(expected_load_result(path, "load-2", entry.mutate)))});
    expect_corrupt(path);
  }
  {
    scratch    dir;
    const auto path = two_revision_ledger(dir);
    require_verifies(path);
    corrupt(path, {"UPDATE requests SET result = X'7b7d' WHERE request_id = 'load-2'"});
    expect_corrupt(path);
  }
}

TEST_CASE("format3 verification rejects an accepted revision identical to the previous active revision") {
  scratch    dir;
  const auto path = two_revision_ledger(dir);
  require_verifies(path);
  // The second revision's five records are raw copies of the first revision's records: equal kind, codec and bytes.
  auto       db       = open_raw(path);
  const auto source   = copy_record(db, static_cast<std::uint64_t>(int_of(db, "SELECT source_id FROM revisions WHERE id = 1")));
  const auto registry = copy_record(db, static_cast<std::uint64_t>(int_of(db, "SELECT registry_id FROM revisions WHERE id = 1")));
  const auto stdlib   = copy_record(db, static_cast<std::uint64_t>(int_of(db, "SELECT stdlib_id FROM revisions WHERE id = 1")));
  const auto interpretation =
      copy_record(db, static_cast<std::uint64_t>(int_of(db, "SELECT interpretation_id FROM revisions WHERE id = 1")));
  const auto versions = copy_record(db, static_cast<std::uint64_t>(int_of(db, "SELECT versions_id FROM revisions WHERE id = 1")));
  const auto event_seq = seq_of_type(path, "declaration_accepted");
  corrupt(path, {std::format("UPDATE revisions SET source_id = {}, registry_id = {}, stdlib_id = {}, interpretation_id = {}, "
                             "versions_id = {} WHERE id = 2",
                             source, registry, stdlib, interpretation, versions),
                 payload_update_sql(path, event_seq, [&](hist::event_payload_v2& payload) {
                   payload.references["source"]         = std::to_string(source);
                   payload.references["registry"]       = std::to_string(registry);
                   payload.references["stdlib"]         = std::to_string(stdlib);
                   payload.references["interpretation"] = std::to_string(interpretation);
                   payload.references["versions"]       = std::to_string(versions);
                 })});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a load noop whose references differ from the active references") {
  // Two sub-cases, each a fresh ledger: one differing reference, and an after head that differs from the active head.
  const std::vector<std::function<void(hist::event_payload_v2&)>> cases = {
      [](hist::event_payload_v2& payload) { payload.references["source"] = "99"; },
      [](hist::event_payload_v2& payload) { payload.after.active_revision = "99"; },
  };
  for (const auto& mutate : cases) {
    scratch    dir;
    const auto path = activated_ledger(dir);
    commit_with(path, [&](const execution_snapshot& s) {
      return load_change(s, "load-2", head_text(s), revision_input(ids_of(s), true));
    });
    require_verifies(path);
    corrupt(path, {payload_update_sql(path, seq_of_type(path, "load_noop"), mutate)});
    expect_corrupt(path);
  }
}

TEST_CASE("execution replay returns the saved bytes including the token after later events") {
  scratch    dir;
  const auto path      = activated_ledger(dir);
  const auto snapshot  = snapshot_of(path);
  const auto change    = claim_change(snapshot, "claim-1", "alpha", "builder");
  auto       committed = commit_execution(path, snapshot, change);
  REQUIRE(committed.has_value());
  REQUIRE(committed->token.has_value());
  commit_load(path, "load-2", seed_revision(path, "two"));
  auto lookup = lookup_execution_request(path, "claim-1", change.request);
  REQUIRE(lookup.has_value());
  REQUIRE(lookup->state == request_state::replay);
  REQUIRE(lookup->result == committed->saved_result);
  REQUIRE(lookup->result.find(*committed->token) != std::string::npos);
}

TEST_CASE("execution same request id with changed content conflicts") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  commit_claim(path, "claim-1", "alpha", "builder");
  const auto snapshot = snapshot_of(path);
  auto       changed  = claim_change(snapshot, "claim-1", "alpha", "builder");
  changed.actor       = "intruder";
  changed.request     = hist::request_v3{.operation        = "claim",
                                         .actor            = "intruder",
                                         .parameters       = params_bytes({{"job", "alpha"}, {"role", "builder"}}),
                                         .profile_argument = std::nullopt};
  const auto before   = row_counts(path);
  auto       lookup   = lookup_execution_request(path, "claim-1", changed.request);
  REQUIRE(lookup.has_value());
  REQUIRE(lookup->state == request_state::conflict);
  require_code(commit_execution(path, snapshot, changed), execution_error_code::request_conflict);
  REQUIRE(row_counts(path) == before);
}

TEST_CASE("execution request id never replays across request versions") {
  scratch    dir;
  const auto path         = activated_ledger(dir);
  const auto load_framing = load_change(snapshot_of(path), "load-1", "none", revision_input(seed_revision(path, "one"))).request;
  // load-1 was stored as request_v2; the same id with a request_v3 framing conflicts rather than replays.
  auto v3    = execution_framing{hist::request_v3{.operation        = "claim",
                                                  .actor            = "tester",
                                                  .parameters       = params_bytes({{"job", "alpha"}, {"role", "builder"}}),
                                                  .profile_argument = std::nullopt}};
  auto cross = lookup_execution_request(path, "load-1", v3);
  REQUIRE(cross.has_value());
  REQUIRE(cross->state == request_state::conflict);
  auto same = lookup_execution_request(path, "load-1", load_framing);
  REQUIRE(same.has_value());
  REQUIRE(same->state == request_state::replay);
  auto claim_as_v2 = lookup_execution_request(path, "activate-1", load_framing);
  REQUIRE(claim_as_v2.has_value());
  REQUIRE(claim_as_v2->state == request_state::conflict);
}

TEST_CASE("execution replay is checked before preconditions") {
  scratch    dir;
  const auto path   = activated_ledger(dir);
  const auto stale  = snapshot_of(path);
  const auto change = claim_change(stale, "claim-1", "alpha", "builder");
  auto       first  = commit_execution(path, stale, change);
  REQUIRE(first.has_value());
  // The same change against the same stale snapshot: the builder is running now, so only the replay can succeed.
  auto second = commit_execution(path, stale, change);
  REQUIRE(second.has_value());
  REQUIRE(second->replayed);
  REQUIRE(second->saved_result == first->saved_result);
}

TEST_CASE("execution failed commit leaves the request id unused") {
  scratch    dir;
  const auto path  = activated_ledger(dir);
  const auto stale = snapshot_of(path);
  // A load moves the active revision, so the stale claim fails its active_revision precondition.
  commit_load(path, "load-2", seed_revision(path, "two"));
  require_code(commit_execution(path, stale, claim_change(stale, "claim-1", "alpha", "builder")),
               execution_error_code::state_conflict);
  auto lookup = lookup_execution_request(path, "claim-1", claim_change(stale, "claim-1", "alpha", "builder").request);
  REQUIRE(lookup.has_value());
  REQUIRE(lookup->state == request_state::unused);
  commit_claim(path, "claim-1", "alpha", "builder");
}

TEST_CASE("execution lost update guard keeps both claims") {
  scratch    dir;
  const auto path     = activated_ledger_with(dir, three_workers());
  const auto snapshot = snapshot_of(path);
  auto       first    = commit_execution(path, snapshot, claim_change(snapshot, "claim-builder", "alpha", "builder"));
  REQUIRE(first.has_value());
  // The second claim was prepared on the same snapshot; its vector entry for the builder is stale and must not be written.
  auto second = commit_execution(path, snapshot, claim_change(snapshot, "claim-checker", "alpha", "checker"));
  REQUIRE(second.has_value());
  auto db = open_raw(path);
  REQUIRE(text_of(db, "SELECT state FROM role_state WHERE role = 'builder'") == "running");
  REQUIRE(text_of(db, "SELECT state FROM role_state WHERE role = 'checker'") == "running");
  REQUIRE(int_of(db, "SELECT run_id FROM role_state WHERE role = 'builder'") == 1);
  REQUIRE(int_of(db, "SELECT run_id FROM role_state WHERE role = 'checker'") == 2);
  require_verifies(path);
}

TEST_CASE("execution snapshot sequence is not a precondition") {
  scratch    dir;
  const auto path     = activated_ledger_with(dir, three_workers());
  const auto snapshot = snapshot_of(path);
  commit_claim(path, "claim-builder", "alpha", "builder");
  REQUIRE(snapshot.last_seq < snapshot_of(path).last_seq);
  // last_seq moved, but no role of the second claim changed, so the claim still succeeds on the older snapshot.
  auto second = commit_execution(path, snapshot, claim_change(snapshot, "claim-checker", "alpha", "checker"));
  REQUIRE(second.has_value());
  REQUIRE_FALSE(second->replayed);
}

TEST_CASE("execution stale fact version refuses state conflict") {
  scratch    dir;
  const auto path  = activated_ledger(dir);
  const auto stale = snapshot_of(path);
  // A committed re-activation keeps every asserted role state and bumps every fact version, so only fact_version fails.
  commit_activate(path, "reactivate-1", "alpha", standard_roles());
  const auto before = row_counts(path);
  require_code(commit_execution(path, stale, activate_change(stale, "reactivate-stale", "alpha", standard_roles())),
               execution_error_code::state_conflict);
  REQUIRE(row_counts(path) == before);
}

TEST_CASE("execution claim between snapshot and activation refuses state conflict") {
  scratch    dir;
  const auto path  = activated_ledger(dir);
  const auto stale = snapshot_of(path);
  commit_claim(path, "claim-1", "alpha", "builder");
  const auto before = row_counts(path);
  require_code(commit_execution(path, stale, activate_change(stale, "reactivate-stale", "alpha", standard_roles())),
               execution_error_code::state_conflict);
  REQUIRE(row_counts(path) == before);
  // The same request id on a fresh snapshot succeeds.
  commit_activate(path, "reactivate-stale", "alpha", standard_roles());
  require_verifies(path);
}

TEST_CASE("execution stale active revision refuses state conflict") {
  scratch    dir;
  const auto path  = activated_ledger(dir);
  const auto stale = snapshot_of(path);
  // The load keeps every role state and bumps no fact version, so the stale claim fails only its active_revision member.
  commit_load(path, "load-2", seed_revision(path, "two"));
  const auto before = row_counts(path);
  require_code(commit_execution(path, stale, claim_change(stale, "claim-1", "alpha", "builder")),
               execution_error_code::state_conflict);
  REQUIRE(row_counts(path) == before);
}

TEST_CASE("execution stale active job refuses state conflict") {
  scratch    dir;
  const auto path = dir.ledger();
  REQUIRE(initialize_format3(path, workspace_name).has_value());
  commit_load(path, "load-1", seed_revision(path, "one"));
  const auto stale = snapshot_of(path);
  // Another activation commits first; the stale activation of beta had expected no active job.
  commit_activate(path, "activate-a", "alpha", standard_roles());
  const auto before = row_counts(path);
  require_code(commit_execution(path, stale,
                                activate_change(stale, "activate-b", "beta",
                                                {role_plan{.role = "worker", .kind = "worker", .after = "ready"}})),
               execution_error_code::state_conflict);
  REQUIRE(row_counts(path) == before);
}

TEST_CASE("execution wrong expected generation refuses state conflict") {
  // EQUIVALENT-MUTANT SITE (L21): every event that changes the generation also bumps fact_version, so this refusal is
  // reached through fact_version. The row pins the code and the refusal; its mutant is an expected survivor.
  scratch    dir;
  const auto path  = activated_ledger(dir);
  const auto stale = snapshot_of(path);
  commit_activate(path, "reactivate-1", "alpha", standard_roles());
  const auto before = row_counts(path);
  require_code(commit_execution(path, stale, claim_change(stale, "claim-1", "alpha", "builder")),
               execution_error_code::state_conflict);
  REQUIRE(row_counts(path) == before);
}

TEST_CASE("execution refuses a change whose expected set is not the fixed set") {
  scratch    dir;
  const auto path     = activated_ledger(dir);
  const auto snapshot = snapshot_of(path);
  const auto before   = row_counts(path);
  auto       refused  = [&](execution_change change) {
    require_code(commit_execution(path, snapshot, change), execution_error_code::invalid_change);
    REQUIRE(row_counts(path) == before);
  };
  const auto base = claim_change(snapshot, "claim-1", "alpha", "builder");
  // Omitted member: the builder fact version is missing.
  {
    auto change = base;
    std::erase_if(change.expected, [](const execution_expectation& member) {
      return member.kind == kind_t::fact_version && member.role == "builder";
    });
    refused(change);
  }
  // Extra member: a fact version for a pair that has no snapshot row.
  {
    auto change  = base;
    auto extra   = member(kind_t::fact_version);
    extra.job    = "alpha";
    extra.role   = "nobody";
    extra.number = 1;
    change.expected.push_back(extra);
    refused(change);
  }
  // Duplicate member: the target role state appears twice.
  {
    auto change = base;
    for (const auto& candidate : base.expected)
      if (candidate.kind == kind_t::role_state)
        change.expected.push_back(candidate);
    refused(change);
  }
  // Value differing from the snapshot: the target generation is not the snapshot generation.
  {
    auto change = base;
    for (auto& candidate : change.expected)
      if (candidate.kind == kind_t::generation_current)
        candidate.number = candidate.number + 100;
    refused(change);
  }
  // A claim whose snapshot target is blocked (the reviewer) is refused.
  {
    auto change = claim_change(snapshot, "claim-1", "alpha", "reviewer");
    refused(change);
  }
  // Positive sub-case: one no_row member for an active-job role without a snapshot row is accepted, and the claim commits.
  {
    auto change   = base;
    auto absent   = member(kind_t::role_state);
    absent.job    = "alpha";
    absent.role   = "extra";
    absent.no_row = true;
    change.expected.push_back(absent);
    auto result = commit_execution(path, snapshot, change);
    REQUIRE(result.has_value());
  }
}

TEST_CASE("execution concurrent claim of one role is a claim conflict") {
  scratch    dir;
  const auto path  = activated_ledger(dir);
  const auto stale = snapshot_of(path);
  commit_claim(path, "claim-1", "alpha", "builder");
  const auto before = row_counts(path);
  auto       result = commit_execution(path, stale, claim_change(stale, "claim-2", "alpha", "builder"));
  REQUIRE_FALSE(result.has_value());
  REQUIRE(result.error().code == execution_error_code::claim_conflict);
  REQUIRE(result.error().state == "running");
  REQUIRE(row_counts(path) == before);
}

namespace {

/// A load that drops the active job: every role of the job retires (rows M13 and M14), running roles fence their runs,
/// and the active job becomes none, so the vector is empty.
execution_change drop_active_job_load(const execution_snapshot& snapshot, std::string_view request_id, const revision_ids& ids) {
  auto change = load_change(snapshot, request_id, head_text(snapshot), revision_input(ids));
  for (const auto& row : snapshot.roles)
    if (row.job == snapshot.active_job)
      change.transitions.push_back(execution_transition_input{.job            = row.job,
                                                              .role           = row.role,
                                                              .before_state   = row.state,
                                                              .before_drifted = row.drifted,
                                                              .after_state    = "retired",
                                                              .after_drifted  = false,
                                                              .reason         = "retired"});
  change.job = "";
  change.vector.clear();
  derive_fences(snapshot, change);
  return change;
}

/// Commit a claim of `claimed` that also carries a derived transition of `carried`, so the carried role reaches `after`.
execution_commit_result commit_carrier(const std::filesystem::path& path, std::string_view request_id, std::string_view claimed,
                                       std::string_view carried, std::string_view after, std::string_view reason) {
  return commit_with(
      path, [&](const execution_snapshot& s) { return carried_claim(s, request_id, "alpha", claimed, carried, after, reason); });
}

/// Refusal helper: an invalid change writes nothing and the ledger keeps its row counts.
void expect_invalid(const std::filesystem::path& path, const execution_snapshot& snapshot, const execution_change& change) {
  const auto before = row_counts(path);
  require_code(commit_execution(path, snapshot, change), execution_error_code::invalid_change);
  REQUIRE(row_counts(path) == before);
}

} // namespace

TEST_CASE("execution claim conflict detail follows the locked state") {
  // Locked blocked, done and retired: claim_conflict with the locked state.
  const std::vector<std::pair<std::string, std::string>> conflicts = {
      {"blocked", "prerequisite_unavailable"}, {"done", "completed"}, {"retired", "retired"}};
  for (const auto& [after, reason] : conflicts) {
    scratch    dir;
    const auto path  = activated_ledger_with(dir, three_workers());
    const auto stale = snapshot_of(path);
    commit_carrier(path, "carrier-1", "checker", "builder", after, reason);
    auto result = commit_execution(path, stale, claim_change(stale, "claim-builder", "alpha", "builder"));
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code == execution_error_code::claim_conflict);
    REQUIRE(result.error().state == after);
  }
  // Locked stale: state_conflict, not claim_conflict.
  {
    scratch    dir;
    const auto path  = activated_ledger_with(dir, three_workers());
    const auto stale = snapshot_of(path);
    commit_carrier(path, "carrier-1", "checker", "builder", "stale", "evidence_inapplicable");
    require_code(commit_execution(path, stale, claim_change(stale, "claim-builder", "alpha", "builder")),
                 execution_error_code::state_conflict);
  }
  // Locked ready while the snapshot said stale: state_conflict.
  {
    scratch    dir;
    const auto path = activated_ledger_with(dir, three_workers());
    commit_carrier(path, "carrier-1", "checker", "builder", "stale", "evidence_inapplicable");
    const auto stale = snapshot_of(path);
    commit_carrier(path, "carrier-2", "linter", "builder", "ready", "prerequisite_available");
    require_code(commit_execution(path, stale, claim_change(stale, "claim-builder", "alpha", "builder")),
                 execution_error_code::state_conflict);
  }
}

TEST_CASE("execution takeover of a superseded run refuses state conflict") {
  scratch    dir;
  const auto path  = claimed_ledger(dir);
  const auto stale = snapshot_of(path);
  commit_takeover_current(path, "takeover-1", "alpha", "builder");
  const auto before = row_counts(path);
  require_code(commit_execution(path, stale, takeover_change(stale, "takeover-2", "alpha", "builder", 1)),
               execution_error_code::state_conflict);
  REQUIRE(row_counts(path) == before);
}

TEST_CASE("execution fenced detail names the run state") {
  scratch    dir;
  const auto path      = activated_ledger(dir);
  const auto claim     = commit_claim(path, "claim-1", "alpha", "builder");
  const auto old_token = *claim.token;
  const auto takeover  = commit_takeover_current(path, "takeover-1", "alpha", "builder");
  const auto new_token = *takeover.token;
  require_code(authorize_run_token(path, old_token), execution_error_code::fenced);
  REQUIRE(authorize_run_token(path, old_token).error().state == "fenced");
  auto wrong   = new_token;
  wrong.back() = wrong.back() == 'a' ? 'b' : 'a';
  require_code(authorize_run_token(path, wrong), execution_error_code::fenced);
  REQUIRE(authorize_run_token(path, wrong).error().state == "active");
  REQUIRE(authorize_run_token(path, new_token).has_value());
}

TEST_CASE("execution takeover naming no run of the role is an invalid change") {
  scratch    dir;
  const auto path     = two_claims_ledger(dir);
  const auto snapshot = snapshot_of(path);
  // A run that does not exist, and the run of another role: neither is the builder's active run.
  expect_invalid(path, snapshot, takeover_change(snapshot, "takeover-x", "alpha", "builder", 999));
  expect_invalid(path, snapshot, takeover_change(snapshot, "takeover-x", "alpha", "builder", 2));
}

TEST_CASE("execution old token is fenced after takeover") {
  scratch    dir;
  const auto path      = activated_ledger(dir);
  const auto old_token = *commit_claim(path, "claim-1", "alpha", "builder").token;
  const auto new_token = *commit_takeover_current(path, "takeover-1", "alpha", "builder").token;
  const auto before    = file_contents(dir.root);
  require_code(authorize_run_token(path, old_token), execution_error_code::fenced);
  REQUIRE(file_contents(dir.root) == before);
  REQUIRE(authorize_run_token(path, new_token).has_value());
}

TEST_CASE("execution wrong secret is fenced") {
  scratch    dir;
  const auto path  = activated_ledger(dir);
  const auto token = *commit_claim(path, "claim-1", "alpha", "builder").token;
  auto       wrong = token;
  wrong.back()     = wrong.back() == 'a' ? 'b' : 'a';
  require_code(authorize_run_token(path, wrong), execution_error_code::fenced);
  REQUIRE(authorize_run_token(path, wrong).error().state == "active");
  REQUIRE(authorize_run_token(path, token).has_value());
}

TEST_CASE("execution run token grammar and secret") {
  scratch    dir;
  const auto path = activated_ledger_with(dir, three_workers());
  commit_activate(path, "reactivate-1", "alpha", three_workers());
  const auto first  = *commit_claim(path, "claim-builder", "alpha", "builder").token;
  const auto second = *commit_claim(path, "claim-checker", "alpha", "checker").token;
  auto       db     = open_raw(path);
  // The generation id of the run differs from its ordinal, so the token must carry the ordinal.
  REQUIRE(int_of(db, "SELECT generation_id FROM runs WHERE id = 1") != 2);
  auto parsed = parse_run_token(first);
  REQUIRE(parsed.has_value());
  REQUIRE(parsed->run_id == 1);
  REQUIRE(parsed->generation_ordinal == 2);
  REQUIRE(format_run_token(*parsed) == first);
  const auto stored_secret = blob_of(db, "SELECT secret FROM runs WHERE id = 1");
  REQUIRE(stored_secret.size() == 16);
  REQUIRE(std::string(parsed->secret.begin(), parsed->secret.end()) == stored_secret);
  auto other = parse_run_token(second);
  REQUIRE(other.has_value());
  REQUIRE(other->secret != parsed->secret);
}

TEST_CASE("execution token text appears only in the saved request result") {
  scratch    dir;
  const auto path  = activated_ledger(dir);
  const auto claim = commit_claim(path, "claim-1", "alpha", "builder");
  const auto token = *claim.token;
  auto       db    = open_raw(path);
  REQUIRE(int_of(db, std::format("SELECT COUNT(*) FROM requests WHERE instr(CAST(result AS TEXT), '{}') > 0", token)) == 1);
  REQUIRE(int_of(db, std::format("SELECT COUNT(*) FROM events WHERE instr(CAST(payload AS TEXT), '{}') > 0", token)) == 0);
  REQUIRE(int_of(db, std::format("SELECT COUNT(*) FROM records WHERE instr(CAST(encoded AS TEXT), '{}') > 0", token)) == 0);
  REQUIRE(int_of(db, std::format("SELECT COUNT(*) FROM transitions WHERE instr(CAST(reason AS TEXT), '{}') > 0", token)) == 0);
}

TEST_CASE("execution regrouped role gets a new generation and no inherited token") {
  scratch    dir;
  const auto path        = activated_ledger(dir);
  const auto token       = *commit_claim(path, "claim-1", "alpha", "builder").token;
  auto       db          = open_raw(path);
  const auto fact_before = int_of(db, "SELECT fact_version FROM role_state WHERE role = 'builder'");
  commit_with(path, [&](const execution_snapshot& s) {
    return role_load(s, "load-3", seed_revision(path, "three"), "builder", "remove");
  });
  commit_with(
      path, [&](const execution_snapshot& s) { return role_load(s, "load-4", seed_revision(path, "four"), "builder", "readd"); });
  REQUIRE(int_of(db, "SELECT ordinal FROM generations WHERE role = 'builder' AND reason = 'recreated'") == 2);
  REQUIRE(int_of(db, "SELECT fact_version FROM role_state WHERE role = 'builder'") > fact_before);
  REQUIRE(text_of(db, "SELECT state FROM run_state WHERE run_id = 1") == "fenced");
  REQUIRE(int_of(db, "SELECT COUNT(*) FROM role_state WHERE role = 'builder' AND run_id IS NULL") == 1);
  require_code(authorize_run_token(path, token), execution_error_code::fenced);
}

TEST_CASE("execution structural load fences the active run") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  commit_claim(path, "claim-1", "alpha", "builder");
  commit_with(path, [&](const execution_snapshot& s) {
    auto change = load_change(s, "load-3", head_text(s), revision_input(seed_revision(path, "three")));
    change.generations.push_back(execution_generation_input{.job       = "alpha",
                                                            .role      = "builder",
                                                            .reason    = "structural_change",
                                                            .structure = structure_of("alpha", "builder", "worker", "unit_two")});
    change.transitions.push_back(execution_transition_input{.job            = "alpha",
                                                            .role           = "builder",
                                                            .before_state   = std::string("running"),
                                                            .before_drifted = false,
                                                            .after_state    = "blocked",
                                                            .after_drifted  = false,
                                                            .reason         = "structural_change"});
    settle_vector(change, s);
    return change;
  });
  auto db = open_raw(path);
  REQUIRE(text_of(db, "SELECT state FROM run_state WHERE run_id = 1") == "fenced");
  REQUIRE(text_of(db, "SELECT state FROM role_state WHERE role = 'builder'") == "blocked");
  REQUIRE(int_of(db, "SELECT COUNT(*) FROM role_state WHERE role = 'builder' AND run_id IS NULL") == 1);
  REQUIRE(text_of(db, "SELECT reason FROM generations WHERE role = 'builder' AND ordinal = 2") == "structural_change");
}

TEST_CASE("execution load that drops the active job retires every role") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  commit_claim(path, "claim-1", "alpha", "builder");
  commit_with(path, [&](const execution_snapshot& s) { return drop_active_job_load(s, "load-3", seed_revision(path, "three")); });
  auto db = open_raw(path);
  REQUIRE(int_of(db, "SELECT COUNT(*) FROM current_state WHERE active_job IS NULL") == 1);
  REQUIRE(int_of(db, "SELECT COUNT(*) FROM current_state WHERE active_activation_id IS NOT NULL") == 1);
  REQUIRE(int_of(db, "SELECT COUNT(*) FROM role_state WHERE state <> 'retired'") == 0);
  REQUIRE(text_of(db, "SELECT state FROM run_state WHERE run_id = 1") == "fenced");
}

TEST_CASE("execution re-activation opens new generations and fences runs") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  commit_claim(path, "claim-1", "alpha", "builder");
  commit_activate(path, "reactivate-1", "alpha", standard_roles());
  auto db = open_raw(path);
  REQUIRE(int_of(db, "SELECT COUNT(*) FROM generations WHERE job = 'alpha' AND role = 'builder' AND reason = 'activated'") == 2);
  REQUIRE(int_of(db, "SELECT ordinal FROM generations WHERE job = 'alpha' AND role = 'builder' AND reason = 'activated' ORDER BY "
                     "id DESC LIMIT 1") == 2);
  REQUIRE(text_of(db, "SELECT state FROM run_state WHERE run_id = 1") == "fenced");
  REQUIRE(text_of(db, "SELECT state FROM role_state WHERE role = 'builder'") == "ready");
  REQUIRE(text_of(db, "SELECT state FROM role_state WHERE role = 'reviewer'") == "blocked");
  REQUIRE(int_of(db, "SELECT COUNT(*) FROM role_state WHERE state = 'retired'") == 0);
}

TEST_CASE("execution activating another job retires the previous job") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  commit_claim(path, "claim-1", "alpha", "builder");
  commit_activate(path, "activate-2", "beta", {role_plan{.role = "worker", .kind = "worker", .after = "ready"}});
  auto db = open_raw(path);
  REQUIRE(int_of(db, "SELECT COUNT(*) FROM role_state WHERE job = 'alpha' AND state <> 'retired'") == 0);
  REQUIRE(text_of(db, "SELECT state FROM run_state WHERE run_id = 1") == "fenced");
  REQUIRE(text_of(db, "SELECT state FROM role_state WHERE job = 'beta' AND role = 'worker'") == "ready");
}

TEST_CASE("execution allocates identifiers as max plus one") {
  scratch    dir;
  const auto path   = activated_ledger_with(dir, three_workers());
  const auto first  = commit_claim(path, "claim-builder", "alpha", "builder");
  const auto second = commit_claim(path, "claim-checker", "alpha", "checker");
  REQUIRE(first.event_seq == 4);
  REQUIRE(second.event_seq == 5);
  auto db = open_raw(path);
  REQUIRE(int_of(db, "SELECT MIN(seq) FROM events WHERE seq > 1") == 2);
  REQUIRE(int_of(db, "SELECT MIN(id) FROM job_activations") == 1);
  REQUIRE(int_of(db, "SELECT MIN(id) FROM runs") == 1);
  REQUIRE(int_of(db, "SELECT MAX(id) FROM runs") == 2);
  REQUIRE(int_of(db, "SELECT MIN(id) FROM dispatches") == 1);
  REQUIRE(int_of(db, "SELECT MAX(id) FROM dispatches") == 2);
}

TEST_CASE("execution claim writes the exact fact rows") {
  scratch    dir;
  const auto path      = activated_ledger(dir);
  const auto snapshot  = snapshot_of(path);
  const auto change    = claim_change(snapshot, "claim-1", "alpha", "builder");
  auto       committed = commit_execution(path, snapshot, change);
  REQUIRE(committed.has_value());
  const auto seq        = committed->event_seq;
  auto       db         = open_raw(path);
  const auto generation = int_of(db, "SELECT id FROM generations WHERE job = 'alpha' AND role = 'builder' AND ordinal = 1");
  REQUIRE(text_of(db, std::format("SELECT type FROM events WHERE seq = {}", seq)) == "run_claimed");
  REQUIRE(int_of(db, std::format("SELECT version FROM events WHERE seq = {}", seq)) == 2);
  REQUIRE(text_of(db, std::format("SELECT job FROM events WHERE seq = {}", seq)) == "alpha");
  REQUIRE(text_of(db, std::format("SELECT role FROM events WHERE seq = {}", seq)) == "builder");
  REQUIRE(int_of(db, std::format("SELECT generation_id FROM events WHERE seq = {}", seq)) == generation);
  REQUIRE(int_of(db, std::format("SELECT run_id FROM events WHERE seq = {}", seq)) == 1);
  auto payload =
      hist::decode_event_payload_v2("run_claimed", blob_of(db, std::format("SELECT payload FROM events WHERE seq = {}", seq)));
  REQUIRE(payload.has_value());
  REQUIRE(payload->operation == "claim");
  REQUIRE(payload->reason == "claimed");
  REQUIRE(payload->references.size() == 5);
  REQUIRE(payload->references.count("provenance") == 0);
  REQUIRE(int_of(db, std::format("SELECT ordinal FROM transitions WHERE event_seq = {}", seq)) == 1);
  REQUIRE(text_of(db, std::format("SELECT before_state FROM transitions WHERE event_seq = {}", seq)) == "ready");
  REQUIRE(text_of(db, std::format("SELECT after_state FROM transitions WHERE event_seq = {}", seq)) == "running");
  REQUIRE(text_of(db, std::format("SELECT reason FROM transitions WHERE event_seq = {}", seq)) == "claimed");
  REQUIRE(text_of(db, "SELECT state FROM run_state WHERE run_id = 1") == "active");
  REQUIRE(int_of(db, "SELECT claimed_seq FROM runs WHERE id = 1") == static_cast<std::int64_t>(seq));
  REQUIRE(text_of(db, "SELECT request_id FROM runs WHERE id = 1") == "claim-1");
  REQUIRE(int_of(db, "SELECT COUNT(*) FROM runs WHERE id = 1 AND provenance_id IS NULL") == 1);
  REQUIRE(int_of(db, std::format("SELECT COUNT(*) FROM dispatches WHERE run_id = 1 AND seq = {} AND kind = 'claim'", seq)) == 1);
  const auto packet_id = int_of(db, "SELECT packet_id FROM dispatches WHERE run_id = 1");
  REQUIRE(int_of(db, "SELECT work_id FROM dispatches WHERE run_id = 1") > 0);
  REQUIRE(int_of(db, "SELECT attachment_id FROM requests WHERE request_id = 'claim-1'") == packet_id);
  auto packet = hist::decode_record(blob_of(db, std::format("SELECT encoded FROM records WHERE id = {}", packet_id)));
  REQUIRE(packet.has_value());
  const auto* file = std::get_if<hist::file_record>(&*packet);
  REQUIRE(file != nullptr);
  REQUIRE(file->mode == 420);
  REQUIRE(file->payload == change.packet->markdown);
}

TEST_CASE("execution recomputes the dispatch digests natively") {
  scratch    dir;
  const auto path     = activated_ledger(dir);
  const auto snapshot = snapshot_of(path);
  const auto base     = claim_change(snapshot, "claim-1", "alpha", "builder");
  // Manifest and W job and role consistency is refused natively; the digest rows are refused before COMMIT (V27).
  {
    auto change             = base;
    change.packet->manifest = with_field(change.packet->manifest, "job", vstr("beta"));
    expect_invalid(path, snapshot, change);
  }
  {
    auto change             = base;
    change.packet->manifest = with_field(change.packet->manifest, "role", vstr("checker"));
    expect_invalid(path, snapshot, change);
  }
  {
    auto change         = base;
    change.packet->work = with_field(change.packet->work, "job", vstr("beta"));
    expect_invalid(path, snapshot, change);
  }
  {
    auto change         = base;
    change.packet->work = with_field(change.packet->work, "role", vstr("checker"));
    expect_invalid(path, snapshot, change);
  }
  {
    auto change = base;
    change.packet->manifest =
        with_field(change.packet->manifest, "work", vrec({{"digest", vstr("xxh3_128:00000000000000000000000000000000")}}));
    expect_invalid(path, snapshot, change);
  }
}

TEST_CASE("execution fact version bumps follow the bumping rule") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  auto       db   = open_raw(path);
  auto       fact = [&](std::string_view role) {
    return int_of(db, std::format("SELECT fact_version FROM role_state WHERE role = '{}'", role));
  };
  REQUIRE(fact("builder") == 1);
  REQUIRE(fact("reviewer") == 1);
  commit_claim(path, "claim-1", "alpha", "builder");
  REQUIRE(fact("builder") == 1);
  commit_takeover_current(path, "takeover-1", "alpha", "builder");
  REQUIRE(fact("builder") == 2);
  commit_with(path, [&](const execution_snapshot& s) {
    return load_change(s, "load-noop", head_text(s), revision_input(ids_of(s), true));
  });
  REQUIRE(fact("builder") == 2);
  REQUIRE(fact("reviewer") == 1);
  commit_with(path, [&](const execution_snapshot& s) {
    return role_load(s, "load-3", seed_revision(path, "three"), "reviewer", "structural");
  });
  REQUIRE(fact("reviewer") == 2);
  REQUIRE(fact("builder") == 2);
}

TEST_CASE("execution first transition requires no role state row") {
  // A competing NULL-first row for extra is produced by a load (a claim cannot open a generation, D44). Limit: the load
  // also moves the active revision, so the stale claim below is refused by the locked checks together; the no-row member
  // cannot be isolated from them without writing ledger rows outside the commit protocol.
  scratch    dir;
  const auto path     = activated_ledger_with(dir, three_workers());
  const auto snapshot = snapshot_of(path);
  REQUIRE(
      commit_execution(path, snapshot, add_role_load(snapshot, "load-extra", seed_revision(path, "extra"), "extra")).has_value());
  // The second claim was prepared before the NULL-first row of extra appeared.
  auto second = claim_change(snapshot, "claim-checker", "alpha", "checker");
  second.generations.push_back(execution_generation_input{
      .job = "alpha", .role = "extra", .reason = "structural_change", .structure = structure_of("alpha", "extra", "worker")});
  second.transitions.push_back(execution_transition_input{.job            = "alpha",
                                                          .role           = "extra",
                                                          .before_state   = std::nullopt,
                                                          .before_drifted = std::nullopt,
                                                          .after_state    = "ready",
                                                          .after_drifted  = false,
                                                          .reason         = "prerequisite_available"});
  settle_vector(second, snapshot);
  require_code(commit_execution(path, snapshot, second), execution_error_code::state_conflict);
}

TEST_CASE("execution locked before state recheck refuses a carried transition") {
  // Only sub-case one is built: the second sub-case (a before_drifted difference) needs a drifted running role, which
  // requires the R16 refresh operation and is recorded as a limit.
  scratch    dir;
  const auto path  = activated_ledger_with(dir, three_workers());
  const auto stale = snapshot_of(path);
  commit_claim(path, "claim-checker", "alpha", "checker");
  const auto before = row_counts(path);
  require_code(commit_execution(
                   path, stale,
                   carried_claim(stale, "claim-builder", "alpha", "builder", "checker", "blocked", "prerequisite_unavailable")),
               execution_error_code::state_conflict);
  REQUIRE(row_counts(path) == before);
  commit_claim(path, "claim-builder", "alpha", "builder");
  require_verifies(path);
}

TEST_CASE("execution noop load writes load_noop and bumps nothing") {
  scratch    dir;
  const auto path             = activated_ledger(dir);
  auto       db               = open_raw(path);
  const auto revisions_before = count_rows(db, "revisions");
  const auto fact_before      = int_of(db, "SELECT fact_version FROM role_state WHERE role = 'builder'");
  const auto noop             = commit_with(
      path, [&](const execution_snapshot& s) { return load_change(s, "load-2", head_text(s), revision_input(ids_of(s), true)); });
  REQUIRE(text_of(db, std::format("SELECT type FROM events WHERE seq = {}", noop.event_seq)) == "load_noop");
  REQUIRE(text_of(db, "SELECT outcome FROM requests WHERE request_id = 'load-2'") == "noop");
  REQUIRE(count_rows(db, "revisions") == revisions_before);
  REQUIRE(count_rows(db, "transitions") == 2);
  REQUIRE(int_of(db, "SELECT fact_version FROM role_state WHERE role = 'builder'") == fact_before);
}

TEST_CASE("execution load commits revision event and request") {
  scratch    dir;
  const auto path   = activated_ledger(dir);
  const auto loaded = commit_load(path, "load-2", seed_revision(path, "two"));
  auto       db     = open_raw(path);
  REQUIRE(int_of(db, "SELECT parent_id FROM revisions WHERE id = 2") == 1);
  REQUIRE(text_of(db, std::format("SELECT type FROM events WHERE seq = {}", loaded.event_seq)) == "declaration_accepted");
  REQUIRE(int_of(db, std::format("SELECT version FROM events WHERE seq = {}", loaded.event_seq)) == 2);
  auto payload = hist::decode_event_payload_v2(
      "declaration_accepted", blob_of(db, std::format("SELECT payload FROM events WHERE seq = {}", loaded.event_seq)));
  REQUIRE(payload.has_value());
  REQUIRE(payload->references.size() == 5);
  REQUIRE(int_of(db, "SELECT codec FROM records WHERE id = (SELECT body_id FROM requests WHERE request_id = 'load-2')") == 2);
  REQUIRE(text_of(db, "SELECT outcome FROM requests WHERE request_id = 'load-2'") == "accepted");
  REQUIRE(loaded.saved_result == expected_load_result(path, "load-2", [](load_result&) {}));
  REQUIRE(blob_of(db, "SELECT result FROM requests WHERE request_id = 'load-2'") == loaded.saved_result);
}

TEST_CASE("execution takeover opens a lineage generation and a new run") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  commit_claim(path, "claim-1", "alpha", "builder");
  auto       db = open_raw(path);
  const auto structure =
      int_of(db, "SELECT structure_id FROM generations WHERE job = 'alpha' AND role = 'builder' AND ordinal = 1");
  const auto fact_before = int_of(db, "SELECT fact_version FROM role_state WHERE role = 'builder'");
  const auto takeover    = commit_takeover_current(path, "takeover-1", "alpha", "builder");
  REQUIRE(text_of(db, "SELECT reason FROM generations WHERE role = 'builder' AND ordinal = 2") == "takeover");
  REQUIRE(int_of(db, "SELECT structure_id FROM generations WHERE role = 'builder' AND ordinal = 2") == structure);
  REQUIRE(int_of(db, "SELECT COUNT(*) FROM runs") == 2);
  REQUIRE(text_of(db, "SELECT state FROM run_state WHERE run_id = 1") == "fenced");
  REQUIRE(text_of(db, "SELECT kind FROM dispatches WHERE run_id = 2") == "takeover");
  REQUIRE(int_of(db, "SELECT fact_version FROM role_state WHERE role = 'builder'") == fact_before + 1);
  REQUIRE(text_of(db, std::format("SELECT type FROM events WHERE seq = {}", takeover.event_seq)) == "run_taken_over");
}

TEST_CASE("execution staged commit that is abandoned leaves no effect") {
  scratch    dir;
  const auto path     = activated_ledger(dir);
  const auto snapshot = snapshot_of(path);
  const auto before   = row_counts(path);
  {
    auto staged = stage_execution(path, snapshot, claim_change(snapshot, "claim-1", "alpha", "builder"));
    REQUIRE(staged.has_value());
    staged->abort();
  }
  REQUIRE(row_counts(path) == before);
  {
    auto staged = stage_execution(path, snapshot, claim_change(snapshot, "claim-1", "alpha", "builder"));
    REQUIRE(staged.has_value());
  }
  REQUIRE(row_counts(path) == before);
  // The id stays usable after the abandoned attempts.
  commit_claim(path, "claim-1", "alpha", "builder");
}

TEST_CASE("execution precondition mismatch leaves the ledger unchanged") {
  scratch    dir;
  const auto path  = activated_ledger(dir);
  const auto stale = snapshot_of(path);
  commit_load(path, "load-2", seed_revision(path, "two"));
  const auto before = file_contents(dir.root);
  require_code(commit_execution(path, stale, claim_change(stale, "claim-1", "alpha", "builder")),
               execution_error_code::state_conflict);
  REQUIRE(file_contents(dir.root) == before);
}

TEST_CASE("execution claim stores provenance only when supplied") {
  {
    scratch    dir;
    const auto path = claimed_ledger(dir);
    auto       db   = open_raw(path);
    REQUIRE(int_of(db, "SELECT COUNT(*) FROM runs WHERE provenance_id IS NULL") == 1);
    auto payload =
        hist::decode_event_payload_v2("run_claimed", blob_of(db, "SELECT payload FROM events WHERE type = 'run_claimed'"));
    REQUIRE(payload.has_value());
    REQUIRE(payload->references.count("provenance") == 0);
  }
  {
    scratch    dir;
    const auto path      = activated_ledger(dir);
    const auto snapshot  = snapshot_of(path);
    auto       committed = commit_execution(path, snapshot, claim_change(snapshot, "claim-1", "alpha", "builder", true));
    REQUIRE(committed.has_value());
    auto db = open_raw(path);
    REQUIRE(int_of(db, "SELECT COUNT(*) FROM runs WHERE provenance_id IS NOT NULL") == 1);
    auto payload =
        hist::decode_event_payload_v2("run_claimed", blob_of(db, "SELECT payload FROM events WHERE type = 'run_claimed'"));
    REQUIRE(payload.has_value());
    REQUIRE(payload->references.count("provenance") == 1);
  }
}

TEST_CASE("execution commit refuses oversize provenance") {
  // At the limit the claim commits; one byte more is refused with the limit name and nothing is written.
  {
    scratch    dir;
    const auto path     = activated_ledger(dir);
    const auto snapshot = snapshot_of(path);
    auto       change   = claim_change(snapshot, "claim-1", "alpha", "builder", true);
    change.provenance   = provenance_with_json_size(65536);
    REQUIRE(commit_execution(path, snapshot, change).has_value());
  }
  {
    scratch    dir;
    const auto path     = activated_ledger(dir);
    const auto snapshot = snapshot_of(path);
    auto       change   = claim_change(snapshot, "claim-1", "alpha", "builder", true);
    change.provenance   = provenance_with_json_size(65537);
    const auto before   = row_counts(path);
    auto       result   = commit_execution(path, snapshot, change);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code == execution_error_code::resource_limit);
    REQUIRE(result.error().limit == "provenance_bytes");
    REQUIRE(result.error().maximum == 65536);
    REQUIRE(row_counts(path) == before);
  }
  {
    // G5: compact JSON at most 65536 bytes with a canonical_v1 size above 65536 is accepted and stored.
    scratch    dir;
    const auto path     = activated_ledger(dir);
    const auto snapshot = snapshot_of(path);
    auto       change   = claim_change(snapshot, "claim-1", "alpha", "builder", true);
    change.provenance   = provenance_with_json_size(65000);
    REQUIRE(bytes_of(change.provenance).size() > 65536);
    REQUIRE(commit_execution(path, snapshot, change).has_value());
    auto db = open_raw(path);
    REQUIRE(int_of(db, "SELECT COUNT(*) FROM runs WHERE provenance_id IS NOT NULL") == 1);
  }
}

TEST_CASE("execution manifest bound at write") {
  scratch    dir;
  const auto path     = activated_ledger(dir);
  const auto snapshot = snapshot_of(path);
  const auto base     = claim_change(snapshot, "claim-1", "alpha", "builder");
  {
    auto change             = base;
    change.packet->manifest = manifest_of_size(base.packet->manifest, manifest_limit);
    REQUIRE(commit_execution(path, snapshot, change).has_value());
  }
  {
    scratch    over_dir;
    const auto over_path     = activated_ledger(over_dir);
    const auto over_snapshot = snapshot_of(over_path);
    auto       change        = claim_change(over_snapshot, "claim-1", "alpha", "builder");
    change.packet->manifest  = manifest_of_size(change.packet->manifest, manifest_limit + 1);
    const auto before        = row_counts(over_path);
    auto       result        = commit_execution(over_path, over_snapshot, change);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code == execution_error_code::resource_limit);
    REQUIRE(result.error().limit == "manifest_bytes");
    REQUIRE(result.error().maximum == manifest_limit);
    REQUIRE(row_counts(over_path) == before);
  }
}

TEST_CASE("execution commit advances current state") {
  scratch    dir;
  const auto path  = activated_ledger(dir);
  const auto claim = commit_claim(path, "claim-1", "alpha", "builder");
  auto       db    = open_raw(path);
  REQUIRE(int_of(db, "SELECT last_seq FROM current_state") == static_cast<std::int64_t>(claim.event_seq));
  REQUIRE(text_of(db, "SELECT active_job FROM current_state") == "alpha");
  REQUIRE(int_of(db, "SELECT active_activation_id FROM current_state") == int_of(db, "SELECT MAX(id) FROM job_activations"));
  REQUIRE(int_of(db, "SELECT active_revision_id FROM current_state") == 1);
}

TEST_CASE("execution refuses malformed changes without effect") {
  scratch    dir;
  const auto path     = activated_ledger(dir);
  const auto snapshot = snapshot_of(path);
  // The ordinal one transition names a role other than the target.
  {
    auto change                     = claim_change(snapshot, "claim-1", "alpha", "builder");
    change.transitions.front().role = "reviewer";
    expect_invalid(path, snapshot, change);
  }
  // A reserved operation.
  {
    auto change      = claim_change(snapshot, "claim-1", "alpha", "builder");
    change.operation = "refresh";
    expect_invalid(path, snapshot, change);
  }
  // A derived transition whose after drift is set for a non-running state.
  {
    auto change = claim_change(snapshot, "claim-1", "alpha", "builder");
    change.transitions.push_back(execution_transition_input{.job            = "alpha",
                                                            .role           = "reviewer",
                                                            .before_state   = std::string("blocked"),
                                                            .before_drifted = false,
                                                            .after_state    = "ready",
                                                            .after_drifted  = true,
                                                            .reason         = "prerequisite_available"});
    expect_invalid(path, snapshot, change);
  }
  // A load naming a record id that does not exist.
  {
    auto ids   = ids_of(snapshot);
    ids.source = 999999;
    expect_invalid(path, snapshot, load_change(snapshot, "load-2", head_text(snapshot), revision_input(ids)));
  }
}

TEST_CASE("execution claim between snapshot and load refuses state conflict") {
  scratch    dir;
  const auto path  = activated_ledger(dir);
  const auto stale = snapshot_of(path);
  commit_claim(path, "claim-1", "alpha", "builder");
  const auto ids    = seed_revision(path, "two");
  const auto before = row_counts(path);
  require_code(commit_execution(path, stale, load_change(stale, "load-2", head_text(stale), revision_input(ids))),
               execution_error_code::state_conflict);
  REQUIRE(row_counts(path) == before);
  commit_load(path, "load-2", ids);
  require_verifies(path);
}

TEST_CASE("execution load compare and swap reports revision conflict") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  commit_load(path, "load-2", seed_revision(path, "two"));
  commit_load(path, "load-3", seed_revision(path, "three"));
  commit_load(path, "load-4", seed_revision(path, "four"));
  const auto snapshot = snapshot_of(path);
  REQUIRE(snapshot.revision->id == 4);
  // (a) The intent expected head 3 against locked head 4: the snapshot value must not satisfy the compare-and-swap.
  auto expected3 = execution_change{};
  expected3      = load_change(snapshot, "load-x", "3", revision_input(seed_revision(path, "x")));
  {
    auto result = commit_execution(path, snapshot, expected3);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code == execution_error_code::revision_conflict);
    REQUIRE(result.error().expected == "3");
    REQUIRE(result.error().actual == "4");
  }
  // (b) The intent 4 matches the snapshot, but the head moved to 5 before the commit.
  const auto stale = snapshot_of(path);
  commit_load(path, "load-5", seed_revision(path, "five"));
  {
    auto result = commit_execution(path, stale, load_change(stale, "load-y", "4", revision_input(seed_revision(path, "y"))));
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code == execution_error_code::revision_conflict);
    REQUIRE(result.error().expected == "4");
    REQUIRE(result.error().actual == "5");
  }
  // (c) An intent of none on a non-empty ledger.
  {
    const auto current = snapshot_of(path);
    auto       result =
        commit_execution(path, current, load_change(current, "load-z", "none", revision_input(seed_revision(path, "z"))));
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code == execution_error_code::revision_conflict);
    REQUIRE(result.error().expected == "none");
    REQUIRE(result.error().actual == "5");
  }
  // (d) The intent equals the locked head, but the snapshot head is stale: the separate active_revision member fails.
  {
    const auto old = snapshot_of(path);
    commit_load(path, "load-6", seed_revision(path, "six"));
    auto result = commit_execution(path, old, load_change(old, "load-w", "6", revision_input(seed_revision(path, "w"))));
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code == execution_error_code::state_conflict);
  }
  // An intent of none on an empty ledger passes the compare-and-swap.
  scratch    empty;
  const auto fresh = empty.ledger();
  REQUIRE(initialize_format3(fresh, workspace_name).has_value());
  REQUIRE(commit_execution(fresh, snapshot_of(fresh),
                           load_change(snapshot_of(fresh), "load-1", "none", revision_input(seed_revision(fresh, "one"))))
              .has_value());
}

TEST_CASE("execution load replay is checked before the compare and swap") {
  scratch    dir;
  const auto path   = activated_ledger(dir);
  const auto stale  = snapshot_of(path);
  const auto ids    = seed_revision(path, "two");
  const auto change = load_change(stale, "load-2", "1", revision_input(ids));
  auto       first  = commit_execution(path, stale, change);
  REQUIRE(first.has_value());
  commit_load(path, "load-3", seed_revision(path, "three"));
  // The head moved to 3, but the same request replays its saved bytes before any compare-and-swap.
  auto replay = commit_execution(path, stale, change);
  REQUIRE(replay.has_value());
  REQUIRE(replay->replayed);
  REQUIRE(replay->saved_result == first->saved_result);
  // A different body under the same id is request_conflict, not revision_conflict.
  auto different = commit_execution(path, stale, load_change(stale, "load-2", "none", revision_input(ids)));
  REQUIRE_FALSE(different.has_value());
  REQUIRE(different.error().code == execution_error_code::request_conflict);
}

namespace {

/// Replace the request framing of a change with a request_v3 of the given operation and parameter bytes.
execution_change with_request(execution_change change, std::string_view operation, std::string_view actor,
                              std::string parameters) {
  change.request = hist::request_v3{.operation        = std::string(operation),
                                    .actor            = std::string(actor),
                                    .parameters       = std::move(parameters),
                                    .profile_argument = std::nullopt};
  return change;
}

/// The member of a change's expected set for one role_state (the target of a claim or takeover).
execution_expectation& role_state_member(execution_change& change, std::string_view role) {
  for (auto& candidate : change.expected)
    if (candidate.kind == kind_t::role_state && candidate.role == role)
      return candidate;
  throw std::logic_error("role_state member missing from the fixture");
}

} // namespace

TEST_CASE("execution interning reuses exact bytes and refuses a hash collision") {
  // Identical W and packet bytes reuse one row: the takeover of the same role repeats both.
  {
    scratch    dir;
    const auto path = activated_ledger(dir);
    commit_claim(path, "claim-1", "alpha", "builder");
    commit_takeover_current(path, "takeover-1", "alpha", "builder");
    auto db = open_raw(path);
    REQUIRE(int_of(db, "SELECT COUNT(DISTINCT work_id) FROM dispatches") == 1);
    REQUIRE(int_of(db, "SELECT COUNT(DISTINCT packet_id) FROM dispatches") == 1);
  }
  // A file row with the packet digest and different bytes refuses hash_collision with the file kind and digest.
  {
    scratch    dir;
    const auto path     = activated_ledger(dir);
    const auto snapshot = snapshot_of(path);
    const auto change   = claim_change(snapshot, "claim-1", "alpha", "builder");
    const auto digest   = hist::digest_record(hist::record{hist::file_record{.mode = 420, .payload = change.packet->markdown}});
    REQUIRE(digest.has_value());
    auto db = open_raw(path);
    insert_frame(db, 1, 1, digest_text(*digest), "different bytes");
    const auto before = row_counts(path);
    auto       result = commit_execution(path, snapshot, change);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code == execution_error_code::hash_collision);
    REQUIRE(result.error().record_kind == "file");
    REQUIRE(result.error().record_digest == digest_hex(*digest));
    REQUIRE(row_counts(path) == before);
  }
  // A value row with the manifest digest and different bytes refuses with the value kind.
  {
    scratch    dir;
    const auto path     = activated_ledger(dir);
    const auto snapshot = snapshot_of(path);
    const auto change   = claim_change(snapshot, "claim-1", "alpha", "builder");
    auto       frame = hist::encode_execution_value_record(hist::codec_dispatch_manifest_v1, bytes_of(change.packet->manifest));
    REQUIRE(frame.has_value());
    auto digest = hist::digest_execution_value_record(*frame);
    REQUIRE(digest.has_value());
    auto db = open_raw(path);
    insert_frame(db, 3, 1, digest_text(*digest), "different bytes");
    auto result = commit_execution(path, snapshot, change);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code == execution_error_code::hash_collision);
    REQUIRE(result.error().record_kind == "value");
    REQUIRE(result.error().record_digest == digest_hex(*digest));
  }
  // A request row with the framing digest and different bytes refuses with the request kind and an EMPTY digest.
  {
    scratch    dir;
    const auto path     = activated_ledger(dir);
    const auto snapshot = snapshot_of(path);
    const auto change   = claim_change(snapshot, "claim-1", "alpha", "builder");
    auto       digest   = hist::digest_request_v3(std::get<hist::request_v3>(change.request));
    REQUIRE(digest.has_value());
    auto db = open_raw(path);
    insert_frame(db, 4, 3, digest_text(*digest), "different bytes");
    auto result = commit_execution(path, snapshot, change);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code == execution_error_code::hash_collision);
    REQUIRE(result.error().record_kind == "request");
    REQUIRE(result.error().record_digest.empty());
  }
}

TEST_CASE("execution refuses a transition whose before state differs from the snapshot") {
  scratch    dir;
  const auto path     = activated_ledger(dir);
  const auto snapshot = snapshot_of(path);
  {
    auto change                             = claim_change(snapshot, "claim-1", "alpha", "builder");
    change.transitions.front().before_state = "blocked";
    expect_invalid(path, snapshot, change);
  }
  {
    // The builder has a row, so a NULL before state is a disagreement as well.
    auto change                               = claim_change(snapshot, "claim-1", "alpha", "builder");
    change.transitions.front().before_state   = std::nullopt;
    change.transitions.front().before_drifted = std::nullopt;
    expect_invalid(path, snapshot, change);
  }
}

TEST_CASE("execution refuses a vector that disagrees with the snapshot") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  commit_activate(path, "activate-2", "beta", {role_plan{.role = "worker", .kind = "worker", .after = "ready"}});
  const auto snapshot = snapshot_of(path);
  const auto base     = claim_change(snapshot, "claim-worker", "beta", "worker");
  {
    auto change = base;
    std::erase_if(change.vector, [](const execution_vector_entry& entry) { return entry.role == "worker"; });
    expect_invalid(path, snapshot, change);
  }
  {
    auto change = base;
    change.vector.push_back(change.vector.front());
    expect_invalid(path, snapshot, change);
  }
  {
    auto change                   = base;
    change.vector.front().drifted = true;
    expect_invalid(path, snapshot, change);
  }
  {
    // Retired rows of the earlier job are not vector entries: carrying them is invalid.
    auto change = base;
    change.vector.push_back(
        execution_vector_entry{.job = "alpha", .role = "builder", .state = "retired", .drifted = false, .run = std::nullopt});
    expect_invalid(path, snapshot, change);
  }
  // The contract-shaped vector of the active job is accepted, and the earlier job's retired rows do not block it.
  REQUIRE(commit_execution(path, snapshot, base).has_value());
}

TEST_CASE("execution refuses a fenced run set that disagrees with the snapshot") {
  scratch    dir;
  const auto path = two_claims_ledger(dir);
  commit_takeover_current(path, "takeover-1", "alpha", "builder");
  const auto snapshot = snapshot_of(path);
  const auto base     = takeover_change(snapshot, "takeover-2", "alpha", "builder", 3);
  {
    // Run 1 is fenced already: naming it is not a fence of an active run.
    auto change        = base;
    change.fenced_runs = {1};
    expect_invalid(path, snapshot, change);
  }
  {
    auto change        = base;
    change.fenced_runs = {999};
    expect_invalid(path, snapshot, change);
  }
  {
    // Missing: the active run 3 of the target is not fenced.
    auto change        = base;
    change.fenced_runs = {};
    expect_invalid(path, snapshot, change);
  }
  {
    // Extra: the checker's active run 2 is fenced although its role does not transition.
    auto change        = base;
    change.fenced_runs = {3, 2};
    expect_invalid(path, snapshot, change);
  }
}

TEST_CASE("execution takeover change must name the snapshot active run") {
  scratch    dir;
  const auto path     = claimed_ledger(dir);
  const auto snapshot = snapshot_of(path);
  expect_invalid(path, snapshot, takeover_change(snapshot, "takeover-x", "alpha", "builder", 999));
  {
    auto change                            = takeover_change(snapshot, "takeover-x", "alpha", "builder", 1);
    change.transitions.front().after_state = "ready";
    expect_invalid(path, snapshot, change);
  }
  {
    auto change                              = takeover_change(snapshot, "takeover-x", "alpha", "builder", 1);
    role_state_member(change, "builder").run = 999;
    expect_invalid(path, snapshot, change);
  }
}

TEST_CASE("execution refuses a request that disagrees with the change") {
  scratch    dir;
  const auto path     = activated_ledger(dir);
  const auto snapshot = snapshot_of(path);
  const auto base     = claim_change(snapshot, "claim-1", "alpha", "builder");
  // Parameter map sub-cases (D41): these carry the mutant of row K52.
  expect_invalid(path, snapshot,
                 with_request(base, "claim", "tester", params_bytes({{"job", "alpha"}, {"role", "builder"}, {"bogus", "x"}})));
  expect_invalid(path, snapshot, with_request(base, "claim", "tester", params_bytes({{"job", "beta"}, {"role", "builder"}})));
  expect_invalid(path, snapshot,
                 with_request(base, "claim", "tester",
                              params_bytes({{"job", "alpha"}, {"role", "builder"}, {"provenance_path", "/private/p.json"}})));
  {
    auto takeover = takeover_change(snapshot, "takeover-x", "alpha", "builder", 1);
    expect_invalid(
        path, snapshot,
        with_request(takeover, "takeover", "tester", params_bytes({{"job", "alpha"}, {"role", "builder"}, {"run_id", "5"}})));
  }
  {
    auto activate =
        activate_change(snapshot, "activate-x", "beta", {role_plan{.role = "worker", .kind = "worker", .after = "ready"}});
    expect_invalid(path, snapshot,
                   with_request(activate, "activate", "tester",
                                params_bytes({{"expect_job", "BAD JOB"}, {"expect_revision", "1"}, {"job", "beta"}})));
  }
  // Masked sub-cases (V26 and the actor equality): the verifier and the request checks also refuse them.
  {
    hist::request_v2 framed;
    framed.operation          = "load";
    framed.actor              = "tester";
    framed.expected_head      = "1";
    framed.workspace_argument = std::string(workspace_name);
    framed.registry_argument  = "registry";
    auto change               = base;
    change.request            = framed;
    expect_invalid(path, snapshot, change);
  }
  expect_invalid(path, snapshot, with_request(base, "takeover", "tester", params_bytes({{"job", "alpha"}, {"role", "builder"}})));
  expect_invalid(path, snapshot, with_request(base, "claim", "intruder", params_bytes({{"job", "alpha"}, {"role", "builder"}})));
}

TEST_CASE("execution saved result is the contract JSON") {
  scratch    dir;
  const auto path      = activated_ledger(dir);
  const auto snapshot  = snapshot_of(path);
  const auto change    = claim_change(snapshot, "claim-1", "alpha", "builder");
  auto       committed = commit_execution(path, snapshot, change);
  REQUIRE(committed.has_value());
  auto db = open_raw(path);
  // The claim_v1 saved result, built from the change and the committed ids.
  const auto            markdown = change.packet->markdown;
  hist::claim_result_v1 oracle;
  oracle.ledger        = text_of(db, "SELECT identity FROM ledger_meta");
  oracle.request_id    = "claim-1";
  oracle.job           = "alpha";
  oracle.role          = "builder";
  oracle.revision      = "1";
  oracle.run_id        = "1";
  oracle.generation    = "1";
  oracle.token         = *committed->token;
  oracle.dispatch_id   = "1";
  oracle.work_fp       = *hash_value("work_exec_v1", change.packet->work);
  oracle.render_digest = *fragment_digest(markdown);
  oracle.bytes         = markdown.size();
  auto claim_expected  = hist::encode_claim_result_v1(oracle);
  REQUIRE(claim_expected.has_value());
  REQUIRE(committed->saved_result == *claim_expected);
  REQUIRE(hist::decode_claim_result_v1(committed->saved_result).has_value());
  REQUIRE(committed->saved_result.back() != '\n');
  // The activate_v1 saved result of activate-1.
  hist::activate_result_v1 activation;
  activation.ledger       = text_of(db, "SELECT identity FROM ledger_meta");
  activation.request_id   = "activate-1";
  activation.revision     = "1";
  activation.job          = "alpha";
  activation.previous_job = "";
  activation.activation   = "1";
  activation.event        = std::to_string(seq_of_type(path, "job_activated"));
  activation.roles        = {hist::activate_role_v1{.role = "builder", .generation = "1", .state = "ready"},
                             hist::activate_role_v1{.role = "reviewer", .generation = "1", .state = "blocked"}};
  auto activate_expected  = hist::encode_activate_result_v1(activation);
  REQUIRE(activate_expected.has_value());
  REQUIRE(text_of(db, "SELECT CAST(result AS TEXT) FROM requests WHERE request_id = 'activate-1'") == *activate_expected);
}

TEST_CASE("execution unknown run token has its own native category") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  // A token that parses but names no run, and a text that does not parse, both have the native category unknown_run.
  require_code(authorize_run_token(path, "99.1.00000000000000000000000000000000"), execution_error_code::unknown_run);
  require_code(authorize_run_token(path, "not-a-token"), execution_error_code::unknown_run);
}

TEST_CASE("execution refuses a change that would break a format3 invariant") {
  // (a) An activation whose generation reason is structural_change.
  {
    scratch    dir;
    const auto path                   = activated_ledger(dir);
    const auto snapshot               = snapshot_of(path);
    auto       change                 = activate_change(snapshot, "activate-a", "alpha", standard_roles());
    change.generations.front().reason = "structural_change";
    expect_invalid(path, snapshot, change);
  }
  // (b) A takeover generation whose structure changed.
  {
    scratch    dir;
    const auto path                      = claimed_ledger(dir);
    const auto snapshot                  = snapshot_of(path);
    auto       change                    = takeover_change(snapshot, "takeover-b", "alpha", "builder", 1);
    change.generations.front().structure = structure_of("alpha", "builder", "worker", "unit_two");
    expect_invalid(path, snapshot, change);
  }
  // (c) A recreated generation whose predecessor row is not retired: the before state is the snapshot ready state.
  {
    scratch    dir;
    const auto path                         = activated_ledger(dir);
    const auto snapshot                     = snapshot_of(path);
    auto       change                       = role_load(snapshot, "load-3", seed_revision(path, "three"), "reviewer", "readd");
    change.transitions.front().before_state = "blocked";
    expect_invalid(path, snapshot, change);
  }
  // (d) A structural_change generation whose structure digest equals its predecessor's.
  {
    scratch    dir;
    const auto path                      = activated_ledger(dir);
    const auto snapshot                  = snapshot_of(path);
    auto       change                    = role_load(snapshot, "load-3", seed_revision(path, "three"), "reviewer", "structural");
    change.generations.front().structure = structure_of("alpha", "reviewer", "reviewer");
    expect_invalid(path, snapshot, change);
  }
  // (e) A NULL-first transition with no generation for its role.
  {
    scratch    dir;
    const auto path     = activated_ledger(dir);
    const auto snapshot = snapshot_of(path);
    auto       roles    = standard_roles();
    roles.push_back(role_plan{.role = "extra", .kind = "worker", .after = "ready"});
    auto change = activate_change(snapshot, "activate-e", "alpha", roles);
    std::erase_if(change.generations, [](const execution_generation_input& generation) { return generation.role == "extra"; });
    expect_invalid(path, snapshot, change);
  }
  // (f) An activation that leaves a role of the previous job non-retired.
  {
    scratch    dir;
    const auto path     = activated_ledger(dir);
    const auto snapshot = snapshot_of(path);
    auto       change =
        activate_change(snapshot, "activate-b", "beta", {role_plan{.role = "worker", .kind = "worker", .after = "ready"}});
    std::erase_if(change.transitions, [](const execution_transition_input& transition) { return transition.job == "alpha"; });
    expect_invalid(path, snapshot, change);
  }
}

TEST_CASE("execution load result is built natively from allocated ids") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  commit_claim(path, "claim-1", "alpha", "builder");
  // Revision 2 is accepted at event sequence 5, so the revision id and the event sequence differ.
  const auto loaded = commit_load(path, "load-2", seed_revision(path, "two"));
  REQUIRE(loaded.event_seq == 5);
  REQUIRE(loaded.saved_result == expected_load_result(path, "load-2", [](load_result&) {}));
  // The noop answers the active revision 2 with its parent 1 and its own event sequence 6.
  const auto noop = commit_with(
      path, [&](const execution_snapshot& s) { return load_change(s, "load-3", head_text(s), revision_input(ids_of(s), true)); });
  REQUIRE(noop.event_seq == 6);
  REQUIRE(noop.saved_result == expected_load_result(path, "load-3", [](load_result&) {}));
  auto db = open_raw(path);
  REQUIRE(int_of(db, "SELECT revision_id FROM requests WHERE request_id = 'load-3'") == 2);
  REQUIRE(blob_of(db, "SELECT result FROM requests WHERE request_id = 'load-3'") == noop.saved_result);
}

TEST_CASE("execution first failing precondition member decides the conflict code") {
  // (a) A claim on a ready role while a concurrent load drops the job: active_revision also fails, so state_conflict.
  {
    scratch    dir;
    const auto path  = activated_ledger(dir);
    const auto stale = snapshot_of(path);
    commit_with(path,
                [&](const execution_snapshot& s) { return drop_active_job_load(s, "load-3", seed_revision(path, "three")); });
    require_code(commit_execution(path, stale, claim_change(stale, "claim-1", "alpha", "builder")),
                 execution_error_code::state_conflict);
  }
  // (b) A stale takeover: the fact version of the target fails before its role state, so state_conflict.
  {
    scratch    dir;
    const auto path  = claimed_ledger(dir);
    const auto stale = snapshot_of(path);
    commit_takeover_current(path, "takeover-1", "alpha", "builder");
    require_code(commit_execution(path, stale, takeover_change(stale, "takeover-2", "alpha", "builder", 1)),
                 execution_error_code::state_conflict);
  }
  // (c) Only role_state(target) fails: the conflict is claim_conflict with the locked state.
  {
    scratch    dir;
    const auto path  = activated_ledger(dir);
    const auto stale = snapshot_of(path);
    commit_claim(path, "claim-1", "alpha", "builder");
    require_code(commit_execution(path, stale, claim_change(stale, "claim-2", "alpha", "builder")),
                 execution_error_code::claim_conflict);
  }
}

TEST_CASE("execution no row role state member refuses after a row appears") {
  // Part one: a claim carries a no_row member for a role of the active job that has no row; no row appears, so it commits.
  {
    scratch    dir;
    const auto path     = activated_ledger(dir);
    const auto snapshot = snapshot_of(path);
    auto       change   = claim_change(snapshot, "claim-1", "alpha", "builder");
    auto       absent   = member(kind_t::role_state);
    absent.job          = "alpha";
    absent.role         = "extra";
    absent.no_row       = true;
    change.expected.push_back(absent);
    REQUIRE(commit_execution(path, snapshot, change).has_value());
  }
  // Part two (L23): a NULL-first row for extra appears between the snapshot and the commit, through a load (a claim
  // cannot open a generation, D44). Limit: the load also moves the active revision, so the stale claim is refused by the
  // locked checks together and the no-row member is not isolated.
  {
    scratch    dir;
    const auto path     = activated_ledger_with(dir, three_workers());
    const auto snapshot = snapshot_of(path);
    REQUIRE(commit_execution(path, snapshot, add_role_load(snapshot, "load-extra", seed_revision(path, "extra"), "extra"))
                .has_value());
    auto second   = claim_change(snapshot, "claim-checker", "alpha", "checker");
    auto absent   = member(kind_t::role_state);
    absent.job    = "alpha";
    absent.role   = "extra";
    absent.no_row = true;
    second.expected.push_back(absent);
    require_code(commit_execution(path, snapshot, second), execution_error_code::state_conflict);
    // The same request id on a fresh snapshot succeeds, because the member is no longer carried.
    commit_claim(path, "claim-checker", "alpha", "checker");
  }
}

// ---------------------------------------------------------------------------------------------------------------------
// Repair round tests for the invariants that the mutation probes found unprotected. Each corruption changes only the
// columns of the one invariant it names and keeps every redundant column consistent, so a single check refuses it.

TEST_CASE("format1 and format2 refuse a malformed run token with the format error first") {
  // The format check precedes any token work: a text that does not parse still reports the ledger version.
  for (int version : {1, 2}) {
    scratch    dir;
    const auto path = dir.ledger();
    if (version == 1)
      REQUIRE(initialize_new(path, workspace_name).has_value());
    else
      REQUIRE(initialize_new_v2(path, workspace_name).has_value());
    require_code(authorize_run_token(path, "not-a-token"), execution_error_code::unsupported_ledger_version);
  }
}

TEST_CASE("format3 verification rejects an event whose request id differs from its request row") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  require_verifies(path);
  // The activation event names another request id; the requests row still names the event. No other column carries the
  // request id of an activation event, so only the request and event bijection can refuse this.
  corrupt(path, {"UPDATE events SET request_id = 'activate-x' WHERE request_id = 'activate-1'"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a request row that names the initialization event") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  require_verifies(path);
  // Event 1 keeps a NULL request id; only the requests row names it.
  corrupt(path, {"INSERT INTO requests(request_id,operation,body_id,event_seq,revision_id,outcome,result,attachment_id) "
                 "VALUES('init-request','load',(SELECT body_id FROM requests WHERE request_id = 'load-1'),1,1,'accepted',X'',"
                 "NULL)"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects generation ordinals that do not start at one") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  require_verifies(path);
  // No run exists, so no saved token carries the ordinal; only the contiguity rule can refuse the lone ordinal 2.
  corrupt(path, {"UPDATE generations SET ordinal = 2 WHERE job = 'alpha' AND role = 'builder'"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a typed column on an event that must not carry it") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  require_verifies(path);
  // A load event carries no job; no other check reads the job column of a declaration_accepted event.
  corrupt(path, {"UPDATE events SET job = 'alpha' WHERE type = 'declaration_accepted'"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a row in every reserved table") {
  const std::vector<std::string> inserts = {
      "INSERT INTO checkpoints(id,run_id,seq,body_id) VALUES(1,1,1,1)",
      "INSERT INTO reservations(id,request_id,run_id,opened_seq,body_id) VALUES(1,'reserved',1,1,1)",
      "INSERT INTO completions(id,run_id,seq,dispatch_id,work_fp,observation_id) "
      "VALUES(1,1,1,1,X'00000000000000000000000000000000',1)",
      "INSERT INTO publications(id,completion_id,work,facet,semantic,payload_id) "
      "VALUES(1,1,'unit','facet',X'00000000000000000000000000000000',1)",
      "INSERT INTO verdicts(id,request_id,run_id,seq,body_id,applicability,decision) "
      "VALUES(1,'verdict',1,1,1,'current','approve')",
      "INSERT INTO repairs(verdict_id,job,role,generation_id,revision) VALUES(1,'alpha','builder',1,1)",
      "INSERT INTO blockers(id,seq,job,kind,verdict_id) VALUES(1,1,'alpha','max_rounds',1)",
      "INSERT INTO reservation_state(reservation_id,state,last_seq) VALUES(1,'open',1)",
      "INSERT INTO blocker_state(blocker_id,state,cleared_seq,last_seq) VALUES(1,'open',NULL,1)"};
  for (const auto& insert : inserts) {
    INFO(insert);
    scratch    dir;
    const auto path = activated_ledger(dir);
    require_verifies(path);
    corrupt(path, {insert});
    expect_corrupt(path);
  }
}

TEST_CASE("format3 verification rejects an event sequence that differs from its position") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  require_verifies(path);
  // Event 4 is renumbered 9 together with every column that compares against the event sequence. The rows that locate an
  // event by position (transitions, requests) and the current state keep 4, so only the contiguity rule of the event
  // loader sees the gap.
  corrupt(path, {"UPDATE events SET seq = 9 WHERE seq = 4", "UPDATE runs SET claimed_seq = 9 WHERE id = 1",
                 "UPDATE dispatches SET seq = 9 WHERE id = 1", "UPDATE run_state SET last_seq = 9 WHERE run_id = 1",
                 "UPDATE role_state SET last_seq = 9 WHERE job = 'alpha' AND role = 'builder'"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a generation opened by another activation") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  commit_activate(path, "activate-2", "beta", {role_plan{.role = "worker", .kind = "worker", .after = "ready"}});
  require_verifies(path);
  // The beta worker generation names the alpha activation (sequence 3), which is a job_activated event with the right
  // type for reason activated but carries no transition of beta worker.
  corrupt(path, {"UPDATE generations SET opened_seq = 3 WHERE job = 'beta' AND role = 'worker'"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a revision parent that disagrees with the previous head") {
  scratch    dir;
  const auto path = two_revision_ledger(dir);
  require_verifies(path);
  // The saved load result is rewritten to match, so the replay head is the only remaining witness of the parent.
  corrupt(path, {"UPDATE revisions SET parent_id = NULL WHERE id = 2",
                 std::format("UPDATE requests SET result = {} WHERE request_id = 'load-2'",
                             blob_literal(expected_load_result(path, "load-2",
                                                               [](load_result& result) { result.parent = std::nullopt; })))});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects load event references that differ from the revision") {
  for (std::string key : {"source", "registry", "stdlib", "interpretation", "versions"}) {
    INFO(key);
    scratch    dir;
    const auto path = two_revision_ledger(dir);
    require_verifies(path);
    const auto other = key == "source" ? std::string("registry") : std::string("source");
    const auto sql   = payload_update_sql(path, seq_of_type(path, "declaration_accepted"), [&](hist::event_payload_v2& payload) {
      payload.references[key] = payload.references[other];
    });
    corrupt(path, {sql});
    expect_corrupt(path);
  }
}

TEST_CASE("execution token secret appears in no stored column except the saved result") {
  scratch                                          dir;
  const auto                                       path   = activated_ledger(dir);
  const auto                                       token  = *commit_claim(path, "claim-1", "alpha", "builder").token;
  const auto                                       secret = token.substr(token.size() - 32);
  auto                                             db     = open_raw(path);
  std::vector<std::pair<std::string, std::string>> columns;
  {
    auto stmt = db.prepare("SELECT m.name, p.name FROM sqlite_master m, pragma_table_info(m.name) p WHERE m.type = 'table'");
    REQUIRE(stmt.has_value());
    while (true) {
      auto step = stmt->step();
      REQUIRE(step.has_value());
      if (*step == step_result::done)
        break;
      auto table  = stmt->text(0);
      auto column = stmt->text(1);
      REQUIRE(table.has_value());
      REQUIRE(column.has_value());
      columns.emplace_back(**table, **column);
    }
  }
  REQUIRE(columns.size() > 40);
  std::int64_t holders = 0;
  for (const auto& [table, column] : columns) {
    if (table == "runs" && column == "secret")
      continue;
    const auto found =
        int_of(db, std::format("SELECT COUNT(*) FROM \"{}\" WHERE instr(CAST(\"{}\" AS BLOB), CAST('{}' AS BLOB)) > 0", table,
                               column, secret));
    if (table == "requests" && column == "result") {
      REQUIRE(found == 1);
      ++holders;
    } else {
      INFO(table << "." << column);
      REQUIRE(found == 0);
    }
  }
  REQUIRE(holders == 1);
}

TEST_CASE("execution token naming another generation ordinal is an unknown run") {
  scratch    dir;
  const auto path   = activated_ledger(dir);
  const auto token  = *commit_claim(path, "claim-1", "alpha", "builder").token;
  auto       parsed = parse_run_token(token);
  REQUIRE(parsed.has_value());
  REQUIRE(authorize_run_token(path, token).has_value());
  parsed->generation_ordinal += 1;
  require_code(authorize_run_token(path, format_run_token(*parsed)), execution_error_code::unknown_run);
}

TEST_CASE("execution refuses a no row member for a role that has a row") {
  scratch    dir;
  const auto path     = activated_ledger(dir);
  const auto snapshot = snapshot_of(path);
  REQUIRE(find_role(snapshot, "alpha", "reviewer") != nullptr);
  auto change   = claim_change(snapshot, "claim-1", "alpha", "builder");
  auto absent   = member(kind_t::role_state);
  absent.job    = "alpha";
  absent.role   = "reviewer";
  absent.no_row = true;
  change.expected.push_back(absent);
  // The snapshot has a row for the reviewer, so the member is invalid at stage time, not a later state conflict.
  expect_invalid(path, snapshot, change);
}

TEST_CASE("execution fenced detail names the locked state of a superseded takeover target") {
  scratch    dir;
  const auto path = activated_ledger_with(dir, three_workers());
  commit_claim(path, "claim-builder", "alpha", "builder");
  const auto stale = snapshot_of(path);
  // A claim of checker also moves the running builder to done, fencing run 1 without bumping any fact version, so only
  // the role state member of the stale takeover fails and the target conflict reports the locked run state.
  commit_with(path, [&](const execution_snapshot& s) {
    auto change = carried_claim(s, "claim-checker", "alpha", "checker", "builder", "done", "completed");
    derive_fences(s, change);
    return change;
  });
  auto result = commit_execution(path, stale, takeover_change(stale, "takeover-2", "alpha", "builder", 1));
  REQUIRE_FALSE(result.has_value());
  REQUIRE(result.error().code == execution_error_code::fenced);
  REQUIRE(result.error().state == "fenced");
}

TEST_CASE("execution refuses a fenced run set that names a run twice") {
  scratch    dir;
  const auto path     = claimed_ledger(dir);
  const auto snapshot = snapshot_of(path);
  auto       change   = takeover_change(snapshot, "takeover-x", "alpha", "builder", 1);
  change.fenced_runs  = {1, 1};
  expect_invalid(path, snapshot, change);
}

// Replaced-ledger scenario. A change is built from ledger A's snapshot, then a same-shaped ledger B with a different
// identity replaces A at the same path (both are made by activated_ledger, so head, active job, activation, role states
// and fact versions match). The stale commit must be refused with path_denied and must leave B byte-identical. Step 5
// keeps the request id committed on a second replacement B2, so only the locked identity check can refuse: a replay of
// B2's saved result is not acceptable. Step 6 restores A and commits the same change; it is a sanity check with no product
// mutant behind it.
TEST_CASE("execution refuses a snapshot taken from a replaced ledger") {
  scratch    dir;
  const auto path   = activated_ledger(dir);
  const auto aside  = path.string() + ".aside";
  const auto snapA  = snapshot_of(path);
  const auto change = claim_change(snapA, "claim-1", "alpha", "builder");

  // Ledger B: same shape as A but a different identity.
  scratch    dirB;
  const auto replacement = activated_ledger(dirB);
  const auto snapB       = snapshot_of(replacement);
  REQUIRE(snapB.identity != snapA.identity);
  REQUIRE(snapB.last_seq == snapA.last_seq);
  REQUIRE(snapB.active_job == snapA.active_job);
  REQUIRE(snapB.activation == snapA.activation);
  REQUIRE(head_text(snapB) == head_text(snapA));
  REQUIRE(snapB.revision.has_value() == snapA.revision.has_value());
  if (snapA.revision) {
    REQUIRE(snapB.revision->source_digest == snapA.revision->source_digest);
    REQUIRE(snapB.revision->stdlib_digest == snapA.revision->stdlib_digest);
  }
  REQUIRE(snapB.roles.size() == snapA.roles.size());
  for (const auto& role : snapA.roles) {
    const auto* other = find_role(snapB, role.job, role.role);
    REQUIRE(other != nullptr);
    REQUIRE(other->state == role.state);
    REQUIRE(other->fact_version == role.fact_version);
  }

  // Step 3: move A aside and put B at the path. Neither rename may fail, and no SQLite sidecar may be left behind.
  std::error_code error;
  std::filesystem::rename(path, aside, error);
  REQUIRE(!error);
  std::filesystem::rename(replacement, path, error);
  REQUIRE(!error);
  for (const auto* suffix : {"-wal", "-shm", "-journal"})
    REQUIRE_FALSE(std::filesystem::exists(path.string() + suffix));

  // Step 4: the stale change must be refused with path_denied; B's bytes and row counts must not move, and B still verifies.
  const auto bytes_before  = file_contents(dir.root);
  const auto counts_before = row_counts(path);
  require_code(commit_execution(path, snapA, change), execution_error_code::path_denied);
  REQUIRE(file_contents(dir.root) == bytes_before);
  REQUIRE(row_counts(path) == counts_before);
  require_verifies(path);

  // Step 5: B2 has the same request id committed by its own snapshot. The stale change must still be refused with
  // path_denied, not answered as a replay of B2's saved result, and B2 must stay byte-identical.
  std::filesystem::remove(path, error);
  REQUIRE(!error);
  scratch    dirB2;
  const auto replacement2 = activated_ledger(dirB2);
  commit_claim(replacement2, "claim-1", "alpha", "builder");
  std::filesystem::rename(replacement2, path, error);
  REQUIRE(!error);
  const auto bytes_b2  = file_contents(dir.root);
  const auto counts_b2 = row_counts(path);
  require_code(commit_execution(path, snapA, change), execution_error_code::path_denied);
  REQUIRE(file_contents(dir.root) == bytes_b2);
  REQUIRE(row_counts(path) == counts_b2);

  // Step 6: restore A and commit the same change. This is a sanity check with no product mutant behind it.
  std::filesystem::remove(path, error);
  REQUIRE(!error);
  std::filesystem::rename(aside, path, error);
  REQUIRE(!error);
  REQUIRE(commit_execution(path, snapA, change).has_value());
  require_verifies(path);
}

TEST_CASE("R17 ledger reservation survives restart and recovery closes both request ids") {
  scratch dir;
  auto    path  = activated_ledger(dir);
  auto    claim = commit_claim(path, "claim-check", "alpha", "builder");
  REQUIRE(claim.token);
  auto token = parse_run_token(*claim.token);
  REQUIRE(token);
  hist::request_v3 request{
      .operation  = "check",
      .actor      = "tester",
      .parameters = params_bytes(
          {{"base_root", "/base"}, {"root", "/root"}, {"run_id", std::to_string(token->run_id)}, {"scratch", "/scratch"}}),
      .profile_argument = std::nullopt};
  auto reserved = reserve_check(path, snapshot_of(path), "check-1", "tester", request, *claim.token);
  INFO((reserved.has_value() ? -1 : static_cast<int>(reserved.error().code)));
  REQUIRE(reserved);
  auto read = read_execution_snapshot(path);
  INFO((read.has_value() ? -1 : static_cast<int>(read.error().code)));
  REQUIRE(read);
  REQUIRE(read->reservations.size() == 1);
  auto pending = lookup_execution_request(path, "check-1", request);
  REQUIRE_FALSE(pending);
  REQUIRE(pending.error().code == execution_error_code::check_in_progress);
  auto again = reserve_check(path, *read, "check-2", "tester", request, *claim.token);
  REQUIRE_FALSE(again);
  REQUIRE(again.error().code == execution_error_code::check_in_progress);
  hist::request_v3 recovery{.operation        = "check-recover",
                            .actor            = "tester",
                            .parameters       = params_bytes({{"confirm_stopped", "true"},
                                                              {"reservation", std::to_string(reserved->id)},
                                                              {"run_id", std::to_string(token->run_id)}}),
                            .profile_argument = std::nullopt};
  auto             recovered = recover_check(path, "recover-check", "tester", recovery, token->run_id, reserved->id);
  INFO((recovered.has_value() ? -1 : static_cast<int>(recovered.error().code)));
  REQUIRE(recovered);
  REQUIRE(verify_execution_ledger(path));
  REQUIRE(read_execution_snapshot(path)->reservations.empty());
  auto replay = lookup_execution_request(path, "check-1", request);
  REQUIRE(replay);
  REQUIRE(replay->state == request_state::replay);
  REQUIRE(replay->result == recovered->saved_result);
  REQUIRE(authorize_run_token(path, *claim.token));
  {
    auto file = open_existing(path, ledger_access::read_only);
    REQUIRE(file);
    for (auto seq : {reserved->opened_seq, recovered->event_seq}) {
      hist::history_query query;
      query.selector = hist::history_selector::event;
      query.id       = seq;
      auto h         = hist::read_history(*file, query);
      REQUIRE(h);
      const auto& expansion = (*h)->fields.at("data")->fields.at("execution");
      CHECK(expansion->fields.at("reservation")->fields.at("id")->string == std::to_string(reserved->id));
    }
  }
  SECTION("C02 malformed interrupted response") {
    corrupt(path,
            {"UPDATE requests SET result=CAST('{\"kind\":\"check\"}' AS BLOB) WHERE operation IN ('check','check-recover')"});
    expect_corrupt(path);
  }
}

TEST_CASE("R17 ledger completion retains evidence and changes run to completed atomically") {
  scratch dir;
  auto    path  = activated_ledger(dir, true);
  auto    claim = commit_claim(path, "claim-check", "alpha", "builder");
  REQUIRE(claim.token);
  auto token = parse_run_token(*claim.token);
  REQUIRE(token);
  hist::request_v3 request{
      .operation  = "check",
      .actor      = "tester",
      .parameters = params_bytes(
          {{"base_root", "/base"}, {"root", "/root"}, {"run_id", std::to_string(token->run_id)}, {"scratch", "/scratch"}}),
      .profile_argument = std::nullopt};
  auto reserved = reserve_check(path, snapshot_of(path), "check-1", "tester", request, *claim.token);
  REQUIRE(reserved);
  auto                   output = "xxh3_128:" + std::string(32, 'a');
  auto                   units  = vlist({vrec({{"work", vstr("unit_one")}, {"output", vstr(output)}})});
  execution_check_result result;
  result.recapture_matches = true;
  result.observation       = vrec({{"version", vint(1)},
                                   {"root", vstr("root")},
                                   {"base", vstr(output)},
                                   {"observation", vstr(output)},
                                   {"units", units},
                                   {"contracts", vlist({})}});
  execution_check_evidence evidence;
  evidence.work       = "unit_one";
  evidence.check      = "planar.boundary_v1";
  evidence.outcome    = "pass";
  evidence.reason     = "passed";
  evidence.vector     = vrec({{"version", vint(1)}, {"targets", units}});
  evidence.definition = vrec({{"version", vint(1)},
                              {"work", vstr("unit_one")},
                              {"check", vstr("planar.boundary_v1")},
                              {"gate", vstr("boundary_v1")},
                              {"cwd", vrec({{"root", vstr("root")}})},
                              {"inputs", vstrs({"own_output", "output_vector"})},
                              {"owned", vstrs({"src/"})},
                              {"frozen", vlist({})},
                              {"excluded", vlist({})},
                              {"producers", vlist({})}});
  result.evidence.push_back(evidence);
  auto snapshot = read_execution_snapshot(path);
  REQUIRE(snapshot);
  execution_transition_input t{.job            = "alpha",
                               .role           = "builder",
                               .before_state   = "running",
                               .before_drifted = false,
                               .after_state    = "done",
                               .after_drifted  = false,
                               .reason         = "completed"};
  SECTION("C03 nonempty facet payload retains directory edges") {
    auto content = planar::engine::file_digest("declaration");
    REQUIRE(content);
    auto facet = hash_value("facet_v1", vrec({{"kind", vstr("files")},
                                              {"normalizer", vstr("exact_bytes_v1")},
                                              {"root", vstr("root")},
                                              {"paths", vstrs({"include/api.h"})},
                                              {"entries", vlist({vrec({{"root", vstr("root")},
                                                                       {"path", vstr("include/api.h")},
                                                                       {"kind", vstr("file")},
                                                                       {"mode", vstr("0644")},
                                                                       {"size", vint(11)},
                                                                       {"content", vstr(*content)}})})}}));
    REQUIRE(facet);
    result.publications.push_back({"unit_one", "api", *facet, {{"include/api.h", "declaration", 420}}});
    auto published = complete_check(path, *snapshot, reserved->id, *claim.token, result, true, {t});
    REQUIRE(published);
    auto after = snapshot_of(path);
    REQUIRE(after.publications.size() == 1);
    {
      auto db = open_raw(path);
      CHECK(int_of(db, "SELECT COUNT(*) FROM tree_edges WHERE parent_id=(SELECT payload_id FROM publications)") == 1);
      CHECK(int_of(db, "SELECT COUNT(*) FROM tree_edges WHERE parent_id=(SELECT child_id FROM tree_edges WHERE parent_id=(SELECT "
                       "payload_id FROM publications))") == 1);
    }
    SECTION("C06 retirement removes former lineage facts but keeps historical publication") {
      const auto old_event = published->event_seq;
      commit_with(path, [&](const execution_snapshot& s) {
        auto change = load_change(s, "retire-builder", head_text(s), revision_input(seed_revision(path, "retired")));
        change.transitions.push_back(execution_transition_input{.job            = "alpha",
                                                                .role           = "builder",
                                                                .before_state   = "done",
                                                                .before_drifted = false,
                                                                .after_state    = "retired",
                                                                .after_drifted  = false,
                                                                .reason         = "retired"});
        settle_vector(change, s);
        return change;
      });
      auto current = snapshot_of(path);
      CHECK(current.completions.empty());
      CHECK(current.publications.empty());
      CHECK(current.observed.empty());
      CHECK(current.evidence.empty());
      {
        auto db = open_raw(path);
        CHECK(int_of(db, "SELECT COUNT(*) FROM completions") == 1);
        CHECK(int_of(db, "SELECT COUNT(*) FROM publications") == 1);
        CHECK(int_of(db, "SELECT COUNT(*) FROM evidence") == 1);
      }
      {
        auto file = open_existing(path, ledger_access::read_only);
        REQUIRE(file);
        hist::history_query query;
        query.selector = hist::history_selector::event;
        query.id       = old_event;
        auto history   = hist::read_history(*file, query);
        REQUIRE(history);
        CHECK((*history)->fields.at("data")->fields.at("execution")->fields.contains("completion"));
      }
      // Filtering a publication out of the current view must not skip historical integrity.
      corrupt(path, {"UPDATE publications SET payload_id=(SELECT source_id FROM revisions LIMIT 1)"});
      expect_corrupt(path);
    }
    SECTION("N8 unrelated complete directory") {
      corrupt(path, {"UPDATE publications SET payload_id=(SELECT source_id FROM revisions LIMIT 1)"});
      expect_corrupt(path);
    }
    SECTION("N8 semantic differs while saved response agrees") {
      const auto changed = "xxh3_128:" + std::string(32, '4');
      corrupt(
          path,
          {"UPDATE publications SET semantic=X'44444444444444444444444444444444'",
           std::format("UPDATE requests SET result=CAST(replace(CAST(result AS TEXT),'{}','{}') AS BLOB) WHERE operation='check'",
                       *facet, changed)});
      expect_corrupt(path);
    }
  }
  SECTION("C03 evidence-only rejects mismatched snapshot identity") {
    auto before        = snapshot_of(path);
    snapshot->identity = "different-ledger";
    auto denied        = complete_check(path, *snapshot, reserved->id, *claim.token, result, false, {});
    REQUIRE_FALSE(denied);
    CHECK(denied.error().code == execution_error_code::path_denied);
    auto after = snapshot_of(path);
    CHECK(after.last_seq == before.last_seq);
    CHECK(after.reservations.size() == before.reservations.size());
  }
  SECTION("C03 evidence-only keeps weaker revision and fact preconditions") {
    snapshot->revision->id += 100;
    for (auto& role : snapshot->roles)
      role.fact_version += 100;
    auto recorded = complete_check(path, *snapshot, reserved->id, *claim.token, result, false, {});
    REQUIRE(recorded);
    CHECK(snapshot_of(path).completions.empty());
  }
  SECTION("C03 boundary vector cannot name another work") {
    result.evidence[0].vector =
        vrec({{"version", vint(1)}, {"targets", vlist({vrec({{"work", vstr("other")}, {"output", vstr(output)}})})}});
    auto denied = complete_check(path, *snapshot, reserved->id, *claim.token, result, true, {t});
    REQUIRE_FALSE(denied);
    CHECK(snapshot_of(path).last_seq == snapshot->last_seq);
  }
  SECTION("C03 boundary vector output must match observation") {
    result.evidence[0].vector =
        vrec({{"version", vint(1)},
              {"targets", vlist({vrec({{"work", vstr("unit_one")}, {"output", vstr("xxh3_128:" + std::string(32, 'b'))}})})}});
    auto denied = complete_check(path, *snapshot, reserved->id, *claim.token, result, true, {t});
    REQUIRE_FALSE(denied);
    CHECK(snapshot_of(path).last_seq == snapshot->last_seq);
  }
  SECTION("C03 observation requires exact own unit membership") {
    auto changed             = std::make_shared<value>(*result.observation);
    changed->fields["units"] = vlist({});
    result.observation       = changed;
    auto denied              = complete_check(path, *snapshot, reserved->id, *claim.token, result, false, {});
    REQUIRE_FALSE(denied);
    CHECK(snapshot_of(path).last_seq == snapshot->last_seq);
  }
  SECTION("C03 observation refuses duplicate units") {
    auto changed             = std::make_shared<value>(*result.observation);
    changed->fields["units"] = vlist({units->items[0], units->items[0]});
    result.observation       = changed;
    auto denied              = complete_check(path, *snapshot, reserved->id, *claim.token, result, false, {});
    REQUIRE_FALSE(denied);
    CHECK(snapshot_of(path).last_seq == snapshot->last_seq);
  }
  SECTION("C11 observation refuses an extra unused unit") {
    const auto before        = snapshot_of(path);
    auto       changed       = std::make_shared<value>(*result.observation);
    changed->fields["units"] = vlist({units->items[0], vrec({{"work", vstr("unused")}, {"output", vstr(output)}})});
    result.observation       = changed;
    auto denied              = complete_check(path, *snapshot, reserved->id, *claim.token, result, false, {});
    REQUIRE_FALSE(denied);
    CHECK(denied.error().code == execution_error_code::invalid_change);
    const auto after = snapshot_of(path);
    CHECK(after.last_seq == before.last_seq);
    CHECK(after.reservations.size() == before.reservations.size());
    CHECK(after.completions.size() == before.completions.size());
  }
  SECTION("R17 completed result and retained corruption checks") {
    auto completed = complete_check(path, *snapshot, reserved->id, *claim.token, result, true, {t});
    INFO((completed.has_value() ? -1 : static_cast<int>(completed.error().code)));
    REQUIRE(completed);
    auto saved_response = planar::engine::codec_decode(completed->saved_result);
    REQUIRE(saved_response);
    auto expected_observation_digest = hash_value("check_observation_v1", result.observation);
    REQUIRE(expected_observation_digest);
    CHECK((*saved_response)->fields.at("observation")->fields.at("digest")->string == *expected_observation_digest);
    auto after = read_execution_snapshot(path);
    INFO((after.has_value() ? -1 : static_cast<int>(after.error().code)));
    REQUIRE(after);
    REQUIRE(after->completions.size() == 1);
    REQUIRE(after->evidence.size() == 1);
    REQUIRE(after->reservations.empty());
    auto authorization = authorize_run_token(path, *claim.token);
    REQUIRE_FALSE(authorization);
    REQUIRE(authorization.error().state == "completed");
    auto recovery = read_recovery_records(path, "alpha", "builder");
    REQUIRE(recovery);
    REQUIRE(recovery->evidence.size() == 1);
    CHECK(recovery->evidence[0]->fields.at("outcome")->string == "pass");
    {
      auto file = open_existing(path, ledger_access::read_only);
      REQUIRE(file);
      hist::history_query query;
      query.selector = hist::history_selector::event;
      query.id       = completed->event_seq;
      auto expanded  = hist::read_history(*file, query);
      REQUIRE(expanded);
      const auto& details = (*expanded)->fields.at("data")->fields.at("execution");
      CHECK(details->fields.at("evidence")->items.size() == 1);
      CHECK(details->fields.contains("completion"));
      CHECK(details->fields.contains("observation"));
      query.selector   = hist::history_selector::request;
      query.request_id = "check-1";
      REQUIRE(hist::read_history(*file, query));
    }
    SECTION("C02 malformed completed response") {
      corrupt(path, {"UPDATE requests SET result=CAST('{\"kind\":\"check\"}' AS BLOB) WHERE request_id='check-1'"});
      expect_corrupt(path);
    }
    SECTION("C02 invalid outcome reason") {
      corrupt(path, {"UPDATE evidence SET reason='not_a_reason'"});
      expect_corrupt(path);
    }
    SECTION("C03 invalid reason agrees with saved response") {
      corrupt(
          path,
          {"UPDATE evidence SET reason='not_a_reason'",
           R"sql(UPDATE requests SET result=CAST(replace(CAST(result AS TEXT),'"reason":"passed"','"reason":"not_a_reason"') AS BLOB) WHERE request_id='check-1')sql"});
      expect_corrupt(path);
    }
    SECTION("C02 missing required boundary evidence") {
      corrupt(path, {"DELETE FROM evidence"});
      expect_corrupt(path);
    }
  }
}

// ---------------------------------------------------------------------------------------------------------------------
// R15-C (draft) additions: the five L2 residual cross-row checks, advisory A1 and the G2 interning hook.

namespace {

/// Repoint one revision record column of revision 1 and its accepting event reference to another record (D43 style).
void repoint_revision(const std::filesystem::path& path, std::string_view column, std::string_view reference,
                      std::uint64_t record) {
  corrupt(path, {std::format("UPDATE revisions SET {} = {} WHERE id = 1", column, record),
                 payload_update_sql(path, seq_of_type(path, "declaration_accepted"), [&](hist::event_payload_v2& payload) {
                   payload.references[std::string(reference)] = std::to_string(record);
                 })});
}

std::string stdlib_hex_of(connection& db) {
  return to_hex(blob_of(db, "SELECT digest FROM records WHERE id = (SELECT stdlib_id FROM revisions WHERE id = 1)"));
}

/// Prepared records of a load candidate for the G2 hook: tagged source, the registry input and a fixed stdlib tree.
execution_prepared_revision prepared_candidate(std::string_view tag) {
  execution_prepared_revision out;
  out.source.profile   = hist::tree_profile::source;
  out.source.files     = {hist::tree_file{"jobs/main.lua", std::format("-- source {}\n", tag), 420},
                          hist::tree_file{"workspace.lua", "return {}\n", 420}};
  out.registry.profile = hist::tree_profile::provenance;
  out.registry.files   = {hist::tree_file{"input.json", std::format("{{\"tag\":\"{}\"}}", tag), 420}};
  out.stdlib.profile   = hist::tree_profile::provenance;
  out.stdlib.files     = {hist::tree_file{"planar/stdlib.lua", "stdlib body prepared\n", 420}};
  std::vector<hist::tree_file_view> views;
  for (const auto& file : out.stdlib.files)
    views.push_back(hist::tree_file_view{file.path, file.payload, file.mode});
  auto digest = hist::digest_tree(views, hist::tree_profile::provenance);
  REQUIRE(digest.has_value());
  const auto versions = vrec({{"stdlib_digest", vstr(digest_hex(*digest))}});
  out.interpretation  = bytes_of(seeded_interpretation(tag, versions));
  out.versions        = bytes_of(versions);
  return out;
}

} // namespace

TEST_CASE("format3 verification rejects directory edges that disagree with the encoded children") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  require_verifies(path);
  // The source root's single edge is renamed: the encoded child stays "source.txt", the edge says "renamed.txt". Tree
  // reconstruction reads the encoded children, so only the edge check can see this.
  corrupt(path, {"UPDATE tree_edges SET name = 'renamed.txt' WHERE name = 'source.txt'"});
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects a revision tree that does not reconstruct") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  require_verifies(path);
  // A source root whose only child is an empty subdirectory: every record and edge is consistent, but the tree is not
  // reachable as files (an empty non-root directory never results from a captured tree).
  auto       db    = open_raw(path);
  const auto empty = store_directory(db, {});
  const auto root  = store_directory(db, {child{"empty", hist::record_kind::directory, empty}});
  repoint_revision(path, "source_id", "source", root.id);
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects an interpretation whose embedded versions differ from the versions record") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  require_verifies(path);
  // The new versions record keeps the right stdlib digest (so the D23 shape check passes) but adds a field the
  // interpretation's embedded versions do not carry.
  auto       db       = open_raw(path);
  const auto versions = store_value(db, vrec({{"stdlib_digest", vstr(stdlib_hex_of(db))}, {"extra", vstr("x")}}));
  repoint_revision(path, "versions_id", "versions", versions.id);
  expect_corrupt(path);
}

TEST_CASE("format3 verification rejects an interpretation naming another workspace") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  require_verifies(path);
  auto       db       = open_raw(path);
  const auto versions = vrec({{"stdlib_digest", vstr(stdlib_hex_of(db))}});
  const auto other    = store_value(db, seeded_interpretation("one", versions, "other_workspace"));
  repoint_revision(path, "interpretation_id", "interpretation", other.id);
  expect_corrupt(path);
  // The same interpretation naming the ledger workspace verifies: the check compares with ledger_meta, not a constant.
  scratch    good_dir;
  const auto good = activated_ledger(good_dir);
  auto       gdb  = open_raw(good);
  const auto same = store_value(gdb, seeded_interpretation("one", vrec({{"stdlib_digest", vstr(stdlib_hex_of(gdb))}})));
  REQUIRE(same.id > 0);
  require_verifies(good);
}

TEST_CASE("format3 verification rejects a recorded family that disagrees with the load profile selector") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  require_verifies(path);
  // The load body gains a profile argument (selector 01) while the recorded interpretation is the embedded family.
  auto             db = open_raw(path);
  hist::request_v2 body;
  body.operation          = "load";
  body.actor              = "tester";
  body.expected_head      = "none";
  body.workspace_argument = std::string(workspace_name);
  body.registry_argument  = "registry";
  body.profile_argument   = "/private/profile";
  auto encoded            = hist::encode_request_v2(body);
  auto digest             = hist::digest_request_v2(body);
  REQUIRE(encoded.has_value());
  REQUIRE(digest.has_value());
  const auto body_id = insert_frame(db, 4, 2, digest_text(*digest), *encoded);
  corrupt(path, {std::format("UPDATE requests SET body_id = {} WHERE request_id = 'load-1'", body_id)});
  expect_corrupt(path);
}

TEST_CASE("execution state bytes measures each dispatch at its own revision") {
  scratch    dir;
  const auto path = activated_ledger(dir);
  // Revisions 2 to 9, a claim at revision 9, then revision 10: the dispatch revision ("9") and the active revision
  // ("10") differ in decimal width, the case advisory A1 names.
  for (int i = 2; i <= 9; ++i)
    commit_load(path, std::format("load-{}", i), seed_revision(path, std::format("rev{}", i)));
  commit_claim(path, "claim-1", "alpha", "builder");
  commit_load(path, "load-10", seed_revision(path, "rev10"));
  const auto snapshot = snapshot_of(path);
  REQUIRE(snapshot.revision.has_value());
  REQUIRE(snapshot.revision->id == 10);
  const auto* builder = find_role(snapshot, "alpha", "builder");
  REQUIRE(builder != nullptr);
  REQUIRE(builder->dispatch.has_value());
  auto db = open_raw(path);
  REQUIRE(builder->dispatch->revision ==
          static_cast<std::uint64_t>(
              int_of(db, std::format("SELECT revision_id FROM dispatches WHERE id = {}", builder->dispatch->id))));
  REQUIRE(builder->dispatch->revision == 9);
  REQUIRE(execution_state_bytes(snapshot) == snapshot.state_bytes);
}

TEST_CASE("execution load interns prepared records inside its own transaction") {
  scratch    dir;
  const auto path = dir.ledger();
  REQUIRE(initialize_format3(path, workspace_name).has_value());
  const auto records_before = [&] {
    auto db = open_raw(path);
    return count_rows(db, "records");
  }();
  auto snapshot     = snapshot_of(path);
  auto change       = load_change(snapshot, "load-1", "none", revision_input(revision_ids{}));
  change.prepared   = prepared_candidate("one");
  const auto result = commit_execution(path, snapshot, change);
  INFO("commit failed with error code " << (result.has_value() ? -1 : static_cast<int>(result.error().code)));
  REQUIRE(result.has_value());
  require_verifies(path);
  auto db = open_raw(path);
  REQUIRE(count_rows(db, "revisions") == 1);
  REQUIRE(count_rows(db, "records") > records_before);
  // The revision roots are directory records with edges written for every encoded child.
  for (const auto* column : {"source_id", "registry_id", "stdlib_id"}) {
    const auto root = int_of(db, std::format("SELECT {} FROM revisions WHERE id = 1", column));
    REQUIRE(int_of(db, std::format("SELECT kind FROM records WHERE id = {}", root)) == 2);
    REQUIRE(int_of(db, std::format("SELECT count(*) FROM tree_edges WHERE parent_id = {}", root)) > 0);
  }
  const auto loaded = snapshot_of(path);
  REQUIRE(loaded.revision.has_value());
  REQUIRE(bytes_of(loaded.revision->interpretation_value) == change.prepared->interpretation);

  // A second candidate sharing the stdlib tree reuses its records (exact-row reuse, D38) and duplicates no edge.
  const auto stdlib_one = int_of(db, "SELECT stdlib_id FROM revisions WHERE id = 1");
  const auto edges_one  = count_rows(db, "tree_edges");
  auto       second     = snapshot_of(path);
  auto       next       = load_change(second, "load-2", "1", revision_input(revision_ids{}));
  next.prepared         = prepared_candidate("two");
  REQUIRE(commit_execution(path, second, next).has_value());
  require_verifies(path);
  REQUIRE(int_of(db, "SELECT stdlib_id FROM revisions WHERE id = 2") == stdlib_one);
  REQUIRE(count_rows(db, "tree_edges") > edges_one);

  // Prepared records are refused on a noop load and on any other operation.
  auto noop     = load_change(snapshot_of(path), "load-3", "2", revision_input(ids_of(snapshot_of(path)), true));
  noop.prepared = prepared_candidate("two");
  require_code(commit_execution(path, snapshot_of(path), noop), execution_error_code::invalid_change);
}

TEST_CASE("execution load with prepared records writes nothing when the commit is refused") {
  scratch    dir;
  const auto path = dir.ledger();
  REQUIRE(initialize_format3(path, workspace_name).has_value());
  auto snapshot     = snapshot_of(path);
  auto change       = load_change(snapshot, "load-1", "7", revision_input(revision_ids{}));
  change.prepared   = prepared_candidate("one");
  const auto before = file_contents(dir.root);
  require_code(commit_execution(path, snapshot, change), execution_error_code::revision_conflict);
  REQUIRE(file_contents(dir.root) == before);
}

namespace {
execution_change recovery_change(const execution_snapshot& snapshot, std::string_view op, std::string_view id,
                                 std::string_view token) {
  const auto* role = find_role(snapshot, "alpha", "builder");
  REQUIRE(role != nullptr);
  REQUIRE(role->run_id);
  execution_change c;
  c.operation      = op;
  c.request_id     = id;
  c.actor          = "tester";
  c.job            = "alpha";
  c.role           = "builder";
  c.run            = role->run_id;
  c.run_token_text = token;
  std::vector<std::pair<std::string, std::string>> params{{"run_id", decimal(*c.run)}};
  if (op == "checkpoint") {
    params.emplace_back("observations_path", "notes.json");
    auto note = planar::engine::codec_decode(R"({"files":[],"next":"continue","progress":"implemented","unresolved":"none"})");
    REQUIRE(note);
    c.checkpoint = *note;
  }
  if (op == "refresh") {
    params.emplace_back("expect_fp", "xxh3_128:" + role->dispatch->work_fp);
    c.packet = make_packet(snapshot, *role, id, "refresh", std::nullopt);
  }
  if (op == "release") {
    c.release_reason = "interrupted";
    params.emplace_back("reason", c.release_reason);
    c.transitions.push_back({.job            = "alpha",
                             .role           = "builder",
                             .before_state   = "running",
                             .before_drifted = role->drifted,
                             .after_state    = "ready",
                             .after_drifted  = false,
                             .reason         = "interrupted"});
  }
  c.request = hist::request_v3{
      .operation = std::string(op), .actor = "tester", .parameters = params_bytes(params), .profile_argument = std::nullopt};
  c.vector = base_vector(snapshot, "alpha");
  if (op == "release")
    for (auto& r : c.vector)
      if (r.role == "builder") {
        r.state   = "ready";
        r.drifted = false;
        r.run.reset();
      }
  c.expected = targeted_expected(snapshot, *role);
  if (op == "checkpoint")
    std::erase_if(c.expected, [](const auto& e) { return e.kind == kind_t::fact_version || e.kind == kind_t::role_state; });
  auto auth = member(kind_t::run_active);
  auth.run  = c.run;
  c.expected.push_back(auth);
  return c;
}
} // namespace
TEST_CASE("C03 release rejects non-derived extra transitions and wrong target") {
  scratch    dir;
  const auto path  = activated_ledger(dir);
  const auto claim = commit_claim(path, "claim-release-shape", "alpha", "builder");
  REQUIRE(claim.token);
  auto snapshot = snapshot_of(path);
  auto change   = recovery_change(snapshot, "release", "release-shape", *claim.token);
  SECTION("target remains first") {
    change.transitions[0].role = "reviewer";
  }
  SECTION("target reason matches request") {
    change.transitions[0].reason = "released";
  }
  SECTION("extra transition must derive prerequisite readiness") {
    const auto* reviewer = find_role(snapshot, "alpha", "reviewer");
    REQUIRE(reviewer);
    change.transitions.push_back({.job            = "alpha",
                                  .role           = "reviewer",
                                  .before_state   = reviewer->state,
                                  .before_drifted = reviewer->drifted,
                                  .after_state    = "done",
                                  .after_drifted  = false,
                                  .reason         = "evidence_inapplicable"});
    for (auto& r : change.vector)
      if (r.role == "reviewer")
        r.state = "done";
  }
  require_code(commit_execution(path, snapshot, change), execution_error_code::invalid_change);
  CHECK(snapshot_of(path).last_seq == snapshot.last_seq);
}

TEST_CASE("R16 native checkpoint refresh release preserve retained recovery and replay") {
  scratch    dir;
  const auto path  = activated_ledger(dir);
  const auto claim = commit_claim(path, "claim-r16", "alpha", "builder");
  REQUIRE(claim.token);
  auto       initial  = snapshot_of(path);
  const auto original = find_role(initial, "alpha", "builder")->dispatch->id;
  CHECK(find_role(initial, "alpha", "builder")->run_request_id == "claim-r16");
  for (auto op : {"checkpoint", "refresh", "release"}) {
    auto snapshot = snapshot_of(path);
    auto change   = recovery_change(snapshot, op, std::string(op) + "-r16", *claim.token);
    auto result   = commit_execution(path, snapshot, change);
    INFO(op);
    INFO((result ? -1 : static_cast<int>(result.error().code)));
    REQUIRE(result);
    auto replay = commit_execution(path, snapshot, change);
    REQUIRE(replay);
    CHECK(replay->replayed);
    CHECK(replay->saved_result == result->saved_result);
    require_verifies(path);
  }
  auto recovery = read_recovery_records(path, "alpha", "builder");
  REQUIRE(recovery);
  CHECK_FALSE(recovery->run);
  REQUIRE(recovery->checkpoint);
  CHECK(recovery->checkpoint->value->fields.at("progress")->string == "implemented");
  REQUIRE(recovery->acknowledged);
  CHECK(recovery->acknowledged->kind == "refresh");
  CHECK(recovery->acknowledged->run_state == "interrupted");
  auto retained = read_retained_dispatch(path, original);
  REQUIRE(retained);
  CHECK(retained->kind == "claim");
  CHECK_FALSE(retained->markdown.empty());
  auto since = find_dispatch_by_render(path, "alpha", "builder", retained->render_fp);
  REQUIRE(since);
  REQUIRE(*since);
  CHECK((*since)->id == recovery->acknowledged->id);
  auto auth = authorize_run_token(path, *claim.token);
  REQUIRE_FALSE(auth);
  CHECK(auth.error().code == execution_error_code::fenced);
  CHECK(auth.error().state == "interrupted");
}
TEST_CASE("R16 native token mutation rejects wrong secret without a write") {
  scratch    dir;
  const auto path  = activated_ledger(dir);
  const auto claim = commit_claim(path, "claim-r16", "alpha", "builder");
  REQUIRE(claim.token);
  auto snapshot = snapshot_of(path);
  auto bad      = *claim.token;
  bad.back()    = bad.back() == '0' ? '1' : '0';
  auto change   = recovery_change(snapshot, "checkpoint", "bad-secret", bad);
  auto result   = commit_execution(path, snapshot, change);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == execution_error_code::fenced);
  CHECK(snapshot_of(path).last_seq == snapshot.last_seq);
}

TEST_CASE("R16 history expands retained execution events and request packets") {
  scratch    dir;
  const auto path  = activated_ledger(dir);
  const auto claim = commit_claim(path, "claim-history", "alpha", "builder");
  REQUIRE(claim.token);
  auto snapshot   = snapshot_of(path);
  auto checkpoint = commit_execution(path, snapshot, recovery_change(snapshot, "checkpoint", "note-history", *claim.token));
  REQUIRE(checkpoint);
  snapshot     = snapshot_of(path);
  auto refresh = commit_execution(path, snapshot, recovery_change(snapshot, "refresh", "refresh-history", *claim.token));
  REQUIRE(refresh);
  auto file = open_existing(path, ledger_access::read_only);
  REQUIRE(file.has_value());
  for (auto seq : {claim.event_seq, checkpoint->event_seq, refresh->event_seq}) {
    hist::history_query query;
    query.selector = hist::history_selector::event;
    query.id       = seq;
    auto result    = hist::read_history(*file, query);
    INFO(seq);
    INFO((result ? -1 : static_cast<int>(result.error().code)));
    REQUIRE(result);
    const auto& data = (*result)->fields.at("data");
    CHECK(data->fields.at("selector")->string == "event");
    REQUIRE(data->fields.contains("execution"));
  }
  for (auto key : {"claim-history", "refresh-history"}) {
    hist::history_query query;
    query.selector   = hist::history_selector::request;
    query.request_id = key;
    auto result      = hist::read_history(*file, query);
    INFO(key);
    REQUIRE(result);
    const auto& saved = (*result)->fields.at("data")->fields.at("result");
    CHECK(saved->fields.contains("packet"));
    CHECK(saved->fields.contains("manifest"));
  }
}

TEST_CASE("R16 refresh checkpoint predictions refuse an intervening checkpoint commit") {
  scratch    dir;
  const auto path  = activated_ledger(dir);
  const auto claim = commit_claim(path, "claim-prediction", "alpha", "builder");
  REQUIRE(claim.token);
  auto snapshot = snapshot_of(path);
  CHECK(snapshot.next_checkpoint_id == 1);
  auto refresh       = recovery_change(snapshot, "refresh", "refresh-prediction", *claim.token);
  auto note          = recovery_change(snapshot, "checkpoint", "note-prediction", *claim.token);
  refresh.checkpoint = note.checkpoint;
  auto& request      = std::get<hist::request_v3>(refresh.request);
  request.parameters = params_bytes({{"run_id", decimal(*refresh.run)},
                                     {"expect_fp", "xxh3_128:" + find_role(snapshot, "alpha", "builder")->dispatch->work_fp},
                                     {"observations_path", "notes.json"}});
  REQUIRE(commit_execution(path, snapshot, note));
  auto refused = commit_execution(path, snapshot, refresh);
  REQUIRE_FALSE(refused);
  CHECK(refused.error().code == execution_error_code::state_conflict);
  auto current = snapshot_of(path);
  CHECK(current.next_checkpoint_id == 2);
  CHECK(current.last_seq == snapshot.last_seq + 1);
}

TEST_CASE("C01 token mutations losing to release report the terminal run state") {
  for (auto op : {"refresh", "release"}) {
    CAPTURE(op);
    scratch    dir;
    const auto path  = activated_ledger(dir);
    const auto claim = commit_claim(path, "claim-race", "alpha", "builder");
    REQUIRE(claim.token);
    const auto stale  = snapshot_of(path);
    const auto change = recovery_change(stale, op, std::string(op) + "-race", *claim.token);
    REQUIRE(commit_execution(path, stale, recovery_change(stale, "release", "release-winner", *claim.token)));
    const auto before = file_contents(dir.root);
    const auto result = commit_execution(path, stale, change);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == execution_error_code::fenced);
    CHECK(result.error().state == "interrupted");
    CHECK(file_contents(dir.root) == before);
  }
}

TEST_CASE("C01 token mutations losing to takeover preserve earlier fact conflict precedence") {
  for (auto op : {"refresh", "release"}) {
    CAPTURE(op);
    scratch    dir;
    const auto path  = activated_ledger(dir);
    const auto claim = commit_claim(path, "claim-race", "alpha", "builder");
    REQUIRE(claim.token);
    const auto stale  = snapshot_of(path);
    const auto change = recovery_change(stale, op, std::string(op) + "-race", *claim.token);
    commit_takeover_current(path, "takeover-winner", "alpha", "builder");
    const auto before = file_contents(dir.root);
    const auto result = commit_execution(path, stale, change);
    REQUIRE_FALSE(result);
    // Takeover changes fact_version before role_state is checked (EX-3 order).
    CHECK(result.error().code == execution_error_code::state_conflict);
    auto authorized = authorize_run_token(path, *claim.token);
    REQUIRE_FALSE(authorized);
    CHECK(authorized.error().code == execution_error_code::fenced);
    CHECK(authorized.error().state == "fenced");
    CHECK(file_contents(dir.root) == before);
  }
}
