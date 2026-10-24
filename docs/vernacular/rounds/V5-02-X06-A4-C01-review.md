# V5-02-X06-A4-C01 independent combined review

Disposition: **approve**. The frozen combined X06-A4 plus C01 documentation
candidate closes **X06A4-R1**. No new required finding was identified. This
approves an architecture document only. No product implementation, compiled API,
build or test gate is reviewed or approved.

The checkout is the shared `dev/planar-v2-vernacular` working tree. HEAD is
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a` at both the start and the end of this
review, and the rewrite is intentionally dirty. The reviewer owns only this
report and `tests/native/v5-02-x06-a4-c01-review-evidence/`. Everything else was
read-only: the frozen snapshot, the C01 author evidence and support, the original
A4 round, review and evidence, the product sources, the exact contracts and every
build tree. No build, test, installed binary, Planar operational tool, skill,
role or queue was used. Nothing was delegated, committed, installed, published or
merged, and no one else's work was reverted.

## What was reviewed

The candidate is the ten files saved under
[the frozen snapshot](../../../tests/native/v5-02-x06-a4-c01-frozen-snapshot/README.md),
not the live working-tree copies. After the candidate was frozen, an operator
documentation cleanup changed eight of the ten live files. That cleanup is
outside the candidate and was not reviewed. The two architecture-bearing
documents, `profile-preparation-architecture.md` and `x06-implementation.md`, are
byte-identical live and in the snapshot.

| Identity | SHA256 | Check |
| --- | --- | --- |
| Candidate JSON, ten files | `b748b0bfd7b8170c0dcc443931315ed7099bf7f049b544fe5942d096be1fc3a4` | matches the round record |
| Preedit-relative diff | `e8cb96b9f0049eca26d63500542ae874f25ffc0097215a424123c5303887d345` | rebuilt byte for byte from the preedit copies and snapshot |
| Author report | `8d0d80c777f7706b5bc0710ee3ea002d388913cec97a6e6e55027e081e65494a` | matches |
| Author artifact manifest | `5966b8517896f6c6a9ed06db4e4e2953fe75aef223e6d6fcfa14f1d2c515cc3f` | matches |

The independent [candidate audit](../../../tests/native/v5-02-x06-a4-c01-review-evidence/candidate-audit.json),
produced by [audit.py](../../../tests/native/v5-02-x06-a4-c01-review-evidence/audit.py),
verifies the following:

- All ten snapshot files match `candidate.json`, and the snapshot holds exactly
  those ten files plus its README.
- All eighteen evidence entries and four support scripts match the artifact
  manifest's hash and size. Evidence membership is exact.
- The ten preedit copies match the author baseline. They are also byte-identical
  to the original A4 candidate (`eec7fe7f...21deb5`) that received X06A4-R1. The
  combined candidate is therefore that exact A4 candidate plus this diff and
  nothing else.
- The diff reconstructs exactly from the preedit copies and the snapshot.
- In the nine status-opening documents, everything below the opening is
  unchanged.
- The snapshot contains 552 local Markdown links. Resolved from their repository
  locations, none is broken.
- All ten links to X06A4-R1 use an anchor that resolves against the immutable A4
  review's heading.
- The author's baseline and final manifests (94,226 members each) differ only in
  the ten authorized documents. No other non-directory entry changed any
  attribute.
- The original A4 review and round, the external-profile contract, every
  `v5-02-x06-a4-*` author and review subtree, and all seven cited source files
  still match the C01 baseline and final manifests.

## Closure of X06A4-R1

R1 found two gaps. The known-load factory admitted the workspace root, registry
and ledger before external registration but left out the selected workspace
declaration file. As a result, the source-capture path held only root authority
for the workspace. R1 required six connected changes, and the snapshot makes all
six consistently.

1. **Bound factory.** In the source-capture section (architecture lines 78–90),
   the bound factory now needs three original receipts from the same profile
   admission: the workspace root, the selected declaration file and the registry.
   It reads the declaration explicitly with its retained file token. Root-only
   construction is explicitly not a substitute.
2. **Known-load admission list.** In the factory protocol (lines 209–221), the
   root and the selected declaration file, admitted with
   `admission_file_role::declaration`, are metadata-admitted before registration,
   together with the registry and ledger. Receipts are reverified after
   registration. The text names the disjoint-directory hard-link gap and states
   that this admission closes it.
3. **Final parse.** Lines 218–219 and 237–239 keep both the root token and the
   file token in the same owner through registration and the X07 parse
   continuation. They move with the owner and its accounting state into
   `prepare_load`. Later discovery cannot replace either token.
4. **First read.** Preparation step 1 (lines 261–274) runs discovery through the
   original root, requires exact `same_selection` with the incoming original
   token, then verifies the actual opened no-follow leaf before any content is
   read. The original token stays the authority after the comparison. A leaf
   replaced between registration and first read is terminal. Cached reads, seal
   and acceptance reverify without rediscovery or an unbound retry.
5. **Retention through precommit.** Lines 324–334 keep the declaration-file token
   next to its root through sealed capture, prepared publication and native
   precommit, and no later step re-admits a replacement. Transaction step 4
   (lines 377–381) runs this concrete receipt verification immediately before
   COMMIT. On failure, lines 395–402 poison the original shared authority before
   any error is adapted.
6. **Future evidence.** The adversarial matrix (lines 450 and 453) and the
   source-capture delivery row (line 471) now require four cases: the
   disjoint-directory module/declaration hard link refused before any external
   chunk, refusal of a leaf replaced between registration and first read, a
   positive distinct-file registry-colocation case, and same_selection plus
   actual-leaf verification without receipt replacement.

The resource text (lines 414–428) keeps the existing cumulative admission
metadata charges for the declaration handle and the temporary
discovery/comparison staging, with no refunds. It explicitly adds no quota
category and says admission reads no source content. The x06-implementation
opening and the eight status openings summarize the same correction. They link
R1, say that combined A4+C01 awaits independent review, and do not approve
themselves.

## Contract conformance

The [X-CAP contract](../external-profile-contract.md#x-cap--strict-capture-and-trust)
is unchanged, and the candidate follows it:

- **Aliases.** X-CAP rejects distinct selected files that share a device and
  inode with a declaration file. It requires every known supplied root and file
  slot to be checked before external Lua evaluation.
- **Admission is not content capture.** Opening metadata and ancestor
  descriptors is not content capture. Discovered declaration-closure files are
  alias-checked before their contents are read.
- **Registry.** Registry may share a directory with trusted configuration, but
  selected-file aliases with the profile or its modules are rejected. The
  candidate's positive colocation case matches this exactly.
- **Reverification timing.** X-CAP requires reverification after registration
  for known slots and again before commit under lock. Both appear in the
  candidate.

No contract, byte recipe, schema, public command, error vocabulary, quota or
product policy is added. The load selector is already an absolute, lexically
normalized declaration-file path. `load_capture::open_workspace` uses its parent
as the root, and the request intent carries that path. So the selected file is
known when the factory runs, and admitting it before registration needs no
content read and no new selector.

## Source citation check

I read every cited mechanism. None of the citations overstates the code.

- [admission.cpp](../../../src/engine/capture/admission.cpp): `admit_root` (line
  321) and `admit_file` (line 337) open their selections through `append`. Its
  `relationships()` check (lines 226–247) compares every trusted-file leaf
  identity against every other non-directory selected leaf. A declaration
  hard-linked to a trusted module therefore refuses during `admit_file` itself.
  `admit_file` does not allow absence, so a missing declaration also refuses
  there. `retain_file` charges each issued handle cumulatively (line 452).
  `same_selection` (line 481) compares the shared state and the selection index.
- [admission.cppm](../../../src/engine/capture/admission.cppm),
  [profile.cppm](../../../src/engine/capture/profile.cppm) and
  [capture.cppm](../../../src/engine/capture/capture.cppm) export the forwarding
  and retained-token overloads the candidate names (`add_root`, `add_input` and
  `read`).
- [runtime.cpp](../../../src/engine/runtime/runtime.cpp): lines 4807–4809 admit
  the extension workspace root and then its declaration file. Lines 5070–5076
  bind the original root and read with `retain_file("extension.workspace")`.
  `extension_invocation::create` (line 4705) calls `command_vm::create` before
  `admit` (line 4743) runs. The candidate's caveat is therefore accurate: this
  path admits after registration and supplies only the mechanism, not
  known-load timing.
- [native.cpp](../../../src/engine/capture/native.cpp) `read_file` (lines
  201–300): a first read through a bound root calls `discover` and requires
  `same_selection` with the incoming authority. It keeps the incoming token
  rather than the discovered one. It refuses discovery after sealing or during
  verification. It calls `authority->verify(f.fd)` on the opened leaf before any
  content is read.
- [admission.t.cpp](../../../src/engine/capture/admission.t.cpp) lines 74–80
  contain existing hard-link refusal coverage for the declaration role. I
  inspected it but did not run it.

## Combined architecture

I found no inconsistency introduced by C01 elsewhere in the combined document:

- The `capturing -> admitted -> registered` lifecycle already says known mode
  checks slots before the registered state.
- The opaque path keeps its contracted late admission and does not claim the
  known-load ordering.
- The downward storage/intent/source-capture graph, the private same-module
  constructor and adoption, the private active frame across phase8 and all-job
  phases, and the single native transaction algorithm with its replay and CAS
  order are unchanged from A4.

The original A4 review's assessment of those sections still applies. A4 raised
only R1, which is now closed, so no original A4 finding remains open.

## Advisory notes

These notes do not block approval. They are reminders for the later code rounds.

- **X06A4C01-N1.** X-CAP says missing supplied paths keep their existing family
  capture errors. The new matrix row tests that a missing declaration refuses
  before registration. The code round should also assert the existing load-family
  diagnostic, not only the timing. The candidate already requires the existing
  load and format adapters with no new error vocabulary.
- **X06A4C01-N2.** The runtime and native citations use "around" line ranges.
  They are accurate today but will drift once extraction starts, so the code
  round's search should re-anchor them.
- **X06A4C01-N3.** Eight live reference openings now differ from the approved
  snapshot because of the operator cleanup. This approval binds to the snapshot
  bytes and to the two unchanged architecture documents. It does not bind to
  whatever wording the cleanup produced.

## Protection and limits

This review used a reviewer-scale
[protection audit](../../../tests/native/v5-02-x06-a4-c01-review-evidence/protection-audit.json),
produced by [protect.py](../../../tests/native/v5-02-x06-a4-c01-review-evidence/protect.py).
It made one baseline pass and one final pass over the whole checkout without
following symlinks. Every regular file outside `build/` was hashed. Build-tree
entries were compared by lstat inode, mode, size and mtime, without content
hashes. Only `.git`, this report and the reviewer evidence subtree are excluded.

The baseline pass ran after the reviewer had already created `audit.py` and
`candidate-audit.json`, which are inside the excluded subtree. The C01 author's
full-content manifests remain the reference for the candidate interval.

This review is static. Documentation verification cannot prove implementation,
allocation accounting, generated runtime variants, transaction races or real
hard-link behavior through the future factory. Those remain required future
evidence. No product build or test was run. The current inventory stays at 918,
and the latest complete independently approved macOS gate stays at 876 per
configuration. Public init creates format1, and fresh profile acceptance remains
refused. A1, A2 and A3 approvals are unchanged. Implementation of the X06
preparation, transactions and shared budgets, plus X07, X08, execution/RA01 and
Linux, all remain open.

**All reviewer files are released. Every build tree stayed protected, and no
build or test process was started.**
