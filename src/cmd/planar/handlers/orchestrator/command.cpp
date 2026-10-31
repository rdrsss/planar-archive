module planar.cmd.authoring.orchestrator;
import std;
import cli11;
namespace planar::cmd::authoring {
CLI::App& attach_orchestrator(CLI::App& root, orchestrator_options& options) {
  auto& command = *root.add_subcommand("orchestrator");
  command.set_help_flag();
  command.allow_extras(false);
  command.fallthrough(false);
  const CLI::Validator explicit_value(
      [](std::string& token) { return token.empty() || token.starts_with("--") ? "Expected explicit value" : ""; }, "VALUE");
  auto add = [&](const char* name, std::string& value) {
    return command.add_option(name, value)
        ->expected(1)
        ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
        ->check(explicit_value)
        ->required();
  };
  add("--workspace", options.workspace);
  add("--registry", options.registry);
  add("--job", options.job);
  return command;
}
response orchestrator_command(const orchestrator_options& options, const readers& io) {
  inputs values;
  values.workspace = options.workspace;
  values.registry  = options.registry;
  return run_stub(verb::orchestrator, std::move(values), io);
}
} // namespace planar::cmd::authoring
