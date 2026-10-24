# R14-A2 independent review

Disposition: **approve**. No findings. This approves only the behavior-preserving
pure-codec extraction; accepted-history support and the integrated R14 milestone
remain outstanding.

Fresh reviewer `/root/r14a2_reviewer` reviewed shared
`dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Read CLAUDE/AGENTS, implementation,
roadmap, v4, ledger contract, R14/A/A2 briefs and the A1/D01 approvals. Candidate,
product, original fixtures and prior evidence were read-only. Owned writes are
this report and `tests/native/r14a2-review-evidence/`; others' work is preserved.
No Planar tools/skills/queue, operator data, installed binary, subagents, commits,
installation, publication or merge. Debug commands were direct, serial and held
exclusively. Debug is released on delivery; release was not used.

The thirteen-file repository-relative candidate manifest SHA256 is
`b454765268653eebfe18681851d09aa0baf60ec1269c9a911ee2d5bc6b509baf`;
its sorted compact JSON files-map identity, without terminal LF, is
`281dbab2c65e9b67d35a3db47fb2afc290a7971ac63aff409667fa7532262cb2`.
All live files and repository-relative `candidate/` snapshots match. The seven
existing-file baselines match previously approved C1/C2/A1/D01 candidate hashes;
the complete baseline-relative diff independently reconstructs exactly, SHA256
`7fa8a76d64a0a15ff69254343e18977726d1f0580ebf4538871a9bf032bde4e1`.
The coder evidence manifest SHA256 is
`51659c14e37930fbc1a341115fbccf9fc3e0c8ec4a5c56427ac6a9504b87f44e`;
all 52 entries relative to `tests/native/r14a2-evidence/` match. All 4,204 local
and 11,993 inherited repository-relative protected hashes match, retaining the
documented owned-path, coordinator-planning/round and Finder exclusions.

`provenance_codec` owns the original DTOs/errors and complete registry,
manifest/assembly, path/budget, verification and tree-digest algorithms. Independent
full-body comparison allows only whitespace, module identity and removing the
embedded wrapper. The existing wrapper retains its exact embedded preparation
body, re-exporting the codec and original owner-backed tree APIs. Historical
verification reads supplied complete files without Lua, filesystem access or
comparison against today's embedded digest.

`interpretation_codec` owns the original DTO/errors, one supported-version table,
entire fixed checker and decoder. Independent full-body comparison confirms
their exact preservation. Its encoder is the original envelope/versions assembly
and canonical encoding, parameterized by bundle/semantic/digest data, and finishes
through the same decoder. The digest argument asserts no authenticity. The live
wrapper still accepts only the captured bundle, computes embedded provenance,
invokes only `interpret_bundle`, retains its original runtime/resource/validation
error handling and derives the digest internally. Shared small shape-access
helpers support live-result extraction; the fixed historical checker and version
policy remain single-sourced. Original names are direct re-exports in the same
namespace, with no replacement DTOs or conversion policy.

Both independently captured pure-test links exclude format, SQLite, runtime,
embedded source, Lua and higher wrapper libraries. Provenance links only its
codec, tree core, inert ledger types, record/canonical/value, pinned xxhash and
test/standard-library support; interpretation links only its codec,
canonical/value and test/standard-library support. CMake preserves test targets
outside default `all`. Storage/schema/refusal, canonical/record/tree behavior,
exact Lua, generic embedding, CLI, vendor, fixtures and goldens remain protected.
The affected architecture/testing references accurately describe the boundary.

Independent macOS/arm64 debug verification is recorded under
`tests/native/r14a2-review-evidence/`, with exact argv/cwd/exit statuses and raw
logs:

- `cmake --build --preset debug --target all planar_tests` passes. The final
  repeated build reports exactly `ninja: no work to do.`
- Focused `replacement.(ledger|canonical|hash|record|tree|provenance|interpretation)`:
  122 passed, all 113 prior case names plus five pure provenance and four pure
  interpretation cases, zero skips.
- Raw CLI acceptance: 247 real processes, 60 host-path checks, all six native
  packet goldens, zero skips. Canonical/hash and record vectors both pass.
- Pinned formatter/Doxygen: 60 active source files pass; the two supplementary
  verification helper files pass with zero warnings.
- Reviewer-only pure interpretation probe: two cases, 54,398 assertions pass.
  Every proper prefix of both saved canonical values and all 256 possible byte
  trailers refuse with the expected malformed category; native codec failures
  retain no validation payload or caller diagnostic. Assembly preserves binary
  historical extension data, owns output independently of retained mutable input
  aliases, and round-trips it. Eight concurrent tasks each perform twenty
  encode/decode cycles with separate differing valid digests.
- Reviewer-only pure provenance probe: one case, 108 assertions pass. All 24
  complete-set permutations retain the same digest and sorted owned bytes after
  original input mutation. Unknown manifest version/scheme retain
  `unsupported_manifest`; invalid registry mode retains `invalid_tree`.
- Wrapper-only import probe: one case, seven assertions plus five compile-time
  signature checks pass. Original exported types/functions remain usable;
  embedded provenance verifies, and a different historical stdlib verifies
  through the old public names.
- Independent audit verifies complete moved algorithms and composition, exact
  baseline/diff identities, all candidate/evidence/protected hashes, and fresh
  byte-identical reversible C2/B1 generated source/interface/manifests with
  collision verification mutation disabled. Final candidate hashes remain current.

Two initial reviewer harness mistakes are preserved separately: the first audit
omitted the older C1 candidate manifest when locating approved provenance hashes;
the first probe attempted mutation through the public const value-pointer type.
The corrected harness includes C1 and retains a mutable input alias explicitly.
These were review-script failures, with no candidate edit or product failure.
Temporary probe objects/executables were hashed before removal; sources and
commands remain. Reviewer `evidence.json` uses paths relative to its own evidence
folder and excludes itself; `review-identity.json` pins this report separately.

No full debug/release milestone gate or Linux execution was run in this focused
round. Existing supported record-only/refusal policy remains unchanged; future
accepted-history functionality requires its own fresh rounds and integrated gates.
