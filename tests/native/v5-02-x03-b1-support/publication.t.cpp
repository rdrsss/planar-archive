// Deterministic publication faults run the mechanically generated production algorithm in an isolated test module.
#include <catch2/catch_test_macros.hpp>
#include <unistd.h>
import std;
import planar.test.ledger_format;
import planar.test.ledger_posix;
using namespace planar::engine::ledger_fault;
namespace shim = planar::test::ledger_posix;
namespace {
struct scratch {
  std::filesystem::path root;
  scratch() {
    char pattern[] = "/tmp/planar-publication-XXXXXX";
    REQUIRE(::mkdtemp(pattern));
    root = std::filesystem::canonical(pattern);
  }
  ~scratch() {
    shim::reset(shim::fault::none, {});
    std::filesystem::remove_all(root);
  }
};
std::string bytes(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(in), {}};
}
void valid(const std::filesystem::path& path, const ledger_metadata& expected) {
  auto reopened = open_existing(path);
  REQUIRE(reopened);
  CHECK(reopened->metadata().format == ledger_format::v2);
  CHECK(reopened->metadata().identity == expected.identity);
  CHECK(reopened->metadata().workspace_id == expected.workspace_id);
  CHECK(reopened->metadata().created_at == expected.created_at);
}
} // namespace
TEST_CASE("ledger format2 publication faults retain complete target only after atomic publication", "[ledger][fault]") {
  for (auto selected : {shim::fault::file_sync, shim::fault::link, shim::fault::parent_sync, shim::fault::cleanup_file,
                        shim::fault::cleanup_directory}) {
    for (int operation : {0, 1, 2}) {
      INFO(std::format("fault={} operation={}", static_cast<int>(selected), operation));
      scratch f;
      auto    source = initialize_new_v2(f.root / "source", "storage_test");
      REQUIRE(source);
      const auto source_bytes = bytes(f.root / "source");
      const auto path         = f.root / "target";
      shim::reset(selected, path);
      auto result = operation == 0   ? initialize_new_v2(path, "storage_test")
                    : operation == 1 ? backup_new(f.root / "source", path)
                                     : restore_new(f.root / "source", path);
      REQUIRE(shim::hits() == 1);
      REQUIRE_FALSE(result);
      const bool published = selected == shim::fault::parent_sync || selected == shim::fault::cleanup_file ||
                             selected == shim::fault::cleanup_directory;
      CHECK(result.error().code == (published ? format_error_code::publication_uncertain : format_error_code::io));
      shim::reset(shim::fault::none, {});
      CHECK(bytes(f.root / "source") == source_bytes);
      if (published) {
        if (operation)
          valid(path, *source);
        else {
          auto reopened = open_existing(path);
          REQUIRE(reopened);
          CHECK(reopened->metadata().workspace_id == "storage_test");
        }
      } else
        CHECK_FALSE(std::filesystem::exists(path));
      if (selected == shim::fault::cleanup_file || selected == shim::fault::cleanup_directory) {
        bool staging = false;
        for (const auto& entry : std::filesystem::directory_iterator(f.root))
          if (entry.path().filename().string().starts_with(".planar-init-"))
            staging = true;
        CHECK(staging);
      }
    }
  }
}
TEST_CASE("ledger format2 publication late sidecar refuses and preserves arriving sidecar", "[ledger][fault]") {
  for (int operation : {0, 1, 2}) {
    scratch f;
    auto    source = initialize_new_v2(f.root / "source", "storage_test");
    REQUIRE(source);
    const auto path = f.root / "target";
    shim::reset(shim::fault::sidecar, path);
    auto result = operation == 0   ? initialize_new_v2(path, "storage_test")
                  : operation == 1 ? backup_new(f.root / "source", path)
                                   : restore_new(f.root / "source", path);
    REQUIRE(shim::hits() == 1);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == format_error_code::recovery_required);
    CHECK_FALSE(std::filesystem::exists(path));
    CHECK(bytes(path.string() + "-journal") == "late-sidecar-sentinel");
  }
}
TEST_CASE("ledger format2 publication no replace preserves target arriving at link boundary", "[ledger][fault]") {
  scratch    f;
  const auto path = f.root / "target";
  shim::reset(shim::fault::target_arrival, path);
  auto result = initialize_new_v2(path, "storage_test");
  REQUIRE(shim::hits() == 1);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == format_error_code::exists);
  CHECK(bytes(path) == "arriving-target-sentinel");
}
