// Effective provenance composition, independent byte vectors and structural historical refusals.
#include <catch2/catch_test_macros.hpp>
import std;
import planar.engine.history.effective_provenance;
import planar.engine.history.effective_catalog;
import planar.engine.canonical;
using namespace planar::engine;
using namespace planar::engine::history;
namespace {
constexpr std::size_t mib = 1048576;
std::string           read(std::string_view name) {
  std::ifstream f(std::string(PLANAR_EFFECTIVE_VECTORS) + "/" + std::string(name), std::ios::binary);
  REQUIRE(f.good());
  return {std::istreambuf_iterator<char>(f), {}};
}
std::string unhex(std::string_view h) {
  std::string out;
  for (std::size_t i = 0; i < h.size(); i += 2)
    out += static_cast<char>(std::stoul(std::string(h.substr(i, 2)), nullptr, 16));
  return out;
}
std::string hex(record_digest d) {
  std::string s;
  for (auto b : d)
    s += std::format("{:02x}", b);
  return s;
}
std::vector<tree_file> fixture() {
  std::istringstream     input(read("files.tsv"));
  std::string            line;
  std::vector<tree_file> files;
  while (std::getline(input, line)) {
    if (line.starts_with('#'))
      continue;
    auto a = line.find('\t'), b = line.find('\t', a + 1);
    files.push_back(
        {line.substr(0, a), unhex(line.substr(b + 1)), static_cast<std::uint32_t>(std::stoul(line.substr(a + 1, b - a - 1)))});
  }
  return files;
}
tree_file& file(std::vector<tree_file>& files, std::string_view path) {
  auto i = std::ranges::find(files, path, &tree_file::path);
  REQUIRE(i != files.end());
  return *i;
}
struct inputs {
  std::vector<tree_file>      embedded;
  std::string                 profile, catalog = read("catalog.bin");
  std::uint32_t               mode = 493;
  std::vector<tree_file>      external;
  std::vector<tree_file_view> modules;
  inputs() {
    for (auto& f : fixture()) {
      if (f.path.starts_with("shipped/"))
        embedded.push_back({f.path.substr(8), f.payload, f.mode});
      else if (f.path == "__planar__/embedded.lua")
        embedded.push_back({"__planar__/assembly.lua", f.payload, f.mode});
      else if (f.path == "__planar__/embedded.bin")
        embedded.push_back({"__planar__/assembly.canonical", f.payload, f.mode});
      else if (f.path == "profile/input.json")
        profile = f.payload;
    }
    auto f   = fixture();
    external = {file(f, "external/ext/z.lua"), file(f, "external/ext/a.lua")};
    for (auto& e : external)
      e.path = e.path.substr(9);
  }
  effective_input view() {
    modules.clear();
    for (auto& e : external)
      modules.push_back({e.path, e.payload, e.mode});
    return {embedded, profile, mode, modules, catalog};
  }
};
auto mut(value_ptr v) {
  return std::const_pointer_cast<value>(v);
}
auto at(value_ptr v, std::string key) {
  return mut(v->fields.at(key));
}
value_ptr decode(std::string_view b) {
  auto d = canonical_decode(b);
  REQUIRE(d);
  return *d;
}
void encode(std::string& out, value_ptr v) {
  auto e = canonical_encode(v);
  REQUIRE(e);
  out = *e;
}
void alter_manifest(std::vector<tree_file>& files, const std::function<void(value_ptr)>& fn) {
  auto& b = file(files, "__planar__/effective.bin").payload;
  auto  v = decode(b);
  fn(v);
  encode(b, v);
}
void rejected(std::vector<tree_file> files) {
  REQUIRE_FALSE(verify_effective_stdlib(files));
}
void reassemble(inputs& in, std::vector<std::pair<std::string, std::string>> sources) {
  std::vector<original_file_view> views;
  std::string                     assembly;
  for (auto& [p, b] : sources) {
    views.push_back({p, b});
    assembly += b + "\n";
  }
  auto p = prepare_stdlib(views, "\n", assembly);
  REQUIRE(p);
  in.embedded = std::move(p->files);
}
} // namespace
TEST_CASE("replacement.effective provenance independent exact bytes and digest") {
  inputs in;
  auto   p = prepare_effective_stdlib(in.view());
  REQUIRE(p);
  auto expected = fixture();
  REQUIRE(p->files.size() == expected.size());
  for (auto& f : expected) {
    auto& actual = file(p->files, f.path);
    REQUIRE(actual.payload == f.payload);
    REQUIRE(actual.mode == f.mode);
  }
  REQUIRE(file(p->files, "__planar__/effective.bin").payload == read("manifest.bin"));
  REQUIRE(hex(p->digest) + "\n" == read("digest.txt"));
  std::ranges::reverse(expected);
  auto v = verify_effective_stdlib(expected);
  REQUIRE(v);
  REQUIRE(v->digest == p->digest);
  for (auto& f : in.embedded)
    f.payload = "destroyed";
  in.profile = "destroyed";
  in.catalog = "destroyed";
  in.external.clear();
  expected.clear();
  REQUIRE(file(p->files, "profile/input.json").payload.starts_with("{ "));
  REQUIRE(v->digest == p->digest);
}
TEST_CASE("replacement.effective provenance raw whitespace modes and source bytes affect identity") {
  inputs in;
  auto   p = prepare_effective_stdlib(in.view());
  REQUIRE(p);
  for (int variant = 0; variant < 5; ++variant) {
    inputs changed;
    if (variant == 0)
      changed.profile += " \n";
    if (variant == 1)
      changed.mode = 420;
    if (variant == 2)
      changed.external[0].mode = 420;
    if (variant == 3)
      changed.external[0].payload += "changed";
    if (variant == 4)
      reassemble(changed, {{"planar/z.lua", "historical bytes"}, {"planar/commands.lua", "changed original"}});
    auto q = prepare_effective_stdlib(changed.view());
    REQUIRE(q);
    REQUIRE(q->digest != p->digest);
    auto v = verify_effective_stdlib(q->files);
    REQUIRE(v);
    REQUIRE(v->digest == q->digest);
  }
  auto changed = fixture();
  file(changed, "external/ext/a.lua").payload += "consistent changed history";
  auto v = verify_effective_stdlib(changed);
  REQUIRE(v);
  REQUIRE(v->digest != p->digest);
}
TEST_CASE("replacement.effective provenance full membership and modes") {
  auto base = fixture();
  for (std::size_t i = 0; i < base.size(); ++i) {
    auto f = base;
    f.erase(f.begin() + i);
    rejected(f);
    f = base;
    f.push_back(f[i]);
    rejected(f);
    f         = base;
    f[i].mode = 0;
    rejected(f);
    if (base[i].mode == 420 && !base[i].path.starts_with("external/")) {
      f         = base;
      f[i].mode = 493;
      rejected(f);
    }
  }
  for (auto path : {"extra.lua", "external/ext/extra.lua", "shipped/extra.lua", "__planar__/extra", "profile/other"}) {
    auto f = base;
    f.push_back({path, "extra", 420});
    rejected(f);
  }
  auto f                             = base;
  file(f, "external/ext/z.lua").path = "external/EXT/z.lua";
  rejected(f);
  f = base;
  f.push_back({"external/ext", "ancestor", 420});
  rejected(f);
}
TEST_CASE("replacement.effective provenance closed exact manifest and framing") {
  for (auto key : {"version", "scheme", "registration", "embedded", "assembly", "profile", "modules", "catalog"}) {
    auto f = fixture();
    alter_manifest(f, [&](auto v) { mut(v)->fields.erase(key); });
    rejected(f);
    f = fixture();
    alter_manifest(f, [&](auto v) {
      auto bad            = std::make_shared<value>();
      bad->kind           = value_kind::boolean;
      mut(v)->fields[key] = bad;
    });
    rejected(f);
  }
  for (auto key : {"scheme", "registration", "embedded", "assembly", "profile"}) {
    auto f = fixture();
    alter_manifest(f, [&](auto v) { at(v, key)->string += "x"; });
    rejected(f);
  }
  auto f = fixture();
  alter_manifest(f, [](auto v) { at(v, "version")->integer = 2; });
  rejected(f);
  f = fixture();
  alter_manifest(f, [](auto v) { mut(v)->fields["extra"] = v->fields.at("version"); });
  rejected(f);
  for (int kind = 0; kind < 3; ++kind) {
    f       = fixture();
    auto& b = file(f, "__planar__/effective.bin").payload;
    if (kind == 0)
      b.clear();
    if (kind == 1)
      b.pop_back();
    if (kind == 2)
      b += 'x';
    rejected(f);
  }
}
TEST_CASE("replacement.effective provenance profile module order and correspondence") {
  inputs in;
  std::swap(in.external[0], in.external[1]);
  REQUIRE_FALSE(prepare_effective_stdlib(in.view()));
  in         = inputs{};
  in.profile = R"({"version":1,"modules":["ext/a.lua","ext/z.lua"]})";
  std::swap(in.external[0], in.external[1]);
  auto p = prepare_effective_stdlib(in.view());
  REQUIRE(p);
  auto baseline = prepare_effective_stdlib(inputs{}.view());
  REQUIRE(baseline);
  REQUIRE(p->digest != baseline->digest);
  for (auto bad : {R"({"version":2,"modules":[]})", R"({"version":1,"modules":["../a.lua"]})", R"({"version":1,"modules":[]})",
                   "{}", "not JSON"}) {
    auto f                                = fixture();
    file(f, "profile/input.json").payload = bad;
    rejected(f);
  }
  for (int kind = 0; kind < 6; ++kind) {
    auto f = fixture();
    alter_manifest(f, [&](auto v) {
      auto list = at(v, "modules");
      if (kind == 0)
        std::swap(list->items[0], list->items[1]);
      if (kind == 1)
        list->items.push_back(list->items[0]);
      if (kind == 2)
        at(list->items[0], "path")->string = "external/ext/absent.lua";
      if (kind == 3)
        at(list->items[0], "mode")->integer = 420;
      if (kind == 4)
        at(list->items[0], "path")->string = "shipped/planar/z.lua";
      if (kind == 5)
        mut(list->items[0])->fields.erase("mode");
    });
    rejected(f);
  }
}
TEST_CASE("replacement.effective provenance embedded source order assembly and catalog owners") {
  auto f = fixture();
  file(f, "shipped/planar/z.lua").payload += "mismatch";
  rejected(f);
  f = fixture();
  file(f, "__planar__/embedded.lua").payload += "mismatch";
  rejected(f);
  f         = fixture();
  auto& b   = file(f, "__planar__/embedded.bin").payload;
  auto  old = decode(b);
  std::swap(at(old, "files")->items[0], at(old, "files")->items[1]);
  encode(b, old);
  rejected(f);
  for (auto key : {"ordinal", "module", "tier"}) {
    f = fixture();
    alter_manifest(f, [&](auto v) {
      auto owner = at(at(v, "catalog")->fields.at("commands")->items[0], "owner");
      if (std::string_view(key) == "ordinal")
        at(owner, key)->integer = 1;
      else
        at(owner, key)->string = "wrong";
    });
    rejected(f);
  }
  f = fixture();
  alter_manifest(f, [](auto v) {
    at(at(at(v, "catalog")->fields.at("commands")->items[0], "descriptor"), "handler_id")->string = "changed.signature";
  });
  rejected(f);
  inputs in;
  auto   bad                  = decode(in.catalog);
  at(bad, "version")->integer = 1;
  encode(in.catalog, bad);
  REQUIRE_FALSE(prepare_effective_stdlib(in.view()));
  in                                  = inputs{};
  auto& oldbytes                      = file(in.embedded, "__planar__/assembly.canonical").payload;
  auto  unsupported                   = decode(oldbytes);
  at(unsupported, "version")->integer = 2;
  encode(oldbytes, unsupported);
  auto r = prepare_effective_stdlib(in.view());
  REQUIRE_FALSE(r);
  REQUIRE(r.error().code == effective_error_code::invalid_provenance);
}
TEST_CASE("replacement.effective provenance external owners bind profile ordinals") {
  inputs in;
  auto   c         = decode(in.catalog);
  auto   owner     = std::make_shared<value>();
  owner->kind      = value_kind::record;
  auto descriptor  = std::make_shared<value>();
  descriptor->kind = value_kind::record;
  auto text        = [](std::string s) {
    auto v    = std::make_shared<value>();
    v->kind   = value_kind::string;
    v->string = std::move(s);
    return v;
  };
  auto ordinal           = std::make_shared<value>();
  ordinal->kind          = value_kind::integer;
  ordinal->integer       = 1;
  owner->fields          = {{"tier", text("external")}, {"module", text("external/ext/z.lua")}, {"ordinal", ordinal}};
  descriptor->fields     = {{"id", text("custom")},
                            {"abi", text("shape_v1")},
                            {"revision", text("r1")},
                            {"stages", text("optional")},
                            {"help", text("shape")}};
  auto entry             = std::make_shared<value>();
  entry->kind            = value_kind::record;
  entry->fields          = {{"owner", owner}, {"descriptor", descriptor}};
  at(c, "shapes")->items = {entry};
  encode(in.catalog, c);
  auto p = prepare_effective_stdlib(in.view());
  REQUIRE(p);
  REQUIRE(verify_effective_stdlib(p->files));
  ordinal->integer = 2;
  encode(in.catalog, c);
  REQUIRE_FALSE(prepare_effective_stdlib(in.view()));
  ordinal->integer = 1;
  std::swap(in.external[0], in.external[1]);
  in.profile = R"({"version":1,"modules":["ext/a.lua","ext/z.lua"]})";
  encode(in.catalog, c);
  REQUIRE_FALSE(prepare_effective_stdlib(in.view()));
  ordinal->integer = 2;
  encode(in.catalog, c);
  REQUIRE(prepare_effective_stdlib(in.view()));
}
TEST_CASE("replacement.effective provenance component and defensive outer resource refusals") {
  inputs in;
  in.profile = std::string(65537, ' ');
  auto r     = prepare_effective_stdlib(in.view());
  REQUIRE_FALSE(r);
  REQUIRE(r.error().code == effective_error_code::resource_limit);
  in                     = inputs{};
  in.external[0].payload = std::string(mib + 1, 'x');
  REQUIRE_FALSE(prepare_effective_stdlib(in.view()));
  in         = inputs{};
  in.catalog = std::string(mib + 1, 'x');
  REQUIRE_FALSE(prepare_effective_stdlib(in.view()));
  in = inputs{};
  in.embedded.resize(4097);
  REQUIRE_FALSE(prepare_effective_stdlib(in.view()));
  in = inputs{};
  in.external.resize(65);
  REQUIRE_FALSE(prepare_effective_stdlib(in.view()));
  auto f = fixture();
  f.resize(8193);
  REQUIRE_FALSE(verify_effective_stdlib(f));
  f                                     = fixture();
  file(f, "external/ext/z.lua").payload = std::string(16 * mib + 1, 'x');
  REQUIRE_FALSE(verify_effective_stdlib(f));
  f                                          = fixture();
  file(f, "__planar__/embedded.lua").payload = std::string(16 * mib, 'x');
  REQUIRE_FALSE(verify_effective_stdlib(f));
  f                                           = fixture();
  file(f, "__planar__/effective.bin").payload = std::string(mib + 1, 'x');
  REQUIRE_FALSE(verify_effective_stdlib(f));
  f = fixture();
  for (int i = 0; i < 33; ++i)
    f.push_back({std::format("external/ext/a{}.lua", i), std::string(mib, 'x'), 420});
  REQUIRE_FALSE(verify_effective_stdlib(f));
}
TEST_CASE("replacement.effective provenance reachable external byte and profile boundaries") {
  inputs in;
  in.external.clear();
  in.profile = R"({"version":1,"modules":[)";
  for (int i = 0; i < 8; ++i) {
    auto p = std::format("ext/m{}.lua", i);
    if (i)
      in.profile += ',';
    in.profile += '"' + p + '"';
    in.external.push_back({p, std::string(mib, 'x'), 420});
  }
  in.profile += "]}";
  in.profile.append(65536 - in.profile.size(), ' ');
  auto r = prepare_effective_stdlib(in.view());
  REQUIRE(r);
  REQUIRE(verify_effective_stdlib(r->files));
  in.external.push_back({"ext/extra.lua", "x", 420});
  in.profile =
      R"({"version":1,"modules":["ext/m0.lua","ext/m1.lua","ext/m2.lua","ext/m3.lua","ext/m4.lua","ext/m5.lua","ext/m6.lua","ext/m7.lua","ext/extra.lua"]})";
  auto excessive = prepare_effective_stdlib(in.view());
  REQUIRE_FALSE(excessive);
  REQUIRE(excessive.error().code == effective_error_code::resource_limit);
}
TEST_CASE("replacement.effective provenance prefixed path byte and depth limits") {
  for (bool shipped : {false, true})
    for (bool depth : {false, true}) {
      auto path = [&](int extra) {
        if (depth) {
          std::string p;
          for (int i = 0; i < 62 + extra; ++i)
            p += "d/";
          return p + "m.lua";
        }
        std::string p;
        for (int i = 0; i < 8; ++i)
          p += std::string(110 + (i == 0 ? extra : 0), 'd') + "/";
        return p + std::string((shipped ? 1016 : 1015) - (p.size() - extra) - 4, 'm') + ".lua";
      };
      inputs in;
      for (int extra : {0, 1}) {
        auto p = path(extra);
        CAPTURE(shipped, depth, extra, p.size());
        if (shipped)
          reassemble(in, {{p, "-- source"}, {"planar/commands.lua", "-- commands"}});
        else {
          in.external = {{p, "-- source", 420}};
          in.profile  = "{\"version\":1,\"modules\":[\"" + p + "\"]}";
        }
        auto r = prepare_effective_stdlib(in.view());
        CAPTURE(shipped, depth, extra, p.size());
        if (extra == 0) {
          REQUIRE(r);
          REQUIRE(verify_effective_stdlib(r->files));
        } else {
          REQUIRE_FALSE(r);
          REQUIRE(r.error().code == effective_error_code::resource_limit);
        }
      }
    }
}
TEST_CASE("replacement.effective provenance combined manifest one MiB ceiling") {
  inputs in;
  auto   c        = decode(in.catalog);
  auto   commands = at(c, "commands");
  // Valid external descriptors provide independently bounded help fields; the catalog alone fits.
  for (int i = 0; i < 59; ++i) {
    auto e                         = mut(canonical_decode(canonical_encode(commands->items[0]).value()).value());
    auto d                         = at(e, "descriptor");
    at(d, "name")->string          = std::format("z{:02}", i);
    at(d, "family")->string        = "extension_read_v1";
    at(d, "handler_id")->string    = std::format("custom.z{:02}.v1", i);
    at(d, "response_kind")->string = "extension_v1";
    at(d, "capabilities")->items.clear();
    at(d, "options")->items.clear();
    for (int j = 0; j < 4; ++j) {
      auto o =
          mut(canonical_decode(canonical_encode(at(commands->items[0], "descriptor")->fields.at("options")->items[0]).value())
                  .value());
      at(o, "name")->string      = std::format("--x{}", j);
      at(o, "required")->boolean = false;
      at(d, "options")->items.push_back(o);
    }
    at(at(e, "owner"), "tier")->string     = "external";
    at(at(e, "owner"), "module")->string   = "external/ext/z.lua";
    at(at(e, "owner"), "ordinal")->integer = 1;
    commands->items.push_back(e);
  }
  encode(in.catalog, c);
  auto initial = prepare_effective_stdlib(in.view());
  REQUIRE(initial);
  auto                                current   = file(initial->files, "__planar__/effective.bin").payload.size();
  std::size_t                         remaining = mib - current;
  std::vector<std::shared_ptr<value>> helps;
  for (auto& e : commands->items) {
    auto d = at(e, "descriptor");
    helps.push_back(at(d, "help"));
    for (auto& o : at(d, "options")->items)
      helps.push_back(at(o, "help"));
  }
  for (auto& h : helps) {
    auto n = std::min(remaining, 4096 - h->string.size());
    h->string.append(n, 'h');
    remaining -= n;
  }
  REQUIRE(remaining == 0);
  encode(in.catalog, c);
  auto exact = prepare_effective_stdlib(in.view());
  REQUIRE(exact);
  REQUIRE(file(exact->files, "__planar__/effective.bin").payload.size() == mib);
  REQUIRE(verify_effective_stdlib(exact->files));
  auto h = std::ranges::find_if(helps, [](auto& v) { return v->string.size() < 4096; });
  REQUIRE(h != helps.end());
  (*h)->string += 'x';
  encode(in.catalog, c);
  REQUIRE(in.catalog.size() < mib);
  auto r = prepare_effective_stdlib(in.view());
  REQUIRE_FALSE(r);
  REQUIRE(r.error().code == effective_error_code::resource_limit);
}

