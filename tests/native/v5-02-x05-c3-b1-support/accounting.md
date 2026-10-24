# C3-B1 storage and traversal

The existing metadata-only 64MiB cumulative metric and100000 receipt ceiling
remain. Each non-dot component, including each physical parent step, passes the
existing prospective640-byte/name room check, charges128+name bytes before its
temporary string/open, and a new retained receipt charges512+name bytes before
allocation. Root and ordinary downward active-chain entries charge sizeof(size_t)
before allocation. A parent step retains its upward name edge but pops the active
chain without allocating a new chain node. Receipt deduplication remains scoped to
same parent receipt, name, directory identity and mode. Charges never refund.

All originally visited downward edges and every physical upward edge remain in
the global receipt set. Upward lookup uses fstatat/openat on the retained child
with no-follow directory flags; its metadata must equal the active ancestor (or
root itself). Existing verification rechecks each retained name against its
retained parent and descriptor metadata. Final containment uses only the popped
active chain. Directory timestamp/sibling membership exclusion is unchanged.
