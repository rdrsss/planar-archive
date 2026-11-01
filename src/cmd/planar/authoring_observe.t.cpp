// RA01-B2 observe tests: the `observe` verb driven through the authoring stack over real observation directories.
// Expected bytes are typed from the authoring contract (RA-6) and the render observations contract, not from the product.
#include <catch2/catch_test_macros.hpp>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.cmd.authoring_stack.bootstrap;
import planar.cmd.response;

namespace {
namespace fs = std::filesystem;

struct observe_result {
  int         status = -1;
  std::string out, err;
};

const std::string digest_a = "xxh3_128:00112233445566778899aabbccddeeff";

/// Per-test observation directory with the helpers to build files.
struct observation_dir {
  fs::path root;
  observation_dir() {
    char pattern[] = "/tmp/planar-ra01-obs-XXXXXX";
    REQUIRE(::mkdtemp(pattern));
    root = fs::canonical(pattern) / "obs";
    fs::create_directories(root);
  }
  observation_dir(const observation_dir&)            = delete;
  observation_dir& operator=(const observation_dir&) = delete;
  ~observation_dir() {
    std::error_code ignored;
    fs::remove_all(root.parent_path(), ignored);
  }
  void put(const std::string& relative, std::string_view bytes) const {
    const auto path = root / relative;
    fs::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << bytes;
    REQUIRE(out.good());
  }
  void mkdir(const std::string& relative) const {
    fs::create_directories(root / relative);
  }
  void remove(const std::string& relative) const {
    fs::remove_all(root / relative);
  }
  void symlink(const std::string& relative, const std::string& target) const {
    fs::create_directories((root / relative).parent_path());
    fs::create_symlink(target, root / relative);
  }
  void fifo(const std::string& relative) const {
    fs::create_directories((root / relative).parent_path());
    REQUIRE(::mkfifo((root / relative).c_str(), 0600) == 0);
  }
  void hard_link(const std::string& from, const std::string& to) const {
    fs::create_directories((root / to).parent_path());
    fs::create_hard_link(root / from, root / to);
  }
  std::string path() const {
    return root.string();
  }
};

observe_result run_stack(const std::vector<std::string>& args) {
  std::vector<std::string_view> views(args.begin(), args.end());
  const auto                    response = planar::cmd::authoring_stack::invoke(views);
  return {response.status, response.output, response.error};
}
observe_result observe(const observation_dir& dir) {
  return run_stack({"observe", "--from", dir.path()});
}

std::string check_json(std::string_view id, std::string_view reported = "pass", std::string_view summary = "ok",
                       std::string_view work = "w1", std::string_view digest = digest_a) {
  return std::format(R"({{"id":"{0}","check":"unit","target":{{"work":"{2}","output":"{3}"}},"reported":"{1}",)"
                     R"("summary":"{4}","vector":[{{"work":"{2}","output":"{3}"}}]}})",
                     id, reported, work, digest, summary);
}
std::string finding_json(std::string_view id, std::string_view text = "t", std::string_view severity = "note") {
  return std::format(R"({{"id":"{}","target":{{"work":"w1","output":"{}"}},"severity":"{}","text":"{}"}})", id, digest_a,
                     severity, text);
}
std::string checkpoint_json(std::string_view progress, std::string_view next, std::string_view unresolved,
                            const std::vector<std::string>& files) {
  std::string list;
  for (const auto& file : files)
    list += (list.empty() ? "" : ",") + std::format("\"{}\"", file);
  return std::format(R"({{"progress":"{}","next":"{}","unresolved":"{}","files":[{}]}})", progress, next, unresolved, list);
}
/// Canonical (sorted keys) renderings of the helpers above.
std::string canon_check(std::string_view id, std::string_view reported = "pass", std::string_view summary = "ok") {
  return std::format(R"({{"check":"unit","id":"{0}","reported":"{1}","summary":"{2}","target":{{"output":"{3}","work":"w1"}},)"
                     R"("vector":[{{"output":"{3}","work":"w1"}}]}})",
                     id, reported, summary, digest_a);
}
std::string canon_finding(std::string_view id, std::string_view text = "t") {
  return std::format(R"({{"id":"{}","severity":"note","target":{{"output":"{}","work":"w1"}},"text":"{}"}})", id, digest_a, text);
}

/// Exact closed error envelope of a native refusal (no file, empty field).
std::string native_error(std::string_view code, std::string_view message) {
  return std::format(R"({{"error":{{"code":"{}","field":"","message":"{}"}},"kind":"error","version":1}})"
                     "\n",
                     code, message);
}
bool has(const std::string& text, std::string_view fragment) {
  return text.find(fragment) != std::string::npos;
}
/// A typed failure: exit 3, empty stdout, and the code, file and field fragments of the closed envelope.
void check_failure(const observe_result& r, std::string_view code, std::string_view file, std::string_view field) {
  CAPTURE(r.err);
  CHECK(r.status == 3);
  CHECK(r.out.empty());
  CHECK(has(r.err, std::format("\"code\":\"{}\"", code)));
  CHECK(has(r.err, std::format("\"file\":\"{}\"", file)));
  CHECK(has(r.err, std::format("\"field\":\"{}\"", field)));
  CHECK(has(r.err, "\"kind\":\"error\""));
}
void check_success(const observe_result& r, std::string_view expected_out) {
  CAPTURE(r.err);
  CHECK(r.status == 0);
  CHECK(r.err.empty());
  CHECK(r.out == expected_out);
}
std::string sorted_empty() {
  return "{\"checks\":[],\"findings\":[],\"version\":1}\n";
}
} // namespace

