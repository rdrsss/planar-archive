// Native-only acceptance ownership, snapshot binding, canonical provenance and exact request intent.
#include <catch2/catch_test_macros.hpp>
#include <sqlite3.h>
#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.engine.acceptance.prepared;
import planar.engine.canonical;
import planar.test.atomic_format;
import planar.test.atomic_storage;
import planar.test.atomic_acceptance;
import planar.test.atomic_digest;
using namespace planar::engine;
using namespace planar::engine::history;
using namespace planar::engine::acceptance;
using namespace planar::engine::ledger;
namespace atomic        = planar::engine::atomic_test;
using format_error_code = atomic::format_error_code;
namespace {
struct temporary {
  std::filesystem::path path;
  temporary() {
    char pattern[] = "/tmp/planar-prepared-XXXXXX";
    path           = std::filesystem::canonical(::mkdtemp(pattern));
    put("workspace/a.lua", std::string("a\0b\r\n", 5));
    put("workspace/b.lua", "second");
    put("registry.json", std::string("r\0z", 3));
    REQUIRE(::chmod((path / "workspace/b.lua").c_str(), 0755) == 0);
  }
  ~temporary() {
    std::filesystem::remove_all(path);
  }
  void put(std::string name, std::string_view bytes) {
    auto file = path / name;
    std::filesystem::create_directories(file.parent_path());
    std::ofstream stream(file, std::ios::binary);
    stream.write(bytes.data(), bytes.size());
  }
  acceptance_capture capture(int registry = 1) {
    native_capture_session session;
    REQUIRE(session.add_root("workspace", path / "workspace"));
    REQUIRE(session.tree("workspace"));
    if (registry == 1) {
      REQUIRE(session.add_input("registry", path / "registry.json"));
      REQUIRE(session.read("registry", "input.json", false));
    } else if (registry == 2) {
      REQUIRE(session.add_root("registry", path / "other"));
      REQUIRE(session.tree("registry"));
    }
    auto result = session.finish_for_acceptance();
    REQUIRE(result);
    return std::move(*result);
  }
};
std::string read_bytes(const std::filesystem::path& file) {
  std::ifstream stream(file, std::ios::binary);
  REQUIRE(stream.good());
  return {std::istreambuf_iterator<char>(stream), {}};
}
std::string hex(const record_digest& digest) {
  std::string out;
  for (auto byte : digest)
    out += std::format("{:02x}", byte);
  return out;
}
struct inputs {
  prepared_provenance     stdlib;
  recorded_interpretation interpretation;
  inputs() {
    const std::array originals{original_file_view{"part.lua", "return {}"}};
    auto             p = prepare_stdlib(originals, "\n", "return {}\n");
    REQUIRE(p);
    stdlib       = std::move(*p);
    auto root    = std::filesystem::path(PLANAR_PREPARED_GOLDENS);
    auto decoded = decode_interpretation(read_bytes(root / "interpretation.canonical"), read_bytes(root / "versions.canonical"));
    REQUIRE(decoded);
    auto p2 = encode_interpretation(decoded->interpretation->fields.at("bundle"), decoded->interpretation->fields.at("semantic"),
                                    hex(stdlib.digest));
    REQUIRE(p2);
    interpretation = std::move(*p2);
  }
  auto prepare(acceptance_capture cap, std::span<const std::string_view> paths) {
    return prepared_acceptance::create(std::move(cap), paths, stdlib.files, interpretation.encoded,
                                       interpretation.versions_encoded);
  }
};
const std::array paths{std::string_view("b.lua"), std::string_view("a.lua")};

} // namespace
namespace {
request_intent intent(temporary& t, std::string_view key, std::optional<std::int64_t> head, std::string_view actor = "tester") {
  auto result =
      request_intent::create(key, actor, head, (t.path / "workspace/a.lua").string(), (t.path / "registry.json").string());
  REQUIRE(result);
  return *result;
}
prepared_acceptance prepare(temporary& t, inputs& i) {
  auto result = i.prepare(t.capture(), paths);
  REQUIRE(result);
  return std::move(*result);
}
void refused(const auto& result, format_error_code code) {
  REQUIRE_FALSE(result);
  CHECK(result.error().code == code);
}
void initial_state(temporary& t) {
  auto file = atomic::open_existing(t.path / "ledger");
  REQUIRE(file);
  auto head = file->read_head();
  REQUIRE(head);
  CHECK_FALSE(head->revision);
  CHECK(head->last_sequence == 1);
  CHECK_FALSE(*file->read_request("first"));
}
} // namespace
TEST_CASE("atomic targeted collisions refuse file directory interpretation and request without prefixes", "[atomic_variant]") {
  for (int kind : {1, 2, 3, 4}) {
    DYNAMIC_SECTION(kind) {
      temporary t;
      inputs    i;
      planar::test::atomic_digest::target_kind = kind;
      struct reset {
        ~reset() {
          planar::test::atomic_digest::target_kind = 0;
        }
      } reset_digest;
      REQUIRE(atomic::initialize_new(t.path / "ledger", "quota"));
      auto candidate = prepare(t, i);
      // One deliberately colliding seed; all other kinds retain their real portable hashes.
      {
        auto writer = atomic::begin_records(t.path / "ledger");
        REQUIRE(writer);
        record seed;
        switch (kind) {
        case 1:
          seed = file_record{420, "collision seed"};
          break;
        case 2:
          seed = directory_record{};
          break;
        case 3:
          seed = value_record{"canonical_v1", std::string(1, '\0')};
          break;
        default:
          seed = request_record{"load", "seed", "none", "/workspace", "/registry"};
          break;
        }
        REQUIRE(writer->intern(seed));
        REQUIRE(writer->commit());
      }
      const auto before = read_bytes(t.path / "ledger");
      auto       result = atomic::complete_acceptance(t.path / "ledger", intent(t, "first", {}), candidate);
      refused(result, format_error_code::hash_collision);
      // Copy the authoritative error after complete_acceptance has unwound its tree
      // callbacks and rolled back the owning native transaction.
      auto retained = result.error();
      REQUIRE(retained.detail);
      const auto& detail = std::get<types::record_collision_detail>(*retained.detail);
      CHECK(detail.kind == static_cast<record_kind>(kind));
      if (kind == 4) {
        CHECK_FALSE(detail.digest);
      } else {
        REQUIRE(detail.digest);
        record_digest expected{};
        expected[0] = static_cast<std::uint8_t>(kind);
        CHECK(*detail.digest == expected);
      }
      CHECK(retained.message.find("first") == std::string::npos);
      CHECK(retained.message.find("tester") == std::string::npos);
      CHECK(retained.message.find(t.path.string()) == std::string::npos);
      CHECK(read_bytes(t.path / "ledger") == before);
      initial_state(t);
    }
  }
}
TEST_CASE("atomic exact private signed allocator boundary covers event and revision exhaustion", "[atomic_variant]") {
  constexpr auto maximum = std::numeric_limits<std::int64_t>::max();
  CHECK(atomic::allocator_boundary(0) == 1);
  CHECK(atomic::allocator_boundary(maximum - 1) == maximum);
  refused(atomic::allocator_boundary(maximum), format_error_code::unsupported_state);
}

