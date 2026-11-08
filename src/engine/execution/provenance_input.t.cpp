// read_provenance tests (execution-contract P2-4, D146, R15-B L17). Files live under a per-test mkdtemp root; the check
// order is pinned with inputs that violate two steps at once, so the earlier step must win.
#include <catch2/catch_test_macros.hpp>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.engine.json_codec;
import planar.engine.value;
import planar.engine.execution.provenance_input;

using namespace planar::engine;
using namespace planar::engine::execution;

namespace {

struct scratch {
  std::filesystem::path root;
  scratch() {
    char  pattern[] = "/tmp/planar-provenance-input-XXXXXX";
    char* made      = ::mkdtemp(pattern);
    REQUIRE(made != nullptr);
    root = std::filesystem::canonical(made);
  }
  ~scratch() {
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
  }
  std::filesystem::path write(std::string_view name, std::string_view bytes) const {
    const auto    path = root / name;
    std::ofstream out(path, std::ios::binary);
    out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    return path;
  }
};

std::string document(std::string_view settings = R"([{"name":"temperature","value":"0"}])",
                     std::string_view model    = R"({"requested":"model_a","actual":{"status":"unknown"}})") {
  return std::format(R"({{"version":1,"driver":{{"name":"driver","version":"1.0"}},"trial":{{"id":"t1"}},)"
                     R"("view":{{"id":"v1"}},"model":{},"session":{{"requested":"s","actual":"s"}},"settings":{}}})",
                     model, settings);
}

void require_error(const std::expected<value_ptr, provenance_error>& result, std::string_view code, std::string_view message,
                   std::string_view field = {}, std::string_view limit = {}) {
  REQUIRE_FALSE(result.has_value());
  INFO("code " << result.error().code << " message " << result.error().message << " field " << result.error().field << " limit "
               << result.error().limit);
  REQUIRE(result.error().code == code);
  REQUIRE(result.error().message == message);
  REQUIRE(result.error().field == field);
  REQUIRE(result.error().limit == limit);
}

constexpr std::string_view limit_message = "Execution resource limit exceeded";

} // namespace

TEST_CASE("provenance input reads and normalizes a valid document") {
  scratch    dir;
  const auto path =
      dir.write("p.json", document(R"([{"name":"zeta","value":"1"},{"name":"alpha","value":{"status":"unknown"}}])"));
  auto result = read_provenance(path);
  INFO((result.has_value() ? std::string{} : result.error().code + " " + result.error().field));
  REQUIRE(result.has_value());
  const auto& settings = (*result)->fields.at("settings")->items;
  REQUIRE(settings.size() == 2);
  REQUIRE(settings[0]->fields.at("name")->string == "alpha");
  REQUIRE(settings[1]->fields.at("name")->string == "zeta");
  REQUIRE((*result)->fields.at("model")->fields.at("actual")->fields.at("status")->string == "unknown");
}

TEST_CASE("provenance input checks the raw file size before any parse") {
  scratch dir;
  // L17: 65537 bytes of invalid JSON is resource_limit provenance_bytes, never schema.
  const auto over = dir.write("over.json", std::string(65537, '{'));
  require_error(read_provenance(over), "resource_limit", limit_message, {}, "provenance_bytes");
  REQUIRE(read_provenance(over).error().maximum == 65536);
  // Exactly 65536 bytes passes the size gate and fails at the grammar step instead.
  const auto at = dir.write("at.json", std::string(65536, '{'));
  require_error(read_provenance(at), "schema", "Invalid JSON", "provenance");
  // A valid document padded with trailing spaces to 65537 bytes is still refused by size (raw bytes, not compact JSON).
  auto padded = document();
  padded.resize(65537, ' ');
  require_error(read_provenance(dir.write("padded.json", padded)), "resource_limit", limit_message, {}, "provenance_bytes");
}

TEST_CASE("provenance input capture refusals reuse the explicit input rows") {
  scratch dir;
  require_error(read_provenance(dir.root / "missing.json"), "io", "Cannot resolve explicit input path");
  std::filesystem::create_directory(dir.root / "folder");
  require_error(read_provenance(dir.root / "folder"), "io", "Explicit input must be a regular file");
  const auto target = dir.write("target.json", document());
  std::filesystem::create_symlink(target, dir.root / "link.json");
  require_error(read_provenance(dir.root / "link.json"), "path_denied", "Explicit input file is a symbolic link");
  REQUIRE(::mkfifo((dir.root / "fifo").c_str(), 0600) == 0);
  require_error(read_provenance(dir.root / "fifo"), "io", "Explicit input must be a regular file");
}

