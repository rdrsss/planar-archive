// R18 ledger read side: the snapshot review value of DR18-8 and the cross-row invariants 21 to 27 of R18-T.
// Every ledger lives under a per-test mkdtemp root. Ledgers are driven through the R15 commit path (load, activate,
// claim), then the verdict commit rows are written by hand with raw SQL in the shape R18-4 prescribes (the verdict
// writer is P3). Each invariant test first shows the consistent row set verifies and reads, then corrupts one column or
// row with every trigger dropped and recreated (D43), and requires ledger_corrupt from the verifier and the reader.
#include <catch2/catch_test_macros.hpp>
#include <stdlib.h>
#include <unistd.h>
import std;
import planar.engine.canonical;
import planar.engine.hash;
import planar.engine.value;
import planar.engine.history.execution_codec;
import planar.engine.history.read;
import planar.engine.history.record;
import planar.engine.history.request_v2;
import planar.engine.history.request_v3;
import planar.engine.ledger.execution;
import planar.engine.ledger.execution_schema;
import planar.engine.ledger.sqlite;

using namespace planar::engine::ledger;
using planar::engine::canonical_encode;
using planar::engine::fragment_digest;
using planar::engine::hash_value;
using planar::engine::value;
using planar::engine::value_kind;
using planar::engine::value_ptr;
namespace hist = planar::engine::history;

