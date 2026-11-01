/// @file
/// @brief Bounded fresh exports and sealed invocation-local command VMs with typed native services and redacted failures;
/// also the profile_invocation owner (open_known_load/open_opaque capture and register once; prepare_load consumes a
/// known-load owner into an opaque move-only prepared_profile after bound capture, route, seal, all-job validation,
/// native correspondence and v2 interpretation derivation inside the one private active frame; prepared_profile exposes
/// only immutable profile_acceptance_data, native receipt verification and counters; begin_frame is private;
/// into_extension adopts opaque owners and refuses known-load owners; observe).
export module planar.engine.runtime;
import std;
export import planar.engine.value;
export import planar.engine.capture.profile;
export import planar.engine.history.provenance_codec;
export import planar.engine.history.interpretation_v2_codec;
export import planar.engine.ledger.completion;
export import planar.engine.history.read;
import planar.engine.acceptance.intent;
import planar.engine.acceptance.source_capture;
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
#ifdef PLANAR_AUTHORING_BUNDLE
/// The six embedded-only authoring verbs; exists only in the authoring stack and its variants.
enum class command_authoring_verb { spec_prompt, critique, ingest, explain_rejection, observe, orchestrator };
/// Embedded-only authoring intent, deliberately outside `command_options` (which stays closed at five alternatives).
/// Path members are private normalized host spellings; unused members stay empty.
struct command_authoring_options {
  command_authoring_verb verb = command_authoring_verb::spec_prompt; ///< Selected authoring verb.
  std::string            workspace;                                  ///< Normalized workspace path or empty.
  std::string            registry;                                   ///< Normalized registry path or empty.
  std::string            diagnostic;                                 ///< Normalized diagnostic file path or empty.
  std::string            from;                                       ///< Normalized observation directory or empty.
  std::string            job;                                        ///< Selected job ID or empty.
};
#endif
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
#ifdef PLANAR_AUTHORING_BUNDLE
  /// @brief Open the private authoring workspace, synchronously and once.
  /// @param options Retained authoring intent; no Lua-selected host path.
  /// @return Exact {file,workspace} or typed failure. Default refuses unsupported authoring.
  virtual command_service_result open_workspace(const command_authoring_options& options);
  /// @brief Capture the declaration routes an authoring prompt needs.
  /// @param options Retained authoring intent.
  /// @param route Bounded copied route from this VM.
  /// @param include Capture scope, `workspace` or `spec`; must match the verb.
  /// @return Authoring capture or typed failure. Default refuses unsupported authoring.
  virtual command_service_result capture_authoring(const command_authoring_options& options, const value_ptr& route,
                                                   std::string_view include);
  /// @brief Capture the declaration routes and the selected job for the orchestrator.
  /// @param options Retained authoring intent.
  /// @param route Bounded copied route from this VM.
  /// @param job Selected job ID.
  /// @return Orchestrator capture or typed failure. Default refuses unsupported authoring.
  virtual command_service_result capture_orchestrator(const command_authoring_options& options, const value_ptr& route,
                                                      std::string_view job);
  /// @brief Read the bounded diagnostic file named by the authoring intent.
  /// @param options Retained authoring intent.
  /// @return Diagnostic bytes facts or typed failure. Default refuses unsupported authoring.
  virtual command_service_result read_diagnostic(const command_authoring_options& options);
  /// @brief Read the observation directory named by the authoring intent.
  /// @param options Retained authoring intent.
  /// @return Observation directory facts or typed failure. Default refuses unsupported authoring.
  virtual command_service_result read_observation_directory(const command_authoring_options& options);
#endif
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
class profile_invocation;   ///< Original profile owner before final parsing, defined below.
class profile_load_stage;   ///< Private internal step of prepare_load, defined below.
class prepared_profile;     ///< Opaque published product of prepare_load, defined below.
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
#ifdef PLANAR_AUTHORING_BUNDLE
  /// @brief Execute one embedded-only authoring verb once with a copied intent and finite native provider.
  /// @param options Authoring intent; never argv or Lua source.
  /// @param services Invocation-local synchronous provider; retained only until this call returns.
  /// @return Exact ok/value or ok/error result, or sanitized bridge failure.
  std::expected<value_ptr, diagnostic> invoke_authoring(command_authoring_options options, command_services& services);
