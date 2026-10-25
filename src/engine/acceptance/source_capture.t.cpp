// Source capture receipt binding, admission ordering, retained seals, moves and terminal refusals.
#include <catch2/catch_test_macros.hpp>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.engine.acceptance.source_capture;
import planar.engine.capture.profile;
import planar.engine.runtime;
import planar.engine.json_codec;
using namespace planar::engine;
using namespace planar::engine::acceptance;
extern "C" void          planar_admission_watch(std::uint64_t device, std::uint64_t inode);
extern "C" std::uint64_t planar_admission_read_attempts();
namespace {
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
std::string read_bytes(const std::filesystem::path& p) {
  std::ifstream f(p, std::ios::binary);
  REQUIRE(f.good());
  return {std::istreambuf_iterator<char>(f), {}};
}
void put_bytes(const std::filesystem::path& p, std::string_view s) {
  std::ofstream f(p, std::ios::binary);
  f << s;
  REQUIRE(f.good());
}
void watch(const std::filesystem::path& p) {
  struct stat s{};
  REQUIRE(::stat(p.c_str(), &s) == 0);
  planar_admission_watch(s.st_dev, s.st_ino);
}
struct fixture {
  std::filesystem::path root, workspace_dir, workspace, registry, profile_file;
  fixture() {
    std::string p = (std::filesystem::temp_directory_path() / "planar-source-XXXXXX").string();
    REQUIRE(mkdtemp(p.data()));
    root = std::filesystem::canonical(p);
    std::filesystem::copy(std::filesystem::path(PLANAR_LOAD_FIXTURE) / "workspace", root / "workspace",
                          std::filesystem::copy_options::recursive);
    std::filesystem::create_directories(root / "trusted");
    workspace_dir = root / "workspace";
    workspace     = workspace_dir / "workspace.lua";
    registry      = root / "registry.json";
    std::filesystem::copy_file(std::filesystem::path(PLANAR_LOAD_FIXTURE) / "registry/quota-tools.json", registry);
    profile_file = root / "trusted/profile.json";
    put_bytes(profile_file, R"({"version":1,"modules":[]})");
  }
  ~fixture() {
    std::filesystem::remove_all(root);
  }
};
profile_capture open_profile(const fixture& f) {
  auto p = profile_capture::open(f.profile_file.string());
  REQUIRE(p);
  return std::move(*p);
}
struct receipts {
  retained_root_receipt root;
  retained_file_receipt declaration;
  retained_file_receipt registry;
};
// Admits the supplied root, declaration and registry through one original profile admission.
receipts admit(profile_capture& p, const fixture& f, const std::filesystem::path& declaration,
               const std::filesystem::path& registry) {
  REQUIRE(p.admit_root("workspace", f.workspace_dir.string()));
  REQUIRE(p.admit_file("declaration", declaration.string(), admission_file_role::declaration));
  REQUIRE(p.admit_file("registry", registry.string(), admission_file_role::registry));
  auto root = p.retain_root("workspace");
  REQUIRE(root);
  auto declared = p.retain_file("declaration");
  REQUIRE(declared);
  auto registered = p.retain_file("registry");
  REQUIRE(registered);
  return {std::move(*root), std::move(*declared), std::move(*registered)};
}
value_ptr captured_bundle(source_capture& source) {
  auto w = source.workspace();
  REQUIRE(w);
  auto route = success("route_workspace", *w);
  auto b     = source.capture(route);
  if (!b)
    INFO(static_cast<int>(b.error().code));
  REQUIRE(b);
  return *b;
}
template <class... A>
concept can_open_bound = requires(A&&... a) { source_capture::open_bound(std::forward<A>(a)...); };
} // namespace
static_assert(!std::is_default_constructible_v<source_capture>);
static_assert(!std::is_copy_constructible_v<source_capture>);
static_assert(std::is_nothrow_move_constructible_v<source_capture>);
static_assert(!can_open_bound<std::string_view, std::string_view>);
static_assert(!can_open_bound<std::string_view, std::string_view, retained_root_receipt, retained_file_receipt>);
static_assert(
    can_open_bound<std::string_view, std::string_view, retained_root_receipt, retained_file_receipt, retained_file_receipt>);

