# X05-A3-C03 allocation-safe seal locations

Status: implementation candidate requiring fresh independent combined
A3+C01+C02+C03 review. C03 addresses the independently observed A3-C02-R5;
this report does not approve its author’s work or close X05-A.

Shared checkout: `dev/planar-v2-vernacular`, HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Coordinator owns the round;
coder owns runtime.cpp, external_registration.t.cpp, new C03 support/evidence,
and the six authorized current reference files. Existing dirty work and every
historical evidence artifact are preserved. No guard/API/build/codec/vendor/
shipped Lua change, legacy Planar tool/role/skill/queue, installed binary,
operator data, commit, installation, publishing or merge was used. Direct
build ownership remained serialized throughout this round.

## R5 reproduced before correction

`red-actual-location-allocation/` contains a newly compiled reversible copy of
the original failing production source. Its generator is copied from the
immutable C02 review generator with only output destination changed; that review
and its original binary/source remain unchanged. The detector registers ten
valid projections, arms replacement global new immediately before production
projection10 location formatting, and refuses the next actual allocation.
Pinned libc++ requests 32 bytes and registration refuses with:

```
resource_limit / External profile resource limit exceeded / catalog.projections[9]
```

The detector exits 1 as expected. The disabled-allocation-fault twin succeeds;
both report zero Lua live bytes after owner destruction. `commands.json` records
argv, expected status and actual status; `delta.json` records exact reversible
substitutions and source/copy hashes. This is a controlled real native allocation
refusal, not natural host exhaustion or an unsafe acceptance.

## Production correction

`module_seal_location` now only selects a trivial native `{kind,index}` token.
Index 0 means active module fallback. No text formatting, allocation, or string
assignment occurs in audit/freezer root selection. The guard API, one shared
inspector/memo, audit-before-any-freeze ordering, graph depth and root order are
unchanged. Both D/F roots still map to the same original per-kind index, including
five shipped commands and prior modules. Before-root and identity-union failures
use the module token. Embedded and repeated-root seams keep their prior semantics.

Before module processing, two independent module-field strings per module are
staged in a local vector while registration_field remains profile. The complete
vector publishes by move only after all stages succeed. Each module installs its
active string by swap. After descriptor collection has advanced registration_field,
a second independent staged string installs the correct seal fallback by swap
before any protected seal operation. This handles the original stale-field cause
without relying on short-string optimization or allocating a catch fallback.

The existing checked cumulative native budget charges, before reserve/staging,
`module_count * (sizeof(module_fields) + 2*32)`: vector element objects and two
independently owned field payload reservations. Module count is already bounded 64;
maximum `profile.modules[64]` length is 19, and 32 reserves its NUL and conservative
payload room. Existing 512 MiB accounting remains cumulative with no refund/reset.
This is a logical-capacity reservation, not an allocator-overhead or RSS bound;
no new quota is introduced. Empty profiles charge 0 for this staging.

The protected seal call has one catch-all error boundary. Exhaustion/Lua memory,
stack flag, bridge_error, Lua status/terminal and native exception failures all
render the pending token outside Lua. Successful and nonseal calls clear the
pending token. Rendering constructs a temporary and swaps only after success.
An actual rendering bad_alloc leaves the preinstalled active module field intact
and reaches the existing external resource diagnostic. Constructor error adapters
move the owned field into registration_error without another field allocation.
Terminal module retries check terminal state before changing the first token;
the public constructor still discards failed owners rather than retrying.

## Verification distinctions

One new Catch2 case runs 18 ordinary configurations: three kinds, indices 10/64,
and successful/writer/direct-native twins. Each uses prior-module entries and a
different kind in module2. Exact existing code, message and original target field
are asserted; command64 includes the five shipped commands. Existing C02’s 36
mixed-kind/module controls remain intact.

`allocations.py` makes an exact reversible production copy and runs ten modes:

- success: unchanged valid behavior with the same instrumented copy;
- removed: arms global-new refusal only around each actual projection10 token
  assignment, observes two D/F selections and zero allocation requests, and
  succeeds. This demonstrates removal of the old allocation; it does not claim
  that fault injection refused an allocation that no longer exists;
- outer-control: retained writer in projection10, allocation fault disabled,
  returns internal / Invalid external profile protocol / catalog.projections[10];
- outer: identical writer and actual remaining field-render allocation refusal,
  requests 32 bytes and returns resource_limit / External profile resource limit
  exceeded / profile.modules[2], never projection9;
