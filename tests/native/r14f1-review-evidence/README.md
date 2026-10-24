# Independent R14-F1 review evidence

The review report is `docs/vernacular/rounds/R14-F1-review.md`. Product source was
read-only. Manifest verification and a separately implemented inventory/generator
audit passed. `export-suite.log/xml` records ten export cases with zero skips.
`probe.cpp` adds reviewer-only cases to the existing scratch native fixture and
imports the unchanged production export module. `build-probe.py` reuses pinned
compiler/module/link flags from the current debug graph without changing it.
Run the built `probe` with `'[reviewer],[reviewer-umask]'`; `probes-final.log`
records three cases/72 assertions. The separate early XML has two testcase nodes
(the Catch JUnit tests attribute counts assertions); the final console log is
the final three-case count. Compiled probe/object files are disposable and not
part of retained evidence. `no-work.log` records the final no-change build.

The umask0777 probe deliberately observes an honest incomplete cleanup/no target
and validates the generated-only token before restoring the scratch stage mode
for fixture disposal. It does not change production permission handling.
