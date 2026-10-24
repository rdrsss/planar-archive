// Typed canonical binary encoding and malformed native-value boundary tests.
#include <catch2/catch_test_macros.hpp>
import std;
import planar.engine.canonical;
using namespace planar::engine;
namespace {
std::shared_ptr<value> node(value_kind kind) {
  auto p  = std::make_shared<value>();
  p->kind = kind;
  return p;
}
std::string hex(std::string_view bytes) {
  std::string out;
  for (unsigned char b : bytes)
    out += std::format("{:02x}", b);
  return out;
}
} // namespace
TEST_CASE("canonical distinguishes typed empty values and integer boundaries", "[canonical]") {
  auto p = node(value_kind::boolean);
  CHECK(hex(*canonical_encode(p)) == "00");
  p->boolean = true;
  CHECK(hex(*canonical_encode(p)) == "01");
  p = node(value_kind::integer);
  for (auto [n, bytes] : std::vector<std::pair<std::int64_t, std::string>>{{0, "020000000000000000"},
                                                                           {-1, "02ffffffffffffffff"},
                                                                           {-2147483648LL, "02ffffffff80000000"},
                                                                           {2147483647LL, "02000000007fffffff"}}) {
    p->integer = n;
    CHECK(hex(*canonical_encode(p)) == bytes);
  }
  CHECK(hex(*canonical_encode(node(value_kind::string))) == "030000000000000000");
  CHECK(hex(*canonical_encode(node(value_kind::list))) == "040000000000000000");
  CHECK(hex(*canonical_encode(node(value_kind::record))) == "050000000000000000");
}
TEST_CASE("canonical preserves raw strings and unsigned UTF8 key order", "[canonical]") {
  auto raw    = node(value_kind::string);
  raw->string = std::string("\0\xff\x80"
                            "A",
                            4);
  CHECK(hex(*canonical_encode(raw)) == "03000000000000000400ff8041");
  auto p         = node(value_kind::record);
  p->fields["é"] = node(value_kind::boolean);
  auto yes       = node(value_kind::boolean);
  yes->boolean   = true;
  p->fields["z"] = yes;
  CHECK(hex(*canonical_encode(p)) == "0500000000000000020300000000000000017a01030000000000000002c3a900");
  auto same         = node(value_kind::record);
  same->fields["z"] = yes;
  same->fields["é"] = p->fields["é"];
  CHECK(canonical_encode(p).value() == canonical_encode(same).value());
  same->fields["e\xcc\x81"] = yes;
  CHECK(canonical_encode(p).value() != canonical_encode(same).value());
  auto array   = node(value_kind::list);
  array->items = {yes, p->fields["é"]};
  CHECK(hex(*canonical_encode(array)) == "0400000000000000020100");
  std::ranges::reverse(array->items);
  CHECK(hex(*canonical_encode(array)) == "0400000000000000020001");
}
TEST_CASE("canonical refuses null private malformed cyclic and out of range input", "[canonical]") {
  CHECK(canonical_encode({}).error().code == "schema");
  for (auto kind : {value_kind::workspace, value_kind::spec, value_kind::job, value_kind::md, value_kind::file,
                    value_kind::immaterial, static_cast<value_kind>(999)})
    CHECK(canonical_encode(node(kind)).error().code == "schema");
  auto p = node(value_kind::integer);
  for (auto n : {-2147483649LL, 2147483648LL}) {
    p->integer = n;
    CHECK(canonical_encode(p).error().code == "schema");
  }
  for (auto kind : {value_kind::boolean, value_kind::integer, value_kind::record, value_kind::list}) {
    p         = node(kind);
    p->string = "extraneous";
    CHECK(canonical_encode(p).error().code == "schema");
  }
  p          = node(value_kind::string);
  p->boolean = true;
  CHECK_FALSE(canonical_encode(p));
  p->boolean = false;
  p->integer = 1;
  CHECK_FALSE(canonical_encode(p));
  p = node(value_kind::record);
  for (auto key : {std::string("\0", 1), std::string("\xff", 1), std::string("\xed\xa0\x80", 3)}) {
    p->fields = {{key, node(value_kind::boolean)}};
    CHECK(canonical_encode(p).error().code == "schema");
  }
  p->fields = {{"valid", {}}};
  CHECK_FALSE(canonical_encode(p));
  auto cycle = node(value_kind::list);
  cycle->items.push_back(cycle);
  CHECK(canonical_encode(cycle).error().code == "schema");
  cycle->items.clear();
  auto record             = node(value_kind::record);
  record->fields["cycle"] = cycle;
  cycle->items.push_back(record);
  CHECK_FALSE(canonical_encode(cycle));
  cycle->items.clear();
  // Shared DAG nodes are valid; they are encoded at every occurrence.
  cycle->items = {p = node(value_kind::boolean), p};
  CHECK(hex(*canonical_encode(cycle)) == "0400000000000000020000");
}
TEST_CASE("canonical bounds depth nodes and bytes without declaration string caps", "[canonical]") {
  auto p      = node(value_kind::string);
  p->string   = std::string(16 * 1024 * 1024, '\xff');
  auto result = canonical_encode(p);
  REQUIRE(result);
  CHECK(result->size() == p->string.size() + 9);
  CHECK(canonical_encode(p, {.bytes = 9}).error().limit == "canonical_bytes");
  CHECK(canonical_encode(p, {.nodes = 0}).error().limit == "canonical_nodes");
  CHECK(canonical_encode(p, {.depth = 0}).error().limit == "canonical_depth");
  auto list   = node(value_kind::list);
  list->items = {node(value_kind::boolean)};
  CHECK(canonical_encode(list, {.depth = 1}).error().maximum == 1);
  CHECK(canonical_encode(list, {.nodes = 1}).error().limit == "canonical_nodes");
  auto record         = node(value_kind::record);
  record->fields["a"] = node(value_kind::boolean);
  CHECK(canonical_encode(record, {.nodes = 2}).error().limit == "canonical_nodes");
  CHECK(canonical_encode(record, {.nodes = 3}));
  value_ptr deep = node(value_kind::boolean);
  for (int i = 0; i < 256; ++i) {
    auto next   = node(value_kind::list);
    next->items = {deep};
    deep        = next;
  }
  CHECK(canonical_encode(deep).error().limit == "canonical_depth");
  CHECK(canonical_encode(deep, {.depth = 9999}).error().maximum == 256);
}

