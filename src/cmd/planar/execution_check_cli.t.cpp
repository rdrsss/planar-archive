// Draft R17/R18 CLI11 grammar: separate options, canonical run tokens and explicit recovery confirmation.
#include <catch2/catch_test_macros.hpp>
import std;
import cli11;
import planar.cmd.execution.check;
import planar.cmd.execution.check_recover;
import planar.cmd.execution.verdict;
using namespace planar::cmd::handlers;
namespace {
void parse(CLI::App& app,std::vector<std::string> args){std::ranges::reverse(args);app.parse(args);}
constexpr std::string_view run="1.2.0123456789abcdef0123456789abcdef";
}
TEST_CASE("R17-D check CLI requires all execution paths and retains provenance"){
 CLI::App app;check_options o;auto& command=attach_check(app,o);
 parse(app,{"check","--ledger","ledger","--run",std::string(run),"--root","root","--base-root","base","--scratch","scratch","--request-id","request","--actor","driver","--provenance","provenance.json"});
 REQUIRE(command.count()==1);REQUIRE(o.run==run);REQUIRE(o.provenance=="provenance.json");REQUIRE(o.base_root=="base");
}
TEST_CASE("R17-D check recovery refuses implicit confirmation and noncanonical ids"){
 for(const auto& confirmation:{"false","1"}){CLI::App app;check_recover_options o;attach_check_recover(app,o);REQUIRE_THROWS_AS(parse(app,{"check-recover","--ledger","ledger","--run-id","1","--reservation","2","--confirm-stopped",confirmation,"--request-id","request","--actor","driver"}),CLI::ParseError);}
 CLI::App app;check_recover_options o;attach_check_recover(app,o);REQUIRE_THROWS_AS(parse(app,{"check-recover","--ledger","ledger","--run-id","01","--reservation","2","--confirm-stopped","true","--request-id","request","--actor","driver"}),CLI::ParseError);
}
TEST_CASE("R18 verdict CLI refuses malformed secret without exposing execution"){
 CLI::App app;verdict_options o;attach_verdict(app,o);
 REQUIRE_THROWS_AS(parse(app,{"verdict","--ledger","ledger","--run","1.2.ABC","--verdict","verdict.json","--request-id","request","--actor","driver"}),CLI::ParseError);
}
