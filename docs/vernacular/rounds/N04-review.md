# N04 independent review

Disposition: **approve**. No blocking findings or requested changes.

Reviewed the shared `dev/planar-v2-vernacular` checkout without modifying the
candidate, prior evidence, or another agent's work. Owned writes are this report
and `tests/native/n04-review-evidence/`. No Planar tooling, installed product,
installer, commit, publication or delegated work was used.

## Candidate identity and scope

`tests/native/n04-evidence/candidate-sha256.txt` covers 339 files and has SHA256
`fcab277606183535abb39e9508990e319d7f65d251f932577f12566deea1bc41`.
`evidence-sha256.txt` covers 29 evidence files and has SHA256
`6dafea98e92b25e7befec8160e7ac6a780606a60f7a186ba362e7bc4a1da2510`.
Independent before/after verification confirms every entry matches. All 321
restored CLI11 files match Git HEAD; all 1,998 protected files match the coder's
before snapshot. No file present in that snapshot was deleted. The moved render
body is whitespace equivalent to the approved prechange body. Raw verification
is `snapshot-before.json` and `snapshot-after.json` in the reviewer evidence.

Read the active instructions, implementation plan, roadmap, v4 design, N04 brief,
render contract, current references, candidate code, coder audit script, before
copies and raw parser/build/gate logs. Historical handler inspection confirms
`handlers/<family>/command.cppm` and `command.cpp`, the dotted module naming and
handler-owned CLI declarations follow the former family pattern; no legacy
handler logic was restored.

## Boundary review

- `src/cmd/planar/bootstrap.cpp:13` creates an invocation-local CLI11 root;
  `:20` retains handler options through parsing and dispatch. CLI11 recognizes
  `render`, options and values and rejects required/duplicate count failures.
  `:24` maps parser failures to the fixed Glaze diagnostic. Engine work occurs
  only after parsing, strict syntax checks and semantic pairing succeed.
- The preflight at `bootstrap.cpp:11` only rejects a long token containing `=`.
  It does not look up commands/options or consume values. Existing policy already
  rejects values starting `--`. The postparse checks at `:36` use CLI11 metadata
  and remaining tokens to reject discarded terminators and repeated verbs.
  Neither check duplicates an argv grammar or rejects supported single-hyphen,
  literal command-name or root-mapping values.
- `handlers/render/command.cpp:179` owns command registration and bindings;
  `:191` delegates scalar count, duplicate and required rules to CLI11. Planar's
  validators enforce explicit values, root mapping and bounded budget policy.
  `:203` owns pairing and rendering. Validators capture no expired local state;
  options own parsed strings and remain valid after the input vector is gone.
- `src/cmd/planar/CMakeLists.txt:1` puts shared response support below bootstrap
  and render. The graph has no handler-to-bootstrap edge or command cycle.
  Response encoding/error status and checked `main.cpp` process I/O preserve the
  existing policy. Exceptions remain contained at handler/bootstrap boundaries.
- `cmake/dependencies.cmake:48` pins CLI11 v2.7.2 and its release archive SHA256,
  uses committed offline sources and the real `cli11` module, disables vendor
  extras and places the vendor subdirectory outside default installation.
  Both preserved generated root install scripts exclude CLI11. This does not
  establish an installation/distribution contract, which remains out of scope.

## Independent verification

Commands below ran serially where they share build artifacts. No product build
was needed: both full raw gates were audited and no candidate changes or new
concerns required repeating them.

1. `python3 tests/native/n04-review-evidence/verify_snapshot.py` before and after
   review: exit 0; exact identities and protected/vendor scope verified.
2. `python3 tests/native/n04-review-evidence/parser_probe.py`: exit 0; **484 cases
   per preset, 968 real CLI processes, zero mismatches**. An independent model of
   the prior strict grammar tested unusual scalar values, every option binding,
   marker/verb/help insertion at every position, reordered groups, budget and
   root/base pairing. Invalid grammar consistently returns usage status 2 with
   no stdout, even with deliberately nonexistent workspace paths; accepted
   grammar reaches the expected I/O boundary. Raw arguments/results are retained
   in `parser-results.json`; the concise result is `parser-summary.json`.
3. `python3 tests/native/n04-review-evidence/path_probe.py`: exit 0; **24 real
   debug/release processes**. Literal workspace values `render`, `-h`, `-` and
   `./--literal` render successfully. Root paths containing `=` succeed. Bracket,
   quote, space and non-ASCII workspace filenames reach the unchanged native
   portable-path refusal rather than becoming CLI usage failures. The initial
   probe's overly broad success expectation and its correction are recorded in
   `path-probe-initial.txt`; final raw results are `path-results.json`.
4. `python3 tests/native/n04-review-evidence/build_lifetime_probe.py`: exit 0.
   Reviewer-only harness linked the current debug modules using recorded compiler
   and link flags. **512 independent handler registrations/dispatches**, with
   parse vectors destroyed before dispatch, and **1,024 bootstrap invocations
   across eight threads** complete with zero failures. The harness sources,
   reproducible compiler driver and `lifetime-probe.log` are retained; temporary
   compiled probe files were removed after execution.
5. `python3 tests/acceptance/bootstrap_cli.py build/rewrite-debug/bin/planar` and
   the same command with `rewrite-release`: both exit 0; each runs **247 real CLI
   processes, six exact native goldens and 60 host path checks, zero skips**.
   See `debug-acceptance.log` and `release-acceptance.log`.

Audited coder full gates: debug and release each pass **121 registered cases
(120 native plus acceptance), zero skips**, pinned formatting and Doxygen for
29 source files, and a no-change build reporting `ninja: no work to do.` The
preserved red trailing-marker and repeated-verb evidence demonstrates the actual
CLI11 compatibility problems addressed by the final parsed-metadata checks.

Evidence is macOS Apple Silicon execution. Linux execution and distribution are
not established by this round. Approval applies only to the exact candidate
identity above; subsequent edits affecting its scope require renewed review.
