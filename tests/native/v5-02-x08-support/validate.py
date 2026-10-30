#!/usr/bin/env python3
"""Closeout validator for V5-02-X08 gates G01-G09 (decisions row 21).

Usage:
  validate.py --evidence <evidence-dir> --repo <repo-root>
  validate.py --self-test

Prints one line per gate, `Gxx PASS <detail>` or `Gxx FAIL <reason>`, and exits 1
when any gate fails. The validator only reads raw logs, inventories,
requirements.json and documentation; it never runs ctest, a build or a product
binary. Python standard library only.

Evidence layout accepted (paths relative to the evidence directory):
  requirements.json
      {"requirements": [{"id": "E01".."E14", "tests": [str], "binary": [str], "logs": [str]}],
       "matrix": [{"row": 1..12, "subclauses": [{"name": str, "tests": [str], "binary": [str]}],
                   "logs": [str]}],
       "platform": [{"id": "G06a".."G06i", "branch": str, "test_or_log": str, "pass_line": str}],
       "images": {"base_digest": str, "llvm": str, "cmake": str, "ninja": str}}
  raw/macos-debug.ctest.log, raw/macos-debug.inventory.txt
  raw/macos-release.ctest.log, raw/macos-release.inventory.txt
      an inventory is `ctest -N` output (`  Test  #n: name`, optional `Total Tests:` line)
      or one name per line; both are reduced to the sorted name list
  raw/prior-names.txt        exactly the 1227 names that existed before X08, one per line (1260 total with X08, C01 and R6)
  raw/*.exit                 exit codes written by the runner; each reads 0
  raw/*lint*.log, raw/*redaction*.log   must exist
  raw/*noop*.log             two no-op builds; each contains "ninja: no work to do."
  linux/container/debug-ctest.log, release-ctest.log     container ctest logs
  linux/container/debug-ctest.exit, release-ctest.exit   each reads 0
  linux/container/debug-inventory-sorted.txt, release-inventory-sorted.txt   sorted names
  linux/container/toolchain.txt   `clang version 23.x.y`, and a `dpkg` line there or an installed `clang-23` row in dpkg-list.txt
  linux/container/*noop*.log or *nowork*.log   at least one no-op build log with the phrase
  linux/container/context-manifest.txt    files COPYed after dockerignore exclusions
  linux/host-manifest.txt, linux/host-manifest-after.txt   runner manifests before and after
  linux/docker-build.exit (reads 0), linux/identity-before.txt, linux/identity-after.txt

The repository root must contain docs/vernacular/acceptance-matrix.md, docs/testing.md,
docs/architecture.md, docs/cli-reference.md, docs/vernacular/roadmap.md and
docs/vernacular/rounds/V5-02-X08.md.

Accepted evidence is only the closeout's raw output; a missing file is a gate failure
naming the file. The self-test (ctest x08.validator_selftest) builds a synthetic good
tree and requires exit 0 on it and a refusal naming the right gate for each mutation.
"""

import argparse
import hashlib
import json
import re
import shutil
import sys
import tempfile
from pathlib import Path

PRIOR_COUNT = 1227
X08_CTEST_NAMES = (
    "x08.e01_no_discovery",
    "x08.e01_registration_policy",
    "x08.e02_envelope_rows_binary",
    "x08.e03_closure_separation_binary",
    "x08.e07_old_goldens_unselected_profile",
    "x08.e07_old_w_vectors_binary",
    "x08.e08_projection_vector_binary",
    "x08.e09_history_id_binary",
    "x08.e10_removed_profile_replay_binary",
    "x08.e12_variant_reads_identically",
    "x08.e12_variant_exports_identically",
    "x08.e12_variant_poison_is_live",
    "x08.e12_variant_poison_is_live_load",
    "x08.e12_variant_poison_is_live_profile_render",
    "x08.e12_variant_audit",
    "x08.e12_sources_and_profile_removed",
    "x08.adv_hardlink_module_refuses_before_chunk",
    "x08.adv_missing_declaration_initial_admission",
    "x08.adv_opaque_ordering_binary",
    "x08.adv_invalid_unselected_job_refuses_all",
    "x08.adv_job_mix_and_phase_failures",
    "x08.adv_registry_colocation_binary",
    "x08.adv_limits_cli",
    "x08.timeout_headroom_531",
    "x08.linux_recipe_pins",
    "x08.validator_selftest",
)
X08_CATCH2_NAMES = (
    "replacement.X08 platform branches are exactly the audited set",
    "replacement.X08 Linux publish and metadata branches keep their no-replace and path-only flags",
)
X08_NAMES = X08_CTEST_NAMES + X08_CATCH2_NAMES
C01_ADDED_NAMES = (
    "replacement.host path maps a regular file followed by parent traversal to the root directory error",
    "replacement.host path keeps missing and unresolvable directory spellings as input path errors",
    "replacement.X08 C01 the C02 budget fixture root has a fixed canonical byte length",
)
R6_ADDED_NAMES = (
    "replacement.X08 C01 native decimal id facts are bounded at INT64_MAX exactly",
    "replacement.X08 C01 role preparation outputs-off boundary pins the request-record conversion charge",
)
EXPECTED_TOTAL = PRIOR_COUNT + len(X08_NAMES) + len(C01_ADDED_NAMES) + len(R6_ADDED_NAMES)
REQUIREMENTS = tuple(f"E{number:02d}" for number in range(1, 15))
BINARY_REQUIREMENTS = ("E01", "E02", "E03", "E07", "E08", "E09", "E10", "E12", "E13")
MATRIX_ROWS = tuple(range(1, 13))
BINARY_MATRIX_ROWS = (3, 5, 6, 12)
MIN_SUBCLAUSES = {3: 5, 5: 6, 6: 4, 12: 4}
PLATFORM_IDS = tuple(f"G06{letter}" for letter in "abcdefghi")
LINUX_DEBUG_LOG = "linux/container/debug-ctest.log"
NOOP_PHRASE = "ninja: no work to do."
ROUND_RECORD = "docs/vernacular/rounds/V5-02-X08.md"
ROADMAP = "docs/vernacular/roadmap.md"

