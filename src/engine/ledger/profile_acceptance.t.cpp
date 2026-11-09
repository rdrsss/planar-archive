// Atomic completion of a real prepared_profile into native v2 ledgers: derived values, replay, CAS, receipts and phases.
#include <catch2/catch_test_macros.hpp>
#include <sqlite3.h>
#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.engine.runtime;
import planar.engine.ledger.acceptance;
import planar.engine.ledger.format;
import planar.engine.acceptance.intent;
import planar.engine.history.tree;
import planar.engine.history.read;
using namespace planar::engine;
using namespace planar::engine::ledger;
namespace {
std::string read_file_bytes(const std::filesystem::path& p) {
  std::ifstream in(p, std::ios::binary);
  return std::string{std::istreambuf_iterator<char>(in), {}};
}
void put(const std::filesystem::path& p, std::string_view bytes) {
  std::ofstream f(p, std::ios::binary | std::ios::trunc);
  f << bytes;
  REQUIRE(f.good());
}
void replace_in_file(const std::filesystem::path& p, std::string_view from, std::string_view to) {
  auto bytes = read_file_bytes(p);
  auto at    = bytes.find(from);
  REQUIRE(at != std::string::npos);
  bytes.replace(at, from.size(), to);
  put(p, bytes);
}
void append_to(const std::filesystem::path& p, std::string_view extra) {
  put(p, read_file_bytes(p) + std::string(extra));
}
void replace_leaf(const std::filesystem::path& target) {
  auto fresh = target;
  fresh += ".fresh";
  put(fresh, read_file_bytes(target));
  std::filesystem::rename(fresh, target);
}
/// Real selected profile with one registered custom shape, a copied quota workspace and registry, and a native v2 ledger.
struct profile_fixture {
  std::filesystem::path root, workspace_dir, declaration, registry, ledger_path, profile, module;
  explicit profile_fixture(int format = 2, std::string_view workspace_id = "quota", bool keep_builtin_split = false) {
    char pattern[] = "/tmp/planar-x06c-XXXXXX";
    REQUIRE(::mkdtemp(pattern));
    root          = std::filesystem::canonical(pattern);
    workspace_dir = root / "workspace";
    declaration   = workspace_dir / "workspace.lua";
    registry      = root / "registry.json";
    ledger_path   = root / "ledger";
    profile       = root / "trusted/profile.json";
    module        = root / "trusted/module.lua";
    std::filesystem::create_directory(root / "trusted");
    put(profile, R"({"version":1,"modules":["module.lua"]})");
    put(module, "registration.shape({id='custom',abi='shape_v1',revision='final',stages='optional',help='shape'},function(input) "
                "return engine.object{ok=true,value=engine.object{obligations=engine.array{},guidance=input.job.id}} end)");
    const std::filesystem::path fixture(PLANAR_SCHEMA_FIXTURE);
    std::filesystem::copy(fixture / "workspace", workspace_dir, std::filesystem::copy_options::recursive);
    std::filesystem::copy_file(fixture / "registry/quota-tools.json", registry);
    replace_in_file(workspace_dir / "combined.lua", "shape = \"fanout_review\"", "shape = \"custom\"");
    if (!keep_builtin_split)
      replace_in_file(workspace_dir / "split.lua", "shape = \"fanout_review\"", "shape = \"custom\"");
    REQUIRE((format == 2 ? initialize_new_v2(ledger_path, workspace_id) : initialize_new(ledger_path, workspace_id)));
  }
  profile_fixture(const profile_fixture&)            = delete;
  profile_fixture& operator=(const profile_fixture&) = delete;
  ~profile_fixture() {
    std::filesystem::remove_all(root);
  }
  std::filesystem::path shared_text() const {
    return workspace_dir / "text/shared.md";
  }
  acceptance::request_intent intent(std::string_view request_id = "req-1", std::optional<std::int64_t> head = std::nullopt,
                                    std::string workspace_override = {}) const {
    auto r = acceptance::request_intent::create_v2(request_id, "actor", head,
                                                   workspace_override.empty() ? declaration.string() : workspace_override,
                                                   registry.string(), profile.string());
    REQUIRE(r);
    return std::move(*r);
  }
  /// Second ledger of the other format at root/name with the same workspace id as the default fixture.
  std::filesystem::path extra_ledger(int format, std::string_view name) const {
    const auto path = root / name;
    REQUIRE((format == 2 ? initialize_new_v2(path, "quota") : initialize_new(path, "quota")));
    return path;
  }
};
prepared_profile prepare(const profile_fixture& f, std::string_view request_id = "req-1",
                         std::optional<std::int64_t> head = std::nullopt) {
  auto i = f.intent(request_id, head);
  auto o = profile_invocation::open_known_load(i, f.ledger_path.string());
  REQUIRE(o);
  auto r = std::move(*o).prepare_load(i);
  REQUIRE(r);
  return std::move(*r);
}
std::string identity_of(const std::filesystem::path& path) {
  auto file = open_existing(path);
  REQUIRE(file);
  return file->metadata().identity;
}
struct outcome {
  std::expected<acceptance_result, format_error> result;
  completion_receipt                             receipt;
};
outcome run(const std::filesystem::path& ledger_path, const acceptance::request_intent& intent, const prepared_profile& profile) {
  completion_receipt receipt;
  auto               result = complete_acceptance(ledger_path, intent, profile, identity_of(ledger_path), receipt);
  return {std::move(result), std::move(receipt)};
}
outcome run(const profile_fixture& f, const acceptance::request_intent& intent, const prepared_profile& profile) {
  return run(f.ledger_path, intent, profile);
}
std::int64_t count_rows(const std::filesystem::path& ledger_path, std::string_view table) {
  sqlite3* db = nullptr;
  REQUIRE(sqlite3_open_v2(ledger_path.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) == SQLITE_OK);
  sqlite3_stmt* stmt = nullptr;
  const auto    sql  = std::format("SELECT count(*) FROM {}", table);
  REQUIRE(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK);
  REQUIRE(sqlite3_step(stmt) == SQLITE_ROW);
  const auto n = sqlite3_column_int64(stmt, 0);
  sqlite3_finalize(stmt);
  sqlite3_close(db);
  return n;
}
void expect_no_rows(const std::filesystem::path& ledger_path) {
  CHECK(count_rows(ledger_path, "requests") == 0);
  CHECK(count_rows(ledger_path, "revisions") == 0);
  CHECK(count_rows(ledger_path, "events") == 1);
}
void expect_refused(const outcome& o, format_error_code code) {
  REQUIRE_FALSE(o.result);
  CHECK(o.result.error().code == code);
  CHECK(o.receipt.phase == completion_phase::not_completed);
}
void expect_untouched(const outcome& o, format_error_code code, const std::filesystem::path& ledger_path,
                      const std::string& before) {
  expect_refused(o, code);
  CHECK(o.receipt.saved_result.empty());
  CHECK(read_file_bytes(ledger_path) == before);
}
std::string flat(const std::vector<history::tree_file>& files) {
  std::string out;
  for (const auto& f : files) {
    out += std::format("{}\x01{}\x01{}\x01", f.path, f.mode, f.payload.size());
    out += f.payload;
    out += '\x02';
  }
  return out;
}
std::vector<history::tree_file> read_tree_of(const format_file& file, std::int64_t id, history::tree_profile profile,
                                             history::record_digest* digest) {
  auto record = file.read_record(std::uint64_t(id));
  REQUIRE(record);
  *digest   = record->reference.digest;
  auto tree = history::read_tree(file, record->reference, profile);
  REQUIRE(tree);
  return std::move(*tree);
}
std::shared_ptr<const profile_acceptance_data> data_of(const prepared_profile& p) {
  auto d = p.data();
  REQUIRE(d);
  REQUIRE(*d);
  return *d;
}
// SQLite's real per-connection hooks drive concurrent mutations and commit failures without product seams.
enum class when { staging, pre_commit_pragma };
struct trace_probe {
  std::function<void(std::string_view)> each;   ///< Runs at every writer statement start.
  std::function<void()>                 action; ///< Runs once at the selected statement.
  when                                  at          = when::staging;
  bool                                  fail_commit = false; ///< Commit hook refuses once rows are staged.
  bool                                  pending     = false;
  bool                                  fired       = false;
};
trace_probe* active_probe = nullptr;
int          trace_statement(unsigned, void* context, void* raw_statement, void*) {
  auto* db = static_cast<sqlite3*>(context);
  if (!active_probe || sqlite3_db_readonly(db, "main") != 0)
    return 0;
  auto&                  probe = *active_probe;
  const std::string_view sql(sqlite3_sql(static_cast<sqlite3_stmt*>(raw_statement)));
  if (probe.each)
    probe.each(sql);
  const bool staging = sql.starts_with("UPDATE current_state SET");
  if (staging)
    probe.pending = true;
  const bool target = probe.at == when::staging ? staging : (probe.pending && sql.starts_with("PRAGMA application_id"));
  if (probe.action && target && !probe.fired) {
    probe.fired = true;
    probe.action();
  }
  return 0;
}
int refuse_commit(void*) {
  if (active_probe && active_probe->fail_commit && active_probe->pending) {
    active_probe->fired = true;
    return 1;
  }
  return 0;
}
int attach_probe(sqlite3* db, char**, const sqlite3_api_routines*) {
  sqlite3_trace_v2(db, SQLITE_TRACE_STMT, trace_statement, db);
  sqlite3_commit_hook(db, refuse_commit, nullptr);
  return SQLITE_OK;
}
struct installed_probe {
  trace_probe probe;
  installed_probe() {
    REQUIRE(active_probe == nullptr);
    active_probe = &probe;
    REQUIRE(sqlite3_auto_extension(reinterpret_cast<void (*)()>(attach_probe)) == SQLITE_OK);
  }
  ~installed_probe() {
    sqlite3_cancel_auto_extension(reinterpret_cast<void (*)()>(attach_probe));
    active_probe = nullptr;
  }
};
struct raw_reader {
  sqlite3* db = nullptr;
  explicit raw_reader(const std::filesystem::path& path) {
    REQUIRE(sqlite3_open_v2(path.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr) == SQLITE_OK);
  }
  ~raw_reader() {
    sqlite3_close(db);
  }
  void sql(std::string_view text) {
    REQUIRE(sqlite3_exec(db, std::string(text).c_str(), nullptr, nullptr, nullptr) == SQLITE_OK);
  }
};
struct mutation {
  std::string                                 name;
  std::function<void(const profile_fixture&)> apply;
};
const auto no_op_mutation = [](const profile_fixture&) {};
} // namespace
TEST_CASE("X06 C profile completion stores the derived trees and values from profile_acceptance_data",
          "[x06c][profile_acceptance]") {
  profile_fixture f;
  auto            p    = prepare(f);
  auto            data = data_of(p);
  // Every pair must be distinguishable, otherwise a swapped store would be invisible.
  REQUIRE(flat(data->registry.files) != flat(data->effective.files));
  REQUIRE(flat(data->source) != flat(data->registry.files));
  REQUIRE(flat(data->source) != flat(data->effective.files));
  REQUIRE(data->interpretation.encoded != data->interpretation.versions_encoded);
  auto done = run(f, f.intent(), p);
  REQUIRE(done.result);
  auto decoded = decode_load_result(done.result->result);
  REQUIRE(decoded);
  CHECK(decoded->revision == 1);
  auto file = open_existing(f.ledger_path);
  REQUIRE(file);
  auto row = file->read_revision(1);
  REQUIRE(row);
  REQUIRE(*row);
  const auto&            refs = (*row)->references;
  history::record_digest digest{};
  CHECK(flat(read_tree_of(*file, refs.source, history::tree_profile::source, &digest)) == flat(data->source));
  CHECK(digest == data->source_digest);
  CHECK(flat(read_tree_of(*file, refs.registry, history::tree_profile::provenance, &digest)) == flat(data->registry.files));
  CHECK(digest == data->registry.digest);
  CHECK(flat(read_tree_of(*file, refs.stdlib, history::tree_profile::effective_provenance, &digest)) ==
        flat(data->effective.files));
  CHECK(digest == data->effective.digest);
  auto interpretation = file->read_record(std::uint64_t(refs.interpretation));
  REQUIRE(interpretation);
  REQUIRE(std::holds_alternative<history::value_record>(interpretation->value));
  CHECK(std::get<history::value_record>(interpretation->value).encoded == data->interpretation.encoded);
  auto versions = file->read_record(std::uint64_t(refs.versions));
  REQUIRE(versions);
  REQUIRE(std::holds_alternative<history::value_record>(versions->value));
  CHECK(std::get<history::value_record>(versions->value).encoded == data->interpretation.versions_encoded);
}
TEST_CASE("X06 C profile completion stores the exact request_v2 body of the bound intent", "[x06c][profile_acceptance]") {
  profile_fixture f;
  auto            p      = prepare(f);
  auto            intent = f.intent();
  auto            done   = run(f, intent, p);
  REQUIRE(done.result);
  const auto& request = **intent.data();
  REQUIRE(request.version2);
  auto file = open_existing(f.ledger_path);
  REQUIRE(file);
  auto row = file->read_request("req-1");
  REQUIRE(row);
  REQUIRE(*row);
  CHECK((*row)->body_encoded == request.encoded);
  CHECK((*row)->body_reference.kind == history::record_kind::request);
  auto body = file->read_record((*row)->body_reference.id);
  REQUIRE(body);
  REQUIRE(std::holds_alternative<history::request_v2>(body->value));
  const auto& stored = std::get<history::request_v2>(body->value);
  CHECK(stored.profile_argument == request.version2->profile_argument);
  REQUIRE(stored.profile_argument);
  CHECK(*stored.profile_argument == f.profile.string());
  CHECK(stored.workspace_argument == f.declaration.string());
  CHECK(stored.registry_argument == f.registry.string());
  CHECK(stored.actor == "actor");
}
TEST_CASE("X06 C profile completion unchanged second request is noop and replay bytes equal the result",
          "[x06c][profile_acceptance]") {
  profile_fixture f;
  auto            first_profile = prepare(f, "req-1");
  auto            first         = run(f, f.intent("req-1"), first_profile);
  REQUIRE(first.result);
  CHECK(first.receipt.phase == completion_phase::committed);
  CHECK(first.receipt.saved_result == first.result->result);
  auto second_profile = prepare(f, "req-2", 1);
  auto second         = run(f, f.intent("req-2", 1), second_profile);
  REQUIRE(second.result);
  auto decoded = decode_load_result(second.result->result);
  REQUIRE(decoded);
  CHECK(decoded->outcome == load_outcome::noop);
  CHECK(decoded->revision == 1);
  CHECK(count_rows(f.ledger_path, "revisions") == 1);
  CHECK(count_rows(f.ledger_path, "requests") == 2);
  auto replay = run(f, f.intent("req-1"), first_profile);
  REQUIRE(replay.result);
  CHECK(replay.result->replayed);
  CHECK(replay.result->result == first.result->result);
  CHECK(replay.receipt.saved_result == first.result->result);
}
TEST_CASE("X06 C profile completion changed profile module creates a new revision", "[x06c][profile_acceptance]") {
  profile_fixture f;
  auto            first_profile = prepare(f, "req-1");
  REQUIRE(run(f, f.intent("req-1"), first_profile).result);
  append_to(f.module, "\n-- module-only change\n");
  auto second_profile = prepare(f, "req-2", 1);
  auto second         = run(f, f.intent("req-2", 1), second_profile);
  REQUIRE(second.result);
  auto decoded = decode_load_result(second.result->result);
  REQUIRE(decoded);
  CHECK(decoded->outcome == load_outcome::accepted);
  CHECK(decoded->revision == 2);
  CHECK(count_rows(f.ledger_path, "revisions") == 2);
}
TEST_CASE("X06 C profile completion refuses an intent that differs from the bound intent", "[x06c][profile_acceptance]") {
  profile_fixture f;
  auto            p      = prepare(f, "req-1");
  const auto      before = read_file_bytes(f.ledger_path);
  auto            other  = run(f, f.intent("other"), p);
  expect_untouched(other, format_error_code::invalid_argument, f.ledger_path, before);
  CHECK(other.result.error().message == "Prepared profile differs from request intent");
  expect_no_rows(f.ledger_path);
}
TEST_CASE("X06 C profile completion refuses a same id intent whose body differs from the bound intent",
          "[x06c][profile_acceptance]") {
  profile_fixture f;
  auto            p      = prepare(f, "req-1");
  const auto      before = read_file_bytes(f.ledger_path);
  auto            other  = run(f, f.intent("req-1", std::nullopt, "/elsewhere/workspace.lua"), p);
  expect_untouched(other, format_error_code::invalid_argument, f.ledger_path, before);
  CHECK(other.result.error().message == "Prepared profile differs from request intent");
  expect_no_rows(f.ledger_path);
  auto bound = run(f, f.intent("req-1"), p);
  REQUIRE(bound.result);
}
TEST_CASE("X06 C profile completion refuses a workspace id different from the ledger", "[x06c][profile_acceptance]") {
  profile_fixture f(2, "demo");
  auto            p      = prepare(f);
  const auto      before = read_file_bytes(f.ledger_path);
  auto            done   = run(f, f.intent(), p);
  expect_untouched(done, format_error_code::invalid_argument, f.ledger_path, before);
  expect_no_rows(f.ledger_path);
}
TEST_CASE("X06 C profile replay precedes candidate inspection and needs no live inputs or profile",
          "[x06c][profile_acceptance]") {
  profile_fixture f;
  auto            p      = prepare(f);
  auto            intent = f.intent();
  auto            first  = run(f, intent, p);
  REQUIRE(first.result);
  std::filesystem::remove_all(f.workspace_dir);
  std::filesystem::remove_all(f.root / "trusted");
  std::filesystem::remove(f.registry);
  auto sink   = std::move(p);
  auto replay = run(f, intent, p);
  REQUIRE(replay.result);
  CHECK(replay.result->replayed);
  CHECK(replay.result->result == first.result->result);
  CHECK(replay.receipt.phase == completion_phase::replayed);
  CHECK(replay.receipt.saved_result == replay.result->result);
}
TEST_CASE("X06 C profile same id with a different body conflicts before the head check", "[x06c][profile_acceptance]") {
  profile_fixture f;
  auto            p     = prepare(f);
  auto            first = run(f, f.intent(), p);
  REQUIRE(first.result);
  std::filesystem::remove_all(f.workspace_dir);
  std::filesystem::remove_all(f.root / "trusted");
  std::filesystem::remove(f.registry);
  auto       sink   = std::move(p);
  const auto before = read_file_bytes(f.ledger_path);
  auto       other  = run(f, f.intent("req-1", 7, "/elsewhere/workspace.lua"), p);
  expect_untouched(other, format_error_code::request_conflict, f.ledger_path, before);
}
TEST_CASE("X06 C profile format mismatch refuses unsupported_version before the head read", "[x06c][profile_acceptance]") {
  profile_fixture f;
  SECTION("v1 ledger with a profile intent and stale head") {
    auto       p      = prepare(f, "req-1", 99);
    const auto v1     = f.extra_ledger(1, "ledger-v1");
    const auto before = read_file_bytes(v1);
    auto       done   = run(v1, f.intent("req-1", 99), p);
    expect_untouched(done, format_error_code::unsupported_version, v1, before);
  }
  SECTION("v2 ledger with a legacy intent and stale head") {
    auto p      = prepare(f, "req-1");
    auto legacy = acceptance::request_intent::create("legacy", "actor", 99, f.declaration.string(), f.registry.string());
    REQUIRE(legacy);
    const auto before = read_file_bytes(f.ledger_path);
    auto       done   = run(f, *legacy, p);
    expect_untouched(done, format_error_code::unsupported_version, f.ledger_path, before);
  }
}
TEST_CASE("X06 C profile stale head refuses revision_conflict with details and no row", "[x06c][profile_acceptance]") {
  profile_fixture f;
  auto            winner = prepare(f, "req-1");
  auto            stale  = prepare(f, "req-2");
  REQUIRE(run(f, f.intent("req-1"), winner).result);
  const auto before  = read_file_bytes(f.ledger_path);
  auto       refused = run(f, f.intent("req-2"), stale);
  expect_untouched(refused, format_error_code::revision_conflict, f.ledger_path, before);
  REQUIRE(refused.result.error().detail);
  const auto* detail = std::get_if<types::revision_conflict_detail>(&*refused.result.error().detail);
  REQUIRE(detail);
  CHECK_FALSE(detail->expected);
  CHECK(detail->actual == 1);
  CHECK(count_rows(f.ledger_path, "requests") == 1);
  CHECK(count_rows(f.ledger_path, "revisions") == 1);
}
TEST_CASE("X06 C racing profile completions have exactly one winner", "[x06c][profile_acceptance]") {
  profile_fixture                                               f;
  auto                                                          a        = prepare(f, "req-a");
  auto                                                          b        = prepare(f, "req-b");
  auto                                                          ia       = f.intent("req-a");
  auto                                                          ib       = f.intent("req-b");
  const auto                                                    identity = identity_of(f.ledger_path);
  std::optional<std::expected<acceptance_result, format_error>> ra, rb;
  std::barrier                                                  ready(2);
  auto work = [&](const auto& intent, const prepared_profile& profile, auto& slot) {
    ready.arrive_and_wait();
    for (int attempt = 0; attempt < 20; ++attempt) {
      completion_receipt receipt;
      slot = complete_acceptance(f.ledger_path, intent, profile, identity, receipt);
      if (*slot || (slot->error().code != format_error_code::busy && slot->error().code != format_error_code::recovery_required))
        break;
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  };
  std::thread first([&] { work(ia, a, ra); });
  std::thread second([&] { work(ib, b, rb); });
  first.join();
  second.join();
  REQUIRE(ra);
  REQUIRE(rb);
  REQUIRE(bool(*ra) != bool(*rb));
  CHECK((!*ra ? ra->error() : rb->error()).code == format_error_code::revision_conflict);
  CHECK(count_rows(f.ledger_path, "requests") == 1);
  CHECK(count_rows(f.ledger_path, "revisions") == 1);
  auto file = open_existing(f.ledger_path);
  REQUIRE(file);
  auto head = file->read_head();
  REQUIRE(head);
  CHECK(head->revision == 1);
}
TEST_CASE("X06 C profile source receipt changes refuse capture_changed with rollback", "[x06c][profile_acceptance]") {
  const std::vector<mutation> changes{
      {"content edit", [](const profile_fixture& f) { append_to(f.shared_text(), "\nchanged\n"); }},
      {"mode change", [](const profile_fixture& f) { REQUIRE(::chmod(f.shared_text().c_str(), 0755) == 0); }},
      {"fresh inode same bytes", [](const profile_fixture& f) { replace_leaf(f.shared_text()); }}};
  for (const auto& change : changes) {
    DYNAMIC_SECTION(change.name) {
      profile_fixture f;
      auto            p      = prepare(f);
      const auto      before = read_file_bytes(f.ledger_path);
      change.apply(f);
      auto refused = run(f, f.intent(), p);
      expect_untouched(refused, format_error_code::capture_changed, f.ledger_path, before);
      expect_no_rows(f.ledger_path);
      auto fresh = prepare(f);
      auto done  = run(f, f.intent(), fresh);
      REQUIRE(done.result);
      CHECK(done.receipt.phase == completion_phase::committed);
    }
  }
}
TEST_CASE("X06 C profile capture receipt changes refuse capture_changed with rollback", "[x06c][profile_acceptance]") {
  const std::vector<mutation> changes{
      {"profile json edit", [](const profile_fixture& f) { replace_in_file(f.profile, "{\"version\"", " {\"version\""); }},
      {"module content edit", [](const profile_fixture& f) { append_to(f.module, "\n-- changed\n"); }},
      {"module fresh inode same bytes", [](const profile_fixture& f) { replace_leaf(f.module); }}};
  for (const auto& change : changes) {
    DYNAMIC_SECTION(change.name) {
      profile_fixture f;
      auto            p      = prepare(f);
      const auto      before = read_file_bytes(f.ledger_path);
      change.apply(f);
      auto refused = run(f, f.intent(), p);
      expect_untouched(refused, format_error_code::capture_changed, f.ledger_path, before);
      expect_no_rows(f.ledger_path);
    }
  }
}
TEST_CASE("X06 C profile receipt race after staging is caught before commit", "[x06c][profile_acceptance]") {
  const std::vector<mutation> races{{"closure file", [](const profile_fixture& f) { append_to(f.shared_text(), "\nraced\n"); }},
                                    {"profile module", [](const profile_fixture& f) { append_to(f.module, "\n-- raced\n"); }},
                                    {"control without change", no_op_mutation}};
  for (const auto& race : races) {
    DYNAMIC_SECTION(race.name) {
      profile_fixture f;
      auto            p      = prepare(f);
      const auto      before = read_file_bytes(f.ledger_path);
      installed_probe installed;
      installed.probe.at     = when::staging;
      installed.probe.action = [&] { race.apply(f); };
      auto done              = run(f, f.intent(), p);
      REQUIRE(installed.probe.fired);
      if (race.name == "control without change") {
        REQUIRE(done.result);
        CHECK(done.receipt.phase == completion_phase::committed);
      } else {
        expect_untouched(done, format_error_code::capture_changed, f.ledger_path, before);
        CHECK(count_rows(f.ledger_path, "requests") == 0);
      }
    }
  }
}
TEST_CASE("X06 C profile receipt race inside the pre-commit metadata check is caught", "[x06c][profile_acceptance]") {
  profile_fixture f;
  auto            p      = prepare(f);
  const auto      before = read_file_bytes(f.ledger_path);
  installed_probe installed;
  installed.probe.at     = when::pre_commit_pragma;
  installed.probe.action = [&] { append_to(f.shared_text(), "\nraced late\n"); };
  auto done              = run(f, f.intent(), p);
  REQUIRE(installed.probe.fired);
  expect_untouched(done, format_error_code::capture_changed, f.ledger_path, before);
  CHECK(count_rows(f.ledger_path, "requests") == 0);
}
TEST_CASE("X06 C profile receipt refusal surfaces only the ledger capture_changed message", "[x06c][profile_acceptance]") {
  const std::vector<mutation> changes{
      {"source change", [](const profile_fixture& f) { append_to(f.shared_text(), "\nchanged\n"); }},
      {"module change", [](const profile_fixture& f) { append_to(f.module, "\n-- changed\n"); }}};
  for (const auto& change : changes) {
    DYNAMIC_SECTION(change.name) {
      profile_fixture f;
      auto            p = prepare(f);
      change.apply(f);
      auto refused = run(f, f.intent(), p);
      REQUIRE_FALSE(refused.result);
      const auto& error = refused.result.error();
      CHECK(error.code == format_error_code::capture_changed);
      CHECK(error.message == "Capture changed before acceptance commit");
      CHECK(error.message.find("Captured inputs changed.") == std::string::npos);
      CHECK(error.message.find("Captured external profile changed") == std::string::npos);
    }
  }
}
TEST_CASE("X06 C no Lua runs while the profile write transaction is open", "[x06c][profile_acceptance]") {
  // The test samples the runtime's instruction and registration counters at every writer statement and detects Lua-side
  // activity under the ledger write lock; the adapter only calls data() and verify_native_receipts(), which run no Lua, so
  // no ledger-side product mutant reaches the VM; the test is killed by mutant X27, a scratch edit of the runtime's
  // verify_native_receipts that simulates a registration, so it guards the runtime-side contract rather than the adapter.
  struct sample {
    std::size_t instructions, registrations;
    bool        operator==(const sample&) const = default;
  };
  profile_fixture f;
  auto            p    = prepare(f, "req-1");
  auto            q    = prepare(f, "req-2", 1);
  const auto      zero = [](const prepared_profile& pp) {
    const auto o = pp.observe();
    return sample{o.instructions, o.registrations};
  };
  const auto sample_run = [&](const prepared_profile& pp, std::string_view request, std::optional<std::int64_t> head,
                              std::vector<sample>& samples) {
    installed_probe installed;
    installed.probe.each = [&](std::string_view) { samples.push_back(zero(pp)); };
    return run(f, f.intent(request, head), pp);
  };
  const auto          before_p = zero(p);
  std::vector<sample> success_samples;
  auto                done = sample_run(p, "req-1", std::nullopt, success_samples);
  REQUIRE(done.result);
  REQUIRE_FALSE(success_samples.empty());
  for (const auto& s : success_samples)
    CHECK(s == before_p);
  CHECK(zero(p) == before_p);
  std::vector<sample> replay_samples;
  auto                replay = sample_run(p, "req-1", std::nullopt, replay_samples);
  REQUIRE(replay.result);
  REQUIRE_FALSE(replay_samples.empty());
  for (const auto& s : replay_samples)
    CHECK(s == before_p);
  const auto before_q = zero(q);
  append_to(f.shared_text(), "\nchanged\n");
  std::vector<sample> refused_samples;
  auto                refused = sample_run(q, "req-2", 1, refused_samples);
  expect_refused(refused, format_error_code::capture_changed);
  REQUIRE_FALSE(refused_samples.empty());
  for (const auto& s : refused_samples)
    CHECK(s == before_q);
  CHECK(zero(q) == before_q);
}
TEST_CASE("X06 C profile completion receipt phases committed and replayed", "[x06c][profile_acceptance]") {
  profile_fixture f;
  auto            p      = prepare(f);
  auto            intent = f.intent();
  auto            first  = run(f, intent, p);
  REQUIRE(first.result);
  CHECK(first.receipt.phase == completion_phase::committed);
  CHECK_FALSE(first.result->replayed);
  CHECK(first.receipt.saved_result == first.result->result);
  auto second = run(f, intent, p);
  REQUIRE(second.result);
  CHECK(second.receipt.phase == completion_phase::replayed);
  CHECK(second.result->replayed);
  CHECK_FALSE(second.receipt.saved_result.empty());
  CHECK(second.receipt.saved_result == first.result->result);
}
TEST_CASE("X06 C profile completion busy commit keeps the proposed bytes", "[x06c][profile_acceptance]") {
  profile_fixture f;
  auto            p      = prepare(f);
  auto            intent = f.intent();
  const auto      before = read_file_bytes(f.ledger_path);
  outcome         busy{std::unexpected(format_error{format_error_code::corrupt, "unset"}), {}};
  {
    raw_reader reader(f.ledger_path);
    reader.sql("BEGIN");
    reader.sql("SELECT count(*) FROM events");
    busy = run(f, intent, p);
    expect_refused(busy, format_error_code::busy);
    REQUIRE_FALSE(busy.receipt.saved_result.empty());
    CHECK(decode_load_result(busy.receipt.saved_result));
  }
  CHECK(read_file_bytes(f.ledger_path) == before);
  expect_no_rows(f.ledger_path);
  auto retry = run(f, intent, p);
  REQUIRE(retry.result);
  CHECK(retry.receipt.phase == completion_phase::committed);
  CHECK(retry.receipt.saved_result == retry.result->result);
  CHECK(retry.receipt.saved_result == busy.receipt.saved_result);
}
TEST_CASE("X06 C profile completion commit hook failure is commit_uncertain", "[x06c][profile_acceptance]") {
  profile_fixture f;
  auto            p      = prepare(f);
  auto            intent = f.intent();
  const auto      before = read_file_bytes(f.ledger_path);
  {
    installed_probe installed;
    installed.probe.fail_commit = true;
    auto done                   = run(f, intent, p);
    REQUIRE(installed.probe.fired);
    REQUIRE_FALSE(done.result);
    CHECK(done.receipt.phase == completion_phase::commit_uncertain);
    REQUIRE_FALSE(done.receipt.saved_result.empty());
    CHECK(decode_load_result(done.receipt.saved_result));
  }
  CHECK(read_file_bytes(f.ledger_path) == before);
  auto retry = run(f, intent, p);
  REQUIRE(retry.result);
  CHECK(retry.receipt.phase == completion_phase::committed);
}
TEST_CASE("X06 C profile completion in-use receipt refuses before any transaction", "[x06c][profile_acceptance]") {
  profile_fixture f;
  auto            p       = prepare(f);
  auto            intent  = f.intent();
  const auto      missing = f.root / "missing-ledger";
  SECTION("phase already set") {
    completion_receipt receipt;
    receipt.phase = completion_phase::committed;
    auto result   = complete_acceptance(missing, intent, p, "identity", receipt);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == format_error_code::invalid_argument);
    CHECK(result.error().message == "Completion receipt is already in use");
    CHECK(receipt.phase == completion_phase::committed);
    CHECK(receipt.saved_result.empty());
  }
  SECTION("saved result already set") {
    completion_receipt receipt;
    receipt.saved_result = "stale";
    auto result          = complete_acceptance(missing, intent, p, "identity", receipt);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == format_error_code::invalid_argument);
    CHECK(result.error().message == "Completion receipt is already in use");
    CHECK(receipt.phase == completion_phase::not_completed);
    CHECK(receipt.saved_result == "stale");
  }
}
TEST_CASE("X06 C profile completion moved-from profile refuses after CAS before any row", "[x06c][profile_acceptance]") {
  profile_fixture f;
  auto            p       = prepare(f);
  auto            sink    = std::move(p);
  const auto      before  = read_file_bytes(f.ledger_path);
  auto            unknown = run(f, f.intent("req-1"), p);
  expect_untouched(unknown, format_error_code::invalid_argument, f.ledger_path, before);
  CHECK(unknown.result.error().message == "Unavailable prepared acceptance");
  expect_no_rows(f.ledger_path);
  auto stale = run(f, f.intent("req-1", 7), p);
  expect_untouched(stale, format_error_code::revision_conflict, f.ledger_path, before);
}
namespace {
struct data_only_surface {
  std::expected<std::shared_ptr<const profile_acceptance_data>, registration_error> data() const;
  std::expected<void, diagnostic>                                                   verify_native_receipts() const;
};
template <class P>
concept completes =
    requires(const std::filesystem::path& path, const acceptance::request_intent& intent, P&& profile, std::string_view identity,
             completion_receipt& receipt) { complete_acceptance(path, intent, std::forward<P>(profile), identity, receipt); };
} // namespace
TEST_CASE("X06 C profile completion surface accepts only the opaque prepared profile", "[x06c][profile_acceptance]") {
  STATIC_REQUIRE(completes<const prepared_profile&>);
  STATIC_REQUIRE_FALSE(std::is_default_constructible_v<prepared_profile>);
  STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<prepared_profile>);
  STATIC_REQUIRE(std::is_nothrow_move_constructible_v<prepared_profile>);
  // Runtime checks rather than STATIC_REQUIRE: a new overload over the data types then fails this test at run time
  // instead of only breaking the build.
  CHECK_FALSE(completes<profile_acceptance_data>);
  CHECK_FALSE(completes<const profile_acceptance_data&>);
  CHECK_FALSE(completes<std::shared_ptr<const profile_acceptance_data>>);
  CHECK_FALSE(completes<std::shared_ptr<profile_acceptance_data>>);
  CHECK_FALSE(completes<const data_only_surface&>);
}

