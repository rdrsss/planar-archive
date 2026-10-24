# V5-02-X01 independent spike review

Disposition: **approve**. No blocking findings or requested changes within the
frozen test-only X01 scope. The spike supplies bounded CLI11 evidence for X02;
it does not approve a product envelope ABI or implement external loading.

Reviewed the shared `dev/planar-v2-vernacular` checkout at HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a` against CLAUDE.md, the implementation
plan/roadmap/v5 direction, X01 narrowed brief and freeze, external-envelope-spike,
proposal lines 145–218, command-contract external gate and raw author evidence.
Reviewer writes are confined to this report and
`tests/native/v5-02-x01-review-evidence/`. No source changes, reversions, Planar
tools/skills/queue, installed binary, operator data, commits, installation,
publication or merge occurred.

The repository-relative seven-file `candidate.json` SHA256 is
`60d8c3c02171da96654458d13f06d36304b756564a90dc5eb3f7c861c83cd8f9`;
`owned.diff` is
`fd081d7888b4ee2b366b063783713161c2bfec9527077e288ce7ec47faf09c41`.
The repository-relative 13-entry built `artifacts.json` SHA256 is
`a8b0247f79960d791fc749b9c9ae34644ad7052870b3fedaf2208d4c80693ad3`;
the 46-entry author-evidence-relative `evidence-sha256.json` SHA256 is
`975210c41656e881b3c4526a80d106d9cf7f3631892529c90426cfc824cc88fe`.
Every entry and all four manifest identities matched. Candidate and compiled
artifact bytes still matched after independent verification.

Independently checked all 14,642 baseline file hashes, current additions under
the baseline's `.git`/`build` directory exclusions, and the original bootstrap
test prefix. Only assigned candidate paths, the preserved AGENTS.md symlink view
and coordinator round changed. Additions are scoped X01/report/evidence files.
Historical/product/golden bytes remain unchanged. AGENTS.md still targets
CLAUDE.md. The status-only documentation changes agree with D8 approval and retain
the recovery, Linux, execution and external-profile limits.

`src/cmd/planar/bootstrap.t.cpp:528` uses the actual five handler binding functions
and pinned CLI11 module with a small extension fixture. Option state outlives
parsing. The parser at line 577 takes its tail only from
`remaining_for_passthrough()`, passes copies of the library's reversed parse stack
to subsequent parsers, and reverses only to preserve/report the original order.
This agrees with `vendor/cli11/9d6a/include/CLI/impl/App_inl.hpp:1219`. The existing
whole-invocation name=value guard is reproduced exactly; there is no additional
option scan, split or shadow parser. Parsed metadata accounts for discarded
terminators. Known shipped parse failures return before final-catalog parsing.

The report accurately labels raw observations: CLI11 accepts empty/option-looking
profile scalars and a plain prefix probe permits zero-subcommand leading flags.
It does not claim those satisfy the proposed pre-profile policy. The separate
scalar test at line 847 verifies the existing explicit-value policy through
CLI11 validation. The optional positional probe at line 796 uses CLI11
`positionals_at_end()` and exact metadata/vector equality: unknown verb tails
retain shipped-looking words, option-looking values and terminators; leading
unknown flags/terminators and malformed shipped invocations are refused. The
44-row matrix distinguishes tail selector-looking values from selectors, pins
help ordering/duplicates/tails, and records final-parse attempts. The extension
fixture's permissive text values are correctly described as library observations,
not a settled extension text ABI.

Independent debug commands, UTC/status records, raw logs, JUnit and archived
LastTest are in the review evidence directory. JUnit/LastTest were saved before
inventory. Build and no-change build both reported `ninja: no work to do.`.
Spike **3/3**, **350 assertions** (309/24/17); focused **41/41**, including all
**seven CLI suites**, zero failures/errors/skips. All 56 printed matrix/probe rows
equal the author's frozen rows. Inventory **405** preserves all **402** baseline
names and adds exactly the three X01 cases. Pinned formatting/Doxygen passed for
**95 active sources**. No full-suite or release rerun was needed or performed.

X02 must review the combined amended envelope/probe/value/help contract and its
implementation tests. The two amendments are separately demonstrated here, not
an integrated production parser. Profile capture/trust/overlap, module admission
and sealing, Lua capabilities/budgets, shapes/projections/identity, help rendering,
response schema, codecs/coexistence and removed-profile replay remain unproved
and gated. Initial external verbs remain read/projection-only. Evidence is macOS
ARM64 debug only. Debug is released idle; release was untouched.