- staging: actual vector-reserve allocation refusal requests 96 bytes for two
  module elements, returns the same external resource message with field profile;
- memory: refuses real Lua growth allocations at selected projection10,
  exercising sticky exhaustion and Lua memory adaptation with field projection10;
- stack: deliberately fills the protected active frame to 2048, triggering the
  real existing freezer stack check and Lua stack exceeded at projection10;
- status: injects a controlled Lua error after real partial metatable installation,
  returning the exact protocol diagnostic at projection10;
- native-badalloc/native-unexpected: a controlled throwing allocator acts only on
  an actual Lua growth request after projection10 selection. These exercise the
  native exception adapters and exact resource/protocol policies respectively.
  They do not claim that production realloc throws C++ exceptions or that host
  exhaustion occurred.

Every failed protected-return path checks the actual module callback’s terminal
retry token and unchanged published identity-set identity. Native exceptions
unwind directly to the outer adapter. Every mode checks zero tracked Lua bytes
after destruction. The runner’s `delta.json` documents instrumentation and the
reverse-equality proof; `commands.json` and individual logs retain raw results.

The adapted retained C02 location runner preserves actual pre-root guard refusal,
controlled partial-freezer and union failures, same-owner terminal retry and zero
cleanup. Its private condition now reads the token because error text is lazy;
its original historical generator is untouched. Repeat, production, reachable,
R1/R2 and final-reservation/high-catalog runners are fresh copies with output root
adaptations. `support/adaptations.json` (under the sibling C03 support directory)
records copied generator identities and modifications. No red detector or prior
failure is overwritten.

## Frozen gate and delivery

Final command results and exact identities follow after completion. No full 578,
RelWithDebInfo/release or Linux gate is claimed. Latest full gate remains 520 per
configuration. Fresh combined independent approval, public profile CLI, B/C
semantics/adapters, X06 profile storage/history interpretation, whole-invocation
orchestration, mandatory external-profile gate, cutover, execution/RA01 and Linux
remain open. Public init creates format1.

The final-1 frozen gate passes all 22 serialized commands. Debug inventory is 578
unique names preserving every 577 prior name. Focused execution is 205/205,
zero failures/errors/skips/disabled cases, in
85.94 seconds. All ten new allocation scenarios pass. Retained repeated-root,
location, production, reachable-native, R1/R2, exact/high/one-byte-short budget
and high-catalog controls pass, including their deliberate red controls. Four
vector checks and four historical exact-source audits pass; A0 forward/reverse
and exact shipped-source checks pass. Pinned formatting/Doxygen checks 117 active
sources. Final all+planar_tests build reports no work.

`final-1/source-freeze.json` and `source-after.json` compare 21172 complete
source entries with zero mismatches. These map repository-relative regular-file
paths to {sha256,mode}, symlinks to {symlink,mode}, and exclude only root .git,
root build and this C03 evidence directory. Every historical artifact, shipped
Lua/vendor file and AGENTS.md symlink is covered. The final release rehash uses
the same complete entry-set semantics. `final-1/scope-audit.json` enumerates only
authorized changes. The evidence baseline was captured after the new red probe
setup, so its incremental `final-1/owned.diff` omits that already-created support
generator; root `owned.diff` is the complete C03 owned diff against the preserved
original eight files and all newly created C03 support files, including that red
generator. Both diff identities remain available without rewriting gate output.

`final-1/build-identities.json` covers 931 build-relative objects, PCMs,
archives, selected executables and configuration files. Source and build release
manifests are `source-release.json` and `build-release.json` under this evidence
root. Artifact manifest paths are evidence-root-relative; it covers every file
except itself and finalization.json. `finalization.json` binds the report,
complete/incremental diffs, release manifests, artifact manifest and exact source/
build identities without recursive hashing. The finalizer is retained as evidence.

Candidate identities before final release rehash:

- Source freeze/after: `00aedef73482d57d51b3578faf51edb2f0702bc3fb5d66d76bfdf03430416fdb`.
- Complete owned diff: `d55974b8d82846851b58d76dcf71c761ed5b4b3492076960ade7d35ab8f61532`.
- Incremental gate diff: `5b556ab853f3c165746f9710d6c8657f238d70af99cdc4d41611aaf4fcd7871b`.
- Build identities: `68d4f3375c744266173946a66ba16becb645fa8e3c4e486e9785b0289be8fe37`.

Release is complete only when finalization.json records matching source/build
identities and released serialized build ownership. No subsequent source, build
or evidence edits follow that release. This candidate requires independent review.
