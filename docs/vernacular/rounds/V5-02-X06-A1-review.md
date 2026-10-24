# V5-02-X06-A1 independent review

Disposition: **approve**, combined pure interpretation-v2 codec and the four
scoped lower canonical-resource adapters. No required changes or unresolved
product choices. Approval is limited to this exact candidate and the structural
recorded-data boundary; it enables no reader or profile acceptance path.

Shared checkout: `dev/planar-v2-vernacular`, starting/current HEAD
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`, intentional dirty rewrite.
Reviewer ownership was only this report,
`tests/native/v5-02-x06-a1-review-evidence/`, and exclusive serialized
`build/rewrite-debug` verification. Product, candidate evidence, prior rounds,
release and install-release inputs remained read-only. No delegation, Planar
operational tools/skills/roles/queue, installed binary, commit, installation,
publication, merge or operator-data operation occurred. No others' edits were
reverted.

## Candidate and independent scope audit

Review considered the active guide, implementation/roadmap and v5 trust/history
direction, complete A1 brief and prerequisite amendment, approved X06-A review,
X06 implementation path, exact X-SHAPE/X-LIMIT/X-STORE, v1 codec, native
canonical/hash/tree/catalog/provenance boundaries and the runtime H recipe.

| Identity | SHA256 |
| --- | --- |
| Candidate JSON, all 25 files | `1b3831fbd549d15e38105dac2ad339da3a2c6223c812f92f07a45d1f130dbaec` |
| Exact reconstructed source/reference diff | `57f0832d9a2cb763dcac28d877ed6bd39f633f6a6f27eca978362f914587e7ea` |
| Author artifact manifest | `55410c2b3137563223a6a0aa4c5ef80ed34725c688b1bd228e4c8450192b9f11` |

The independent [candidate audit](../../../tests/native/v5-02-x06-a1-review-evidence/candidate-audit.json)
verifies all 25 current candidate hashes, all 49 author evidence entries and
three support files with exact membership/size/hash, original preedit copies
against the original baseline, and byte-for-byte diff reconstruction. Vector
artifacts are individually frozen in the candidate rather than included in the
text diff. The original 93,777-member baseline and 93,818-member final protection
are independently reconciled. The review's own full-hash baseline contains
93,879 members and matches every author-final protected item; the additional
61 entries are the author support/evidence subtree.

The unchanged v1 codec/interface/tests and exact external-profile contract are
verified against the original baseline. Runtime, shipped Lua, readers,
acceptance, storage and CLI source hashes remain unchanged. CMake only appends
the pure module/test registration. Eight status-document historical bodies
remain exact; the X06 path accurately distinguishes recursive const-facing
`value_ptr` from raw-tree or same-VM authority. All 509 local Markdown file links
in candidate documents resolve. AGENTS.md still links to CLAUDE.md and
compile_commands.json retains its original symlink identity/target.

## Source assessment

At [interpretation_v2_codec.cpp](../../../src/engine/history/interpretation_v2_codec.cpp),
lines 45–139 retain the v1 structural container checks locally and add the
required job shape and optional selected-shape structure. B/S jobs correspond
positionally with equal counts, sorted unique identity and matching portable
file paths. Lines 180–234 enforce exact E/selected fields, sorted unique E jobs,
tagged H syntax, normalized obligation source/ID/phase/quality and referenced
work/assignment/role correspondence, per-job/total obligation bounds, and
guidance UTF-8/no-NUL/byte limits. They do not rerun current declaration defaults,
shape semantics, graph policy or Lua.

Lines 236–311 construct a private verified context from raw files by calling
`verify_effective_stdlib`, independently decoding the verified raw embedded and
effective manifests, recovering actual ordered shipped/external paths and
contextually validating the asserted catalog. Canonical catalog equality,
recomputed native portable-tree digest and exact custom selection membership
must all agree. Classification follows verified catalog ownership. Both S and E
must contain the same descriptor ID/revision and recomputed **41-byte** tagged
H; V instead contains the **32-lowercase-hex** raw tree digest. The H preimage
uses the complete descriptor, logical `{tier,module}` owner without ordinal,
and lowercase hex of the exact original selected-module bytes. It matches
[runtime.cpp](../../../src/engine/runtime/runtime.cpp), lines 3566–3610.

Lines 313–376 preserve the agreed v2 precedence: both encoded byte preflights;
interpretation then separate-V canonical decode; fixed structure/selection
syntax and embedded-V equality; supported constants; raw-tree verification;
contextual catalog validity and exact cross-record correspondence. Broad catalog
component bounds may refuse before contextual validation as specified. Resource
errors remain distinct from malformed, unsupported, invalid tree and
correspondence errors. Encoder canonicalization handles aliases/cycles/depth
before inspection, derives V/E from its private verified context and passes the
same final checker. Public APIs accept no supplied digest, prepared-provenance
aggregate or catalog as authority. Returned values/bytes sever input aliases.

The lower changes are exactly three canonical-result adapters in
[effective_provenance.cpp](../../../src/engine/history/effective_provenance.cpp),
lines 151–157, 208–214 and 239–243, and one in
[provenance_codec.cpp](../../../src/engine/history/provenance_codec.cpp),
lines 129–135. They preserve `resource_limit` while retaining ordinary malformed
classification and success bytes. The actual pre-correction evidence is
[second-tests.xml](../../../tests/native/v5-02-x06-a1-evidence/second-tests.xml):
1,656 failed assertions, first decoder refusal 1244/4881, resource misclassified
as invalid_tree. This review inspected that preserved red run and snapshots;
it did not rebuild the pre-correction candidate or claim a second reproduction.
Top-level allocation sweeps independently reran successfully. Direct lower
sweeps allow unchanged lower raw bad_alloc sites to propagate; they verify the
returned canonical-adapter categories, not a new all-allocation lower API promise.

The source graph and native undefined-symbol audit show no Lua/runtime,
filesystem capture or SQLite/storage execution dependency. Existing transitive
`ledger_types` is pure vocabulary. Binary/non-callable original module bytes
are valid historical inputs: neither this result nor a self-consistent complete
alternate recorded tree proves callability, semantic honesty, live selection,
same-VM origin or acceptance authority.

## Independent verification

| Executed check | Result |
| --- | --- |
| CTest with the exact absolute focused list, JUnit output | **88/88**, zero failures/errors/skips/disabled, 43.94 seconds. |
| Private copied-source `[review]` checks against unchanged candidate libraries | **4 cases / 122 assertions**, zero failures/errors/skips. |
| Current JSON inventory versus approved C5 names | **892 names**, every prior **876** preserved, exactly 16 additions. |
| Serialized final `all planar_tests` build | `ninja: no work to do.` |
| Independent oracle and source/link/symbol audit | Pass; exact baseline H/tree and three independently recomputed changed H/tree vectors. |

The private cases add raw-file order invariance and decoder lifetime/ownership;
missing/extra/duplicate/reordered B/S correspondence through both public APIs;
combined malformed/future/tree/catalog precedence; and independently computed
changes to selected original bytes, descriptor help and logical module path.
Matching recomputed canonical records roundtrip, while old H claims against each
changed raw tree refuse. The verification-only Python grammar and pinned upstream
C xxHash first pass 11 established canonical byte controls and four portable
record byte/digest controls, then reproduce the original H/tree before producing
these three vectors. No native product codec, hash function or Lua generates the
oracle values.

Raw sources, argv, output XML and [private dependency proof](../../../tests/native/v5-02-x06-a1-review-evidence/private-proof.json)
are retained. The compile uses recursively expanded original response files,
explicit existing PCMs and disabled implicit modules. All 797 captured dependency
identities, including all 754 actual compile dependencies and Ninja journals,
retain exact mode/hash/mtime and other recorded stat attributes through the run.
The initial proof checker compared `catch2/../catch2` spelling to normalized
Ninja paths and failed after passing tests; its failure is preserved. Physical
normalization resolves the already captured identities without resnapshotting,
recompiling or rerunning tests. Loader output records existing pinned libc++;
there is no copied-runtime claim. Only owned disposable object/executable/dylib
outputs were removed after proof, with their hashes preserved.

Author evidence also records 16 new cases/49,579 assertions, pinned formatting
and Doxygen over 121 sources, independent vectors and original no-work gates.
These artifacts were audited; formatting/Doxygen were not redundantly rerun.
The focused rerun includes the new allocation sweeps and old v1/provenance,
catalog/tree/record/canonical/hash cases. Controlled one-shot bad_alloc injection
is not natural host exhaustion or an RSS guarantee. Enclosing byte limits may
prevent some simultaneous maximum component counts from fitting.

## Protection, limits and release

The [review protection audit](../../../tests/native/v5-02-x06-a1-review-evidence/protection-audit.json)
compares independent full-content hash boundaries, including root dotfiles,
symlink identities/targets, all build trees, every candidate/evidence file and
all prior rounds. Only .git, this review report/evidence and the exact coordinator
A1 round are exempt; only Debug verification outputs may change. Release and
install-release were never invoked. Candidate identities are rechecked after
verification. The final review manifest binds this report and all review
artifacts without a hash cycle.

No full 892-case configuration gate, release test/build, Linux, reader dispatch,
profile preparation/transactions, shared whole-invocation budget, public CLI or
mandatory external-profile completion is approved here. The latest complete
approved macOS gate remains C5's 876/configuration. Public init creates format1;
X06 follow-on slices, X07, X08, execution/RA01 and Linux remain open.

**Debug and all reviewer files are released. No build/test process remains.**