TEST_CASE("RA01-B2 observe refuses a symlink entry", "[authoring][ra01]") {
  // Mutants: entries opened without O_NOFOLLOW (the link is followed and read: success), or the S_ISREG test dropped.
  const auto expected = native_error("path_denied", "Links and special files forbidden");
  SECTION("under checks") {
    observation_dir d;
    d.put("outside.json", check_json("c1"));
    d.symlink("checks/a.json", "../outside.json");
    const auto r = observe(d);
    CHECK(r.status == 3);
    CHECK(r.out.empty());
    CHECK(r.err == expected);
  }
  SECTION("under findings") {
    observation_dir d;
    d.put("outside.json", finding_json("f1"));
    d.symlink("findings/a.json", "../outside.json");
    const auto r = observe(d);
    CHECK(r.status == 3);
    CHECK(r.out.empty());
    CHECK(r.err == expected);
  }
  SECTION("as checkpoint.json") {
    observation_dir d;
    d.put("outside.json", checkpoint_json("p", "n", "u", {}));
    d.symlink("checkpoint.json", "outside.json");
    const auto r = observe(d);
    CHECK(r.status == 3);
    CHECK(r.out.empty());
    CHECK(r.err == expected);
  }
  SECTION("dangling link") {
    observation_dir d;
    d.symlink("checks/a.json", "missing.json");
    const auto r = observe(d);
    CHECK(r.status == 3);
    CHECK(r.err == expected);
  }
}

TEST_CASE("RA01-B2 observe refuses a FIFO entry", "[authoring][ra01]") {
  // Mutant: the regular-file check dropped (the FIFO reads as empty and the code changes to schema); a blocking open
  // would hang here, so entries must be opened O_NONBLOCK or inspected first.
  const auto expected = native_error("path_denied", "Links and special files forbidden");
  for (const std::string where : {"checks/a.json", "findings/a.json", "checkpoint.json"}) {
    SECTION(where) {
      observation_dir d;
      d.fifo(where);
      const auto r = observe(d);
      CHECK(r.status == 3);
      CHECK(r.out.empty());
      CHECK(r.err == expected);
    }
  }
}

TEST_CASE("RA01-B2 observe refuses a hard-link alias", "[authoring][ra01]") {
  // Mutant: the (st_dev, st_ino) set check skipped (the aliased bytes validate and the document prints).
  const auto expected = native_error("path_denied", "Hard-link input alias");
  SECTION("a check and a finding") {
    observation_dir d;
    d.put("checks/a.json", check_json("c1"));
    d.hard_link("checks/a.json", "findings/b.json");
    const auto r = observe(d);
    CHECK(r.status == 3);
    CHECK(r.out.empty());
    CHECK(r.err == expected);
  }
  SECTION("two checks") {
    observation_dir d;
    d.put("checks/a.json", check_json("c1"));
    d.hard_link("checks/a.json", "checks/b.json");
    const auto r = observe(d);
    CHECK(r.status == 3);
    CHECK(r.err == expected);
  }
  SECTION("the checkpoint and a check") {
    observation_dir d;
    d.put("checkpoint.json", checkpoint_json("p", "n", "u", {}));
    d.hard_link("checkpoint.json", "checks/a.json");
    const auto r = observe(d);
    CHECK(r.status == 3);
    CHECK(r.err == expected);
  }
}

