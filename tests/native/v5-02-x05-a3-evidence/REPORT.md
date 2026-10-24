# X05-A3 captured ordered registration candidate

Status: candidate pending fresh independent review; no self-approval. Shared
checkout `dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The coordinator owns the round.
Existing dirty sources, old tests, shipped Lua, vendor and historical evidence
were preserved. No legacy Planar tools, roles, skills, queue, installed binaries,
operator data, commit, install, publish or merge were used.

## Production behavior and ownership

`command_vm::create(profile_snapshot)` returns either a complete registration-only
VM or `registration_error { diagnostic detail; std::string field; }`. Embedded
`create()` keeps its existing diagnostic API. Before external evaluation, runtime
redecodes the bounded profile, checks exact root/path/mode and ordered module
correspondence, 1 MiB/module and 8 MiB aggregate, source text and source-only
admission. It copies the complete validated closure into private immutable
ownership, severing caller aliases. The caller must keep its input stable during
construction and retain/reverify physical capture receipts when later admission
or acceptance requires it. No filesystem reload or receipt substitution occurs.
The real-capture test rewrites then removes the selected module after capture;
both VMs still register original bytes and produce identical effective digests,
while capture verification refuses changed receipts.

Native setup constructs external `_ENV` before evaluating embedded source. It
copies the pristine curated global/library references, replaces `_G` with itself,
engine with a separate constructor-only kind/object/array table, and registration
with exactly three native add/shape/projection functions. Explicit fixed native
sites enroll these exact API identities before any source executes; native
snapshot never traverses opaque C upvalues. The shipped bundle executes first,
then receives/enrolls its five original engine helpers without intervening source.
Original globals, all shipped handlers and the safe external environment audit
and freeze together. No global assignment or raw enumeration of frozen globals
is used to construct the external environment. External closures have no original
`_G` or hash/JSON engine escape; embedded handlers keep their original engine.

External modules load from private captured bytes as source under native logical
`external/<listed path>` names, in one VM and declared order. Native checked
`lua_setupvalue` installs the safe environment. `LUA_MULTRET` preserves actual
return count; even `return nil` refuses. No read service or supplied input bytes
are present. Native active owner ordinals assign every registration, including
ordinary saved aliases/pcall during active initialization. There is no invented
requirement that F's source equal its owner or that the immediate caller be Lua.
Saved callbacks fail inactive registration after module execution.

Each API bounds attempts/retention before allocation, then retains strong Lua D/F
references in native state. Failure is sticky even if Lua catches it. At module
end, final D is converted and copied, options/capabilities normalize, the complete
partial catalog revalidates through pure catalog-v2 policy, and exact D/F roots
audit/seal before the next source chunk. A2's successful identity set, backing
map, stable iterator and terminal sealing failure behavior remain in force.
Descriptor initialization may finish before this boundary. Private dispatch proves
retained F reads the final D and prior aliases after GC, and mutation still fails.
The three namespaces, all reserved shipped identities, exact schemas, native
pairings, external capabilities, help and count limits use the existing pure
validator; no dummy shipped shape/projection adapters were added.

Pure validator diagnostics in canonical order map back to original native
registration order. Profile decoder fields remain exact. Runtime phase errors use
native profile/module/catalog locations, never host selectors, source, traceback
or digest. Non-allocation unexpected exceptions map internal; `std::bad_alloc`
maps resource_limit. The controlled exception probes cover constructor and new
callback paths independently, with a red variant reverting only the constructor
catch-all policy and failing the same classification detector.

After registration, original embedded provenance plus exact captured profile,
ordered module bytes/modes and canonical catalog feed unchanged pure effective
preparation. The owner retains canonical catalog bytes, effective digest and the
required complete immutable provenance; it does not retain another duplicate old
tree. Profile-owner `invoke` explicitly refuses before typed services/provider
work. Old embedded create/catalog/help/packets/invocation retain their behavior.
Real B/C shape/projection semantics and adapters remain open.

## Resource accounting

Registration keeps one 64 MiB live Lua allocator and one cumulative 10-million
instruction counter for shipped and every external module. Stack2048,
graph-depth256, four-million converted nodes and existing checked cumulative
512 MiB native conversion accounting remain. No stage resets, new scan quota,
public 4 MiB arena or blanket native identity policy were introduced. The
single-chunk instruction control succeeds while three identical chunks exhaust
at module3; caught Lua allocation/instruction errors and the 65th shape attempt
remain terminal. A1/A2's actual protected-frame stack and PMR invariants remain
covered by the focused old tests; this report does not claim another independently
injected stack-allocation boundary experiment.

Native conversion allocations and guard PMR requests continue using existing
checked charges with no refund. Pure helper code uses ordinary standard-library
allocation, so A1's PMR adapter alone does not account for registration integration.
The runtime adds pre-entry conservative **logical capacity reservations**, charged
cumulatively to the same existing native ceiling; these are not allocator hooks,
RSS measurements or a claim to count allocator headers/rounded capacities:

- Catalog composition reserves 4096 bytes per original/retained entry plus a root
  for wrapper/owner/maps/list sorting scratch before composition. Normal structural
  `validate()` charges the complete composed graph before pure validation, covering
  decoder/checker graph work under the existing per-node/string/map policy. Three
  bounded 1 MiB canonical buffer slots cover encode result, validated owned bytes
  and the runtime's retained bytes. Each boundary pays again, without refund.
- Provenance prepares four embedded tree digests: initial `prepare_stdlib`;
  `verify_stdlib`'s initial digest; its nested `prepare_stdlib`; final effective
  digest. Each tree digest constructs a file record and an encoded record buffer;
  three owned trees copy payloads. Thus embedded payload capacity pays
  `4*2+3=11` passes. External/profile payloads pay final digest plus output ownership,
  `1*2+1=3` passes. Multiplications occur after explicit 16 MiB embedded and
  captured 1 MiB/8 MiB/profile bounds; additions go through checked `charge()`.
- Each applicable tree pass reserves 2048 bytes per upper-bounded path component,
  plus eight copies of prefixed path capacity, for tree nodes/vector growth,
  child maps, folded/name strings, directory-record children and encoded children.
  An extra4096/file covers views/order metadata and manifest nodes. The complete
  catalog receives structural charges again for effective decode/manifest work.
  Four bounded 1 MiB canonical slots cover effective/catalog/seal buffers and the
  small immutable shipped manifest passes; current shipped order is fixed ten
  files with short paths. Future changes to pure allocation recipes, pinned
  libraries or shipped metadata size require revalidating these reservations.

Recipes are visible at runtime.cpp:1893,2042; pure catalog encoding/decoding at
history/effective_catalog.cpp:311–331; old prepare/verify ownership at
history/provenance_codec.cpp:55–64,71–157; effective decode/encode/verify/digest/own
at history/effective_provenance.cpp:163–183; file-record and directory construction
at history/tree_core.cpp:127–207 and encoded record hashing at history/record.cpp:256.
Fixed C++ owner fields, stack frames, allocator bookkeeping and library internal
capacity overhead remain outside the charged logical-conversion metric, as in
existing conversion accounting. Pure helpers still enforce their own byte/node/
file/path bounds and propagate allocation refusal. The private native-exhaustion
probe reaches the real guard/conversion refusal under the unchanged512MiB cap.

## Verification and preserved development results

Verification result and final identities are recorded below after frozen checks.
The full source map excludes only root `.git`, root `build` and this A3 evidence
root; modes and symlink targets are included. Final source snapshots include all
current docs and support. No expensive full-tree hashing is claimed per command;
source freezes before the serialized gate and rehashes after it and at release.

All development outputs remain under this evidence root. Initial compilation
failed an incorrectly ordered tree_file_view initializer; the subsequent builds
pass. Four observed targeted fixture failures were corrected without weakening
product detectors: wrong sorted command index; reserved with-profile command
combined with an independently malformed dotted handler; macOS `/tmp` symlink
instead of canonical capture selector; an instruction fixture whose three loops
were collectively below the limit. The final fixtures use the intended isolated
conditions. The initial catch-all native exception classification was corrected
on coordinator source review and then demonstrated with controlled green/red
production copies. Eight final private scenarios include actual retained dispatch
and pure historical verification, inactive saved callback, same-pointer unknown
native identity, native exhaustion, constructor unexpected/bad_alloc and callback
unexpected/bad_alloc. Cleanup probes check that real Lua destruction releases all
tracked live Lua allocation on success and each refusal.

No full570 suite, RelWithDebInfo/release or Linux gate is claimed. Latest full gate
remains520 per configuration. A3 remains a candidate pending fresh independent
review. Public profile CLI, B/C semantics/adapters, X06 interpretation/profile
history acceptance, whole-invocation caller receipt orchestration, public cutover,
execution/RA01 and Linux remain open. Public init creates format1.

## Frozen gate result and candidate identity

[Exact commands](attempt01/commands.json) complete all15 checks, each exit0.
The serialized Debug all+planar_tests builds pass. Inventory is570 unique names,
preserving all555 starting names. The broad focused selector executes
**197/197**, zero failures/errors/skips/disabled cases, **87.18 seconds**:
15 A3 cases plus182 prior command/runtime/guard/CLI/acceptance/profile/effective
cases. [JUnit](attempt01/focused.xml), [raw log](attempt01/focused.log),
[matched names and statuses](attempt01/test-summary.json). This is a focused
scope gate, not a full570 run. Embedded packet/golden/CLI tests remain unchanged.

Eight [private production scenarios](attempt01-production-probes/commands.json)
and the catch-all red control pass. The successful fixture retains three real
callables after GC and reports18374200 cumulative charged native bytes and3000
executed instructions; no reset is involved. Its direct unknown-C-root refusal
is supplemented with a distinct **Lua closure retaining** an exact unknown native
identity with the same C pointer as enrolled digest callbacks. The
[supplemental runner](unknown-reachable-probe.py) differs from frozen support only
by that private fixture transformation, with exact reverse equality in
[delta](unknown-reachable-delta.json); all eight scenarios and red control rerun
successfully in [supplemental evidence](supplemental-reachable-native/commands.json).
The reachable-native refusal therefore does not rely only on the direct-C-root
prohibition. Candidate product/docs/tests/support did not change for this probe.

Canonical, record-v1, request-v2 and effective-provenance vectors pass. Historical
B2/D6/D6-C01/D7-B exact source audits and A0 runtime/interface forward/reverse and
shipped-source audits pass. Pinned formatting/Doxygen checks117 active sources.
Final no-change build reports `ninja: no work to do.`. Source remains byte/mode/
symlink-identical before/after gate and at release. Scope audit confirms old tests,
shipped Lua, historical support/evidence and vendor were untouched. AGENTS.md is
still the CLAUDE.md symlink.

- Source-freeze SHA256: `56cdefe4bae53ebccf8b3e28b436592acb15ea08aa16c5c5cadf7da125087130`.
- Exact owned.diff SHA256: `0fad899cfdac93df1fa6438c9ecd6d63138af2fdfb66252d20360d30327bfe6b`.
- Build identity manifest SHA256: `0d8d0ff27c4216cc4ceaa5cdf9884bf678371a2d7eff9c25bbb478d186e3c313`.

[Finalization](finalization.json) binds this completed report, source/diff/build
identities and the complete artifact manifest without a circular report hash.
Serialized build ownership is released only after final source/artifact hashing;
no subsequent candidate or evidence edits follow release.
