# Toolchain reference and history

The active renderer uses C++26 named modules and `import std;`. Configure with
CMake 4.3/4.4 and Ninja. The reviewed LLVM floor is 23 with modules-enabled
libc++; historical macOS probes pinned LLVM 23.1.0, CMake 4.4.2 and Ninja 1.13.2.
Actual integrated gate tool versions are recorded in R09 evidence.

`cmake/llvm-toolchain.cmake` validates clang/clang++, libc++ headers and
`libc++.modules.json`. `-DPLANAR_LLVM_PREFIX=/path/to/llvm` (or that environment
variable) wins; otherwise discovery checks Homebrew, apt.llvm.org prefixes and
PATH compiler candidates. Missing/invalid explicit prefixes fail. The libc++
manifest may be under `lib/c++`, `lib`, or `lib/<subdir>`. macOS Apple Silicon
uses Homebrew's `/opt/homebrew/opt/llvm`; `/usr/local/opt/llvm` and Linux
`/usr/lib/llvm-23` are supported discovery layouts, not replacement test claims.

Debug/release presets use `build/rewrite-debug` and `build/rewrite-release`.
They retain shared libc++ linking and a toolchain rpath. No dist preset,
installer or distribution gate is available. The toolchain's optional
`PLANAR_PORTABLE` archive-link mechanism is retained from earlier toolchain
work; it is not a supported replacement distribution profile.

## Derived import-std / embed flag set

CMake selects the experimental import-std UUID before enabling C++:
`451f2fe2-a8a2-47c3-bc32-94786d8fc91b` for 4.3.x and
`f35a9ac6-8463-4d38-8eec-5d6008153e7d` for 4.4.x. Unknown later minors fail
rather than guessing. First-party targets set `CXX_MODULE_STD=ON` and C++26.
The discovered compiler prefix supplies `-stdlib=libc++ -nostdinc++ -isystem
<prefix>/include/c++/v1`; link flags use `-stdlib=libc++ -L<libdir>
-Wl,-rpath,<libdir>`. The manifest locates `libdir`. On macOS this prevents
silently mixing SDK libc++ headers with the pinned runtime.

Tool paths are cached. If Homebrew removes a keg, create a fresh build directory
and reconfigure; reconfiguring a stale cache can retain nonexistent absolute
scanner/archive paths. No source build copies binaries into root `bin/` or
creates the root `compile_commands.json` convenience link automatically.

`scripts/rewrite-check.py` derives `clang-format` beside the cached
`CMAKE_CXX_COMPILER`; it never silently substitutes a PATH formatter.
Doxygen is external (historically 1.18.0). `scripts/cpp-lint-doxygen.sh` retries
up to three signal crashes only when output contains no diagnostic. A
`file:line: diagnostic` fails immediately, even followed by a crash. The active
gate generates its Doxyfile in temporary storage; no legacy root Doxyfile is used.

## Historical findings retained for future toolchain changes

The following are earlier toolchain experiments, not integrated renderer or
Linux evidence. The former Linux suite's 4,063 passing cases and portable TLS/
bundle checks belonged to the removed product. Its CI/Docker/release scripts
were retired in R09; they establish no replacement coverage.

- LLVM 22.1.8 was the interim pin before LLVM 23.1.0; macOS probes reproduced
  on 23.1.0 on 2026-09-04. The floor remains 23.
- Pack indexing parses in `-std=c++2c`; `std::expected` works through `import std`.
- `#embed` emitted `-Wc23-extensions` under the historical pin, requiring target
  specific `-Wno-c23-extensions` with `-Werror`. The renderer embeds Lua using
  generated modules instead of the removed migration/template embed targets.
- Doxygen 1.18.0 sometimes died with SIGBUS without diagnostics on an unchanged
  tree. Genuine warnings must never be retried into a false pass.
- That Doxygen also misparsed identifiers containing `override` in template
  arguments of trailing return types. Renaming the identifier fixed it without
  weakening documentation warnings. The original occurrence was in the removed
  legacy workspace engine; this is historical troubleshooting guidance.

## Validation evidence

All commands below ran in the foreground on this machine (macOS, Apple
Silicon, Homebrew) on 2026-08-21 against the then-pinned LLVM 22.1.8, and
were re-run and reproduced identically on 2026-09-04 against the now-pinned
LLVM 23.1.0. Probe sources are committed at
`scripts/toolchain-probes/probe_cxx2c.cpp` and
`scripts/toolchain-probes/probe_embed.cpp` (plus its embedded fixture
`probe_embed_data.txt`); the `import std;` probe is a throwaway scratch
CMake project (not committed — its shape is fully specified by the flag set
above and centurion's `CMakeLists.txt`/`CMakePresets.json` pattern it
mirrors) built under `cmake -S . -B build -G Ninja` with the exact
`CMAKE_CXX_FLAGS`/`CMAKE_EXE_LINKER_FLAGS`/`CMAKE_CXX_STDLIB_MODULES_JSON`
values listed above.

| Probe | Command | Exit | Evidence |
|---|---|---|---|
| `-std=c++2c` + pack indexing | `clang++ -std=c++2c -stdlib=libc++ -Wall -Wextra scripts/toolchain-probes/probe_cxx2c.cpp -o /tmp/probe_cxx2c && /tmp/probe_cxx2c` | 0 | `pack indexing ok: 7`, no diagnostics |
| `#embed` | `clang++ -std=c++2c -stdlib=libc++ -Wall -Wextra scripts/toolchain-probes/probe_embed.cpp -o /tmp/probe_embed && /tmp/probe_embed` | 0 | `embedded 25 bytes: planar-cpp26-embed-probe`; one `-Wc23-extensions` warning (see above) |
| `#embed` under `-Werror` | `clang++ -std=c++2c -stdlib=libc++ -Wall -Wextra -Werror scripts/toolchain-probes/probe_embed.cpp -o /tmp/probe_embed_werror` | 1 | `error: #embed is a Clang extension [-Werror,-Wc23-extensions]` — confirms the suppression is load-bearing |
| `#embed` under `-Werror -Wno-c23-extensions` | same, plus `-Wno-c23-extensions` | 0 | clean build, correct output — confirms the fix |
| `import std;` configure | `cmake -S . -B build -G Ninja -DCMAKE_C_COMPILER=... -DCMAKE_CXX_COMPILER=.../clang++ -DCMAKE_CXX_FLAGS="-stdlib=libc++ -nostdinc++ -isystem .../include/c++/v1" -DCMAKE_EXE_LINKER_FLAGS="-stdlib=libc++ -L.../lib/c++ -Wl,-rpath,.../lib/c++" -DCMAKE_CXX_STDLIB_MODULES_JSON=.../lib/c++/libc++.modules.json` | 0 | Only the expected CMake "experimental `import std;`" author warning |
| `import std;` build + run | `cmake --build build && ./build/probe_import_std` | 0 | `sorted: 1 2 3` / `expected ok` — `std::vector`, `std::ranges::sort`, `std::println`, and `std::expected` all resolve and link through libc++'s std module |

No C++26 feature this task needed to verify failed outright; the only
caveat is the `#embed` extension-warning interaction with warnings-as-errors,
documented above.