TEST_CASE("RA01-B2 observe refuses layout violations without echoing names", "[authoring][ra01]") {
  // Mutants: the raw name echoed in the error file field, or a non-.json file accepted (the observe.lua/directory.cpp
  // layout rules). The invalid UTF-8 branch below is applied by the prober in the Linux container (D-41, W2-82).
  const auto expected = native_error("path_denied", "Invalid captured path");
  const auto refused = [&](const char* label, const std::function<void(const observation_dir&)>& build, std::string_view secret) {
    SECTION(label) {
      observation_dir d;
      build(d);
      const auto r = observe(d);
      CAPTURE(r.err);
      CHECK(r.status == 3);
      CHECK(r.out.empty());
      CHECK(r.err == expected);
      CHECK_FALSE(has(r.err, "\"file\""));
      if (!secret.empty())
        CHECK_FALSE(has(r.err, secret));
    }
  };
  refused("extra top-level file", [](const observation_dir& d) { d.put("secretnotes.txt", "x"); }, "secretnotes");
  refused("other subdirectory", [](const observation_dir& d) { d.mkdir("secretdir"); }, "secretdir");
  refused("non-json file under checks", [](const observation_dir& d) { d.put("checks/secretreadme.txt", "x"); }, "secretreadme");
  refused("non-json file under findings", [](const observation_dir& d) { d.put("findings/secretnote.md", "x"); }, "secretnote");
  refused("uppercase extension", [](const observation_dir& d) { d.put("checks/a.JSON", check_json("c1")); }, "");
  refused("checks is a file", [](const observation_dir& d) { d.put("checks", "x"); }, "");
  refused("findings is a file", [](const observation_dir& d) { d.put("findings", "x"); }, "");
  refused("checkpoint.json is a directory", [](const observation_dir& d) { d.mkdir("checkpoint.json"); }, "");
  refused(
      "control byte in a checks name", [](const observation_dir& d) { d.put("checks/secret\x01.json", check_json("c1")); },
      "secret");
  refused("control byte in a top-level name", [](const observation_dir& d) { d.put("secret\x07.json", "x"); }, "secret");
  refused(
      "name outside the portable grammar", [](const observation_dir& d) { d.put("checks/secret*name.json", check_json("c1")); },
      "secret");
#if defined(__linux__)
  // Linux only: APFS cannot create a name that is invalid UTF-8.
  refused(
      "invalid UTF-8 name under checks", [](const observation_dir& d) { d.put("checks/secret\xff\xfe.json", check_json("c1")); },
      "secret");
  refused("invalid UTF-8 top-level name", [](const observation_dir& d) { d.put("secret\xc3.json", "x"); }, "secret");
#endif
}

TEST_CASE("RA01-B2 observe ignores dot names", "[authoring][ra01]") {
  // Mutants: dot names inspected (the dot symlink or FIFO is refused), or opened (a FIFO would block). The bound
  // counting of dot names is pinned by the directory seam tests.
  observation_dir d;
  d.put("checks/a.json", check_json("c1"));
  d.symlink(".hidden-link", "checks/a.json");
  d.fifo(".hidden-fifo");
  d.put(".DS_Store", "binary");
  d.mkdir(".git");
  d.symlink("checks/.dot-link", "a.json");
  d.fifo("findings/.dot-fifo");
  d.put("findings/.not-json", "x");
  const auto r = observe(d);
  check_success(r, "{\"checks\":[" + canon_check("c1") + "],\"findings\":[],\"version\":1}\n");
}

TEST_CASE("RA01-B2 observe output over one MiB is refused as observation_bytes", "[authoring][ra01]") {
  // Mutant: execute() uses the generic 16 MiB output ceiling for observe (the 1.3 MB document prints).
  const std::string summary(4096, 's');
  SECTION("positive control under the ceiling") {
    observation_dir d;
    for (int i = 0; i < 200; ++i)
      d.put(std::format("checks/c{:04}.json", i), check_json(std::format("c{:04}", i), "pass", summary));
    const auto r = observe(d);
    CHECK(r.status == 0);
    CHECK(r.err.empty());
    CHECK(r.out.size() > 800000);
    CHECK(r.out.size() < 1048576);
  }
  SECTION("over the ceiling") {
    observation_dir d;
    for (int i = 0; i < 300; ++i)
      d.put(std::format("checks/c{:04}.json", i), check_json(std::format("c{:04}", i), "pass", summary));
    const auto r = observe(d);
    CAPTURE(r.err.size());
    CHECK(r.status == 3);
    CHECK(r.out.empty());
    CHECK(has(r.err, "\"code\":\"resource_limit\""));
    CHECK(has(r.err, "\"details\":{\"limit\":\"observation_bytes\",\"maximum\":1048576}"));
  }
}

TEST_CASE("RA01-B2 observe refuses duplicate ids within checks and within findings", "[authoring][ra01]") {
  // Mutants: duplicates detected across checks and findings together (the shared-id control fails); the earlier file
  // blamed instead of the later one in name order (files are created in reverse order).
  SECTION("checks") {
    observation_dir d;
    d.put("checks/b.json", check_json("same"));
    d.put("checks/a.json", check_json("same"));
    const auto r = observe(d);
    check_failure(r, "reference", "checks/b.json", "id");
    CHECK(has(r.err, "\"message\":\"Duplicate value\""));
  }
  SECTION("findings") {
    observation_dir d;
    d.put("findings/y.json", finding_json("same"));
    d.put("findings/x.json", finding_json("same"));
    const auto r = observe(d);
    check_failure(r, "reference", "findings/y.json", "id");
    CHECK(has(r.err, "\"message\":\"Duplicate value\""));
  }
  SECTION("the same id in a check and a finding is allowed") {
    observation_dir d;
    d.put("checks/a.json", check_json("same"));
    d.put("findings/a.json", finding_json("same"));
    const auto r = observe(d);
    check_success(r, "{\"checks\":[" + canon_check("same") + "],\"findings\":[" + canon_finding("same") + "],\"version\":1}\n");
  }
}

