module planar.cmd.bootstrap;
import std;
import cli11;
import planar.cmd.invocation;
import planar.cmd.planar.handlers.render;
import planar.cmd.planar.handlers.init;
import planar.cmd.planar.handlers.history;
import planar.cmd.planar.handlers.source_export;
import planar.cmd.planar.handlers.load;

namespace planar::cmd {
namespace {
bool catalog_matches(const engine::value_ptr& catalog, const CLI::App& root) {
  const auto& commands = catalog->fields.at("commands")->items;
  if (commands.size() != root.get_subcommands({}).size())
    return false;
  for (const auto& descriptor : commands) {
    auto*       command = root.get_subcommand(descriptor->fields.at("name")->string);
    const auto& options = descriptor->fields.at("options")->items;
    if (options.size() != command->get_options().size())
      return false;
    for (const auto& option : options) {
      auto* compiled = command->get_option_no_throw(option->fields.at("name")->string);
      if (!compiled || compiled->get_required() != option->fields.at("required")->boolean)
        return false;
      if (compiled->get_lnames().size() != 1 || !compiled->get_snames().empty() || compiled->get_positional())
        return false;
      const auto             compiled_type = compiled->get_type_name();
      const std::string_view type          = compiled->get_expected_max() == 0   ? "flag"
                                             : compiled_type.starts_with("INT")  ? "decimal"
                                             : compiled_type.starts_with("TEXT") ? "text"
                                                                                 : "invalid";
      if (type != "flag" && (compiled->get_expected_min() != 1 || compiled->get_expected_max() != 1))
        return false;
      if (compiled->get_multi_option_policy() != CLI::MultiOptionPolicy::Throw)
        return false;
      if (option->fields.at("type")->string != type)
        return false;
    }
  }
  return true;
}
} // namespace

response invoke(std::span<const std::string_view> arguments) {
  try {
    // CLI11 accepts --name=value. The sole preflight syntax restriction preserves
    // Planar's separate-token interface, without recognizing or consuming options.
    if (std::ranges::any_of(arguments, [](std::string_view token) { return token.starts_with("--") && token.contains('='); }))
      return detail::usage();
    CLI::App root{"", "planar"};
    root.set_help_flag();
    root.allow_extras(false);
    root.fallthrough(false);
    root.require_subcommand(0, 1);
    bool help = false;
    root.add_flag("--help", help)->multi_option_policy(CLI::MultiOptionPolicy::Throw);
    handlers::render_options  options;
    auto&                     command = handlers::attach_render(root, options);
    handlers::init_options    init_options;
    auto&                     init_command = handlers::attach_init(root, init_options);
    handlers::history_options history_options;
    auto&                     history_command = handlers::attach_history(root, history_options);
    handlers::export_options  export_options;
    auto&                     export_command = handlers::attach_export(root, export_options);
    handlers::load_options    load_options;
    auto&                     load_command = handlers::attach_load(root, load_options);
    std::vector<std::string>  tokens(arguments.rbegin(), arguments.rend());
    try {
      root.parse(tokens);
    } catch (const CLI::ParseError&) {
      return detail::usage();
    }
    if (help && arguments.size() == 1)
      return {0,
              "Usage: planar render --workspace FILE --registry FILE --root ID=DIRECTORY --job ID --role ID\n"
              "Optional: --capture-outputs --base-root ID=DIRECTORY --observations FILE --budget-bytes N\n"
              "planar init --ledger FILE --workspace-id ID\n"
              "planar history --ledger FILE [--after-event ID] [--limit N]\n"
              "  or --revision ID | --request-id ID | --at-event ID\n"
              "planar export --ledger PATH --revision ID --target NEW_DIRECTORY\n"
              "planar load --ledger PATH --workspace FILE_OR_NAME --registry FILE --expect-revision ID_OR_NONE --request-id ID "
              "--actor TEXT\n"
              "planar --help\n",
              ""};
    // CLI11 can discard positional terminators after scalar values. Count only
    // library-parsed bindings against our exact separate-token interface.
    const auto selected_commands =
        command.count() + init_command.count() + history_command.count() + export_command.count() + load_command.count();
    std::size_t parsed_tokens = selected_commands + root.count("--help");
    for (const auto* registered : {&command, &init_command, &history_command, &export_command, &load_command})
      for (const auto* option : registered->get_options())
        parsed_tokens += option->count() * (option->get_expected_max() == 0 ? 1 : 2);
    if (help || selected_commands != 1 || !root.remaining(true).empty() || parsed_tokens != arguments.size())
      return detail::usage();
    if (command.count() == 1) {
      const auto& source = options.values[2];
      const auto& base   = options.values[5];
      if (options.capture_outputs != !base.empty() ||
          (options.capture_outputs && source.substr(0, source.find('=')) != base.substr(0, base.find('='))))
        return detail::usage();
    }
    auto vm = engine::command_vm::create();
    if (!vm)
      return detail::failed(detail::native_error(vm.error()));
    if (!catalog_matches(vm->catalog(), root))
      return detail::failed(detail::native_error({"internal", "Invalid embedded command catalog"}));
    if (load_command.count() == 1)
      return handlers::load_command(load_options, &*vm);
    if (export_command.count() == 1)
      return handlers::export_command(export_options, &*vm);
    if (history_command.count() == 1)
      return handlers::history_command(history_options, &*vm);
    if (init_command.count() == 1)
      return handlers::init_command(init_options, &*vm);
    return handlers::render_command(options, &*vm);
  } catch (...) {
    return {4, "", ""};
  }
}
} // namespace planar::cmd
