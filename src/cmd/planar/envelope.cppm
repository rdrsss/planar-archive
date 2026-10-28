/// @file
/// @brief With-profile prefix envelope over CLI11 and exact selected-help formatting; runtime-free and database-free.
///
/// Entry points are parse_prefix, which classifies the leading `with-profile` tokens without any profile access, and
/// format_selected_help, which renders already extracted plain catalog data. The module imports no runtime types so that
/// generated command variants can rename the runtime without touching it. Expected failures return usage or a typed
/// diagnostic; nothing throws across the boundary.
export module planar.cmd.envelope;
import std;
import cli11;
export import planar.engine.json_codec;

export namespace planar::cmd::envelope {
using engine::diagnostic;
/// Outcome of the with-profile prefix parse; owns every byte.
struct prefix_result {
  /// Closed classification of the prefix.
  enum class kind {
    usage,  ///< Prefix syntax refusal; the profile is not accessed.
    help,   ///< Sole selected help for the selector.
    profile ///< Selector plus a nonempty original tail.
  };
  kind                     outcome = kind::usage; ///< Classification.
  std::string              selector;              ///< Raw selector text, never normalized here.
  std::vector<std::string> tail;                  ///< Original tail tokens in argv order.
  std::vector<std::string> passthrough;           ///< The same tail as CLI11's reverse-order parse stack, unchanged.
};
/// CLI11 state of the with-profile subcommand; must stay alive and unmoved from attachment through classification.
struct prefix_binding {
  std::string  selector;                  ///< Raw selector text filled by CLI11.
  bool         help            = false;   ///< Counted ordinary help flag filled by CLI11.
  CLI::App*    command         = nullptr; ///< The attached with-profile subcommand, owned by its root.
  CLI::Option* selector_option = nullptr; ///< The scalar selector option, owned by the subcommand.
  CLI::Option* help_option     = nullptr; ///< The counted help flag, owned by the subcommand.
};
/// Attach the with-profile subcommand to a root that also carries the shipped families.
/// @param root Root owned by the caller; its subcommand keeps references into binding.
/// @param binding Option storage that outlives parsing and classification.
/// @return The attached subcommand. It uses prefix_command(), a required scalar selector with Throw multiplicity
/// (nonempty, not beginning `--`, valid UTF-8 without NUL) and a counted `--help` flag with default help disabled.
CLI::App& attach_prefix(CLI::App& root, prefix_binding& binding);
/// Classify a successfully parsed with-profile prefix from CLI11 metadata alone.
/// @param binding Binding whose subcommand was selected by a parse that did not throw.
/// @param argument_count Number of tokens in the complete invocation.
/// @return Usage, selected help or profile-with-tail; no filesystem, environment or profile access.
prefix_result classify_prefix(const prefix_binding& binding, std::size_t argument_count);
/// Classify the with-profile prefix of a complete argument list over a root that carries no shipped family.
/// @param arguments Complete arguments excluding argv[0], borrowed for this call.
/// @return Usage, selected help or profile-with-tail; no filesystem, environment or profile access.
/// @note A shipped invocation is usage here; production dispatch parses one root that carries the shipped families.
prefix_result parse_prefix(std::span<const std::string_view> arguments);
/// Plain-data option of a selected catalog command.
struct help_option {
  std::string name;     ///< Descriptor option name including leading dashes.
  std::string type;     ///< Descriptor type name.
  bool        required; ///< True when the option is required.
  std::string help;     ///< Raw help text, escaped on output.
};
/// Plain-data command of a selected catalog.
struct help_command {
  std::string              name;    ///< Command name.
  std::string              help;    ///< Raw help text, escaped on output.
  std::vector<help_option> options; ///< Options in any order; output sorts them.
};
/// Format deterministic selected-catalog help.
/// @param commands Plain catalog data in any order.
/// @param ceiling Maximum output bytes.
/// @return UTF-8 LF help text, or a resource_limit diagnostic when the output would exceed the ceiling.
std::expected<std::string, diagnostic> format_selected_help(std::span<const help_command> commands,
                                                            std::size_t                   ceiling = 1048576);
} // namespace planar::cmd::envelope
