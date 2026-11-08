// R18-B-core P4: src/stdlib/planar/review.lua over hand-built execution_snapshot_v1 tables (execution-contract-r18.md
// R18-1 to R18-6, round note V5-02-R18-B-core-DRAFT.md P4). A raw Lua state with the base, table and string libraries
// only loads a test-only R17-C `checks` shim (DRAFT-STUB R17-C, DB-R9) and then the review.lua bytes from
// PLANAR_SOURCE_ROOT as one chunk, as the embedding concatenates stdlib files, and returns the local `review` table.
// Snapshots are Lua tables in the execution_snapshot_v1 key shape (Part 2 read_execution, R17-7 completions/observed/
// evidence, DR18-8 review); `test_current` is a test-only key the shim and the current_work hook read for the current
// work_exec_v1 digest of each role. Each expected string was checked by hand against the clause its test names. No test writes
// files, so no temporary root is created; every section builds its own Lua state.
#include <catch2/catch_test_macros.hpp>
extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}
import std;

namespace {
// DRAFT-STUB R17-C: R17-7 current_completion, applicable_completion and unit_output over the snapshot keys completions,
// observed and test_current (the current W digest); checks.lua replaces it at integration.
constexpr std::string_view checks_shim = R"lua(-- DRAFT-STUB R17-C: test-only checks shim (R17-7 definitions over completions, observed and the test current digest).
local checks={}
do
  local function current_digest(snapshot,role)
    local t=snapshot.test_current and snapshot.test_current[role]
    if t then return t.digest end
    return nil
  end
  local function lineage(snapshot,role)
    local out={}
    for _,c in ipairs(snapshot.completions or {}) do
      if c.job==snapshot.active_job and c.role==role then out[#out+1]=c end
    end
    table.sort(out,function(a,b) return tonumber(a.seq)>tonumber(b.seq) end)
    return out
  end
  local function observed(snapshot,work)
    for _,o in ipairs(snapshot.observed or {}) do if o.work==work then return o.output end end
    return nil
  end
  function checks.unit_output(completion,work)
    for _,u in ipairs(completion.units or {}) do if u.work==work then return u.output end end
    return nil
  end
  function checks.current_completion(snapshot,role) return lineage(snapshot,role)[1] end
  local function published(snapshot,work)
    for _,r in ipairs(snapshot.roles or {}) do
      if r.job==snapshot.active_job then
        for _,w in ipairs(r.generation.structure.work or {}) do
          if w==work then
            local c=checks.current_completion(snapshot,r.role)
            return c and checks.unit_output(c,work)
          end
        end
      end
    end
    return nil
  end
  function checks.applicable_completion(snapshot,role)
    local digest=current_digest(snapshot,role)
    for _,c in ipairs(lineage(snapshot,role)) do
      local ok=c.work_fp==digest
      for _,u in ipairs(c.units or {}) do if observed(snapshot,u.work)~=u.output then ok=false end end
      for _,v in ipairs(c.vectors or {}) do
        for _,t in ipairs(v.targets or {}) do if published(snapshot,t.work)~=t.output then ok=false end end
      end
      if ok then return c end
    end
    return nil
  end
end)lua";

// Test helpers: digest text, the base snapshot (workers alpha owning a and beta owning b, reviewer rev targeting both,
// running with run 10), verdict documents and rows, and a deterministic encoder with sorted keys.
constexpr std::string_view test_library = R"lua(-- Test helpers: digests, the base execution_snapshot_v1 fixture, verdict documents and a deterministic encoder.
function H(c) return "xxh3_128:"..string.rep(c,32) end
review.hooks.current_work=function(snapshot,r)
  local t=snapshot.test_current and snapshot.test_current[r.role]
  if t then return {work=t.work,digest=t.digest} end
  return nil
end
function base()
  local W={shape={kind="loop",max_rounds=2},repair="0"}
  local function worker(role,gen,work)
    return {job="j",role=role,kind="worker",generation={id=gen,ordinal="1",structure={job="j",role=role,kind="worker",
      work={work},targets={}}},state="done",drifted=false,fact_version="3",run="none"}
  end
  return {
    active_revision="1",active_job="j",activation="1",
    roles={
      worker("alpha","1","a"),worker("beta","2","b"),
      {job="j",role="rev",kind="reviewer",generation={id="3",ordinal="1",structure={job="j",role="rev",kind="reviewer",
        work={},targets={"a","b"}}},state="running",drifted=false,fact_version="2",
        run={id="10",generation="3",dispatch={id="7",work_fp=H("c"),render_fp=H("d"),revision="1",work=W}}},
    },
    completions={
      {id="1",job="j",role="alpha",generation="1",run="4",dispatch="5",work_fp=H("1"),seq="20",
        units={{work="a",output=H("a")}},vectors={}},
      {id="2",job="j",role="beta",generation="2",run="6",dispatch="6",work_fp=H("2"),seq="22",
        units={{work="b",output=H("b")}},vectors={}},
    },
    observed={{work="a",output=H("a"),seq="20"},{work="b",output=H("b"),seq="22"}},
    evidence={},
    review={count="0",verdicts={},blockers={},repairs={}},
    test_current={alpha={digest=H("1")},beta={digest=H("2")},rev={digest=H("c"),work=W}},
  }
end
function role(s,name) for _,r in ipairs(s.roles) do if r.role==name then return r end end end
function doc(over)
  local d={version=1,targets={{work="a",output=H("a"),work_fp=H("1")},{work="b",output=H("b"),work_fp=H("2")}},
    policy=H("c"),decision="approve",findings={},repair={},summary="ok"}
  for k,v in pairs(over or {}) do d[k]=v end
  return d
end
function verdict(id,seq,decision,over,extra)
  local body=review.validate_verdict(doc(over or {decision=decision}),base()).value
  local v={id=id,request_id="req"..id,run="10",seq=seq,applicability="current",decision=decision,body=body}
  for k,x in pairs(extra or {}) do v[k]=x end
  return v
end
function enc(v)
  local t=type(v)
  if t=="nil" then return "nil" end
  if t=="boolean" or t=="number" then return tostring(v) end
  if t=="string" then return v end
  local n,count=#v,0
  for _ in pairs(v) do count=count+1 end
  local parts={}
  if count==n then
    for i=1,n do parts[i]=enc(v[i]) end
    return "["..table.concat(parts,",").."]"
  end
  local ks={}
  for k in pairs(v) do ks[#ks+1]=k end
  table.sort(ks)
  for _,k in ipairs(ks) do parts[#parts+1]=k.."="..enc(v[k]) end
  return "{"..table.concat(parts,",").."}"
end
function lines(t) return table.concat(t,"\n") end)lua";

// One Lua state per section with review loaded as the global `review` and the test library installed.
struct review_vm {
  lua_State* l = luaL_newstate();
  review_vm() {
    REQUIRE(l != nullptr);
    luaL_requiref(l, "_G", luaopen_base, 1);
    lua_pop(l, 1);
    luaL_requiref(l, "table", luaopen_table, 1);
    lua_pop(l, 1);
    luaL_requiref(l, "string", luaopen_string, 1);
    lua_pop(l, 1);
    std::ifstream in(std::filesystem::path(PLANAR_SOURCE_ROOT) / "src" / "stdlib" / "planar" / "review.lua",
                     std::ios::binary);
    const std::string source{std::istreambuf_iterator<char>(in), {}};
    REQUIRE_FALSE(source.empty());
    std::string chunk(checks_shim);
    chunk += "\n";
    chunk += source;
    chunk += "\nreturn review";
    run(chunk, "review.lua", 1);
    REQUIRE(lua_istable(l, -1));
    lua_setglobal(l, "review");
    run(test_library, "test_library", 0);
  }
  review_vm(const review_vm&)            = delete;
  review_vm& operator=(const review_vm&) = delete;
  ~review_vm() {
    if (l)
      lua_close(l);
  }
  // Loads and calls one text chunk; a failure reports the Lua message.
  void run(std::string_view text, const char* name, int results) {
    const int loaded = luaL_loadbufferx(l, text.data(), text.size(), name, "t");
    CAPTURE(message());
    REQUIRE(loaded == LUA_OK);
    const int called = lua_pcall(l, 0, results, 0);
    CAPTURE(message());
    REQUIRE(called == LUA_OK);
  }
  std::string message() const {
    return lua_type(l, -1) == LUA_TSTRING ? std::string(lua_tostring(l, -1)) : std::string{};
  }
  // Evaluates a chunk that returns one string and returns its bytes.
  std::string eval(std::string_view code) {
    run(code, "case", 1);
    REQUIRE(lua_type(l, -1) == LUA_TSTRING);
    std::size_t size = 0;
    const char* data = lua_tolstring(l, -1, &size);
    std::string out(data, size);
    lua_pop(l, 1);
    return out;
  }
};
} // namespace


// R18-1 item 3: complete when every target owner has an applicable completion; an owner whose observed output moved
// drops out.
TEST_CASE("R18-B review.lua target vector complete and incomplete", "[r18][review_lua]") {
  SECTION("complete") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local v=review.target_vector(base(),"j"); return tostring(v.complete).." "..#v.targets.." "..v.targets[1].work.."="..v.targets[1].output.."/"..v.targets[1].work_fp)lua") ==
          "true 2 a=xxh3_128:aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/xxh3_128:11111111111111111111111111111111");
  }
  SECTION("incomplete when the observed output moved") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.observed[2].output=H("e"); local v=review.target_vector(s,"j"); return tostring(v.complete).." "..#v.targets.." "..v.units[2].work.."="..tostring(v.units[2].entry))lua") ==
          "false 1 b=nil");
  }
  SECTION("incomplete when an owner has no completion") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); table.remove(s.completions,2); local v=review.target_vector(s,"j"); return tostring(v.complete).." "..#v.targets)lua") ==
          "false 1");
  }
  SECTION("incomplete without a reviewer") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); table.remove(s.roles,3); local v=review.target_vector(s,"j"); return tostring(v.complete).." "..#v.units)lua") ==
          "false 0");
  }
}