TEST_CASE("atomic variant final receipt mutation cannot publish changed capture", "[atomic_variant]") {
  temporary t;
  inputs    i;
  REQUIRE(atomic::initialize_new(t.path / "ledger", "quota"));
  auto       candidate = prepare(t, i);
  const auto before    = read_bytes(t.path / "ledger");
  t.put("workspace/a.lua", "changed receipt");
  refused(atomic::complete_acceptance(t.path / "ledger", intent(t, "first", {}), candidate), format_error_code::capture_changed);
  CHECK(read_bytes(t.path / "ledger") == before);
  initial_state(t);
}

TEST_CASE("atomic codec2 forced request collision rolls back and redacts private digest", "[atomic_variant][request_v2]") {
  temporary t;
  inputs    i;
  planar::test::atomic_digest::target_kind = 4;
  struct reset {
    ~reset() {
      planar::test::atomic_digest::target_kind = 0;
    }
  } reset_digest;
  REQUIRE(atomic::initialize_new_v2(t.path / "ledger", "quota"));
  {
    auto writer = atomic::begin_records(t.path / "ledger");
    REQUIRE(writer);
    REQUIRE(writer->intern_request_v2(request_v2{"load", "secret-seed", "none", "/private/workspace", "/private/registry", {}}));
    REQUIRE(writer->commit());
  }
  const auto before    = read_bytes(t.path / "ledger");
  auto       candidate = prepare(t, i);
  auto       request   = request_intent::create_v2("first", "tester", {}, "/workspace", "/registry");
  REQUIRE(request);
  auto result = atomic::complete_acceptance(t.path / "ledger", *request, candidate);
  refused(result, format_error_code::hash_collision);
  REQUIRE(result.error().detail);
  const auto detail = std::get<types::record_collision_detail>(*result.error().detail);
  CHECK(detail.kind == record_kind::request);
  CHECK_FALSE(detail.digest);
  CHECK(result.error().message == "Unequal record digest collision");
  CHECK(read_bytes(t.path / "ledger") == before);
  initial_state(t);
}

TEST_CASE("atomic forced request digests separate codec buckets and preserve version body conflict",
          "[atomic_variant][request_v2]") {
  temporary t;
  inputs    i;
  planar::test::atomic_digest::target_kind = 4;
  struct reset {
    ~reset() {
      planar::test::atomic_digest::target_kind = 0;
    }
  } reset_digest;
  REQUIRE(atomic::initialize_new_v2(t.path / "ledger", "quota"));
  request_record legacy{"load", "tester", "none", "/workspace", "/registry"};
  auto           request = request_intent::create_v2("first", "tester", {}, "/workspace", "/registry");
  REQUIRE(request);
  atomic::record_reference old_ref, new_ref;
  {
    auto writer = atomic::begin_records(t.path / "ledger");
    REQUIRE(writer);
    auto old   = writer->intern(legacy);
    auto fresh = writer->intern_request_v2(*(**request->data()).version2);
    REQUIRE(old);
    REQUIRE(fresh);
    old_ref = *old;
    new_ref = *fresh;
    CHECK(old_ref.digest == new_ref.digest);
    CHECK(old_ref.id != new_ref.id);
    CHECK(writer->intern(legacy)->id == old_ref.id);
    CHECK(writer->intern_request_v2(*(**request->data()).version2)->id == new_ref.id);
    REQUIRE(writer->commit());
  }
  auto candidate = prepare(t, i);
  auto accepted  = atomic::complete_acceptance(t.path / "ledger", *request, candidate);
  REQUIRE(accepted);
  {
    auto file = atomic::open_existing(t.path / "ledger");
    REQUIRE(file);
    CHECK(std::holds_alternative<request_record>(file->read_record(old_ref.id)->value));
    CHECK(std::holds_alternative<request_v2>(file->read_record(new_ref.id)->value));
  }
  auto old_intent = request_intent::create("first", "tester", {}, "/workspace", "/registry");
  REQUIRE(old_intent);
  const auto before = read_bytes(t.path / "ledger");
  refused(atomic::complete_acceptance(t.path / "ledger", *old_intent, candidate), format_error_code::request_conflict);
  CHECK(read_bytes(t.path / "ledger") == before);
}