TEST_CASE("source capture factories require three original receipts and refuse moved and poisoned authority",
          "[source_capture]") {
  fixture f;
  SECTION("moved declaration receipt is refused as a capture error") {
    auto p = open_profile(f);
    auto r = admit(p, f, f.workspace, f.registry);
    {
      retained_file_receipt sink = std::move(r.declaration);
    }
    auto source = source_capture::open_bound(f.workspace.string(), f.registry.string(), std::move(r.root),
                                             std::move(r.declaration), std::move(r.registry));
    REQUIRE_FALSE(source);
    CHECK(source.error().code == source_error_code::capture);
    CHECK(source.error().detail.code == "capture_changed");
    CHECK_FALSE(p.verify());
  }
  SECTION("poisoned declaration receipt is refused before any declaration read") {
    auto p = open_profile(f);
    auto r = admit(p, f, f.workspace, f.registry);
    r.declaration.poison();
    auto source = source_capture::open_bound(f.workspace.string(), f.registry.string(), std::move(r.root),
                                             std::move(r.declaration), std::move(r.registry));
    REQUIRE_FALSE(source);
    CHECK(source.error().code == source_error_code::capture);
    CHECK(source.error().detail.code == "capture_changed");
    CHECK_FALSE(p.verify());
  }
  SECTION("moved registry receipt is refused at construction before any read") {
    auto p = open_profile(f);
    auto r = admit(p, f, f.workspace, f.registry);
    {
      retained_file_receipt sink = std::move(r.registry);
    }
    watch(f.workspace);
    auto source = source_capture::open_bound(f.workspace.string(), f.registry.string(), std::move(r.root),
                                             std::move(r.declaration), std::move(r.registry));
    REQUIRE_FALSE(source);
    CHECK(source.error().code == source_error_code::capture);
    CHECK(source.error().detail.code == "capture_changed");
    CHECK(planar_admission_read_attempts() == 0);
    CHECK_FALSE(p.verify());
  }
}

TEST_CASE("source capture refuses receipts outside one original admission at construction", "[source_capture]") {
  fixture f;
  SECTION("registry receipt from a second independent admission") {
    auto p1 = open_profile(f);
    auto p2 = open_profile(f);
    REQUIRE(p1.admit_root("workspace", f.workspace_dir.string()));
    REQUIRE(p1.admit_file("declaration", f.workspace.string(), admission_file_role::declaration));
    REQUIRE(p2.admit_file("registry", f.registry.string(), admission_file_role::registry));
    auto root     = p1.retain_root("workspace");
    auto declared = p1.retain_file("declaration");
    auto registry = p2.retain_file("registry");
    REQUIRE(root);
    REQUIRE(declared);
    REQUIRE(registry);
    watch(f.workspace);
    auto source = source_capture::open_bound(f.workspace.string(), f.registry.string(), std::move(*root), std::move(*declared),
                                             std::move(*registry));
    REQUIRE_FALSE(source);
    CHECK(source.error().code == source_error_code::capture);
    CHECK(source.error().detail.code == "capture_changed");
    CHECK(planar_admission_read_attempts() == 0);
    CHECK_FALSE(p1.verify());
    CHECK_FALSE(p2.verify());
  }
  SECTION("moved-from root receipt is refused at construction before any read") {
    auto p = open_profile(f);
    auto r = admit(p, f, f.workspace, f.registry);
    {
      retained_root_receipt sink = std::move(r.root);
    }
    watch(f.workspace);
    auto source = source_capture::open_bound(f.workspace.string(), f.registry.string(), std::move(r.root),
                                             std::move(r.declaration), std::move(r.registry));
    REQUIRE_FALSE(source);
    CHECK(source.error().code == source_error_code::capture);
    CHECK(source.error().detail.code == "capture_changed");
    CHECK(planar_admission_read_attempts() == 0);
    CHECK_FALSE(p.verify());
  }
  SECTION("poisoned registry receipt is refused at construction before any read") {
    auto p = open_profile(f);
    auto r = admit(p, f, f.workspace, f.registry);
    r.registry.poison();
    watch(f.workspace);
    auto source = source_capture::open_bound(f.workspace.string(), f.registry.string(), std::move(r.root),
                                             std::move(r.declaration), std::move(r.registry));
    REQUIRE_FALSE(source);
    CHECK(source.error().code == source_error_code::capture);
    CHECK(source.error().detail.code == "capture_changed");
    CHECK(planar_admission_read_attempts() == 0);
    CHECK_FALSE(p.verify());
  }
  SECTION("poisoned root receipt is refused at construction before any read") {
    auto p = open_profile(f);
    auto r = admit(p, f, f.workspace, f.registry);
    r.root.poison();
    watch(f.workspace);
    auto source = source_capture::open_bound(f.workspace.string(), f.registry.string(), std::move(r.root),
                                             std::move(r.declaration), std::move(r.registry));
    REQUIRE_FALSE(source);
    CHECK(source.error().code == source_error_code::capture);
    CHECK(source.error().detail.code == "capture_changed");
    CHECK(planar_admission_read_attempts() == 0);
    CHECK_FALSE(p.verify());
  }
}