TEST_CASE("replacement.effective provenance maximum reachable file membership") {
  inputs                                           in;
  std::vector<std::pair<std::string, std::string>> sources{{"planar/z.lua", "z"}, {"planar/commands.lua", "commands"}};
  for (int i = 0; i < 4092; ++i)
    sources.push_back({std::format("planar/m{:04}.lua", i), ""});
  reassemble(in, std::move(sources));
  in.external.clear();
  in.profile = R"({"version":1,"modules":[)";
  for (int i = 0; i < 64; ++i) {
    auto path = std::format("ext/m{:02}.lua", i);
    if (i)
      in.profile += ',';
    in.profile += '"' + path + '"';
    in.external.push_back({path, "", 420});
  }
  in.profile += "]}";
  auto p = prepare_effective_stdlib(in.view());
  REQUIRE(p);
  REQUIRE(p->files.size() == 4162);
  REQUIRE(verify_effective_stdlib(p->files));
  auto old_digest = p->digest;
  p->digest.fill(0);
  auto verified = verify_effective_stdlib(p->files);
  REQUIRE(verified);
  REQUIRE(verified->digest == old_digest);
  in.embedded.push_back({"planar/extra.lua", "", 420});
  auto r = prepare_effective_stdlib(in.view());
  REQUIRE_FALSE(r);
  REQUIRE(r.error().code == effective_error_code::resource_limit);
}
