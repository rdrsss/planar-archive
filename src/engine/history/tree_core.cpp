module planar.engine.history.tree_core;
import std;
import planar.engine.value;
namespace planar::engine::history {
namespace {
constexpr std::size_t mib = 1024 * 1024;
struct limits {
  std::size_t bytes;
  std::size_t files;
  std::size_t directories;
};
std::optional<limits> budgets(tree_profile profile) {
  switch (profile) {
  case tree_profile::source:
    return limits{64 * mib, 4096, 8192};
  case tree_profile::provenance:
    return limits{16 * mib, 4096, 1 + 4096 * 63};
  case tree_profile::effective_provenance:
    return limits{32 * mib, 8192, 1 + 8192 * 63};
  }
  return std::nullopt;
}
auto invalid() {
  return std::unexpected(tree_error{tree_error_code::invalid_tree, {}});
}
auto excessive() {
  return std::unexpected(tree_error{tree_error_code::resource_limit, {}});
}
auto storage(ledger::types::format_error error) {
  return std::unexpected(tree_error{tree_error_code::storage, std::move(error)});
}
std::string folded(std::string_view input) {
  std::string result(input);
  for (auto& c : result)
    if (c >= 'A' && c <= 'Z')
      c += 'a' - 'A';
  return result;
}
struct build_node {
  std::string                        name;
  std::optional<std::size_t>         file;
  std::map<std::string, std::size_t> children;
  ledger::types::record_reference    reference{};
};
struct occurrence {
  ledger::types::record_reference reference;
  std::string                     path;
  std::size_t                     depth;
};
std::expected<std::vector<tree_file>, tree_error> reconstruct(const tree_record_reader&       reader,
                                                              ledger::types::record_reference root, tree_profile profile) {
  const auto bound = budgets(profile);
  if (!bound || root.kind != record_kind::directory)
    return invalid();
  std::vector<occurrence> pending{{root, {}, 0}};
  std::vector<tree_file>  result;
  std::size_t             files = 0, directories = 1, bytes = 0;
  while (!pending.empty()) {
    auto current = std::move(pending.back());
    pending.pop_back();
    // A decoded record is destroyed before the next iteration: no record-sized scratch per depth.
    auto stored = reader(current.reference.id);
    if (!stored)
      return storage(stored.error());
    if (stored->reference.kind != current.reference.kind || stored->reference.digest != current.reference.digest)
      return invalid();
    if (current.reference.kind == record_kind::file) {
      auto& file = std::get<file_record>(stored->value);
      if (file.payload.size() > bound->bytes - bytes)
        return excessive();
      bytes += file.payload.size();
      result.push_back({std::move(current.path), std::move(file.payload), file.mode});
      continue;
    }
    const auto& children = std::get<directory_record>(stored->value).children;
    if (children.empty() && current.depth != 0)
      return invalid();
    // Count and validate every child before allocating its traversal occurrence.
    for (const auto& child : children) {
      if (child.kind != record_kind::file && child.kind != record_kind::directory)
        return invalid();
      if (current.depth == 64)
        return excessive();
      const auto length = current.path.size() + (current.path.empty() ? 0 : 1) + child.name.size();
      if (length > 1024)
        return excessive();
      if (child.kind == record_kind::file) {
        if (files == bound->files)
          return excessive();
        ++files;
      } else {
        if (directories == bound->directories)
          return excessive();
        ++directories;
      }
    }
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
      std::string path = current.path;
      if (!path.empty())
        path += '/';
      path += it->name;
      if (!valid_path(path, false))
        return invalid();
      pending.push_back({{it->id, it->kind, it->digest}, std::move(path), current.depth + 1});
    }
  }
  std::ranges::sort(result, {}, &tree_file::path);
  return result;
}
template <class Intern>
std::expected<ledger::types::record_reference, tree_error> construct_tree(std::span<const tree_file_view> files,
                                                                          tree_profile profile, Intern intern) {
  const auto bound = budgets(profile);
  if (!bound)
    return invalid();
  if (files.size() > bound->files)
    return excessive();
  std::size_t bytes = 0;
  for (const auto& file : files) {
    if (file.path.size() > 1024 || std::ranges::count(file.path, '/') >= 64 || file.payload.size() > 16 * mib ||
        file.payload.size() > bound->bytes - bytes)
      return excessive();
    if (!valid_path(file.path, false) || (file.mode != 420 && file.mode != 493))
      return invalid();
    bytes += file.payload.size();
  }
  std::vector<build_node> nodes{{{}, {}, {}, {}}};

  std::size_t directories = 1;
  for (std::size_t i = 0; i < files.size(); ++i) {
    auto        path   = files[i].path;
    std::size_t parent = 0, offset = 0;
    while (true) {
      const auto slash    = path.find('/', offset);
      const auto end      = slash == std::string_view::npos ? path.size() : slash;
      const auto name     = path.substr(offset, end - offset);
      auto       key      = folded(name);
      auto       existing = nodes[parent].children.find(key);
      const bool leaf     = slash == std::string_view::npos;
      if (existing != nodes[parent].children.end()) {
        const auto& node = nodes[existing->second];
        if (node.name != name || node.file || leaf)
          return invalid();
        parent = existing->second;
      } else {
        if (!leaf && directories == bound->directories)
          return excessive();
        if (!leaf)
          ++directories;
        const auto index = nodes.size();
        nodes.push_back({std::string(name), leaf ? std::optional(i) : std::nullopt, {}, {}});
        nodes[parent].children.emplace(std::move(key), index);
        parent = index;
      }
      if (leaf)
        break;
      offset = end + 1;
    }
  }
  // Parents were inserted before children. Reverse order is a bounded postorder.
  for (std::size_t n = nodes.size(); n-- > 0;) {
    record value;
    if (nodes[n].file) {
      const auto& file = files[*nodes[n].file];
      value            = file_record{file.mode, std::string(file.payload)};
    } else {
      directory_record directory;
      directory.children.reserve(nodes[n].children.size());
      for (const auto& [key, index] : nodes[n].children) {
        const auto& ref = nodes[index].reference;
        directory.children.push_back({nodes[index].name, ref.kind, ref.id, ref.digest});
      }
      std::ranges::sort(directory.children, {}, &record_child::name);
      value = std::move(directory);
    }
    auto ref = intern(value);
    if (!ref)
      return std::unexpected(ref.error());
    nodes[n].reference = *ref;
  }
  return nodes[0].reference;
}
} // namespace
std::expected<ledger::types::record_reference, tree_error>
build_tree_core(std::span<const tree_file_view> files, tree_profile profile, const tree_record_intern& intern) {
  return construct_tree(files, profile, [&](const record& value) -> std::expected<ledger::types::record_reference, tree_error> {
    auto ref = intern(value);
    if (!ref)
      return storage(ref.error());
    return *ref;
  });
}
std::expected<record_digest, tree_error> digest_tree(std::span<const tree_file_view> files, tree_profile profile) {
  auto result =
      construct_tree(files, profile, [](const record& value) -> std::expected<ledger::types::record_reference, tree_error> {
        auto digest = digest_record(value);
        if (!digest) {
          if (digest.error().code == record_error_code::resource_limit)
            return excessive();
          return invalid();
        }
        return ledger::types::record_reference{
            1, std::holds_alternative<file_record>(value) ? record_kind::file : record_kind::directory, *digest};
      });
  if (!result)
    return std::unexpected(result.error());
  return result->digest;
}
std::expected<std::vector<tree_file>, tree_error> read_tree_core(ledger::types::record_reference root, tree_profile profile,
                                                                 const tree_record_reader& reader) {
  return reconstruct(reader, root, profile);
}
} // namespace planar::engine::history
