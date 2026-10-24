// Native-only acceptance ownership, snapshot binding, canonical provenance and exact request intent.
#include <catch2/catch_test_macros.hpp>
#include <sys/stat.h>
#include <unistd.h>
import std;
import planar.engine.acceptance.prepared;
import planar.engine.canonical;
import planar.engine.capture.host_path;
using namespace planar::engine;
using namespace planar::engine::history;
using namespace planar::engine::acceptance;
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
void             error_is(const auto& result, preparation_error_code code) {
  REQUIRE_FALSE(result);
  CHECK(result.error().code == code);
}
value_ptr changed(value_ptr original, std::string key, value_ptr replacement) {
  auto copy                    = std::make_shared<value>(*original);
  copy->fields[std::move(key)] = std::move(replacement);
  return copy;
}
value_ptr text(std::string bytes) {
  auto copy    = std::make_shared<value>();
  copy->kind   = value_kind::string;
  copy->string = std::move(bytes);
  return copy;
}
void reencode(inputs& i, value_ptr envelope, value_ptr versions) {
  auto a = canonical_encode(envelope), b = canonical_encode(versions);
  REQUIRE(a);
  REQUIRE(b);
  i.interpretation.encoded          = *a;
  i.interpretation.versions_encoded = *b;
}
} // namespace
static_assert(!std::is_default_constructible_v<prepared_acceptance>);
static_assert(!std::is_copy_constructible_v<prepared_acceptance>);
static_assert(!std::is_default_constructible_v<request_intent>);
TEST_CASE("acceptance prepared binds selected exact snapshot bytes after disk changes", "[acceptance]") {
  temporary t;
  inputs    i;
  auto      cap = t.capture();
  t.put("workspace/a.lua", "changed after sealing");
  t.put("registry.json", "changed registry");
  auto result = i.prepare(std::move(cap), paths);
  REQUIRE(result);
  auto data = *result->data();
  REQUIRE(data->source.size() == 2);
  CHECK(data->source[0].path == "a.lua");
  CHECK(data->source[0].payload == std::string("a\0b\r\n", 5));
  CHECK(data->source[1].mode == 493);
  CHECK(data->registry.files[0].payload == std::string("r\0z", 3));
  CHECK(data->workspace_id == "quota");
  CHECK(data->interpretation.encoded == i.interpretation.encoded);
  CHECK(result->verify().error().code == "capture_changed");
  CHECK(*result->data() == data);
  CHECK(data->source[0].payload == std::string("a\0b\r\n", 5));
}
TEST_CASE("acceptance prepared selects only explicit workspace paths and ignores other logical roots", "[acceptance]") {
  temporary t;
  inputs    i;
  t.put("other/a.lua", "external");
  native_capture_session session;
  REQUIRE(session.add_root("workspace", t.path / "workspace"));
  REQUIRE(session.tree("workspace"));
  REQUIRE(session.add_root("external", t.path / "other"));
  REQUIRE(session.tree("external"));
  REQUIRE(session.add_input("registry", t.path / "registry.json"));
  REQUIRE(session.read("registry", "input.json", false));
  auto cap = session.finish_for_acceptance();
  REQUIRE(cap);
  const std::array selected{std::string_view("a.lua")};
  auto             result = i.prepare(std::move(*cap), selected);
  REQUIRE(result);
  auto data = *result->data();
  REQUIRE(data->source.size() == 1);
  CHECK(data->source[0].payload == std::string("a\0b\r\n", 5));
  CHECK(result->verify());
  CHECK(result->verify());
}
TEST_CASE("acceptance prepared refuses empty duplicate absent invalid and excessive selections", "[acceptance]") {
  temporary t;
  inputs    i;
  for (const std::vector<std::string_view>& selected : {std::vector<std::string_view>{},
                                                        {"a.lua", "a.lua"},
                                                        {"missing"},
                                                        {"../a.lua"},
                                                        {"/a.lua"},
                                                        {"A.lua"},
                                                        {"registry/input.json"},
                                                        {"a.lua/child"}})
    error_is(i.prepare(t.capture(), selected), preparation_error_code::invalid_source);
  std::vector<std::string_view> many(4097, "a.lua");
  error_is(i.prepare(t.capture(), many), preparation_error_code::resource_limit);
  std::string      long_path(1025, 'a');
  const std::array excessive{std::string_view(long_path)};
  error_is(i.prepare(t.capture(), excessive), preparation_error_code::resource_limit);
}
TEST_CASE("acceptance prepared requires exactly registry input json", "[acceptance]") {
  temporary t;
  inputs    i;
  error_is(i.prepare(t.capture(0), paths), preparation_error_code::invalid_registry);
  t.put("other/wrong.json", "{}");
  error_is(i.prepare(t.capture(2), paths), preparation_error_code::invalid_registry);
  t.put("other/input.json", "{}");
  error_is(i.prepare(t.capture(2), paths), preparation_error_code::invalid_registry);
  std::filesystem::remove(t.path / "other/wrong.json");
  CHECK(i.prepare(t.capture(2), paths));
}
TEST_CASE("acceptance prepared reverifies stdlib files manifest aliases mode and recorded digest", "[acceptance]") {
  temporary t;
  inputs    good;
  for (int mutation = 0; mutation < 7; ++mutation) {
    inputs i;
    if (mutation == 0)
      i.stdlib.files[0].payload += 'x';
    if (mutation == 1)
      i.stdlib.files[0].mode = 493;
    if (mutation == 2)
      i.stdlib.files[0].mode = 0;
    if (mutation == 3)
      i.stdlib.files.pop_back();
    if (mutation == 4)
      i.stdlib.files.push_back(i.stdlib.files.back());
    if (mutation == 5)
      i.stdlib.files.push_back({"PART.lua", "return {}", 420});
    if (mutation == 6) {
      auto v = changed(i.interpretation.versions, "stdlib_digest", text(std::string(32, '0')));
      reencode(i, changed(i.interpretation.interpretation, "versions", v), v);
    }
    error_is(i.prepare(t.capture(), paths), preparation_error_code::invalid_stdlib);
  }
  // Supplied metadata is never accepted as a digest authority.
  good.stdlib.digest.fill(0);
  CHECK(good.prepare(t.capture(), paths));
}
TEST_CASE("acceptance prepared bounds external inputs before owning bytes", "[acceptance]") {
  temporary t;
  inputs    i;
  auto      many = i.stdlib.files;
  many.resize(4097);
  error_is(prepared_acceptance::create(t.capture(), paths, many, i.interpretation.encoded, i.interpretation.versions_encoded),
           preparation_error_code::resource_limit);
  auto large = std::string(16 * 1024 * 1024 + 1, 'x');
  for (bool versions : {false, true})
    error_is(prepared_acceptance::create(t.capture(), paths, i.stdlib.files, versions ? i.interpretation.encoded : large,
                                         versions ? large : i.interpretation.versions_encoded),
             preparation_error_code::resource_limit);
  i.stdlib.files.back().payload = large;
  error_is(i.prepare(t.capture(), paths), preparation_error_code::resource_limit);
}
TEST_CASE("acceptance prepared rejects canonical version mismatch unsupported and workspace shape", "[acceptance]") {
  temporary t;
  for (int mutation = 0; mutation < 5; ++mutation) {
    inputs i;
    if (mutation == 0)
      i.interpretation.encoded += 'x';
    if (mutation == 1) {
      auto v                            = changed(i.interpretation.versions, "stdlib_digest", text(std::string(32, '0')));
      i.interpretation.versions_encoded = *canonical_encode(v);
    }
    if (mutation == 2) {
      auto v = changed(i.interpretation.versions, "projection", text("future_v9"));
      reencode(i, changed(i.interpretation.interpretation, "versions", v), v);
    }
    if (mutation == 3 || mutation == 4) {
      auto b    = i.interpretation.interpretation->fields.at("bundle");
      auto w    = b->fields.at("workspace");
      auto bad  = std::make_shared<value>();
      bad->kind = value_kind::integer;
      auto v    = changed(i.interpretation.interpretation, "bundle",
                          mutation == 3 ? changed(b, "workspace", bad) : changed(b, "workspace", changed(w, "id", bad)));
      reencode(i, v, i.interpretation.versions);
    }
    error_is(i.prepare(t.capture(), paths),
             mutation == 2 ? preparation_error_code::unsupported_version : preparation_error_code::invalid_interpretation);
  }
}
TEST_CASE("acceptance prepared owns values across moves destruction and final native verification", "[acceptance]") {
  temporary t;
  inputs    i;
  auto      result = i.prepare(t.capture(), paths);
  REQUIRE(result);
  auto data  = *result->data();
  auto moved = std::move(*result);
  error_is(result->data(), preparation_error_code::moved_from);
  CHECK(result->verify().error().code == "capture_changed");
  CHECK(moved.verify());
  auto other = i.prepare(t.capture(), paths);
  REQUIRE(other);
  *other = std::move(moved);
  error_is(moved.data(), preparation_error_code::moved_from);
  CHECK(*other->data() == data);
  auto* same = &*other;
  *other     = std::move(*same);
  CHECK(other->verify());
  REQUIRE(::chmod((t.path / "workspace/b.lua").c_str(), 0644) == 0);
  CHECK(other->verify().error().code == "capture_changed");
  CHECK(data->source[1].mode == 493);
  i.stdlib.files.clear();
  i.interpretation = {};
  other            = std::unexpected(preparation_error{preparation_error_code::moved_from});
  CHECK(data->stdlib.files.size() == 3);
  CHECK_FALSE(data->interpretation.encoded.empty());
}
TEST_CASE("acceptance request intent preserves exact shared record bytes and positive signed64 heads", "[acceptance]") {
  for (auto head :
       {std::optional<std::int64_t>{}, std::optional<std::int64_t>{1}, std::optional<std::int64_t>{9223372036854775807LL}}) {
    auto result =
        request_intent::create("request_1.A:-", "drivér", head, "/nonexistent/workspace.lua", "/nonexistent/registry.json");
    REQUIRE(result);
    auto data = *result->data();
    CHECK(data->request_id == "request_1.A:-");
    CHECK(data->expected_head == head);
    CHECK(data->fields.operation == "load");
    CHECK(data->fields.actor == "drivér");
    CHECK(data->fields.expected_head == (head ? std::to_string(*head) : "none"));
    CHECK(data->encoded == *encode_record(data->fields));
    auto again = request_intent::create("another", "drivér", head, "/nonexistent/workspace.lua", "/nonexistent/registry.json");
    REQUIRE(again);
    CHECK((*again->data())->encoded == data->encoded);
    auto moved = std::move(*result);
    error_is(result->data(), preparation_error_code::moved_from);
    CHECK((*moved.data())->encoded == data->encoded);
  }
}
TEST_CASE("acceptance request intent rejects grammar relative nonnormalized and invalid text arguments", "[acceptance]") {
  for (auto id : {"", "has space", "slash/id", "é"})
    error_is(request_intent::create(id, "actor", {}, "/w", "/r"), preparation_error_code::invalid_request);
  for (auto actor : {std::string{}, std::string("a\0b", 3), std::string("\xff")})
    error_is(request_intent::create("id", actor, {}, "/w", "/r"), preparation_error_code::invalid_request);
  for (auto n : {0LL, -1LL, std::numeric_limits<long long>::min()})
    error_is(request_intent::create("id", "actor", n, "/w", "/r"), preparation_error_code::invalid_request);
  for (auto arg : {std::string{}, std::string("relative"), std::string("/a/../b"), std::string("/a/./b"), std::string("/a//b"),
                   std::string("/a\0b", 4), std::string("/\xff")}) {
    error_is(request_intent::create("id", "actor", {}, arg, "/r"), preparation_error_code::invalid_request);
    error_is(request_intent::create("id", "actor", {}, "/w", arg), preparation_error_code::invalid_request);
  }
  CHECK(request_intent::create(std::string(128, 'a'), std::string(256, 'a'), {}, "/", "/r/"));
  error_is(request_intent::create(std::string(129, 'a'), "actor", {}, "/w", "/r"), preparation_error_code::resource_limit);
  error_is(request_intent::create("id", std::string(257, 'a'), {}, "/w", "/r"), preparation_error_code::resource_limit);
}
TEST_CASE("acceptance prepared rejects malformed canonical manifest and retains original parsed ownership", "[acceptance]") {
  temporary t;
  inputs    i;
  auto manifest = std::ranges::find_if(i.stdlib.files, [](const auto& f) { return f.path == "__planar__/assembly.canonical"; });
  REQUIRE(manifest != i.stdlib.files.end());
  manifest->payload += 'x';
  error_is(i.prepare(t.capture(), paths), preparation_error_code::invalid_stdlib);
  inputs good;
  // Externally retained parsed pointers are not an input authority: the factory
  // independently decodes only the exact two canonical byte strings.
  good.interpretation.interpretation = {};
  good.interpretation.versions       = {};
  auto result                        = good.prepare(t.capture(), paths);
  REQUIRE(result);
  CHECK((*result->data())->interpretation.interpretation);
  CHECK((*result->data())->interpretation.versions);
}
TEST_CASE("acceptance prepared enforces source directory ceiling on genuine captured selections", "[acceptance]") {
  temporary                t;
  inputs                   i;
  std::vector<std::string> names;
  // Four private directory ancestors per file: 2048 files plus the root
  // exceed the 8192-directory source ceiling by one.
  for (int n = 0; n < 2048; ++n) {
    auto name = std::format("d{}/a/b/c/f", n);
    t.put("workspace/" + name, "");
    names.push_back(std::move(name));
  }
  std::vector<std::string_view> selected;
  for (auto& name : names)
    selected.push_back(name);
  t.put("workspace/d0/a/b/c/q/r/s/f", "");
  auto cap = t.capture();
  error_is(i.prepare(std::move(cap), selected), preparation_error_code::resource_limit);
  selected.pop_back();
  selected.push_back("d0/a/b/c/q/r/s/f"); // 8189 + three new directories = 8192.
  CHECK(i.prepare(t.capture(), selected));
}

