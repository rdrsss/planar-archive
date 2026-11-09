# C09 independent test-source review

Disposition: **approve the bounded test-only correction**. No product source, test source, build artifact or database was modified by this reviewer. No build, test or mutation was rerun; source and retained raw evidence were inspected. Full platform gates and final integrated approval remain separate.

The final registered script matches the writer identity: SHA256 `c71b35de1b578ec747693363869049b90d5a641dc512f8f5a9e24422d1751ba0`. The existing C04 registration still invokes this script with an explicit candidate binary and repository working directory. The fixture and oracle remain confined to its temporary root.

## Predicate isolation

The two facets belong to the same producer, have distinct publication/payload/semantic identities, and each payload has exactly one immediate tree child and one recursive file leaf. The genuine second facet semantic is independently recomputed and compared with its actual recorded value before corruption. The mixed semantic then uses the original facet's declared `other/a` path together with the swapped payload's actual `other/b` entry, retaining root, kind, mode, size and bytes. This matches the render contract's distinct declared-path and entry fields and removes the semantic-recomputation masking present in the earlier witness.

The corresponding saved check response is changed consistently: the original semantic must occur exactly once, the mixed value must be absent initially, persisted bytes must actually change and remove the original, and exactly one request row is updated. Original immutable trigger SQL is restored before status and exact check replay. The test requires exit4 and `ledger_corrupt` from both reads. The recorded mutant disables only the declared-path equality arm while retaining count, ordering, mode and semantic checks.

The portable oracle reuses the independent vector encoder/Hash layout and compiles the pinned vendored xxhash0.8.3 C source into the fixture temporary root. It introduces no installed/Homebrew dependency or product hook. Its seed-zero numeric high/low formatting agrees with the existing oracle and is validated against a genuine publication in this test.

## Prior controls and sensitivity evidence

The original facet's three publication rows still prove historical payload distinction and current duplicate-payload reuse. Filtering this query by the original facet is appropriate after adding the second facet; the new facet separately must have three historical publication rows. The four prior corruption scenarios and their eight status/replay assertions remain intact.

Inspected final baseline and same-mutant raw logs and independently matched every file/binary/log hash in the writer identity. Baseline: six scenarios, ten corruption-reader assertions, zero failures/skips. Same compiled N8 path-equality mutant: six scenarios, ten assertions, exactly two failures and zero skips; only the new status and replay checks return unexpected exit0. All earlier eight assertions and the genuine semantic/equal-count controls pass. The initial quote SyntaxError and earlier pre-final leaf-count logs are accurately excluded. This establishes the registered test's sensitivity to the previously surviving fault without changing product code.

The writer recorded final source installation at Debug gate796/start797, before its publication entry. Coordinator must confirm the gate executed this version, or explicitly rerun the registered entry. This review does not claim that gate result or Linux portability execution. No blocker or new product choice was found. Review/source/evidence hashes are in `tests/native/r15-r18-c09-evidence/review/IDENTITY.json`.