namespace {

using kind_t = execution_expectation::kind_t;

constexpr std::string_view workspace_name = "alpha_workspace";

struct scratch {
  std::filesystem::path root;
  scratch() {
    char  pattern[] = "/tmp/planar-review-test-XXXXXX";
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
// Raw SQL.

connection open_raw(const std::filesystem::path& path) {
  auto db = connection::open(path, open_mode::read_write);
  REQUIRE(db.has_value());
  return std::move(*db);
}

struct blob {
  std::string bytes;
};
using arg = std::variant<std::int64_t, std::string, blob, std::nullptr_t>;

void exec(connection& db, std::string_view sql, const std::vector<arg>& args = {}) {
  auto stmt = db.prepare(sql);
  INFO(sql);
  REQUIRE(stmt.has_value());
  for (std::size_t i = 0; i < args.size(); ++i) {
    const int index = static_cast<int>(i + 1);
    if (const auto* n = std::get_if<std::int64_t>(&args[i]))
      REQUIRE(stmt->bind_integer(index, *n).has_value());
    else if (const auto* t = std::get_if<std::string>(&args[i]))
      REQUIRE(stmt->bind_text(index, *t).has_value());
    else if (const auto* b = std::get_if<blob>(&args[i]))
      REQUIRE(stmt->bind_blob(index, b->bytes).has_value());
    else
      REQUIRE(stmt->bind_null(index).has_value());
  }
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
  auto column = stmt->integer(0);
  REQUIRE(column.has_value());
  REQUIRE(column->has_value());
  return **column;
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
    REQUIRE((name && *name && sql && *sql));
    out.emplace_back(**name, **sql);
  }
  return out;
}

/// Apply corrupting SQL with every trigger dropped, then recreate each trigger with its stored text (D43).
void corrupt(const std::filesystem::path& path, const std::vector<std::string>& statements) {
  auto db       = open_raw(path);
  auto triggers = trigger_texts(db);
  exec(db, "PRAGMA foreign_keys=OFF");
  for (const auto& [name, sql] : triggers) {
    (void)sql;
    exec(db, std::format("DROP TRIGGER \"{}\"", name));
  }
  for (const auto& statement : statements)
    exec(db, statement);
  for (const auto& [name, sql] : triggers) {
    (void)name;
    exec(db, sql);
  }
}

// ---------------------------------------------------------------------------------------------------------------------
// Values.

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
// Records and revision seeding.

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

std::uint64_t last_id(connection& db) {
  return static_cast<std::uint64_t>(int_of(db, "SELECT last_insert_rowid()"));
}

stored store_record(connection& db, const hist::record& rec) {
  auto encoded = hist::encode_record(rec);
  REQUIRE(encoded.has_value());
  auto digest = hist::digest_record(rec);
  REQUIRE(digest.has_value());
  exec(db, "INSERT INTO records(kind,codec,digest,encoded) VALUES(?1,1,?2,?3)",
       {static_cast<std::int64_t>(hist::kind_of(rec)), blob{digest_text(*digest)}, blob{*encoded}});
  return stored{last_id(db), *digest};
}

stored store_directory(connection& db, const std::vector<child>& children) {
  hist::directory_record directory;
  for (const auto& entry : children)
    directory.children.push_back(hist::record_child{entry.name, entry.kind, entry.node.id, entry.node.digest});
  const auto   node    = store_record(db, hist::record{directory});
  std::int64_t ordinal = 0;
  for (const auto& entry : children) {
    exec(db, "INSERT INTO tree_edges(parent_id,ordinal,name,child_id) VALUES(?1,?2,?3,?4)",
         {static_cast<std::int64_t>(node.id), ordinal, entry.name, static_cast<std::int64_t>(entry.node.id)});
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

std::uint64_t store_execution_value(connection& db, std::string_view codec_id, std::string_view canonical) {
  auto frame = hist::encode_execution_value_record(codec_id, canonical);
  REQUIRE(frame.has_value());
  auto digest = hist::digest_execution_value_record(*frame);
  REQUIRE(digest.has_value());
  exec(db, "INSERT INTO records(kind,codec,digest,encoded) VALUES(3,1,?1,?2)", {blob{digest_text(*digest)}, blob{*frame}});
  return last_id(db);
}

std::uint64_t store_request_v3(connection& db, const hist::request_v3& request) {
  auto encoded = hist::encode_request_v3(request);
  REQUIRE(encoded.has_value());
  auto digest = hist::digest_request_v3(request);
  REQUIRE(digest.has_value());
  exec(db, "INSERT INTO records(kind,codec,digest,encoded) VALUES(4,3,?1,?2)", {blob{digest_text(*digest)}, blob{*encoded}});
  return last_id(db);
}

struct revision_ids {
  std::uint64_t source = 0, registry = 0, stdlib = 0, interpretation = 0, versions = 0;
};

revision_ids seed_revision(const std::filesystem::path& path, std::string_view tag) {
  auto db             = open_raw(path);
  auto lib_file       = store_file(db, "stdlib body v1");
  auto stdlib         = store_directory(db, {child{"stdlib.lua", hist::record_kind::file, lib_file}});
  auto source_file    = store_file(db, std::format("source {}", tag));
  auto source         = store_directory(db, {child{"source.txt", hist::record_kind::file, source_file}});
  auto input_file     = store_file(db, std::format("{{\"tag\":\"{}\"}}", tag));
  auto registry       = store_directory(db, {child{"input.json", hist::record_kind::file, input_file}});
  auto version_value  = vrec({{"stdlib_digest", vstr(digest_hex(stdlib.digest))}});
  auto versions       = store_value(db, version_value);
  auto interpretation = store_value(
      db, vrec({{"tag", vstr(std::string(tag))},
                {"versions", version_value},
                {"bundle", vrec({{"workspace", vrec({{"id", vstr(std::string(workspace_name))}})}, {"jobs", vlist({})}})}}));
  return revision_ids{source.id, registry.id, stdlib.id, interpretation.id, versions.id};
}

hist::role_structure_v1 structure_of(std::string_view job, std::string_view role, std::string_view kind) {
  hist::role_structure_v1 out;
  out.job   = std::string(job);
  out.role  = std::string(role);
  out.kind  = std::string(kind);
  out.shape = "linear";
  if (kind == "worker")
    out.work.push_back("unit_one");
  else
    out.targets.push_back("unit_one");
  return out;
}

// ---------------------------------------------------------------------------------------------------------------------
// Snapshot helpers and change builders (the R15 commit path).

execution_snapshot snapshot_of(const std::filesystem::path& path) {
  auto snapshot = read_execution_snapshot(path);
  INFO("snapshot read failed with error code " << (snapshot.has_value() ? -1 : static_cast<int>(snapshot.error().code)));
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

void require_verifies(const std::filesystem::path& path) {
  auto verified = verify_execution_ledger(path);
  INFO("verification failed with error code " << (verified.has_value() ? -1 : static_cast<int>(verified.error().code)));
  REQUIRE(verified.has_value());
  REQUIRE(read_execution_snapshot(path).has_value());
}

void expect_corrupt(const std::filesystem::path& path) {
  auto verified = verify_execution_ledger(path);
  REQUIRE_FALSE(verified.has_value());
  REQUIRE(verified.error().code == execution_error_code::ledger_corrupt);
  auto read = read_execution_snapshot(path);
  REQUIRE_FALSE(read.has_value());
  REQUIRE(read.error().code == execution_error_code::ledger_corrupt);
}

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

std::vector<execution_expectation> lifecycle_expected(const execution_snapshot& snapshot, const std::vector<std::string>& jobs) {
  auto selected = [&](const execution_role& row) { return std::ranges::find(jobs, row.job) != jobs.end(); };
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
  for (const auto& row : snapshot.roles)
    if (row.job == job)
      out.push_back(execution_vector_entry{
          .job = row.job, .role = row.role, .state = row.state, .drifted = row.drifted, .run = row.run_id});
  return out;
}

/// W of the fixture: the 24 keys with the render shape record carrying max_rounds (DB-R5, DB-R6).
value_ptr make_work(std::string_view job, std::string_view role, std::int64_t max_rounds) {
  return vrec({{"versions", vstrs({"1", "1", "1", "1", "1", "1", "1"})},
               {"workspace", vstr(std::string(workspace_name))},
               {"spec", vstr("spec_one")},
               {"job", vstr(std::string(job))},
               {"role", vstr(std::string(role))},
               {"root", vstr("root")},
               {"base", vstr("base")},
               {"goal", vstr("goal")},
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
               {"shape", vrec({{"kind", vstr("linear")},
                               {"max_rounds", vint(max_rounds)},
                               {"mapping", vlist({vrec({{"work", vstr("unit_one")}, {"role", vstr("builder")}})})},
                               {"reviewer", vstr("reviewer")}})},
               {"start_facets", vstrs({})},
               {"repair", vstr("0")}});
}

execution_packet make_packet(const execution_snapshot& snapshot, const execution_role& row, std::string_view request_id,
                             std::int64_t max_rounds) {
  REQUIRE(snapshot.revision.has_value());
  execution_packet packet;
  packet.markdown = std::format("# Packet for {}/{}\n\nDispatch packet.\n", row.job, row.role);
  packet.work     = make_work(row.job, row.role, max_rounds);
  auto work_fp    = hash_value("work_exec_v1", packet.work);
  REQUIRE(work_fp.has_value());
  auto render_fp = fragment_digest(packet.markdown);
  REQUIRE(render_fp.has_value());
  packet.manifest = vrec({
      {"version", vint(1)},
      {"revision", vstr(decimal(snapshot.revision->id))},
      {"job", vstr(row.job)},
      {"role", vstr(row.role)},
      {"generation", vstr(decimal(row.generation_ordinal))},
      {"run", vrec({{"request", vstr(std::string(request_id))}, {"kind", vstr("claim")}})},
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

void commit_or_fail(const std::filesystem::path& path, const execution_snapshot& snapshot, const execution_change& change) {
  auto result = commit_execution(path, snapshot, change);
  INFO("operation " << change.operation << " request " << change.request_id);
  INFO("commit failed with error code " << (result.has_value() ? -1 : static_cast<int>(result.error().code)));
  REQUIRE(result.has_value());
}

void commit_load(const std::filesystem::path& path, std::string_view request_id, const revision_ids& ids) {
  const auto       snapshot = snapshot_of(path);
  execution_change change;
  change.operation  = "load";
  change.request_id = request_id;
  change.actor      = "tester";
  hist::request_v2 request;
  request.operation          = "load";
  request.actor              = "tester";
  request.expected_head      = head_text(snapshot);
  request.workspace_argument = std::string(workspace_name);
  request.registry_argument  = "registry";
  change.request             = request;
  change.job                 = snapshot.active_job;
  change.revision            = execution_revision_input{.source         = ids.source,
                                                        .registry       = ids.registry,
                                                        .stdlib         = ids.stdlib,
                                                        .interpretation = ids.interpretation,
                                                        .versions       = ids.versions,
                                                        .noop           = false};
  change.vector              = base_vector(snapshot, snapshot.active_job);
  change.expected            = lifecycle_expected(snapshot, {});
  commit_or_fail(path, snapshot, change);
}

/// Activate job alpha with a worker builder (ready) and a reviewer (ready), so either can be claimed.
void commit_activate(const std::filesystem::path& path) {
  const auto       snapshot = snapshot_of(path);
  execution_change change;
  change.operation  = "activate";
  change.request_id = "activate-1";
  change.actor      = "tester";
  change.job        = "alpha";
  change.request    = hist::request_v3{
      .operation        = "activate",
      .actor            = "tester",
      .parameters       = params_bytes({{"expect_job", "NONE"}, {"expect_revision", head_text(snapshot)}, {"job", "alpha"}}),
      .profile_argument = std::nullopt};
  for (const auto& [role, kind] :
       std::vector<std::pair<std::string, std::string>>{{"builder", "worker"}, {"reviewer", "reviewer"}}) {
    change.generations.push_back(execution_generation_input{
        .job = "alpha", .role = role, .reason = "activated", .structure = structure_of("alpha", role, kind)});
    change.transitions.push_back(execution_transition_input{.job            = "alpha",
                                                            .role           = role,
                                                            .before_state   = std::nullopt,
                                                            .before_drifted = std::nullopt,
                                                            .after_state    = "ready",
                                                            .after_drifted  = false,
                                                            .reason         = "activated"});
    change.vector.push_back(
        execution_vector_entry{.job = "alpha", .role = role, .state = "ready", .drifted = false, .run = std::nullopt});
  }
  change.expected = lifecycle_expected(snapshot, {"alpha"});
  commit_or_fail(path, snapshot, change);
}

void commit_claim(const std::filesystem::path& path, std::string_view request_id, std::string_view role,
                  std::int64_t max_rounds) {
  const auto  snapshot = snapshot_of(path);
  const auto* row      = find_role(snapshot, "alpha", role);
  REQUIRE(row != nullptr);
  execution_change change;
  change.operation  = "claim";
  change.request_id = request_id;
  change.actor      = "tester";
  change.request    = hist::request_v3{.operation        = "claim",
                                       .actor            = "tester",
                                       .parameters       = params_bytes({{"job", "alpha"}, {"role", std::string(role)}}),
                                       .profile_argument = std::nullopt};
  change.job        = "alpha";
  change.role       = std::string(role);
  change.transitions.push_back(execution_transition_input{.job            = row->job,
                                                          .role           = row->role,
                                                          .before_state   = row->state,
                                                          .before_drifted = false,
                                                          .after_state    = "running",
                                                          .after_drifted  = false,
                                                          .reason         = "claimed"});
  change.vector = base_vector(snapshot, "alpha");
  for (auto& entry : change.vector)
    if (entry.role == role)
      entry =
          execution_vector_entry{.job = entry.job, .role = entry.role, .state = "running", .drifted = false, .run = std::nullopt};
  change.packet   = make_packet(snapshot, *row, request_id, max_rounds);
  change.expected = targeted_expected(snapshot, *row);
  commit_or_fail(path, snapshot, change);
}

/// Fresh ledger: one revision loaded, alpha activated, and the given role claimed with the given W max_rounds.
std::filesystem::path claimed_ledger(const scratch& dir, std::string_view role = "reviewer", std::int64_t max_rounds = 2) {
  const auto path = dir.ledger();
  REQUIRE(initialize_format3(path, workspace_name).has_value());
  commit_load(path, "load-1", seed_revision(path, "one"));
  commit_activate(path);
  commit_claim(path, "claim-1", role, max_rounds);
  return path;
}

// ---------------------------------------------------------------------------------------------------------------------
// Hand-written verdict commit rows (R18-4 shape): value record, request body, event, transition, request, verdict,
// run_state completed, reviewer role_state, optional repairs and blocker, current_state.

constexpr std::string_view fingerprint_one = "xxh3_128:0123456789abcdef0123456789abcdef";
constexpr std::string_view fingerprint_two = "xxh3_128:fedcba9876543210fedcba9876543210";

struct verdict_plan {
  std::string                                        request_id    = "verdict-1";
  std::string                                        role          = "reviewer"; ///< Role whose run records the verdict.
  std::string                                        decision      = "approve";
  std::string                                        applicability = "current";
  std::vector<std::string>                           repair;
  std::string                                        after = "done"; ///< After-state of the recording role.
  std::vector<std::pair<std::string, std::uint64_t>> repairs;        ///< Repairs rows written: role and revision.
  std::optional<std::string>                         blocker;        ///< Kind of the blocker the verdict opens.
  std::uint64_t                                      count      = 0; ///< Saved review count after the commit.
  std::uint64_t                                      max_rounds = 2; ///< Saved max_rounds.
};

struct verdict_ids {
  std::uint64_t verdict = 0;
  std::uint64_t seq     = 0;
  std::uint64_t run     = 0;
  std::uint64_t blocker = 0;
};

hist::verdict_v1 document_of(const verdict_plan& plan) {
  hist::verdict_v1 doc;
  doc.targets   = {{"unit_one", std::string(fingerprint_one), std::string(fingerprint_two)}};
  doc.policy    = std::string(fingerprint_two);
  doc.decision  = plan.decision;
  doc.repair    = plan.repair;
  doc.summary   = "Reviewed.";
  doc.authority = std::string(hist::verdict_authority_reviewer);
  if (plan.decision == "reject")
    doc.findings = {{"F-1", "must", "unit_one", std::string("src/a.cpp"), 3, std::nullopt, "Fix it."}};
  return doc;
}

verdict_ids record_verdict(const std::filesystem::path& path, const verdict_plan& plan) {
  const auto  snapshot = snapshot_of(path);
  const auto* row      = find_role(snapshot, "alpha", plan.role);
  REQUIRE(row != nullptr);
  REQUIRE(row->run_id.has_value());
  const auto* builder = find_role(snapshot, "alpha", "builder");
  REQUIRE(builder != nullptr);
  verdict_ids ids;
  ids.run           = *row->run_id;
  ids.seq           = snapshot.last_seq + 1;
  const auto head   = snapshot.revision->id;
  auto       db     = open_raw(path);
  ids.verdict       = static_cast<std::uint64_t>(int_of(db, "SELECT COALESCE(MAX(id),0)+1 FROM verdicts"));
  auto encoded_body = hist::encode_verdict_v1(document_of(plan));
  REQUIRE(encoded_body.has_value());
  const auto body_id      = store_execution_value(db, hist::codec_verdict_v1, *encoded_body);
  const auto request_body = store_request_v3(
      db, hist::request_v3{.operation  = "verdict",
                           .actor      = "tester",
                           .parameters = params_bytes({{"run_id", decimal(ids.run)}, {"verdict_path", "/private/verdict.json"}}),
                           .profile_argument = std::nullopt});
  hist::event_payload_v2 payload;
  payload.operation  = "verdict";
  payload.before     = hist::payload_position{decimal(head), "alpha"};
  payload.after      = hist::payload_position{decimal(head), "alpha"};
  payload.reason     = "completed";
  payload.references = {{"body", decimal(body_id)}, {"run", decimal(ids.run)}, {"verdict", decimal(ids.verdict)}};
  auto payload_bytes = hist::encode_event_payload_v2("verdict_recorded", payload);
  REQUIRE(payload_bytes.has_value());
  if (plan.blocker)
    ids.blocker = static_cast<std::uint64_t>(int_of(db, "SELECT COALESCE(MAX(id),0)+1 FROM blockers"));

  hist::verdict_result_v1 result;
  result.ledger        = snapshot.identity;
  result.request_id    = plan.request_id;
  result.job           = "alpha";
  result.role          = plan.role;
  result.revision      = decimal(head);
  result.run           = decimal(ids.run);
  result.event         = decimal(ids.seq);
  result.verdict_id    = decimal(ids.verdict);
  result.applicability = plan.applicability;
  result.decision      = plan.decision;
  result.admitted      = !plan.repairs.empty();
  for (const auto& [role, revision] : plan.repairs)
    result.repairs.push_back(hist::verdict_result_repair{role, decimal(builder->generation_ordinal), decimal(revision)});
  result.count      = decimal(plan.count);
  result.max_rounds = decimal(plan.max_rounds);
  if (plan.blocker)
    result.opened.push_back(hist::verdict_result_blocker{decimal(ids.blocker), *plan.blocker});
  result.roles = {hist::verdict_result_role{plan.role, plan.after, false}};
  auto saved   = hist::encode_verdict_result_v1(result);
  REQUIRE(saved.has_value());

  const auto seq = static_cast<std::int64_t>(ids.seq);
  exec(db,
       "INSERT INTO events(seq,type,version,revision_id,request_id,actor,occurred_at,payload,job,role,generation_id,run_id) "
       "VALUES(?1,'verdict_recorded',2,?2,?3,'tester','2026-11-08T00:00:00.000Z',?4,'alpha',?5,?6,?7)",
       {seq, static_cast<std::int64_t>(head), plan.request_id, blob{*payload_bytes}, plan.role,
        static_cast<std::int64_t>(row->generation_id), static_cast<std::int64_t>(ids.run)});
  exec(db,
       "INSERT INTO transitions(event_seq,ordinal,job,role,generation_id,before_state,after_state,before_drifted,after_drifted,"
       "reason) VALUES(?1,1,'alpha',?2,?3,'running',?4,0,0,'completed')",
       {seq, plan.role, static_cast<std::int64_t>(row->generation_id), plan.after});
  exec(db,
       "INSERT INTO requests(request_id,operation,body_id,event_seq,revision_id,outcome,result,attachment_id) "
       "VALUES(?1,'verdict',?2,?3,?4,'recorded',?5,NULL)",
       {plan.request_id, static_cast<std::int64_t>(request_body), seq, static_cast<std::int64_t>(head), blob{*saved}});
  exec(db, "INSERT INTO verdicts(id,request_id,run_id,seq,body_id,applicability,decision) VALUES(?1,?2,?3,?4,?5,?6,?7)",
       {static_cast<std::int64_t>(ids.verdict), plan.request_id, static_cast<std::int64_t>(ids.run), seq,
        static_cast<std::int64_t>(body_id), plan.applicability, plan.decision});
  for (const auto& [role, revision] : plan.repairs)
    exec(db, "INSERT INTO repairs(verdict_id,job,role,generation_id,revision) VALUES(?1,'alpha',?2,?3,?4)",
         {static_cast<std::int64_t>(ids.verdict), role, static_cast<std::int64_t>(builder->generation_id),
          static_cast<std::int64_t>(revision)});
  if (plan.blocker) {
    exec(db, "INSERT INTO blockers(id,seq,job,kind,verdict_id) VALUES(?1,?2,'alpha',?3,?4)",
         {static_cast<std::int64_t>(ids.blocker), seq, *plan.blocker, static_cast<std::int64_t>(ids.verdict)});
    exec(db, "INSERT INTO blocker_state(blocker_id,state,cleared_seq,last_seq) VALUES(?1,'open',NULL,?2)",
         {static_cast<std::int64_t>(ids.blocker), seq});
  }
  exec(db, "UPDATE run_state SET state='completed', last_seq=?1 WHERE run_id=?2", {seq, static_cast<std::int64_t>(ids.run)});
  exec(db,
       "UPDATE role_state SET state=?1, drifted=0, run_id=NULL, fact_version=fact_version+1, last_seq=?2 "
       "WHERE job='alpha' AND role=?3",
       {plan.after, seq, plan.role});
  exec(db, "UPDATE current_state SET last_seq=?1", {seq});
  return ids;
}

verdict_plan approve_plan() {
  return verdict_plan{};
}

verdict_plan admitted_plan() {
  verdict_plan plan;
  plan.decision = "reject";
  plan.repair   = {"builder"};
  plan.after    = "blocked";
  plan.repairs  = {{"builder", 1}};
  plan.count    = 1;
  return plan;
}

verdict_plan unowned_plan() {
  verdict_plan plan;
  plan.decision = "reject";
  plan.blocker  = "unowned_rejection";
  return plan;
}

/// Each corruption runs on a fresh ledger with the consistent set first verified, and must be refused.
void expect_each_corrupt(const verdict_plan&                                                plan,
                         const std::function<std::vector<std::string>(const verdict_ids&)>& corruption) {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  const auto ids  = record_verdict(path, plan);
  require_verifies(path);
  corrupt(path, corruption(ids));
  expect_corrupt(path);
}

} // namespace

// ---------------------------------------------------------------------------------------------------------------------
// Snapshot review value (DR18-8).

TEST_CASE("R18-B fresh format3 ledger reads an empty review", "[r18][ledger_review]") {
  scratch    dir;
  const auto path = dir.ledger();
  REQUIRE(initialize_format3(path, workspace_name).has_value());
  const auto fresh = snapshot_of(path);
  CHECK(fresh.review.count == 0);
  CHECK(fresh.review.verdicts.empty());
  CHECK(fresh.review.blockers.empty());
  CHECK(fresh.review.repairs.empty());
  CHECK(fresh.completions.empty());
  CHECK(execution_state_bytes(fresh) == fresh.state_bytes);

  scratch    claimed_dir;
  const auto claimed  = claimed_ledger(claimed_dir);
  const auto snapshot = snapshot_of(claimed);
  CHECK(snapshot.review.count == 0);
  CHECK(snapshot.review.verdicts.empty());
  CHECK(snapshot.review.blockers.empty());
  CHECK(snapshot.review.repairs.empty());
  CHECK(snapshot.completions.empty());
  CHECK(execution_state_bytes(snapshot) == snapshot.state_bytes);
}

TEST_CASE("R18-B a consistent approve verdict verifies and reads into the review", "[r18][ledger_review]") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  const auto ids  = record_verdict(path, approve_plan());
  require_verifies(path);
  const auto snapshot = snapshot_of(path);
  REQUIRE(snapshot.review.verdicts.size() == 1);
  const auto& verdict = snapshot.review.verdicts.front();
  CHECK(verdict.id == ids.verdict);
  CHECK(verdict.request_id == "verdict-1");
  CHECK(verdict.run == ids.run);
  CHECK(verdict.seq == ids.seq);
  CHECK(verdict.applicability == "current");
  CHECK(verdict.decision == "approve");
  auto stored   = hist::encode_verdict_v1(verdict.body);
  auto expected = hist::encode_verdict_v1(document_of(approve_plan()));
  REQUIRE(stored.has_value());
  REQUIRE(expected.has_value());
  CHECK(*stored == *expected);
  CHECK(snapshot.review.count == 0);
  CHECK(snapshot.review.repairs.empty());
  CHECK(snapshot.review.blockers.empty());
  const auto* reviewer = find_role(snapshot, "alpha", "reviewer");
  REQUIRE(reviewer != nullptr);
  CHECK(reviewer->state == "done");
  CHECK_FALSE(reviewer->run_id.has_value());
  // The review value counts toward the D24 measure in its one shared definition.
  CHECK(snapshot.state_bytes == execution_state_bytes(snapshot));
  auto without   = snapshot;
  without.review = execution_review{};
  CHECK(execution_state_bytes(without) < snapshot.state_bytes);
}

TEST_CASE("R18-B an admitted rejection reads its count and current-lineage repair revision", "[r18][ledger_review]") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  record_verdict(path, admitted_plan());
  require_verifies(path);
  const auto snapshot = snapshot_of(path);
  CHECK(snapshot.review.count == 1);
  REQUIRE(snapshot.review.repairs.size() == 1);
  CHECK(snapshot.review.repairs[0].job == "alpha");
  CHECK(snapshot.review.repairs[0].role == "builder");
  CHECK(snapshot.review.repairs[0].revision == 1);
  CHECK(snapshot.review.blockers.empty());
  REQUIRE(snapshot.review.verdicts.size() == 1);
  CHECK(snapshot.review.verdicts[0].decision == "reject");
}

TEST_CASE("R18-B an unowned rejection reads its open blocker", "[r18][ledger_review]") {
  scratch    dir;
  const auto path = claimed_ledger(dir);
  const auto ids  = record_verdict(path, unowned_plan());
  require_verifies(path);
  const auto snapshot = snapshot_of(path);
  CHECK(snapshot.review.count == 0);
  REQUIRE(snapshot.review.blockers.size() == 1);
  CHECK(snapshot.review.blockers[0].id == ids.blocker);
  CHECK(snapshot.review.blockers[0].kind == "unowned_rejection");
  CHECK(snapshot.review.blockers[0].verdict == ids.verdict);
  CHECK(snapshot.review.blockers[0].seq == ids.seq);
  CHECK(snapshot.review.repairs.empty());
}

TEST_CASE("R18-B a capped rejection replays with the reviewer W max_rounds", "[r18][ledger_review]") {
  // max_rounds 0 in the reviewer's acknowledged W: the first owned rejection is already capped (DB-R6).
  scratch    dir;
  const auto path = claimed_ledger(dir, "reviewer", 0);
  auto       plan = verdict_plan{};
  plan.decision   = "reject";
  plan.repair     = {"builder"};
  plan.blocker    = "max_rounds";
  plan.max_rounds = 0;
  record_verdict(path, plan);
  require_verifies(path);
  const auto snapshot = snapshot_of(path);
  REQUIRE(snapshot.review.blockers.size() == 1);
  CHECK(snapshot.review.blockers[0].kind == "max_rounds");
  CHECK(snapshot.review.count == 0);

  // The same rejection recorded as admitted contradicts the cap of 0.
  scratch    admitted_dir;
  const auto admitted = claimed_ledger(admitted_dir, "reviewer", 0);
  auto       wrong    = admitted_plan();
  wrong.max_rounds    = 0;
  record_verdict(admitted, wrong);
  expect_corrupt(admitted);
}

// ---------------------------------------------------------------------------------------------------------------------
// Invariants 21 to 27, each refused as ledger_corrupt after the consistent set is accepted.

TEST_CASE("R18-B invariant 21 verdict request event bijection", "[r18][ledger_review]") {
  // The verdict names a request of another operation.
  expect_each_corrupt(approve_plan(),
                      [](const verdict_ids&) { return std::vector<std::string>{"UPDATE verdicts SET request_id='claim-1'"}; });
  // A verdict_recorded event without its verdicts row.
  expect_each_corrupt(approve_plan(), [](const verdict_ids&) { return std::vector<std::string>{"DELETE FROM verdicts"}; });
  // The verdict body is not a verdict_v1 value record.
  expect_each_corrupt(approve_plan(), [](const verdict_ids&) {
    return std::vector<std::string>{"UPDATE verdicts SET body_id=(SELECT work_id FROM dispatches LIMIT 1)"};
  });
  // The stored decision differs from the decoded document.
  expect_each_corrupt(approve_plan(),
                      [](const verdict_ids&) { return std::vector<std::string>{"UPDATE verdicts SET decision='reject'"}; });
}

TEST_CASE("R18-B invariant 22 a verdict run is a completed reviewer run", "[r18][ledger_review]") {
  // The same consistent rows on a worker's run: everything matches except the role kind.
  scratch    dir;
  const auto path = claimed_ledger(dir, "builder");
  auto       plan = approve_plan();
  plan.role       = "builder";
  record_verdict(path, plan);
  expect_corrupt(path);
}

TEST_CASE("R18-B invariant 23 repairs equal the document repair list", "[r18][ledger_review]") {
  expect_each_corrupt(admitted_plan(), [](const verdict_ids& ids) {
    return std::vector<std::string>{std::format(
        "INSERT INTO repairs(verdict_id,job,role,generation_id,revision) SELECT {},'alpha','reviewer',generation_id,1 "
        "FROM role_state WHERE role='reviewer'",
        ids.verdict)};
  });
  expect_each_corrupt(admitted_plan(),
                      [](const verdict_ids&) { return std::vector<std::string>{"UPDATE repairs SET job='beta'"}; });
}

TEST_CASE("R18-B invariant 24 repair revisions are contiguous per lineage", "[r18][ledger_review]") {
  expect_each_corrupt(admitted_plan(),
                      [](const verdict_ids&) { return std::vector<std::string>{"UPDATE repairs SET revision=2"}; });
}

TEST_CASE("R18-B invariant 25 approve verdicts have no repairs and no blockers", "[r18][ledger_review]") {
  expect_each_corrupt(approve_plan(), [](const verdict_ids& ids) {
    return std::vector<std::string>{
        std::format("INSERT INTO repairs(verdict_id,job,role,generation_id,revision) SELECT {},'alpha','builder',generation_id,1 "
                    "FROM role_state WHERE role='builder'",
                    ids.verdict)};
  });
  expect_each_corrupt(approve_plan(), [](const verdict_ids& ids) {
    return std::vector<std::string>{
        std::format("INSERT INTO blockers(id,seq,job,kind,verdict_id) VALUES(1,{},'alpha','unowned_rejection',{})", ids.seq,
                    ids.verdict),
        std::format("INSERT INTO blocker_state(blocker_id,state,cleared_seq,last_seq) VALUES(1,'open',NULL,{})", ids.seq)};
  });
  // A second blocker on one verdict.
  expect_each_corrupt(unowned_plan(), [](const verdict_ids& ids) {
    return std::vector<std::string>{
        std::format("INSERT INTO blockers(id,seq,job,kind,verdict_id) VALUES(2,{},'alpha','max_rounds',{})", ids.seq,
                    ids.verdict),
        std::format("INSERT INTO blocker_state(blocker_id,state,cleared_seq,last_seq) VALUES(2,'open',NULL,{})", ids.seq)};
  });
}

TEST_CASE("R18-B invariant 26 blocker and blocker_state pairing and clearing order", "[r18][ledger_review]") {
  // cleared_seq must be greater than the opening seq.
  expect_each_corrupt(unowned_plan(), [](const verdict_ids& ids) {
    return std::vector<std::string>{
        std::format("UPDATE blocker_state SET state='cleared', cleared_seq={0}, last_seq={0}", ids.seq)};
  });
  // A blockers row without its state row.
  expect_each_corrupt(unowned_plan(), [](const verdict_ids&) { return std::vector<std::string>{"DELETE FROM blocker_state"}; });
  // A state row without its blockers row.
  expect_each_corrupt(unowned_plan(), [](const verdict_ids& ids) {
    return std::vector<std::string>{
        std::format("INSERT INTO blocker_state(blocker_id,state,cleared_seq,last_seq) VALUES(9,'open',NULL,{})", ids.seq)};
  });
  // An open blocker whose last_seq is not its opening seq.
  expect_each_corrupt(unowned_plan(),
                      [](const verdict_ids&) { return std::vector<std::string>{"UPDATE blocker_state SET last_seq=1"}; });
}

TEST_CASE("R18-B invariant 27 admission replay reproduces admitted capped and unowned", "[r18][ledger_review]") {
  // An admitted rejection (count 0 below max_rounds 2) without its repairs row.
  expect_each_corrupt(admitted_plan(), [](const verdict_ids&) { return std::vector<std::string>{"DELETE FROM repairs"}; });
  // An unowned rejection with no blocker open for the job must open one.
  expect_each_corrupt(unowned_plan(), [](const verdict_ids&) {
    return std::vector<std::string>{"DELETE FROM blocker_state", "DELETE FROM blockers"};
  });
  // An unowned rejection that opened the wrong kind.
  expect_each_corrupt(unowned_plan(),
                      [](const verdict_ids&) { return std::vector<std::string>{"UPDATE blockers SET kind='max_rounds'"}; });
}

// ---------------------------------------------------------------------------------------------------------------------
// R18 P3: the verdict commit through stage_execution. Every ledger is driven through the real R15 commit path (load,
// activate, claim, takeover). A worker completes through a real load commit (running -> done, its run fenced) plus the
// DRAFT-STUB R17-C completions row and observation record written by SQL in the test (DB-R9): R17 check_recorded does
// not exist in this lane and the reader checks completions structurally only (DB-R18).

namespace {

constexpr std::string_view output_one   = "xxh3_128:11111111111111111111111111111111";
constexpr std::string_view output_two   = "xxh3_128:22222222222222222222222222222222";
constexpr std::string_view output_three = "xxh3_128:33333333333333333333333333333333";

value_ptr with_field(const value_ptr& base, const std::string& key, const value_ptr& item) {
  value copy       = *base;
  copy.fields[key] = item;
  return std::make_shared<const value>(std::move(copy));
}

/// W of the fixture with a chosen repair revision text, so each repaired claim has a new work fingerprint.
value_ptr review_work(std::string_view role, std::int64_t max_rounds, std::string_view repair) {
  return with_field(make_work("alpha", role, max_rounds), "repair", vstr(std::string(repair)));
}

execution_packet packet_with(const execution_snapshot& snapshot, const execution_role& row, std::string_view request_id,
                             const value_ptr& work) {
  auto packet = make_packet(snapshot, row, request_id, 2);
  packet.work = work;
  auto digest = hash_value("work_exec_v1", work);
  REQUIRE(digest.has_value());
  packet.manifest = with_field(packet.manifest, "work", vrec({{"digest", vstr(*digest)}}));
  return packet;
}

/// Claim a role of alpha (ready or stale) with a W of the given max_rounds and repair text; returns the token.
std::string claim_with(const std::filesystem::path& path, std::string_view request_id, std::string_view role,
                       std::int64_t max_rounds = 2, std::string_view repair = "0") {
  const auto  snapshot = snapshot_of(path);
  const auto* row      = find_role(snapshot, "alpha", role);
  REQUIRE(row != nullptr);
  execution_change change;
  change.operation  = "claim";
  change.request_id = request_id;
  change.actor      = "tester";
  change.request    = hist::request_v3{.operation        = "claim",
                                       .actor            = "tester",
                                       .parameters       = params_bytes({{"job", "alpha"}, {"role", std::string(role)}}),
                                       .profile_argument = std::nullopt};
  change.job        = "alpha";
  change.role       = std::string(role);
  change.transitions.push_back(execution_transition_input{.job            = row->job,
                                                          .role           = row->role,
                                                          .before_state   = row->state,
                                                          .before_drifted = false,
                                                          .after_state    = "running",
                                                          .after_drifted  = false,
                                                          .reason         = "claimed"});
  change.vector = base_vector(snapshot, "alpha");
  for (auto& entry : change.vector)
    if (entry.role == role)
      entry =
          execution_vector_entry{.job = entry.job, .role = entry.role, .state = "running", .drifted = false, .run = std::nullopt};
  change.packet   = packet_with(snapshot, *row, request_id, review_work(role, max_rounds, repair));
  change.expected = targeted_expected(snapshot, *row);
  auto result     = commit_execution(path, snapshot, change);
  INFO("claim failed with error code " << (result.has_value() ? -1 : static_cast<int>(result.error().code)));
  REQUIRE(result.has_value());
  REQUIRE(result->token.has_value());
  return *result->token;
}

/// Take over the running role of alpha; the new generation has reason takeover and the same structure (EX-5).
void takeover_role(const std::filesystem::path& path, std::string_view request_id, std::string_view role) {
  const auto  snapshot = snapshot_of(path);
  const auto* row      = find_role(snapshot, "alpha", role);
  REQUIRE(row != nullptr);
  REQUIRE(row->run_id.has_value());
  const auto       previous = *row->run_id;
  execution_change change;
  change.operation  = "takeover";
  change.request_id = request_id;
  change.actor      = "tester";
  change.request =
      hist::request_v3{.operation  = "takeover",
                       .actor      = "tester",
                       .parameters = params_bytes({{"job", "alpha"}, {"role", std::string(role)}, {"run_id", decimal(previous)}}),
                       .profile_argument = std::nullopt};
  change.job          = "alpha";
  change.role         = std::string(role);
  change.previous_run = previous;
  change.generations.push_back(
      execution_generation_input{.job = "alpha", .role = std::string(role), .reason = "takeover", .structure = row->structure});
  change.transitions.push_back(execution_transition_input{.job            = "alpha",
                                                          .role           = std::string(role),
                                                          .before_state   = std::string("running"),
                                                          .before_drifted = false,
                                                          .after_state    = "running",
                                                          .after_drifted  = false,
                                                          .reason         = "taken_over"});
  change.fenced_runs.push_back(previous);
  change.vector = base_vector(snapshot, "alpha");
  for (auto& entry : change.vector)
    if (entry.role == role)
      entry =
          execution_vector_entry{.job = entry.job, .role = entry.role, .state = "running", .drifted = false, .run = std::nullopt};
  change.packet   = packet_with(snapshot, *row, request_id, row->dispatch->work);
  change.expected = targeted_expected(snapshot, *row);
  commit_or_fail(path, snapshot, change);
}

/// A load of a fresh revision whose change carries the given transitions of alpha and the blockers it clears.
execution_change load_with(const execution_snapshot& snapshot, std::string_view request_id, const revision_ids& ids,
                           const std::vector<execution_transition_input>& transitions,
                           const std::vector<std::uint64_t>&              cleared = {}) {
  execution_change change;
  change.operation  = "load";
  change.request_id = request_id;
  change.actor      = "tester";
  hist::request_v2 request;
  request.operation          = "load";
  request.actor              = "tester";
  request.expected_head      = head_text(snapshot);
  request.workspace_argument = std::string(workspace_name);
  request.registry_argument  = "registry";
  change.request             = request;
  change.job                 = snapshot.active_job;
  change.revision            = execution_revision_input{.source         = ids.source,
                                                        .registry       = ids.registry,
                                                        .stdlib         = ids.stdlib,
                                                        .interpretation = ids.interpretation,
                                                        .versions       = ids.versions,
                                                        .noop           = false};
  change.transitions         = transitions;
  change.vector              = base_vector(snapshot, snapshot.active_job);
  for (const auto& t : transitions) {
    for (auto& entry : change.vector)
      if (entry.role == t.role)
        entry =
            execution_vector_entry{.job = t.job, .role = t.role, .state = t.after_state, .drifted = false, .run = std::nullopt};
    const auto* row = find_role(snapshot, t.job, t.role);
    if (row && row->state == "running" && t.after_state != "running")
      change.fenced_runs.push_back(*row->run_id);
  }
  change.expected         = lifecycle_expected(snapshot, {snapshot.active_job});
  change.blockers_cleared = cleared;
  return change;
}

execution_transition_input step_to(const execution_role& row, std::string after, std::string reason) {
  return execution_transition_input{.job            = row.job,
                                    .role           = row.role,
                                    .before_state   = row.state,
                                    .before_drifted = row.drifted,
                                    .after_state    = std::move(after),
                                    .after_drifted  = false,
                                    .reason         = std::move(reason)};
}

/// DRAFT-STUB R17-C: the running builder completes. A real load moves it running -> done (its run fenced) and a
/// blocked or stale reviewer -> ready; then the completions row and its observation record are written by SQL.
void complete_builder(const std::filesystem::path& path, std::string_view request_id, std::string_view output) {
  const auto  snapshot = snapshot_of(path);
  const auto* builder  = find_role(snapshot, "alpha", "builder");
  REQUIRE(builder != nullptr);
  REQUIRE(builder->state == "running");
  REQUIRE(builder->dispatch.has_value());
  const auto                              run      = *builder->run_id;
  const auto                              dispatch = builder->dispatch->id;
  std::vector<execution_transition_input> transitions{step_to(*builder, "done", "completed")};
  const auto*                             reviewer = find_role(snapshot, "alpha", "reviewer");
  if (reviewer && (reviewer->state == "blocked"))
    transitions.push_back(step_to(*reviewer, "ready", "prerequisite_available"));
  commit_or_fail(path, snapshot, load_with(snapshot, request_id, seed_revision(path, request_id), transitions));
  const auto after = snapshot_of(path);
  auto       db    = open_raw(path);
  const auto observation =
      store_value(db, vrec({{"units", vlist({vrec({{"work", vstr("unit_one")}, {"output", vstr(std::string(output))}})})}}));
  exec(db,
       "INSERT INTO completions(id,run_id,seq,dispatch_id,work_fp,observation_id) "
       "SELECT COALESCE((SELECT MAX(id) FROM completions),0)+1,?1,?2,?3,work_fp,?4 FROM dispatches WHERE id=?3",
       {static_cast<std::int64_t>(run), static_cast<std::int64_t>(after.last_seq), static_cast<std::int64_t>(dispatch),
        static_cast<std::int64_t>(observation.id)});
}

/// Fresh ledger: load, activate (builder and reviewer ready), builder claimed and completed with output_one, reviewer
/// claimed with the given max_rounds. Returns the reviewer token.
std::string reviewing_ledger(const std::filesystem::path& path, std::int64_t max_rounds = 2) {
  REQUIRE(initialize_format3(path, workspace_name).has_value());
  commit_load(path, "load-1", seed_revision(path, "one"));
  commit_activate(path);
  claim_with(path, "claim-builder-1", "builder", max_rounds, "0");
  complete_builder(path, "complete-1", output_one);
  return claim_with(path, "claim-reviewer-1", "reviewer", max_rounds, "0");
}

struct verdict_input {
  std::string                request_id = "verdict-1";
  std::string                decision   = "approve";
  std::vector<std::string>   repair;
  std::optional<std::string> output;  ///< Document target output; default the current one.
  std::optional<std::string> work_fp; ///< Document target work_fp; default the current one.
  std::optional<std::string> policy;  ///< Document policy; default the current policy.
  std::vector<std::uint64_t> cleared;
  std::string                verdict_path = "/private/verdict.json";
};

/// The current target vector as Lua derives it (R18-1 item 3): the latest lineage completion of each target owner.
std::vector<hist::verdict_target> current_vector(const execution_snapshot& snapshot) {
  const auto* reviewer = find_role(snapshot, "alpha", "reviewer");
  REQUIRE(reviewer != nullptr);
  std::vector<hist::verdict_target> out;
  for (const auto& work : reviewer->structure.targets) {
    const execution_completion* latest = nullptr;
    for (const auto& completion : snapshot.completions)
      if (completion.role == "builder")
        latest = &completion;
    if (!latest)
      continue;
    for (const auto& unit : latest->units)
      if (unit.work == work)
        out.push_back(hist::verdict_target{work, unit.output, "xxh3_128:" + latest->work_fp});
  }
  return out;
}

value_ptr document_value(const verdict_input& in, const std::vector<hist::verdict_target>& targets, const std::string& policy) {
  std::vector<value_ptr> items;
  for (const auto& t : targets)
    items.push_back(vrec({{"work", vstr(t.work)}, {"output", vstr(t.output)}, {"work_fp", vstr(t.work_fp)}}));
  std::vector<value_ptr> findings;
  if (in.decision == "reject")
    findings.push_back(vrec({{"id", vstr("F-1")},
                             {"severity", vstr("must")},
                             {"work", vstr("unit_one")},
                             {"path", vstr("src/a.cpp")},
                             {"line", vint(3)},
                             {"text", vstr("Fix it.\nThen rerun.")}}));
  return vrec({{"version", vint(1)},
               {"targets", vlist(items)},
               {"policy", vstr(policy)},
               {"decision", vstr(in.decision)},
               {"findings", vlist(findings)},
               {"repair", vstrs(in.repair)},
               {"summary", vstr("Reviewed.")}});
}

/// A complete verdict change derived from the snapshot exactly as review.lua would build it (R18 execution_change_v1).
execution_change verdict_change(const execution_snapshot& snapshot, const verdict_input& in) {
  const auto* reviewer = find_role(snapshot, "alpha", "reviewer");
  REQUIRE(reviewer != nullptr);
  REQUIRE(reviewer->run_id.has_value());
  REQUIRE(reviewer->dispatch.has_value());
  execution_verdict_facts facts;
  facts.role         = "reviewer";
  facts.run          = *reviewer->run_id;
  facts.current.work = reviewer->dispatch->work;
  auto policy        = hist::work_exec_digest(facts.current.work);
  REQUIRE(policy.has_value());
  facts.current.policy  = *policy;
  facts.current.targets = current_vector(snapshot);
  const auto shape      = facts.current.work->fields.at("shape");
  facts.max_rounds      = static_cast<std::uint64_t>(shape->fields.at("max_rounds")->integer);
  auto targets          = facts.current.targets;
  for (auto& target : targets) {
    target.output  = in.output.value_or(target.output);
    target.work_fp = in.work_fp.value_or(target.work_fp);
  }
  facts.document = document_value(in, targets, in.policy.value_or(facts.current.policy));
  auto typed     = hist::verdict_from_value(facts.document);
  REQUIRE(typed.has_value());
  typed->authority = std::string(hist::verdict_authority_reviewer);
  facts.verdict    = *typed;

  const bool complete = facts.current.targets.size() == reviewer->structure.targets.size();
  bool       same     = targets.size() == facts.current.targets.size();
  for (std::size_t i = 0; same && i < targets.size(); ++i)
    same = targets[i].work == facts.current.targets[i].work && targets[i].output == facts.current.targets[i].output &&
           targets[i].work_fp == facts.current.targets[i].work_fp;
  const bool current = complete && same && facts.verdict.policy == facts.current.policy;
  if (!current)
    facts.admission = "historical";
  else if (in.decision == "approve")
    facts.admission = "approve";
  else if (in.repair.empty())
    facts.admission = "unowned";
  else
    facts.admission = snapshot.review.count < facts.max_rounds ? "admitted" : "capped";
  if (facts.admission == "admitted")
    facts.repairs = in.repair;
  const std::string kind = facts.admission == "unowned" ? "unowned_rejection" : facts.admission == "capped" ? "max_rounds" : "";
  if (!kind.empty() && std::ranges::none_of(snapshot.review.blockers, [&](const execution_blocker& b) {
        return b.kind == kind && std::ranges::find(in.cleared, b.id) == in.cleared.end();
      }))
    facts.blockers_opened.push_back(kind);

  execution_change change;
  change.operation  = "verdict";
  change.request_id = in.request_id;
  change.actor      = "tester";
  change.request =
      hist::request_v3{.operation        = "verdict",
                       .actor            = "tester",
                       .parameters       = params_bytes({{"run_id", decimal(facts.run)}, {"verdict_path", in.verdict_path}}),
                       .profile_argument = std::nullopt};
  change.job    = "alpha";
  change.role   = "reviewer";
  bool all_done = true;
  for (const auto& row : snapshot.roles)
    if (row.job == "alpha" && row.structure.kind == "worker" && row.state != "done")
      all_done = false;
  const std::string after = facts.admission == "admitted"     ? "blocked"
                            : facts.admission == "historical" ? (all_done ? "stale" : "blocked")
                                                              : "done";
  change.transitions.push_back(step_to(*reviewer, after, "completed"));
  for (const auto& role : facts.repairs) {
    const auto* row = find_role(snapshot, "alpha", role);
    REQUIRE(row != nullptr);
    change.transitions.push_back(step_to(*row, row->state == "done" ? "stale" : "blocked", "repair_admitted"));
  }
  change.vector = base_vector(snapshot, "alpha");
  for (const auto& t : change.transitions)
    for (auto& entry : change.vector)
      if (entry.role == t.role)
        entry =
            execution_vector_entry{.job = t.job, .role = t.role, .state = t.after_state, .drifted = false, .run = std::nullopt};
  change.expected = {revision_member(snapshot), active_job_member(snapshot)};
  for (const auto& row : snapshot.roles)
    if (row.job == "alpha")
      change.expected.push_back(fact_member(row));
  change.expected.push_back(state_member(*reviewer));
  change.expected.push_back(generation_member(*reviewer));
  auto active   = member(kind_t::run_active);
  active.job    = "alpha";
  active.role   = "reviewer";
  active.run    = facts.run;
  active.number = reviewer->generation_id;
  change.expected.push_back(active);
  change.expected.push_back(member(kind_t::request_unused));
  change.blockers_cleared = in.cleared;
  change.verdict          = std::move(facts);
  return change;
}

execution_commit_result commit_verdict(const std::filesystem::path& path, const verdict_input& in) {
  const auto snapshot = snapshot_of(path);
  auto       result   = commit_execution(path, snapshot, verdict_change(snapshot, in));
  INFO("verdict failed with error code " << (result.has_value() ? -1 : static_cast<int>(result.error().code)));
  REQUIRE(result.has_value());
  return *result;
}

hist::verdict_result_v1 saved_of(const execution_commit_result& result) {
  auto decoded = hist::decode_verdict_result_v1(result.saved_result);
  REQUIRE(decoded.has_value());
  return *decoded;
}

verdict_input reject(std::string request_id, std::vector<std::string> repair = {"builder"}) {
  verdict_input in;
  in.request_id = std::move(request_id);
  in.decision   = "reject";
  in.repair     = std::move(repair);
  return in;
}

std::string state_of(const execution_snapshot& snapshot, std::string_view role) {
  const auto* row = find_role(snapshot, "alpha", role);
  REQUIRE(row != nullptr);
  return row->state;
}

std::int64_t count_rows(const std::filesystem::path& path, std::string_view sql) {
  auto db = open_raw(path);
  return int_of(db, sql);
}

/// One repair round after an admitted rejection: the builder is claimed with the new revision, completes with the
/// output, and the reviewer (blocked -> ready) is claimed again.
void repair_round(const std::filesystem::path& path, std::string_view tag, std::string_view repair, std::string_view output) {
  claim_with(path, std::format("claim-builder-{}", tag), "builder", 2, repair);
  complete_builder(path, std::format("complete-{}", tag), output);
  claim_with(path, std::format("claim-reviewer-{}", tag), "reviewer", 2, "0");
}

void require_code(const std::expected<execution_commit_result, execution_error>& result, execution_error_code code) {
  INFO("got " << (result.has_value() ? -1 : static_cast<int>(result.error().code)) << " want " << static_cast<int>(code));
  REQUIRE_FALSE(result.has_value());
  REQUIRE(result.error().code == code);
}

} // namespace

TEST_CASE("R18-B verdict approve of the current vector completes the reviewer", "[r18][verdict_commit]") {
  scratch    dir;
  const auto path   = dir.ledger();
  const auto token  = reviewing_ledger(path);
  const auto before = snapshot_of(path);
  const auto result = commit_verdict(path, verdict_input{});
  require_verifies(path);
  const auto saved = saved_of(result);
  CHECK(saved.applicability == "current");
  CHECK(saved.decision == "approve");
  CHECK_FALSE(saved.admitted);
  CHECK(saved.repairs.empty());
  CHECK(saved.count == "0");
  CHECK(saved.max_rounds == "2");
  CHECK(saved.opened.empty());
  REQUIRE(saved.roles.size() == 1);
  CHECK(saved.roles[0].role == "reviewer");
  CHECK(saved.roles[0].state == "done"); // M37
  CHECK(saved.event == decimal(result.event_seq));

  const auto after = snapshot_of(path);
  CHECK(state_of(after, "reviewer") == "done");
  CHECK(find_role(after, "alpha", "reviewer")->fact_version == find_role(before, "alpha", "reviewer")->fact_version + 1);
  CHECK(find_role(after, "alpha", "builder")->fact_version == find_role(before, "alpha", "builder")->fact_version);
  REQUIRE(after.review.verdicts.size() == 1);
  CHECK(after.review.verdicts[0].applicability == "current");
  CHECK(after.review.count == 0);
  CHECK(after.review.repairs.empty());
  CHECK(after.review.blockers.empty());
  CHECK(count_rows(path, "SELECT COUNT(*) FROM run_state WHERE state='completed'") == 1);
  // The completed reviewer run no longer authorizes its token (fenced with run_state completed).
  auto authorized = authorize_run_token(path, token);
  REQUIRE_FALSE(authorized.has_value());
  CHECK(authorized.error().code == execution_error_code::fenced);
  CHECK(authorized.error().state == "completed");
}

TEST_CASE("R18-B verdict admitted rejection writes every effect in one commit", "[r18][verdict_commit]") {
  scratch    dir;
  const auto path = dir.ledger();
  reviewing_ledger(path);
  const auto events = count_rows(path, "SELECT COUNT(*) FROM events");
  const auto result = commit_verdict(path, reject("verdict-1"));
  require_verifies(path);
  CHECK(count_rows(path, "SELECT COUNT(*) FROM events") == events + 1);
  const auto saved = saved_of(result);
  CHECK(saved.admitted);
  CHECK(saved.count == "1");
  REQUIRE(saved.repairs.size() == 1);
  CHECK(saved.repairs[0].role == "builder");
  CHECK(saved.repairs[0].generation == "1");
  CHECK(saved.repairs[0].revision == "1");
  REQUIRE(saved.roles.size() == 2);
  CHECK(saved.roles[0].role == "builder");
  CHECK(saved.roles[0].state == "stale"); // M39
  CHECK(saved.roles[1].role == "reviewer");
  CHECK(saved.roles[1].state == "blocked"); // M42

  const auto after = snapshot_of(path);
  CHECK(state_of(after, "builder") == "stale");
  CHECK(state_of(after, "reviewer") == "blocked");
  CHECK(after.review.count == 1);
  REQUIRE(after.review.repairs.size() == 1);
  CHECK(after.review.repairs[0].revision == 1);
  // All rows of the one commit share its sequence.
  const auto seq = static_cast<std::int64_t>(result.event_seq);
  CHECK(count_rows(path, std::format("SELECT COUNT(*) FROM transitions WHERE event_seq={}", seq)) == 2);
  CHECK(count_rows(path, std::format("SELECT COUNT(*) FROM verdicts WHERE seq={}", seq)) == 1);
  CHECK(count_rows(path, "SELECT COUNT(*) FROM repairs") == 1);
  CHECK(count_rows(path, std::format("SELECT COUNT(*) FROM requests WHERE event_seq={} AND outcome='recorded'", seq)) == 1);
  CHECK(count_rows(path,
                   "SELECT reason='repair_admitted' FROM transitions WHERE role='builder' ORDER BY event_seq DESC LIMIT 1") == 1);
}

TEST_CASE("R18-B verdict admitted rejection keeps a blocked worker blocked (M44)", "[r18][verdict_commit]") {
  scratch    dir;
  const auto path = dir.ledger();
  reviewing_ledger(path);
  // The worker loses a prerequisite after completing: done -> blocked by a load (M17 to M22 shape).
  const auto snapshot = snapshot_of(path);
  commit_or_fail(path, snapshot,
                 load_with(snapshot, "unavailable", seed_revision(path, "unavailable"),
                           {step_to(*find_role(snapshot, "alpha", "builder"), "blocked", "prerequisite_unavailable")}));
  const auto result = commit_verdict(path, reject("verdict-1"));
  require_verifies(path);
  const auto saved = saved_of(result);
  CHECK(saved.admitted);
  REQUIRE(saved.roles.size() == 2);
  CHECK(saved.roles[0].state == "blocked");
  CHECK(state_of(snapshot_of(path), "builder") == "blocked");
}

TEST_CASE("R18-B verdict on a stale vector or policy is historical (A06)", "[r18][verdict_commit]") {
  auto run = [](const std::function<void(verdict_input&)>& mutate) {
    scratch    dir;
    const auto path = dir.ledger();
    reviewing_ledger(path);
    auto in = reject("verdict-1");
    mutate(in);
    const auto result = commit_verdict(path, in);
    require_verifies(path);
    const auto saved = saved_of(result);
    CHECK(saved.applicability == "historical");
    CHECK_FALSE(saved.admitted);
    CHECK(saved.repairs.empty());
    CHECK(saved.count == "0");
    CHECK(saved.opened.empty());
    REQUIRE(saved.roles.size() == 1);
    CHECK(saved.roles[0].state == "stale"); // M43
    const auto after = snapshot_of(path);
    CHECK(state_of(after, "builder") == "done");
    CHECK(after.review.count == 0);
    CHECK(after.review.blockers.empty());
    CHECK(count_rows(path, "SELECT COUNT(*) FROM repairs") == 0);
  };
  SECTION("stale output") {
    run([](verdict_input& in) { in.output = std::string(output_two); });
  }
  SECTION("old work fingerprint") {
    run([](verdict_input& in) { in.work_fp = std::string(fingerprint_one); });
  }
  SECTION("wrong policy") {
    run([](verdict_input& in) { in.policy = std::string(fingerprint_two); });
  }
}

TEST_CASE("R18-B verdict third owned rejection is capped and a raising load clears it (A14)", "[r18][verdict_commit]") {
  scratch    dir;
  const auto path = dir.ledger();
  reviewing_ledger(path);
  CHECK(saved_of(commit_verdict(path, reject("verdict-1"))).count == "1");
  repair_round(path, "2", "1", output_two);
  CHECK(saved_of(commit_verdict(path, reject("verdict-2"))).count == "2");
  CHECK(snapshot_of(path).review.repairs[0].revision == 2);
  repair_round(path, "3", "2", output_three);
  const auto capped = saved_of(commit_verdict(path, reject("verdict-3")));
  require_verifies(path);
  CHECK_FALSE(capped.admitted);
  CHECK(capped.repairs.empty());
  CHECK(capped.count == "2");
  REQUIRE(capped.opened.size() == 1);
  CHECK(capped.opened[0].kind == "max_rounds");
  REQUIRE(capped.roles.size() == 1);
  CHECK(capped.roles[0].state == "done"); // M37: not admitted
  auto snapshot = snapshot_of(path);
  CHECK(state_of(snapshot, "builder") == "done");
  REQUIRE(snapshot.review.blockers.size() == 1);
  const auto blocker = snapshot.review.blockers[0].id;

  // A contract-only load (no blockers cleared) does not reset the count.
  commit_or_fail(path, snapshot, load_with(snapshot, "load-contract", seed_revision(path, "contract"), {}));
  snapshot = snapshot_of(path);
  CHECK(snapshot.review.count == 2);
  CHECK(snapshot.review.blockers.size() == 1);

  // The load that raises max_rounds clears the blocker (DB-R7: the raise itself is Lua's judgement).
  commit_or_fail(path, snapshot, load_with(snapshot, "load-raise", seed_revision(path, "raise"), {}, {blocker}));
  snapshot = snapshot_of(path);
  CHECK(snapshot.review.blockers.empty());
  CHECK(snapshot.review.count == 2);
  CHECK(count_rows(path, std::format("SELECT cleared_seq=last_seq AND state='cleared' FROM blocker_state WHERE blocker_id={}",
                                     blocker)) == 1);
  require_verifies(path);

  // Naming the cleared blocker again is refused under the lock.
  require_code(commit_execution(path, snapshot, load_with(snapshot, "load-again", seed_revision(path, "again"), {}, {blocker})),
               execution_error_code::state_conflict);
}

TEST_CASE("R18-B verdict activate clears every open blocker and resets the count", "[r18][verdict_commit]") {
  scratch    dir;
  const auto path = dir.ledger();
  reviewing_ledger(path);
  CHECK(saved_of(commit_verdict(path, reject("verdict-1", {}))).opened.size() == 1); // unowned_rejection
  auto snapshot = snapshot_of(path);
  REQUIRE(snapshot.review.blockers.size() == 1);
  CHECK(snapshot.review.blockers[0].kind == "unowned_rejection");
  CHECK(snapshot.review.count == 0);

  auto reactivate = [&](const execution_snapshot& s, std::vector<std::uint64_t> cleared) {
    execution_change change;
    change.operation  = "activate";
    change.request_id = "reactivate-1";
    change.actor      = "tester";
    change.job        = "alpha";
    change.request    = hist::request_v3{
        .operation        = "activate",
        .actor            = "tester",
        .parameters       = params_bytes({{"expect_job", "alpha"}, {"expect_revision", head_text(s)}, {"job", "alpha"}}),
        .profile_argument = std::nullopt};
    for (const auto& [role, kind] :
         std::vector<std::pair<std::string, std::string>>{{"builder", "worker"}, {"reviewer", "reviewer"}}) {
      change.generations.push_back(execution_generation_input{
          .job = "alpha", .role = role, .reason = "activated", .structure = structure_of("alpha", role, kind)});
      change.transitions.push_back(step_to(*find_role(s, "alpha", role), "ready", "activated"));
      change.vector.push_back(
          execution_vector_entry{.job = "alpha", .role = role, .state = "ready", .drifted = false, .run = std::nullopt});
    }
    change.expected         = lifecycle_expected(s, {"alpha"});
    change.blockers_cleared = std::move(cleared);
    return change;
  };
  // An activation that does not clear the open blocker is an invalid change.
  require_code(commit_execution(path, snapshot, reactivate(snapshot, {})), execution_error_code::invalid_change);
  commit_or_fail(path, snapshot, reactivate(snapshot, {snapshot.review.blockers[0].id}));
  require_verifies(path);
  snapshot = snapshot_of(path);
  CHECK(snapshot.review.blockers.empty());
  CHECK(snapshot.review.count == 0);
}

TEST_CASE("R18-B verdict count is per activation after a re-activation", "[r18][verdict_commit]") {
  scratch    dir;
  const auto path = dir.ledger();
  reviewing_ledger(path);
  commit_verdict(path, reject("verdict-1"));
  CHECK(snapshot_of(path).review.count == 1);
  // Re-activate alpha (builder stale, reviewer blocked -> ready), then a full round admits with count 1 again.
  const auto       s = snapshot_of(path);
  execution_change change;
  change.operation  = "activate";
  change.request_id = "reactivate-1";
  change.actor      = "tester";
  change.job        = "alpha";
  change.request =
      hist::request_v3{.operation  = "activate",
                       .actor      = "tester",
                       .parameters = params_bytes({{"expect_job", "alpha"}, {"expect_revision", head_text(s)}, {"job", "alpha"}}),
                       .profile_argument = std::nullopt};
  for (const auto& [role, kind] :
       std::vector<std::pair<std::string, std::string>>{{"builder", "worker"}, {"reviewer", "reviewer"}}) {
    change.generations.push_back(execution_generation_input{
        .job = "alpha", .role = role, .reason = "activated", .structure = structure_of("alpha", role, kind)});
    change.transitions.push_back(step_to(*find_role(s, "alpha", role), "ready", "activated"));
    change.vector.push_back(
        execution_vector_entry{.job = "alpha", .role = role, .state = "ready", .drifted = false, .run = std::nullopt});
  }
  change.expected = lifecycle_expected(s, {"alpha"});
  commit_or_fail(path, s, change);
  auto after = snapshot_of(path);
  CHECK(after.review.count == 0);
  CHECK(after.review.repairs.empty()); // a new activated generation starts a new lineage
  claim_with(path, "claim-builder-2", "builder", 2, "0");
  complete_builder(path, "complete-2", output_two);
  claim_with(path, "claim-reviewer-2", "reviewer", 2, "0");
  const auto saved = saved_of(commit_verdict(path, reject("verdict-2")));
  CHECK(saved.admitted);
  CHECK(saved.count == "1");
  CHECK(saved.repairs[0].revision == "1");
  require_verifies(path);
}

TEST_CASE("R18-B verdict empty repair opens unowned_rejection once", "[r18][verdict_commit]") {
  scratch    dir;
  const auto path = dir.ledger();
  reviewing_ledger(path);
  const auto saved = saved_of(commit_verdict(path, reject("verdict-1", {})));
  require_verifies(path);
  CHECK_FALSE(saved.admitted);
  REQUIRE(saved.opened.size() == 1);
  CHECK(saved.opened[0].kind == "unowned_rejection");
  REQUIRE(saved.roles.size() == 1);
  CHECK(saved.roles[0].state == "done");
  CHECK(snapshot_of(path).review.count == 0);
}

TEST_CASE("R18-B verdict same request id replays the saved bytes and an unequal body conflicts", "[r18][verdict_commit]") {
  scratch    dir;
  const auto path = dir.ledger();
  reviewing_ledger(path);
  const auto snapshot = snapshot_of(path);
  const auto change   = verdict_change(snapshot, reject("verdict-1"));
  auto       first    = commit_execution(path, snapshot, change);
  REQUIRE(first.has_value());
  const auto events = count_rows(path, "SELECT COUNT(*) FROM events");
  auto       again  = commit_execution(path, snapshot, change);
  REQUIRE(again.has_value());
  CHECK(again->replayed);
  CHECK(again->saved_result == first->saved_result);
  CHECK(count_rows(path, "SELECT COUNT(*) FROM events") == events);
  CHECK(count_rows(path, "SELECT MAX(revision) FROM repairs") == 1);
  // A later snapshot replays too (the request is answered before the snapshot checks).
  auto later = commit_execution(path, snapshot_of(path), change);
  REQUIRE(later.has_value());
  CHECK(later->saved_result == first->saved_result);
  auto in         = reject("verdict-1");
  in.verdict_path = "/private/other.json";
  require_code(commit_execution(path, snapshot, verdict_change(snapshot, in)), execution_error_code::request_conflict);
}

TEST_CASE("R18-B verdict of a taken-over reviewer run is fenced", "[r18][verdict_commit]") {
  scratch    dir;
  const auto path     = dir.ledger();
  const auto token    = reviewing_ledger(path);
  const auto snapshot = snapshot_of(path);
  const auto change   = verdict_change(snapshot, verdict_input{});
  takeover_role(path, "takeover-reviewer", "reviewer");
  const auto events = count_rows(path, "SELECT COUNT(*) FROM events");
  auto       result = commit_execution(path, snapshot, change);
  require_code(result, execution_error_code::fenced);
  CHECK(result.error().state == "fenced");
  CHECK(count_rows(path, "SELECT COUNT(*) FROM events") == events);
  auto authorized = authorize_run_token(path, token);
  REQUIRE_FALSE(authorized.has_value());
  CHECK(authorized.error().code == execution_error_code::fenced);
}

TEST_CASE("R18-B verdict every step-5 mismatch is an invalid change and writes nothing", "[r18][verdict_commit]") {
  scratch    dir;
  const auto path = dir.ledger();
  reviewing_ledger(path);
  const auto snapshot       = snapshot_of(path);
  const auto events         = count_rows(path, "SELECT COUNT(*) FROM events");
  auto       expect_invalid = [&](const std::string& name, const std::function<void(execution_change&)>& mutate) {
    INFO(name);
    auto change = verdict_change(snapshot, reject("verdict-1"));
    mutate(change);
    require_code(commit_execution(path, snapshot, change), execution_error_code::invalid_change);
    CHECK(count_rows(path, "SELECT COUNT(*) FROM events") == events);
  };
  expect_invalid("verdict differs from the document normalization",
                 [](execution_change& c) { c.verdict->verdict.summary = "Other."; });
  expect_invalid("document authority external", [](execution_change& c) {
    c.verdict->document = with_field(c.verdict->document, "authority", vstr("external"));
  });
  expect_invalid("current W differs from the dispatch W", [](execution_change& c) {
    c.verdict->current.work = with_field(c.verdict->current.work, "goal", vstr("other"));
  });
  expect_invalid("policy is not the W digest",
                 [](execution_change& c) { c.verdict->current.policy = std::string(fingerprint_two); });
  expect_invalid("current target names no completion",
                 [](execution_change& c) { c.verdict->current.targets[0].output = std::string(output_three); });
  expect_invalid("max_rounds differs from the W shape", [](execution_change& c) { c.verdict->max_rounds = 3; });
  expect_invalid("admission case differs", [](execution_change& c) { c.verdict->admission = "capped"; });
  expect_invalid("repairs differ", [](execution_change& c) { c.verdict->repairs.clear(); });
  expect_invalid("blockers opened differ", [](execution_change& c) { c.verdict->blockers_opened = {"max_rounds"}; });
  expect_invalid("reviewer after state differs", [](execution_change& c) { c.transitions[0].after_state = "done"; });
  expect_invalid("repaired worker missing", [](execution_change& c) { c.transitions.pop_back(); });
  expect_invalid("expected set lacks run_active", [](execution_change& c) {
    std::erase_if(c.expected, [](const execution_expectation& m) { return m.kind == kind_t::run_active; });
  });
  expect_invalid("vector differs", [](execution_change& c) { c.vector[0].state = "ready"; });
  expect_invalid("run_id parameter differs", [](execution_change& c) {
    c.request = hist::request_v3{.operation        = "verdict",
                                 .actor            = "tester",
                                 .parameters       = params_bytes({{"run_id", "99"}, {"verdict_path", "/private/verdict.json"}}),
                                 .profile_argument = std::nullopt};
  });
  expect_invalid("unknown blocker cleared", [](execution_change& c) { c.blockers_cleared = {7}; });
  // A repair role that owns no target (the reviewer) is native invalid_change (DB-R8).
  {
    auto in  = reject("verdict-1", {"reviewer"});
    auto doc = verdict_change(snapshot, in);
    require_code(commit_execution(path, snapshot, doc), execution_error_code::invalid_change);
  }
  require_verifies(path);
}

TEST_CASE("R18-B verdict repair revision survives a takeover in the lineage", "[r18][verdict_commit]") {
  scratch    dir;
  const auto path = dir.ledger();
  reviewing_ledger(path);
  commit_verdict(path, reject("verdict-1"));
  claim_with(path, "claim-builder-2", "builder", 2, "1");
  takeover_role(path, "takeover-builder", "builder");
  auto snapshot = snapshot_of(path);
  REQUIRE(snapshot.review.repairs.size() == 1);
  CHECK(snapshot.review.repairs[0].revision == 1);
  CHECK(find_role(snapshot, "alpha", "builder")->generation_ordinal == 2);
  complete_builder(path, "complete-2", output_two);
  claim_with(path, "claim-reviewer-2", "reviewer", 2, "0");
  const auto saved = saved_of(commit_verdict(path, reject("verdict-2")));
  REQUIRE(saved.repairs.size() == 1);
  CHECK(saved.repairs[0].generation == "2");
  CHECK(saved.repairs[0].revision == "2");
  require_verifies(path);
}

TEST_CASE("R18-B history expansion of verdict_recorded and the saved response", "[r18][verdict_commit]") {
  scratch    dir;
  const auto path = dir.ledger();
  reviewing_ledger(path);
  const auto result   = commit_verdict(path, reject("verdict-1"));
  auto       expanded = read_verdict_expansion(path, result.event_seq);
  REQUIRE(expanded.has_value());
  REQUIRE(expanded->has_value());
  const auto& execution = **expanded;
  const auto& verdict   = execution->fields.at("verdict");
  CHECK(verdict->fields.at("request_id")->string == "verdict-1");
  CHECK(verdict->fields.at("applicability")->string == "current");
  CHECK(verdict->fields.at("decision")->string == "reject");
  CHECK(verdict->fields.at("body")->fields.at("authority")->string == "reviewer_role_v1");
  const auto& repairs = execution->fields.at("repairs")->items;
  REQUIRE(repairs.size() == 1);
  CHECK(repairs[0]->fields.at("role")->string == "builder");
  CHECK(repairs[0]->fields.at("generation")->string == "1");
  CHECK(repairs[0]->fields.at("revision")->string == "1");
  CHECK(execution->fields.at("blockers")->fields.at("opened")->items.empty());
  CHECK(execution->fields.at("blockers")->fields.at("cleared")->items.empty());
  const auto& native = execution->fields.at("provenance")->fields.at("native");
  CHECK(native->fields.at("role")->string == "reviewer");
  CHECK(native->fields.at("job")->string == "alpha");
  CHECK(native->fields.at("work_fp")->string.starts_with("xxh3_128:"));
  CHECK(native->fields.at("source")->fields.at("digest")->string.starts_with("xxh3_128:"));
  // The canonical bytes of the expansion carry no token.
  auto bytes = canonical_encode(execution);
  REQUIRE(bytes.has_value());
  CHECK(bytes->find("secret") == std::string::npos);

  auto owner = open_existing(path, ledger_access::read_only);
  REQUIRE(owner);
  hist::history_query query;
  query.selector = hist::history_selector::event;
  query.id       = result.event_seq;
  auto projected = hist::read_history(*owner, query);
  REQUIRE(projected);
  const auto& projection = (*projected)->fields.at("data")->fields.at("execution");
  CHECK(projection->fields.at("verdict")->fields.at("decision")->string == "reject");
  CHECK(projection->fields.at("repairs")->items.size() == 1);
  query.selector   = hist::history_selector::request;
  query.request_id = "verdict-1";
  REQUIRE(hist::read_history(*owner, query));
  auto recovery = read_recovery_records(path, "alpha", "builder");
  REQUIRE(recovery);
  CHECK(recovery->findings.size() == verdict->fields.at("body")->fields.at("findings")->items.size());
  auto saved = read_saved_result(path, "verdict-1");
  REQUIRE(saved.has_value());
  REQUIRE(saved->has_value());
  CHECK(**saved == result.saved_result);
  auto none = read_saved_result(path, "no-such-request");
  REQUIRE(none.has_value());
  CHECK_FALSE(none->has_value());
  // A claim event (seq 4: initialized, load, activate, claim) has no verdict expansion.
  auto claim = read_verdict_expansion(path, 4);
  REQUIRE(claim.has_value());
  CHECK_FALSE(claim->has_value());
}
