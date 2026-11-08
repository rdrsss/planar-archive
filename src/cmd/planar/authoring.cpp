module planar.cmd.authoring;
import std;
import planar.engine.capture.host_path;
namespace planar::cmd::authoring {
namespace {
constexpr std::string_view final_help_line = "planar --help\n";

/// Build a diagnostic field by field; the text must come from the closed message table.
engine::diagnostic make_diagnostic(std::string_view code, std::string_view text) {
  engine::diagnostic built;
  built.code.assign(code);
  built.message.assign(text);
  return built;
}

/// Lazily read cwd snapshot shared by every stage of one invocation; read at most once.
class cwd_snapshot {
public:
  explicit cwd_snapshot(const readers& io) : _io(io) {
  }
  /// Cwd view for a path spelling: absent for absolute spellings, otherwise read once.
  std::expected<std::optional<std::string_view>, engine::diagnostic> for_path(std::string_view argument) {
    if (argument.starts_with('/'))
      return std::optional<std::string_view>{};
    if (!_cwd) {
      auto captured = _io.cwd();
      if (!captured)
        return std::unexpected(make_diagnostic("io", "Cannot obtain invocation cwd"));
      _cwd = std::move(*captured);
    }
    return std::optional<std::string_view>(*_cwd);
  }

private:
  const readers&             _io;
  std::optional<std::string> _cwd;
};

/// Selector stage: expand a workspace name or keep an explicit file lexical; nothing is resolved on disk.
std::expected<void, engine::diagnostic> prepare_selector(std::string& workspace, const readers& io, cwd_snapshot& cwd) {
  const auto named = engine::workspace_is_name(workspace);
  if (!named)
    return std::unexpected(named.error());
  std::optional<std::string> directory;
  if (*named) {
    auto captured = io.workspace_directory();
    if (!captured)
      return std::unexpected(captured.error());
    directory = std::move(*captured);
  }
  auto expanded = engine::expand_workspace(workspace, directory ? std::optional<std::string_view>(*directory) : std::nullopt);
  if (!expanded)
    return std::unexpected(expanded.error());
  if (auto checked = engine::validate_host_path(*expanded); !checked)
    return std::unexpected(checked.error());
  auto base = cwd.for_path(*expanded);
  if (!base)
    return std::unexpected(base.error());
  auto prepared = engine::prepare_host_path(*expanded, *base);
  if (!prepared)
    return std::unexpected(prepared.error());
  workspace = std::move(prepared->intent);
  return {};
}

/// Explicit-path stage: validate, prepare against the shared cwd, then resolve on disk.
std::expected<void, engine::diagnostic> prepare_explicit(std::span<std::string* const> files, std::string* directory,
                                                         cwd_snapshot& cwd, bool remap_missing_leaf) {
  std::vector<std::string*> all(files.begin(), files.end());
  if (directory)
    all.push_back(directory);
  for (const auto* path : all)
    if (auto checked = engine::validate_host_path(*path); !checked)
      return std::unexpected(checked.error());
  for (auto* path : all) {
    auto base = cwd.for_path(*path);
    if (!base)
      return std::unexpected(base.error());
    auto prepared = engine::prepare_host_path(*path, *base);
    if (!prepared)
      return std::unexpected(prepared.error());
    *path = std::move(prepared->spelling);
  }
  for (auto* path : files) {
    auto resolved = engine::resolve_explicit_file(*path);
    if (!resolved) {
      // A leaf that does not exist at all is an unresolvable input, not a wrong kind of file.
      std::error_code ignored;
      const auto      status = std::filesystem::symlink_status(*path, ignored);
      if (remap_missing_leaf && resolved.error().message == "Explicit input must be a regular file" &&
          (ignored || status.type() == std::filesystem::file_type::not_found))
        return std::unexpected(make_diagnostic("io", "Cannot resolve explicit input path"));
      return std::unexpected(resolved.error());
    }
    *path = std::move(*resolved);
  }
  if (directory) {
    auto resolved = engine::resolve_explicit_directory(*directory);
    if (!resolved) {
      if (resolved.error().message == "Cannot resolve explicit input path")
        return std::unexpected(make_diagnostic("io", "Cannot resolve explicit root directory"));
      return std::unexpected(resolved.error());
    }
    *directory = std::move(*resolved);
  }
  return {};
}
} // namespace
std::string_view verb_name(verb which) {
  switch (which) {
  case verb::spec_prompt:
    return "spec-prompt";
  case verb::critique:
    return "critique";
  case verb::ingest:
    return "ingest";
  case verb::explain_rejection:
    return "explain-rejection";
  case verb::observe:
    return "observe";
  case verb::orchestrator:
    return "orchestrator";
  }
  return {};
}
std::array<std::string_view, 6> root_help_lines() {
  return {"planar spec-prompt --workspace FILE_OR_NAME --registry FILE\n",
          "planar critique --workspace FILE_OR_NAME --registry FILE\n",
          "planar ingest --workspace FILE_OR_NAME --registry FILE\n",
          "planar explain-rejection --diagnostic FILE\n",
          "planar observe --from DIRECTORY\n",
          "planar orchestrator --workspace FILE_OR_NAME --registry FILE --job ID\n  or --ledger PATH\n"};
}
std::optional<std::string> splice_root_help(std::string_view default_help) {
  if (!default_help.ends_with(final_help_line))
    return std::nullopt;
  const auto at = default_help.size() - final_help_line.size();
  if (at != 0 && default_help[at - 1] != '\n')
    return std::nullopt;
  std::string out(default_help.substr(0, at));
  for (const auto line : root_help_lines())
    out += line;
  out += final_help_line;
  return out;
}
readers process_readers() {
  const auto cwd = []() -> std::expected<std::string, engine::diagnostic> {
    try {
      return std::filesystem::current_path().lexically_normal().string();
    } catch (const std::filesystem::filesystem_error&) {
      return std::unexpected(make_diagnostic("io", "Cannot obtain invocation cwd"));
    }
  };
  const auto workspace_directory = []() -> std::expected<std::string, engine::diagnostic> {
    const auto* value = std::getenv("PLANAR_WORKSPACE_DIR");
    return engine::snapshot_workspace_directory(value ? std::optional<std::string_view>(value) : std::nullopt);
  };
  return {cwd, workspace_directory};
}
std::expected<void, engine::diagnostic> prepare_inputs(verb which, inputs& values, const readers& io) {
  cwd_snapshot cwd(io);
  if (which == verb::orchestrator) {
    const bool live     = !values.ledger.empty();
    const bool declared = !values.workspace.empty() && !values.registry.empty() && !values.job.empty();
    if ((live && (!values.workspace.empty() || !values.registry.empty() || !values.job.empty())) || (!live && !declared))
      return std::unexpected(make_diagnostic("usage", "Invalid command syntax"));
    if (live) {
      std::array<std::string*, 1> files{&values.ledger};
      return prepare_explicit(files, nullptr, cwd, false);
    }
  }
  switch (which) {
  case verb::spec_prompt:
  case verb::critique:
  case verb::ingest:
  case verb::orchestrator: {
    if (auto selected = prepare_selector(values.workspace, io, cwd); !selected)
      return std::unexpected(selected.error());
    std::array<std::string*, 1> files{&values.registry};
    // Only --diagnostic and --from remap an unresolvable leaf; --registry keeps the
    // resolve_explicit_file messages (authoring-contract Messages rows :489 and :492 win over
    // Common rule 5, :37-42; wave 1 D8 read literally matches render).
    return prepare_explicit(files, nullptr, cwd, false);
  }
  case verb::explain_rejection: {
    std::array<std::string*, 1> files{&values.diagnostic};
    return prepare_explicit(files, nullptr, cwd, true);
  }
  case verb::observe:
    return prepare_explicit({}, &values.from, cwd, false);
  }
  return {};
}
response refuse_stub() {
  try {
    return detail::failed(detail::native_error(make_diagnostic("internal", "Invalid embedded command protocol")));
  } catch (...) {
    return {4, "", ""};
  }
}
response run_stub(verb which, inputs values, const readers& io) {
  try {
    if (auto prepared = prepare_inputs(which, values, io); !prepared)
      return detail::failed(detail::native_error(prepared.error()));
    return refuse_stub();
  } catch (...) {
    return {4, "", ""};
  }
}
response run(verb which, inputs values, const readers& io, const executor& execute) {
  try {
    if (auto prepared = prepare_inputs(which, values, io); !prepared)
      return detail::failed(detail::native_error(prepared.error()));
    if (!execute)
      return refuse_stub();
    return execute(which, values);
  } catch (...) {
    return {4, "", ""};
  }
}
} // namespace planar::cmd::authoring
