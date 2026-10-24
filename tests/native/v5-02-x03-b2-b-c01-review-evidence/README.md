# X03-B2-B-C01 independent review evidence

The independent review is in
`docs/vernacular/rounds/V5-02-X03-B2-B-C01-review.md`.
All paths in this evidence manifest resolve relative to this directory.
Commands ran in `<repo>` on the shared checkout;
source, source tests and author artifacts were read-only. Only this new directory
and the review report were written. Debug commands were serialized.

`run.py` records exact argv, cwd, Unix start/end timestamps and exit status, with
combined raw output in the corresponding log. The focused CTest JUnit and copied
LastTest preserve case outcomes and full fault diagnostics. `results.py` checks
209 passing cases with no errors/skips, parses raw fault ordinals separately for
both formats, checks prior inventory preservation, and verifies vector/lint/no-work
results. It also checks the author's frozen red/green outcomes and the preserved
red snapshot/manifest timestamps preceding red execution; this review did not
rebuild the unfixed product or mutate any product source.

`audit.py` independently verifies the exact supplied manifest identities, all
candidate snapshots and author artifacts, a root-only `.git/`/`build/` exclusion
filesystem walk, baseline preservation, complete available-diff reconstruction,
minimal product delta, frozen red support equality, and reversible generated
variants. The only additional exclusions are C01 author/review evidence and this
review report. All prior B2-B author/review artifacts remain covered by the frozen
source manifest; the 119 prior author artifacts also match their own manifest.

Two initial reviewer audit attempts failed due to audit assumptions: the artifact
manifest uses a list of path/hash rows rather than a map, and inherited variants
have legitimate substitution counts above one. `audit-initial.py`,
`audit-second.py` and both failure logs retain those attempts. The corrected
script honors the recorded count while still requiring all new C01 substitutions
to be unique. No product/test changes resulted. Every later audit passes.

The final artifact manifest freezes every other file under this directory and
excludes itself. The review report is outside this directory and records that
manifest's digest. No release/full integrated gate or Linux verification was run.