TEST_CASE("X06 C C01 profile completion stores a built-in job beside a custom job", "[x06c][c01][profile_acceptance]") {
  profile_fixture mixed(2, "quota", true);
  profile_fixture custom;
  auto            p      = prepare(mixed);
  auto            q      = prepare(custom);
  auto            data   = data_of(p);
  auto            others = data_of(q);
  // The mixed fixture must differ from the all-custom one, otherwise a dropped built-in classification would be invisible.
  REQUIRE(data->interpretation.encoded.find("fanout_review") != std::string::npos);
  REQUIRE(data->interpretation.encoded.find("custom") != std::string::npos);
  REQUIRE(data->interpretation.encoded != others->interpretation.encoded);
  // Limit: the effective stdlib tree does not depend on job shapes, so only the interpretation bytes differ.
  auto done = run(mixed, mixed.intent(), p);
  REQUIRE(done.result);
  auto decoded = decode_load_result(done.result->result);
  REQUIRE(decoded);
  CHECK(decoded->revision == 1);
  auto file = open_existing(mixed.ledger_path);
  REQUIRE(file);
  auto row = file->read_revision(1);
  REQUIRE(row);
  REQUIRE(*row);
  const auto&            refs = (*row)->references;
  history::record_digest digest{};
  auto                   interpretation = file->read_record(std::uint64_t(refs.interpretation));
  REQUIRE(interpretation);
  REQUIRE(std::holds_alternative<history::value_record>(interpretation->value));
  CHECK(std::get<history::value_record>(interpretation->value).encoded == data->interpretation.encoded);
  CHECK(flat(read_tree_of(*file, refs.stdlib, history::tree_profile::effective_provenance, &digest)) ==
        flat(data->effective.files));
  CHECK(flat(read_tree_of(*file, refs.registry, history::tree_profile::provenance, &digest)) == flat(data->registry.files));
  const auto bytes_after_first = read_file_bytes(mixed.ledger_path);
  const auto requests          = count_rows(mixed.ledger_path, "requests");
  const auto revisions         = count_rows(mixed.ledger_path, "revisions");
  auto       replay            = run(mixed, mixed.intent(), p);
  REQUIRE(replay.result);
  CHECK(replay.result->replayed);
  CHECK(replay.result->result == done.result->result);
  CHECK(count_rows(mixed.ledger_path, "requests") == requests);
  CHECK(count_rows(mixed.ledger_path, "revisions") == revisions);
  CHECK(read_file_bytes(mixed.ledger_path) == bytes_after_first);
}

