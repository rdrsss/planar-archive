# V5-02-X01 CLI11 envelope spike

Candidate for independent review. This is a test-only spike; no external profile
loading is present in the shipped binary. The approved R14-D8 baseline has
402 registered tests and independently reviewed 402/402 debug/release results.
This spike adds three test cases; it does not rerun the full milestone or release.

The fixture in `src/cmd/planar/bootstrap.t.cpp` uses the pinned CLI11 named module,
the five actual shipped handler bindings (including their validators), and one
small final-catalog fixture: `inspect --text TEXT [--flag]`. It never creates a VM,
reads a profile, registers Lua, captures files, performs replay or calls handlers.
The final fixture models catalog shape, not catalog admission, sealing or trust.

## Parsing observations and bounded recommendation

The initial root recognizes shipped commands and a `with-profile` subcommand.
The envelope uses `prefix_command()`, a required scalar selector with Throw
multiplicity, and a plain counted help flag. At the first unrecognized token it
retains the suffix. `remaining_for_passthrough()` returns **reverse argv order**,
ready for `CLI::App::parse(vector&)`; reversing it for reporting recovers the
exact original suffix. Both the embedded probe and fresh final catalog receive
copies of this original parse-stack vector. No manual argument splitting or
option scan is used. The existing whole-invocation `--name=value` guard is copied
exactly; CLI11 owns all other lexical recognition.

The first matrix characterizes that recipe, including metadata checks, nonempty
selector policy and the proposed help convention. A label `accepted` means only
these test parsers accepted; it does not mean a profile or command would execute.
`prefix:metadata`, `probe:metadata` and `final:metadata` are test wrapper checks,
not CLI11 exception names. Exception rows report CLI11's actual `get_name()`.
Counts are command selection counts plus twice each parsed scalar option count
and once each parsed flag count. Even failed parses expose partial counts; they
are observations, not proof of a successfully consumed invocation.

Two amendments are needed before adopting the draft recipe:

1. Add the existing explicit-value validator (`!empty && !starts_with("--")`)
   to the profile scalar. Raw CLI11 accepts `--help`, `--bogus` and `--` as values
   immediately after `--stdlib-profile`, and accepts empty scalar strings.
   The scalar-only third test proves the existing validator rejects those values
   through CLI11's normal validation API. `./--help` remains an explicit filename.
   Validating the parsed scalar is not another argument parser. No logic should
   reinterpret a missing selector value as a verb.
2. Make the embedded probe distinguish a syntactically valid unknown command
   from a leading unknown option **before** profile capture. A probe with only
   `prefix_command()` cannot do that: `--bogus inspect ...` remains opaque and
   fails only in the final parser. The second fixture proves a library-only
   solution: add an optional vector positional `opaque-tail` with `expected(-1)`
   and `positionals_at_end()` to the embedded-catalog probe. Unknown command
   tails become exact ordered positional data; their option-looking values,
   shipped-looking words and terminators remain uninterpreted. A leading unknown
   flag or terminator leaves zero positional bindings and is refused from CLI11
   metadata. Known shipped commands still select their real fixed handlers;
   malformed signatures never reach final catalog parsing. Require either one
   completely parsed shipped command, or a nonempty positional vector exactly
   equal to the original tail, with no extras and exact count equality. This is
   a bounded recommendation for X02 review, not a changed product contract.

Metadata rejects discarded terminators around known options/commands. For unknown
commands a terminator inside the opaque suffix is intentionally retained until
final parsing, where the fixture rejects a discarded terminator by token count.
`--` consumed as a scalar value is a separate case addressed by value validation.
Do not reject a `--stdlib-profile` token after a verb merely because its spelling
looks like the selector: the raw fixture accepts it as a text option's value.
Final catalog value policy remains explicit; the real shipped bindings already
reject text values beginning `--`. The test extension deliberately omits that
validator to expose CLI11 behavior, and does not establish an extension text ABI.

