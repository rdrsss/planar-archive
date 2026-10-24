# V5-02-X03-A independent review

Disposition: **approve**. No blocking findings or requested changes for this bounded
pure request-v2 codec candidate. Review does not close the external-profile runtime
gate or approve a storage/public admission switch.

The fresh independent reviewer used the shared checkout on
`dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, and made no source or candidate-document
edits. Ownership was limited to this report and
`tests/native/v5-02-x03-a-review-evidence/`. No Planar tools, installed binary,
operator data, commit, install, publication or merge was used. Debug build ownership
was released by the author before serialized reviewer checks and is now released.

## Exact reviewed identity

Author evidence root: `tests/native/v5-02-x03-a-evidence/`.

- Repository-relative 13-file candidate manifest SHA256:
  `3a1054b716f1542127d44abea62c37e7bc52dd79802d72d10f2630ebeeefc5b5`.
- Exact baseline-to-candidate owned diff SHA256:
  `a553608bad5e526abff3d3a2b0242b10eebad68299cdd2b77b2088dbd0944a23`.
- Evidence-relative 63-entry author artifact manifest SHA256:
  `430f11c862d4666a9958dfbe39f1941ccae8134a5f2b6b8b8512df9f4bd52828`.

The reviewer independently checked every candidate file and candidate snapshot,
every author artifact and artifact coverage, and reconstructed the owned diff.
Existing owned baseline snapshots match the complete baseline manifest. All
14,764 baseline regular files were checked: no missing file or unexpected change;
only the eight existing owned files changed and five owned files were added.
The coordinator round is separately allowed. The reviewer evidence/report are
separately owned additions. `AGENTS.md` remains a symlink to `CLAUDE.md`.
Candidate and author artifact checks passed again after verification.

## Contract and implementation assessment

`request_v2.cpp:8,31–59,74–89,118–142` implements X-STORE exactly: ASCII
`PLANAR-REQUEST`, byte02, five big-endian u64 length-prefixed fields, selector00
without suffix or selector01 with one length-prefixed profile selector. Operation
is exactly `load`. Seed-zero XXH3-128 hashes the complete frame and uses canonical
16-byte order. No record-v1 wrapper or captured content/digest is added.

Whole-frame checked accounting includes all framing, selector and optional suffix
before encoding/copying. Decoding checks the complete 64 MiB bound first and
checks claimed lengths against applicable bounds and remaining input before
constructing owned field strings (`request_v2.cpp:95–113`). Unknown magic/version,
tag, incomplete fields and trailing bytes refuse. Positive canonical signed-64
head validation avoids overflow. Boundary failures return `record_error` with only
a closed code; bad allocation/length errors map to resource refusal, exposing no
request bytes or host selector.

The path seam meets X-STORE's inherited local codec rules. Old `record.cpp:84–96`
and `record.cppm:36–43` validate workspace/registry as nonempty UTF-8 without NUL,
retaining exact spelling. The new codec deliberately does the same; it performs
no normalization or host lookup. Existing `prepared.cpp:11–15,38–61` separately
requires normalized absolute workspace/registry intent before acceptance. It is
unchanged. Profile selectors add X-STORE's normalized absolute lexical check in
the new codec. X-CLI retains the 64 MiB host-selector ceiling; the 1024-byte
portable profile/module path limits do not redefine this private selector.

The new module has narrow downward dependencies and no ledger/runtime/CLI
consumer. Existing record-v1, prepared acceptance, storage schemas/openers,
public commands, Lua and old W source/fixtures are unchanged against baseline.
The CMake change only registers the separate module/test target and vector path.

The four new tests compare literal full-frame and digest fixtures, include
embedded/profile selectors, UTF-8 and INT64_MAX, exercise every proper fixture
truncation, malformed tags/versions/suffixes/trailing bytes, all field length
claims, invalid text/head/profile grammar, actor256/257, exact 64 MiB/+1 bounds,
and exact v1/v2 byte/digest inequality with cross-decoder refusal. The Python
oracle independently constructs bytes with `struct.pack`, compiles the pinned
upstream hash C source separately, and forms canonical digest order from numeric
high/low words; it does not import the product codec. Its use of the same pinned
hash algorithm is appropriate for framing/endian verification. No claim of
independent cryptographic algorithm validation is made.

Typed format-aware request coexistence and forced interner collision/redaction
proof remain explicitly required in X03-B alongside their actual storage consumer
(`external-profile-implementation.md`, X03-A/X03-B rows). This pure codec has no
interner and its v1/v2 inequality tests do not claim collision-safe storage proof.
No obligation was removed. The status-document diff accurately records independent
X02-C01 approval and the subsequent operator P1 A/P2 A disposition, preserving
reviewed mechanics, historical records and the open X08 runtime gate.

## Independent verification and raw evidence

Reviewer evidence paths below are relative to
`tests/native/v5-02-x03-a-review-evidence/`. Each executed command has a raw log
and result JSON with exact argv, cwd, UTC start/end and exit code. `verify.py`
records the checks. All commands exited zero, serialized in `build/rewrite-debug`.

| Check | Result / raw evidence |
| --- | --- |
| `cmake --build build/rewrite-debug --target all planar_tests` | Passed; `build.log`, `build-result.json`; existing graph was current, Ninja did no work. |
| Focused CTest record/request-v2/prepared/acceptance-intent/host-path selection | **27/27 passed, zero skips**; `focused.log`, `focused.xml`, `focused-result.json`, exact names in `focused-summary.json`. Four new codec, nine old record, thirteen acceptance/prepared, one host-path case. |
| Preserve CTest diagnostics before inventory | `LastTest.log` copied immediately after the focused run. |
| `python3 tests/vectors/records/generate.py --check` | All **10** old vectors verified; `record-vectors.log`. |
| `python3 tests/vectors/records/generate-request-v2.py --check` | Both **2** new full-frame/hash vectors verified; `request-vectors.log`. |
| `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | Passed pinned clang-format `--Werror` and Doxygen across **98** active C++ files; `lint.log`. |
| Repeat all + planar_tests build | `ninja: no work to do.`; `no-work.log`. |
| CTest inventory after saving focused artifacts | **409** names; preserves all **405** approved baseline names and adds exactly four; `inventory.log`, `inventory-comparison.json`. This is inventory, not a full-suite run. |
| Candidate/artifact/baseline protections | `integrity.json`, `supplemental-integrity.json`; identities above remained current. |

## Limits and handoff

This is a new unadmitted pure codec, so no bug-fix pre-failure demonstration is
applicable. Review ran focused debug checks on macOS; it did not run a release,
full milestone or Linux gate. Bounds are logical input/copy ceilings, not RSS
promises. Typed coexistence, schema2 consumers, exact transactional collision
handling/redaction and public runtime admission remain later reviewed work.
No external-profile capture/evaluation/acceptance or execution/RA01 acceptance is
inferred. The coordinator may advance X03-B after checking this exact candidate
is still current. Any candidate edit affecting this scope requires renewed review.
