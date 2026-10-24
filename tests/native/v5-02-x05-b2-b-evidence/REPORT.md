# X05-B2-B codec reservation candidate

The candidate adds `codec_encode_reserved` as an optional synchronous native
reservation interface. Ordinary encode delegates to precisely the same validator,
copy builder and Glaze writer with a null hook; existing callers are unchanged.
Independent review is pending. No F dispatch or runtime integration is claimed.

The validator counts value visits, array entries, object entries, value strings,
key payload bytes, value payload bytes and minimum unescaped JSON bytes. Its
active-path array has 256 container slots: containers at depths 0..255 fit;
a scalar leaf at depth256 does not require another slot. Repeated aliases are
visited and copied repeatedly; only active ancestors constitute cycles. Integer
length uses stack `to_chars`, avoiding `to_string` allocation before the hook.

The checked reservation is:

- Graph: `sizeof(node) + array_entries*sizeof(node) + object_entries*sizeof(pair<string,node>)`.
  Node variant/container metadata and object-key string objects are already in
  these slots and are not counted again as separate graph allocations.
- Strings: key bytes + value bytes + `string_count*string_capacity_slack`.
  `string_capacity_slack=max(__STDCPP_DEFAULT_NEW_ALIGNMENT__,sizeof(void*))+2`
  follows pinned libc++ string's `__align_allocation_size`: round size+1 to that
  alignment, with at most two exceptional boundary bytes. This
  conservatively includes inline small-string payloads already in graph slots.
- Workspace: `sizeof(encoder) + 257*(2*sizeof(node)+sizeof(node::object))`.
  This explicitly conservative allowance covers result/return node slots and
  local container metadata per possible build depth. These overlap moved graph
  objects; they are logical capacity slots, not claims of independent heap copies,
  complete C++ stack size, allocator bookkeeping or process RSS.
- Output: `6*minimum_bytes+1024`, plus the same library string-capacity slack. The string is moved into
  the result, so the retained output is charged once, without an imaginary copy.

Each copied vector reserves its source entry count before insertion, preventing
geometric growth. Glaze writes into the existing fixed span. The successful
validation/reservation path allocates no heap memory before the hook; workspace
already lives on the native stack and is included in the reported charge. A hook
refusal is before graph/string/output construction. Returned hook diagnostics move
unchanged; bad_alloc/length_error map to existing output resource refusal and an
unexpected exception maps to redacted internal. This is a one-shot allocation
refusal boundary, not a persistent exhausted-host-heap guarantee.

The input must remain stable throughout the synchronous call, including the
trusted native hook. There is no detached plan, second policy walker, mutation
permission or protection against C++ code violating that precondition. A null
function pointer acts like ordinary encode even with a context pointer.

All formula operations use checked addition/multiplication. Existing hard caps
make metric overflow unreachable on supported 64-bit hosts: visits <=4,000,000;
each entry belongs to one nonroot visit, so entries <=3,999,999; each key/string
payload sum <=16,777,216 by the minimum-byte validation; copied string count is
<= visits+entries; output <=100,664,320. Graph slots have fixed compile-time
sizes and workspace is fixed. No larger-limit test knob is introduced.

Six added codec cases cover known metrics, determinism, exact native-budget fit,
one-byte-short refusal, typed diagnostics, null hooks, callback exception types,
validation precedence, 256-container scalar-leaf and 257-container refusal,
repeated aliases, complete 16MiB/+1 transport and control-character expansion.
The existing four-million-visit test now exercises the reserved API and proves
4,000,001 refuses without a second hook call. Existing byte vectors stay exact.
Transport success does not establish shape semantic acceptance: valid shapes
cannot approach that transport ceiling under their stricter semantic limits.

`first-build.log` preserves the initial test-only compile error (expected values
compared without an error equality operator); the test now checks success and
compares contained strings. `second-build.log` passes. `first-codec.log/xml`
records 46 codec-name matches with zero failures, errors or skips. The independent
`first-probes/phase.xml` records two production-codec allocation observer cases:
zero requests before the hook, eight after admission in the mixed repeated
record fixture, zero total requests on typed hook refusal, and no prehook
allocation for the 256-container/int32-min path. This is native allocation phase
observation, not a global allocator product policy or RSS measurement.

The first frozen gate was deliberately interrupted during focused CTest after
inspection found that string payload+terminator alone omitted pinned libc++
capacity rounding. Its complete logs/freeze are preserved in `superseded-gate-01/`;
no test failure was its trigger. Source edits began only after its process ended.
The candidate now reserves alignment/exception slack for every copied key/value
string and the output string. This is capacity slack, without charging the
embedded string object again. Final observers also compare actual aggregate
request bytes and largest request against total and output storage allowances.
Pinned header evidence is in `libcxx-string-capacity.txt`.

The final serialized focused Debug gate and exact source freeze are in
`final-gate/`; all failed outputs are preserved. It builds all+planar_tests,
checks the 612 inventory against all prior606 and582 names, exercises codec and
current runtime/CLI/history/ledger/hash/canonical callers, verifies vectors and
reversible source audits, runs pinned formatting/Doxygen, and requires a no-work
build. This is a focused foundation gate, not the full B milestone or release gate.
B2-C must separately charge raw converted input and compose this API with retained
F dispatch and immutable inputs. Profile CLI, X06–X08, execution/RA01 and Linux
remain open; public init creates format1.

Final result: the frozen gate passes all 346 selected cases, zero failures/errors/
skips/disabled; the separate serialized direct ledger-history-codec selection
passes 11/11 with the same zero counts. All 612 inventory names preserve the
prior606 (and prior582). The final two allocation observer cases pass, including
aggregate requested-byte and largest-request bounds. All four canonical/source
vector checks, six reversible source audits, pinned formatting/Doxygen and both
no-work builds pass. `final-gate/source-stability.json` and
`handoff-source-stability.json` prove unchanged frozen source through the gate,
supplement and handoff. `candidate-manifest.json`, `candidate.diff` and its SHA256
identify the scoped candidate; `artifact-manifest.json` hashes the evidence.

This candidate is ready for independent review; it is not self-approved. No
commit, install, publish, merge, profile invocation or full B milestone gate was
performed. The Debug build-directory ownership is released after final evidence
and hash finalization; no later edits by this coder are intended.
