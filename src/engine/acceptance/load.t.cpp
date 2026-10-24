// Declaration-only load orchestration through real trusted routing and interpretation.
#include <catch2/catch_test_macros.hpp>
#include <unistd.h>
import std;
import planar.engine.acceptance.load;
import planar.engine.runtime;
import planar.engine.history.provenance;
import planar.engine.canonical;
import planar.engine.json_codec;
using namespace planar::engine;
using namespace planar::engine::acceptance;
namespace {
value_ptr text(std::string s) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = std::move(s);
  return v;
}
value_ptr obj(std::map<std::string, value_ptr> f) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::record;
  v->fields = std::move(f);
  return v;
}
value_ptr arr(std::vector<value_ptr> a) {
  auto v   = std::make_shared<value>();
  v->kind  = value_kind::list;
  v->items = std::move(a);
  return v;
}
value_ptr change(value_ptr v, std::string k, value_ptr x) {
  auto c       = std::make_shared<value>(*v);
  c->fields[k] = std::move(x);
  return c;
}
std::string bytes(const std::filesystem::path& p) {
  std::ifstream f(p, std::ios::binary);
  REQUIRE(f.good());
  return {std::istreambuf_iterator<char>(f), {}};
}
void write(const std::filesystem::path& p, std::string_view s) {
  std::ofstream f(p, std::ios::binary);
  f << s;
  REQUIRE(f.good());
}
value_ptr invoke(std::string_view entry, value_ptr input) {
  auto r = run_trusted(entry, input);
  REQUIRE(r);
  return *r;
}
value_ptr success(std::string_view entry, value_ptr input) {
  auto r = invoke(entry, input);
  if (r->fields.contains("error")) {
    auto e = codec_encode(r);
    INFO(*e);
  }
  REQUIRE_FALSE(r->fields.contains("error"));
  return r->fields.at("value");
}
struct fixture {
  std::filesystem::path root, workspace, registry;
  fixture() {
    std::string p = (std::filesystem::temp_directory_path() / "planar-load-XXXXXX").string();
    REQUIRE(mkdtemp(p.data()));
    root = std::filesystem::canonical(p);
    std::filesystem::copy(std::filesystem::path(PLANAR_LOAD_FIXTURE) / "workspace", root / "workspace",
                          std::filesystem::copy_options::recursive);
    workspace = root / "workspace/workspace.lua";
    registry  = root / "registry.json";
    std::filesystem::copy_file(std::filesystem::path(PLANAR_LOAD_FIXTURE) / "registry/quota-tools.json", registry);
  }
  ~fixture() {
    std::filesystem::remove_all(root);
  }
  load_capture open() {
    auto o = load_capture::open_workspace(workspace.string(), registry.string());
    REQUIRE(o);
    return std::move(*o);
  }
  value_ptr route(load_capture& o) {
    auto w = o.workspace();
    REQUIRE(w);
    return success("route_workspace", *w);
  }
  value_ptr capture(load_capture& o) {
    auto r = route(o);
    auto b = o.capture(r);
    if (!b)
      INFO(static_cast<int>(b.error().code));
    REQUIRE(b);
    return *b;
  }
  value_ptr interpreted(load_capture& o) {
    return success("interpret_bundle", capture(o));
  }
  prepared_acceptance prepare(load_capture& o) {
    auto i = interpreted(o);
    auto s = history::prepare_embedded_stdlib();
    REQUIRE(s);
    auto p = o.prepare(i->fields.at("bundle"), i->fields.at("semantic"), s->files);
    if (!p)
      INFO(static_cast<int>(p.error().code));
    REQUIRE(p);
    return std::move(*p);
  }
};
std::vector<history::tree_file> stdlib() {
  auto s = history::prepare_embedded_stdlib();
  REQUIRE(s);
  return s->files;
}
void invalid_preparation(fixture& f, const std::function<value_ptr(value_ptr)>& modify, bool semantic = false) {
  auto o = f.open();
  auto i = f.interpreted(o);
  auto b = i->fields.at("bundle"), m = i->fields.at("semantic");
  if (semantic)
    m = modify(m);
  else
    b = modify(b);
  auto p = o.prepare(b, m, stdlib());
  REQUIRE_FALSE(p);
  CHECK(p.error().code == load_error_code::invalid_interpretation);
  auto again = o.prepare(i->fields.at("bundle"), i->fields.at("semantic"), stdlib());
  REQUIRE_FALSE(again);
  CHECK(again.error().code == load_error_code::invalid_state);
}
} // namespace
TEST_CASE("load capture prepares complete multi-job closure and exact trusted provenance without source trees", "[load]") {
  fixture f;
  auto    o = f.open();
  auto    p = f.prepare(o);
  auto    d = p.data();
  REQUIRE(d);
  CHECK((*d)->workspace_id == "quota");
  REQUIRE((*d)->interpretation.interpretation->fields.at("bundle")->fields.at("jobs")->items.size() == 2);
  auto                     source = (*d)->source;
  std::vector<std::string> names;
  for (auto& file : source) {
    names.push_back(file.path);
    CHECK(file.payload == bytes(f.root / "workspace" / file.path));
    CHECK(file.mode == 420);
  }
  CHECK(names == std::vector<std::string>{"combined.lua", "spec.lua", "split.lua", "text/quota-api.md", "text/shared.md",
                                          "workspace.lua"});
  CHECK(std::ranges::is_sorted(names));
  CHECK(std::ranges::find(names, "workspace.lua") != names.end());
  CHECK(std::ranges::find(names, "combined.lua") != names.end());
  CHECK(std::ranges::find(names, "split.lua") != names.end());
  CHECK(std::ranges::find(names, "spec.lua") != names.end());
  CHECK(std::ranges::find(names, "text/quota-api.md") != names.end());
  CHECK((*d)->registry.files.size() == 1);
  CHECK((*d)->registry.files[0].path == "input.json");
  CHECK((*d)->registry.files[0].payload == bytes(f.registry));
  auto s = stdlib();
  REQUIRE((*d)->stdlib.files.size() == s.size());
  for (std::size_t n = 0; n < s.size(); ++n) {
    CHECK((*d)->stdlib.files[n].path == s[n].path);
    CHECK((*d)->stdlib.files[n].payload == s[n].payload);
  }
  CHECK_FALSE(std::filesystem::exists(f.root / "source"));
  CHECK_FALSE(std::filesystem::exists(f.root / "base"));
  CHECK(p.verify());
  auto again = o.prepare({}, {}, s);
  REQUIRE_FALSE(again);
  CHECK(again.error().code == load_error_code::invalid_state);
}
TEST_CASE("load capture rejects malformed and mismatched routes before routed reads", "[load]") {
  fixture f;
  for (int n = 0; n < 9; ++n) {
    auto o    = f.open();
    auto r    = f.route(o);
    auto jobs = r->fields.at("jobs")->items;
    switch (n) {
    case 0:
      r = change(r, "extra", text("x"));
      break;
    case 1:
      r = change(r, "file", text("other.lua"));
      break;
    case 2:
      r = change(r, "spec", text("missing.lua"));
      break;
    case 3:
      std::ranges::reverse(jobs);
      r = change(r, "jobs", arr(jobs));
      break;
    case 4:
      jobs.push_back(jobs[0]);
      r = change(r, "jobs", arr(jobs));
      break;
    case 5:
      jobs[0] = change(jobs[0], "file", text("../outside.lua"));
      r       = change(r, "jobs", arr(jobs));
      break;
    case 6:
      r = change(r, "workspace", change(r->fields.at("workspace"), "id", text("other")));
      break;
    case 8:
      r = change(r, "workspace", change(r->fields.at("workspace"), "checks", text("bad container")));
      break;
    case 7:
      r = change(r, "registry", obj({{"id", text("other")}, {"revision", text("v1")}}));
      break;
    }
    std::filesystem::rename(f.root / "workspace/spec.lua", f.root / "workspace/spec.saved");
    auto b = o.capture(r);
    REQUIRE_FALSE(b);
    CHECK(b.error().code == load_error_code::invalid_route);
    std::filesystem::rename(f.root / "workspace/spec.saved", f.root / "workspace/spec.lua");
    CHECK_FALSE(o.capture(r));
  }
}
TEST_CASE("load capture trusted all-job validation preserves unselected and duplicate logical diagnostics", "[load]") {
  fixture f;
  for (auto scenario : {"invalid", "role", "work", "job", "job_header", "constructor"}) {
    auto target   = f.root / (std::string(scenario) == "work"  ? "workspace/spec.lua"
                              : std::string(scenario) == "job" ? "workspace/workspace.lua"
                                                               : "workspace/split.lua");
    auto original = bytes(target);
    auto altered  = original;
    if (std::string(scenario) == "invalid") {
      auto pos = altered.find("version = 1");
      REQUIRE(pos != std::string::npos);
      altered.replace(pos, 11, "version = 2");
    } else if (std::string(scenario) == "job_header") {
      auto pos = altered.find("id = \"split\"");
      REQUIRE(pos != std::string::npos);
      altered.replace(pos, 12, "id = \"other\"");
    } else if (std::string(scenario) == "constructor") {
      auto pos = altered.find("p.job");
      REQUIRE(pos != std::string::npos);
      altered.replace(pos, 5, "p.spec");
    } else if (std::string(scenario) == "job") {
      auto pos = altered.find("id = \"split\"");
      REQUIRE(pos != std::string::npos);
      altered.replace(pos, 12, "id = \"combined\"");
    } else {
      std::string token = std::string(scenario) == "work" ? "id = \"command\"" : "id = \"cli_worker\"";
      auto        pos   = altered.find(token);
      if (pos == std::string::npos) {
        INFO(altered);
        FAIL("fixture identity unavailable");
      }
      altered.replace(pos, token.size(), std::string(scenario) == "work" ? "id = \"library\"" : "id = \"library_worker\"");
    }
    write(target, altered);
    auto o = f.open();
    auto w = o.workspace();
    REQUIRE(w);
    auto r = invoke("route_workspace", *w);
    if (std::string(scenario) == "job") {
      REQUIRE(r->fields.contains("error"));
      CHECK(r->fields.at("error")->fields.at("file")->string == "workspace.lua");
    } else {
      REQUIRE(r->fields.contains("value"));
      auto b = o.capture(r->fields.at("value"));
      REQUIRE(b);
      auto i = invoke("interpret_bundle", *b);
      REQUIRE(i->fields.contains("error"));
      auto e = i->fields.at("error");
      CHECK(e->fields.at("file")->string == (std::string(scenario) == "work" ? "spec.lua" : "split.lua"));
      CHECK_FALSE(e->fields.at("field")->string.empty());
      CHECK(e->fields.at("code")->string == (std::string(scenario) == "invalid"       ? "unsupported_version"
                                             : std::string(scenario) == "constructor" ? "schema"
                                                                                      : "reference"));
    }
    write(target, original);
  }
}
TEST_CASE("load capture refuses B M membership headers and captured reference mismatches", "[load]") {
  fixture f;
  invalid_preparation(f, [](auto b) { return change(b, "workspace_file", text("other.lua")); });
  invalid_preparation(f, [](auto b) { return change(b, "workspace", change(b->fields.at("workspace"), "id", text("other"))); });
  invalid_preparation(f, [](auto b) { return change(b, "spec", change(b->fields.at("spec"), "id", text("other"))); });
  invalid_preparation(f, [](auto b) {
    auto j = b->fields.at("jobs")->items;
    j.pop_back();
    return change(b, "jobs", arr(j));
  });
  invalid_preparation(f, [](auto b) {
    auto j = b->fields.at("jobs")->items;
    j[0]   = change(j[0], "file", text("wrong.lua"));
    return change(b, "jobs", arr(j));
  });
  invalid_preparation(
      f,
      [](auto m) {
        auto j = m->fields.at("jobs")->items;
        j.pop_back();
        return change(m, "jobs", arr(j));
      },
      true);
  invalid_preparation(f,
                      [](auto b) { return change(b, "registry", change(b->fields.at("registry"), "revision", text("other"))); });
  invalid_preparation(f, [](auto b) {
    auto s         = b->fields.at("spec");
    auto contracts = s->fields.at("contracts")->items;
    auto expected  = contracts[0]->fields.at("expected");
    contracts[0]   = change(contracts[0], "expected", change(expected, "bytes", text("forged")));
    return change(b, "spec", change(s, "contracts", arr(contracts)));
  });
}
TEST_CASE("load capture retains original selected receipts and ignores unrelated siblings", "[load]") {
  for (int n = 0; n < 7; ++n) {
    fixture f;
    auto    o     = f.open();
    auto    p     = f.prepare(o);
    auto    saved = p.data();
    REQUIRE(saved);
    switch (n) {
    case 0:
      write(f.root / "workspace/text/quota-api.md", "changed");
      break;
    case 1:
      std::filesystem::permissions(f.root / "workspace/spec.lua", std::filesystem::perms::owner_exec,
                                   std::filesystem::perm_options::add);
      break;
    case 2:
      std::filesystem::rename(f.root / "workspace/text", f.root / "workspace/old-text");
      std::filesystem::copy(f.root / "workspace/old-text", f.root / "workspace/text", std::filesystem::copy_options::recursive);
      break;
    case 3:
      std::filesystem::rename(f.root / "workspace", f.root / "old-workspace");
      std::filesystem::copy(f.root / "old-workspace", f.root / "workspace", std::filesystem::copy_options::recursive);
      break;
    case 4:
      write(f.registry, bytes(f.registry) + " ");
      break;
    case 5:
      std::filesystem::rename(f.root / "workspace/spec.lua", f.root / "workspace/saved.lua");
      std::filesystem::create_symlink("saved.lua", f.root / "workspace/spec.lua");
      break;
    case 6:
      write(f.root / "workspace/unrelated.txt", "unrelated");
      std::filesystem::create_directories(f.root / "workspace/other");
      break;
    }
    auto verified = p.verify();
    if (n == 6)
      CHECK(verified);
    else {
      REQUIRE_FALSE(verified);
      CHECK(verified.error().code == "capture_changed");
    }
    auto after = p.data();
    REQUIRE(after);
    CHECK((*saved)->source_digest == (*after)->source_digest);
  }
}
TEST_CASE("load capture state moves failures paths aliases provenance and bounds are terminal", "[load]") {
  fixture f;
  CHECK_FALSE(load_capture::open_workspace("relative.lua", f.registry.string()));
  CHECK_FALSE(load_capture::open_workspace((f.root / "workspace/./workspace.lua").string(), f.registry.string()));
  {
    auto o      = f.open();
    auto failed = o.capture({});
    REQUIRE_FALSE(failed);
    CHECK(failed.error().code == load_error_code::invalid_state);
    CHECK_FALSE(o.workspace());
  }
  {
    auto o     = f.open();
    auto moved = std::move(o);
    CHECK_FALSE(o.workspace());
    auto* alias = &moved;
    moved       = std::move(*alias);
    auto p      = f.prepare(moved);
    CHECK(p.verify());
  }
  {
    auto o     = f.open();
    auto i     = f.interpreted(o);
    auto moved = std::move(o);
    CHECK_FALSE(o.prepare({}, {}, {}));
    auto s = stdlib();
    s[0].payload += 'x';
    auto p = moved.prepare(i->fields.at("bundle"), i->fields.at("semantic"), s);
    CHECK_FALSE(p);
    CHECK_FALSE(moved.prepare({}, {}, {}));
  }
  {
    auto o = f.open();
    auto i = f.interpreted(o);
    auto s = stdlib();
    s.resize(4097);
    auto p = o.prepare(i->fields.at("bundle"), i->fields.at("semantic"), s);
    REQUIRE_FALSE(p);
    CHECK(p.error().code == load_error_code::resource_limit);
  }
  {
    auto o = f.open();
    auto r = f.route(o);
    std::filesystem::remove(f.registry);
    CHECK_FALSE(o.capture(r));
    CHECK_FALSE(o.capture(r));
  }
  {
    std::filesystem::create_symlink("workspace/workspace.lua", f.root / "alias.lua");
    CHECK_FALSE(load_capture::open_workspace((f.root / "alias.lua").string(), (f.root / "other.json").string()));
  }
}
TEST_CASE("load capture seals original workspace and refuses missing linked or oversized references", "[load]") {
  for (int n = 0; n < 5; ++n) {
    fixture    f;
    auto       o        = f.open();
    auto       r        = f.route(o);
    const auto selected = f.root / "workspace/text/quota-api.md";
    switch (n) {
    case 0:
      write(f.workspace, bytes(f.workspace) + "\n-- changed after opening\n");
      break;
    case 1:
      std::filesystem::remove(selected);
      break;
    case 2:
      std::filesystem::rename(selected, f.root / "workspace/text/saved.md");
      std::filesystem::create_symlink("saved.md", selected);
      break;
    case 3:
      write(selected, std::string(1024 * 1024 + 1, 'x'));
      break;
    case 4:
      std::filesystem::rename(f.registry, f.root / "saved-registry.json");
      std::filesystem::create_symlink("saved-registry.json", f.registry);
      break;
    }
    auto result = o.capture(r);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == (n == 3 ? load_error_code::resource_limit : load_error_code::capture));
    if (n == 0)
      CHECK(result.error().detail.code == "capture_changed");
    CHECK_FALSE(o.capture(r));
  }
  fixture f;
  auto    excessive = load_capture::open_workspace(f.workspace.string(), std::string(64 * 1024 * 1024 + 1, 'x'));
  REQUIRE_FALSE(excessive);
  CHECK(excessive.error().code == load_error_code::resource_limit);
  auto a = f.open();
  auto i = f.interpreted(a);
  auto b = f.open();
  b      = std::move(a);
  CHECK_FALSE(a.workspace());
  auto p = b.prepare(i->fields.at("bundle"), i->fields.at("semantic"), stdlib());
  REQUIRE(p);
  CHECK(p->verify());
}
