# R14-F2 independent export CLI review

Disposition: **approve**. No blocking findings or unresolved product choices.
Fresh reviewer `/root/r14f2_reviewer` reviewed the shared
`dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The intentional pre-existing rewrite
was preserved. Product files were read-only; writes were confined to this report
and `tests/native/r14f2-review-evidence/`.

## Exact candidate and evidence

- `tests/native/r14f2-evidence/candidate-files.json`: SHA256
  `153f9831b3a30b3df6f41095aafb0447b7bafdddb5c1ece285391ea1d1b31bf5`.
  All **16** current members matched independently at entry and finalization.
- Implementation `evidence-sha256.json`: SHA256
  `558a48e001aa15e11aaa327cefc564dc15c0832b686076102dca0a4cd611b118`.
  All 38 folder-relative members matched. The interim count of 17 candidate
  members is superseded by the actual 16-member manifest.
- Reviewer `tests/native/r14f2-review-evidence/evidence-sha256.json`: SHA256
  `c5b6843f7be4e83eba9c35851412cb98a99f641ac403c2d8a66dd6be23c62f8f`;
  11 raw records, scripts and results, excluding the manifest and its hash file.
- Independent audit retained all 5,046 protected baseline files, including native
  F1, and the `AGENTS.md -> CLAUDE.md` symlink. All five configured generated
  variants matched fresh generation byte-for-byte and reported exact reversal.
  The generic usage expectation in `bootstrap_init.py` reverses to its baseline
  hash. All 327 prior registered names remain; four F2 additions give 331.

## Contract assessment

`handlers/export/command.cpp:28` attaches required scalar options through CLI11
with explicit-value and canonical positive signed-64 validators, duplicate
refusal and no filesystem work during binding. `bootstrap.cpp` retains the sole
equals syntax guard and counts CLI11 parsed bindings, registers export in root
help, and rejects extra commands, command help and positionals. No shadow parser
or handler-to-bootstrap edge was introduced. The export handler depends downward
on response, native source export and CLI11; its test-only fault executable stays
outside the default `all` target.

`command.cpp:102` passes the original explicit target spelling directly to the
approved native service. There is no host-file helper or lexical normalization
before physical traversal. Success encodes exactly version, kind, ledger,
revision and source through the existing Glaze codec, with every database ID a
string and one terminal LF. There is no target path, source payload, automatic
retry/cleanup, registry injection or interpretation execution in this layer.

`command.cpp:44` preserves the approved error/status mapping and ignores private
native message/location bytes except a validated generated cleanup basename.
`stage_token` checks the exact prefix, length and lowercase hex grammar before
projection. Malformed internal tokens become a redacted internal refusal.
Cleanup guidance does not assert continued ownership or permission to delete the
named occupant. Native I/O and cleanup categories have direct adapter evidence;
they are not misrepresented as actual CLI filesystem-fault injection.

`command.cpp:98` completely buffers both uncertainty diagnostics before native
publication can begin and refuses without side effects if either cannot be built.
Native publication uncertainty returns that existing fallback. After native
success the publication flag precedes result allocation/encoding; refusal or an
exception returns the local fallback by nonthrowing move, with empty stdout and
operation `export`. Only the complete success response carries the prebuilt
operation-`stdout` diagnostic. Prepublication errors never acquire it.

`main.cpp:36` writes those existing diagnostic bytes after a failed stdout write
without constructing a replacement diagnostic. Empty stdout for semantic failure
does not trigger the write-failure path. The existing SIGPIPE and checked-write
policy is preserved. Render/init/history retain their original stdout-failure
behavior. Actual closed-pipe tests close the read end before child launch, so
they have no reader timing race; complete target preservation and unchanged
ledger bytes are checked after the export output failure.

The F2 generator changes only the handler module/namespace and two exact sites.
One controlled `std::bad_alloc` and the one-byte encoding ceiling occur strictly
after real unchanged native F1 publication. Both verify prebuilt fallback,
empty stdout, preserved complete target and unchanged ledger. Production source
is never mutated. These are meaningful boundary probes without claiming real
allocator exhaustion or native publication fault coverage beyond F1.

## Independent verification

Executed serially with exclusive debug build ownership:

1. `python3 tests/native/r14f2-review-evidence/audit.py`: current candidate and raw
   evidence hashes, protected inventory, five exact/reversible variants, prior
   test names and the post-gate delta all passed. Repeated at finalization.
2. `cmake --build build/rewrite-debug --target all planar_tests`: zero steps.
3. `ctest --test-dir build/rewrite-debug --output-on-failure -R
   'replacement\.(cli|init_cli|history_cli|export_cli)$|replacement.export
   (adapters|binds|postpublication)|replacement.help is explicit'
   --output-junit <review-evidence>/focused.xml`: eight matched, eight passed,
   zero failures/skips. The retained LastTest log records 247 render, 60 init,
   115 history and 55 export/output actual CLI processes. The four unit cases
   include redacted adapters, original binding, root help and controlled
   post-publication refusals.
4. `python3 tests/native/r14f2-review-evidence/probe.py`: 21 additional actual CLI
   probes passed. They check physical real-parent/`..` traversal with a quoted,
   Unicode/newline target, an option-looking target prefixed by `./`, missing and
   symlink parent/`..` refusal, malformed final targets, whitespace/non-ASCII/
   overflow ID spellings and extra/duplicate options. Success bytes match an
   independent read-only metadata oracle. Refusals create no artifacts; ledger
   and scratch legacy-database sentinel bytes remain unchanged and paths are
   redacted. An initial reviewer expectation classified malformed final targets
   as `path_denied`; the native contract returns `invalid_argument`, correctly
   mapped to `schema`. That expectation was corrected and its failed log retained.
5. Final `cmake --build build/rewrite-debug --target all planar_tests`: zero steps.

The implementation's authoritative serialized full gate was inspected: 331
passes, zero skips, 86-source formatting/Doxygen and a zero-step rebuild. Its
earlier overlapping focused run is exploratory only. The sole post-full-gate
change is `bootstrap_export.py` strengthening socket refusal and the success
identity oracle, followed by the affected 55-process suite. The independent
audit confirms unchanged production and all other test files against that gate;
the reviewer reran the strengthened suite. A redundant full gate was unnecessary.

## Limits and handoff

Approval covers the F2 export CLI and its publication-aware output boundary on
macOS ARM64 debug, composed with approved native F1. No release-preset, Linux
compilation/execution, actual allocator exhaustion or hardware power-loss
verification is claimed. Load membership remains pending and outside scope.
Integrated instruction/status refresh remains coordinator work. No old Planar
tools, installed binary, operator data, install, commit, publish or merge was used.
Debug build ownership is released to the coordinator. This reviewer is complete
and should not be redispatched for later rounds.
