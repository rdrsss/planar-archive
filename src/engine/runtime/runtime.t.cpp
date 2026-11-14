// Native trusted runtime boundary and separately embedded adversarial exports.
#include <catch2/catch_test_macros.hpp>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
import std;
import planar.engine.runtime;
import planar.engine.runtime_test;
import planar.engine.runtime_budget_test;
import planar.engine.runtime_tight_test;
import planar.engine.lua;
import planar.engine.hash;
import planar.engine.acceptance.intent;
import planar.engine.json_codec;
using namespace planar::engine;
namespace {
value_ptr text(std::string bytes) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = std::move(bytes);
  return v;
}
value_ptr object(std::map<std::string, value_ptr> fields = {}) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::record;
  v->fields = std::move(fields);
  return v;
}
auto test(std::string_view name, value_ptr input = object()) {
  return runtime_test::run_trusted(name, input);
}
void equal(const value_ptr& a, const value_ptr& b) {
  REQUIRE(a);
  REQUIRE(b);
  CHECK(a.get() != b.get());
  CHECK(a->kind == b->kind);
  CHECK(a->string == b->string);
  CHECK(a->integer == b->integer);
  CHECK(a->boolean == b->boolean);
  REQUIRE(a->fields.size() == b->fields.size());
  REQUIRE(a->items.size() == b->items.size());
  for (const auto& [key, child] : a->fields)
    equal(child, b->fields.at(key));
  for (std::size_t i = 0; i < a->items.size(); ++i)
    equal(a->items[i], b->items[i]);
}
} // namespace
TEST_CASE("runtime preserves private declaration tags and raw captured bytes", "[runtime]") {
  auto loaded = load_declaration(
      "return planar.spec {nested={}, list=planar.list {}, prose=planar.md('a'), file=planar.file('x.md'), "
      "note=planar.immaterial(planar.md('optional')), job=planar.job {version=1}, workspace=planar.workspace {version=1}}");
  REQUIRE(loaded);
  auto result = run_trusted("copy", loaded->root);
  REQUIRE(result);
  equal(loaded->root, *result);
  std::string raw("\0\xff\r\n", 4);
  auto        copied = run_trusted("copy", object({{"raw", text(raw)}, {"__kind", text("user key")}}));
  REQUIRE(copied);
  CHECK((*copied)->fields.at("raw")->string == raw);
  CHECK((*copied)->fields.at("__kind")->string == "user key");
  auto array = test("containers");
  REQUIRE(array);
  REQUIRE((*array)->kind == value_kind::list);
  REQUIRE((*array)->items.size() == 3);
  CHECK((*array)->items[0]->kind == value_kind::record);
  CHECK((*array)->items[1]->kind == value_kind::list);
  CHECK((*array)->items[2]->fields.at("list")->string == "ordinary user key");
  for (const auto& [key, child] : loaded->root->fields) {
    auto k = test("kinds", child);
    REQUIRE(k);
    CHECK_FALSE((*k)->string.empty());
  }
}
TEST_CASE("runtime mutations cannot affect captured native input", "[runtime]") {
  auto input   = object({{"nested", object({{"value", text("old")}})}});
  auto changed = test("mutate", input);
  REQUIRE(changed);
  CHECK((*changed)->fields.at("changed")->boolean);
  CHECK((*changed)->fields.at("nested")->fields.at("value")->string == "new");
  CHECK_FALSE(input->fields.contains("changed"));
  CHECK(input->fields.at("nested")->fields.at("value")->string == "old");
  auto copy = run_trusted("copy", input);
  REQUIRE(copy);
  equal(input, *copy);
}
TEST_CASE("runtime native hash primitives match direct canonical identities", "[runtime]") {
  auto containers = test("containers");
  REQUIRE(containers);
  auto request = object({{"domain", text("work_v1")}, {"value", *containers}});
  auto hashed  = run_trusted("hash_value", request);
  REQUIRE(hashed);
  REQUIRE(hash_value("work_v1", *containers));
  CHECK((*hashed)->string == *hash_value("work_v1", *containers));
  auto raw  = text(std::string("\0\xff", 2));
  auto file = run_trusted("file_digest", raw);
  REQUIRE(file);
  CHECK((*file)->string == *file_digest(raw->string));
  auto fragment = run_trusted("fragment_digest", text("# exact\r\n"));
  REQUIRE(fragment);
  CHECK((*fragment)->string == *fragment_digest("# exact\r\n"));
  auto invalid = run_trusted("fragment_digest", raw);
  REQUIRE_FALSE(invalid);
  CHECK(invalid.error().code == "schema");
  auto private_value = load_declaration("return planar.spec {}");
  REQUIRE(private_value);
  auto private_hash = run_trusted("hash_value", object({{"domain", text("work_v1")}, {"value", private_value->root}}));
  REQUIRE_FALSE(private_hash);
  CHECK(private_hash.error().code == "schema");
}
TEST_CASE("runtime states are fresh deterministic and separated from declarations", "[runtime]") {
  for (int i = 0; i < 3; ++i) {
    auto fresh = test("fresh");
    REQUIRE(fresh);
    CHECK((*fresh)->integer == 1);
    auto caps = test("capabilities");
    REQUIRE(caps);
    CHECK((*caps)->fields.at("missing")->boolean);
    CHECK((*caps)->fields.at("useful")->string == "ABCxyA");
    CHECK((*caps)->fields.at("integer")->integer == 2);
    auto denied = load_declaration("return engine.file_digest('a')");
    REQUIRE_FALSE(denied);
    CHECK(denied.error().code == "capability_denied");
    CHECK_FALSE(load_declaration("return math.max(1,2)"));
  }
  auto first  = load_declaration("return planar.job {id='first'}");
  auto second = load_declaration("return planar.job {id='second'}");
  REQUIRE(first);
  REQUIRE(second);
  for (auto input : {first->root, second->root, first->root}) {
    auto result = run_trusted("copy", input);
    REQUIRE(result);
    equal(input, *result);
  }
  CHECK_FALSE(run_trusted("capabilities", object()));
  CHECK_FALSE(run_trusted("../tests/stdlib/bridge.lua", object()));
  CHECK_FALSE(run_trusted("return {}", object()));
}
TEST_CASE("runtime refuses unsupported trusted returns and malformed native inputs", "[runtime]") {
  for (auto name : {"function", "float", "nil", "many", "cycle", "sparse", "mixed", "key", "meta"}) {
    INFO(name);
    auto result = test("malformed", text(name));
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "internal");
  }
  auto null = run_trusted("copy", nullptr);
  REQUIRE_FALSE(null);
  auto malformed  = std::make_shared<value>();
  malformed->kind = value_kind::list;
  malformed->fields.emplace("bad", text("x"));
  CHECK_FALSE(run_trusted("copy", malformed));
  auto cycle  = std::make_shared<value>();
  cycle->kind = value_kind::record;
  cycle->fields.emplace("cycle", cycle);
  CHECK_FALSE(run_trusted("copy", cycle));
  cycle->fields.clear();
  auto signed_value     = std::make_shared<value>();
  signed_value->kind    = value_kind::integer;
  signed_value->integer = std::numeric_limits<std::int64_t>::min();
  auto copied           = run_trusted("copy", signed_value);
  REQUIRE(copied);
  CHECK((*copied)->integer == signed_value->integer);
}
TEST_CASE("runtime bounds Lua execution allocation and native amplification", "[runtime]") {
  for (auto [entry, limit] : std::array{std::pair{"loop", "instructions"}, std::pair{"allocate", "lua_bytes"},
                                        std::pair{"depth", "depth"}, std::pair{"amplify", "native_bytes"}}) {
    INFO(entry);
    auto result = test(entry);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == "resource_limit");
    CHECK(result.error().limit == limit);
    CHECK(run_trusted("copy", object()));
  }
  auto deep = object();
  for (int i = 0; i < 257; ++i)
    deep = object({{"x", deep}});
  auto invalid = run_trusted("copy", deep);
  REQUIRE_FALSE(invalid);
  CHECK(invalid.error().limit == "depth");
  auto stack = test("stack");
  REQUIRE_FALSE(stack);
}
TEST_CASE("runtime retains rich stdlib diagnostics as structural data", "[runtime]") {
  auto result = test("diagnostic");
  REQUIRE(result);
  const auto& error = (*result)->fields.at("error")->fields;
  CHECK(error.at("field")->string == "job.roles");
  CHECK(error.at("file")->string == "job.lua");
  CHECK(error.at("details")->fields.at("expected")->string == "list");
}

