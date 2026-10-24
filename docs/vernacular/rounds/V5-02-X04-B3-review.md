# V5-02-X04-B3 independent review

Verdict: **approve**. No blocking findings for the pure effective provenance
assembly and standalone structural verification slice. This review does not
close the integrated X04-B gate or mandatory external-profile gate.

Reviewed the shared `dev/planar-v2-vernacular` checkout at
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, CLAUDE.md, the complete B3 brief
including coordinator corrections, implementation plan, roadmap, v5 direction,
X-REG/X-LIMIT/X-STORE, approved B1/B2 reviews and the unchanged lower codecs.
Reviewer ownership is this report and
`tests/native/v5-02-x04-b3-review-evidence/` only. Product sources, author evidence
and unrelated rewrite changes were read-only. No Planar tool/skill/queue,
installed binary, operator data, commit, installation, publication or merge was
used. No other agent was dispatched by this reviewer.

## Frozen candidate and evidence integrity

- Candidate manifest: `19160aa0c14307b9db56979534bb10e57043b7a6e6275e4b810d837b10423d4b`.
- Owned diff: `8fe36e59720778b6bd061a83b1f91752ae92ba198df2793ec3ffc51c5a07a97a`.
- Author artifact manifest: `f35b19c06748ec4eade16343f96d48d7ed09cbf03f0a09770ba9ef1eedfe0e7d`.

Independently matched all 22 candidate paths to live and final-owned copies,
all 881 author artifacts, all 726 debug build identities, and all 17,398
protected final filesystem entries. Recomputed the exact 21 baseline changes
and complete owned diff; `protect.py` already existed at baseline and is the
twenty-second candidate path. Root-only `.git/` and `build/` exclusions preserve
nested historical paths. `AGENTS.md -> CLAUDE.md` remains intact. Each of the five
source-archiving attempts has 150 copies matching its individual freeze; the
final documentation/support gate records its complete freeze without another
source archive. Product implementation `.cpp`/`.cppm` is identical across all
five attempts. Final product/tests/CMake match the passing fifth attempt, and
all final candidate paths match the sixth gate. Reproduced forward and reverse
substitutions for all five generated storage/export families against current
source/generated hashes and bytes. Initial, final and finalization integrity
records are in reviewer evidence.

## Contract assessment

The module imports only standard, profile codec, catalog, canonical and old
provenance layers. It adds no filesystem, runtime, Lua, CLI or storage
implementation dependency. CMake lines 413–418 add the new module and colocated
tests through the existing helper; tests stay outside default `all`. Existing
profile/catalog/tree/record/provenance codecs, runtime, shipped Lua and historical
fixtures remain unchanged.

`effective_provenance.cpp:107` takes complete explicit old embedded files, raw
profile bytes/mode, ordered unprefixed modules and canonical catalog bytes. The
bounded metadata preflight at line 68 applies effective 32 MiB/8192 files,
per-file 16 MiB, old embedded 16 MiB/4096 files, external 1 MiB each/8 MiB total/64
files, profile 65536, normalized modes and portable path/segment/depth rules.
Checked addition cannot overflow because every successful accumulation remains
within its cap. Prefixes consume the original path/depth allowance. Only bounded
path/view metadata is constructed before this preflight; payload ownership and
old-name adaptation follow component limits. These are byte/count ceilings,
not a capacity/RSS guarantee or whole-invocation budget proof.

Lines 133–161 re-decode raw profile order and the old canonical manifest's
original order; sorted tree membership never supplies source ordinals. Lines
163–175 independently validate catalog owners/native pairings, encode the exact
eight-field combined manifest within 1 MiB, and reuse the unchanged complete old
manifest/assembly verifier. Old synthetic names map exactly to embedded.lua and
embedded.bin; originals and modules receive their specified prefixes, without
an external concatenated assembly. Line 176 recomputes the portable record_v1
tree digest, and successful results own sorted bytes without retaining caller
aliases.

