# X04-A1 native metadata admission candidate

Coder candidate awaiting independent review. No self-approval. Shared checkout
`dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. No commits, installed commands,
operator data, Planar tools, installation, publication or merge were used.

## Delivered boundary

`planar.engine.capture.admission` provides a move-only, thread-confined capability
created from an explicit absolute trusted root. `select_trusted` admits an explicit
absolute regular file within that physical tree, preserving spaced/Unicode profile
filenames; portable module spelling remains A2 policy. Staged `admit_root`,
`admit_file` and `admit_target` retain initial identities, modes and no-follow
ancestor/name relationships. Files additionally retain size/mtime/ctime. Rechecks
compare these original receipts, never replace them and never enumerate siblings.
Directory timestamps are excluded so unrelated siblings remain irrelevant.

Workspace/source/base roots must be physically disjoint from the trusted root in
both directions; supplied roots may overlap each other. Observations/declarations
must remain outside it. Registry directory colocation is allowed. Selected trusted
file aliases with other selected trusted files and supplied registry/observation/
ledger/declaration files refuse reciprocally, including when trusted selection is
added later. Existing export directory trees and absent final targets with existing
parents retain receipts; trusted overlap and selected-file target aliases refuse.
An unrelated existing regular-file export target remains a native path refusal.
Missing inputs and symlinks retain native error mappings. New overlap/change/limit/
protocol diagnostics use the exact X-LIMIT messages and contain no host paths.

`native_capture_session::read(root,path,admission,slot,text)` opens the no-follow
input, checks its descriptor against original admission metadata **before** size
allocation or data reads, then uses existing native capture. A standalone profile
still reads logically as `input.json`; module capture uses its normal relative path.
Cached-file reuse also verifies the newly opened descriptor and retained bytes.
Callers of the original overload retain its behavior. Duplicate private receipt
keys refuse rather than replacing receipts. Failed or moved capabilities refuse;
verification has no callbacks or Lua. Self-move preserves ownership.

This proves native prerequisites only. No profile JSON/module closure service,
external Lua registration, CLI phase ordering, result authorization, acceptance
receipt integration or commit wiring is implemented. A2 and later rounds must
consume the original capability and apply their required phase/sealing checks.
Cooperative filesystem checks cannot prevent hostile writes after a check or
observe every change-and-revert. Linux and release execution remain untested here.

## Bounds and allocation accounting

X-LIMIT supplies 65 selected trusted files, 10000 total selected files and the
64MiB native/capture ceiling; trusted files count toward the total. Directory roots
do not consume file slots. Internal metadata receipts and selections are capped at
100000, reusing the existing native `implementation::enumeration_budget::charge`
entry/ancestor ceiling in `src/engine/capture/native.cpp`, not inventing a public
X-LIMIT100000 file policy. Private receipt keys use the existing1024 path ceiling;
host selectors retain the inherited64MiB host ceiling from `host_path.cpp`.

The independent64MiB admission charge conservatively accounts for bookkeeping and
retained strings/chains:512 bytes plus name/key length per new receipt/selection,
128 plus component length for temporary traversal copies,8 bytes per chain index,
and128 plus name length for absent leaves. It reserves both a temporary component
and potential new receipt before copying/opening. Directory receipts deduplicate
only when original parent/name/identity/mode match. Charges are cumulative and not
refunded. Descriptor counts are bounded; this is not an RSS, capacity or kernel
memory guarantee. Native content capture retains its separate inherited budgets;
A2 must compose complete content quotas rather than summing away limits.

## Verification

`commands.json` records exact executed final commands, exit codes and timings.
`source-freeze.json` / `source-stability.json` bind all candidate product/support
sources throughout the serialized final verification. `debug-build-identities.json`
binds the resulting binary, admission test executable, observer and admission object.

- Debug `all planar_tests` succeeds. Inventory451 preserves all430 X03 names and
  adds21 admission cases; `inventory-preservation.json` lists each name. This is
  inventory preservation, not a full451-case execution claim.
- Focused capture/path/acceptance:120/120 pass, zero failures/errors/skips, including
  all21 new cases and existing capture/path/acceptance cases. Raw CTest/JUnit:
  `debug-focused.log`, `debug-focused.xml`, `focused-results.json`.
- Canonical, record-v1 and request-v2 vectors pass. Four inherited runtime/CLI
  audits and five exact forward/reverse storage/export source variants pass.
- Pinned clang-format/Doxygen checks101 active first-party sources; observer
  support formatting passes. The final serialized build says no work to do.

`syscalls.cpp` is a test-only dynamic interposer keyed by explicit device/inode;
it observes read/pread/readv/mmap attempts. macOS dyld interposition forwards from
its own image to the original function. Linux syscall forwarding is present but
has not been executed. Runtime read positive controls prove interception works for
profile, module, registry, observation and declaration inodes. Admission, staged
addition and repeated metadata verification show zero observed data-read attempts.
A successful native bound read produces observed attempts. A replaced profile
inode shows zero attempts before refusal. The admission object also has no
undefined content-reading or directory-enumeration API symbols (`nm` audit).
This is bounded, positive-controlled API observation plus compiled/source evidence,
not universal kernel syscall tracing or proof from binary/large fixture contents.

`sensitivity.py` applies exactly reversible controlled negative variants: omitting
pre-read descriptor binding, and inserting a metadata-phase read attempt. Each
exact one-test run fails; the script restores exact original source bytes, builds
again, and records hashes/substitutions in `sensitivity.json`. These are negative
controls, not claims about an old production defect. Earlier observer-only build/
discovery failures are preserved in `build-instrumentation*.log` and were corrected;
only final positive-controlled results support acceptance.

Two additional regressions were demonstrated failing before their fixes:
`component-bound-before-test.log` shows an at-host-ceiling component reached native
I/O before cumulative allocation charging; the final component test now refuses
`resource_limit` before the temporary copy. `target-alias-before-test.log` shows
regular target aliases taking path_denied; the final target-alias test now requires
unsafe_path before unrelated target-type refusal. The initial hard-link test's
fixture created a new link after admission, legitimately changing ctime; the fixture
now creates its alias before admission (`focused-initial.log` preserves that run).

## Frozen delivery

`baseline-filesystem.json` freezes the complete pre-edit regular-file hashes/modes
and symlink targets; only root `.git/`, root `build/` and this new evidence directory
are excluded. Root-only exclusions preserve nested directories with those names.
Raw starting status/diff/HEAD/branch and existing owned snapshots are retained.
`final-filesystem.json`, `ownership-audit.json`, `candidate-manifest.json`,
`candidate-source/`, `owned.diff` and `identity.json` freeze the reviewed candidate.
`artifact-manifest.json` hashes all evidence except itself. No source or evidence
edits follow ownership release.

The authoritative final target-alias pass is separately archived under
`passes/03-final-target-alias/`; its manifest binds the final candidate manifest,
source freeze and all final command outputs. `pass-history.json` records two
superseded successful pass summaries. Their standard raw output/freezes were
overwritten by subsequent runs before separate archival was requested, so those
summaries are not independently hash-bound evidence and do not support review.
The original failed regression logs and final sensitivity restoration are retained.
