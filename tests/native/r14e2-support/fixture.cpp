// Scratch-only fixture generator: native capture and atomic acceptance; exceptional version mutation restores integrity.
#include <sqlite3.h>
import std;
import planar.engine.acceptance.prepared;
import planar.engine.ledger.format;
import planar.engine.history.provenance_codec;
import planar.engine.history.interpretation_codec;
import planar.engine.canonical;
using namespace planar::engine;
using namespace planar::engine::history;
using namespace planar::engine::ledger;
using namespace planar::engine::acceptance;
namespace {
template <class T> auto take(T&& result) {
  if (!result)
    throw std::runtime_error("fixture boundary refused");
  if constexpr (!std::is_void_v<typename std::remove_cvref_t<T>::value_type>)
    return std::move(*result);
}
std::string bytes(const std::filesystem::path& p) {
  std::ifstream in(p, std::ios::binary);
  if (!in)
    throw std::runtime_error("fixture input");
  return {std::istreambuf_iterator<char>(in), {}};
}
void put(const std::filesystem::path& p, std::string_view s) {
  std::ofstream out(p, std::ios::binary);
  out.write(s.data(), s.size());
  if (!out)
    throw std::runtime_error("fixture output");
}
value_ptr text(std::string s) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = std::move(s);
  return v;
}
value_ptr changed(const value_ptr& original, std::string key, value_ptr replacement) {
  auto v                    = std::make_shared<value>(*original);
  v->fields[std::move(key)] = std::move(replacement);
  return v;
}
std::string hex(const record_digest& d) {
  std::string s;
  for (auto b : d)
    s += std::format("{:02x}", b);
  return s;
}
void sql(sqlite3* db, const char* s) {
  if (sqlite3_exec(db, s, nullptr, nullptr, nullptr) != SQLITE_OK)
    throw std::runtime_error("fixture SQL");
}
} // namespace
int main(int argc, char** argv) {
  try {
    if (argc != 3)
      return 2;
    const auto        root   = std::filesystem::canonical(argv[1]);
    const std::string mode   = argv[2];
    const auto        dbpath = root / "ledger";
    take(initialize_new(dbpath, "quota"));
    if (mode == "empty")
      return 0;
    std::filesystem::create_directory(root / "workspace");
    put(root / "workspace/a.lua", "original");
    put(root / "registry.json", "registry");
    const std::array originals{original_file_view{"part.lua", "return {}"}};
    auto             stdlib = take(prepare_stdlib(originals, "\n", "return {}\n"));
    auto             gold   = std::filesystem::path(PLANAR_PREPARED_GOLDENS);
    auto saved  = take(decode_interpretation(bytes(gold / "interpretation.canonical"), bytes(gold / "versions.canonical")));
    auto bundle = saved.interpretation->fields.at("bundle");
    if (mode == "oversize")
      bundle = changed(bundle, "spec",
                       changed(bundle->fields.at("spec"), "historical_extension", text(std::string(9 * 1024 * 1024, '\x01'))));
    auto interpreted = take(encode_interpretation(bundle, saved.interpretation->fields.at("semantic"), hex(stdlib.digest)));
    const std::array paths{std::string_view("a.lua")};
    auto             accept = [&](std::string key, std::optional<std::int64_t> head) {
      native_capture_session session;
      take(session.add_root("workspace", root / "workspace"));
      take(session.tree("workspace"));
      take(session.add_input("registry", root / "registry.json"));
      take(session.read("registry", "input.json", false));
      auto capture   = take(session.finish_for_acceptance());
      auto candidate = take(prepared_acceptance::create(std::move(capture), paths, stdlib.files, interpreted.encoded,
                                                        interpreted.versions_encoded));
      auto intent    = take(request_intent::create(key, "fixture:actor", head, (root / "PRIVATE_INTENT_WORKSPACE.lua").string(),
                                                   (root / "PRIVATE_INTENT_REGISTRY.json").string()));
      take(complete_acceptance(dbpath, intent, candidate));
    };
    accept("first", {});
    if (mode == "history") {
      accept("noop", 1);
      put(root / "workspace/a.lua", "changed");
      accept("changed", 1);
      put(root / "workspace/a.lua", "original");
      accept("reverted", 2);
    }
    if (mode == "unknown") {
      // Native acceptance rejects unsupported versions. Mutate only the two immutable
      // value records after accepted publication, recomputing real record_v1/digests;
      // restore exact triggers and require the production verified opener afterward.
      auto refs = [&] {
        auto file     = take(open_existing(dbpath));
        auto revision = take(file.read_revision(1));
        if (!revision)
          throw std::runtime_error("fixture revision");
        return revision->references;
      }();
      interpreted.versions       = changed(interpreted.versions, "interpretation", text("interpretation_v99"));
      interpreted.interpretation = changed(interpreted.interpretation, "versions", interpreted.versions);
      const std::array<std::pair<std::int64_t, value_ptr>, 2> values{
          {{refs.interpretation, interpreted.interpretation}, {refs.versions, interpreted.versions}}};
      // Release the read snapshot before the narrowly scoped test-only writer.
      sqlite3* db = nullptr;
      if (sqlite3_open_v2(dbpath.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr) != SQLITE_OK)
        return 4;
      sql(db, "BEGIN IMMEDIATE");
      sql(db, "DROP TRIGGER records_no_update");
      for (const auto& [id, v] : values) {
        record        r         = value_record{"canonical_v1", take(canonical_encode(v))};
        auto          encoded   = take(encode_record(r));
        auto          digest    = take(digest_record(r));
        sqlite3_stmt* statement = nullptr;
        if (sqlite3_prepare_v2(db, "UPDATE records SET digest=?1,encoded=?2 WHERE id=?3", -1, &statement, nullptr) != SQLITE_OK)
          return 4;
        sqlite3_bind_blob(statement, 1, digest.data(), digest.size(), SQLITE_TRANSIENT);
        sqlite3_bind_blob(statement, 2, encoded.data(), encoded.size(), SQLITE_TRANSIENT);
        sqlite3_bind_int64(statement, 3, id);
        if (sqlite3_step(statement) != SQLITE_DONE)
          return 4;
        sqlite3_finalize(statement);
      }
      sql(db, "CREATE TRIGGER records_no_update BEFORE UPDATE ON records BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END");
      sql(db, "COMMIT");
      sqlite3_close(db);
    }
    take(open_existing(dbpath));
    return 0;
  } catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
    return 4;
  }
}
