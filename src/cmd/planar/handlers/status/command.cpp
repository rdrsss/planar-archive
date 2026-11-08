module planar.cmd.execution.status;
import std;
import cli11;
import planar.cmd.invocation;
namespace planar::cmd::handlers {
CLI::App& attach_status(CLI::App& root, status_options& options) {
  auto& command=*root.add_subcommand("status");
  command.set_help_flag(); command.allow_extras(false); command.fallthrough(false);
  const CLI::Validator explicit_value([](std::string& token) { return token.empty() || token.starts_with("--") ? "Expected explicit value" : ""; }, "VALUE");
  auto* ledger=command.add_option("--ledger", options.ledger)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(explicit_value)->required();
  auto* role=command.add_option("--role", options.role)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(explicit_value);
  auto* request_id=command.add_option("--request-id", options.request_id)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(explicit_value);
  auto* workspace=command.add_option("--workspace", options.workspace)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(explicit_value);
  auto* registry=command.add_option("--registry", options.registry)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(explicit_value);
  role->excludes(request_id); workspace->needs(registry); registry->needs(workspace);
  (void)ledger;
  return command;
}
response status_command(const status_options& options, engine::command_vm* vm) {
  return execute(engine::command_status_options{options.ledger, options.role, options.request_id, options.workspace, options.registry}, vm);
}
response status_with_profile(const status_options& options, std::string_view selector) {
  return execute_with_profile(engine::command_status_options{options.ledger, options.role, options.request_id, options.workspace, options.registry}, selector);
}
}
