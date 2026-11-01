module planar.cmd.authoring.observe;
import std;
import cli11;
namespace planar::cmd::authoring {
CLI::App& attach_observe(CLI::App& root, observe_options& options) {
  auto& command = *root.add_subcommand("observe");
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
  add("--from", options.from);
  return command;
}
response observe_command(const observe_options& options, const readers& io, const executor& execute) {
  inputs values;
  values.from = options.from;
  return run(verb::observe, std::move(values), io, execute);
}
} // namespace planar::cmd::authoring