# Older ctest prints `100% tests passed, 0 tests failed out of N`; the pinned ctest prints `100% tests passed out of N` on success.
SUMMARY_RE = re.compile(r"(\d+)% tests passed(?:, (\d+) tests failed)? out of (\d+)")
RESULT_RE = re.compile(r"Test\s+#(\d+):\s+(.+?)\s+\.{2,}\s*(\S.*?)\s*$")
PASSED_RE = re.compile(r"Passed(\s+[\d.]+ sec)?")
INVENTORY_RE = re.compile(r"^\s*Test\s+#\d+:\s+(.*?)\s*$")
CLANG_RE = re.compile(r"clang version (\d+)\.(\d+)\.(\d+)")


class Problem(Exception):
    """A gate failure with a human-readable reason."""


class Tree:
    """A directory whose files the gates read; every read failure is a Problem."""

    def __init__(self, root):
        self.root = Path(root)

    def text(self, rel):
        try:
            return (self.root / rel).read_text(encoding="utf-8")
        except (OSError, UnicodeDecodeError):
            raise Problem(f"missing or unreadable {rel}") from None

    def load_json(self, rel):
        try:
            return json.loads(self.text(rel))
        except ValueError:
            raise Problem(f"{rel} is not valid JSON") from None

    def exists(self, rel):
        return (self.root / rel).is_file()

    def matching(self, subdir, pattern):
        base = self.root / subdir
        if not base.is_dir():
            return []
        return sorted(
            path.relative_to(self.root).as_posix() for path in base.glob(pattern) if path.is_file()
        )


def lines_of(text):
    """Nonblank lines with surrounding whitespace removed."""
    return [line.strip() for line in text.splitlines() if line.strip()]


def inventory_names(text):
    """Test names from a `ctest -N` listing or a one-name-per-line list, in file order."""
    names = []
    for line in text.splitlines():
        if not line.strip() or line.lstrip().startswith(("Total Tests:", "Test project ")):
            continue
        match = INVENTORY_RE.match(line)
        names.append(match.group(1) if match else line.strip())
    return names


def inventory_problems(names, label):
    problems = []
    if len(names) != EXPECTED_TOTAL:
        problems.append(f"{label} has {len(names)} names, expected {EXPECTED_TOTAL}")
    if len(set(names)) != len(names):
        problems.append(f"{label} repeats a name")
    return problems


def ctest_problems(text, label, expected):
    """Check one ctest log against the expected inventory set; returns the problems found."""
    problems = []
    summaries = SUMMARY_RE.findall(text)
    if len(summaries) != 1:
        problems.append(f"{label} has {len(summaries)} ctest summary lines, expected 1")
    else:
        percent, failed, total = (int(value or 0) for value in summaries[0])
        if total != EXPECTED_TOTAL:
            problems.append(f"{label} summary total is {total}, expected {EXPECTED_TOTAL}")
        if failed:
            problems.append(f"{label} has {failed} failed tests")
        if percent != 100:
            problems.append(f"{label} reports {percent}% passed")
    for marker in ("Skipped", "Not Run"):
        if marker in text:
            problems.append(f"{label} contains '{marker}'")
    results = []
    for line in text.splitlines():
        match = RESULT_RE.search(line)
        if match:
            results.append(match.groups())
    if len(results) != EXPECTED_TOTAL:
        problems.append(f"{label} has {len(results)} per-test results, expected {EXPECTED_TOTAL}")
    not_passed = [name for _, name, status in results if not PASSED_RE.fullmatch(status)]
    if not_passed:
        problems.append(f"{label} has {len(not_passed)} results not Passed, first {not_passed[0]!r}")
    names = [name for _, name, _ in results]
    if len(set(names)) != len(names):
        problems.append(f"{label} repeats a per-test name")
    if set(names) != expected:
        missing = len(expected - set(names))
        extra = len(set(names) - expected)
        problems.append(f"{label} per-test names differ from its inventory (missing {missing}, extra {extra})")
    return problems


def exit_problems(tree, rel):
    code = tree.text(rel).strip()
    return [] if code == "0" else [f"{rel} is {code!r}, expected 0"]


