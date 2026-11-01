// Direct tests of the internal observation directory seam: bounds, ordering and the recheck (RA01-B2 wave 2).
module;
#include <catch2/catch_test_macros.hpp>
#include <sys/stat.h>
#include <unistd.h>
module planar.engine.capture.directory;
import std;
import :recheck;
namespace {
namespace fs = std::filesystem;
using namespace planar::engine;

/// Per-test temporary observation directory.
struct observation_tree {
  fs::path root;
  observation_tree() {
    char pattern[] = "/tmp/planar-ra01-dir-XXXXXX";
    REQUIRE(::mkdtemp(pattern));
    root = fs::canonical(pattern);
  }
  observation_tree(const observation_tree&)            = delete;
  observation_tree& operator=(const observation_tree&) = delete;
  ~observation_tree() {
    std::error_code ignored;
    fs::remove_all(root, ignored);
  }
  void put(const std::string& relative, std::string_view bytes) const {
    const auto path = root / relative;
    fs::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << bytes;
    REQUIRE(out.good());
  }
  std::expected<observation_directory, diagnostic> read(std::function<void()> hook = {}) const {
    return recheck_detail::read_observation_directory_impl(root.string(), std::move(hook));
  }
};
std::string numbered(std::string_view prefix, int index) {
  return std::format("{}{:04}.json", prefix, index);
}
/// Whole text of a product source file, read by the source-guard tests.
std::string read_source(const fs::path& path) {
  std::ifstream in(path, std::ios::binary);
  return std::string{std::istreambuf_iterator<char>(in), {}};
}
/// Whitespace-stripped text: the spelling every source guard compares against.
std::string squash_space(std::string_view text) {
  std::string out;
  for (const char c : text)
    if (!std::isspace(static_cast<unsigned char>(c)))
      out += c;
  return out;
}
/// Number of non-overlapping occurrences of needle in text.
std::size_t occurrences(std::string_view text, std::string_view needle) {
  std::size_t n = 0;
  for (auto at = text.find(needle); at != std::string_view::npos; at = text.find(needle, at + needle.size()))
    ++n;
  return n;
}
bool has_text(std::string_view text, std::string_view needle) {
  return text.find(needle) != std::string_view::npos;
}
/// Restores a path's permission bits on scope exit so the temporary tree can be removed.
struct mode_guard {
  fs::path  path;
  fs::perms restore;
  ~mode_guard() {
    std::error_code ignored;
    fs::permissions(path, restore, fs::perm_options::replace, ignored);
  }
};
} // namespace

TEST_CASE("RA01-B2 observe bounds listed names at 1024 counting dot names", "[capture][ra01]") {
  // Mutants: bound >= 1024 (the 1024 cases refuse); counter reset per listing (the split 1025+ case is accepted);
  // dot names or subdirectories excluded from the count (the dot case is accepted).
  SECTION("1024 names in one listing, two of them the subdirectories") {
    observation_tree t;
    for (int i = 0; i < 1022; ++i)
      t.put("checks/" + numbered("c", i), "{}");
    fs::create_directory(t.root / "findings");
    const auto accepted = t.read();
    REQUIRE(accepted);
    CHECK(accepted->entries.size() == 1022);
    t.put("checks/" + numbered("c", 1022), "{}");
    const auto refused = t.read();
    REQUIRE_FALSE(refused);
    CHECK(refused.error().code == "resource_limit");
    CHECK(refused.error().limit == "observation_entries");
    CHECK(refused.error().maximum == 1024);
    CHECK(refused.error().message == "Capture limit exceeded");
  }
  SECTION("dot names count toward the bound") {
    observation_tree t;
    fs::create_directory(t.root / "checks");
    fs::create_directory(t.root / "findings");
    for (int i = 0; i < 10; ++i)
      t.put(std::format(".dot{}", i), "ignored");
    for (int i = 0; i < 1012; ++i)
      t.put("checks/" + numbered("c", i), "{}");
    const auto accepted = t.read();
    REQUIRE(accepted);
    CHECK(accepted->entries.size() == 1012);
    t.put(".one-more", "ignored");
    const auto refused = t.read();
    REQUIRE_FALSE(refused);
    CHECK(refused.error().limit == "observation_entries");
    CHECK(refused.error().maximum == 1024);
  }
  SECTION("the counter is shared across root, checks and findings") {
    observation_tree t;
    for (int i = 0; i < 511; ++i)
      t.put("checks/" + numbered("c", i), "{}");
    for (int i = 0; i < 511; ++i)
      t.put("findings/" + numbered("f", i), "{}");
    const auto accepted = t.read();
    REQUIRE(accepted);
    CHECK(accepted->entries.size() == 1022);
    // 2 + 513 + 512 = 1027 names in total while each single listing stays below 1024.
    t.put("checks/" + numbered("c", 511), "{}");
    t.put("checks/" + numbered("c", 512), "{}");
    t.put("findings/" + numbered("f", 511), "{}");
    const auto refused = t.read();
    REQUIRE_FALSE(refused);
    CHECK(refused.error().limit == "observation_entries");
    CHECK(refused.error().maximum == 1024);
  }
}

