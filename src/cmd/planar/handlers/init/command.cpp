module planar.cmd.planar.handlers.init;
import std;
import cli11;
import planar.cmd.invocation;
namespace planar::cmd::handlers {
CLI::App& attach_init(CLI::App& root, init_options& options) {
  auto& command = *root.add_subcommand("init");
  command.set_help_flag();
  command.allow_extras(false);
  command.fallthrough(false);
  const CLI::Validator explicit_value(
      [](std::string& value) { return value.empty() || value.starts_with("--") ? "Expected explicit nonempty value" : ""; },
      "VALUE");
  command.add_option("--ledger", options.ledger)
      ->required()
      ->expected(1)
      ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
      ->check(explicit_value);
  command.add_option("--workspace-id", options.workspace_id)
      ->required()
      ->expected(1)
      ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
      ->check(explicit_value);
  return command;
}
engine::command_options init_intent(const init_options& options) {
  return engine::command_init_options{options.ledger, options.workspace_id};
}
response init_command(const init_options& options, engine::command_vm* vm) {
  return execute(init_intent(options), vm);
}
} // namespace planar::cmd::handlers
