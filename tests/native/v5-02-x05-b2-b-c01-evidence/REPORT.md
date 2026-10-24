# X05-B2-B-C01 encode diagnostic construction correction

Candidate pending fresh independent combined B2-B+C01 review. This focused
correction addresses B2B-R1; it does not approve itself or close B2/B.

The shared checkout is `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Intentional rewrite changes and
others' work were preserved. Only `src/engine/json_codec/codec.cpp`, new C01
support/evidence and the six assigned current status documents changed.
`codec.t.cpp` and the interface remain byte-identical to the B2-B candidate.
The private native probes provide meaningful actual-allocation coverage without
adding duplicate registered tests or product allocation hooks. No Planar tools,
legacy roles/skills/queue, commits, installed binary, operator data, installation,
publication or merge were used. This coder held exclusive serialized Debug build
ownership until finalization and release.

The existing shared encode implementation now encloses its original work and
exception handlers in an outer allocation boundary. Normal refusal diagnostics,
callback `bad_alloc`/`length_error` adaptation and unexpected callback exception
redaction retain their existing messages and precedence. If constructing any of
those diagnostics throws `bad_alloc` or `length_error`, the outer boundary returns
the existing `resource_limit` / `output_bytes` diagnostic using the effective byte
limit. Ordinary encode already delegates to the shared reserved implementation,
so it receives the same protection. Typed callback refusals still move directly,
including their original long message, limit and maximum. No encoding,
reservation formula, capacity, validation order, API, decode, runtime, Lua,
serializer, dependency, storage or limit policy changed.

The final fallback diagnostic may itself allocate. The demonstrated guarantee is
controlled one-shot allocation refusal with later requests permitted, not a
persistent globally exhausted heap guarantee. Nothing here proves shape semantic
acceptance, retained F dispatch, immutable F input or whole-invocation accounting.

## Red and green evidence

`adversarial.cpp` is byte-identical to the independent B2-B review source, and
the original review artifacts remain immutable. Before rebuilding the correction,
`red/adversarial.xml` reproduces the original two failed cases and seven failed
escape assertions, exit 42: ordinary/reserved null input with default and zero-node
limits, plus preconstructed callback `bad_alloc`, `length_error`, and
`runtime_error`. Each failure denies one actual `operator new` request and permits
all later allocations; callback exceptions are constructed before observation.
No hook runs for invalid input. This is an inherited ordinary encode gap and the
new reserved boundary gap, rather than a claimed ordinary encode regression.

The supplemental private controls preserve normal null/internal and zero-node
resource precedence, all three ordinary callback diagnostic mappings, and verify
exact output resource diagnostics after callback diagnostic allocation refusal.
Their first red run also fails; `red/runner-status.txt` records the initial runner
expected-exit-table mismatch and retained outputs. The runner was corrected to
recognize both expected-red suites. `red/runner-at-run.py` reconstructs its initial
version; `prepare.py` preserves the original deterministic preparation script.
No product regression failure was discarded.

`green/` and the final frozen `final-probes/` rerun the unchanged review's five
cases, the original two phase observers and two supplemental controls. All pass.
The review suite retains all eight actual graph/string/output failure ordinals,
513 SSO/alignment fixtures, prehook allocation absence, aggregate requested-byte
and output capacity bounds, invalid-input no-callback behavior, long moved first
typed refusal and null-hook ownership. The new callback checks additionally
require exact `resource_limit`, `output_bytes`, effective maximum and zero offset,
with no escaping exception. The separate native observer confirms reservation
phase ordering and conservative capacity bounds. These observations do not
measure process RSS or establish a new product allocator policy.

## Frozen focused Debug gate

`final-gate/source-freeze.json` is the single pre-gate source/status freeze.
Product, tests, support and current status prose were frozen before the final
serialized gate. `final-gate/commands.json` records exact commands and results;
JUnit, LastTest logs, vectors, audit results and pinned lint output are retained.
The verified inventory is 612, the exact prior B2-B focused selection is 346,
and the separate direct history/codec selection is 11. Prior 606/600/582 names
must all remain present. This is a focused Debug correction gate, not a full 612
suite, release gate, full B milestone or Linux gate.

Final result: Debug configure and `cmake --build build/rewrite-debug --target
all planar_tests` pass; all 612 inventory names remain, preserving prior
606/600/582. The exact 346-case focused selection and separate 11-case direct
history/codec selection pass with zero failures/errors/skips/disabled cases.
All nine private cases (2 phase + 5 unchanged adversarial + 2 supplemental)
pass, including the seven one-shot diagnostic controls, eight copy/output
allocation ordinals and all 513 capacity fixtures. Independent shape-definition
bytes, all four common vector checks and all six reversible audits pass.
Pinned formatting/Doxygen checks all 118 active C++ sources; the final no-change
all+planar_tests build performs no work. The single 387-entry source freeze
remains equal through the full gate and finalization. No full registered suite,
release or Linux rerun is claimed.

`candidate-manifest.json` identifies the combined codec implementation/interface/
tests, current status prose, new support and report. `candidate.diff` is the C01
increment against preserved baseline files, plus new support/report; its SHA256
is recorded separately. The unchanged B2-B interface and tests are included in
the combined manifest. `artifact-manifest.json` hashes retained evidence;
`historical-evidence-preservation.json` verifies original B2-B candidate and
review artifact manifests. `finalization.json` records the final frozen-source
check and passing summaries. Build ownership is released only after these files
are finalized; this coder will make no later candidate edits.

B2-C retained F dispatch and invocation integration, B3 selected identity/W,
B4 render, X06 recorded history/interpretation/acceptance, X07 public cutover,
X08 mandatory external-profile acceptance, execution/RA01 and Linux remain open.
Public init creates format1. The latest full gate remains A4's 582 per build.
