# Canonical v1 native vectors

`canonical-v1.tsv` contains 41 literal rows: name, C(value) hexadecimal bytes,
domain, H(domain,value). No expected value is computed by production Planar
code. The first 12 byte literals are hand-transcribed from the typed contract in
`docs/vernacular/render-contract.md`; the Python oracle asserts its independent
encoding agrees with those literals. They cover booleans, signed integer bounds,
empty string/array/object, raw NUL and non-UTF-8, arrays, and unsigned UTF-8 key
sorting. Strings are not Unicode-normalized.

The next 25 rows use deterministic byte payloads `byte[i] = i mod 256` with
lengths named by `file_<length>`. Including the file-domain envelope, lengths
51/52/53, 163/164/165, 179/180/181 and 947/948/949 exercise XXH3 message-size
boundaries 127/128/129, 239/240/241, 255/256/257 and 1023/1024/1025.
Raw string length boundaries 255/256/257 independently check big-endian lengths.
The final four rows are selected base/observation/unit/role records matching the
reviewed recipes, using logical source file `a`, base bytes `old`, current bytes
`new`, normalized mode `0644`, base label `initial`, unit `library`, and role
`builder`. These are selected data, not a second implementation of Lua policy.

Reproduce or check offline, using Python 3 and a C compiler:

```sh
python3 tests/vectors/generate.py --check
# Deliberate regeneration for a reviewed contract change:
python3 tests/vectors/generate.py
```

The oracle uses Python `struct.pack('>q'/'Q')`, byte concatenation and unsigned
UTF-8 sorting. It builds the unmodified upstream C primitive into a temporary
shared library and calls it through ctypes. Numeric `high` then `low` formatting
provides expected digest representation independently of
`XXH128_canonicalFromHash`; production uses that API. The Python encoder and
oracle never load/link a Planar module. Expected C and H bytes are checked into
the repository, and native tests read them directly.

The upstream hash primitive is crosschecked against 20 independently published
seed-zero literals in `upstream-xxh3-128.tsv`, selected from
[xxHash v0.8.3 sanity_test_vectors.h](https://github.com/Cyan4973/xxHash/blob/v0.8.3/tests/sanity_test_vectors.h),
array `XSUM_XXH128_testdata` (not the subsequent secret-based array). Each row
contains input length and high64/low64 hexadecimal digest. The upstream
[fillTestBuffer](https://github.com/Cyan4973/xxHash/blob/v0.8.3/tests/sanity_test.c)
starts uint64 state at 2654435761, emits `state >> 56`, and multiplies by
11400714785074694797 modulo 2^64 for each byte. Empty input's published value
is `99aa06d3014798d86001c324468d497f`. Both the Python oracle and native tests
crosscheck these upstream literals; native tests also verify seed-zero API
equivalence and canonical byte conversion. The primitive is upstream xxHash,
not an independent reimplementation of XXH3. Independence comes from the
published known answers and independently implemented contract encoding.

Source provenance: archive
`https://codeload.github.com/Cyan4973/xxHash/tar.gz/refs/tags/v0.8.3`, SHA256
`aae608dfe8213dfd05d909a57718ef82f30722c392344583d3f39050c7f29a80`.
The archive was downloaded and verified during R06. The unmodified library files
and BSD-2-Clause license are committed under `vendor/xxhash/0.8.3/`; no network
is used for configuration, compilation, testing or vector replay. SHA256s:

- `xxhash.c`: `5c3591fe6e6c86a619eb26760e9520e37a6fd5152882ab5ad93f912e2a855966`
- `xxhash.h`: `17973c0dc49d9854ca26caa191f0e12f7a424b68858d9a78de3860d959d85e4b`
- `LICENSE`: `6ffedbc0f7878612d2b23589f1ff2ab15633e1df7963a5d9fc750ec5500c7e7a`

Upstream vector excerpts retain the same BSD-2-Clause licensing and attribution
as the vendored source. Fresh-process determinism is checked by repeated native
CTest executions against these literals. See R06 evidence for actual commands,
counts and host limitations. Canonical JSON transport remains R08 scope.
