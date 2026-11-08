// Live orchestrator grammar keeps the declaration form and ledger form exclusive.
#include <catch2/catch_test_macros.hpp>
import std;
import cli11;
import planar.cmd.authoring.orchestrator;
using namespace planar::cmd::authoring;
namespace {
void parse(CLI::App& app, std::vector<std::string> args) {
  std::ranges::reverse(args);
  app.parse(args);
}
} // namespace
TEST_CASE("R15-D orchestrator accepts ledger alone") {
  CLI::App             app;
  orchestrator_options options;
  attach_orchestrator(app, options);
  parse(app, {"orchestrator", "--ledger", "state.db"});
  REQUIRE(options.ledger == "state.db");
  REQUIRE(options.workspace.empty());
}
TEST_CASE("R15-D orchestrator refuses mixed and partial forms") {
  for (auto args : std::vector<std::vector<std::string>>{{"orchestrator"},
                                                         {"orchestrator", "--workspace", "workspace.lua"},
                                                         {"orchestrator", "--ledger", "state.db", "--job", "job"},
                                                         {"orchestrator", "--workspace", "workspace.lua", "--registry",
                                                          "registry.json", "--job", "job", "--ledger", "state.db"}}) {
    CLI::App             app;
    orchestrator_options options;
    attach_orchestrator(app, options);
    REQUIRE_THROWS_AS(parse(app, args), CLI::ParseError);
  }
}
