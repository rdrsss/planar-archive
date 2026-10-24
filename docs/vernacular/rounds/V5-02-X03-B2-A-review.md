# V5-02-X03-B2-A independent review

Decision: **approve** the bounded typed stored-reader slice. No blocking product
or test finding. This approval does not close B2-B native acceptance, X06 profile
interpretation/provenance, full format2 acceptance or the external-profile gate.

Reviewed independently on the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The author released ownership before
review. The reviewer owns only this report and
`tests/native/v5-02-x03-b2-a-review-evidence/`; no product, source tests, author
artifacts or status documents were changed. No subagents, old Planar tools/skills,
queue, installed binary, operator data, commit, installation, publication or merge.

## Frozen candidate and preservation

Repository path base: `<repo>`.

| Identity | SHA256 |
| --- | --- |
| Author `candidate-manifest.json` | `eb22abd8b7a33a16213cfb5a5579d01c4a206e2166dd6a9ff5acd9923740da6b` |
| Author `owned.diff` | `888a8b1f6e9f0d78f83f373f1eabebf8172209c18537e9acf0fd246918c98dbb` |
| Author `artifact-manifest.json` | `963feff0e3f6a075818b612dcd9b5ff306750421356b400ac518958b4aed13f0` |
| Author `final-source-manifest.json` | `d3c63e44ea9432fece7f5d6fe939f621932e1839ae49bc983fd0f5b62dd7e683` |

Author manifest/artifact names in this report resolve below
`tests/native/v5-02-x03-b2-a-evidence/`. The candidate manifest's `files[].path`
resolves from its recorded repository path base; `candidate-owned/` contains
matching copies. The artifact manifest uses the author evidence directory as base
and excludes itself. Every candidate file and author artifact matches before and
after independent verification. All 13 final-source snapshot copies match their
pre-verification manifest. Immutable record/request codecs, prepared state,
acceptance logic, CLI and history/export consumers remain byte-identical to their
baseline; only the assigned ledger implementation/tests, narrow wiring and status
documents changed.

The complete-baseline claim needs an explicit qualification. The larger retained
pre-edit manifest covers 14,980 regular files and two symlinks, but omits these
four historical nested-build files:

- `tests/native/r14-d6-c01-evidence/red-source/build/rewrite-debug/src/engine/planar_load_c01_source.cppm`
- `tests/native/r14-d6-c01-evidence/red-source/build/rewrite-debug/src/engine/r14-d6-c01-generated/runtime_0.cpp`
- `tests/native/r14-d6-c01-evidence/red-source/build/rewrite-debug/src/engine/r14-d6-c01-generated/commands_0.lua`
- `tests/native/r14-d6-c01-evidence/red-source/build/rewrite-debug/src/engine/r14-d6-c01-generated/runtime_0.cppm`

A root-only `.git/` and `build/` exclusion walk finds exactly these four omitted
current files, totaling 946,649 bytes, and no additional symlinks. Every one
matches its hash in the retained first 7,273-entry pre-edit manifest; AGENTS.md is
correctly represented separately as a symlink to CLAUDE.md in the larger manifest.
The union of the two original pre-edit manifests therefore supplies preservation
evidence for 14,984 regular files and two symlinks. Neither original manifest was
rewritten or reconstructed. The larger manifest alone is not complete under the
documented root-only exclusion. Independent audits find no missing baseline file,
no unexpected changed baseline file, and stable symlink targets. The report and
review evidence resolve this description limitation without altering the frozen
author evidence.

## Contract and implementation review

Read the active CLAUDE guide, implementation plan, roadmap, v5 direction, assigned
B2-A round, X-STORE exact contract, bounded implementation path and author handoff.
The requested format2 prohibition of directory request children applies to both
codecs; the explicitly retained v1 generic graph behavior is tested separately.

- `src/engine/ledger/types.cppm:52` introduces a separate stored-value alternative
  for request_v2 while retaining immutable record_v1. The shared encoder dispatches
  by the alternative, preserving complete codec1/codec2 byte identities.
- `src/engine/ledger/format.cpp:407` bounds row metadata before obtaining blobs;
  codec2 requires format2/kind4, invokes the approved complete-frame decoder and
  digest, and refuses outgoing request edges. Codec1 follows the existing reader.
  `format.cpp:483` rejects either request codec as a format2 directory child.
  Exact schema/integrity checks also reject illegal kind/codec pairs.
- `format.cpp:516` retains exhaustive bucket-member byte comparison, now binding
  the precise codec with kind and digest. Digests remain insufficient for equality;
  no v1/v2 cross-codec byte equivalence or relaxed bucket reuse is introduced.
  Native v2 interning/forced collision transaction proof remains B2-B work.
