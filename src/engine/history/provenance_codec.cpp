module planar.engine.history.provenance_codec;
import std;
import planar.engine.canonical;
namespace planar::engine::history {
namespace {
constexpr std::size_t      maximum       = 16 * 1024 * 1024;
constexpr std::string_view chunk_path    = "__planar__/assembly.lua";
constexpr std::string_view manifest_path = "__planar__/assembly.canonical";
auto                       refuse(provenance_error_code code = provenance_error_code::invalid_provenance) {
  return std::unexpected(provenance_error{code});
}
provenance_error_code tree_code(tree_error error) {
  return error.code == tree_error_code::resource_limit ? provenance_error_code::resource_limit
                                                       : provenance_error_code::invalid_tree;
}
bool add(std::size_t& total, std::size_t size) {
  if (size > maximum - total)
    return false;
  total += size;
  return true;
}
value_ptr text(std::string_view bytes) {
  auto v    = std::make_shared<value>();
  v->kind   = value_kind::string;
  v->string = bytes;
  return v;
}
value_ptr manifest(std::span<const original_file_view> files, std::string_view separator) {
  auto list  = std::make_shared<value>();
  list->kind = value_kind::list;
  for (const auto& file : files)
    list->items.push_back(text(file.path));
  auto version     = std::make_shared<value>();
  version->kind    = value_kind::integer;
  version->integer = 1;
  auto root        = std::make_shared<value>();
  root->kind       = value_kind::record;
  root->fields = {{"version", version}, {"scheme", text("ordered_concat_v1")}, {"files", list}, {"separator", text(separator)}};
  return root;
}
bool reserved(std::string_view path) {
  auto                       first = path.substr(0, path.find('/'));
  constexpr std::string_view name  = "__planar__";
  if (first.size() != name.size())
    return false;
  for (std::size_t i = 0; i < name.size(); ++i) {
    auto c = first[i];
    if (c >= 'A' && c <= 'Z')
      c += 'a' - 'A';
    if (c != name[i])
      return false;
  }
  return true;
}
std::expected<prepared_provenance, provenance_error> own(std::span<const tree_file_view> files) {
  auto digest = digest_tree(files, tree_profile::provenance);
  if (!digest)
    return refuse(tree_code(digest.error()));
  prepared_provenance result{{}, *digest};
  result.files.reserve(files.size());
  for (const auto& file : files)
    result.files.push_back({std::string(file.path), std::string(file.payload), file.mode});
  std::ranges::sort(result.files, {}, &tree_file::path);
  return result;
}
} // namespace
std::expected<prepared_provenance, provenance_error> prepare_registry(std::string_view bytes, std::uint32_t mode) {
  const std::array files{tree_file_view{"input.json", bytes, mode}};
  return own(files);
}
std::expected<prepared_provenance, provenance_error> prepare_stdlib(std::span<const original_file_view> originals,
                                                                    std::string_view separator, std::string_view assembly) {
  if (originals.size() > 4094 || separator.size() > maximum || assembly.size() > maximum)
    return refuse(provenance_error_code::resource_limit);
  std::size_t total = assembly.size(), assembled_size = 0;
  for (const auto& file : originals) {
    if (file.path.size() > 1024 || file.bytes.size() > maximum || !add(total, file.bytes.size()) ||
        !add(assembled_size, file.bytes.size()) || !add(assembled_size, separator.size()))
      return refuse(provenance_error_code::resource_limit);
    if (reserved(file.path))
      return refuse();
  }
  if (assembled_size != assembly.size())
    return refuse();
  // Metadata scratch is independently bounded by count, path and separator limits.
  // Encode directly through the shared codec before copying any file payloads.
  auto encoded = canonical_encode(manifest(originals, separator), {.bytes = maximum - total});
  if (!encoded || !add(total, encoded->size()))
    return refuse(provenance_error_code::resource_limit);
  std::size_t offset = 0;
  for (const auto& file : originals) {
    if (assembly.substr(offset, file.bytes.size()) != file.bytes)
      return refuse();
    offset += file.bytes.size();
    if (assembly.substr(offset, separator.size()) != separator)
      return refuse();
    offset += separator.size();
  }
  std::vector<tree_file_view> files;
  files.reserve(originals.size() + 2);
  for (const auto& file : originals)
    files.push_back({file.path, file.bytes, 420});
  files.push_back({chunk_path, assembly, 420});
  files.push_back({manifest_path, *encoded, 420});
  return own(files);
}
std::expected<prepared_provenance, provenance_error> verify_stdlib(std::span<const tree_file> files) {
  if (files.size() > 4096)
    return refuse(provenance_error_code::resource_limit);
  std::vector<tree_file_view> views;
  std::size_t                 total = 0;
  const tree_file *           chunk = nullptr, *metadata = nullptr;
  for (const auto& file : files) {
    if (!add(total, file.payload.size()))
      return refuse(provenance_error_code::resource_limit);
    if (file.mode != 420)
      return refuse();
    views.push_back({file.path, file.payload, file.mode});
    if (file.path == chunk_path)
      chunk = &file;
    if (file.path == manifest_path)
      metadata = &file;
  }
  auto digest = digest_tree(views, tree_profile::provenance);
  if (!digest)
    return refuse(tree_code(digest.error()));
  if (!chunk || !metadata)
    return refuse();
  auto decoded = canonical_decode(metadata->payload, {.depth = 4, .nodes = 8200, .bytes = maximum});
  if (!decoded)
    return refuse(decoded.error().code == "resource_limit" ? provenance_error_code::resource_limit
                                                           : provenance_error_code::invalid_provenance);
  if ((*decoded)->kind != value_kind::record || (*decoded)->fields.size() != 4)
    return refuse();
  const auto& fields = (*decoded)->fields;
  for (auto key : {"version", "scheme", "files", "separator"})
    if (!fields.contains(key))
      return refuse();
  const auto& version   = *fields.at("version");
  const auto& scheme    = *fields.at("scheme");
  const auto& list      = *fields.at("files");
  const auto& separator = *fields.at("separator");
  if (version.kind != value_kind::integer || scheme.kind != value_kind::string || list.kind != value_kind::list ||
      separator.kind != value_kind::string)
    return refuse();
  if (version.integer != 1 || scheme.string != "ordered_concat_v1")
    return refuse(provenance_error_code::unsupported_manifest);
  if (files.size() < 2 || list.items.size() != files.size() - 2)
    return refuse();
  std::vector<original_file_view> originals;
  std::set<std::string_view>      seen;
  for (const auto& item : list.items) {
    if (item->kind != value_kind::string || reserved(item->string) || !seen.insert(item->string).second)
      return refuse();
    auto file = std::ranges::find(files, item->string, &tree_file::path);
    if (file == files.end())
      return refuse();
    originals.push_back({file->path, file->payload});
  }
  return prepare_stdlib(originals, separator.string, chunk->payload);
}
} // namespace planar::engine::history