TEST_CASE("RA01-B2 observe errors carry the logical file and object-relative field", "[authoring][ra01]") {
  // Mutant: the observations.checks[1] prefix left on the field path (or the file replaced by a host path).
  const auto no_prefix = [](const observe_result& r) {
    CHECK_FALSE(has(r.err, "observations."));
    CHECK_FALSE(has(r.err, "checks[1]"));
    CHECK_FALSE(has(r.err, "findings[1]"));
    CHECK_FALSE(has(r.err, "\"field\":\"checkpoint."));
    CHECK_FALSE(has(r.err, "/tmp/"));
  };
  SECTION("unknown field in a check") {
    observation_dir d;
    d.put("checks/a.json", R"({"id":"c","check":"unit","target":{"work":"w1","output":")" + digest_a +
                               R"("},"reported":"pass","summary":"s","vector":[],"bogus":1})");
    const auto r = observe(d);
    check_failure(r, "schema", "checks/a.json", "bogus");
    CHECK(has(r.err, "\"message\":\"Unknown field\""));
    no_prefix(r);
  }
  SECTION("unknown nested field in a finding target") {
    observation_dir d;
    d.put("findings/b.json",
          R"({"id":"f","target":{"work":"w1","output":")" + digest_a + R"(","extra":1},"severity":"note","text":"t"})");
    const auto r = observe(d);
    check_failure(r, "schema", "findings/b.json", "target.extra");
    no_prefix(r);
  }
  SECTION("unknown field in the checkpoint") {
    observation_dir d;
    d.put("checkpoint.json", R"({"progress":"p","next":"n","unresolved":"u","files":[],"more":true})");
    const auto r = observe(d);
    check_failure(r, "schema", "checkpoint.json", "more");
    no_prefix(r);
  }
  SECTION("null value") {
    observation_dir d;
    d.put("checks/a.json", R"({"id":"c","check":"unit","target":{"work":"w1","output":")" + digest_a +
                               R"("},"reported":"pass","summary":null,"vector":[]})");
    const auto r = observe(d);
    // The shared strict codec refuses a null before normalization, exactly as render does for the same bytes.
    check_failure(r, "schema", "checks/a.json", "$");
    CHECK(has(r.err, "\"message\":\"JSON codec refused input\""));
    no_prefix(r);
  }
  SECTION("non-integer number where a string is required") {
    observation_dir d;
    d.put("findings/a.json", R"({"id":"f","target":{"work":"w1","output":")" + digest_a + R"("},"severity":"note","text":1.5})");
    const auto r = observe(d);
    // The shared strict codec refuses a non-integer number before normalization, exactly as render does.
    check_failure(r, "schema", "findings/a.json", "$");
    CHECK(has(r.err, "\"message\":\"JSON codec refused input\""));
  }
  SECTION("root value that is not an object") {
    observation_dir d;
    d.put("checks/a.json", "[]");
    const auto r = observe(d);
    check_failure(r, "schema", "checks/a.json", "$");
    no_prefix(r);
  }
  SECTION("duplicate keys") {
    observation_dir d;
    d.put("findings/a.json",
          R"({"id":"f","id":"g","target":{"work":"w1","output":")" + digest_a + R"("},"severity":"note","text":"t"})");
    const auto r = observe(d);
    CHECK(r.status == 3);
    CHECK(has(r.err, "\"code\":\"schema\""));
    CHECK(has(r.err, "\"file\":\"findings/a.json\""));
  }
  SECTION("invalid UTF-8") {
    observation_dir d;
    d.put("checks/a.json", std::string("{\"id\":\"c\",\"summary\":\"\xff\xfe\"}"));
    const auto r = observe(d);
    CHECK(r.status == 3);
    CHECK(has(r.err, "\"code\":\"schema\""));
    CHECK(has(r.err, "\"file\":\"checks/a.json\""));
  }
  SECTION("NUL byte") {
    observation_dir d;
    std::string     nul = "{\"id\":\"c\",\"summary\":\"a";
    nul.push_back('\0');
    nul += "b\"}";
    d.put("checks/a.json", nul);
    const auto r = observe(d);
    CHECK(r.status == 3);
    CHECK(has(r.err, "\"code\":\"schema\""));
    CHECK(has(r.err, "\"file\":\"checks/a.json\""));
  }
  SECTION("escaped NUL") {
    observation_dir d;
    d.put("findings/a.json",
          R"({"id":"f","target":{"work":"w1","output":")" + digest_a + R"("},"severity":"note","text":"a\u0000b"})");
    const auto r = observe(d);
    CHECK(r.status == 3);
    CHECK(has(r.err, "\"code\":\"schema\""));
    CHECK(has(r.err, "\"file\":\"findings/a.json\""));
  }
}

