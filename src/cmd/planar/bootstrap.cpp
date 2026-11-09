module planar.cmd.bootstrap;
import std;
import cli11;
import planar.cmd.envelope;
import planar.cmd.invocation;
import planar.cmd.planar.handlers.render;
import planar.cmd.planar.handlers.init;
import planar.cmd.planar.handlers.history;
import planar.cmd.planar.handlers.source_export;
import planar.cmd.planar.handlers.load;
import planar.cmd.planar.handlers.extension;
import planar.cmd.execution.activate;
import planar.cmd.execution.status;
import planar.cmd.execution.preview;
import planar.cmd.execution.claim;
import planar.cmd.execution.refresh;
import planar.cmd.execution.checkpoint;
import planar.cmd.execution.release;
import planar.cmd.execution.recover;
import planar.cmd.execution.check;
import planar.cmd.execution.check_recover;
import planar.cmd.execution.verdict;

import planar.cmd.execution.takeover;
#ifdef PLANAR_AUTHORING_BUNDLE
import planar.cmd.authoring;
import planar.cmd.authoring.spec_prompt;
import planar.cmd.authoring.critique;
import planar.cmd.authoring.ingest;
import planar.cmd.authoring.explain_rejection;
import planar.cmd.authoring.observe;
import planar.cmd.authoring.orchestrator;
#endif