def toolchain_problems(tree):
    text = tree.text("linux/container/toolchain.txt")
    problems = []
    match = CLANG_RE.search(text)
    if not match:
        problems.append("toolchain.txt has no exact clang version")
    elif match.group(1) != "23":
        problems.append(f"toolchain.txt clang major is {match.group(1)}, expected 23")
    if not any("dpkg" in line for line in text.splitlines()) and not (
        tree.exists("linux/container/dpkg-list.txt")
        and any(line.startswith("ii") and " clang-23 " in line for line in tree.text("linux/container/dpkg-list.txt").splitlines())
    ):
        problems.append("toolchain.txt has no dpkg line and dpkg-list.txt has no installed clang-23 row")
    return problems


def manifest_problems(tree):
    context = lines_of(tree.text("linux/container/context-manifest.txt"))
    host = lines_of(tree.text("linux/host-manifest.txt"))
    after = lines_of(tree.text("linux/host-manifest-after.txt"))
    problems = []
    if not context:
        problems.append("context-manifest.txt is empty")
    if context != host:
        problems.append("context-manifest.txt differs from host-manifest.txt")
    if host != after:
        problems.append("host-manifest-after.txt differs from host-manifest.txt")
    return problems


def identity_problems(tree):
    before = tree.text("linux/identity-before.txt")
    after = tree.text("linux/identity-after.txt")
    problems = []
    if not before.strip():
        problems.append("identity-before.txt is empty")
    if before != after:
        problems.append("identity-before.txt differs from identity-after.txt")
    return problems


def gate_g01(ev, _repo):
    prior = lines_of(ev.text("raw/prior-names.txt"))
    inventory = inventory_names(ev.text("raw/macos-debug.inventory.txt"))
    problems = []
    if len(prior) != PRIOR_COUNT or len(set(prior)) != PRIOR_COUNT:
        problems.append(f"prior-names.txt has {len(set(prior))} unique names, expected {PRIOR_COUNT}")
    problems += inventory_problems(inventory, "macos debug inventory")
    present = set(inventory)
    absent = [name for name in prior if name not in present]
    if absent:
        problems.append(f"{len(absent)} prior names missing from the debug inventory, first {absent[0]!r}")
    new_absent = [name for name in X08_NAMES + C01_ADDED_NAMES + R6_ADDED_NAMES if name not in present]
    if new_absent:
        problems.append(f"X08, C01 and R6 names missing from the debug inventory: {', '.join(new_absent[:3])}")
    problems += ctest_problems(ev.text("raw/macos-debug.ctest.log"), "macos debug ctest", present)
    return problems, f"{EXPECTED_TOTAL} tests, 0 failed, 0 skipped, {PRIOR_COUNT} prior names present"


def gate_g02(ev, _repo):
    debug = inventory_names(ev.text("raw/macos-debug.inventory.txt"))
    release = inventory_names(ev.text("raw/macos-release.inventory.txt"))
    problems = inventory_problems(release, "macos release inventory")
    if sorted(release) != sorted(debug):
        problems.append("macos release inventory differs from the debug inventory")
    problems += ctest_problems(ev.text("raw/macos-release.ctest.log"), "macos release ctest", set(release))
    return problems, f"{EXPECTED_TOTAL} tests, 0 failed, 0 skipped, release names equal debug"


def gate_g03(ev, _repo):
    problems = []
    exits = ev.matching("raw", "*.exit")
    if not exits:
        problems.append("no raw/*.exit files")
    for rel in exits:
        problems += exit_problems(ev, rel)
    for kind in ("lint", "redaction"):
        if not ev.matching("raw", f"*{kind}*.log"):
            problems.append(f"no raw {kind} log")
    noops = ev.matching("raw", "*noop*.log")
    if len(noops) < 2:
        problems.append(f"{len(noops)} no-op build logs, expected 2")
    for rel in noops:
        if NOOP_PHRASE not in ev.text(rel):
            problems.append(f"{rel} lacks {NOOP_PHRASE!r}")
    return problems, f"{len(exits)} exit files zero, {len(noops)} no-op builds with zero steps"


def gate_g04(ev, _repo):
    debug = inventory_names(ev.text("raw/macos-debug.inventory.txt"))
    linux = inventory_names(ev.text("linux/container/debug-inventory-sorted.txt"))
    problems = inventory_problems(linux, "linux debug inventory")
    if sorted(linux) != sorted(debug):
        problems.append("linux debug inventory differs from the macOS debug inventory")
    problems += ctest_problems(ev.text("linux/container/debug-ctest.log"), "linux debug ctest", set(linux))
    problems += exit_problems(ev, "linux/container/debug-ctest.exit")
    problems += exit_problems(ev, "linux/docker-build.exit")
    problems += toolchain_problems(ev)
    problems += manifest_problems(ev)
    problems += identity_problems(ev)
    if not any(NOOP_PHRASE in ev.text(rel) for rel in ev.matching("linux/container", "*noop*.log") + ev.matching("linux/container", "*nowork*.log")):
        problems.append(f"no linux container no-op build log with {NOOP_PHRASE!r}")
    return problems, f"{EXPECTED_TOTAL} tests, 0 failed, 0 skipped; inventory equal; manifests and identity equal"


