module planar.cmd.authoring.critique;
import std;
import cli11;
namespace planar::cmd::authoring {
CLI::App& attach_critique(CLI::App& root, critique_options& options) {
  auto& command = *root.add_subcommand("critique");
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
  return command;
}
response critique_command(const critique_options& options, const readers& io, const executor& execute) {
  inputs values;
  values.workspace = options.workspace;
  values.registry  = options.registry;
  return run(verb::critique, std::move(values), io, execute);
}
} // namespace planar::cmd::authoring
