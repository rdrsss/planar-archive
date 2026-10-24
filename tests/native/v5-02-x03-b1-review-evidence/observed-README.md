# X03-B1 coder handoff

Candidate for fresh independent review; no self-approval. Shared checkout HEAD
3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a on dev/planar-v2-vernacular.
Owned changes and exact baseline/candidate snapshots are identified in owned.diff
and candidate-manifest.json. No Planar tools, installed binaries, operator data,
commit, install, publication or merge were used. Debug build ownership is released
at handoff; verification processes have completed.

The native initializer initialize_new_v2 publishes only new explicit format2
files through existing staging/verification/no-overwrite mechanisms. Existing
initialize_new and public init retain format1. The selected exact format2 schema
changes only the metadata CHECK and record codec CHECK; user_version is2 and
application_id is unchanged. ledger_metadata retains an enum discriminator, so
publication/backup/restore verify format as well as identity/workspace/time.
No ALTER, implicit upgrade, repair or conversion tool is introduced.

This is deliberately partial lifecycle support. A read-only verifier accepts
format2 with supported codec1 records, including historical request-v1 records
in native fixtures, while any kind4/codec2 row refuses unsupported_state before
record decoding. Format2 read_write open refuses unsupported_state after readonly
verification and before rollback/reopening writable. begin_records and both
complete_acceptance overloads funnel through that transition, so no format1
request acceptance enters format2. Explicit recovery refuses format2 by its
header before creating scratch or opening writable. Native backup/restore into
new targets remain supported and preserve format/identity. B2 must provide typed
v1/v2 request storage and reviewed integration before removing these restrictions.
Full format2 request equality/interner/collision/redaction/CAS/replay/crash/full/
recovery and accepted-history support remain mandatory B2 obligations. No public
profile admission, prepared/request-codec/Lua changes or CLI cutover is claimed.

Seven new registered tests extend the baseline from409 to416 with no removed or
renamed baseline cases. Four native cases cover exact schema delta, new metadata,
codec1 reads, writable/writer/recovery closure, version/header/schema/metadata/
foreign/future refusal, codec2 refusal, backup/restore identity/format preservation,
invalid paths and no-overwrite/sidecar guards. Three additional tests reuse the
existing reversible publication syscall variant for native format2 initializer,
backup and restore: staged sync/link failures, uncertain postpublication sync/
cleanup, arriving sidecars and arriving destination. Existing v1 test names are
preserved; its future-version fixture advances2 to3 because2 is now recognized.
The format1-schema/header2 and format2-schema/header1 mismatch cases refuse corrupt.
This is new native functionality, not a previously admitted behavior bug; no
pre-fix runtime regression is claimed for the newly introduced initializer API.

Final serialized debug build all+planar_tests passes. final-focused.log/xml and
final-LastTest.log contain167 passing focused ledger/history/record/request/
prepared/acceptance/publication cases, zero skips,47.01 seconds. This includes
existing reliability and D8 crash/full tests on format1; they do not prove format2
acceptance. The first167-case pass before the added reverse header mismatch section
is separately preserved. JUnit and LastTest were preserved before final inventory.
Record-v1 oracle verifies10 independent vectors; request-v2 oracle verifies2.
Pinned clang-format --Werror and Doxygen pass for98 active first-party files;
new support C++ files also pass pinned formatting. no-work.log confirms no work
for all+planar_tests. No release/full milestone/Linux gate is claimed.

pre-b1-planar is a copy of build/rewrite-debug/bin/planar taken before source
edits. baseline-build.log then reports no work for all+planar_tests, establishing
that the current pre-B1 source executable was already built; its copied hash is
pre-b1-planar.sha256 and dynamic dependencies are recorded by otool. Only pinned
libc++ and system libSystem are dynamically referenced. final-old-binary.log and
old-binary-results.json record8 actual executable invocations: history and load
against a native-created format2 file, each clean or with journal/WAL/SHM sentinel.
All return exit3 unsupported_ledger_version and preserve exact complete scratch
directory entry/content hashes, including the sidecars. HOME and PLANAR_DB are
explicit scratch paths; no installed binary is invoked. The fixture helper calls
the actual candidate initializer. Fixture and frozen old executable hashes are
in the result. An initial setup attempt refused a /var symlink spelling before
reaching the old executable; resolving the scratch root physically fixed the
harness. The final script additionally covers load's pre-write admission and was
rerun after the focused suite; no product or native test source changed then.

build-attempt-1.log retains a test-only compile failure from trying to reset an
expected<statement> with {}; a lexical statement lifetime corrected it. Second
build and final build pass. No test assertions were weakened by that correction.
The first oldbinary setup failure is preserved from the tool transcript in
old-binary-setup-attempt-1.txt and is explained above; it is not represented as an executable refusal success.

baseline-status/head/git.diff and baseline/ exact owned files were frozen before
edits. baseline-manifest.json reconstructs the complete non-build/non-.git file
inventory using those frozen owned pre-edit bytes, before status documentation
changes; the provenance is explicit. It excludes B1 support/evidence additions.
protection-audit.json checks every baseline regular file plus additions; unrelated
source, v1 codecs, prepared state, CLI, Lua and existing evidence remain unchanged.
Coordinator round records alone are allowed outside coder ownership. AGENTS.md
remains the CLAUDE.md symlink. verification-inputs.json records the source/native
suite snapshot before final verification; verification-input-comparison.json
checks exact stability and explains the later support-only oldbinary expansion.

Each final command has raw log and result JSON with argv/cwd/UTC times/exit code.
Manifest path bases are explicit. candidate-owned contains exact owned snapshots,
owned.diff compares pre-B1 baseline to candidate including new support files,
identity.json hashes the candidate manifest/diff, and artifact-manifest.json hashes
all evidence artifacts except itself. freeze.py regenerates these manifests and
checks protection, test inventory, JUnit results and no-work status. Do not edit
this candidate during independent review.
