#!/usr/bin/env python3
# R20-CARRY-A evidence tool (decision D36/D40): classifies every added or removed line of the four edited CMakeLists
# files plus x08.cmake into the allowed classes T, R, D, A, C and fails on any other line.
#
#   T  history/ledger: only `TEST_TIMEOUT <n>` appended to the two named planar_rewrite_module TESTS lines
#   R  runtime: the block appended at the END of the file (add_test, set_tests_properties, comments, blanks)
#   D  deleted old read-rewrite-reconfigure triplet lines at the nine embed sites
#   A  added REMAP/LITERAL arguments and per-site remap list construction with its conditionals
#   C  comment lines at touched sites
#
# Usage: classify_cmake_diff.py [repo_root]    (default: current directory; runs `git diff -U0`)
import re
import subprocess
import sys

FILES = [
    "src/engine/history/CMakeLists.txt",
    "src/engine/ledger/CMakeLists.txt",
    "src/engine/runtime/CMakeLists.txt",
    "src/cmd/planar/CMakeLists.txt",
    "tests/native/v5-02-x08-support/x08.cmake",
]
TIMEOUT_FILES = {"src/engine/history/CMakeLists.txt", "src/engine/ledger/CMakeLists.txt"}
RUNTIME = "src/engine/runtime/CMakeLists.txt"

MODULE_VAR = r"(?:source_module|d7b_source_module|x08_module)"
D_PATTERNS = [
    re.compile(r"^\s*set\(" + MODULE_VAR + r"\b"),
    re.compile(r"^\s*file\(READ \"\$\{" + MODULE_VAR + r"\}\""),
    re.compile(r"^\s*file\(RELATIVE_PATH (?:generated_logical|d7b_logical|prompts_logical|x08_portable)\b"),
    re.compile(r"^\s*string\(REPLACE .*(?:source_text|x08_module_text)"),
    re.compile(r"^\s*file\(CONFIGURE OUTPUT \"\$\{" + MODULE_VAR + r"\}\""),
    re.compile(r"^\s*set\(prompts_logical\)\s*$"),
    re.compile(r"^\s*if\(x08_portable MATCHES"),
    re.compile(r"^\s*if\(variant EQUAL (?:1|21)\)\s*$"),
    re.compile(r"^\s*endif\(\)\s*$"),
    re.compile(r"^\s*\"\$\{PROJECT_SOURCE_DIR\}/src/stdlib/planar/bootstrap\.lua\"\)\s*$"),
    re.compile(r"^\s*planar_embed_lua\(planar_x08_bundle_source_0 .*\$\{x08_paths\}\)\s*$"),
    re.compile(r"^\s*$"),
]
A_PATTERNS = [
    re.compile(r"\bREMAP\b"),
    re.compile(r"\bLITERAL\b"),
    re.compile(r"^\s*set\((?:x05_remap|authoring_remap|x08_remap)\b"),
    re.compile(r"^\s*list\(APPEND (?:authoring_remap|x08_remap)\b"),
    re.compile(r"^\s*if\(variant EQUAL (?:1|21)\)\s*$"),
    re.compile(r"^\s*endif\(\)\s*$"),
    re.compile(r"^\s*\"\$\{PROJECT_SOURCE_DIR\}/src/stdlib/planar/bootstrap\.lua\"\s*$"),
    re.compile(r"^\s*planar_embed_lua\(planar_x08_bundle_source_0 .*\$\{x08_paths\}\s*$"),
    re.compile(r"^\s*$"),
]
R_PATTERNS = [
    re.compile(r"^\s*add_test\(NAME replacement\.r20\."),
    re.compile(r"^\s*\"\$\{PROJECT_SOURCE_DIR\}/tests/native/r20-carry-a-support/"),
    re.compile(r"^\s*set_tests_properties\(replacement\.r20\."),
    re.compile(r"^\s*$"),
]
T_ADDED = re.compile(r"\sTEST_TIMEOUT \d+\)\s*$")


def git_diff(root, path):
    out = subprocess.run(["git", "diff", "-U0", "--", path], cwd=root, check=True, capture_output=True, text=True)
    return out.stdout


def parse(diff):
    """Yield (sign, text, old_start, old_count, tail_hunk) for each changed line."""
    hunks = []
    cur = None
    for line in diff.splitlines():
        m = re.match(r"^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@", line)
        if m:
            cur = {"old_start": int(m.group(1)), "old_count": int(m.group(2) or 1), "lines": []}
            hunks.append(cur)
        elif cur is not None and line[:1] in "+-" and not line.startswith(("+++", "---")):
            cur["lines"].append((line[0], line[1:]))
    return hunks


def old_line_count(root, path):
    out = subprocess.run(["git", "show", f"HEAD:{path}"], cwd=root, check=True, capture_output=True, text=True)
    return len(out.stdout.splitlines())


def classify(path, hunks, root):
    result = []  # (cls, sign, text)
    runtime_len = old_line_count(root, path) if path == RUNTIME else None
    for hunk in hunks:
        removed = [t for s, t in hunk["lines"] if s == "-"]
        for sign, text in hunk["lines"]:
            cls = None
            stripped = text.strip()
            if path in TIMEOUT_FILES:
                if sign == "+" and T_ADDED.search(text):
                    base = T_ADDED.sub(")", text)
                    if base in removed:
                        cls = "T"
                elif sign == "-" and any(a.startswith(text[:-1]) and T_ADDED.search(a) for s2, a in hunk["lines"] if s2 == "+"):
                    cls = "T"
            elif path == RUNTIME:
                at_end = sign == "+" and hunk["old_count"] == 0 and hunk["old_start"] == runtime_len
                if at_end:
                    if stripped.startswith("#") and "R20-CARRY-A" in text or stripped.startswith("# tests/native/r20") or (
                            stripped.startswith("#") and result and result[-1][0] == "R"):
                        cls = "R"
                    elif any(p.search(text) for p in R_PATTERNS):
                        cls = "R"
                elif stripped.startswith("#"):
                    cls = "C"
                elif sign == "-" and any(p.search(text) for p in D_PATTERNS):
                    cls = "D"
                elif sign == "+" and any(p.search(text) for p in A_PATTERNS):
                    cls = "A"
            else:
                if stripped.startswith("#"):
                    cls = "C"
                elif sign == "-" and any(p.search(text) for p in D_PATTERNS):
                    cls = "D"
                elif sign == "+" and any(p.search(text) for p in A_PATTERNS):
                    cls = "A"
            result.append((cls, sign, text))
    return result


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else "."
    counts = {c: 0 for c in "TRDAC"}
    unclassified = []
    for path in FILES:
        for cls, sign, text in classify(path, parse(git_diff(root, path)), root):
            if cls is None:
                unclassified.append((path, sign, text))
            else:
                counts[cls] += 1
        per_file = classify(path, parse(git_diff(root, path)), root)
        print(f"{path}: {len(per_file)} changed lines")
    for c in "TRDAC":
        print(f"class {c}: {counts[c]}")
    print(f"UNCLASSIFIED: {len(unclassified)}")
    for path, sign, text in unclassified:
        print(f"UNCLASSIFIED {path}: {sign}{text}")
    return 1 if unclassified else 0


if __name__ == "__main__":
    sys.exit(main())
