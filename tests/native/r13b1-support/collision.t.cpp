// Verification-only forced collisions against a mechanically generated format module.
#include <catch2/catch_test_macros.hpp>
#include <unistd.h>
import std;
import planar.test.record_format;
import planar.test.record_digest;
import planar.engine.history.record;
import planar.engine.ledger.sqlite;
namespace test_format = planar::engine::record_test;
using namespace planar::engine::history;
namespace {
struct scratch {
  std::filesystem::path root;
  scratch() {
    char pattern[] = "/tmp/planar-record-collision-XXXXXX";
    auto made      = ::mkdtemp(pattern);
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
void inject(const std::filesystem::path& path, std::int64_t id, const record& value) {
  using namespace planar::engine::ledger;
  auto db = connection::open(path, open_mode::read_write);
  REQUIRE(db);
  auto q = db->prepare("INSERT INTO records VALUES(?1,?2,1,?3,?4)");
  REQUIRE(q);
  auto digest = planar::test::forced_digest(value).value();
  REQUIRE(q->bind_integer(1, id));
  REQUIRE(q->bind_integer(2, static_cast<std::int64_t>(kind_of(value))));
  REQUIRE(q->bind_blob(3, {reinterpret_cast<const char*>(digest.data()), digest.size()}));
  REQUIRE(q->bind_blob(4, encode_record(value).value()));
  REQUIRE(q->step() == step_result::done);
}
} // namespace
TEST_CASE("ledger forced collisions refuse every kind and roll back prefix records", "[ledger][collision]") {
  const std::array<std::pair<record, record>, 4> pairs{
      {{file_record{420, "first"}, file_record{420, "second"}},
       {directory_record{}, directory_record{{{"a", record_kind::file, 1, {}}}}},
       {value_record{"canonical_v1", std::string(1, '\0')}, value_record{"canonical_v1", std::string(1, '\1')}},
       {request_record{"load", "actor", "none", "workspace", "registry"},
        request_record{"load", "other", "none", "workspace", "registry"}}}};
  for (std::size_t i = 0; i < pairs.size(); ++i) {
    INFO(i);
    scratch f;
    auto    path        = f.root / "db";
    auto    initialized = test_format::initialize_new(path, "collision");
    INFO((initialized ? "initialized" : initialized.error().message));
    REQUIRE(initialized);
    auto first = pairs[i].first, second = pairs[i].second;
    {
      auto writer = test_format::begin_records(path);
      REQUIRE(writer);
      if (i == 1) {
        auto leaf = writer->intern(file_record{420, "child"});
        REQUIRE(leaf);
        auto& edge  = std::get<directory_record>(second).children[0];
        edge.id     = leaf->id;
        edge.digest = leaf->digest;
      }
      auto stored = writer->intern(first);
      REQUIRE(stored);
      REQUIRE(writer->commit());
    }
    const auto                               before = bytes(path);
    std::optional<test_format::format_error> retained;
    {
      auto writer = test_format::begin_records(path);
      REQUIRE(writer);
      // Prefix uses a different kind, so it does not collide with the target bucket.
      if (i == 0)
        REQUIRE(writer->intern(directory_record{}));
      else if (i != 1)
        REQUIRE(writer->intern(file_record{420, "prefix"}));
      else
        REQUIRE(writer->intern(value_record{"canonical_v1", std::string(1, '\0')}));
      auto exact = writer->intern(first);
      REQUIRE(exact);
      auto result = writer->intern(second);
      CHECK_FALSE(result);
      if (!result) {
        CHECK(result.error().code == test_format::format_error_code::hash_collision);
        retained = result.error();
      }
      CHECK_FALSE(writer->commit());
      CHECK_FALSE(writer->intern(first));
    }
    REQUIRE(retained);
    REQUIRE(retained->detail);
    const auto& detail = std::get<planar::engine::ledger::types::record_collision_detail>(*retained->detail);
    CHECK(detail.kind == kind_of(second));
    if (i == 3) {
      CHECK_FALSE(detail.digest);
    } else {
      REQUIRE(detail.digest);
      CHECK(*detail.digest == planar::test::forced_digest(second).value());
    }
    CHECK(retained->message.find("workspace") == std::string::npos);
    CHECK(retained->message.find("registry") == std::string::npos);
    CHECK(retained->message.find("actor") == std::string::npos);
    CHECK(bytes(path) == before);
    auto reopened = test_format::open_existing(path);
    REQUIRE(reopened);
    CHECK_FALSE(reopened->read_record(i == 1 ? 3 : 2));
  }
}
TEST_CASE("ledger forced directory collisions compare exact database local child IDs", "[ledger][collision]") {
  scratch f;
  auto    path        = f.root / "db";
  auto    initialized = test_format::initialize_new(path, "collision");
  INFO((initialized ? "initialized" : initialized.error().message));
  REQUIRE(initialized);
  record leaf = file_record{420, "same child"};
  inject(path, 1, leaf);
  inject(path, 2, leaf);
  auto   hash   = planar::test::forced_digest(leaf).value();
  record first  = directory_record{{{"a", record_kind::file, 1, hash}}};
  record second = directory_record{{{"a", record_kind::file, 2, hash}}};
  {
    auto writer = test_format::begin_records(path);
    REQUIRE(writer);
    REQUIRE(writer->intern(first));
    REQUIRE(writer->commit());
  }
  const auto before = bytes(path);
  auto       writer = test_format::begin_records(path);
  REQUIRE(writer);
  auto result = writer->intern(second);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == test_format::format_error_code::hash_collision);
  CHECK_FALSE(writer->commit());
  CHECK(bytes(path) == before);
}
TEST_CASE("ledger forced ambiguous exact plus unequal buckets refuse every opening path unchanged", "[ledger][collision]") {
  for (bool reverse : {false, true}) {
    scratch f;
    auto    path        = f.root / "db";
    auto    initialized = test_format::initialize_new(path, "collision");
    INFO((initialized ? "initialized" : initialized.error().message));
    REQUIRE(initialized);
    record first = file_record{420, "exact"}, other = file_record{420, "unequal"};
    inject(path, 1, first);
    inject(path, 2, reverse ? other : first);
    inject(path, 3, reverse ? first : other);
    const auto before = bytes(path);
    for (auto access : {test_format::ledger_access::read_only, test_format::ledger_access::read_write}) {
      auto result = test_format::open_existing(path, access);
      REQUIRE_FALSE(result);
      CHECK(result.error().code == test_format::format_error_code::corrupt);
    }
    CHECK_FALSE(test_format::begin_records(path));
    CHECK_FALSE(test_format::backup_new(path, f.root / "backup"));
    CHECK_FALSE(test_format::restore_new(path, f.root / "restore"));
    CHECK(bytes(path) == before);
    CHECK_FALSE(std::filesystem::exists(f.root / "backup"));
    CHECK_FALSE(std::filesystem::exists(path.string() + "-journal"));
  }
}
