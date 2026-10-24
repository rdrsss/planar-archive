# X01 frozen author evidence

Candidate is test-only and awaits independent review. The baseline was captured
before edits: full repository-relative hash inventory excludes .git/build,
tracked binary diff and status/HEAD, and six owned preexisting source copies.
Candidate-source copies and candidate.json contain seven owned candidate paths.
owned.diff is reconstructed against those exact baseline copies; the new report
uses an empty baseline. The coordinator round is independently owned.

Raw exploratory attempts and the intermediate failing build are retained. Final
commands.json records executed final check argv/UTC/status; development build
commands were `cmake --build build/rewrite-debug --target all planar_tests`
(build-first success, build-second failure, build-third success, build-final
success). Initial exploratory CTest argv appear in raw verbose logs. Final source
was unchanged during final checks. JUnit/LastTest copies precede inventory.

Final spike: 3/3 cases, 350 assertions. Focused: 41/41, zero failures/skips;
all seven actual CLI suites. Inventory405 preserves baseline402. Pinned
formatting/Doxygen95 and zero-work rebuild pass. This is bounded macOS ARM64
debug evidence; no new release/full gate is claimed. Neither generated variant
inputs nor generators/CMake/product source changed; no additional variant audit
is required for this test-only scope. Existing variants were rebuilt and actual
CLI variants executed in focused suites.

Manifest bases are explicit: candidate.json/artifacts.json use repository root;
evidence-sha256.json uses this evidence directory and excludes itself. The latter
includes frozen baseline/source/diff/report/log artifacts; it is written last.
protection-audit.json compares all baseline file bytes, allows only assigned
candidate paths, AGENTS.md's preserved CLAUDE.md symlink view, and the coordinator
round. No historical evidence/golden/product bytes changed. Debug is released
for the fresh reviewer; release remained idle.
