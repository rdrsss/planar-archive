# X06-A documentation candidate evidence

Status: candidate, awaiting fresh independent review; no self-approval.
Starting HEAD: 3171c49e7b92d3abda01bfbb8bfb0c6b29d4c67a, intentional dirty rewrite.
Shared checkout; coder changes only assigned documentation and this new evidence.

The contract diff clarifies the existing 41-byte tagged shape-definition identity
and raw 32-hex effective-tree digest without changing recipes or versions. The new
X06 path bounds A1 to pure v2 codec/raw-tree verification, then separates readers,
same-VM preparation, profile transaction acceptance and receipt/budget composition.
Eight current-status openings point to the path and independently approved C5
876/configuration macOS gate; preserved later historical prose keeps its scope.

Verification commands:

```
python3 tests/native/v5-02-x06-a-evidence/verify.py baseline
python3 tests/native/v5-02-x06-a-evidence/verify.py final
python3 tests/native/v5-02-x06-a-evidence/identity_checks.py
```

Baseline was captured exactly once before any documentation edit. It records
93736 filesystem members, SHA256 for regular files, modes and device/inode,
symlink identities/targets and directory membership. The inventory covers the
checkout including all three build trees and material root dotfiles
.clang-format/.clang-tidy/.gitattributes/.gitignore. Only .git, this new evidence
subtree and the exact coordinator-owned X06-A round file are exempt. All prior
rounds/evidence, including C5, remain protected. Preedit copies are preserved.
An initial link-check failure from missing URI decoding is preserved in
initial-link-check.txt; the verifier now decodes the existing historical path.
Final check results and exact candidate diff are in checks.json/candidate.diff;
baseline.json and final-protection.json bind the protected tree.

No product builds, tests, probes, formatting gate or Doxygen run was executed;
this round changes documentation only. No build ownership was acquired; Debug,
RelWithDebInfo and install-release remain protected and available to their owner.
No installed binary, operator data, Planar workflow, commit, install, publication
or merge was used. X06 implementation, public cutover, mandatory external-profile
acceptance, execution/RA01 and Linux remain open. Historical approved product
results are cited, not rerun or expanded by this documentation check.

Results: 525 local Markdown links pass; all 93736 baseline members were
compared by full hash/identity inventories, with no unexpected changes. Exactly
ten owned documentation paths changed. Tagged-definition/raw-digest and symlink
checks pass. artifact-manifest.json binds the evidence and candidate document
hashes; no build ownership remains held.