// R18-1 item 4: the hook digest unchanged; without a hook the acknowledged dispatch W of a non-drifted running
// reviewer, prefixed when bare (DB-R5).
TEST_CASE("R18-B review.lua policy passes the current W digest through", "[r18][review_lua]") {
  SECTION("hook digest") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.test_current.rev.digest=H("7"); local p=review.policy(s,"rev"); return p.digest.." "..p.work.shape.kind)lua") ==
          "xxh3_128:77777777777777777777777777777777 loop");
  }
  SECTION("dispatch digest without hook") {
    review_vm vm;
    CHECK(vm.eval(R"lua(review.hooks.current_work=nil; local s=base(); s.roles[3].run.dispatch.work_fp=string.rep("9",32); return review.policy(s,"rev").digest)lua") ==
          "xxh3_128:99999999999999999999999999999999");
  }
  SECTION("drifted reviewer has no policy without hook") {
    review_vm vm;
    CHECK(vm.eval(R"lua(review.hooks.current_work=nil; local s=base(); s.roles[3].drifted=true; return tostring(review.policy(s,"rev")))lua") ==
          "nil");
  }
  SECTION("max_rounds from shape, decimal string and default") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); local a=review.max_rounds(s); s.test_current.rev.work={shape={max_rounds="5"}}; local b=review.max_rounds(s); s.test_current.rev.work={shape="loop"}; return a.." "..b.." "..review.max_rounds(s))lua") ==
          "2 5 2");
  }
}