namespace {
class x07_c02_history_services final : public command_services {
public:
  int                    calls = 0;
  command_service_result open_workspace(const command_render_options&) override {
    ++calls;
    return {};
  }
  command_service_result capture_render(const command_render_options&, const value_ptr&) override {
    ++calls;
    return {};
  }
  command_service_result initialize_ledger(const command_init_options&) override {
    ++calls;
    return {};
  }
  command_service_result read_history(const command_history_options&, const command_history_selector&) override {
    ++calls;
    return {};
  }
  command_service_result export_source(const command_export_options&) override {
    ++calls;
    return {};
  }
};
} // namespace
TEST_CASE("X07 C02 loser prepared after a winner commit refuses revision_conflict and not capture_changed",
          "[x07c02][profile_acceptance]") {
  profile_fixture f;
  auto            loser_intent = f.intent("req-2");
  auto            opened       = profile_invocation::open_known_load(loser_intent, f.ledger_path.string());
  REQUIRE(opened);
  auto winner = prepare(f, "req-1");
  REQUIRE(run(f, f.intent("req-1"), winner).result);
  auto prepared = std::move(*opened).prepare_load(loser_intent);
  if (!prepared) {
    CAPTURE(prepared.error().failure.detail.code, prepared.error().failure.detail.message);
    FAIL("prepare_load refused");
  }
  REQUIRE(prepared);
  const auto before  = read_file_bytes(f.ledger_path);
  auto       refused = run(f, loser_intent, *prepared);
  expect_untouched(refused, format_error_code::revision_conflict, f.ledger_path, before);
  REQUIRE(refused.result.error().detail);
  const auto* detail = std::get_if<types::revision_conflict_detail>(&*refused.result.error().detail);
  REQUIRE(detail);
  CHECK_FALSE(detail->expected);
  CHECK(detail->actual == 1);
  CHECK(count_rows(f.ledger_path, "requests") == 1);
  CHECK(count_rows(f.ledger_path, "revisions") == 1);
  CHECK(count_rows(f.ledger_path, "events") == 2);
}
TEST_CASE("X07 C02 prepared profile native receipts ignore a committed ledger change", "[x07c02][profile_acceptance]") {
  profile_fixture f;
  auto            loser  = prepare(f, "req-2");
  auto            winner = prepare(f, "req-1");
  REQUIRE(run(f, f.intent("req-1"), winner).result);
  auto verified = loser.verify_native_receipts();
  if (!verified) {
    CAPTURE(verified.error().code, verified.error().message);
    CHECK(verified);
  }
  CHECK(loser.verify_native_receipts());
}
TEST_CASE("X07 C02 post-preparation input change plus winner commit still refuses revision_conflict",
          "[x07c02][profile_acceptance]") {
  profile_fixture f;
  auto            stale  = prepare(f, "req-2");
  auto            winner = prepare(f, "req-1");
  REQUIRE(run(f, f.intent("req-1"), winner).result);
  append_to(f.shared_text(), "\nchanged\n");
  const auto before  = read_file_bytes(f.ledger_path);
  auto       refused = run(f, f.intent("req-2"), stale);
  expect_untouched(refused, format_error_code::revision_conflict, f.ledger_path, before);
  CHECK(refused.result.error().code != format_error_code::capture_changed);
  CHECK(count_rows(f.ledger_path, "requests") == 1);
  CHECK(count_rows(f.ledger_path, "revisions") == 1);
}
TEST_CASE("X07 C02 history family ledger receipt stays strict at the built-in admission site", "[x07c02][profile_acceptance]") {
  profile_fixture         f;
  command_history_options options{f.ledger_path.string(), std::nullopt, std::nullopt, std::nullopt,
                                  std::nullopt,           std::nullopt, std::nullopt};
  auto                    owner = profile_invocation::open_with_profile(f.profile.string(), options);
  REQUIRE(owner);
  append_to(f.ledger_path, "x");
  x07_c02_history_services services;
  auto                     result = std::move(*owner).run_known(options, services);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "capture_changed");
  CHECK(result.error().message == "Captured external profile changed");
  CHECK(services.calls == 0);
}
