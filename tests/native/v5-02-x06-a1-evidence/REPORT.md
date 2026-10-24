# X06-A1 candidate evidence

Candidate awaiting independent review. Shared checkout on `dev/planar-v2-vernacular`,
starting HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`; intentional dirty rewrite.
The exact source/reference/vector snapshot is `candidate.json`, with reconstructed
preedit diff `candidate.diff`. This report does not approve the candidate.

The new pure `interpretation_v2_codec` exposes only mandatory-rawtree decode and
encode. Decode checks both 16 MiB limits before parsing, interpretation then V,
fixed B/S/E/V syntax and embedded-V equality, supported constants, native raw-tree
verification, contextual asserted catalog validation, then exact catalog/digest/
custom-selection correspondence. Encoding canonicalizes B/S, verifies raw files,
derives V/E and runs the same final checker with its private verified context.
Neither accepts caller digest/catalog/prepared aggregates as authority. Results
own canonical values and bytes, sever aliases, and are ordinary recorded data.

The private tree context calls `verify_effective_stdlib`, decodes verified raw
manifests to recover actual ordered shipped/external paths, validates E.catalog
against those paths, recomputes the raw 32-digit tree digest and exact 41-byte
shape H. H retains exact descriptor and logical owner `{tier,module}` including
its recorded module path, excludes ordinal, and hashes lowercase original module
byte hex. The reorder test changes ordinal/tree/catalog/V while retaining H for
unchanged selected descriptor/logical-module/bytes. No Lua, filesystem or SQL is
used by the new boundary; `no-lua-proof.json` preserves the actual native link and
symbol audit. Existing transitive `ledger_types` is pure type vocabulary.

## Scoped prerequisite correction and preserved failures

The coordinator amended ownership for precisely four canonical-result adapters:
three in `effective_provenance.cpp`, one in `provenance_codec.cpp`. They preserve
`resource_limit` instead of collapsing canonical allocation/resource failures
into malformed provenance/catalog. Both files' preedit hashes were checked
against the original baseline before copying/editing. Ordinary malformed results,
v1 interpretation source/precedence and all accepted encodings remain unchanged.

`second-tests.xml` records the actual before-fix failure: **1,656 failed checks**,
first decoder allocation **1244 of 4881**, where resource became invalid_tree.
`allocation-red-codec.cpp`, `allocation-red-test.cpp` and both lower preedit files
preserve the failing candidate. The final top-level exhaustive allocation sweeps
and direct lower-adapter sweeps pass. This is controlled one-shot native
allocation failure evidence, without a natural host-exhaustion or RSS claim.

Two earlier development compile failures are preserved in `first-build.log`
(test mixed `auto` declaration; source in `first-build-test.cpp`) and
`second-build.log` (private helper collided with `std::bind` through ADL; source
in `second-build-codec.cpp`). The helper is now `verify_correspondence`.
`focused-listpath-tool-failure.log` is a CTest invocation issue: relative list
path was resolved from the build directory; the successful command uses an
absolute path. The first native-link checker rejected the existing pure
`ledger_types` by an overbroad prefix; the refined actual storage-module check
and no-Lua symbol check pass. The first final full walk also detected two newly generated Python cache entries
in the old effective-vector directory. They were removed; the new generator now
disables bytecode writes. `final-before-oracle-cache-cleanup.json` preserves that
walk, and the final v2 oracle check/full walk were repeated. Product inputs and
product gates were unchanged. These tool/fixture issues are separate from the
preserved product resource regression.

## Executed verification

| Check | Result |
| --- | --- |
| `cmake --preset debug`, serialized `cmake --build build/rewrite-debug --target all planar_tests` | Pass; final candidate build in `fifth-build.log`. |
| New binary with JUnit reporter (`third-tests.xml`) | **16 cases, 49,579 assertions**, zero failures/errors/skips; 20.080 seconds. |
| Focused CTest list (`focused-names.txt`) | **88/88**, zero failures/errors/skips/disabled; 43.38 seconds, `focused-tests.xml/log`. |
| Inventory | **892 names**, every prior C5 **876** name preserved, exactly 16 additions; `inventory-preservation.json`. |
| Independent v2, effective-provenance and canonical-v1 generators `--check` | Pass; `vectors.log`. New v2 vectors use declarative Python grammar and pinned upstream C xxHash, with existing independent hand-byte canonical/record controls. |
| `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` | Pinned formatting and Doxygen pass over **121 active sources**. |
| Final all+planar_tests build | `ninja: no work to do.`, `no-work.log`. |
| Source/ownership audit | v1 codec, exact external contract, runtime and shipped Lua unchanged; CMake append only; eight reference historical bodies unchanged; AGENTS and compile_commands symlinks preserved. |

The focused set includes new v2, old v1 codec, effective catalog/provenance,
old provenance codec, tree/core, record, canonical and hash binaries. New cases
exercise independent mixed shipped/custom/pipeline and empty-module profiles,
exact bytes and tagged H versus raw tree digest, encode/decode, input alias
severing, malformed/future/version precedence, sorted B/S/E and exact selections,
normalized obligations/work/role correspondence and guidance bounds, raw tree
membership/mode/order/owner/module/embedded/catalog tampering, matching forged
claims, 16 MiB preflight, canonical node-count preallocation/depth/cycle refusal,
8192-file/1 MiB module limits, guidance 65536/65537 and obligation 10000/10001.
The inherited canonical/tree suites cover their lower primitives; no claim is
made that every maximum-sized combination fits the tighter enclosing byte limit.
Original module bytes are deliberately non-callable and include binary bytes,
showing structural historical verification without live Lua policy execution.

## Protection and limits

`baseline.json` is the original full-hash **93,777-member** inventory, captured
once before product/reference edits; preedit copies were never overwritten.
`final.json` is another full-hash walk; `protection-result.json` records exact
allowed changes and protected release/prior-evidence preservation. Material root
dotfiles, symlink identity/target and all build trees are covered. Only `.git`,
the exact coordinator A1 round and this new support/evidence subtree are exempt;
Debug changes are explicitly allowed and release/install-release remain protected.
No cached intermediate substitutes for either full walk.

No full 892-test run, release test/build, Linux gate, current profile reader,
same-VM prepared profile acceptance, profile transaction, public CLI/cutover,
X06 cumulative shared-budget or mandatory external-profile completion is claimed.
The complete approved gate remains C5's 876/configuration. No installed binary,
operator data, Planar tools/skills/roles/queue, commit, installation, publication
or merge was used. Debug and owned inputs are released to independent review.