TEST_CASE("RA01-B2 observe refuses an oversize file as observation_file_bytes", "[capture][ra01]") {
  // Mutants: per-file limit 65537 (the 65537 case is accepted); size checked after reading (not observable here, the
  // boundary pair still pins the number).
  observation_tree t;
  t.put("checks/a.json", std::string(65536, 'x'));
  const auto accepted = t.read();
  REQUIRE(accepted);
  REQUIRE(accepted->entries.size() == 1);
  CHECK(accepted->entries[0].size == 65536);
  CHECK(accepted->entries[0].bytes.size() == 65536);
  t.put("checks/a.json", std::string(65537, 'x'));
  const auto refused = t.read();
  REQUIRE_FALSE(refused);
  CHECK(refused.error().code == "resource_limit");
  CHECK(refused.error().limit == "observation_file_bytes");
  CHECK(refused.error().maximum == 65536);
  CHECK(refused.error().message == "Capture limit exceeded");
}

TEST_CASE("RA01-B2 observe refuses more than four MiB as observation_total_bytes", "[capture][ra01]") {
  // Mutant: the running total check deleted (the one-byte-over case is accepted); a >= comparison (the exact case
  // is refused).
  observation_tree t;
  for (int i = 0; i < 64; ++i)
    t.put("checks/" + numbered("c", i), std::string(65536, 'x'));
  const auto accepted = t.read();
  REQUIRE(accepted);
  CHECK(accepted->entries.size() == 64);
  t.put("checks/" + numbered("c", 64), "x");
  const auto refused = t.read();
  REQUIRE_FALSE(refused);
  CHECK(refused.error().code == "resource_limit");
  CHECK(refused.error().limit == "observation_total_bytes");
  CHECK(refused.error().maximum == 4194304);
  CHECK(refused.error().message == "Capture limit exceeded");
}

TEST_CASE("RA01-B2 observe reports an unsafe entry before a later size limit", "[capture][ra01]") {
  // Mutant: a size pre-pass over all entries before the kind checks (the later oversize file reports resource_limit).
  SECTION("per-file limit") {
    observation_tree t;
    fs::create_directories(t.root / "checks");
    t.put("checks/b.json", std::string(65537, 'x'));
    fs::create_symlink("b.json", t.root / "checks/a.json");
    const auto refused = t.read();
    REQUIRE_FALSE(refused);
    CHECK(refused.error().code == "path_denied");
    CHECK(refused.error().message == "Links and special files forbidden");
  }
  SECTION("total limit") {
    observation_tree t;
    fs::create_directories(t.root / "checks");
    fs::create_symlink("b000.json", t.root / "checks/a.json");
    for (int i = 0; i < 65; ++i)
      t.put("checks/" + numbered("b", i), std::string(65536, 'x'));
    const auto refused = t.read();
    REQUIRE_FALSE(refused);
    CHECK(refused.error().code == "path_denied");
    CHECK(refused.error().message == "Links and special files forbidden");
  }
}

