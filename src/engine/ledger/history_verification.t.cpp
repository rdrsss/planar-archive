// Scratch accepted-history fixtures use public record preparation and private test-only SQL publication.
#include <catch2/catch_test_macros.hpp>
#include <signal.h>
#include <sqlite3.h>
#include <sys/wait.h>
#include <unistd.h>
import std;
import planar.engine.ledger.format;
import planar.engine.acceptance.prepared;
import planar.engine.ledger.acceptance;
import planar.engine.ledger.history_codec;
import planar.engine.history.tree_core;
import planar.engine.history.provenance;
import planar.engine.history.interpretation_codec;
import planar.engine.history.interpretation_v2_codec;
import planar.engine.canonical;
import planar.engine.history.read;
import planar.engine.source_export;
import planar.engine.json_codec;
using namespace planar::engine;
using namespace planar::engine::ledger;
using namespace planar::engine::history;
namespace {
std::string bytes(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  REQUIRE(in.good());
  return {std::istreambuf_iterator<char>(in), {}};
}
value_ptr changed(value_ptr original, std::string key, value_ptr replacement) {
  auto value                    = std::make_shared<planar::engine::value>(*original);
  value->fields[std::move(key)] = std::move(replacement);
  return value;
}
value_ptr text(std::string value) {
  auto result    = std::make_shared<planar::engine::value>();
  result->kind   = value_kind::string;
  result->string = std::move(value);
  return result;
}
std::string encode(value_ptr value) {
  auto encoded = canonical_encode(value);
  REQUIRE(encoded);
  return *encoded;
}
std::string hex(const record_digest& digest) {
  std::string result;
  for (auto byte : digest)
    result += std::format("{:02x}", byte);
  return result;
}
struct raw_db {
  sqlite3* db = nullptr;
  explicit raw_db(const std::filesystem::path& path) {
    REQUIRE(sqlite3_open_v2(path.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr) == SQLITE_OK);
    sql("PRAGMA foreign_keys=OFF");
  }
  ~raw_db() {
    sqlite3_close(db);
  }
  void sql(std::string_view command) {
    INFO(command);
    REQUIRE(sqlite3_exec(db, std::string(command).c_str(), nullptr, nullptr, nullptr) == SQLITE_OK);
  }
  void bind_insert(std::string_view command, std::initializer_list<std::string_view> values) {
    sqlite3_stmt* statement = nullptr;
    REQUIRE(sqlite3_prepare_v2(db, command.data(), int(command.size()), &statement, nullptr) == SQLITE_OK);
    int index = 1;
    for (auto value : values)
      REQUIRE(sqlite3_bind_blob(statement, index++, value.data(), int(value.size()), SQLITE_TRANSIENT) == SQLITE_OK);
    auto result = sqlite3_step(statement);
    sqlite3_finalize(statement);
    REQUIRE(result == SQLITE_DONE);
  }
  void mutate(std::string_view table, std::string_view command) {
    auto trigger = std::format("{}_no_update", table);
    sql(std::format("DROP TRIGGER {}", trigger));
    sql(command);
    sql(std::format("CREATE TRIGGER {} BEFORE UPDATE ON {} BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END", trigger,
                    table));
  }
};
std::vector<tree_file> a3_files() {
  std::istringstream     in(bytes(std::filesystem::path(PLANAR_V2_VECTORS) / "files.tsv"));
  std::string            line;
  std::vector<tree_file> out;
  while (std::getline(in, line)) {
    if (line.starts_with('#'))
      continue;
    auto        a = line.find('\t'), b = line.find('\t', a + 1);
    std::string payload;
    for (std::size_t i = b + 1; i < line.size(); i += 2)
      payload += static_cast<char>(std::stoul(line.substr(i, 2), nullptr, 16));
    out.push_back({line.substr(0, a), payload, static_cast<std::uint32_t>(std::stoul(line.substr(a + 1, b - a - 1)))});
  }
  return out;
}
std::vector<tree_file> a3_large_files() {
  auto                   raw = a3_files();
  std::vector<tree_file> originals;
  auto                   original_manifest = std::ranges::find(raw, "__planar__/embedded.bin", &tree_file::path);
  REQUIRE(original_manifest != raw.end());
  auto original_order = canonical_decode(original_manifest->payload);
  REQUIRE(original_order);
  for (const auto& path : (*original_order)->fields.at("files")->items) {
    auto file = std::ranges::find(raw, "shipped/" + path->string, &tree_file::path);
    REQUIRE(file != raw.end());
    originals.push_back({path->string, file->payload, 420});
  }
  for (int n = int(originals.size()); n < 4094; ++n)
    originals.push_back({std::format("padding/f{:04}.lua", n), "", 420});
  std::vector<original_file_view> views;
  std::string                     assembly;
  for (const auto& file : originals) {
    views.push_back({file.path, file.payload});
    assembly += file.payload;
  }
  auto embedded = prepare_stdlib(views, "", assembly);
  REQUIRE(embedded);
  REQUIRE(embedded->files.size() == 4096);
  auto manifest = std::ranges::find(raw, "__planar__/effective.bin", &tree_file::path);
  REQUIRE(manifest != raw.end());
  auto decoded = canonical_decode(manifest->payload);
  REQUIRE(decoded);
  auto profile = std::ranges::find(raw, "profile/input.json", &tree_file::path);
  REQUIRE(profile != raw.end());
  std::vector<tree_file_view> modules;
  for (auto path : {"external/ext/z.lua", "external/ext/a.lua"}) {
    auto file = std::ranges::find(raw, path, &tree_file::path);
    REQUIRE(file != raw.end());
    modules.push_back({std::string_view(file->path).substr(9), file->payload, file->mode});
  }
  auto catalog   = encode((*decoded)->fields.at("catalog"));
  auto effective = prepare_effective_stdlib({embedded->files, profile->payload, profile->mode, modules, catalog});
  REQUIRE(effective);
  REQUIRE(effective->files.size() > 4096);
  return std::move(effective->files);
}
struct fixture {
  std::filesystem::path           root, path;
  std::string                     workspace_selector, registry_selector, profile_selector;
  ledger_metadata                 metadata;
  std::array<event_references, 2> refs{};
  std::array<std::int64_t, 4>     bodies{};
  std::int64_t                    spare_value = 0, spare_tree = 0;
  explicit fixture(std::string_view variant = {}) {
    std::string pattern = (std::filesystem::temp_directory_path() / "planar-history-XXXXXX").string();
    REQUIRE(mkdtemp(pattern.data()));
    root               = std::filesystem::canonical(pattern);
    path               = root / "ledger";
    const auto goldens = std::filesystem::path(PLANAR_HISTORY_GOLDENS);
    auto decoded = decode_interpretation(bytes(goldens / "interpretation.canonical"), bytes(goldens / "versions.canonical"));
    REQUIRE(decoded);
    auto workspace = decoded->interpretation->fields.at("bundle")->fields.at("workspace")->fields.at("id")->string;
    bool profile   = variant.starts_with("profile");
    if (profile)
      workspace = "workspace";
    auto initialized = profile ? initialize_new_v2(path, workspace) : initialize_new(path, workspace);
    INFO((initialized ? "initialized" : initialized.error().message));
    REQUIRE(initialized);
    metadata    = *initialized;
    auto writer = begin_records(path);
    REQUIRE(writer);
    auto intern = [&](const record& record) {
      auto ref = writer->intern(record);
      REQUIRE(ref);
      return *ref;
    };
    auto tree = [&](std::span<const tree_file_view> files, tree_profile profile) {
      auto ref = build_tree_core(files, profile, [&](const record& value) { return writer->intern(value); });
      REQUIRE(ref);
      return *ref;
    };
    auto stdlib = prepare_embedded_stdlib();
    REQUIRE(stdlib);
    std::vector<tree_file_view> stdlib_files;
    for (const auto& file : stdlib->files)
      stdlib_files.push_back({file.path, file.payload, file.mode});
    if (variant == "stdlib_assembly")
      for (auto& file : stdlib_files)
        if (file.path == "__planar__/assembly.lua")
          file.payload = "changed";
    std::string unknown_embedded_manifest;
    if (variant == "unknown_embedded_manifest")
      for (auto& file : stdlib_files)
        if (file.path == "__planar__/assembly.canonical") {
          auto manifest = canonical_decode(file.payload);
          REQUIRE(manifest);
          auto version              = std::make_shared<value>(*(*manifest)->fields.at("version"));
          version->integer          = 99;
          unknown_embedded_manifest = encode(changed(*manifest, "version", version));
          file.payload              = unknown_embedded_manifest;
        }
    auto effective_files = profile ? (variant == "profile_large" ? a3_large_files() : a3_files()) : std::vector<tree_file>{};
    if (variant == "profile_unknown_manifest") {
      auto manifest = std::ranges::find(effective_files, "__planar__/effective.bin", &tree_file::path);
      REQUIRE(manifest != effective_files.end());
      auto decoded_manifest = canonical_decode(manifest->payload);
      REQUIRE(decoded_manifest);
      auto m               = std::make_shared<value>(**decoded_manifest);
      auto version         = std::make_shared<value>(*m->fields.at("version"));
      version->integer     = 99;
      m->fields["version"] = version;
      manifest->payload    = encode(m);
    }
    if (variant == "profile_tree")
      effective_files.back().payload += "forged raw bytes";
    if (profile) {
      stdlib_files.clear();
      for (const auto& file : effective_files)
        stdlib_files.push_back({file.path, file.payload, file.mode});
    }
    auto        stdlib_root    = tree(stdlib_files, profile ? tree_profile::effective_provenance : tree_profile::provenance);
    std::string registry_bytes = bytes(std::filesystem::path(PLANAR_HISTORY_FIXTURE) / "registry/quota-tools.json");
    std::array<tree_file_view, 1> registry_files{
        {{variant == "registry_name" ? "wrong.json" : "input.json", registry_bytes, 420}}};
    auto registry = tree(registry_files, tree_profile::provenance);
    auto versions = changed(decoded->versions, "stdlib_digest",
                            text(variant == "stdlib_digest" ? std::string(32, '0') : hex(stdlib->digest)));
    if (variant.starts_with("unknown_interpretation"))
      versions = changed(versions, "interpretation", text("interpretation_v99"));
    auto envelope = changed(decoded->interpretation, "versions", versions);
    if (variant == "workspace" || variant == "unknown_interpretation_workspace") {
      auto bundle = envelope->fields.at("bundle");
      envelope    = changed(envelope, "bundle",
                            changed(bundle, "workspace", changed(bundle->fields.at("workspace"), "id", text("other_workspace"))));
    }
    if (profile) {
      auto raw = canonical_decode(bytes(std::filesystem::path(PLANAR_V2_VECTORS) / "interpretation.bin"));
      REQUIRE(raw);
      envelope = *raw;
      versions = envelope->fields.at("versions");
      if (variant == "profile_large") {
        auto updated = encode_interpretation_v2(envelope->fields.at("bundle"), envelope->fields.at("semantic"), effective_files);
        REQUIRE(updated);
        envelope = updated->interpretation;
        versions = updated->versions;
      }
      if (variant == "profile_unknown_interpretation" || variant == "profile_unknown_workspace") {
        versions = changed(versions, "interpretation", text("interpretation_v99"));
        envelope = changed(envelope, "versions", versions);
      }
      if (variant == "profile_workspace" || variant == "profile_unknown_workspace") {
        auto bundle = envelope->fields.at("bundle");
        envelope = changed(envelope, "bundle",
                           changed(bundle, "workspace", changed(bundle->fields.at("workspace"), "id", text("wrong_workspace"))));
      }
      if (variant == "profile_digest") {
        versions = changed(versions, "stdlib_digest", text(std::string(32, '0')));
        envelope = changed(envelope, "versions", versions);
      }
      if (variant == "profile_catalog") {
        auto extensions = envelope->fields.at("extensions");
        auto catalog    = extensions->fields.at("catalog");
        auto empty      = std::make_shared<value>();
        empty->kind     = value_kind::list;
        envelope        = changed(envelope, "extensions", changed(extensions, "catalog", changed(catalog, "shapes", empty)));
      }
      if (variant == "profile_selection") {
        auto extensions    = envelope->fields.at("extensions");
        auto selected      = std::make_shared<value>(*extensions->fields.at("selected_shapes"));
        selected->items[0] = changed(selected->items[0], "definition", text("xxh3_128:" + std::string(32, '0')));
        envelope           = changed(envelope, "extensions", changed(extensions, "selected_shapes", selected));
      }
      if (variant == "profile_resource") {
        auto semantic  = envelope->fields.at("semantic");
        auto jobs      = std::make_shared<value>(*semantic->fields.at("jobs"));
        auto selected  = jobs->items[0]->fields.at("selected_shape");
        jobs->items[0] = changed(jobs->items[0], "selected_shape", changed(selected, "guidance", text(std::string(65537, 'x'))));
        envelope       = changed(envelope, "semantic", changed(semantic, "jobs", jobs));
      }
      // Fixture workspace is selected from recorded v2, independently of current files/runtime.
      if (variant != "profile_workspace" && variant != "profile_unknown_workspace")
        REQUIRE(envelope->fields.at("bundle")->fields.at("workspace")->fields.at("id")->string == workspace);
    }
    auto version_ref = intern(value_record{
        "canonical_v1", encode(variant == "profile_versions" ? changed(versions, "projection", text("other")) : versions)});
    if (variant == "profile_malformed")
      envelope = text("wrong");
    auto interpretation =
        intern(value_record{"canonical_v1", variant == "malformed_interpretation" ? encode(text("wrong")) : encode(envelope)});
    spare_value = std::int64_t(intern(value_record{"canonical_v1", encode(decoded->versions)}).id);
    std::vector<tree_file> source_files;
    const auto             source_root = std::filesystem::path(PLANAR_HISTORY_FIXTURE) / "workspace";
    for (const auto& entry : std::filesystem::recursive_directory_iterator(source_root))
      if (entry.is_regular_file())
        source_files.push_back({entry.path().lexically_relative(source_root).generic_string(), bytes(entry.path()), 420});
    std::vector<tree_file_view> source_views;
    for (const auto& file : source_files)
      source_views.push_back({file.path, file.payload, file.mode});
    auto source = tree(source_views, tree_profile::source);
    if (variant == "source_limit" || variant == "registry_limit") {
      auto             leaf = intern(file_record{420, "bounded repeated leaf"});
      directory_record directory;
      for (int i = 0; i < 4097; ++i)
        directory.children.push_back({std::format("f{:04}", i), leaf.kind, leaf.id, leaf.digest});
      auto over_limit = intern(directory);
      if (variant == "source_limit")
        source = over_limit;
      else
        registry = over_limit;
    }
    if (variant == "source_kind")
      source = interpretation;
    if (variant == "versions_mismatch")
      version_ref = intern(value_record{"canonical_v1", encode(changed(versions, "projection", text("projection_other")))});
    auto declaration = std::ranges::find(source_files, "workspace.lua", &tree_file::path);
    REQUIRE(declaration != source_files.end());
    declaration->payload += "\n-- exact captured comment change\n";
    source_views.clear();
    for (const auto& file : source_files)
      source_views.push_back({file.path, file.payload, file.mode});
    auto changed_source = tree(source_views, tree_profile::source);
    spare_tree          = std::int64_t(changed_source.id);
    refs[0] = {std::int64_t(source.id), std::int64_t(registry.id), std::int64_t(stdlib_root.id), std::int64_t(interpretation.id),
               std::int64_t(version_ref.id)};
    refs[1] = refs[0];
    refs[1].source = std::int64_t(changed_source.id);
    if (profile) {
      auto put = [&](std::filesystem::path target, std::string_view payload) {
        std::filesystem::create_directories(target.parent_path());
        std::ofstream out(target, std::ios::binary);
        out.write(payload.data(), payload.size());
        REQUIRE(out.good());
      };
      workspace_selector = (root / "original/source/workspace.lua").string();
      registry_selector  = (root / "original/registry.json").string();
      profile_selector   = (root / "original/runtime/profile/input.json").string();
      for (const auto& file : source_files)
        put(root / "original/source" / file.path, file.payload);
      put(registry_selector, registry_bytes);
      for (const auto& file : effective_files)
        put(root / "original/runtime" / file.path, file.payload);
    }
    const std::array<std::string, 4> heads{"none", "1", "3", "3"};
    for (std::size_t i = 0; i < bodies.size(); ++i) {
      if (profile) {
        auto ref = writer->intern_request_v2(
            {"load", "driver:history", heads[i], workspace_selector, registry_selector, profile_selector});
        REQUIRE(ref);
        bodies[i] = std::int64_t(ref->id);
      } else
        bodies[i] = std::int64_t(intern(request_record{"load", "driver:history", heads[i], "workspace.lua", "registry.json"}).id);
    }
    REQUIRE(writer->commit());
    if (variant == "duplicate_acceptance") {
      raw_db db(path);
      db.sql(std::format("INSERT INTO records SELECT 999999,kind,codec,digest,encoded FROM records WHERE id={}", refs[0].source));
      db.sql(std::format("INSERT INTO tree_edges SELECT 999999,ordinal,name,child_id FROM tree_edges WHERE parent_id={}",
                         refs[0].source));
      refs[1].source = 999999;
    }
    publish();
    if (profile) {
      REQUIRE(std::filesystem::exists(workspace_selector));
      REQUIRE(std::filesystem::exists(profile_selector));
      std::filesystem::remove_all(root / "original");
      REQUIRE_FALSE(std::filesystem::exists(root / "original"));
    }
  }
  ~fixture() {
    std::filesystem::remove_all(root);
  }
  void publish() {
    raw_db db(path);
    db.sql("PRAGMA foreign_keys=ON");
    db.sql("BEGIN IMMEDIATE");
    const std::array<std::int64_t, 4> revisions{1, 3, 3, 8};
    const std::array<std::int64_t, 4> before{0, 1, 3, 3};
    const std::array<std::int64_t, 4> parents{0, 1, 1, 3};
    for (std::size_t i = 0; i < 4; ++i) {
      bool        noop     = i == 2;
      const auto& ref      = refs[i == 1 || i == 2 ? 1 : 0];
      const auto  revision = revisions[i], sequence = std::int64_t(i + 2);
      const auto  request_id = std::format("request:{}", i + 1);
      // Deliberately regressing clocks and gapped revision IDs are valid context, never causality.
      const auto time = std::format("2026-01-0{}T00:00:00.000Z", 9 - i);
      if (!noop)
        db.sql(std::format("INSERT INTO revisions VALUES({},{},{},{},{},{},{},'driver:history','{}')", revision,
                           before[i] ? std::to_string(before[i]) : "NULL", ref.source, ref.registry, ref.stdlib,
                           ref.interpretation, ref.versions, time));
      auto event  = encode_history_event({before[i] ? std::optional{before[i]} : std::nullopt, revision,
                                          noop ? event_reason::identical_declaration : event_reason::declaration_accepted, ref});
      auto result = encode_load_result({metadata.identity, request_id, noop ? load_outcome::noop : load_outcome::accepted,
                                        revision, parents[i] ? std::optional{parents[i]} : std::nullopt, sequence});
      REQUIRE(event);
      REQUIRE(result);
      db.bind_insert(std::format("INSERT INTO events VALUES({},'{}',1,{},'{}','driver:history','{}',?1)", sequence,
                                 noop ? "load_noop" : "declaration_accepted", revision, request_id, time),
                     {*event});
      db.bind_insert(std::format("INSERT INTO requests VALUES('{}',{},{},{},'{}',?1)", request_id, bodies[i], sequence, revision,
                                 noop ? "noop" : "accepted"),
                     {*result});
    }
    db.sql("UPDATE current_state SET active_revision_id=8,last_seq=5");
    db.sql("COMMIT");
  }
  void refuse(format_error_code expected = format_error_code::corrupt) {
    auto original = bytes(path);
    for (auto access : {ledger_access::read_only, ledger_access::read_write}) {
      auto opened = open_existing(path, access);
      REQUIRE_FALSE(opened);
      CHECK(opened.error().code == expected);
      CHECK(bytes(path) == original);
    }
    CHECK_FALSE(begin_records(path));
    CHECK_FALSE(backup_new(path, root / "backup"));
    CHECK_FALSE(std::filesystem::exists(root / "backup"));
    CHECK(bytes(path) == original);
    CHECK(std::distance(std::filesystem::directory_iterator(root), std::filesystem::directory_iterator{}) == 1);
  }
};
} // namespace
TEST_CASE("ledger complete accepted change noop and revert history reopens writes backs up and restores", "[ledger][history]") {
  fixture f;
  for (auto access : {ledger_access::read_only, ledger_access::read_write}) {
    auto opened = open_existing(f.path, access);
    REQUIRE(opened);
    CHECK(opened->metadata().identity == f.metadata.identity);
    CHECK(opened->read_record(std::uint64_t(f.refs[0].source)));
  }
  {
    auto writer = begin_records(f.path);
    REQUIRE(writer);
    REQUIRE(writer->intern(file_record{420, "future scratch record"}));
    REQUIRE(writer->commit());
  }
  REQUIRE(backup_new(f.path, f.root / "backup"));
  REQUIRE(restore_new(f.root / "backup", f.root / "restored"));
  CHECK(open_existing(f.root / "restored", ledger_access::read_write));
}
TEST_CASE("ledger accepted history rejects schema valid relational and chronology corruption unchanged", "[ledger][history]") {
  const std::array<std::pair<std::string_view, std::string_view>, 15> mutations{
      {{"revisions", "UPDATE revisions SET parent_id=1 WHERE id=8"},
       {"revisions", "UPDATE revisions SET actor='other' WHERE id=3"},
       {"revisions", "UPDATE revisions SET accepted_at='2026-01-01T00:00:00.000Z' WHERE id=3"},
       {"events", "UPDATE events SET actor='other' WHERE seq=4"},
       {"events", "UPDATE events SET occurred_at='invalid' WHERE seq=4"},
       {"events", "UPDATE events SET revision_id=1 WHERE seq=4"},
       {"events", "UPDATE events SET request_id='missing' WHERE seq=4"},
       {"events", "UPDATE events SET type='declaration_accepted' WHERE seq=4"},
       {"events", "UPDATE events SET seq=6 WHERE seq=5"},
       {"requests", "UPDATE requests SET revision_id=1 WHERE request_id='request:3'"},
       {"requests", "UPDATE requests SET outcome='accepted' WHERE request_id='request:3'"},
       {"requests", "UPDATE requests SET body_id=1 WHERE request_id='request:3'"},
       {"requests", "UPDATE requests SET request_id='different' WHERE request_id='request:3'"},
       {"requests", "UPDATE requests SET result=x'7b7d' WHERE request_id='request:3'"},
       {"events", "UPDATE events SET payload=x'7b7d' WHERE seq=4"}}};
  for (const auto& [table, command] : mutations) {
    INFO(command);
    fixture f;
    {
      raw_db db(f.path);
      db.mutate(table, command);
    }
    f.refuse();
  }
}
TEST_CASE("ledger accepted history rejects orphan and projection rows", "[ledger][history]") {
  for (int kind = 0; kind < 4; ++kind) {
    fixture f;
    {
      raw_db db(f.path);
      if (kind == 0)
        db.sql("UPDATE current_state SET active_revision_id=3");
      if (kind == 1)
        db.sql("UPDATE current_state SET last_seq=4");
      if (kind == 2)
        db.sql(std::format("INSERT INTO revisions VALUES(9,8,{},{},{},{},{},'other','2026-01-01T00:00:00.000Z')",
                           f.refs[0].source, f.refs[0].registry, f.refs[0].stdlib, f.refs[0].interpretation, f.refs[0].versions));
      if (kind == 3)
        db.sql(std::format("INSERT INTO requests VALUES('orphan',{},1,1,'accepted',x'7b7d')", f.bodies[0]));
    }
    f.refuse();
  }
}
TEST_CASE("ledger accepted history validates complete provenance workspace and interpretation shapes", "[ledger][history]") {
  for (auto variant :
       {"registry_name", "stdlib_assembly", "stdlib_digest", "workspace", "malformed_interpretation", "source_kind",
        "versions_mismatch", "source_limit", "registry_limit", "unknown_interpretation_workspace", "unknown_embedded_manifest"}) {
    INFO(variant);
    fixture f(variant);
    f.refuse();
  }
}
TEST_CASE("ledger accepted history permits raw access to shape verified unsupported interpretation versions",
          "[ledger][history]") {
  fixture f("unknown_interpretation");
  auto    opened = open_existing(f.path, ledger_access::read_write);
  REQUIRE(opened);
  auto stored = opened->read_record(std::uint64_t(f.refs[0].interpretation));
  REQUIRE(stored);
  auto versions = opened->read_record(std::uint64_t(f.refs[0].versions));
  REQUIRE(versions);
  auto decoded =
      decode_interpretation(std::get<value_record>(stored->value).encoded, std::get<value_record>(versions->value).encoded);
  REQUIRE_FALSE(decoded);
  CHECK(decoded.error().code == interpretation_error_code::unsupported_version);
  REQUIRE(backup_new(f.path, f.root / "backup"));
}
TEST_CASE("ledger accepted history checks saved noop parent event references request body and bounded metadata",
          "[ledger][history]") {
  for (int kind = 0; kind < 8; ++kind) {
    INFO(kind);
    fixture f;
    {
      raw_db db(f.path);
      if (kind == 0)
        db.mutate("requests", "UPDATE requests SET result=CAST(replace(CAST(result AS "
                              "TEXT),'\"parent\":\"1\"','\"parent\":\"none\"') AS BLOB) WHERE request_id='request:3'");
      if (kind == 1)
        db.mutate("requests", "UPDATE requests SET result=CAST(replace(CAST(result AS TEXT),'\"event\":\"4\"','\"event\":\"3\"') "
                              "AS BLOB) WHERE request_id='request:3'");
      if (kind == 2)
        db.mutate("events", std::format("UPDATE events SET payload=CAST(replace(CAST(payload AS "
                                        "TEXT),'\"source\":\"{}\"','\"source\":\"{}\"') AS BLOB) WHERE seq=4",
                                        f.refs[1].source, f.refs[0].source));
      if (kind == 3)
        db.mutate("requests", std::format("UPDATE requests SET body_id={} WHERE request_id='request:3'", f.bodies[0]));
      if (kind == 4)
        db.mutate("events", "UPDATE events SET actor='' WHERE seq=4");
      if (kind == 5)
        db.mutate("events", "UPDATE events SET actor=CAST(zeroblob(2097152) AS TEXT) WHERE seq=4");
      if (kind == 6)
        db.mutate("events", "UPDATE events SET request_id=hex(zeroblob(1048576)) WHERE seq=4");
      if (kind == 7)
        db.mutate("events", "UPDATE events SET actor=CAST(x'ff' AS TEXT) WHERE seq=4");
    }
    f.refuse();
  }
}
TEST_CASE("ledger accepted history distinguishes unknown event and result payload versions", "[ledger][history]") {
  for (bool result : {false, true}) {
    fixture f;
    {
      raw_db db(f.path);
      if (result)
        db.mutate("requests", "UPDATE requests SET result=CAST(replace(CAST(result AS TEXT),'\"version\":1','\"version\":2') AS "
                              "BLOB) WHERE request_id='request:3'");
      else
        db.mutate(
            "events",
            "UPDATE events SET payload=CAST(replace(CAST(payload AS TEXT),'\"version\":1','\"version\":2') AS BLOB) WHERE seq=4");
    }
    f.refuse(format_error_code::unsupported_state);
  }
}
TEST_CASE("ledger complete accepted history survives explicit quiescent hot journal recovery", "[ledger][history][recovery]") {
  fixture f;
  auto    original = bytes(f.path);
  int     pipes[2];
  REQUIRE(pipe(pipes) == 0);
  auto child = fork();
  REQUIRE(child >= 0);
  if (child == 0) {
    close(pipes[0]);
    sqlite3* db     = nullptr;
    int      result = sqlite3_open(f.path.c_str(), &db);
    if (result == SQLITE_OK)
      result = sqlite3_exec(db,
                            "PRAGMA cache_size=1;PRAGMA synchronous=FULL;BEGIN IMMEDIATE;INSERT INTO records "
                            "VALUES(999999,1,1,zeroblob(16),zeroblob(200000));",
                            nullptr, nullptr, nullptr);
    if (result == SQLITE_OK)
      result = sqlite3_db_cacheflush(db);
    char ready = result == SQLITE_OK ? 'Y' : 'N';
    if (write(pipes[1], &ready, 1) != 1)
      _exit(91);
    for (;;)
      pause();
  }
  close(pipes[1]);
  char ready = 0;
  REQUIRE(read(pipes[0], &ready, 1) == 1);
  close(pipes[0]);
  REQUIRE(kill(child, SIGKILL) == 0);
  int status = 0;
  REQUIRE(waitpid(child, &status, 0) == child);
  REQUIRE(ready == 'Y');
  REQUIRE(WIFSIGNALED(status));
  auto refused = open_existing(f.path);
  REQUIRE_FALSE(refused);
  CHECK(refused.error().code == format_error_code::recovery_required);
  REQUIRE(recover_quiescent(f.path));
  CHECK(bytes(f.path) == original);
  CHECK(open_existing(f.path, ledger_access::read_write));
}
TEST_CASE("ledger accepted history negotiates unknown event row versions separately from outer format", "[ledger][history]") {
  fixture f;
  {
    raw_db db(f.path);
    db.sql("PRAGMA ignore_check_constraints=ON");
    db.mutate("events", "UPDATE events SET version=2 WHERE seq=4");
  }
  f.refuse(format_error_code::unsupported_state);
}

