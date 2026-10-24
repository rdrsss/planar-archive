module planar.engine.history.provenance;
import planar.engine.runtime_source;
namespace planar::engine::history {
std::expected<prepared_provenance, provenance_error> prepare_embedded_stdlib() {
  auto                            embedded_files = embedded::originals();
  std::vector<original_file_view> originals;
  for (const auto& file : embedded_files)
    originals.push_back({file.path, file.bytes});
  return prepare_stdlib(originals, embedded::separator(), embedded::source());
}
} // namespace planar::engine::history