TEST_CASE("workspace names and explicit targets frame the same pure future load request", "[prepared][workspace_selector]") {
  const auto named = expand_workspace("demo", "/not-present/other/../workspaces");
  REQUIRE(named);
  const auto named_path    = prepare_host_path(*named);
  const auto explicit_path = prepare_host_path("/not-present/workspaces/demo/./workspace.lua");
  REQUIRE(named_path);
  REQUIRE(explicit_path);
  CHECK(named_path->intent == explicit_path->intent);
  auto name_request = request_intent::create("request", "actor", {}, named_path->intent, "/not-present/registry.json");
  auto explicit_request =
      request_intent::create("another-request", "actor", {}, explicit_path->intent, "/not-present/registry.json");
  REQUIRE(name_request);
  REQUIRE(explicit_request);
  CHECK((*name_request->data())->encoded == (*explicit_request->data())->encoded);
  const auto changed_name = expand_workspace("demo", "/different-workspaces");
  REQUIRE(changed_name);
  const auto changed_path = prepare_host_path(*changed_name);
  REQUIRE(changed_path);
  auto changed = request_intent::create("request", "actor", {}, changed_path->intent, "/not-present/registry.json");
  REQUIRE(changed);
  CHECK((*name_request->data())->encoded != (*changed->data())->encoded);
  // This frames private native intent only: there is no public load or replay service.
}