TEST_CASE("provenance input grammar violations are schema with field provenance") {
  require_error(parse_provenance("{"), "schema", "Invalid JSON", "provenance");
  require_error(parse_provenance(document() + "x"), "schema", "Invalid JSON", "provenance");
  require_error(parse_provenance(R"({"version":1,"version":1})"), "schema", "Invalid JSON", "provenance");
  require_error(parse_provenance(R"({"version":null})"), "schema", "Invalid JSON", "provenance");
}

TEST_CASE("provenance input bounds follow the P2-4 check order") {
  // Depth: a settings value object sits at depth 4 and is a valid unknown leaf; one more level is depth 5.
  REQUIRE(parse_provenance(document(R"([{"name":"a","value":{"status":"unknown"}}])")).has_value());
  require_error(parse_provenance(document(R"([{"name":"a","value":{"status":{"x":"y"}}}])")), "resource_limit", limit_message, {},
                "provenance_depth");
  // Depth wins over settings count: 33 settings and a depth-5 value.
  std::string many = "[";
  for (int i = 0; i < 33; ++i)
    many += std::format(R"({}{{"name":"s{:02}","value":"v"}})", i ? "," : "", i);
  many += "]";
  require_error(parse_provenance(document(many)), "resource_limit", limit_message, {}, "provenance_settings");
  auto deep_many = many;
  deep_many.insert(deep_many.size() - 1, R"(,{"name":"deep","value":{"status":{"x":"y"}}})");
  require_error(parse_provenance(document(deep_many)), "resource_limit", limit_message, {}, "provenance_depth");
  // Settings count wins over string length.
  auto long_many = many;
  long_many.insert(long_many.size() - 1, std::format(R"(,{{"name":"long","value":"{}"}})", std::string(1025, 'x')));
  require_error(parse_provenance(document(long_many)), "resource_limit", limit_message, {}, "provenance_settings");
  // A 1025-byte leaf is resource_limit, never schema; 1024 bytes is accepted.
  require_error(parse_provenance(document(std::format(R"([{{"name":"a","value":"{}"}}])", std::string(1025, 'x')))),
                "resource_limit", limit_message, {}, "provenance_string_bytes");
  REQUIRE(parse_provenance(document(std::format(R"([{{"name":"a","value":"{}"}}])", std::string(1024, 'x')))).has_value());
  // String length wins over shape: an unknown key with a 1025-byte value.
  auto unknown_long = document();
  unknown_long.insert(1, std::format(R"("extra":"{}",)", std::string(1025, 'x')));
  require_error(parse_provenance(unknown_long), "resource_limit", limit_message, {}, "provenance_string_bytes");
}

TEST_CASE("provenance input shape violations name the failing key path") {
  auto missing = document();
  missing.replace(missing.find(R"("trial":{"id":"t1"},)"), std::string_view(R"("trial":{"id":"t1"},)").size(), "");
  require_error(parse_provenance(missing), "schema", "Required field", "provenance.trial");
  auto unknown = document();
  unknown.insert(1, R"("extra":"x",)");
  require_error(parse_provenance(unknown), "schema", "Unknown field", "provenance.extra");
  require_error(parse_provenance(document(R"([])", R"({"requested":"","actual":"a"})")), "schema", "Invalid string",
                "provenance.model.requested");
  require_error(parse_provenance(document(R"([{"name":"a","value":{"status":"known"}}])")), "schema", "Invalid string",
                "provenance.settings.0.value");
  require_error(parse_provenance(document(R"([{"name":{"status":"unknown"},"value":"v"}])")), "schema", "Invalid string",
                "provenance.settings.0.name");
  require_error(parse_provenance(document(R"([{"name":"a","value":"1"},{"name":"a","value":"2"}])")), "schema", "Duplicate value",
                "provenance.settings.1.name");
  require_error(parse_provenance(document(R"({})")), "schema", "Expected list", "provenance.settings");
  auto version = document();
  version.replace(version.find(R"("version":1)"), 11, R"("version":2)");
  require_error(parse_provenance(version), "schema", "Unsupported schema version", "provenance.version");
}