TEST_CASE("RA01-B2 observe lists entries sorted by category and name", "[capture][ra01]") {
  // Mutant: the std::ranges::sort of the listing removed (observable where readdir is unsorted, so the prober applies
  // it in the Linux container); category order checkpoint, check, finding replaced by name order.
  observation_tree t;
  // Created in reverse of the expected order; unsigned byte order puts "Z" before "a" (a name that case-insensitive APFS cannot
  // confuse with another entry) and the UTF-8 name last.
  t.put("findings/z.json", "{\"z\":1}");
  t.put("findings/a.json", "{\"a\":1}");
  t.put("checks/\xC3\xA9.json", "{}");
  t.put("checks/b.json", "{\"b\":1}");
  t.put("checks/a.json", "{\"a\":1}");
  t.put("checks/Z.json", "{\"Z\":1}");
  t.put("checkpoint.json", "{\"cp\":1}");
  const auto result = t.read();
  REQUIRE(result);
  std::vector<std::string> names, categories;
  for (const auto& entry : result->entries) {
    names.push_back(entry.name);
    categories.push_back(entry.category);
  }
  const std::vector<std::string> expected_names{"checkpoint.json",      "checks/Z.json",   "checks/a.json",  "checks/b.json",
                                                "checks/\xC3\xA9.json", "findings/a.json", "findings/z.json"};
  const std::vector<std::string> expected_categories{"checkpoint", "check", "check", "check", "check", "finding", "finding"};
  CHECK(names == expected_names);
  CHECK(categories == expected_categories);
  for (const auto& entry : result->entries) {
    CAPTURE(entry.name);
    struct stat status{};
    REQUIRE(::stat((t.root / entry.name).c_str(), &status) == 0);
    CHECK(entry.size == entry.bytes.size());
    CHECK(entry.inode == static_cast<std::uint64_t>(status.st_ino));
    CHECK(entry.device == static_cast<std::uint64_t>(status.st_dev));
  }
  CHECK(result->entries[0].bytes == "{\"cp\":1}");
  CHECK(result->entries[2].bytes == "{\"a\":1}");
}

TEST_CASE("RA01-B2 observe refuses a directory changed before the recheck", "[capture][ra01]") {
  // Mutant: skip step 4 (return after the first read): every case is then accepted. Also pins no retry (hook count 1).
  enum class change { overwrite, replace_inode, add_file, remove_file };
  const auto name_of = [](change c) {
    switch (c) {
    case change::overwrite:
      return "overwrite";
    case change::replace_inode:
      return "replace_inode";
    case change::add_file:
      return "add_file";
    case change::remove_file:
      return "remove_file";
    }
    return "";
  };
  SECTION("unchanged control") {
    observation_tree t;
    t.put("checks/a.json", "old");
    int        calls  = 0;
    const auto result = t.read([&] { ++calls; });
    REQUIRE(result);
    CHECK(calls == 1);
    CHECK(result->entries.size() == 1);
  }
  for (const auto kind : {change::overwrite, change::replace_inode, change::add_file, change::remove_file}) {
    SECTION(name_of(kind)) {
      observation_tree t;
      t.put("checks/a.json", "old");
      t.put("findings/f.json", "keep");
      int        calls  = 0;
      const auto result = t.read([&] {
        ++calls;
        switch (kind) {
        case change::overwrite:
          t.put("checks/a.json", "new");
          break;
        case change::replace_inode:
          t.put("checks/tmp-source", "old");
          fs::rename(t.root / "checks/tmp-source", t.root / "checks/a.json");
          break;
        case change::add_file:
          t.put("checks/b.json", "added");
          break;
        case change::remove_file:
          fs::remove(t.root / "checks/a.json");
          break;
        }
      });
      REQUIRE_FALSE(result);
      CHECK(calls == 1);
      CHECK(result.error().code == "capture_changed");
      if (kind == change::overwrite)
        CHECK(result.error().message == "File changed during read");
      else
        CHECK(result.error().message == "Directory membership or identity changed");
    }
  }
}