TEST_CASE("acceptance request v2 owns exact normalized embedded and profile selectors", "[prepared][request_v2]") {
  auto embedded = request_intent::create_v2("key", "actor", {}, "/workspace", "/registry");
  auto profile  = request_intent::create_v2("key", "actor", {}, "/workspace", "/registry", "/private/profile.json");
  REQUIRE(embedded);
  REQUIRE(profile);
  auto e = embedded->data();
  auto p = profile->data();
  REQUIRE(e);
  REQUIRE(p);
  REQUIRE((**e).version2);
  REQUIRE((**p).version2);
  CHECK_FALSE((**e).version2->profile_argument);
  CHECK((**p).version2->profile_argument == "/private/profile.json");
  CHECK((**e).encoded == *encode_request_v2(*(**e).version2));
  CHECK((**p).encoded == *encode_request_v2(*(**p).version2));
  CHECK((**e).encoded != (**p).encoded);
  auto old = request_intent::create("key", "actor", {}, "/workspace", "/registry");
  REQUIRE(old);
  CHECK_FALSE((**old->data()).version2);
  CHECK((**old->data()).encoded != (**e).encoded);
  for (auto path : {"relative", "/a/../b", "/a/./b", ""})
    CHECK_FALSE(request_intent::create_v2("key", "actor", {}, "/workspace", "/registry", path));
  CHECK_FALSE(request_intent::create_v2("key", "actor", 0, "/workspace", "/registry"));
}
