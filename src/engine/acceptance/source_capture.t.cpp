// Source capture receipt binding, admission ordering, retained seals, moves and terminal refusals.
#include <catch2/catch_test_macros.hpp>
#include <cerrno>
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
  SECTION("moved declaration receipt is refused as invalid_state") {
    auto p = open_profile(f);
    auto r = admit(p, f, f.workspace, f.registry);
    {
      retained_file_receipt sink = std::move(r.declaration);
    }
    auto source = source_capture::open_bound(f.workspace.string(), f.registry.string(), std::move(r.root),
                                             std::move(r.declaration), std::move(r.registry));
    REQUIRE_FALSE(source);
    CHECK(source.error().code == source_error_code::invalid_state);
    CHECK(source.error().detail.code.empty());
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
    CHECK(source.error().code == source_error_code::invalid_state);
    CHECK(source.error().detail.code.empty());
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
    CHECK(source.error().code == source_error_code::invalid_state);
    CHECK(source.error().detail.code.empty());
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
    CHECK(p.last_admission_refusal_was_absent());
    auto adapted = known_declaration_error(absent.error().detail, p.last_admission_refusal_was_absent());
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
    auto denied = known_declaration_error(diagnostic{"path_denied", "Hard-link input alias"}, false);
    CHECK(denied.code == source_error_code::capture);
    CHECK(denied.detail.code == "path_denied");
    CHECK(denied.detail.message == "Hard-link input alias");
    auto limited = known_declaration_error(diagnostic{"resource_limit", "Resource limit", "bytes", 16}, false);
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

namespace {
struct permission_restore {
  std::filesystem::path p;
  ~permission_restore() {
    std::error_code ec;
    std::filesystem::permissions(p, std::filesystem::perms::owner_all, std::filesystem::perm_options::replace, ec);
  }
};
} // namespace

TEST_CASE("source capture absence is a signal from admission, not a message match", "[source_capture]") {
  fixture f;
  SECTION("a genuinely missing declaration signals absence and adapts to the embedded diagnostic") {
    auto p      = open_profile(f);
    auto absent = f.workspace_dir / "missing.lua";
    REQUIRE(p.admit_root("workspace", f.workspace_dir.string()));
    auto missing = p.admit_file("declaration", absent.string(), admission_file_role::declaration);
    REQUIRE_FALSE(missing);
    CHECK(missing.error().detail.code == "io");
    CHECK(missing.error().detail.message == "Cannot open confined input");
    CHECK(p.last_admission_refusal_was_absent());
    auto adapted = known_declaration_error(missing.error().detail, p.last_admission_refusal_was_absent());
    CHECK(adapted.code == source_error_code::capture);
    CHECK(adapted.detail.code == "io");
    CHECK(adapted.detail.message == "Cannot inspect confined input");
    CHECK(adapted.detail.limit.empty());
    CHECK(adapted.detail.maximum == 0);
    // The same message without the signal is never reported as missing.
    auto unsignalled = known_declaration_error(missing.error().detail, false);
    CHECK(unsignalled.code == source_error_code::capture);
    CHECK(unsignalled.detail.code == "io");
    CHECK(unsignalled.detail.message == "Cannot open confined input");
  }
  SECTION("a present declaration in a permission-denied directory is not reported as missing") {
    if (::geteuid() == 0) {
      SUCCEED("root bypasses directory permissions");
    } else {
      auto locked = f.workspace_dir / "locked";
      std::filesystem::create_directories(locked);
      put_bytes(locked / "workspace.lua", "return {}");
      permission_restore guard{locked};
      std::filesystem::permissions(
          locked, std::filesystem::perms::owner_read | std::filesystem::perms::group_read | std::filesystem::perms::others_read,
          std::filesystem::perm_options::replace);
      struct stat probe{};
      errno = 0;
      if (::stat((locked / "workspace.lua").c_str(), &probe) == 0 || errno != EACCES) {
        SUCCEED("directory search is not denied on this platform");
      } else {
        auto p = open_profile(f);
        REQUIRE(p.admit_root("workspace", f.workspace_dir.string()));
        auto denied = p.admit_file("declaration", (locked / "workspace.lua").string(), admission_file_role::declaration);
        REQUIRE_FALSE(denied);
        CHECK(denied.error().detail.code == "io");
        CHECK(denied.error().detail.message == "Cannot open confined input");
        CHECK_FALSE(p.last_admission_refusal_was_absent());
        auto adapted = known_declaration_error(denied.error().detail, p.last_admission_refusal_was_absent());
        CHECK(adapted.code == source_error_code::capture);
        CHECK(adapted.detail.code == "io");
        CHECK(adapted.detail.message == "Cannot open confined input");
        CHECK(adapted.detail.message != "Cannot inspect confined input");
      }
    }
  }
  SECTION("admission refusal messages for these cases are byte identical to the original strings") {
    auto p = open_profile(f);
    REQUIRE(p.admit_root("workspace", f.workspace_dir.string()));
    auto missing = p.admit_file("declaration", (f.workspace_dir / "missing.lua").string(), admission_file_role::declaration);
    REQUIRE_FALSE(missing);
    CHECK(std::string_view(missing.error().detail.code) == "io");
    CHECK(std::string_view(missing.error().detail.message) == "Cannot open confined input");
    auto q       = open_profile(f);
    auto through = q.admit_file("declaration", (f.workspace / "child.lua").string(), admission_file_role::declaration);
    REQUIRE_FALSE(through);
    CHECK(std::string_view(through.error().detail.code) == "path_denied");
    CHECK(std::string_view(through.error().detail.message) == "Cannot open confined input");
    CHECK_FALSE(q.last_admission_refusal_was_absent());
  }
}

TEST_CASE("source capture N1 unreadable declaration is not reported as missing", "[source_capture]") {
  fixture            f;
  permission_restore guard{f.workspace};
  auto               p = open_profile(f);
  REQUIRE(p.admit_root("workspace", f.workspace_dir.string()));
  std::filesystem::permissions(f.workspace, std::filesystem::perms::none);
  auto admitted = p.admit_file("declaration", f.workspace.string(), admission_file_role::declaration);
  if (::geteuid() == 0) {
    CHECK(admitted.has_value());
  } else {
#ifdef __APPLE__
    // O_EVTONLY metadata open is permission checked: admission itself refuses with EACCES.
    REQUIRE_FALSE(admitted);
    CHECK(admitted.error().detail.code == "io");
    CHECK(admitted.error().detail.message == "Cannot open confined input");
    CHECK_FALSE(p.last_admission_refusal_was_absent());
    auto adapted = known_declaration_error(admitted.error().detail, p.last_admission_refusal_was_absent());
    CHECK(adapted.code == source_error_code::capture);
    CHECK(adapted.detail.code == "io");
    CHECK(adapted.detail.message == "Cannot open confined input");
#else
    // O_PATH admits the file; the failure belongs to the later content read, never to absence.
    REQUIRE(admitted.has_value());
    CHECK_FALSE(p.last_admission_refusal_was_absent());
#endif
  }
}

TEST_CASE("source capture N2 refuses swapped receipt roles", "[source_capture]") {
  fixture f;
  auto    p = open_profile(f);
  SECTION("registry receipt in the declaration position and vice versa") {
    auto r = admit(p, f, f.workspace, f.registry);
    watch(f.workspace);
    auto source = source_capture::open_bound(f.workspace.string(), f.registry.string(), std::move(r.root), std::move(r.registry),
                                             std::move(r.declaration));
    REQUIRE_FALSE(source);
    CHECK(source.error().code == source_error_code::capture);
    CHECK(source.error().detail.code == "capture_changed");
    CHECK(planar_admission_read_attempts() == 0);
    CHECK_FALSE(p.verify());
  }
  for (auto role : {admission_file_role::ledger, admission_file_role::observation}) {
    auto pp = open_profile(f);
    REQUIRE(pp.admit_root("workspace", f.workspace_dir.string()));
    REQUIRE(pp.admit_file("declaration", f.workspace.string(), admission_file_role::declaration));
    REQUIRE(pp.admit_file("registry", f.registry.string(), role));
    auto root = pp.retain_root("workspace");
    auto decl = pp.retain_file("declaration");
    auto reg  = pp.retain_file("registry");
    REQUIRE(root);
    REQUIRE(decl);
    REQUIRE(reg);
    watch(f.workspace);
    auto source = source_capture::open_bound(f.workspace.string(), f.registry.string(), std::move(*root), std::move(*decl),
                                             std::move(*reg));
    REQUIRE_FALSE(source);
    CHECK(source.error().code == source_error_code::capture);
    CHECK(source.error().detail.code == "capture_changed");
    CHECK(planar_admission_read_attempts() == 0);
    CHECK_FALSE(pp.verify());
  }
}

TEST_CASE("source capture N3 moved-from receipt is invalid_state but poisoned and foreign stay capture_changed",
          "[source_capture]") {
  fixture f;
  auto    p = open_profile(f);
  auto    r = admit(p, f, f.workspace, f.registry);
  watch(f.workspace);
  SECTION("moved-from declaration") {
    {
      retained_file_receipt sink = std::move(r.declaration);
    }
  }
  SECTION("moved-from registry") {
    {
      retained_file_receipt sink = std::move(r.registry);
    }
  }
  SECTION("moved-from root") {
    {
      retained_root_receipt sink = std::move(r.root);
    }
  }
  if (r.root.moved_from() || r.declaration.moved_from() || r.registry.moved_from()) {
    auto source = source_capture::open_bound(f.workspace.string(), f.registry.string(), std::move(r.root),
                                             std::move(r.declaration), std::move(r.registry));
    REQUIRE_FALSE(source);
    CHECK(source.error().code == source_error_code::invalid_state);
    CHECK(planar_admission_read_attempts() == 0);
    CHECK_FALSE(p.verify());
  }
  SECTION("poisoned declaration") {
    r.declaration.poison();
    auto source = source_capture::open_bound(f.workspace.string(), f.registry.string(), std::move(r.root),
                                             std::move(r.declaration), std::move(r.registry));
    REQUIRE_FALSE(source);
    CHECK(source.error().code == source_error_code::capture);
    CHECK(source.error().detail.code == "capture_changed");
  }
  SECTION("receipts from two different admissions") {
    auto p2     = open_profile(f);
    auto r2     = admit(p2, f, f.workspace, f.registry);
    auto source = source_capture::open_bound(f.workspace.string(), f.registry.string(), std::move(r.root),
                                             std::move(r2.declaration), std::move(r.registry));
    REQUIRE_FALSE(source);
    CHECK(source.error().code == source_error_code::capture);
    CHECK(source.error().detail.code == "capture_changed");
  }
}

TEST_CASE("source capture receipt accessors report moved_from and role", "[source_capture]") {
  fixture f;
  auto    p = open_profile(f);
  auto    r = admit(p, f, f.workspace, f.registry);
  CHECK_FALSE(r.root.moved_from());
  CHECK_FALSE(r.declaration.moved_from());
  CHECK(r.declaration.role() == admission_file_role::declaration);
  CHECK(r.registry.role() == admission_file_role::registry);
  r.declaration.poison();
  CHECK_FALSE(r.declaration.live());
  CHECK_FALSE(r.declaration.moved_from());
  CHECK(r.declaration.role() == admission_file_role::declaration);
  CHECK_FALSE(r.root.live());
  CHECK_FALSE(r.root.moved_from());
  retained_file_receipt moved = std::move(r.declaration);
  CHECK(r.declaration.moved_from());
  CHECK_FALSE(r.declaration.role().has_value());
  CHECK(moved.role() == admission_file_role::declaration);
  retained_root_receipt moved_root = std::move(r.root);
  CHECK(r.root.moved_from());
  CHECK_FALSE(moved_root.moved_from());
}

namespace {
value_ptr change(const value_ptr& v, std::string_view key, value_ptr replacement) {
  auto c                      = std::make_shared<value>(*v);
  c->fields[std::string(key)] = std::move(replacement);
  return c;
}
value_ptr text_value(std::string s) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = std::move(s);
  return v;
}
value_ptr list_value(std::vector<value_ptr> items) {
  auto v   = std::make_shared<value>();
  v->kind  = value_kind::list;
  v->items = std::move(items);
  return v;
}
bool is_text_reference(const value_ptr& v) {
  auto k = v->fields.find("kind");
  return v->fields.contains("file") && v->fields.contains("bytes") && k != v->fields.end() && k->second &&
         k->second->string == "text";
}
void text_reference_paths(const value_ptr& v, std::set<std::string>& out) {
  if (!v)
    return;
  if (is_text_reference(v))
    out.insert(v->fields.at("file")->fields.at("path")->string);
  for (auto& [k, c] : v->fields)
    text_reference_paths(c, out);
  for (auto& c : v->items)
    text_reference_paths(c, out);
}
// Rewrites the first text reference found (depth first) through the supplied function; returns the rebuilt value.
value_ptr rewrite_first_reference(const value_ptr& v, const std::function<value_ptr(const value_ptr&)>& fn, bool& done) {
  if (!v || done)
    return v;
  if (is_text_reference(v)) {
    done = true;
    return fn(v);
  }
  auto c = std::make_shared<value>(*v);
  for (auto& [k, child] : c->fields)
    child = rewrite_first_reference(child, fn, done);
  for (auto& child : c->items)
    child = rewrite_first_reference(child, fn, done);
  return c;
}
source_capture open_embedded_fixture(const fixture& f) {
  auto source = source_capture::open_embedded(f.workspace.string(), f.registry.string());
  REQUIRE(source);
  return std::move(*source);
}
value_ptr interpreted_bundle(source_capture& source) {
  auto b = success("interpret_bundle", captured_bundle(source));
  return b->fields.at("bundle");
}
} // namespace

