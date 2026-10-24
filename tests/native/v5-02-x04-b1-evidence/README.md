# X04-B1 candidate evidence

Candidate awaiting independent review. Shared checkout at HEAD
3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a; no commit or installation.

The existing bounded Glaze decoder is extracted into the pure profile_codec
module. extraction-proof.json proves byte-identical adapters and decoder body
(after the exported symbol rename), with only value/std imports and the existing
capture call position retained. profile_error is re-exported through capture.

The distinct effective_provenance tree profile uses 32 MiB/8192 files and
1 + 8192 * 63 directories. File-count checks in build and reconstruction use
the selected profile. Source64 MiB/4096 files/8192 directories and original
provenance16 MiB/4096 files/1 + 4096 * 63 directories remain unchanged.
All lower record/path/depth/mode bounds and canonical/record codecs remain exact.
At most63 non-root directory ancestors per admitted file derives the effective
directory cap. Sharing reduces directory count; a separate over-cap directory
fixture within the other limits is unreachable. Existing tree.t.cpp:338 tests
source directories8192/8193, and :398 tests induced provenance directories with
expanded occurrences. Both execute in the focused run.

New memory-only decoder tests cover ordered results, schema/UTF-8/error fields,
arbitrarily large unsupported integer versions, path aliases and resource limits.
New callback tree tests cover files8192/8193, total32 MiB/+1, per-file16 MiB/+1,
depth64/65 and path1024/+1/segment128/+1, build/digest/read parity, and expanded
shared references. Existing source/provenance byte and file boundaries also run.
Invalid build inputs reach zero interner callbacks. Reconstruction assumes
codec-validated record payload/mode/segment invariants, as its API already states.

attempt01 freezes sources before any command and retains raw command argv/times/
exits, logs, JUnit and immediate CTest temporaries. Debug all+planar_tests builds;
497-name inventory preserves all489 approved A3 names and adds8. Focused regex
replacement.*(profile|capture|tree|provenance|acceptance) passes177/177, zero
failures/errors/skips (88.15 seconds). Canonical, record-v1 and request-v2 vectors,
four inherited runtime/CLI audits,107-source pinned formatting/Doxygen and final
no-work build pass. No failed product command or hidden repair occurred.

Only seven owned current-status Markdown files changed afterward. attempt02-final-
docs verifies unchanged product/tests against attempt01, preserves497 names,
rechecks all five generated storage/export families forward and reverse, and
passes serialized build,107-source lint/Doxygen and no-work. No repeat177 run
was necessary. No full497 suite, release or Linux execution is claimed. The
independently approved A3 full489 debug/RelWithDebInfo gates remain authoritative.
B2 catalog, B3 effective provenance, live registration/CLI/acceptance, shared
invocation budgets and the mandatory external-profile gate remain open.

Baseline/final filesystem manifests use root-only .git/build exclusions and
exclude this evidence directory, preserving nested historical paths and symlinks.
Owned originals/final copies and owned.diff bind the exact candidate. The final
artifact manifest covers this report and all evidence except itself. Build
ownership is released only after all reports/manifests are complete.