#endif

private:
  friend class extension_invocation;
  friend class profile_invocation;
  friend class profile_load_stage;
  /// Private active-frame token: one extension owner or one profile owner, never public and never both.
  struct active_frame {
    extension_invocation* extension;     ///< Original extension owner whose bridge checks apply.
    const void*           profile_owner; ///< Identity of the profile_invocation that began the frame.
    active_frame() noexcept : extension(nullptr), profile_owner(nullptr) {
    }
    /// @brief Build an extension frame; a null pointer is the public no-frame state.
    /// @param owner Extension owner or null.
    active_frame(extension_invocation* owner) noexcept : extension(owner), profile_owner(nullptr) {
    }
    /// @brief Report whether this frame runs inside an already begun invocation.
    /// @return True for either an extension or a profile owner frame.
    bool nested() const noexcept {
      return extension != nullptr || profile_owner != nullptr;
    }
  };
  std::expected<projection_definition, registration_error>
  resolve_projection_definition_impl(std::string_view id, std::size_t original_index, const active_frame& frame = {},
                                     registration_error* active_failure = nullptr);
  std::expected<shape_definition, registration_error> resolve_shape_definition_impl(std::string_view    id,
                                                                                    const active_frame& frame);
  std::expected<shape_response, registration_error>   invoke_shape_impl(std::string_view id, const value_ptr& input,
                                                                        const active_frame& frame);
  std::expected<value_ptr, registration_error> validate_profile_impl(const value_ptr& validation, const value_ptr& selection,
                                                                     const value_ptr& render = {}, const active_frame& frame = {},
                                                                     value_ptr* basis = nullptr);
  std::expected<value_ptr, registration_error> validate_profile_phases(value_ptr validation, value_ptr selection,
                                                                       const value_ptr& render, const active_frame& frame,
                                                                       value_ptr* basis);
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
  friend class profile_invocation;
  explicit extension_invocation(std::unique_ptr<implementation> owner);
  explicit extension_invocation(profile_capture&& capture, command_vm&& vm);
  static std::expected<command_vm, registration_error> register_vm(const profile_capture& capture);
  std::expected<void, registration_error>              fail(registration_error error);
  const value*                                         prepare_role();
  std::unique_ptr<implementation>                      _owner;
};
/// Observational counters of a profile owner; carries no authority and no mutable VM access.
struct profile_invocation_observation {
  bool        live            = false; ///< True for an owner that has not been moved from, failed or consumed.
  bool        consumed        = false; ///< True once into_extension consumed the owner.
  bool        known_load      = false; ///< True only for open_known_load; false for open_opaque and moved-from owners.
  bool        frame_begun     = false; ///< True once prepare_load started the single active invocation frame (private start).
  std::size_t instructions    = 0;     ///< Cumulative VM instructions (state::instructions).
  std::size_t native_bytes    = 0;     ///< Cumulative charged native bytes (state::native_bytes).
  std::size_t owner_charge    = 0;     ///< Native bytes the factory charged for this owner's own allocation (0 when moved from).
  std::size_t registrations   = 0; ///< Cumulative register_vm calls made for this owner's original capture (1 after a successful
                                   ///< factory; unchanged across into_extension).
  std::size_t tag_entries     = 0; ///< Observation only: entries currently in the VM's registry tag table (0 when no VM).
  std::size_t backing_entries = 0; ///< Observation only: entries currently in the VM's registry backing table (0 when no VM).
  std::size_t temporary_entries =
      0; ///< Observation only: tag and backing entries absent from the shape baseline (0 without one).
};
/// Failure of prepare_load. Either a native or protocol `failure`, or an ordinary trusted semantic refusal in `semantic`.
struct profile_preparation_error {
  registration_error failure;  ///< Native or protocol failure; meaningful when `semantic` is null.
  value_ptr          semantic; ///< Inner E of a trusted {error=E} refusal (route or job validation); null for native failure.
  bool cleanup_ran = false;    ///< Observation only: true once the frame cleanup ran (false when refused before the frame began).
  std::size_t peak_temporary_entries = 0; ///< Observation only: temporary registry entries present just before the release.
  std::size_t temporary_entries_after_release = 0; ///< Observation only: temporary registry entries remaining after the release.
};
/// Observational counters of a prepared stage; carries no authority and no access to its private state.
struct profile_load_stage_observation {
  bool        live             = false; ///< True while the stage owns its state (false once moved-from).
  bool        frame_begun      = false; ///< True once the single frame began.
  bool        sealed           = false; ///< True once bytes and topology were sealed into the original capture.
  bool        bundle_retained  = false; ///< True while the normalized bundle B is retained privately.
  bool        model_retained   = false; ///< True while the finalized model S is retained privately.
  std::size_t jobs             = 0;     ///< Number of jobs in S.
  std::size_t shape_dispatches = 0;     ///< Cumulative custom shape F calls (one per custom job).
  std::size_t instructions     = 0;     ///< Cumulative VM instructions of the original frame.
  std::size_t native_bytes     = 0;     ///< Cumulative charged native bytes of the original frame.
  std::size_t registrations    = 0;     ///< Registration count of the original capture (1).
  std::size_t tag_entries      = 0;     ///< Observation only: entries currently in the VM's registry tag table.
  std::size_t backing_entries  = 0;     ///< Observation only: entries currently in the VM's registry backing table.
  std::size_t temporary_entries =
      0; ///< Observation only: tag and backing entries absent from the shape baseline (0 without one).
  bool        cleanup_ran            = false; ///< Observation only: true once the frame cleanup ran.
  std::size_t peak_temporary_entries = 0;     ///< Observation only: temporary registry entries present just before the release.
  /// Observation only: true when the explicit shape baseline already existed just before the route call, so every nested
  /// call's temporaries lie outside it.
  bool baseline_before_route = false;
  /// Observation only: cumulative instruction counter at four points of prepare_load: after the frame began, after the route
  /// call, just before final validation (after its input was built) and after validation returned.
  std::array<std::size_t, 4> instruction_marks{};
  /// Observation only: cumulative native byte counter at the same four points as instruction_marks.
  std::array<std::size_t, 4> native_byte_marks{};
};
/// Original profile capture paired with its sealed VM before final command parsing.
/// Opaque and move-only: copying is deleted, there is no public default or pointer constructor, and moved-from
/// operations refuse with a typed protocol diagnostic at field profile. Moving transfers the heap implementation, so
/// addresses (including borrowed catalog bytes) and counters survive. Self-move preserves state.
/// The known-load factory alone records the supplied workspace root, declaration, registry and ledger admissions; an opaque
/// owner can never prepare built-in load, and there is intentionally no upgrade API.
/// into_extension adopts opaque owners only and refuses known-load owners.
/// @note Thread-confined. Each factory captures the profile once and runs the shipped plus external registration chunks
/// exactly once; into_extension never recaptures or re-registers, and no handler or command code runs.
class profile_invocation {
public:
  /// @brief Capture the selected profile once, metadata-admit the supplied slots, register once and reverify.
  /// @param request_intent Codec2 load intent carrying the profile selector, workspace and registry arguments.
  /// @param ledger_path Native-only ledger selection used for alias admission; never added to request framing.
  /// @return Owner with original receipts retained, or a typed failure with no partial owner; no source content is read.
  static std::expected<profile_invocation, registration_error> open_known_load(const acceptance::request_intent& request_intent,
                                                                               std::string_view                  ledger_path);
  /// @brief Capture and register a selected profile with no supplied-slot knowledge.
  /// @param profile_selector Absolute profile file selector.
  /// @return Owner whose known_load is false, or a typed failure with no partial owner.
  static std::expected<profile_invocation, registration_error> open_opaque(std::string_view profile_selector);
  /// @brief Closed factory for render, init, history and export: capture, metadata-admit the family's slots, register once.
  /// @param profile_selector Absolute normalized profile selector.
  /// @param family Exactly the parsed family options; a load alternative is refused before any capture.
  /// @return Owner retaining the admitted options, or a typed failure with no partial owner.
  static std::expected<profile_invocation, registration_error> open_with_profile(std::string_view       profile_selector,
                                                                                 const command_options& family);
  /// @brief Closed factory for load: the intent overload that forwards to the known-load path.
  /// @param request_intent Codec2 load intent carrying the profile selector, workspace and registry arguments.
  /// @param ledger_path Native-only ledger selection used for alias admission.
  /// @return Known-load owner or a typed failure with no partial owner.
  static std::expected<profile_invocation, registration_error> open_with_profile(const acceptance::request_intent& request_intent,
                                                                                 std::string_view                  ledger_path);
  /// @brief Dispatch the admitted shipped family once on the retained VM and consume this owner.
  /// @param options Must equal the options retained at admission exactly; otherwise refuses with the protocol diagnostic.
  /// @param services Invocation-local native services, called synchronously.
  /// @return The retained callable's response record, or a closed diagnostic; refusal of unequal or non-admitted options
  /// leaves the owner unconsumed.
  /// @note Thread-confined. The exported command_vm::invoke keeps refusing profile owners.
  std::expected<value_ptr, diagnostic> run_known(command_options options, command_services& services) &&;
  /// @brief Return the immutable sealed catalog metadata.
  /// @return Exact catalog value, or null when moved-from, consumed or failed.
  value_ptr catalog() const;
  /// @brief Borrow canonical catalog-v2 bytes for final parsing.
  /// @return Stable bytes (addresses survive moves), empty when moved-from, consumed or failed.
  std::string_view effective_catalog() const;
  /// @brief Consume a known-load owner: bind the final intent, capture, route, seal and validate every job in one frame.
  /// Runs phases 1 to 3 of the preparation architecture. The owner is consumed (moved-from) before any fallible work, so a
  /// refusal or failure never allows a retry and nothing is published on failure. The final intent must equal the bound
  /// intent in request id, expected head, encoded bytes and the workspace, registry and profile selectors. Original receipts
  /// are reverified before capture and after staging. Exactly one frame begins; instruction and native counters are
  /// cumulative and never reset. No selection, render, projection or history service runs.
  /// @param final_intent Final parsed load intent; no caller bundle, model or capture is accepted anywhere.
  /// Runs phases 4 to 6 as well: native correspondence, effective tree reassembly, v2 interpretation encoding and source
  /// tree derivation, all charged to the same frame counters; the original receipts are reverified after derivation and the
  /// owner is marked consumed before any fallible outward copy. Nothing from Lua or a caller supplies a digest or record.
  /// @return Opaque prepared profile holding the original receipts and dormant VM privately plus immutable acceptance data,
  /// or a typed error whose `semantic` is non-null only for an ordinary trusted semantic refusal (never for a native failure).
  /// @note Thread-confined. Opaque, consumed, failed and moved-from owners refuse with a protocol error at field profile.
  std::expected<prepared_profile, profile_preparation_error> prepare_load(const acceptance::request_intent& final_intent) &&;
  /// @brief Transfer the original capture and VM into an extension owner without recapture or re-registration.
  /// Known-load owners are refused (adoption cannot change mode; known load goes through prepare_load and X07 extension
  /// dispatch uses opaque owners only); the refusal consumes nothing and leaves the owner usable.
  /// @return Extension owner; refuses on moved-from, consumed, failed, frame-begun or known-load owners and publishes nothing.
  std::expected<extension_invocation, registration_error> into_extension() &&;
  /// @brief Read counters without authority.
  /// @return Snapshot; zeroed with live false when moved-from.
  profile_invocation_observation observe() const noexcept;
  profile_invocation(const profile_invocation&)            = delete;
  profile_invocation& operator=(const profile_invocation&) = delete;
  /// @brief Transfer the stable owner; the source thereafter refuses.
  /// @param other Owner to consume.
  profile_invocation(profile_invocation&& other) noexcept;
  /// @brief Release this owner and transfer another; self-move preserves state.
  /// @param other Owner to consume.
  /// @return This owner.
  profile_invocation& operator=(profile_invocation&& other) noexcept;
  /// Release the VM and all original native receipts.
  ~profile_invocation();

private:
  struct implementation;
  friend class profile_load_stage;
  friend class prepared_profile;
  explicit profile_invocation(std::unique_ptr<implementation> owner);
  static std::expected<void, registration_error> begin_frame(implementation& owner);
  static std::expected<void, registration_error> fail(implementation& owner, registration_error error);
  /// Capture, metadata-admit the family's slots, register once and reverify; reached only by the closed factory.
  static std::expected<profile_invocation, registration_error> open_admitted(std::string_view       profile_selector,
                                                                             const command_options& family);
  std::unique_ptr<implementation>                              _owner;
};
/// Private product of prepare_load: the consumed owner, sealed source capture, normalized bundle and finalized model.
/// Move-only with no default or public constructor; the only public member is observe(). There is intentionally no accessor
/// for the bundle, model, VM, capture or receipts, and no route to any transaction API.
/// @note Thread-confined; destruction releases the VM and all native receipts.
class profile_load_stage {
public:
  /// @brief Read counters without authority.
  /// @return Snapshot; zeroed with live false when moved-from.
  profile_load_stage_observation observe() const noexcept;
  profile_load_stage(const profile_load_stage&)            = delete;
  profile_load_stage& operator=(const profile_load_stage&) = delete;
  /// @brief Transfer the stable stage; the source thereafter reports not live.
  /// @param other Stage to consume.
  profile_load_stage(profile_load_stage&& other) noexcept;
  /// @brief Release this stage and transfer another; self-move preserves state.
  /// @param other Stage to consume.
  /// @return This stage.
  profile_load_stage& operator=(profile_load_stage&& other) noexcept;
  /// Release the VM and all native receipts.
  ~profile_load_stage();

private:
  struct implementation;
  friend class profile_invocation;
  friend class prepared_profile;
  explicit profile_load_stage(std::unique_ptr<implementation> state);
  std::unique_ptr<implementation> _impl;
};
/// Immutable values derived natively by prepare_load; every field is an owned value with no pointer into the VM, the
/// normalized bundle, the model or any capture.
struct profile_acceptance_data {
  std::vector<history::tree_file>     source;         ///< Selected workspace declaration closure, sorted by portable path.
  history::record_digest              source_digest;  ///< Pure source tree identity of `source`.
  history::prepared_provenance        registry;       ///< Exactly the captured registry input.json provenance.
  history::prepared_provenance        effective;      ///< Verified raw effective stdlib tree and its digest.
  history::recorded_interpretation_v2 interpretation; ///< Encoded and versions bytes with decoded values (each at most 16 MiB).
  std::string                         workspace_id;   ///< Workspace identity from the derived interpretation structure.
  std::shared_ptr<const acceptance::request_data> bound_intent; ///< Exact intent bound when the owner was opened.
};
/// Opaque published product of prepare_load: the consumed owner (original receipts, dormant VM and counters), the sealed
/// source capture and immutable derived acceptance data.
/// Move-only with no default or public constructor; moved-from operations refuse with a typed protocol failure. Nothing
/// accepts a prepared_profile or profile_acceptance_data as construction authority, and the dormant VM is never callable.
/// @note Thread-confined; destruction releases the VM and all native receipts.
class prepared_profile {
public:
  /// @brief Retrieve the immutable derived values; observational and never grants construction authority.
  /// @return Shared immutable data, or a protocol failure at field profile when moved-from.
  std::expected<std::shared_ptr<const profile_acceptance_data>, registration_error> data() const;
  /// @brief Retrieve the exact bound intent without touching the VM.
  /// @return Shared immutable intent, or null when moved-from.
  std::shared_ptr<const acceptance::request_data> bound_intent() const;
  /// @brief Repeat the original native receipt checks (source capture then profile capture) at the precommit boundary.
  /// Runs no Lua, admits nothing, retries nothing and allocates no new receipts.
  /// @return Success or the first failure; moved-from owners refuse as capture_changed.
  std::expected<void, diagnostic> verify_native_receipts() const;
  /// @brief Read counters of the retained stage without authority.
  /// @return Snapshot; zeroed with live false when moved-from.
  profile_load_stage_observation observe() const noexcept;
  prepared_profile(const prepared_profile&)            = delete;
  prepared_profile& operator=(const prepared_profile&) = delete;
  /// @brief Transfer the stable prepared profile; the source thereafter refuses.
  /// @param other Prepared profile to consume.
  prepared_profile(prepared_profile&& other) noexcept;
  /// @brief Release this prepared profile and transfer another; self-move preserves state.
  /// @param other Prepared profile to consume.
  /// @return This prepared profile.
  prepared_profile& operator=(prepared_profile&& other) noexcept;
  /// Release the VM and all native receipts.
  ~prepared_profile();

private:
  struct implementation;
  friend class profile_invocation;
  explicit prepared_profile(std::unique_ptr<implementation> state);
  std::unique_ptr<implementation> _impl;
};
} // namespace planar::engine