Standalone verification at line 187 applies all component limits before owned
embedded scratch, validates whole-tree topology, decodes exact manifest/module
fieldsets and modes, reconstitutes explicit historical inputs and repeats
preparation. Lines 248–255 compare exact canonical manifest, full membership,
every payload and every mode. Fixed version/scheme/registration/path values are
enforced by this reconstruction equality. No supplied public digest, present
runtime or callable semantics is trusted. Structurally consistent changed bytes
correctly produce a new digest. Comparison with a caller-held expected stored
digest remains the later storage boundary's responsibility. Typed errors retain
only a closed category; unsupported old embedded manifests map to invalid
provenance as scoped in the brief.

The new pure module performs no digest-based reuse. Existing exact record/tree
framing and inherited real storage collision variants remain the relevant
collision evidence; no fake interner is presented as storage verification.

## Raw and independent verification

Parsed actual JUnit testcase elements and immediate LastTest copies, rather
than interpreting Catch2 assertion counts as case counts. Author attempt01 is
88/97, attempt02 88/98, attempt03 14/16, attempt04 11/12 and attempt05 12/12.
Failures occur only in new fixtures; inherited cases never fail. All have zero
skips. The preserved failures and fixture-only changes support the stated
boolean/key encoding and test-setup dispositions; unchanged implementation
hashes prevent attributing the later pass to a hidden product fix. The final
520-name inventory preserves all 508 prior names. The broader focused claim is
exactly 86 inherited passes from attempt02 plus 12 new passes from attempt05:
98 distinct cases across composed evidence, not one green 98-case run.

Independently checked the Python vector encoder against the canonical grammar
in render-contract lines 248–261 and portable record framing in ledger-contract
lines 121–145: boolean tags stand alone, map keys include their string tag,
unsigned byte sorting is explicit, directory hash preimages omit only local
record IDs, and XXH3 uses seed zero/high64-first big-endian bytes. Its actual
encoder/hash helpers verify 12 established canonical byte vectors and four
existing file/directory byte/digest vectors before checking the five new
artifacts. No product serialization code is imported or copied as the oracle.
The native test compares exact independent manifest/files and portable digest.

Author final gates also retain successful canonical/record/request vectors,
four inherited runtime/CLI audits, pinned 113-source formatting/Doxygen and
no-work builds. Reviewer `verify.py` executed under a complete filesystem freeze
excluding only root `.git/`, root `build/` and reviewer-owned outputs:

1. `cmake --build build/rewrite-debug --target all planar_tests`: no work.
2. Fresh CTest inventory: exactly the author's 520 names.
3. Exact B3 CTest selection: 12/12 passing, zero failures/errors/skips; JUnit and
   immediate LastTest/CTestCostData retained.
4. Independently compiled/linked reviewer probes with the candidate's pinned
   compile flags/module map and unchanged libraries: 4/4 passing. They cover an
   empty external closure, consistent reordered shipped history with corrected
   owner ordinal, nested original/external namespace spellings, and reachable
   old embedded aggregate exactly 16 MiB with +1 resource refusal in both APIs.
5. Independent effective vector check passes; final all/tests build does no work.

No reviewer command or probe failed. Commands, UTC times, argv, exits, raw
outputs and final identity checks are retained. Source remained unchanged
throughout verification. No full-suite duplicate was run.

This is focused macOS ARM64 Debug evidence. It is not a full 520-case,
RelWithDebInfo or Linux result. The latest full gates remain approved X04-A3 at
489/489 each. Component bounds make a valid complete effective tree at most
4162 files and below 32 MiB; B1 supplies exact outer-boundary evidence, while
B3 enforces those bounds defensively. Historical verification proves structural
correspondence, not callability or semantic honesty. Integrated B full gates,
live external registration/Lua, public profile CLI, interpretation-v2/profile
history acceptance, whole-invocation shared budgets, public cutover,
execution/RA01 and Linux remain open. Public init still creates format1.
Approval is limited to the exact candidate identities above; edits affecting
that scope require renewed review.
