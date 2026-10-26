// Pure recorded interpretation-v2: independent bytes, raw-tree binding and closed failure precedence.
#include <catch2/catch_test_macros.hpp>
import std;
import planar.engine.history.interpretation_v2_codec;
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
using code = interpretation_v2_error_code;
std::string read(std::string_view name) {
  std::ifstream f(std::string(PLANAR_V2_VECTORS) + "/" + std::string(name), std::ios::binary);
  REQUIRE(f.good());
  return {std::istreambuf_iterator<char>(f), {}};
}
std::vector<tree_file> files(std::string_view name = "files.tsv") {
  std::istringstream     in(read(name));
  std::string            line;
  std::vector<tree_file> out;
  while (std::getline(in, line)) {
    if (line.starts_with('#'))
      continue;
    auto        a = line.find('\t'), b = line.find('\t', a + 1);
    std::string bytes;
    for (std::size_t i = b + 1; i < line.size(); i += 2)
      bytes += static_cast<char>(std::stoul(line.substr(i, 2), nullptr, 16));
    out.push_back({line.substr(0, a), bytes, static_cast<std::uint32_t>(std::stoul(line.substr(a + 1, b - a - 1)))});
  }
  return out;
}
auto mut(value_ptr v) {
  return std::const_pointer_cast<value>(v);
}
auto at(value_ptr v, std::string key) {
  return mut(v->fields.at(key));
}
auto item(value_ptr v, std::size_t i = 0) {
  return mut(v->items.at(i));
}
value_ptr str(std::string text) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = std::move(text);
  return v;
}
value_ptr num(std::int64_t n) {
  auto v     = std::make_shared<value>();
  v->kind    = value_kind::integer;
  v->integer = n;
  return v;
}
value_ptr list(std::vector<value_ptr> items = {}) {
  auto v   = std::make_shared<value>();
  v->kind  = value_kind::list;
  v->items = std::move(items);
  return v;
}
value_ptr obj() {
  auto v  = std::make_shared<value>();
  v->kind = value_kind::record;
  return v;
}
std::string enc(value_ptr v) {
  auto e = canonical_encode(v, {256, 4000000, 16 * 1048576});
  REQUIRE(e);
  return *e;
}
value_ptr dec(std::string_view bytes) {
  auto v = canonical_decode(bytes, {256, 4000000, 16 * 1048576});
  REQUIRE(v);
  return *v;
}
value_ptr fixture() {
  return dec(read("interpretation.bin"));
}
void refused(value_ptr root, code expected, std::vector<tree_file> tree = files()) {
  auto r = decode_interpretation_v2(enc(root), enc(at(root, "versions")), tree);
  REQUIRE_FALSE(r);
  CHECK(r.error().code == expected);
}
void alter_file(std::vector<tree_file>& tree, std::string_view name, const std::function<void(tree_file&)>& change) {
  auto i = std::ranges::find(tree, name, &tree_file::path);
  REQUIRE(i != tree.end());
  change(*i);
}
void alter_manifest(std::vector<tree_file>& tree, const std::function<void(value_ptr)>& change) {
  alter_file(tree, "__planar__/effective.bin", [&](auto& f) {
    auto m = dec(f.payload);
    change(m);
    f.payload = enc(m);
  });
}
} // namespace
TEST_CASE("v2 independent mixed and empty profile canonical bytes bind and roundtrip", "[interpretation_v2]") {
  for (auto prefix : {"", "empty-"}) {
    auto tree  = files(std::string(prefix) + "files.tsv");
    auto bytes = read(std::string(prefix) + "interpretation.bin"), vbytes = read(std::string(prefix) + "versions.bin");
    auto r = decode_interpretation_v2(bytes, vbytes, tree);
    REQUIRE(r);
    CHECK(r->encoded == bytes);
    CHECK(r->versions_encoded == vbytes);
    auto e = encode_interpretation_v2(at(r->interpretation, "bundle"), at(r->interpretation, "semantic"), tree);
    REQUIRE(e);
    CHECK(e->encoded == bytes);
    CHECK(e->versions_encoded == vbytes);
  }
  auto root = fixture();
  auto h    = at(item(at(at(root, "semantic"), "jobs")), "selected_shape")->fields.at("definition")->string;
  CHECK(h + "\n" == read("definition.txt"));
  CHECK(h.size() == 41);
  CHECK(at(at(root, "versions"), "stdlib_digest")->string + "\n" == read("digest.txt"));
}
TEST_CASE("v2 results sever all B S raw tree aliases", "[interpretation_v2]") {
  auto root = fixture();
  auto b = at(root, "bundle"), s = at(root, "semantic");
  auto tree = files();
  auto r    = encode_interpretation_v2(b, s, tree);
  REQUIRE(r);
  auto exact                                                        = r->encoded;
  at(at(b, "workspace"), "id")->string                              = "changed";
  at(at(item(at(s, "jobs")), "selected_shape"), "guidance")->string = "changed";
  tree.clear();
  CHECK(enc(r->interpretation) == exact);
  CHECK(r->encoded == read("interpretation.bin"));
}
TEST_CASE("v2 fixed keys kinds B S job ordering and selected syntax are malformed", "[interpretation_v2]") {
  for (auto key : {"version", "bundle", "semantic", "versions", "extensions"}) {
    auto r = fixture();
    mut(r)->fields.erase(key);
    auto d = decode_interpretation_v2(enc(r), read("versions.bin"), files());
    REQUIRE_FALSE(d);
    CHECK(d.error().code == code::malformed);
  }
  for (auto key : {"workspace", "spec", "registry", "jobs", "workspace_file", "spec_file", "registry_file"}) {
    auto r                       = fixture();
    at(r, "bundle")->fields[key] = num(1);
    refused(r, code::malformed);
  }
  for (auto key : {"dependencies", "jobs"}) {
    auto r                         = fixture();
    at(r, "semantic")->fields[key] = obj();
    refused(r, code::malformed);
  }
  for (auto key : {"id", "revision", "definition", "guidance", "obligations"}) {
    auto r = fixture();
    at(item(at(at(r, "semantic"), "jobs")), "selected_shape")->fields.erase(key);
    refused(r, code::malformed);
  }
  for (auto key : {"job", "id", "revision", "definition"}) {
    auto r = fixture();
    item(at(at(r, "extensions"), "selected_shapes"))->fields.erase(key);
    refused(r, code::malformed);
  }
  {
    auto r    = fixture();
    auto jobs = at(at(r, "bundle"), "jobs");
    std::swap(jobs->items[0], jobs->items[1]);
    refused(r, code::malformed);
  }
  {
    auto r                                                  = fixture();
    at(item(at(at(r, "semantic"), "jobs")), "file")->string = "other.lua";
    refused(r, code::malformed);
  }
  {
    auto r = fixture();
    auto e = at(at(r, "extensions"), "selected_shapes");
    e->items.push_back(e->items[0]);
    refused(r, code::malformed);
  }
}
TEST_CASE("v2 exact supported constants and embedded versions equality precede tree", "[interpretation_v2]") {
  for (auto key : {"declarations", "normalization", "semantics", "selection", "normalizer", "canonical", "hash", "interpretation",
                   "projection", "registration", "provenance"}) {
    auto r                             = fixture();
    at(at(r, "versions"), key)->string = "future";
    refused(r, code::unsupported_version, {});
  }
  {
    auto r                    = fixture();
    at(r, "version")->integer = 3;
    refused(r, code::unsupported_version, {});
  }
  {
    auto r                                          = fixture();
    at(at(r, "extensions"), "registration")->string = "future";
    refused(r, code::unsupported_version, {});
  }
  {
    auto r                    = fixture();
    at(r, "version")->integer = 3;
    auto s                    = at(item(at(at(r, "semantic"), "jobs")), "selected_shape");
    s->fields.erase("guidance");
    refused(r, code::malformed, {});
  }
  {
    auto r                                          = fixture();
    at(at(r, "versions"), "interpretation")->string = "future";
    auto d                                          = decode_interpretation_v2(enc(r), read("versions.bin"), {});
    REQUIRE_FALSE(d);
    CHECK(d.error().code == code::malformed);
  }
  {
    auto r                                         = fixture();
    at(at(r, "versions"), "stdlib_digest")->string = "XX";
    refused(r, code::malformed, {});
  }
}
TEST_CASE("v2 selected definition spelling refuses untagged uppercase malformed UTF8 and NUL", "[interpretation_v2]") {
  for (auto value : {std::string(32, 'a'), "xxh3_128:" + std::string(32, 'A'), "xxh3_128:" + std::string(31, 'a'),
                     std::string("\xff", 1), std::string("\0", 1)}) {
    auto r                                                                              = fixture();
    at(at(item(at(at(r, "semantic"), "jobs")), "selected_shape"), "definition")->string = value;
    refused(r, code::malformed);
  }
  for (auto value : {std::string("\xff", 1), std::string("\0", 1)}) {
    auto r                                                                            = fixture();
    at(at(item(at(at(r, "semantic"), "jobs")), "selected_shape"), "guidance")->string = value;
    refused(r, code::malformed);
  }
}
TEST_CASE("v2 normalized obligations validate exact shape source sorted IDs work roles phase quality", "[interpretation_v2]") {
  for (auto [key, value] : std::vector<std::pair<std::string, std::string>>{{"id", "edge"},
                                                                            {"id", "shape:wrong:edge"},
                                                                            {"id", "shape:custom:Bad"},
                                                                            {"source", "declaration"},
                                                                            {"producer", "missing"},
                                                                            {"consumer", "alpha"},
                                                                            {"producer_role", "other"},
                                                                            {"consumer_role", "missing"},
                                                                            {"phase", "accepted"},
                                                                            {"quality", "available"}}) {
    auto r             = fixture();
    auto o             = item(at(at(item(at(at(r, "semantic"), "jobs")), "selected_shape"), "obligations"));
    at(o, key)->string = value;
    refused(r, code::malformed);
  }
  {
    auto r = fixture();
    auto o = at(at(item(at(at(r, "semantic"), "jobs")), "selected_shape"), "obligations");
    o->items.push_back(o->items[0]);
    refused(r, code::malformed);
  }
  {
    auto r                                                                                              = fixture();
    item(at(at(item(at(at(r, "semantic"), "jobs")), "selected_shape"), "obligations"))->fields["extra"] = str("extra");
    refused(r, code::malformed);
  }
}
TEST_CASE("v2 custom shipped classification and exact E membership cannot be substituted", "[interpretation_v2]") {
  for (auto field : {"id", "revision", "definition"}) {
    auto r = fixture();
    at(item(at(at(r, "extensions"), "selected_shapes")), field)->string =
        std::string(field) == "definition" ? "xxh3_128:" + std::string(32, '0') : "other";
    refused(r, code::invalid_correspondence);
  }
  {
    auto r = fixture();
    at(at(r, "extensions"), "selected_shapes")->items.clear();
    refused(r, code::invalid_correspondence);
  }
  {
    auto r                                                                        = fixture();
    at(item(at(at(r, "semantic"), "jobs")), "selected_shape")->fields["revision"] = str("other");
    refused(r, code::invalid_correspondence);
  }
  {
    auto r = fixture();
    item(at(at(r, "semantic"), "jobs"))->fields.erase("selected_shape");
    refused(r, code::invalid_correspondence);
  }
  {
    auto r                                                           = fixture();
    item(at(at(r, "semantic"), "jobs"), 1)->fields["selected_shape"] = at(item(at(at(r, "semantic"), "jobs")), "selected_shape");
    refused(r, code::invalid_correspondence);
  }
  {
    auto r                                                              = fixture();
    at(at(item(at(at(r, "bundle"), "jobs")), "value"), "shape")->string = "unknown";
    refused(r, code::invalid_correspondence);
  }
  {
    // A shipped job (no selected shape) renamed to an unknown shape has no catalog entry and is no reserved shape.
    auto r = fixture();
    REQUIRE_FALSE(item(at(at(r, "semantic"), "jobs"), 1)->fields.contains("selected_shape"));
    at(at(item(at(at(r, "bundle"), "jobs"), 1), "value"), "shape")->string = "unknown";
    refused(r, code::invalid_correspondence);
  }
}
TEST_CASE("v2 asserted catalog requires actual manifest owner ordinals before equality", "[interpretation_v2]") {
  {
    auto r                                                                                      = fixture();
    at(at(item(at(at(at(r, "extensions"), "catalog"), "shapes")), "owner"), "ordinal")->integer = 2;
    refused(r, code::malformed);
  }
  {
    auto r                                                                                           = fixture();
    at(at(item(at(at(at(r, "extensions"), "catalog"), "shapes")), "descriptor"), "revision")->string = "different";
    refused(r, code::invalid_correspondence);
  }
  {
    auto r = fixture();
    at(at(at(r, "extensions"), "catalog"), "shapes")->items.clear();
    refused(r, code::invalid_correspondence);
  }
  {
    auto r = fixture();
    at(at(r, "extensions"), "catalog")->fields.erase("shapes");
    refused(r, code::malformed);
  }
  {
    auto r = fixture();
    at(at(r, "extensions"), "catalog")->fields.erase("shapes");
    refused(r, code::invalid_tree, {});
  }
}
TEST_CASE("v2 raw membership mode manifest order owner module and embedded tampering refuse", "[interpretation_v2]") {
  for (auto path : {"external/ext/z.lua", "profile/input.json", "shipped/planar/z.lua", "__planar__/embedded.lua",
                    "__planar__/embedded.bin", "__planar__/effective.bin"}) {
    auto tree = files();
    alter_file(tree, path, [](auto& f) { f.payload += "tampered"; });
    auto r = decode_interpretation_v2(read("interpretation.bin"), read("versions.bin"), tree);
    REQUIRE_FALSE(r);
    CHECK((r.error().code == code::invalid_tree || r.error().code == code::invalid_correspondence));
  }
  {
    auto t = files();
    t.pop_back();
    refused(fixture(), code::invalid_tree, t);
  }
  {
    auto t = files();
    t.push_back({"extra.lua", "extra", 420});
    refused(fixture(), code::invalid_tree, t);
  }
  {
    auto t = files();
    t.push_back(t[0]);
    refused(fixture(), code::invalid_tree, t);
  }
  {
    auto t = files();
    alter_file(t, "external/ext/z.lua", [](auto& f) { f.mode = 420; });
    refused(fixture(), code::invalid_tree, t);
  }
  {
    auto t = files();
    alter_manifest(t, [](auto m) {
      auto a = at(m, "modules");
      std::swap(a->items[0], a->items[1]);
    });
    refused(fixture(), code::invalid_tree, t);
  }
  {
    auto t = files();
    alter_manifest(t, [](auto m) { at(at(item(at(at(m, "catalog"), "shapes")), "owner"), "ordinal")->integer = 2; });
    refused(fixture(), code::invalid_tree, t);
  }
}
TEST_CASE("v2 forged selfconsistent catalog digest and definition still require raw tree binding", "[interpretation_v2]") {
  auto r                                                                              = fixture();
  at(at(r, "versions"), "stdlib_digest")->string                                      = std::string(32, '0');
  at(at(item(at(at(r, "semantic"), "jobs")), "selected_shape"), "definition")->string = "xxh3_128:" + std::string(32, '0');
  at(item(at(at(r, "extensions"), "selected_shapes")), "definition")->string          = "xxh3_128:" + std::string(32, '0');
  refused(r, code::invalid_correspondence);
  auto t = files();
  alter_file(t, "external/ext/z.lua", [](auto& f) { f.payload += "-- comment"; });
  auto b = at(r, "bundle"), s = at(r, "semantic");
  auto e = encode_interpretation_v2(b, s, t);
  REQUIRE_FALSE(e);
  CHECK(e.error().code == code::invalid_correspondence);
}
TEST_CASE("v2 preallocation byte tree guidance obligation and cyclic graph limits are typed", "[interpretation_v2]") {
  std::string over(16 * 1048576 + 1, 'x');
  for (bool interpretation : {true, false}) {
    auto r = decode_interpretation_v2(interpretation ? over : "bad", interpretation ? "bad" : over, {});
    REQUIRE_FALSE(r);
    CHECK(r.error().code == code::resource_limit);
  }
  {
    auto r                                                                            = fixture();
    at(at(item(at(at(r, "semantic"), "jobs")), "selected_shape"), "guidance")->string = std::string(65537, 'x');
    refused(r, code::resource_limit);
  }
  {
    auto r = fixture();
    auto o = at(at(item(at(at(r, "semantic"), "jobs")), "selected_shape"), "obligations");
    o->items.resize(10001, o->items[0]);
    refused(r, code::resource_limit);
  }
  {
    auto t = files();
    alter_file(t, "external/ext/z.lua", [](auto& f) { f.payload = std::string(1048577, 'x'); });
    refused(fixture(), code::resource_limit, t);
  }
  {
    std::vector<tree_file> t(8193);
    refused(fixture(), code::resource_limit, t);
  }
  {
    auto r     = fixture();
    auto cycle = mut(list());
    cycle->items.push_back(cycle);
    auto e = encode_interpretation_v2(cycle, at(r, "semantic"), files());
    REQUIRE_FALSE(e);
    CHECK((e.error().code == code::malformed || e.error().code == code::resource_limit));
    cycle->items.clear();
  }
  {
    auto      r    = fixture();
    value_ptr deep = str("leaf");
    for (int i = 0; i < 257; ++i)
      deep = list({deep});
    auto e = encode_interpretation_v2(deep, at(r, "semantic"), files());
    REQUIRE_FALSE(e);
    CHECK(e.error().code == code::resource_limit);
  }
}
TEST_CASE("v2 unrelated module bytes and reordered ordinals change tree but preserve selected shape H", "[interpretation_v2]") {
  auto original = fixture();
  auto b = at(original, "bundle"), s = at(original, "semantic");
  for (bool reordered : {false, true}) {
    auto t = files();
    if (reordered) {
      alter_file(t, "profile/input.json",
                 [](auto& f) { f.payload = "{\"version\":1,\"modules\":[\"ext/a.lua\",\"ext/z.lua\"]}"; });
      alter_manifest(t, [](auto m) {
        auto modules = at(m, "modules");
        std::swap(modules->items[0], modules->items[1]);
        at(at(item(at(at(m, "catalog"), "shapes")), "owner"), "ordinal")->integer = 2;
      });
    } else
      alter_file(t, "external/ext/a.lua", [](auto& f) { f.payload += "-- unrelated comment\n"; });
    auto result = encode_interpretation_v2(b, s, t);
    REQUIRE(result);
    CHECK(result->encoded != read("interpretation.bin"));
    CHECK(at(result->versions, "stdlib_digest")->string != at(at(original, "versions"), "stdlib_digest")->string);
    CHECK(at(at(item(at(at(result->interpretation, "semantic"), "jobs")), "selected_shape"), "definition")->string + "\n" ==
          read("definition.txt"));
    auto decoded = decode_interpretation_v2(result->encoded, result->versions_encoded, t);
    REQUIRE(decoded);
  }
}
TEST_CASE("v2 valid normalized guidance boundary and preallocation canonical counts are explicit", "[interpretation_v2]") {
  {
    auto r                                                                            = fixture();
    at(at(item(at(at(r, "semantic"), "jobs")), "selected_shape"), "guidance")->string = std::string(65536, 'x');
    auto e = encode_interpretation_v2(at(r, "bundle"), at(r, "semantic"), files());
    REQUIRE(e);
  }
  {
    std::string exact(16 * 1048576, 'x');
    auto        d = decode_interpretation_v2(exact, "bad", {});
    REQUIRE_FALSE(d);
    CHECK(d.error().code == code::malformed);
  }
  // Canonical list count above four million refuses before allocating its members.
  {
    std::string   encoded(1, '\x04');
    std::uint64_t count = 4000001;
    for (int shift = 56; shift >= 0; shift -= 8)
      encoded += static_cast<char>((count >> shift) & 255);
    auto d = decode_interpretation_v2(encoded, read("versions.bin"), {});
    REQUIRE_FALSE(d);
    CHECK(d.error().code == code::resource_limit);
  }
}
TEST_CASE("v2 every native allocation failure remains a redacted typed resource refusal", "[interpretation_v2][allocation]") {
  auto root = fixture();
  auto b = at(root, "bundle"), s = at(root, "semantic");
  auto t     = files();
  auto bytes = read("interpretation.bin"), versions = read("versions.bin");
  for (bool encoder : {false, true}) {
    allocation_probe::calls     = 0;
    allocation_probe::countdown = -1;
    allocation_probe::active    = true;
    auto baseline               = encoder ? encode_interpretation_v2(b, s, t) : decode_interpretation_v2(bytes, versions, t);
    allocation_probe::active    = false;
    auto count                  = allocation_probe::calls;
    REQUIRE(baseline);
    REQUIRE(count > 100);
    for (std::size_t n = 0; n < count; ++n) {
      allocation_probe::calls     = 0;
      allocation_probe::countdown = static_cast<std::int64_t>(n);
      allocation_probe::active    = true;
      std::optional<std::expected<recorded_interpretation_v2, interpretation_v2_error>> result;
      bool                                                                              escaped = false;
      try {
        result = encoder ? encode_interpretation_v2(b, s, t) : decode_interpretation_v2(bytes, versions, t);
      } catch (...) {
        escaped = true;
      }
      allocation_probe::active = false;
      INFO("encoder=" << encoder << " allocation=" << n << " of " << count);
      REQUIRE_FALSE(escaped);
      REQUIRE(result);
      REQUIRE_FALSE(*result);
      CHECK(result->error().code == code::resource_limit);
    }
  }
}
TEST_CASE("v2 prerequisite lower provenance adapters preserve canonical resource failures", "[interpretation_v2][allocation]") {
  auto                        tree = files();
  std::vector<tree_file>      embedded;
  std::vector<tree_file_view> modules;
  std::string                 profile, catalog;
  for (auto& f : tree) {
    if (f.path.starts_with("shipped/"))
      embedded.push_back({f.path.substr(8), f.payload, f.mode});
    else if (f.path == "__planar__/embedded.lua")
      embedded.push_back({"__planar__/assembly.lua", f.payload, f.mode});
    else if (f.path == "__planar__/embedded.bin")
      embedded.push_back({"__planar__/assembly.canonical", f.payload, f.mode});
    else if (f.path == "profile/input.json")
      profile = f.payload;
    else if (f.path == "__planar__/effective.bin")
      catalog = enc(at(dec(f.payload), "catalog"));
  }
  for (auto path : {"external/ext/z.lua", "external/ext/a.lua"}) {
    auto i = std::ranges::find(tree, path, &tree_file::path);
    REQUIRE(i != tree.end());
    modules.push_back({std::string_view(i->path).substr(9), i->payload, i->mode});
  }
  effective_input input{embedded, profile, 493, modules, catalog};
  for (int operation = 0; operation < 3; ++operation) {
    auto call = [&]() -> std::optional<bool> {
      if (operation == 0) {
        auto r = verify_stdlib(embedded);
        if (r)
          return {};
        return r.error().code == provenance_error_code::resource_limit;
      }
      auto r = operation == 1 ? prepare_effective_stdlib(input) : verify_effective_stdlib(tree);
      if (r)
        return {};
      return r.error().code == effective_error_code::resource_limit;
    };
    allocation_probe::calls     = 0;
    allocation_probe::countdown = -1;
    allocation_probe::active    = true;
    auto good                   = call();
    allocation_probe::active    = false;
    auto count                  = allocation_probe::calls;
    REQUIRE_FALSE(good);
    REQUIRE(count > 10);
    for (std::size_t n = 0; n < count; ++n) {
      allocation_probe::countdown = static_cast<std::int64_t>(n);
      allocation_probe::active    = true;
      std::optional<bool> result;
      bool                thrown = false;
      try {
        result = call();
      } catch (const std::bad_alloc&) {
        thrown = true;
      }
      allocation_probe::active = false;
      INFO("lower operation=" << operation << " allocation=" << n);
      if (!thrown) {
        REQUIRE(result);
        CHECK(*result);
      }
    }
  }
}
TEST_CASE("v2 per job normalized obligation maximum accepts exactly ten thousand sorted IDs", "[interpretation_v2]") {
  auto root        = fixture();
  auto selected    = at(item(at(at(root, "semantic"), "jobs")), "selected_shape");
  auto obligations = at(selected, "obligations");
  auto prototype   = obligations->items[0];
  obligations->items.clear();
  for (int i = 0; i < 10000; ++i) {
    auto o          = std::make_shared<value>(*prototype);
    o->fields["id"] = str(std::format("shape:custom:e{:05}", i));
    obligations->items.push_back(o);
  }
  auto encoded = encode_interpretation_v2(at(root, "bundle"), at(root, "semantic"), files());
  REQUIRE(encoded);
  CHECK(at(at(item(at(at(encoded->interpretation, "semantic"), "jobs")), "selected_shape"), "obligations")->items.size() ==
        10000);
}
