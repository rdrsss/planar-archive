"""CLI11 grammar regressions with valid fixture controls and fixed usage diagnostics."""

def check_parser(args, run):
    """Exercise each binding, grammar boundary and semantic option relationship."""
    valid = args()
    baseline = run(valid)
    # The library may recognize a terminator, but this interface has no positional tail.
    run(valid + ["--"], 2, "usage")
    options = [("--workspace", "workspace/workspace.lua"), ("--registry", "registry/quota-tools.json"),
               ("--root", "source=current"), ("--job", "combined"), ("--role", "builder"),
               ("--base-root", "source=base"), ("--observations", "observations/empty.json"),
               ("--budget-bytes", "32768")]
    for name, value in options:
        original = valid + ([name, value] if name == "--budget-bytes" else [])
        run(original + [name, value], 2, "usage")
        index = original.index(name)
        for replacement in ["", "--unknown"]:
            changed = original.copy()
            changed[index + 1] = replacement
            run(changed, 2, "usage")
        run(original[:index] + original[index + 2:] + [name], 2, "usage")
        run(original[:index] + [name + "=" + value] + original[index + 2:], 2, "usage")
    for name, _ in options[:5]:
        index = valid.index(name)
        run(valid[:index] + valid[index + 2:], 2, "usage")
    for extra in [["--capture-outputs"], ["--capture-outputs", "true"], ["--help"], ["--unknown"],
                  ["-h"], ["-w", "x"], ["--", "extra"], ["extra"], ["render"]]:
        run(valid + extra, 2, "usage")
    for prefix in [[], ["unknown"], ["--help", "--help"], ["--help", "true"], ["--help", "--"],
                   ["--help", "render"], ["--", "render"], ["-h"], ["render", "--help"]]:
        run(prefix, 2, "usage")
    run(["--help"] + valid, 2, "usage")
    run(["unknown"] + valid[1:], 2, "usage")
    run(args(capture=False) + ["--capture-outputs", "true", "--base-root", "source=base"], 2, "usage")
    for value in ["0", "00", "01", "+1", "-1", "1e3", "1.0", "1048577", "99999999", " 1", "1 "]:
        run(valid + ["--budget-bytes", value], 2, "usage")
    for value in ["missing", "=current", "source="]:
        changed = valid.copy()
        changed[changed.index("--root") + 1] = value
        run(changed, 2, "usage")
    changed = valid.copy()
    changed[changed.index("--base-root") + 1] = "other=base"
    run(changed, 2, "usage")
    run(args(capture=False) + ["--capture-outputs"], 2, "usage")
    run(args(capture=False) + ["--base-root", "source=base"], 2, "usage")
    changed = args(capture=False)
    changed[changed.index("--root") + 1] = "other=current"
    run(changed, 3, "reference")
    # Complete option groups may be reordered. Values beginning with a single '-'
    # remain values and reach semantic validation rather than becoming options.
    groups = [["--workspace", "workspace/workspace.lua"], ["--registry", "registry/quota-tools.json"],
              ["--root", "source=current"], ["--job", "combined"], ["--role", "builder"],
              ["--capture-outputs"], ["--base-root", "source=base"],
              ["--observations", "observations/empty.json"]]
    assert run(["render"] + [v for group in reversed(groups) for v in group]) == baseline
    run(valid + ["--budget-bytes", "1048576"])
    changed = valid.copy()
    changed[changed.index("--job") + 1] = "-unknown"
    run(changed, 3, "schema")
    return baseline