def gate_g05(ev, _repo):
    macos = inventory_names(ev.text("raw/macos-release.inventory.txt"))
    linux = inventory_names(ev.text("linux/container/release-inventory-sorted.txt"))
    problems = inventory_problems(linux, "linux release inventory")
    if sorted(linux) != sorted(macos):
        problems.append("linux release inventory differs from the macOS release inventory")
    problems += ctest_problems(ev.text("linux/container/release-ctest.log"), "linux release ctest", set(linux))
    problems += exit_problems(ev, "linux/container/release-ctest.exit")
    return problems, f"{EXPECTED_TOTAL} tests, 0 failed, 0 skipped; inventory equal to macOS release"


def gate_g06(ev, _repo):
    requirements = ev.load_json("requirements.json")
    problems = []
    platform = requirements.get("platform")
    if not isinstance(platform, list):
        return ["requirements.json has no platform list"], ""
    linux_log = ev.text(LINUX_DEBUG_LOG)
    rows = {}
    for row in platform:
        if not isinstance(row, dict):
            problems.append("a platform row is not an object")
            continue
        if row.get("id") in rows:
            problems.append(f"platform row {row.get('id')} repeats")
        rows[row.get("id")] = row
    for pid in PLATFORM_IDS:
        row = rows.get(pid)
        if row is None:
            problems.append(f"platform row {pid} missing")
            continue
        for field in ("branch", "test_or_log", "pass_line"):
            value = row.get(field)
            if not isinstance(value, str) or not value.strip():
                problems.append(f"platform row {pid} has an empty {field}")
        line = row.get("pass_line")
        if isinstance(line, str) and line.strip() and line.strip() not in linux_log:
            problems.append(f"platform row {pid} pass_line does not occur in {LINUX_DEBUG_LOG}")
    for pid in sorted(set(rows) - set(PLATFORM_IDS), key=str):
        problems.append(f"unexpected platform row {pid}")
    images = requirements.get("images")
    if not isinstance(images, dict):
        problems.append("requirements.json has no images object")
    else:
        for key in ("base_digest", "llvm", "cmake", "ninja"):
            if not isinstance(images.get(key), str) or not images[key].strip():
                problems.append(f"images.{key} is not recorded")
    return problems, f"{len(PLATFORM_IDS)} platform rows with branch, test or log and pass line; images recorded"


def index_rows(items, key, label, problems):
    indexed = {}
    if not isinstance(items, list):
        problems.append(f"{label} is not a list")
        return indexed
    for item in items:
        if not isinstance(item, dict) or key not in item:
            problems.append(f"a {label} entry lacks {key}")
            continue
        if item[key] in indexed:
            problems.append(f"{label} {item[key]} repeats")
        indexed[item[key]] = item
    return indexed


def check_names(label, names, inventory, problems):
    if not isinstance(names, list) or not all(isinstance(name, str) for name in names):
        problems.append(f"{label} names are not a list of strings")
        return
    for name in names:
        if name not in inventory:
            problems.append(f"{label} names {name!r}, which is not in the macOS debug inventory")


def check_logs(ev, label, logs, problems):
    if not isinstance(logs, list) or not logs:
        problems.append(f"{label} has no raw log paths")
        return
    for rel in logs:
        if not isinstance(rel, str) or not ev.exists(rel):
            problems.append(f"{label} log {rel!r} not found")


def gate_g07(ev, _repo):
    requirements = ev.load_json("requirements.json")
    inventory = set(inventory_names(ev.text("raw/macos-debug.inventory.txt")))
    problems = []
    entries = index_rows(requirements.get("requirements"), "id", "requirement", problems)
    for rid in REQUIREMENTS:
        entry = entries.get(rid)
        if entry is None:
            problems.append(f"requirement {rid} missing")
            continue
        tests = entry.get("tests", [])
        binary = entry.get("binary", [])
        check_names(rid, tests, inventory, problems)
        check_names(rid, binary, inventory, problems)
        if rid == "E14":
            continue
        if not tests:
            problems.append(f"requirement {rid} has no test names")
        check_logs(ev, rid, entry.get("logs"), problems)
        if rid in BINARY_REQUIREMENTS and not binary:
            problems.append(f"requirement {rid} has an empty binary list")
    matrix = index_rows(requirements.get("matrix"), "row", "matrix row", problems)
    for row in MATRIX_ROWS:
        entry = matrix.get(row)
        if entry is None:
            problems.append(f"matrix row {row} missing")
            continue
        subclauses = entry.get("subclauses")
        if not isinstance(subclauses, list):
            problems.append(f"matrix row {row} has no sub-clause list")
            continue
        minimum = MIN_SUBCLAUSES.get(row, 1)
        if len(subclauses) < minimum:
            problems.append(f"matrix row {row} has {len(subclauses)} sub-clauses, plan requires {minimum}")
        row_binary = []
        for index, sub in enumerate(subclauses, 1):
            label = f"matrix row {row} sub-clause {index}"
            if not isinstance(sub, dict) or not sub.get("name"):
                problems.append(f"{label} has no name")
                continue
            tests = sub.get("tests", [])
            binary = sub.get("binary", [])
            if not tests and not binary:
                problems.append(f"{label} names no tests")
            check_names(label, tests, inventory, problems)
            check_names(label, binary, inventory, problems)
            if isinstance(binary, list):
                row_binary += binary
        check_logs(ev, f"matrix row {row}", entry.get("logs"), problems)
        if row in BINARY_MATRIX_ROWS and not row_binary:
            problems.append(f"matrix row {row} has no binary test")
    return problems, (
        f"{len(REQUIREMENTS)} requirements and 12 matrix rows; names in inventory; "
        "binary lists present; logs exist"
    )