// R18-3: stale output, old work_fp, wrong policy and an incomplete vector are historical with no repair or blocker.
TEST_CASE("R18-B review.lua classify current versus historical", "[r18][review_lua]") {
  SECTION("current approve") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); local v=review.validate_verdict(doc(),s).value; return enc(review.classify(v,s,2)))lua") ==
          "{applicability=current,blockers_opened=[],case=approve,repairs=[]}");
  }
  SECTION("stale output") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); local d=doc({decision="reject",repair={"alpha"}}); d.targets[1].output=H("e"); local v=review.validate_verdict(d,s).value; return enc(review.classify(v,s,2)))lua") ==
          "{applicability=historical,blockers_opened=[],case=historical,repairs=[]}");
  }
  SECTION("old work_fp") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); local d=doc({decision="reject",repair={"alpha"}}); d.targets[2].work_fp=H("9"); local v=review.validate_verdict(d,s).value; return enc(review.classify(v,s,2)))lua") ==
          "{applicability=historical,blockers_opened=[],case=historical,repairs=[]}");
  }
  SECTION("wrong policy") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); local v=review.validate_verdict(doc({decision="reject",repair={"alpha"},policy=H("f")}),s).value; return enc(review.classify(v,s,2)))lua") ==
          "{applicability=historical,blockers_opened=[],case=historical,repairs=[]}");
  }
  SECTION("incomplete vector") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); local v=review.validate_verdict(doc({decision="reject",repair={"alpha"}}),s).value; s.observed[1].output=H("e"); return enc(review.classify(v,s,2)))lua") ==
          "{applicability=historical,blockers_opened=[],case=historical,repairs=[]}");
  }
}

// R18-5 (A14): counts 0 and 1 admit, count 2 caps and opens max_rounds; an empty repair is unowned; an open blocker of
// the kind suppresses a second, unless the same commit clears it (DB-R19).
TEST_CASE("R18-B review.lua classify admitted capped and unowned with max_rounds 2", "[r18][review_lua]") {
  SECTION("count 0") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); local v=review.validate_verdict(doc({decision="reject",repair={"alpha","beta"}}),s).value; return enc(review.classify(v,s,2)))lua") ==
          "{applicability=current,blockers_opened=[],case=admitted,repairs=[alpha,beta]}");
  }
  SECTION("count 1") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.count="1"; local v=review.validate_verdict(doc({decision="reject",repair={"alpha"}}),s).value; return enc(review.classify(v,s,2)))lua") ==
          "{applicability=current,blockers_opened=[],case=admitted,repairs=[alpha]}");
  }
  SECTION("count 2") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.count="2"; local v=review.validate_verdict(doc({decision="reject",repair={"alpha"}}),s).value; return enc(review.classify(v,s,2)))lua") ==
          "{applicability=current,blockers_opened=[max_rounds],case=capped,repairs=[]}");
  }
  SECTION("count 2 with an open max_rounds blocker") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.count="2"; s.review.blockers={{id="4",seq="31",job="j",kind="max_rounds",verdict="3"}}; local v=review.validate_verdict(doc({decision="reject",repair={"alpha"}}),s).value; return enc(review.classify(v,s,2)))lua") ==
          "{applicability=current,blockers_opened=[],case=capped,repairs=[]}");
  }
  SECTION("unowned") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); local v=review.validate_verdict(doc({decision="reject"}),s).value; return enc(review.classify(v,s,2)))lua") ==
          "{applicability=current,blockers_opened=[unowned_rejection],case=unowned,repairs=[]}");
  }
  SECTION("unowned with an open unowned blocker") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.blockers={{id="4",seq="31",job="j",kind="unowned_rejection",verdict="3"}}; local v=review.validate_verdict(doc({decision="reject"}),s).value; return enc(review.classify(v,s,2)))lua") ==
          "{applicability=current,blockers_opened=[],case=unowned,repairs=[]}");
  }
  SECTION("unowned when the same commit clears the open blocker") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.blockers={{id="4",seq="31",job="j",kind="unowned_rejection",verdict="3"}}; local v=review.validate_verdict(doc({decision="reject"}),s).value; return enc(review.classify(v,s,2,{"4"})))lua") ==
          "{applicability=current,blockers_opened=[unowned_rejection],case=unowned,repairs=[]}");
  }
  SECTION("max_rounds from the reviewer W when omitted") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.count="2"; s.test_current.rev.work={shape={max_rounds=3}}; local v=review.validate_verdict(doc({decision="reject",repair={"alpha"}}),s).value; return review.classify(v,s).case)lua") ==
          "admitted");
  }
}

