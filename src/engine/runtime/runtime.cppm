/// @file
/// @brief Bounded fresh exports and sealed invocation-local command VMs with typed native services and redacted failures.
export module planar.engine.runtime;
import std;
export import planar.engine.value;
export import planar.engine.capture.profile;
export import planar.engine.history.provenance_codec;
export import planar.engine.ledger.completion;
export import planar.engine.history.read;
export namespace planar::engine {
/// @brief Evaluate one product-selected export in a fresh trusted state.
/// @param entry Name in the embedded export table; never executable source or a path.
/// @param input Captured immutable structural data; strings may contain arbitrary bytes.
/// @return Independent typed result or sanitized native failure. Structured schema diagnostics are ordinary result data.
/// @note Thread-safe; no shared state, ambient I/O, process, clock, randomness or declaration environment.
std::expected<value_ptr, diagnostic> run_trusted(std::string_view entry, const value_ptr& input);
/// Parsed render intent retained natively; no host strings cross into Lua.
struct command_render_options {
  std::string                 workspace;               ///< Required private declaration path.
  std::string                 registry;                ///< Required private registry path.
  std::string                 root;                    ///< Required private ID=directory mapping.
  std::string                 job;                     ///< Required job identifier.
  std::string                 role;                    ///< Required role identifier.
  std::optional<std::string>  base_root;               ///< Optional private base mapping.
  std::optional<std::string>  observations;            ///< Optional private observations path.
  std::optional<std::int32_t> budget_bytes;            ///< Checked 1..1048576 value.
  bool                        capture_outputs = false; ///< Presence of the capture flag.
};
/// Parsed initialization intent; native publication revalidates workspace identity.
struct command_init_options {
  std::string ledger;       ///< Explicit private ledger path.
  std::string workspace_id; ///< Declaration workspace identity.
};
/// Parsed history intent; absent IDs remain distinguishable from explicit selectors.
struct command_history_options {
  std::string                 ledger;      ///< Explicit private ledger path.
  std::optional<std::int64_t> revision;    ///< Positive canonical revision ID.
  std::optional<std::int64_t> at_event;    ///< Positive canonical state-event ID.
  std::optional<std::int64_t> after_event; ///< Positive canonical page cursor.
  std::optional<std::string>  request_id;  ///< Bounded request key.
  std::optional<std::int32_t> limit;       ///< Checked 1..100 page size.
};
/// Parsed export intent retained throughout publication and response validation.
struct command_export_options {
  std::string  ledger;       ///< Explicit private ledger path.
  std::string  target;       ///< Explicit private output target.
  std::int64_t revision = 0; ///< Positive canonical CLI revision.
};
/// Native load intent; expected token and numeric head must agree exactly.
struct command_load_options {
  std::string                 ledger;          ///< Private ledger path.
  std::string                 workspace;       ///< Private normalized workspace path.
  std::string                 registry;        ///< Private normalized registry path.
  std::string                 expect_revision; ///< Canonical positive decimal or explicit none.
  std::string                 request_id;      ///< Bounded ASCII request key.
  std::string                 actor;           ///< Bounded nonempty UTF-8 driver assertion.
  std::optional<std::int64_t> expected_head;   ///< Numeric expected head, absent exactly for none.
};
/// Exactly one compiled family intent, copied into the invocation owner.
using command_options = std::variant<command_render_options, command_init_options, command_history_options,
                                     command_export_options, command_load_options>;
/// Verified history selector passed to the finite native boundary.
struct command_history_selector {
  std::string  selector;   ///< Fixed selector name.
  std::string  id;         ///< Exact canonical decimal ID.
  std::string  request_id; ///< Exact request key or empty.
  std::int32_t limit = 50; ///< Page limit, including the native default.
};
/// Native operation result. Exactly one member must be present; error is the existing typed E value.
struct command_service_result {
  value_ptr value; ///< Deep-copied bounded facts on success.
  value_ptr error; ///< Exact retained native failure.
};
/// Finite invocation-local native services. Providers own transactions entirely; no Lua callbacks are provided.
/// @note Implementations are called synchronously without concurrent VM access. They must release write locks before returning.
class command_services {
public:
  virtual ~command_services() = default;
  /// @brief Capture the declaration at the private workspace option.
  /// @param options Retained parsed native intent.
  /// @return Captured portable filename/declaration or typed native error.
  virtual command_service_result open_workspace(const command_render_options& options) = 0;
  /// @brief Capture/evaluate only the normalized route under the existing confinement checks.
  /// @param options Retained parsed native intent.
  /// @param route Deep-copied normalized route.
  /// @return Existing render input or typed native error.
  virtual command_service_result capture_render(const command_render_options& options, const value_ptr& route) = 0;
  /// @brief Exclusively initialize the explicit ledger.
  /// @param options Retained parsed native intent.
  /// @return Published initialization facts or typed native error.
  virtual command_service_result initialize_ledger(const command_init_options& options) = 0;
  /// @brief Read verified recorded values without executing recorded policy.
  /// @param options Retained parsed native intent.
  /// @param selector Native-verified semantic selector.
  /// @return Existing history envelope or typed native error.
  virtual command_service_result read_history(const command_history_options&  options,
                                              const command_history_selector& selector) = 0;
  /// @brief Publish verified source to the explicit target.
  /// @param options Retained parsed native intent.
  /// @return Published source facts or typed native error.
  virtual command_service_result export_source(const command_export_options& options) = 0;
  /// @brief Open the private normalized load workspace, synchronously and once.
  /// @param options Retained native intent; no Lua-selected host path.
  /// @return Exact {file,workspace} or typed failure. Default refuses unsupported load.
  virtual command_service_result open_workspace(const command_load_options& options);
  /// @brief Capture all declaration routes with the retained native load owner.
  /// @param options Retained native intent.
  /// @param route Bounded copied route from this VM.
  /// @return Exact {bundle} or typed failure. Default refuses unsupported load.
  virtual command_service_result capture_load(const command_load_options& options, const value_ptr& route);
  /// @brief Prepare and complete native acceptance, releasing all locks before return.
  /// @param options Retained native intent.
  /// @param bundle Normalized bundle from the retained callable.
  /// @param semantic Semantic value from the retained callable.
  /// @param provenance Immutable VM-owned verified files; valid only during this synchronous call.
  /// @param completion VM-owned receipt; retain saved bytes before commit and update phase without allocation.
  /// @return Complete saved load envelope or typed failure. Default refuses unsupported load.
  /// @note Providers own D5 correspondence/preparation and atomic completion; no Lua callback is available under locks.
  virtual command_service_result accept_load(const command_load_options& options, const value_ptr& bundle,
                                             const value_ptr& semantic, std::span<const history::tree_file> provenance,
                                             ledger::completion_receipt& completion);
};
/// Redacted selected-profile refusal retaining its native logical location.
struct registration_error {
  diagnostic  detail; ///< Closed diagnostic, never source bytes or host selectors.
  std::string field;  ///< Profile, profile.modules[N] or catalog location.
};
/// Exact immutable trusted preparation or assembly error, distinct from a handler semantic error.
struct assembly_error {
  value_ptr error; ///< Raw-validated exact inner E, including its original details.
};
/// First native failure from extension execution, retained even when Lua catches it.
using extension_execution_error =
    std::variant<registration_error, ledger::format_error, history::history_read_error, assembly_error>;
/// Owned selected external shape identity; no VM references or writable aliases escape.
struct shape_definition {
  std::string id;       ///< Registered shape ID.
  std::string revision; ///< Final registered semantic revision label.
  std::string digest;   ///< Exact shape_definition_v1 canonical XXH3-128 identity.
};
/// Owned selected external projection identity; no callable or VM references escape.
struct projection_definition {
  std::string id;       ///< Registered projection ID.
  std::string revision; ///< Final registered semantic revision label.
  std::string digest;   ///< Exact projection_definition_v1 canonical XXH3-128 identity.
};
/// Trusted read-data discriminator; authenticity is the synchronous native caller's responsibility.
enum class projection_input_kind { role_context_v1, history_snapshot_v1 };
/// Owned admitted projection result, independent of the retained VM lifetime.
struct projection_response {
  projection_definition definition; ///< Exact identity bound before the one retained call.
  value_ptr             result;     ///< Exact version1 section envelope after raw admission and encoded size validation.
};
/// Owned raw external shape response, awaiting trusted Lua semantic validation.
struct shape_response {
  shape_definition definition; ///< Identity bound before executing the retained callable.
  value_ptr        result;     ///< Bounded ordinary transport data; semantic refusal remains ordinary data.
};
/// Already parsed option; names omit --. Omitted records mean absent.
/// Present flags have no token; present scalars have exactly one token; absent records have no token.
/// Borrowed input is used only during synchronous preparation; CLI11 owns future argv parsing.
struct extension_parsed_option {
  std::string_view                name;            ///< Exact descriptor name without --.
  bool                            present = false; ///< Authoritative parsed presence.
  std::optional<std::string_view> token;           ///< Scalar token, never an argv sequence.
};
/// Capability-bound pending host selector kind; no filesystem admission is implied.
enum class extension_host_kind { workspace, registry, root, base_root, observations, ledger };
/// Native-only selector awaiting receipt-backed capture; never a value_ptr or Lua value.
struct extension_pending_host {
  extension_host_kind kind;     ///< Capability-bound purpose.
  std::string         selector; ///< Owned uninterpreted host selector, without normalization.
};
/// Typed pending-slot reference; absent host options have no index.
struct extension_slot_index {
  std::optional<std::size_t> index; ///< Index into this result's pending_hosts only.
};
/// Owned scalar or pending-slot reference. History IDs retain exact decimal strings.
using extension_option_value = std::variant<std::string, std::int32_t, bool, extension_slot_index>;
/// One descriptor option, including every absent option.
struct extension_prepared_option {
  std::string            name;            ///< Exact descriptor name without --.
  bool                   present = false; ///< Authoritative presence independent of default value.
  extension_option_value value;           ///< Typed default, supplied scalar or pending slot.
};
/// Pure native protocol preparation, independent of the VM lifetime; not a Lua-ready invocation.
struct extension_prepared_invocation {
  std::string                             command;       ///< Actual sealed external command name.
  std::vector<extension_prepared_option>  options;       ///< Complete canonical descriptor order.
  std::vector<extension_pending_host>     pending_hosts; ///< Native-only unadmitted selectors.
  std::optional<command_history_selector> history_query; ///< Effective selector/cursor/limit, without a ledger path.
};
/// Closed preparation refusal classification.
enum class extension_preparation_failure { usage, protocol, resource };
/// Redacted refusal with the existing shared terminal diagnostic when resource-bound.
struct extension_preparation_error {
  extension_preparation_failure kind;    ///< User syntax, trusted protocol or terminal resource refusal.
  registration_error            failure; ///< No supplied values or host selectors appear here.
};
class extension_invocation; ///< Original captured invocation owner, defined below.
/// Move-only single-invocation retained trusted command VM; no global cache or job-facing API.
/// @note Not concurrently callable. Moving preserves native ownership and Lua references; destruction releases all state.
class command_vm {
public:
  struct implementation; ///< Opaque stable native owner.
  /// @brief Register the embedded bundle, audit retained closure writes/native identities, then freeze its graph.
  /// @note Sealing shares checked native allocation accounting; invocation-local scratch remains writable.
  /// @return Complete sealed registry or redacted internal/resource failure.
  static std::expected<command_vm, diagnostic> create();
  /// @brief Register a structurally checked captured profile without filesystem access.
  /// @param profile Complete captured closure, stable during this call; bytes are copied into private immutable ownership.
  /// @return Sealed catalog-v2 registration owner or redacted refusal. Capture receipt verification remains the caller's duty.
  /// @note Generic command invocation refuses; the internal pure shape/projection bridges expose no services.
  static std::expected<command_vm, registration_error> create(profile_snapshot profile);
  /// @brief Borrow canonical catalog-v2 bytes for a selected-profile owner.
  /// @return Exact immutable canonical bytes, empty for embedded or moved-from owners.
  std::string_view effective_catalog() const;
  /// @brief Borrow the exact effective provenance digest for a selected-profile owner.
  /// @return Lowercase portable digest, empty for embedded or moved-from owners.
  std::string_view effective_digest() const;
  /// @brief Resolve an external shape's final descriptor and original selected source identity without executing Lua.
  /// @param id Trusted selected registered shape ID; user reference validation belongs to semantic policy.
  /// @return Owned metadata or the existing redacted protocol/resource refusal at catalog.shapes.
  /// @note Each lookup recomputes deterministically and charges cumulative native work before allocation, without resetting
  /// any budget. Resource failures leave the owner terminal; protocol refusal does not consume it. No invocation starts.
  std::expected<shape_definition, registration_error> resolve_shape_definition(std::string_view id);
  /// @brief Resolve a final external projection descriptor and original source identity without executing Lua.
  /// @param id Trusted selected registered projection ID.
  /// @return Owned identity or redacted protocol/resource refusal at catalog.projections.
  /// @note Charges cumulative native work before allocation without starting invocation or resetting budgets.
  /// Resource failures retain the shared first terminal error; unknown IDs leave a valid owner usable.
  std::expected<projection_definition, registration_error> resolve_projection_definition(std::string_view id);
  /// @brief Prepare already parsed options for an actual retained sealed external command, without executing Lua.
  /// @param name Exact registered external command name.
  /// @param options Borrowed parsed records; unknown, duplicate and inconsistent records refuse as usage.
  /// @return Owned typed options/presence, pending host slots and effective native history intent, or closed refusal.
  /// @note No paths are admitted, services called, invocation transition started or budget reset. Allocations charge the
  /// cumulative native metric; resource failures retain the first terminal diagnostic. Generic profile invoke stays closed.
  std::expected<extension_prepared_invocation, extension_preparation_error>
  prepare_extension_invocation(std::string_view name, std::span<const extension_parsed_option> options);
  /// @brief Execute an external shape in its retained VM with immutable normalized arguments.
  /// @param id Trusted selected shape ID.
  /// @param input Exact version/job/spec/assignments/reviewer normalized transport, already base-validated.
  /// @return Bound definition and one raw JSON-compatible result, or redacted native refusal.
  /// @note Admission protocol errors do not consume the owner. Execution failures are terminal; repeated successes share
  /// invocation instructions and cumulative conversion budgets. No services are exposed; Lua owns shape semantics.
  std::expected<shape_response, registration_error> invoke_shape(std::string_view id, const value_ptr& input);
  /// @brief Execute one registered projection on immutable trusted read data, without services.
  /// @param id Registered external projection ID.
  /// @param kind Exact authenticated input kind, matching the descriptor.
  /// @param input Immutable structurally valid record; caller guarantees authentic selected-role or verified-history data.
  /// @return Bound identity and owned admitted sections, or a redacted protocol/resource failure.
  /// @note This internal bridge does not authenticate read data. Execution failures are terminal and share all budgets.
  std::expected<projection_response, registration_error> invoke_projection(std::string_view id, projection_input_kind kind,
                                                                           const value_ptr& input);
  /// @brief Compose trusted semantic phases and registered shapes in this retained profile owner.
  /// @param validation Existing bundle/capture structural validation input; no context or callbacks are accepted.
  /// @return Ordinary semantic value/error envelope or a redacted terminal native failure.
  /// @note Repeated operations share cumulative invocation budgets and never access providers.
  std::expected<value_ptr, registration_error> validate_profile(const value_ptr& validation);
  /// @brief Validate original selection material and select over its authentic same-call semantic result.
  /// @param selection Existing bundle/capture/files/job/role request; no model, context or definitions are accepted.
  /// @return Ordinary selection value/error envelope or a terminal redacted native failure.
  /// @note Semantic and selection phases share cumulative budgets; every custom job runs once per operation.
  std::expected<value_ptr, registration_error> select_profile(const value_ptr& selection);
  /// @brief Normalize original render intent, validate all jobs once, select and render authentic same-call material.
  /// @param render Existing bundle/capture/files/job/role intent with optional byte budget; no model is accepted.
  /// @return Ordinary render value/error envelope or a terminal redacted native failure.
  /// @note All phases share cumulative budgets and immutable transport; no providers or public commands are opened.
  std::expected<value_ptr, registration_error> render_profile(const value_ptr& render);
  /// @brief Transfer the stable native owner and all retained references.
  /// @param other VM to consume.
  command_vm(command_vm&& other) noexcept;
  /// @brief Release this VM and transfer another stable owner.
  /// @param other VM to consume.
  /// @return This VM.
  command_vm& operator=(command_vm&& other) noexcept;
  /// @brief Release all remaining invocation state and Lua references.
  ~command_vm();
  /// @brief Return immutable native catalog data; no callable or mutable Lua table escapes.
  /// @return Exact command_catalog_v1 value, or null for a moved-from VM.
  value_ptr catalog() const;
  /// @brief Return the sealed catalog/owner/handler/bundle identity bytes.
  /// @return Canonical sealing bytes, empty for a moved-from VM.
  std::string_view seal() const;
  /// @brief Report whether export service success requires the publication-uncertain fallback after any later failure.
  /// @return True once native export reports successful publication, including malformed result or Lua exhaustion.
  bool export_may_be_complete() const;
  /// @brief Report authoritative native completion after any later handler or conversion failure.
  /// @return True for committed, replayed or uncertain completion; false before completion.
  bool load_may_be_complete() const;
  /// @brief Borrow the native completion output retained through terminal invocation and moves.
  /// @return Stable const receipt until this owner is destroyed or replaced; null for a moved-from VM.
  const ledger::completion_receipt* load_completion() const;
  /// @brief Execute the retained callable once with a copied typed intent and finite native provider.
  /// @param options ABI-validated parsed family intent, never argv or Lua source.
  /// @param services Invocation-local synchronous provider; retained only until this call returns.
  /// @return Exact ok/value or ok/error result, or sanitized bridge failure. Native service errors always win.
  std::expected<value_ptr, diagnostic> invoke(command_options options, command_services& services);

private:
  friend class extension_invocation;
  std::expected<projection_definition, registration_error>
  resolve_projection_definition_impl(std::string_view id, std::size_t original_index, extension_invocation* active = nullptr,
                                     registration_error* active_failure = nullptr);
  std::expected<shape_definition, registration_error> resolve_shape_definition_impl(std::string_view      id,
                                                                                    extension_invocation* active);
  std::expected<shape_response, registration_error>   invoke_shape_impl(std::string_view id, const value_ptr& input,
                                                                        extension_invocation* active);
  std::expected<value_ptr, registration_error> validate_profile_impl(const value_ptr& validation, const value_ptr& selection,
                                                                     const value_ptr&      render = {},
                                                                     extension_invocation* active = nullptr,
                                                                     value_ptr*            basis  = nullptr);
  static std::expected<command_vm, registration_error> create_impl(profile_snapshot profile);
  explicit command_vm(std::unique_ptr<implementation> owner);
  std::unique_ptr<implementation> _owner;
};
/// Original captured profile paired with its sealed VM and one private extension admission.
/// @note Thread-confined; retained read services keep the same original owner. Moving transfers all original receipts.
class extension_invocation {
public:
  /// @brief Register a VM from the original moved capture's immutable snapshot.
  /// @param capture Original trusted capture, consumed without recapture or receipt substitution.
  /// @return Paired registered owner or typed redacted failure; failed creation publishes no owner.
  static std::expected<extension_invocation, registration_error> create(profile_capture capture);
  /// @brief Borrow the sealed catalog for final native parsing.
  /// @return Canonical bytes, empty for failed or moved owners; no mutable descriptor escapes.
  std::string_view effective_catalog() const;
  /// @brief Prepare final parsed scalars and metadata-admit every supplied host slot once.
  /// @param name Actual sealed external command name.
  /// @param options Exact already parsed records; neither argv nor a mutable prepared invocation.
  /// @param explicit_cwd Caller-snapshotted cwd for relative paths; never reacquired internally.
  /// @param workspace_directory Caller-snapshotted workspace environment for bare names.
  /// @return Success only after original receipt verification; every refusal permanently consumes admission.
  std::expected<void, registration_error> admit(std::string_view name, std::span<const extension_parsed_option> options,
                                                std::optional<std::string_view> explicit_cwd,
                                                std::optional<std::string_view> workspace_directory = {});
  /// @brief Recheck original profile, module and late-slot receipts after successful admission.
  /// @return Success or first terminal failure; calling before admission refuses as protocol.
  std::expected<void, registration_error> verify();
  /// @brief Prepare and privately retain native history once from the original admitted intent and receipts.
  /// @return Success or original typed registration, format or history failure; refusal is terminal.
  /// @note No Lua runs. The entire read transaction, connection and descriptor adapter close before return.
  std::expected<void, std::variant<registration_error, ledger::format_error, history::history_read_error>> prepare_history();
  /// @brief Execute a retained pure, role or history handler once with immutable input and closed native context.
  /// @return Exact extension success or inner semantic error envelope; native failures poison the owner.
  /// @note Rechecks original receipts before execution and before releasing either response branch.
  std::expected<value_ptr, extension_execution_error> execute();
  /// @brief Transfer the complete owner; the source thereafter refuses.
  /// @param other Owner to consume.
  extension_invocation(extension_invocation&& other) noexcept;
  /// @brief Release this owner and transfer another; self-move preserves state.
  /// @param other Owner to consume.
  /// @return This owner.
  extension_invocation& operator=(extension_invocation&& other) noexcept;
  /// Release the VM and all original native receipts.
  ~extension_invocation();

private:
  struct implementation;
  explicit extension_invocation(std::unique_ptr<implementation> owner);
  std::expected<void, registration_error> fail(registration_error error);
  const value*                            prepare_role();
  std::unique_ptr<implementation>         _owner;
};
} // namespace planar::engine
