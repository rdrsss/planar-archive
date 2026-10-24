# X04-A integrated native capture evidence

Status: A1 native metadata admission and A2 strict bounded profile capture are
independently approved. X04-A3 is the integrated native verification candidate,
awaiting independent review. It adds one composition case and no product code.
The mandatory external-profile gate remains open.

The candidate is based on shared branch `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The prior inventory is 488: all 430
X03-B3 names, 21 A1 admission cases and 37 A2 profile cases. A3 retains every name
and adds exactly one case, making 489. The only source change is the added case in
`src/engine/capture/profile.t.cpp`; implementation, CMake and vendors are unchanged.

## Native obligations and evidence

All test names below are Catch2 names, registered by CTest with the `replacement.`
prefix. The source-owned cases live in
[`admission.t.cpp`](../../src/engine/capture/admission.t.cpp) and
[`profile.t.cpp`](../../src/engine/capture/profile.t.cpp). Contracts remain
[X-CAP](external-profile-contract.md#x-cap--strict-capture-and-trust) and
[X-LIMIT](external-profile-contract.md#x-limit--cumulative-ceilings-and-errors).

| Obligation | Current native evidence | Boundary of the claim |
| --- | --- | --- |
| Strict exact profile schema, integer version, UTF-8/NUL/BOM and trailing grammar | `profile capture strict schema refuses malformed and ambiguous JSON`, `integer versions have no machine integer classification ceiling`, `UTF8 NUL BOM and escaped scalar validation is strict`, `escaped keys and portable paths use decoded library strings` | Pinned Glaze owns parsing/decoding; unknown/duplicate/escaped keys and arbitrary-size integer classification are covered. No external Lua executes. |
| Bounded parsing before retained growth | Profile cases `input bound checked before observed profile read`, `module count zero sixtyfour and sixtyfive`, `path byte and segment bounds include escaped spellings`, `oversized raw escaped string and key refuse bounded scratch`, `node and depth ceilings refuse resource exhaustion`, `exact decoded key limit and surrogate escape boundaries`, `exact structural node ceiling before container growth` | Profile65536 bytes; 0..64 modules; depth4/nodes132; decoded strings/paths1024; portable segments128. Raw string scratch and strict grammar reasoning are independently reviewed in A2. |
| Ordered exact source closure; no discovery or bytecode | `retains exact whitespace modes and module list order`, `portable suffix segment namespace and alias policy`, `allows explicit stdlib directory and excludes unlisted siblings`, `source only modules reject bytecode NUL invalid UTF8` | Exact profile whitespace, declared module order, normalized modes and original bytes are retained. Unlisted siblings are irrelevant. |
| All portable policy and selected identities precede module reads | `validates full portable list before any module contents`, `selects all module metadata before any module contents`, `selected hard link aliases refuse before first module read`, `profile module hard link alias refuses` | Positive-controlled API observer proves watched reads are absent at refusals; no universal kernel trace is claimed. |
| Root containment, selected aliases and metadata-only staged admission | Admission cases `roots reject equality and both ancestry directions`, `standalone roles preserve registry colocation policy`, `selected hard links refuse every supplied file role`, `staged trusted selections check earlier supplied aliases`, `export targets retain absence and reject trusted overlap`; profile cases `staged root file and target checks retain initial receipts`, `staged overlap missing and aliases fail terminally`, `staged admission consumes no supplied file content` | Native seams check supplied slots and retain original receipts. Registry directory colocation is permitted, observations remain separate, and export target overlap refuses. Actual CLI timing and service use remain later integration. |
| Wrapper composition for captured module versus staged file | New `profile capture staged module aliases refuse without reads and poison owner` | Both registry and observation roles hard-link a listed module before capture, establish valid owner/retained bytes, positively observe a direct read, reset the observer, then require exact logical `unsafe_path`/`profile` refusal and zero watched reads. Both `verify` and `snapshot` subsequently refuse `capture_changed`; retained bytes stay original. |
| No-follow paths, ancestors, modes, selected identity, unrelated sibling tolerance | Admission symlink/ancestor/mode/sibling tests and profile cases `final intermediate and root symlinks refuse`, `receipt modes include normalized equal file mode changes`, `selected directory ancestor replacement refuses`, `directory modes and trusted root replacement retain receipts`, `repeated seals retain selected original bytes after failure` | Original descriptor/name/ancestor receipts are retained through repeated checks. These are cooperative filesystem guarantees. |
| Bound descriptor before allocation/read; bounded rereads and terminal owners | Profile cases `bound native seam checks original descriptor before ceiling and read`, `bounded native sealing reread refuses growth before observed content read`, `bounded acceptance rereads preserve original charged byte ceiling`, `bounded native cached read honors newly tighter caller ceilings`, staged descriptor binding/replacement tests and move/self-move cases | Per-module1MiB, aggregate original module8MiB and original charged-byte reread bounds are exercised. A2 preserves the demonstrated growth regression and exact fix. |
| Local metadata count/accounting bounds | Admission cases `move failed and bounded states refuse safely`, `host and private receipt key bounds reject before path opening`, `charges a host component before its temporary copy` | Local65 trusted/10000 selected files,64MiB accounting and internal receipt ceiling are tested/reviewed. Whole-invocation sharing with existing input capture is not implemented. |

[A1 independent review](rounds/V5-02-X04-A1-review.md) and
[A2 independent review](rounds/V5-02-X04-A2-review.md) retain the exact product
candidate identities, boundary reasoning, meaningful failing regressions and
prior focused results. Their old full-inventory counts were not full-suite runs.
A3 carries their current product bytes through the full gate below.

## Executed integrated gate

Both serialized configurations run `cmake --preset debug`/`release`, then
`cmake --build build/rewrite-<configuration> --target all planar_tests`. The release
preset is **RelWithDebInfo**. Each inventory contains489 distinct names and retains
all488 prior names. Exactly one full CTest suite per configuration executes
**489/489**, with zero failures, errors, skips or disabled cases and every status
`run`. Actual names are compared between inventory and JUnit. Absolute JUnit
paths and immediately copied `LastTest.log`/`CTestCostData.txt` are retained.

Both configurations also pass pinned formatting/Doxygen for104 active sources,
canonical vectors (12 hand byte, 20 upstream hash and 41 canonical/domain),
10 record-v1 and 2 request-v2 vectors, four inherited runtime/CLI exact
source audits, and exact forward/reverse substitution checks for five generated
storage/export families. Both final `all planar_tests` builds report
`ninja: no work to do.`. Build configuration, binary/object identities, raw commands,
UTC timestamps, durations and exit codes are retained under
[`tests/native/v5-02-x04-a3-evidence/attempt01/`](../../tests/native/v5-02-x04-a3-evidence/attempt01/).

The complete pre-edit filesystem manifest excludes only root `.git/`, root
`build/`, and the A3 evidence directory. Nested historical `build` directories
remain protected. A pre-command source freeze includes the test/support candidate;
complete filesystem equality is checked before/after every gate command. Product,
test, support and configuration bytes remain unchanged through both gates. Current
status prose and this evidence map are finalized afterward; the final stability
and ownership audit explicitly distinguishes those documentation edits. AGENTS.md
remains the CLAUDE.md symlink. This round does not rerun historical failing variants
or claim that archived failures were produced by the current candidate.

## Remaining integration obligations

The native owner provides initial/staged metadata admission, original content and
repeated receipt checks. It does not establish actual known shipped-slot ordering
after early replay miss/before external evaluation, or late extension-slot ordering
after sealed CLI11 parsing/before handler/read service. External registration,
result authorization and final under-lock acceptance/commit receipt composition
remain X05/X06/X07 responsibilities. No Lua runs under lock.

Whole-invocation64MiB/10000 budget sharing with existing captures remains open;
local owner limits do not prove cumulative service accounting. Effective provenance,
interpretation-v2, profile acceptance/history, public profile CLI/cutover and X08
acceptance remain open. Public init still creates format1. Execution/RA01 and
Linux acceptance remain future work. This gate runs macOS ARM64 only.

The unchanged test-only observer records selected device/inode content operations
at read/pread/readv/mmap APIs, with positive controls; it is not universal kernel
tracing. Retained receipts cannot prevent hostile writes after a check or detect
every change-and-revert. This verification candidate awaits independent review
and does not close the mandatory external-profile gate.
