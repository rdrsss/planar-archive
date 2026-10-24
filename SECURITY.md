# Security policy

Report security problems privately through this repository's
[GitHub private vulnerability reporting](https://github.com/rdrsss/planar/security/advisories/new).
Include the source revision, platform, build preset and reproducing inputs.
This is a personal project without a response-time commitment.

The current render-only replacement reads explicit declarations, trusted
registry and optional observations, and optionally captures source/base trees.
Reports about sandbox escapes, unbounded resource use, filesystem confinement,
identity errors or disclosure of host paths/data are in scope. Render executes
no checks or processes and writes no database or state. Its filesystem capture
requires cooperative stable inputs; it is not an atomic OS snapshot.

This development branch has no installer or released replacement package.
Historical Planar releases implement a different product; identify their exact
revision when reporting an issue. For vendored vulnerabilities, report upstream
and also notify this repository so its pin can be reviewed.
