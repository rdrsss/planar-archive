# N01 rename evidence

The candidate moves all 22 current native infrastructure files from `src/kernel/`
to `src/engine/`, renames the C++ modules/namespaces and trusted Lua binding,
and updates current build/test references and prose. Existing target names,
protocol versions, hash domains and fixture bytes remain unchanged.

`before.json` records exact prechange hashes and protected artifact hashes;
`before-files.json` retains the prechange owned-file contents. These intentionally
retain the historical name. `preexisting-status.txt` captures the shared checkout
before implementation. `rename.diff` compares against that actual starting
state, rather than against the legacy implementation in Git.

`candidate-files.json` freezes all owned current files plus the removed old
paths. SHA256: 161b5f6a02be6a1e26258dde2e412b084e9a66659a3ae236d9614c69630239ae.
`audit.json` records post-verification integrity and active build-registry checks.
The coordinator-owned live N01 round record is excluded from protected equality;
all other prior evidence, historical planning, fixtures and vectors stayed exact.

Executed serially, both with exit code zero:

- `make test-all PRESET=debug`
- `make test-all PRESET=release`

Each complete gate executed 113 registered tests with zero skips, checked pinned
formatting and Doxygen on 25 active source files, and ended with a no-work build.
Each acceptance log reports 150 real CLI processes and six exact native goldens.
The `*-gate.log` files contain complete gate output; `*-last-test.log` files
retain raw CTest case output. Registry snapshots document configured new paths.

No new behavior tests were needed for the terminology change. No installed
binary, legacy database, Planar tooling, installation or publication was used.
Host coverage remains macOS Apple Silicon; Linux was not executed.