namespace {
std::string unhex(std::string_view text) {
  std::string bytes;
  for (std::size_t i = 0; i < text.size(); i += 2)
    bytes += static_cast<char>(std::stoul(std::string(text.substr(i, 2)), nullptr, 16));
  return bytes;
}
} // namespace
TEST_CASE("canonical decoder reproduces independent exact typed vectors", "[canonical]") {
  for (auto text : {"00", "01", "020000000000000000", "02ffffffffffffffff", "02ffffffff80000000", "02000000007fffffff",
                    "030000000000000000", "03000000000000000400ff8041", "040000000000000000", "050000000000000000",
                    "0400000000000000020100", "0500000000000000020300000000000000017a01030000000000000002c3a900"}) {
    auto bytes   = unhex(text);
    auto decoded = canonical_decode(bytes);
    REQUIRE(decoded);
    CHECK(canonical_encode(*decoded).value() == bytes);
    for (std::size_t size = 0; size < bytes.size(); ++size)
      CHECK_FALSE(canonical_decode(std::string_view(bytes).substr(0, size)));
    CHECK_FALSE(canonical_decode(bytes + '\0'));
  }
  CHECK(canonical_decode(unhex("05000000000000000103000000000000000000")));
  CHECK((*canonical_decode(unhex("040000000000000000")))->kind == value_kind::list);
  CHECK((*canonical_decode(unhex("050000000000000000")))->kind == value_kind::record);
  CHECK((*canonical_decode(unhex("03000000000000000400ff8041")))->string == std::string("\0\xff\x80"
                                                                                        "A",
                                                                                        4));
  auto root                = node(value_kind::list);
  auto nested              = node(value_kind::record);
  nested->fields["UTF8-é"] = node(value_kind::list);
  root->items              = {nested, node(value_kind::boolean), node(value_kind::integer)};
  auto encoded             = canonical_encode(root).value();
  CHECK(canonical_encode(canonical_decode(encoded).value()).value() == encoded);
}
TEST_CASE("canonical decoder refuses noncanonical integers tags keys and counts", "[canonical]") {
  for (auto text :
       {"06", "ff", "020000000080000000", "02ffffffff7fffffff", "028000000000000000", "030000000000000001", "03ffffffffffffffff",
        "04ffffffffffffffff", "05ffffffffffffffff", "04000000000000000200", "0500000000000000010000",
        "0500000000000000010300000000000000010000", "050000000000000001030000000000000001ff00",
        "050000000000000001030000000000000003eda08000", "05000000000000000203000000000000000162000300000000000000016100",
        "05000000000000000203000000000000000161000300000000000000016100"}) {
    INFO(text);
    CHECK_FALSE(canonical_decode(unhex(text)));
  }
}
TEST_CASE("canonical decoder bounds bytes nodes keys depth and claimed allocations", "[canonical]") {
  auto list = unhex("04000000000000000100");
  CHECK(canonical_decode(list, {.depth = 1}).error().limit == "canonical_depth");
  CHECK(canonical_decode(list, {.nodes = 1}).error().limit == "canonical_nodes");
  CHECK(canonical_decode(list, {.bytes = 9}).error().limit == "canonical_bytes");
  CHECK(canonical_decode(list, {.bytes = 10}));
  CHECK(canonical_decode(unhex("00"), {.depth = 0}).error().limit == "canonical_depth");
  CHECK(canonical_decode(unhex("00"), {.nodes = 0}).error().limit == "canonical_nodes");
  auto map = unhex("0500000000000000010300000000000000016100");
  CHECK(canonical_decode(map, {.nodes = 2}).error().limit == "canonical_nodes");
  CHECK(canonical_decode(map, {.nodes = 3}));
  auto deep = std::string();
  for (int i = 0; i < 255; ++i)
    deep += unhex("040000000000000001");
  CHECK(canonical_decode(deep + '\0'));
  deep += unhex("040000000000000001");
  CHECK(canonical_decode(deep + '\0', {.depth = 9999}).error().maximum == 256);
  CHECK(canonical_decode(unhex("04ffffffffffffffff")).error().limit == "canonical_nodes");
  // In-range but physically impossible counts must not reserve the claimed inventory.
  CHECK(canonical_decode(unhex("0400000000003d08ff")).error().code == "schema");
  CHECK(canonical_decode(unhex("030000000010000000")).error().code == "schema");
}
