// Scratch-only native storage checks. No installed or operator databases are used.
#include <catch2/catch_test_macros.hpp>
#include <unistd.h>
import std;
import planar.engine.ledger.storage;
import planar.engine.ledger.format;
using namespace planar::engine::ledger;
namespace hist = planar::engine::history;
namespace {
struct scratch {
  std::filesystem::path root;
  scratch() {
    char  pattern[] = "/tmp/planar-storage-XXXXXX";
    auto* made      = ::mkdtemp(pattern);
    REQUIRE(made);
    root = std::filesystem::canonical(made);
  }
  ~scratch() {
    std::filesystem::remove_all(root);
  }
};
std::string bytes(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(in), {}};
}
void initialized(const std::filesystem::path& path) {
  REQUIRE(initialize_new(path, "main_ws"));
}
hist::record sample() {
  return hist::file_record{420, "storage sample"};
}
constexpr std::string_view unavailable = "Unavailable record transaction";
} // namespace
static_assert(std::is_nothrow_move_constructible_v<write_transaction>);
static_assert(std::is_nothrow_move_assignable_v<write_transaction>);
static_assert(!std::is_copy_constructible_v<write_transaction>);
static_assert(!std::is_copy_assignable_v<write_transaction>);
static_assert(std::is_nothrow_move_constructible_v<read_snapshot>);
static_assert(std::is_nothrow_move_assignable_v<read_snapshot>);
static_assert(!std::is_copy_constructible_v<read_snapshot>);
static_assert(!std::is_copy_assignable_v<read_snapshot>);
TEST_CASE("storage write transaction matches the record writer byte for byte", "[storage]") {
  scratch    f;
  const auto direct = f.root / "direct";
  const auto legacy = f.root / "records";
  initialized(direct);
  initialized(legacy);
  {
    auto tx = begin_write(direct);
    REQUIRE(tx);
    REQUIRE(tx->intern_value(sample()));
    REQUIRE(tx->verify_before_commit(commit_site::record_writer));
    REQUIRE(tx->commit(commit_site::record_writer));
    // The transaction is unavailable after commit.
    auto again = tx->read_head();
    REQUIRE_FALSE(again);
    CHECK(again.error().code == format_error_code::invalid_argument);
    CHECK(again.error().message == unavailable);
  }
  {
    auto writer = begin_records(legacy);
    REQUIRE(writer);
    REQUIRE(writer->intern(sample()));
    REQUIRE(writer->commit());
  }
  auto a = open_existing(direct);
  auto b = open_existing(legacy);
  REQUIRE(a);
  REQUIRE(b);
  auto left  = a->read_record(1);
  auto right = b->read_record(1);
  REQUIRE(left);
  REQUIRE(right);
  CHECK(left->reference.id == right->reference.id);
  CHECK(left->reference.digest == right->reference.digest);
  auto x = planar::engine::ledger::types::encode_stored_record(*left);
  auto y = planar::engine::ledger::types::encode_stored_record(*right);
  REQUIRE(x);
  REQUIRE(y);
  CHECK(*x == *y);
}
TEST_CASE("storage abort and destruction roll back", "[storage]") {
  scratch    f;
  const auto path = f.root / "db";
  initialized(path);
  const auto before = bytes(path);
  {
    auto tx = begin_write(path);
    REQUIRE(tx);
    REQUIRE(tx->intern_value(sample()));
    tx->abort();
    tx->abort();
    CHECK_FALSE(tx->read_head());
  }
  CHECK(bytes(path) == before);
  {
    auto tx = begin_write(path);
    REQUIRE(tx);
    REQUIRE(tx->intern_value(sample()));
  }
  CHECK(bytes(path) == before);
  auto reopened = open_existing(path);
  REQUIRE(reopened);
  CHECK_FALSE(reopened->read_record(1));
}
TEST_CASE("storage failed operation poisons and rolls back the transaction", "[storage]") {
  scratch    f;
  const auto path = f.root / "db";
  initialized(path);
  const auto before = bytes(path);
  {
    auto tx = begin_write(path);
    REQUIRE(tx);
    REQUIRE(tx->intern_value(sample()));
    hist::record_child bogus;
    bogus.name  = "a";
    bogus.id    = 999;
    auto failed = tx->intern_value(hist::directory_record{{bogus}});
    REQUIRE_FALSE(failed);
    CHECK(failed.error().code == format_error_code::invalid_argument);
    CHECK(failed.error().message == "Invalid record child reference");
    auto check = [](const auto& result) {
      REQUIRE_FALSE(result);
      CHECK(result.error().code == format_error_code::invalid_argument);
      CHECK(result.error().message == unavailable);
    };
    check(tx->read_head());
    check(tx->intern_value(sample()));
    check(tx->read_record(1));
    check(tx->verify_before_commit(commit_site::record_writer));
    check(tx->commit(commit_site::record_writer));
  }
  CHECK(bytes(path) == before);
  auto reopened = open_existing(path);
  REQUIRE(reopened);
  CHECK_FALSE(reopened->read_record(1));
}
TEST_CASE("storage entry and precommit identity checks are separate and site specific", "[storage]") {
  struct variant {
    commit_site      site;
    std::string_view message;
  };
  const std::array<variant, 2> variants{variant{commit_site::record_writer, "Ledger identity changed before commit"},
                                        variant{commit_site::acceptance, "Ledger identity changed before acceptance commit"}};
  for (const auto& wanted : variants) {
    scratch    f;
    const auto path  = f.root / "db";
    const auto moved = f.root / "moved";
    initialized(path);
    auto tx = begin_write(path);
    REQUIRE(tx);
    // Entry verification already succeeded; replacing the named file now is observable only at the precommit check.
    std::filesystem::rename(path, moved);
    auto verified = tx->verify_before_commit(wanted.site);
    REQUIRE_FALSE(verified);
    CHECK(verified.error().code == format_error_code::unsafe_path);
    CHECK(verified.error().message == wanted.message);
    auto later = tx->commit(wanted.site);
    REQUIRE_FALSE(later);
    CHECK(later.error().message == unavailable);
  }
}
TEST_CASE("storage begin_write refuses a missing ledger", "[storage]") {
  scratch f;
  auto    tx = begin_write(f.root / "missing");
  REQUIRE_FALSE(tx);
  CHECK(tx.error().code == format_error_code::not_found);
}
TEST_CASE("storage request miss does not poison the transaction", "[storage]") {
  scratch    f;
  const auto path = f.root / "db";
  initialized(path);
  auto tx = begin_write(path);
  REQUIRE(tx);
  auto miss = tx->read_request("absent-request");
  REQUIRE(miss);
  CHECK_FALSE(*miss);
  auto head = tx->read_head();
  REQUIRE(head);
  CHECK_FALSE(head->revision);
  CHECK(head->last_sequence == 1);
  CHECK(tx->intern_value(sample()));
  tx->abort();
}
