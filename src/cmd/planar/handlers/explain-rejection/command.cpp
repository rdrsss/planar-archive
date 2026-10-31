module planar.cmd.authoring.explain_rejection;
import std;
import cli11;
namespace planar::cmd::authoring {
CLI::App& attach_explain_rejection(CLI::App& root, explain_rejection_options& options) {
  auto& command = *root.add_subcommand("explain-rejection");
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
  add("--diagnostic", options.diagnostic);
  return command;
}
response explain_rejection_command(const explain_rejection_options& options, const readers& io) {
  inputs values;
  values.diagnostic = options.diagnostic;
  return run_stub(verb::explain_rejection, std::move(values), io);
}
} // namespace planar::cmd::authoring