TEST_CASE("RA01-B2 observe applies structural checks only", "[authoring][ra01]") {
  // Mutant: observe reuses render's semantic checks (a well-formed digest for a missing output, an unregistered check
  // or a mismatching vector is then refused as observation_target_mismatch).
  SECTION("malformed target digest in a check") {
    observation_dir d;
    d.put("checks/a.json", check_json("c1", "pass", "ok", "w1", "xxh3_128:XYZ"));
    const auto r = observe(d);
    check_failure(r, "schema", "checks/a.json", "target.output");
    CHECK(has(r.err, "\"message\":\"Malformed output digest\""));
  }
  SECTION("uppercase hex is malformed") {
    observation_dir d;
    auto            upper = finding_json("f1");
    upper.replace(upper.find("00112233"), 8, "AABBCCDD");
    d.put("findings/a.json", upper);
    const auto r = observe(d);
    check_failure(r, "schema", "findings/a.json", "target.output");
    CHECK(has(r.err, "\"message\":\"Malformed output digest\""));
  }
  SECTION("malformed vector digest") {
    observation_dir d;
    d.put("checks/a.json", R"({"id":"c","check":"unit","target":{"work":"w1","output":")" + digest_a +
                               R"("},"reported":"pass","summary":"s","vector":[{"work":"w1","output":"xxh3_128:00"}]})");
    const auto r = observe(d);
    CHECK(r.status == 3);
    CHECK(r.out.empty());
    CHECK(has(r.err, "\"code\":\"schema\""));
    CHECK(has(r.err, "\"file\":\"checks/a.json\""));
    CHECK(has(r.err, "\"field\":\"vector["));
    CHECK(has(r.err, "].output\""));
    CHECK(has(r.err, "\"message\":\"Malformed output digest\""));
  }
  SECTION("well-formed digest for output that does not exist is accepted") {
    observation_dir d;
    d.put("checks/a.json", R"({"id":"c","check":"nonexistent_check","target":{"work":"no_such_unit","output":")" + digest_a +
                               R"("},"reported":"inconclusive","summary":"s","vector":[]})");
    d.put("findings/a.json",
          R"({"id":"f","target":{"work":"no_such_unit","output":")" + digest_a + R"("},"severity":"required","text":"t"})");
    d.put("checkpoint.json", checkpoint_json("p", "n", "u", {"anywhere/at/all.txt"}));
    const auto r = observe(d);
    CHECK(r.status == 0);
    CHECK(r.err.empty());
    CHECK_FALSE(has(r.out, "observation_target_mismatch"));
    CHECK(has(r.out, "\"no_such_unit\""));
    CHECK(has(r.out, "\"nonexistent_check\""));
    CHECK(has(r.out, "\"anywhere/at/all.txt\""));
  }
}

TEST_CASE("RA01-B2 observe enforces the checkpoint bounds", "[authoring][ra01]") {
  // Mutants: the file count check dropped, or 33 used as the bound; the canonical size check dropped.
  const auto many = [](int count, std::size_t length) {
    std::vector<std::string> files;
    for (int i = 0; i < count; ++i) {
      auto name = std::format("d/f{:02}-", i);
      name.append(length > name.size() + 4 ? length - name.size() - 4 : 0, 'p');
      files.push_back(name + ".txt");
    }
    return files;
  };
  const std::string two_k(2048, 'x');
  SECTION("32 paths and a mid-size checkpoint are accepted") {
    observation_dir d;
    d.put("checkpoint.json", checkpoint_json(two_k, two_k, two_k, many(32, 12)));
    const auto r = observe(d);
    CHECK(r.status == 0);
    CHECK(r.err.empty());
  }
  SECTION("33 paths") {
    observation_dir d;
    d.put("checkpoint.json", checkpoint_json("p", "n", "u", many(33, 12)));
    const auto r = observe(d);
    check_failure(r, "resource_limit", "checkpoint.json", "");
    CHECK(has(r.err, "\"details\":{\"limit\":\"checkpoint_files\",\"maximum\":32}"));
    CHECK(has(r.err, "\"message\":\"Too many checkpoint paths\""));
  }
  SECTION("canonical JSON over 8192 bytes") {
    observation_dir d;
    d.put("checkpoint.json", checkpoint_json(two_k, two_k, two_k, many(32, 100)));
    const auto r = observe(d);
    check_failure(r, "resource_limit", "checkpoint.json", "");
    CHECK(has(r.err, "\"details\":{\"limit\":\"checkpoint_bytes\",\"maximum\":8192}"));
    CHECK(has(r.err, "\"message\":\"Checkpoint canonical JSON too large\""));
  }
}