TEST_CASE("source capture correspondence returns the exact sorted workspace paths for a good bundle", "[source_capture]") {
  fixture f;
  auto    source = open_embedded_fixture(f);
  auto    bundle = interpreted_bundle(source);
  auto    paths  = verify_load_correspondence(bundle, source);
  REQUIRE(paths);
  CHECK(std::ranges::is_sorted(*paths));
  CHECK(std::ranges::adjacent_find(*paths) == paths->end());
  std::vector<std::string> expected;
  for (auto& entry : std::filesystem::recursive_directory_iterator(f.workspace_dir))
    if (entry.is_regular_file())
      expected.push_back(std::filesystem::relative(entry.path(), f.root).generic_string());
  // Workspace-root snapshot paths are relative to the workspace directory; the registry is excluded.
  for (auto& e : expected)
    e = std::filesystem::path(e).lexically_relative("workspace").generic_string();
  std::ranges::sort(expected);
  CHECK(*paths == expected);
  CHECK(paths->size() > 1);
  CHECK(source.sealed_ready());
  // Correspondence is read-only: repeating it gives the same answer.
  auto again = verify_load_correspondence(bundle, source);
  REQUIRE(again);
  CHECK(*again == *paths);
}

TEST_CASE("source capture correspondence refuses every single bundle tamper as mismatch", "[source_capture]") {
  fixture f;
  auto    source = open_embedded_fixture(f);
  auto    bundle = interpreted_bundle(source);
  REQUIRE(verify_load_correspondence(bundle, source));
  auto w    = bundle->fields.at("workspace");
  auto jobs = bundle->fields.at("jobs")->items;
  REQUIRE(jobs.size() >= 2);
  std::set<std::string> referenced;
  text_reference_paths(bundle, referenced);
  REQUIRE_FALSE(referenced.empty());
  auto paths = verify_load_correspondence(bundle, source);
  REQUIRE(paths);
  std::vector<std::pair<std::string, value_ptr>> tampers;
  tampers.emplace_back("workspace_file", change(bundle, "workspace_file", text_value("other.lua")));
  tampers.emplace_back("spec_file", change(bundle, "spec_file", text_value("other.lua")));
  tampers.emplace_back("registry_file", change(bundle, "registry_file", text_value("other.json")));
  tampers.emplace_back("workspace id", change(bundle, "workspace", change(w, "id", text_value("other"))));
  tampers.emplace_back("spec id", change(bundle, "spec", change(bundle->fields.at("spec"), "id", text_value("other"))));
  {
    auto wj = w->fields.at("jobs")->items;
    wj[0]   = change(wj[0], "id", text_value("zzz-other"));
    tampers.emplace_back("workspace route job id", change(bundle, "workspace", change(w, "jobs", list_value(wj))));
  }
  {
    auto j = jobs;
    j[0]   = change(j[0], "file", text_value("wrong.lua"));
    tampers.emplace_back("bundle job file", change(bundle, "jobs", list_value(j)));
  }
  {
    auto j = jobs;
    j[0]   = change(j[0], "value", change(j[0]->fields.at("value"), "id", text_value("other")));
    tampers.emplace_back("job value id", change(bundle, "jobs", list_value(j)));
  }
  {
    auto j = jobs;
    j[0]   = change(j[0], "value", change(j[0]->fields.at("value"), "version", text_value("9.9.9")));
    tampers.emplace_back("job value version", change(bundle, "jobs", list_value(j)));
  }
  {
    auto j = jobs;
    j.pop_back();
    tampers.emplace_back("missing job", change(bundle, "jobs", list_value(j)));
  }
  tampers.emplace_back("registry revision",
                       change(bundle, "registry", change(bundle->fields.at("registry"), "revision", text_value("other"))));
  tampers.emplace_back("registry id",
                       change(bundle, "registry", change(bundle->fields.at("registry"), "id", text_value("other"))));
  {
    // Swapped files keep every route valid, distinct and sorted: only equality with the captured route table refuses.
    auto wj = w->fields.at("jobs")->items;
    REQUIRE(wj.size() >= 2);
    const auto first_file = wj[0]->fields.at("file")->string;
    wj[0]                 = change(wj[0], "file", wj[1]->fields.at("file"));
    wj[1]                 = change(wj[1], "file", text_value(first_file));
    tampers.emplace_back("workspace route table swapped files", change(bundle, "workspace", change(w, "jobs", list_value(wj))));
  }
  {
    // The same routes in reverse order build an equal id to file table: only the sorted-route requirement refuses.
    auto wj = w->fields.at("jobs")->items;
    std::ranges::reverse(wj);
    tampers.emplace_back("workspace routes unsorted", change(bundle, "workspace", change(w, "jobs", list_value(wj))));
  }
  {
    auto root = w->fields.at("root");
    tampers.emplace_back("workspace root id",
                         change(bundle, "workspace", change(w, "root", change(root, "id", text_value("other")))));
    tampers.emplace_back("workspace root base",
                         change(bundle, "workspace", change(w, "root", change(root, "base", text_value("other")))));
  }
  tampers.emplace_back("workspace spec reference", change(bundle, "workspace", change(w, "spec", text_value("other.lua"))));
  // An extra registry header field leaves the registry id and revision agreeing with the bundle: only the whole
  // header comparison against the declaration refuses.
  tampers.emplace_back(
      "workspace registry header",
      change(bundle, "workspace", change(w, "registry", change(w->fields.at("registry"), "extra", text_value("x")))));
  {
    bool done    = false;
    auto changed = rewrite_first_reference(
        bundle, [](const value_ptr& r) { return change(r, "bytes", text_value(r->fields.at("bytes")->string + "x")); }, done);
    REQUIRE(done);
    tampers.emplace_back("reference bytes", changed);
  }
  {
    bool done    = false;
    auto removed = rewrite_first_reference(
        bundle,
        [](const value_ptr&) {
          auto empty  = std::make_shared<value>();
          empty->kind = value_kind::record;
          return empty;
        },
        done);
    REQUIRE(done);
    tampers.emplace_back("missing reference", removed);
  }
  {
    // An extra reference to a real captured file the declarations never reference.
    auto snapshot = source.sealed()->snapshot();
    REQUIRE(snapshot);
    std::string unused_path, unused_bytes;
    for (auto& file : **snapshot)
      if (file.root == "workspace" && !referenced.contains(file.path)) {
        unused_path  = file.path;
        unused_bytes = file.bytes;
        break;
      }
    REQUIRE_FALSE(unused_path.empty());
    auto file_record            = std::make_shared<value>();
    file_record->kind           = value_kind::record;
    file_record->fields["root"] = text_value("workspace");
    file_record->fields["path"] = text_value(unused_path);
    auto ref                    = std::make_shared<value>();
    ref->kind                   = value_kind::record;
    ref->fields["kind"]         = text_value("text");
    ref->fields["file"]         = file_record;
    ref->fields["bytes"]        = text_value(unused_bytes);
    tampers.emplace_back("extra reference", change(bundle, "extra", ref));
  }
  for (auto& [name, tampered] : tampers) {
    INFO(name);
    auto r = verify_load_correspondence(tampered, source);
    REQUIRE_FALSE(r);
    CHECK(r.error().code == correspondence_error_code::mismatch);
    CHECK(r.error().detail.code.empty());
  }
  // The source is untouched by refusals: the untampered bundle still corresponds.
  auto good = verify_load_correspondence(bundle, source);
  REQUIRE(good);
  CHECK(*good == *paths);
}

TEST_CASE("source capture correspondence refuses an unsealed or released source as mismatch", "[source_capture]") {
  fixture f;
  SECTION("opened but never captured") {
    auto source = open_embedded_fixture(f);
    auto r      = verify_load_correspondence(std::make_shared<value>(), source);
    REQUIRE_FALSE(r);
    CHECK(r.error().code == correspondence_error_code::mismatch);
  }
  SECTION("sealed capture released to preparation") {
    auto source = open_embedded_fixture(f);
    auto bundle = interpreted_bundle(source);
    REQUIRE(verify_load_correspondence(bundle, source));
    auto released = source.release_sealed();
    REQUIRE(released);
    auto r = verify_load_correspondence(bundle, source);
    REQUIRE_FALSE(r);
    CHECK(r.error().code == correspondence_error_code::mismatch);
  }
}