// R18-5 clearing: a load raising max_rounds above the count, an unowned blocker whose verdict is no longer current
// against the next view, and every activate or job-clearing load.
TEST_CASE("R18-B review.lua blockers to clear", "[r18][review_lua]") {
  SECTION("load raises max_rounds") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.count="2"; s.review.blockers={{id="5",seq="31",job="j",kind="max_rounds",verdict="3"}}; return enc(review.blockers_to_clear(s,{operation="load",job="j",max_rounds=3})))lua") ==
          "[5]");
  }
  SECTION("load keeps max_rounds") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.count="2"; s.review.blockers={{id="5",seq="31",job="j",kind="max_rounds",verdict="3"}}; return enc(review.blockers_to_clear(s,{operation="load",job="j",max_rounds=2})))lua") ==
          "[]");
  }
  SECTION("a check commit never clears max_rounds") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.count="2"; s.review.blockers={{id="5",seq="31",job="j",kind="max_rounds",verdict="3"}}; return enc(review.blockers_to_clear(s,{operation="check",job="j",max_rounds=9})))lua") ==
          "[]");
  }
  SECTION("unowned stays while its verdict is current") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.verdicts={verdict("3","30","reject")}; s.review.blockers={{id="6",seq="30",job="j",kind="unowned_rejection",verdict="3"}}; return enc(review.blockers_to_clear(s,{operation="load",job="j",max_rounds=2})))lua") ==
          "[]");
  }
  SECTION("unowned clears when the vector changes") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.verdicts={verdict("3","30","reject")}; s.review.blockers={{id="6",seq="30",job="j",kind="unowned_rejection",verdict="3"}}; local n=base(); n.observed[2].output=H("e"); n.completions[2].units[1].output=H("e"); return enc(review.blockers_to_clear(s,{operation="check",job="j",snapshot=n})))lua") ==
          "[6]");
  }
  SECTION("unowned clears when the policy changes") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.verdicts={verdict("3","30","reject")}; s.review.blockers={{id="6",seq="30",job="j",kind="unowned_rejection",verdict="3"}}; local n=base(); n.test_current.rev.digest=H("8"); return enc(review.blockers_to_clear(s,{operation="load",job="j",max_rounds=2,snapshot=n})))lua") ==
          "[6]");
  }
  SECTION("activate clears every open blocker") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.verdicts={verdict("3","30","reject")}; s.review.blockers={{id="6",seq="30",job="j",kind="unowned_rejection",verdict="3"},{id="5",seq="29",job="j",kind="max_rounds",verdict="2"}}; return enc(review.blockers_to_clear(s,{operation="activate",job="j"})))lua") ==
          "[5,6]");
  }
  SECTION("a job-clearing load clears every open blocker") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.blockers={{id="5",seq="29",job="j",kind="max_rounds",verdict="2"}}; return enc(review.blockers_to_clear(s,{operation="load",job="",max_rounds=0})))lua") ==
          "[5]");
  }
}