TEST_CASE("RA01-B2 observe reports the first error in checks, findings, checkpoint order", "[authoring][ra01]") {
  // Mutant: the checkpoint (or findings) validated before checks. Every file is invalid in its own way and the first
  // error in unsigned name order, checks then findings then checkpoint, wins.
  observation_dir d;
  const auto      with_extra = [](std::string json, std::string_view key) {
    json.pop_back();
    return json + std::format(",\"{}\":1}}", key);
  };
  d.put("checkpoint.json", with_extra(checkpoint_json("p", "n", "u", {}), "cp_bad"));
  d.put("findings/a.json", with_extra(finding_json("f"), "find_bad"));
  d.put("checks/z.json", with_extra(check_json("c"), "check_bad"));
  d.put("checks/b.json", with_extra(check_json("c"), "b_bad"));
  check_failure(observe(d), "schema", "checks/b.json", "b_bad");
  d.remove("checks/b.json");
  check_failure(observe(d), "schema", "checks/z.json", "check_bad");
  d.remove("checks/z.json");
  check_failure(observe(d), "schema", "findings/a.json", "find_bad");
  d.remove("findings/a.json");
  check_failure(observe(d), "schema", "checkpoint.json", "cp_bad");
}

TEST_CASE("RA01-B2 observe prints sorted bare observations", "[authoring][ra01]") {
  // Mutants: checks sorted by file name instead of id (files are named opposite to their ids); a wrapper key such as
  // {kind:"observe"}; the checkpoint key present without checkpoint.json. Keys are in canonical sorted order.
  observation_dir d;
  d.put("checks/a.json", check_json("z_id", "fail", "second"));
  d.put("checks/b.json", check_json("a_id", "pass", "first"));
  d.put("findings/a.json", finding_json("y_id", "later"));
  d.put("findings/b.json", finding_json("x_id", "earlier"));
  SECTION("without a checkpoint") {
    check_success(observe(d), "{\"checks\":[" + canon_check("a_id", "pass", "first") + "," +
                                  canon_check("z_id", "fail", "second") + "],\"findings\":[" + canon_finding("x_id", "earlier") +
                                  "," + canon_finding("y_id", "later") + "],\"version\":1}\n");
  }
  SECTION("with a checkpoint") {
    d.put("checkpoint.json", checkpoint_json("p", "n", "u", {"src/b.txt", "src/a.txt"}));
    check_success(observe(d), "{\"checkpoint\":{\"files\":[\"src/b.txt\",\"src/a.txt\"],\"next\":\"n\",\"progress\":\"p\","
                              "\"unresolved\":\"u\"},\"checks\":[" +
                                  canon_check("a_id", "pass", "first") + "," + canon_check("z_id", "fail", "second") +
                                  "],\"findings\":[" + canon_finding("x_id", "earlier") + "," + canon_finding("y_id", "later") +
                                  "],\"version\":1}\n");
  }
}

TEST_CASE("RA01-B2 observe empty directory prints canonical empty observations", "[authoring][ra01]") {
  // Mutant: the empty arrays omitted. The contract's literal {"version":1,"checks":[],"findings":[]} is the logical
  // order; the contract also fixes the canonical JSON key order as sorted, which is what is typed here.
  SECTION("empty directory") {
    observation_dir d;
    check_success(observe(d), sorted_empty());
  }
  SECTION("missing checks and findings directories are empty, present empty ones too") {
    observation_dir d;
    d.mkdir("checks");
    check_success(observe(d), sorted_empty());
    d.remove("checks");
    d.mkdir("findings");
    check_success(observe(d), sorted_empty());
  }
}

TEST_CASE("RA01-B2 observe output is independent of creation order", "[authoring][ra01]") {
  // Mutant: listing order kept instead of sorting by id (the second directory is created in the reverse order).
  const std::vector<std::pair<std::string, std::string>> files{{"checks/a.json", check_json("m")},
                                                               {"checks/b.json", check_json("c")},
                                                               {"checks/c.json", check_json("t")},
                                                               {"findings/a.json", finding_json("q")},
                                                               {"findings/b.json", finding_json("b")},
                                                               {"findings/c.json", finding_json("k")},
                                                               {"checkpoint.json", checkpoint_json("p", "n", "u", {"a.txt"})}};
  observation_dir                                        forward, backward;
  for (const auto& [name, bytes] : files)
    forward.put(name, bytes);
  for (auto it = files.rbegin(); it != files.rend(); ++it)
    backward.put(it->first, it->second);
  const auto first  = observe(forward);
  const auto second = observe(backward);
  CHECK(first.status == 0);
  CHECK(first.out.size() > 100);
  CHECK(first.out == second.out);
  CHECK(first.err == second.err);
  CHECK(first.out ==
        "{\"checkpoint\":{\"files\":[\"a.txt\"],\"next\":\"n\",\"progress\":\"p\",\"unresolved\":\"u\"},\"checks\":[" +
            canon_check("c") + "," + canon_check("m") + "," + canon_check("t") + "],\"findings\":[" + canon_finding("b") + "," +
            canon_finding("k") + "," + canon_finding("q") + "],\"version\":1}\n");
}

