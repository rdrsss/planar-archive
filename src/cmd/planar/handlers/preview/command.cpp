module planar.cmd.execution.preview;
import std;
import cli11;
import planar.cmd.invocation;
namespace planar::cmd::handlers {
CLI::App& attach_preview(CLI::App& root, preview_options& options) {
  auto& command=*root.add_subcommand("preview");
  command.set_help_flag(); command.allow_extras(false); command.fallthrough(false);
  const CLI::Validator explicit_value([](std::string& token) { return token.empty() || token.starts_with("--") ? "Expected explicit value" : ""; }, "VALUE");
  auto* ledger=command.add_option("--ledger", options.ledger)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(explicit_value)->required();
  auto* role=command.add_option("--role", options.role)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(explicit_value)->required();
  command.add_option("--since",options.since)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(explicit_value);
  (void)ledger; (void)role;
  return command;
}
response preview_command(const preview_options& options, engine::command_vm* vm) {
  return execute(engine::command_preview_options{options.ledger, options.role, options.since}, vm);
}
response preview_with_profile(const preview_options& options, std::string_view selector) {
  return execute_with_profile(engine::command_preview_options{options.ledger, options.role, options.since}, selector);
}
}