TEST_CASE("RA01-B2 C01 observe maps an unreadable entry to cannot inspect", "[capture][ra01]") {
  // Kills C01-01 (scan_directory child openat errno mapped to denied_name), C01-02 (read_file openat mapped to denied_kind
  // for every errno) and C01-03 (observation root open mapped to denied_kind). Root-aware: as root, mode-000 entries still
  // open, so each section records SUCCEED and this case does not exercise the refusal; macOS (non-root) does.
  SECTION("unreadable checks directory") {
    if (::geteuid() == 0) {
      SUCCEED("permission denial is not observable as root");
    } else {
      observation_tree t;
      t.put("checks/a.json", "{}");
      mode_guard restore{t.root / "checks", fs::perms::owner_all};
      fs::permissions(t.root / "checks", fs::perms::none, fs::perm_options::replace);
      const auto result = t.read();
      REQUIRE_FALSE(result);
      CHECK(result.error().code == "io");
      CHECK(result.error().message == "Cannot inspect confined input");
      CHECK(result.error().limit.empty());
    }
  }
  SECTION("unreadable check file") {
    if (::geteuid() == 0) {
      SUCCEED("permission denial is not observable as root");
    } else {
      observation_tree t;
      t.put("checks/a.json", "{}");
      mode_guard restore{t.root / "checks/a.json", fs::perms::owner_read | fs::perms::owner_write};
      fs::permissions(t.root / "checks/a.json", fs::perms::none, fs::perm_options::replace);
      const auto result = t.read();
      REQUIRE_FALSE(result);
      CHECK(result.error().code == "io");
      CHECK(result.error().message == "Cannot inspect confined input");
      CHECK(result.error().limit.empty());
    }
  }
  SECTION("unreadable root") {
    if (::geteuid() == 0) {
      SUCCEED("permission denial is not observable as root");
    } else {
      observation_tree t;
      t.put("checks/a.json", "{}");
      mode_guard restore{t.root, fs::perms::owner_all};
      fs::permissions(t.root, fs::perms::none, fs::perm_options::replace);
      const auto result = t.read();
      REQUIRE_FALSE(result);
      CHECK(result.error().code == "io");
      CHECK(result.error().message == "Cannot inspect confined input");
      CHECK(result.error().limit.empty());
    }
  }
}

TEST_CASE("RA01-B2 C01 observe maps a failed entry inspection after listing to cannot inspect", "[capture][ra01]") {
  // Guards list()'s openat(dir, ".") failure on a readable but unsearchable directory (cannot_inspect must not become
  // denied_name; mutant E02c). It does not reach the later fstatat (E02, E02b), which needs a race or fault seam: a stated
  // limit. Root-aware: as root the permission bits do not deny, so each section records SUCCEED.
  SECTION("read-only checks directory: listed but not searchable") {
    if (::geteuid() == 0) {
      SUCCEED("permission denial is not observable as root");
    } else {
      observation_tree t;
      t.put("checks/a.json", "{}");
      mode_guard restore{t.root / "checks", fs::perms::owner_all};
      fs::permissions(t.root / "checks", fs::perms::owner_read, fs::perm_options::replace);
      const auto result = t.read();
      REQUIRE_FALSE(result);
      CHECK(result.error().code == "io");
      CHECK(result.error().message == "Cannot inspect confined input");
      CHECK(result.error().limit.empty());
    }
  }
  SECTION("read-only root: listed but not searchable") {
    if (::geteuid() == 0) {
      SUCCEED("permission denial is not observable as root");
    } else {
      observation_tree t;
      t.put("checks/a.json", "{}");
      mode_guard restore{t.root, fs::perms::owner_all};
      fs::permissions(t.root, fs::perms::owner_read, fs::perm_options::replace);
      const auto result = t.read();
      REQUIRE_FALSE(result);
      CHECK(result.error().code == "io");
      CHECK(result.error().message == "Cannot inspect confined input");
      CHECK(result.error().limit.empty());
    }
  }
}

TEST_CASE("RA01-B2 C01 observe refuses a symlink root", "[capture][ra01]") {
  // Kills C01-05: dropping O_NOFOLLOW from the observation root open follows the link and reads the populated target.
  // The pair is errno-independent (ELOOP on macOS, ENOTDIR on Linux), so the section asserts only code and message.
  observation_tree t;
  t.put("real/checks/a.json", "{}");
  fs::create_symlink("real", t.root / "link");
  SECTION("symlink to a populated directory is refused") {
    const auto result = recheck_detail::read_observation_directory_impl((t.root / "link").string(), {});
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "io");
    CHECK(result.error().message == "Cannot inspect confined input");
  }
  SECTION("control: the real directory reads") {
    const auto result = recheck_detail::read_observation_directory_impl((t.root / "real").string(), {});
    REQUIRE(result);
    CHECK(result->entries.size() == 1);
  }
}

