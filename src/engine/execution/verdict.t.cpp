// R18-2 verdict reader checks: every check-order step with its code, message and field, the EX-8 verdict boundaries,
// authority normalization (R18-7) and the approve rules. Each file test owns its temporary root.
#include <catch2/catch_test_macros.hpp>
#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.engine.value;
import planar.engine.history.execution_codec;
import planar.engine.execution.verdict;
using namespace planar::engine;
using namespace planar::engine::execution;
namespace {

constexpr std::string_view digest_a = "xxh3_128:00112233445566778899aabbccddeeff";
constexpr std::string_view digest_b = "xxh3_128:ffeeddccbbaa99887766554433221100";
constexpr std::string_view policy   = "xxh3_128:0123456789abcdef0123456789abcdef";

struct temporary {
  std::filesystem::path path;
  temporary() {
    char pattern[] = "/tmp/planar-verdict-XXXXXX";
    path           = std::filesystem::canonical(::mkdtemp(pattern));
  }
  ~temporary() {
    std::error_code ignored;
    std::filesystem::remove_all(path, ignored);
  }
  temporary(const temporary&)                       = delete;
  temporary&            operator=(const temporary&) = delete;
  std::filesystem::path put(const std::string& name, const std::string& bytes) const {
    auto p = path / name;
    std::ofstream(p, std::ios::binary) << bytes;
    return p;
  }
};

std::string json_string(std::string_view s) {
  std::string out = "\"";
  out += s;
  out += '"';
  return out;
}

// One finding object as JSON text; extra is spliced in verbatim before the closing brace.
std::string finding_json(std::string_view id, std::string_view severity, std::string_view work, std::string_view text,
                         std::string_view extra = "") {
  std::string out = R"({"id":)" + json_string(id) + R"(,"severity":)" + json_string(severity) + R"(,"work":)" +
                    json_string(work) + R"(,"text":)" + json_string(text);
  out += extra;
  out += "}";
  return out;
}

struct document {
  std::string decision = "reject";
  std::string targets = std::string(R"([{"work":"lexer","output":")") + std::string(digest_a) + R"(","work_fp":")" +
                        std::string(digest_b) + R"("},{"work":"parser","output":")" + std::string(digest_b) + R"(","work_fp":")" +
                        std::string(digest_a) + R"("}])";
  std::vector<std::string>   findings{finding_json("F-1", "must", "parser", "Fix it.", R"(,"path":"src/p.cpp","line":7)")};
  std::string                repair  = R"(["builder"])";
  std::string                summary = "Needs work.";
  std::optional<std::string> authority;
  std::string                extra; // spliced before the closing brace

  std::string json() const {
    std::string out = R"({"version":1,"targets":)" + targets + R"(,"policy":)" + json_string(policy) + R"(,"decision":)" +
                      json_string(decision) + R"(,"findings":[)";
    for (std::size_t i = 0; i < findings.size(); ++i) {
      if (i)
        out += ",";
      out += findings[i];
    }
    out += R"(],"repair":)" + repair + R"(,"summary":)" + json_string(summary);
    if (authority)
      out += R"(,"authority":)" + json_string(*authority);
    out += extra;
    out += "}";
    return out;
  }
};

void refused(const std::expected<value_ptr, verdict_error>& result, std::string_view code, std::string_view message,
             std::string_view field) {
  REQUIRE_FALSE(result);
  CHECK(result.error().code == code);
  CHECK(result.error().message == message);
  CHECK(result.error().field == field);
}

void limited(const std::expected<value_ptr, verdict_error>& result, std::string_view limit, std::uint64_t maximum) {
  refused(result, "resource_limit", "Execution resource limit exceeded", "");
  CHECK(result.error().limit == limit);
  CHECK(result.error().maximum == maximum);
}

std::vector<std::string> notes(std::size_t n) {
  std::vector<std::string> out;
  for (std::size_t i = 0; i < n; ++i)
    out.push_back(finding_json(std::format("N{}", i), "note", "lexer", "n"));
  return out;
}

} // namespace