TEST_CASE("ledger accepted history refuses identical content hidden behind duplicate record IDs", "[ledger][history]") {
  fixture f("duplicate_acceptance");
  f.refuse();
}
namespace {
std::uint64_t c1_noop_body(fixture& f, std::string argument = "workspace.lua") {
  auto writer = begin_records(f.path);
  REQUIRE(writer);
  auto body = writer->intern(request_record{"load", "driver:history", "8", std::move(argument), "registry.json"});
  REQUIRE(body);
  auto id = body->id;
  REQUIRE(writer->commit());
  return id;
}
void c1_append_noop(raw_db& db, const fixture& f, std::uint64_t body, std::int64_t seq) {
  auto key     = std::format("later:{}", seq);
  auto payload = encode_history_event({8, 8, event_reason::identical_declaration, f.refs[0]});
  auto result  = encode_load_result({f.metadata.identity, key, load_outcome::noop, 8, 3, seq});
  REQUIRE(payload);
  REQUIRE(result);
  db.bind_insert(
      std::format("INSERT INTO events VALUES({},'load_noop',1,8,'{}','driver:history','2026-01-01T00:00:00.000Z',?1)", seq, key),
      {*payload});
  db.bind_insert(std::format("INSERT INTO requests VALUES('{}',{},{},8,'noop',?1)", key, body, seq), {*result});
  db.sql(std::format("UPDATE current_state SET last_seq={}", seq));
}
std::string c1_sql_bytes(const std::filesystem::path& path, std::string_view sql) {
  raw_db        db(path);
  sqlite3_stmt* row = nullptr;
  REQUIRE(sqlite3_prepare_v2(db.db, sql.data(), int(sql.size()), &row, nullptr) == SQLITE_OK);
  REQUIRE(sqlite3_step(row) == SQLITE_ROW);
  const auto* data = sqlite3_column_blob(row, 0);
  auto        size = sqlite3_column_bytes(row, 0);
  std::string bytes(size ? static_cast<const char*>(data) : "", size);
  REQUIRE(sqlite3_finalize(row) == SQLITE_OK);
  return bytes;
}
void c1_check_unavailable(format_file& owner, const fixture& f) {
  CHECK_FALSE(owner.read_head());
  CHECK_FALSE(owner.read_revision(1));
  CHECK_FALSE(owner.read_event(1));
  CHECK_FALSE(owner.read_request("request:1"));
  CHECK_FALSE(owner.read_events_after(0, 1));
  CHECK_FALSE(owner.read_record(std::uint64_t(f.refs[0].source)));
  CHECK(owner.metadata().identity == f.metadata.identity);
}
} // namespace
TEST_CASE("ledger typed reads preserve exact initial change noop revert rows and replay bytes", "[ledger][history][typed]") {
  fixture f;
  for (auto access : {ledger_access::read_only, ledger_access::read_write}) {
    auto owner = open_existing(f.path, access);
    REQUIRE(owner);
    auto head = owner->read_head();
    REQUIRE(head);
    CHECK(head->revision == 8);
    CHECK(head->last_sequence == 5);
    for (std::int64_t seq = 1; seq <= 5; ++seq) {
      auto event = owner->read_event(std::uint64_t(seq));
      REQUIRE(event);
      REQUIRE(*event);
      CHECK((**event).sequence == seq);
      CHECK((**event).version == 1);
      CHECK((**event).payload == c1_sql_bytes(f.path, std::format("SELECT payload FROM events WHERE seq={}", seq)));
      auto decoded = decode_history_event((**event).payload);
      REQUIRE(decoded);
      if (seq == 1) {
        CHECK((**event).type == "ledger_initialized");
        CHECK_FALSE((**event).request_id);
        CHECK_FALSE((**event).revision);
        CHECK((**event).actor == "planar:init");
        CHECK((**event).occurred_at == f.metadata.created_at);
        CHECK_FALSE(decoded->after);
        continue;
      }
      const std::array<std::int64_t, 4> ids{1, 3, 3, 8}, parents{0, 1, 1, 3};
      auto                              index = std::size_t(seq - 2);
      CHECK((**event).type == (seq == 4 ? "load_noop" : "declaration_accepted"));
      CHECK((**event).revision == ids[index]);
      CHECK((**event).request_id == std::format("request:{}", seq - 1));
      CHECK((**event).actor == "driver:history");
      CHECK((**event).occurred_at == std::format("2026-01-0{}T00:00:00.000Z", 9 - index));
      auto revision = owner->read_revision(std::uint64_t(*decoded->after));
      REQUIRE(revision);
      REQUIRE(*revision);
      CHECK((**revision).id == ids[index]);
      CHECK((**revision).parent == (parents[index] ? std::optional{parents[index]} : std::nullopt));
      CHECK((**revision).references == *decoded->references);
      CHECK((**revision).references == f.refs[seq == 3 || seq == 4 ? 1 : 0]);
      CHECK((**revision).actor == "driver:history");
      CHECK((**revision).accepted_at == std::format("2026-01-0{}T00:00:00.000Z", seq == 4 ? 8 : 9 - index));
      auto request = owner->read_request(std::format("request:{}", seq - 1));
      REQUIRE(request);
      REQUIRE(*request);
      CHECK((**request).request_id == std::format("request:{}", seq - 1));
      CHECK((**request).body_reference.id == std::uint64_t(f.bodies[index]));
      CHECK((**request).body_reference.kind == record_kind::request);
      CHECK((**request).body_encoded ==
            c1_sql_bytes(f.path, std::format("SELECT encoded FROM records WHERE id={}", f.bodies[index])));
      CHECK((**request).result ==
            c1_sql_bytes(f.path, std::format("SELECT result FROM requests WHERE request_id='request:{}'", seq - 1)));
      auto body = decode_record((**request).body_encoded);
      REQUIRE(body);
      auto typed = std::get_if<request_record>(&*body);
      REQUIRE(typed);
      CHECK(typed->workspace_argument == "workspace.lua");
      CHECK(typed->registry_argument == "registry.json");
      auto digest = digest_record(*body);
      REQUIRE(digest);
      CHECK((**request).body_reference.digest == *digest);
      CHECK((**request).event_sequence == seq);
      CHECK((**request).revision == ids[index]);
      CHECK((**request).outcome == (seq == 4 ? load_outcome::noop : load_outcome::accepted));
      auto saved = decode_load_result((**request).result);
      REQUIRE(saved);
      CHECK(saved->parent == (parents[index] ? std::optional{parents[index]} : std::nullopt));
    }
  }
}
TEST_CASE("ledger typed reads distinguish optional misses from invalid arguments and retain owned rows",
          "[ledger][history][typed]") {
  fixture                     f;
  std::optional<request_row>  retained;
  std::optional<revision_row> revision;
  event_page                  page;
  {
    auto opened = open_existing(f.path);
    REQUIRE(opened);
    auto owner = std::move(*opened);
    CHECK_FALSE(opened->read_head());
    CHECK_FALSE(opened->read_revision(1));
    CHECK_FALSE(opened->read_event(1));
    CHECK_FALSE(opened->read_request("request:1"));
    CHECK_FALSE(opened->read_events_after(0, 1));
    for (auto id : {std::uint64_t{2}, std::uint64_t{std::numeric_limits<std::int64_t>::max()}}) {
      auto absent = owner.read_revision(id);
      REQUIRE(absent);
      CHECK_FALSE(*absent);
    }
    auto absent_event = owner.read_event(std::uint64_t(std::numeric_limits<std::int64_t>::max()));
    REQUIRE(absent_event);
    CHECK_FALSE(*absent_event);
    for (auto key : {std::string("absent"), std::string(128, 'z')}) {
      auto absent = owner.read_request(key);
      REQUIRE(absent);
      CHECK_FALSE(*absent);
    }
    auto result = owner.read_request("request:1");
    REQUIRE(result);
    retained = std::move(*result);
    auto row = owner.read_revision(1);
    REQUIRE(row);
    revision    = std::move(*row);
    auto events = owner.read_events_after(0, 2);
    REQUIRE(events);
    page = std::move(*events);
    REQUIRE(owner.read_record(std::uint64_t(f.refs[0].source)));
    REQUIRE(owner.read_head());
  }
  REQUIRE(retained);
  CHECK(decode_record(retained->body_encoded));
  CHECK(decode_load_result(retained->result));
  REQUIRE(revision);
  CHECK(revision->actor == "driver:history");
  REQUIRE(page.events.size() == 2);
  CHECK(decode_history_event(page.events.back().payload));
}
TEST_CASE("ledger typed pages enforce exclusive ascending bounded snapshot cursors", "[ledger][history][typed]") {
  fixture f;
  auto    body = c1_noop_body(f);
  {
    raw_db db(f.path);
    db.sql("BEGIN IMMEDIATE");
    for (std::int64_t seq = 6; seq <= 115; ++seq)
      c1_append_noop(db, f, body, seq);
    db.sql("COMMIT");
  }
  auto owner = open_existing(f.path);
  REQUIRE(owner);
  auto first = owner->read_events_after(0, 100);
  REQUIRE(first);
  REQUIRE(first->events.size() == 100);
  CHECK(first->next_after == 100);
  for (std::size_t i = 0; i < 100; ++i)
    CHECK(first->events[i].sequence == std::int64_t(i + 1));
  auto second = owner->read_events_after(std::uint64_t(*first->next_after), 100);
  REQUIRE(second);
  REQUIRE(second->events.size() == 15);
  CHECK(second->events.front().sequence == 101);
  CHECK(second->events.back().sequence == 115);
  CHECK_FALSE(second->next_after);
  for (auto cursor :
       {std::uint64_t{114}, std::uint64_t{115}, std::uint64_t{116}, std::uint64_t(std::numeric_limits<std::int64_t>::max())}) {
    auto page = owner->read_events_after(cursor, 1);
    REQUIRE(page);
    CHECK(page->events.size() == (cursor == 114 ? 1 : 0));
    CHECK_FALSE(page->next_after);
  }
  auto single = owner->read_events_after(0, 1);
  REQUIRE(single);
  REQUIRE(single->events.size() == 1);
  CHECK(single->events.front().sequence == 1);
  CHECK(single->next_after == 1);
}
TEST_CASE("ledger typed read failures invalidate every read method while cached metadata survives", "[ledger][history][typed]") {
  fixture f;
  for (int bad = 0; bad < 12; ++bad) {
    auto owner = open_existing(f.path);
    REQUIRE(owner);
    auto check = [&](const auto& result) {
      REQUIRE_FALSE(result);
      CHECK(result.error().code == (bad == 11 ? format_error_code::not_found : format_error_code::invalid_argument));
    };
    auto over = std::uint64_t(std::numeric_limits<std::int64_t>::max()) + 1;
    if (bad == 0)
      check(owner->read_revision(0));
    if (bad == 1)
      check(owner->read_revision(over));
    if (bad == 2)
      check(owner->read_event(0));
    if (bad == 3)
      check(owner->read_event(over));
    if (bad == 4)
      check(owner->read_request(""));
    if (bad == 5)
      check(owner->read_request(std::string(129, 'a')));
    if (bad == 6)
      check(owner->read_request("bad/key"));
    if (bad == 7)
      check(owner->read_request(std::string("bad\0key", 7)));
    if (bad == 8)
      check(owner->read_events_after(0, 0));
    if (bad == 9)
      check(owner->read_events_after(0, 101));
    if (bad == 10)
      check(owner->read_events_after(over, 1));
    if (bad == 11)
      check(owner->read_record(std::uint64_t(std::numeric_limits<std::int64_t>::max())));
    c1_check_unavailable(*owner, f);
  }
}
TEST_CASE("ledger typed reads permit raw unsupported interpretation without poisoning history", "[ledger][history][typed]") {
  fixture f("unknown_interpretation");
  auto    owner = open_existing(f.path);
  REQUIRE(owner);
  auto event = owner->read_event(4);
  REQUIRE(event);
  REQUIRE(*event);
  auto decoded = decode_history_event((**event).payload);
  REQUIRE(decoded);
  auto revision = owner->read_revision(std::uint64_t(*decoded->after));
  REQUIRE(revision);
  REQUIRE(*revision);
  auto raw      = owner->read_record(std::uint64_t((**revision).references.interpretation));
  auto versions = owner->read_record(std::uint64_t((**revision).references.versions));
  REQUIRE(raw);
  REQUIRE(versions);
  auto interpreted =
      decode_interpretation(std::get<value_record>(raw->value).encoded, std::get<value_record>(versions->value).encoded);
  REQUIRE_FALSE(interpreted);
  CHECK(interpreted.error().code == interpretation_error_code::unsupported_version);
  REQUIRE(owner->read_head());
  REQUIRE(owner->read_request("request:3"));
  REQUIRE(owner->read_events_after(0, 100));
}
TEST_CASE("ledger typed DELETE snapshot blocks commit until released then fresh reads observe later head",
          "[ledger][history][typed]") {
  fixture f;
  auto    body  = c1_noop_body(f);
  auto    owner = open_existing(f.path);
  REQUIRE(owner);
  std::optional<format_file> pinned(std::move(*owner));
  raw_db                     db(f.path);
  CHECK(c1_sql_bytes(f.path, "PRAGMA journal_mode") == "delete");
  db.sql("BEGIN IMMEDIATE");
  c1_append_noop(db, f, body, 6);
  REQUIRE(sqlite3_exec(db.db, "COMMIT", nullptr, nullptr, nullptr) == SQLITE_BUSY);
  auto head = pinned->read_head();
  REQUIRE(head);
  CHECK(head->last_sequence == 5);
  auto absent = pinned->read_event(6);
  REQUIRE(absent);
  CHECK_FALSE(*absent);
  auto request = pinned->read_request("later:6");
  REQUIRE(request);
  CHECK_FALSE(*request);
  auto tail = pinned->read_events_after(4, 100);
  REQUIRE(tail);
  REQUIRE(tail->events.size() == 1);
  CHECK(tail->events.front().sequence == 5);
  CHECK_FALSE(tail->next_after);
  pinned.reset();
  db.sql("COMMIT");
  auto fresh = open_existing(f.path);
  REQUIRE(fresh);
  head = fresh->read_head();
  REQUIRE(head);
  CHECK(head->revision == 8);
  CHECK(head->last_sequence == 6);
  request = fresh->read_request("later:6");
  REQUIRE(request);
  REQUIRE(*request);
  CHECK((**request).event_sequence == 6);
  CHECK(c1_sql_bytes(f.path, "PRAGMA journal_mode") == "delete");
}

