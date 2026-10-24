"""Real replacement CLI acceptance: sealed inputs, computed goldens and independent hashes."""
import copy
import ctypes
import importlib.util
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

REPO = Path(__file__).resolve().parents[2]
FIXTURE = REPO / "fixtures/render/quota"
binary = str(Path(sys.argv[1]).resolve())
write_goldens = sys.argv[2:] == ["--write-goldens"]
assert not sys.argv[2:] or write_goldens
sys.dont_write_bytecode = True
from bootstrap_paths import check_paths
from bootstrap_parser import check_parser
spec = importlib.util.spec_from_file_location("oracle", REPO / "tests/vectors/generate.py")
oracle = importlib.util.module_from_spec(spec)
spec.loader.exec_module(oracle)
scenarios = 0


def canonical(value):
    def encode(v):
        if isinstance(v, str):
            # Independent standard-library JSON string oracle, with Glaze's C0 hex case.
            encoded = json.dumps(v, ensure_ascii=False)
            return re.sub(r"\\u00[0-9a-f]{2}", lambda m: m[0].upper().replace("U", "u"), encoded)
        if isinstance(v, bool):
            return "true" if v else "false"
        if isinstance(v, int):
            return str(v)
        if isinstance(v, list):
            return "[" + ",".join(map(encode, v)) + "]"
        assert isinstance(v, dict)
        return "{" + ",".join(encode(k) + ":" + encode(v[k]) for k in sorted(v, key=lambda k: k.encode())) + "}"
    return encode(value).encode()