def section(text, heading):
    """The text from a level-two heading to the next level-two heading, or None."""
    start = text.find(heading)
    if start < 0:
        return None
    end = text.find("\n## ", start + len(heading))
    return text[start:] if end < 0 else text[start:end]


def gate_g08(_ev, repo):
    problems = []
    matrix = repo.text("docs/vernacular/acceptance-matrix.md")
    row = next((line for line in matrix.splitlines() if line.startswith("| A08 ")), "")
    if not row:
        problems.append("acceptance-matrix.md has no A08 row")
    else:
        if "X08" not in row:
            problems.append("A08 row does not cite X08")
        if "limit" not in row.lower():
            problems.append("A08 row does not restate limits")
    if "X08 acceptance gate" not in repo.text("docs/testing.md"):
        problems.append("docs/testing.md lacks 'X08 acceptance gate'")
    for rel in ("docs/architecture.md", "docs/cli-reference.md"):
        if "X08" not in repo.text(rel):
            problems.append(f"{rel} does not describe X08")
    opening = section(repo.text(ROADMAP), "## Current status")
    if opening is None:
        problems.append("roadmap has no Current status opening")
    else:
        for token in ("X08", "RA01"):
            if token not in opening:
                problems.append(f"roadmap opening does not mention {token}")
    return problems, "A08 row limits, testing gate, architecture and CLI X08 anchors, roadmap opening"


def gate_g09(_ev, repo):
    record = repo.text(ROUND_RECORD).lower()
    roadmap = repo.text(ROADMAP).lower()
    problems = []
    if "this has not been done" in record:
        problems.append("round record still says 'this has not been done'")
    if "#531 timing test about 15.5 s of 30 s" in roadmap:
        problems.append("roadmap still says '#531 timing test about 15.5 s of 30 s'")
    if "25704378" not in record and "25704378" not in roadmap:
        problems.append("neither the round record nor the roadmap records commit 25704378")
    return problems, "stale #531 texts absent; commit 25704378 recorded"


GATES = (
    ("G01", gate_g01),
    ("G02", gate_g02),
    ("G03", gate_g03),
    ("G04", gate_g04),
    ("G05", gate_g05),
    ("G06", gate_g06),
    ("G07", gate_g07),
    ("G08", gate_g08),
    ("G09", gate_g09),
)


def run_gates(evidence, repo):
    """Run every gate; returns (gate, passed, detail) tuples where detail is the reason on failure."""
    evidence, repo = Tree(evidence), Tree(repo)
    results = []
    for gate, check in GATES:
        try:
            problems, detail = check(evidence, repo)
        except Problem as exc:
            problems, detail = [str(exc)], ""
        if problems:
            results.append((gate, False, "; ".join(problems)))
        else:
            results.append((gate, True, detail))
    return results


# Self-test: a synthetic good tree and one mutation per refusal the validator must make.

IDENTITY = "candidate frozen-self-test\n"
MANIFEST_LINE = "src/file_0001.cpp"


def write(path, text):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def ctest_log(names):
    lines = ["Test project /scratch/build/rewrite-debug\n"]
    for number, name in enumerate(names, 1):
        lines.append(f"{number:>4}/{len(names)} Test #{number}: {name} {'.' * 40}   Passed    0.01 sec\n")
    lines.append(
        f"\n100% tests passed, 0 tests failed out of {len(names)}\n\nTotal Test time (real) =  12.34 sec\n"
    )
    return "".join(lines)


def inventory_listing(names):
    body = "".join(f"  Test  #{number}: {name}\n" for number, name in enumerate(names, 1))
    return body + f"Total Tests: {len(names)}\n"


def requirement_entries():
    entries = []
    for index, rid in enumerate(REQUIREMENTS):
        tests = [] if rid == "E14" else [f"prior.case_{index:04d}"]
        binary = [X08_NAMES[index % len(X08_NAMES)]] if rid in BINARY_REQUIREMENTS else []
        entries.append({"id": rid, "tests": tests, "binary": binary, "logs": ["raw/macos-debug.ctest.log"]})
    return entries


def matrix_entries():
    rows = []
    for row in MATRIX_ROWS:
        count = MIN_SUBCLAUSES.get(row, 1)
        subclauses = []
        for clause in range(1, count + 1):
            binary = []
            if row in BINARY_MATRIX_ROWS and clause == count:
                binary = [X08_NAMES[row % len(X08_NAMES)]]
            subclauses.append({"name": f"row {row} clause {clause}", "tests": [f"prior.case_{row:04d}"], "binary": binary})
        rows.append({"row": row, "subclauses": subclauses, "logs": ["raw/macos-debug.ctest.log"]})
    return rows


