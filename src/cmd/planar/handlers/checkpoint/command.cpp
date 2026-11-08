module planar.cmd.execution.checkpoint;
import std;
import cli11;
import planar.cmd.invocation;
import planar.engine.ledger.execution;
namespace planar::cmd::handlers {
CLI::App& attach_checkpoint(CLI::App& root,checkpoint_options& o) {
  o.operation="checkpoint";
  auto& command=*root.add_subcommand("checkpoint"); command.set_help_flag(); command.allow_extras(false); command.fallthrough(false);
  const CLI::Validator value([](std::string& t) {return t.empty() || t.starts_with("--") ? "Expected explicit value" : "";},"VALUE");
  auto* ledger=command.add_option("--ledger",o.ledger)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(value)->required();
  auto* run=command.add_option("--run",o.run)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(value)->required();
  auto* request_id=command.add_option("--request-id",o.request_id)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(value)->required();
  auto* actor=command.add_option("--actor",o.actor)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(value)->required();
  auto* observations=command.add_option("--observations",o.observations)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(value)->required();
  (void)ledger;
  (void)request_id; (void)actor; (void)observations;
  run->check(CLI::Validator([](std::string& t){return engine::ledger::parse_run_token(t) ? "" : "Expected run token";},"TOKEN"));
  return command;
}
response checkpoint_command(const checkpoint_options& o,engine::command_vm* vm) {return execute(o,vm);}
response checkpoint_with_profile(const checkpoint_options& o,std::string_view selector) {return execute_with_profile(o,selector);}
}