TEST_CASE("source capture refuses a registry hard-linked to the declaration as path_denied", "[source_capture]") {
  fixture f;
  auto    colocated = f.workspace_dir / "registry.json";
  REQUIRE(::link(f.workspace.c_str(), colocated.c_str()) == 0);
  auto p      = open_profile(f);
  auto r      = admit(p, f, f.workspace, colocated);
  auto source = source_capture::open_bound(f.workspace.string(), colocated.string(), std::move(r.root), std::move(r.declaration),
                                           std::move(r.registry));
  REQUIRE(source);
  auto route = success("route_workspace", *source->workspace());
  auto b     = source->capture(route);
  REQUIRE_FALSE(b);
  CHECK(b.error().code == source_error_code::capture);
  CHECK(b.error().detail.code == "path_denied");
  CHECK(b.error().detail.message == "Hard-link input alias");
  CHECK_FALSE(source->sealed_ready());
  CHECK_FALSE(p.verify());
}

TEST_CASE("source capture admission precedes registration and reads no declaration content", "[source_capture]") {
  fixture f;
  auto    p = open_profile(f);
  watch(f.workspace);
  auto r = admit(p, f, f.workspace, f.registry);
  CHECK(planar_admission_read_attempts() == 0);
  CHECK(p.verify());
  CHECK(planar_admission_read_attempts() == 0);
  auto source = source_capture::open_bound(f.workspace.string(), f.registry.string(), std::move(r.root), std::move(r.declaration),
                                           std::move(r.registry));
  REQUIRE(source);
  CHECK(planar_admission_read_attempts() > 0);
}

TEST_CASE("source capture refuses a hard-linked trusted declaration before content or factory", "[source_capture]") {
  fixture f;
  put_bytes(f.root / "trusted/a.lua", "return 'private module bytes'\n");
  put_bytes(f.profile_file, R"({"version":1,"modules":["a.lua"]})");
  std::filesystem::remove(f.workspace);
  REQUIRE(::link((f.root / "trusted/a.lua").c_str(), f.workspace.c_str()) == 0);
  auto p = open_profile(f);
  REQUIRE(p.admit_root("workspace", f.workspace_dir.string()));
  watch(f.workspace);
  auto admitted = p.admit_file("declaration", f.workspace.string(), admission_file_role::declaration);
  REQUIRE_FALSE(admitted);
  CHECK(admitted.error().detail.code == "unsafe_path");
  CHECK(planar_admission_read_attempts() == 0);
}