namespace {
struct c1_fault_vfs {
  struct file {
    sqlite3_file  base;
    sqlite3_file* real;
    c1_fault_vfs* owner;
    bool          journal;
  };
  static inline c1_fault_vfs* active   = nullptr;
  sqlite3_vfs*                original = sqlite3_vfs_find(nullptr);
  sqlite3_vfs                 wrapper  = *original;
  sqlite3_io_methods          methods{};
  bool                        armed = false;
  int                         hits  = 0;
  c1_fault_vfs() {
    REQUIRE_FALSE(active);
    active           = this;
    methods.iVersion = 1;
    methods.xClose   = [](sqlite3_file* p) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xClose(f.real);
    };
    methods.xRead = [](sqlite3_file* p, void* b, int n, sqlite3_int64 o) {
      auto& f = *reinterpret_cast<file*>(p);
      if (f.owner->armed && !f.journal) {
        f.owner->armed = false;
        ++f.owner->hits;
        std::println("R14-C1 injected SQLITE_IOERR_READ offset={} bytes={}", o, n);
        return SQLITE_IOERR_READ;
      }
      return f.real->pMethods->xRead(f.real, b, n, o);
    };
    methods.xWrite = [](sqlite3_file* p, const void* b, int n, sqlite3_int64 o) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xWrite(f.real, b, n, o);
    };
    methods.xTruncate = [](sqlite3_file* p, sqlite3_int64 n) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xTruncate(f.real, n);
    };
    methods.xSync = [](sqlite3_file* p, int flags) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xSync(f.real, flags);
    };
    methods.xFileSize = [](sqlite3_file* p, sqlite3_int64* n) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xFileSize(f.real, n);
    };
    methods.xLock = [](sqlite3_file* p, int n) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xLock(f.real, n);
    };
    methods.xUnlock = [](sqlite3_file* p, int n) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xUnlock(f.real, n);
    };
    methods.xCheckReservedLock = [](sqlite3_file* p, int* n) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xCheckReservedLock(f.real, n);
    };
    methods.xFileControl = [](sqlite3_file* p, int op, void* arg) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xFileControl(f.real, op, arg);
    };
    methods.xSectorSize = [](sqlite3_file* p) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xSectorSize(f.real);
    };
    methods.xDeviceCharacteristics = [](sqlite3_file* p) {
      auto& f = *reinterpret_cast<file*>(p);
      return f.real->pMethods->xDeviceCharacteristics(f.real);
    };
    wrapper.zName    = "planar-r14c1-read-test";
    wrapper.szOsFile = static_cast<int>(sizeof(file) + alignof(std::max_align_t) + original->szOsFile);
    wrapper.xOpen    = [](sqlite3_vfs*, const char* name, sqlite3_file* p, int flags, int* out) {
      auto& f       = *reinterpret_cast<file*>(p);
      auto  address = reinterpret_cast<std::uintptr_t>(p) + sizeof(file);
      address       = (address + alignof(std::max_align_t) - 1) & ~(alignof(std::max_align_t) - 1);
      f.real        = reinterpret_cast<sqlite3_file*>(address);
      f.owner       = active;
      f.journal     = (flags & SQLITE_OPEN_MAIN_JOURNAL) != 0;
      const int rc  = active->original->xOpen(active->original, name, f.real, flags, out);
      if (rc == SQLITE_OK)
        f.base.pMethods = &active->methods;
      return rc;
    };
    REQUIRE(sqlite3_vfs_register(&wrapper, 1) == SQLITE_OK);
  }
  ~c1_fault_vfs() {
    sqlite3_vfs_register(original, 1);
    sqlite3_vfs_unregister(&wrapper);
    active = nullptr;
  }
};

} // namespace

