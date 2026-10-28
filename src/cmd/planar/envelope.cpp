module planar.cmd.envelope;
import std;
import cli11;
namespace planar::cmd::envelope {
namespace {
const CLI::Validator& selector_value() {
  static const CLI::Validator validator(
      [](std::string& value) {
        return value.empty() || value.starts_with("--") || !engine::valid_text(value) ? "Expected explicit profile selector" : "";
      },
      "FILE");
  return validator;
}
void append_escaped(std::string& out, std::string_view text) {
  for (const char byte : text) {
    const auto code = static_cast<unsigned char>(byte);
    if (byte == '\\')
      out += "\\\\";
    else if (byte == '\n')
      out += "\\n";
    else if (byte == '\t')
      out += "\\t";
    else if (code < 0x20 || code == 0x7f)
      out += std::format("\\u{:04x}", static_cast<unsigned>(code));
    else
      out += byte;
  }
}
} // namespace
CLI::App& attach_prefix(CLI::App& root, prefix_binding& binding) {
  auto& command = *root.add_subcommand("with-profile");
  command.set_help_flag();
  command.prefix_command();
  binding.command         = &command;
  binding.selector_option = command.add_option("--stdlib-profile", binding.selector)
                                ->required()
                                ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
                                ->check(selector_value());
  binding.help_option     = command.add_flag("--help", binding.help)->multi_option_policy(CLI::MultiOptionPolicy::Throw);
  return command;
}
prefix_result classify_prefix(const prefix_binding& binding, std::size_t argument_count) {
  prefix_result result;
  if (!binding.command || !binding.selector_option || !binding.help_option)
    return result;
  // CLI11 hands back its reverse-order parse stack; the tail for reporting is its exact reverse.
  result.passthrough = binding.command->remaining_for_passthrough();
  result.tail.assign(result.passthrough.rbegin(), result.passthrough.rend());
  const std::size_t consumed = binding.command->count() + 2 * binding.selector_option->count() + binding.help_option->count();
  if (binding.command->count() != 1 || binding.selector_option->count() != 1 || binding.selector.empty() ||
      consumed + result.tail.size() != argument_count)
    return result;
  result.selector = binding.selector;
  if (binding.help_option->count() != 0)
    result.outcome = result.tail.empty() ? prefix_result::kind::help : prefix_result::kind::usage;
  else
    result.outcome = result.tail.empty() ? prefix_result::kind::usage : prefix_result::kind::profile;
  return result;
}
prefix_result parse_prefix(std::span<const std::string_view> arguments) {
  try {
    CLI::App root{"", "planar"};
    root.set_help_flag();
    root.allow_extras(false);
    root.fallthrough(false);
    root.require_subcommand(0, 1);
    prefix_binding binding;
    attach_prefix(root, binding);
    std::vector<std::string> stack(arguments.rbegin(), arguments.rend());
    try {
      root.parse(stack);
    } catch (const CLI::ParseError&) {
      return {};
    }
    return classify_prefix(binding, arguments.size());
  } catch (...) {
    return {};
  }
}
std::expected<std::string, diagnostic> format_selected_help(std::span<const help_command> commands, std::size_t ceiling) {
  const auto refused = [] { return std::unexpected(diagnostic{"resource_limit", "External profile resource limit exceeded"}); };
  try {
    std::vector<const help_command*> ordered;
    ordered.reserve(commands.size());
    for (const auto& command : commands)
      ordered.push_back(&command);
    // std::string comparison is by unsigned byte value, so UTF-8 high bytes sort after ASCII.
    std::ranges::sort(ordered, {}, [](const help_command* command) -> const std::string& { return command->name; });
    std::string out = "Usage: planar with-profile --stdlib-profile FILE COMMAND [OPTIONS]\n\nCommands:\n";
    if (out.size() > ceiling)
      return refused();
    for (const auto* command : ordered) {
      out += "  ";
      out += command->name;
      out += '\t';
      append_escaped(out, command->help);
      out += '\n';
      if (out.size() > ceiling)
        return refused();
      std::vector<const help_option*> options;
      options.reserve(command->options.size());
      for (const auto& option : command->options)
        options.push_back(&option);
      std::ranges::sort(options, {}, [](const help_option* option) -> const std::string& { return option->name; });
      for (const auto* option : options) {
        out += "    ";
        out += option->name;
        out += ' ';
        out += option->type;
        out += option->required ? " required" : " optional";
        out += '\t';
        append_escaped(out, option->help);
        out += '\n';
        if (out.size() > ceiling)
          return refused();
      }
    }
    return out;
  } catch (const std::bad_alloc&) {
    return refused();
  } catch (const std::length_error&) {
    return refused();
  }
}
} // namespace planar::cmd::envelope