- The chronological verifier at `format.cpp:861` retains event/revision parent,
  active references, actor/time, complete provenance/interpretation, no-op content,
  saved result, request ID/event/revision/outcome and final projection checks.
  `format.cpp:993` applies operation/actor/expected-head correspondence to both
  request types. Accepted profile requests refuse with a fixed unsupported_state
  message at `format.cpp:1006`; orphan profile requests can decode structurally.
- Public history uses saved result/event/revision data and never emits request
  argument bytes or request digests. Added verifier failures use fixed messages
  without private text/details. Embedded mixed-codec fixtures exercise all four
  public history selectors and source export through existing native services.
- The format2 writable refusal at `format.cpp:1201` precedes writable SQLite open;
  `begin_records` still uses that gate. Explicit recovery refuses format2 at
  `format.cpp:2052` before any scratch/original writable handle. Public init still
  calls the unchanged format1 path. Backup/restore retain selected format/identity.

The fixture helper at `history_verification.t.cpp:894` creates a real new native
format2 ledger, then uses test-only SQL to copy relational rows and rewrite
selected request frames/digests. These are reader fixtures, with no product
conversion API or native format2 accepted-operation proof. Supported embedded v2
history uses unchanged recorded v1 interpretation/provenance without current Lua.
Accepted profile history remains unsupported until X06 supplies the matching
recorded interpretation/provenance verification.

## Regression and evidence assessment

The retained pre-fix regression log shows one matched test failing at native
`open_existing`, before the reader integration; baseline product bytes and build
output remain frozen. The newly added regression test source was not separately
frozen before that first execution. This limits exact reproduction of that first
red run and is disclosed, rather than retrospectively reconstructed. The final
source retains its native open/read check and adds typed-value assertions.

The first broad build's namespace typo and first focused run's two failed
assertions remain in raw logs/XML/LastTest. The corrections are justified:
requests 3 and 4 share one interned body ID, so rewriting it makes both codec2;
changing that expected alternative preserves mixed codec1/codec2 coverage.
The approved pure decoder classifies a malformed frame-version byte as
invalid_record, mapped to corrupt; the corrected expected error preserves refusal,
unchanged bytes, absent journal and redaction checks. The independent passing run
uses the exact frozen final source, without weakening these checks.

## Independent verification

Commands ran serially in the owned debug build directory. Review raw result JSONs
contain exact argv/cwd/start/end/exit status. JUnit and LastTest were saved before
inventory. The initial review command mistakenly named a nonexistent build preset
`rewrite-debug`; its CMake refusal is retained in `build.log`, and the corrected
explicit-directory command succeeded. No source edit was needed.

| Check | Result |
| --- | --- |
| `cmake --build build/rewrite-debug --target all planar_tests` | Pass; no work |
| `ctest --test-dir build/rewrite-debug --output-on-failure -R 'ledger\|history\|export\|record\|request\|prepared\|acceptance\|publication' --output-junit …/focused.xml` | 188/188 pass, zero skips, 59.25 seconds |
| `python3 tests/vectors/records/generate.py --check` | 10 independent record-v1 vectors |
| `python3 tests/vectors/records/generate-request-v2.py --check` | 2 independent request-v2 vectors |
| `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | Pinned formatting --Werror/Doxygen pass, 98 active sources |
| Final `cmake --build build/rewrite-debug --target all planar_tests` | No work |
| `ctest --test-dir build/rewrite-debug --show-only=json-v1` | 421 names; all 416 B1 baseline names retained, five added |

Independent results reproduce the author's 188-case final focused gate. The scope
includes prior v1 record/history/acceptance/fault cases and new typed/mixed format2
reads, malformed/private refusal, profile structural/unsupported boundary,
directory prohibition, embedded history/export and backup/restore. V1 fault cases
do not demonstrate v2 acceptance atomicity.

Review evidence path base:
`<repo>/tests/native/v5-02-x03-b2-a-review-evidence`.
Its `artifact-manifest.json` excludes itself and freezes every other review raw
artifact. Its SHA256 is
`89b16fc8c11091de45b7242afcc4cd7369250d530ca276c5d8ba5ca943b6063e`.

This is macOS ARM64 focused debug verification. Full release/milestone/Linux,
native format2 request insertion/interner/collision/CAS/replay/crash/full/recovery,
X06 profile provenance/interpretation and the public external-profile runtime gate
remain required later. Approval permits advancement only within the established
round sequence while this exact reviewed candidate remains current.