TEST_CASE("RA01-B2 observe output is structurally accepted by render observations", "[authoring][ra01]") {
  // Mutant: an extra top-level key in the observe output (render's strict validator then refuses it as schema).
  const fs::path fixture = fs::path(PLANAR_B2_WORKSPACE_FIXTURE).parent_path().parent_path();
  const auto     render  = [&](const std::string& observations) {
    return run_stack({"render", "--workspace", (fixture / "workspace/workspace.lua").string(), "--registry",
                      (fixture / "registry/quota-tools.json").string(), "--root", "source=" + (fixture / "current").string(),
                      "--job", "combined", "--role", "builder", "--observations", observations});
  };
  observation_dir d;
  SECTION("empty observations are accepted by render") {
    const auto produced = observe(d);
    REQUIRE(produced.status == 0);
    d.put("../observations.json", produced.out);
    const auto r = render((d.root.parent_path() / "observations.json").string());
    CAPTURE(r.err);
    CHECK(r.status == 0);
  }
  SECTION("a well-formed target is refused by render as a target mismatch, not as schema") {
    d.put("findings/a.json", finding_json("f1"));
    const auto produced = observe(d);
    REQUIRE(produced.status == 0);
    d.put("../observations.json", produced.out);
    const auto r = render((d.root.parent_path() / "observations.json").string());
    CAPTURE(r.err);
    CHECK(r.status == 3);
    CHECK(r.out.empty());
    CHECK(has(r.err, "\"code\":\"observation_target_mismatch\""));
    CHECK_FALSE(has(r.err, "\"code\":\"schema\""));
  }
}

// ---- RA01-B2 W2-C01 corrective (package T-B) -----------------------------------------------------------------------------

namespace {
/// Restores owner access to a path when the scope ends, so the observation tree can be removed by its owner.
struct c01_mode_restore {
  fs::path path;
  ~c01_mode_restore() {
    std::error_code ignored;
    fs::permissions(path, fs::perms::owner_all, ignored);
  }
};
/// Whitespace-free text of a product source file beside this test (src/cmd/planar); empty when it cannot be read.
std::string c01_squashed_source(const fs::path& file) {
  std::ifstream     in(file, std::ios::binary);
  const std::string text{std::istreambuf_iterator<char>(in), {}};
  std::string       out;
  for (const char c : text)
    if (!std::isspace(static_cast<unsigned char>(c)))
      out += c;
  return out;
}
std::size_t c01_count(std::string_view text, std::string_view needle) {
  std::size_t count = 0;
  for (auto at = text.find(needle); at != std::string_view::npos; at = text.find(needle, at + needle.size()))
    ++count;
  return count;
}
/// The closed io envelope of an unreadable observation entry or root (details carry the capture operation).
const std::string c01_io_envelope = R"({"error":{"code":"io","details":{"operation":"capture"},)"
                                    R"("field":"","message":"Cannot inspect confined input"},"kind":"error","version":1})"
                                    "\n";
} // namespace

// Mutant (C01-04, directory.cpp cannot_inspect at line 49): the message changed to `Cannot open confined input`, or the
// pair changed to another code. Also observes C01-01 and C01-02 at the CLI level (an unreadable entry reclassified as
// path_denied). Root-aware: as root, mode 000 does not deny the open, so the sections succeed without exercising it.
TEST_CASE("RA01-B2 C01 observe refuses an unreadable entry as io exit 4", "[authoring][ra01]") {
  SECTION("unreadable checks directory") {
    if (::geteuid() == 0) {
      SUCCEED("permission denial is not observable as root");
    } else {
      observation_dir d;
      d.put("checks/a.json", check_json("c1"));
      fs::permissions(d.root / "checks", fs::perms::none);
      const c01_mode_restore restore{d.root / "checks"};
      const auto             r = observe(d);
      CHECK(r.status == 4);
      CHECK(r.out.empty());
      CHECK(r.err == c01_io_envelope);
    }
  }
  SECTION("unreadable check file") {
    if (::geteuid() == 0) {
      SUCCEED("permission denial is not observable as root");
    } else {
      observation_dir d;
      d.put("checks/a.json", check_json("c1"));
      fs::permissions(d.root / "checks/a.json", fs::perms::none);
      const c01_mode_restore restore{d.root / "checks/a.json"};
      const auto             r = observe(d);
      CHECK(r.status == 4);
      CHECK(r.out.empty());
      CHECK(r.err == c01_io_envelope);
    }
  }
}

// Mutant (C01-33, D-C01-05 A; directory.cpp read_observation_directory_impl root open at line 260): the cannot_inspect()
// result for the root becomes denied_kind() (path_denied, exit 3), or any byte of the io pair changes. Root-aware: as
// root, mode 000 does not deny the open, so the test succeeds without exercising the refusal.
TEST_CASE("RA01-B2 C01 observe refuses an unreadable root as io exit 4", "[authoring][ra01]") {
  if (::geteuid() == 0) {
    SUCCEED("permission denial is not observable as root");
    return;
  }
  observation_dir d;
  d.put("checks/a.json", check_json("c1"));
  fs::permissions(d.root, fs::perms::none);
  const c01_mode_restore restore{d.root};
  const auto             r = observe(d);
  CHECK(r.status == 4);
  CHECK(r.out.empty());
  CHECK(r.err == c01_io_envelope);
}