def build_good(root):
    """Write a complete synthetic evidence tree and repository under root."""
    evidence = root / "evidence"
    repo = root / "repo"
    prior = [f"prior.case_{index:04d}" for index in range(PRIOR_COUNT)]
    names = sorted(prior + list(X08_NAMES) + list(C01_ADDED_NAMES) + list(R6_ADDED_NAMES))
    if len(names) != EXPECTED_TOTAL:
        raise RuntimeError("self-test inventory is not the expected size")
    raw = evidence / "raw"
    linux = evidence / "linux" / "container"
    host = evidence / "linux"

    write(raw / "prior-names.txt", "".join(f"{name}\n" for name in prior))
    write(raw / "macos-debug.inventory.txt", inventory_listing(names))
    write(raw / "macos-release.inventory.txt", inventory_listing(names))
    write(raw / "macos-debug.ctest.log", ctest_log(names))
    write(raw / "macos-release.ctest.log", ctest_log(names))
    for exit_file in ("macos-debug.ctest", "macos-release.ctest", "lint", "redaction", "noop-1", "noop-2"):
        write(raw / f"{exit_file}.exit", "0\n")
    write(raw / "lint.log", "clang-format --Werror and Doxygen: clean\n")
    write(raw / "redaction.log", "redaction check over the explicit file list: clean\n")
    write(raw / "noop-1.log", f"{NOOP_PHRASE}\n")
    write(raw / "noop-2.log", f"{NOOP_PHRASE}\n")

    sorted_listing = "".join(f"{name}\n" for name in names)
    write(linux / "debug-inventory-sorted.txt", sorted_listing)
    write(linux / "release-inventory-sorted.txt", sorted_listing)
    write(linux / "debug-ctest.log", ctest_log(names))
    write(linux / "release-ctest.log", ctest_log(names))
    write(linux / "debug-ctest.exit", "0\n")
    write(linux / "release-ctest.exit", "0\n")
    write(linux / "noop-debug.log", f"{NOOP_PHRASE}\n")
    write(linux / "toolchain.txt", "clang version 23.1.0\ndpkg-query: clang-23 1:23.1.0-1~exp1\n")
    manifest = "".join(
        f"{hashlib.sha256(name.encode()).hexdigest()}  src/file_{index:04d}.cpp\n"
        for index, name in enumerate(names[:40])
    )
    write(linux / "context-manifest.txt", manifest)
    write(host / "host-manifest.txt", manifest)
    write(host / "host-manifest-after.txt", manifest)
    write(host / "docker-build.exit", "0\n")
    write(host / "identity-before.txt", IDENTITY)
    write(host / "identity-after.txt", IDENTITY)

    requirements = {
        "requirements": requirement_entries(),
        "matrix": matrix_entries(),
        "platform": [
            {
                "id": pid,
                "branch": f"linux branch {pid}",
                "test_or_log": "linux/container/debug-ctest.log",
                "pass_line": ctest_log(names).splitlines()[1 + index].strip(),
            }
            for index, pid in enumerate(PLATFORM_IDS)
        ],
        "images": {
            "base_digest": "sha256:" + "a" * 64,
            "llvm": "23.1.0",
            "cmake": "4.3.1",
            "ninja": "1.13.0",
        },
    }
    write(evidence / "requirements.json", json.dumps(requirements, indent=1, sort_keys=True))

    write(
        repo / "docs/vernacular/acceptance-matrix.md",
        "| A08 | Capability/mutation/resource abuse bounded; Render | X08 external proof; limits restated. |\n",
    )
    write(repo / "docs/testing.md", "The X08 acceptance gate runs the closeout validator.\n")
    write(repo / "docs/architecture.md", "X08 external profile acceptance is part of the gate.\n")
    write(repo / "docs/cli-reference.md", "X08 external profile commands are documented.\n")
    write(
        repo / ROADMAP,
        "# Planar rewrite roadmap\n\n## Current status\n\n"
        "- X08 closed with RA01 next; #531 satisfied by commit 25704378.\n\n"
        "## Next work\n\n| Step | Scope |\n",
    )
    write(
        repo / ROUND_RECORD,
        "# V5-02-X08\n\nQ7 A is satisfied by commit 25704378; the test is identified by name.\n",
    )


def replace_once(path, old, new):
    text = path.read_text(encoding="utf-8")
    if old not in text:
        raise RuntimeError(f"self-test mutation did not apply to {path.name}: {old!r}")
    path.write_text(text.replace(old, new, 1), encoding="utf-8")


def append(path, text):
    path.write_text(path.read_text(encoding="utf-8") + text, encoding="utf-8")


def edit_json(path, change):
    data = json.loads(path.read_text(encoding="utf-8"))
    change(data)
    path.write_text(json.dumps(data, indent=1, sort_keys=True), encoding="utf-8")


def mutate_skipped(root):
    replace_once(root / "evidence/raw/macos-debug.ctest.log", "   Passed    0.01 sec", "   ***Skipped  0.00 sec")


def mutate_skipped_trailer(root):
    # Every per-test result still reads Passed and the summary is unchanged; only the marker scan can refuse this.
    append(root / "evidence/raw/macos-debug.ctest.log", "\nThe following tests did not run:\n\t   1 - prior.case_0000 (Skipped)\n")


def mutate_not_run_trailer(root):
    append(root / "evidence/raw/macos-debug.ctest.log", "\nThe following tests did not run:\n\t   1 - prior.case_0000 (Not Run)\n")


def mutate_failed(root):
    log = root / "evidence/raw/macos-debug.ctest.log"
    replace_once(log, "   Passed    0.01 sec", "   ***Failed   0.01 sec")
    replace_once(log, f"100% tests passed, 0 tests failed out of {EXPECTED_TOTAL}", f"99% tests passed, 1 tests failed out of {EXPECTED_TOTAL}")


