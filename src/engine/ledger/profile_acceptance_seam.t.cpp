// Direct tests of the internal acceptance seam: effective stdlib and provenance-limit interning through fake candidates.
module;
#include <catch2/catch_test_macros.hpp>
#include <sqlite3.h>
#include <sys/stat.h>
#include <unistd.h>
module planar.engine.ledger.acceptance;
import std;
import planar.engine.runtime;
import :seam;
import planar.engine.ledger.format;
import planar.engine.acceptance.intent;
import planar.engine.history.tree;
import planar.engine.history.read;
using namespace planar::engine;
using namespace planar::engine::ledger;
using namespace planar::engine::ledger::seam_detail;
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
} // namespace
namespace {
struct fake_candidate : native_candidate {
  std::shared_ptr<const profile_acceptance_data> data;
  std::vector<history::tree_file>                files;
  bool                                           effective;
  fake_candidate(std::shared_ptr<const profile_acceptance_data> d, bool effective_limits)
      : data(std::move(d)), effective(effective_limits) {
    for (int n = 0; n < 4097; ++n)
      files.push_back(history::tree_file{std::format("f{:04d}.lua", n), "x", 420});
  }
  bool selects_profile() const noexcept override {
    return true;
  }
  std::expected<candidate_values, format_error> values(const acceptance::request_data&) const override {
    return candidate_values{data->workspace_id,
                            data->source,
                            data->registry.files,
                            files,
                            effective,
                            data->interpretation.encoded,
                            data->interpretation.versions_encoded,
                            std::shared_ptr<const void>(data)};
  }
  bool verify() const override {
    return true;
  }
};
} // namespace

TEST_CASE("X06 C seam interns an effective stdlib closure beyond the original tree limits", "[atomic_acceptance][x06c]") {
  profile_fixture f;
  auto            p              = prepare(f);
  auto            shared         = data_of(p);
  const auto      intent         = f.intent();
  const auto      control        = f.extra_ledger(2, "control");
  const auto      control_before = read_file_bytes(control);
  {
    fake_candidate     fake(shared, false);
    completion_receipt receipt;
    auto               result = complete_native_candidate(control, intent, fake, identity_of(control), receipt);
    CHECK_FALSE(result);
    CHECK(receipt.phase != completion_phase::committed);
    CHECK(read_file_bytes(control) == control_before);
  }
  // Clause: verify_before_commit re-derives every stored tree against the recorded interpretation, so a fabricated 4097-file
  // closure cannot commit (a real effective closure never exceeds the embedded 4096 + 64 modules + manifest). Reaching that
  // verification instead of the tree-limit refusal proves the effective profile bound was used when interning the stdlib.
  const auto control_message = [&] {
    fake_candidate     fake(shared, false);
    completion_receipt receipt;
    auto               result = complete_native_candidate(control, intent, fake, identity_of(control), receipt);
    REQUIRE_FALSE(result);
    return result.error().message;
  }();
  const auto         ledger_before = read_file_bytes(f.ledger_path);
  fake_candidate     fake(shared, true);
  completion_receipt receipt;
  auto               result = complete_native_candidate(f.ledger_path, intent, fake, identity_of(f.ledger_path), receipt);
  REQUIRE_FALSE(result);
  CHECK(result.error().message == "Accepted history is inconsistent");
  CHECK(control_message != result.error().message);
  CHECK(receipt.phase != completion_phase::committed);
  CHECK(read_file_bytes(f.ledger_path) == ledger_before);
}
namespace {
struct registry_candidate final : native_candidate {
  std::shared_ptr<const profile_acceptance_data> data;
  std::vector<history::tree_file>                registry;
  registry_candidate(std::shared_ptr<const profile_acceptance_data> d, std::string payload) : data(std::move(d)) {
    registry.push_back(history::tree_file{"input.json", payload, 420});
    registry.push_back(history::tree_file{"input2.json", std::move(payload), 420});
  }
  bool selects_profile() const noexcept override {
    return true;
  }
  std::expected<candidate_values, format_error> values(const acceptance::request_data&) const override {
    return candidate_values{data->workspace_id,
                            data->source,
                            registry,
                            data->effective.files,
                            true,
                            data->interpretation.encoded,
                            data->interpretation.versions_encoded,
                            std::shared_ptr<const void>(data)};
  }
  bool verify() const override {
    return true;
  }
};
} // namespace

TEST_CASE("X06 C C01 seam interns the registry tree with the provenance limits", "[atomic_acceptance][x06c][c01]") {
  profile_fixture f;
  auto            p      = prepare(f);
  auto            shared = data_of(p);
  const auto      intent = f.intent();
  // The probe must be installed before any ledger connection opens.
  installed_probe installed;
  // Provenance trees hold at most 16 MiB; the source profile would allow 64 MiB. Two 9 MiB registry files (18 MiB total, each
  // under the 16 MiB per-file cap) exceed the provenance budget but fit the source budget, so they must be refused while
  // interning, before any row exists.
  const auto         before = read_file_bytes(f.ledger_path);
  registry_candidate big(shared, std::string(9 * 1024 * 1024, 'r'));
  completion_receipt receipt;
  auto               result = complete_native_candidate(f.ledger_path, intent, big, identity_of(f.ledger_path), receipt);
  REQUIRE_FALSE(result);
  CHECK(result.error().message != "Accepted history is inconsistent");
  CHECK(receipt.phase == completion_phase::not_completed);
  CHECK(read_file_bytes(f.ledger_path) == before);
  expect_no_rows(f.ledger_path);
  // Under the correct seam the oversize registry tree is refused while interning, so no `UPDATE current_state SET` staging
  // statement ran; a mutant that interns the registry tree with the source profile is refused only at pre-commit
  // verification after staging, so pending would be true.
  CHECK_FALSE(installed.probe.pending);
}