// R18-6: approval accepts; a producer output change withdraws it; an open blocker, a recreated role (A22/A33) and a
// later current rejection refuse it; a restored vector re-applies (DR18-5); why-not codes in the closed order.
TEST_CASE("R18-B review.lua acceptance", "[r18][review_lua]") {
  SECTION("approve accepts every unit") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.verdicts={verdict("1","30","approve")}; return enc(review.acceptance(s)))lua") ==
          "[{accepted=true,role=alpha,why=[],work=a},{accepted=true,role=beta,why=[],work=b}]");
  }
  SECTION("a producer output change withdraws acceptance") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.verdicts={verdict("1","30","approve")}; s.observed[1].output=H("e"); s.completions[1].units[1].output=H("e"); return enc(review.acceptance(s)))lua") ==
          "[{accepted=false,role=alpha,why=[no_current_approval],work=a},{accepted=false,role=beta,why=[no_current_approval],work=b}]");
  }
  SECTION("an open blocker") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.verdicts={verdict("1","30","approve")}; s.review.blockers={{id="4",seq="31",job="j",kind="unowned_rejection",verdict="1"}}; return enc(review.acceptance(s)))lua") ==
          "[{accepted=false,role=alpha,why=[blocker_open],work=a},{accepted=false,role=beta,why=[blocker_open],work=b}]");
  }
  SECTION("a recreated role has no approval") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.verdicts={verdict("1","30","approve")}; role(s,"alpha").generation.lineage_opened="35"; return enc(review.acceptance(s)))lua") ==
          "[{accepted=false,role=alpha,why=[no_current_approval],work=a},{accepted=false,role=beta,why=[no_current_approval],work=b}]");
  }
  SECTION("a takeover generation in the lineage keeps the approval") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.verdicts={verdict("1","30","approve")}; local g=role(s,"alpha").generation; g.id="12"; g.lineage={"1","12"}; g.lineage_opened="8"; return enc(review.acceptance(s)))lua") ==
          "[{accepted=true,role=alpha,why=[],work=a},{accepted=true,role=beta,why=[],work=b}]");
  }
  SECTION("a verdict from outside the reviewer lineage does not count") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.verdicts={verdict("1","30","approve",nil,{generation="2",run="9"})}; return enc(review.acceptance(s)))lua") ==
          "[{accepted=false,role=alpha,why=[no_current_approval],work=a},{accepted=false,role=beta,why=[no_current_approval],work=b}]");
  }
  SECTION("a historical approval does not count") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.verdicts={verdict("1","30","approve",nil,{applicability="historical"})}; return enc(review.acceptance(s)))lua") ==
          "[{accepted=false,role=alpha,why=[no_current_approval],work=a},{accepted=false,role=beta,why=[no_current_approval],work=b}]");
  }
  SECTION("a restored vector re-applies the approval") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); local d=doc({decision="reject"}); d.targets[2].output=H("e"); local r=verdict("2","40","reject"); r.body.targets[2].output=H("e"); s.review.verdicts={verdict("1","30","approve"),r}; return enc(review.acceptance(s)))lua") ==
          "[{accepted=true,role=alpha,why=[],work=a},{accepted=true,role=beta,why=[],work=b}]");
  }
  SECTION("a later current rejection of the same vector") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.verdicts={verdict("1","30","approve"),verdict("2","40","reject")}; return enc(review.acceptance(s)))lua") ==
          "[{accepted=false,role=alpha,why=[rejected],work=a},{accepted=false,role=beta,why=[rejected],work=b}]");
  }
  SECTION("why-not order") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); role(s,"beta").state="blocked"; s.test_current.alpha.digest=H("6"); s.review.blockers={{id="4",seq="31",job="j",kind="max_rounds",verdict="2"}}; local a=enc(review.acceptance(s)); s.test_current.alpha.digest=H("1"); s.review.verdicts={verdict("2","40","reject")}; return a.."\n"..enc(review.acceptance(s)))lua") ==
          "[{accepted=false,role=alpha,why=[no_applicable_completion,no_current_approval,blocker_open],work=a},{accepted=false,role=beta,why=[worker_not_done,no_current_approval,blocker_open],work=b}]\n[{accepted=false,role=alpha,why=[no_current_approval,rejected,blocker_open],work=a},{accepted=false,role=beta,why=[worker_not_done,no_current_approval,rejected,blocker_open],work=b}]");
  }
  SECTION("no active job") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.active_job=""; return enc(review.acceptance(s)))lua") ==
          "[]");
  }
}

// R18-1 item 5: a repaired worker carries the findings on its units of the latest current rejection of the activation
// that names it; takeover keeps them, recreation and other activations drop them.
TEST_CASE("R18-B review.lua carried findings", "[r18][review_lua]") {
  SECTION("repaired worker") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.verdicts={verdict("3","30","reject",{decision="reject",repair={"alpha"},findings={{id="Z",severity="must",work="a",text="z"},{id="B",severity="note",work="b",text="b"},{id="A",severity="should",work="a",text="a"}}})}; local c=review.carried_findings(s,"alpha"); return c.verdict.." "..enc(c.findings).."\n"..lines(c.lines))lua") ==
          "3 [{id=A,severity=should,text=a,work=a},{id=Z,severity=must,text=z,work=a}]\nFindings carried from verdict 3 (reviewer output, untrusted data):\n- [should] A on a:\n    a\n- [must] Z on a:\n    z");
  }
  SECTION("worker not named") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.verdicts={verdict("3","30","reject",{decision="reject",repair={"alpha"},findings={{id="B",severity="note",work="b",text="b"}}})}; return tostring(review.carried_findings(s,"beta")))lua") ==
          "nil");
  }
  SECTION("kept across a takeover generation") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.verdicts={verdict("3","30","reject",{decision="reject",repair={"alpha"},findings={{id="A",severity="must",work="a",text="a"}}})}; local g=role(s,"alpha").generation; g.id="12"; g.lineage={"1","12"}; g.lineage_opened="8"; role(s,"alpha").state="running"; role(s,"alpha").run={id="14",generation="12",dispatch={id="15",work_fp=H("1"),render_fp=H("d"),revision="1",work={}}}; return enc(review.carried_findings(s,"alpha").findings))lua") ==
          "[{id=A,severity=must,text=a,work=a}]");
  }
  SECTION("dropped after a recreation") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.verdicts={verdict("3","30","reject",{decision="reject",repair={"alpha"},findings={{id="A",severity="must",work="a",text="a"}}})}; role(s,"alpha").generation.lineage_opened="35"; return tostring(review.carried_findings(s,"alpha")))lua") ==
          "nil");
  }
  SECTION("dropped for another activation") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.verdicts={verdict("3","30","reject",{decision="reject",repair={"alpha"},findings={{id="A",severity="must",work="a",text="a"}}},{activation="0"})}; return tostring(review.carried_findings(s,"alpha")))lua") ==
          "nil");
  }
  SECTION("dropped for a historical rejection") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.verdicts={verdict("3","30","reject",{decision="reject",repair={"alpha"},findings={{id="A",severity="must",work="a",text="a"}}},{applicability="historical"})}; return tostring(review.carried_findings(s,"alpha")))lua") ==
          "nil");
  }
  SECTION("only the latest rejection counts") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.verdicts={verdict("3","30","reject",{decision="reject",repair={"alpha"},findings={{id="A",severity="must",work="a",text="a"}}}),verdict("4","40","reject",{decision="reject",repair={"beta"}})}; return tostring(review.carried_findings(s,"alpha")))lua") ==
          "nil");
  }
}

