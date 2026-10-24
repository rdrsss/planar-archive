# X04-B2 author candidate

Status: implemented candidate awaiting fresh independent review. Shared checkout
`dev/planar-v2-vernacular`, HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`.
No commit, installation, publication, merge, Planar tool/skill/queue, installed
binary, operator database, runtime/Lua/CLI integration or subagent was used.

## Scope and boundary

New `history/effective_catalog.{cppm,cpp,t.cpp}` provides pure catalog-v2 structural
validation with explicit ordered unprefixed embedded-original and profile-module
path manifests. `validate_catalog` bounds canonical traversal before retaining
caller input; `decode_catalog` refuses more than 1 MiB before decode. Successful
results privately own canonical bytes and a fresh decoded value, severing mutable
input aliases. Imports are only value/canonical and std; CMake adds one module and
its colocated test executable outside default all. No existing runtime, shipped
Lua, CLI, profile codec, provenance codec, canonical codec or vendor source changed.

Closed `catalog_error_code` distinguishes invalid catalog, identity conflict and
resource exhaustion with only `catalog` or one-based catalog list fields. Future
live adapters can map protocol/identity/resource refusals to the contracted
internal/reference/resource_limit vocabulary; historical adapters can map to
invalid provenance. No host bytes, names, paths or help are interpolated into errors.

Validation covers exact fieldsets/kinds, grammar, help controls/UTF-8/4096 bytes,
64 commands/32 options/64 shapes/64 projections, canonical sorted lists/options/
capabilities, duplicate command/handler/option and per-kind shape/projection IDs,
reserved identities and explicit tier/module/ordinal correspondence. Shipped
commands require exactly current five native pairings from original manifest
`planar/commands.lua`, including decimal history/export descriptors and all six
required load options. Other shipped commands refuse. Reserved shape/projection
IDs require shipped ownership when present; no literal shipped help/revision,
fixed shape/projection source module or prohibition on extra shipped shapes/
projections is invented.

External role/history capabilities are mutually exclusive; projection requires
exactly one. All optional descriptors remain mandatory parts of each read signature
with required=false. History IDs are optional only for revision/at-event/after-event
on history_read_v1. Extra nonhost text/decimal/flag options remain allowed; recognized
incompatible host-bearing options refuse for read-capable commands. Pure unknown
text remains data. Catalogs may contain projections for both input kinds. Invocation
selectors/defaults/ID conversion/projection ID matching and callable identity are
not historically proved here. Only exact native handler IDs are reserved; a
nonreserved `shipped.custom.v1` external handler ID is permitted by the grammar.

Source manifests preserve original/profile portable path policy and external
reserved roots, lowercase .lua suffix and prefix case-alias checks. Manifest
membership does not access the filesystem. Normalized modes, effective prefixed
tree paths, byte/mode/profile correspondence and the entire effective manifest
bound belong to B3.

## Bounds

Canonical bytes are at most 1048576. Schema-derived depth7 is
root/list/entry/descriptor/options/option/field, including object keys. Node ceiling
22921 is `9 + 64*(25 + 32*9 + 3) + 128*21`: root and keys9; command wrapper,
descriptor and owner25, option and keys9, known capability vocabulary3; each
shape/projection wrapper/descriptor/owner21. This is a safe structural upper
bound; required shipped signatures and mutually exclusive external read
capabilities tighten successful catalogs further. It introduces no arbitrary
public depth/node allowance. The combined maximal count fixture with all59
external commands at32 options and64 shapes/64 projections passes below the
byte ceiling. Exact1 MiB catalog passes and one extra byte refuses. Catalog alone
does not establish B3's combined effective manifest1 MiB bound.

## Attempts and verification

Every command sequence used a unique filesystem freeze, root-only `.git/` and
`build/` exclusions, before/after equality checks, archived active sources and
serialized build-directory ownership. Baseline owned originals and symlink
records are retained. No source edits occurred during command sequences.

- attempt01: configure passed; build failed because test helper `history` collided
  with the namespace. The raw failed compile and exact candidate freeze remain.
- attempt02: corrected helper; Debug all+planar_tests and508 inventory passed,
  preserving all497 prior names. Focused run97/98: all87 pre-existing cases pass;
  one new fixture was inconsistent (shipped module commands.lua, ordinal1 naming
  json.lua). Product correctly refused it. Immediate JUnit/LastTest/CTestCostData
  are retained; no failure is reported as a pass.
- attempt03-classification-regression: corrected fixture plus targeted regression
  assertion. Build passed; one catalog test failed as expected because malformed
  handler grammar incorrectly shared identity_conflict with duplicate IDs. The
  observed assertion `1 == 0` establishes failure before the two-line branch fix.
- attempt04-final: final code/tests/current docs frozen. Debug all+planar_tests,
 508-name inventory, all11 catalog cases, canonical vectors (12 hand-byte,
 20 upstream hashes,41 canonical/domain),10 record and2 request-v2 vectors,
 four inherited command/CLI source audits, pinned formatting/Doxygen for110
 active sources and final no-work all+planar_tests all pass. Catalog JUnit has
 zero failures/errors/skips; final build says `ninja: no work to do.`.

The final test refinements retain the same11 names and add maximal schema count,
64/65 external modules, explicit scalar-kind checks and separate malformed grammar
versus duplicate identity classification. The sole product change after the
broader attempt02 separates malformed family/handler grammar (invalid_catalog)
from actual handler duplication (identity_conflict). All87 existing focused cases
have unchanged source and dependencies and are not repeated unnecessarily.

`source-audit.json` binds exact old runtime/Lua/profile/canonical/provenance hashes,
pure imports and native option signature tuple equality. Finalization also checks
forward/reverse exact-source substitutions for all five existing generated
storage/export families and binds final candidate, snapshots and complete artifacts.
Final support additions are separately bound by the candidate manifest; active
product/tests/current docs equal the final passing attempt04 freeze.

This is focused macOS ARM64 Debug evidence, not a full508-case suite, release or
Linux result. Last full milestone evidence remains approved X04-A3 at489/489 in
Debug and RelWithDebInfo. B1 is independently approved; B2 is a candidate. B3,
live registration/CLI/acceptance integration, whole-invocation shared budgets,
mandatory external-profile gate, public cutover, execution/RA01 and Linux remain
open. Public init still creates format1.
