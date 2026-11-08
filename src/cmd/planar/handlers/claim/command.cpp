module planar.cmd.execution.claim;
import std;
import cli11;
import planar.cmd.invocation;
namespace planar::cmd::handlers {
CLI::App& attach_claim(CLI::App& root,claim_options& o) {
  auto& command=*root.add_subcommand("claim"); command.set_help_flag(); command.allow_extras(false); command.fallthrough(false);
  const CLI::Validator value([](std::string& token) {return token.empty() || token.starts_with("--") ? "Expected explicit value" : "";},"VALUE");
  command.add_option("--ledger",o.ledger)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(value)->required();
  command.add_option("--job",o.job)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(value)->required();
  command.add_option("--role",o.role)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(value)->required();
  command.add_option("--request-id",o.request_id)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(value)->required();
  command.add_option("--actor",o.actor)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(value)->required();
  command.add_option("--provenance",o.provenance)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(value);
  return command;
}
response claim_command(const claim_options& o,engine::command_vm* vm) { return execute(o,vm); }
response claim_with_profile(const claim_options& o,std::string_view selector) { return execute_with_profile(o,selector); }
}
