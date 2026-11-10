#!/usr/bin/env python3
"""Compare the R19 engine split to the retained, actually configured baseline.

No build or configure is performed. Relocations are fixed by the reviewed owner
table and the baseline generated-file inventory, never learned from differences.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys


DIRECTORIES = (
    "acceptance", "budget", "canonical", "capture", "execution", "export",
    "hash", "history", "json_codec", "ledger", "lua", "process", "runtime",
    "runtime/closure_guard", "value",
)
GENERATED_DIRECTORIES = {
    "r12c2-generated": "ledger", "r13b1-generated": "ledger",
    "r14c3b2-generated": "ledger", "v5-02-x03-b2-b-c01-generated": "ledger",
    "r14f1-generated": "export", "v5-02-b1-generated": "runtime",
    "r14-d6-generated": "runtime", "r14-d6-c01-generated": "runtime",
    "v5-02-x05-a0-generated": "runtime",
}
RUNTIME_GENERATED = re.compile(
    r"(?:runtime_(?:test|budget_test|tight_test)|shapes_test)\.cppm?$|"
    r"planar_(?:runtime(?:_test)?|shapes_test|load_c01)_source\.cppm$|"
    r"planar_(?:load_source_[0-5]|command_source_(?:[0-9]|1[0-9]|20)|"
    r"x05_source_[01])\.cppm$"
)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def read_json(path):
    return json.loads(Path(path).read_text())


def load_targets(reply):
    indices = sorted(reply.glob("index-*.json"))
    if not indices:
        raise RuntimeError(f"No File API index in {reply}")
    index = read_json(indices[-1])
    model_entry = next(x for x in index["objects"] if x["kind"] == "codemodel")
    model = read_json(reply / model_entry["jsonFile"])
    if len(model["configurations"]) != 1:
        raise RuntimeError("Expected one Ninja configuration")
    targets = {}
    for entry in model["configurations"][0]["targets"]:
        if entry["name"] in targets:
            raise RuntimeError("Duplicate target " + entry["name"])
        targets[entry["name"]] = read_json(reply / entry["jsonFile"])
    return targets


def target_owner(name, target):
    if target.get("paths", {}).get("source") != "src/engine":
        return None
    owners = set()
    for source in target.get("sources", []):
        path = Path(source["path"])
        if str(path).startswith("src/engine/"):
            owners.add(str(path.parent.relative_to("src/engine")))
    if len(owners) == 1:
        owner = owners.pop()
        if owner not in DIRECTORIES:
            raise RuntimeError(f"Unreviewed source owner {name}: {owner}")
        return owner
    exact = {"planar_admission_syscalls": "capture", "planar_lua_trusted_libs": "lua",
             "planar_history_fixture": "history", "planar_format2_fixture": "ledger"}
    if name in exact:
        return exact[name]
    prefixes = (("planar_atomic_", "ledger"), ("planar_record_", "ledger"),
                ("planar_metadata_", "ledger"), ("planar_ledger_", "ledger"),
                ("planar_export_", "export"), ("planar_runtime_", "runtime"),
                ("planar_command_", "runtime"), ("planar_load_", "runtime"),
                ("planar_shapes_", "runtime"), ("planar_x05_", "runtime"))
    for prefix, owner in prefixes:
        if name.startswith(prefix):
            return owner
    raise RuntimeError(f"No reviewed owner for engine target {name}")


def generated_owner(path):
    tail = Path(path).parts[2:]  # inventory paths begin src/engine/
    if len(tail) > 1 and tail[0] in GENERATED_DIRECTORIES:
        return GENERATED_DIRECTORIES[tail[0]]
    if len(tail) == 1 and RUNTIME_GENERATED.fullmatch(tail[0]):
        return "runtime"
    raise RuntimeError("No reviewed generated owner for " + path)


def make_relocations(repo, build, baseline_targets, generated):
    replacements = {}
    paths = {}
    owners = {}
    # Each output path is fixed from its baseline target's reviewed owner.
    for name, target in baseline_targets.items():
        owner = target_owner(name, target)
        if owner is None:
            continue
        owners[name] = owner
        for artifact in target.get("artifacts", []):
            old = artifact["path"]
            if not old.startswith("src/engine/"):
                raise RuntimeError("Unexpected baseline engine artifact " + old)
            new = "src/engine/" + owner + "/" + old.removeprefix("src/engine/")
            paths[old] = new
    generated_paths = {}
    for old in generated:
        owner = generated_owner(old)
        new = "src/engine/" + owner + "/" + old.removeprefix("src/engine/")
        generated_paths[old] = new
        paths[old] = new
    for old, new in paths.items():
        for prefix in ("", str(build) + "/", str(build.relative_to(repo)) + "/"):
            replacements[prefix + new] = prefix + old
    # Exact generated directories appear in module file-set bases and one macro.
    for old, new in generated_paths.items():
        for prefix in (str(build) + "/", str(build.relative_to(repo)) + "/"):
            replacements[prefix + str(Path(new).parent)] = prefix + str(Path(old).parent)
    # CMake emits an rpath to the one moved shared library directory.
    old_shared = str(build / "src/engine")
    new_shared = str(build / "src/engine/capture")
    replacements["-Wl,-rpath," + new_shared] = "-Wl,-rpath," + old_shared
    return owners, paths, generated_paths, sorted(replacements.items(), key=lambda x: -len(x[0]))


def transform(value, replacements):
    if isinstance(value, str):
        # Simultaneous replacement prevents a newly restored prefix being rebased twice.
        if not replacements:
            return value
        pattern = re.compile("|".join(re.escape(new) for new, _ in replacements))
        table = dict(replacements)
        return pattern.sub(lambda m: table[m.group()], value)
    if isinstance(value, list):
        return [transform(x, replacements) for x in value]
    if isinstance(value, dict):
        return {k: transform(v, replacements) for k, v in value.items()}
    return value


def without_backtraces(value):
    if isinstance(value, list):
        return [without_backtraces(v) for v in value]
    if isinstance(value, dict):
        return {k: without_backtraces(v) for k, v in value.items()
                if k not in {"backtrace", "backtraces"}}
    return value


def canonical_target(target, replacements):
    target = transform(without_backtraces(target), replacements)
    groups = target.get("compileGroups", [])
    sources = []
    for row in target.get("sources", []):
        result = {k: v for k, v in row.items()
                  if k not in {"sourceGroupIndex", "compileGroupIndex", "fileSetIndex", "fileSetIndexes"}}
        if "compileGroupIndex" in row:
            result["compile"] = {k: v for k, v in groups[row["compileGroupIndex"]].items()
                                 if k != "sourceIndexes"}
        indices = row.get("fileSetIndexes", [row["fileSetIndex"]] if "fileSetIndex" in row else [])
        result["file_sets"] = [target["fileSets"][i] for i in indices]
        sources.append(result)
    def edges(key):
        return sorted({e["id"].split("::@")[0] for e in target.get(key, [])})
    return {"type": target["type"], "name": target["name"],
            "sources": sorted(sources, key=lambda x: (x["path"], json.dumps(x, sort_keys=True))),
            "compile_dependencies": edges("compileDependencies"),
            "dependencies": edges("dependencies"),
            "artifacts": target.get("artifacts", []),
            "link": target.get("link"), "archive": target.get("archive"),
            "isGeneratorProvided": target.get("isGeneratorProvided", False)}


def ninja_graph(path):
    text = Path(path).read_text().replace("$\n", "")
    graph = {}
    def words(text):
        escaped = []
        def stash(match):
            escaped.append(match.group(1))
            return f"\x01{len(escaped)-1}\x02"
        text = re.sub(r"\$(.)", stash, text)
        return [re.sub(r"\x01(\d+)\x02", lambda m: escaped[int(m.group(1))], w)
                for w in text.split() if w not in {"|", "||", "|@"}]
    for line in text.splitlines():
        match = re.match(r"^build (.*?): (\S+)(.*)$", line)
        if match:
            outputs = words(match[1])
            inputs = words(match[3])
            for output in outputs:
                graph[output] = inputs
    return graph


def target_closure(graph, targets, start):
    seen = set()
    todo = [start]
    while todo:
        node = todo.pop()
        if node in seen:
            continue
        seen.add(node)
        todo.extend(graph.get(node, []))
    return sorted(name for name, t in targets.items()
                  if name in seen or any(a["path"] in seen for a in t.get("artifacts", [])))


def run_verification(repo, evidence, preset, output):
    repo = repo.resolve()
    build = repo / "build" / ("rewrite-" + preset)
    baseline = evidence / "baseline"
    before = load_targets(baseline / (preset + "-file-api"))
    after = load_targets(build / ".cmake/api/v1/reply")
    generated = read_json(baseline / (preset + "-generated.json"))
    owners, paths, generated_paths, replacements = make_relocations(repo, build, before, generated)
    errors = []
    def need(ok, text):
        if not ok:
            errors.append(text)
    pinned = read_json(evidence / "verification" / (preset + "-OWNER-ALLOWLIST.json"))
    need(pinned["owners"] == owners and pinned["artifacts_and_generated"] == paths and
         pinned["generated"] == generated_paths and pinned["replacements"] == dict(replacements),
         "owner/path allowlist changed since pre-comparison pin")
    need(set(before) == set(after), "target name set differs")
    target_differences = {}
    for name in sorted(set(before) & set(after)):
        if name in owners:
            need(after[name].get("paths", {}).get("source") == "src/engine/" + owners[name],
                 "target not created by its owning directory: " + name)
        old = canonical_target(before[name], [])
        new = canonical_target(after[name], replacements)
        if old != new:
            target_differences[name] = {"before": old, "after": new}
            errors.append("target semantics differ: " + name)
    engine = repo / "src/engine"
    actual_dirs = {str(p.relative_to(engine)) for p in engine.rglob("*") if p.is_dir()}
    need(actual_dirs == set(DIRECTORIES), "engine physical directory inventory differs")
    for relative in ("", *DIRECTORIES):
        directory = engine / relative
        cmake = directory / "CMakeLists.txt"
        need(cmake.is_file(), "missing CMakeLists: " + str(cmake))
        if not cmake.is_file():
            continue
        text = re.sub(r"#.*", "", cmake.read_text())
        need(not re.search(r"\bproject\s*\(", text, re.I), "nested project(): " + relative)
        children = re.findall(r"\badd_subdirectory\s*\(\s*\"?([A-Za-z_]+)\"?\s*\)", text, re.I)
        expected = sorted(p.name for p in directory.iterdir() if p.is_dir())
        need(sorted(children) == expected, "immediate child reachability differs: " + relative)
    old_tests = read_json(baseline / (preset + "-ctest.json"))["tests"]
    current = subprocess.run(["ctest", "--test-dir", str(build), "--show-only=json-v1"],
                             cwd=repo, capture_output=True, check=True)
    new_tests = json.loads(current.stdout)["tests"]
    (output.parent / "ctest-inventory.json").write_bytes(current.stdout)
    def test_table(rows, changed):
        table = {}
        for row in rows:
            if row["name"] in table:
                raise RuntimeError("Duplicate test name: " + row["name"])
            values = {k: v for k, v in row.items() if k not in {"name", "backtrace"}}
            if changed:
                command = row.get("command", [])
                # The owning target fixes the exact allowed Catch2 cwd relocation.
                artifact_to_name = {str(build / a["path"]): n for n, t in after.items()
                                    for a in t.get("artifacts", [])}
                name = artifact_to_name.get(command[0] if command else "")
                owner = owners.get(name)
                if owner:
                    for prop in values.get("properties", []):
                        if prop["name"] == "WORKING_DIRECTORY" and prop["value"] == str(build / "src/engine" / owner):
                            prop["value"] = str(build / "src/engine")
                values = transform(values, replacements)
            table[row["name"]] = values
        return table
    old_table = test_table(old_tests, False)
    new_table = test_table(new_tests, True)
    need(bool(old_table) and set(old_table) == set(new_table), "registered test name set differs")
    test_differences = {n: {"before": old_table[n], "after": new_table[n]}
                        for n in old_table.keys() & new_table.keys() if old_table[n] != new_table[n]}
    need(not test_differences, "registered test command/properties differ")
    old_ninja = ninja_graph(baseline / (preset + "-build.ninja"))
    new_ninja = ninja_graph(build / "build.ninja")
    closures = {}
    for start in ("all", "planar_tests"):
        old = target_closure(old_ninja, before, start)
        new = target_closure(new_ninja, after, start)
        closures[start] = {"before": old, "after": new}
        need(old == new and bool(old), "target closure differs: " + start)
    old_configure = set(old_ninja.get("build.ninja", []))
    new_configure = {transform(p, replacements) for p in new_ninja.get("build.ninja", [])}
    new_component_inputs = {str(repo / "src/engine" / d / "CMakeLists.txt") for d in DIRECTORIES}
    need(new_configure - new_component_inputs == old_configure,
         "configure dependency set differs beyond exact new component CMake inputs")
    generated_rows = {}
    actual_generated = {str(p.relative_to(build))
                        for directory in DIRECTORIES if "/" not in directory
                        for p in (build / "src/engine" / directory).rglob("*")
                        if p.is_file() and p.suffix in {".cpp", ".cppm", ".lua"}
                        and "CMakeFiles" not in p.parts}
    need(actual_generated == set(generated_paths.values()), "candidate generated-file inventory differs")
    for old, new in generated_paths.items():
        old_file = build / old
        new_file = build / new
        need(old_file.is_file() and new_file.is_file(), "missing generated baseline/candidate: " + old)
        if not old_file.is_file() or not new_file.is_file():
            continue
        old_bytes = old_file.read_bytes()
        new_bytes = new_file.read_bytes()
        need(digest(old_bytes) == generated[old]["sha256"], "retained generated baseline changed: " + old)
        # Only exact allowed path strings can differ; all code/payload bytes remain compared.
        normalized = transform(new_bytes.decode(), replacements).encode()
        need(old_bytes == normalized, "generated payload differs: " + old)
        generated_rows[old] = {"candidate": new, "baseline_sha256": digest(old_bytes),
                               "candidate_sha256": digest(new_bytes),
                               "path_normalized_sha256": digest(normalized)}
    source_before = read_json(evidence / "SOURCE-BEFORE.json")
    current_non_cmake = {str(p.relative_to(repo)) for p in (repo / "src").rglob("*")
                         if p.is_file() and p.name != "CMakeLists.txt"}
    need(current_non_cmake == set(source_before), "non-CMake source membership changed")
    for path, expected in source_before.items():
        need((repo / path).is_file() and digest((repo / path).read_bytes()) == expected,
             "non-CMake source changed: " + path)
    details = {"target_differences": target_differences, "test_differences": test_differences,
               "closures": closures, "configure_dependencies": {"before": sorted(old_configure),
               "after_normalized": sorted(new_configure)}, "generated": generated_rows}
    details_path = output.parent / "equivalence-details.json"
    details_path.write_text(json.dumps(details, indent=2) + "\n")
    result = {"preset": preset, "targets": len(before), "engine_targets": len(owners),
              "registered_tests": len(new_tests), "generated_payloads": len(generated_rows),
              "directory_cmake_files": 16, "explicit_relocations": dict(replacements),
              "details_sha256": digest(details_path.read_bytes()), "errors": errors}
    output.write_text(json.dumps(result, indent=2) + "\n")
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, required=True)
    parser.add_argument("--evidence", type=Path, required=True)
    parser.add_argument("--preset", choices=("debug", "release"), required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    result = run_verification(args.repo, args.evidence.resolve(), args.preset, args.output)
    print(json.dumps({k: v for k, v in result.items() if k != "explicit_relocations"}))
    return int(bool(result["errors"]))


if __name__ == "__main__":
    sys.exit(main())