TEST_CASE("source capture refuses a declaration replaced between registration and first read", "[source_capture]") {
  fixture f;
  auto    p = open_profile(f);
  auto    r = admit(p, f, f.workspace, f.registry);
  std::filesystem::rename(f.workspace, f.root / "saved-workspace.lua");
  std::filesystem::copy_file(f.root / "saved-workspace.lua", f.workspace);
  watch(f.workspace);
  auto source = source_capture::open_bound(f.workspace.string(), f.registry.string(), std::move(r.root), std::move(r.declaration),
                                           std::move(r.registry));
  REQUIRE_FALSE(source);
  CHECK(source.error().code == source_error_code::capture);
  CHECK(source.error().detail.code == "capture_changed");
  CHECK(planar_admission_read_attempts() == 0);
  CHECK_FALSE(p.verify());
}

TEST_CASE("source capture refuses retained authority for another selection or admission", "[source_capture]") {
  fixture f;
  SECTION("declaration receipt retained for a different admitted file") {
    auto p = open_profile(f);
    REQUIRE(p.admit_root("workspace", f.workspace_dir.string()));
    REQUIRE(p.admit_file("declaration", f.workspace.string(), admission_file_role::declaration));
    REQUIRE(p.admit_file("spec", (f.workspace_dir / "spec.lua").string(), admission_file_role::declaration));
    REQUIRE(p.admit_file("registry", f.registry.string(), admission_file_role::registry));
    auto root     = p.retain_root("workspace");
    auto spec     = p.retain_file("spec");
    auto registry = p.retain_file("registry");
    REQUIRE(root);
    REQUIRE(spec);
    REQUIRE(registry);
    auto source = source_capture::open_bound(f.workspace.string(), f.registry.string(), std::move(*root), std::move(*spec),
                                             std::move(*registry));
    REQUIRE_FALSE(source);
    CHECK(source.error().code == source_error_code::capture);
    CHECK(source.error().detail.message == "Retained file authority cannot be replaced");
  }
  SECTION("declaration receipt from a second independent admission") {
    auto p1 = open_profile(f);
    auto p2 = open_profile(f);
    REQUIRE(p1.admit_root("workspace", f.workspace_dir.string()));
    REQUIRE(p1.admit_file("registry", f.registry.string(), admission_file_role::registry));
    REQUIRE(p2.admit_file("declaration", f.workspace.string(), admission_file_role::declaration));
    auto root     = p1.retain_root("workspace");
    auto declared = p2.retain_file("declaration");
    auto registry = p1.retain_file("registry");
    REQUIRE(root);
    REQUIRE(declared);
    REQUIRE(registry);
    watch(f.workspace);
    auto source = source_capture::open_bound(f.workspace.string(), f.registry.string(), std::move(*root), std::move(*declared),
                                             std::move(*registry));
    REQUIRE_FALSE(source);
    CHECK(source.error().code == source_error_code::capture);
    CHECK(source.error().detail.code == "capture_changed");
    CHECK(source.error().detail.message == "Captured external profile changed");
    CHECK(planar_admission_read_attempts() == 0);
    CHECK_FALSE(p1.verify());
    CHECK_FALSE(p2.verify());
  }
  SECTION("successful bound open keeps the original authority through repeated plain reads") {
    auto p      = open_profile(f);
    auto r      = admit(p, f, f.workspace, f.registry);
    auto source = source_capture::open_bound(f.workspace.string(), f.registry.string(), std::move(r.root),
                                             std::move(r.declaration), std::move(r.registry));
    REQUIRE(source);
    auto bundle = captured_bundle(*source);
    CHECK(bundle->fields.at("registry")->fields.at("bytes")->string == read_bytes(f.registry));
    REQUIRE(source->verify());
    CHECK(p.verify());
    CHECK_FALSE(source->workspace());
  }
}

