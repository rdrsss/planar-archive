# R13-B2 independent review evidence

Fresh reviewer `/root/r13b2_reviewer` used the shared checkout read-only for
product files and wrote only the assigned report and this evidence directory.
Commands ran directly and serially with exclusive debug ownership. No Planar
operations, installed binaries, operator data, commit/install/publish or merge.

`commands.json` preserves exact commands, exit codes and non-default working
directories. `build.log`, `focused.log`, `focused.xml` and `ctest-LastTest.log`
record a no-work product/test build and 93 focused tests, zero failures/skips.
Both vector suites and active/helper lint pass. `no-work.log` records the final
zero-step build. `audit.py` independently verifies the live candidate, snapshots,
exact diff, all 42 coder evidence entries, prior evidence, 3746 protected inputs,
and fresh reversible generated C2/B1 bytes. It writes generation snapshots only
inside this reviewer-owned directory. `audit.log` records its successful run.

`adversarial.cpp` is a verification-only harness compiled using the tree test's
module map and linked to the candidate product libraries. Its global allocation
interceptor throws once at the first allocation inside `build_tree` or writer
`read_tree`, after earlier pending records exist. Each path catches `bad_alloc`,
proves the fault fired, refuses commit and verifies prior pending records absent
through a fresh reader. Two further scenarios check invalid-profile refusal and
rollback. Final result: one case, four scenarios, 46 assertions, zero skips.
No product fault switch or source change was used. Compile/link commands are in
`commands.json`; intermediate object/executable hashes are retained in
`ephemeral-artifacts.json`, and the rebuildable binary artifacts were removed.

The initial probe used `/tmp` without canonicalizing macOS's symlinked prefix;
the existing safe opener refused initialization before the probe ran (exit 42,
one failed assertion). `adversarial-initial.cpp` and `adversarial-initial.log`
preserve that verification setup error. The corrected probe canonicalizes its
own temporary root like the product tests. No product change resolved it.

`evidence.json` maps folder-relative paths to SHA256 and excludes only itself.
The review report is outside this manifest and has a separate delivery hash.
All verification is macOS Apple Silicon debug; integrated R13 debug/release and
Linux remain later gates. Debug is released with the final review delivery.