TEST_CASE("R18-B verdict parse accepts and normalizes a valid document", "[r18][verdict]") {
  const document doc;
  auto           parsed = parse_verdict_bytes(doc.json());
  REQUIRE(parsed);
  CHECK_FALSE((*parsed)->fields.contains("authority"));
  auto normalized = normalize_verdict(*parsed);
  REQUIRE(normalized);
  CHECK(normalized->authority == "reviewer_role_v1");
  CHECK(normalized->decision == "reject");
  CHECK(normalized->targets.size() == 2);
  CHECK(normalized->findings.at(0).line == std::optional<std::int64_t>(7));
  CHECK(normalized->repair == std::vector<std::string>{"builder"});
  // The normalized document encodes; an explicit reviewer_role_v1 authority normalizes to the same bytes.
  auto bytes = history::encode_verdict_v1(*normalized);
  REQUIRE(bytes);
  auto explicit_doc      = doc;
  explicit_doc.authority = "reviewer_role_v1";
  auto explicit_parsed   = parse_verdict_bytes(explicit_doc.json());
  REQUIRE(explicit_parsed);
  auto explicit_normalized = normalize_verdict(*explicit_parsed);
  REQUIRE(explicit_normalized);
  auto explicit_bytes = history::encode_verdict_v1(*explicit_normalized);
  REQUIRE(explicit_bytes);
  CHECK(*explicit_bytes == *bytes);
}

TEST_CASE("R18-B verdict step 2 byte bound", "[r18][verdict]") {
  const auto base = document{}.json();
  // Trailing JSON whitespace pads the document to exactly 1 MiB, which passes the bound.
  auto exact = base + std::string(1048576 - base.size(), ' ');
  REQUIRE(exact.size() == 1048576);
  CHECK(parse_verdict_bytes(exact));
  limited(parse_verdict_bytes(exact + " "), "verdict_bytes", 1048576);
  // The bound precedes grammar.
  limited(parse_verdict_bytes(std::string(1048577, '{')), "verdict_bytes", 1048576);
}

TEST_CASE("R18-B verdict step 3 grammar", "[r18][verdict]") {
  const auto grammar = [](const std::string& bytes) {
    refused(parse_verdict_bytes(bytes), "schema", "JSON codec refused input", "verdict");
  };
  const document doc;
  grammar("");
  grammar("{");
  grammar(doc.json() + "x");
  grammar(doc.json() + "{}");
  auto duplicate  = doc;
  duplicate.extra = R"(,"summary":"again")";
  grammar(duplicate.json());
  auto null_value  = doc;
  null_value.extra = R"(,"authority":null)";
  grammar(null_value.json());
  auto fractional     = doc;
  fractional.findings = {finding_json("F-1", "note", "lexer", "t", R"(,"path":"a","line":1.5)")};
  grammar(fractional.json());
  auto exponent     = doc;
  exponent.findings = {finding_json("F-1", "note", "lexer", "t", R"(,"path":"a","line":1e2)")};
  grammar(exponent.json());
  auto huge     = doc;
  huge.findings = {finding_json("F-1", "note", "lexer", "t", R"(,"path":"a","line":2147483648)")};
  grammar(huge.json());
  auto nul    = doc;
  nul.summary = "a\\u0000b";
  grammar(nul.json());
  // Grammar precedes the finding bounds.
  auto many_and_duplicate     = doc;
  many_and_duplicate.findings = notes(257);
  many_and_duplicate.extra    = R"(,"summary":"again")";
  grammar(many_and_duplicate.json());
}

TEST_CASE("R18-B verdict step 4 bounds and boundaries", "[r18][verdict]") {
  auto at_limit     = document{};
  at_limit.findings = notes(256);
  at_limit.decision = "approve";
  at_limit.repair   = "[]";
  CHECK(parse_verdict_bytes(at_limit.json()));
  auto over     = at_limit;
  over.findings = notes(257);
  limited(parse_verdict_bytes(over.json()), "verdict_findings", 256);

  auto text     = document{};
  text.findings = {finding_json("F-1", "note", "lexer", std::string(4096, 't'))};
  CHECK(parse_verdict_bytes(text.json()));
  text.findings = {finding_json("F-1", "note", "lexer", std::string(4097, 't'))};
  limited(parse_verdict_bytes(text.json()), "verdict_finding_bytes", 4096);

  auto summary    = document{};
  summary.summary = std::string(16384, 's');
  CHECK(parse_verdict_bytes(summary.json()));
  summary.summary = std::string(16385, 's');
  limited(parse_verdict_bytes(summary.json()), "verdict_summary_bytes", 16384);

  // Order within step 4: count, then finding text, then summary; and step 4 precedes the field table.
  auto all     = document{};
  all.findings = notes(257);
  all.findings.push_back(finding_json("F-x", "note", "lexer", std::string(4097, 't')));
  all.summary = std::string(16385, 's');
  all.extra   = R"(,"bogus":1)";
  limited(parse_verdict_bytes(all.json()), "verdict_findings", 256);
  all.findings = {finding_json("F-x", "note", "lexer", std::string(4097, 't'))};
  limited(parse_verdict_bytes(all.json()), "verdict_finding_bytes", 4096);
  all.findings = {};
  limited(parse_verdict_bytes(all.json()), "verdict_summary_bytes", 16384);
  all.summary = "";
  refused(parse_verdict_bytes(all.json()), "schema", "Unknown field", "verdict.bogus");
}

