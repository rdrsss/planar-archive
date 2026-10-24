# X05-C2-B0 borrowed Glaze counting feasibility

Candidate submitted for fresh independent review; no self-approval. Shared checkout
`dev/planar-v2-vernacular`, HEAD `3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a`.
Author ownership is only the new `v5-02-x05-c2-b0-support/` and
`v5-02-x05-c2-b0-evidence/` directories. Existing changes were preserved. No
product, registered tests, CMake, codec, Lua, CLI, provider, policy or current prose
was modified. No shared build/suite was run. No Planar tools/skills/roles/queues,
subagents, commit, installation, publication, merge or operator data were used.
The active guide, plan/roadmap/v5, complete B0 and B refinement, exact X-PROJ and
X-LIMIT, codec options/reservations and pinned writer/stream buffer traits were
read. Pending reserved-section policy has no effect on this spike.

## Result

A bounded adapter using pinned Glaze serialization and `buffer_traits` is feasible.
It counts before any generic native AST copy: a fixed 64-element array contains
borrowed ID/text string views, and a span selects the occupied prefix. The counter
returns a small typed status and size, retaining no JSON output. Fixture-owned
strings stay alive throughout each synchronous call. Glaze alone writes numeric,
boolean, array/object syntax and string escaping.

The stock `glz::basic_ostream_buffer` is **not** a 64KiB scratch guarantee. It uses
an ordinary allocator vector and absolute logical indices. A sixfold-escaped
1MiB text requires 12,583,032 bytes in its second actual allocation, plus its
65,536-byte first allocation. On the 64-section/16MiB control-text fixture,
observed cumulative requests reach 198,260,890 bytes across seven requests. All
request sizes are logged, and all observed request positions are faulted.

The submitted alternative has one fixed 6,325,248-byte vector allocation and no
resize API. Its traits identify it as bounded and streaming, so Glaze's own
`ensure_space` checks physical remaining capacity. It implements the pinned
stream adapter's offset/flush/finalize mechanics without JSON-specific writing.
Scratch is transient, reused after each flush and destroyed before return; no
complete encoded result string is accumulated by counting. For small results,
scratch naturally contains the final chunk, then is destroyed.

Overflow is sticky and nonthrowing. The writer continues over the bounded source;
there is **no immediate cancellation claim**. Once a write would exceed 16MiB,
the sink records overflow, stops adding to its count, and ignores subsequent
writes. After normal writer/finalize return, the caller receives typed overflow.
The returned partial count on refusal is not the full encoded size. The sink
never dereferences content. Bounds use checked subtraction before addition.

## Pinned source proof and allocation contract

Paths below are protected by `source-before.json` and `protected-final.json`.

* `src/engine/json_codec/codec.cpp:111–116` enables the same three codec options:
  trailing-whitespace validation, UTF-8 validation and control escaping. The
  prototype copies these options, not an alternative escaping policy.
* `vendor/glaze/a4a7/include/glaze/json/write.hpp:861–890` reserves
  `ix + 10 + 6*n` for strings under control escaping. The actual writer preserves
  non-ASCII UTF-8 bytes; no `escaped_length` or handwritten escaping counter is used.
* `core/ostream_buffer.hpp:58–62,88–95,117–125` constructs/grows ordinary vector
  storage, maps logical indices through a flush offset and flushes at half the
  default capacity. `core/buffer_traits.hpp:174–185` doubles the **absolute**
  required logical size for vector-like buffers. The stream's resize then
  allocates that logical size minus its offset. Thus 64KiB is only an initial
  allocation/flush threshold; it cannot be used as a native accounting bound.
* `core/buffer_traits.hpp:108–112` declares `flush_buffer` unconditionally
  `noexcept`. A throwing stream sink reached through incremental flush therefore
  terminates. The isolated `--throw-flush` subprocess calls the real helper and
  exits with SIGABRT (-6); `final-corrected/throw-flush.log` preserves libc++abi's
  uncaught `std::bad_alloc` diagnostic. No throw is used for counter overflow.
* `json/write.hpp:1239` skips array preallocation for bounded buffers;
  `1160–1188,1395–1407` check/write the compact array separator, flush there,
  serialize the next section and propagate context errors.
