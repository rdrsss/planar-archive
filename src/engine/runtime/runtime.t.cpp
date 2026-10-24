// Native trusted runtime boundary and separately embedded adversarial exports.
#include <catch2/catch_test_macros.hpp>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
import std;
import planar.engine.runtime;
import planar.engine.runtime_test;
import planar.engine.lua;
import planar.engine.hash;
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
