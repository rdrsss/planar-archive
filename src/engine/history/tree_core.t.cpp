// Pure tree parity uses in-memory typed callbacks without format owners or SQLite.
#include <catch2/catch_test_macros.hpp>
import std;
import planar.engine.history.tree_core;
using namespace planar::engine::history;
namespace leaf = planar::engine::ledger::types;
namespace {
struct memory_store {
  std::vector<leaf::stored_record>                          records;
  std::expected<leaf::record_reference, leaf::format_error> intern(const record& value) {
    auto digest = digest_record(value);
    REQUIRE(digest);
    leaf::record_reference ref{records.size() + 17,
                               std::holds_alternative<file_record>(value) ? record_kind::file : record_kind::directory, *digest};
    records.push_back({ref, value});
    return ref;
  }
  std::expected<leaf::stored_record, leaf::format_error> read(std::uint64_t id) {
    for (const auto& stored : records)
      if (stored.reference.id == id)
        return stored;
    return std::unexpected(leaf::format_error{leaf::format_error_code::not_found, "missing"});
  }
};
} // namespace
TEST_CASE("tree core exact portable digest and reconstruction without storage", "[tree]") {
  memory_store      store;
  const std::string binary("\0\xff\r\n", 4);
  const std::array  files{tree_file_view{"z", ""}, tree_file_view{"src/run", binary, 493}, tree_file_view{"src/a", "a\r\n"}};
  for (auto profile : {tree_profile::source, tree_profile::provenance}) {
    auto root = build_tree_core(files, profile, [&](const record& value) { return store.intern(value); });
    REQUIRE(root);
    auto digest = digest_tree(files, profile);
    REQUIRE(digest);
    CHECK(root->digest == *digest);
    auto result = read_tree_core(*root, profile, [&](std::uint64_t id) { return store.read(id); });
    REQUIRE(result);
    REQUIRE(result->size() == 3);
    CHECK((*result)[0].path == "src/a");
    CHECK((*result)[0].payload == "a\r\n");
    CHECK((*result)[1].path == "src/run");
    CHECK((*result)[1].payload == binary);
    CHECK((*result)[1].mode == 493);
    CHECK((*result)[2].path == "z");
    CHECK((*result)[2].payload.empty());
  }
}
TEST_CASE("tree core rejects invalid inputs before interning and matches digest refusals", "[tree]") {
  for (const auto& paths :
       {std::vector<std::string>{"A/x", "a/y"}, {"a", "a"}, {"a", "a/b"}, {"../bad"}, {std::string(1025, 'x')}}) {
    std::vector<tree_file_view> files;
    for (const auto& path : paths)
      files.push_back({path, ""});
    std::size_t calls  = 0;
    auto        root   = build_tree_core(files, tree_profile::source,
                                         [&](const record&) -> std::expected<leaf::record_reference, leaf::format_error> {
                                  ++calls;
                                  return std::unexpected(leaf::format_error{leaf::format_error_code::io, "unexpected"});
                                         });
    auto        digest = digest_tree(files, tree_profile::source);
    REQUIRE_FALSE(root);
    REQUIRE_FALSE(digest);
    CHECK(root.error().code == digest.error().code);
    CHECK_FALSE(root.error().storage);
    CHECK(calls == 0);
  }
}
TEST_CASE("tree core preserves original typed callback failures and exception ownership", "[tree]") {
  const std::array files{tree_file_view{"a", "payload"}};
  for (auto code :
       {leaf::format_error_code::io, leaf::format_error_code::storage_full, leaf::format_error_code::hash_collision}) {
    std::size_t  calls = 0;
    memory_store store;
    auto         root = build_tree_core(files, tree_profile::source,
                                        [&](const record& value) -> std::expected<leaf::record_reference, leaf::format_error> {
                                  if (++calls == 2)
                                    return std::unexpected(leaf::format_error{code, "original refusal"});
                                  return store.intern(value);
                                        });
    REQUIRE_FALSE(root);
    CHECK(calls == 2);
    CHECK(store.records.size() == 1);
    CHECK(root.error().code == tree_error_code::storage);
    REQUIRE(root.error().storage);
    CHECK(root.error().storage->code == code);
    CHECK(root.error().storage->message == "original refusal");
    auto result = read_tree_core({17, record_kind::directory, {}}, tree_profile::source,
                                 [&](std::uint64_t id) -> std::expected<leaf::stored_record, leaf::format_error> {
                                   CHECK(id == 17);
                                   return std::unexpected(leaf::format_error{code, "original read refusal"});
                                 });
    REQUIRE_FALSE(result);
    CHECK(result.error().code == tree_error_code::storage);
    REQUIRE(result.error().storage);
    CHECK(result.error().storage->code == code);
    CHECK(result.error().storage->message == "original read refusal");
  }
  CHECK_THROWS_AS(
      build_tree_core(files, tree_profile::source,
                      [](const record&) -> std::expected<leaf::record_reference, leaf::format_error> { throw std::bad_alloc(); }),
      std::bad_alloc);
}
TEST_CASE("tree core reconstruction rejects identity and empty nonroot directories", "[tree]") {
  memory_store store;
  auto         empty = store.intern(directory_record{});
  REQUIRE(empty);
  auto root = store.intern(directory_record{{record_child{"empty", empty->kind, empty->id, empty->digest}}});
  REQUIRE(root);
  auto read   = [&](std::uint64_t id) { return store.read(id); };
  auto result = read_tree_core(*root, tree_profile::source, read);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == tree_error_code::invalid_tree);
  auto wrong = *root;
  wrong.digest[0] ^= 1;
  result = read_tree_core(wrong, tree_profile::source, read);
  REQUIRE_FALSE(result);
  CHECK(result.error().code == tree_error_code::invalid_tree);
  CHECK(read_tree_core(*empty, tree_profile::source, read)->empty());
}
TEST_CASE("tree core effective provenance counts expanded file occurrences", "[tree][effective]") {
  memory_store store;
  auto         file = store.intern(file_record{420, ""});
  REQUIRE(file);
  // Root aliases share one directory and one file record; budgets count expansions.
  directory_record group;
  for (int i = 0; i < 128; ++i)
    group.children.push_back({std::format("f{:03}", i), file->kind, file->id, file->digest});
  auto branch = store.intern(group);
  REQUIRE(branch);
  for (int count : {32, 64, 65}) {
    directory_record top;
    for (int i = 0; i < count; ++i)
      top.children.push_back({std::format("d{:03}", i), branch->kind, branch->id, branch->digest});
    auto root = store.intern(top);
    REQUIRE(root);
    for (auto profile : {tree_profile::source, tree_profile::provenance, tree_profile::effective_provenance}) {
      auto       result  = read_tree_core(*root, profile, [&](std::uint64_t id) { return store.read(id); });
      const auto maximum = profile == tree_profile::effective_provenance ? 8192 : 4096;
      if (count * 128 <= maximum) {
        REQUIRE(result);
        CHECK(result->size() == std::size_t(count * 128));
      } else {
        REQUIRE_FALSE(result);
        CHECK(result.error().code == tree_error_code::resource_limit);
      }
    }
  }
}
TEST_CASE("tree core effective provenance file boundary validates before callbacks", "[tree][effective]") {
  std::vector<std::string> names;
  for (int i = 0; i < 8193; ++i)
    names.push_back(std::format("f{:05}", i));
  std::vector<tree_file_view> files;
  for (auto& name : names)
    files.push_back({name, ""});
  for (auto profile : {tree_profile::source, tree_profile::provenance, tree_profile::effective_provenance}) {
    const std::size_t maximum = profile == tree_profile::effective_provenance ? 8192 : 4096;
    memory_store      store;
    auto              input = std::span(files).first(maximum);
    auto              root  = build_tree_core(input, profile, [&](const record& value) { return store.intern(value); });
    REQUIRE(root);
    auto digest = digest_tree(input, profile);
    REQUIRE(digest);
    CHECK(root->digest == *digest);
    auto result = read_tree_core(*root, profile, [&](std::uint64_t id) { return store.read(id); });
    REQUIRE(result);
    CHECK(result->size() == maximum);
    auto extra = store.intern(file_record{420, ""});
    REQUIRE(extra);
    auto directory = std::get<directory_record>(store.read(root->id)->value);
    directory.children.push_back({names[maximum], extra->kind, extra->id, extra->digest});
    auto over = store.intern(directory);
    REQUIRE(over);
    auto overflow = read_tree_core(*over, profile, [&](std::uint64_t id) { return store.read(id); });
    REQUIRE_FALSE(overflow);
    CHECK(overflow.error().code == tree_error_code::resource_limit);
    std::size_t calls   = 0;
    auto        refused = build_tree_core(std::span(files).first(maximum + 1), profile, [&](const record& value) {
      ++calls;
      return store.intern(value);
    });
    REQUIRE_FALSE(refused);
    CHECK(refused.error().code == tree_error_code::resource_limit);
    CHECK(calls == 0);
    CHECK_FALSE(digest_tree(std::span(files).first(maximum + 1), profile));
  }
}
TEST_CASE("tree core effective provenance byte budgets preserve original ceilings", "[tree][effective]") {
  const std::string payload(16 * 1024 * 1024, 'x');
  for (auto profile : {tree_profile::source, tree_profile::provenance, tree_profile::effective_provenance}) {
    const std::size_t           count = profile == tree_profile::source ? 4 : profile == tree_profile::provenance ? 1 : 2;
    std::vector<tree_file_view> files;
    for (std::size_t i = 0; i < count; ++i)
      files.push_back({std::string_view("abcd").substr(i, 1), payload});
    memory_store store;
    auto         root = build_tree_core(files, profile, [&](const record& value) { return store.intern(value); });
    REQUIRE(root);
    auto digest = digest_tree(files, profile);
    REQUIRE(digest);
    CHECK(root->digest == *digest);
    auto read   = [&](std::uint64_t id) { return store.read(id); };
    auto result = read_tree_core(*root, profile, read);
    REQUIRE(result);
    CHECK(result->size() == count);
    files.push_back({"z", "x"});
    std::size_t calls   = 0;
    auto        refused = build_tree_core(files, profile, [&](const record& value) {
      ++calls;
      return store.intern(value);
    });
    REQUIRE_FALSE(refused);
    CHECK(refused.error().code == tree_error_code::resource_limit);
    CHECK(calls == 0);
    CHECK_FALSE(digest_tree(files, profile));
    auto extra = store.intern(file_record{420, "x"});
    REQUIRE(extra);
    auto directory = std::get<directory_record>(store.read(root->id)->value);
    directory.children.push_back({"z", extra->kind, extra->id, extra->digest});
    auto over = store.intern(directory);
    REQUIRE(over);
    auto overflow = read_tree_core(*over, profile, read);
    REQUIRE_FALSE(overflow);
    CHECK(overflow.error().code == tree_error_code::resource_limit);
  }
}
TEST_CASE("tree core effective provenance preserves lower path depth and file limits", "[tree][effective]") {
  std::string deepest;
  for (int i = 0; i < 63; ++i)
    deepest += "d/";
  deepest += "f";
  // At most 63 non-root directory ancestors per file derives 1 + 8192 * 63.
  // Shared ancestors reduce the total; no larger directory fixture is reachable.
  std::string longest;
  for (int i = 0; i < 7; ++i)
    longest += std::string(127, 'x') + '/';
  longest += std::string(128, 'y');
  REQUIRE(longest.size() == 1024);
  for (const auto& path : {deepest, longest}) {
    const std::array files{tree_file_view{path, ""}};
    memory_store     store;
    auto             root =
        build_tree_core(files, tree_profile::effective_provenance, [&](const record& value) { return store.intern(value); });
    REQUIRE(root);
    REQUIRE(digest_tree(files, tree_profile::effective_provenance));
    auto result = read_tree_core(*root, tree_profile::effective_provenance, [&](std::uint64_t id) { return store.read(id); });
    REQUIRE(result);
    CHECK(result->front().path == path);
  }
  const std::string oversized(16 * 1024 * 1024 + 1, 'x');
  const auto        too_deep = deepest + "/x", too_long = longest + "x", segment = std::string(129, 'x');
  for (const auto& file : {tree_file_view{too_deep, ""}, tree_file_view{too_long, ""}, tree_file_view{segment, ""},
                           tree_file_view{"f", oversized}}) {
    std::size_t      calls = 0;
    const std::array files{file};
    auto             result = build_tree_core(files, tree_profile::effective_provenance,
                                              [&](const record&) -> std::expected<leaf::record_reference, leaf::format_error> {
                                    ++calls;
                                    return std::unexpected(leaf::format_error{leaf::format_error_code::io, "unexpected"});
                                              });
    REQUIRE_FALSE(result);
    CHECK(calls == 0);
    CHECK_FALSE(digest_tree(files, tree_profile::effective_provenance));
  }
  // Build a codec-valid chain whose expansion exceeds the shared depth limit.
  memory_store store;
  auto         ref = store.intern(file_record{420, ""});
  REQUIRE(ref);
  for (int i = 0; i < 65; ++i) {
    ref = store.intern(directory_record{{record_child{"d", ref->kind, ref->id, ref->digest}}});
    REQUIRE(ref);
  }
  auto deep = read_tree_core(*ref, tree_profile::effective_provenance, [&](std::uint64_t id) { return store.read(id); });
  REQUIRE_FALSE(deep);
  CHECK(deep.error().code == tree_error_code::resource_limit);
  const std::array empty{tree_file_view{"f", ""}};
  CHECK_FALSE(digest_tree(empty, static_cast<tree_profile>(99)));
}
