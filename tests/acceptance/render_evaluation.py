#!/usr/bin/env python3
"""Reproduce R10 operator examples with an explicit replacement binary; never update fixtures.

Writes actual envelopes, Markdown, invocation records and mutation patches to a NEW
output directory. Imports only the side-effect-free independent vector encoder.
No checks are executed and no operator feedback or approval is synthesized.
"""
import argparse
import ctypes
import difflib
import hashlib
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


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def snapshot(root):
    return {str(p.relative_to(root)): (p.stat().st_mode & 0o777, sha(p))
            for p in sorted(root.rglob("*")) if p.is_file()}


def sections(value):
    text = value["packet"]["markdown"]
    return {m.group(1): m.group(0).rstrip("\n") for m in re.finditer(
        r"(?ms)^## ([a-z_]+)\n\n.*?(?=^## [a-z_]+\n\n|\Z)", text)}


def identities(value):
    m = value["manifest"]
    return {"work": m["work"]["digest"], "render": m["render"]["digest"],
            "role_output": m.get("role_output"), "observation": m["observation"]["digest"],
            "outputs": {o["work"]: o["output"] for o in m["outputs"]},
            "start_facets": m["work"]["value"]["start_facets"]}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("binary", type=Path, help="explicit built replacement binary")
    parser.add_argument("output", type=Path, help="new evidence directory outside input trees")
    args = parser.parse_args()
    binary, output = args.binary.resolve(strict=True), args.output.resolve()
    if not binary.is_file() or not os.access(binary, os.X_OK):
        parser.error("binary must be an executable file")
    if output == FIXTURE or FIXTURE in output.parents:
        parser.error("output must be outside the fixture")
    output.mkdir(parents=True, exist_ok=False)
    sys.dont_write_bytecode = True
    spec = importlib.util.spec_from_file_location("r10_oracle", REPO / "tests/vectors/generate.py")
    oracle = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(oracle)
    binary_before = sha(binary)
    commands, packets, comparisons = [], {}, []
    fixture_before = snapshot(FIXTURE)

    def save_json(path, value):
        path.write_text(json.dumps(value, sort_keys=True, indent=2, ensure_ascii=False) + "\n")

    with tempfile.TemporaryDirectory(prefix="planar-r10-evaluation-") as scratch:
        scratch = Path(scratch)
        home = scratch / "home"
        home.mkdir()
        (home / "planar.db").write_bytes(b"R10 sentinel: must remain unchanged")
        environment = {"PATH": "/usr/bin:/bin", "HOME": str(home),
                       "PLANAR_DB": str(home / "planar.db"), "PYTHONDONTWRITEBYTECODE": "1",
                       "TMPDIR": str(scratch)}
        home_before = snapshot(home)
        library = scratch / "xxhash.so"
        cc = shutil.which("cc")
        assert cc, "C compiler required for independent upstream hash oracle"
        compile_args = [cc, "-shared", "-fPIC", "-O2", str(REPO / "vendor/xxhash/0.8.3/xxhash.c"),
                        "-o", str(library)]
        result = subprocess.run(compile_args, capture_output=True, env=environment, timeout=60)
        assert result.returncode == 0, result.stderr.decode()
        (output / "oracle-compile.stderr").write_bytes(result.stderr)
        xx = ctypes.CDLL(str(library))
        xx.XXH3_128bits.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
        xx.XXH3_128bits.restype = oracle.Hash

        def digest(domain, value):
            data = oracle.encode(["planar", "canonical_v1", domain, value])
            h = xx.XXH3_128bits(data, len(data))
            return f"xxh3_128:{h.high:016x}{h.low:016x}"

        def verify(value):
            p, m = value["packet"], value["manifest"]
            w = m["work"]["value"]
            markdown = p["markdown"].encode()
            assert digest("work_v1", w) == m["work"]["digest"]
            assert digest("fragment_v1", markdown) == m["render"]["digest"]
            assert len(markdown) == m["render"]["bytes"]
            assert sum(s["bytes"] for s in m["sections"] if s["included"]) == len(markdown)
            files = {}
            for f in m["files"]:
                data = bytes.fromhex(f["bytes_hex"])
                assert len(data) == f["size"] and digest("file_bytes_v1", data) == f["digest"]
                files[(f["root"], f["path"])] = data
            base = digest("base_v1", {"root": w["root"], "label": "initial",
                "entries": m["base"]["entries"], "excluded": w["ownership"]["excluded"]})
            assert base == m["base"]["digest"]
            assert digest("observation_v1", {"root": w["root"], "base": base,
                "entries": m["observation"]["entries"], "excluded": w["ownership"]["excluded"]}) == m["observation"]["digest"]
            for root, tree in [("base:source", m["base"]["entries"]),
                               ("current:source", m["observation"]["entries"])]:
                for entry in tree:
                    data = files[(root, entry["path"])]
                    assert len(data) == entry["size"]
                    assert digest("file_bytes_v1", data) == entry["content"]
            for unit in m["outputs"]:
                paths = {"command": ["src/cli/", "tests/cli/"],
                         "library": ["src/core/", "tests/core/"]}[unit["work"]]
                entries = [e for e in m["observation"]["entries"]
                           if any(e["path"].startswith(path) for path in paths)]
                assert digest("unit_output_v1", {"work": unit["work"], "root": w["root"],
                    "base": base, "paths": paths, "entries": entries}) == unit["output"]
            if p["kind"] == "worker":
                units = [{"work": u["work"], "output": u["output"]}
                         for u in m["outputs"] if u["work"] in p["work"]]
                assert digest("role_output_v1", {"role": p["role"], "units": units}) == m["role_output"]
            for contract in w["contracts"]:
                assert digest("contract_v1", {"kind": "text", "normalizer": "exact_bytes_v1",
                    "expected": contract["declaration"]["expected"]}) == contract["semantic"]
            for facet in w["start_facets"]:
                dependency = next(d for d in w["dependencies"] if d["id"] == facet["dependency"])
                declaration = next(f for f in w["facets"] if f["id"] == dependency["ref"])
                paths = declaration["paths"]
                entries = [e for e in m["observation"]["entries"] if any(
                    e["path"].startswith(path) if path.endswith("/") else e["path"] == path for path in paths)]
                assert digest("facet_v1", {"kind": "files", "normalizer": "exact_bytes_v1",
                    "root": w["root"], "paths": paths, "entries": entries}) == facet["semantic"]
            if p["kind"] == "reviewer":
                assert {t["work"]: t["output"] for t in m["targets"]} == {
                    u["work"]: u["output"] for u in m["outputs"]}
            assert "Render-only preview. No run is active." in p["markdown"]

        def case(name, edits=()):
            root = scratch / name
            # Copy only invocation inputs, never authored expectations or native goldens.
            root.mkdir()
            for directory in ["workspace", "registry", "current", "base", "observations"]:
                shutil.copytree(FIXTURE / directory, root / directory)
            patches = []
            for relative, old, new in edits:
                path = root / relative
                before = path.read_text()
                assert before.count(old) == 1, (relative, old)
                after = before.replace(old, new)
                path.write_text(after)
                patches.extend(difflib.unified_diff(before.splitlines(True), after.splitlines(True),
                               fromfile="a/" + relative, tofile="b/" + relative))
            if edits:
                (output / (name + ".patch")).write_text("".join(patches))
            return root

        def run(name, root, job="split", role="cli_worker", observations="observations/empty.json"):
            argv = ["render", "--workspace", "workspace/workspace.lua", "--registry", "registry/quota-tools.json",
                    "--root", "source=current", "--capture-outputs", "--base-root", "source=base",
                    "--job", job, "--role", role, "--observations", observations]
            before = snapshot(root)
            result = subprocess.run([str(binary), *argv], cwd=root, env=environment,
                                    capture_output=True, timeout=30)
            commands.append({"name": name, "cwd_case": root.name, "argv": ["<explicit-binary>", *argv],
                             "returncode": result.returncode, "stderr_bytes": len(result.stderr)})
            assert result.returncode == 0 and not result.stderr, (name, result.stderr.decode())
            assert str(scratch).encode() not in result.stdout and str(REPO).encode() not in result.stdout
            assert snapshot(root) == before and snapshot(home) == home_before, "CLI altered captured inputs or HOME"
            value = json.loads(result.stdout)
            verify(value)
            (output / (name + ".json")).write_bytes(result.stdout)
            (output / (name + ".md")).write_text(value["packet"]["markdown"])
            (output / (name + ".stderr")).write_bytes(result.stderr)
            packets[name] = value
            return value

        def compare(label, before, after, changed, stable):
            a, b = identities(packets[before]), identities(packets[after])
            def get(value, key):
                for part in key.split("."):
                    value = value[part]
                return value
            for key in changed:
                assert get(a, key) != get(b, key), (label, "expected change", key)
            for key in stable:
                assert get(a, key) == get(b, key), (label, "expected stable", key)
            sa, sb = sections(packets[before]), sections(packets[after])
            changed_sections = [s for s in [*sa, *[s for s in sb if s not in sa]] if sa.get(s) != sb.get(s)]
            comparisons.append({"case": label, "before_file": before + ".json", "after_file": after + ".json",
                "before": a, "after": b, "expected_changed": changed, "expected_stable": stable,
                "changed_sections": changed_sections})
            diff = difflib.unified_diff(packets[before]["packet"]["markdown"].splitlines(True),
                packets[after]["packet"]["markdown"].splitlines(True), fromfile=before + ".md", tofile=after + ".md")
            (output / (label + ".markdown.diff")).write_text("".join(diff))

        root = case("baseline")
        run("combined-builder", root, "combined", "builder")
        targets = identities(packets["combined-builder"])["outputs"]
        resolved = (root / "observations/reviewer.symbolic.json").read_text().replace(
            "$O_COMMAND", targets["command"]).replace("$O_LIBRARY", targets["library"])
        (root / "reviewer-resolved.json").write_text(resolved)
        (output / "reviewer-resolved.json").write_text(resolved)
        run("combined-reviewer", root, "combined", "reviewer", "reviewer-resolved.json")
        run("split-library_worker", root, role="library_worker")
        run("split-cli_worker", root)
        run("split-reviewer", root, role="reviewer", observations="reviewer-resolved.json")
        run("split-cli_worker-recovery", root, observations="observations/recovery.json")
        for name in list(packets):
            assert (output / (name + ".json")).read_bytes() == (FIXTURE / "native" / (name + ".json")).read_bytes()
        run("split-cli_worker-repeat", root)
        assert packets["split-cli_worker-repeat"] == packets["split-cli_worker"]
        same = ["work", "outputs", "role_output", "observation"]
        compare("recovery", "split-cli_worker", "split-cli_worker-recovery", ["render"], same)
        edits = {
            "notes": [("workspace/workspace.lua", "The driver supplies stable sibling source snapshots.",
                       "The driver supplies stable sibling source snapshots. R10 optional reading note.")],
            "requirement": [("workspace/spec.lua", "write invalid arguments to stderr only.",
                             "write invalid arguments to stderr only. Preserve a trailing newline.")],
            "own-output": [("current/src/cli/main.py", 'import sys', '# R10 output-only comment.\nimport sys')],
            "upstream-complete": [("current/src/core/quota.py", 'from threading import Lock',
                                   '# R10 upstream captured-byte comment.\nfrom threading import Lock')],
        }
        start = [("workspace/spec.lua", 'phase = "complete", quality = "done"',
                  'phase = "start", quality = "available"'),
                 ("workspace/spec.lua", 'checks = L { "integration" }', 'checks = L { "contract" }')]
        edits["start-before"] = start
        edits["start-after"] = start + edits["upstream-complete"]
        for name, changes in edits.items():
            changed_root = case(name, changes)
            run(name + "-command", changed_root)
            run(name + "-library", changed_root, role="library_worker")
        compare("notes-command", "split-cli_worker", "notes-command", ["render"], same)
        compare("notes-library", "split-library_worker", "notes-library", ["render"], same)
        compare("requirement-command", "split-cli_worker", "requirement-command", ["work", "render"],
                ["outputs", "role_output", "observation"])
        compare("requirement-library", "split-library_worker", "requirement-library", [], same + ["render"])
        compare("own-command", "split-cli_worker", "own-output-command",
                ["render", "outputs.command", "role_output", "observation"], ["work", "outputs.library"])
        compare("own-library", "split-library_worker", "own-output-library", ["observation", "outputs.command"],
                ["work", "render", "role_output", "outputs.library"])
        compare("complete-command", "split-cli_worker", "upstream-complete-command",
                ["render", "outputs.library", "observation"], ["work", "role_output", "outputs.command", "start_facets"])
        compare("complete-library", "split-library_worker", "upstream-complete-library",
                ["render", "outputs.library", "role_output", "observation"], ["work", "outputs.command"])
        compare("start-command", "start-before-command", "start-after-command",
                ["work", "render", "start_facets", "outputs.library", "observation"], ["outputs.command", "role_output"])
        compare("start-library", "start-before-library", "start-after-library",
                ["render", "outputs.library", "role_output", "observation"], ["work", "outputs.command"])
        assert not identities(packets["upstream-complete-command"])["start_facets"]
        assert identities(packets["start-before-command"])["start_facets"]
        assert snapshot(FIXTURE) == fixture_before and snapshot(home) == home_before
        assert sha(binary) == binary_before, "candidate binary changed during evaluation"
        before_text = packets["split-library_worker"]["packet"]["markdown"]
        after_text = packets["own-output-library"]["packet"]["markdown"]
        assert before_text == after_text
        assert "Session note supplied for this preview; history unavailable" in packets[
            "split-cli_worker-recovery"]["packet"]["markdown"]
        save_json(output / "comparisons.json", comparisons)
        save_json(output / "commands.json", commands)
        save_json(output / "fixture-inputs.json", {k: v for k, v in fixture_before.items()
            if k.split("/")[0] in {"workspace", "registry", "current", "base", "observations"}})
        save_json(output / "provenance.json", {"binary": str(binary), "binary_sha256": sha(binary),
            "oracle_source_sha256": sha(REPO / "tests/vectors/generate.py"),
            "xxhash_source_sha256": sha(REPO / "vendor/xxhash/0.8.3/xxhash.c"),
            "helper_sha256": sha(Path(__file__)), "platform": sys.platform,
            "oracle_compile": ["<cc>", "-shared", "-fPIC", "-O2", "vendor/xxhash/0.8.3/xxhash.c", "-o", "<scratch>/xxhash.so"],
            "environment": {k: v.replace(str(scratch), "<scratch>") for k, v in environment.items()},
            "cli_invocations": len(commands), "goldens_compared": 6, "identity_comparisons": len(comparisons),
            "input_and_home_unchanged": True, "fresh_process_repeat_equal": True, "skips": 0})
    print(f'{len(commands)} real CLI invocations; 6 exact native goldens; {len(comparisons)} identity comparisons; '
          'independent hashes verified; inputs/HOME unchanged; 0 skips')


if __name__ == "__main__":
    main()