* These fixed reflected types have no skipped fields. Their reflected-object
  path (`2344,2469–2545`) does **not** incrementally flush fields. The analysis
  therefore uses a whole section between array flushes, not a field-flush
  assumption. Static reflected field preflight is
  `round_up_to_nearest_16(maximum_key_size + write_padding_bytes)`; longest key
  `mandatory` has nine bytes and pinned padding is 256 (`core/opts.hpp:34`), so
  this is 272 bytes. String views make `fixed_padding<section>` zero.
* `core/write.hpp:19–39` returns a writer error without finalize; success calls
  finalize exactly once. The custom flush/finalize and sink write are explicitly
  nonthrowing and allocate nothing. The surrounding count boundary catches the
  only scratch allocation failure and unexpected C++ exceptions into enum status.

The scratch derivation assumes already-admitted input: version 1, at most 64
sections, ASCII declaration IDs of 1..64 bytes, each text <=1MiB, cumulative text
<=16MiB, and valid UTF-8/no NUL text. Existing declaration ID grammar is at
`src/stdlib/planar/schema.lua:25–32` and native `runtime.cpp:1803`.

Immediately after an array separator flush, pending bytes are <32,768. This holds
whether a flush happened (pending becomes zero) or the threshold was not reached.
The first section's root prefix is <64 bytes, also below that threshold. A full
section occupies at most `6*1MiB + 101`: its unescaped ID contributes <=64 and
fixed key/quote/punctuation/boolean framing contributes <=37 (false is longer).
The three section field positions, including before the large text, therefore
need at most the preceding residual plus the section maximum. The largest static
preflight adds 272 bytes; including section/array/root closing characters and
separator checks gives a conservative extra margin <374 bytes beyond `6*1MiB`.
The string preflight's extra ten is also within that margin. Four pinned writer
padding units, `4*256=1024`, dominate all these positions. Hence
`32768 + 6*1048576 + 4*256 = 6325248` bounds physical scratch. This is a derived
capacity, not an additional input quota. A smaller scratch safely returns Glaze
buffer-overflow; the test maps that to `status::writer`, distinct from encoded
output overflow. Production should treat failure of this proved internal bound
as an internal defect, not misreport it as the 16MiB output limit.

Work after overflow is finite: text scanning is bounded by admitted raw 16MiB
and section count 64. Even charging six bytes per raw text byte, <=102 framing/
separator bytes per section and <64 root bytes bounds writer output/work by
100,669,888 bytes. Pending bytes and logical offsets remain far below size_t and
streamsize limits. One scratch allocation is both the maximum live payload and
the complete cumulative dynamic request for the bounded adapter; there are no
refunds, resets, hidden output allocations or per-flush allocations. The fixed
views/buffer/sink are stack objects whose logical workspace must also be charged
under the eventual product budget. This is neither RSS nor allocator-overhead
accounting.

The isolated observer replaces global new/new[] only in this test executable.
It observes the entire count call, including constructor, writer, flush,
finalize, destruction and exception adaptation. Pinned vector<char> storage uses
ordinary new, not aligned allocation; the fixed nonallocating writer and enum
error path have no other allocator source. Fixture/reference AST/output work is
outside observation by design. There is no proposal to route product allocation
globally.

## Verification and raw evidence

Final frozen source identity is `frozen-support.json`; `final-corrected/counting.cpp`
is identical to its counting source. `final-corrected/commands.json` records exact
expanded compile/link/run argv, exits and durations, including the expected
SIGABRT subprocess. It imports only **private copies** of all referenced PCMs and
links only private copies of the unchanged codec/value archives. All 888 shared
.o/.pcm/.a files were enumerated and copied, with content identity recorded in
`copied-compiled.json`. Response files were recursively expanded before compile.
Outputs, dependencies and private modules remain within owned evidence. No shared
object or module is a compiler output.

Commands used:

```
python3 tests/native/v5-02-x05-c2-b0-support/protect.py freeze
python3 tests/native/v5-02-x05-c2-b0-support/run.py final-corrected
/opt/homebrew/opt/llvm/bin/clang-format --dry-run --Werror tests/native/v5-02-x05-c2-b0-support/counting.cpp
python3 tests/native/v5-02-x05-c2-b0-support/protect.py verify
```

After the final helper-selector correction, the frozen standalone binary,
pinned formatting and full protection comparison were rerun. Exact commands and
raw logs are `final-verification-commands.json`, `frozen-counting.log`, `format.log`
and `protected.log`.