TEST_CASE("ledger typed request read fault invalidates all methods after real SQLite IOERR", "[ledger][history][typed]") {
  fixture f;
  auto    body = c1_noop_body(f, std::string(8 * 1024 * 1024, 'q'));
  {
    raw_db db(f.path);
    db.sql("BEGIN IMMEDIATE");
    c1_append_noop(db, f, body, 6);
    db.sql("COMMIT");
  }
  c1_fault_vfs vfs;
  auto         owner = open_existing(f.path);
  REQUIRE(owner);
  vfs.armed   = true;
  auto result = owner->read_request("later:6");
  REQUIRE_FALSE(result);
  CHECK(result.error().code == format_error_code::io);
  CHECK(vfs.hits == 1);
  c1_check_unavailable(*owner, f);
  // A fresh verified owner can read the unchanged valid bytes after the one-shot I/O fault.
  auto fresh = open_existing(f.path);
  REQUIRE(fresh);
  auto replay = fresh->read_request("later:6");
  REQUIRE(replay);
  REQUIRE(*replay);
  auto decoded = decode_record((**replay).body_encoded);
  REQUIRE(decoded);
  CHECK(std::get<request_record>(*decoded).workspace_argument.size() == 8 * 1024 * 1024);
}
TEST_CASE("ledger typed initialized snapshot has absent head and revision but owned initialization event",
          "[ledger][history][typed]") {
  std::string pattern = (std::filesystem::temp_directory_path() / "planar-c1-initial-XXXXXX").string();
  REQUIRE(mkdtemp(pattern.data()));
  struct cleanup {
    std::filesystem::path root;
    ~cleanup() {
      std::filesystem::remove_all(root);
    }
  } scratch{std::filesystem::canonical(pattern)};
  auto path     = scratch.root / "ledger";
  auto metadata = initialize_new(path, "empty");
  REQUIRE(metadata);
  auto owner = open_existing(path);
  REQUIRE(owner);
  auto head = owner->read_head();
  REQUIRE(head);
  CHECK_FALSE(head->revision);
  CHECK(head->last_sequence == 1);
  auto revision = owner->read_revision(1);
  REQUIRE(revision);
  CHECK_FALSE(*revision);
  auto event = owner->read_event(1);
  REQUIRE(event);
  REQUIRE(*event);
  auto encoded = encode_history_event({{}, {}, event_reason::initialized, {}});
  REQUIRE(encoded);
  CHECK((**event).payload == *encoded);
  auto page = owner->read_events_after(0, 100);
  REQUIRE(page);
  REQUIRE(page->events.size() == 1);
  CHECK_FALSE(page->next_after);
  auto request = owner->read_request("first-load");
  REQUIRE(request);
  CHECK_FALSE(*request);
  REQUIRE(owner->read_head());
}

