# V5-02-C implementation candidate

Candidate only; independent review is pending. Shared checkout on
`dev/planar-v2-vernacular`, HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`.
Only assigned files and new C support/evidence were changed. No Planar tools,
skills, queues, installed binaries, operator data, commits, installation,
publication or merge were used. Runtime VM/storage primitives were unchanged.

Both final gates are complete. **Coder ownership of both `build/rewrite-debug`
and `build/rewrite-release` is explicitly released; both directories are idle.**

## Implementation

`capture.host_path` classifies workspace selectors without filesystem access:
absolute/slash/case-sensitive `.lua` values are explicit files; other values must
match the exact 1–64-byte ASCII name grammar. Invalid bare spellings produce the
existing usage envelope before path limits. Names alone consult the explicitly
set environment; missing/empty/relative/invalid text yields usage, with no HOME or
other discovery. The borrowed environment view validates text, then the 64 MiB
bound, before snapshot ownership. Expansion normalizes its base and checks every
name/suffix join before concatenation. Explicit files ignore the environment.

Pure `prepare_host_path` returns preserved absolute spelling and a lexical-normal
absolute intent target. The original `normalize_intent_path(argument, explicit_cwd)`
API and its required-cwd contract remain unchanged. Render uses preserved spelling
for physical parent traversal and existing file/root/captured alias checks. The
new path pair is native only; neither host name nor path crosses Lua or public
portable identities.

`prepare_invocation_paths` runs after registration/catalog admission and before
native services. It validates selected workspace/registry/root/base/observations
or ledger/target paths, reads cwd once only if any needs relative resolution, and
makes all selected paths absolute from that one snapshot. All-absolute invocations
perform no cwd read. Standard-library cwd acquisition necessarily creates an
owning filesystem path; once acquired, validation and checked joins precede all
derived path concatenations. No cwd/environment is reread during services.
Native source-mapping refusal still precedes missing routed spec/registry/root
reads; parser and registration precedence remain intact.

Names are delivered for **render only**. Init syntax is unchanged. The existing
pure `request_intent` factory is reused only in tests to compare name/explicit
intent equality and differing environment targets without filesystem input.
There is no public load/replay service, profile loader or scaffolding change.

## Verification and exact adaptations

The preimplementation actual-binary `bad.name` regression fails with the old io
status 4 and passes with usage status 2 (`regression-red.log`, `regression-green.log`).
Initial native focused selection passed 15/15. Broader path/capture/CLI/export
selection passed 62/63; its sole failure was the historical directory-input test
using bare `workspace`, which now denotes a name. After reporting the exact need
and coordinator confirmation, only that input became `./workspace`. Its complete
regular-file io envelope, 60 host-path checks and 247 render CLI processes remain
unchanged. `cli-adaptation-audit.json` proves the single substitution. The new
unset-environment bare-name case pins the intended semantic difference. The next
19-test focused run passed with zero skips.

The earlier full debug checkpoint passed 369/369, zero skips, lint89 and no-work.
It is retained under `debug-before-snapshot-bound/` and its log/variant audit;
`before-snapshot-bound/` freezes the five subsequently changed source inputs.
A final allocation-order audit tightened the environment snapshot before ownership
and added a distinct borrowed oversized-view/NUL-before-size regression. Its first
compile omitted the test's explicit host-path module import; `build-05.log` retains
that failure and `build-06.log` the corrected build. The final nine-case focused
selection passed with zero skips. No assertion or bound was weakened.

Final direct serialized commands:

```sh
cmake --build build/rewrite-debug --target all planar_tests
ctest --test-dir build/rewrite-debug -R 'replacement.(workspace|host path|intent path|invocation path|cli$|retained_cli$|.*request intent)' --output-on-failure --output-junit tests/native/v5-02-c-evidence/focused-03.xml
ctest --test-dir build/rewrite-debug -R 'replacement.(workspace|invocation |prepared native paths)' --output-on-failure --output-junit tests/native/v5-02-c-evidence/focused-snapshot.xml
python3 tests/native/v5-02-c-support/gate.py build/rewrite-debug tests/native/v5-02-c-evidence/debug-final
cmake --preset release
python3 tests/native/v5-02-c-support/gate.py build/rewrite-release tests/native/v5-02-c-evidence/release-final
python3 tests/native/v5-02-b2-support/audit-variants.py build/rewrite-debug tests/native/v5-02-c-evidence/debug-variants.json
python3 tests/native/v5-02-b2-support/audit-variants.py build/rewrite-release tests/native/v5-02-c-evidence/release-variants.json
python3 tests/vectors/generate.py --check
python3 tests/native/v5-02-c-support/freeze.py candidate
```

The final gates each pass **370/370 CTest entries, zero skips/failures**, preserving
all 361 baseline names: 364 unit entries and six actual CLI acceptance suites.
Each gate retains raw JUnit and LastTest before subsequent inventory, checks pinned
formatting/Doxygen over 89 active source files, and finishes with a zero-work build.
The final selector suite executes **41 actual CLI processes** in each configuration;
these are separate from CTest counts. Existing actual suites retain 247 render,
60 init, 115 history, 55 export/output and 99 retained-dispatch processes (the latter
also uses its existing fixture helper). Test tags/process counts are not assertion
execution counts; the static source assertion inventory is labeled accordingly.

Both exact audits regenerate all seven historical/current variant families plus
the fixed-export runtime harness, match their configured files, and match each
other across debug/release. **No historical support generator needed adaptation.**
The vector check passes 12 hand byte, 20 upstream hash and 41 canonical/domain
vectors unchanged. Five Python helper/acceptance inputs parse successfully.

Host: macOS26.5.2 ARM64, Clang23.1.4, CMake4.4.4, Ninja1.13.2. Release uses
RelWithDebInfo. Linux compilation/runtime, hardware power loss and remaining R14
load integration remain unverified; this checkpoint does not close them.

## Frozen identity and preservation

`baseline.json` and `baseline/` freeze 19 original inputs. `candidate.json` and
`candidate/` freeze 23 inputs (the 19 originals plus four new support scripts).
Their manifest paths are **repository-root-relative**. `owned.diff` is the exact
baseline-relative no-index diff, with new support files compared to `/dev/null`.
`build-binaries.json` records 12 debug/release executable hashes with repository
paths. Current candidate bytes match the frozen copies.

`protection-audit.json` verifies all 3,948 protected original inputs unchanged,
including prior evidence/support, fixtures/goldens/vectors, user v5 and runtime VM
implementation/interface. `contract-symlink-audit.json` separately verifies the
exact approved contract body and `AGENTS.md -> CLAUDE.md` symlink. Only permitted
status prose changes in that contract. The coordinator-owned C round is untouched.

`artifacts.json` freezes evidence with paths relative to
`tests/native/v5-02-c-evidence/`; it excludes itself and the separately hashed
handoff identity file to avoid a hash cycle. Final manifest/diff hashes and counts
are recorded in `handoff-identities.json` and the coder's handoff. Approval must
come from a fresh independent reviewer.
