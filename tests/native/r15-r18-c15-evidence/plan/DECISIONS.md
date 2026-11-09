# C15 mapping decisions

- Map only the exact three whitespace-only Python lines and one prose EOF LF.
- Generate proposed bytes in evidence; never edit candidate source during mapping.
- Preserve line positions and compare AST attributes, semantic token positions,
  whole-module marshaling and every nested code object at identical compilation
  filename, flags and optimization settings.
- Retain C14 full-gate evidence as tested-original bytes plus only explicitly
  reviewed exact old/new Python pairs. Require focused registered workflows in
  both host configurations and fresh independent review before acceptance.
- Preserve existing publication originals/backups; declare and inverse the
  exact prose EOF normalization independently of path redaction.