Help is a normal counted option, with default CLI11 help behavior disabled.
The fixture accepts both `with-profile --stdlib-profile p --help` and
`with-profile --help --stdlib-profile p`; it rejects help plus a tail, duplicate
help and per-command help. Recommend documenting either option order for the
selected-catalog help exception. Enforcing one spelling would need separately
reviewed CLI11 parse-order metadata checks. No help capture or stable help renderer
is implemented. Existing public `planar --help` stays covered by bootstrap tests.

## Exact argv and results

Every argv omits only executable `planar`. P/Q/F are phase-one, embedded-probe and
final-catalog count observations; final says whether final parse was attempted.
The complete exact ordered tails and profile strings are in `matrix.json` and
raw `spike-final.log`; the assertions pin those alongside every table outcome.

| Case | argv | Outcome | P/Q/F | final |
| --- | --- | --- | --- | --- |
| ordered-extension | `["with-profile", "--stdlib-profile", "p.json", "inspect", "--text", "value", "--flag"]` | `accepted` | 3/0/4 | true |
| shipped-history | `["with-profile", "--stdlib-profile", "p.json", "history", "--ledger", "missing"]` | `accepted` | 3/3/3 | true |
| no-selector | `["with-profile", "inspect", "--text", "value"]` | `prefix:RequiredError` | 1/0/0 | false |
| no-value | `["with-profile", "--stdlib-profile"]` | `prefix:ArgumentMismatch` | 1/0/0 | false |
| empty-value | `["with-profile", "--stdlib-profile", "", "inspect", "--text", "value"]` | `prefix:metadata` | 3/0/0 | false |
| verb-as-value | `["with-profile", "--stdlib-profile", "inspect", "--text", "value"]` | `final:ExtrasError` | 3/0/0 | true |
| duplicate-selector | `["with-profile", "--stdlib-profile", "p", "--stdlib-profile", "q", "inspect", "--text", "v"]` | `prefix:ArgumentMismatch` | 5/0/0 | false |
| unknown-option-value | `["with-profile", "--stdlib-profile", "--bogus", "inspect", "--text", "v"]` | `accepted` | 3/0/3 | true |
| known-option-value | `["with-profile", "--stdlib-profile", "--help", "inspect", "--text", "v"]` | `accepted` | 3/0/3 | true |
| missing-help-value | `["with-profile", "--stdlib-profile", "--help"]` | `prefix:empty-tail` | 3/0/0 | false |
| extra-prefix-option | `["with-profile", "--stdlib-profile", "p", "--bogus", "inspect", "--text", "v"]` | `final:ExtrasError` | 3/0/3 | true |
| option-before-selector | `["with-profile", "--bogus", "--stdlib-profile", "p", "inspect", "--text", "v"]` | `prefix:RequiredError` | 1/0/0 | false |
| unknown-verb | `["with-profile", "--stdlib-profile", "p", "absent", "--text", "v"]` | `final:ExtrasError` | 3/0/0 | true |
| profile-as-tail-value | `["with-profile", "--stdlib-profile", "p", "inspect", "--text", "--stdlib-profile"]` | `accepted` | 3/0/3 | true |
| tail-selector | `["with-profile", "--stdlib-profile", "p", "inspect", "--stdlib-profile", "q", "--text", "v"]` | `final:ExtrasError` | 3/0/3 | true |
| tail-option-value | `["with-profile", "--stdlib-profile", "p", "inspect", "--text", "--bogus"]` | `accepted` | 3/0/3 | true |
| tail-known-option-value | `["with-profile", "--stdlib-profile", "p", "inspect", "--text", "--flag"]` | `accepted` | 3/0/3 | true |
| no-tail | `["with-profile", "--stdlib-profile", "p"]` | `prefix:empty-tail` | 3/0/0 | false |
| prefix-terminator | `["with-profile", "--stdlib-profile", "p", "--", "inspect", "--text", "v"]` | `final:metadata` | 3/0/0 | true |
| tail-terminator | `["with-profile", "--stdlib-profile", "p", "inspect", "--text", "v", "--"]` | `final:metadata` | 3/0/3 | true |
| terminator-value | `["with-profile", "--stdlib-profile", "--", "inspect", "--text", "v"]` | `accepted` | 3/0/3 | true |
| selected-help | `["with-profile", "--stdlib-profile", "p", "--help"]` | `selected-help` | 4/0/0 | false |
| help-before-selector | `["with-profile", "--help", "--stdlib-profile", "p"]` | `selected-help` | 4/0/0 | false |
| help-with-tail | `["with-profile", "--stdlib-profile", "p", "--help", "inspect", "--text", "v"]` | `prefix:help-tail` | 4/0/0 | false |
| help-after-verb | `["with-profile", "--stdlib-profile", "p", "inspect", "--help"]` | `final:RequiredError` | 3/0/1 | true |
| duplicate-help | `["with-profile", "--stdlib-profile", "p", "--help", "--help"]` | `prefix:ArgumentMismatch` | 5/0/0 | false |
| shipped-missing | `["with-profile", "--stdlib-profile", "p", "history"]` | `probe:RequiredError` | 3/1/0 | false |
| shipped-duplicate | `["with-profile", "--stdlib-profile", "p", "history", "--ledger", "a", "--ledger", "b"]` | `probe:ArgumentMismatch` | 3/5/0 | false |
| shipped-extra | `["with-profile", "--stdlib-profile", "p", "history", "--ledger", "a", "--bogus", "b"]` | `probe:ExtrasError` | 3/3/0 | false |
| shipped-terminator | `["with-profile", "--stdlib-profile", "p", "history", "--ledger", "a", "--"]` | `probe:metadata` | 3/3/0 | false |
| name-equals | `["with-profile", "--stdlib-profile=p", "inspect", "--text", "v"]` | `syntax` | 0/0/0 | false |
| tail-name-equals | `["with-profile", "--stdlib-profile", "p", "inspect", "--text=v"]` | `syntax` | 0/0/0 | false |
| root-terminator | `["--", "with-profile", "--stdlib-profile", "p", "inspect", "--text", "v"]` | `prefix:metadata` | 3/0/0 | false |
| shipped-init | `["with-profile", "--stdlib-profile", "p", "init", "--ledger", "a", "--workspace-id", "w"]` | `accepted` | 3/5/5 | true |
| shipped-export | `["with-profile", "--stdlib-profile", "p", "export", "--ledger", "a", "--revision", "1", "--target", "out"]` | `accepted` | 3/7/7 | true |
| shipped-load | `["with-profile", "--stdlib-profile", "p", "load", "--ledger", "a", "--workspace", "w", "--registry", "r", "--expect-revision", "none", "--request-id", "k", "--actor", "a"]` | `accepted` | 3/13/13 | true |
| shipped-render | `["with-profile", "--stdlib-profile", "p", "render", "--workspace", "w", "--registry", "r", "--root", "s=src", "--job", "j", "--role", "r"]` | `accepted` | 3/11/11 | true |
| init-missing | `["with-profile", "--stdlib-profile", "p", "init", "--ledger", "a"]` | `probe:RequiredError` | 3/3/0 | false |
| export-malformed | `["with-profile", "--stdlib-profile", "p", "export", "--ledger", "a", "--revision", "bad", "--target", "out"]` | `probe:ValidationError` | 3/7/0 | false |
| load-missing | `["with-profile", "--stdlib-profile", "p", "load", "--ledger", "a"]` | `probe:RequiredError` | 3/3/0 | false |
| render-missing | `["with-profile", "--stdlib-profile", "p", "render", "--workspace", "w"]` | `probe:RequiredError` | 3/3/0 | false |
| help-terminator | `["with-profile", "--stdlib-profile", "p", "--help", "--"]` | `prefix:help-tail` | 4/0/0 | false |
| empty-tail-value | `["with-profile", "--stdlib-profile", "p", "inspect", "--text", ""]` | `accepted` | 3/0/3 | true |
| second-command | `["with-profile", "--stdlib-profile", "p", "history", "--ledger", "a", "inspect", "--text", "v"]` | `probe:ExtrasError` | 3/3/0 | false |