def mutate_prior_missing(root):
    replace_once(root / "evidence/raw/prior-names.txt", "prior.case_0000\n", "prior.never_existed\n")


def mutate_release_inventory(root):
    replace_once(root / "evidence/raw/macos-release.inventory.txt", ": prior.case_0005\n", ": prior.case_0005x\n")


def mutate_linux_inventory(root):
    replace_once(root / "evidence/linux/container/debug-inventory-sorted.txt", "prior.case_0010\n", "prior.case_0010x\n")


def mutate_row12_missing(root):
    edit_json(
        root / "evidence/requirements.json",
        lambda data: data.__setitem__("matrix", [row for row in data["matrix"] if row["row"] != 12]),
    )


def mutate_row3_four_clauses(root):
    def change(data):
        row = next(row for row in data["matrix"] if row["row"] == 3)
        row["subclauses"].pop(0)

    edit_json(root / "evidence/requirements.json", change)


def mutate_binary_empty(root):
    def change(data):
        entry = next(entry for entry in data["requirements"] if entry["id"] == "E01")
        entry["binary"] = []

    edit_json(root / "evidence/requirements.json", change)


def mutate_row6_binary_empty(root):
    def change(data):
        row = next(row for row in data["matrix"] if row["row"] == 6)
        for sub in row["subclauses"]:
            sub["binary"] = []

    edit_json(root / "evidence/requirements.json", change)


def mutate_context_manifest(root):
    replace_once(root / "evidence/linux/container/context-manifest.txt", MANIFEST_LINE, "src/file_0001.cpx")


def mutate_platform_missing(root):
    edit_json(
        root / "evidence/requirements.json",
        lambda data: data.__setitem__("platform", [row for row in data["platform"] if row["id"] != "G06c"]),
    )


def mutate_platform_i_missing(root):
    edit_json(
        root / "evidence/requirements.json",
        lambda data: data.__setitem__("platform", [row for row in data["platform"] if row["id"] != "G06i"]),
    )


def mutate_platform_stale_line(root):
    def change(data):
        data["platform"][0]["pass_line"] = "149/1255 Test  #149: stale.name ...   Passed    0.00 sec"

    edit_json(root / "evidence/requirements.json", change)


def mutate_platform_stale_last(root):
    def change(data):
        data["platform"][-1]["pass_line"] = "149/1255 Test  #149: stale.name ...   Passed    0.00 sec"

    edit_json(root / "evidence/requirements.json", change)


def mutate_platform_release_only_line(root):
    # The line exists in the Linux release log only; a check against any other log accepts it.
    append(root / "evidence/linux/container/release-ctest.log", "\nrelease-only marker line Passed\n")

    def change(data):
        data["platform"][3]["pass_line"] = "release-only marker line Passed"

    edit_json(root / "evidence/requirements.json", change)


def mutate_platform_extra_row(root):
    def change(data):
        row = dict(data["platform"][0])
        row["id"] = "G06z"
        data["platform"].append(row)

    edit_json(root / "evidence/requirements.json", change)


def mutate_platform_duplicate_row(root):
    edit_json(root / "evidence/requirements.json", lambda data: data["platform"].append(dict(data["platform"][2])))


def mutate_platform_empty_field(field):
    def mutate(root):
        edit_json(root / "evidence/requirements.json", lambda data: data["platform"][1].__setitem__(field, "  "))

    return mutate


def mutate_images_absent(root):
    edit_json(root / "evidence/requirements.json", lambda data: data.pop("images"))


def mutate_images_empty_key(root):
    edit_json(root / "evidence/requirements.json", lambda data: data["images"].__setitem__("llvm", ""))


def mutate_r6_name_renamed(root):
    # The count, the per-test set and the release comparison stay consistent; only the R6 required-name check can refuse this.
    name = R6_ADDED_NAMES[0]
    for rel in ("evidence/raw/macos-debug.inventory.txt", "evidence/raw/macos-debug.ctest.log"):
        replace_once(root / rel, name, "replacement.not an R6 name")


def mutate_dpkg_absent(root):
    write(root / "evidence/linux/container/toolchain.txt", "clang version 23.1.0\n")


def mutate_dpkg_unrelated_list(root):
    write(root / "evidence/linux/container/toolchain.txt", "clang version 23.1.0\n")
    write(root / "evidence/linux/container/dpkg-list.txt", "ii  libc6 2.39 arm64 C library\nrc  clang-23 1:23 arm64 removed\n")


def accept_dpkg_list_fallback(root):
    write(root / "evidence/linux/container/toolchain.txt", "clang version 23.1.0\n")
    write(root / "evidence/linux/container/dpkg-list.txt", "ii  libc6 2.39 arm64 C library\nii  clang-23 1:23.1.0 arm64 compiler\n")


def mutate_round_record_stale(root):
    append(root / "repo" / ROUND_RECORD, "\nThe #531 split has not been done.\nthis has not been done\n")


def mutate_roadmap_stale(root):
    append(root / "repo" / ROADMAP, "- #531 timing test about 15.5 s of 30 s\n")


def mutate_identity(root):
    replace_once(root / "evidence/linux/identity-after.txt", "frozen-self-test", "frozen-self-tesX")


