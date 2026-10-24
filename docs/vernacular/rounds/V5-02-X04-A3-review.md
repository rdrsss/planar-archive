# V5-02-X04-A3 independent review

Disposition: **approve** for the integrated native X04-A capture gate. No blocking
findings. A1 and A2 remain independently approved; this review closes their native
integration gate only. The mandatory external-profile gate remains open.

Reviewed in the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, against CLAUDE.md, implementation,
roadmap/v5 direction, the X04-A/A3 briefs and search disposition, A1/A2 approvals,
X-CAP/X-LIMIT and the bounded external-profile implementation path. Reviewer writes
are confined to this report and `tests/native/v5-02-x04-a3-review-evidence/`.
Product, tests, author evidence and status documents remain unchanged. No Planar
tool/skill/queue, installed binary, operator data, subagent, commit, installation,
publication or merge was used.

## Exact candidate and evidence integrity

| Identity | SHA256 |
| --- | --- |
| Author candidate manifest | `90ef44013b3031783bd8f54681c16e6a670673bdf8f155a18359917eb999871d` |
| Author owned diff | `824f7b82f35e3329c88a83118bf98aa04fa297d0ff5c9380519f0c092f7de2cb` |
| Author artifact manifest | `e9544b10fa33ecf96d01166844d33cb6ed60fe72a9ada002b94fc37543831b9d` |
| Author tested source freeze | `d5aa29591c76c5efc7130fec4bbf0eb64a97699f419579336e5f338e69c24b47` |
| Author debug pass manifest | `9766a540e7d797e00ed5bcf8a15f4a3c40f15fc176ca74b35f5e7aa40a3d7f8b` |
| Author RelWithDebInfo pass manifest | `1ba1cf6c25920d08cf29deb88e8d23f6310b25238fe84b9647c4b592bfad3c8a` |

Independent checks match all **10 candidate paths** to live and final-owned copies,
all **240 author artifact entries**, both **26-entry pass manifests**, **134** tested
source copies, and all **15,988 protected final filesystem entries**. Both live
build identity sets match: **711 debug** and **708 release** entries, including
executables, objects, module files, cache, compile commands and build graph.
Configuration caches confirm Debug and RelWithDebInfo respectively.

Recomputed baseline-to-final differences equal the ten assigned paths. Removing
the single new 28-line test insertion reproduces the complete prior profile test
file exactly; all 37 earlier cases remain and the file now has 38. All **1,794**
source/build-tool/vendor entries selected from A2's final filesystem manifest match
the A3 pre-edit baseline. The tested freeze differs from final files only in eight
assigned Markdown documents finalized after the gates. Root-only `.git/` and
`build/` exclusions retain nested historical build paths; AGENTS.md remains the
CLAUDE.md symlink. These checks were repeated after independent verification and
at final release. The reviewer artifact manifest freezes this report and reviewer
evidence, excluding itself.

## Composition assessment

The new case at `src/engine/capture/profile.t.cpp:445` meaningfully covers the
missing profile-owner composition. For each registry/observation role it creates
a hard link to the listed module **before** capture, obtains a valid captured
owner and retained snapshot, and successfully verifies it. This avoids confusing
alias refusal with a link-induced ctime change. The supplied path is outside the
trusted root, so selected module identity is the relevant refusal condition.

A direct read through that alias positively exercises the unchanged inode observer.
The second `watch` resets its count before staged `admit_file`. The test requires
`unsafe_path`, field `profile`, the exact closed overlap message and zero watched
read attempts, then requires both `verify` and `snapshot` to refuse
`capture_changed`, still without reads. The earlier retained module bytes remain
original. `profile.cpp:345` delegates to retained metadata admission and poisons the
owner on failure; the existing terminal checks make the tested behavior explicit.
Both loop roles execute in each passing 118-assertion run.

The A1/A2 approved product bytes and boundary reasoning remain applicable. The
coverage map correctly distinguishes strict local profile/module capture, bounded
original-content rereads, metadata-only staged admission and retained receipts
from later service composition. No new product behavior or unreviewed format,
parser, library, CLI or Lua change is introduced by A3.

## Full gate evidence and independent verification

The author's raw inventory, JUnit, LastTest and CTestCostData agree on exactly
**489 distinct executed names per configuration**, preserving all **488 prior
names** and adding only the new composition case. Every JUnit status is `run`,
with zero failures, errors, skips or disabled cases. LastTest contains exactly 489
passed executions with the same names; CTestCostData also has those 489 names.
Raw CTest elapsed times are **319.13 seconds debug** and **210.99 seconds
RelWithDebInfo**. Recorded commands are serialized and contain exactly one full
suite per configuration. Immediate raw copies and source-freeze links are retained.
No second full suite was necessary.

Both author configurations retain passing canonical vectors (12 hand-byte,
20 upstream-hash and 41 canonical/domain), 10 record-v1 and two request-v2 vectors,
four runtime/CLI source audits, 104-source pinned formatting/Doxygen, and final
`all planar_tests` no-work builds. Independently recomputed forward and reverse
substitutions for all five storage/export generated families match original and
generated hashes/bytes in both builds.

Independent commands, UTC times, exits and raw output are retained under
`tests/native/v5-02-x04-a3-review-evidence/`:

1. Both serialized `all planar_tests` builds reported `ninja: no work to do.`.
2. Both fresh CTest inventories contain exactly the author's 489 names.
3. Exact-name CTest runs of the added composition case pass **1/1** in debug and
   **1/1** in RelWithDebInfo, each with **118 assertions**, all status `run` and
   zero failures/errors/skips. JUnit and immediate LastTest/CTestCostData are saved.
4. All four inherited runtime/CLI audits were independently regenerated into
   scratch for each configuration. Each report equals the author's report,
   including exact runtime harnesses and embedded originals/assembly.
5. Both subsequent `all planar_tests` builds again reported no work. Debug pinned
   formatting/Doxygen independently passed for **104 active source files**.
6. Candidate hashes were checked after every product command. Final integrity
   checks also confirm unchanged author evidence, protected filesystem and both
   live build identity sets.

One initial reviewer audit assertion expected older CTest summary wording
(`100% tests passed, 0 tests failed out of 489`). This CMake version reports
`100% tests passed out of 489`. It failed before any product command, after the
JUnit and LastTest checks had passed. Only reviewer runner wording was corrected.
The initial runner, disclosure and a read-only reproduction of its raw stderr
are retained; no candidate or product failure was hidden or repaired.

## Limits carried forward

Approval covers native X04-A on macOS ARM64, with debug and RelWithDebInfo full
gate evidence and independent focused execution. Linux remains untested. The
observer establishes positive-controlled read/pread/readv/mmap API observation;
it is not universal kernel tracing. Cooperative receipts cannot prevent hostile
writes after a check or detect every change-and-revert.

Whole-invocation 64 MiB/10000 budget sharing, actual known/late CLI slot ordering,
external Lua registration, result authorization, effective provenance,
interpretation-v2/profile acceptance and history, final under-lock receipt/commit
composition, public profile CLI/cutover and X08 acceptance remain open. Public
commands remain render/init/history/export/load and public init creates format1.
Execution/RA01 and Linux acceptance remain future work. This approval does not
close the mandatory external-profile gate or authorize those later integrations.
