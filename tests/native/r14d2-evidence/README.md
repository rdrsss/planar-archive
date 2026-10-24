# R14-D2 selected-closure ancestor receipts

This is coder evidence, not independent approval. Shared checkout HEAD is in
`starting-head.txt`; the pre-existing rewrite is preserved. `baseline.json`
freezes 11,875 non-generated files before edits. `owned.json` names ten authorized
source/reference/instruction files. Coordinator-owned `docs/vernacular/` planning
is excluded from protection, except the authorized narrow implementation status
edit is included in the candidate. `AGENTS.md -> CLAUDE.md` remains intact.
`before/`, `candidate/`, `candidate.diff` and `candidate.json` identify exact edits.

The native read now stages intermediate logical-root/path directory identities
for successful retained file reads, compares shared receipts during discovery,
and rechecks original receipts while rereading at both seals and every acceptance
verification. Receipt maps are private; selected reads add no public topology.
There is no recursive unrelated enumeration or held descriptor per ancestor.
Unique receipts use an independent instance of existing 100,000-entry/64 MiB
accounting. One read is additionally bounded by the 1024-byte portable path limit;
repeated retained reads do not traverse or duplicate receipts. Allocation failures
retain the existing uncaught allocation-exception boundary; no new callback or
bypass was added. Verification does not mutate receipts or case-alias maps,
including initial sealing. Whole-tree membership checks remain independent.

`red/` freezes the unmodified native implementation/interface and added regression.
`red-test.log` records the required failure: after sealing a nested file without
calling tree(), rename its parent, create a new parent and hardlink the exact
original file inode into it. Existing verify incorrectly succeeded. One matching
test failed (CTest exit 8). `red-build.log` first exposed a stale host tool cache;
`red-fresh-build.log` records the successful fresh rebuild before this red test.
No red commit was authorized or created.

The original debug cache referenced removed Homebrew LLVM23.1.3_2 scanner paths.
Per the existing toolchain reference it was preserved as
`build/rewrite-debug-pre-r14d2-stale`; `cmake --preset debug` configured a fresh
standard directory with current LLVM23.1.4. Toolchain policy/config sources were
unchanged; this is not pin relaxation. `toolchain.json` records exact tools.
Release was untouched; prior G release evidence used LLVM23.1.3 and is not D2
release evidence.

Six new tests cover nested ancestor replacement during discovery/initial sealing/
repeat verification, unchanged repeated verification and moved capability,
private topology, shared repeated reads, hardlink/case refusal, selected file
identity/bytes/mode, failed read staging, and ancestor removal/link replacement.
Unrelated siblings include edits, additions, rename/removal, oversized sparse
files, chmod-000 files, symlinks and FIFOs. Ancestor chmod and nonexecutable file
permission changes remain legal; executable normalization changes refuse.
Existing root and whole-tree cases remain intact. Permission evidence is this
host's chmod behavior, not a claim about execution as another user.

`audit.py` preserves all 331 G names and compares all five configured test variants
with newly regenerated exact/reversible outputs. It protects 11,614 files,
including previous evidence, product/build/Lua/vendor/CLI/test helpers. The native
target command log establishes no Lua/ledger linkage. Both vector check logs and
helper syntax lint are retained. Actual command/results are recorded below after
verification completion.

No load service/CLI, schema, serialization or public source format changed. No
legacy tools, queues, claims, installed binaries, operator state, commits,
installation, publishing or merging were used. This evidence is macOS ARM64
debug; Linux, integrated load release and hardware power-loss remain unverified.

Executed results (all source-built, explicit scratch test roots):

- Fresh configure and `cmake --build build/rewrite-debug --target all planar_tests`:
  passed after stale-cache recovery; final build logs retained.
- `ctest --test-dir build/rewrite-debug -R 'replacement.capture'
  --output-on-failure --output-junit <absolute capture.xml>`: 31/31 passed,
  zero skips (25 existing + six new), 39.19 seconds.
- `python3 scripts/rewrite-check.py gate --build-dir build/rewrite-debug`:
  exit 0, all 337 passed, zero skips, 139.22-second CTest, formatting/Doxygen
  over 86 active C++ files and zero-work rebuild. Full log/LastTest retained.
  The helper deletes its temporary JUnit; focused JUnit reports are retained.
- `ctest --test-dir build/rewrite-debug -V -R
  '^replacement\.(cli|init_cli|history_cli|export_cli|acceptance |atomic )'
  --output-junit <absolute dependent.xml>`: 28/28 passed, zero skips;
  four CLI suites plus 12 prepared/request and 12 atomic cases, 21.78 seconds.
  Actual CLI process counts remain 247 render, 60 init, 115 history, 55 export;
  render additionally reports 60 host-path checks and six native goldens.
- Both vector generators with `--check`: 12 hand byte vectors, 20 upstream
  hashes, 41 canonical/domain vectors, ten independent record vectors passed.
- `audit.py`: 11,614 protected files unchanged, original 331 names preserved,
  337 current names, five exact/reversible configured variants verified.
- Helper syntax lint uses `compile()` without generated bytecode writes.
- Final direct `all planar_tests` rebuild: `ninja: no work to do.`

Execution caveat: the initial focused capture run was still finishing when the
full gate's no-work build/first CLI test began. There was brief test-only overlap,
no concurrent compilation or source edit. Both passed; the full gate ran its
own complete serial test sequence, and the dependent rerun began after gate exit.
This is recorded rather than claiming perfect command serialization.

The frozen candidate manifest SHA256 is
`3da96db6f13b830b48d05bb024aa810e403f3cd5a0caa8f79b7358a2e74d7936`.
Debug build-directory ownership is released for fresh independent review. Release
remains untouched. No coder approval is claimed.

`execution-timestamps.json` preserves file birth/mtime evidence: focused capture
started at 08:58:48 EDT and ended 08:59:27; full gate began 08:59:22, so
its start overlapped the preceding focused run by approximately five seconds.
The full gate ended 09:01:43; dependent completion was 09:02:10.
The final dependent tool session returned exit 0 before the final build/audit.
The full gate was not serialized relative to the initial focused run.
