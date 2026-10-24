/// @file
/// @brief Buffered command responses and canonical diagnostic helpers; no shared mutable state.
export module planar.cmd.response;
import std;
export import planar.engine.json_codec;
import planar.engine.ledger.types;

export namespace planar::cmd {
/// Buffered process result; each invocation owns all bytes and is safe for concurrent calls.
struct response {
  int         status;         ///< Process exit status.
  std::string output;         ///< Complete stdout bytes.
  std::string error;          ///< Complete stderr bytes.
  std::string stdout_error{}; ///< Optional prebuilt stdout-write diagnostic after successful publication.
};
/// Encode a checked stdout-write failure through the product codec.
/// @return Status 4 and structured stderr, or empty stderr on allocation failure.
response stdout_failure();
} // namespace planar::cmd
export namespace planar::cmd::detail {
using namespace planar::engine;
/// Construct a string node.
/// @param value Owned UTF-8 bytes.
/// @return Typed string node.
value_ptr text(std::string value);
/// Construct an integer node.
/// @param number Signed integer.
/// @return Typed integer node.
value_ptr integer(std::int64_t number);
/// Construct a record node.
/// @param fields Owned fields.
/// @return Typed record node.
value_ptr object(std::map<std::string, value_ptr> fields);
/// Construct a list node.
/// @param items Owned ordered elements.
/// @return Typed list node.
value_ptr array(std::vector<value_ptr> items = {});
/// Convert a native diagnostic into transport fields.
/// @param error Native boundary diagnostic.
/// @return Structured diagnostic value.
value_ptr native_error(const diagnostic& error);
/// Convert a typed ledger refusal to stable redacted transport fields.
/// @param error Native category; native message bytes are deliberately ignored.
/// @return Redacted public diagnostic with validated typed details; malformed payloads become internal errors.
value_ptr ledger_error(const engine::ledger::types::format_error& error);
/// Encode an error with the product codec and status policy.
/// @param error Structured error value.
/// @return Buffered failure; allocation failures can leave stderr empty.
response failed(value_ptr error);
/// Encode the fixed invocation diagnostic without host arguments.
/// @return Status 2 and canonical stderr.
response usage();
} // namespace planar::cmd::detail