The separate opaque-probe matrix uses the exact tails below. All accepted unknown
tails are byte/order-equal to their CLI11 positional vector. Failure rows never
admit a profile; this fixture performs no profile operation in any row.

| Tail argv | Admitted | Shipped | Count | CLI11 error |
| --- | --- | --- | --- | --- |
| `["inspect", "--text", "v", "--flag"]` | 1 | 0 | 4 | `` |
| `["inspect", "history", "--text", "--stdlib-profile"]` | 1 | 0 | 4 | `` |
| `["absent", "--unknown", "--", "history"]` | 1 | 0 | 4 | `` |
| `["--bogus", "inspect", "--text", "v"]` | 0 | 0 | 0 | `` |
| `["--help"]` | 0 | 0 | 0 | `` |
| `["--", "inspect", "--text", "v"]` | 0 | 0 | 0 | `` |
| `["history", "--ledger", "a"]` | 1 | 1 | 3 | `` |
| `["history"]` | 0 | 1 | 1 | `RequiredError` |
| `["history", "--ledger", "a", "--bogus", "b"]` | 0 | 1 | 3 | `ExtrasError` |
| `["history", "--ledger", "a", "--"]` | 0 | 1 | 3 | `` |
| `["history", "--ledger", "a", "inspect", "--text", "v"]` | 0 | 1 | 3 | `ExtrasError` |
| `[]` | 0 | 0 | 0 | `` |

