/// @file
/// @brief Authoring verb vocabulary, root help splice, process readers and input preparation (RA-1 wave 1 skeleton).
/// Entry points: root_help_lines, splice_root_help, process_readers, prepare_inputs and run_stub.
/// Error boundaries: every fallible call returns a typed diagnostic or a buffered response; nothing throws by contract.
export module planar.cmd.authoring;
import std;
export import planar.cmd.response;
import planar.engine.capture.host_path;

export namespace planar::cmd::authoring {
/// The six authoring verbs in RA-1 order.
enum class verb { spec_prompt, critique, ingest, explain_rejection, observe, orchestrator };
/// Public command spelling of a verb.
/// @param which Verb to name.
/// @return Static spelling such as `explain-rejection`.
std::string_view verb_name(verb which);
/// The six RA-1 root help lines, one per verb in RA-1 order, each newline terminated.
/// @return Static views.
std::array<std::string_view, 6> root_help_lines();
/// Insert the six RA-1 lines before the final `planar --help` line of a default root help text.
/// @param default_help Complete default help bytes.
/// @return Spliced bytes, or `std::nullopt` when the final line is absent.
std::optional<std::string> splice_root_help(std::string_view default_help);
/// Deferred process observation returning text or a typed diagnostic.
using reader = std::function<std::expected<std::string, engine::diagnostic>()>;
/// Injectable process readers; each call is independent and thread-safe when its reader is.
struct readers {
  reader cwd;                 ///< Invocation current directory.
  reader workspace_directory; ///< Directory holding the selected workspace.
};
/// Readers backed by the real process state.
/// @return Readers observing the live process.
readers process_readers();
/// Raw option values of one authoring invocation; unused members stay empty.
struct inputs {
  std::string workspace;  ///< Workspace argument.
  std::string registry;   ///< Registry argument.
  std::string diagnostic; ///< Diagnostic argument.
  std::string from;       ///< Observation directory argument.
};
/// Resolve the host paths a verb names into canonical spellings.
/// @param which Selected verb.
/// @param values Raw values, replaced in place by resolved values.
/// @param io Process observation seam.
/// @return Success, or the redacted path or io diagnostic.
std::expected<void, engine::diagnostic> prepare_inputs(verb which, inputs& values, const readers& io);
/// The refusal response of an authoring verb whose implementation is not wired.
/// @return Buffered response.
response refuse_stub();
/// Run one authoring verb end to end.
/// @param which Selected verb.
/// @param values Raw option values.
/// @param io Process observation seam.
/// @return Buffered response.
response run_stub(verb which, inputs values, const readers& io);
} // namespace planar::cmd::authoring