// Mutants (C01-18 and C01-19, observe.lua line 62 `> 8192`): the bound becomes `> 8193` (8193 bytes accepted) or
// `>= 8192` (8192 bytes refused). The canonical form is typed here: compact JSON with sorted keys and an ASCII path, with
// the pad length in `unresolved` chosen so the whole text is exactly the target size.
TEST_CASE("RA01-B2 C01 observe checkpoint canonical size is bounded at exactly 8192 bytes", "[authoring][ra01]") {
  // Mutants (C01-18 and C01-19, observe.lua line 62 `> 8192`): the bound becomes `> 8193` (8193 bytes accepted) or
  // `>= 8192` (8192 bytes refused). The canonical form is typed here: compact JSON with sorted keys and ASCII paths. The
  // three strings are each 2048 bytes (the contract's per-string bound) and the 32 paths (the file bound) carry the pad,
  // each path segment kept under the 128-byte portable segment bound.
  const std::string progress(2048, 'p');
  const std::string next(2048, 'n');
  const std::string unresolved(2048, 'u');
  const auto        canonical_with = [&](std::size_t extra) {
    std::string list;
    std::size_t left = extra;
    for (int i = 0; i < 32; ++i) {
      const auto take = std::min<std::size_t>(left, 100);
      left -= take;
      list += std::format("{}\"d/f{:02}-{}.txt\"", i ? "," : "", i, std::string(take, 'a'));
    }
    REQUIRE(left == 0);
    return std::format(R"({{"files":[{}],"next":"{}","progress":"{}","unresolved":"{}"}})", list, next, progress, unresolved);
  };
  const auto canonical_of = [&](std::size_t target) {
    const auto base = canonical_with(0).size();
    REQUIRE(target >= base);
    return canonical_with(target - base);
  };
  SECTION("8192 canonical bytes are accepted") {
    const auto canonical = canonical_of(8192);
    REQUIRE(canonical.size() == 8192);
    observation_dir d;
    d.put("checkpoint.json", canonical);
    const auto r = observe(d);
    CAPTURE(r.err);
    CHECK(r.status == 0);
    CHECK(r.err.empty());
  }
  SECTION("8193 canonical bytes are refused as checkpoint_bytes") {
    const auto canonical = canonical_of(8193);
    REQUIRE(canonical.size() == 8193);
    observation_dir d;
    d.put("checkpoint.json", canonical);
    const auto r = observe(d);
    check_failure(r, "resource_limit", "checkpoint.json", "");
    CHECK(has(r.err, "\"details\":{\"limit\":\"checkpoint_bytes\",\"maximum\":8192}"));
    CHECK(has(r.err, "\"message\":\"Checkpoint canonical JSON too large\""));
  }
}

// Mutants (C01-44, invocation.cpp read_observation_directory, D-C01-32): `throw digest.error();` restored, so an
// allocation failure passes through instead of observation_total_bytes 4194304. C02-02 drops the `Canonical allocation
// failed` alternative, C02-04 turns || into && or writes a wrong limit name or maximum in the return, C02-36 inserts a
// separate canonical_bytes branch before the Hash branch. Both messages and the exact condition spelling are pinned. The
// helper body is bounded by its signature and the first #endif after it, with no fixed character window. SOURCE GUARD: no
// allocator fault hook may be added, so the guard pins the spelling of the remap, not that an allocation fails.
TEST_CASE("RA01-B2 C01 observe digest allocation failure is a bound refusal", "[authoring][ra01]") {
  const auto text = c01_squashed_source(fs::path(__FILE__).parent_path() / "invocation.cpp");
  REQUIRE_FALSE(text.empty());
  CHECK(c01_count(text, "throwobservation_digest_failure(std::move(digest.error()));") == 1);
  CHECK_FALSE(text.contains("throwdigest.error();"));
  // The helper body runs from its definition signature (not the call site) to the first #endif after it.
  const std::string signature = "diagnosticobservation_digest_failure(diagnosticerror){";
  std::string       helper;
  if (const auto at = text.find(signature); at != std::string::npos) {
    const auto stop = text.find("#endif", at);
    if (stop != std::string::npos)
      helper = text.substr(at, stop - at);
  }
  CHECK_FALSE(helper.empty());
  CHECK(helper.contains("Hashallocationfailed"));
  CHECK(helper.contains("Canonicalallocationfailed"));
  CHECK(helper.contains("capture_limit(\"observation_total_bytes\",4194304)"));
  CHECK(c01_count(helper, "if(error.code==\"resource_limit\"&&(error.message==\"Hashallocationfailed\"||error.message=="
                          "\"Canonicalallocationfailed\"))"
                          "returncapture_limit(\"observation_total_bytes\",4194304);") == 1);
  CHECK(c01_count(helper, "capture_limit(") == 1);
}
