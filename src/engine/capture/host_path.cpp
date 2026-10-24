module planar.engine.capture.host_path;
import std;
namespace planar::engine {
namespace {
std::string host_file(std::string_view input) {
  // Resolve the parent with filesystem semantics: a symlink followed by '..'
  // must not be collapsed lexically. Preserve the leaf for capture's no-follow
  // open, and reject file/ or file/. rather than turning them into valid files.
  const auto absolute = std::filesystem::absolute(std::filesystem::path(input));
  const auto status   = std::filesystem::symlink_status(absolute);
  if (std::filesystem::is_symlink(status))
    throw diagnostic{"path_denied", "Explicit input file is a symbolic link"};
  if (!std::filesystem::is_regular_file(status))
    throw diagnostic{"io", "Explicit input must be a regular file"};
  return (std::filesystem::canonical(absolute.parent_path()) / absolute.filename()).string();
}
std::string host_directory(std::string_view input) {
  const auto absolute = std::filesystem::absolute(std::filesystem::path(input));
  const auto resolved = std::filesystem::canonical(absolute);
  if (!std::filesystem::is_directory(resolved))
    throw diagnostic{"io", "Explicit root must be a directory"};

  // Walk original components, including components inside link targets. This
  // preserves '..' semantics while recognizing root-link/, root-link/. and
  // root-link/child/.. as the same prohibited final-root alias. Intermediate
  // links to different directories remain valid host canonicalization.
  std::deque<std::filesystem::path> pending;
  for (const auto& component : absolute.relative_path())
    pending.push_back(component);
  auto        current = absolute.root_path();
  std::size_t links   = 0;
  while (!pending.empty()) {
    const auto component = pending.front();
    pending.pop_front();
    if (component.empty() || component == ".")
      continue;
    if (component == "..") {
      current = current.parent_path();
      continue;
    }
    const auto candidate = current / component;
    const auto status    = std::filesystem::symlink_status(candidate);
    if (std::filesystem::is_symlink(status)) {
      if (++links > 64)
        throw diagnostic{"path_denied", "Too many symbolic links in explicit root"};
      if (std::filesystem::equivalent(std::filesystem::canonical(candidate), resolved))
        throw diagnostic{"path_denied", "Explicit root is a symbolic link target"};
      const auto target = std::filesystem::read_symlink(candidate);
      if (target.is_absolute())
        current = target.root_path();
      const auto relative = target.relative_path();
      for (auto it = relative.end(); it != relative.begin();)
        pending.push_front(*--it);
    } else {
      if (!std::filesystem::is_directory(status))
        throw diagnostic{"io", "Cannot resolve explicit root directory"};
      current = candidate;
    }
  }
  return resolved.string();
}
constexpr std::size_t       intent_path_max = 64 * 1024 * 1024;
std::unexpected<diagnostic> invalid_path(bool cwd = false) {
  return std::unexpected(diagnostic{"path_denied", cwd ? "Invalid explicit input cwd" : "Invalid explicit input path"});
}
std::unexpected<diagnostic> path_limit() {
  return std::unexpected(diagnostic{"resource_limit", "Explicit input path limit exceeded", "path_bytes", intent_path_max});
}
std::expected<void, diagnostic> check_workspace_directory(std::optional<std::string_view> directory) {
  if (!directory || directory->empty() || !directory->starts_with('/') || !valid_text(*directory))
    return std::unexpected(diagnostic{"usage", "Expected command options or --help; see --help"});
  if (directory->size() > intent_path_max)
    return path_limit();
  return {};
}
} // namespace
std::expected<std::string, diagnostic> resolve_explicit_file(std::string_view argument) {
  try {
    return host_file(argument);
  } catch (const diagnostic& error) {
    return std::unexpected(error);
  } catch (const std::filesystem::filesystem_error&) {
    return std::unexpected(diagnostic{"io", "Cannot resolve explicit input path"});
  }
}
std::expected<std::string, diagnostic> resolve_explicit_directory(std::string_view argument) {
  try {
    return host_directory(argument);
  } catch (const diagnostic& error) {
    return std::unexpected(error);
  } catch (const std::filesystem::filesystem_error&) {
    return std::unexpected(diagnostic{"io", "Cannot resolve explicit input path"});
  }
}
std::expected<std::string, diagnostic> normalize_intent_path(std::string_view argument, std::string_view explicit_cwd) {
  if (argument.size() > intent_path_max || explicit_cwd.size() > intent_path_max)
    return path_limit();
  if (argument.empty() || !valid_text(argument))
    return invalid_path();
  if (explicit_cwd.empty() || !valid_text(explicit_cwd))
    return invalid_path(true);
  const std::filesystem::path cwd(explicit_cwd);
  if (!cwd.is_absolute() || cwd.lexically_normal().native() != explicit_cwd)
    return invalid_path(true);
  const std::filesystem::path path(argument);
  const std::size_t           separator = explicit_cwd.ends_with('/') ? 0 : 1;
  if (!path.is_absolute() &&
      (separator > intent_path_max - explicit_cwd.size() || argument.size() > intent_path_max - explicit_cwd.size() - separator))
    return path_limit();
  auto normalized = (path.is_absolute() ? path : cwd / path).lexically_normal().string();
  if (normalized.size() > intent_path_max)
    return path_limit();
  return normalized;
}
std::expected<bool, diagnostic> workspace_is_name(std::string_view selector) {
  if (selector.starts_with('/') || selector.contains('/') || selector.ends_with(".lua"))
    return false;
  const auto alnum = [](char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'); };
  if (selector.empty() || selector.size() > 64 || !alnum(selector.front()) ||
      !std::ranges::all_of(selector.substr(1), [&](char c) { return alnum(c) || c == '_' || c == '-'; }))
    return std::unexpected(diagnostic{"usage", "Expected command options or --help; see --help"});
  return true;
}
std::expected<void, diagnostic> validate_host_path(std::string_view argument) {
  if (argument.empty() || !valid_text(argument))
    return invalid_path();
  if (argument.size() > intent_path_max)
    return path_limit();
  return {};
}
std::expected<void, diagnostic> validate_workspace_directory(std::optional<std::string_view> directory) {
  return check_workspace_directory(directory);
}
std::expected<void, diagnostic> validate_host_path_inputs(std::string_view                argument,
                                                          std::optional<std::string_view> explicit_cwd) {
  if (auto checked = validate_host_path(argument); !checked)
    return checked;
  if (argument.starts_with('/'))
    return {};
  if (!explicit_cwd || explicit_cwd->empty() || !valid_text(*explicit_cwd))
    return invalid_path(true);
  if (explicit_cwd->size() > intent_path_max)
    return path_limit();
  if (!explicit_cwd->starts_with('/'))
    return invalid_path(true);
  return {};
}
std::expected<prepared_host_path, diagnostic> prepare_host_path(std::string_view                argument,
                                                                std::optional<std::string_view> explicit_cwd) {
  if (auto checked = validate_host_path_inputs(argument, explicit_cwd); !checked)
    return std::unexpected(checked.error());
  std::string spelling;
  if (argument.starts_with('/')) {
    spelling = argument;
  } else {
    const std::filesystem::path cwd(*explicit_cwd);
    if (cwd.lexically_normal().native() != *explicit_cwd)
      return invalid_path(true);
    const std::size_t separator = explicit_cwd->ends_with('/') ? 0 : 1;
    if (separator > intent_path_max - explicit_cwd->size() ||
        argument.size() > intent_path_max - explicit_cwd->size() - separator)
      return path_limit();
    spelling = *explicit_cwd;
    if (separator)
      spelling += '/';
    spelling += argument;
  }
  auto intent = std::filesystem::path(spelling).lexically_normal().string();
  if (intent.size() > intent_path_max)
    return path_limit();
  return prepared_host_path{std::move(spelling), std::move(intent)};
}
std::expected<std::string, diagnostic> snapshot_workspace_directory(std::optional<std::string_view> directory) {
  if (auto checked = validate_workspace_directory(directory); !checked)
    return std::unexpected(checked.error());
  return std::string(*directory);
}
std::expected<std::string, diagnostic> expand_workspace(std::string_view selector, std::optional<std::string_view> directory) {
  auto named = workspace_is_name(selector);
  if (!named)
    return std::unexpected(named.error());
  if (!*named) {
    if (auto checked = validate_host_path(selector); !checked)
      return std::unexpected(checked.error());
    return std::string(selector);
  }
  if (auto checked = validate_workspace_directory(directory); !checked)
    return std::unexpected(checked.error());
  auto                       base      = std::filesystem::path(*directory).lexically_normal().string();
  constexpr std::string_view suffix    = "/workspace.lua";
  const std::size_t          separator = base.ends_with('/') ? 0 : 1;
  if (base.size() > intent_path_max || separator > intent_path_max - base.size() ||
      selector.size() > intent_path_max - base.size() - separator ||
      suffix.size() > intent_path_max - base.size() - separator - selector.size())
    return path_limit();
  if (separator)
    base += '/';
  base += selector;
  base += suffix;
  return base;
}
} // namespace planar::engine
