# X06-A4 architecture author evidence

Documentation-only candidate awaiting independent review. No product, test or
build configuration changes; no builds, test execution, Planar tools, installed
binary, commit, installation, publication, merge or operator-data action.
The API sketch was not compiled and is not implementation evidence.

Shared checkout dev/planar-v2-vernacular, HEAD
3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a; intentional pre-existing dirty rewrite.
Only ten owned documentation files changed. The coordinator separately owns the
exact A4 round record. No unrelated edits were reverted.

## Candidate and checks

- Candidate JSON SHA256: `eec7fe7ff64bcdc46f52993491e3ddfcaaa82938be52b71fecff108a3121deb5`.
- Exact preedit-relative diff SHA256: `fcc7366fa05d666c7874ca55dbd3ef852838f190509f18a86c16b4b609921fb7`.
- Original baseline SHA256: `c7828499e3e0a87627d67871d79357d04e9bc36604fa65e2f296c21c1175d294`.
- Final full protection SHA256: `35834cb5266e718098db5025d66629992a83b679b6a84392ae93e229ee88ce5c`.
- Original baseline entries: 94187; final entries: 94188.
- Local Markdown file links checked: 535; unresolved: zero.
- Nine pre-existing historical document bodies remain byte-exact; original
  preedit copies match the baseline. The new architecture document is additional.
- All non-owned file/link entries preserve content, mode, identity, timestamps
  and size. Directory metadata-only changes reflect authorized membership/write
  activity; directory identity/mode and every non-owned member are preserved.
- Every build tree, root dotfile, symlink and all prior evidence is protected;
  [protection-result.json](protection-result.json) lists build member/byte counts.
  AGENTS.md remains a symlink to CLAUDE.md; compile_commands.json preserves its
  original symlink. No build tree was granted a mutable exemption.

Protection traverses with lstat, hashes every regular file, does not follow
symlinks, and records directory membership plus link targets. The only exact
exclusions are root .git, this A4 support/evidence pair and
`docs/vernacular/rounds/V5-02-X06-A4.md`; no prior-round subtree is excluded.
One baseline and one final full-content pass were used; later report bookkeeping
uses the frozen manifests and rechecks only candidate/preedit/evidence files.

Executed commands: `python3 tests/native/v5-02-x06-a4-support/protect.py baseline`,
`python3 tests/native/v5-02-x06-a4-support/audit.py`,
`python3 tests/native/v5-02-x06-a4-support/protect.py final`, and
`python3 tests/native/v5-02-x06-a4-support/finalize.py`.
The audit constructs [candidate.diff](candidate.diff) from saved preedit bytes
and current candidates, hashes [candidate.json](candidate.json), checks
URI-decoded local links and preserved historical bodies/symlinks. Finalization
compares both full manifests, validates all candidate hashes and writes a
SHA256/size artifact manifest covering evidence and support. That manifest omits
only itself to avoid self-reference. The coordinator can record its final hash.

## Architectural result and limits

The proposed runtime owner privately captures/registers once, survives final
parsing, routes and validates all jobs within one private active invocation frame,
and constructs immutable prepared values with original native/profile receipts.
A higher shared completion module consumes the opaque runtime prepared type.
Downward storage/intent/source capture extraction removes the actual dependency
cycle and preserves native transaction boundaries. The document specifies
staged edges, constructor/adoption paths, receipt and counter lifetimes, native
replay/CAS/commit ordering and adversarial future checks.

Latest reader approval remains A3, current inventory 918, latest complete approved
macOS gate 876/configuration. Fresh profile acceptance remains refused and public
init creates format1. No new gate, shared budget, RSS, X07/X08, execution/RA01 or
Linux acceptance is claimed. Independent review is required before dependent code.