namespace {
// Relational fixture publication only: native format2 acceptance is deliberately unavailable in B2-A.
void b2a_format2(fixture& f) {
  auto target = f.root / "format2";
  auto meta   = initialize_new_v2(target, f.metadata.workspace_id);
  REQUIRE(meta);
  {
    raw_db db(target);
    db.sql(std::format("ATTACH DATABASE '{}' AS fixture", f.path.string()));
    db.sql("BEGIN IMMEDIATE");
    for (auto table : {"records", "tree_edges", "revisions"})
      db.sql(std::format("INSERT INTO {} SELECT * FROM fixture.{}", table, table));
    db.sql("INSERT INTO events SELECT * FROM fixture.events WHERE seq>1");
    db.sql(std::format("INSERT INTO requests SELECT request_id,body_id,event_seq,revision_id,outcome,CAST(replace(CAST(result AS "
                       "TEXT),'{}','{}') AS BLOB) FROM fixture.requests",
                       f.metadata.identity, meta->identity));
    db.sql("UPDATE current_state SET active_revision_id=8,last_seq=5");
    db.sql("COMMIT");
  }
  std::filesystem::remove(f.path);
  f.path     = target;
  f.metadata = *meta;
}
void b2a_replace(fixture& f, std::size_t index, request_v2 body, int codec = 2, int kind = 4, std::string_view mutation = {}) {
  auto encoded = encode_request_v2(body);
  auto digest  = digest_request_v2(body);
  REQUIRE(encoded);
  REQUIRE(digest);
  if (mutation == "trailing")
    *encoded += 'x';
  if (mutation == "version")
    (*encoded)[13] = 3;
  if (mutation == "tag")
    encoded->back() = 2;
  if (mutation == "length")
    (*encoded)[14] = char(0xff);
  if (mutation == "digest")
    (*digest)[0] ^= 1;
  raw_db db(f.path);
  db.sql("PRAGMA ignore_check_constraints=ON");
  db.sql("DROP TRIGGER records_no_update");
  db.bind_insert(
      std::format("UPDATE records SET kind={},codec={},digest=?1,encoded=?2 WHERE id={}", kind, codec, f.bodies[index]),
      {std::string_view(reinterpret_cast<const char*>(digest->data()), digest->size()), *encoded});
  db.sql("CREATE TRIGGER records_no_update BEFORE UPDATE ON records BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END");
}
request_v2 b2a_body(std::size_t index = 0) {
  const std::array<std::string, 4> heads{"none", "1", "3", "3"};
  return {"load", "driver:history", heads[index], "/private/workspace-secret", "/private/registry-secret", {}};
}
void b2a_refuse(fixture& f, format_error_code code) {
  auto before = bytes(f.path);
  auto result = open_existing(f.path);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == code);
  CHECK(result.error().message.find("secret") == std::string::npos);
  CHECK(result.error().message.find("/private") == std::string::npos);
  CHECK_FALSE(result.error().detail);
  CHECK(bytes(f.path) == before);
  CHECK_FALSE(std::filesystem::exists(f.path.string() + "-journal"));
}
} // namespace
TEST_CASE("ledger format2 mixed request codecs preserve recorded history export and privacy",
          "[ledger][history][format2][request_v2]") {
  fixture f;
  b2a_format2(f);
  for (std::size_t i : {0u, 2u})
    b2a_replace(f, i, b2a_body(i));
  const auto before = bytes(f.path);
  {
    auto file = open_existing(f.path);
    REQUIRE(file);
    for (std::size_t i = 0; i < 4; ++i) {
      auto row = file->read_request(std::format("request:{}", i + 1));
      REQUIRE(row);
      REQUIRE(*row);
      auto stored = file->read_record(std::uint64_t(f.bodies[i]));
      REQUIRE(stored);
      CHECK(std::holds_alternative<request_v2>(stored->value) == (i != 1));
      CHECK(types::encode_stored_record(*stored).value() == (**row).body_encoded);
      if (i != 1) {
        auto body = b2a_body(i);
        CHECK((**row).body_encoded != encode_record(request_record{body.operation, body.actor, body.expected_head,
                                                                   body.workspace_argument, body.registry_argument})
                                          .value());
      }
    }
    for (auto selector :
         {history_selector::page, history_selector::revision, history_selector::request, history_selector::state}) {
      auto result = read_history(*file, {selector, 1, 50, "request:1"});
      REQUIRE(result);
      auto json = codec_encode(*result);
      REQUIRE(json);
      CHECK(json->find("secret") == std::string::npos);
      CHECK(json->find("/private") == std::string::npos);
    }
  }
  auto exported = source_export::export_source(f.path, 1, f.root / "exported");
  REQUIRE(exported);
  CHECK(bytes(f.root / "exported/workspace.lua") ==
        bytes(std::filesystem::path(PLANAR_HISTORY_FIXTURE) / "workspace/workspace.lua"));
  CHECK(bytes(f.path) == before);
  {
    auto writer = begin_records(f.path);
    REQUIRE(writer);
    writer->abort();
  }
  {
    auto writable = open_existing(f.path, ledger_access::read_write);
    REQUIRE(writable);
  }
  CHECK(bytes(f.path) == before);
  REQUIRE(backup_new(f.path, f.root / "backup"));
  REQUIRE(restore_new(f.root / "backup", f.root / "restored"));
  REQUIRE(open_existing(f.root / "restored"));
}
TEST_CASE("ledger format2 request-v2 malformed and mismatched rows refuse unchanged privately",
          "[ledger][history][format2][request_v2]") {
  for (std::string mode : {"trailing", "version", "tag", "length", "digest", "actor", "head", "kind", "codec", "v1codec", "event",
                           "result", "reference", "profile", "format1"}) {
    INFO(mode);
    fixture f;
    if (mode != "format1")
      b2a_format2(f);
    auto body = b2a_body();
    if (mode == "actor")
      body.actor = "wrong";
    if (mode == "head")
      body.expected_head = "1";
    if (mode == "profile")
      body.profile_argument = "/private/profile-secret";
    b2a_replace(f, 0, body, mode == "codec" ? 3 : mode == "v1codec" ? 1 : 2, mode == "kind" ? 1 : 4, mode);
    if (mode == "event") {
      raw_db db(f.path);
      db.mutate("events", "UPDATE events SET actor='other' WHERE seq=2");
    }
    if (mode == "result") {
      raw_db db(f.path);
      db.mutate("requests", "UPDATE requests SET outcome='noop' WHERE request_id='request:1'");
    }
    if (mode == "reference") {
      raw_db db(f.path);
      db.mutate("revisions", std::format("UPDATE revisions SET source_id={} WHERE id=1", f.bodies[0]));
    }
    b2a_refuse(f, mode == "codec" ? format_error_code::unsupported_state : format_error_code::corrupt);
  }
}
TEST_CASE("ledger request children of either codec refuse format2 directories", "[ledger][format2][request_v2]") {
  for (int mode : {0, 1, 2}) {
    fixture f;
    if (mode != 0)
      b2a_format2(f);
    bool codec2 = mode == 2;
    if (codec2)
      b2a_replace(f, 0, b2a_body());
    auto body = open_existing(f.path);
    REQUIRE(body);
    auto child = body->read_record(std::uint64_t(f.bodies[0]));
    REQUIRE(child);
    auto ref = child->reference;
    body     = std::unexpected(format_error{format_error_code::corrupt, "released"});
    directory_record directory{{{"request", record_kind::request, ref.id, ref.digest}}};
    auto             encoded = encode_record(directory);
    auto             digest  = digest_record(directory);
    REQUIRE(encoded);
    REQUIRE(digest);
    {
      raw_db db(f.path);
      db.bind_insert("INSERT INTO records VALUES(999999,2,1,?1,?2)",
                     {std::string_view(reinterpret_cast<const char*>(digest->data()), digest->size()), *encoded});
      db.sql(std::format("INSERT INTO tree_edges VALUES(999999,0,'request',{})", ref.id));
    }
    if (mode == 0) {
      REQUIRE(open_existing(f.path)); // Preserve the v1 generic record graph contract.
    } else {
      b2a_refuse(f, format_error_code::corrupt);
    }
  }
}