with tempfile.TemporaryDirectory(prefix="planar-r08b-cli-") as scratch:
    scratch = Path(scratch).resolve()
    root = scratch / "first"
    shutil.copytree(FIXTURE, root)
    home = scratch / "hostile-home"
    home.mkdir()
    (home / ".planar").mkdir()
    (home / ".planar/planar.db").write_bytes(b"operator sentinel: never open as SQLite")
    (home / ".planar/config.toml").write_bytes(b"invalid configuration")
    environment = dict(os.environ, HOME=str(home), PLANAR_DB=str(home / ".planar/planar.db"),
                       PLANAR_CONFIG_PATH=str(home / ".planar/config.toml"))
    library = scratch / "xxhash.so"
    subprocess.run(["cc", "-shared", "-fPIC", "-O2", str(REPO / "vendor/xxhash/0.8.3/xxhash.c"),
                    "-o", str(library)], check=True)
    xx = ctypes.CDLL(str(library))
    xx.XXH3_128bits.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
    xx.XXH3_128bits.restype = oracle.Hash

    def digest(domain, value):
        data = oracle.encode(["planar", "canonical_v1", domain, value])
        h = xx.XXH3_128bits(data, len(data))
        return f"xxh3_128:{h.high:016x}{h.low:016x}"

    def args(job="combined", role="builder", capture=True, observations="observations/empty.json"):
        result = ["render", "--workspace", "workspace/workspace.lua", "--registry", "registry/quota-tools.json",
                  "--root", "source=current", "--job", job, "--role", role]
        if capture:
            result += ["--capture-outputs", "--base-root", "source=base"]
        if observations:
            result += ["--observations", observations]
        return result

    def run(arguments=None, status=0, code=None, cwd=None):
        global scenarios
        scenarios += 1
        result = subprocess.run([binary, *(args() if arguments is None else arguments)],
                                cwd=cwd or root, env=environment, capture_output=True, timeout=30)
        assert result.returncode == status, (arguments, status, result.returncode, result.stderr[:1000])
        if status == 0 and arguments == ["--help"]:
            assert b"Usage: planar render" in result.stdout and not result.stderr
            return result
        payload = result.stdout if status == 0 else result.stderr
        value = json.loads(payload)
        assert payload == canonical(value) + b"\n", payload[:200]
        assert str(scratch).encode() not in payload and str(REPO).encode() not in payload
        if status == 0:
            assert not result.stderr
            assert value["version"] == 1 and value["kind"] == "render"
        else:
            assert not result.stdout and value["kind"] == "error" and value["version"] == 1
            assert value["error"]["code"] == code, value
            error = value["error"]
            assert isinstance(error["field"], str) and isinstance(error["message"], str)
            assert set(error) <= {"code", "message", "field", "file", "line", "details"}
            if "file" in error:
                assert isinstance(error["file"], str) and not error["file"].startswith("/")
            if "details" in error:
                assert isinstance(error["details"], dict)
            if code == "usage":
                assert error["field"] == ""
            if code == "io":
                assert error["field"] == "" and error["details"] == {"operation": "capture"}
            if code == "budget_exceeded":
                assert error["field"] == "render.budget_bytes"
                assert error["details"]["mandatory_bytes"] > error["details"]["budget_bytes"]
                assert sum(s["bytes"] for s in error["details"]["sections"]) == error["details"]["mandatory_bytes"]
        return value

    def verify(value):
        packet, m = value["packet"], value["manifest"]
        markdown = packet["markdown"].encode()
        assert m["render"]["bytes"] == len(markdown)
        assert sum(s["bytes"] for s in m["sections"] if s["included"]) == len(markdown)
        sections = re.split(rb"(?m)^## ", markdown)[1:]
        included = [s for s in m["sections"] if s["included"]]
        assert len(sections) == len(included)
        for index, (section, meta) in enumerate(zip(sections, included)):
            assert section.split(b"\n", 1)[0].decode() == meta["id"]
            assert len(section) + 3 + (index > 0) - (index + 1 < len(sections)) == meta["bytes"]
        assert digest("fragment_v1", markdown) == m["render"]["digest"]
        assert digest("work_v1", m["work"]["value"]) == m["work"]["digest"]
        W = m["work"]["value"]
        files = {}
        for file in m["files"]:
            data = bytes.fromhex(file["bytes_hex"])
            assert len(data) == file["size"]
            assert digest("file_bytes_v1", data) == file["digest"]
            files[(file["root"], file["path"])] = data
        if m["base"]["status"] == "captured":
            base = digest("base_v1", {"root": W["root"], "label": "initial", "entries": m["base"]["entries"],
                                      "excluded": W["ownership"]["excluded"]})
            assert base == m["base"]["digest"]
            assert digest("observation_v1", {"root": W["root"], "base": base, "entries": m["observation"]["entries"],
                                             "excluded": W["ownership"]["excluded"]}) == m["observation"]["digest"]
            boundaries = {"command": ["src/cli/", "tests/cli/"], "library": ["src/core/", "tests/core/"]}
            for output in m["outputs"]:
                paths = boundaries[output["work"]]
                entries = [f for f in m["observation"]["entries"] if any(f["path"].startswith(p) for p in paths)]
                assert digest("unit_output_v1", {"work": output["work"], "root": W["root"], "base": base,
                                                 "paths": paths, "entries": entries}) == output["output"]
            if packet["kind"] == "worker":
                units = [{"work": o["work"], "output": o["output"]} for o in m["outputs"] if o["work"] in packet["work"]]
                assert digest("role_output_v1", {"role": packet["role"], "units": units}) == m["role_output"]
            else:
                assert "role_output" not in m
            for entry in m["base"]["entries"]:
                assert digest("file_bytes_v1", files[("base:source", entry["path"])]) == entry["content"]
            for entry in m["observation"]["entries"]:
                assert digest("file_bytes_v1", files[("current:source", entry["path"])]) == entry["content"]
        for contract in W["contracts"]:
            declaration = contract["declaration"]
            assert digest("contract_v1", {"kind": "text", "normalizer": "exact_bytes_v1", "expected": declaration["expected"]}) == contract["semantic"]
        assert "Render-only preview. No run is active." in packet["markdown"]
        return value

    def snapshot(path):
        return sorted((str(p.relative_to(path)), "directory" if p.is_dir() else p.read_bytes()) for p in path.rglob("*"))

    before = snapshot(root), snapshot(home)
    run(["--help"])
    check_parser(args, run)
    for arguments in [[], ["task", "list"], ["render"], ["render", "--help"], args() + ["--since", "x"],
                      args() + ["--workspace", "again"], args() + ["extra"], args() + ["--capture-outputs"],
                      args(capture=False) + ["--base-root", "source=base"],
                      args(capture=False) + ["--capture-outputs"], args() + ["--budget-bytes=10"]]:
        run(arguments, 2, "usage")
    for budget in ["0", "01", "-1", "+1", "1.5", "1048577", "9999999999999999999"]:
        run(args() + ["--budget-bytes", budget], 2, "usage")
    run(args() + ["--budget-bytes", "1"], 3, "budget_exceeded")
    baseline = verify(run())
    assert verify(run()) == baseline, "fresh process determinism"
    assert before == (snapshot(root), snapshot(home)), "invocations created state or altered inputs"
    second = scratch / "another-host-location"
    shutil.copytree(root, second)
    assert run(cwd=second) == baseline, "host paths entered identities"
    outputs = {o["work"]: o["output"] for o in baseline["manifest"]["outputs"]}
    symbolic = (root / "observations/reviewer.symbolic.json").read_text()
    resolved = symbolic.replace("$O_COMMAND", outputs["command"]).replace("$O_LIBRARY", outputs["library"])
    (root / "reviewer-resolved.json").write_text(resolved)
    native = FIXTURE / "native"
    if write_goldens:
        native.mkdir(exist_ok=True)
        (native / "reviewer-resolved.json").write_text(resolved)
    else:
        assert (native / "reviewer-resolved.json").read_text() == resolved
    packets = {}
    for name, job, role, obs in [
        ("combined-builder", "combined", "builder", "observations/empty.json"),
        ("combined-reviewer", "combined", "reviewer", "reviewer-resolved.json"),
        ("split-library_worker", "split", "library_worker", "observations/empty.json"),
        ("split-cli_worker", "split", "cli_worker", "observations/empty.json"),
        ("split-reviewer", "split", "reviewer", "reviewer-resolved.json"),
        ("split-cli_worker-recovery", "split", "cli_worker", "observations/recovery.json")]:
        value = verify(run(args(job, role, observations=obs)))
        packets[name] = value
        if write_goldens:
            (native / (name + ".json")).write_bytes(canonical(value) + b"\n")
            (native / (name + ".md")).write_text(value["packet"]["markdown"])
        else:
            assert (native / (name + ".json")).read_bytes() == canonical(value) + b"\n", name
            assert (native / (name + ".md")).read_text() == value["packet"]["markdown"]
        original = (FIXTURE / "expected" / (name + ".md")).read_text()
        W = value["manifest"]["work"]["value"]
        prose = [W["goal"]["bytes"], *[u["summary"]["bytes"] for u in W["units"]]]
        prose += [r["text"]["bytes"] for k in ["constraints", "prescriptions"] for r in W[k]]
        prose += [c["text"]["bytes"] for milestone in W["milestones"] for c in milestone["acceptance"]]
        prose += [c["declaration"][k]["bytes"] for c in W["contracts"] for k in ["expected", "review_note"]]
        for text in prose:
            assert text in value["packet"]["markdown"], (name, text)
            assert text in original, ("R03 exact source prose comparison", name, text)
        for boundary in W["ownership"]["work"]:
            for path in boundary["paths"]:
                assert path in original and path in value["packet"]["markdown"]
        selected_paths = [path for boundary in W["ownership"]["work"] for path in boundary["paths"]]
        selected_paths += [path for facet in W["facets"] for path in facet["paths"]]
        for file in value["manifest"]["files"]:
            if file["root"].startswith(("current:", "base:")) and any(
                    file["path"].startswith(path) if path.endswith("/") else file["path"] == path for path in selected_paths):
                text = bytes.fromhex(file["bytes_hex"]).decode()
                assert text in original and text in value["packet"]["markdown"], (name, file["root"], file["path"])
        markdown = value["packet"]["markdown"]
        for check in W["checks"]:
            assert f'Check {check["id"]}; toolchain: {check["toolchain"]}.' in markdown
            assert f'Timeout: {check["timeout_ms"]} ms; output cap: {check["max_output_bytes"]} bytes; selection: {check["selection"]}.' in markdown
            for key in ["argv", "inputs"]:
                assert canonical(check[key]).decode() in markdown
            assert check["coverage"] in markdown
        for toolchain in W["toolchains"]:
            assert markdown.count(f'Toolchain {toolchain["id"]}; trusted driver configuration.') == 1
            for key in ["command", "environment"]:
                assert canonical(toolchain[key]).decode() in markdown
            assert toolchain["description"] in markdown
        for key in ["base", "observation"]:
            assert value["manifest"][key]["digest"] not in markdown
        assert "projection_v2" in value["manifest"]["versions"]
        if role == "reviewer":
            assert "Review target boundary." in value["packet"]["markdown"]
            assert "Exclusive worker grant." not in value["packet"]["markdown"]
    recovery, command = packets["split-cli_worker-recovery"], packets["split-cli_worker"]
    assert recovery["manifest"]["work"] == command["manifest"]["work"]
    assert recovery["manifest"]["role_output"] == command["manifest"]["role_output"]
    assert recovery["manifest"]["render"]["digest"] != command["manifest"]["render"]["digest"]
    assert "Session note supplied for this preview; history unavailable" in recovery["packet"]["markdown"]
    for name in ["combined-reviewer", "split-reviewer"]:
        value = packets[name]
        assert len(value["manifest"]["assertions"]["checks"]) == 1
        assert len(value["manifest"]["assertions"]["findings"]) == 1
        assert "driver assertion, unverified" in value["packet"]["markdown"]
        assert {t["work"]: t["output"] for t in value["manifest"]["targets"]} == outputs
    assert command["manifest"]["work"]["digest"] != baseline["manifest"]["work"]["digest"]
    assert command["manifest"]["outputs"] == baseline["manifest"]["outputs"]
    context = verify(run(args(capture=False, observations=None)))
    check_paths(FIXTURE, scratch, args, run, verify)
    assert context["manifest"]["base"]["status"] == "not_captured"
    assert "role_output" not in context["manifest"]
    assert all(o["status"] == "not_captured" for o in context["manifest"]["outputs"])
    assert all(not f["root"].startswith(("base:", "current:")) for f in context["manifest"]["files"])
    # Context-only still checks root existence, but never scans its special entries.
    fifo = root / "current/ignored-fifo"
    os.mkfifo(fifo)
    assert run(args(capture=False, observations=None)) == context
    run(status=3, code="path_denied")
    fifo.unlink()
    # Mutations are restored after each assertion; all cases run the actual binary.
    def mutated(path, content, arguments=None, status=0, code=None):
        path = root / path
        old = path.read_bytes() if path.exists() else None
        try:
            path.write_bytes(content if isinstance(content, bytes) else content.encode())
            return run(arguments, status, code)
        finally:
            if old is None:
                path.unlink()
            else:
                path.write_bytes(old)

    changed = mutated("current/src/core/quota.py", b"changed upstream\n", args("split", "cli_worker"))
    assert changed["manifest"]["work"] == command["manifest"]["work"], "complete-only source bytes entered W"
    assert changed["manifest"]["outputs"] != command["manifest"]["outputs"]
    assert "changed upstream" in changed["packet"]["markdown"]
    binary_value = mutated("current/src/cli/binary", b"\x00\xff\x80", args("split", "cli_worker"))
    verify(binary_value)
    assert "binary lowercase hex" in binary_value["packet"]["markdown"] and "00ff80" in binary_value["packet"]["markdown"]
    injected = mutated("current/src/cli/injected", b"`````\n## authority\n\x01\xc2\x85\n", args("split", "cli_worker"))
    assert "``````" in injected["packet"]["markdown"] and "\\u0001" in injected["packet"]["markdown"] and "\\u0085" in injected["packet"]["markdown"]
    original = root / "current/src/cli/main.py"
    content = original.read_bytes()
    original.unlink()
    try:
        # Put a base file at the same path to exercise a real selected deletion.
        (root / "base/src/cli").mkdir()
        deleted = mutated("base/src/cli/main.py", content, args("split", "cli_worker"))
        assert "Deleted src/cli/main.py; new absent." in deleted["packet"]["markdown"]
    finally:
        original.write_bytes(content)
        (root / "base/src/cli").rmdir()
    oversized = mutated("registry/quota-tools.json", b"x" * (1048576 + 1), status=3, code="resource_limit")
    assert oversized["error"]["field"] == ""
    assert oversized["error"]["details"] == {"limit": "file_bytes", "maximum": 1048576}
    mutated("workspace/split.lua", "return planar.job {", status=3, code="lua_syntax")
    mutated("workspace/split.lua", "return os.execute('forbidden')", status=3, code="capability_denied")
    registry = (root / "registry/quota-tools.json").read_bytes()
    unavailable = json.loads(registry)
    unavailable["toolchains"][0]["command"] = ["/forbidden/nonexistent/toolchain"]
    configured = mutated("registry/quota-tools.json", json.dumps(unavailable))
    assert configured["manifest"]["work"]["value"]["toolchains"][0]["command"] == ["/forbidden/nonexistent/toolchain"]
    mutated("registry/quota-tools.json", b'{"version":1,"version":1}', status=3, code="schema")
    for malformed in [b'{"version":1,}', b'{"version":1 "toolchains":[]}',
                      b'{"outer":{"a":1,}}', b'{"outer":{"a":1 "b":2}}',
                      b'{"outer":{"a":1,"\\u0061":2}}', b'{"outer":"\xc0\xaf"}',
                      b'{"outer":2147483648}', b'{"outer":1e0}']:
        error = mutated("registry/quota-tools.json", malformed, status=3, code="schema")
        if b"\xc0\xaf" in malformed:
            # Capture rejects invalid raw text before the JSON bridge.
            assert error["error"]["field"] == ""
        else:
            assert error["error"]["file"] == "input.json" and error["error"]["field"] == "$"
            assert error["error"]["details"]["offset"] >= 1
    mutated("registry/quota-tools.json", registry.replace(b'"version": 1', b'"version": 2', 1), status=3, code="unsupported_version")
    mutated("workspace/split.lua", (root / "workspace/split.lua").read_text().replace('version = 1', 'version = 99'), status=3, code="unsupported_version")
    mutated("workspace/split.lua", (root / "workspace/split.lua").read_text().replace('work = L { "library" }', 'work = L { "missing_unit" }', 1), status=3, code="reference")
    run(args(observations="observations/reviewer.symbolic.json"), 3, "schema")
    run(args(capture=False, observations="reviewer-resolved.json"), 3, "observation_target_mismatch")
    stale = json.loads(resolved)
    stale["checks"][0]["target"]["output"] = "xxh3_128:" + "0" * 32
    mutated("bad.json", json.dumps(stale), args(observations="bad.json"), 3, "observation_target_mismatch")
    wrong = json.loads(resolved)
    wrong["checks"][0]["vector"] = wrong["checks"][0]["vector"][:1]
    mutated("bad.json", json.dumps(wrong), args("split", "library_worker", observations="bad.json"), 3, "observation_target_mismatch")
    check = json.loads(resolved)
    check["checks"][0]["check"] = "nonexistent"
    mutated("bad.json", json.dumps(check), args("split", "library_worker", observations="bad.json"), 3, "reference")
    for invalid in ['{"version":1,"checks":[],"findings":[],"x":true}', '{"version":1,"checks":null}', '{bad']:
        mutated("bad.json", invalid, args(observations="bad.json"), 3, "schema")
    checkpoint = json.loads((root / "observations/recovery.json").read_text())
    checkpoint["checkpoint"]["files"] = ["src/core/quota.py"]
    mutated("bad.json", json.dumps(checkpoint), args("split", "cli_worker", observations="bad.json"), 3, "ownership")
    checkpoint["checkpoint"]["files"] = ["src/cli/main.py"]
    checkpoint["checkpoint"]["progress"] = "x" * 2049
    mutated("bad.json", json.dumps(checkpoint), args("split", "cli_worker", observations="bad.json"), 3, "schema")
    budget = run(args() + ["--budget-bytes", str(baseline["manifest"]["render"]["bytes"])])
    assert budget["packet"] == baseline["packet"]
    trimmed = verify(run(args() + ["--budget-bytes", str(baseline["manifest"]["render"]["bytes"] - 1)]))
    assert trimmed["manifest"]["omitted"] and "omissions" in trimmed["packet"]["markdown"]
    run(args() + ["--budget-bytes", "1048576"])
    badroot = args(capture=False)
    badroot[badroot.index("source=current")] = "other=current"
    run(badroot, 3, "reference")
    missingroot = args(capture=False)
    missingroot[missingroot.index("source=current")] = "source=missing"
    run(missingroot, 4, "io")
    # Current/base roots are checked even before any traversal.
    overlap = args()
    overlap[overlap.index("source=base")] = "source=current"
    run(overlap, 3, "path_denied")
    symlink = root / "root-link"
    symlink.symlink_to(root / "current", target_is_directory=True)
    linked = args(capture=False)
    linked[linked.index("source=current")] = "source=root-link"
    run(linked, 3, "path_denied")
    symlink.unlink()
    symlink = root / "current/src/cli/link"
    symlink.symlink_to(root / "current/src/core/quota.py")
    run(status=3, code="path_denied")
    symlink.unlink()
    hardlink = root / "current/src/cli/hard"
    os.link(root / "current/src/core/quota.py", hardlink)
    run(status=3, code="path_denied")
    hardlink.unlink()
    # Shared segment aliases across roots work on case-sensitive and insensitive hosts.
    (root / "current/ALIAS").mkdir()
    (root / "base/alias").mkdir()
    (root / "current/ALIAS/a").write_bytes(b"alias")
    (root / "base/alias/b").write_bytes(b"alias")
    run(status=3, code="ownership")
    shutil.rmtree(root / "current/ALIAS")
    shutil.rmtree(root / "base/alias")
    empty = scratch / "empty"
    shutil.copytree(root, empty)
    shutil.rmtree(empty / "current")
    shutil.rmtree(empty / "base")
    (empty / "current").mkdir()
    (empty / "base").mkdir()
    vacant = run(cwd=empty)
    verify(vacant)
    assert all(not o["present"] and o["status"] == "captured" for o in vacant["manifest"]["outputs"])
    assert "No selected current or base files." in vacant["packet"]["markdown"]
    # Unselected source and excluded files: full checkout identity vs stable selected W.
    unrelated = mutated("current/unowned", b"retained unowned\n")
    assert unrelated["manifest"]["work"] == baseline["manifest"]["work"]
    assert unrelated["manifest"]["observation"] != baseline["manifest"]["observation"]
    (root / "current/generated").mkdir()
    excluded = mutated("current/generated/file", b"excluded\n")
    assert excluded == baseline
    (root / "current/generated").rmdir()
    # Force backpressure: a large real retained file makes the envelope exceed pipe capacity.
    (root / "current/unowned-large").write_bytes(b"a" * (1024 * 1024))
    read_fd, write_fd = os.pipe()
    os.set_blocking(write_fd, False)
    proc = subprocess.Popen([binary, *args()], cwd=root, env=environment, stdout=write_fd, stderr=subprocess.PIPE)
    os.close(write_fd)
    error = proc.stderr.read()
    assert proc.wait(timeout=30) == 4 and json.loads(error)["error"]["code"] == "io"
    partial = b""
    while True:
        part = os.read(read_fd, 65536)
        if not part:
            break
        partial += part
    os.close(read_fd)
    assert partial and len(partial) < 2 * 1024 * 1024
    (root / "current/unowned-large").unlink()
    scenarios += 1
    # No successful status when stdout is closed, including a broken pipe.
    proc = subprocess.Popen([binary, *args()], cwd=root, env=environment, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    proc.stdout.close()
    error = proc.stderr.read()
    assert proc.wait(timeout=30) == 4 and json.loads(error)["error"]["code"] == "io"
    assert error == canonical(json.loads(error)) + b"\n"
    scenarios += 1
    proc = subprocess.run([binary, "--help"], cwd=root, env=environment, capture_output=True,
                          preexec_fn=lambda: os.close(1), timeout=30)
    assert proc.returncode == 4 and not proc.stdout and json.loads(proc.stderr)["error"]["details"]["operation"] == "stdout"
    scenarios += 1
    proc = subprocess.run([binary, "unknown"], cwd=root, env=environment, capture_output=True,
                          preexec_fn=lambda: os.close(2), timeout=30)
    assert proc.returncode == 4 and not proc.stdout
    scenarios += 1
    assert snapshot(home) == before[1], "ambient state changed"
    (root / "reviewer-resolved.json").unlink()
    assert snapshot(root) == before[0], "command created state beyond harness changes"
    print(f"{scenarios} real CLI processes passed; six native goldens; independent canonical/hash/prose/section checks; zero skips")
