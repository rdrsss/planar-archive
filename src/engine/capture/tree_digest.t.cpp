// Check capture skips only .git directories and otherwise applies render path and kind refusals.
#include <catch2/catch_test_macros.hpp>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.engine.capture.tree_digest;
using namespace planar::engine::capture;
namespace {
struct temporary_tree {
  std::filesystem::path path;
  int                   fd;
  temporary_tree() {
    char name[] = "/tmp/planar-tree-digest-XXXXXX";
    path        = ::mkdtemp(name);
    fd          = ::open(path.c_str(), O_RDONLY | O_DIRECTORY);
  }
  ~temporary_tree() {
    ::close(fd);
    std::error_code ec;
    std::filesystem::remove_all(path, ec);
  }
};
} // namespace
TEST_CASE("C01 RT3 git pointer files are refused like render capture") {
  temporary_tree t;
  std::ofstream(t.path / ".git") << "gitdir: elsewhere\n";
  auto captured = capture_tree_digest(t.fd, {});
  REQUIRE_FALSE(captured);
  REQUIRE(captured.error().code == "path_denied");
}
TEST_CASE("C01 RT3 git directories alone receive implicit exclusion") {
  temporary_tree t;
  std::filesystem::create_directory(t.path / ".git");
  std::filesystem::create_symlink("missing", t.path / ".git" / "ignored");
  auto captured = capture_tree_digest(t.fd, {});
  REQUIRE(captured);
  REQUIRE(captured->entries.empty());
}
TEST_CASE("C01 RT3 git links and special files are refused") {
  temporary_tree t;
  SECTION("dangling symlink") {
    std::filesystem::create_symlink("missing", t.path / ".git");
  }
  SECTION("directory symlink") {
    std::filesystem::create_directory(t.path / "real");
    std::filesystem::create_directory_symlink("real", t.path / ".git");
  }
  SECTION("fifo") {
    REQUIRE(::mkfifo((t.path / ".git").c_str(), 0600) == 0);
  }
  auto captured = capture_tree_digest(t.fd, {});
  REQUIRE_FALSE(captured);
  REQUIRE(captured.error().code == "path_denied");
}

TEST_CASE("C01 RT3 mixed case git names retain render path refusal") {
  temporary_tree t;
  SECTION("file") {
    std::ofstream(t.path / ".GiT") << "pointer";
  }
  SECTION("directory") {
    std::filesystem::create_directory(t.path / ".GiT");
  }
  auto captured = capture_tree_digest(t.fd, {});
  REQUIRE_FALSE(captured);
  REQUIRE(captured.error().code == "path_denied");
}
