module planar.cmd.execution.recover;
import std;
import cli11;
import planar.cmd.invocation;
import planar.engine.ledger.execution;
namespace planar::cmd::handlers {
CLI::App& attach_recover(CLI::App& root, recover_options& o) {

  auto& command = *root.add_subcommand("recover");
  command.set_help_flag();
  command.allow_extras(false);
  command.fallthrough(false);
  const CLI::Validator value([](std::string& t) { return t.empty() || t.starts_with("--") ? "Expected explicit value" : ""; },
                             "VALUE");
  auto*                ledger = command.add_option("--ledger", o.ledger)
                                    ->expected(1)
                                    ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
                                    ->check(value)
                                    ->required();
  auto*                role   = command.add_option("--role", o.role)
                                    ->expected(1)
                                    ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
                                    ->check(value)
                                    ->required();
  auto*                root_option =
      command.add_option("--root", o.root)->expected(1)->multi_option_policy(CLI::MultiOptionPolicy::Throw)->check(value);
  auto* base_root = command.add_option("--base-root", o.base_root)
                        ->expected(1)
                        ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
                        ->check(value);
  (void)ledger;
  (void)role;
  root_option->needs(base_root);
  base_root->needs(root_option);
  command.get_option("--role")->check(CLI::Validator(
      [](std::string& t) {
        return !t.empty() && t.size() <= 64 && t.front() >= 'a' && t.front() <= 'z' &&
                       std::ranges::all_of(t, [](char c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'; })
                   ? ""
                   : "Expected identifier";
      },
      "ID"));
  return command;
}
response recover_command(const recover_options& o, engine::command_vm* vm) {
  return execute(o, vm);
}
response recover_with_profile(const recover_options& o, std::string_view selector) {
  return execute_with_profile(o, selector);
}
} // namespace planar::cmd::handlers
