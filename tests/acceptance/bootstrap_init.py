"""Explicit new-ledger CLI acceptance, isolated from all operator state."""
import json
import os
from pathlib import Path
import re
import sqlite3
import subprocess
import sys
import tempfile

binary = str(Path(sys.argv[1]).resolve())
count = 0
messages = {
    "usage": "Expected command options or --help; see --help",
    "schema": "Invalid ledger arguments",
    "path_denied": "Ledger path requires existing real parent directories and a valid target",
    "ledger_exists": "Ledger target already exists",
}
with tempfile.TemporaryDirectory(prefix="planar-init-private-") as temporary:
    root = Path(temporary).resolve()
    home = root / "home"
    home.mkdir()
    (home / ".planar").mkdir()
    sentinel = home / ".planar/planar.db"
    sentinel.write_bytes(b"private operator sentinel")
    environment = dict(os.environ, HOME=str(home), PLANAR_DB=str(sentinel))

    def run(args, code=None):
        global count
        count += 1
        result = subprocess.run([binary, *args], cwd=root, env=environment, capture_output=True, timeout=30)
        if code:
            assert result.returncode == (2 if code == "usage" else 3), (args, result)
            expected = {"version": 1, "kind": "error", "error": {
                "code": code, "message": messages[code], "field": ""}}
            assert result.stdout == b""
            assert result.stderr == (json.dumps(expected, sort_keys=True, separators=(",", ":")) + "\n").encode(), result
            assert b"private" not in result.stderr and str(root).encode() not in result.stderr
            return expected
        assert result.returncode == 0 and result.stderr == b"", (args, result)
        value = json.loads(result.stdout)
        assert result.stdout == (json.dumps(value, sort_keys=True, separators=(",", ":")) + "\n").encode()
        assert set(value) == {"version", "kind", "ledger", "workspace", "active_revision", "event"}
        assert value | {"ledger": "ID"} == {"version": 1, "kind": "init", "ledger": "ID", "workspace": "sample", "active_revision": "none", "event": "1"}
        assert re.fullmatch("[0-9a-f]{32}", value["ledger"])
        return value

    def args(target="private.db", workspace="sample"):
        return ["init", "--ledger", str(target), "--workspace-id", workspace]

    # Strict grammar must refuse before creating any target.
    valid = args()
    invalid = [[], ["init"], ["init", "--help"], ["--help", *valid], valid + ["--help"],
               valid + ["--"], valid + ["extra"], valid + ["--unknown"], valid + ["-h"],
               valid + ["init"], valid + ["render"], ["--", *valid], ["--help", "--help"]]
    for index in [1, 3]:
        name, value = valid[index:index + 2]
        invalid += [valid + [name, value], valid[:index] + valid[index + 2:],
                    valid[:index] + [name], valid[:index] + [name, ""] + valid[index + 2:],
                    valid[:index] + [name, "--private"] + valid[index + 2:],
                    valid[:index] + [name + "=" + value] + valid[index + 2:]]
    render = ["render", "--workspace", "private.lua", "--registry", "private.json", "--root", "source=private", "--job", "combined", "--role", "builder"]
    invalid += [valid + render, render + valid, valid + ["--workspace", "private.lua"], valid + ["--ledger", "private2.db"]]
    for invocation in invalid:
        run(invocation, "usage")
        assert not (root / "private.db").exists()
    for workspace in ["Private", "private-id", "1private", "a" * 65, "private/path", "-private", "privaté"]:
        run(args(workspace=workspace), "schema")
        assert not (root / "private.db").exists()
    for target in [".", "..", "private/", "/"]:
        run(args(target), "schema")
    for target in ["missing/private.db", "missing/../private.db"]:
        run(args(target), "path_denied")
        assert not (root / "private.db").exists()
    (root / "real").mkdir()
    (root / "alias").symlink_to(root / "real", target_is_directory=True)
    for target in ["alias/private.db", "alias/../private.db"]:
        run(args(target), "path_denied")
    for target, content in [("empty.db", b""), ("foreign.db", b"private unrelated bytes")]:
        path = root / target
        path.write_bytes(content)
        run(args(target), "ledger_exists")
        assert path.read_bytes() == content
    directory = root / "directory.db"
    directory.mkdir()
    (directory / "private").write_bytes(b"retained")
    run(args(directory), "ledger_exists")
    assert list(directory.iterdir()) == [directory / "private"]
    assert (directory / "private").read_bytes() == b"retained"
    for target, destination in [("link.db", root / "foreign.db"), ("dangling.db", root / "absent.db"), ("dirlink.db", directory)]:
        link = root / target
        link.symlink_to(destination)
        run(args(link), "ledger_exists")
        assert link.is_symlink() and link.readlink() == destination
    successes = [args(), args(root / "absolute.db"), args("real/../dot.db"),
                 ["init", "--workspace-id", "sample", "--ledger", "reordered.db"], args("-single.db")]
    identities = set()
    for invocation in successes:
        value = run(invocation)
        assert value["ledger"] not in identities
        identities.add(value["ledger"])
        path = root / invocation[invocation.index("--ledger") + 1]
        before = path.read_bytes()
        with sqlite3.connect(f"file:{path}?mode=ro", uri=True) as db:
            tables = {row[0] for row in db.execute("SELECT name FROM sqlite_schema WHERE type='table'")}
            assert {"ledger_meta", "events", "revisions", "records", "requests", "tree_edges", "current_state"} <= tables
            assert db.execute("SELECT count(*) FROM events").fetchone() == (1,)
            assert db.execute("SELECT active_revision_id,last_seq FROM current_state").fetchone() == (None, 1)
            assert db.execute("SELECT seq,type,version,revision_id,request_id,actor FROM events").fetchone() == (1, "ledger_initialized", 1, None, None, "planar:init")
            for table in ["revisions", "records", "requests"]:
                assert db.execute(f"SELECT count(*) FROM {table}").fetchone() == (0,)
            # EX-1 cutover: public init creates format3; initialization response/event shapes remain unchanged.
            assert db.execute("PRAGMA user_version").fetchone() == (3,)
            assert db.execute("SELECT format FROM ledger_meta").fetchone() == (3,)
            meta = db.execute("SELECT * FROM ledger_meta").fetchone()
            assert value["ledger"] in meta and "sample" in meta
        run(invocation, "ledger_exists")
        assert path.read_bytes() == before
    assert sentinel.read_bytes() == b"private operator sentinel"
    assert sorted(p.name for p in home.rglob("*")) == [".planar", "planar.db"]
    assert not any(p.name.startswith(".planar-init-") for p in root.rglob("*"))
print(f"Init CLI acceptance passed: {count} processes; zero skips")