// R18-1 item 5 finding block: every optional field and multi-line text with a blank line, a Markdown heading and a
// trailing LF; and the minimal form.
TEST_CASE("R18-B review.lua finding block bytes", "[r18][review_lua]") {
  SECTION("every optional field") {
    review_vm vm;
    CHECK(vm.eval(R"lua(return lines(review.finding_lines({id="F-1.x:y",severity="must",work="a",path="src/dir/x.cpp",line=2147483647,prescription="fix_it",text="first\n\n# heading\n  indented\n"})))lua") ==
          "- [must] F-1.x:y on a at src/dir/x.cpp:2147483647 (prescription fix_it):\n    first\n    \n    # heading\n      indented\n    ");
  }
  SECTION("path without line") {
    review_vm vm;
    CHECK(vm.eval(R"lua(return lines(review.finding_lines({id="F",severity="should",work="b",path="README.md",text="one"})))lua") ==
          "- [should] F on b at README.md:\n    one");
  }
  SECTION("minimal") {
    review_vm vm;
    CHECK(vm.eval(R"lua(return lines(review.finding_lines({id="N",severity="note",work="a",text="x"})))lua") ==
          "- [note] N on a:\n    x");
  }
}

// R18-2 steps 5 to 7 in Lua: normalization, the field table of DB-R10/DB-R11, unsupported_policy and the reference
// refusal.
TEST_CASE("R18-B review.lua validate_verdict", "[r18][review_lua]") {
  SECTION("normalization adds the reviewer authority") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local r=review.validate_verdict(doc(),base()); return r.value.authority.." "..tostring(r.error))lua") ==
          "reviewer_role_v1 nil");
  }
  SECTION("explicit reviewer authority") {
    review_vm vm;
    CHECK(vm.eval(R"lua(return review.validate_verdict(doc({authority="reviewer_role_v1"}),base()).value.authority)lua") ==
          "reviewer_role_v1");
  }
  SECTION("repair role outside the job") {
    review_vm vm;
    CHECK(vm.eval(R"lua(return enc(review.validate_verdict(doc({decision="reject",repair={"alpha","zeta"}}),base()).error))lua") ==
          "{code=reference,field=verdict.repair.1,message=Unknown selected role}");
  }
  SECTION("repair role is the reviewer") {
    review_vm vm;
    CHECK(vm.eval(R"lua(return enc(review.validate_verdict(doc({decision="reject",repair={"rev"}}),base()).error))lua") ==
          "{code=reference,field=verdict.repair.0,message=Unknown selected role}");
  }
  SECTION("repair worker owns no target") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.roles[3].generation.structure.targets={"a"}; local d=doc({decision="reject",repair={"beta"}}); d.targets={d.targets[1]}; return enc(review.validate_verdict(d,s).error))lua") ==
          "{code=reference,field=verdict.repair.0,message=Unknown selected role}");
  }
  SECTION("external authority") {
    review_vm vm;
    CHECK(vm.eval(R"lua(return enc(review.validate_verdict(doc({authority="external"}),base()).error))lua") ==
          "{code=unsupported_policy,field=,message=Execution policy is not supported}");
  }
  SECTION("approve with a must finding") {
    review_vm vm;
    CHECK(vm.eval(R"lua(return enc(review.validate_verdict(doc({findings={{id="A",severity="must",work="a",text="t"}}}),base()).error))lua") ==
          "{code=schema,field=verdict.decision,message=Invalid enum value}");
  }
  SECTION("approve with a repair") {
    review_vm vm;
    CHECK(vm.eval(R"lua(return enc(review.validate_verdict(doc({repair={"alpha"}}),base()).error))lua") ==
          "{code=schema,field=verdict.decision,message=Invalid enum value}");
  }
  SECTION("unknown keys sorted, first wins") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local d=doc(); d.zz=1; d.extra=2; return enc(review.validate_verdict(d,base()).error))lua") ==
          "{code=schema,field=verdict.extra,message=Unknown field}");
  }
  SECTION("missing keys in order") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local d=doc(); d.summary=nil; d.policy=nil; return enc(review.validate_verdict(d,base()).error))lua") ==
          "{code=schema,field=verdict.policy,message=Required field}");
  }
  SECTION("version") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local a=enc(review.validate_verdict(doc({version="1"}),base()).error); return a.." "..enc(review.validate_verdict(doc({version=2}),base()).error))lua") ==
          "{code=schema,field=verdict.version,message=Expected version integer} {code=schema,field=verdict.version,message=Unsupported schema version}");
  }
  SECTION("targets empty and unsorted") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local d=doc(); d.targets={d.targets[2],d.targets[1]}; return enc(review.validate_verdict(doc({targets={}}),base()).error).." "..enc(review.validate_verdict(d,base()).error))lua") ==
          "{code=schema,field=verdict.targets,message=List is too short} {code=schema,field=verdict.targets.1.work,message=Duplicate value}");
  }
  SECTION("line without path") {
    review_vm vm;
    CHECK(vm.eval(R"lua(return enc(review.validate_verdict(doc({decision="reject",findings={{id="A",severity="note",work="a",line=3,text="t"}}}),base()).error))lua") ==
          "{code=schema,field=verdict.findings.0.path,message=Required field}");
  }
  SECTION("finding on a work that is not a target") {
    review_vm vm;
    CHECK(vm.eval(R"lua(return enc(review.validate_verdict(doc({decision="reject",findings={{id="A",severity="note",work="c",text="t"}}}),base()).error))lua") ==
          "{code=schema,field=verdict.findings.0.work,message=Invalid enum value}");
  }
  SECTION("duplicate finding id") {
    review_vm vm;
    CHECK(vm.eval(R"lua(return enc(review.validate_verdict(doc({decision="reject",findings={{id="A",severity="note",work="a",text="t"},{id="A",severity="note",work="b",text="u"}}}),base()).error))lua") ==
          "{code=schema,field=verdict.findings.1.id,message=Duplicate value}");
  }
  SECTION("bad digest and bad path") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local a=enc(review.validate_verdict(doc({policy="xxh3_128:ABC"}),base()).error); return a.." "..enc(review.validate_verdict(doc({decision="reject",findings={{id="A",severity="note",work="a",path="../x",text="t"}}}),base()).error))lua") ==
          "{code=schema,field=verdict.policy,message=Malformed output digest} {code=schema,field=verdict.findings.0.path,message=Invalid portable path}");
  }
  SECTION("text with NUL and empty text") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local a=enc(review.validate_verdict(doc({decision="reject",findings={{id="A",severity="note",work="a",text="a\0b"}}}),base()).error); return a.." "..enc(review.validate_verdict(doc({decision="reject",findings={{id="A",severity="note",work="a",text=""}}}),base()).error))lua") ==
          "{code=schema,field=verdict.findings.0.text,message=Invalid string} {code=schema,field=verdict.findings.0.text,message=Invalid string}");
  }
}