TEST_CASE("recorded profile history reconstructs effective v2 and projects every selector",
          "[ledger][history][profile_history]") {
  fixture f("profile");
  auto    file = open_existing(f.path);
  REQUIRE(file);
  for (auto selector : {history_selector::page, history_selector::revision, history_selector::request, history_selector::state}) {
    auto result = read_history(*file, {selector, 1, 50, "request:1"});
    REQUIRE(result);
    auto json = codec_encode(*result);
    REQUIRE(json);
    CHECK(json->find("/private") == std::string::npos);
  }
}

TEST_CASE("recorded profile unknown interpretation preserves raw export without semantic payload",
          "[ledger][history][profile_history]") {
  fixture f("profile_unknown_interpretation");
  auto    file = open_existing(f.path);
  REQUIRE(file);
  auto result = read_history(*file, {history_selector::revision, 1, 50, {}});
  REQUIRE_FALSE(result);
  CHECK(result.error().code == history_read_code::unsupported_interpretation);
  for (auto selector : {history_selector::page, history_selector::state, history_selector::request})
    REQUIRE(read_history(*file, {selector, 3, 50, "request:3"}));
  REQUIRE(source_export::export_source(f.path, 1, f.root / "exported"));
  CHECK(bytes(f.root / "exported/workspace.lua") ==
        bytes(std::filesystem::path(PLANAR_HISTORY_FIXTURE) / "workspace/workspace.lua"));
  CHECK_FALSE(std::filesystem::exists(f.root / "exported/profile"));
  CHECK_FALSE(std::filesystem::exists(f.root / "exported/external"));
}
TEST_CASE("recorded profile actual raw tree workspace digest catalog and selection forgeries refuse",
          "[ledger][history][profile_history]") {
  for (auto mode : {"profile_tree", "profile_workspace", "profile_unknown_workspace", "profile_digest", "profile_catalog",
                    "profile_selection", "profile_versions", "profile_malformed", "profile_resource"}) {
    INFO(mode);
    fixture f(mode);
    f.refuse();
  }
}
TEST_CASE("recorded profile unknown manifest refuses typed unsupported before export", "[ledger][history][profile_history]") {
  fixture f("profile_unknown_manifest");
  f.refuse(format_error_code::unsupported_state);
  auto result = source_export::export_source(f.path, 1, f.root / "exported");
  REQUIRE_FALSE(result);
  CHECK_FALSE(std::filesystem::exists(f.root / "exported"));
}
TEST_CASE("recorded history requires request selector family on accepted and noop events", "[ledger][history][profile_history]") {
  for (bool profile : {false, true}) {
    for (std::size_t index : {0u, 2u}) {
      INFO(profile << " index=" << index);
      fixture f(profile ? "profile" : "");
      if (!profile)
        b2a_format2(f);
      auto body = b2a_body(index);
      if (!profile)
        body.profile_argument = "/private/profile-secret";
      b2a_replace(f, index, body);
      f.refuse();
    }
  }
}
TEST_CASE("recorded effective provenance refuses request v1 independently of format2 schema",
          "[ledger][history][profile_history]") {
  for (std::size_t index : {0u, 2u}) {
    fixture        f("profile");
    auto           typed = b2a_body(index);
    request_record body{typed.operation, typed.actor, typed.expected_head, typed.workspace_argument, typed.registry_argument};
    auto           encoded = encode_record(body);
    auto           digest  = digest_record(body);
    REQUIRE(encoded);
    REQUIRE(digest);
    raw_db db(f.path);
    db.sql("DROP TRIGGER records_no_update");
    db.bind_insert(std::format("UPDATE records SET codec=1,digest=?1,encoded=?2 WHERE id={}", f.bodies[index]),
                   {std::string_view(reinterpret_cast<const char*>(digest->data()), digest->size()), *encoded});
    db.sql("CREATE TRIGGER records_no_update BEFORE UPDATE ON records BEGIN SELECT RAISE(ABORT,'immutable ledger row'); END");
    f.refuse();
  }
}

