# R13-D01 documentation correction evidence

Shared checkout; coder owns only `docs/testing.md` and this directory. The only
reference edit changes the current registry sentence from 216/215 to 226/225.
The one Python acceptance case, adjacent 247 CLI invocations, and all historical
R09 counts remain byte-for-byte unchanged. No builds, tests, Planar tools,
installed binaries, commits, installation, publication or merge were invoked.

`testing.before.md`, `testing.after.md`, and `documentation.diff` freeze the exact
edit. `candidate.before.json` preserves the C2 13-file candidate map;
`candidate.after.json` records the updated map. Map keys are repository-relative.
Identity is SHA256 of compact sorted-key UTF-8 JSON of the files map using comma
and colon separators without terminal LF, matching C2 convention. Twelve other
C2 candidate files retain their approved hashes. No coder approval is claimed.

Read-only Python checks parsed both existing C2 review registry JSON files and
matched both existing gate logs: each has 226 passing registered tests, comprising
225 native commands and one Python acceptance command, with zero skips.
Sources: `../r13c2-review-evidence/{debug,release}-registry.json` and
`../r13c2-review-evidence/gate-{debug,release}.log`. These are frozen existing
results, not newly executed tests. Both prior C2 evidence manifests were verified;
`protected-prior-evidence.json` additionally freezes every file in both directories
(excluding OS `.DS_Store` metadata only), including manifests and snapshots.
`verification.json` records exact counts and hashes. `evidence.json` maps all
files in this directory, relative to this directory, excluding itself.
