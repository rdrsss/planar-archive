# N03-A independent codec review

Disposition: **request-changes**.

Reviewed the shared checkout on `dev/planar-v2-vernacular` at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. Product and coder evidence were
read-only; reviewer ownership was this report and `tests/native/n03a-review-evidence/`.
No other builder, Planar tool, installed binary, database, commit, installation,
publication, or delegation was used. Unrelated changes were preserved.

## Finding N03-A-01 — malformed objects pass the strict decoder (high)

`src/engine/json_codec/codec.cpp:260` trusts a successful structural Glaze read
as complete JSON syntax validation. Its object alternative at line 10 is a
`vector<pair<string,node>>`. In the pinned Glaze reader this path accepts `}`
at the top of every member iteration (`vendor/glaze/a4a7/include/glaze/json/read.hpp:2536`)
and merely consumes a comma if present after each member (line 2601). It does
not require the separator or require a member after a comma.

The independent public API probe demonstrates that both invalid inputs succeed:

```json
{"a":1,}
{"a":1 "b":2}
```

The first has a trailing comma; the second omits the required comma. Both are
converted to ordinary values instead of receiving a `syntax` refusal. This
violates N03-A's strict-input contract and would weaken the existing Lua JSON
contract upon integration. Existing tests cover array separator failures but
omit these object cases.

Required correction: use Glaze to validate complete object grammar and add
meaningful regressions for both cases and nested variants. Preserve decoded
duplicate detection, safe byte offsets, the raw-byte/structural ceilings, and
the requirement that Glaze owns grammar and string escaping. Do not implement
a native punctuation scanner or patch the pinned vendor tree.

A bounded library-only experiment confirms a suitable route exists:
`glz::validate_json` (`read.hpp:5217`) rejects these forms, adjacent members
without whitespace, nested trailing commas, malformed arrays, and trailing
values. It accepts valid objects and leaves duplicate-key policy to the
structural adapter. The experiment is evidence for corrective search, not a
reviewed integration design. Any added validation pass must remain bounded and
retain the adapter's error/resource semantics.

## Identity and verification

Before review, `shasum -a 256 -c` passed for all six candidate files and all
20 coordinator-frozen evidence files. The same files were rehashed after all
probes, with every entry unchanged. Candidate manifest SHA256:
`be91023a004e95f84438bef200dec496f3d4afb98f2bb61e0d82c77df6e65131`.
Frozen-evidence manifest SHA256:
`fb2ced609623048e81a0eb7aa1b82d11f0349200e9f560a5e273c126bbfee5f7`.
`git diff --exit-code HEAD -- vendor/glaze` passed before and after; 663 files
remain restored, matching HEAD tree
`a72092a5a3df75fb952bc4646ed5a345d4c54746`.

Executed verification, serialized:

- Compiled `tests/native/n03a-review-evidence/probe.cpp` with the pinned LLVM,
  libc++, existing module map and exact candidate static libraries. Command
  and empty successful compiler output are preserved in `compile-command.json`
  and `compile.log`.
- Ran that independent public API probe: **243 checks, 2 failures**, both the
  finding above. `probe.log` preserves the failures. Other checks cover numeric
  overflow/forms, object/array truncation and separators, decoded duplicate
  Unicode keys, malformed UTF-8 in values/keys, surrogate boundaries, all 32
  C0 output bytes, valid round trips, mixed 128/129-container depths, zero
  budgets, shared references, indirect cycles, native integer range and nested
  syntax offsets. Probe does not mirror the adapter's internal algorithm.
- Compiled and ran the library-only `glaze-validation.cpp`: **15 cases,
  0 failures**, including 9 invalid and 6 valid inputs. Commands and output
  are in `glaze-validation-command.json`, `glaze-validation-compile.log`, and
  `glaze-validation.log`.
- Re-ran `build/rewrite-debug/src/engine/planar_json_codec_tests '[codec]'`:
  **6 cases / 81 assertions pass**, zero skips (`codec-tests.log`). This also
  exercises exact 1 MiB input, 100,000 decoder nodes, 128 decoder containers,
  16 MiB output, 256 encoder containers, 4,000,000 encoder visits and refusals
  immediately beyond them. These passing tests do not resolve the independent
  syntax finding.
- Final identity evidence is `identity-after.json`.

## Other assessed boundaries and limitations

No additional blocking finding was identified in the scoped inspection.
Grammar/numeric conversion/string escaping calls are delegated to Glaze; the
postparse `.eE` check enforces integer-only application policy. Native output
validation precedes structural expansion. Checked byte subtraction and hard
ceilings bound the `6 * minimum_bytes + 1024` fixed writer buffer to 96 MiB +
1 KiB; parsed/converted structural copies and the caller's source tree are
additional memory. These are native allocations, not Lua allocator charges or
a total RSS guarantee. This review did not inject allocation failure, measure
RSS, run sanitizers, or exercise concurrent callers. Context/state is local to
each call on inspection; no shared mutable codec state was found. Diagnostics
use fixed safe text without host paths. Binary canonical encoding is untouched.

The codec is additive and isolated; existing product consumers still use the
previous custom JSON paths. This review does not approve the user's overall
JSON replacement goal. A fresh corrective N03-A round must resolve this finding
before N03-B integration/removal, followed by its independent review and full
product gates. Existing 113 product tests, release and Linux gates were not
rerun in this focused review.
