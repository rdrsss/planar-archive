/// @file
/// @brief Immutable structural declaration values and native boundary diagnostics.
export module planar.engine.value;
import std;
export namespace planar::engine {
/// Typed refusal without host paths or interpreter internals.
struct diagnostic {
  std::string code;         ///< Contract error code.
  std::string message;      ///< Safe explanatory text.
  std::string limit   = {}; ///< Resource name, when applicable.
  std::size_t maximum = 0;  ///< Resource maximum, when applicable.
};
/// Structural kinds; product schemas remain trusted Lua policy.
enum class value_kind { boolean, integer, string, record, list, workspace, spec, job, md, file, immaterial };
struct value;
/// Immutable shared ownership; consumers cannot mutate captured declarations.
using value_ptr = std::shared_ptr<const value>;
/// Structural node. Only fields appropriate to kind are populated.
struct value {
  value_kind                       kind;            ///< Structural or private constructor tag.
  bool                             boolean = false; ///< Boolean payload.
  std::int64_t                     integer = 0;     ///< Integer payload.
  std::string                      string;          ///< String, Markdown or file-reference payload.
  std::map<std::string, value_ptr> fields;          ///< Sorted record fields.
  std::vector<value_ptr>           items;           ///< Ordered list or wrapper payload.
};
/// @brief Validate portable relative file or directory boundary syntax.
/// @param path Relative portable path.
/// @param directory Whether a trailing slash is required.
/// @return True only for the contract grammar.
bool valid_path(std::string_view path, bool directory = false);
/// @brief Validate Unicode scalar UTF-8 and refuse NUL.
/// @param bytes Raw text bytes.
/// @return Whether the bytes are valid non-NUL UTF-8.
bool valid_text(std::string_view bytes);
} // namespace planar::engine
