# Stage 0 note (PKG-T)

The identity files in this directory (`identity-src-support.txt`, `identity-acceptance.txt`) were taken AFTER the
PKG-T edits (an ordering slip), so they show the post-edit `accept_test.py` and `x08_recipe.py`. The pre-edit state
is recoverable: `x08_recipe.py` was tracked and unmodified at HEAD (`git show HEAD:tests/acceptance/x08_recipe.py`
sha256 below); the pre-edit `accept_test.py` is the current file with the HeavyDriverTest class and its three import
lines removed (sha256 below, reconstructed in scratch). The Python before-list (`gate/test-names/before/py-worktree-c03.txt`,
134 ids) is the ast list with the HeavyDriverTest ids removed. `raw-hashes.txt` and `evidence-schema.sha256` are
unaffected by the slip (no raw tree or evidence schema was edited).

pre-edit accept_test.py sha256: c6ae7b28851063836cc57ae1a77640976caecfa8c5c741672b81e6a797a93d8e
pre-edit x08_recipe.py sha256 (HEAD blob): bb763c443c87f95a9a184eed480d742db1d9ec93ce5e22562f9ef027572b5804
