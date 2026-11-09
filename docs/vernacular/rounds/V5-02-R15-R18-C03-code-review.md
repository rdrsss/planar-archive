# R15–R18 C03 independent code review

Disposition: **request-changes** for N8 below. This is scoped native/Lua and
contract review, not final integrated acceptance. No product or test source was
edited, and no build ran. The reviewed native/Lua files still match
`plan/candidate-native-review.json`; generated scope hashes and amendment hashes
are in `review/identity.json` under this round's evidence directory.

## N8 — P2: publication payload is not bound to its facet identity

`src/engine/ledger/execution.cpp:2570–2593` accepts a publication payload whenever
its referenced record exists and has directory kind; it checks only the semantic
blob's length. The reconstructed saved response at lines 2349–2359 includes the
publication identity/semantic but not its payload. Nothing verifies that the
payload contains that facet's declared paths or hashes to its recorded semantic.

Reproduced with the actual source-built Debug CLI after two successful grouped
checks with nonempty facet publications. In a fresh temporary ledger, replace
`publications.payload_id` with the existing declaration source directory record,
dropping and exactly restoring the publication immutability triggers for the
corruption. The source directory is a valid complete tree, so record/edge checks
still pass. `status` exits **0** and accepts the corrupted ledger. Both a historical
publication and the latest current publication were tested independently. The
current case changed publication 2's payload from record 61 to record 4.

This violates R17-1's publication row meaning (`execution-contract-r17.md:39`):
payloads retain exact facet-path bytes, and `semantic` is the facet semantic
identity. The approved `render-contract.md:335–337` supplies its exact derivation.
A valid but unrelated tree cannot stand in for those retained bytes. This matters
to historical reconstruction as well as current published artifacts.

Evidence: `review/publication_probe.py`, `review/publication_probe.log` (historical)
and `review/publication_current_probe.log` (current). The script currently selects
the latest row; the first log preserves the earlier historical selection. Each
assertion failure is the intended demonstrated defect, not a passing result.

Correction: for every publication, derive its declaration and owning unit from
its completion's retained dispatch W; validate the selected facet path set and
mode-normalized payload tree, and recompute the exact `facet_v1` semantic from
those retained files and declared root/paths. Apply the check to historical rows
as well as current snapshot publications. Add tests changing only the payload to
a different valid tree, and independent semantic/payload mismatch coverage with
valid historical and duplicate-tree reuse controls. Reuse the existing canonical
and hashing adapters. Do not rerun checks or read the live checkout during ledger
verification.

## Assessed corrections

N5 now derives per-consumer complete-phase vectors, including grouped internal
producers, and compares their outputs with the same reservation's retained
observation. Boundary rows retain their own-only exception. The planner's capture
union and verifier agree on actual selected vector producers. The R17 amendment
clarifies the approved render semantics rather than adopting the former faulty
shared-external-producer implementation.

N6 applies identity comparison unconditionally under the write lock while keeping
full head/fact comparison completion-only. The paired stale-head/fact positive
control is retained. PR3 projects the released target before rederiving readiness;
writer and replay constrain the target first and the narrow permitted dependent
transition grammar. The R16 amendment covers all three conflicting clauses and
explicitly extends EX-4/M20–M22 carriers; no-fact-bump semantics remain intact.
N7 correctly reuses `intern_codec1` so fresh publication directory records receive
edges while existing exact records are reused without duplicate edge insertion.
The paired N3 invalid-reason regression removes the saved-response mismatch that
previously masked the vocabulary invariant.

Independently executed:

- C03 release CLI: **2 scenarios, zero failures/skips**, both reasons, exact replay
  and unchanged fact versions (`review/release.log`).
- C03 grouped/corruption CLI: **4 scenarios, 6 corruption assertions, zero
  failures/skips** (`review/vector.log`).

The coder's 163-case native run and independent grouped/split publication-byte
workflow were inspected as evidence, not rerun here. Final full Debug/Release/
Linux gates and C03 mutation probes remain pending. The known external-profile
load cutover failure belongs to the separate integration package; it is not
certified by this scoped review. No R19 real-project claim is made.
