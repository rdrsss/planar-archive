module planar.engine.history.tree;
import std;
namespace planar::engine::history {
namespace {
struct abort_guard {
  ledger::record_writer& writer;
  bool                   succeeded = false;
  ~abort_guard() {
    if (!succeeded)
      writer.abort();
  }
};
} // namespace
std::expected<ledger::record_reference, tree_error> build_tree(ledger::record_writer&          writer,
                                                               std::span<const tree_file_view> files, tree_profile profile) {
  abort_guard guard{writer};
  auto        result = build_tree_core(files, profile, [&](const record& value) { return writer.intern(value); });
  guard.succeeded    = result.has_value();
  return result;
}
std::expected<std::vector<tree_file>, tree_error> read_tree(const ledger::format_file& reader, ledger::record_reference root,
                                                            tree_profile profile) {
  return read_tree_core(root, profile, [&](std::uint64_t id) { return reader.read_record(id); });
}
std::expected<std::vector<tree_file>, tree_error> read_tree(ledger::record_writer& writer, ledger::record_reference root,
                                                            tree_profile profile) {
  abort_guard guard{writer};
  auto        result = read_tree_core(root, profile, [&](std::uint64_t id) { return writer.read_record(id); });
  guard.succeeded    = result.has_value();
  return result;
}
} // namespace planar::engine::history
