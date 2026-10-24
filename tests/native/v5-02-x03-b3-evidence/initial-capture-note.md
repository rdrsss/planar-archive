# Initial capture runner limitation

The contemporaneous `baseline-complete-filesystem.json` was written before the
first build or any status edit. Its walk excludes only root `.git/`, root `build/`
and the new B3 support/evidence directories, and records symlink targets separately.
The subsequent HEAD/branch/status/raw binary git-diff commands all exited zero.
The runner then attempted to decode the saved raw diff as UTF-8 for display and
raised `UnicodeDecodeError` at byte position 36433442. This note summarizes the
tool-returned traceback; it is not a contemporaneous raw process stderr file.
The 53 MiB `baseline-diff.log` preserves the original subprocess bytes unchanged.

Only the new B3 runner was corrected to decode display text with replacement.
The original baseline filesystem manifest was not regenerated. The separate
`baseline-build-identities-delayed.json` was captured after debug configure/build
had begun, so it does not claim to be a preconfigure binary/cache snapshot.
Per-configuration final build identity manifests bind the actual tested executables,
cache, compilation database and graph. No product or existing test was changed.