TEST_CASE("RA01-B2 C01 observe bad_alloc refusal carries observation_total_bytes", "[capture][ra01]") {
  // Kills C01-06: the scan_once bad_alloc catch returning the two-argument diagnostic again (no limit name, no maximum).
  // SOURCE GUARD with a stated limit: no allocator fault hook exists and none may be added, so the handler is pinned by
  // its spelling (the anchored catch, the unique limit with the private allocation flag, and no bare two-argument pair).
  const auto text = squash_space(read_source(fs::path(__FILE__).parent_path() / "directory.cpp"));
  REQUIRE_FALSE(text.empty());
  CHECK(
      occurrences(
          text,
          "catch(conststd::bad_alloc&){returnstd::unexpected(scan_error{limit(\"observation_total_bytes\",max_total),true});}") ==
      1);
  CHECK(occurrences(text, "limit(\"observation_total_bytes\",max_total),true}") == 1);
  CHECK_FALSE(has_text(text, "diagnostic{\"resource_limit\",\"Capturelimitexceeded\"}"));
}

TEST_CASE("RA01-B2 C01 observe recheck bad_alloc is a limit and not a membership change", "[capture][ra01]") {
  // Kills C01-38: the second-scan failure branch mapping every failure back to membership (the carve-out removed).
  // SOURCE GUARD with a stated limit: no allocator fault hook exists and none may be added. The private scan_error flag is
  // read by name, and the unconditional membership return is absent. Behaviour that a legitimate second-scan limit stays
  // membership changed is pinned by the behavioural test RA01-B2 C01 observe recheck growth past the total bound.
  const auto text = squash_space(read_source(fs::path(__FILE__).parent_path() / "directory.cpp"));
  REQUIRE_FALSE(text.empty());
  CHECK(has_text(text, "limit(\"observation_total_bytes\",max_total),true}"));
  CHECK(has_text(text, "second.error().allocation"));
  CHECK_FALSE(has_text(text, "if(!second)returnstd::unexpected(membership)"));
}

TEST_CASE("RA01-B2 C01 observe recheck growth past the total bound stays membership changed", "[capture][ra01]") {
  // Kills C01-39: a carve-out decided by the limit name (second.error().error.limit == "observation_total_bytes") instead of
  // the private allocation flag. 64 files of 65536 bytes make a 4194304-byte first scan that passes; the hook then adds a
  // one-byte file, so the second scan fails with observation_total_bytes for a non-allocation reason and must stay
  // capture_changed. Passes on the current product.
  observation_tree t;
  for (int i = 0; i < 64; ++i)
    t.put("checks/" + std::format("f{:03}.json", i), std::string(65536, 'x'));
  int        calls  = 0;
  const auto result = t.read([&] {
    ++calls;
    t.put("checks/z.json", "x");
  });
  CHECK(calls == 1);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == "capture_changed");
  CHECK(result.error().message == "Directory membership or identity changed");
  CHECK(result.error().limit.empty());
}

TEST_CASE("RA01-B2 C01 observe applies precedence in sorted name order", "[capture][ra01]") {
  // Kills C01-07: deleting the std::ranges::sort of list() leaves readdir order. Fixture A (names a000, a001, z000..z059)
  // and fixture B (names b000, b001, y000..y059) have DISJOINT name sets. Each fixture is one fixed readdir order per
  // platform (APFS and ext4 htree return a fixed order for a given name set), so the mutant is detected by the fixed order
  // of each name set, not by a probability; a surviving macOS run is recorded by the prober, which applies the mutant in
  // the Linux container. Kills C01-08: a reversed sort lists z* and y* first and fails both sections deterministically.
  SECTION("fixture A: unsafe symlink a000 precedes oversize names") {
    observation_tree t;
    fs::create_directories(t.root / "checks");
    fs::create_symlink("x", t.root / "checks/a000.json");
    t.put("checks/a001.json", std::string(65537, 'x'));
    for (int i = 0; i < 60; ++i)
      t.put("checks/" + std::format("z{:03}.json", i), std::string(65537, 'x'));
    const auto result = t.read();
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "path_denied");
    CHECK(result.error().message == "Links and special files forbidden");
  }
  SECTION("fixture B: oversize b000 precedes unsafe symlinks") {
    observation_tree t;
    fs::create_directories(t.root / "checks");
    t.put("checks/b000.json", std::string(65537, 'x'));
    fs::create_symlink("x", t.root / "checks/b001.json");
    for (int i = 0; i < 60; ++i)
      fs::create_symlink("x", t.root / "checks" / std::format("y{:03}.json", i));
    const auto result = t.read();
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "resource_limit");
    CHECK(result.error().limit == "observation_file_bytes");
    CHECK(result.error().maximum == 65536);
    CHECK(result.error().message == "Capture limit exceeded");
  }
}
