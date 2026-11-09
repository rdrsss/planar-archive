# Planar CLI

The rewrite provides declaration rendering and acceptance, recorded history and export,
and draft execution commands for run ownership, recovery, checks and review. The
execution surface is integrated under the operator's implementation-first
workflow; it has not passed the final integrated review or acceptance gate. See the
[roadmap](vernacular/roadmap.md#current-status) for the exact delivery checkpoint.

Render is database-free. Ledger paths are explicit; new ledgers use format3.
Execution commands refuse older ledger formats. The source-built binary is not
installed over an existing Planar installation.

CLI11 separate-token parsing, native paths and Glaze diagnostics remain current;
v5's illustrative positional examples are not additional syntax.
`planar --help` prints fixed root usage and exits 0 without Lua registration.
Parser usage errors precede embedded registration failures. Workspace selector
and path validation follow successful catalog validation. Ordinary commands validate
the complete embedded catalog and invoke its retained Lua callable through the
compiled family services. Invalid shipped registration returns redacted `internal`
(exit 4); bounded registration exhaustion returns `resource_limit` (exit 3).
Select the source-built binary explicitly, for example
`build/rewrite-debug/bin/planar`. There is no version command or legacy CLI.

## Draft execution workflow

Load declarations into a ledger, select a job with `activate`, inspect `status` or
`preview`, then `claim` a role. The claim result contains the run token and the
retained dispatch packet. The driver runs the session and owns the working tree.
Use a stable request ID for each mutation: retrying the same request returns its
recorded result; reusing the ID with a different request refuses.

| Command | Required options | Optional options |
| --- | --- | --- |
| `activate` | `--ledger`, `--job`, `--expect-revision`, `--expect-job`, `--request-id`, `--actor` | |
| `status` | `--ledger` | `--role` or `--request-id`; `--workspace` and `--registry` together |
| `preview` | `--ledger`, `--role` | `--since` retained render digest |
| `claim` | `--ledger`, `--job`, `--role`, `--request-id`, `--actor` | `--provenance` |
| `takeover` | `--ledger`, `--job`, `--role`, `--run-id`, `--request-id`, `--actor` | `--provenance` |
| `refresh` | `--ledger`, `--run`, `--expect-fp`, `--request-id`, `--actor` | `--observations` |
| `checkpoint` | `--ledger`, `--run`, `--observations`, `--request-id`, `--actor` | |
| `release` | `--ledger`, `--run`, `--reason`, `--request-id`, `--actor` | `--observations`, `--provenance` |
| `recover` | `--ledger`, `--role` | `--root` and `--base-root` together |
| `check` | `--ledger`, `--run`, `--root`, `--base-root`, `--scratch`, `--request-id`, `--actor` | `--provenance` |
| `check-recover` | `--ledger`, `--run-id`, `--reservation`, `--confirm-stopped true`, `--request-id`, `--actor` | |
| `verdict` | `--ledger`, `--run`, `--verdict`, `--request-id`, `--actor` | |

`--run` takes the claim token; `--run-id` takes its numeric run identifier.
`--expect-fp` and `--since` take `xxh3_128:` digests. `release --reason` is
`released` or `interrupted`. Check roots are directory paths; recovery roots use
`logical_id=directory`. The current, base and scratch directories must be physically disjoint; aliases and
ancestor overlap are refused.

Checkpoints are unverified session notes and do not complete work. `refresh`
acknowledges current work for the same run; `recover` reads retained context
without acknowledging it. A lost claim response can be recovered through
`status --request-id` or `history --request-id`. `history --event` expands the
retained event; ordinary status, history events and recovery do not reveal tokens.

`check` runs the accepted required check set, compares current and base outputs,
and records evidence. A recorded failing or inconclusive outcome still exits 0;
the response distinguishes recorded evidence from a completed role. A stopped
check leaves a durable reservation: once the driver confirms the process stopped,
`check-recover` interrupts it without completing the role. A reviewer submits a
JSON verdict against exact output and policy identities; current rejections can
admit repairs or open a blocker at the job's review limit. Review readiness requires
every targeted worker to be done; retained completion evidence alone does not
make an active worker run reviewable. Recreated reviewer roles cannot inherit
approval from an earlier generation.

Checks run trusted registry commands with bounded time, output, cwd and environment
and no OS sandbox; escaping descendants are not tracked.

The authoring bundle's `orchestrator --ledger PATH` renders live next-action
guidance. Its existing database-free form uses `--workspace`, `--registry` and
`--job` together. The production bundle does not expose authoring verbs.

The exact interfaces and failure order are defined by the
[execution contract](vernacular/execution-contract.md) and the draft
[recovery](vernacular/execution-contract-r16.md),
[checks](vernacular/execution-contract-r17.md) and
[review](vernacular/execution-contract-r18.md) contracts.

## Render preview

```sh
build/rewrite-debug/bin/planar render \
  --workspace fixtures/render/quota/workspace/workspace.lua \
  --registry fixtures/render/quota/registry/quota-tools.json \
  --root source=fixtures/render/quota/current \
  --capture-outputs --base-root source=fixtures/render/quota/base \
  --job split --role cli_worker \
  --observations fixtures/render/quota/observations/recovery.json \
  --budget-bytes 32768
```

| Option | Meaning |
| --- | --- |
| `--workspace FILE_OR_NAME` | Required declaration path or explicit-environment name; references its spec and all listed jobs. |
| `--registry FILE` | Required, explicitly trusted driver JSON describing toolchain/check definitions. |
| `--root ID=DIRECTORY` | Required existing source directory; ID must match the workspace root. A spelling that names a regular file (or a link to one) followed by `..` is `io` "Cannot resolve explicit root directory" (exit 4) on macOS and Linux. |
| `--job ID` | Required declared job selection. |
| `--role ID` | Required role in that job. |
| `--capture-outputs` | Capture complete current and base trees under declared exclusions. Requires `--base-root`. |
| `--base-root ID=DIRECTORY` | Base tree with the same logical root ID; requires `--capture-outputs`. |
| `--observations FILE` | Optional untrusted JSON assertions/checkpoint; defaults to empty observations. |
| `--budget-bytes N` | Preview budget override, positive decimal without leading zeros, at most 1,048,576. |

For `--root` and `--base-root`, a final-root alias spelled `alias/file/..` (a link to a root
directory, then a regular file, then `..`) is that same `io` exit-4 refusal as of X08-C01;
earlier it was `path_denied` "Explicit root is a symbolic link target" (exit 3) on macOS.
No message text changed.

CLI11 parses the command and option grammar; Planar supplies fixed help and
canonical usage diagnostics without embedding library messages or host values.
`--help` is accepted only as the sole argument. No shorthand, subcommand help
or positional tail (including a `--` marker) is supported.

Options appear at most once. Required options appear exactly once. Options take
separate values; `--option=value`, stdin inputs, workspace discovery and config
lookup are unsupported. Render paths resolve from one invocation cwd snapshot
through actual filesystem ancestry, then canonicalize. Final input file links,
final root aliases, root overlap, special files and captured path aliases are
refused. Ancestor symlinks may resolve to a valid canonical parent; `..` keeps
filesystem semantics. See the [capture contract](vernacular/render-contract.md#capture-boundary).

A workspace value beginning with `/`, containing `/`, or ending case-sensitive
`.lua` is an explicit file and completely ignores `PLANAR_WORKSPACE_DIR`. Otherwise
it must match `[A-Za-z0-9][A-Za-z0-9_-]{0,63}` and expands to
`<PLANAR_WORKSPACE_DIR>/<name>/workspace.lua`. Only names read that variable, once;
it must be explicitly set, nonempty, absolute, valid UTF-8 and NUL-free. Its base
is normalized lexically without expanding links, tilde or variables. There is no
HOME fallback. Invalid bare names or environment give usage (exit 2).

After catalog validation and before command services, native preparation validates
selected private paths and snapshots cwd once if any is relative. All-absolute
invocations need no cwd lookup. Paths retain their spelling for physical render
traversal; load uses the separate lexical absolute target in its retained request intent.
Invalid path text gives `path_denied` (exit 3); 64 MiB input/join/normalized limits
give `resource_limit` (exit 3). Cwd lookup failure gives redacted `io` (exit 4).
Workspace naming applies to commands accepting a workspace selector, including
render, load and status's working-declaration options. Init creates an explicit
ledger. See the [workspace contract](vernacular/command-contract.md) and
[load contract](vernacular/load-command-contract.md) for selection, replay and
per-command option boundaries.

Without capture, source/base files are not scanned and no output identity is
emitted. The source directory must still exist and satisfy root confinement.
Reviewer previews explicitly say exact targets are unavailable. Capturing an
empty tree instead computes its empty output identity. Only captured packets
include actual source and base differences.

Success writes exactly one canonical UTF-8 JSON envelope plus LF to stdout,
with `version`, `kind`, `packet` and `manifest`; stderr is empty. Packet Markdown
states that no run is active and checks have not been executed. The manifest
includes scoped work, fragment and captured file identities and the exact
selected check definitions. Hashes use versioned canonical encoding and
seed-zero XXH3-128, rendered as `xxh3_128:` followed by 32 lowercase hex digits.
Driver observations remain explicitly unverified assertions.

| Status | Meaning |
| --- | --- |
| 0 | Help or complete command success. |
| 2 | Invalid invocation or unsupported operation. |
| 3 | Invocation semantics, declaration/schema/capability/resource/capture or ledger format refusal. |
| 4 | I/O, internal, ledger corruption, busy or full-storage failure. |

Failures emit one canonical JSON error on stderr and no stdout before output
writing begins. A short or failed stdout write returns an I/O failure; discard
any partial output. Render creates no state, ledger, cache or output files.
Shell redirection is the caller's choice.

The complete schemas, identity recipes, diagnostics, exclusions, resource limits
and budget behavior are authoritative in the
[render contract](vernacular/render-contract.md) and
[declaration contract](vernacular/declarations.md). Render itself performs no claim, refresh, check execution or approval operation.
Those operations use the separate draft execution commands described above.

Canonical JSON uses pinned Glaze string spelling: short C0 escapes where
available and uppercase hexadecimal otherwise, sorted UTF-8 keys and no extra
whitespace. Checkpoint accounting and Markdown JSON descriptors use this same
codec. Output failure diagnostics also use the codec; if native allocation
prevents diagnostic encoding, the process returns status 4 with no fabricated
JSON fallback.

`init` requires both explicit options exactly once:

```sh
build/rewrite-debug/bin/planar init --ledger /existing/real/parent/new.db --workspace-id sample
```

The workspace ID is ASCII `[a-z][a-z0-9_]{0,63}`. Initialization creates only a
new empty version-one ledger, with one initialization event and no active
revision. It creates no parent directories, declarations or checkout. There is no
default ledger or HOME/database discovery. Relative targets resolve from invocation
cwd; every original parent component must traverse an existing real directory
without following links. Missing ancestors followed by `..` still refuse. Any
existing target, including an empty file, directory, symlink or dangling link,
returns `ledger_exists` without replacing it. Invalid workspace/path shapes return
`schema`; unsafe or missing parents return `path_denied`. These semantic refusals
exit 3; malformed option grammar exits 2.

After native commit and no-replace publication succeed, stdout is exactly the
canonical Glaze envelope plus LF:

```json
{"active_revision":"none","event":"1","kind":"init","ledger":"<32 lowercase hex digits>","version":1,"workspace":"sample"}
```

Initialization errors use the same error envelope and empty stdout. Storage
messages omit host argument bytes; I/O details are `{"operation":"ledger"}`.
If publication is uncertain, exit 4 reports that the ledger may be published and
instructs the caller to inspect the explicit target before retrying. It does not
delete or retry a possibly published ledger. Explicit recovery requirements also
exit 4. A failed stdout write can follow successful publication; inspect the
target rather than treating missing stdout as proof that no ledger exists.


`history` reads one verified snapshot from an explicit existing ledger:

```sh
planar history --ledger FILE [--after-event ID] [--limit N]
planar history --ledger FILE --revision ID
planar history --ledger FILE --request-id KEY
planar history --ledger FILE --at-event ID
```

Paging defaults to 50 events, allows 1 through 100, and uses an exclusive
positive event cursor. Events ascend by sequence; `next_after` appears only when
more events remain. Selectors exclude one another and both paging options.
IDs are canonical positive decimal strings up to signed-64 maximum; explicit
zero, signs, leading zeros and overflow are invocation errors. Request keys use
ASCII `[A-Za-z0-9_.:-]{1,128}`. Every option uses a separate token exactly once.

The envelope is `{version:1,kind:"history",ledger,active_revision,last_event,data}`.
All database IDs are strings; absent IDs use `"none"`. Outer head/last-event
always describe the current snapshot, including when reading older state.
Page data contains `selector:"page"`, `events` and optional `next_after`; each
event has `sequence,type,version,revision,request_id,actor,occurred_at,payload`.
Payload is a structured public event value. Revision data contains
`selector:"revision"`, `revision`, complete `interpretation` and `versions`.
Revision metadata has `id,parent,source,registry,stdlib,interpretation,versions,actor,accepted_at`.
Request data contains `selector:"request"` and the decoded saved load `result`.
Private request bodies and host intent arguments are never projected. State data
contains `selector:"state"`, `event`, `state:{active_revision}` and matching
`revision` metadata (or `"none"` after initialization).

Reads do not activate revisions, execute Lua or consult current declaration policy.
Unknown valid selectors return `revision_not_found`, `request_not_found` or
`event_not_found`, exit 3. Unsupported recorded interpretation returns
`unsupported_interpretation`, exit 3, while page, request and state reads remain
available. Malformed stored data returns `ledger_corrupt`, exit 4. Storage errors
use the same redacted mapping as init. Entire stdout including its final LF is
bounded to 16,777,216 bytes; an oversized view returns `resource_limit` with
`{limit:"output_bytes",maximum:16777216}` and empty stdout, without truncation.

`export` publishes the exact recorded source tree into an explicit new directory:

```sh
planar export --ledger FILE --revision ID --target NEW_DIRECTORY
```

All three options are required, supplied once as separate tokens. Revision IDs
are canonical positive signed-64 decimal strings. Original path components must
traverse existing real parent directories without symlinks; relative paths resolve
from invocation cwd. No parent directories are created and every existing target
kind refuses with `target_exists`, exit 3. Export never merges or overwrites.

Success is canonical JSON plus LF:

```json
{"kind":"export","ledger":"<32 lowercase hex digits>","revision":"1","source":"12","version":1}
```

Database IDs are strings. The exported files preserve recorded bytes, with file
modes 0644 or 0755 and directory modes 0755. Membership contains only the recorded
source closure, with no injected registry or stdlib. Export reads one verified
snapshot, leaves the ledger unchanged, and can export older or reverted revisions
and revisions with an unsupported interpretation. It does not execute declarations.

Failures have empty stdout and redacted diagnostics. Unknown revisions,
`target_exists` and `path_denied` exit 3. Storage errors use the ledger mapping;
export I/O failures exit 4 with `details:{operation:"export"}`. If staging cleanup
is incomplete, `export_incomplete` exits 4 with
`details:{operation:"export",cleanup:"incomplete",location:".planar-export-<32 lowercase hex digits>"}`.
Inspect this sibling name in the explicit target parent; the name does not prove
that its current occupant belongs to this invocation or is safe to delete.

An uncertain native publication or result-encoding failure after publication
returns exit 4 and `Export may be complete; inspect the explicit target before retrying`,
with operation `export`. A failed stdout write after successful export uses the
same guidance with operation `stdout`. Both diagnostics are buffered before export
can publish. Inspect the target before retrying; missing stdout does not establish
that publication failed. Export never automatically deletes a published target.

## Load candidate

```sh
build/rewrite-debug/bin/planar load --ledger /absolute/ledger.db \
  --workspace /absolute/workspace.lua --registry /absolute/registry.json \
  --expect-revision none --request-id first-load --actor driver
```

All six options are required text values, occur once and use separate tokens.
`--expect-revision` is exactly `none` or a canonical positive signed-64 decimal.
Request IDs contain 1–128 ASCII letters, digits or `_.:-`; actor assertions contain
1–256 non-NUL UTF-8 bytes. Workspace names use the same explicit
`PLANAR_WORKSPACE_DIR` expansion as render. Load uses absolute lexical workspace
and registry targets for both request identity and capture. Ledger paths retain
parent spelling and require traversable real parent directories. No source/base mappings,
job selection, observations or profile options are accepted.

Load routes and interprets every declared job in the retained command VM, captures
only declarations and their text-reference closure plus the registry, and records
exact original embedded provenance. It returns the native saved load envelope plus
one LF. Identical request intent returns the original saved bytes before reading
current inputs or comparing current head. Reusing the ID with changed actor,
expected head, workspace or registry intent returns `request_conflict`. A fresh ID
is required for new acceptance intent. Named and explicit paths match only when
they expand to identical lexical targets.

After lost output or a diagnostic saying load may be complete, retry the same
request ID with its original arguments or inspect `history --request-id ID`.
A stdout write failure is exit 4 and may leave partial output; discard that output.
Precommit refusals preserve history and leave failed IDs reusable. The exact
[load contract](vernacular/load-command-contract.md) defines the finite services,
completion receipt, diagnostics and remaining acceptance gates.

## With-profile envelope (X07)

The with-profile envelope is `planar with-profile --stdlib-profile FILE COMMAND [OPTIONS]`.
Selected help has two counted orders: `planar with-profile --stdlib-profile FILE --help`
and `planar with-profile --help --stdlib-profile FILE`. The envelope is
parsed by CLI11 in `src/cmd/planar/envelope.cpp`; the only code that runs
before CLI11 is the whole-invocation `--name=value` syntax guard over all argv
tokens including the tail. Both refusals are usage errors with exit 2, and the
profile is not accessed.

- The selector is a scalar option (a duplicate is usage), nonempty, not starting
  with `--`, valid UTF-8 without NUL. `./--help` is a valid selector.
- The prefix is exactly three tokens (four when `--help` is counted). Selected
  help exits 0 only for the two counted-help rows; other help placements are usage.
- An option before the selector, or a root `--` terminator, is prefix usage.
- The selector is normalized lexically against one cwd snapshot, with no
  existence check, under a 64 MiB path ceiling. The request ceiling wins for load.
- Order after the prefix: probe (shipped signature or opaque extension tail),
  capture, known-slot admission, registration, final parse from the sealed
  catalog with the same validators, then dispatch. A broken profile is reported
  before an unknown command. A known shipped-signature failure is usage with
  no extension fallback.
- Known slot admission (before registration): render refuses overlap and alias
  as `unsafe_path`; init is alias-only; history refuses a ledger alias; export
  refuses a ledger alias or target overlap; load refuses its known slots through
  the production CLI. A missing ledger is exit 3.
- Load replays an equal saved request before any profile capture, conflicts on
  an unequal body before profile access, and refuses a format1 ledger before
  profile access.
- Registration and preparation refusals print only the closed message set
  of the [external profile contract](vernacular/external-profile-contract.md); fields,
  host paths and tracebacks are never printed. Preparation usage prints the
  public usage bytes; preparation semantic errors exit by code; malformed
  trusted payloads are redacted to internal protocol.

Public `init` creates a format3 ledger under the
[execution contract](vernacular/execution-contract.md#ex-1-format3).
The [X07 addendum](vernacular/x07-addendum.md) records the earlier format2
profile cutover; supported recorded-format readers remain separate from the
current creation format.

`with-profile` is the only profile selector. No environment variable selects a
profile, and Planar does not discover a `stdlib.d` directory in the working
directory or `HOME`. A root-level `--stdlib-profile` flag is usage (exit 2), and an
extension verb without `with-profile` is usage (exit 2). Only an explicit
`with-profile --stdlib-profile FILE` prefix can select a profile. The X08 binary
cases for these rules are `x08.e01_no_discovery` and `x08.e01_registration_policy`;
the envelope-row and closure-separation cases are `x08.e02_envelope_rows_binary`
and `x08.e03_closure_separation_binary`. See the
[X08 acceptance gate](testing.md#x08-acceptance-gate) for how they run.
