// Independently specified canonical/hash vectors and selected captured identity recipes.
#include <catch2/catch_test_macros.hpp>
#include <unistd.h>
#include <xxhash.h>
import std;
import planar.engine.hash;
import planar.engine.capture;
using namespace planar::engine;
namespace {
std::shared_ptr<value> node(value_kind kind) {
  auto p  = std::make_shared<value>();
  p->kind = kind;
  return p;
}
value_ptr text(std::string s) {
  auto p    = node(value_kind::string);
  p->string = std::move(s);
  return p;
}
value_ptr integer(std::int64_t n) {
  auto p     = node(value_kind::integer);
  p->integer = n;
  return p;
}
value_ptr boolean(bool b) {
  auto p     = node(value_kind::boolean);
  p->boolean = b;
  return p;
}
value_ptr array(std::vector<value_ptr> items = {}) {
  auto p   = node(value_kind::list);
  p->items = std::move(items);
  return p;
}
std::shared_ptr<value> object(std::map<std::string, value_ptr> fields = {}) {
  auto p    = node(value_kind::record);
  p->fields = std::move(fields);
  return p;
}
std::string hex(std::string_view bytes) {
  std::string out;
  for (unsigned char b : bytes)
    out += std::format("{:02x}", b);
  return out;
}
struct vector_row {
  std::string bytes, domain, digest;
};
std::map<std::string, vector_row> vectors() {
  std::ifstream input(std::string(PLANAR_VECTOR_DIR) + "/canonical-v1.tsv");
  REQUIRE(input.good());
  std::map<std::string, vector_row> rows;
  std::string                       line;
  while (std::getline(input, line)) {
    std::istringstream parts(line);
    std::string        name;
    vector_row         row;
    REQUIRE(static_cast<bool>(std::getline(parts, name, '\t')));
    REQUIRE(static_cast<bool>(std::getline(parts, row.bytes, '\t')));
    REQUIRE(static_cast<bool>(std::getline(parts, row.domain, '\t')));
    REQUIRE(static_cast<bool>(std::getline(parts, row.digest)));
    REQUIRE(rows.emplace(name, row).second);
  }
  REQUIRE(rows.size() == 41);
  return rows;
}
void check_vector(const std::map<std::string, vector_row>& rows, std::string name, const value_ptr& p) {
  INFO(name);
  const auto& row   = rows.at(name);
  auto        bytes = canonical_encode(p);
  REQUIRE(bytes);
  CHECK(hex(*bytes) == row.bytes);
  CHECK(hash_value(row.domain, p) == row.digest);
}
value_ptr tree(const captured_file& file) {
  auto digest = file_digest(file.bytes);
  REQUIRE(digest);
  return array({object({{"root", text("source")},
                        {"path", text(file.path)},
                        {"kind", text("file")},
                        {"mode", text(file.mode)},
                        {"size", integer(file.bytes.size())},
                        {"content", text(*digest)}})});
}
struct temporary {
  std::filesystem::path path;
  temporary() {
    char pattern[] = "/tmp/planar-hash-XXXXXX";
    path           = std::filesystem::canonical(::mkdtemp(pattern));
  }
  ~temporary() {
    std::filesystem::remove_all(path);
  }
  captured_file capture(std::string bytes) {
    std::ofstream(path / "a", std::ios::binary) << bytes;
    std::filesystem::permissions(path / "a", std::filesystem::perms::owner_read | std::filesystem::perms::owner_write |
                                                 std::filesystem::perms::group_read | std::filesystem::perms::others_read);
    capture_session session;
    REQUIRE(session.add_root("source", path));
    REQUIRE(session.tree("source"));
    auto snapshot = session.finish();
    REQUIRE(snapshot);
    REQUIRE((*snapshot)->size() == 1);
    return (**snapshot)[0];
  }
};
} // namespace
TEST_CASE("hash matches independent hand canonical and domain vectors", "[hash]") {
  auto                             rows  = vectors();
  std::map<std::string, value_ptr> cases = {{"false", boolean(false)},
                                            {"true", boolean(true)},
                                            {"zero", integer(0)},
                                            {"negative", integer(-1)},
                                            {"minimum", integer(-2147483648LL)},
                                            {"maximum", integer(2147483647LL)},
                                            {"empty_string", text("")},
                                            {"raw", text(std::string("\0\xff\x80"
                                                                     "A",
                                                                     4))},
                                            {"empty_array", array()},
                                            {"empty_object", object()},
                                            {"array", array({boolean(true), boolean(false)})},
                                            {"object", object({{"é", boolean(false)}, {"z", boolean(true)}})}};
  for (const auto& [name, p] : cases)
    check_vector(rows, name, p);
  std::size_t count = 0;
  for (const auto& [name, row] : rows) {
    if (!name.starts_with("file_"))
      continue;
    std::string bytes;
    for (int i = 0, n = std::stoi(name.substr(5)); i < n; ++i)
      bytes += static_cast<char>(i % 256);
    check_vector(rows, name, text(bytes));
    CHECK(file_digest(bytes) == row.digest);
    ++count;
  }
  CHECK(count == 25);
  CHECK(file_digest("same").value() != fragment_digest("same").value());
  CHECK(hash_value("work_v1", text("same")).value() != hash_value("render_v1", text("same")).value());
  CHECK(hash_value("vector_v1", {}).error().code == "schema");
  CHECK(hash_value(std::string("\0", 1), text("")).error().code == "schema");
  CHECK(fragment_digest(std::string("\xff", 1)).error().code == "schema");
}
TEST_CASE("hash validates published upstream seed zero XXH3 length vectors", "[hash]") {
  std::string   bytes;
  std::uint64_t state = 2654435761ULL;
  for (int i = 0; i < 1025; ++i) {
    bytes += static_cast<char>(state >> 56);
    state *= 11400714785074694797ULL;
  }
  std::ifstream input(std::string(PLANAR_VECTOR_DIR) + "/upstream-xxh3-128.tsv");
  REQUIRE(input.good());
  std::size_t length, count = 0;
  std::string expected;
  while (input >> length >> expected) {
    auto hash = XXH3_128bits(bytes.data(), length);
    CHECK(std::format("{:016x}{:016x}", hash.high64, hash.low64) == expected);
    XXH128_canonical_t canonical;
    XXH128_canonicalFromHash(&canonical, hash);
    CHECK(hex(std::string_view(reinterpret_cast<const char*>(canonical.digest), 16)) == expected);
    CHECK(XXH128_isEqual(hash, XXH3_128bits_withSeed(bytes.data(), length, 0)));
    ++count;
  }
  CHECK(count == 20);
}
TEST_CASE("hash selected captured base observation unit and role recipes remain separate", "[hash]") {
  auto      rows = vectors();
  temporary base_dir, current_dir, other_dir;
  auto      base_file    = base_dir.capture("old");
  auto      current_file = current_dir.capture("new");
  auto      same_file    = other_dir.capture("new");
  auto base = object({{"root", text("source")}, {"label", text("initial")}, {"entries", tree(base_file)}, {"excluded", array()}});
  check_vector(rows, "base", base);
  auto base_hash = *hash_value("base_v1", base);
  auto observation =
      object({{"root", text("source")}, {"base", text(base_hash)}, {"entries", tree(current_file)}, {"excluded", array()}});
  check_vector(rows, "observation", observation);
  auto unit = object({{"work", text("library")},
                      {"root", text("source")},
                      {"base", text(base_hash)},
                      {"paths", array({text("a")})},
                      {"entries", tree(current_file)}});
  check_vector(rows, "unit", unit);
  auto unit_hash = *hash_value("unit_output_v1", unit);
  auto role =
      object({{"role", text("builder")}, {"units", array({object({{"work", text("library")}, {"output", text(unit_hash)}})})}});
  check_vector(rows, "role", role);
  // Different host locations/timestamps cannot affect logical captured identity.
  observation->fields["entries"] = tree(same_file);
  CHECK(hash_value("observation_v1", observation) == rows.at("observation").digest);
  unit->fields["entries"] = tree(same_file);
  CHECK(hash_value("unit_output_v1", unit) == unit_hash);
  // Grouping changes the role record while preserving exact per-unit targets.
  role->fields["role"] = text("library_worker");
  CHECK(hash_value("role_output_v1", role) != rows.at("role").digest);
  CHECK(hash_value("unit_output_v1", unit) == unit_hash);
  role->fields["units"] = array({object({{"work", text("command")}, {"output", text(unit_hash)}}),
                                 object({{"work", text("library")}, {"output", text(unit_hash)}})});
  CHECK(hash_value("role_output_v1", role) != rows.at("role").digest);
  unit->fields["work"] = text("command");
  CHECK(hash_value("unit_output_v1", unit) != unit_hash);
  unit->fields["work"]    = text("library");
  unit->fields["entries"] = array();
  CHECK(hash_value("unit_output_v1", unit) != unit_hash);
  CHECK(*canonical_encode(unit->fields["entries"]) == std::string("\x04\0\0\0\0\0\0\0\0", 9));
  // Changing captured content moves output observation but never mutates retained bytes.
  auto edited                    = current_dir.capture("edit");
  observation->fields["entries"] = tree(edited);
  CHECK(hash_value("observation_v1", observation) != rows.at("observation").digest);
  CHECK(current_file.bytes == "new");
  CHECK(file_digest(current_file.bytes).value() != file_digest(edited.bytes).value());
  base->fields["entries"] = tree(edited);
  CHECK(hash_value("base_v1", base) != base_hash);
  base->fields["entries"] = tree(base_file);
  base->fields["label"]   = text("other");
  CHECK(hash_value("base_v1", base) != base_hash);
}
