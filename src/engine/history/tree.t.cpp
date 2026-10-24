// Complete immutable tree checks use only captured views and independently owned scratch ledgers.
#include <catch2/catch_test_macros.hpp>
#include <unistd.h>
import std;
import planar.engine.history.tree;
import planar.engine.ledger.sqlite;
using namespace planar::engine::history;
using namespace planar::engine::ledger;
namespace {
constexpr auto source     = tree_profile::source;
constexpr auto provenance = tree_profile::provenance;
struct scratch {
  std::filesystem::path root;
  std::filesystem::path db;
  scratch() {
    char pattern[] = "/tmp/planar-tree-XXXXXX";
    REQUIRE(::mkdtemp(pattern));
    root = std::filesystem::canonical(pattern);
    db   = root / "db";
    REQUIRE(initialize_new(db, "tree_test"));
  }
  ~scratch() {
    std::filesystem::remove_all(root);
  }
};
record_writer writer_for(const scratch& f) {
  auto writer = begin_records(f.db);
  REQUIRE(writer);
  return std::move(*writer);
}
record_reference build(record_writer& writer, std::span<const tree_file_view> files, tree_profile profile = source) {
  auto root = build_tree(writer, files, profile);
  REQUIRE(root);
  auto digest = digest_tree(files, profile);
  REQUIRE(digest);
  CHECK(*digest == root->digest);
  return *root;
}
record_reference intern(record_writer& writer, const record& value) {
  auto ref = writer.intern(value);
  REQUIRE(ref);
  return *ref;
}
record_child child(std::string name, record_reference ref) {
  return {std::move(name), ref.kind, ref.id, ref.digest};
}
void empty_after_failure(const scratch& f, record_writer& writer) {
  REQUIRE_FALSE(writer.commit());
  auto reader = open_existing(f.db);
  REQUIRE(reader);
  auto missing = reader->read_record(1);
  REQUIRE_FALSE(missing);
  CHECK(missing.error().code == format_error_code::not_found);
}
record_reference chain(record_writer& writer, record_reference leaf, std::size_t count) {
  for (std::size_t i = 0; i < count; ++i)
    leaf = intern(writer, directory_record{{child("a", leaf)}});
  return leaf;
}
std::vector<tree_file_view> views(const std::vector<std::string>& paths, std::string_view payload = {}) {
  std::vector<tree_file_view> out;
  for (const auto& path : paths)
    out.push_back({path, payload});
  return out;
}
} // namespace
TEST_CASE("tree exact bytes modes sorted topology and immutable history", "[tree]") {
  scratch           f;
  record_reference  old{}, fresh{};
  const std::string binary("\0\xff\r\n", 4);
  const std::array  files{tree_file_view{"z", "", 420}, tree_file_view{"src/run", binary, 493},
                          tree_file_view{"src/a", "a\r\n", 420}};
  {
    auto writer = writer_for(f);
    old         = build(writer, files);
    CHECK(build(writer, files).id == old.id);
    auto out = read_tree(writer, old, source);
    REQUIRE(out);
    REQUIRE(out->size() == 3);
    CHECK((*out)[0].path == "src/a");
    CHECK((*out)[1].payload == binary);
    CHECK((*out)[1].mode == 493);
    CHECK((*out)[2].path == "z");
    CHECK((*out)[2].payload.empty());
    auto stored = writer.read_record(old.id);
    REQUIRE(stored);
    CHECK(std::get<directory_record>(stored->value).children[0].name == "src");
    REQUIRE(writer.commit());
  }
  {
    auto writer = writer_for(f);
    // No timestamp or host metadata exists at this boundary. Rename, delete and same-length edit are exact bytes.
    const std::array edited{tree_file_view{"renamed/run", binary, 493}, tree_file_view{"renamed/a", "b\r\n", 420}};
    fresh = build(writer, edited);
    CHECK(fresh.digest != old.digest);
    REQUIRE(writer.commit());
  }
  std::vector<tree_file> owned;
  {
    auto reader = open_existing(f.db);
    REQUIRE(reader);
    auto original = read_tree(*reader, old, source);
    REQUIRE(original);
    CHECK(original->size() == 3);
    CHECK((*original)[0].payload == "a\r\n");
    owned        = std::move(*original);
    auto updated = read_tree(*reader, fresh, source);
    REQUIRE(updated);
    CHECK(updated->size() == 2);
    CHECK((*updated)[0].path == "renamed/a");
  }
  CHECK(owned[1].payload == binary);
  scratch other;
  auto    writer = writer_for(other);
  intern(writer, file_record{420, "unrelated ID displacement"});
  CHECK(build(writer, files).digest == old.digest);
  const std::array mode_edit{tree_file_view{"z", "", 493}, files[1], files[2]};
  CHECK(build(writer, mode_edit).digest != old.digest);
}
TEST_CASE("tree empty root composition explicit abort and abandonment", "[tree]") {
  scratch f;
  {
    auto writer = writer_for(f);
    auto root   = build(writer, {});
    auto out    = read_tree(writer, root, source);
    REQUIRE(out);
    CHECK(out->empty());
    intern(writer, value_record{"canonical_v1", std::string(1, '\0')});
    const std::array files{tree_file_view{"x", "data"}};
    build(writer, files, provenance);
    const std::array mixed{tree_file_view{"a", "lower"}, tree_file_view{"B", "upper"}};
    auto             mixed_root = build(writer, mixed);
    auto             sorted     = read_tree(writer, mixed_root, source);
    REQUIRE(sorted);
    CHECK(sorted->at(0).path == "B");
    CHECK(sorted->at(1).path == "a");
    REQUIRE(writer.commit());
  }
}
TEST_CASE("tree pending records rollback through idempotent abort and abandoned writer", "[tree]") {
  scratch f;
  {
    auto writer = writer_for(f);
    build(writer, {});
    writer.abort();
    writer.abort();
    empty_after_failure(f, writer);
  }
  {
    auto writer = writer_for(f);
    build(writer, {});
  }
  auto writer = writer_for(f);
  writer.abort();
  empty_after_failure(f, writer);
}
TEST_CASE("tree input duplicate aliases conflicts grammar and modes abort prior pending records", "[tree]") {
  const std::vector<std::vector<tree_file_view>> cases{{{"a", "x"}, {"a", "x"}},
                                                       {{"A/x", ""}, {"a/y", ""}},
                                                       {{"a", ""}, {"a/b", ""}},
                                                       {{"a/b", ""}, {"a", ""}},
                                                       {{"a/X", ""}, {"a/x", ""}},
                                                       {{"../x", ""}},
                                                       {{".git/x", ""}},
                                                       {{"/x", ""}},
                                                       {{"a//b", ""}},
                                                       {{"a/", ""}},
                                                       {{"a", "", 0600}},
                                                       {{"", ""}}};
  for (const auto& files : cases) {
    scratch f;
    auto    writer = writer_for(f);
    build(writer, {});
    auto result = build_tree(writer, files, source);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == tree_error_code::invalid_tree);
    auto pure = digest_tree(files, source);
    REQUIRE_FALSE(pure);
    CHECK(pure.error().code == result.error().code);
    empty_after_failure(f, writer);
  }
}
TEST_CASE("tree typed roots digest references and non-tree children refuse atomically", "[tree]") {
  for (unsigned scenario = 0; scenario < 7; ++scenario) {
    scratch          f;
    auto             writer = writer_for(f);
    auto             file   = intern(writer, file_record{420, "bytes"});
    auto             empty  = build(writer, {});
    record_reference root   = empty;
    switch (scenario) {
    case 0:
      root = file;
      break;
    case 1:
      root.digest[0] ^= 1;
      break;
    case 2:
      root.id = file.id;
      break;
    case 3:
      root.id = 99999;
      break;
    case 4:
      root = intern(writer, directory_record{{child("empty", empty)}});
      break;
    case 5: {
      auto value = intern(writer, value_record{"canonical_v1", std::string(1, '\0')});
      root       = intern(writer, directory_record{{child("value", value)}});
      break;
    }
    case 6: {
      auto request = intern(writer, request_record{"load", "driver", "none", "workspace", "registry"});
      root         = intern(writer, directory_record{{child("request", request)}});
      break;
    }
    }
    auto result = read_tree(writer, root, source);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == (scenario == 3 ? tree_error_code::storage : tree_error_code::invalid_tree));
    if (scenario == 3) {
      REQUIRE(result.error().storage);
      CHECK(result.error().storage->code == format_error_code::not_found);
    }
    empty_after_failure(f, writer);
  }
}
TEST_CASE("tree immutable reader malformed roots fail without successful partial output", "[tree]") {
  scratch          f;
  record_reference root{};
  {
    auto writer = writer_for(f);
    root        = build(writer, {});
    REQUIRE(writer.commit());
  }
  auto reader = open_existing(f.db);
  REQUIRE(reader);
  auto bad = root;
  bad.digest[3] ^= 1;
  auto result = read_tree(*reader, bad, source);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == tree_error_code::invalid_tree);
  CHECK(read_tree(*reader, root, source)->empty());
}
TEST_CASE("tree exact and excessive byte budgets source and provenance", "[tree]") {
  const std::string payload(16 * 1024 * 1024, 'x');
  for (auto profile : {source, provenance}) {
    scratch           f;
    auto              writer = writer_for(f);
    const std::array  files{tree_file_view{"a", payload}, tree_file_view{"b", payload}, tree_file_view{"c", payload},
                            tree_file_view{"d", payload}};
    const std::size_t count = profile == source ? 4 : 1;
    auto              root  = build(writer, std::span(files).first(count), profile);
    auto              out   = read_tree(writer, root, profile);
    REQUIRE(out);
    CHECK(out->size() == count);
    CHECK(out->back().payload.size() == payload.size());
    auto too_many = std::vector<tree_file_view>(files.begin(), files.begin() + count);
    too_many.push_back({"e", "!"});
    auto result = build_tree(writer, too_many, profile);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == tree_error_code::resource_limit);
    empty_after_failure(f, writer);
  }
  scratch           f;
  auto              writer = writer_for(f);
  const std::string excessive_payload(16 * 1024 * 1024 + 1, 'x');
  const std::array  files{tree_file_view{"a", excessive_payload}};
  auto              result = build_tree(writer, files, source);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == tree_error_code::resource_limit);
}
TEST_CASE("tree exact and excessive file bounds for both profiles and expanded shared records", "[tree]") {
  std::vector<std::string> paths;
  for (unsigned i = 0; i < 4097; ++i)
    paths.push_back(std::format("f{:04}", i));
  auto files = views(paths, "shared");
  for (auto profile : {source, provenance}) {
    scratch f;
    auto    writer = writer_for(f);
    auto    root   = build(writer, std::span(files).first(4096), profile);
    auto    out    = read_tree(writer, root, profile);
    REQUIRE(out);
    CHECK(out->size() == 4096);
    auto too_many = build_tree(writer, files, profile);
    REQUIRE_FALSE(too_many);
    CHECK(too_many.error().code == tree_error_code::resource_limit);
    empty_after_failure(f, writer);
    auto             next = writer_for(f);
    auto             leaf = intern(next, file_record{420, "shared"});
    directory_record expanded;
    for (const auto& path : paths)
      expanded.children.push_back(child(path, leaf));
    auto over   = intern(next, expanded);
    auto result = read_tree(next, over, profile);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == tree_error_code::resource_limit);
    empty_after_failure(f, next);
  }
}
TEST_CASE("tree exact and excessive path length depth and portable segment boundaries", "[tree]") {
  std::string depth;
  for (unsigned i = 0; i < 63; ++i)
    depth += "a/";
  depth += 'f';
  std::string length;
  for (unsigned i = 0; i < 7; ++i)
    length += std::string(128, 'a') + '/';
  length += std::string(121, 'b');
  REQUIRE(length.size() == 1024);
  for (auto profile : {source, provenance}) {
    for (const auto& path : {depth, length}) {
      scratch          f;
      auto             writer = writer_for(f);
      const std::array files{tree_file_view{path, "x"}};
      auto             root = build(writer, files, profile);
      auto             out  = read_tree(writer, root, profile);
      REQUIRE(out);
      CHECK(out->at(0).path == path);
    }
    for (const auto& path : {depth + "/x", length + "b"}) {
      scratch          f;
      auto             writer = writer_for(f);
      const std::array files{tree_file_view{path, ""}};
      auto             result = build_tree(writer, files, profile);
      REQUIRE_FALSE(result);
      CHECK(result.error().code == tree_error_code::resource_limit);
    }
    scratch f;
    auto    writer   = writer_for(f);
    auto    leaf     = intern(writer, file_record{});
    auto    too_deep = chain(writer, leaf, 65);
    auto    result   = read_tree(writer, too_deep, profile);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == tree_error_code::resource_limit);
    empty_after_failure(f, writer);
  }
}
TEST_CASE("tree exact source directories and independent provenance directory allowance", "[tree]") {
  std::vector<std::string> paths;
  for (unsigned i = 0; i < 130; ++i) {
    auto path = std::format("d{:03}/", i);
    for (unsigned j = 0; j < 62; ++j)
      path += "a/";
    paths.push_back(path + 'f');
  }
  paths.push_back("extra/f"); // 1 + 130*63 + 1 = 8192 directories including root.
  auto files = views(paths);
  {
    scratch f;
    auto    writer = writer_for(f);
    auto    root   = build(writer, files);
    REQUIRE(read_tree(writer, root, source));
    paths.back() = "extra/more/f";
    files        = views(paths);
    auto over    = build_tree(writer, files, source);
    REQUIRE_FALSE(over);
    CHECK(over.error().code == tree_error_code::resource_limit);
    empty_after_failure(f, writer);
  }
  scratch f;
  auto    writer = writer_for(f);
  auto    root   = build(writer, files, provenance);
  REQUIRE(read_tree(writer, root, provenance));
  auto over = read_tree(writer, root, source);
  REQUIRE_FALSE(over);
  CHECK(over.error().code == tree_error_code::resource_limit);
  empty_after_failure(f, writer);
}
TEST_CASE("tree expanded shared subtrees enforce payload occurrences and profile difference", "[tree]") {
  scratch          f;
  auto             writer = writer_for(f);
  auto             leaf   = intern(writer, file_record{420, std::string(8 * 1024 * 1024, 'x')});
  auto             branch = chain(writer, leaf, 1);
  directory_record directory;
  for (unsigned i = 0; i < 8; ++i)
    directory.children.push_back(child(std::format("d{}", i), branch));
  auto root  = intern(writer, directory);
  auto exact = read_tree(writer, root, source);
  REQUIRE(exact);
  CHECK(exact->size() == 8);
  directory.children.push_back(child("d8", branch));
  auto excessive_root = intern(writer, directory);
  auto source_over    = read_tree(writer, excessive_root, source);
  REQUIRE_FALSE(source_over);
  CHECK(source_over.error().code == tree_error_code::resource_limit);
  empty_after_failure(f, writer);
  auto next = writer_for(f);
  leaf      = intern(next, file_record{420, std::string(8 * 1024 * 1024, 'x')});
  branch    = chain(next, leaf, 1);
  root      = intern(next, directory_record{{child("a", branch), child("b", branch)}});
  REQUIRE(read_tree(next, root, provenance));
  auto over_root = intern(next, directory_record{{child("a", branch), child("b", branch), child("c", branch)}});
  auto over      = read_tree(next, over_root, provenance);
  REQUIRE_FALSE(over);
  CHECK(over.error().code == tree_error_code::resource_limit);
  empty_after_failure(f, next);
}
TEST_CASE("tree provenance maximum induced directories count shared occurrences", "[tree]") {
  scratch f;
  auto    writer = writer_for(f);
  auto    leaf   = intern(writer, file_record{});
  // 63 directory occurrences per branch, file at depth 64, root depth zero.
  auto             branch = chain(writer, leaf, 63);
  directory_record directory;
  for (unsigned i = 0; i < 4096; ++i)
    directory.children.push_back(child(std::format("d{:04}", i), branch));
  auto root   = intern(writer, directory);
  auto result = read_tree(writer, root, provenance);
  REQUIRE(result);
  CHECK(result->size() == 4096);
  CHECK(result->front().path.starts_with("d0000/"));
  CHECK(std::ranges::count(result->front().path, '/') == 63);
  auto over = read_tree(writer, root, source);
  REQUIRE_FALSE(over);
  CHECK(over.error().code == tree_error_code::resource_limit);
  empty_after_failure(f, writer);
}
TEST_CASE("tree preserves typed storage refusal without hash overrides", "[tree]") {
  scratch f;
  {
    // A valid immutable ID one below exhaustion leaves room for the new file, but not its parent directory.
    auto db = connection::open(f.db, open_mode::read_write);
    REQUIRE(db);
    auto encoded = encode_record(file_record{420, "existing"});
    auto digest  = digest_record(file_record{420, "existing"});
    REQUIRE(encoded);
    REQUIRE(digest);
    auto q = db->prepare("INSERT INTO records VALUES(9223372036854775806,1,1,?1,?2)");
    REQUIRE(q);
    REQUIRE(q->bind_blob(1, std::string_view(reinterpret_cast<const char*>(digest->data()), digest->size())));
    REQUIRE(q->bind_blob(2, *encoded));
    REQUIRE(q->step() == step_result::done);
  }
  auto             writer = writer_for(f);
  const std::array files{tree_file_view{"a", "new"}};
  auto             result = build_tree(writer, files, source);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == tree_error_code::storage);
  REQUIRE(result.error().storage);
  CHECK(result.error().storage->code == format_error_code::unsupported_state);
  REQUIRE_FALSE(writer.commit());
  auto reader = open_existing(f.db);
  REQUIRE(reader);
  CHECK(reader->read_record(9223372036854775806ULL));
  auto rolled_back = reader->read_record(9223372036854775807ULL);
  REQUIRE_FALSE(rolled_back);
  CHECK(rolled_back.error().code == format_error_code::not_found);
}
