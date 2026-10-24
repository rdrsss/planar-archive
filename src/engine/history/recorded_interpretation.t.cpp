// Pure historical dispatch verifies provenance even when fixed interpretation semantics are unsupported.
#include <catch2/catch_test_macros.hpp>
import std;
import planar.engine.history.recorded_interpretation;
import planar.engine.canonical;
namespace allocation_probe {
thread_local bool         active    = false;
thread_local std::int64_t countdown = -1;
thread_local std::size_t  calls     = 0;
} // namespace allocation_probe
void* operator new(std::size_t n) {
  if (allocation_probe::active) {
    ++allocation_probe::calls;
    if (allocation_probe::countdown == 0) {
      allocation_probe::active = false;
      throw std::bad_alloc();
    }
    if (allocation_probe::countdown > 0)
      --allocation_probe::countdown;
  }
  if (auto* p = std::malloc(n ? n : 1))
    return p;
  throw std::bad_alloc();
}
void operator delete(void* p) noexcept {
  std::free(p);
}
void* operator new[](std::size_t n) {
  return ::operator new(n);
}
void operator delete[](void* p) noexcept {
  ::operator delete(p);
}
using namespace planar::engine;
using namespace planar::engine::history;
namespace {
using code = recorded_error_code;
std::string read(std::string_view dir, std::string_view name) {
  std::ifstream in(std::string(dir) + "/" + std::string(name), std::ios::binary);
  REQUIRE(in.good());
  return {std::istreambuf_iterator<char>(in), {}};
}
value_ptr dec(std::string_view bytes) {
  auto r = canonical_decode(bytes);
  REQUIRE(r);
  return *r;
}
std::string enc(value_ptr v) {
  auto r = canonical_encode(v);
  REQUIRE(r);
  return *r;
}
auto mut(value_ptr v) {
  return std::const_pointer_cast<value>(v);
}
auto at(value_ptr v, std::string key) {
  return mut(v->fields.at(key));
}
value_ptr num(std::int64_t n) {
  auto v     = std::make_shared<value>();
  v->kind    = value_kind::integer;
  v->integer = n;
  return v;
}
value_ptr str(std::string s) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = std::move(s);
  return v;
}
std::vector<tree_file> effective() {
  std::istringstream     in(read(PLANAR_V2_VECTORS, "files.tsv"));
  std::string            line;
  std::vector<tree_file> files;
  while (std::getline(in, line)) {
    if (line.starts_with('#'))
      continue;
    auto        a = line.find('\t'), b = line.find('\t', a + 1);
    std::string bytes;
    for (std::size_t i = b + 1; i < line.size(); i += 2)
      bytes += static_cast<char>(std::stoul(line.substr(i, 2), nullptr, 16));
    files.push_back({line.substr(0, a), std::move(bytes), static_cast<std::uint32_t>(std::stoul(line.substr(a + 1, b - a - 1)))});
  }
  return files;
}
std::string hex(record_digest digest) {
  std::string s;
  for (auto b : digest)
    s += std::format("{:02x}", b);
  return s;
}
struct input {
  value_ptr              root;
  std::vector<tree_file> files;
  std::string            workspace() const {
    return at(at(at(root, "bundle"), "workspace"), "id")->string;
  }
  auto check() const {
    return verify_recorded_interpretation(enc(root), enc(at(root, "versions")), files, workspace());
  }
};
input v2() {
  return {dec(read(PLANAR_V2_VECTORS, "interpretation.bin")), effective()};
}
input v1() {
  auto root = dec(read(PLANAR_V1_GOLDENS, "interpretation.canonical"));
  auto tree = prepare_stdlib(std::array{original_file_view{"a.lua", "recorded bytes"}}, "\n", "recorded bytes\n");
  REQUIRE(tree);
  at(root, "versions")->fields["stdlib_digest"] = str(hex(tree->digest));
  return {root, std::move(tree->files)};
}
void refused(const input& in, code expected) {
  auto r = in.check();
  REQUIRE_FALSE(r);
  CHECK(r.error().code == expected);
}
void manifest(input& in, const std::function<void(value_ptr)>& change) {
  auto i = std::ranges::find(in.files, "__planar__/effective.bin", &tree_file::path);
  REQUIRE(i != in.files.end());
  auto m = dec(i->payload);
  change(m);
  i->payload = enc(m);
}
} // namespace
TEST_CASE("recorded dispatch preserves original v1 decoder supported future and malformed precedence",
          "[recorded_interpretation]") {
  auto b = read(PLANAR_V1_GOLDENS, "interpretation.canonical"), v = read(PLANAR_V1_GOLDENS, "versions.canonical");
  REQUIRE(decode_interpretation(b, v));
  auto root                    = dec(b);
  mut(root)->fields["version"] = num(99);
  auto future                  = decode_interpretation(enc(root), v);
  REQUIRE_FALSE(future);
  CHECK(future.error().code == interpretation_error_code::unsupported_version);
  at(root, "semantic")->fields.erase("jobs");
  auto bad = decode_interpretation(enc(root), v);
  REQUIRE_FALSE(bad);
  CHECK(bad.error().code == interpretation_error_code::malformed);
}
TEST_CASE("recorded dispatch supports both fixed families with recomputed tree workspace and owned bytes",
          "[recorded_interpretation]") {
  for (auto in : {v1(), v2()}) {
    auto r = in.check();
    REQUIRE(r);
    CHECK(r->status == recorded_status::decoded);
    REQUIRE(r->decoded);
    CHECK(r->decoded->encoded == enc(in.root));
    auto verified =
        r->family == recorded_family::embedded ? verify_stdlib(in.files)->digest : verify_effective_stdlib(in.files)->digest;
    CHECK(r->digest == verified);
    auto encoded                            = r->decoded->encoded;
    at(in.root, "versions")->fields["hash"] = str("changed");
    in.files.clear();
    in.root.reset();
    CHECK(r->decoded->encoded == encoded);
    CHECK(at(r->decoded->versions, "hash")->string == "xxh3_128_v1");
  }
}
TEST_CASE("recorded family markers are presence hints and numeric future version stays embedded", "[recorded_interpretation]") {
  auto in                         = v1();
  mut(in.root)->fields["version"] = num(2);
  CHECK(classify_recorded_interpretation(enc(in.root), enc(at(in.root, "versions"))) == recorded_family::embedded);
  auto r = in.check();
  REQUIRE(r);
  CHECK(r->status == recorded_status::unsupported_interpretation);
  CHECK_FALSE(r->decoded);
  for (auto marker : {"provenance", "registration"}) {
    auto old                                = enc(in.root);
    at(in.root, "versions")->fields[marker] = num(0);
    CHECK(classify_recorded_interpretation(enc(in.root), enc(at(in.root, "versions"))) == recorded_family::effective);
    in.root                       = dec(old);
    auto separate                 = dec(enc(at(in.root, "versions")));
    mut(separate)->fields[marker] = num(0);
    CHECK(classify_recorded_interpretation(enc(in.root), enc(separate)) == recorded_family::effective);
  }
  mut(in.root)->fields["extensions"] = num(0);
  CHECK(classify_recorded_interpretation(enc(in.root), enc(at(in.root, "versions"))) == recorded_family::effective);
  CHECK(classify_recorded_interpretation(enc(num(2)), enc(num(3))) == recorded_family::embedded);
}
TEST_CASE("recorded unknown semantics still require workspace and raw tree digest correspondence", "[recorded_interpretation]") {
  for (auto in : {v1(), v2()}) {
    mut(in.root)->fields["version"] = num(999);
    auto r                          = in.check();
    REQUIRE(r);
    CHECK(r->status == recorded_status::unsupported_interpretation);
    CHECK_FALSE(r->decoded);
    auto wrong = verify_recorded_interpretation(enc(in.root), enc(at(in.root, "versions")), in.files, "wrong");
    REQUIRE_FALSE(wrong);
    CHECK(wrong.error().code == code::invalid_correspondence);
    at(in.root, "versions")->fields["stdlib_digest"] = str(std::string(32, '0'));
    refused(in, code::invalid_correspondence);
  }
}
TEST_CASE("recorded unsupported v2 cannot skip changed actual raw provenance", "[recorded_interpretation]") {
  auto in                         = v2();
  mut(in.root)->fields["version"] = num(999);
  auto i                          = std::ranges::find(in.files, "external/shapes.lua", &tree_file::path);
  if (i == in.files.end())
    i = std::ranges::find_if(in.files, [](auto& f) { return f.path.starts_with("external/"); });
  REQUIRE(i != in.files.end());
  i->payload += "changed";
  // A changed external module remains structurally valid provenance but has a different recomputed digest.
  refused(in, code::invalid_correspondence);
  in.files.erase(i);
  refused(in, code::invalid_tree);
}
TEST_CASE("recorded unsupported interpretation refuses malformed structure and separate versions mismatch",
          "[recorded_interpretation]") {
  for (auto in : {v1(), v2()}) {
    mut(in.root)->fields["version"]         = num(999);
    auto separate                           = enc(at(in.root, "versions"));
    at(in.root, "versions")->fields["hash"] = str("future");
    auto r                                  = verify_recorded_interpretation(enc(in.root), separate, in.files, in.workspace());
    REQUIRE_FALSE(r);
    CHECK(r.error().code == code::malformed);
    at(in.root, "semantic")->fields.erase("jobs");
    refused(in, code::malformed);
  }
}
TEST_CASE("recorded unknown effective manifest differs from unknown interpretation and malformed manifest",
          "[recorded_interpretation]") {
  for (auto name : {"version", "scheme"}) {
    auto in                         = v2();
    mut(in.root)->fields["version"] = num(999);
    manifest(in, [&](auto m) { mut(m)->fields[name] = name == std::string_view("version") ? num(99) : str("future"); });
    refused(in, code::unsupported_manifest);
    manifest(in, [](auto m) { mut(m)->fields["modules"] = str("malformed"); });
    refused(in, code::malformed);
  }
  auto in = v2();
  manifest(in, [](auto m) {
    at(m, "catalog")->fields["commands"] = num(3);
    mut(m)->fields["version"]            = num(99);
  });
  refused(in, code::malformed);
}
TEST_CASE("recorded original embedded manifest keeps typed future manifest refusal", "[recorded_interpretation]") {
  auto in = v1();
  auto i  = std::ranges::find(in.files, "__planar__/assembly.canonical", &tree_file::path);
  REQUIRE(i != in.files.end());
  auto m                    = dec(i->payload);
  mut(m)->fields["version"] = num(999);
  i->payload                = enc(m);
  refused(in, code::unsupported_manifest);
}
TEST_CASE("recorded marker mismatches cannot grant effective provenance authority", "[recorded_interpretation]") {
  auto in  = v1();
  in.files = effective();
  refused(in, code::malformed);
  in = v2();
  mut(in.root)->fields.erase("extensions");
  at(in.root, "versions")->fields.erase("registration");
  at(in.root, "versions")->fields.erase("provenance");
  CHECK(classify_recorded_interpretation(enc(in.root), enc(at(in.root, "versions"))) == recorded_family::embedded);
  refused(in, code::malformed);
  in                                 = v2();
  mut(in.root)->fields["extensions"] = num(0);
  refused(in, code::malformed);
}
TEST_CASE("recorded paired failures follow canonical topology provenance decoder correspondence order",
          "[recorded_interpretation]") {
  auto in                         = v2();
  mut(in.root)->fields["version"] = num(999);
  at(in.root, "semantic")->fields.erase("jobs");
  in.files.front().mode = 0;
  refused(in, code::invalid_tree);
  auto r = verify_recorded_interpretation("broken", enc(at(in.root, "versions")), in.files, "wrong");
  REQUIRE_FALSE(r);
  CHECK(r.error().code == code::malformed);
  in = v2();
  at(in.root, "semantic")->fields.erase("jobs");
  manifest(in, [](auto m) { mut(m)->fields["version"] = num(99); });
  refused(in, code::unsupported_manifest);
  in = v2();
  at(in.root, "semantic")->fields.erase("jobs");
  at(in.root, "versions")->fields["stdlib_digest"] = str(std::string(32, '0'));
  refused(in, code::malformed);
}
TEST_CASE("recorded canonical inputs preflight both byte limits and bounded depth before family hints",
          "[recorded_interpretation]") {
  auto        in = v1();
  std::string too_big(16 * 1048576 + 1, 'x');
  for (bool first : {true, false}) {
    auto r = classify_recorded_interpretation(first ? too_big : "bad", first ? "bad" : too_big);
    REQUIRE_FALSE(r);
    CHECK(r.error().code == code::resource_limit);
  }
  // Independent framing builds depth257 without asking the bounded product encoder to create invalid bytes.
  std::string bytes;
  auto        shallow = std::make_shared<value>();
  shallow->kind       = value_kind::list;
  shallow->items      = {num(0)};
  auto one = enc(shallow), leaf = enc(num(0));
  REQUIRE(one.ends_with(leaf));
  for (int i = 0; i < 256; ++i)
    bytes += one.substr(0, one.size() - leaf.size());
  bytes += leaf;
  auto r = classify_recorded_interpretation(bytes, enc(at(in.root, "versions")));
  REQUIRE_FALSE(r);
  CHECK(r.error().code == code::resource_limit);
}
TEST_CASE("recorded raw tree fixed byte ceilings distinguish embedded16 and effective32 MiB", "[recorded_interpretation]") {
  for (bool use_effective : {false, true}) {
    auto in  = use_effective ? v2() : v1();
    in.files = {{"a", std::string(16 * 1048576, 'x'), 420}};
    if (use_effective)
      in.files.push_back({"b", std::string(16 * 1048576, 'y'), 420});
    auto exact = in.check();
    REQUIRE_FALSE(exact);
    CHECK(exact.error().code == code::malformed);
    in.files.push_back({"c", "x", 420});
    refused(in, code::resource_limit);
  }
}
TEST_CASE("recorded raw tree fixed count ceilings bound admission before provenance", "[recorded_interpretation]") {
  for (bool use_effective : {false, true}) {
    auto in = use_effective ? v2() : v1();
    in.files.clear();
    std::size_t count = use_effective ? 8192 : 4096;
    for (std::size_t i = 0; i < count; ++i)
      in.files.push_back({std::format("f{:05}", i), "", 420});
    auto exact = in.check();
    REQUIRE_FALSE(exact);
    CHECK(exact.error().code == code::malformed);
    in.files.push_back({"z", "", 420});
    refused(in, code::resource_limit);
  }
}
TEST_CASE("recorded embedded valid provenance reaches4096 file ceiling with owned supported decode",
          "[recorded_interpretation]") {
  std::vector<std::string> paths;
  paths.reserve(4094);
  for (int i = 0; i < 4094; ++i)
    paths.push_back(std::format("f{:04}", i));
  std::vector<original_file_view> originals;
  for (auto& p : paths)
    originals.push_back({p, ""});
  auto tree = prepare_stdlib(originals, "", "");
  REQUIRE(tree);
  REQUIRE(tree->files.size() == 4096);
  auto in                                          = v1();
  in.files                                         = std::move(tree->files);
  at(in.root, "versions")->fields["stdlib_digest"] = str(hex(tree->digest));
  auto r                                           = in.check();
  REQUIRE(r);
  REQUIRE(r->decoded);
}
TEST_CASE("recorded dispatch maps every one-shot allocation refusal to typed resource", "[recorded_interpretation]") {
  for (bool effective_family : {false, true}) {
    auto in    = effective_family ? v2() : v1();
    auto bytes = enc(in.root), versions = enc(at(in.root, "versions")), workspace = in.workspace();
    for (bool only_classify : {true, false}) {
      allocation_probe::calls     = 0;
      allocation_probe::countdown = -1;
      allocation_probe::active    = true;
      bool success = only_classify ? static_cast<bool>(classify_recorded_interpretation(bytes, versions))
                                   : static_cast<bool>(verify_recorded_interpretation(bytes, versions, in.files, workspace));
      allocation_probe::active = false;
      REQUIRE(success);
      auto calls = allocation_probe::calls;
      REQUIRE(calls > 0);
      for (std::size_t i = 0; i < calls; ++i) {
        allocation_probe::countdown = static_cast<std::int64_t>(i);
        allocation_probe::active    = true;
        auto error = only_classify ? classify_recorded_interpretation(bytes, versions).error().code
                                   : verify_recorded_interpretation(bytes, versions, in.files, workspace).error().code;
        allocation_probe::active = false;
        INFO("family=" << effective_family << " classify=" << only_classify << " allocation=" << i << "/" << calls);
        CHECK(error == code::resource_limit);
      }
    }
  }
}
TEST_CASE("recorded valid embedded provenance reaches exact16MiB aggregate ceiling", "[recorded_interpretation]") {
  constexpr std::size_t cap = 16 * 1048576;
  std::string           payload, separator;
  std::array            originals{original_file_view{"a", ""}, original_file_view{"b", ""}};
  auto                  empty = prepare_stdlib(originals, "", "");
  REQUIRE(empty);
  std::size_t base = 0;
  for (auto& f : empty->files)
    base += f.payload.size();
  if ((cap - base) % 2)
    separator = "\n";
  auto adjusted = prepare_stdlib(originals, separator, separator + separator);
  REQUIRE(adjusted);
  base = 0;
  for (auto& f : adjusted->files)
    base += f.payload.size();
  REQUIRE((cap - base) % 2 == 0);
  payload.assign((cap - base) / 2, 'x');
  originals[0].bytes = payload;
  auto tree          = prepare_stdlib(originals, separator, payload + separator + separator);
  REQUIRE(tree);
  std::size_t total = 0;
  for (auto& f : tree->files)
    total += f.payload.size();
  CHECK(total == cap);
  auto in                                          = v1();
  in.files                                         = std::move(tree->files);
  at(in.root, "versions")->fields["stdlib_digest"] = str(hex(tree->digest));
  auto r                                           = in.check();
  REQUIRE(r);
  REQUIRE(r->decoded);
  in.files.push_back({"extra", "x", 420});
  refused(in, code::resource_limit);
}
TEST_CASE("recorded future fields and malformed manifest markers never become unsupported semantics",
          "[recorded_interpretation]") {
  for (auto in : {v1(), v2()}) {
    mut(in.root)->fields["version"] = num(99);
    mut(in.root)->fields["future"]  = num(1);
    refused(in, code::malformed);
    mut(in.root)->fields.erase("future");
    at(in.root, "versions")->fields["future"] = num(1);
    refused(in, code::malformed);
  }
  for (auto name : {"version", "scheme", "registration", "embedded", "assembly", "profile", "modules", "catalog"}) {
    auto in = v2();
    manifest(in, [&](auto m) {
      mut(m)->fields["version"] = num(99);
      mut(m)->fields[name]      = str("wrong");
    });
    if (std::string_view(name) == "scheme" || std::string_view(name) == "registration" || std::string_view(name) == "embedded" ||
        std::string_view(name) == "assembly" || std::string_view(name) == "profile")
      refused(in, code::unsupported_manifest);
    else
      refused(in, code::malformed);
  }
}
TEST_CASE("recorded direct old decoder keeps embedded versions allocation refusal typed", "[recorded_interpretation]") {
  auto in    = v1();
  auto bytes = enc(in.root), versions = enc(at(in.root, "versions"));
  allocation_probe::calls     = 0;
  allocation_probe::countdown = -1;
  allocation_probe::active    = true;
  auto initial                = decode_interpretation(bytes, versions);
  allocation_probe::active    = false;
  REQUIRE(initial);
  auto calls = allocation_probe::calls;
  REQUIRE(calls > 0);
  for (std::size_t i = 0; i < calls; ++i) {
    allocation_probe::countdown = static_cast<std::int64_t>(i);
    allocation_probe::active    = true;
    try {
      auto r                   = decode_interpretation(bytes, versions);
      allocation_probe::active = false;
      INFO("direct old decoder allocation=" << i << "/" << calls);
      REQUIRE_FALSE(r);
      CHECK(r.error().code == interpretation_error_code::resource_limit);
    } catch (const std::bad_alloc&) {
      // Unchanged lower API has raw allocation sites; the enclosing new public helper catches these.
      allocation_probe::active = false;
    }
  }
}
