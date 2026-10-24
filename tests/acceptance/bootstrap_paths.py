"""Real CLI regressions for host path routing and equivalent root spellings."""
import json
from pathlib import Path
import shutil


def check_paths(fixture, scratch, args, run, verify):
    root = scratch / "path-fixture"
    external = scratch / "path-external"
    shutil.copytree(fixture, root)
    shutil.copytree(fixture, external)
    (external / "subdir").mkdir()
    (root / "hop").symlink_to(external / "subdir", target_is_directory=True)
    (root / "alias").symlink_to(external, target_is_directory=True)
    (root / "nested").symlink_to("hop", target_is_directory=True)
    (root / "root-link").symlink_to("current", target_is_directory=True)
    (root / "base-link").symlink_to("base", target_is_directory=True)
    (external / "current/src/cli/main.py").write_bytes(b"canonical current marker\n")
    (external / "base/base-marker").write_bytes(b"canonical base marker\n")
    spec = external / "workspace/spec.lua"
    spec.write_text(spec.read_text().replace("Provide an atomic", "Canonical workspace: provide an atomic"))
    registry = external / "registry/quota-tools.json"
    data = json.loads(registry.read_text())
    data["toolchains"][0]["command"] = ["canonical-registry-marker"]
    registry.write_text(json.dumps(data))
    observation = external / "observations/recovery.json"
    data = json.loads(observation.read_text())
    data["checkpoint"]["progress"] = "Canonical observations marker."
    observation.write_text(json.dumps(data))
    failures = []
    checks = 0

    def check(name, action):
        nonlocal checks
        checks += 1
        try:
            action()
            print(f"host-path PASS {name}", flush=True)
        except AssertionError as error:
            failures.append(name)
            print(f"host-path FAIL {name}: {error}", flush=True)

    def refused(arguments, status, code, message):
        # run already checks canonical bytes, newline, streams and redaction.
        # Equality here pins every envelope field and the exact safe diagnostic.
        result = run(arguments, status, code, cwd=root)
        error = {"code": code, "field": "", "message": message}
        if code == "io":
            error["details"] = {"operation": "capture"}
        assert result == {"version": 1, "kind": "error", "error": error}, result

    def options(option, value, observations="observations/empty.json"):
        result = args("split", "cli_worker", observations=observations)
        result[result.index(option) + 1] = value
        return result

    # Direct and lexical candidates contain deliberately distinct bytes. Compare
    # entire envelopes, and independently verify retained-byte/content digests.
    cases = [("--root", "source=", "current", "manifest", "observation"),
             ("--base-root", "source=", "base", "manifest", "base"),
             ("--workspace", "", "workspace/workspace.lua", "manifest", "work"),
             ("--registry", "", "registry/quota-tools.json", "manifest", "work"),
             ("--observations", "", "observations/recovery.json", "manifest", "render")]
    for option, prefix, path, _, identity in cases:
        obs = "observations/recovery.json" if option == "--observations" else "observations/empty.json"
        direct = verify(run(options(option, prefix + str(external / path), obs), cwd=root))
        lexical = verify(run(options(option, prefix + path, obs), cwd=root))
        assert direct["manifest"][identity] != lexical["manifest"][identity], option
        print(f"host-path ORACLE {option} canonical={direct['manifest'][identity].get('digest')} "
              f"lexical={lexical['manifest'][identity].get('digest')}", flush=True)

        def routed(spelling):
            result = verify(run(options(option, prefix + spelling, obs), cwd=root))
            assert result == direct, f"{option} selected lexical bytes instead of canonical bytes"

        for alias in ["hop/..", "nested/..", "alias"]:
            check(f"{option} {alias}/{path}", lambda alias=alias: routed(alias + "/" + path))

    baseline = verify(run(args("split", "cli_worker"), cwd=root))
    for option, prefix, path in [("--root", "source=", "current"), ("--base-root", "source=", "base")]:
        link = "root-link" if option == "--root" else "base-link"
        # The base fixture has no child directory, so make one for the spelling.
        (root / path / "spelling-child").mkdir()
        for suffix in ["", "/", "/.", "/spelling-child/..", "/spelling-child/../."]:
            check(f"{option} refuse {link}{suffix}",
                  lambda suffix=suffix: refused(options(option, prefix + link + suffix), 3, "path_denied", "Explicit root is a symbolic link target"))
        for spelling in [path + "/", path + "/.", path + "/spelling-child/..", "./" + path]:
            # Empty source directories affect topology only; all envelopes should
            # agree with the ordinary direct spelling in this same tree.
            direct = verify(run(options(option, prefix + path), cwd=root))
            check(f"{option} ordinary {spelling}",
                  lambda spelling=spelling: same(run(options(option, prefix + spelling), cwd=root), direct))
        (root / path / "spelling-child").rmdir()

    # Ordinary file parents retain ./ and ../ semantics; final symlinks stay
    # prohibited. Malformed file spellings must not become valid after cleanup.
    for option, path in [("--workspace", "workspace/workspace.lua"),
                         ("--registry", "registry/quota-tools.json"),
                         ("--observations", "observations/empty.json")]:
        parent, leaf = path.split("/")
        (root / parent / "file-link").symlink_to(leaf)
        for suffix in ["", "/", "/.", "/../" + leaf]:
            spelling = path + suffix
            if not suffix:
                spelling = parent + "/./../" + path
                check(f"{option} ordinary {spelling}",
                      lambda spelling=spelling: same(run(options(option, spelling), cwd=root), baseline))
            else:
                check(f"{option} invalid file {spelling}",
                      lambda spelling=spelling: refused(options(option, spelling), 4, "io", "Explicit input must be a regular file"))
        check(f"{option} final file symlink",
              lambda: refused(options(option, parent + "/file-link"), 3, "path_denied", "Explicit input file is a symbolic link"))
        check(f"{option} directory instead of file",
              lambda: refused(options(option, "./" + parent if option == "--workspace" else parent), 4, "io", "Explicit input must be a regular file"))
        check(f"{option} missing parent",
              lambda: refused(options(option, "missing/../" + path), 4, "io", "Explicit input must be a regular file"))
        (root / parent / "file-link").unlink()
    for option in ["--root", "--base-root"]:
        for spelling in ["registry/quota-tools.json", "registry/quota-tools.json/..", "missing/../current"]:
            check(f"{option} invalid directory {spelling}",
                  lambda spelling=spelling: refused(options(option, "source=" + spelling), 4, "io",
                                          "Explicit root must be a directory" if spelling == "registry/quota-tools.json"
                                          else "Cannot resolve explicit root directory" if spelling.endswith("/..")
                                          else "Cannot resolve explicit input path"))
    assert not failures, f"{len(failures)}/{checks} host path checks failed: {failures}"
    print(f"{checks} host path checks passed; zero skips", flush=True)


def same(actual, expected):
    assert actual == expected
