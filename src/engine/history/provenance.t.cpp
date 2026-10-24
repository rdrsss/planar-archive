// Exact provenance preparation, generated source bytes and historical manifest verification.
#include <catch2/catch_test_macros.hpp>
#include <unistd.h>
import std;
import planar.engine.history.provenance;
import planar.engine.canonical;
import planar.engine.runtime_source;
using namespace planar::engine;
using namespace planar::engine::history;
namespace {
auto prepare(std::span<const original_file_view> files, std::string_view separator = "\n") {
  std::string chunk;
  for (const auto& f : files) {
    chunk += f.bytes;
    chunk += separator;
  }
  return prepare_stdlib(files, separator, chunk);
}
std::string read(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  REQUIRE(in);
  return {std::istreambuf_iterator<char>(in), {}};
}
void change_manifest(std::vector<tree_file>& files, std::function<void(value&)> edit) {
  auto it = std::ranges::find(files, "__planar__/assembly.canonical", &tree_file::path);
  REQUIRE(it != files.end());
  auto decoded = canonical_decode(it->payload);
  REQUIRE(decoded);
  auto changed = std::make_shared<value>(**decoded);
  edit(*changed);
  auto encoded = canonical_encode(changed);
  REQUIRE(encoded);
  it->payload = *encoded;
}
value_ptr string_value(std::string text) {
  auto p    = std::make_shared<value>();
  p->kind   = value_kind::string;
  p->string = std::move(text);
  return p;
}
} // namespace
TEST_CASE("provenance registry retains exact captured bytes and normalized mode", "[provenance]") {
  const std::string bytes("\0\xff\r\n", 4);
  for (auto mode : {420u, 493u}) {
    auto result = prepare_registry(bytes, mode);
    REQUIRE(result);
    REQUIRE(result->files.size() == 1);
    CHECK(result->files[0].path == "input.json");
    CHECK(result->files[0].payload == bytes);
    CHECK(result->files[0].mode == mode);
    const std::array views{tree_file_view{"input.json", bytes, mode}};
    CHECK(result->digest == *digest_tree(views, tree_profile::provenance));
  }
  CHECK_FALSE(prepare_registry(bytes, 0600));
  CHECK(prepare_registry(std::string(16 * 1024 * 1024, 'x'), 420));
  CHECK_FALSE(prepare_registry(std::string(16 * 1024 * 1024 + 1, 'x'), 420));
}
TEST_CASE("provenance generated originals and legacy assembled chunk match exact authored bytes", "[provenance]") {
  const std::array names{"planar/json.lua",         "planar/schema.lua",       "planar/semantic.lua", "planar/selection.lua",
                         "planar/presentation.lua", "planar/observations.lua", "planar/render.lua",   "planar/acceptance.lua",
                         "planar/commands.lua",     "planar/bootstrap.lua"};
  auto             originals = embedded::originals();
  REQUIRE(originals.size() == names.size());
  std::string legacy;
  for (std::size_t i = 0; i < names.size(); ++i) {
    CHECK(originals[i].path == names[i]);
    auto authored = read(std::filesystem::path(PLANAR_STDLIB_DIR) / names[i]);
    CHECK(originals[i].bytes == authored);
    legacy += authored;
    legacy += '\n';
  }
  CHECK(embedded::separator() == "\n");
  CHECK(embedded::source() == legacy);
  auto result = prepare_embedded_stdlib();
  REQUIRE(result);
  CHECK(result->files.size() == names.size() + 2);
  auto verified = verify_stdlib(result->files);
  REQUIRE(verified);
  CHECK(verified->digest == result->digest);
  for (const auto& file : result->files)
    CHECK(file.mode == 420);
  char pattern[] = "/tmp/planar-provenance-XXXXXX";
  REQUIRE(::mkdtemp(pattern));
  const auto root = std::filesystem::canonical(pattern);
  struct cleanup {
    std::filesystem::path path;
    ~cleanup() {
      std::filesystem::remove_all(path);
    }
  } cleanup_root{root};
  const auto db = root / "db";
  REQUIRE(ledger::initialize_new(db, "provenance_test"));
  auto writer = ledger::begin_records(db);
  REQUIRE(writer);
  std::vector<tree_file_view> views;
  for (const auto& file : result->files)
    views.push_back({file.path, file.payload, file.mode});
  auto stored = build_tree(*writer, views, tree_profile::provenance);
  REQUIRE(stored);
  CHECK(stored->digest == result->digest);
  REQUIRE(writer->commit());
  auto reader = ledger::open_existing(db);
  REQUIRE(reader);
  auto reconstructed = read_tree(*reader, *stored, tree_profile::provenance);
  REQUIRE(reconstructed);
  auto historical = verify_stdlib(*reconstructed);
  REQUIRE(historical);
  CHECK(historical->digest == result->digest);
  REQUIRE(historical->files.size() == result->files.size());
  for (std::size_t i = 0; i < result->files.size(); ++i) {
    CHECK(historical->files[i].path == result->files[i].path);
    CHECK(historical->files[i].payload == result->files[i].payload);
    CHECK(historical->files[i].mode == result->files[i].mode);
  }
}
TEST_CASE("provenance source order separator and boundaries remain portable identity", "[provenance]") {
  const std::array a{original_file_view{"a", "x"}, original_file_view{"b", "yz"}};
  auto             base = prepare(a, "");
  REQUIRE(base);
  const std::array reordered{a[1], a[0]};
  const std::array boundary{original_file_view{"a", "xy"}, original_file_view{"b", "z"}};
  const std::array edited{original_file_view{"a", "X"}, a[1]};
  CHECK(base->digest != prepare(reordered, "")->digest);
  CHECK(base->digest != prepare(boundary, "")->digest);
  CHECK(base->digest != prepare(edited, "")->digest);
  CHECK(base->digest != prepare(a, std::string_view("\0", 1))->digest);
  auto verified = verify_stdlib(base->files);
  REQUIRE(verified);
  CHECK(verified->digest == base->digest);
  CHECK(prepare_stdlib({}, "\n", ""));
  CHECK_FALSE(prepare_stdlib(a, "\n", "xyz\n"));
}
TEST_CASE("provenance rejects unsafe duplicate aliased and reserved original paths", "[provenance]") {
  for (std::string_view path : {"__planar__/a", "__PLANAR__/a", "__planar__", "../a", "a//b", "/a", ".git/x"}) {
    const std::array files{original_file_view{path, ""}};
    CHECK_FALSE(prepare(files));
  }
  for (const auto& files :
       std::vector<std::vector<original_file_view>>{{{"a", ""}, {"a", ""}}, {{"A/x", ""}, {"a/y", ""}}, {{"a", ""}, {"a/b", ""}}})
    CHECK_FALSE(prepare(files));
}
TEST_CASE("provenance historical verification rejects incomplete malformed unsupported and wrong-mode sets", "[provenance]") {
  const std::array originals{original_file_view{"a", "x"}, original_file_view{"b", "y"}};
  auto             base = prepare(originals);
  REQUIRE(base);
  for (unsigned scenario = 0; scenario < 11; ++scenario) {
    auto files = base->files;
    switch (scenario) {
    case 0:
      files.pop_back();
      break;
    case 1:
      files.push_back({"extra", "", 420});
      break;
    case 2:
      files.push_back(files.back());
      break;
    case 3:
      files[0].mode = 493;
      break;
    case 4:
      files[0].payload += '!';
      break;
    case 5:
      files[1].payload = "bad";
      break;
    case 6:
      change_manifest(files, [](value& v) { v.fields.erase("scheme"); });
      break;
    case 7:
      change_manifest(files, [](value& v) { v.fields["scheme"] = string_value("future"); });
      break;
    case 8:
      change_manifest(files, [](value& v) { v.fields["version"] = string_value("1"); });
      break;
    case 9:
      change_manifest(files, [](value& v) {
        auto l            = std::make_shared<value>(*v.fields.at("files"));
        l->items[1]       = l->items[0];
        v.fields["files"] = l;
      });
      break;
    case 10:
      change_manifest(files, [](value& v) {
        auto l            = std::make_shared<value>(*v.fields.at("files"));
        l->items[0]       = string_value("missing");
        v.fields["files"] = l;
      });
      break;
    }
    INFO(scenario);
    CHECK_FALSE(verify_stdlib(files));
  }
}
TEST_CASE("provenance complete set count and exact byte bounds include both synthetics", "[provenance]") {
  std::vector<std::string> paths;
  for (unsigned i = 0; i < 4095; ++i)
    paths.push_back(std::format("f{:04}", i));
  std::vector<original_file_view> files;
  for (const auto& p : paths)
    files.push_back({p, ""});
  auto exact_count = prepare(std::span(files).first(4094), "");
  REQUIRE(exact_count);
  CHECK(exact_count->files.size() == 4096);
  CHECK_FALSE(prepare(files, ""));
  const std::array empty{original_file_view{"aa", ""}};
  auto             overhead = prepare(empty, "");
  REQUIRE(overhead);
  std::size_t bytes = 0;
  for (const auto& f : overhead->files)
    bytes += f.payload.size();
  // Two exact payload copies (original and chunk); the two-byte path makes framing parity exact.
  std::string      payload((16 * 1024 * 1024 - bytes) / 2, 'x');
  const std::array large{original_file_view{"aa", payload}};
  auto             exact = prepare(large, "");
  REQUIRE(exact);
  std::size_t actual = 0;
  for (const auto& f : exact->files)
    actual += f.payload.size();
  CHECK(actual == 16 * 1024 * 1024);
  payload += 'x';
  const std::array over{original_file_view{"aa", payload}};
  CHECK_FALSE(prepare(over, ""));
  CHECK_FALSE(prepare_stdlib(empty, std::string(16 * 1024 * 1024 + 1, 'x'), ""));
}