// R18-6 status_v1.review: with an active job (decimal strings, open blockers sorted by id, acceptance) and without one.
TEST_CASE("R18-B review.lua status_review", "[r18][review_lua]") {
  SECTION("active job") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.count="1"; s.review.blockers={{id="9",seq="41",job="j",kind="unowned_rejection",verdict="8"},{id="4",seq="31",job="j",kind="max_rounds",verdict="3"}}; return enc(review.status_review(s)))lua") ==
          "{acceptance=[{accepted=false,role=alpha,why=[no_current_approval,blocker_open],work=a},{accepted=false,role=beta,why=[no_current_approval,blocker_open],work=b}],blockers=[{id=4,kind=max_rounds,opened=31,verdict=3},{id=9,kind=unowned_rejection,opened=41,verdict=8}],count=1,external_approval=not_configured,max_rounds=2}");
  }
  SECTION("no active job") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.active_job=""; s.roles={}; return enc(review.status_review(s)))lua") ==
          "{acceptance=[],blockers=[],count=0,external_approval=not_configured,max_rounds=none}");
  }
}

// R18-4 execution_change_v1 for verdict: admitted, historical and approve transitions, the expected set, and the
// reconcile hook with ordinal 1 kept on the reviewer.
TEST_CASE("R18-B review.lua verdict_change", "[r18][review_lua]") {
  SECTION("admitted rejection") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.count="1"; local v=review.validate_verdict(doc({decision="reject",repair={"alpha"}}),s).value; local c=review.verdict_change(s,v,"10",{request_id="q",snapshot_seq="44"}).value; return c.facts.case.." "..enc(c.facts.repairs).." "..enc(c.transitions).."\n"..enc(c.vector).."\n"..enc(c.expected))lua") ==
          "admitted [alpha] [{after_drifted=false,after_state=blocked,before_drifted=false,before_state=running,job=j,reason=completed,role=rev},{after_drifted=false,after_state=stale,before_drifted=false,before_state=done,job=j,reason=repair_admitted,role=alpha}]\n[{drifted=false,job=j,role=alpha,state=stale},{drifted=false,job=j,role=beta,state=done},{drifted=false,job=j,role=rev,state=blocked}]\n[{kind=request_unused,text=q},{kind=active_revision,revision=1},{kind=active_job,text=j},{job=j,kind=fact_version,number=3,role=alpha},{job=j,kind=fact_version,number=3,role=beta},{job=j,kind=fact_version,number=2,role=rev},{drifted=false,job=j,kind=role_state,role=rev,run=10,state=running},{job=j,kind=generation_current,number=3,role=rev},{job=j,kind=run_active,number=3,role=rev,run=10}]");
  }
  SECTION("approve") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); local v=review.validate_verdict(doc(),s).value; local c=review.verdict_change(s,v,"10").value; return c.facts.case.." "..enc(c.transitions))lua") ==
          "approve [{after_drifted=false,after_state=done,before_drifted=false,before_state=running,job=j,reason=completed,role=rev}]");
  }
  SECTION("historical with a worker not done") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); role(s,"beta").state="blocked"; local v=review.validate_verdict(doc({policy=H("f")}),s).value; local c=review.verdict_change(s,v,"10").value; return c.facts.case.." "..c.transitions[1].after_state)lua") ==
          "historical blocked");
  }
  SECTION("historical with every worker done") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); local v=review.validate_verdict(doc({policy=H("f")}),s).value; local c=review.verdict_change(s,v,"10").value; return c.facts.case.." "..c.transitions[1].after_state)lua") ==
          "historical stale");
  }
  SECTION("capped opens max_rounds") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.review.count="2"; local v=review.validate_verdict(doc({decision="reject",repair={"alpha"}}),s).value; local c=review.verdict_change(s,v,"10").value; return c.facts.case.." "..enc(c.facts.blockers_opened).." "..c.transitions[1].after_state.." "..#c.transitions)lua") ==
          "capped [max_rounds] done 1");
  }
  SECTION("blocked repaired worker stays blocked") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); local v=review.validate_verdict(doc({decision="reject",repair={"alpha"}}),s).value; role(s,"alpha").state="blocked"; local c=review.verdict_change(s,v,"10").value; return enc(c.transitions[2]))lua") ==
          "{after_drifted=false,after_state=blocked,before_drifted=false,before_state=blocked,job=j,reason=repair_admitted,role=alpha}");
  }
  SECTION("wrong run is a protocol error") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); local v=review.validate_verdict(doc(),s).value; return enc(review.verdict_change(s,v,"11").error))lua") ==
          "{code=internal,field=,message=Invalid embedded command protocol}");
  }
  SECTION("reconcile hook rows sorted after the reviewer") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); local v=review.validate_verdict(doc({decision="reject",repair={"beta"}}),s).value; review.hooks.reconcile=function(snap,c) table.insert(c.transitions,{job="j",role="aardvark",before_state="blocked",before_drifted=false,after_state="ready",after_drifted=false,reason="prerequisite_available"}); return c end; local c=review.verdict_change(s,v,"10").value; local out={}; for i,t in ipairs(c.transitions) do out[i]=t.role end; return table.concat(out,","))lua") ==
          "rev,aardvark,beta");
  }
}