TEST_CASE("recorded effective history selects wider fixed tree profile for valid closure above4096 files",
          "[ledger][history][profile_history]") {
  fixture f("profile_large");
  auto    file = open_existing(f.path);
  REQUIRE(file);
  auto revision = read_history(*file, {history_selector::revision, 1, 50, {}});
  REQUIRE(revision);
  REQUIRE(source_export::export_source(f.path, 1, f.root / "exported"));
}

TEST_CASE("recorded profile saved accepted and noop replay precede unusable preparation while fresh profile stays refused",
          "[ledger][history][profile_history]") {
  using namespace planar::engine::acceptance;
  fixture f("profile");
  auto    scratch = f.root / "candidate";
  std::filesystem::create_directories(scratch / "workspace");
  {
    std::ofstream out(scratch / "workspace/workspace.lua");
    out << "return {}";
  }
  {
    std::ofstream out(scratch / "registry.json");
    out << "{}";
  }
  native_capture_session capture;
  REQUIRE(capture.add_root("workspace", scratch / "workspace"));
  REQUIRE(capture.tree("workspace"));
  REQUIRE(capture.add_input("registry", scratch / "registry.json"));
  REQUIRE(capture.read("registry", "input.json", false));
  auto sealed = capture.finish_for_acceptance();
  REQUIRE(sealed);
  auto stdlib = prepare_embedded_stdlib();
  REQUIRE(stdlib);
  auto goldens = std::filesystem::path(PLANAR_HISTORY_GOLDENS);
  auto decoded = decode_interpretation(bytes(goldens / "interpretation.canonical"), bytes(goldens / "versions.canonical"));
  REQUIRE(decoded);
  auto interpretation = encode_interpretation(decoded->interpretation->fields.at("bundle"),
                                              decoded->interpretation->fields.at("semantic"), hex(stdlib->digest));
  REQUIRE(interpretation);
  std::array paths{std::string_view("workspace.lua")};
  auto       prepared = prepared_acceptance::create(std::move(*sealed), paths, stdlib->files, interpretation->encoded,
                                                    interpretation->versions_encoded);
  REQUIRE(prepared);
  auto retained = std::move(*prepared);
  std::filesystem::remove_all(scratch);
  CHECK_FALSE(prepared->data());
  CHECK_FALSE(std::filesystem::exists(f.workspace_selector));
  CHECK_FALSE(std::filesystem::exists(f.registry_selector));
  CHECK_FALSE(std::filesystem::exists(f.profile_selector));
  auto original = bytes(f.path);
  for (auto index : {1, 3}) {
    auto request = request_intent::create_v2(std::format("request:{}", index), "driver:history",
                                             index == 1 ? std::optional<std::int64_t>{} : std::optional<std::int64_t>{3},
                                             f.workspace_selector, f.registry_selector, f.profile_selector);
    REQUIRE(request);
    auto file = open_existing(f.path);
    REQUIRE(file);
    auto saved = file->read_request(std::format("request:{}", index));
    REQUIRE(saved);
    REQUIRE(*saved);
    auto replay = complete_acceptance(f.path, *request, *prepared);
    REQUIRE(replay);
    CHECK(replay->replayed);
    CHECK(replay->result == (**saved).result);
    completion_receipt receipt;
    auto               locked = complete_acceptance(f.path, *request, *prepared, f.metadata.identity, receipt);
    REQUIRE(locked);
    CHECK(locked->replayed);
    CHECK(receipt.phase == completion_phase::replayed);
    CHECK(locked->result == receipt.saved_result);
  }
  auto fresh = request_intent::create_v2("fresh-profile", "driver:history", {}, f.workspace_selector, f.registry_selector,
                                         f.profile_selector);
  REQUIRE(fresh);
  auto refused = complete_acceptance(f.path, *fresh, *prepared);
  REQUIRE_FALSE(refused);
  CHECK(refused.error().code == format_error_code::unsupported_state);
  CHECK(bytes(f.path) == original);
}