namespace planar::cmd {
namespace {
/// Default root help bytes; the bytes are the shipped contract.
constexpr std::string_view default_root_help =
    "Usage: planar render --workspace FILE --registry FILE --root ID=DIRECTORY --job ID --role ID\n"
    "Optional: --capture-outputs --base-root ID=DIRECTORY --observations FILE --budget-bytes N\n"
    "planar init --ledger FILE --workspace-id ID\n"
    "planar history --ledger FILE [--after-event ID] [--limit N]\n"
    "  or --revision ID | --request-id ID | --at-event ID | --event ID\n"
    "planar export --ledger PATH --revision ID --target NEW_DIRECTORY\n"
    "planar load --ledger PATH --workspace FILE_OR_NAME --registry FILE --expect-revision ID_OR_NONE --request-id ID "
    "--actor TEXT\n"
    "planar activate --ledger PATH --job ID --expect-revision ID_OR_NONE --expect-job JOB_OR_NONE --request-id ID --actor "
    "TEXT\n"
    "planar status --ledger PATH [--role ID | --request-id ID] [--workspace FILE_OR_NAME --registry FILE]\n"
    "planar preview --ledger PATH --role ID [--since RENDER_FP]\n"
    "planar claim --ledger PATH --job ID --role ID --request-id ID --actor TEXT [--provenance FILE]\n"
    "planar takeover --ledger PATH --job ID --role ID --run-id ID --request-id ID --actor TEXT [--provenance FILE]\n"
    "planar refresh --ledger PATH --run TOKEN --expect-fp WORK_FP --request-id ID --actor TEXT [--observations FILE]\n"
    "planar checkpoint --ledger PATH --run TOKEN --observations FILE --request-id ID --actor TEXT\n"
    "planar release --ledger PATH --run TOKEN --reason REASON --request-id ID --actor TEXT [--observations FILE] [--provenance "
    "FILE]\n"
    "planar recover --ledger PATH --role ID [--root ID=DIRECTORY --base-root ID=DIRECTORY]\n"
    "planar check --ledger PATH --run TOKEN --root DIRECTORY --base-root DIRECTORY --scratch DIRECTORY --request-id ID --actor "
    "TEXT [--provenance FILE]\n"
    "planar check-recover --ledger PATH --run-id ID --reservation ID --confirm-stopped true --request-id ID --actor TEXT\n"
    "planar verdict --ledger PATH --run TOKEN --verdict FILE --request-id ID --actor TEXT\n"
    "planar --help\n";
} // namespace
namespace {
bool catalog_matches(const engine::value_ptr& catalog, const CLI::App& root) {
  // The with-profile envelope is the one root subcommand that is not part of the embedded catalog.
  const auto& commands = catalog->fields.at("commands")->items;
  if (commands.size() + 1 != root.get_subcommands({}).size())
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

/// Option storage and bindings of the five shipped families; alive from parsing through dispatch.
struct shipped_families {
  handlers::render_options  render;
  handlers::init_options    init;
  handlers::history_options history;
  handlers::export_options  source_export;
  handlers::load_options    load;
  // Activate binds through the real handler module in every composition (DD27), hence the qualified names.
  ::planar::cmd::handlers::activate_options      activate;
  CLI::App*                                      activate_command = nullptr;
  ::planar::cmd::handlers::status_options        status;
  ::planar::cmd::handlers::preview_options       preview;
  CLI::App*                                      status_command  = nullptr;
  CLI::App*                                      preview_command = nullptr;
  ::planar::cmd::handlers::claim_options         claim;
  ::planar::cmd::handlers::takeover_options      takeover;
  CLI::App*                                      claim_command = nullptr;
  ::planar::cmd::handlers::refresh_options       refresh;
  CLI::App*                                      refresh_command = nullptr;
  ::planar::cmd::handlers::checkpoint_options    checkpoint;
  CLI::App*                                      checkpoint_command = nullptr;
  ::planar::cmd::handlers::release_options       release;
  CLI::App*                                      release_command = nullptr;
  ::planar::cmd::handlers::recover_options       recover;
  CLI::App*                                      recover_command = nullptr;
  ::planar::cmd::handlers::check_options         check;
  CLI::App*                                      check_command = nullptr;
  ::planar::cmd::handlers::check_recover_options check_recover;
  CLI::App*                                      check_recover_command = nullptr;
  ::planar::cmd::handlers::verdict_options       verdict;
  CLI::App*                                      verdict_command = nullptr;

  CLI::App* takeover_command = nullptr;
  CLI::App* render_command   = nullptr;
  CLI::App* init_command     = nullptr;
  CLI::App* history_command  = nullptr;
  CLI::App* export_command   = nullptr;
  CLI::App* load_command     = nullptr;
  void      attach(CLI::App& root) {
    render_command  = &handlers::attach_render(root, render);
    init_command    = &handlers::attach_init(root, init);
    history_command = &handlers::attach_history(root, history);
    export_command  = &handlers::attach_export(root, source_export);
    load_command    = &handlers::attach_load(root, load);
    // P2-7 order: the R15 verbs follow load.
    activate_command      = &::planar::cmd::handlers::attach_activate(root, activate);
    status_command        = &::planar::cmd::handlers::attach_status(root, status);
    preview_command       = &::planar::cmd::handlers::attach_preview(root, preview);
    claim_command         = &::planar::cmd::handlers::attach_claim(root, claim);
    refresh_command       = &::planar::cmd::handlers::attach_refresh(root, refresh);
    checkpoint_command    = &::planar::cmd::handlers::attach_checkpoint(root, checkpoint);
    release_command       = &::planar::cmd::handlers::attach_release(root, release);
    recover_command       = &::planar::cmd::handlers::attach_recover(root, recover);
    check_command         = &::planar::cmd::handlers::attach_check(root, check);
    check_recover_command = &::planar::cmd::handlers::attach_check_recover(root, check_recover);
    verdict_command       = &::planar::cmd::handlers::attach_verdict(root, verdict);

    takeover_command = &::planar::cmd::handlers::attach_takeover(root, takeover);
  }
  std::size_t selected() const {
    return render_command->count() + init_command->count() + history_command->count() + export_command->count() +
           load_command->count() + activate_command->count() + status_command->count() + preview_command->count() +
           claim_command->count() + refresh_command->count() + checkpoint_command->count() + release_command->count() +
           check_command->count() + check_recover_command->count() + verdict_command->count() + recover_command->count() +
           takeover_command->count();
  }
  /// Tokens CLI11 bound to the selected command: its name plus one per flag and two per scalar option.
  std::size_t parsed_tokens() const {
    std::size_t total = selected();
    for (const auto* registered :
         {render_command, init_command, history_command, export_command, load_command, activate_command, status_command,
          preview_command, claim_command, refresh_command, checkpoint_command, release_command, check_command,
          check_recover_command, verdict_command, recover_command, takeover_command})
      for (const auto* option : registered->get_options())
        total += option->count() * (option->get_expected_max() == 0 ? 1 : 2);
    return total;
  }
  /// Render output capture and its base mapping must agree, exactly as for a direct invocation.
  bool render_consistent() const {
    const auto& source = render.values[2];
    const auto& base   = render.values[5];
    return render.capture_outputs == !base.empty() &&
           (!render.capture_outputs || source.substr(0, source.find('=')) == base.substr(0, base.find('=')));
  }
  /// Typed intent of the selected family.
  engine::command_options intent() const {
    if (load_command->count() == 1)
      return handlers::load_intent(load);
    if (export_command->count() == 1)
      return handlers::export_intent(source_export);
    if (history_command->count() == 1)
      return handlers::history_intent(history);
    if (init_command->count() == 1)
      return handlers::init_intent(init);
    return handlers::render_intent(render);
  }
};

#ifdef PLANAR_AUTHORING_BUNDLE
/// Option storage and bindings of the six authoring families; alive from parsing through dispatch.
struct authoring_families {
  authoring::spec_prompt_options       spec_prompt;
  authoring::critique_options          critique;
  authoring::ingest_options            ingest;
  authoring::explain_rejection_options explain_rejection;
  authoring::observe_options           observe;
  authoring::orchestrator_options      orchestrator;
  std::array<CLI::App*, 6>             commands{};
  void                                 attach(CLI::App& root) {
    commands[0] = &authoring::attach_spec_prompt(root, spec_prompt);
    commands[1] = &authoring::attach_critique(root, critique);
    commands[2] = &authoring::attach_ingest(root, ingest);
    commands[3] = &authoring::attach_explain_rejection(root, explain_rejection);
    commands[4] = &authoring::attach_observe(root, observe);
    commands[5] = &authoring::attach_orchestrator(root, orchestrator);
  }
  std::size_t selected() const {
    std::size_t total = 0;
    for (const auto* command : commands)
      total += command->count();
    return total;
  }
  /// Tokens CLI11 bound to the selected command: its name plus one per flag and two per scalar option.
  std::size_t parsed_tokens() const {
    std::size_t total = selected();
    for (const auto* command : commands)
      for (const auto* option : command->get_options())
        total += option->count() * (option->get_expected_max() == 0 ? 1 : 2);
    return total;
  }
  /// Dispatch the selected authoring verb, or answer nothing when none is selected.
  /// @param vm Stack VM retained for the one authoring invocation.
  std::optional<response> dispatch(engine::command_vm* vm) const {
    const authoring::executor execute = [vm](authoring::verb which, const authoring::inputs& values) {
      engine::command_authoring_options options;
      switch (which) {
      case authoring::verb::spec_prompt:
        options.verb = engine::command_authoring_verb::spec_prompt;
        break;
      case authoring::verb::critique:
        options.verb = engine::command_authoring_verb::critique;
        break;
      case authoring::verb::ingest:
        options.verb = engine::command_authoring_verb::ingest;
        break;
      case authoring::verb::explain_rejection:
        options.verb = engine::command_authoring_verb::explain_rejection;
        break;
      case authoring::verb::observe:
        options.verb = engine::command_authoring_verb::observe;
        break;
      case authoring::verb::orchestrator:
        options.verb = engine::command_authoring_verb::orchestrator;
        break;
      }
      options.workspace  = values.workspace;
      options.registry   = values.registry;
      options.diagnostic = values.diagnostic;
      options.from       = values.from;
      options.job        = values.job;
      options.ledger     = values.ledger;
      return execute_authoring(std::move(options), vm);
    };
    const authoring::readers io = authoring::process_readers();
    if (commands[0]->count() == 1)
      return authoring::spec_prompt_command(spec_prompt, io, execute);
    if (commands[1]->count() == 1)
      return authoring::critique_command(critique, io, execute);
    if (commands[2]->count() == 1)
      return authoring::ingest_command(ingest, io, execute);
    if (commands[3]->count() == 1)
      return authoring::explain_rejection_command(explain_rejection, io, execute);
    if (commands[4]->count() == 1)
      return authoring::observe_command(observe, io, execute);
    if (commands[5]->count() == 1)
      return authoring::orchestrator_command(orchestrator, io, execute);
    return std::nullopt;
  }
};
#endif

/// Stored text, decimal and history ID rules of a sealed extension option, matching the shipped validators.
std::string explicit_text(std::string& token) {
  return token.empty() || token.starts_with("--") || token.size() > 65536 || !engine::valid_text(token)
             ? "Expected explicit value"
             : "";
}
std::string canonical_decimal(std::string& token, std::size_t digits, std::int64_t maximum) {
  if (token.empty() || token.size() > digits || token.front() < '1' || token.front() > '9')
    return "Expected positive decimal";
  std::int64_t number = 0;
  auto [end, error]   = std::from_chars(token.data(), token.data() + token.size(), number);
  return error != std::errc{} || end != token.data() + token.size() || number <= 0 || number > maximum
             ? "Expected positive decimal"
             : "";
}

/// Final parser generated from a sealed catalog; option storage stays alive through dispatch.
struct extension_parser {
  struct option_binding {
    std::string  name; ///< Descriptor name including leading dashes.
    CLI::Option* option = nullptr;
    std::string* value  = nullptr; ///< Null for flags.
  };
  struct command_binding {
    std::string                 name;
    CLI::App*                   app = nullptr;
    std::vector<option_binding> options;
  };
  CLI::App                     root{"", "planar"};
  std::deque<std::string>      values;
  std::vector<command_binding> commands;
  /// Build one subcommand per external descriptor; false for any malformed sealed catalog.
  bool build(const engine::value_ptr& catalog) {
    root.set_help_flag();
    root.allow_extras(false);
    root.fallthrough(false);
    root.require_subcommand(0, 1);
    const CLI::Validator text_rule(&explicit_text, "TEXT");
    const CLI::Validator decimal_rule([](std::string& token) { return canonical_decimal(token, 10, 2147483647); }, "DECIMAL");
    const CLI::Validator id_rule(
        [](std::string& token) { return canonical_decimal(token, 19, std::numeric_limits<std::int64_t>::max()); }, "ID");
    for (const auto& item : catalog->fields.at("commands")->items) {
      const auto& descriptor = item->fields.contains("descriptor") ? item->fields.at("descriptor") : item;
      if (descriptor->fields.at("family")->string != "extension_read_v1")
        continue;
      commands.emplace_back();
      auto& binding = commands.back();
      binding.name  = descriptor->fields.at("name")->string;
      binding.app   = root.add_subcommand(binding.name);
      binding.app->set_help_flag();
      binding.app->allow_extras(false);
      binding.app->fallthrough(false);
      for (const auto& declared : descriptor->fields.at("options")->items) {
        option_binding bound;
        bound.name       = declared->fields.at("name")->string;
        const auto& type = declared->fields.at("type")->string;
        if (type == "flag") {
          bound.option = binding.app->add_flag(bound.name)->multi_option_policy(CLI::MultiOptionPolicy::Throw);
        } else if (type == "text" || type == "decimal" || type == "history_id") {
          bound.value  = &values.emplace_back();
          bound.option = binding.app->add_option(bound.name, *bound.value)
                             ->expected(1)
                             ->multi_option_policy(CLI::MultiOptionPolicy::Throw)
                             ->check(type == "text"      ? text_rule
                                     : type == "decimal" ? decimal_rule
                                                         : id_rule);
        } else {
          return false;
        }
        if (declared->fields.at("required")->boolean)
          bound.option->required();
        binding.options.push_back(std::move(bound));
      }
    }
    return true;
  }
};

/// Plain-data view of every sealed catalog command for deterministic help.
std::vector<envelope::help_command> help_commands(const engine::value_ptr& catalog) {
  std::vector<envelope::help_command> out;
  for (const auto& item : catalog->fields.at("commands")->items) {
    const auto&            descriptor = item->fields.contains("descriptor") ? item->fields.at("descriptor") : item;
    envelope::help_command command{descriptor->fields.at("name")->string, descriptor->fields.at("help")->string, {}};
    for (const auto& option : descriptor->fields.at("options")->items)
      command.options.push_back({option->fields.at("name")->string, option->fields.at("type")->string,
                                 option->fields.at("required")->boolean, option->fields.at("help")->string});
    out.push_back(std::move(command));
  }
  return out;
}

/// Parsed accounting of an embedded-catalog probe: command name, flags and scalars, plus opaque positional tokens.
std::size_t probe_consumed(const CLI::App& probe, const CLI::Option& opaque) {
  std::size_t consumed = opaque.count();
  for (const auto* command : probe.get_subcommands()) {
    consumed += command->count();
    for (const auto* option : command->get_options())
      consumed += option->count() * (option->get_expected_max() == 0 ? 1 : 2);
  }
  return consumed;
}

/// Open the selected profile once and answer selected help or dispatch the admitted extension tail.
response open_selected(std::string_view selector, bool help, const std::vector<std::string>& passthrough,
                       const std::vector<std::string>& tail, invocation_snapshot& snapshot) {
  auto owner = engine::profile_invocation::open_opaque(selector);
  if (!owner)
    return detail::registration_failed(owner.error().detail);
  const auto catalog = owner->catalog();
  if (!catalog)
    return detail::extension_protocol_failed();
  if (help) {
    const auto commands  = help_commands(catalog);
    auto       formatted = envelope::format_selected_help(commands);
    if (!formatted)
      return detail::extension_resource_failed(formatted.error());
    return {0, std::move(*formatted), ""};
  }
  // The final parser is generated from the sealed catalog and keeps its option storage alive through dispatch.
  extension_parser parser;
  if (!parser.build(catalog))
    return detail::extension_protocol_failed();
  auto stack = passthrough;
  try {
    parser.root.parse(stack);
  } catch (const CLI::ParseError&) {
    return detail::extension_usage_failed();
  }
  const extension_parser::command_binding* selected = nullptr;
  std::size_t                              consumed = 0;
  for (const auto& command : parser.commands) {
    if (command.app->count() == 0)
      continue;
    selected = &command;
    consumed += command.app->count();
    for (const auto& option : command.options)
      consumed += option.option->count() * (option.value ? 2 : 1);
  }
  if (!selected || consumed != tail.size() || !parser.root.remaining(true).empty())
    return detail::extension_usage_failed();
  std::vector<engine::extension_parsed_option> parsed;
  for (const auto& option : selected->options) {
    if (option.option->count() == 0)
      continue;
    engine::extension_parsed_option record;
    record.name    = std::string_view(option.name).substr(2);
    record.present = true;
    if (option.value)
      record.token = std::string_view(*option.value);
    parsed.push_back(record);
  }
  if (auto completed = complete_process_snapshot(snapshot); !completed)
    return detail::failed(detail::native_error(completed.error()));
  return handlers::extension_command(std::move(*owner), selected->name, parsed, snapshot);
}

/// With-profile invocation after the root parse accepted the envelope.
response invoke_with_profile(const envelope::prefix_binding& binding, std::size_t argument_count) {
  auto prefix = envelope::classify_prefix(binding, argument_count);
  if (prefix.outcome == envelope::prefix_result::kind::usage)
    return detail::usage();
  invocation_snapshot snapshot;
  if (prefix.outcome == envelope::prefix_result::kind::help) {
    auto selector = prepare_process_profile_selector(prefix.selector, snapshot);
    if (!selector)
      return detail::failed(detail::native_error(selector.error()));
    return open_selected(*selector, true, {}, {}, snapshot);
  }
  // Embedded probe: every shipped family plus an optional opaque tail. It receives its own copy of CLI11's reverse-order
  // parse stack, identical to the one the final parser receives later.
  CLI::App probe{"", "planar"};
  probe.set_help_flag();
  probe.allow_extras(false);
  probe.fallthrough(false);
  probe.require_subcommand(0, 1);
  shipped_families         families;
  std::vector<std::string> opaque;
  families.attach(probe);
  probe.prefix_command(true);
  auto* positional = probe.add_option("opaque-tail", opaque)->expected(-1);
  probe.positionals_at_end();
  auto stack = prefix.passthrough;
  try {
    probe.parse(stack);
  } catch (const CLI::ParseError&) {
    return detail::usage();
  }
  const bool shipped = families.selected() != 0;
  if (probe_consumed(probe, *positional) != prefix.tail.size() || !probe.remaining(true).empty() ||
      (!shipped && (opaque.empty() || opaque != prefix.tail)))
    return detail::usage();
  if (shipped) {
    if (families.selected() != 1 || (families.render_command->count() == 1 && !families.render_consistent()))
      return detail::usage();
    if (families.activate_command->count() == 1)
      return ::planar::cmd::handlers::activate_with_profile(families.activate, prefix.selector);
    if (families.status_command->count() == 1)
      return ::planar::cmd::handlers::status_with_profile(families.status, prefix.selector);
    if (families.preview_command->count() == 1)
      return ::planar::cmd::handlers::preview_with_profile(families.preview, prefix.selector);
    if (families.takeover_command->count() == 1)
      return ::planar::cmd::handlers::takeover_with_profile(families.takeover, prefix.selector);
    if (families.check_command->count() == 1)
      return ::planar::cmd::handlers::check_with_profile(families.check, prefix.selector);
    if (families.check_recover_command->count() == 1)
      return ::planar::cmd::handlers::check_recover_with_profile(families.check_recover, prefix.selector);
    if (families.verdict_command->count() == 1)
      return ::planar::cmd::handlers::verdict_with_profile(families.verdict, prefix.selector);
    if (families.recover_command->count() == 1)
      return ::planar::cmd::handlers::recover_with_profile(families.recover, prefix.selector);
    if (families.release_command->count() == 1)
      return ::planar::cmd::handlers::release_with_profile(families.release, prefix.selector);
    if (families.checkpoint_command->count() == 1)
      return ::planar::cmd::handlers::checkpoint_with_profile(families.checkpoint, prefix.selector);
    if (families.refresh_command->count() == 1)
      return ::planar::cmd::handlers::refresh_with_profile(families.refresh, prefix.selector);
    if (families.claim_command->count() == 1)
      return ::planar::cmd::handlers::claim_with_profile(families.claim, prefix.selector);
    // The probe state is the final state: the same options object is admitted and dispatched.
    return execute_with_profile(families.intent(), prefix.selector);
  }
  auto selector = prepare_process_profile_selector(prefix.selector, snapshot);
  if (!selector)
    return detail::failed(detail::native_error(selector.error()));
  return open_selected(*selector, false, prefix.passthrough, prefix.tail, snapshot);
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
    envelope::prefix_binding binding;
    const auto&              prefix_command = envelope::attach_prefix(root, binding);
    shipped_families         families;
    families.attach(root);
#ifdef PLANAR_AUTHORING_BUNDLE
    authoring_families extra;
    extra.attach(root);
#endif
    std::vector<std::string> tokens(arguments.rbegin(), arguments.rend());
    try {
      root.parse(tokens);
    } catch (const CLI::ParseError&) {
      return detail::usage();
    }
    if (prefix_command.count() != 0)
      return invoke_with_profile(binding, arguments.size());
    if (help && arguments.size() == 1) {
#ifdef PLANAR_AUTHORING_BUNDLE
      if (auto spliced = authoring::splice_root_help(default_root_help))
        return {0, std::move(*spliced), ""};
      return detail::failed(detail::native_error({"internal", "Invalid embedded command catalog"}));
#else
      return {0, std::string(default_root_help), ""};
#endif
    }
    // CLI11 can discard positional terminators after scalar values. Count only
    // library-parsed bindings against our exact separate-token interface.
#ifdef PLANAR_AUTHORING_BUNDLE
    const std::size_t selected_commands = families.selected() + extra.selected();
    const std::size_t parsed_tokens     = root.count("--help") + families.parsed_tokens() + extra.parsed_tokens();
#else
    const auto        selected_commands = families.selected();
    const std::size_t parsed_tokens     = root.count("--help") + families.parsed_tokens();
#endif
    if (help || selected_commands != 1 || !root.remaining(true).empty() || parsed_tokens != arguments.size())
      return detail::usage();
    if (families.render_command->count() == 1 && !families.render_consistent())
      return detail::usage();
    auto vm = engine::command_vm::create();
    if (!vm)
      return detail::failed(detail::native_error(vm.error()));
    if (!catalog_matches(vm->catalog(), root))
      return detail::failed(detail::native_error({"internal", "Invalid embedded command catalog"}));
#ifdef PLANAR_AUTHORING_BUNDLE
    if (auto answered = extra.dispatch(&*vm))
      return std::move(*answered);
#endif
    if (families.activate_command->count() == 1)
      return ::planar::cmd::handlers::activate_command(families.activate);
    if (families.status_command->count() == 1)
      return ::planar::cmd::handlers::status_command(families.status);
    if (families.preview_command->count() == 1)
      return ::planar::cmd::handlers::preview_command(families.preview);
    if (families.takeover_command->count() == 1)
      return ::planar::cmd::handlers::takeover_command(families.takeover);
    if (families.check_command->count() == 1)
      return ::planar::cmd::handlers::check_command(families.check);
    if (families.check_recover_command->count() == 1)
      return ::planar::cmd::handlers::check_recover_command(families.check_recover);
    if (families.verdict_command->count() == 1)
      return ::planar::cmd::handlers::verdict_command(families.verdict);
    if (families.recover_command->count() == 1)
      return ::planar::cmd::handlers::recover_command(families.recover);
    if (families.release_command->count() == 1)
      return ::planar::cmd::handlers::release_command(families.release);
    if (families.checkpoint_command->count() == 1)
      return ::planar::cmd::handlers::checkpoint_command(families.checkpoint);
    if (families.refresh_command->count() == 1)
      return ::planar::cmd::handlers::refresh_command(families.refresh);
    if (families.claim_command->count() == 1)
      return ::planar::cmd::handlers::claim_command(families.claim);
    if (families.load_command->count() == 1)
      return handlers::load_command(families.load, &*vm);
    if (families.export_command->count() == 1)
      return handlers::export_command(families.source_export, &*vm);
    if (families.history_command->count() == 1)
      return handlers::history_command(families.history, &*vm);
    if (families.init_command->count() == 1)
      return handlers::init_command(families.init, &*vm);
    return handlers::render_command(families.render, &*vm);
  } catch (...) {
    return {4, "", ""};
  }
}
} // namespace planar::cmd