## Verification and limits

Direct debug `cmake --build build/rewrite-debug --target all planar_tests` succeeds.
Final spike selection passes 3/3 CTest cases: 309 assertions in the 44-row matrix,
24 assertions in the 12-row opaque probe, and 17 assertions in the seven-value
scalar fixture. Focused verification passes **41/41**, zero failures/skips,
including all command-unit tests, host-path/workspace-selector cases and all
seven actual CLI suites. Current inventory is **405** and preserves every one
of the 402 baseline names. The seven CLI suites exercise the existing shipped
product; none receives external profile support from these fixture parsers.

`python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug` passes
pinned formatting and Doxygen for 95 active sources. The final direct build
reports `ninja: no work to do.`. Raw JUnit and LastTest were archived before
inventory. Exact command argv, UTC start/end and exit statuses for final checks
are in `commands.json`. Development build logs retain the broad generated-variant
rebuilds triggered by CMake regeneration after test-file edits; no generator,
variant input or CMake source was edited. No source edit occurred during final
checks. Documentation/status/evidence finalization follows those checks.

The first 34-case and second 44-case matrix runs are exploratory library
observations, with weak outcome assertions; they are retained, not counted as the
final meaningful verification. Final code freezes all 44 outcomes, profile values,
ordered tails, three count fields and final-attempt flags, plus the separate
12-tail probe admission/shipped checks and seven profile-scalar cases. One
intermediate build failed because a test-local snapshot lambda was not inserted;
`build-second.log` preserves it. This was a spike authoring failure, with no
product parser fix or product regression claimed.

No external loading, module ordering/admission/immutability, native trust or
physical overlap, Lua budget/capability, format coexistence, new codec, removed-
profile replay, actual selected-catalog help, response schema, shape or projection
behavior is demonstrated here. X02 must review exact envelopes and these APIs,
selected-shape identity and response alternatives, and concrete physical-overlap
policy alternatives before dependent implementation. Initial external verbs
remain read/projection-only under the already accepted authority boundary.

No product parser/loader/schema/codec/CMake/vendor change, full milestone/release
rerun, Linux execution, installed binary or operator data access is included.
Historical evidence and goldens remain protected. Candidate approval belongs to
a fresh independent reviewer.
