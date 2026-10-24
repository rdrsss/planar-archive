# X01 independent review evidence

Disposition is recorded in `docs/vernacular/rounds/V5-02-X01-review.md`.
`initial-audit.json` verifies the frozen seven candidate, 13 built and 46 author
evidence entries, their manifest hashes, all 14,642 baseline entries, bootstrap
test prefix and AGENTS.md symlink. Manifest bases are explicit there.

`commands.json` records the independent serialized debug commands with UTC times
and statuses. Spike/focused JUnit and LastTest were archived before inventory.
`verification-summary.json` checks 3/3 spike cases (350 assertions), 41/41 focused
cases with seven CLI suites and zero skips, exact 56 printed raw rows against
author evidence, inventory405 preserving402, and unchanged candidate/artifact
hashes after verification. Lint checks95; both builds do no work.

`addition-audit.json` uses the baseline's exclusion of all .git/build directories.
The retained first audit accidentally excluded only root build, reporting four
existing nested generated-build paths as new; the corrected audit excludes them
and finds no unexpected additions. This was an inventory-method correction,
not a source or test change. No evidence files were removed.

Reviewer evidence manifest uses repository-root paths, includes the review report
and every evidence file except itself, and is frozen last. Release was untouched;
debug is released idle. No product source edits or full-suite/release gate.