TEST_CASE("R18-B verdict step 5 shape and field table", "[r18][verdict]") {
  const auto shape = [](const document& doc, std::string_view field, std::string_view message) {
    refused(parse_verdict_bytes(doc.json()), "schema", message, field);
  };
  refused(parse_verdict_bytes("[]"), "schema", "Expected object", "verdict");
  auto unknown  = document{};
  unknown.extra = R"(,"extra":true)";
  shape(unknown, "verdict.extra", "Unknown field");
  refused(parse_verdict_bytes(R"({"version":1})"), "schema", "Required field", "verdict.targets");

  auto version  = document{};
  version.extra = "";
  {
    auto bytes = version.json();
    bytes.replace(bytes.find(R"("version":1)"), 11, R"("version":2)");
    refused(parse_verdict_bytes(bytes), "schema", "Unsupported schema version", "verdict.version");
  }

  auto line_without_path     = document{};
  line_without_path.findings = {finding_json("F-1", "note", "lexer", "t", R"(,"line":3)")};
  shape(line_without_path, "verdict.findings.0.path", "Required field");

  auto non_integer_line     = document{};
  non_integer_line.findings = {finding_json("F-1", "note", "lexer", "t", R"(,"path":"a","line":"3")")};
  shape(non_integer_line, "verdict.findings.0.line", "Integer out of range");

  auto unsorted    = document{};
  unsorted.targets = std::string(R"([{"work":"parser","output":")") + std::string(digest_a) + R"(","work_fp":")" +
                     std::string(digest_a) + R"("},{"work":"lexer","output":")" + std::string(digest_a) + R"(","work_fp":")" +
                     std::string(digest_a) + R"("}])";
  shape(unsorted, "verdict.targets.1.work", "Duplicate value");
  auto duplicate_targets    = document{};
  duplicate_targets.targets = std::string(R"([{"work":"lexer","output":")") + std::string(digest_a) + R"(","work_fp":")" +
                              std::string(digest_a) + R"("},{"work":"lexer","output":")" + std::string(digest_a) +
                              R"(","work_fp":")" + std::string(digest_a) + R"("}])";
  shape(duplicate_targets, "verdict.targets.1.work", "Duplicate value");
  auto empty_targets    = document{};
  empty_targets.targets = "[]";
  shape(empty_targets, "verdict.targets", "List is too short");

  auto unsorted_repair   = document{};
  unsorted_repair.repair = R"(["tester","builder"])";
  shape(unsorted_repair, "verdict.repair.1", "Duplicate value");
  auto duplicate_repair   = document{};
  duplicate_repair.repair = R"(["builder","builder"])";
  shape(duplicate_repair, "verdict.repair.1", "Duplicate value");

  auto bad_digest     = document{};
  bad_digest.targets  = R"([{"work":"lexer","output":"xxh3_128:XYZ","work_fp":"xxh3_128:00"}])";
  bad_digest.findings = {};
  shape(bad_digest, "verdict.targets.0.output", "Malformed output digest");

  auto foreign_work     = document{};
  foreign_work.findings = {finding_json("F-1", "note", "emitter", "t")};
  shape(foreign_work, "verdict.findings.0.work", "Invalid enum value");

  auto bad_decision     = document{};
  bad_decision.decision = "defer";
  shape(bad_decision, "verdict.decision", "Invalid enum value");

  auto bad_path     = document{};
  bad_path.findings = {finding_json("F-1", "note", "lexer", "t", R"(,"path":"/abs")")};
  shape(bad_path, "verdict.findings.0.path", "Invalid portable path");

  auto empty_text     = document{};
  empty_text.findings = {finding_json("F-1", "note", "lexer", "")};
  shape(empty_text, "verdict.findings.0.text", "Invalid string");

  auto bad_id     = document{};
  bad_id.findings = {finding_json("F 1", "note", "lexer", "t")};
  shape(bad_id, "verdict.findings.0.id", "Invalid ID");

  auto authority_type  = document{};
  authority_type.extra = R"(,"authority":7)";
  shape(authority_type, "verdict.authority", "Invalid string");
}