TEST_CASE("runtime prevents finalizers from bypassing instruction limits", "[runtime][r07ac01]") {
  auto child = fork();
  REQUIRE(child >= 0);
  if (child == 0) {
    auto result = test("finalizer");
    _exit(result && (*result)->boolean ? 0 : 1);
  }
  int  status = 0;
  bool exited = false;
  for (int i = 0; i < 200; ++i) {
    if (waitpid(child, &status, WNOHANG) == child) {
      exited = true;
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  if (!exited) {
    kill(child, SIGKILL);
    waitpid(child, &status, 0);
  }
  REQUIRE(exited);
  REQUIRE(WIFEXITED(status));
  CHECK(WEXITSTATUS(status) == 0);
}
TEST_CASE("runtime repairs traversal state after caught conversion failures", "[runtime][r07ac01]") {
  auto repaired = test("repair");
  REQUIRE(repaired);
  CHECK((*repaired)->fields.empty());
  auto exhausted = test("cumulative");
  REQUIRE_FALSE(exhausted);
  CHECK(exhausted.error().code == "resource_limit");
  CHECK(exhausted.error().limit == "native_bytes");
}
TEST_CASE("runtime identifies the diagnostic actually escaping Lua", "[runtime][r07ac01]") {
  for (auto [mode, code, limit] :
       std::array{std::tuple{"loop", "resource_limit", "instructions"}, std::tuple{"memory", "resource_limit", "lua_bytes"},
                  std::tuple{"other", "internal", ""}, std::tuple{"old", "schema", ""}}) {
    INFO(mode);
    auto result = test("escaping", text(mode));
    REQUIRE_FALSE(result);
    CHECK(result.error().code == code);
    CHECK(result.error().limit == limit);
    if (std::string_view(mode) == "old")
      CHECK(result.error().message == "Invalid fragment text");
  }
  auto many = test("caught_many");
  REQUIRE(many);
  CHECK((*many)->boolean);
}
TEST_CASE("runtime enforces immaterial text payloads on both boundaries", "[runtime][r07ac01]") {
  auto invalid  = std::make_shared<value>();
  invalid->kind = value_kind::immaterial;
  auto boolean  = std::make_shared<value>();
  boolean->kind = value_kind::boolean;
  invalid->items.push_back(boolean);
  CHECK_FALSE(run_trusted("copy", invalid));
  for (auto constructor : {"md", "file"}) {
    auto loaded = load_declaration(std::format("return planar.spec {{note=planar.immaterial(planar.{}('x'))}}", constructor));
    REQUIRE(loaded);
    CHECK(run_trusted("copy", loaded->root));
    CHECK_FALSE(test("invalid_wrapper", loaded->root->fields.at("note")));
  }
}

TEST_CASE("runtime makes caught quota terminal before protected calls return", "[runtime][r07ac02]") {
  for (auto entry : {"caught_quota", "repeated_quota", "handler_quota", "nested_quota"}) {
    INFO(entry);
    auto child = fork();
    REQUIRE(child >= 0);
    if (child == 0) {
      auto result = test(entry);
      // xpcall is unsupported: its attempted call must fail promptly.
      bool expected = !result && (std::string_view(entry) == "handler_quota"
                                      ? result.error().code == "internal"
                                      : result.error().code == "resource_limit" && result.error().limit == "instructions");
      _exit(expected ? 0 : 1);
    }
    int  status = 0;
    bool exited = false;
    for (int i = 0; i < 300; ++i) {
      if (waitpid(child, &status, WNOHANG) == child) {
        exited = true;
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (!exited) {
      kill(child, SIGKILL);
      waitpid(child, &status, 0);
    }
    CHECK(exited);
    CHECK(WIFEXITED(status));
    if (WIFEXITED(status))
      CHECK(WEXITSTATUS(status) == 0);
  }
}
TEST_CASE("runtime preserves ordinary protected call arity errors and nesting", "[runtime][r07ac02]") {
  auto result = test("protected_calls");
  REQUIRE(result);
  REQUIRE((*result)->fields.size() == 7);
  for (const auto& [name, passed] : (*result)->fields) {
    INFO(name);
    CHECK(passed->boolean);
  }
}

TEST_CASE("runtime refuses native string pattern paths within a subprocess bound", "[runtime][r07ac03]") {
  for (auto path : {"find", "method_find", "match", "method_match", "gmatch", "method_gmatch", "gsub", "method_gsub"}) {
    INFO(path);
    auto child = fork();
    REQUIRE(child >= 0);
    if (child == 0) {
      auto result = test("native_pattern", text(path));
      _exit(!result && result.error().code == "internal" ? 0 : 1);
    }
    int  status = 0;
    bool exited = false;
    for (int i = 0; i < 400; ++i) {
      if (waitpid(child, &status, WNOHANG) == child) {
        exited = true;
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (!exited) {
      kill(child, SIGKILL);
      waitpid(child, &status, 0);
    }
    CHECK(exited);
    CHECK(WIFEXITED(status));
    if (WIFEXITED(status))
      CHECK(WEXITSTATUS(status) == 0);
  }
}
TEST_CASE("runtime retains deterministic nonpattern string operations", "[runtime][r07ac03]") {
  auto caps = test("capabilities");
  REQUIRE(caps);
  CHECK((*caps)->fields.at("missing")->boolean);
  auto result = test("strings");
  REQUIRE(result);
  REQUIRE((*result)->fields.size() == 7);
  for (const auto& [name, passed] : (*result)->fields) {
    INFO(name);
    CHECK(passed->boolean);
  }
}

TEST_CASE("runtime refuses native count paths within a subprocess bound", "[runtime][r07ac04]") {
  for (auto path : {"rep", "method_rep", "move"}) {
    INFO(path);
    auto child = fork();
    REQUIRE(child >= 0);
    if (child == 0) {
      auto result = test("native_count", text(path));
      _exit(!result && result.error().code == "internal" ? 0 : 1);
    }
    int  status = 0;
    bool exited = false;
    for (int i = 0; i < 400; ++i) {
      if (waitpid(child, &status, WNOHANG) == child) {
        exited = true;
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (!exited) {
      kill(child, SIGKILL);
      waitpid(child, &status, 0);
    }
    CHECK(exited);
    CHECK(WIFEXITED(status));
    if (WIFEXITED(status))
      CHECK(WEXITSTATUS(status) == 0);
  }
  auto caps = test("capabilities");
  REQUIRE(caps);
  CHECK((*caps)->fields.at("missing")->boolean);
}

TEST_CASE("runtime refuses native sparse table range helpers", "[runtime][r07ac05]") {
  for (auto path : {"insert", "remove", "sort"}) {
    INFO(path);
    auto result = test("native_table", text(path));
    CHECK_FALSE(result);
    if (!result)
      CHECK(result.error().code == "internal");
  }
}
TEST_CASE("runtime exposes only the selected bounded table helpers", "[runtime][r07ac05]") {
  auto result = test("tables");
  REQUIRE(result);
  REQUIRE((*result)->fields.size() == 6);
  for (const auto& [name, passed] : (*result)->fields) {
    INFO(name);
    CHECK(passed->boolean);
  }
}

TEST_CASE("runtime Glaze callbacks roundtrip structural data and ordinary failures", "[runtime][codec]") {
  auto result = test("json_roundtrip", text(R"({"é":[],"a":{"empty":{},"x":-2147483648},"b":false})"));
  REQUIRE(result);
  CHECK((*result)->fields.at("encoded")->string == R"({"a":{"empty":{},"x":-2147483648},"b":false,"é":[]})");
  CHECK((*result)->fields.at("value")->fields.at("é")->kind == value_kind::list);
  auto recovery = test("json_error_recovery");
  REQUIRE(recovery);
  for (auto key : {"ordinary", "recovered", "encode_ok", "value_absent", "old_token"})
    CHECK((*recovery)->fields.at(key)->boolean);
  CHECK((*recovery)->fields.at("code")->string == "schema");
  CHECK((*recovery)->fields.at("offset")->integer >= 1);
  CHECK((*recovery)->fields.at("encoded")->string == R"(["\n\t\r\b\f\u001F",true])");
  for (auto mode : {"cycle", "sparse"}) {
    auto invalid = test("json_encode_refusal", text(mode));
    REQUIRE(invalid);
    CHECK((*invalid)->fields.at("absent")->boolean);
    CHECK((*invalid)->fields.at("error")->fields.at("code")->string == "internal");
  }
}
TEST_CASE("runtime Glaze decode bounds remain ordinary typed data and declarations cannot call codecs", "[runtime][codec]") {
  for (auto input : {std::string(1048577, ' '), std::string(129, '[') + "0" + std::string(129, ']')}) {
    auto result = test("json_roundtrip", text(input));
    REQUIRE(result);
    CHECK((*result)->fields.at("error")->fields.at("code")->string == "resource_limit");
    CHECK((*result)->fields.at("error")->fields.at("details")->fields.at("maximum")->integer > 0);
  }
  for (auto input : {"return engine.json_decode('{}')", "return engine.json_encode({})"})
    CHECK_FALSE(load_declaration(input));
}

namespace {
struct native_history_fixture {
  std::filesystem::path root;
  native_history_fixture(bool v2 = false) {
    char pattern[] = "/tmp/planar-native-history-XXXXXX";
    REQUIRE(::mkdtemp(pattern));
    root = std::filesystem::canonical(pattern);
    std::filesystem::create_directory(root / "trusted");
    std::ofstream(root / "trusted/profile.json") << R"({"version":1,"modules":["module.lua"]})";
    std::ofstream(root / "trusted/module.lua") << R"(
registration.add({name='inspect',family='extension_read_v1',handler_id='custom.inspect',
help='inspect',response_kind='extension_v1',capabilities=engine.array{'history_read_v1'},
options=engine.array{
{name='--ledger',type='text',required=true,help='ledger'},
{name='--revision',type='history_id',required=false,help='revision'},
{name='--at-event',type='history_id',required=false,help='event'},
{name='--after-event',type='history_id',required=false,help='cursor'},
{name='--request-id',type='text',required=false,help='request'},
{name='--limit',type='decimal',required=false,help='limit'}
}},function() error('native history preparation must never execute Lua') end)
)";
    REQUIRE((v2 ? ledger::initialize_new_v2(root / "ledger", "demo") : ledger::initialize_new(root / "ledger", "demo")));
  }
  ~native_history_fixture() {
    std::filesystem::remove_all(root);
  }
  /// Extension owner adopted from an opaque profile_invocation factory (the factory budget stays attached). This is the
  /// extension frame after into_extension; the profile-owned prepare_load frame is covered by the known-load tests. The
  /// existing C4 tests use extension_invocation::create, which has no factory.
  extension_invocation profile_owner() {
    auto opaque = profile_invocation::open_opaque((root / "trusted/profile.json").string());
    REQUIRE(opaque);
    auto extension = std::move(*opaque).into_extension();
    REQUIRE(extension);
    return std::move(*extension);
  }
  extension_invocation owner() {
    auto capture = profile_capture::open((root / "trusted/profile.json").string());
    REQUIRE(capture);
    auto result = extension_invocation::create(std::move(*capture));
    REQUIRE(result);
    return std::move(*result);
  }
  void admit(extension_invocation& owner, std::vector<extension_parsed_option> options = {}) {
    auto ledger_path = (root / "ledger").string();
    options.insert(options.begin(), {"ledger", true, ledger_path});
    auto admitted = owner.admit("inspect", options, {});
    if (!admitted)
      INFO(admitted.error().detail.code << " " << admitted.error().detail.message << " " << admitted.error().field);
    REQUIRE(admitted);
  }
};
} // namespace
TEST_CASE("C4 B2 original history owner prepares both formats without Lua and releases reader locks", "[runtime][c4b2]") {
  for (bool v2 : {false, true}) {
    native_history_fixture f(v2);
    auto                   owner = f.owner();
    f.admit(owner);
    REQUIRE(owner.prepare_history());
    REQUIRE(owner.verify());
    auto writer = ledger::connection::open(f.root / "ledger", ledger::open_mode::read_write, 0);
    REQUIRE(writer);
    auto transaction = writer->begin();
    REQUIRE(transaction);
    REQUIRE(transaction->commit());
    auto again = owner.prepare_history();
    REQUIRE_FALSE(again);
    REQUIRE(std::holds_alternative<registration_error>(again.error()));
    CHECK(std::get<registration_error>(again.error()).detail.code == "internal");
    CHECK_FALSE(owner.execute());
  }
}
TEST_CASE("C4 B2 original history selectors preserve typed misses and original receipt priority", "[runtime][c4b2]") {
  for (auto option : {"revision", "at-event", "request-id"}) {
    native_history_fixture f;
    auto                   owner = f.owner();
    f.admit(owner, {{option, true, "2"}});
    auto result = owner.prepare_history();
    REQUIRE_FALSE(result);
    REQUIRE(std::holds_alternative<history::history_read_error>(result.error()));
    auto expected = std::string_view(option) == "revision"   ? history::history_read_code::revision_not_found
                    : std::string_view(option) == "at-event" ? history::history_read_code::event_not_found
                                                             : history::history_read_code::request_not_found;
    CHECK(std::get<history::history_read_error>(result.error()).code == expected);
    auto retry = owner.prepare_history();
    REQUIRE_FALSE(retry);
    CHECK(std::get<history::history_read_error>(retry.error()).code == expected);
  }
}
TEST_CASE("C4 B2 changed original profile or admitted ledger refuses before native preparation", "[runtime][c4b2]") {
  for (bool profile : {false, true}) {
    native_history_fixture f;
    auto                   owner = f.owner();
    f.admit(owner);
    std::ofstream(f.root / (profile ? "trusted/module.lua" : "ledger"), std::ios::app) << "changed";
    auto result = owner.prepare_history();
    REQUIRE_FALSE(result);
    REQUIRE(std::holds_alternative<registration_error>(result.error()));
    CHECK(std::get<registration_error>(result.error()).detail.code == "capture_changed");
    auto moved = std::move(owner);
    CHECK_FALSE(owner.prepare_history());
    auto retry = moved.prepare_history();
    REQUIRE_FALSE(retry);
    CHECK(std::get<registration_error>(retry.error()).detail.code == "capture_changed");
  }
}
TEST_CASE("C4 B2 appended admitted ledger refuses verify before any ledger read", "[runtime][c4b2]") {
  native_history_fixture f;
  auto                   owner = f.owner();
  f.admit(owner);
  std::ofstream(f.root / "ledger", std::ios::app) << "changed";
  auto checked = owner.verify();
  REQUIRE_FALSE(checked);
  CHECK(checked.error().detail.code == "capture_changed");
  CHECK(checked.error().detail.message == "Captured external profile changed");
  CHECK_FALSE(owner.verify());
}
TEST_CASE("C4 B2 invalid and moved history owners refuse and invalid ledger keeps typed format error", "[runtime][c4b2]") {
  native_history_fixture f;
  auto                   owner = f.owner();
  CHECK_FALSE(owner.prepare_history());
  std::array late{extension_parsed_option{"ledger", true, "unused"}};
  CHECK_FALSE(owner.admit("inspect", late, {}));
  auto rejected = f.owner();
  f.admit(rejected);
  auto original = rejected.execute();
  REQUIRE_FALSE(original);
  auto native = rejected.prepare_history();
  REQUIRE_FALSE(native);
  REQUIRE(std::holds_alternative<registration_error>(native.error()));
  CHECK(std::get<registration_error>(native.error()).detail.message ==
        std::get<registration_error>(original.error()).detail.message);
  auto retained = rejected.verify();
  REQUIRE_FALSE(retained);
  CHECK(retained.error().detail.message == std::get<registration_error>(original.error()).detail.message);
  auto second = f.owner();
  std::ofstream(f.root / "ledger", std::ios::trunc) << "bad header";
  f.admit(second);
  auto result = second.prepare_history();
  REQUIRE_FALSE(result);
  REQUIRE(std::holds_alternative<ledger::format_error>(result.error()));
  auto moved = std::move(second);
  CHECK_FALSE(second.prepare_history());
  auto retry = moved.prepare_history();
  REQUIRE_FALSE(retry);
  CHECK(std::get<ledger::format_error>(retry.error()).code == std::get<ledger::format_error>(result.error()).code);
}

namespace {
void history_handler(native_history_fixture& f, std::string_view body) {
  auto          path = f.root / "trusted/module.lua";
  std::ifstream input(path);
  std::string   source(std::istreambuf_iterator<char>{input}, {});
  auto          start = source.find("function() error(");
  if (start == std::string::npos)
    start = source.find("function(inv,ctx,...)");
  REQUIRE(start != std::string::npos);
  source.replace(start, std::string::npos, "function(inv,ctx,...) assert(select('#',...)==0); " + std::string(body) + " end)");
  std::ofstream(path) << source;
}
constexpr std::string_view history_success =
    "return {ok=true,value={version=1,kind='extension',command=inv.command,data={type='history',value=h}}}";
constexpr std::string_view history_semantic = "return {ok=false,error={code='schema',message='intentional',field='input'}}";
void history_protocol(const std::expected<value_ptr, extension_execution_error>& result, std::string_view code = "internal") {
  REQUIRE_FALSE(result);
  REQUIRE(std::holds_alternative<registration_error>(result.error()));
  CHECK(std::get<registration_error>(result.error()).detail.code == code);
}
} // namespace
TEST_CASE("C4 B3 actual retained history handler reads both formats and returns immutable native facts", "[runtime][c4b3]") {
  for (bool v2 : {false, true}) {
    native_history_fixture f(v2);
    history_handler(f, "assert(type(inv.options.ledger)=='userdata' and inv.present.ledger); "
                       "assert(type(ctx.read_history)=='function'); local h=ctx.read_history({}); "
                       "assert(h.kind=='history' and h.version==1 and h.last_event=='1'); "
                       "assert(h.data.selector=='page' and #h.data.events==1); "
                       "assert(not pcall(function() h.last_event='2' end)); "
                       "assert(not pcall(function() h.data.events[1].id='9' end)); " +
                           std::string(history_success));
    auto owner = f.owner();
    f.admit(owner);
    auto response = owner.execute();
    REQUIRE(response);
    CHECK((*response)->fields.at("value")->fields.at("data")->fields.at("value")->fields.at("last_event")->string == "1");
    auto writer = ledger::connection::open(f.root / "ledger", ledger::open_mode::read_write, 0);
    REQUIRE(writer);
    auto tx = writer->begin();
    REQUIRE(tx);
    REQUIRE(tx->commit());
    CHECK_FALSE(owner.execute());
  }
}
TEST_CASE("C4 B3 history is lazy and semantic responses need no content read", "[runtime][c4b3]") {
  native_history_fixture f;
  std::ofstream(f.root / "ledger", std::ios::trunc) << "invalid header admitted by metadata only";
  history_handler(f, history_semantic);
  auto owner = f.owner();
  f.admit(owner);
  auto response = owner.execute();
  REQUIRE(response);
  CHECK_FALSE((*response)->fields.at("ok")->boolean);
  history_handler(f, "local h=ctx.read_history({}); " + std::string(history_success));
  auto control = f.owner();
  f.admit(control);
  auto read = control.execute();
  REQUIRE_FALSE(read);
  CHECK(std::holds_alternative<ledger::format_error>(read.error()));
  auto retry = control.execute();
  REQUIRE_FALSE(retry);
  CHECK(std::get<ledger::format_error>(retry.error()).code == std::get<ledger::format_error>(read.error()).code);
}
TEST_CASE("C4 B3 ID strings absence presence and limit defaults reach actual retained handler", "[runtime][c4b3]") {
  for (auto name : {"revision", "at-event", "after-event"}) {
    for (auto id : {"", "1", "2147483647", "2147483648", "9223372036854775807"}) {
      native_history_fixture f;
      std::string assertion = "assert(type(inv.options['" + std::string(name) + "'])=='string' and inv.options['" + name +
                              "']=='" + (std::string_view(id).empty() ? "0" : id) + "' and inv.present['" + name +
                              "']==" + (std::string_view(id).empty() ? "false" : "true") + "); ";
      history_handler(f, assertion +
                             "assert(inv.options.limit==0 and math.type(inv.options.limit)=='integer' and "
                             "not inv.present.limit); assert(inv.options['request-id']=='' and "
                             "not inv.present['request-id']); " +
                             std::string(history_semantic));
      auto owner = f.owner();
      f.admit(owner, std::string_view(id).empty() ? std::vector<extension_parsed_option>{}
                                                  : std::vector<extension_parsed_option>{{name, true, id}});
      REQUIRE(owner.execute());
    }
    for (auto id : {"0", "00", "01", "+1", "-1", "1.0", "1e0", "", " ", "9223372036854775808"}) {
      native_history_fixture f;
      auto                   owner = f.owner();
      auto                   path  = (f.root / "ledger").string();
      std::array             options{extension_parsed_option{"ledger", true, path}, extension_parsed_option{name, true, id}};
      auto                   admitted = owner.admit("inspect", options, {});
      REQUIRE_FALSE(admitted);
      CHECK(admitted.error().detail.code == "usage");
    }
  }
}
TEST_CASE("C4 B3 page cursor and explicit limit reach actual service with exact ID ABI", "[runtime][c4b3]") {
  for (auto limit : {"1", "100"}) {
    native_history_fixture f;
    history_handler(f, "assert(inv.options['after-event']=='1' and inv.present['after-event']); "
                       "assert(math.type(inv.options.limit)=='integer' and inv.present.limit); "
                       "local h=ctx.read_history({}); assert(#h.data.events==0); " +
                           std::string(history_success));
    auto owner = f.owner();
    f.admit(owner, {{"after-event", true, "1"}, {"limit", true, limit}});
    REQUIRE(owner.execute());
  }
  for (auto limit : {"0", "101", "2147483647", "2147483648", "9223372036854775807"}) {
    native_history_fixture f;
    auto                   owner = f.owner();
    auto                   path  = (f.root / "ledger").string();
    std::array             options{extension_parsed_option{"ledger", true, path}, extension_parsed_option{"limit", true, limit}};
    auto                   admitted = owner.admit("inspect", options, {});
    REQUIRE_FALSE(admitted);
    CHECK(admitted.error().detail.code == "usage");
  }
}
TEST_CASE("C4 B3 caught argument repeated and missing service attempts poison invocation", "[runtime][c4b3]") {
  for (auto attempt : {"ctx.read_history()", "ctx.read_history({}, {})", "ctx.read_history({limit=1})",
                       "ctx.read_history({inv.options.ledger})", "ctx.read_history(inv.options.ledger)",
                       "ctx.read_history({}); ctx.read_history({})", "ctx.read_role({})", "ctx.project({id='x'})"}) {
    native_history_fixture f;
    history_handler(f, "pcall(function() " + std::string(attempt) + " end); " + std::string(history_semantic));
    auto owner = f.owner();
    f.admit(owner);
    history_protocol(owner.execute());
    history_protocol(owner.execute());
  }
}
TEST_CASE("C4 B3 exact history correspondence permits copies and refuses fabricated summaries or facts", "[runtime][c4b3]") {
  for (auto body : {"local h={}; ",
                    "local h=ctx.read_history({}); local r={}; for k,v in pairs(h) do r[k]=v end; "
                    "r.last_event='2'; h=r; ",
                    "local h=ctx.read_history({}); local r={}; for k,v in pairs(h) do r[k]=v end; r.extra=true; h=r; "}) {
    native_history_fixture f;
    history_handler(f, std::string(body) + std::string(history_success));
    auto owner = f.owner();
    f.admit(owner);
    history_protocol(owner.execute());
  }
  native_history_fixture f;
  history_handler(f, "local h=ctx.read_history({}); local r={}; for k,v in pairs(h) do r[k]=v end; h=r; " +
                         std::string(history_success));
  auto owner = f.owner();
  f.admit(owner);
  REQUIRE(owner.execute());
  history_handler(f, "return {ok=true,value={version=1,kind='extension',command=inv.command,data={type='summary',text='x'}}}");
  auto summary = f.owner();
  f.admit(summary);
  history_protocol(summary.execute());
}
TEST_CASE("C4 B3 original typed service failures survive Lua catches later errors moves and retries", "[runtime][c4b3]") {
  for (auto tail : {history_semantic, std::string_view("error('later')"), history_success}) {
    native_history_fixture f;
    history_handler(f, "pcall(function() ctx.read_history({}) end); " + std::string(tail));
    auto owner = f.owner();
    f.admit(owner, {{"revision", true, "9223372036854775807"}});
    auto result = owner.execute();
    REQUIRE_FALSE(result);
    REQUIRE(std::holds_alternative<history::history_read_error>(result.error()));
    CHECK(std::get<history::history_read_error>(result.error()).code == history::history_read_code::revision_not_found);
    auto moved = std::move(owner);
    auto retry = moved.execute();
    REQUIRE_FALSE(retry);
    CHECK(std::get<history::history_read_error>(retry.error()).code == history::history_read_code::revision_not_found);
  }
}
TEST_CASE("C4 B3 manual native preparation cannot bypass once-only service and original receipt verification",
          "[runtime][c4b3]") {
  native_history_fixture f;
  history_handler(f, "local h=ctx.read_history({}); " + std::string(history_success));
  auto owner = f.owner();
  f.admit(owner);
  REQUIRE(owner.prepare_history());
  history_protocol(owner.execute());
  for (auto file : {"ledger", "trusted/module.lua"}) {
    native_history_fixture changed;
    history_handler(changed, "while true do end");
    auto captured = changed.owner();
    changed.admit(captured);
    std::ofstream(changed.root / file, std::ios::app) << "changed";
    history_protocol(captured.execute(), "capture_changed");
  }
}
TEST_CASE("C4 B3 opaque ledger cannot stringify serialize substitute or expose host path", "[runtime][c4b3]") {
  for (auto body : {"local h=inv.options.ledger.path; ", "local h=string.format('%s',inv.options.ledger); ",
                    "inv.options.ledger='replacement'; ", "local h=ctx.read_history({ledger=inv.options.ledger}); "}) {
    native_history_fixture f;
    history_handler(f, std::string(body) + std::string(history_semantic));
    auto owner = f.owner();
    f.admit(owner);
    history_protocol(owner.execute());
  }
}
TEST_CASE("C4 B3 caught completed Lua memory failure after native service remains terminal", "[runtime][c4b3]") {
  native_history_fixture f;
  history_handler(f, "local h=ctx.read_history({}); pcall(function() local x='a'; while true do x=x..x end end); " +
                         std::string(history_success));
  auto owner = f.owner();
  f.admit(owner);
  history_protocol(owner.execute(), "resource_limit");
  history_protocol(owner.execute(), "resource_limit");
}
TEST_CASE("C4 B3 history selector conflicts refuse before handler regardless of ID magnitude", "[runtime][c4b3]") {
  for (auto id : {"1", "9223372036854775807"}) {
    for (auto other : {"at-event", "after-event", "limit", "request-id"}) {
      native_history_fixture f;
      history_handler(f, "while true do end");
      auto       owner = f.owner();
      auto       path  = (f.root / "ledger").string();
      std::array options{extension_parsed_option{"ledger", true, path}, extension_parsed_option{"revision", true, id},
                         extension_parsed_option{other, true, "1"}};
      auto       admitted = owner.admit("inspect", options, {});
      REQUIRE_FALSE(admitted);
      CHECK(admitted.error().detail.code == "usage");
    }
  }
}

TEST_CASE("C4 C1 retained projection frame preserves aliased input across sequential calls", "[runtime][projection-frame]") {
  auto profile     = std::make_shared<profile_closure>();
  profile->profile = {"profile", "profile.json", "0644", R"({"version":1,"modules":["projection.lua"]})"};
  profile->modules.push_back({"modules", "projection.lua", "0644", R"(
registration.projection({id='frame',abi='projection_v1',revision='r1',input='history_snapshot_v1',help='frame'},
function(input)
 assert(input.left==input.right and input.left.text=='retained')
 assert(not pcall(function() input.left.text='changed' end))
 return {version=1,sections=engine.array{{id='frame_section',text=input.right.text,mandatory=false}}}
end))"});
  auto vm = command_vm::create(profile);
  REQUIRE(vm);
  auto shared = object({{"text", text("retained")}});
  auto input  = object({{"left", shared}, {"right", shared}});
  for (int i = 0; i != 3; ++i) {
    auto result = vm->invoke_projection("frame", projection_input_kind::history_snapshot_v1, input);
    REQUIRE(result);
    CHECK(result->result->fields.at("sections")->items.at(0)->fields.at("text")->string == "retained");
    CHECK(shared->fields.at("text")->string == "retained");
  }
}
// CL-1: a caught illegal registration is the first terminal failure; a later exhaustion inside the same retained projection
// call must not replace it with resource_limit.
TEST_CASE("CL-1 retained projection keeps a caught illegal registration over later exhaustion",
          "[runtime][projection-frame][cl1]") {
  for (std::string tail : {"", "while true do end", "local t={} while true do t[#t+1]={} end"}) {
    INFO(tail);
    auto profile     = std::make_shared<profile_closure>();
    profile->profile = {"profile", "profile.json", "0644", R"({"version":1,"modules":["projection.lua"]})"};
    profile->modules.push_back(
        {"modules", "projection.lua", "0644",
         "local saved=registration.projection; "
         "local P={id='frame',abi='projection_v1',revision='r1',input='history_snapshot_v1',help='frame'}; "
         "registration.projection(P,function(input) assert(not pcall(saved,P,function()end)); " +
             tail + " return {version=1,sections=engine.array{}} end)"});
    auto vm = command_vm::create(profile);
    REQUIRE(vm);
    auto result = vm->invoke_projection("frame", projection_input_kind::history_snapshot_v1, object());
    REQUIRE_FALSE(result);
    INFO(result.error().detail.code << " " << result.error().detail.message << " " << result.error().detail.limit);
    CHECK(result.error().detail.code == "internal");
    CHECK(result.error().detail.message == "Invalid external profile protocol");
    CHECK(result.error().detail.limit.empty());
  }
}

namespace {
void projection_diagnostic(const std::expected<value_ptr, extension_execution_error>& r) {
  if (!r && std::holds_alternative<registration_error>(r.error())) {
    const auto& e = std::get<registration_error>(r.error());
    WARN(e.detail.code << " " << e.detail.message << " " << e.field);
  }
}
constexpr std::string_view projection_success = "return {ok=true,value={version=1,kind='extension',command=inv.command,data=p}}";
void history_projection(native_history_fixture& f, std::string_view body,
                        std::string_view projection =
                            "return {version=1,sections=engine.array{{id='custom',text='facts '..h.last_event,mandatory=false}}}",
                        std::string_view kind = "history_snapshot_v1") {
  history_handler(f, body);
  auto          path = f.root / "trusted/module.lua";
  std::ifstream input(path);
  std::string   source(std::istreambuf_iterator<char>{input}, {});
  auto          at = source.find("engine.array{'history_read_v1'}");
  REQUIRE(at != std::string::npos);
  source.replace(at, std::string("engine.array{'history_read_v1'}").size(),
                 "engine.array{'history_read_v1','projection_apply_v1'}");
  std::ofstream(path) << "registration.projection({id='custom',abi='projection_v1',revision='r1',input='" << kind
                      << "',help='history'},function(h) " << projection << " end)\n"
                      << source;
}
} // namespace
TEST_CASE("C4 C2 real original history projection and trusted assembly run in both formats", "[runtime][c4c2]") {
  for (bool v2 : {false, true}) {
    native_history_fixture f(v2);
    history_projection(f,
                       "local n=0; for k,v in pairs(ctx) do assert(k=='read_history' or k=='project'); n=n+1 end; "
                       "assert(n==2 and rawget(ctx,'project')==ctx.project); "
                       "local h=ctx.read_history({}); local p=ctx.project({id='custom'}); "
                       "assert(p.type=='projection' and p.id=='custom' and p.revision=='r1' and #p.definition==41); "
                       "assert(not pcall(function() p.text='forged' end)); "
                       "assert(not pcall(function() p.sections[1].bytes=0 end)); " +
                           std::string(projection_success),
                       "assert(h.kind=='history' and #h.ledger==32 and h.last_event=='1'); "
                       "assert(not pcall(function() h.last_event='2' end)); "
                       "return {version=1,sections=engine.array{{id='custom',text='facts '..h.last_event,mandatory=false}}}");
    auto owner = f.owner();
    f.admit(owner);
    auto result = owner.execute();
    projection_diagnostic(result);
    REQUIRE(result);
    REQUIRE((*result)->fields.at("ok")->boolean);
    auto data = (*result)->fields.at("value")->fields.at("data");
    auto file = ledger::open_existing(f.root / "ledger");
    REQUIRE(file);
    CHECK(data->fields.at("text")->string == std::format("## recorded_history\n\nRecorded history; ledger {}; last event 1. No "
                                                         "current-policy reinterpretation.\n\n## custom\n\nfacts 1",
                                                         file->metadata().identity));
    const auto& sections = data->fields.at("sections")->items;
    REQUIRE(sections.size() == 2);
    CHECK(sections[0]->fields.at("id")->string == "recorded_history");
    CHECK(sections[0]->fields.at("included")->boolean);
    CHECK(data->fields.at("omitted")->items.empty());
    CHECK_FALSE(owner.execute());
  }
}
TEST_CASE("C4 C2 services are lazy and performed history remains a permitted response", "[runtime][c4c2]") {
  for (auto body : {std::string(history_semantic), "local h=ctx.read_history({}); " + std::string(history_success),
                    "local h=ctx.read_history({}); local p=ctx.project({id='custom'}); " + std::string(history_success)}) {
    native_history_fixture f;
    history_projection(f, body);
    if (body == history_semantic)
      std::ofstream(f.root / "ledger", std::ios::trunc) << "not SQLite";
    auto owner = f.owner();
    f.admit(owner);
    auto result = owner.execute();
    projection_diagnostic(result);
    REQUIRE(result);
    CHECK((*result)->fields.at("ok")->boolean == (body != history_semantic));
  }
}
TEST_CASE("C4 C2 project ordering exact arguments once-only and missing methods stay terminal", "[runtime][c4c2]") {
  for (auto attempt :
       {"ctx.project({id='custom'})", "ctx.read_history({}); ctx.project({})",
        "ctx.read_history({}); ctx.project({id='custom',input={}})", "ctx.read_history({}); ctx.project({id='custom'}, {})",
        "ctx.read_history({}); ctx.project({id=1})", "ctx.read_history({}); ctx.project(function() end)",
        "ctx.read_history({}); ctx.project({id='unknown'})",
        "ctx.read_history({}); ctx.project({id='custom'}); ctx.project({id='custom'})",
        "ctx.read_history({}); ctx.project({id='custom'}); ctx.read_history({})", "local missing=rawget(ctx,'read_role')"}) {
    INFO(attempt);
    native_history_fixture f;
    history_projection(f, "pcall(function() " + std::string(attempt) + " end); " + std::string(history_semantic));
    auto owner = f.owner();
    f.admit(owner);
    history_protocol(owner.execute());
  }
}
TEST_CASE("C4 C2 projection input kind and fabricated DATA refuse", "[runtime][c4c2]") {
  for (auto kind : {"role_context_v1", "history_snapshot_v1"}) {
    native_history_fixture f;
    history_projection(
        f,
        "local h=ctx.read_history({}); local p=ctx.project({id='custom'}); "
        "local copy={}; for k,v in pairs(p) do copy[k]=v end; copy.text='forged'; p=copy; " +
            std::string(projection_success),
        std::string_view(kind) == "role_context_v1" ? "while true do end" : "return {version=1,sections=engine.array{}}", kind);
    auto owner = f.owner();
    f.admit(owner);
    history_protocol(owner.execute());
  }
  native_history_fixture f;
  history_projection(f, "local h=ctx.read_history({}); local "
                        "p={type='projection',id='custom',revision='r1',definition='fake',text='',sections=engine.array{},"
                        "omitted=engine.array{}}; " +
                            std::string(projection_success));
  auto owner = f.owner();
  f.admit(owner);
  history_protocol(owner.execute());
}
TEST_CASE("C4 C2 one MiB assembly reverses optional removals and preserves metadata", "[runtime][c4c2]") {
  native_history_fixture f;
  history_projection(
      f, "local h=ctx.read_history({}); local p=ctx.project({id='custom'}); " + std::string(projection_success),
      "local x='x'; for i=1,19 do x=x..x end; return {version=1,sections=engine.array{"
      "{id='first',text=x,mandatory=false},{id='second',text=x,mandatory=false},{id='third',text='tail',mandatory=false}}}");
  auto owner = f.owner();
  f.admit(owner);
  auto result = owner.execute();
  projection_diagnostic(result);
  REQUIRE(result);
  auto data = (*result)->fields.at("value")->fields.at("data");
  CHECK(data->fields.at("text")->string.size() <= 1048576);
  const auto& omitted = data->fields.at("omitted")->items;
  REQUIRE(omitted.size() == 2);
  CHECK(omitted[0]->string == "third");
  CHECK(omitted[1]->string == "second");
  const auto& sections = data->fields.at("sections")->items;
  REQUIRE(sections.size() == 5);
  CHECK(sections[1]->fields.at("included")->boolean);
  CHECK_FALSE(sections[2]->fields.at("included")->boolean);
  CHECK(sections.back()->fields.at("id")->string == "omissions");
  CHECK(data->fields.at("text")->string.ends_with("## omissions\n\nOptional sections omitted: third, second.\n"));
}
TEST_CASE("C4 C2 trusted budget E preserves exact details through catches moves and retries", "[runtime][c4c2]") {
  native_history_fixture f;
  history_projection(
      f, "local h=ctx.read_history({}); pcall(function() ctx.project({id='custom'}) end); " + std::string(history_semantic),
      "local x='x'; for i=1,20 do x=x..x end; return {version=1,sections=engine.array{{id='required',text=x,mandatory=true}}}");
  auto owner = f.owner();
  f.admit(owner);
  auto result = owner.execute();
  projection_diagnostic(result);
  REQUIRE_FALSE(result);
  REQUIRE(std::holds_alternative<assembly_error>(result.error()));
  auto e = std::get<assembly_error>(result.error()).error;
  CHECK(e->fields.at("code")->string == "budget_exceeded");
  auto details = e->fields.at("details");
  CHECK(details->fields.at("budget_bytes")->integer == 1048576);
  CHECK(details->fields.at("mandatory_bytes")->integer > 1048576);
  REQUIRE(details->fields.at("sections")->items.size() == 2);
  auto moved = std::move(owner);
  CHECK_FALSE(owner.execute());
  CHECK_FALSE(moved.verify());
  CHECK_FALSE(moved.prepare_history());
  CHECK_FALSE(moved.admit("inspect", {}, {}));
  auto retry = moved.execute();
  REQUIRE_FALSE(retry);
  REQUIRE(std::holds_alternative<assembly_error>(retry.error()));
  CHECK(std::get<assembly_error>(retry.error()).error == e);
}
TEST_CASE("C4 C2 original native history failure precedes projection and later handler errors", "[runtime][c4c2]") {
  native_history_fixture f;
  history_projection(
      f, "pcall(function() ctx.read_history({}) end); pcall(function() ctx.project({id='custom'}) end); error('later')");
  std::ofstream(f.root / "ledger", std::ios::trunc) << "bad header";
  auto owner = f.owner();
  f.admit(owner);
  auto result = owner.execute();
  projection_diagnostic(result);
  REQUIRE_FALSE(result);
  CHECK(std::holds_alternative<ledger::format_error>(result.error()));
}
TEST_CASE("C4 C2 raw projection admission refuses malformed reserved and oversized retained output", "[runtime][c4c2]") {
  for (auto source :
       {"return {version=1,sections=engine.array{{id='recorded_history',text='x',mandatory=false}}}",
        "return {version=1,sections=engine.array{{id='custom',text='x',mandatory=false,extra=1}}}",
        "local s={}; for i=1,65 do s[i]={id='s'..i,text='x',mandatory=false} end; return {version=1,sections=engine.array(s)}",
        "local x='x'; for i=1,20 do x=x..x end; return "
        "{version=1,sections=engine.array{{id='custom',text=x..'x',mandatory=false}}}",
        "return {version=1,sections=engine.array{}},{}"}) {
    native_history_fixture f;
    history_projection(
        f, "local h=ctx.read_history({}); pcall(function() ctx.project({id='custom'}) end); " + std::string(history_semantic),
        source);
    auto owner = f.owner();
    f.admit(owner);
    auto result = owner.execute();
    REQUIRE_FALSE(result);
    REQUIRE(std::holds_alternative<registration_error>(result.error()));
    auto code = std::get<registration_error>(result.error()).detail.code;
    CHECK((code == "internal" || code == "resource_limit"));
    CHECK_FALSE(owner.execute());
  }
}

TEST_CASE("C4 C2 first service protocol refusal survives later instruction exhaustion", "[runtime][c4c2]") {
  native_history_fixture f;
  history_projection(f, "pcall(function() ctx.project({id='custom'}) end); while true do end");
  auto owner = f.owner();
  f.admit(owner);
  history_protocol(owner.execute());
}
// CL-1 through the extension projection callback: the projection catches its own illegal registration and then exhausts; the
// retained projection call must report the caught registration failure, not resource_limit.
TEST_CASE("CL-1 extension projection callback keeps a caught illegal registration over later exhaustion",
          "[runtime][c4c2][cl1]") {
  for (std::string tail : {"", "while true do end", "local t={} while true do t[#t+1]={} end"}) {
    INFO(tail);
    native_history_fixture f;
    history_projection(f, "local h=ctx.read_history({}); local p=ctx.project({id='custom'}); " + std::string(projection_success),
                       "assert(not pcall(registration.projection,{},function()end)); " + tail +
                           " return {version=1,sections=engine.array{}}");
    auto owner = f.owner();
    f.admit(owner);
    auto result = owner.execute();
    projection_diagnostic(result);
    REQUIRE_FALSE(result);
    REQUIRE(std::holds_alternative<registration_error>(result.error()));
    const auto& error = std::get<registration_error>(result.error());
    CHECK(error.detail.code == "internal");
    CHECK(error.detail.message == "Invalid external profile protocol");
    CHECK(error.detail.limit.empty());
  }
}

extern "C" void          planar_admission_watch(std::uint64_t device, std::uint64_t inode);
extern "C" std::uint64_t planar_admission_read_attempts();
namespace {
template <class T>
concept has_prepare_load = requires(T& p, const acceptance::request_intent& i) { p.prepare_load(i); };
template <class T>
concept can_prepare_load_rvalue = requires(T&& p, const acceptance::request_intent& i) { std::move(p).prepare_load(i); };
template <class T>
concept has_begin_frame = requires(T& p) { p.begin_frame(); } || requires(T& p, int i) { p.begin_frame(i); };
template <class T>
concept has_stage_access = requires(T& p) { p.data(); } || requires(T& p) { p.bundle(); } || requires(T& p) { p.model(); } ||
                           requires(T& p) { p.vm(); } || requires(T& p) { p.capture(); } || requires(T& p) { p.source(); };
template <class T>
concept has_prepared = requires(T& p) { p.prepared(); };
template <class T>
concept has_vm_accessor = requires(T& p) { p.vm(); };
template <class T>
concept has_upgrade = requires(T& p) { p.upgrade(); };
template <class... A>
concept can_open_known = requires(A&&... a) { profile_invocation::open_known_load(std::forward<A>(a)...); };
template <class... A>
concept can_open_opaque = requires(A&&... a) { profile_invocation::open_opaque(std::forward<A>(a)...); };
static_assert(!std::is_copy_constructible_v<profile_invocation>);
static_assert(!std::is_copy_assignable_v<profile_invocation>);
static_assert(!std::is_default_constructible_v<profile_invocation>);
static_assert(std::is_nothrow_move_constructible_v<profile_invocation>);
static_assert(std::is_nothrow_move_assignable_v<profile_invocation>);
static_assert(can_open_known<const acceptance::request_intent&, std::string_view>);
static_assert(!can_open_known<const acceptance::request_intent&>);
static_assert(!can_open_known<const acceptance::request_intent&, std::string_view, std::string_view>);
static_assert(!can_open_known<std::string_view, std::string_view>);
static_assert(can_open_opaque<std::string_view>);
static_assert(!can_open_opaque<std::string_view, std::string_view>);
static_assert(!can_open_opaque<const acceptance::request_intent&, std::string_view>);
static_assert(!has_prepare_load<profile_invocation>);
static_assert(can_prepare_load_rvalue<profile_invocation>);
static_assert(!has_begin_frame<profile_invocation>);
static_assert(!std::is_copy_constructible_v<profile_load_stage>);
static_assert(!std::is_copy_assignable_v<profile_load_stage>);
static_assert(!std::is_default_constructible_v<profile_load_stage>);
static_assert(std::is_nothrow_move_constructible_v<profile_load_stage>);
static_assert(std::is_nothrow_move_assignable_v<profile_load_stage>);
static_assert(!has_stage_access<profile_load_stage>);
static_assert(!std::is_constructible_v<profile_load_stage, std::unique_ptr<int>>);
static_assert(!has_prepared<profile_invocation>);
static_assert(!has_vm_accessor<profile_invocation>);
static_assert(!has_upgrade<profile_invocation>);
template <class T>
concept has_data_member = requires(T& p) { p.data(); };
template <class T>
concept has_catalog_member = requires(T& p) { p.catalog(); };
template <class... A>
concept can_prepare_with = requires(profile_invocation&& o, A&&... a) { std::move(o).prepare_load(std::forward<A>(a)...); };
static_assert(!std::is_copy_constructible_v<prepared_profile>);
static_assert(!std::is_copy_assignable_v<prepared_profile>);
static_assert(!std::is_default_constructible_v<prepared_profile>);
static_assert(std::is_nothrow_move_constructible_v<prepared_profile>);
static_assert(std::is_nothrow_move_assignable_v<prepared_profile>);
static_assert(!std::is_constructible_v<prepared_profile, std::unique_ptr<int>>);
static_assert(!std::is_constructible_v<prepared_profile, profile_acceptance_data>);
static_assert(!std::is_constructible_v<prepared_profile, const profile_acceptance_data&>);
static_assert(!std::is_constructible_v<prepared_profile, std::shared_ptr<const profile_acceptance_data>>);
static_assert(!std::is_constructible_v<prepared_profile, acceptance::request_data>);
static_assert(!std::is_constructible_v<prepared_profile, value_ptr>);
static_assert(!std::is_constructible_v<prepared_profile, std::string>);
static_assert(!std::is_constructible_v<prepared_profile, profile_load_stage>);
static_assert(!has_upgrade<prepared_profile>);
static_assert(!has_vm_accessor<prepared_profile>);
static_assert(!has_catalog_member<prepared_profile>);
static_assert(has_data_member<prepared_profile>);
static_assert(std::is_same_v<decltype(std::declval<const prepared_profile&>().data()),
                             std::expected<std::shared_ptr<const profile_acceptance_data>, registration_error>>);
static_assert(std::is_same_v<decltype(std::declval<const prepared_profile&>().bound_intent()),
                             std::shared_ptr<const acceptance::request_data>>);
static_assert(
    std::is_same_v<decltype(std::declval<const prepared_profile&>().verify_native_receipts()), std::expected<void, diagnostic>>);
static_assert(
    std::is_same_v<decltype(std::declval<profile_invocation&&>().prepare_load(std::declval<const acceptance::request_intent&>())),
                   std::expected<prepared_profile, profile_preparation_error>>);
// No prepare_load overload takes a caller bundle, model, tree, acceptance data, stage or prepared object.
static_assert(can_prepare_with<const acceptance::request_intent&>);
static_assert(!can_prepare_with<>);
static_assert(!can_prepare_with<value_ptr>);
static_assert(!can_prepare_with<std::string>);
static_assert(!can_prepare_with<std::vector<history::tree_file>>);
static_assert(!can_prepare_with<history::prepared_provenance>);
static_assert(!can_prepare_with<profile_acceptance_data>);
static_assert(!can_prepare_with<std::shared_ptr<const acceptance::request_data>>);
static_assert(!can_prepare_with<profile_load_stage>);
static_assert(!can_prepare_with<prepared_profile>);
static_assert(!can_prepare_with<const acceptance::request_intent&, value_ptr>);
static_assert(!can_prepare_with<const acceptance::request_intent&, value_ptr, value_ptr>);
static_assert(!can_prepare_with<const acceptance::request_intent&, value_ptr, value_ptr, std::vector<history::tree_file>>);
static_assert(!can_prepare_with<value_ptr, value_ptr>);
static_assert(!can_prepare_with<value_ptr, value_ptr, std::vector<history::tree_file>>);

void watch_path(const std::filesystem::path& p) {
  struct stat s{};
  REQUIRE(::stat(p.c_str(), &s) == 0);
  planar_admission_watch(s.st_dev, s.st_ino);
}
void put(const std::filesystem::path& p, std::string_view bytes) {
  std::ofstream f(p, std::ios::binary | std::ios::trunc);
  f << bytes;
  REQUIRE(f.good());
}
struct b3a_fixture {
  mutable native_history_fixture inner;
  std::filesystem::path          root, workspace_dir, declaration, registry, ledger, profile, module;
  b3a_fixture() : inner(false), root(inner.root) {
    workspace_dir = root / "workspace";
    declaration   = workspace_dir / "workspace.lua";
    registry      = root / "registry.json";
    ledger        = root / "ledger";
    profile       = root / "trusted/profile.json";
    module        = root / "trusted/module.lua";
    std::filesystem::create_directory(workspace_dir);
    put(declaration, "-- never read by known load admission");
    put(registry, "{}");
  }
  acceptance::request_intent intent() const {
    auto r = acceptance::request_intent::create_v2("req-1", "actor", std::nullopt, declaration.string(), registry.string(),
                                                   profile.string());
    REQUIRE(r);
    return std::move(*r);
  }
  std::expected<profile_invocation, registration_error> open(std::string_view ledger_path = {}) const {
    auto i = intent();
    return profile_invocation::open_known_load(i, ledger_path.empty() ? ledger.string() : std::string(ledger_path));
  }
  profile_invocation clean() const {
    auto o = open();
    if (!o)
      INFO(o.error().detail.code << " " << o.error().detail.message << " " << o.error().field);
    REQUIRE(o);
    return std::move(*o);
  }
  void admit(extension_invocation& owner) const {
    inner.admit(owner);
  }
};
void expect_denied(const std::expected<profile_invocation, registration_error>& r, std::string_view field = "profile") {
  REQUIRE_FALSE(r);
  CHECK(r.error().detail.code == "unsafe_path");
  CHECK(r.error().field == field);
}
} // namespace
TEST_CASE("X06 B3 A profile invocation static surface offers no prepare, vm or upgrade access", "[runtime][x06b3a]") {
  SUCCEED("compile-time properties asserted at namespace scope");
}
TEST_CASE("X06 B3 A known load opens once with original receipts and no frame", "[runtime][x06b3a]") {
  b3a_fixture f;
  auto        owner = f.clean();
  auto        seen  = owner.observe();
  CHECK(seen.live);
  CHECK(seen.known_load);
  CHECK_FALSE(seen.frame_begun);
  CHECK_FALSE(seen.consumed);
  CHECK_FALSE(owner.effective_catalog().empty());
  CHECK(owner.catalog());
}
TEST_CASE("X06 B3 A known load reads no declaration or registry content and admits metadata before registration",
          "[runtime][x06b3a]") {
  b3a_fixture f;
  for (auto path : {f.declaration, f.registry, f.ledger}) {
    watch_path(path);
    auto owner = f.clean();
    CHECK(planar_admission_read_attempts() == 0);
  }
  // Positive control: the harness counts the trusted module read.
  watch_path(f.module);
  auto owner = f.clean();
  CHECK(planar_admission_read_attempts() > 0);
}
TEST_CASE("X06 B3 A supplied slot aliasing a trusted module refuses with unsafe_path and no partial owner", "[runtime][x06b3a]") {
  for (auto which : {"declaration", "registry", "ledger"}) {
    b3a_fixture f;
    auto        alias = std::string_view(which) == "declaration" ? f.declaration
                        : std::string_view(which) == "registry"  ? f.registry
                                                                 : f.root / "ledger-alias";
    std::filesystem::remove(alias);
    std::filesystem::create_hard_link(f.module, alias);
    auto r = std::string_view(which) == "ledger" ? f.open(alias.string()) : f.open();
    INFO(which);
    expect_denied(r);
    std::filesystem::remove(alias);
    if (std::string_view(which) != "ledger") {
      put(alias, "{}");
    }
    auto again = f.open();
    CHECK(again);
  }
}
TEST_CASE("X06 B3 A admission precedes registration so an invalid module never executes", "[runtime][x06b3a]") {
  b3a_fixture   f;
  std::ifstream in(f.module, std::ios::binary);
  std::string   valid{std::istreambuf_iterator<char>(in), {}};
  in.close();
  put(f.module, "this is not lua (((");
  std::filesystem::remove(f.declaration);
  std::filesystem::create_hard_link(f.module, f.declaration);
  auto r = f.open();
  expect_denied(r);
  // With the alias removed the same invalid module is a registration refusal, proving the order above is observable.
  std::filesystem::remove(f.declaration);
  put(f.declaration, "-- declaration");
  auto invalid = f.open();
  REQUIRE_FALSE(invalid);
  CHECK(invalid.error().detail.code != "unsafe_path");
  put(f.module, valid);
  CHECK(f.open());
}
TEST_CASE("X06 B3 A missing declaration refuses with the declaration adapter and leaves no owner", "[runtime][x06b3a]") {
  b3a_fixture f;
  std::filesystem::remove(f.declaration);
  auto r = f.open();
  REQUIRE_FALSE(r);
  CHECK(r.error().detail.code == "io");
  CHECK(r.error().detail.message == "Cannot inspect confined input");
  CHECK(r.error().field == "profile");
  put(f.declaration, "-- declaration");
  CHECK(f.open());
}
namespace {
/// Restores directory permissions on scope exit.
struct mode_guard {
  std::filesystem::path path;
  ::mode_t              original;
  mode_guard(std::filesystem::path p, ::mode_t restricted) : path(std::move(p)) {
    struct stat s{};
    REQUIRE(::stat(path.c_str(), &s) == 0);
    original = s.st_mode & 07777;
    REQUIRE(::chmod(path.c_str(), restricted) == 0);
  }
  mode_guard(const mode_guard&)            = delete;
  mode_guard& operator=(const mode_guard&) = delete;
  ~mode_guard() {
    ::chmod(path.c_str(), original);
  }
};
std::uint64_t capture_reads(const std::filesystem::path& watched, const std::filesystem::path& profile) {
  watch_path(watched);
  auto capture = profile_capture::open(profile.string());
  REQUIRE(capture);
  // Each factory verifies its original capture once after registration; that reverification is part of one capture.
  REQUIRE(capture->verify());
  return planar_admission_read_attempts();
}
} // namespace
TEST_CASE("X06 B3 A declaration in an unsearchable directory refuses as open, not inspect", "[runtime][x06b3a]") {
  if (::geteuid() == 0) {
    SUCCEED("permission denial is not observable as root");
    return;
  }
  b3a_fixture f;
  {
    mode_guard guard(f.workspace_dir, 0444);
    auto       r = f.open();
    REQUIRE_FALSE(r);
    CHECK(r.error().detail.code == "io");
    CHECK(r.error().detail.message == "Cannot open confined input");
    CHECK(r.error().detail.message != "Cannot inspect confined input");
    CHECK(r.error().field == "profile");
  }
  CHECK(f.open());
}
TEST_CASE("X06 B3 A factories capture exactly once and adoption reads nothing", "[runtime][x06b3a]") {
  b3a_fixture f;
  const auto  module_reads  = capture_reads(f.module, f.profile);
  const auto  profile_reads = capture_reads(f.profile, f.profile);
  REQUIRE(module_reads > 0);
  {
    watch_path(f.module);
    auto owner = f.clean();
    CHECK(planar_admission_read_attempts() == module_reads);
  }
  {
    watch_path(f.profile);
    auto owner = f.clean();
    CHECK(planar_admission_read_attempts() == profile_reads);
  }
  {
    watch_path(f.module);
    auto owner = profile_invocation::open_opaque(f.profile.string());
    REQUIRE(owner);
    CHECK(planar_admission_read_attempts() == module_reads);
  }
  {
    watch_path(f.profile);
    auto owner = profile_invocation::open_opaque(f.profile.string());
    REQUIRE(owner);
    CHECK(planar_admission_read_attempts() == profile_reads);
  }
  for (bool known : {true, false}) {
    watch_path(f.module);
    std::optional<profile_invocation> owner;
    if (known)
      owner.emplace(f.clean());
    else {
      auto o = profile_invocation::open_opaque(f.profile.string());
      REQUIRE(o);
      owner.emplace(std::move(*o));
    }
    const auto created = planar_admission_read_attempts();
    CHECK(created == module_reads);
    auto bytes = owner->effective_catalog().data();
    if (known) {
      // Known-load owners are never adopted: refusal reads nothing and leaves the owner live.
      auto refused = std::move(*owner).into_extension();
      REQUIRE_FALSE(refused);
      CHECK(refused.error().field == "profile");
      CHECK(planar_admission_read_attempts() == created);
      CHECK(owner->observe().live);
      CHECK_FALSE(owner->observe().consumed);
      continue;
    }
    auto extension = std::move(*owner).into_extension();
    REQUIRE(extension);
    CHECK(planar_admission_read_attempts() == created);
    CHECK(extension->effective_catalog().data() == bytes);
    f.admit(*extension);
    REQUIRE(extension->prepare_history());
    REQUIRE(extension->verify());
    CHECK(extension->effective_catalog().data() == bytes);
  }
}
TEST_CASE("X06 B3 A factory registration counters are deterministic across factories", "[runtime][x06b3a]") {
  // Equal instructions/native_bytes only show deterministic cumulative work; they CANNOT detect a second register_vm,
  // because each call builds a fresh VM. The registrations observable (kept on the capture) is what detects it.
  b3a_fixture f;
  auto        known  = f.clean();
  auto        opaque = profile_invocation::open_opaque(f.profile.string());
  REQUIRE(opaque);
  auto again = f.clean();
  CHECK(known.observe().instructions > 0);
  CHECK(known.observe().native_bytes > 0);
  CHECK(known.observe().instructions == opaque->observe().instructions);
  // Known load charges its metadata admissions (workspace root, declaration, registry, ledger) to the one factory budget (B1),
  // so it is deterministic and strictly above the opaque open, which admits none of them.
  {
    auto second = profile_invocation::open_opaque(f.profile.string());
    REQUIRE(second);
    CHECK(second->observe().native_bytes == opaque->observe().native_bytes);
  }
  CHECK(known.observe().native_bytes > opaque->observe().native_bytes);
  CHECK(known.observe().instructions == again.observe().instructions);
  CHECK(known.observe().native_bytes == again.observe().native_bytes);
  CHECK(known.observe().registrations == 1);
  CHECK(opaque->observe().registrations == 1);
  CHECK(again.observe().registrations == 1);
}
TEST_CASE("X06 B3 A registration count stays one across into_extension", "[runtime][x06b3a]") {
  b3a_fixture f;
  auto        owner = f.clean();
  CHECK(owner.observe().registrations == 1);
  auto refused = std::move(owner).into_extension();
  REQUIRE_FALSE(refused);
  CHECK(refused.error().field == "profile");
  CHECK(owner.observe().registrations == 1);
  CHECK_FALSE(owner.observe().consumed);
  auto opaque = profile_invocation::open_opaque(f.profile.string());
  REQUIRE(opaque);
  CHECK(opaque->observe().registrations == 1);
  auto adopted_opaque = std::move(*opaque).into_extension();
  REQUIRE(adopted_opaque);
  CHECK(opaque->observe().registrations == 1);
  CHECK(opaque->observe().consumed);
}
TEST_CASE("X06 B3 A request intents without a codec2 profile selector refuse", "[runtime][x06b3a]") {
  b3a_fixture f;
  auto        embedded =
      acceptance::request_intent::create_v2("req-1", "actor", std::nullopt, f.declaration.string(), f.registry.string());
  REQUIRE(embedded);
  auto r = profile_invocation::open_known_load(*embedded, f.ledger.string());
  REQUIRE_FALSE(r);
  CHECK(r.error().field == "profile");
  auto legacy = acceptance::request_intent::create("req-1", "actor", std::nullopt, f.declaration.string(), f.registry.string());
  REQUIRE(legacy);
  auto l = profile_invocation::open_known_load(*legacy, f.ledger.string());
  REQUIRE_FALSE(l);
  CHECK(l.error().field == "profile");
  auto moved_from = f.intent();
  auto sink       = std::move(moved_from);
  auto m          = profile_invocation::open_known_load(moved_from, f.ledger.string());
  REQUIRE_FALSE(m);
  CHECK(m.error().field == "profile");
  CHECK(f.open());
}
TEST_CASE("X06 B3 A opaque open needs no supplied slots, reads none and is never known load", "[runtime][x06b3a]") {
  b3a_fixture missing;
  std::filesystem::remove_all(missing.workspace_dir);
  std::filesystem::remove(missing.registry);
  std::filesystem::remove(missing.ledger);
  auto none = profile_invocation::open_opaque(missing.profile.string());
  REQUIRE(none);
  CHECK_FALSE(none->observe().known_load);
  CHECK(none->observe().live);
  CHECK_FALSE(none->effective_catalog().empty());
  b3a_fixture f;
  for (auto path : {f.declaration, f.registry, f.ledger}) {
    watch_path(path);
    auto owner = profile_invocation::open_opaque(f.profile.string());
    REQUIRE(owner);
    CHECK(planar_admission_read_attempts() == 0);
  }
  auto bad = profile_invocation::open_opaque((f.root / "trusted/absent.json").string());
  REQUIRE_FALSE(bad);
  CHECK_FALSE(bad.error().detail.code.empty());
}
TEST_CASE("X06 B3 A owner moves preserve address and counters and moved-from owners refuse", "[runtime][x06b3a]") {
  b3a_fixture        f;
  auto               a      = f.clean();
  auto               bytes  = a.effective_catalog().data();
  auto               before = a.observe();
  profile_invocation b(std::move(a));
  CHECK(b.effective_catalog().data() == bytes);
  auto after = b.observe();
  CHECK(after.live);
  CHECK(after.instructions == before.instructions);
  CHECK(after.native_bytes == before.native_bytes);
  CHECK(after.known_load == before.known_load);
  CHECK_FALSE(a.observe().live);
  CHECK_FALSE(a.catalog());
  CHECK(a.effective_catalog().empty());
  auto frame = std::move(a).prepare_load(f.intent());
  REQUIRE_FALSE(frame);
  CHECK(frame.error().failure.field == "profile");
  CHECK_FALSE(frame.error().semantic);
  auto ext = std::move(a).into_extension();
  REQUIRE_FALSE(ext);
  CHECK(ext.error().field == "profile");
  auto c = f.clean();
  c      = std::move(b);
  CHECK(c.effective_catalog().data() == bytes);
  CHECK(c.observe().instructions == before.instructions);
  CHECK_FALSE(b.observe().live);
  auto& self = c;
  c          = std::move(self);
  CHECK(c.effective_catalog().data() == bytes);
  CHECK(c.observe().live);
  CHECK(c.observe().instructions == before.instructions);
  CHECK(c.observe().instructions == c.observe().instructions);
}
namespace {
std::string read_file_bytes(const std::filesystem::path& p) {
  std::ifstream in(p, std::ios::binary);
  return std::string{std::istreambuf_iterator<char>(in), {}};
}
void replace_in_file(const std::filesystem::path& p, std::string_view from, std::string_view to) {
  auto bytes = read_file_bytes(p);
  auto at    = bytes.find(from);
  REQUIRE(at != std::string::npos);
  bytes.replace(at, from.size(), to);
  put(p, bytes);
}
/// Real selected profile with one registered custom shape, a copied quota workspace and registry, and a native ledger.
struct b3b_fixture {
  std::filesystem::path root, workspace_dir, declaration, registry, ledger_path, profile, module;
  std::size_t           custom_jobs = 0;
  static std::string    default_body() {
    return "return engine.object{ok=true,value=engine.object{obligations=engine.array{},guidance=input.job.id}}";
  }
  explicit b3b_fixture(std::string body = default_body(), bool combined_custom = true, bool split_custom = true) {
    char pattern[] = "/tmp/planar-b3b-XXXXXX";
    REQUIRE(::mkdtemp(pattern));
    root          = std::filesystem::canonical(pattern);
    workspace_dir = root / "workspace";
    declaration   = workspace_dir / "workspace.lua";
    registry      = root / "registry.json";
    ledger_path   = root / "ledger";
    profile       = root / "trusted/profile.json";
    module        = root / "trusted/module.lua";
    std::filesystem::create_directory(root / "trusted");
    put(profile, R"({"version":1,"modules":["module.lua"]})");
    put(module,
        "registration.shape({id='custom',abi='shape_v1',revision='final',stages='optional',help='shape'},function(input) " +
            body + " end)");
    const std::filesystem::path fixture(PLANAR_SCHEMA_FIXTURE);
    std::filesystem::copy(fixture / "workspace", workspace_dir, std::filesystem::copy_options::recursive);
    std::filesystem::copy_file(fixture / "registry/quota-tools.json", registry);
    if (combined_custom) {
      replace_in_file(workspace_dir / "combined.lua", "shape = \"fanout_review\"", "shape = \"custom\"");
      ++custom_jobs;
    }
    if (split_custom) {
      replace_in_file(workspace_dir / "split.lua", "shape = \"fanout_review\"", "shape = \"custom\"");
      ++custom_jobs;
    }
    REQUIRE(ledger::initialize_new(ledger_path, "demo"));
  }
  b3b_fixture(const b3b_fixture&)            = delete;
  b3b_fixture& operator=(const b3b_fixture&) = delete;
  ~b3b_fixture() {
    std::filesystem::remove_all(root);
  }
  acceptance::request_intent intent(std::string_view request_id = "req-1", std::string_view actor = "actor",
                                    std::optional<std::int64_t> head = std::nullopt, std::string workspace_override = {},
                                    std::string profile_override = {}) const {
    auto r = acceptance::request_intent::create_v2(
        request_id, actor, head, workspace_override.empty() ? declaration.string() : workspace_override, registry.string(),
        profile_override.empty() ? profile.string() : profile_override);
    REQUIRE(r);
    return std::move(*r);
  }
  profile_invocation open() const {
    auto i = intent();
    auto o = profile_invocation::open_known_load(i, ledger_path.string());
    if (!o)
      INFO(o.error().detail.code << " " << o.error().detail.message << " " << o.error().field);
    REQUIRE(o);
    return std::move(*o);
  }
};
std::string describe(const profile_preparation_error& e) {
  return std::format("failure={}/{}/{}/{} semantic={}", e.failure.detail.code, e.failure.detail.message, e.failure.field,
                     e.failure.detail.limit, e.semantic ? "yes" : "no");
}
void expect_protocol(const std::expected<prepared_profile, profile_preparation_error>& r) {
  REQUIRE_FALSE(r);
  CHECK(r.error().failure.field == "profile");
  CHECK(r.error().failure.detail.code == "internal");
  CHECK(r.error().failure.detail.message == "Invalid external profile protocol");
  CHECK_FALSE(r.error().semantic);
}
} // namespace
TEST_CASE("X06 B3 A begin_frame is once only and blocks adoption", "[runtime][x06b3a]") {
  // begin_frame is private; its once-only and adoption-blocking behavior is observable through prepare_load.
  b3a_fixture f;
  auto        owner = f.clean();
  CHECK_FALSE(owner.observe().frame_begun);
  auto other = acceptance::request_intent::create_v2("req-2", "actor", std::nullopt, f.declaration.string(), f.registry.string(),
                                                     f.profile.string());
  REQUIRE(other);
  auto refused = std::move(owner).prepare_load(*other);
  REQUIRE_FALSE(refused);
  CHECK(refused.error().failure.field == "profile");
  CHECK_FALSE(refused.error().semantic);
  CHECK_FALSE(owner.observe().live);
  CHECK_FALSE(owner.observe().frame_begun);
  auto second = std::move(owner).prepare_load(f.intent());
  REQUIRE_FALSE(second);
  CHECK(second.error().failure.field == "profile");
  auto ext = std::move(owner).into_extension();
  REQUIRE_FALSE(ext);
  CHECK(ext.error().field == "profile");
  CHECK_FALSE(owner.observe().consumed);
  auto opaque = profile_invocation::open_opaque(f.profile.string());
  REQUIRE(opaque);
  CHECK_FALSE(opaque->observe().frame_begun);
  auto not_known = std::move(*opaque).prepare_load(f.intent());
  REQUIRE_FALSE(not_known);
  CHECK(opaque->observe().live);
  CHECK_FALSE(opaque->observe().frame_begun);
  CHECK(std::move(*opaque).into_extension());
  // A real successful frame: the owner is consumed, the stage reports the begun frame, and neither a second
  // preparation nor adoption can succeed afterwards.
  b3b_fixture g;
  auto        live  = g.open();
  auto        stage = std::move(live).prepare_load(g.intent());
  REQUIRE(stage);
  CHECK(stage->observe().frame_begun);
  CHECK(stage->observe().registrations == 1);
  expect_protocol(std::move(live).prepare_load(g.intent()));
  auto adopted = std::move(live).into_extension();
  REQUIRE_FALSE(adopted);
  CHECK(adopted.error().field == "profile");
  CHECK_FALSE(live.observe().live);
}
TEST_CASE("X06 B3 A into_extension adopts the same VM and receipts without recapture", "[runtime][x06b3a]") {
  b3a_fixture f;
  auto        opened = profile_invocation::open_opaque(f.profile.string());
  REQUIRE(opened);
  auto&       owner = *opened;
  auto        bytes = owner.effective_catalog().data();
  std::string copy(owner.effective_catalog());
  auto        extension = std::move(owner).into_extension();
  REQUIRE(extension);
  CHECK(extension->effective_catalog().data() == bytes);
  CHECK(std::string(extension->effective_catalog()) == copy);
  auto seen = owner.observe();
  CHECK(seen.consumed);
  CHECK_FALSE(seen.live);
  CHECK_FALSE(owner.catalog());
  CHECK(owner.effective_catalog().empty());
  auto again = std::move(owner).into_extension();
  REQUIRE_FALSE(again);
  CHECK(again.error().field == "profile");
  auto frame = std::move(owner).prepare_load(f.intent());
  REQUIRE_FALSE(frame);
  CHECK(frame.error().failure.field == "profile");
  f.admit(*extension);
  REQUIRE(extension->prepare_history());
  REQUIRE(extension->verify());
  // Adoption evidence on an opaque owner whose declaration is never read: same catalog bytes, one registration before
  // and after, and no admission read of the watched declaration. The interposer tracks one inode, so only the
  // declaration is watched here.
  b3b_fixture g;
  {
    // Positive control: with the same watch armed on the same declaration, a known-load owner of the same fixture
    // shape reads it during prepare_load, so a zero count below can only mean the opaque path never read it.
    auto known = g.open();
    // Reset the shared read counter through a file nobody reads, so a count left by an earlier test cannot satisfy the
    // control: only an armed watch on the declaration itself can make it positive.
    const auto unread = g.root / "unread.txt";
    put(unread, "unread");
    watch_path(unread);
    REQUIRE(planar_admission_read_attempts() == 0);
    watch_path(g.declaration);
    auto prepared = std::move(known).prepare_load(g.intent());
    REQUIRE(prepared);
    CHECK(planar_admission_read_attempts() > 0);
  }
  watch_path(g.declaration);
  auto other = profile_invocation::open_opaque(g.profile.string());
  REQUIRE(other);
  auto other_bytes = other->effective_catalog().data();
  CHECK(other->observe().registrations == 1);
  auto adopted = std::move(*other).into_extension();
  REQUIRE(adopted);
  CHECK(adopted->effective_catalog().data() == other_bytes);
  CHECK(other->observe().consumed);
  CHECK(other->observe().registrations == 1);
  CHECK(planar_admission_read_attempts() == 0);
}
TEST_CASE("X06 B3 A opaque owner adopts into a working extension", "[runtime][x06b3a]") {
  b3a_fixture f;
  auto        opaque = profile_invocation::open_opaque(f.profile.string());
  REQUIRE(opaque);
  auto bytes     = opaque->effective_catalog().data();
  auto extension = std::move(*opaque).into_extension();
  REQUIRE(extension);
  CHECK(extension->effective_catalog().data() == bytes);
  CHECK(opaque->observe().consumed);
  f.admit(*extension);
  REQUIRE(extension->prepare_history());
  REQUIRE(extension->verify());
}
TEST_CASE("X06 B3 A public validate_profile keeps its cumulative public semantics on a fresh registration", "[runtime][x06b3a]") {
  b3a_fixture f;
  auto        capture = profile_capture::open(f.profile.string());
  REQUIRE(capture);
  auto snapshot = capture->snapshot();
  REQUIRE(snapshot);
  auto vm = command_vm::create(*snapshot);
  REQUIRE(vm);
  auto first = vm->validate_profile(object());
  auto retry = vm->validate_profile(object());
  REQUIRE(first.has_value() == retry.has_value());
  if (first)
    CHECK(*codec_encode(*first) == *codec_encode(*retry));
  else
    CHECK(first.error().detail.code == retry.error().detail.code);
  // The owner exposes no VM accessor, so no public call can run inside its active frame.
  static_assert(!has_vm_accessor<profile_invocation>);
  auto owner  = f.clean();
  auto before = owner.observe();
  CHECK_FALSE(before.frame_begun);
  owner.observe();
  auto moved = std::move(owner);
  CHECK(moved.observe().instructions == before.instructions);
}
TEST_CASE("X06 B3 B known-load owner refuses into_extension terminally-free and stays usable", "[runtime][x06b3b]") {
  b3a_fixture f;
  auto        owner = f.clean();
  auto        first = std::move(owner).into_extension();
  REQUIRE_FALSE(first);
  CHECK(first.error().field == "profile");
  auto seen = owner.observe();
  CHECK(seen.live);
  CHECK_FALSE(seen.consumed);
  CHECK(seen.known_load);
  CHECK(owner.catalog());
  auto second = std::move(owner).into_extension();
  REQUIRE_FALSE(second);
  CHECK(second.error().field == first.error().field);
  CHECK(second.error().detail.code == first.error().detail.code);
  CHECK(second.error().detail.message == first.error().detail.message);
  CHECK(owner.observe().live);
  CHECK_FALSE(owner.observe().consumed);
  CHECK(owner.catalog());
  auto opaque = profile_invocation::open_opaque(f.profile.string());
  REQUIRE(opaque);
  auto adopted = std::move(*opaque).into_extension();
  REQUIRE(adopted);
  CHECK(opaque->observe().consumed);
}
TEST_CASE("X06 B3 B owner implementation allocation is charged identically by both factories", "[runtime][x06b3b]") {
  // No VM-only native_bytes counter is observable, so the charge itself cannot be isolated here; this asserts the
  // observable invariant: both factories charge the identical amount (equal, nonzero, and repeatable).
  b3a_fixture f;
  auto        known  = f.clean();
  auto        again  = f.clean();
  auto        opaque = profile_invocation::open_opaque(f.profile.string());
  REQUIRE(opaque);
  CHECK(known.observe().native_bytes > 0);
  // Known load additionally charges its metadata admissions to the one factory budget (B1); the owner charge itself is
  // compared exactly by the next test.
  {
    auto second = profile_invocation::open_opaque(f.profile.string());
    REQUIRE(second);
    CHECK(second->observe().native_bytes == opaque->observe().native_bytes);
  }
  CHECK(known.observe().native_bytes > opaque->observe().native_bytes);
  CHECK(known.observe().native_bytes == again.observe().native_bytes);
}
TEST_CASE("X06 B3 B owner allocation charge is observable and covers the owner's own storage", "[runtime][x06b3b]") {
  // The factory charge for the owner allocation is reported separately, so removing it from either factory is detected.
  // Lower bound from public types: the owner holds at least a capture and a VM, plus the fixed 64 byte allowance.
  constexpr std::size_t floor = sizeof(profile_capture) + sizeof(command_vm) + 64;
  b3a_fixture           f;
  auto                  known  = f.clean();
  auto                  opaque = profile_invocation::open_opaque(f.profile.string());
  REQUIRE(opaque);
  CHECK(known.observe().owner_charge >= floor);
  CHECK(opaque->observe().owner_charge >= floor);
  CHECK(known.observe().owner_charge == opaque->observe().owner_charge);
  CHECK(known.observe().native_bytes > known.observe().owner_charge);
}
TEST_CASE("X06 B3 B prepare_load succeeds through phase 2 for custom and mixed jobs", "[runtime][x06b3b]") {
  for (auto [combined, split] : {std::pair{true, true}, std::pair{false, true}, std::pair{false, false}}) {
    b3b_fixture f(b3b_fixture::default_body(), combined, split);
    auto        owner  = f.open();
    auto        before = owner.observe();
    auto        stage  = std::move(owner).prepare_load(f.intent());
    if (!stage)
      INFO(describe(stage.error()));
    REQUIRE(stage);
    auto seen = stage->observe();
    CHECK(seen.live);
    CHECK(seen.frame_begun);
    CHECK(seen.sealed);
    CHECK(seen.bundle_retained);
    CHECK(seen.model_retained);
    CHECK(seen.jobs == 2);
    CHECK(seen.shape_dispatches == f.custom_jobs);
    CHECK(seen.instructions > 0);
    CHECK(seen.instructions > before.instructions);
    CHECK(seen.registrations == 1);
    CHECK_FALSE(owner.observe().live);
    // Observation performs no filesystem access after the seal.
    // The interposer tracks one inode and each watch_path resets the counter: watch the declaration alone.
    watch_path(f.declaration);
    (void)stage->observe();
    CHECK(planar_admission_read_attempts() == 0);
    watch_path(f.registry);
    (void)stage->observe();
    CHECK(planar_admission_read_attempts() == 0);
    auto moved = std::move(*stage);
    CHECK_FALSE(stage->observe().live);
    CHECK(moved.observe().live);
    CHECK(moved.observe().shape_dispatches == f.custom_jobs);
    auto& self = moved;
    moved      = std::move(self);
    CHECK(moved.observe().live);
  }
}
TEST_CASE("X06 B3 B prepare_load reads the declaration through the watched interposer", "[runtime][x06b3b]") {
  // Positive control for the zero-read assertions: the same watch on the same fixture does see admission reads.
  b3b_fixture f;
  auto        owner = f.open();
  watch_path(f.declaration);
  auto stage = std::move(owner).prepare_load(f.intent());
  REQUIRE(stage);
  CHECK(planar_admission_read_attempts() > 0);
}
TEST_CASE("X06 B3 B an invalid unselected job fails all-job validation and publishes nothing", "[runtime][x06b3b]") {
  b3b_fixture f(b3b_fixture::default_body(), true, false);
  replace_in_file(f.workspace_dir / "split.lua", "work = L { \"library\" } },\n    { id = \"cli_worker\"",
                  "work = L { \"nosuch\" } },\n    { id = \"cli_worker\"");
  auto owner = f.open();
  auto r     = std::move(owner).prepare_load(f.intent());
  REQUIRE_FALSE(r);
  INFO(describe(r.error()));
  REQUIRE(r.error().semantic);
  CHECK(r.error().semantic->kind == value_kind::record);
  CHECK(r.error().semantic->fields.contains("code"));
  CHECK(r.error().failure.detail.code.empty());
  CHECK(r.error().failure.field.empty());
  CHECK_FALSE(owner.observe().live);
  expect_protocol(std::move(owner).prepare_load(f.intent()));
}
TEST_CASE("X06 B3 B a route phase refusal is ordinary semantic data before any job is captured", "[runtime][x06b3b]") {
  // The workspace declaration itself is invalid, so the trusted route phase (phase 8) answers with a semantic error.
  {
    // Positive control: with a valid declaration the same watch observes specification reads during preparation.
    b3b_fixture control;
    auto        controlled = control.open();
    watch_path(control.workspace_dir / "spec.lua");
    REQUIRE(std::move(controlled).prepare_load(control.intent()));
    REQUIRE(planar_admission_read_attempts() > 0);
  }
  b3b_fixture f;
  replace_in_file(f.declaration, "toolchains = L { \"python\" }", "toolchains = L { \"python\", \"python\" }");
  auto owner = f.open();
  watch_path(f.workspace_dir / "spec.lua");
  auto r = std::move(owner).prepare_load(f.intent());
  REQUIRE_FALSE(r);
  INFO(describe(r.error()));
  REQUIRE(r.error().semantic);
  CHECK(r.error().semantic->kind == value_kind::record);
  CHECK(r.error().semantic->fields.contains("code"));
  CHECK(r.error().failure.detail.code.empty());
  CHECK(r.error().failure.field.empty());
  // Phase 8 refused before source capture read the specification or any job, and before shape dispatch.
  CHECK(planar_admission_read_attempts() == 0);
  CHECK_FALSE(owner.observe().live);
  expect_protocol(std::move(owner).prepare_load(f.intent()));
}
TEST_CASE("X06 B3 B a declaration the bound source cannot open refuses with its native diagnostic", "[runtime][x06b3b]") {
  // The declaration content is first read when prepare_load opens the bound source, after both profile checks pass.
  // A declaration that fails to load there must surface the native diagnostic under the profile field, not the
  // generic protocol refusal, and must not reach the route phase.
  b3b_fixture f;
  put(f.declaration, "this is not a workspace declaration (");
  auto owner = f.open();
  auto r     = std::move(owner).prepare_load(f.intent());
  REQUIRE_FALSE(r);
  INFO(describe(r.error()));
  CHECK(r.error().failure.field == "profile");
  CHECK(r.error().failure.detail.code == "lua_syntax");
  CHECK(r.error().failure.detail.message == "Invalid Lua text chunk");
  CHECK_FALSE(r.error().semantic);
  CHECK_FALSE(r.error().cleanup_ran);
  CHECK_FALSE(owner.observe().live);
  expect_protocol(std::move(owner).prepare_load(f.intent()));
}
TEST_CASE("X06 B3 B a custom job refusal after earlier shape dispatch is ordinary semantic data", "[runtime][x06b3b]") {
  b3b_fixture f("if input.job.id=='split' then return {ok=false,error={code='schema',message='refused'}} end " +
                b3b_fixture::default_body());
  auto        owner = f.open();
  auto        r     = std::move(owner).prepare_load(f.intent());
  REQUIRE_FALSE(r);
  INFO(describe(r.error()));
  CHECK(r.error().semantic);
  CHECK_FALSE(owner.observe().live);
  expect_protocol(std::move(owner).prepare_load(f.intent()));
}
TEST_CASE("X06 B3 B every job is validated exactly once", "[runtime][x06b3b]") {
  b3b_fixture                f;
  std::optional<std::size_t> instructions, bytes;
  for (int round = 0; round < 3; ++round) {
    auto owner = f.open();
    auto stage = std::move(owner).prepare_load(f.intent());
    REQUIRE(stage);
    auto seen = stage->observe();
    CHECK(seen.shape_dispatches == f.custom_jobs);
    CHECK(seen.jobs == f.custom_jobs);
    if (!instructions) {
      instructions = seen.instructions;
      bytes        = seen.native_bytes;
    }
    CHECK(seen.instructions == *instructions);
    CHECK(seen.native_bytes == *bytes);
  }
}
TEST_CASE("X06 B3 B declaration leaf replaced before prepare_load refuses terminally before content", "[runtime][x06b3b]") {
  for (bool registry_file : {false, true}) {
    b3b_fixture f;
    auto        owner  = f.open();
    auto        target = registry_file ? f.registry : f.declaration;
    auto        fresh  = target;
    fresh += ".new";
    put(fresh, read_file_bytes(target));
    watch_path(fresh);
    std::filesystem::rename(fresh, target);
    auto r = std::move(owner).prepare_load(f.intent());
    REQUIRE_FALSE(r);
    INFO(describe(r.error()));
    CHECK(r.error().failure.detail.code == "capture_changed");
    CHECK(r.error().failure.field == "profile");
    CHECK_FALSE(r.error().semantic);
    CHECK_FALSE(r.error().cleanup_ran);
    CHECK(planar_admission_read_attempts() == 0);
    CHECK_FALSE(owner.observe().live);
    expect_protocol(std::move(owner).prepare_load(f.intent()));
  }
}
TEST_CASE("X06 B3 B final intent must equal the bound intent", "[runtime][x06b3b]") {
  b3b_fixture f;
  struct variant {
    std::string                name;
    acceptance::request_intent intent;
  };
  std::vector<variant> variants;
  variants.push_back({"request id", f.intent("req-2")});
  variants.push_back({"actor", f.intent("req-1", "other")});
  variants.push_back({"expected head", f.intent("req-1", "actor", 7)});
  variants.push_back({"profile", f.intent("req-1", "actor", std::nullopt, {}, (f.root / "trusted/other.json").string())});
  variants.push_back({"workspace", f.intent("req-1", "actor", std::nullopt, (f.workspace_dir / "other.lua").string())});
  for (auto& v : variants) {
    INFO(v.name);
    auto owner = f.open();
    auto r     = std::move(owner).prepare_load(v.intent);
    expect_protocol(r);
    CHECK_FALSE(owner.observe().live);
    expect_protocol(std::move(owner).prepare_load(f.intent()));
  }
  // An equal freshly created intent succeeds.
  auto owner = f.open();
  CHECK(std::move(owner).prepare_load(f.intent()));
}
TEST_CASE("X06 B3 B opaque consumed failed and moved-from owners refuse prepare_load", "[runtime][x06b3b]") {
  b3b_fixture f;
  auto        opaque = profile_invocation::open_opaque(f.profile.string());
  REQUIRE(opaque);
  expect_protocol(std::move(*opaque).prepare_load(f.intent()));
  CHECK(opaque->observe().live);
  CHECK_FALSE(opaque->observe().frame_begun);
  CHECK(std::move(*opaque).into_extension());
  expect_protocol(std::move(*opaque).prepare_load(f.intent()));
  auto known = f.open();
  auto moved = std::move(known);
  expect_protocol(std::move(known).prepare_load(f.intent()));
  auto stage = std::move(moved).prepare_load(f.intent());
  REQUIRE(stage);
  expect_protocol(std::move(moved).prepare_load(f.intent()));
  auto failed = f.open();
  expect_protocol(std::move(failed).prepare_load(f.intent("req-9")));
  expect_protocol(std::move(failed).prepare_load(f.intent()));
  CHECK_FALSE(failed.observe().live);
  CHECK_FALSE(std::move(failed).into_extension());
}
TEST_CASE("X06 B3 B one frame cumulative counters never reset", "[runtime][x06b3b]") {
  b3b_fixture f;
  auto        owner  = f.open();
  auto        before = owner.observe();
  CHECK_FALSE(before.frame_begun);
  CHECK(before.instructions > 0);
  auto stage = std::move(owner).prepare_load(f.intent());
  REQUIRE(stage);
  auto after = stage->observe();
  CHECK(after.frame_begun);
  CHECK(after.instructions > before.instructions);
  CHECK(after.native_bytes > before.native_bytes);
  CHECK(after.registrations == before.registrations);
  auto twin = f.open();
  auto next = std::move(twin).prepare_load(f.intent());
  REQUIRE(next);
  CHECK(next->observe().instructions == after.instructions);
  CHECK(next->observe().native_bytes == after.native_bytes);
  // The owner cannot start another frame: it is moved-from.
  expect_protocol(std::move(owner).prepare_load(f.intent()));
}
TEST_CASE("X06 B3 B cumulative instruction exhaustion preserves the first error and cleans nested roots", "[runtime][x06b3b]") {
  b3b_fixture f("local n=0; for i=1,B3B_LOOP do n=n+1 end; " + b3b_fixture::default_body());
  replace_in_file(f.module, "B3B_LOOP", "30000000");
  auto owner  = f.open();
  auto before = owner.observe();
  auto r      = std::move(owner).prepare_load(f.intent());
  REQUIRE_FALSE(r);
  INFO(describe(r.error()));
  CHECK_FALSE(r.error().semantic);
  CHECK(r.error().failure.detail.code == "resource_limit");
  CHECK(r.error().failure.detail.limit == "instructions");
  CHECK(r.error().failure.detail.maximum == 100000000);
  CHECK(r.error().failure.field == "catalog.shapes[1]");
  CHECK_FALSE(owner.observe().live);
  CHECK(owner.observe().instructions == 0);
  CHECK(r.error().cleanup_ran);
  CHECK(r.error().peak_temporary_entries > 0);
  CHECK(r.error().temporary_entries_after_release == 0);
  expect_protocol(std::move(owner).prepare_load(f.intent()));
  CHECK(before.instructions < 100000000);
  // A single custom job under the same body fits, proving the second dispatch is what exhausts the cumulative budget.
  b3b_fixture single("local n=0; for i=1,B3B_LOOP do n=n+1 end; " + b3b_fixture::default_body(), false, true);
  replace_in_file(single.module, "B3B_LOOP", "30000000");
  auto one = single.open();
  auto ok  = std::move(one).prepare_load(single.intent());
  REQUIRE(ok);
  CHECK(ok->observe().instructions > before.instructions);
  CHECK(ok->observe().instructions < 100000000);
}
TEST_CASE("X06 B3 B capture change before prepare_load refuses capture_changed with zero declaration reads",
          "[runtime][x06b3b]") {
  // Same-length edits of captured state that the declaration and registry leaf checks cannot see.
  for (int which = 0; which < 4; ++which) {
    INFO("iteration " << which);
    b3b_fixture f;
    auto        owner = f.open();
    if (which == 0)
      replace_in_file(f.module, "help='shape'", "help='shapf'");
    else if (which == 1)
      put(f.profile, R"({"modules":["module.lua"],"version":1})");
    else if (which == 3) {
      // Same-size, same-mtime content edit of a captured module: only the content reverification can observe it.
      const auto stamp = std::filesystem::last_write_time(f.module);
      replace_in_file(f.module, "help='shape'", "help='shapf'");
      std::filesystem::last_write_time(f.module, stamp);
    } else {
      auto fresh = f.ledger_path;
      fresh += ".new";
      put(fresh, read_file_bytes(f.ledger_path));
      std::filesystem::rename(fresh, f.ledger_path);
    }
    watch_path(f.declaration);
    auto r = std::move(owner).prepare_load(f.intent());
    REQUIRE_FALSE(r);
    INFO(describe(r.error()));
    CHECK(r.error().failure.detail.code == "capture_changed");
    CHECK(r.error().failure.field == "profile");
    CHECK_FALSE(r.error().semantic);
    CHECK_FALSE(r.error().cleanup_ran);
    CHECK(planar_admission_read_attempts() == 0);
    CHECK_FALSE(owner.observe().live);
    expect_protocol(std::move(owner).prepare_load(f.intent()));
  }
}
TEST_CASE("X06 B3 B one cumulative instruction and native byte budget spans route validation and preparation",
          "[runtime][x06b3b]") {
  b3b_fixture f;
  auto        owner  = f.open();
  auto        before = owner.observe();
  auto        stage  = std::move(owner).prepare_load(f.intent());
  REQUIRE(stage);
  auto        seen = stage->observe();
  const auto& i    = seen.instruction_marks;
  const auto& n    = seen.native_byte_marks;
  // begin_invocation restarts the instruction counter once at the frame start, so mark 0 may be below the owner value.
  CHECK(i[0] < i[1]);
  CHECK(i[1] <= i[2]);
  CHECK(i[2] < i[3]);
  CHECK(i[3] <= seen.instructions);
  CHECK(i[1] > before.instructions);
  CHECK(before.native_bytes <= n[0]);
  CHECK(n[0] < n[1]);
  CHECK(n[1] < n[2]);
  CHECK(n[2] < n[3]);
  CHECK(n[3] < seen.native_bytes);
  // F-dominated case: one loop-heavy custom job succeeds and its cost lands between the validation marks.
  b3b_fixture single("local n=0; for i=1,B3B_LOOP do n=n+1 end; " + b3b_fixture::default_body(), false, true);
  replace_in_file(single.module, "B3B_LOOP", "30000000");
  auto one = std::move(single.open()).prepare_load(single.intent());
  REQUIRE(one);
  auto heavy = one->observe();
  CHECK(heavy.instruction_marks[3] >= heavy.instruction_marks[2] + 30000000);
  CHECK(heavy.instructions >= heavy.instruction_marks[3]);
  CHECK(heavy.instructions < 100000000);
}
TEST_CASE("X06 B3 B success releases nested temporaries to the exact pre invocation membership", "[runtime][x06b3b]") {
  for (auto [combined, split] : {std::pair{true, true}, std::pair{false, true}, std::pair{false, false}}) {
    b3b_fixture f(b3b_fixture::default_body(), combined, split);
    auto        owner  = f.open();
    auto        before = owner.observe();
    CHECK(before.temporary_entries == 0);
    auto stage = std::move(owner).prepare_load(f.intent());
    REQUIRE(stage);
    auto seen = stage->observe();
    CHECK(seen.shape_dispatches == f.custom_jobs);
    CHECK(seen.baseline_before_route);
    CHECK(seen.cleanup_ran);
    if (f.custom_jobs > 0)
      CHECK(seen.peak_temporary_entries > 0);
    CHECK(seen.temporary_entries == 0);
    CHECK(seen.tag_entries == before.tag_entries);
    CHECK(seen.backing_entries == before.backing_entries);
  }
}
TEST_CASE("X06 B3 B failure paths release nested temporaries", "[runtime][x06b3b]") {
  struct scenario {
    std::string name;
    std::string body;
    bool        invalid_unselected;
    bool        long_loop;
  };
  for (const auto& sc : {scenario{"native F error", "error('shape failure')", false, false},
                         scenario{"invalid unselected job", b3b_fixture::default_body(), true, false},
                         scenario{"instruction exhaustion",
                                  "local n=0; for i=1,B3B_LOOP do n=n+1 end; " + b3b_fixture::default_body(), false, true}}) {
    INFO(sc.name);
    b3b_fixture f(sc.body, true, !sc.invalid_unselected);
    if (sc.long_loop)
      replace_in_file(f.module, "B3B_LOOP", "30000000");
    if (sc.invalid_unselected)
      replace_in_file(f.workspace_dir / "split.lua", "work = L { \"library\" } },\n    { id = \"cli_worker\"",
                      "work = L { \"nosuch\" } },\n    { id = \"cli_worker\"");
    auto owner = f.open();
    auto r     = std::move(owner).prepare_load(f.intent());
    REQUIRE_FALSE(r);
    INFO(describe(r.error()));
    CHECK(r.error().cleanup_ran);
    CHECK(r.error().peak_temporary_entries > 0);
    CHECK(r.error().temporary_entries_after_release == 0);
    CHECK_FALSE(owner.observe().live);
  }
  // Refusals before the frame begins run no cleanup.
  b3b_fixture g;
  auto        owner = g.open();
  auto        early = std::move(owner).prepare_load(g.intent("req-other"));
  REQUIRE_FALSE(early);
  CHECK_FALSE(early.error().cleanup_ran);
}
TEST_CASE("X06 B3 B profile frame branches stay reachable and first terminal native failures are preserved",
          "[runtime][x06b3b]") {
  // Nested F execution failure (non-resource): the retained call fails inside the profile frame and stays terminal.
  b3b_fixture f("error('shape failure')");
  auto        owner = f.open();
  auto        r     = std::move(owner).prepare_load(f.intent());
  REQUIRE_FALSE(r);
  INFO(describe(r.error()));
  CHECK_FALSE(r.error().semantic);
  CHECK(r.error().failure.field == "catalog.shapes[1]");
  CHECK_FALSE(owner.observe().live);
  expect_protocol(std::move(owner).prepare_load(f.intent()));
  // Unregistered shape selected by a job: trusted phase 0 refuses as ordinary data before any F dispatch.
  b3b_fixture g;
  replace_in_file(g.workspace_dir / "split.lua", "shape = \"custom\"", "shape = \"nonexistent\"");
  auto missing = std::move(g.open()).prepare_load(g.intent());
  REQUIRE_FALSE(missing);
  INFO(describe(missing.error()));
  CHECK(missing.error().semantic);
}
TEST_CASE("X06 B3 B prepare_load reverifies captured profile content exactly three times", "[runtime][x06b3b]") {
  b3b_fixture f;
  auto        owner = f.open();
  // Each content verification reads the selected module twice (profile reverification is two-pass), so exactly three
  // verifications give six reads. This asserts the count only; the order of the verifications is not observable here.
  watch_path(f.module);
  auto stage = std::move(owner).prepare_load(f.intent());
  REQUIRE(stage);
  CHECK(planar_admission_read_attempts() == 6);
}
TEST_CASE("X06 B3 B exhaustion depends on route consumption because the instruction budget is one cumulative counter",
          "[runtime][x06b3b]") {
  // Calibration (Debug, default body, one custom job, loop `for i=1,N do n=n+1 end` in the custom shape):
  //   route (marks[1]-marks[0]) = 9000 instructions, independent of N;
  //   validation + preparation (marks[3]-marks[2]) = 121000 + 2*N instructions (measured at N = 1000, 1e6, 2e6:
  //   123000, 2121000, 4121000).
  // The heavy N below makes validation alone 99995000 (< 100000000) while route + validation is 100004000, so only
  // a budget that carries the route's consumption into validation can exhaust.
  constexpr int          heavy_n = 49937000;
  constexpr int          light_n = 49927000;
  constexpr std::int64_t budget  = 100000000;
  auto                   body    = [](int n) {
    return std::string("local n=0; for i=1,") + std::to_string(n) + " do n=n+1 end; " + b3b_fixture::default_body();
  };
  // Light variant: route + validation fits; each phase and the total stay below the budget.
  b3b_fixture light(body(light_n), false, true);
  auto        ok = std::move(light.open()).prepare_load(light.intent());
  if (!ok)
    INFO(describe(ok.error()));
  REQUIRE(ok);
  const auto seen  = ok->observe();
  const auto route = seen.instruction_marks[1] - seen.instruction_marks[0];
  const auto valid = seen.instruction_marks[3] - seen.instruction_marks[2];
  CHECK(route > 0);
  CHECK(seen.instructions < budget);
  CHECK(valid < budget);
  // Validation alone for the heavy variant: the light measurement plus the extra loop iterations at 2 per iteration.
  const auto heavy_valid = valid + 2 * static_cast<std::int64_t>(heavy_n - light_n);
  CHECK(heavy_valid < budget);
  CHECK(heavy_valid + route > budget);
  // Heavy variant: the same fixture shape with a slightly longer loop exhausts only because route cost is counted.
  b3b_fixture heavy(body(heavy_n), false, true);
  auto        owner  = heavy.open();
  auto        before = owner.observe();
  auto        r      = std::move(owner).prepare_load(heavy.intent());
  REQUIRE_FALSE(r);
  INFO(describe(r.error()));
  CHECK_FALSE(r.error().semantic);
  CHECK(r.error().failure.detail.code == "resource_limit");
  CHECK(r.error().failure.detail.limit == "instructions");
  CHECK(r.error().failure.detail.maximum == 100000000);
  CHECK(before.instructions < budget);
}

namespace {
/// Controlled allocator hook for the B3-C windows: counts operator new calls while armed and, on one chosen call,
/// either renames a pre-made file over its target (never allocating) or throws std::bad_alloc.
struct alloc_probe {
  bool        armed = false, fired = false, throw_on_trigger = false;
  std::size_t count = 0, trigger = 0;
  const char* rename_from = nullptr;
  const char* rename_to   = nullptr;
  /// In-place edit, performed instead of the rename when `edit_path` is set: one byte overwritten at `edit_offset`
  /// and the recorded modification time restored, so only content verification can see it. Uses raw system calls only.
  const char*     edit_path   = nullptr;
  std::size_t     edit_offset = 0;
  char            edit_byte   = 0;
  struct timespec edit_mtime{};
};
thread_local alloc_probe probe;
} // namespace
void* operator new(std::size_t bytes) {
  if (probe.armed) {
    ++probe.count;
    if (probe.trigger != 0 && probe.count == probe.trigger) {
      probe.armed = false;
      probe.fired = true;
      if (probe.throw_on_trigger)
        throw std::bad_alloc{};
      if (probe.edit_path) {
        if (int fd = ::open(probe.edit_path, O_WRONLY); fd >= 0) {
          (void)::pwrite(fd, &probe.edit_byte, 1, static_cast<off_t>(probe.edit_offset));
          const struct timespec times[2] = {{0, UTIME_OMIT}, probe.edit_mtime};
          (void)::futimens(fd, times);
          (void)::close(fd);
        }
      } else if (probe.rename_from && probe.rename_to)
        (void)::rename(probe.rename_from, probe.rename_to);
    }
  }
  if (auto result = std::malloc(bytes ? bytes : 1))
    return result;
  throw std::bad_alloc{};
}
void operator delete(void* pointer) noexcept {
  std::free(pointer);
}
void* operator new[](std::size_t bytes) {
  return ::operator new(bytes);
}
void operator delete[](void* pointer) noexcept {
  ::operator delete(pointer);
}
namespace {
std::string digest_hex(const history::record_digest& digest) {
  std::string out;
  for (auto byte : digest)
    out += std::format("{:02x}", byte);
  return out;
}
std::uint32_t normalized_mode(const std::filesystem::path& p) {
  struct stat s{};
  REQUIRE(::stat(p.c_str(), &s) == 0);
  return (s.st_mode & 0100) != 0 ? 0755u : 0644u;
}
/// Every regular file below the workspace directory, sorted by portable path, read from disk.
std::vector<history::tree_file> workspace_files_on_disk(const std::filesystem::path& root) {
  std::vector<history::tree_file> out;
  for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
    if (!entry.is_regular_file())
      continue;
    out.push_back({std::filesystem::relative(entry.path(), root).generic_string(), read_file_bytes(entry.path()),
                   normalized_mode(entry.path())});
  }
  std::ranges::sort(out, {}, &history::tree_file::path);
  return out;
}
std::vector<history::tree_file_view> views_of(const std::vector<history::tree_file>& files) {
  std::vector<history::tree_file_view> views;
  for (const auto& f : files)
    views.push_back({f.path, f.payload, f.mode});
  return views;
}
prepared_profile prepare_ok(const b3b_fixture& f) {
  auto              owner = f.open();
  auto              r     = std::move(owner).prepare_load(f.intent());
  const std::string note  = r ? std::string("ok") : describe(r.error());
  INFO(note);
  REQUIRE(r);
  return std::move(*r);
}
std::shared_ptr<const profile_acceptance_data> data_of(const prepared_profile& p) {
  auto d = p.data();
  REQUIRE(d);
  REQUIRE(*d);
  return *d;
}
void expect_capture_changed(const std::expected<void, diagnostic>& r) {
  REQUIRE_FALSE(r);
  CHECK(r.error().code == "capture_changed");
}
/// Replaces the leaf with a fresh inode holding the same bytes.
void replace_leaf(const std::filesystem::path& target) {
  auto fresh = target;
  fresh += ".fresh";
  put(fresh, read_file_bytes(target));
  std::filesystem::rename(fresh, target);
}
/// Same-size content edit in place with the original modification time restored: only content verification sees it.
void edit_same_size_keep_mtime(const std::filesystem::path& target) {
  const auto stamp = std::filesystem::last_write_time(target);
  auto       bytes = read_file_bytes(target);
  REQUIRE_FALSE(bytes.empty());
  bytes[0] = bytes[0] == 'x' ? 'y' : 'x';
  put(target, bytes);
  std::filesystem::last_write_time(target, stamp);
}
std::string same_size_variant(std::string bytes) {
  REQUIRE_FALSE(bytes.empty());
  bytes[bytes.size() / 2] = bytes[bytes.size() / 2] == 'x' ? 'y' : 'x';
  return bytes;
}
constexpr std::array<std::pair<bool, bool>, 3> b3c_shapes{std::pair{true, true}, std::pair{false, true}, std::pair{false, false}};
} // namespace
TEST_CASE("X06 B3 C success publishes source registry effective tree and bound intent for each fixture shape",
          "[runtime][x06b3c]") {
  std::set<std::string> source_digests;
  for (auto [combined, split] : b3c_shapes) {
    INFO("combined=" << combined << " split=" << split);
    b3b_fixture f(b3b_fixture::default_body(), combined, split);
    auto        prepared = prepare_ok(f);
    auto        shared   = data_of(prepared);
    const auto& d        = *shared;
    // Source: the selected closure is the whole fixture workspace, sorted, and its identity is derived independently.
    REQUIRE_FALSE(d.source.empty());
    CHECK(std::ranges::is_sorted(d.source, {}, &history::tree_file::path));
    const auto disk = workspace_files_on_disk(f.workspace_dir);
    REQUIRE(d.source.size() == disk.size());
    for (std::size_t i = 0; i < disk.size(); ++i) {
      CHECK(d.source[i].path == disk[i].path);
      CHECK(d.source[i].payload == disk[i].payload);
      CHECK(d.source[i].mode == disk[i].mode);
    }
    auto expected_source = history::digest_tree(views_of(disk), history::tree_profile::source);
    REQUIRE(expected_source);
    CHECK(d.source_digest == *expected_source);
    source_digests.insert(digest_hex(d.source_digest));
    // Registry: exactly one input.json carrying the captured registry bytes.
    REQUIRE(d.registry.files.size() == 1);
    CHECK(d.registry.files[0].path == "input.json");
    CHECK(d.registry.files[0].payload == read_file_bytes(f.registry));
    auto registry_digest = history::digest_tree(views_of(d.registry.files), history::tree_profile::provenance);
    REQUIRE(registry_digest);
    CHECK(d.registry.digest == *registry_digest);
    // Effective tree: reassembled raw stdlib including the manifest, the profile and the external module.
    auto has_path = [&](std::string_view path) {
      return std::ranges::any_of(d.effective.files, [&](const auto& file) { return file.path == path; });
    };
    CHECK(has_path("__planar__/effective.bin"));
    CHECK(has_path("profile/input.json"));
    CHECK(has_path("external/module.lua"));
    auto profile_it = std::ranges::find(d.effective.files, "profile/input.json", &history::tree_file::path);
    REQUIRE(profile_it != d.effective.files.end());
    CHECK(profile_it->payload == read_file_bytes(f.profile));
    auto effective_digest = history::digest_tree(views_of(d.effective.files), history::tree_profile::effective_provenance);
    REQUIRE(effective_digest);
    CHECK(d.effective.digest == *effective_digest);
    CHECK(d.effective.digest != d.registry.digest);
    CHECK(d.workspace_id == "quota");
    // Bound intent: the exact bytes of the intent the owner was opened with.
    auto bound = prepared.bound_intent();
    REQUIRE(bound);
    auto expected_intent = f.intent();
    auto expected_data   = expected_intent.data();
    REQUIRE(expected_data);
    REQUIRE(*expected_data);
    CHECK(bound->encoded == (*expected_data)->encoded);
    CHECK(bound->request_id == "req-1");
    CHECK(bound == d.bound_intent);
    // Interpretation bytes are present and within the codec bound.
    constexpr std::size_t bound_bytes = 16u * 1024u * 1024u;
    CHECK_FALSE(d.interpretation.encoded.empty());
    CHECK_FALSE(d.interpretation.versions_encoded.empty());
    CHECK(d.interpretation.encoded.size() <= bound_bytes);
    CHECK(d.interpretation.versions_encoded.size() <= bound_bytes);
    CHECK(d.interpretation.interpretation);
    CHECK(d.interpretation.versions);
  }
  // Each shape selects different job files, so the source identity cannot be a constant.
  CHECK(source_digests.size() == b3c_shapes.size());
}
TEST_CASE("X06 B3 C derived interpretation decodes against the raw effective tree with equal separate versions",
          "[runtime][x06b3c]") {
  for (auto [combined, split] : b3c_shapes) {
    INFO("combined=" << combined << " split=" << split);
    b3b_fixture f(b3b_fixture::default_body(), combined, split);
    auto        prepared = prepare_ok(f);
    auto        shared   = data_of(prepared);
    const auto& d        = *shared;
    auto        decoded =
        history::decode_interpretation_v2(d.interpretation.encoded, d.interpretation.versions_encoded, d.effective.files);
    REQUIRE(decoded);
    CHECK(decoded->encoded == d.interpretation.encoded);
    CHECK(decoded->versions_encoded == d.interpretation.versions_encoded);
    REQUIRE(d.interpretation.versions);
    REQUIRE(d.interpretation.versions->fields.contains("stdlib_digest"));
    CHECK(d.interpretation.versions->fields.at("stdlib_digest")->string == digest_hex(d.effective.digest));
    // The separate versions value carries the digest as one of its recorded fields, never an empty record.
    CHECK(d.interpretation.versions->fields.size() > 1);
    // The binding is to the raw tree: a one byte change of any effective file, or of the versions, is refused.
    auto tampered = d.effective.files;
    tampered.front().payload += "x";
    CHECK_FALSE(history::decode_interpretation_v2(d.interpretation.encoded, d.interpretation.versions_encoded, tampered));
    auto other_versions   = d.interpretation.versions_encoded;
    other_versions.back() = other_versions.back() == '0' ? '1' : '0';
    CHECK_FALSE(history::decode_interpretation_v2(d.interpretation.encoded, other_versions, d.effective.files));
  }
}
TEST_CASE("X06 B3 C published data is immutable shared and independent of the prepared object", "[runtime][x06b3c]") {
  static_assert(std::is_same_v<std::remove_cvref_t<decltype(*std::declval<const prepared_profile&>().data())>,
                               std::shared_ptr<const profile_acceptance_data>>);
  static_assert(std::is_const_v<std::remove_reference_t<decltype(**std::declval<const prepared_profile&>().data())>>);
  b3b_fixture                     f;
  std::optional<prepared_profile> prepared;
  prepared.emplace(prepare_ok(f));
  auto first  = data_of(*prepared);
  auto second = data_of(*prepared);
  CHECK(first.get() == second.get());
  CHECK(prepared->bound_intent().get() == first->bound_intent.get());
  // Snapshot every published value, then mutate copies of the vectors and strings: a later read is unchanged.
  const auto source_before        = first->source;
  const auto effective_before     = first->effective.files;
  const auto registry_before      = first->registry.files;
  const auto encoded_before       = first->interpretation.encoded;
  const auto versions_before      = first->interpretation.versions_encoded;
  const auto source_digest_before = first->source_digest;
  const auto workspace_id_before  = first->workspace_id;
  auto       source_copy          = first->source;
  source_copy.front().payload += "mutated";
  source_copy.pop_back();
  auto effective_copy = first->effective.files;
  effective_copy.front().payload.clear();
  auto encoded_copy = first->interpretation.encoded;
  encoded_copy += "mutated";
  auto third = data_of(*prepared);
  REQUIRE(third->source.size() == source_before.size());
  for (std::size_t i = 0; i < source_before.size(); ++i) {
    CHECK(third->source[i].path == source_before[i].path);
    CHECK(third->source[i].payload == source_before[i].payload);
  }
  CHECK(third->effective.files.front().payload == effective_before.front().payload);
  CHECK(third->interpretation.encoded == encoded_before);
  // The data survives destruction of the prepared object (VM, receipts and stage) and reads identically.
  std::weak_ptr<const profile_acceptance_data> weak = first;
  second.reset();
  third.reset();
  prepared.reset();
  REQUIRE_FALSE(weak.expired());
  CHECK(first.use_count() == 1);
  REQUIRE(first->source.size() == source_before.size());
  for (std::size_t i = 0; i < source_before.size(); ++i)
    CHECK(first->source[i].payload == source_before[i].payload);
  CHECK(first->source_digest == source_digest_before);
  CHECK(first->workspace_id == workspace_id_before);
  CHECK(first->interpretation.encoded == encoded_before);
  CHECK(first->interpretation.versions_encoded == versions_before);
  REQUIRE(first->registry.files.size() == registry_before.size());
  CHECK(first->registry.files[0].payload == registry_before[0].payload);
  // Two preparations of the same fixture publish equal values in distinct objects.
  auto again = prepare_ok(f);
  auto other = data_of(again);
  CHECK(other.get() != first.get());
  CHECK(other->source_digest == source_digest_before);
  CHECK(other->interpretation.encoded == encoded_before);
  first.reset();
  CHECK(weak.expired());
}
TEST_CASE("X06 B3 C success consumes the owner, refuses retry and moved-from prepared objects refuse", "[runtime][x06b3c]") {
  b3b_fixture f;
  auto        owner = f.open();
  CHECK(owner.observe().live);
  auto r = std::move(owner).prepare_load(f.intent());
  REQUIRE(r);
  CHECK_FALSE(owner.observe().live);
  expect_protocol(std::move(owner).prepare_load(f.intent()));
  auto ext = std::move(owner).into_extension();
  REQUIRE_FALSE(ext);
  // The prepared object itself is live and observable.
  CHECK(r->observe().live);
  CHECK(r->observe().frame_begun);
  auto expected_data = data_of(*r);
  auto p             = std::move(*r);
  // Moved-from: every operation refuses with a typed value, nothing is published and nothing throws.
  auto none = r->data();
  REQUIRE_FALSE(none);
  CHECK(none.error().field == "profile");
  CHECK(none.error().detail.code == "internal");
  CHECK(none.error().detail.message == "Invalid external profile protocol");
  CHECK_FALSE(r->bound_intent());
  auto verified = r->verify_native_receipts();
  expect_capture_changed(verified);
  CHECK(verified.error().message == "Captured inputs changed.");
  CHECK_FALSE(r->observe().live);
  // The destination keeps the same published object and stays live.
  CHECK(data_of(p).get() == expected_data.get());
  CHECK(p.observe().live);
  CHECK(p.verify_native_receipts());
  // Self-move preserves state.
  auto& self = p;
  p          = std::move(self);
  CHECK(p.observe().live);
  CHECK(data_of(p).get() == expected_data.get());
  CHECK(p.bound_intent());
  CHECK(p.verify_native_receipts());
  // Move assignment releases the old state of the destination and takes the source's.
  b3b_fixture                                  g;
  auto                                         q     = prepare_ok(g);
  std::weak_ptr<const profile_acceptance_data> old_q = data_of(q);
  auto                                         old_p = data_of(p);
  q                                                  = std::move(p);
  CHECK(old_q.expired());
  CHECK(data_of(q).get() == old_p.get());
  CHECK_FALSE(p.observe().live);
  CHECK_FALSE(p.data());
  CHECK_FALSE(p.bound_intent());
  expect_capture_changed(p.verify_native_receipts());
}
TEST_CASE("X06 B3 C verify_native_receipts is repeatable on an untouched prepared profile and reads without consuming",
          "[runtime][x06b3c]") {
  b3b_fixture f;
  auto        prepared = prepare_ok(f);
  const auto  before   = prepared.observe();
  const auto  data     = data_of(prepared);
  CHECK(prepared.verify_native_receipts());
  CHECK(prepared.verify_native_receipts());
  const auto after = prepared.observe();
  CHECK(after.live);
  CHECK(after.instructions == before.instructions);
  CHECK(after.native_bytes == before.native_bytes);
  CHECK(after.registrations == before.registrations);
  CHECK(data_of(prepared).get() == data.get());
  // Verification re-reads captured content (the positive control), reads the same amount every time and admits nothing
  // new: the counts for the declaration, a closure member, the registry, the profile and the module repeat exactly.
  for (const auto& path : {f.declaration, f.workspace_dir / "split.lua", f.registry, f.profile, f.module}) {
    INFO(path.string());
    watch_path(path);
    REQUIRE(prepared.verify_native_receipts());
    const auto first_reads = planar_admission_read_attempts();
    CHECK(first_reads > 0);
    REQUIRE(prepared.verify_native_receipts());
    CHECK(planar_admission_read_attempts() == 2 * first_reads);
  }
}
TEST_CASE("X06 B3 C verify_native_receipts fails with capture_changed after any captured input changes post publication",
          "[runtime][x06b3c]") {
  struct target {
    std::string                                              name;
    std::function<std::filesystem::path(const b3b_fixture&)> path;
  };
  const std::vector<target> targets{{"declaration", [](const b3b_fixture& f) { return f.declaration; }},
                                    {"specification", [](const b3b_fixture& f) { return f.workspace_dir / "spec.lua"; }},
                                    {"job file", [](const b3b_fixture& f) { return f.workspace_dir / "split.lua"; }},
                                    {"text reference", [](const b3b_fixture& f) { return f.workspace_dir / "text/shared.md"; }},
                                    {"registry", [](const b3b_fixture& f) { return f.registry; }},
                                    {"profile", [](const b3b_fixture& f) { return f.profile; }},
                                    {"module", [](const b3b_fixture& f) { return f.module; }}};
  for (const auto& t : targets) {
    for (bool content_only : {false, true}) {
      INFO(t.name << (content_only ? " same-size edit with restored mtime" : " leaf replaced"));
      b3b_fixture f;
      auto        prepared = prepare_ok(f);
      REQUIRE(prepared.verify_native_receipts());
      const auto path = t.path(f);
      if (content_only)
        edit_same_size_keep_mtime(path);
      else
        replace_leaf(path);
      auto verified = prepared.verify_native_receipts();
      expect_capture_changed(verified);
      // The refusal is stable: verification never retries or recaptures.
      expect_capture_changed(prepared.verify_native_receipts());
      // Published data stays readable and unchanged: verification neither consumes nor rewrites it.
      CHECK(prepared.observe().live);
      REQUIRE(prepared.data());
    }
  }
}
TEST_CASE("X06 B3 C replacement before publication refuses terminally and publishes nothing", "[runtime][x06b3c]") {
  struct target {
    std::string                                              name;
    std::function<std::filesystem::path(const b3b_fixture&)> path;
  };
  const std::vector<target> targets{{"declaration", [](const b3b_fixture& f) { return f.declaration; }},
                                    {"registry", [](const b3b_fixture& f) { return f.registry; }},
                                    {"profile", [](const b3b_fixture& f) { return f.profile; }},
                                    {"module", [](const b3b_fixture& f) { return f.module; }}};
  for (const auto& t : targets) {
    for (bool content_only : {false, true}) {
      INFO(t.name << (content_only ? " same-size edit with restored mtime" : " leaf replaced"));
      b3b_fixture f;
      auto        owner = f.open();
      const auto  path  = t.path(f);
      if (content_only)
        edit_same_size_keep_mtime(path);
      else
        replace_leaf(path);
      auto r = std::move(owner).prepare_load(f.intent());
      REQUIRE_FALSE(r);
      INFO(describe(r.error()));
      CHECK(r.error().failure.detail.code == "capture_changed");
      CHECK(r.error().failure.field == "profile");
      CHECK_FALSE(r.error().semantic);
      CHECK_FALSE(owner.observe().live);
      expect_protocol(std::move(owner).prepare_load(f.intent()));
    }
  }
  // A closure member is captured by prepare_load itself, so an edit before the call is captured fresh and published.
  b3b_fixture base;
  auto        unchanged = prepare_ok(base);
  b3b_fixture edited;
  const auto  split_path = edited.workspace_dir / "split.lua";
  replace_in_file(split_path, "budget_bytes = 131072", "budget_bytes = 131073");
  auto fresh = prepare_ok(edited);
  auto d     = data_of(fresh);
  auto it    = std::ranges::find(d->source, "split.lua", &history::tree_file::path);
  REQUIRE(it != d->source.end());
  CHECK(it->payload == read_file_bytes(split_path));
  CHECK(it->payload.find("131073") != std::string::npos);
  CHECK(d->source_digest != data_of(unchanged)->source_digest);
  CHECK(fresh.verify_native_receipts());
}
TEST_CASE("X06 B3 C derivation is charged to the one frame and counters never reset", "[runtime][x06b3c]") {
  for (auto [combined, split] : b3c_shapes) {
    INFO("combined=" << combined << " split=" << split);
    b3b_fixture f(b3b_fixture::default_body(), combined, split);
    auto        owner  = f.open();
    auto        before = owner.observe();
    auto        r      = std::move(owner).prepare_load(f.intent());
    REQUIRE(r);
    const auto seen = r->observe();
    const auto d    = data_of(*r);
    const auto n3   = seen.native_byte_marks[3];
    // Counters are cumulative: nothing reset them between validation and publication, nor since the owner opened.
    CHECK(seen.native_bytes >= n3);
    CHECK(seen.native_bytes > before.native_bytes);
    CHECK(seen.instructions >= seen.instruction_marks[3]);
    CHECK(seen.native_bytes >= seen.native_byte_marks[2]);
    // Everything after the last validation mark is native derivation charged to the same counter. Lower bound from the
    // published values alone: four times the encoded interpretation and versions, three times the sealed source bytes,
    // and twice the effective tree payload.
    std::size_t source_bytes = d->registry.files[0].payload.size();
    for (const auto& file : d->source)
      source_bytes += file.payload.size();
    std::size_t effective_bytes = 0;
    for (const auto& file : d->effective.files)
      effective_bytes += file.payload.size();
    const auto interpretation_bytes = d->interpretation.encoded.size() + d->interpretation.versions_encoded.size();
    const auto derivation_floor =
        4 * interpretation_bytes + 3 * source_bytes + 2 * effective_bytes + sizeof(profile_acceptance_data);
    CHECK(seen.native_bytes - n3 >= derivation_floor);
    CHECK(seen.native_bytes - n3 >= interpretation_bytes);
    // Observation, data reads and verification charge and reset nothing.
    (void)r->data();
    (void)r->verify_native_receipts();
    (void)r->bound_intent();
    const auto later = r->observe();
    CHECK(later.native_bytes == seen.native_bytes);
    CHECK(later.instructions == seen.instructions);
    CHECK(later.native_byte_marks == seen.native_byte_marks);
    // Independent twin preparations charge identically, so the figure is deterministic and not an accident of state.
    auto twin = prepare_ok(f);
    CHECK(twin.observe().native_bytes == seen.native_bytes);
    CHECK(twin.observe().instructions == seen.instructions);
  }
}
namespace {
struct sweep_outcome {
  bool        fired = false, prepared = false, verified = false;
  std::string failure_code, failure_field;
  bool        semantic   = false;
  bool        owner_live = false, cleanup_ran = false, retry_protocol = false;
  std::string payload_on_publish;
};
/// Opens a fresh owner on `f`, arms the allocator hook for exactly the prepare_load call and reports what happened.
sweep_outcome run_with_hook(const b3b_fixture& f, std::size_t trigger, bool throw_mode, const std::string& from,
                            const std::string& to, const std::function<void(const prepared_profile&, sweep_outcome&)>& after,
                            const std::function<void(alloc_probe&)>& arm_edit = {}) {
  sweep_outcome out;
  auto          owner    = f.open();
  auto          intent   = f.intent();
  probe                  = {};
  probe.armed            = true;
  probe.trigger          = trigger;
  probe.throw_on_trigger = throw_mode;
  probe.rename_from      = from.empty() ? nullptr : from.c_str();
  probe.rename_to        = to.empty() ? nullptr : to.c_str();
  if (arm_edit)
    arm_edit(probe);
  auto r         = std::move(owner).prepare_load(intent);
  probe.armed    = false;
  out.fired      = probe.fired;
  out.prepared   = r.has_value();
  out.owner_live = owner.observe().live;
  if (r) {
    out.verified = r->verify_native_receipts().has_value();
    if (after)
      after(*r, out);
  } else {
    out.failure_code   = r.error().failure.detail.code;
    out.failure_field  = r.error().failure.field;
    out.semantic       = static_cast<bool>(r.error().semantic);
    out.cleanup_ran    = r.error().cleanup_ran;
    auto again         = std::move(owner).prepare_load(f.intent());
    out.retry_protocol = !again && again.error().failure.detail.code == "internal" &&
                         again.error().failure.detail.message == "Invalid external profile protocol" &&
                         again.error().failure.field == "profile" && !again.error().semantic;
  }
  return out;
}
std::size_t count_allocations(const b3b_fixture& f) {
  auto owner  = f.open();
  auto intent = f.intent();
  probe       = {};
  probe.armed = true;
  auto r      = std::move(owner).prepare_load(intent);
  probe.armed = false;
  REQUIRE(r);
  return probe.count;
}
} // namespace
TEST_CASE("X06 B3 C a file replaced at any allocation inside prepare_load is never published as verified", "[runtime][x06b3c]") {
  struct target {
    std::string name;
    std::string relative;
    bool        captured_at_open; ///< True for the receipts the factory took; false for closure members prepare_load captures.
    std::function<std::filesystem::path(const b3b_fixture&)> path;
  };
  const std::vector<target> targets{
      {"declaration", "declaration", true, [](const b3b_fixture& f) { return f.declaration; }},
      {"registry", "registry", true, [](const b3b_fixture& f) { return f.registry; }},
      {"module", "module", true, [](const b3b_fixture& f) { return f.module; }},
      {"job file", "split.lua", false, [](const b3b_fixture& f) { return f.workspace_dir / "split.lua"; }},
      {"text reference", "text/shared.md", false, [](const b3b_fixture& f) { return f.workspace_dir / "text/shared.md"; }}};
  b3b_fixture f;
  const auto  total = count_allocations(f);
  REQUIRE(total > 1000);
  // Positions: every allocation of the tail, and an even stride of total/40 over the whole call.
  std::vector<std::size_t> positions;
  for (std::size_t back = 0; back < 16; ++back)
    positions.push_back(total - back);
  for (std::size_t n = 1; n < total; n += total / 40)
    positions.push_back(n);
  positions.push_back(total + 1);
  for (const auto& t : targets) {
    INFO(t.name);
    const auto path     = t.path(f);
    const auto original = read_file_bytes(path);
    // The job file must stay a valid declaration so an early replacement is captured fresh, not refused as semantic data.
    auto variant = same_size_variant(original);
    if (t.relative == "split.lua") {
      variant   = original;
      auto from = variant.find("131072");
      REQUIRE(from != std::string::npos);
      variant.replace(from, 6, "131073");
    }
    auto alt = path;
    alt += ".alt";
    const auto  alt_text = alt.string(), path_text = path.string();
    std::size_t refused = 0, fresh = 0, late = 0, never_fired = 0;
    std::size_t earliest_late = total + 2;
    // Closure members are checked again by both the final source reverification and the profile capture
    // reverification: sample the last two and a half thousand allocations densely so a widened window shows up as a
    // late publication far from the end.
    auto sampled = positions;
    if (!t.captured_at_open)
      for (std::size_t back = 16; back < 2500; back += 40)
        sampled.push_back(total - back);
    for (const auto n : sampled) {
      INFO("allocation " << n);
      put(path, original);
      put(alt, variant);
      auto       o       = run_with_hook(f, n, false, alt_text, path_text, [&](const prepared_profile& p, sweep_outcome& out) {
        auto d = p.data();
        if (d && *d && !t.captured_at_open) {
          auto it = std::ranges::find((*d)->source, t.relative, &history::tree_file::path);
          if (it != (*d)->source.end())
            out.payload_on_publish = it->payload;
        }
      });
      const auto on_disk = read_file_bytes(path);
      if (!o.fired) {
        ++never_fired;
        CHECK(o.prepared);
        CHECK(o.verified);
        CHECK(on_disk == original);
        continue;
      }
      CHECK(on_disk == variant);
      CHECK_FALSE(o.owner_live);
      if (!o.prepared) {
        ++refused;
        CHECK(o.failure_code == "capture_changed");
        CHECK(o.failure_field == "profile");
        CHECK_FALSE(o.semantic);
        CHECK(o.retry_protocol);
        continue;
      }
      // Published: the verifier must agree with the disk. Verified implies the published bytes are the disk bytes.
      if (t.captured_at_open) {
        CHECK_FALSE(o.verified);
        ++late;
        earliest_late = std::min(earliest_late, n);
      } else if (o.payload_on_publish == variant) {
        CHECK(o.verified);
        ++fresh;
      } else {
        CHECK(o.payload_on_publish == original);
        CHECK_FALSE(o.verified);
        ++late;
        earliest_late = std::min(earliest_late, n);
      }
    }
    CHECK(refused > 0);
    CHECK(never_fired == 1);
    // Publication with a stale capture is possible only after the last original receipt check: the trailing
    // allocations (measured: one), never a phase. The window widens to the whole derivation only when both
    // reverifications are skipped (a single skipped one is masked by the other).
    CHECK(late > 0);
    CHECK(total + 1 - earliest_late <= 4);
    if (!t.captured_at_open)
      CHECK(fresh > 0);
    put(path, original);
    std::filesystem::remove(alt);
  }
}
TEST_CASE("X06 B3 C an in place same size edit at any allocation inside prepare_load is never published as verified",
          "[runtime][x06b3c]") {
  // The edit keeps the inode and the size, and the modification time is restored, but the captures compare content,
  // so the restore is not load-bearing. Both the source capture reverification and the profile capture reverification
  // refuse the edit, so a single skipped source reverification is masked by the profile capture reverification. This
  // test was measured to guard a skip of both pre-publication reverifications and the ignoring of their results.
  struct target {
    std::string                                              name;
    std::string                                              relative;
    bool                                                     captured_at_open;
    std::function<std::filesystem::path(const b3b_fixture&)> path;
  };
  const std::vector<target> targets{
      {"declaration", "declaration", true, [](const b3b_fixture& f) { return f.declaration; }},
      {"registry", "registry", true, [](const b3b_fixture& f) { return f.registry; }},
      {"job file", "split.lua", false, [](const b3b_fixture& f) { return f.workspace_dir / "split.lua"; }},
      {"text reference", "text/shared.md", false, [](const b3b_fixture& f) { return f.workspace_dir / "text/shared.md"; }}};
  b3b_fixture f;
  const auto  total = count_allocations(f);
  REQUIRE(total > 1000);
  std::vector<std::size_t> positions;
  for (std::size_t back = 0; back < 16; ++back)
    positions.push_back(total - back);
  for (std::size_t n = 1; n < total; n += total / 20)
    positions.push_back(n);
  positions.push_back(total + 1);
  for (const auto& t : targets) {
    INFO(t.name);
    const auto path     = t.path(f);
    const auto original = read_file_bytes(path);
    REQUIRE(original.size() > 4);
    // The edited job file must stay a valid declaration, so an early edit is captured fresh, not refused as semantic data.
    auto        variant = same_size_variant(original);
    std::size_t offset  = original.size() / 2;
    if (t.relative == "split.lua") {
      offset = original.find("131072");
      REQUIRE(offset != std::string::npos);
      variant         = original;
      variant[offset] = '3';
    }
    const char  edit_byte = variant[offset];
    struct stat st{};
    REQUIRE(::stat(path.c_str(), &st) == 0);
#ifdef __APPLE__
    const struct timespec mtime = st.st_mtimespec;
#else
    const struct timespec mtime = st.st_mtim;
#endif
    const auto path_text = path.string();
    const auto restore   = [&] {
      put(path, original);
      const struct timespec times[2] = {{0, UTIME_OMIT}, mtime};
      REQUIRE(::utimensat(AT_FDCWD, path_text.c_str(), times, 0) == 0);
    };
    std::size_t refused = 0, fresh = 0, late = 0, never_fired = 0;
    std::size_t earliest_late = total + 2;
    auto        sampled       = positions;
    if (!t.captured_at_open)
      for (std::size_t back = 16; back < 2500; back += 40)
        sampled.push_back(total - back);
    for (const auto n : sampled) {
      INFO("allocation " << n);
      restore();
      auto o = run_with_hook(
          f, n, false, "", "",
          [&](const prepared_profile& p, sweep_outcome& out) {
            auto d = p.data();
            if (d && *d && !t.captured_at_open) {
              auto it = std::ranges::find((*d)->source, t.relative, &history::tree_file::path);
              if (it != (*d)->source.end())
                out.payload_on_publish = it->payload;
            }
          },
          [&](alloc_probe& pr) {
            pr.edit_path   = path_text.c_str();
            pr.edit_offset = offset;
            pr.edit_byte   = edit_byte;
            pr.edit_mtime  = mtime;
          });
      const auto on_disk = read_file_bytes(path);
      if (!o.fired) {
        ++never_fired;
        CHECK(o.prepared);
        CHECK(o.verified);
        CHECK(on_disk == original);
        continue;
      }
      CHECK(on_disk == variant);
      CHECK_FALSE(o.owner_live);
      if (!o.prepared) {
        ++refused;
        CHECK(o.failure_code == "capture_changed");
        CHECK(o.failure_field == "profile");
        CHECK_FALSE(o.semantic);
        CHECK(o.retry_protocol);
        continue;
      }
      if (t.captured_at_open) {
        CHECK_FALSE(o.verified);
        ++late;
        earliest_late = std::min(earliest_late, n);
      } else if (o.payload_on_publish == variant) {
        CHECK(o.verified);
        ++fresh;
      } else {
        CHECK(o.payload_on_publish == original);
        CHECK_FALSE(o.verified);
        ++late;
        earliest_late = std::min(earliest_late, n);
      }
    }
    CHECK(refused > 0);
    CHECK(never_fired == 1);
    CHECK(late > 0);
    CHECK(total + 1 - earliest_late <= 4);
    if (!t.captured_at_open)
      CHECK(fresh > 0);
    restore();
  }
}
TEST_CASE("X06 B3 C allocation failure after validation maps to a resource refusal and leaves nothing retryable",
          "[runtime][x06b3c]") {
  b3b_fixture f;
  const auto  total = count_allocations(f);
  REQUIRE(total > 1000);
  std::vector<std::size_t> positions;
  for (std::size_t back = 0; back < 32; ++back)
    positions.push_back(total - back);
  for (std::size_t back = 32; back < 2400; back += 97)
    positions.push_back(total - back);
  std::size_t resource = 0, verification = 0;
  for (const auto n : positions) {
    INFO("allocation " << n << " of " << total);
    auto o = run_with_hook(f, n, true, {}, {}, {});
    REQUIRE(o.fired);
    // A throw never yields a prepared object: nothing is published after a failure.
    REQUIRE_FALSE(o.prepared);
    CHECK_FALSE(o.semantic);
    CHECK(o.failure_field == "profile");
    CHECK(o.cleanup_ran);
    CHECK_FALSE(o.owner_live);
    // The owner is consumed and the first terminal failure is not replaced by the protocol refusal of a retry.
    CHECK(o.retry_protocol);
    if (o.failure_code == "resource_limit")
      ++resource;
    else {
      CHECK(o.failure_code == "capture_changed");
      ++verification;
    }
  }
  // std::bad_alloc maps to the external resource refusal; only a throw inside a native reverification maps to
  // capture_changed (the mapping of the capture layer, unchanged).
  CHECK(resource > 0);
  CHECK(verification > 0);
  // Other exception kinds cannot be injected through the allocator, so their mapping to a protocol refusal is not
  // exercised here.
  auto healthy = prepare_ok(f);
  CHECK(healthy.verify_native_receipts());
}
namespace {
struct derivation_figures {
  std::size_t charged = 0, source_bytes = 0, source_files = 0, interpretation_bytes = 0, effective_bytes = 0, effective_files = 0;
};
/// Prepares a freshly edited fixture and reports the native bytes charged after the last validation mark together with
/// the sizes of the values the derivation covers, all read from the published data.
derivation_figures derivation_of(const std::function<void(const b3b_fixture&)>& edit) {
  b3b_fixture f;
  edit(f);
  auto              owner = f.open();
  auto              r     = std::move(owner).prepare_load(f.intent());
  const std::string note  = r ? std::string("ok") : describe(r.error());
  INFO(note);
  REQUIRE(r);
  const auto         seen = r->observe();
  const auto         d    = data_of(*r);
  derivation_figures out;
  out.charged      = seen.native_bytes - seen.native_byte_marks[3];
  out.source_bytes = d->registry.files[0].payload.size();
  for (const auto& file : d->source)
    out.source_bytes += file.payload.size();
  out.source_files         = d->source.size();
  out.interpretation_bytes = d->interpretation.encoded.size() + d->interpretation.versions_encoded.size();
  for (const auto& file : d->effective.files)
    out.effective_bytes += file.payload.size();
  out.effective_files = d->effective.files.size();
  return out;
}
std::string padding(std::size_t n) {
  return std::string(n, 'p');
}
void append_to(const std::filesystem::path& p, std::string_view extra) {
  put(p, read_file_bytes(p) + std::string(extra));
}
} // namespace
TEST_CASE("X06 B3 C each derivation charge scales exactly with the bytes it covers", "[runtime][x06b3c]") {
  // Between two fixtures that differ only in sizes, the charges after the last validation mark differ by exactly
  // 6 per effective payload byte (phase 4 reassembly 4, encoder 2), 3 per sealed source byte and 4 per encoded
  // interpretation byte (post-encode charge); the per-file terms (4096 per sealed file, 1024 per effective file) are in
  // the expectation and stay unchanged because no variant adds a file. Removing any one of the byte charges, or
  // resetting the counter before the derivation, changes a difference.
  const auto base = derivation_of([](const b3b_fixture&) {});
  REQUIRE(base.charged > 0);
  constexpr std::size_t pad = 7001;
  struct variant {
    std::string                             name;
    std::function<void(const b3b_fixture&)> edit;
  };
  const std::vector<variant> variants{
      {"module bytes", [](const b3b_fixture& f) { append_to(f.module, "\n--" + padding(pad)); }},
      {"profile bytes", [](const b3b_fixture& f) { append_to(f.profile, std::string(pad, ' ')); }},
      {"referenced text bytes", [](const b3b_fixture& f) { append_to(f.workspace_dir / "text/shared.md", padding(pad)); }}};
  for (const auto& v : variants) {
    INFO(v.name);
    const auto other = derivation_of(v.edit);
    const auto delta = [](std::size_t after, std::size_t before) {
      return static_cast<std::int64_t>(after) - static_cast<std::int64_t>(before);
    };
    const auto expected =
        6 * delta(other.effective_bytes, base.effective_bytes) + 3 * delta(other.source_bytes, base.source_bytes) +
        4 * delta(other.interpretation_bytes, base.interpretation_bytes) + 4096 * delta(other.source_files, base.source_files) +
        1024 * delta(other.effective_files, base.effective_files);
    INFO("effective " << other.effective_bytes - base.effective_bytes << " source " << other.source_bytes - base.source_bytes
                      << " interpretation " << delta(other.interpretation_bytes, base.interpretation_bytes) << " files "
                      << delta(other.source_files, base.source_files));
    CHECK(delta(other.charged, base.charged) == expected);
    CHECK(other.charged > base.charged);
  }
}
TEST_CASE("X06 B3 C published workspace id follows the declaration and source modes are normalised", "[runtime][x06b3c]") {
  b3b_fixture f;
  replace_in_file(f.workspace_dir / "workspace.lua", "id = \"quota\"", "id = \"ledgerq\"");
  replace_in_file(f.workspace_dir / "spec.lua", "id = \"quota\"", "id = \"ledgerq\"");
  const auto executable = f.workspace_dir / "text/shared.md";
  REQUIRE(::chmod(executable.c_str(), 0755) == 0);
  put(f.workspace_dir / "unreferenced.txt", "never captured");
  const auto group_writable = f.workspace_dir / "text/quota-api.md";
  REQUIRE(::chmod(group_writable.c_str(), 0664) == 0);
  auto        prepared = prepare_ok(f);
  auto        shared   = data_of(prepared);
  const auto& d        = *shared;
  CHECK(d.workspace_id == "ledgerq");
  auto it = std::ranges::find(d.source, "text/shared.md", &history::tree_file::path);
  REQUIRE(it != d.source.end());
  CHECK(it->mode == 0755u);
  auto other = std::ranges::find(d.source, "text/quota-api.md", &history::tree_file::path);
  REQUIRE(other != d.source.end());
  CHECK(other->mode == 0644u);
  for (const auto& file : d.source) {
    CHECK((file.mode == 0644u || file.mode == 0755u));
    CHECK(file.path != "unreferenced.txt"); // Only the captured closure is sealed into the source tree.
  }
  // The identity is derived from the executable bit too: the same bytes with the bit cleared digest differently.
  auto disk = workspace_files_on_disk(f.workspace_dir);
  std::erase_if(disk, [](const auto& file) { return file.path == "unreferenced.txt"; });
  auto expected = history::digest_tree(views_of(disk), history::tree_profile::source);
  REQUIRE(expected);
  CHECK(d.source_digest == *expected);
  REQUIRE(::chmod(executable.c_str(), 0644) == 0);
  auto plain = prepare_ok(f);
  CHECK(data_of(plain)->source_digest != d.source_digest);
}
// ---- X06 D whole-invocation budget tests ----
namespace {
constexpr std::size_t budget_native_max                   = PLANAR_BUDGET_NATIVE_MAX;
constexpr std::size_t budget_instruction_max              = PLANAR_BUDGET_INSTRUCTION_MAX;
constexpr std::size_t budget_registration_instruction_max = PLANAR_BUDGET_REGISTRATION_INSTRUCTION_MAX;
constexpr std::size_t budget_node_max                     = PLANAR_BUDGET_NODE_MAX;
constexpr std::size_t hook_quantum = 1000; // runtime.cpp hook(): one count event adds exactly 1000 instructions.

struct probe_result {
  bool                       opened = false, prepared = false, semantic = false, cleanup_ran = false, owner_live_after = false;
  bool                       retry_protocol = false;
  std::string                code, message, limit, field;
  std::size_t                maximum = 0, temporary_after = 0, peak_temporary = 0;
  std::size_t                open_instructions = 0, open_native = 0, instructions = 0, native_bytes = 0, shape_dispatches = 0;
  std::array<std::size_t, 4> imarks{}, nmarks{};
};
template <class Registration> void take_registration(probe_result& p, const Registration& e) {
  p.code    = e.detail.code;
  p.message = e.detail.message;
  p.limit   = e.detail.limit;
  p.maximum = e.detail.maximum;
  p.field   = e.field;
}
// Opens, prepares and retries one fixture in the harness namespace `Inv` and flattens every observation.
template <class Inv> probe_result run_probe(const b3b_fixture& f) {
  probe_result p;
  auto         intent = f.intent();
  auto         opened = Inv::open_known_load(intent, f.ledger_path.string());
  if (!opened) {
    take_registration(p, opened.error());
    return p;
  }
  p.opened            = true;
  auto       owner    = std::move(*opened);
  const auto ob       = owner.observe();
  p.open_instructions = ob.instructions;
  p.open_native       = ob.native_bytes;
  auto prepared       = std::move(owner).prepare_load(f.intent());
  p.owner_live_after  = owner.observe().live;
  if (prepared) {
    p.prepared         = true;
    const auto seen    = prepared->observe();
    p.instructions     = seen.instructions;
    p.native_bytes     = seen.native_bytes;
    p.shape_dispatches = seen.shape_dispatches;
    p.imarks           = seen.instruction_marks;
    p.nmarks           = seen.native_byte_marks;
    p.cleanup_ran      = seen.cleanup_ran;
  } else {
    const auto& e = prepared.error();
    take_registration(p, e.failure);
    p.semantic        = static_cast<bool>(e.semantic);
    p.cleanup_ran     = e.cleanup_ran;
    p.temporary_after = e.temporary_entries_after_release;
    p.peak_temporary  = e.peak_temporary_entries;
  }
  auto retry       = std::move(owner).prepare_load(f.intent());
  p.retry_protocol = !retry && retry.error().failure.field == "profile" && retry.error().failure.detail.code == "internal" &&
                     retry.error().failure.detail.message == "Invalid external profile protocol" && !retry.error().semantic;
  return p;
}
std::string brief(const probe_result& p) {
  return std::format(
      "opened={} prepared={} semantic={} code={} limit={} field={} max={} instr={} native={} cleanup={} tmp_after={}", p.opened,
      p.prepared, p.semantic, p.code, p.limit, p.field, p.maximum, p.instructions, p.native_bytes, p.cleanup_ran,
      p.temporary_after);
}
} // namespace
namespace {
// Largest value in [lo, hi) that still passes, for a predicate that passes at lo and fails at hi and is monotone between.
// Callers bracket the boundary from a measured linear estimate, so a boundary outside the bracket fails the REQUIREs.
std::size_t last_passing(std::size_t lo, std::size_t hi, const std::function<bool(std::size_t)>& passes) {
  std::size_t probes = 2;
  REQUIRE(passes(lo));
  REQUIRE_FALSE(passes(hi));
  while (hi - lo > 1) {
    const auto mid = lo + (hi - lo) / 2;
    ++probes;
    (passes(mid) ? lo : hi) = mid;
  }
  INFO("bisect probes " << probes);
  REQUIRE(probes <= 24);
  return lo;
}
using budget_invocation = runtime_budget_test::profile_invocation;
// Fixture with optional registration-side module padding and frame-side referenced-text padding.
probe_result padded_probe(std::size_t module_pad, std::size_t text_pad) {
  b3b_fixture f;
  if (module_pad)
    append_to(f.module, "\n--" + padding(module_pad));
  if (text_pad)
    append_to(f.workspace_dir / "text/shared.md", padding(text_pad));
  return run_probe<budget_invocation>(f);
}
void expect_native_exhaustion(const probe_result& p) {
  INFO(brief(p) << " message=" << p.message);
  CHECK_FALSE(p.prepared);
  CHECK_FALSE(p.semantic);
  CHECK(p.code == "resource_limit");
  CHECK(p.limit == "native_bytes");
  CHECK(p.maximum == budget_native_max);
  CHECK(p.message == "Native conversion allocation exceeded");
}
void expect_terminal_cleaned(const probe_result& p) {
  CHECK(p.cleanup_ran);
  CHECK(p.temporary_after == 0);
  CHECK_FALSE(p.owner_live_after);
  CHECK(p.retry_protocol);
}
struct native_sweep {
  std::size_t pad = 0, step = 0, native_at_pass = 0, mark3_at_pass = 0;
};
// Finds the largest frame-side text pad that prepares under the small native ceiling, with exact per-byte slope.
native_sweep sweep_native_boundary(std::size_t module_pad) {
  constexpr std::size_t probe_pad = 20000;
  const auto            r0        = padded_probe(module_pad, 0);
  const auto            r1        = padded_probe(module_pad, probe_pad);
  INFO(brief(r0) << " | " << brief(r1));
  REQUIRE(r0.prepared);
  REQUIRE(r1.prepared);
  REQUIRE(r1.native_bytes > r0.native_bytes);
  native_sweep out;
  CHECK((r1.native_bytes - r0.native_bytes) % probe_pad == 0);
  out.step = (r1.native_bytes - r0.native_bytes) / probe_pad;
  REQUIRE(out.step > 0);
  const auto estimate = probe_pad + (budget_native_max - r1.native_bytes) / out.step;
  out.pad             = last_passing(estimate - 64, estimate + 64, [&](std::size_t pad) {
    const auto r = padded_probe(module_pad, pad);
    return r.prepared;
  });
  const auto pass     = padded_probe(module_pad, out.pad);
  REQUIRE(pass.prepared);
  out.native_at_pass = pass.native_bytes;
  out.mark3_at_pass  = pass.nmarks[3];
  const auto fail    = padded_probe(module_pad, out.pad + 1);
  expect_native_exhaustion(fail);
  return out;
}
bool same_counters(const profile_load_stage_observation& a, const profile_load_stage_observation& b) {
  return a.instructions == b.instructions && a.native_bytes == b.native_bytes && a.instruction_marks == b.instruction_marks &&
         a.native_byte_marks == b.native_byte_marks && a.registrations == b.registrations &&
         a.shape_dispatches == b.shape_dispatches && a.jobs == b.jobs && a.sealed == b.sealed && a.frame_begun == b.frame_begun;
}
} // namespace
TEST_CASE("X06 D capture stage charges every sealed byte to the one native counter", "[runtime][x06d]") {
  // Mutant I1: delete the capture-charge loop in prepare_load. Measured from the open-time native counter (so the
  // workspace declaration open, the native reads and the bundle copies are all inside the window), a padded captured
  // file moves mark(2) by exactly two bytes per payload byte: one native read charge and one retained copy charge.
  struct window {
    std::int64_t interval = 0, captured = 0;
  };
  const auto measure = [](const b3b_fixture& f) {
    auto       owner  = f.open();
    const auto opened = owner.observe().native_bytes;
    auto       r      = std::move(owner).prepare_load(f.intent());
    REQUIRE(r);
    const auto seen = r->observe();
    const auto d    = data_of(*r);
    window     w;
    w.interval = static_cast<std::int64_t>(seen.native_byte_marks[2]) - static_cast<std::int64_t>(opened);
    w.captured = static_cast<std::int64_t>(d->registry.files[0].payload.size());
    for (const auto& file : d->source)
      w.captured += static_cast<std::int64_t>(file.payload.size());
    return w;
  };
  b3b_fixture base;
  const auto  baseline = measure(base);
  CHECK(baseline.interval >= baseline.captured);
  for (std::size_t pad : {7001ul, 20011ul}) {
    INFO("pad " << pad);
    b3b_fixture f;
    append_to(f.workspace_dir / "text/shared.md", padding(pad));
    const auto padded = measure(f);
    CHECK(padded.interval - baseline.interval == 2 * static_cast<std::int64_t>(pad));
    CHECK(padded.captured - baseline.captured >= static_cast<std::int64_t>(pad));
    CHECK(padded.interval >= padded.captured + static_cast<std::int64_t>(pad));
  }
}
TEST_CASE("X06 D frame start inherits the factory counters and the prepared object keeps them", "[runtime][x06d]") {
  // Mutants I2: begin_invocation zeroing native_bytes, or re-registering the capture before the frame.
  b3b_fixture f;
  auto        owner  = f.open();
  const auto  before = owner.observe();
  CHECK(before.registrations == 1);
  CHECK(before.native_bytes > 0);
  auto r = std::move(owner).prepare_load(f.intent());
  REQUIRE(r);
  const auto seen = r->observe();
  // The workspace declaration open runs before the frame and charges the native counter directly, so mark(0) may exceed it.
  CHECK(seen.native_byte_marks[0] >= before.native_bytes);
  CHECK(seen.registrations == 1);
  CHECK(seen.registrations == before.registrations);
  CHECK(seen.native_bytes > before.native_bytes);
  CHECK(seen.native_byte_marks[0] <= seen.native_byte_marks[1]);
  CHECK_FALSE(owner.observe().live);
}
TEST_CASE("X06 D each custom shape dispatch adds to the one budget and nothing is refunded", "[runtime][x06d]") {
  // Mutant I3: restore instructions and native bytes after each shape dispatch.
  const auto  body = "local n=0; for i=1,100000 do n=n+1 end; " + b3b_fixture::default_body();
  b3b_fixture one(body, false, true);
  b3b_fixture two(body, true, true);
  const auto  a = run_probe<profile_invocation>(one);
  const auto  b = run_probe<profile_invocation>(two);
  INFO(brief(a) << " | " << brief(b));
  REQUIRE(a.prepared);
  REQUIRE(b.prepared);
  CHECK(a.shape_dispatches == 1);
  CHECK(b.shape_dispatches == 2);
  CHECK(b.instructions > a.instructions);
  CHECK(b.native_bytes > a.native_bytes);
  CHECK(b.instructions - a.instructions >= 200000 - 2000);
  CHECK(a.instructions >= 200000);
  // Mutants I3a (route refund), I3c (nested phase refund) and I3e (native-only dispatch refund): the route and validation
  // windows each keep their own consumption, and a dispatch's discarded conversion stays charged.
  CHECK(a.imarks[1] > a.imarks[0]);
  CHECK(a.nmarks[1] > a.nmarks[0]);
  CHECK(a.imarks[3] - a.imarks[2] >= 200000 - 2000);
  CHECK(b.imarks[3] - b.imarks[2] - (a.imarks[3] - a.imarks[2]) >= 200000 - 2000);
  // The extra payload rides in the converted response (an unconverted Lua table adds no native charge), so each dispatch's
  // conversion cost is observable and a refund of it is detectable.
  const auto  junk_body = std::string("local n=0; for i=1,100000 do n=n+1 end; ") +
                          "return engine.object{ok=true,value=engine.object{obligations=engine.array{},guidance=input.job.id..'" +
                          std::string(4000, 'x') + "'}}";
  b3b_fixture junk_one(junk_body, false, true);
  b3b_fixture junk_two(junk_body, true, true);
  const auto  c = run_probe<profile_invocation>(junk_one);
  const auto  d = run_probe<profile_invocation>(junk_two);
  INFO(brief(c) << " | " << brief(d));
  REQUIRE(c.prepared);
  REQUIRE(d.prepared);
  const auto window = [](const probe_result& p) {
    return static_cast<std::int64_t>(p.nmarks[3]) - static_cast<std::int64_t>(p.nmarks[2]);
  };
  // Two dispatches of the same body add exactly the per-dispatch cost; the 4000-byte converted payload is part of it.
  CHECK((window(d) - window(c)) - (window(b) - window(a)) >= 2000);
}
TEST_CASE("X06 D native ceiling spans registration and frame at a small ceiling", "[runtime][x06d]") {
  // Mutants I5: begin_invocation forgetting registration charges, or charge() comparing against the ceiling alone.
  const auto base = padded_probe(0, 0);
  INFO(brief(base));
  REQUIRE(base.prepared);
  REQUIRE(base.native_bytes + 2 * 1024 * 1024 <= budget_native_max);
  const auto plain      = sweep_native_boundary(0);
  const auto registered = sweep_native_boundary(65536);
  INFO("frame pad " << plain.pad << " with registration pad " << registered.pad);
  CHECK(registered.pad < plain.pad);
  CHECK(plain.step == registered.step);
  for (const auto& sweep : {plain, registered}) {
    CHECK(sweep.native_at_pass <= budget_native_max);
    CHECK(budget_native_max - sweep.native_at_pass < sweep.step * 2);
  }
  // The registration pad charges native bytes both at open and again in the frame, so the lost headroom exceeds the pad.
  CHECK((plain.pad - registered.pad) * plain.step >= 65536);
}
TEST_CASE("X06 D invocation instruction ceiling is shared by every stage at a small ceiling", "[runtime][x06d]") {
  // Mutant I6b: reset the instruction counter after each custom shape dispatch; also shows dispatches share the counter.
  // (Mutant I6, a reset between route and validation, is killed by the X06 B3 B route consumption test.)
  const auto make = [](bool two, std::size_t n) {
    b3b_fixture f("local n=0; for i=1,LOOPN do n=n+1 end; " + b3b_fixture::default_body(), two, true);
    replace_in_file(f.module, "LOOPN", std::to_string(n));
    return run_probe<budget_invocation>(f);
  };
  struct boundary {
    std::size_t n = 0, slope = 0, at_zero = 0, at_pass = 0;
  };
  const auto find = [&](bool two) {
    constexpr std::size_t reference = 200000;
    const auto            r0        = make(two, 0);
    const auto            r1        = make(two, reference);
    INFO(brief(r0) << " | " << brief(r1));
    REQUIRE(r0.prepared);
    REQUIRE(r1.prepared);
    boundary b;
    CHECK((r1.instructions - r0.instructions) % reference == 0);
    b.slope   = (r1.instructions - r0.instructions) / reference;
    b.at_zero = r0.instructions;
    REQUIRE(b.slope > 0);
    const auto estimate = reference + (budget_instruction_max - r1.instructions) / b.slope;
    b.n                 = last_passing(estimate - 1100, estimate + 1100, [&](std::size_t n) { return make(two, n).prepared; });
    const auto pass     = make(two, b.n);
    REQUIRE(pass.prepared);
    b.at_pass       = pass.instructions;
    const auto fail = make(two, b.n + 1);
    INFO(brief(fail));
    CHECK_FALSE(fail.prepared);
    CHECK_FALSE(fail.semantic);
    CHECK(fail.code == "resource_limit");
    CHECK(fail.limit == "instructions");
    CHECK(fail.maximum == budget_instruction_max);
    CHECK(fail.field.starts_with("catalog.shapes"));
    CHECK(fail.cleanup_ran);
    CHECK(fail.temporary_after == 0);
    CHECK_FALSE(fail.owner_live_after);
    CHECK(fail.retry_protocol);
    return b;
  };
  const auto one = find(false);
  const auto two = find(true);
  for (const auto& b : {one, two}) {
    CHECK(b.at_pass < budget_instruction_max);
    CHECK(budget_instruction_max - b.at_pass <= 2 * hook_quantum);
  }
  CHECK(two.n < one.n);
  const std::size_t slack = (two.at_zero > one.at_zero ? (two.at_zero - one.at_zero) / one.slope : 0) + 2 * hook_quantum;
  INFO("one " << one.n << " two " << two.n << " slack " << slack);
  CHECK(2 * two.n <= one.n + slack);
  CHECK(2 * two.n + slack >= one.n);
}
TEST_CASE("X06 D registration instruction ceiling is separate and refuses at open with no owner", "[runtime][x06d]") {
  // Mutant I7: the registration instruction limit replaced by the invocation ceiling or dropped.
  const auto make = [](std::size_t n) {
    b3b_fixture f;
    put(f.module, "for i=1," + std::to_string(n) + " do end\n" + read_file_bytes(f.module));
    return run_probe<budget_invocation>(f);
  };
  constexpr std::size_t reference = 100000;
  const auto            r0        = make(0);
  const auto            r1        = make(reference);
  INFO(brief(r0) << " | " << brief(r1));
  REQUIRE(r0.prepared);
  REQUIRE(r1.prepared);
  CHECK((r1.open_instructions - r0.open_instructions) % reference == 0);
  const auto slope = (r1.open_instructions - r0.open_instructions) / reference;
  REQUIRE(slope > 0);
  REQUIRE(budget_registration_instruction_max < budget_instruction_max);
  const auto estimate = reference + (budget_registration_instruction_max - r1.open_instructions) / slope;
  const auto boundary = last_passing(estimate - 3000, estimate + 3000, [&](std::size_t n) { return make(n).opened; });
  const auto pass     = make(boundary);
  REQUIRE(pass.opened);
  CHECK(pass.open_instructions < budget_registration_instruction_max);
  CHECK(budget_registration_instruction_max - pass.open_instructions <= 2 * hook_quantum);
  const auto fail = make(boundary + 1);
  INFO(brief(fail));
  CHECK_FALSE(fail.opened);
  CHECK(fail.code == "resource_limit");
  CHECK(fail.limit == "instructions");
  CHECK(fail.field == "profile.modules[1]");
  CHECK(fail.maximum == budget_registration_instruction_max);
}
TEST_CASE("X06 D conversion node ceiling refuses with nodes at a small ceiling", "[runtime][x06d]") {
  // Mutant I8: the node comparison scaled to twice the ceiling. Exactness is a recorded limit: only adjacent pass and fail.
  const auto make = [](std::size_t entries) {
    b3b_fixture f("local t={} for i=1,ENTRIES do t[i]=i end return engine.object{ok=true,value=engine.object{obligations="
                  "engine.array(t),guidance=input.job.id}}",
                  false, true);
    replace_in_file(f.module, "ENTRIES", std::to_string(entries));
    return run_probe<budget_invocation>(f);
  };
  const auto passes = [&](std::size_t entries) {
    const auto r = make(entries);
    return r.prepared || (r.semantic && r.code.empty());
  };
  // Measured adjacent refusing entry count for this fixture and the small test variant (C07 calibration).
  // A broad upper probe wastes native headroom while testing nodes. The upper-refuses prerequisite also directly kills
  // a doubled-node ceiling; the adjacent boundary, typed refusal and cleanup assertions below remain unchanged.
  constexpr std::size_t refusing_entries = 5481;
  const auto            boundary         = last_passing(0, refusing_entries, passes);
  INFO("last passing entries " << boundary);
  CHECK(boundary > 0);
  CHECK(boundary < budget_node_max);
  const auto fail = make(boundary + 1);
  INFO(brief(fail) << " message=" << fail.message);
  CHECK_FALSE(fail.prepared);
  CHECK_FALSE(fail.semantic);
  CHECK(fail.code == "resource_limit");
  CHECK(fail.limit == "nodes");
  CHECK(fail.maximum == budget_node_max);
  CHECK(fail.message == "Conversion node count exceeded");
  CHECK(fail.cleanup_ran);
  CHECK(fail.temporary_after == 0);
  CHECK_FALSE(fail.owner_live_after);
  CHECK(fail.retry_protocol);
}
TEST_CASE("X06 D native exhaustion is resource_limit terminal and cleans nested roots", "[runtime][x06d]") {
  // Mutants I9: cleanup skipped, or charge() clamping instead of refusing. (I1b needs a single capture-stage overshoot,
  // which small fixtures cannot reach: the capture charge is 1 per byte against 21 per byte elsewhere. Recorded limit.)
  {
    // (a) Registration side: four 1 MiB modules exhaust the native ceiling at open, so no owner exists.
    b3b_fixture f;
    std::string modules = "[\"module.lua\"";
    for (int k = 0; k < 4; ++k) {
      const auto name = "pad" + std::to_string(k) + ".lua";
      put(f.root / "trusted" / name, "--" + padding(1048576 - 2));
      modules += ",\"" + name + "\"";
    }
    put(f.profile, R"({"version":1,"modules":)" + modules + "]}");
    const auto p = run_probe<budget_invocation>(f);
    expect_native_exhaustion(p);
    CHECK_FALSE(p.opened);
    CHECK_FALSE(p.cleanup_ran);
  }
  {
    // (b) Frame side, one byte past the exact boundary and far past it through a near-maximum module.
    const auto sweep = sweep_native_boundary(0);
    const auto edge  = padded_probe(0, sweep.pad + 1);
    expect_native_exhaustion(edge);
    CHECK(edge.opened);
    expect_terminal_cleaned(edge);
    CHECK(edge.peak_temporary > 0);
    const auto far = padded_probe(1000000, 0);
    expect_native_exhaustion(far);
    CHECK(far.opened);
    expect_terminal_cleaned(far);
    CHECK(far.message == edge.message);
  }
}
TEST_CASE("X06 D a failure caught by Lua stays terminal with the first diagnostic", "[runtime][x06d]") {
  // Mutant I10: protected_call ignoring an exhausted instruction counter after the enclosing pcall caught the error.
  const auto make = [](std::size_t big) {
    b3b_fixture f("pcall(function() local n=0 for i=1,BIGLOOP do n=n+1 end end); " + b3b_fixture::default_body());
    replace_in_file(f.module, "BIGLOOP", std::to_string(big));
    return run_probe<budget_invocation>(f);
  };
  const auto control = make(1000);
  INFO(brief(control));
  CHECK(control.prepared);
  const auto fail = make(budget_instruction_max);
  INFO(brief(fail));
  CHECK_FALSE(fail.prepared);
  CHECK_FALSE(fail.semantic);
  CHECK(fail.code == "resource_limit");
  CHECK(fail.limit == "instructions");
  CHECK(fail.maximum == budget_instruction_max);
  CHECK(fail.field == "catalog.shapes[1]");
  expect_terminal_cleaned(fail);
}
TEST_CASE("X06 D moving the prepared profile transfers counters and move assignment is safe", "[runtime][x06d]") {
  // Mutant I11: the move constructor or assignment dropping the implementation instead of transferring it.
  b3b_fixture fa;
  b3b_fixture fb(b3b_fixture::default_body(), false, true);
  auto        a      = prepare_ok(fa);
  auto        b      = prepare_ok(fb);
  const auto  seen_a = a.observe();
  const auto  seen_b = b.observe();
  REQUIRE(seen_a.live);
  REQUIRE(seen_b.live);
  CHECK_FALSE(same_counters(seen_a, seen_b));
  prepared_profile c(std::move(a));
  CHECK(c.observe().live);
  CHECK(same_counters(c.observe(), seen_a));
  CHECK(c.verify_native_receipts());
  CHECK_FALSE(a.observe().live);
  auto refused = a.data();
  REQUIRE_FALSE(refused);
  CHECK(refused.error().detail.code == "internal");
  CHECK(refused.error().field == "profile");
  expect_capture_changed(a.verify_native_receipts());
  auto earlier = data_of(c);
  c            = std::move(b);
  CHECK(c.observe().live);
  CHECK(same_counters(c.observe(), seen_b));
  CHECK_FALSE(b.observe().live);
  CHECK(c.verify_native_receipts());
  CHECK_FALSE(earlier->workspace_id.empty());
  CHECK(earlier != data_of(c));
  auto& self = c;
  c          = std::move(self);
  CHECK(c.observe().live);
  CHECK(same_counters(c.observe(), seen_b));
  CHECK(c.verify_native_receipts());
}
TEST_CASE("X06 D destroying the consumed owner wrapper cannot erase prepared counters", "[runtime][x06d]") {
  // Mutant I12: prepare_load erasing the native counter when it marks the owner consumed.
  b3b_fixture                     f;
  std::optional<prepared_profile> kept;
  profile_load_stage_observation  before;
  {
    auto owner = f.open();
    auto r     = std::move(owner).prepare_load(f.intent());
    REQUIRE(r);
    kept.emplace(std::move(*r));
    {
      auto copy = data_of(*kept);
      CHECK_FALSE(copy->workspace_id.empty());
    }
    before = kept->observe();
    CHECK(before.native_bytes > 0);
    CHECK_FALSE(owner.observe().live);
  }
  const auto after = kept->observe();
  CHECK(same_counters(after, before));
  CHECK(after.live);
  CHECK(kept->verify_native_receipts());
}
TEST_CASE("X06 D lower inherited limits still win over the composed ceilings", "[runtime][x06d]") {
  // Mutant I4 (scratch edit of the capture module): raising a per-module, total or declaration limit to the composed value.
  const auto open_with_modules = [](std::size_t count, std::size_t each) {
    b3b_fixture f;
    std::string modules = "[\"module.lua\"";
    for (std::size_t k = 1; k < count; ++k) {
      const auto name = "pad" + std::to_string(k) + ".lua";
      put(f.root / "trusted" / name, "--" + padding(each - 2));
      modules += ",\"" + name + "\"";
    }
    put(f.profile, R"({"version":1,"modules":)" + modules + "]}");
    if (each != 0) {
      auto text = read_file_bytes(f.module);
      REQUIRE(text.size() < each);
      put(f.module, text + "\n--" + padding(each - text.size() - 3));
    }
    auto intent = f.intent();
    return profile_invocation::open_known_load(intent, f.ledger_path.string());
  };
  constexpr std::size_t one_mib = 1024 * 1024;
  {
    auto exact = open_with_modules(1, one_mib);
    CHECK(exact);
    auto over = open_with_modules(1, one_mib + 1);
    REQUIRE_FALSE(over);
    CHECK(over.error().detail.code == "resource_limit");
  }
  {
    auto total = open_with_modules(8, one_mib);
    CHECK(total);
    auto over = open_with_modules(9, one_mib);
    REQUIRE_FALSE(over);
    CHECK(over.error().detail.code == "resource_limit");
  }
  {
    b3b_fixture f;
    auto        text = read_file_bytes(f.declaration);
    put(f.declaration, "for i=1,1500000 do local n=i*i end\n" + text);
    const auto p = run_probe<profile_invocation>(f);
    INFO(brief(p) << " message=" << p.message);
    CHECK_FALSE(p.prepared);
    CHECK(p.code == "resource_limit");
    CHECK(p.limit == "instructions");
    CHECK(p.maximum == 1000000);
    CHECK(p.message == "Lua instruction limit exceeded");
  }
}
TEST_CASE("X06 D prepared_profile has no new construction path", "[runtime][x06d]") {
  // Mutant I17: a public default or converting constructor appearing on the prepared type.
  constexpr bool default_constructible = std::is_default_constructible_v<prepared_profile>;
  constexpr bool copy_constructible    = std::is_copy_constructible_v<prepared_profile>;
  constexpr bool copy_assignable       = std::is_copy_assignable_v<prepared_profile>;
  constexpr bool move_constructible    = std::is_move_constructible_v<prepared_profile>;
  constexpr bool from_data             = std::is_constructible_v<prepared_profile, profile_acceptance_data>;
  constexpr bool from_data_ptr        = std::is_constructible_v<prepared_profile, std::shared_ptr<const profile_acceptance_data>>;
  constexpr bool from_stage_view      = std::is_constructible_v<prepared_profile, profile_load_stage_observation>;
  constexpr bool from_invocation_view = std::is_constructible_v<prepared_profile, profile_invocation_observation>;
  constexpr bool from_value           = std::is_constructible_v<prepared_profile, value_ptr>;
  constexpr bool from_stage           = std::is_constructible_v<prepared_profile, profile_load_stage>;
  constexpr bool from_owner           = std::is_constructible_v<prepared_profile, profile_invocation>;
  CHECK(default_constructible == false);
  CHECK(copy_constructible == false);
  CHECK(copy_assignable == false);
  CHECK(move_constructible == true);
  CHECK(from_data == false);
  CHECK(from_data_ptr == false);
  CHECK(from_stage_view == false);
  CHECK(from_invocation_view == false);
  CHECK(from_value == false);
  CHECK(from_stage == false);
  CHECK(from_owner == false);
  constexpr bool stage_default = std::is_default_constructible_v<profile_load_stage>;
  constexpr bool stage_copy    = std::is_copy_constructible_v<profile_load_stage>;
  constexpr bool owner_default = std::is_default_constructible_v<profile_invocation>;
  constexpr bool owner_copy    = std::is_copy_constructible_v<profile_invocation>;
  CHECK(stage_default == false);
  CHECK(stage_copy == false);
  CHECK(owner_default == false);
  CHECK(owner_copy == false);
}
namespace {
// Adds `count` extra job files (ids a1..aN, sorting before every other job, copies of the combined job) so a fixture can carry
// more prefixable declaration chunks than the per-chunk limit would otherwise allow under a raised composed ceiling. Returns
// their file names.
std::vector<std::string> add_extra_jobs(const b3b_fixture& f, std::size_t count) {
  std::vector<std::string> names;
  for (std::size_t i = 1; i <= count; ++i) {
    const auto id   = "a" + std::to_string(i);
    auto       job  = read_file_bytes(f.workspace_dir / "combined.lua");
    const auto from = std::string_view("id = \"combined\"");
    auto       at   = job.find(from);
    REQUIRE(at != std::string::npos);
    job.replace(at, from.size(), "id = \"" + id + "\"");
    put(f.workspace_dir / (id + ".lua"), job);
    replace_in_file(f.workspace_dir / "workspace.lua", "{ id = \"combined\", file = \"combined.lua\" }",
                    "{ id = \"" + id + "\", file = \"" + id + ".lua\" }, { id = \"combined\", file = \"combined.lua\" }");
    names.push_back(id + ".lua");
  }
  return names;
}
void prefix_file(const std::filesystem::path& p, std::string_view head) {
  put(p, std::string(head) + read_file_bytes(p));
}
// Extra-job counts and chunk/shape spins follow the measured 29,000,000-instruction ceiling. Each declaration stays below
// its inherited 1,000,000 bound; source costs, formulas and adjacent controls are recorded in
// docs/vernacular/rounds/V5-02-R15-R18-C07-calibration.md.
constexpr std::size_t c01_extra_jobs_stages = 10;
constexpr std::size_t c01_extra_jobs_sink   = 25;
constexpr std::size_t c01_extra_jobs_first  = 24;
constexpr std::size_t c01_extra_jobs_native = 25;
// One empty counting loop: about one VM instruction per iteration.
std::string spin(std::size_t iterations) {
  return "for i=1," + std::to_string(iterations) + " do end\n";
}
} // namespace
TEST_CASE("X06 D C01 workspace declaration instructions are carried into the invocation counter", "[runtime][x06d][c01]") {
  // Mutant: begin_invocation zeroes the instruction counter without carrying the pre-frame declaration instructions.
  const auto measure = [](std::size_t n) {
    b3b_fixture f;
    prefix_file(f.declaration, spin(n));
    return prepare_ok(f).observe().instruction_marks[0];
  };
  const auto small = measure(100000);
  const auto large = measure(200000);
  INFO("small " << small << " large " << large);
  CHECK(small >= 100000 - hook_quantum);
  REQUIRE(large > small);
  CHECK(large - small >= 90000);
}
TEST_CASE("X06 D C01 spec and job declaration instructions join the invocation counter", "[runtime][x06d][c01]") {
  // Mutant: the spec or job declaration is loaded without the factory budget, so its chunk never reaches the counter.
  std::vector<std::size_t> deltas;
  for (const char* name : {"spec.lua", "combined.lua", "split.lua"}) {
    INFO(name);
    const auto total = [&](std::size_t n) {
      b3b_fixture f;
      prefix_file(f.workspace_dir / name, spin(n));
      return prepare_ok(f).observe().instructions;
    };
    const auto small = total(100000);
    const auto large = total(200000);
    REQUIRE(large > small);
    CHECK(large - small >= 90000);
    deltas.push_back(large - small);
  }
  for (const auto delta : deltas) {
    const auto diff = delta > deltas[0] ? delta - deltas[0] : deltas[0] - delta;
    CHECK(diff <= 2 * hook_quantum);
  }
}
TEST_CASE("X06 D C01 declaration chunks exhaust the composed invocation instruction ceiling across stages",
          "[runtime][x06d][c01]") {
  // Mutants: declaration chunks are not charged to the factory budget, the composed refusal is not enforced, or it reports
  // the inherited 1,000,000 chunk maximum. Four chunks of 900,000 stay below the per-chunk limit and only together
  // (with the custom shape loop) approach the 4,000,000 composed ceiling.
  const auto make = [](std::size_t last) {
    b3b_fixture f("local n=0; for i=1,8170000 do n=n+1 end; " + b3b_fixture::default_body(), false, true);
    const auto  extras = add_extra_jobs(f, c01_extra_jobs_stages);
    for (const char* name : {"workspace.lua", "spec.lua", "combined.lua"})
      prefix_file(f.workspace_dir / name, spin(900000));
    for (const auto& name : extras)
      prefix_file(f.workspace_dir / name, spin(900000));
    prefix_file(f.workspace_dir / "split.lua", spin(last));
    return run_probe<budget_invocation>(f);
  };
  const auto r0 = make(0);
  const auto r1 = make(100000);
  INFO(brief(r0) << " | " << brief(r1));
  REQUIRE(r0.prepared);
  REQUIRE(r1.prepared);
  REQUIRE(r1.instructions > r0.instructions);
  CHECK((r1.instructions - r0.instructions) % 100000 == 0);
  const auto slope = (r1.instructions - r0.instructions) / 100000;
  REQUIRE(slope > 0);
  REQUIRE(r1.instructions < budget_instruction_max);
  CHECK(r0.instructions > (3 + c01_extra_jobs_stages) * 900000);
  const auto estimate = 100000 + (budget_instruction_max - r1.instructions) / slope;
  const auto boundary = last_passing(estimate - 1100, estimate + 1100, [&](std::size_t n) { return make(n).prepared; });
  const auto pass     = make(boundary);
  REQUIRE(pass.prepared);
  CHECK(pass.instructions < budget_instruction_max);
  CHECK(budget_instruction_max - pass.instructions <= 2 * hook_quantum);
  const auto fail = make(boundary + 1);
  INFO(brief(fail) << " message=" << fail.message);
  CHECK_FALSE(fail.prepared);
  CHECK_FALSE(fail.semantic);
  CHECK(fail.code == "resource_limit");
  CHECK(fail.limit == "instructions");
  CHECK(fail.maximum == budget_instruction_max);
  CHECK(fail.maximum != 1000000);
  CHECK(fail.message == "Lua instruction limit exceeded");
  expect_terminal_cleaned(fail);
}
TEST_CASE("X06 D C01 retained declaration values exhaust the composed native ceiling in the declaration stage",
          "[runtime][x06d][c01]") {
  // Mutants: the declaration loader charges its conversion bytes only against its own 64 MiB cap, or reports that cap. A
  // discarded planar.list of K booleans costs 640 bytes per element, crossing the small composed ceiling well below both
  // the loader's own native cap and its 100,000 node cap.
  const auto make = [](std::size_t k) {
    b3b_fixture f;
    prefix_file(f.workspace_dir / "combined.lua",
                "local big={} for i=1," + std::to_string(k) + " do big[i]=true end local keep=planar.list(big)\n");
    return run_probe<budget_invocation>(f);
  };
  const std::size_t hi       = budget_native_max / 640 + 1;
  const auto        boundary = last_passing(0, hi, [&](std::size_t k) { return make(k).prepared; });
  INFO("last passing elements " << boundary);
  CHECK(boundary > 0);
  CHECK(boundary < hi);
  const auto fail = make(boundary + 1);
  expect_native_exhaustion(fail);
  expect_terminal_cleaned(fail);
}
TEST_CASE("X06 D C01 declaration Lua memory counts against the composed live Lua ceiling", "[runtime][x06d][c01]") {
  // Mutants: declaration Lua memory is not live memory against the composed ceiling, or the refusal reports the loader's
  // own 64 MiB maximum. About 50 MB of live strings exceeds the small ceiling and stays under 64 MiB; about 13 MB passes.
  const auto run = [](std::string_view text) {
    b3b_fixture f;
    prefix_file(f.workspace_dir / "combined.lua", text);
    return run_probe<budget_invocation>(f);
  };
  const auto small = run("local x='a' for i=1,22 do x=x..x end local y=x..x\n");
  INFO(brief(small));
  CHECK(small.prepared);
  const auto big = run("local x='a' for i=1,23 do x=x..x end local y=x..x local z=y..x\n");
  INFO(brief(big) << " message=" << big.message);
  CHECK_FALSE(big.prepared);
  CHECK_FALSE(big.semantic);
  CHECK(big.code == "resource_limit");
  CHECK(big.limit == "lua_bytes");
  CHECK(big.maximum == budget_native_max);
  expect_terminal_cleaned(big);
}
TEST_CASE("X06 D C01 admission metadata charges are part of the factory counter at open", "[runtime][x06d][c01]") {
  // Mutant: the workspace declaration open is not charged, so its path and receipt bytes never reach the counter.
  constexpr std::size_t short_name = 8, delta = 200;
  const auto            open_native = [](std::size_t length) {
    b3b_fixture f;
    const auto  dir = f.root / std::string(length, 'w');
    std::filesystem::rename(f.workspace_dir, dir);
    f.workspace_dir = dir;
    f.declaration   = dir / "workspace.lua";
    const auto p    = run_probe<budget_invocation>(f);
    INFO(brief(p));
    REQUIRE(p.opened);
    return p.open_native;
  };
  const auto shorter = open_native(short_name);
  const auto longer  = open_native(short_name + delta);
  INFO("short " << shorter << " long " << longer);
  REQUIRE(longer > shorter);
  CHECK((longer - shorter) % delta == 0);
}
TEST_CASE("X06 D C01 native boundary is exercised by the post-validation derivation charges", "[runtime][x06d][c01]") {
  // Mutants: a derivation charge after mark(3) is unchecked or removed. At the exact boundary the validation mark sits far
  // below the ceiling, so the refusal one byte later comes from the later phases.
  const auto sweep = sweep_native_boundary(0);
  INFO("pad " << sweep.pad << " mark3 " << sweep.mark3_at_pass << " final " << sweep.native_at_pass);
  CHECK(sweep.mark3_at_pass + sweep.step < budget_native_max);
  CHECK(budget_native_max - sweep.mark3_at_pass > 4096);
  CHECK(sweep.native_at_pass >= sweep.mark3_at_pass);
}
TEST_CASE("X06 D C01 a caught Lua error still leaves the budget counters monotone", "[runtime][x06d][c01]") {
  // Mutant: a shape that catches an error with pcall restores the instruction or native counters it had before the call.
  const auto make = [](std::string_view prefix) {
    b3b_fixture f(std::string(prefix) + b3b_fixture::default_body());
    return run_probe<budget_invocation>(f);
  };
  const auto base = make("local n=0; for i=1,1000 do n=n+1 end; ");
  REQUIRE(base.prepared);
  for (const char* caught : {"local ok=pcall(function() for i=1,100000 do end error({}) end); ",
                             "local ok=pcall(function() for i=1,100000 do end error('x') end); "}) {
    INFO(caught);
    // The caught call itself runs a loop well above the hook granularity (100 instructions) so its cost is observable.
    const auto r = make(std::string("local n=0; for i=1,1000 do n=n+1 end; ") + caught);
    INFO(brief(r));
    REQUIRE(r.prepared);
    for (std::size_t i = 0; i + 1 < r.imarks.size(); ++i) {
      CHECK(r.imarks[i] <= r.imarks[i + 1]);
      CHECK(r.nmarks[i] <= r.nmarks[i + 1]);
    }
    CHECK(r.instructions >= r.imarks[3]);
    CHECK(r.native_bytes >= r.nmarks[3]);
    CHECK(r.instructions > base.instructions);
    CHECK(r.imarks[3] - r.imarks[2] > base.imarks[3] - base.imarks[2]);
  }
}
// Observable half of the 25 percent margin rule: docs/vernacular/rounds/V5-02-R15-R18-C07-calibration.md, under X06-D-C04
// D9-D11. The ordinary control costs 31,121,964 native bytes, 131,200 instructions, 22,643,664 open native bytes and 5,000
// open instructions. Live Lua, nodes and role-flow native are measured in scratch copies without adding public fields.
// These checks also reject a ceiling between the measured usage and its required four-thirds margin.
TEST_CASE("X06 D C03 ordinary fixtures keep a quarter of every observable ceiling free", "[runtime][x06d][c03]") {
  const auto p = padded_probe(0, 0);
  INFO(brief(p));
  REQUIRE(p.opened);
  REQUIRE(p.prepared);
  CHECK(p.native_bytes * 4 <= budget_native_max * 3);
  CHECK(p.instructions * 4 <= budget_instruction_max * 3);
  CHECK(p.open_native * 4 <= budget_native_max * 3);
  CHECK(p.open_instructions * 4 <= budget_registration_instruction_max * 3);
  // The margin checks above are upper bounds on usage and cannot notice a ceiling lowered toward the heaviest must-succeed
  // fixture, so derive that fixture: the native sweep boundary run. Instruction and native cost per pad byte come from two
  // padded probes, are extrapolated to the native ceiling, and the instruction ceiling must keep a quarter free of the
  // extrapolated boundary cost. No ceiling number is hard-coded.
  constexpr std::size_t probe_pad = 20000;
  const auto            q         = padded_probe(0, probe_pad);
  INFO(brief(q));
  REQUIRE(q.prepared);
  REQUIRE(q.native_bytes > p.native_bytes);
  REQUIRE(q.instructions > p.instructions);
  // q is an ordinary sample too: its larger native charge must retain the same headroom as p.
  CHECK(q.native_bytes * 4 <= budget_native_max * 3);
  CHECK(q.instructions * 4 <= budget_instruction_max * 3);
  CHECK(q.open_native * 4 <= budget_native_max * 3);
  CHECK(q.open_instructions * 4 <= budget_registration_instruction_max * 3);
  const auto native_slope      = (q.native_bytes - p.native_bytes) / probe_pad;
  const auto instruction_slope = (q.instructions - p.instructions) / probe_pad;
  REQUIRE(native_slope > 0);
  REQUIRE(budget_native_max > p.native_bytes);
  const auto boundary_pad = (budget_native_max - p.native_bytes) / native_slope;
  const auto extrapolated = p.instructions + instruction_slope * boundary_pad;
  INFO("extrapolated boundary instructions " << extrapolated);
  CHECK(budget_instruction_max * 3 >= 4 * extrapolated);
  // Registration: the heaviest must-succeed registration run is the ordinary open (5,000 instructions); the registration
  // bisect fixture is a bracketing fixture and has no derived bound here, so the ceiling stays pinned at the final value
  // (mutant: REGISTRATION_INSTRUCTION_MAX lowered toward the ordinary open use, which the margin check alone passes).
  CHECK(budget_registration_instruction_max == 1000000);
}
// Mutants T3, T3b, T3c, T3d (off-by-one) and T16 (wrong maximum): factory_budget::charge_instructions skips or shifts the
// ceiling comparison. The fixture has no custom shape, so no runtime VM tick follows the last declaration chunk and the
// runtime hook cannot mask the sink: the last passing total is exactly one loader quantum (100) below the ceiling.
TEST_CASE("X06 D C01 the composed sink alone enforces the invocation instruction ceiling on declaration chunks",
          "[runtime][x06d][c01]") {
  const auto make = [](std::size_t last) {
    b3b_fixture f(b3b_fixture::default_body(), false, false);
    const auto  extras = add_extra_jobs(f, c01_extra_jobs_sink);
    for (const char* name : {"workspace.lua", "spec.lua", "combined.lua"})
      prefix_file(f.workspace_dir / name, spin(985000));
    for (const auto& name : extras)
      prefix_file(f.workspace_dir / name, spin(985000));
    prefix_file(f.workspace_dir / "split.lua", spin(last));
    return run_probe<budget_invocation>(f);
  };
  const auto r0 = make(0);
  const auto r1 = make(100000);
  INFO(brief(r0) << " | " << brief(r1));
  REQUIRE(r0.prepared);
  REQUIRE(r1.prepared);
  REQUIRE(r1.instructions > r0.instructions);
  const auto slope = (r1.instructions - r0.instructions) / 100000;
  REQUIRE(slope == 1);
  const auto estimate = 100000 + (budget_instruction_max - r1.instructions) / slope;
  const auto boundary = last_passing(estimate - 1100, estimate + 1100, [&](std::size_t n) { return make(n).prepared; });
  const auto pass     = make(boundary);
  REQUIRE(pass.prepared);
  CHECK(budget_instruction_max - pass.instructions == 100);
  const auto fail = make(boundary + 1);
  INFO(brief(fail) << " message=" << fail.message);
  CHECK_FALSE(fail.prepared);
  CHECK_FALSE(fail.semantic);
  CHECK(fail.code == "resource_limit");
  CHECK(fail.limit == "instructions");
  CHECK(fail.maximum == budget_instruction_max);
  CHECK(fail.message == "Lua instruction limit exceeded");
  expect_terminal_cleaned(fail);
}
// Mutant T5e: the attached factory sink's release_lua is a no-op, so each declaration chunk's live Lua bytes stay charged.
// Each of the four chunks peaks near 12.6 MB of live strings, which fits the 26.7 MB ceiling only when freed between chunks.
TEST_CASE("X06 D C01 declaration live Lua memory is released between chunks", "[runtime][x06d][c01]") {
  b3b_fixture f;
  for (const char* name : {"workspace.lua", "spec.lua", "combined.lua", "split.lua"})
    prefix_file(f.workspace_dir / name, "local x='a' for i=1,22 do x=x..x end local y=x..x\n");
  const auto r = run_probe<budget_invocation>(f);
  INFO(brief(r) << " message=" << r.message);
  CHECK(r.opened);
  CHECK(r.prepared);
}
// Mutant T14: open_opaque opens the capture without the factory budget. The profile and module reads are charged by the
// sink at open for both factories, so one padded module byte moves both owners' counters by the same amount.
TEST_CASE("X06 D C01 opaque factory charges module reads like the known-load factory", "[runtime][x06d][c01]") {
  const auto counters = [](std::size_t pad) {
    b3b_fixture f;
    if (pad)
      append_to(f.module, "\n--" + padding(pad));
    auto known  = f.open();
    auto opaque = profile_invocation::open_opaque(f.profile.string());
    REQUIRE(opaque);
    return std::pair(known.observe().native_bytes, opaque->observe().native_bytes);
  };
  const auto [known0, opaque0] = counters(0);
  const auto [known1, opaque1] = counters(20000);
  CHECK(known1 - known0 >= 20000);
  CHECK(opaque1 - opaque0 == known1 - known0);
}
// Mutant I3e (public placement): invoke_shape refunds the native bytes charged while running and encoding the shape. Repeated
// direct calls share one cumulative native budget, so a response of a quarter megabyte per call exhausts the small ceiling.
TEST_CASE("X06 D C01 repeated direct shape calls never refund native bytes", "[runtime][x06d][c01]") {
  auto p     = std::make_shared<profile_closure>();
  p->profile = {"profile", "profile.json", "0644", R"({"version":1,"modules":["m1.lua"]})"};
  p->modules.push_back(
      {"modules", "m1.lua", "0644",
       "registration.shape({id='custom',abi='shape_v1',revision='final',stages='optional',help='shape'},function(input) "
       "local s='01234567' for i=1,15 do s=s..s end return engine.object{big=s} end)"});
  auto vm = runtime_budget_test::command_vm::create(p);
  REQUIRE(vm);
  auto        input     = object({{"version",
                                   [] {
                          auto v     = std::make_shared<value>();
                          v->kind    = value_kind::integer;
                          v->integer = 1;
                          return v;
                                   }()},
                                  {"job", object()},
                                  {"spec", object()},
                                  {"assignments", object()},
                                  {"reviewer", object()}});
  std::size_t calls     = 0;
  bool        exhausted = false;
  for (; calls < 400 && !exhausted; ++calls) {
    auto response = vm->invoke_shape("custom", input);
    if (!response) {
      exhausted = true;
      CHECK(response.error().detail.code == "resource_limit");
      CHECK(response.error().detail.limit == "native_bytes");
      CHECK(response.error().detail.maximum == budget_native_max);
    }
  }
  INFO("calls " << calls);
  CHECK(exhausted);
  CHECK(calls > 4);
}

// ---- X06 D C02 composed-sink, stage and terminal-failure tests ----
namespace {
using tight_invocation                      = runtime_tight_test::profile_invocation;
constexpr std::size_t tight_instruction_max = PLANAR_TIGHT_INSTRUCTION_MAX;
// Adds a third job file that sorts before the others, so a workspace has four prefixable declaration chunks after the
// workspace chunk's own (workspace, spec and three jobs).
void add_alpha_job(const b3b_fixture& f) {
  auto job = read_file_bytes(f.workspace_dir / "combined.lua");
  auto at  = job.find("id = \"combined\"");
  REQUIRE(at != std::string::npos);
  job.replace(at, std::string_view("id = \"combined\"").size(), "id = \"alpha\"");
  put(f.workspace_dir / "alpha.lua", job);
  replace_in_file(f.workspace_dir / "workspace.lua", "{ id = \"combined\", file = \"combined.lua\" }",
                  "{ id = \"alpha\", file = \"alpha.lua\" }, { id = \"combined\", file = \"combined.lua\" }");
}
} // namespace
// Mutants T3 (delete the post-frame comparison in charge_instructions), T3d (`>` for `>=`) and T16 (maximum + 1). The last
// chunk errors after a spin, so the composed sink refusal must be the first failure; the boundary is anchored on the valid
// control's public stage marks instead of on its own refusal point.
TEST_CASE("X06 D C02 the composed sink refusal is the first failure on a declaration chunk that crosses the invocation ceiling",
          "[runtime][x06d][c02]") {
  // Raised controls put equality at 992,700 in the last chunk for the measured 29,000,000 ceiling;
  // smaller prefixes can let the inherited chunk limit mask the composed refusal (C07 calibration, Dependent controls).
  constexpr std::size_t prefix = 999000;
  const auto            make   = [&](std::size_t last, bool erroring) {
    b3b_fixture f(b3b_fixture::default_body(), false, false);
    add_alpha_job(f);
    const auto extras = add_extra_jobs(f, c01_extra_jobs_first);
    for (const char* name : {"workspace.lua", "spec.lua", "alpha.lua", "combined.lua"})
      prefix_file(f.workspace_dir / name, spin(prefix));
    for (const auto& name : extras)
      prefix_file(f.workspace_dir / name, spin(prefix));
    prefix_file(f.workspace_dir / "split.lua", spin(last) + (erroring ? "error('refused')\n" : ""));
    return run_probe<budget_invocation>(f);
  };
  // Valid controls at two sizes a hundred apart in loader quanta: the capture stage (marks 1 to 2) pays the quantized chunk
  // charges, so the slope is exactly one instruction per spin iteration.
  constexpr std::size_t a = 100000, b = 200000;
  const auto            va = make(a, false);
  const auto            vb = make(b, false);
  INFO(brief(va) << " | " << brief(vb));
  REQUIRE(va.prepared);
  REQUIRE(vb.prepared);
  CHECK(va.imarks[1] == vb.imarks[1]);
  REQUIRE(vb.imarks[2] - vb.imarks[1] > va.imarks[2] - va.imarks[1]);
  CHECK((vb.imarks[2] - va.imarks[2]) == (b - a));
  // The exact-equality case must be reachable, otherwise `>=` and `>` could not differ: the running total moves in whole
  // loader quanta, so the ceiling is met exactly only when the control's capture total is congruent to it.
  CHECK((budget_instruction_max - va.imarks[2]) % 100 == 0);
  const std::size_t n_eq = a + (budget_instruction_max - va.imarks[2]);
  // Quantum phase of the valid chunk: the smallest k at which the total steps above its value at `a`.
  std::size_t rho = 0;
  {
    const auto probe = make(a + 99, false);
    REQUIRE(probe.prepared);
    if (probe.imarks[2] != va.imarks[2]) {
      std::size_t lo = 0, hi = 99; // total at a+lo == va, total at a+hi > va
      while (hi - lo > 1) {
        const auto mid = lo + (hi - lo) / 2;
        const auto r   = make(a + mid, false);
        REQUIRE(r.prepared);
        (r.imarks[2] == va.imarks[2] ? lo : hi) = mid;
      }
      rho = 100 - hi;
    }
  }
  // The valid chunk would first reach the ceiling exactly at n_valid; the erroring chunk differs only by its (shorter) tail.
  const std::size_t n_valid       = n_eq - rho;
  const auto        reached_error = [&](std::size_t n) {
    const auto p = make(n, true);
    return !p.prepared && p.code != "resource_limit";
  };
  const auto n_e = last_passing(n_valid - 1100, n_valid + 1100, reached_error);
  INFO("n_eq " << n_eq << " rho " << rho << " n_valid " << n_valid << " n_e " << n_e);
  // The refusal is one quantum step: the first refused size sits within one tail length below the valid one. Under `>` the
  // boundary moves a whole quantum later, outside this window.
  const long delta = static_cast<long>(n_e + 1) - static_cast<long>(n_valid);
  INFO("delta " << delta);
  CHECK(delta >= 0);
  CHECK(delta < 100);
  const auto before = make(n_e, true);
  CHECK_FALSE(before.prepared);
  CHECK(before.code != "resource_limit");
  const auto fail = make(n_e + 1, true);
  INFO(brief(fail) << " message=" << fail.message);
  CHECK_FALSE(fail.prepared);
  CHECK_FALSE(fail.semantic);
  CHECK(fail.code == "resource_limit");
  CHECK(fail.limit == "instructions");
  CHECK(fail.maximum == budget_instruction_max);
  CHECK(fail.maximum != 1000000);
  CHECK(fail.message == "Lua instruction limit exceeded");
  expect_terminal_cleaned(fail);
}
// Mutant T3c: the carried (pre-frame) branch of charge_instructions has no comparison. The tight variant's ceiling (500,000)
// is below the inherited per-chunk limit (1,000,000), so only the composed sink can refuse the workspace chunk.
TEST_CASE("X06 D C02 the composed sink refuses a pre-frame declaration chunk at the carried ceiling", "[runtime][x06d][c02]") {
  const auto make = [](std::size_t n) {
    b3b_fixture f;
    prefix_file(f.workspace_dir / "workspace.lua", spin(n) + "error('refused')\n");
    return run_probe<tight_invocation>(f);
  };
  const auto below = make(300000);
  INFO(brief(below));
  CHECK(below.opened);
  CHECK_FALSE(below.prepared);
  CHECK(below.code != "resource_limit");
  const auto above = make(600000);
  INFO(brief(above) << " message=" << above.message);
  CHECK(above.opened);
  CHECK_FALSE(above.prepared);
  CHECK_FALSE(above.semantic);
  CHECK(above.code == "resource_limit");
  CHECK(above.limit == "instructions");
  CHECK(above.maximum == tight_instruction_max);
  CHECK(above.maximum != 1000000);
  CHECK(above.message == "Lua instruction limit exceeded");
  CHECK_FALSE(above.cleanup_ran);
}
// Route-stage exhaustion. The route function is trusted stdlib code, so its cost is driven through data: many declared jobs
// with long (valid) paths make route_workspace loop for millions of instructions inside the begun frame. spec.lua is invalid
// Lua, so reaching the capture stage is observable as a non-resource failure.
// Mutant row-T8: delete the exhausted(s) refusal after the route call and map every route failure to external_protocol.
TEST_CASE("X06 D C01 route stage exhaustion refuses before any declaration is captured", "[runtime][x06d][c01]") {
  const auto make = [](std::size_t jobs) {
    b3b_fixture f;
    put(f.workspace_dir / "spec.lua", "this is not lua\n");
    const std::string segment(1, 'a');
    std::string       list = "jobs = L { ";
    for (std::size_t i = 0; i < jobs; ++i) {
      // One-byte segments cost the most route instructions per byte (clause C3-4g): the boundary then falls well under the
      // 1 MiB text quota; the larger control below uses one-and-a-quarter times the boundary.
      std::string path;
      for (int k = 0; k < 480; ++k)
        path += segment + "/";
      path += "f" + std::to_string(i) + ".lua";
      list += std::format("{}{{ id = \"j{}\", file = \"{}\" }}", i ? ", " : "", i, path);
    }
    list += " },";
    replace_in_file(f.workspace_dir / "workspace.lua",
                    "jobs = L { { id = \"combined\", file = \"combined.lua\" }, { id = \"split\", file = \"split.lua\" } },",
                    list);
    REQUIRE(read_file_bytes(f.workspace_dir / "workspace.lua").size() < 1048576);
    return run_probe<budget_invocation>(f);
  };
  const auto reached_capture = [&](std::size_t jobs) {
    const auto p = make(jobs);
    return p.opened && !p.prepared && p.code != "resource_limit";
  };
  REQUIRE(reached_capture(1));
  std::size_t lo = 1, hi = 2;
  while (reached_capture(hi)) {
    lo = hi;
    hi *= 2;
    REQUIRE(hi <= 4096);
  }
  const auto boundary = last_passing(lo, hi, reached_capture);
  const auto pass     = make(boundary);
  const auto fail     = make(boundary + 1);
  INFO("boundary jobs " << boundary << " | pass " << brief(pass) << " | fail " << brief(fail));
  CHECK(fail.opened);
  CHECK_FALSE(fail.prepared);
  CHECK_FALSE(fail.semantic);
  CHECK(fail.code == "resource_limit");
  CHECK(fail.limit == "instructions");
  CHECK(fail.maximum == budget_instruction_max);
  CHECK(fail.message == "Lua instruction limit exceeded");
  CHECK(fail.cleanup_ran);
  CHECK(fail.temporary_after == 0);
  CHECK_FALSE(fail.owner_live_after);
  CHECK(fail.retry_protocol);
  // Far past the boundary the refusal is the same (the route is stopped, nothing later runs).
  const auto far = make(boundary + boundary / 4);
  CHECK(far.code == "resource_limit");
  CHECK(far.maximum == budget_instruction_max);
  CHECK(far.cleanup_ran);
}
// Mutants row-T9a (capture-stage source failure mapped to a protocol error) and row-T9b (phase-6 charge deleted).
TEST_CASE("X06 D C01 native exhaustion after all-job validation refuses in preparation and cleans nested roots",
          "[runtime][x06d][c01]") {
  {
    // (a) Capture stage after the route: five chunks of 985,000 stay below the per-chunk limit but exceed the composed
    // ceiling together.
    b3b_fixture f(b3b_fixture::default_body(), false, true);
    add_alpha_job(f);
    const auto extras = add_extra_jobs(f, c01_extra_jobs_native);
    for (const char* name : {"workspace.lua", "spec.lua", "alpha.lua", "combined.lua", "split.lua"})
      prefix_file(f.workspace_dir / name, spin(985000));
    for (const auto& name : extras)
      prefix_file(f.workspace_dir / name, spin(985000));
    const auto p = run_probe<budget_invocation>(f);
    INFO(brief(p) << " message=" << p.message);
    CHECK(p.opened);
    CHECK_FALSE(p.prepared);
    CHECK_FALSE(p.semantic);
    CHECK(p.code == "resource_limit");
    CHECK(p.limit == "instructions");
    CHECK(p.maximum == budget_instruction_max);
    expect_terminal_cleaned(p);
  }
  {
    // (b) Post-validation phases: one byte past the exact boundary the refusal still happens in preparation.
    const auto sweep = sweep_native_boundary(0);
    const auto edge  = padded_probe(0, sweep.pad + 1);
    expect_native_exhaustion(edge);
    CHECK(edge.opened);
    expect_terminal_cleaned(edge);
    CHECK(edge.peak_temporary > 0);
    CHECK(sweep.mark3_at_pass + sweep.step < budget_native_max);
    // Slope pin: everything charged after validation (marks[3]) costs at least 7 per added text byte; the phase-6 charge
    // (3 per source byte plus 4096 per sealed file) is part of it.
    constexpr std::size_t pad = 20000;
    const auto            p0  = padded_probe(0, 0);
    const auto            p1  = padded_probe(0, pad);
    REQUIRE(p0.prepared);
    REQUIRE(p1.prepared);
    const auto post0 = p0.native_bytes - p0.nmarks[3];
    const auto post1 = p1.native_bytes - p1.nmarks[3];
    REQUIRE(post1 > post0);
    INFO("post-validation charge " << post0 << " -> " << post1);
    CHECK((post1 - post0) % pad == 0);
    // Measured slope is 7 per padded byte: 3 from the phase-6 source charge plus 4 from the later derivation charges. Deleting
    // or weakening the phase-6 charge lowers it below 7 even though the other charges alone still exceed 3.
    CHECK((post1 - post0) / pad >= 7);
  }
}
// Mutants M_R4a (delete the extension_history.refused return) and M_R4b (swap it with the exhaustion branch), in the
// extension frame adopted from the factory.
TEST_CASE("X06 D C02 a protocol or semantic failure caught by pcall in a profile-owned frame stays terminal",
          "[runtime][x06d][c02]") {
  for (auto tail : {history_semantic, history_success, std::string_view("error('later')"), std::string_view("while true do end")})
    for (auto misuse : {"ctx.read_history()", "ctx.read_history({}); ctx.read_history({})"}) {
      INFO(tail << " | " << misuse);
      native_history_fixture f;
      history_handler(f, "pcall(function() " + std::string(misuse) + " end); " + std::string(tail));
      auto owner = f.profile_owner();
      f.admit(owner);
      history_protocol(owner.execute());
      history_protocol(owner.execute());
    }
  for (auto tail :
       {history_semantic, history_success, std::string_view("error('later')"), std::string_view("while true do end")}) {
    INFO(tail);
    native_history_fixture f;
    history_handler(f, "pcall(function() ctx.read_history({}) end); " + std::string(tail));
    auto owner = f.profile_owner();
    f.admit(owner, {{"revision", true, "9223372036854775807"}});
    auto result = owner.execute();
    REQUIRE_FALSE(result);
    if (std::holds_alternative<history::history_read_error>(result.error())) {
      CHECK(std::get<history::history_read_error>(result.error()).code == history::history_read_code::revision_not_found);
      auto retry = owner.execute();
      REQUIRE_FALSE(retry);
      REQUIRE(std::holds_alternative<history::history_read_error>(retry.error()));
      CHECK(std::get<history::history_read_error>(retry.error()).code == history::history_read_code::revision_not_found);
    } else {
      FAIL("expected the typed history_read_error to survive the tail");
    }
  }
}
// The illegal registration is caught by pcall in the custom shape, which runs inside the validation phase of the begun
// frame through call_retained_value (runtime.cpp:4351 and 4689); the exhausting tail turns the first failure into a race
// between bridge_error and exhaustion. Mutant it kills: M_O2 (swap the bridge_error and exhausted checks in
// call_retained_value, runtime.cpp:2071-2074). It does not kill M_O3 (call_retained_assembly), M_O5 (prepare_load route),
// X2 (profile_invocation::fail precedence) or X3 (post-route bridge_error check); those are recorded equivalent in the round
// record (plan/INVARIANTS.md E2, E3, E3b, E5). X4 (command_failure guard, runtime.cpp:1201) is not equivalent and is killed
// by the C03 direct shape test.
TEST_CASE("X06 D C02 a protocol failure caught by pcall in a known-load prepare frame stays terminal", "[runtime][x06d][c02]") {
  std::optional<probe_result> first;
  for (auto tail : {std::string(b3b_fixture::default_body()), std::string("error('later')"), std::string("while true do end")}) {
    INFO(tail);
    b3b_fixture f;
    put(f.module, "local saved=registration.shape; local D={id='custom',abi='shape_v1',revision='final',stages='optional',"
                  "help='shape'}; registration.shape(D,function(input) assert(not pcall(saved,D,function()end)); " +
                      tail + " end)");
    const auto p = run_probe<budget_invocation>(f);
    INFO(brief(p) << " message=" << p.message);
    CHECK(p.opened);
    CHECK_FALSE(p.prepared);
    CHECK_FALSE(p.semantic);
    CHECK(p.code == "internal");
    CHECK(p.message == "Invalid external profile protocol");
    CHECK(p.limit.empty());
    expect_terminal_cleaned(p);
    if (!first)
      first = p;
    CHECK(p.code == first->code);
    CHECK(p.message == first->message);
    CHECK(p.field == first->field);
  }
}

namespace {
struct ra01_option {
  std::string name;
  std::string type;
  bool        required;
};
struct ra01_row {
  std::string              name;
  std::string              capability;
  std::vector<ra01_option> options;
};
std::string ra01_text(const value_ptr& v, std::string_view field) {
  REQUIRE(v);
  REQUIRE(v->fields.contains(std::string(field)));
  return v->fields.at(std::string(field))->string;
}
std::string ra01_read_file(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  REQUIRE(in);
  return std::string(std::istreambuf_iterator<char>{in}, {});
}
std::size_t ra01_count(const std::string& hay, const std::string& needle) {
  std::size_t n = 0;
  for (auto at = hay.find(needle); at != std::string::npos; at = hay.find(needle, at + 1))
    ++n;
  return n;
}
} // namespace
TEST_CASE("RA01-B1 default embedded catalog preserves foundational schemas among seventeen rows", "[runtime][ra01-b1]") {
  const std::vector<ra01_row> expected{
      {"export", "export_source_v1", {{"--ledger", "text", true}, {"--revision", "decimal", true}, {"--target", "text", true}}},
      {"history",
       "read_history_v1",
       {{"--after-event", "decimal", false},
        {"--at-event", "decimal", false},
        {"--event", "decimal", false},
        {"--ledger", "text", true},
        {"--limit", "decimal", false},
        {"--request-id", "text", false},
        {"--revision", "decimal", false}}},
      {"init", "initialize_ledger_v1", {{"--ledger", "text", true}, {"--workspace-id", "text", true}}},
      {"load",
       "accept_load_v1",
       {{"--actor", "text", true},
        {"--expect-revision", "text", true},
        {"--ledger", "text", true},
        {"--registry", "text", true},
        {"--request-id", "text", true},
        {"--workspace", "text", true}}},
      {"render",
       "render_capture_v1",
       {{"--base-root", "text", false},
        {"--budget-bytes", "decimal", false},
        {"--capture-outputs", "flag", false},
        {"--job", "text", true},
        {"--observations", "text", false},
        {"--registry", "text", true},
        {"--role", "text", true},
        {"--root", "text", true},
        {"--workspace", "text", true}}}};
  auto vm = command_vm::create();
  REQUIRE(vm);
  auto catalog = vm->catalog();
  REQUIRE(catalog);
  const auto& rows = catalog->fields.at("commands")->items;
  REQUIRE(rows.size() == 17);
  std::vector<std::string> names;
  for (const auto& row : rows)
    names.push_back(ra01_text(row, "name"));
  CHECK(names == std::vector<std::string>{"activate", "check", "check-recover", "checkpoint", "claim", "export", "history",
                                          "init", "load", "preview", "recover", "refresh", "release", "render", "status",
                                          "takeover", "verdict"});
  for (const auto& want : expected) {
    const auto found = std::ranges::find_if(rows, [&](const auto& row) { return ra01_text(row, "name") == want.name; });
    REQUIRE(found != rows.end());
    const auto& row = *found;
    INFO(want.name);
    CHECK(ra01_text(row, "name") == want.name);
    CHECK(ra01_text(row, "family") == want.name);
    CHECK(ra01_text(row, "handler_id") == "shipped." + want.name + ".v1");
    CHECK(ra01_text(row, "response_kind") == want.name + "_v1");
    const auto& caps = row->fields.at("capabilities")->items;
    REQUIRE(caps.size() == 1);
    CHECK(caps[0]->string == want.capability);
    const auto&              opts = row->fields.at("options")->items;
    std::vector<ra01_option> actual;
    for (const auto& o : opts)
      actual.push_back({ra01_text(o, "name"), ra01_text(o, "type"), o->fields.at("required")->boolean});
    std::ranges::sort(actual, {}, &ra01_option::name);
    REQUIRE(actual.size() == want.options.size());
    for (std::size_t k = 0; k < actual.size(); ++k) {
      CHECK(actual[k].name == want.options[k].name);
      CHECK(actual[k].type == want.options[k].type);
      CHECK(actual[k].required == want.options[k].required);
    }
  }
}
TEST_CASE("RA01-B1 command options variant stays closed at twelve members", "[runtime][ra01-b1]") {
  CHECK(
      std::same_as<planar::engine::command_options,
                   std::variant<command_render_options, command_init_options, command_history_options, command_export_options,
                                command_load_options, command_activate_options, command_status_options, command_preview_options,
                                command_run_options, command_recovery_options, command_recover_options, command_check_options>>);
}
TEST_CASE("RA01-B1 commands.lua keeps one handler function and one registration line per authoring verb", "[runtime][ra01-b1]") {
  const auto source = ra01_read_file(std::filesystem::path(__FILE__).parent_path() / "../../stdlib/planar/commands.lua");
  const std::array<std::pair<std::string, std::string>, 6> verbs{{{"spec-prompt", "spec_prompt"},
                                                                  {"critique", "critique"},
                                                                  {"ingest", "ingest"},
                                                                  {"explain-rejection", "explain_rejection"},
                                                                  {"observe", "observe"},
                                                                  {"orchestrator", "orchestrator"}}};
  for (const auto& [verb, fn] : verbs) {
    INFO(verb);
    CHECK(ra01_count(source, "local function " + fn + "_command(") == 1);
  }
  const std::string gate_open = "\n  if registration.authoring then\n";
  const auto        open_at   = source.find(gate_open);
  REQUIRE(open_at != std::string::npos);
  CHECK(ra01_count(source, gate_open) == 1);
  const auto body_at  = open_at + gate_open.size();
  const auto close_at = source.find("\n  end\n", body_at - 1);
  REQUIRE(close_at != std::string::npos);
  std::string expected;
  for (const auto& [verb, fn] : verbs)
    expected += "    registration.add(authoring_descriptor(\"" + verb + "\")," + fn + "_command)\n";
  CHECK(source.substr(body_at, close_at + 1 - body_at) == expected);
  // None of the authoring registrations may appear outside the gate.
  CHECK(ra01_count(source, "registration.add(authoring_descriptor(") == 6);
  for (const auto& [verb, fn] : verbs)
    CHECK(ra01_count(source, "registration.add(authoring_descriptor(\"" + verb + "\"),") == 1);
}
