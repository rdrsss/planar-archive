# V5-02-X05-C2-B0 independent review

Verdict: **approve**, scoped to test-only borrowed Glaze counting feasibility.
No blocking findings. This approves neither a production helper nor projection
execution/admission, rooted Lua scanning, budget integration, public admission,
reserved-section policy, a full milestone, release runtime or Linux acceptance.

The shared checkout is `dev/planar-v2-vernacular` at
`3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`. The reviewer was not alone in the
checkout, preserved all pre-existing changes, and owned only this report and
`tests/native/v5-02-x05-c2-b0-review-evidence/`. Product, registered tests, author
support/evidence, coordinator rounds, historical/current prose and shared builds
were read-only. No Planar tools, skills, roles, queues, subagents, shared rebuilds,
suites, commits, installation, publication, merge or operator data were used.

The review read the active guide, implementation/roadmap/v5 direction, complete
B0 brief and C2-B refinement, exact X-PROJ/X-LIMIT, author report and source,
pinned Glaze paths, unchanged codec options and existing reserved-encoding and
native transport accounting. The protected baseline was captured before reviewer
production of reports or probes; `baseline.json`, `identity.json` and
`git-before.txt` retain it.

## Candidate and provenance

The approved final support identity is the three-entry
`tests/native/v5-02-x05-c2-b0-evidence/frozen-support.json`, SHA256
`83907021ad6e9ddb84498f81fa28dc77d3d5d9709984e2bee55af4f453da78f2`.
The 79-entry author artifact manifest SHA256 is
`cabab27a6298b0eeb8ce38898bd20167b5ee60c97dcee2efd292d1bc30a1f408`;
the author report SHA256 is
`c13071e5a1dfbfa6dc166025fe402f534436d6a15c099042057b4a7728616674`.
The counting source SHA256 is
`6d5dfa467bd6c1ef07b11caa8a9c9cca5b2cb8f06e6d4ac2cf305e04f05e59e0`.

Independent `verify_candidate.py` checked every manifest/support entry and all
seven nested copied-compiled manifests, including failed/historical attempts:
888 entries each, exact private membership, every actual byte digest, and exact
mapping back to the 888 unchanged shared compiled inputs. Expanded author compile
argv contain private module inputs and link argv contain private archives. Final
corrected source equals the frozen support source. Older frozen support manifests
are historical and do not supersede the final manifest. All checks passed;
`candidate-audit.json` is the raw result. The author report's preserved failed
attempts and corrected protection selector accurately delimit its final evidence.

## Source findings

The prototype delegates JSON syntax/escaping to the pinned Glaze writer with the
same options as `src/engine/json_codec/codec.cpp:111`. Its fixed array/span of
borrowed section views creates no generic value graph for counting. Count returns
only an enum and counters, destroys scratch before return, and retains no encoded
output. Fixture ownership proves synchronous view lifetime only.

The stock stream adapter is unsuitable as a fixed 64 KiB allocation contract:
`core/ostream_buffer.hpp:58`, `:88` and `core/buffer_traits.hpp:174` allow growth
using absolute logical indices. Its incremental flush also passes through the
unconditionally `noexcept` helper at `buffer_traits.hpp:108`. A throwing sink
therefore terminates. Both properties were independently reproduced.

The submitted fixed traits avoid vector-like resizing. Bounded capacity checks
compare logical required size against offset plus fixed physical storage. The
adapter advances its offset only after consuming a complete flush; the sink does
not dereference chunk bytes, allocate or throw. Sticky overflow uses subtraction
before addition and remains distinct from Glaze scratch refusal and allocation
failure. Counting does not immediately cancel serialization after output overflow;
it finishes the already-admitted bounded input, then refuses. The finite bound
is acceptable for this feasibility scope and is explicitly disclosed.