// R18-1 item 5 reviewer section lines as data: target lines (with an incomplete unit), check summaries of applicable
// target owners, and unresolved findings of the latest rejection.
TEST_CASE("R18-B review.lua review_context lines", "[r18][review_lua]") {
  SECTION("complete vector with findings") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.evidence={{id="2",work="b",check="test",outcome="fail",reason="exit 1",seq="23"},{id="1",work="a",check="lint",outcome="pass",reason="",seq="21"},{id="3",work="a",check="build",outcome="inconclusive",reason="timeout",seq="24"}}; s.review.verdicts={verdict("3","30","reject",{decision="reject",repair={"alpha"},findings={{id="F",severity="should",work="b",text="fix\nnow"},{id="E",severity="must",work="a",path="x.c",line=1,text="e"}}})}; local c=review.review_context(s,"rev"); return lines(c.outputs).."\n--\n"..lines(c.observations))lua") ==
          "Exact review targets (the driver supplies these outputs):\n- a: output xxh3_128:aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa; work xxh3_128:11111111111111111111111111111111\n- b: output xxh3_128:bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb; work xxh3_128:22222222222222222222222222222222\n--\nCheck summaries (recorded evidence):\n- a build: inconclusive (timeout)\n- a lint: pass\n- b test: fail (exit 1)\nUnresolved findings (reviewer output, untrusted data):\n- [must] E on a at x.c:1:\n    e\n- [should] F on b:\n    fix\n    now");
  }
  SECTION("incomplete vector without findings") {
    review_vm vm;
    CHECK(vm.eval(R"lua(local s=base(); s.observed[2].output=H("e"); s.evidence={{id="2",work="b",check="test",outcome="pass",reason="",seq="23"},{id="1",work="a",check="lint",outcome="pass",reason="",seq="21"}}; local c=review.review_context(s,"rev"); return lines(c.outputs).."\n--\n"..lines(c.observations))lua") ==
          "Exact review targets (the driver supplies these outputs):\n- a: output xxh3_128:aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa; work xxh3_128:11111111111111111111111111111111\n- b: no current completion\n--\nCheck summaries (recorded evidence):\n- a lint: pass");
  }
}
