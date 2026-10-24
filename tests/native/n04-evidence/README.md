# N04 CLI11 restoration and render handler evidence

Candidate is uncommitted and requires independent review. Shared checkout edits
from prior rounds were preserved. No Planar tools, installed binary, database,
installer, publishing command or commit was invoked.

CLI11 v2.7.2 was restored using `git archive HEAD vendor/cli11 | tar -x`.
All 321 files match HEAD, including the upstream `9d6a` tree and cache metadata.
The active offline CMake declaration pins the codeload release URL and archive
SHA256; it enables the real `cli11` module and precompiled library, with tests,
examples, docs and install options disabled. No dependency download occurred.
Upstream module CMake still creates uninvoked installation metadata; the root
install scripts exclude CLI11 because its subdirectory is EXCLUDE_FROM_ALL.
Copies of both generated root scripts are retained. `make install` remains an
unsupported operation.

`bootstrap` owns CLI11 root registration and selected-command dispatch. The
render handler owns option registration/state, semantic pairing, host paths,
capture routing, trusted evaluation and rendering. Response/diagnostic helpers
sit below bootstrap and the handler. The existing checked process entry point
is unchanged. `audit.py` compares the moved render body and capture helpers to
the prechange bootstrap modulo whitespace; both comparisons pass. It also checks
1,998 protected files (engine, stdlib, fixtures, Glaze, historical planning and
prior evidence) remain unchanged. This round introduces no deleted paths.

CLI11 recognizes commands/options, consumes their values, validates required
counts and rejects duplicate bindings. The only raw-token preflight rejects
long tokens containing `=` to preserve separate values. After parsing, library
option counts and expected-value metadata account for the exact token count;
CLI11's remaining tokens and selected-command count reject discarded positional
terminators and repeated verbs. No argv command lookup or shadow option parser
exists. Fixed help and canonical Glaze usage diagnostics preserve product output
without exposing CLI11 diagnostics or host argument values.

Final commands, run serially per build directory:

- `make test-all` (`debug-gate-final.log`): exit 0; 121/121 registered cases,
  zero skips; pinned formatting and Doxygen pass for 29 active C++ source files;
  no-change `all planar_tests` build reports `ninja: no work to do.`
- `make test-all PRESET=release` (`release-gate-final.log`): exit 0; the same
  121/121 cases, zero skips; the same formatting/Doxygen and no-change checks.
- Both acceptance runs execute 247 real CLI processes and preserve all six
  existing native goldens. Raw process count/output is in each `*-last-test.log`.
- `python3 tests/native/n04-evidence/audit.py`: exit 0, exact restoration and
  preservation results in `source-audit.json` and `source-audit.log`.
- Scoped `git diff --check`: exit 0 (`diff-check.log` is empty).

The parser matrix checks every value option's duplicate/empty/missing/equals
forms, required omissions, flags and help, unknown verbs/options/shorthand,
positionals/terminators, budget grammar/bounds, mapping/pairing/mismatched IDs,
reordered valid invocations and a maximum-budget success. Two direct CLI11 tests
prove handler registration parses without capture and rejects duplicate bindings
before dispatch. Existing acceptance retains path confinement, output failures,
canonical/hash/prose checks and sentinel state isolation.

Development traces remain available: `red-parser-marker.log` demonstrates that
CLI11 initially accepted a trailing terminator; `focused-test.log` shows why
remaining tokens alone were insufficient; `parser-green.log` then exposed a
repeated verb. Both regressions are fixed using parsed CLI11 metadata. Initial
build logs retain the corrected module ownership/link issue. Later parser
attempts also retain corrected test expectations: budget changes can affect the
packet and a hyphenated job ID is a semantic schema error. Final gates above
supersede those development runs. The first complete debug gate predates the
last three added parser scenarios; the final debug and release gates include all
247 processes.

This is macOS Apple Silicon evidence; Linux execution is not claimed.
`candidate-sha256.txt` identifies all owned product/reference/test/vendor files;
`evidence-sha256.txt` identifies this evidence excluding itself.
