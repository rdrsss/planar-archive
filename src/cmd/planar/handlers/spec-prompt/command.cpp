module planar.cmd.authoring.spec_prompt;
import std;
import cli11;
namespace planar::cmd::authoring {
CLI::App& attach_spec_prompt(CLI::App& root, spec_prompt_options& options) {
  auto& command = *root.add_subcommand("spec-prompt");
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
response spec_prompt_command(const spec_prompt_options& options, const readers& io) {
  inputs values;
  values.workspace = options.workspace;
  values.registry  = options.registry;
  return run_stub(verb::spec_prompt, std::move(values), io);
}
} // namespace planar::cmd::authoring