The scratch derivation holds for exactly the stated fixed reflected types and
admission preconditions: version1, <=64 sections, declaration IDs of <=64 ASCII
bytes, <=1 MiB text each and <=16 MiB raw text total. In
`json/write.hpp:1239` bounded storage bypasses array preallocation;
`:1160` and `:1395` flush at separators. After each separator, unflushed data is
<32768 bytes; the first section begins behind a much smaller root prefix.
The non-skipping reflected object path (`:2469`) has no field flush. Strings
reserve `ix+10+6*n` (`:861`), and static field checks need at most
round16(9+256)=272 bytes. A whole maximum section is <=6 MiB+101 bytes. These
field, string, separator and closing positions fit the report's conservative
32768+6 MiB+1024 = **6,325,248-byte** scratch. Low-level unchecked dumps were
reviewed with their caller padding, rather than assumed independently checked.
Total logical output/work remains below 100,669,888 bytes under the admitted raw
bound, so offset/streamsize arithmetic is also bounded on the pinned target.

The actual bounded call makes one vector<char> allocation of precisely 6,325,248
bytes. Its pinned ordinary-new storage requires no aligned allocation observer;
all remaining writer/sink/error operations are fixed nonallocating operations.
Constructor failure is caught outside the writer, and flush/finalize are
nonthrowing. Future code must precharge this allocation before construction and
charge fixed logical workspace, with no refund/reset. It must preserve the
separate scratch-defect, encoded-output and resource failure classifications.
No production charging or Lua longjmp/rooted lifetime boundary was demonstrated.

## Independent verification

All compilation outputs, objects, PCM copies, link dependencies and commands are
inside the reviewer evidence directory. Before compilation, all 888 shared
.o/.pcm/.a files were re-enumerated, hashed, copied privately and hash checked.
The reviewer also disabled implicit modules/module maps; compilation succeeded
using explicit private module inputs. No shared compile or suite was requested.

Executed commands and raw stdout/stderr are in `commands.json`,
`rerun-commands.json` and their named logs:

- Fresh submitted compile/link/run: **17 cases, 308 assertions, 61 actual
  allocation fault positions**, passing. This reproduces exact/below/above
  16 MiB encoding, worst escaping, 64 sections and all submitted controls.
- Fresh independent adversarial compile/link/run: **20 cases, 391 assertions,
  83 actual allocation fault positions**, passing. Nineteen fixtures place the
  real Glaze-derived pending prefix at threshold-9 through threshold+9 before a
  maximum 1 MiB sixfold-escaped string with maximum IDs. Another uses 64 mixed
  valid UTF-8/control sections. Additional assertions exercise five tighter sink
  ceilings including exact success, four fixed scratch capacities, finalization,
  checked count invariants, and sticky negative-length refusal. Each fixture
  compares counting to real Glaze output and unchanged codec_encode.
- Independent throwing-flush child: expected **SIGABRT (-6)** and libc++abi
  bad_alloc diagnostic, preserving proof that throwing cancellation is unsafe.
- Pinned clang-format `--dry-run --Werror` over the frozen author source passes.

The reviewer harness's first compile renamed main while including the submitted
source, triggering -Wreturn-type because implicit return is specific to main.
The failed source/command/log are preserved in `attempt-1/`. A local diagnostic
scope around the unused renamed fixture main fixes only the reviewer harness;
no author or product change was made. The corrected independent run passed.
These are standalone spike counts, not registered Catch2/CTest counts.

Final `final_check.py` confirms exact membership and SHA256 equality with the
reviewer's baseline: **2,286** source/vendor/root/reference files, **20,807**
historical test/evidence files, **890** Debug compiled/metadata files (all **888**
compiled files plus compile_commands.json/build.ninja), and all **3,492** release
files. It separately rechecks the author's final 3 support and 79 artifact entries,
the reviewer's 888 private copies, unchanged HEAD and AGENTS.md -> CLAUDE.md.
`protected-final.json` and its log retain the result. The reviewer artifact
manifest binds this report and reviewer evidence, with private dependencies
bound through `copied-compiled.json`; the manifest excludes itself.

All reviewer edits are released. No later edits are planned. Ordinary dependent
production work still requires a separately scoped fresh implementation/review;
this feasibility approval supplies no reserved-ID policy decision.
