# R14-A2 candidate and raw verification evidence

Shared checkout: `<repo>`, branch
`dev/planar-v2-vernacular`, starting HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Existing uncommitted rewrite and
other work were preserved. The coordinator owns round/planning records. No
Planar tools, queue, installed binary, operator data, commits, installation or
publication were used.

The thirteen-file candidate is frozen in `candidate/`. `candidate.json` maps
repository-root-relative paths to SHA256 and hashes the sorted compact JSON file
map for its identity:
`281dbab2c65e9b67d35a3db47fb2afc290a7971ac63aff409667fa7532262cb2`.
`baseline/` contains the seven existing files captured before owned edits;
`baseline.json` hashes those starting-checkout bytes. Six new files have empty
baselines. `candidate.diff` compares only these owned files against those bytes.
Earlier approved evidence is never rewritten. `evidence.json` uses paths relative
to this evidence directory and excludes itself to avoid recursive hashing.

`history.provenance_codec` owns the exact existing provenance errors, source
views, prepared DTO and registry/stdlib prepare/verify algorithms. Its only module
edges are tree_core and canonical, with value available through canonical.
`history.provenance` re-exports that codec and the existing owner-backed tree API;
only its original `prepare_embedded_stdlib` function imports generated source.
No historical verification compares against current embedded bytes.

`history.interpretation_codec` owns the exact existing interpretation error and
recorded DTO, fixed supported-version table, checker and historical decoder.
Its pure `encode_interpretation(bundle, semantic, stdlib_digest)` entry point
assembles the original envelope/versions and finishes through the same decoder.
The digest parameter is data, without any authenticity assertion. The live
`history.interpretation` wrapper still prepares embedded provenance internally,
invokes only `interpret_bundle`, retains the exact typed live failure handling,
and passes its internally derived digest to that codec. Its public live API still
accepts only the captured bundle. Existing names are retained by direct module
re-exports in the same namespace, without duplicate DTOs or conversion aliases.
The pure interpretation codec depends only on canonical/value.

`audit.py` checks the entire moved provenance implementation modulo module/import
changes, the unchanged embedded wrapper function, the entire interpretation
helpers/version table/checker/decoder, original envelope assembly and encode/decode
composition modulo explicit input parameters, unchanged live invocation/failure
handling/internal digest loop, and exact moved DTO/error declarations. Direct
link commands in `provenance-codec-link.txt` and
`interpretation-codec-link.txt` contain no format, SQLite, runtime, generated
embedded source, Lua or wrapper libraries. No storage policy or accepted-history
support is introduced; format/schema/SQLite, record/canonical/tree algorithms,
CLI, Lua, generic embedding, fixtures, goldens and vendor remain byte-identical.

`commands.json` records exact verification argv, working directory and exit codes.
Direct serialized debug configure and `all planar_tests` build passed. The
focused regex includes both new codec prefixes and executes 122 cases: every
one of the 113 A1 case names plus five isolated provenance and four isolated
interpretation cases, zero skips. Existing wrapper/storage tests remain intact.
The isolated interpretation tests read reviewed canonical fixtures directly and
require both pure encoded outputs to equal the saved bytes; historical differing
digests/policy data, map/list distinction, signed-32 limits, fixed shape/version
refusals and inclusive 16 MiB boundaries remain covered without Lua/storage links.
Pure provenance tests cover exact binary registry/modes, source boundaries/order,
assembly/manifest verification, paths and synthetic-inclusive bounds.

Verbose CLI CTest output confirms one passing CTest case containing 247 real CLI
processes and all six unchanged packet goldens. Both canonical/hash and record
vector `--check` commands pass. Active pinned formatter/Doxygen passes for 60
source files; supplementary helper lint passes for two files. The final serialized
`all planar_tests` build runs zero Ninja steps. All recorded checks passed on their
first attempt; the final freeze only improves testing-reference wording and does
not change tested source. Both zero-work build logs are retained.

`protected.json` hashes 4,204 unowned source/vendor/fixture/vector/helper/reference/
prior-evidence files from this starting checkout. `inherited-protected.json`
checks 11,993 earlier approved hashes, excluding owned paths, Finder metadata and
coordinator planning/round files. `coordinator-baseline-differences.json` records
intentional newer implementation/ledger-contract/roadmap hashes against the R13
baseline; those planning files were not edited by this coder. Existing earlier
failure logs remain protected. Fresh C2/B1 generation is byte-identical to the
configured source, interface and manifest; reversibility, exact namespace/digest
substitution counts and disabled verification mutation are checked and their new
round-local manifests retained.

Verification is host macOS debug only. Release/Linux were not run; full integrated
R14 milestone gates remain outstanding. Debug is released for fresh independent
review upon delivery of the coder report. This evidence does not approve the
candidate.