def mutate_testing_anchor(root):
    replace_once(root / "repo/docs/testing.md", "X08 acceptance gate", "acceptance gate")


ACCEPTED_VARIANTS = (("dpkg-list.txt installed clang-23 row without a toolchain dpkg line", accept_dpkg_list_fallback),)

MUTATIONS = (
    ("skipped=1 in the macOS debug log", "G01", mutate_skipped),
    ("a Skipped trailer with all results Passed", "G01", mutate_skipped_trailer),
    ("a Not Run trailer with all results Passed", "G01", mutate_not_run_trailer),
    ("failed=1 in the macOS debug log", "G01", mutate_failed),
    ("a prior name missing from the inventory", "G01", mutate_prior_missing),
    ("release inventory differs from debug", "G02", mutate_release_inventory),
    ("Linux inventory differs from macOS", "G04", mutate_linux_inventory),
    ("requirements.json missing matrix row 12", "G07", mutate_row12_missing),
    ("row 3 with 4 sub-clauses", "G07", mutate_row3_four_clauses),
    ("a required binary list empty", "G07", mutate_binary_empty),
    ("matrix row 6 binary flags absent", "G07", mutate_row6_binary_empty),
    ("context-manifest differs from host-manifest", "G04", mutate_context_manifest),
    ("a platform row missing", "G06", mutate_platform_missing),
    ("platform row G06i (the audited test branch) missing", "G06", mutate_platform_i_missing),
    ("a platform pass_line absent from the Linux debug log", "G06", mutate_platform_stale_line),
    ("a stale pass_line on the last platform row", "G06", mutate_platform_stale_last),
    ("a pass_line present only in the Linux release log", "G06", mutate_platform_release_only_line),
    ("an unexpected extra platform row", "G06", mutate_platform_extra_row),
    ("a repeated platform row", "G06", mutate_platform_duplicate_row),
    ("an empty platform branch", "G06", mutate_platform_empty_field("branch")),
    ("an empty platform test_or_log", "G06", mutate_platform_empty_field("test_or_log")),
    ("an empty platform pass_line", "G06", mutate_platform_empty_field("pass_line")),
    ("the images record absent", "G06", mutate_images_absent),
    ("an images key empty", "G06", mutate_images_empty_key),
    ("an R6 required name absent with the count intact", "G01", mutate_r6_name_renamed),
    ("no dpkg line and no dpkg-list", "G04", mutate_dpkg_absent),
    ("no dpkg line and a dpkg-list without an installed clang-23 row", "G04", mutate_dpkg_unrelated_list),
    ("stale text present in the round record", "G09", mutate_round_record_stale),
    ("stale text present in the roadmap", "G09", mutate_roadmap_stale),
    ("identity-before differs from identity-after", "G04", mutate_identity),
    ("testing.md X08 anchor removed", "G08", mutate_testing_anchor),
)


def self_test():
    if PRIOR_COUNT != 1227 or EXPECTED_TOTAL != 1260:
        print(f"validator_selftest: expected PRIOR_COUNT 1227 and EXPECTED_TOTAL 1260, "
              f"got {PRIOR_COUNT} and {EXPECTED_TOTAL}")
        return 1
    with tempfile.TemporaryDirectory() as scratch:
        base = Path(scratch)
        good = base / "good"
        build_good(good)
        for gate, passed, detail in run_gates(good / "evidence", good / "repo"):
            if not passed:
                print(f"validator_selftest: good tree fails {gate}: {detail}")
                return 1
        for index, (label, gate, mutate) in enumerate(MUTATIONS):
            tree = base / f"mutant-{index:02d}"
            shutil.copytree(good, tree)
            mutate(tree)
            failed = {name for name, passed, _ in run_gates(tree / "evidence", tree / "repo") if not passed}
            if gate not in failed:
                print(f"validator_selftest: {label!r} was not refused by {gate} (failed: {sorted(failed)})")
                return 1
            print(f"validator_selftest: {label!r} refused by {gate}")
        for index, (label, variant) in enumerate(ACCEPTED_VARIANTS):
            tree = base / f"variant-{index:02d}"
            shutil.copytree(good, tree)
            variant(tree)
            failed = {name for name, passed, _ in run_gates(tree / "evidence", tree / "repo") if not passed}
            if failed:
                print(f"validator_selftest: {label!r} was refused by {sorted(failed)}")
                return 1
            print(f"validator_selftest: {label!r} accepted")
    print("validator_selftest passed")
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(description="V5-02-X08 closeout validator (gates G01-G09).")
    parser.add_argument("--evidence", type=Path, help="evidence directory (tests/native/v5-02-x08-evidence)")
    parser.add_argument("--repo", type=Path, help="repository root")
    parser.add_argument("--self-test", action="store_true", help="run the validator's synthetic self-test")
    args = parser.parse_args(argv)
    if args.self_test:
        return self_test()
    if args.evidence is None or args.repo is None:
        parser.error("--evidence and --repo are required unless --self-test is given")
    results = run_gates(args.evidence, args.repo)
    for gate, passed, detail in results:
        print(f"{gate} {'PASS' if passed else 'FAIL'} {detail}")
    return 0 if all(passed for _, passed, _ in results) else 1


if __name__ == "__main__":
    sys.exit(main())
