module planar.engine.capture.profile;
import std;
import planar.engine.capture.host_path;
namespace planar::engine {
namespace {
profile_error error(std::string_view code, std::string field = "profile") {
  std::string_view message = "Invalid external profile structure";
  if (code == "unsupported_version")
    message = "Unsupported external profile version";
  else if (code == "path")
    message = "Invalid external module path";
  else if (code == "resource_limit")
    message = "External profile resource limit exceeded";
  else if (code == "capture_changed")
    message = "Captured external profile changed";
  return {{std::string(code), std::string(message)}, std::move(field)};
}
std::string module_field(std::size_t i) {
  return std::format("profile.modules[{}]", i + 1);
}
// Propagation preserves established native messages while keeping host selectors private.
template <class T> T require(std::expected<T, diagnostic> result, const std::string& field = "profile") {
  if (!result)
    throw profile_error{result.error(), field};
  return std::move(*result);
}
void require(std::expected<void, diagnostic> result, const std::string& field = "profile") {
  if (!result)
    throw profile_error{result.error(), field};
}
} // namespace
/// @cond
struct profile_capture::implementation {
  capture_admission  admission;
  acceptance_capture content;
  profile_snapshot   original;
  mutable bool       failed = false;
  struct operation {
    implementation& state;
    bool            complete = false;
    ~operation() {
      if (!complete) {
        state.failed = true;
        state.admission.poison();
      }
    }
  };
  implementation(capture_admission a, acceptance_capture c, profile_snapshot p)
      : admission(std::move(a)), content(std::move(c)), original(std::move(p)) {
  }
};
/// @endcond
profile_capture::profile_capture(std::unique_ptr<implementation> state) : _impl(std::move(state)) {
}
profile_capture::~profile_capture()                          = default;
profile_capture::profile_capture(profile_capture&&) noexcept = default;
profile_capture& profile_capture::operator=(profile_capture&& other) noexcept {
  if (this != &other)
    _impl = std::move(other._impl);
  return *this;
}
std::expected<profile_capture, profile_error> profile_capture::open(std::string_view selector) {
  try {
    require(validate_host_path(selector));
    if (selector.empty() || selector.front() != '/')
      return std::unexpected(error("path"));
    auto slash     = selector.rfind('/');
    auto directory = std::string(selector.substr(0, slash ? slash : 1));
    auto admission = require(capture_admission::create(directory));
    require(admission.select_trusted("profile", selector));
    native_capture_session session;
    require(session.add_input("profile", std::string(selector)));
    auto profile = require(session.read_bounded("profile", "input.json", admission, "profile", 65536, 65536, false));
    auto paths   = decode_profile(profile.bytes);
    if (!paths)
      return std::unexpected(paths.error());
    if (!paths->empty())
      require(session.add_root("modules", directory));
    // Full-list policy and all physical selections precede every module read.
    for (std::size_t i = 0; i < paths->size(); ++i)
      require(admission.select_trusted(module_field(i), directory + "/" + (*paths)[i]), module_field(i));
    auto closure          = std::make_shared<profile_closure>();
    profile.root          = "profile";
    profile.path          = "profile.json";
    closure->profile      = std::move(profile);
    std::size_t remaining = 8388608;
    for (std::size_t i = 0; i < paths->size(); ++i) {
      auto file = require(session.read_bounded("modules", (*paths)[i], admission, module_field(i), 1048576, remaining, false),
                          module_field(i));
      if (!file.bytes.empty() && static_cast<unsigned char>(file.bytes.front()) == 0x1b)
        return std::unexpected(error("schema", module_field(i)));
      if (!valid_text(file.bytes))
        return std::unexpected(error("schema", module_field(i)));
      remaining -= file.bytes.size();
      closure->modules.push_back(std::move(file));
    }
    require(admission.verify());
    auto content = require(session.finish_for_acceptance());
    require(admission.verify());
    return profile_capture(std::make_unique<implementation>(std::move(admission), std::move(content), std::move(closure)));
  } catch (const profile_error& e) {
    return std::unexpected(e);
  } catch (const std::bad_alloc&) {
    return std::unexpected(error("resource_limit"));
  }
}
std::expected<profile_snapshot, profile_error> profile_capture::snapshot() const {
  if (!_impl || _impl->failed || !_impl->admission.healthy())
    return std::unexpected(error("capture_changed"));
  return _impl->original;
}
std::expected<void, profile_error> profile_capture::admit_root(std::string_view slot, std::string_view directory) {
  if (!_impl || _impl->failed || !_impl->admission.healthy())
    return std::unexpected(error("capture_changed"));
  implementation::operation guard{*_impl};
  auto                      result = _impl->admission.admit_root(slot, directory);
  if (!result) {
    _impl->failed = true;
    _impl->admission.poison();
    return std::unexpected(profile_error{result.error(), "profile"});
  }
  guard.complete = true;
  return {};
}
std::expected<void, profile_error> profile_capture::admit_file(std::string_view slot, std::string_view file,
                                                               admission_file_role role) {
  if (!_impl || _impl->failed || !_impl->admission.healthy())
    return std::unexpected(error("capture_changed"));
  implementation::operation guard{*_impl};
  auto                      result = _impl->admission.admit_file(slot, file, role);
  if (!result) {
    _impl->failed = true;
    _impl->admission.poison();
    return std::unexpected(profile_error{result.error(), "profile"});
  }
  guard.complete = true;
  return {};
}
std::expected<void, profile_error> profile_capture::admit_target(std::string_view slot, std::string_view directory) {
  if (!_impl || _impl->failed || !_impl->admission.healthy())
    return std::unexpected(error("capture_changed"));
  implementation::operation guard{*_impl};
  auto                      result = _impl->admission.admit_target(slot, directory);
  if (!result) {
    _impl->failed = true;
    _impl->admission.poison();
    return std::unexpected(profile_error{result.error(), "profile"});
  }
  guard.complete = true;
  return {};
}
std::expected<void, profile_error> profile_capture::verify_file(std::string_view slot, int descriptor) const {
  if (!_impl || _impl->failed || !_impl->admission.healthy())
    return std::unexpected(error("capture_changed"));
  implementation::operation guard{*_impl};
  auto                      result = _impl->admission.verify_file(slot, descriptor);
  if (!result) {
    _impl->failed = true;
    _impl->admission.poison();
    return std::unexpected(profile_error{result.error(), "profile"});
  }
  guard.complete = true;
  return {};
}
std::expected<void, profile_error> profile_capture::verify_root(std::string_view slot, int descriptor) const {
  if (!_impl || _impl->failed || !_impl->admission.healthy())
    return std::unexpected(error("capture_changed"));
  implementation::operation guard{*_impl};
  auto                      result = _impl->admission.verify_root(slot, descriptor);
  if (!result) {
    _impl->failed = true;
    _impl->admission.poison();
    return std::unexpected(profile_error{std::move(result.error()), "profile"});
  }
  guard.complete = true;
  return {};
}
std::expected<retained_root_receipt, profile_error> profile_capture::retain_root(std::string_view slot) {
  if (!_impl || _impl->failed || !_impl->admission.healthy())
    return std::unexpected(error("capture_changed"));
  implementation::operation guard{*_impl};
  try {
    auto token = _impl->admission.retain_root(slot);
    if (!token)
      return std::unexpected(profile_error{std::move(token.error()), "profile"});
    guard.complete = true;
    return std::move(*token);
  } catch (const std::bad_alloc&) {
    _impl->admission.poison();
    return std::unexpected(error("resource_limit"));
  }
}
std::expected<retained_file_receipt, profile_error> profile_capture::retain_file(std::string_view slot) {
  if (!_impl || _impl->failed || !_impl->admission.healthy())
    return std::unexpected(error("capture_changed"));
  implementation::operation guard{*_impl};
  try {
    auto token = _impl->admission.retain_file(slot);
    if (!token)
      return std::unexpected(profile_error{std::move(token.error()), "profile"});
    guard.complete = true;
    return std::move(*token);
  } catch (const std::bad_alloc&) {
    _impl->admission.poison();
    return std::unexpected(error("resource_limit"));
  }
}
std::expected<void, profile_error> profile_capture::bind_root(native_capture_session& session, std::string logical,
                                                              std::string directory, std::string_view slot) {
  if (!_impl || _impl->failed || !_impl->admission.healthy())
    return std::unexpected(error("capture_changed"));
  implementation::operation guard{*_impl};
  try {
    auto token = _impl->admission.retain_root(slot);
    if (!token)
      return std::unexpected(profile_error{std::move(token.error()), "profile"});
    auto result = session.add_root(std::move(logical), std::move(directory), std::move(*token));
    if (!result)
      return std::unexpected(profile_error{std::move(result.error()), "profile"});
    guard.complete = true;
    return {};
  } catch (const std::bad_alloc&) {
    _impl->admission.poison();
    return std::unexpected(error("resource_limit"));
  }
}
std::expected<void, profile_error> profile_capture::bind_input(native_capture_session& session, std::string logical,
                                                               std::string file, std::string_view slot) {
  if (!_impl || _impl->failed || !_impl->admission.healthy())
    return std::unexpected(error("capture_changed"));
  implementation::operation guard{*_impl};
  try {
    auto token = _impl->admission.retain_file(slot);
    if (!token)
      return std::unexpected(profile_error{std::move(token.error()), "profile"});
    auto result = session.add_input(std::move(logical), std::move(file), std::move(*token));
    if (!result)
      return std::unexpected(profile_error{std::move(result.error()), "profile"});
    guard.complete = true;
    return {};
  } catch (const std::bad_alloc&) {
    _impl->admission.poison();
    return std::unexpected(error("resource_limit"));
  }
}
std::expected<captured_file, profile_error> profile_capture::read(native_capture_session& session, std::string_view root,
                                                                  std::string_view path, std::string_view slot, bool text) {
  return read_bounded(session, root, path, slot, text ? 1048576 : 16777216, 67108864, text);
}
std::expected<captured_file, profile_error> profile_capture::read_bounded(native_capture_session& session, std::string_view root,
                                                                          std::string_view path, std::string_view slot,
                                                                          std::size_t byte_ceiling, std::size_t remaining_bytes,
                                                                          bool text) {
  if (!_impl || _impl->failed || !_impl->admission.healthy())
    return std::unexpected(error("capture_changed"));
  implementation::operation guard{*_impl};
  try {
    auto token = _impl->admission.retain_file(slot);
    if (!token) {
      _impl->admission.poison();
      return std::unexpected(profile_error{std::move(token.error()), "profile"});
    }
    auto result = session.read_bounded(root, path, std::move(*token), byte_ceiling, remaining_bytes, text);
    if (!result) {
      _impl->admission.poison();
      return std::unexpected(profile_error{std::move(result.error()), "profile"});
    }
    guard.complete = true;
    return std::move(*result);
  } catch (const std::bad_alloc&) {
    _impl->admission.poison();
    return std::unexpected(error("resource_limit"));
  }
}
std::expected<void, profile_error> profile_capture::verify() const {
  if (!_impl || _impl->failed || !_impl->admission.healthy())
    return std::unexpected(error("capture_changed"));
  implementation::operation guard{*_impl};
  try {
    if (!_impl->admission.verify() || !_impl->content.verify() || !_impl->admission.verify()) {
      _impl->failed = true;
      _impl->admission.poison();
      return std::unexpected(error("capture_changed"));
    }
    guard.complete = true;
    return {};
  } catch (const std::bad_alloc&) {
    _impl->admission.poison();
    return std::unexpected(error("resource_limit"));
  }
}
} // namespace planar::engine
