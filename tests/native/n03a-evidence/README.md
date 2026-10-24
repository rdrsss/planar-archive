# N03-A codec candidate evidence

This bounded candidate restores Glaze and adds an independent library adapter.
It does not change the active product JSON writer, Lua JSON parser, Lua
presentation serializer, fixtures, contracts, or binary canonical/hash encoding.
N03-B must integrate this adapter and remove those handwritten product paths.
No commit, installed binary, legacy runtime, database, queue or Planar tool used.

## API for N03-B

Import `planar.engine.json_codec`, link `planar_json_codec`.
`codec_decode(string_view, codec_limits)` returns
`expected<value_ptr, codec_error>`. Defaults and hard ceilings: raw input 1 MiB,
128 nested containers (including empty terminals), 100,000 visited values.
`codec_encode(value_ptr, codec_limits)` returns
`expected<string, codec_error>`. Defaults and hard ceilings: payload 16 MiB,
256 nested containers, 4,000,000 visited values (repeated references count).
Callers can tighten limits, not raise hard ceilings. LF is outside payload.

`codec_error.detail` is the existing native diagnostic shape:
`{code,message,limit,maximum}`. `codec_error.offset` is a zero-based byte offset;
N03-B must add one when adapting to the current Lua JSON schema details contract.
Decode syntax failures use `syntax`; duplicate keys, null, non-int32 numeric
forms, decoded NUL use `schema`; ceilings/allocation failures use
`resource_limit` with `input_bytes`, `json_depth`, `json_nodes` or
`output_bytes`. Invalid native output values use `internal`. Messages contain
no host path. Scalar policy errors identify scalar start; duplicate/NUL-key
errors identify enclosing object start. Syntax errors retain furthest library
failure before variant rewind; boundary refusals identify the failing value.
Encoding and allocation refusal offsets are zero. JSON input NUL policy is
stricter than output: encode accepts/escapes NUL as required for captured source.

Native input values must be ordinary boolean/int32/string/record/list values.
Empty object/array kinds remain distinct. Object keys sort by unsigned UTF-8
bytes. Glaze alone performs grammar parsing, numeric conversion, string
unescaping and output escaping. The adapter rejects numeric spellings containing
`.eE` only after Glaze parsed the token; no JSON number grammar is implemented.
`-0` is accepted and encodes as `0`. All C0 output uses Glaze's own escapes,
including short escapes and uppercase hexadecimal. No postprocessing rewrites.

## Native allocation boundary

All these allocations are native and are **not metered by the Lua allocator**.
N03-B must preserve that distinction when documenting runtime memory limits.
The ~96 MiB figure below bounds the serialization buffer, not total process RSS
or total adapter memory. The adapter also owns bounded structural conversions.

Decode checks raw bytes before parsing and uses per-call Glaze callbacks to
charge nodes and container depth before descending. Glaze arrays/objects append
one element before invoking its node reader; at most one extra default element
exists when a node ceiling refuses. Their vector capacity growth and strings
are bounded by 100,000 nodes and 1 MiB captured input; there is no unbounded full
parse before applying policy. Library native recursion also caps at 256; the
adapter's 128 ceiling applies earlier. Decoded duplicate object keys remain in
Glaze's vector-of-pairs until validated. The immutable conversion creates at
most 100,000 value allocations plus their maps/vectors and decoded strings;
both the parsed representation and converted tree coexist until return.

Encode first traverses without a converted tree or output buffer, refusing
cycles, tags, invalid text, integer range, nodes/depth, and a *minimum* encoded
byte charge. String/key byte lengths and punctuation are charged using checked
subtraction, so no unbounded multiplication or output expansion occurs first.
Then it converts at most 4,000,000 visited nodes into Glaze's recursive variant,
copying source strings/keys whose aggregate bytes cannot exceed the 16 MiB
minimum payload bound. Vector capacity growth is implementation-defined and
bounded by this node count; the source tree remains caller-owned.

Glaze's control-escaping writer requires `6*n` scratch capacity for each string,
even if no bytes require escaping. The adapter supplies a fixed `span<char>`
backed by `6 * minimum_payload_bytes + 1024` bytes, at most 100,664,320 bytes
(96 MiB + 1 KiB). Glaze cannot resize that span. Its capacity checks bound every
write; final actual payload size must still be <=16 MiB. Escaping expansion can
therefore be refused after bounded serialization, without returning partial
output. This is an explicit bounded native allocation, not a claim of a 16 MiB
working set. `bad_alloc` produces a typed refusal.

## Verification

Executed directly and serialized against `build/rewrite-debug`:

- `cmake --preset debug`: pass (`configure.log`).
- `cmake --build build/rewrite-debug --target all planar_tests`: pass
  (`build-final.log`; earlier compile/documentation iterations retained).
- `build/rewrite-debug/src/engine/planar_json_codec_tests '[codec]'`: pass,
  6 cases / 81 assertions, no skips (`tests-final.log`). Exact 1 MiB decode,
  100,000 decode nodes, 128 decode containers, 16 MiB output, 256 encode
  containers, 4,000,000 encoder visits and their next-excessive refusals covered.
- `python3 scripts/rewrite-check.py lint --build-dir build/rewrite-debug`: pass,
  pinned clang-format and Doxygen, 28 active sources (`lint-final.log`).
- `cmake --build build/rewrite-debug --target all planar_tests`: no work
  (`no-change.log`).
- `git diff --exit-code HEAD -- vendor/glaze`: pass (`vendor-exact.log` empty).
  663 restored files; exact HEAD vendor tree `a72092a5a3df75fb952bc4646ed5a345d4c54746`.
  Full blob/mode/path inventory: `restored-vendor-inventory.txt`.

First diagnostics regression failed because the library rewound a nested syntax
failure offset to zero; the preserved actual transcript is
`offset-regression.txt`. Per-call furthest failure retention fixes it, and the
same assertion now passes. Compile/lint failures are also retained as development
evidence. Existing product tests were built, not re-executed here; full debug /
release product gates and Linux coverage belong to N03-B integration.