TEST_CASE("source capture retained flow survives moves and detects post-seal mutation", "[source_capture]") {
  for (int n = 0; n < 4; ++n) {
    INFO("mutation " << n);
    fixture f;
    auto    p      = open_profile(f);
    auto    r      = admit(p, f, f.workspace, f.registry);
    auto    source = source_capture::open_bound(f.workspace.string(), f.registry.string(), std::move(r.root),
                                                std::move(r.declaration), std::move(r.registry));
    REQUIRE(source);
    auto bundle = captured_bundle(*source);
    // spec.lua is discovered through the workspace root and must be part of the sealed workspace closure.
    bool spec_captured = false;
    for (auto& file : bundle->fields.at("files")->items)
      spec_captured = spec_captured || file->fields.at("path")->string == "spec.lua";
    REQUIRE(spec_captured);
    REQUIRE(source->sealed_ready());
    auto moved = std::move(*source);
    CHECK_FALSE(source->workspace());
    REQUIRE(moved.verify());
    auto target = source_capture::open_embedded(f.workspace.string(), f.registry.string());
    REQUIRE(target);
    source_capture owner = std::move(*target);
    owner                = std::move(moved);
    REQUIRE(owner.verify());
    switch (n) {
    case 0:
      put_bytes(f.workspace, read_bytes(f.workspace) + "\n-- changed after seal\n");
      break;
    case 1:
      put_bytes(f.registry, read_bytes(f.registry) + " ");
      break;
    case 2:
      std::filesystem::permissions(f.workspace, std::filesystem::perms::owner_exec, std::filesystem::perm_options::add);
      break;
    case 3:
      put_bytes(f.workspace_dir / "spec.lua", read_bytes(f.workspace_dir / "spec.lua") + "\n-- changed after seal\n");
      break;
    }
    auto verified = owner.verify();
    REQUIRE_FALSE(verified);
    CHECK(verified.error().code == source_error_code::capture);
    CHECK(verified.error().detail.code == "capture_changed");
    CHECK_FALSE(p.verify());
  }
}

TEST_CASE("source capture registry colocation binds standalone input and refuses trusted alias", "[source_capture]") {
  SECTION("registry beside the declaration is bound as input.json and excluded from workspace files") {
    fixture f;
    auto    colocated = f.workspace_dir / "registry.json";
    std::filesystem::copy_file(f.registry, colocated);
    auto p      = open_profile(f);
    auto r      = admit(p, f, f.workspace, colocated);
    auto source = source_capture::open_bound(f.workspace.string(), colocated.string(), std::move(r.root),
                                             std::move(r.declaration), std::move(r.registry));
    REQUIRE(source);
    auto bundle = captured_bundle(*source);
    CHECK(bundle->fields.at("registry")->fields.at("file")->string == "input.json");
    CHECK(bundle->fields.at("registry")->fields.at("bytes")->string == read_bytes(colocated));
    bool workspace_declared = false;
    for (auto& file : bundle->fields.at("files")->items) {
      CHECK(file->fields.at("path")->string != "registry.json");
      workspace_declared = workspace_declared || file->fields.at("path")->string == "workspace.lua";
    }
    CHECK(workspace_declared);
    REQUIRE(source->verify());
  }
  SECTION("registry hard-linked to a trusted module is refused at admission") {
    fixture f;
    put_bytes(f.root / "trusted/a.lua", "return 'private module bytes'\n");
    put_bytes(f.profile_file, R"({"version":1,"modules":["a.lua"]})");
    auto colocated = f.workspace_dir / "registry.json";
    REQUIRE(::link((f.root / "trusted/a.lua").c_str(), colocated.c_str()) == 0);
    auto p = open_profile(f);
    REQUIRE(p.admit_root("workspace", f.workspace_dir.string()));
    auto admitted = p.admit_file("registry", colocated.string(), admission_file_role::registry);
    REQUIRE_FALSE(admitted);
    CHECK(admitted.error().detail.code == "unsafe_path");
  }
}

