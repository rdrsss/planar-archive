# X03-A coder handoff

Candidate for independent review; no self-approval. Shared checkout HEAD
3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a, dev/planar-v2-vernacular.
Only the 13 owned files in candidate-manifest.json changed or were added outside
this evidence directory. No commit/install/publish/merge or operator data access.
Debug build ownership is released; no process remains using it.

The separate pure request_v2 module implements exact PLANAR-REQUEST byte02,
five BE-u64 length-prefixed fields and selector00/01 plus optional profile suffix.
The operation is exactly load; digest hashes the complete frame with seed-zero
XXH3-128. Whole-frame bound is 64 MiB for both embedded and profile frames.
Errors reuse redacted record_error categories, with bounded claims before allocation.
No record-v1 wrapper, storage schema, prepared representation or public admission
switch is introduced. Record-v1/prepared/ledger/public command sources and old W
remain byte-identical to baseline (full protection audit).

Path validation deliberately retains the v1 record codec's local nonempty
UTF-8/no-NUL rule for workspace and registry, preserving exact argument spelling.
The existing prepared request_intent separately requires normalized absolute
workspace/registry arguments. Only profile_argument adds absolute lexical-normal
validation in this codec. No host filesystem read or cwd resolution occurs.
The independent vectors intentionally retain relative workspace/registry spelling
and include a UTF-8 absolute profile selector; tests make this seam explicit.

Status-only documents now record independent X02-C01 approval and operator P1 A/
P2 A. The implementation path explicitly allocates typed request coexistence and
forced interner request collision/redaction proof to X03-B with its storage
consumer. Those requirements remain mandatory; codec tests provide no storage
collision proof. Reviewed contract mechanics are preserved.

Verification was serialized in build/rewrite-debug. build.log is the initial
successful build; final-build.log builds the final source. final-focused.log/xml
and final-LastTest.log are the final 27-case run: four new request cases, nine old
record cases, thirteen acceptance/prepared cases, one host prepared path case.
All 27 passed with zero skips; test-summary.json records exact selected names and
counts. The earlier focused14 run is retained and superseded by the 27-case run.
Record oracle --check verifies all ten old vectors; new separate Python struct/
pinned upstream xxHash oracle verifies two full-frame literal/digest vectors.
The fixtures check embedded/profile, UTF-8 actor/profile and INT64_MAX head.
Tests cover every proper vector truncation, suffix/tag/version/trailing refusal,
all field length overflow claims, invalid text/head/profile grammar, actor256/257,
exact 64 MiB/+1 boundaries and exact v1/v2 bytes/digest inequality.
No pre-fix regression applies: this is a new, unadmitted pure codec, not a bug fix.

Lint passes pinned clang-format --Werror and Doxygen across 98 active files.
no-work.log reports ninja: no work to do. Each command has a raw log and paired
result JSON with argv, cwd, UTC timestamps and exit status. JUnit/LastTest were
saved before the full inventory. inventory-comparison.json checks all 405 prior
registered names remain and exactly four new names yield 409; this is inventory,
not a claim all 409 ran. No release/full milestone, public external profile,
transactional collision or Linux acceptance is claimed.

baseline-manifest.json contains SHA256 for all 14,764 baseline regular files,
excluding .git, build and this evidence root. Baseline status/diff/HEAD and exact
owned snapshots were saved before edits. candidate-owned contains exact current
owned snapshots. owned.diff compares baseline to candidate, including new files.
candidate-manifest.json uses repository-root paths; identity.json identifies the
manifest and exact diff. protection-audit.json proves no unrelated changes or
missing baseline files; AGENTS.md remains the CLAUDE.md symlink. Artifact paths
are relative to this evidence root; artifact-manifest.json excludes itself.
freeze.py regenerates snapshots, protection/inventory comparisons and manifests.
