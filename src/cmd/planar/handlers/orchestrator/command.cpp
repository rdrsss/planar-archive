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
        ->check(explicit_value);
  };
  auto* workspace = add("--workspace", options.workspace);
  auto* registry  = add("--registry", options.registry);
  auto* job       = add("--job", options.job);
  auto* ledger    = add("--ledger", options.ledger);
  workspace->needs(registry)->needs(job)->excludes(ledger);
  registry->needs(workspace)->needs(job)->excludes(ledger);
  job->needs(workspace)->needs(registry)->excludes(ledger);
  command.require_option(1);
  return command;
}
response orchestrator_command(const orchestrator_options& options, const readers& io, const executor& execute) {
  inputs values;
  values.workspace = options.workspace;
  values.registry  = options.registry;
  values.job       = options.job;
  values.ledger    = options.ledger;
  return run(verb::orchestrator, std::move(values), io, execute);
}
} // namespace planar::cmd::authoring