TEST_CASE("source capture vanished declaration reports admitted change beside embedded missing", "[source_capture]") {
  fixture f;
  auto    missing  = f.workspace_dir / "missing.lua";
  auto    embedded = source_capture::open_embedded(missing.string(), f.registry.string());
  REQUIRE_FALSE(embedded);
  CHECK(embedded.error().code == source_error_code::capture);
  CHECK(embedded.error().detail.code == "io");
  CHECK(embedded.error().detail.message == "Cannot inspect confined input");
  CHECK(embedded.error().detail.limit.empty());
  CHECK(embedded.error().detail.maximum == 0);
  SECTION("missing selected declaration keeps the existing load-family diagnostic through the known-declaration adapter") {
    auto p = open_profile(f);
    REQUIRE(p.admit_root("workspace", f.workspace_dir.string()));
    auto absent = p.admit_file("declaration", missing.string(), admission_file_role::declaration);
    REQUIRE_FALSE(absent);
    // The raw admission diagnostic is pinned here; the known-load factory must adapt it.
    CHECK(absent.error().detail.code == "io");
    CHECK(absent.error().detail.message == "Cannot open confined input");
    CHECK(absent.error().field == "profile");
    auto adapted = known_declaration_error(absent.error().detail);
    CHECK(adapted.code == source_error_code::capture);
    CHECK(adapted.detail.code == "io");
    CHECK(adapted.detail.message == "Cannot inspect confined input");
    CHECK(adapted.detail.limit.empty());
    CHECK(adapted.detail.maximum == 0);
    CHECK(adapted.code == embedded.error().code);
    CHECK(adapted.detail.code == embedded.error().detail.code);
    CHECK(adapted.detail.message == embedded.error().detail.message);
    CHECK(adapted.detail.limit == embedded.error().detail.limit);
    CHECK(adapted.detail.maximum == embedded.error().detail.maximum);
  }
  SECTION("known declaration adapter passes other codes through unchanged") {
    auto denied = known_declaration_error(diagnostic{"path_denied", "Hard-link input alias"});
    CHECK(denied.code == source_error_code::capture);
    CHECK(denied.detail.code == "path_denied");
    CHECK(denied.detail.message == "Hard-link input alias");
    auto limited = known_declaration_error(diagnostic{"resource_limit", "Resource limit", "bytes", 16});
    CHECK(limited.code == source_error_code::resource_limit);
    CHECK(limited.detail.code == "resource_limit");
    CHECK(limited.detail.limit == "bytes");
    CHECK(limited.detail.maximum == 16);
  }
  SECTION("declaration removed after admission is an admitted-change refusal with zero content reads") {
    // Post-admission vanish is an admitted-change refusal recorded against X-CAP in REPORT.md.
    auto p = open_profile(f);
    auto r = admit(p, f, f.workspace, f.registry);
    watch(f.workspace);
    std::filesystem::remove(f.workspace);
    auto bound = source_capture::open_bound(f.workspace.string(), f.registry.string(), std::move(r.root),
                                            std::move(r.declaration), std::move(r.registry));
    REQUIRE_FALSE(bound);
    CHECK(bound.error().code == source_error_code::capture);
    CHECK(bound.error().detail.code == "capture_changed");
    CHECK(bound.error().detail.message == "Captured external profile changed");
    CHECK(planar_admission_read_attempts() == 0);
    CHECK_FALSE(p.verify());
  }
}

TEST_CASE("source capture terminal failure after capture refuses later operations", "[source_capture]") {
  fixture f;
  auto    source = source_capture::open_embedded(f.workspace.string(), f.registry.string());
  REQUIRE(source);
  auto w = source->workspace();
  REQUIRE(w);
  auto route = success("route_workspace", *w);
  std::filesystem::remove(f.registry);
  auto failed = source->capture(route);
  REQUIRE_FALSE(failed);
  CHECK(failed.error().code == source_error_code::capture);
  CHECK_FALSE(source->sealed_ready());
  auto again = source->capture(route);
  REQUIRE_FALSE(again);
  CHECK(again.error().code == source_error_code::invalid_state);
  auto reread = source->workspace();
  REQUIRE_FALSE(reread);
  CHECK(reread.error().code == source_error_code::invalid_state);
  auto verified = source->verify();
  REQUIRE_FALSE(verified);
  CHECK(verified.error().code == source_error_code::invalid_state);
}
