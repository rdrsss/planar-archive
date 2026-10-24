# R14-D1 independent review

Disposition: **approve**. No findings or required changes for the bounded shared
host-path extraction and pure explicit-cwd normalizer. The pending load membership
question is outside this approval; no load service or membership policy is added.

Reviewed in the shared checkout on `dev/planar-v2-vernacular`, starting revision
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Reviewer wrote only this report and
`tests/native/r14d1-review-evidence/`, preserved others' work, and made no product
edits. No Planar tools, installed binaries, queue, claims, commit, installation,
publishing or operator state were used.

The exact candidate identity is
`33c92e663087dff697b86d49c0b5e22d8ee510f5c3ac63a9643a833d93298808`.
It is independently reproduced by SHA256 of the sorted compact JSON file/hash
mapping in the coder's `candidate.json`. All nine current files match their
candidate snapshots and declared SHA256 values. All 40 manifested coder artifacts
and all 4,828 protected files match; the exact protected inventory and instruction
symlink also match. Coder artifact-manifest SHA256 is
`7f6d8a643eba5301661909525c1d36ed454c25b807e92984dce6b6a2fb1e2f0e`.
The final independent audit repeated these checks after verification.

The review checked these boundaries:

- The private `host_file` and `host_directory` bodies are byte-identical to the
  saved renderer baseline. An independent whole-file reconstruction proves that
  the renderer differs only by import, helper removal, `required()` adapter
  calls and formatting. Original option spellings, physical parent link/`..`
  behavior, final file/root alias refusal and malformed trailing file refusal
  remain intact. The expected wrappers return diagnostic throws unchanged and
  sanitize `filesystem_error` to exact `io` / `Cannot resolve explicit input
  path`; allocation failures still reach the unchanged renderer outer handling.
- `normalize_intent_path` performs only textual and lexical path operations,
  validates non-NUL UTF-8 and absolute normalized explicit cwd, checks both
  individual inputs before constructing paths, checks relative join size before
  concatenation, and checks the normalized output. Ceiling arithmetic cannot
  underflow after its earlier guards. Render does not call the normalizer.
  Its documented future caller requirement uses the same returned path for
  request intent and capture; this review does not claim a future service exists.
- CMake adds a downward value-only path module. The actual native path test link
  contains host_path, value, standard module and Catch2 only, with no Lua,
  capture, ledger, history or acceptance library. Three generated ledger test
  variants independently regenerate byte-identically and reverse-identically.
- Existing CLI test invocations and assertions remain; the added refusal helper
  also checks every diagnostic-envelope field. Preliminary compile and test
  failures are retained in coder evidence. Incorrect new expectations were
  corrected to the exact old algorithms, without weakening prior assertions.
  The retained exact-root-join regression passes after the new normalizer's
  separator-budget correction.

Independent commands and results are recorded under
[`tests/native/r14d1-review-evidence/`](../../../tests/native/r14d1-review-evidence/):

- `python3 tests/native/r14d1-review-evidence/audit.py`: passed twice. It verifies
  candidate/artifact/protected hashes, exact helper bodies and renderer flow,
  pure normalizer non-use by render, three generations, actual native link and
  all 309 distinct raw full-gate passing names against the current registry.
- `python3 tests/native/r14d1-review-evidence/run-adversarial.py`: pinned compiler
  with `-Wall -Wextra -Wpedantic -Werror`, linking the actual product modules;
  passed 37 independent checks. These cover relative link physical traversal,
  lexical/physical divergence, final aliases after traversal, root clamping,
  trailing normalized cwd, valid Unicode, malformed UTF-8/NUL, invalid cwd
  shapes, exact 64 MiB cwd plus absolute argument and over-budget relative
  join/input. Two checks succeed with the ambient cwd deleted, demonstrating
  that normalization does not require ambient cwd or filesystem existence.
- `cmake --build build/rewrite-debug --target all planar_tests`: zero steps,
  `ninja: no work to do.`
- `ctest --test-dir build/rewrite-debug -R '^replacement\.(host path|intent path)' --output-on-failure`:
  **8/8 passed**, zero skips.
- `python3 tests/acceptance/bootstrap_cli.py build/rewrite-debug/bin/planar`:
  **247 real processes**, 60 host-path checks, six native goldens, independent
  canonical/hash/prose/section checks; zero skips.
- `python3 tests/vectors/generate.py --check`: 12 hand byte vectors, 20 upstream
  hashes and 41 canonical/domain vectors passed.

The coder's complete 309-case debug gate was independently audited against its
raw output and current registry rather than redundantly rerun. Its raw lint gate
checks 74 active C++ sources with formatting/Doxygen, and its no-change build does
no work. The protected-file and inventory checks preserve all earlier tests,
including the recorded 151 history, 25 capture, 12 prepared and 12 atomic cases.
There are no unexplained skips.

Reviewer evidence is frozen by `artifact-manifest.json`, SHA256
`04485790f12445e70f9ac79b4413f9608b7baa474ee074a866fbb5fb5a1feca3`.
The probe source and exact compiler/link command are retained; its temporary
executable was removed after verification. Evidence limits remain macOS Apple
Silicon debug only. No release/Linux, hostile filesystem race protection,
allocation fault injection or load-service behavior is claimed.

The debug directory is released: reviewer verification is complete and no builds
or tests remain running. Approval applies only while the frozen candidate remains
current; edits affecting this scope require renewed review.