TEST_CASE("R18-B verdict step 6 authority", "[r18][verdict]") {
  auto external      = document{};
  external.authority = "external";
  auto parsed        = parse_verdict_bytes(external.json());
  REQUIRE(parsed); // steps 2 to 5 accept any authority string
  auto normalized = normalize_verdict(*parsed);
  REQUIRE_FALSE(normalized);
  CHECK(normalized.error().code == "unsupported_policy");
  CHECK(normalized.error().message == "Execution policy is not supported");
  CHECK(normalized.error().field.empty());

  auto empty      = document{};
  empty.authority = "";
  auto empty_norm = normalize_verdict(*parse_verdict_bytes(empty.json()));
  REQUIRE_FALSE(empty_norm);
  CHECK(empty_norm.error().code == "unsupported_policy");

  // Step 6 precedes step 7: an external approval with a must finding is unsupported_policy.
  auto external_approve      = external;
  external_approve.decision  = "approve";
  auto external_approve_norm = normalize_verdict(*parse_verdict_bytes(external_approve.json()));
  REQUIRE_FALSE(external_approve_norm);
  CHECK(external_approve_norm.error().code == "unsupported_policy");
}

TEST_CASE("R18-B verdict step 7 approve rules", "[r18][verdict]") {
  auto approve_must     = document{};
  approve_must.decision = "approve";
  approve_must.repair   = "[]";
  auto must             = normalize_verdict(*parse_verdict_bytes(approve_must.json()));
  REQUIRE_FALSE(must);
  CHECK(must.error().code == "schema");
  CHECK(must.error().field == "verdict.decision");

  auto approve_repair     = document{};
  approve_repair.decision = "approve";
  approve_repair.findings = {finding_json("F-1", "should", "lexer", "t")};
  auto repair             = normalize_verdict(*parse_verdict_bytes(approve_repair.json()));
  REQUIRE_FALSE(repair);
  CHECK(repair.error().field == "verdict.decision");

  approve_repair.repair = "[]";
  auto clean            = normalize_verdict(*parse_verdict_bytes(approve_repair.json()));
  REQUIRE(clean);
  CHECK(clean->decision == "approve");

  // A rejection with an empty repair list is valid here (it is the unowned case of R18-5).
  auto unowned   = document{};
  unowned.repair = "[]";
  CHECK(normalize_verdict(*parse_verdict_bytes(unowned.json())));
}

TEST_CASE("R18-B verdict normalize re-applies the field table", "[r18][verdict]") {
  CHECK_FALSE(normalize_verdict(nullptr));
  auto text  = std::make_shared<value>();
  text->kind = value_kind::string;
  auto bad   = normalize_verdict(text);
  REQUIRE_FALSE(bad);
  CHECK(bad.error().code == "schema");
  CHECK(bad.error().field == "verdict");
}

TEST_CASE("R18-B verdict step 1 file capture", "[r18][verdict]") {
  temporary  t;
  const auto good = t.put("verdict.json", document{}.json());
  auto       read = read_verdict_file(good);
  REQUIRE(read);
  CHECK(normalize_verdict(*read));

  const auto file_refused = [](const std::filesystem::path& p, std::string_view code, std::string_view message) {
    auto result = read_verdict_file(p);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == code);
    CHECK(result.error().message == message);
    CHECK(result.error().field.empty());
  };
  file_refused(t.path / "missing.json", "io", "Cannot resolve explicit input path");
  file_refused(t.path / "missing" / "deeper.json", "io", "Cannot resolve explicit input path");
  std::filesystem::create_symlink(good, t.path / "link.json");
  file_refused(t.path / "link.json", "path_denied", "Explicit input file is a symbolic link");
  std::filesystem::create_directory(t.path / "dir");
  file_refused(t.path / "dir", "io", "Explicit input must be a regular file");
  REQUIRE(::mkfifo((t.path / "fifo").c_str(), 0600) == 0);
  file_refused(t.path / "fifo", "io", "Explicit input must be a regular file");

  // Step 2 through the file: the descriptor size gate maps to verdict_bytes with the execution message.
  const auto base   = document{}.json();
  const auto at_max = t.put("max.json", base + std::string(1048576 - base.size(), ' '));
  CHECK(read_verdict_file(at_max));
  const auto over = t.put("over.json", base + std::string(1048577 - base.size(), ' '));
  auto       big  = read_verdict_file(over);
  limited(big, "verdict_bytes", 1048576);

  // Later steps keep their own codes through the file reader.
  const auto garbage = t.put("garbage.json", "not json");
  refused(read_verdict_file(garbage), "schema", "JSON codec refused input", "verdict");
}