**17 cases, 308 assertions and 61 allocation-failure positions pass.** These are
standalone spike counts, not Catch2/CTest inventory or a milestone gate. Cases
include empty sections/text, both booleans, ASCII/quotes/backslash/all non-NUL C0
controls, multibyte UTF-8, 64 sections, one 1MiB worst-escaped text, 16MiB raw text,
exact encoded 16,777,215 / 16,777,216 / 16,777,217 bytes, 64 worst-escaped sections,
16 maximum-text sections, threshold alignment, maximum IDs, and 64 maximum-ID
sections alternating tiny/worst-escaped text. Two integer-extreme cases are
serializer-only controls, not projection version admission. The alternating case
has 32 flushes and first overflow at flush six; later writes cannot mask refusal.
Real writer finalization overflow uses a tightened test counter limit, while
fixed-scratch refusal skips finalization. Ordinary success finalizes exactly once.

Each case counts **before** constructing its equivalent generic native graph.
An independent real Glaze full-output serialization supplies the actual byte
length, and unchanged `codec_encode` encodes the equivalent graph or refuses
above its unchanged 16MiB limit. Below/at limit, byte counts match both outputs;
Glaze parses both outputs into owning typed structures and verifies every field.
Codec key sorting changes order but not field content, escaping or total size.
The oversized generic graph is constructed afterward solely as an independent
reference test, not proposed production behavior.

Both bounded scratch constructor failure and every stock-buffer growth request
are injected separately. Every fault returns allocation status and no output
success. The stock allocation sequence and bounded single request are printed
for every fixture. This is controlled failure, not natural host exhaustion.

## Preserved attempts and protection

`attempt-1` preserves a compile failure: pinned Glaze generic values have no
operator==. It was replaced by independent typed Glaze reads and field comparison.
`attempt-2` preserves a fixture failure: resizing boundary text without a fill
character introduced NUL, changing escaped size; the fixture now fills ASCII.
`attempt-3`, `attempt-4` and `final` preserve successful earlier corpora.
`final-expanded` preserves a fixture failure after adding alternating text: its
512-byte scratch negative control accidentally selected the new tiny first
section. The corrected control selects the large second section. Historical
frozen manifests are retained. A first protection selector mistakenly included
two Debug metadata files in the 888 compiled-only comparison; this was a helper
membership bug, not changed shared files. The final selector separately verifies
the 888 compiled files and the 701 Debug object/module/metadata files.

Final protection compares exact file membership and SHA256: 2,286 source/vendor/
root/reference files, 20,807 historical test/evidence files, all 888 Debug compiled
objects/modules/archives, 701 Debug object/module/build-metadata files, and all
3,492 release files. Every comparison passes. AGENTS.md remains the CLAUDE.md
symlink. `git-before.txt` records the large pre-existing rewrite diff; no unrelated
changes were reverted. Only the two owned new directories are the candidate.

## Future production API and limits

A separately reviewed helper can take `span<const projection_section_view>`
plus the existing invocation accounting owner, and return
`expected<size_t, projection_count_error>`. The raw protected Lua scanner must
first validate/root the exact return, keep its strings stable and create at most
64 borrowed views. **That lifetime/longjmp/rooting integration is not delivered.**

The helper must reserve its fixed logical workspace and bind a request-local
allocator/hook before the scratch constructor's first allocation. A
`vector<char,budget_allocator<char>>` allocated once at the proved capacity can
charge the exact request synchronously before ordinary allocation, with no
refund/reset and no global routing. Allocation refusal becomes the existing
resource error outside protected Lua. A synchronous precharge hook for the exact
single vector payload is another possible reviewed implementation. The current
spike has no such product API. Its nonthrowing sink and traits must remain
nonthrowing; capacity overflow, encoded output overflow and allocation refusal
must remain distinguishable. Only successful size preflight permits generic AST
copy, and later existing AST/conversion allocations remain cumulatively charged.

No external projection invocation/admission, reserved-ID policy, rooted Lua
lifetime, whole-invocation orchestration, public profile CLI, history/acceptance,
Linux or release runtime claim is made. Fresh independent review is required
before any production helper is implemented.

All owned edits are released for coordinator/reviewer use. No further edits are
planned; this report does not approve its own candidate.
