#!/usr/bin/env python3
"""Direct replacement verification. Never runs installed/legacy Planar binaries."""
import argparse
import json
from pathlib import Path
import os
import subprocess
import tempfile
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]

def run(command, **kwargs):
    print("+ " + " ".join(map(str, command)), flush=True)
    return subprocess.run(list(map(str, command)), cwd=ROOT, check=True, **kwargs)

def lint(build):
    commands = json.loads((build / "compile_commands.json").read_text())
    sources = sorted({str(Path(row["file"]).resolve()) for row in commands
                      if Path(row["file"]).resolve().is_relative_to(ROOT / "src")})
    if not sources:
        raise RuntimeError("No active first-party C++ sources")
    cache = (build / "CMakeCache.txt").read_text().splitlines()
    compiler = next(row.split("=", 1)[1] for row in cache if row.startswith("CMAKE_CXX_COMPILER:"))
    formatter = Path(compiler).parent / "clang-format"
    run([formatter, "--dry-run", "--Werror", *sources])
    with tempfile.TemporaryDirectory(prefix="planar-doxygen-") as scratch:
        config = Path(scratch) / "Doxyfile"
        config.write_text('\n'.join([
            'PROJECT_NAME = "Planar replacement"', 'QUIET = YES',
            'GENERATE_HTML = NO', 'GENERATE_LATEX = NO', 'GENERATE_XML = YES',
            'WARNINGS = YES', 'WARN_IF_UNDOCUMENTED = YES',
            'WARN_IF_DOC_ERROR = YES', 'WARN_NO_PARAMDOC = YES',
            'WARN_AS_ERROR = FAIL_ON_WARNINGS', 'EXTRACT_ALL = NO',
            'FILE_PATTERNS = *.cpp *.cppm', 'EXTENSION_MAPPING = cppm=C++',
            'MACRO_EXPANSION = YES', 'EXPAND_ONLY_PREDEF = YES', 'PREDEFINED = export=',
            'INPUT = ' + ' '.join('"' + source + '"' for source in sources),
            'OUTPUT_DIRECTORY = "' + scratch + '"',
        ]) + '\n')
        run(["bash", ROOT / "scripts/cpp-lint-doxygen.sh", config])
    print(f"Formatting and Doxygen checked {len(sources)} active source files", flush=True)

def test(build):
    listing = run(["ctest", "--test-dir", build, "--show-only=json-v1"], capture_output=True, text=True)
    count = len(json.loads(listing.stdout)["tests"])
    if count == 0:
        raise RuntimeError("Zero registered tests")
    with tempfile.TemporaryDirectory(prefix="planar-ctest-") as scratch:
        report = Path(scratch) / "tests.xml"
        # Each case owns a temporary root, so cases run in parallel; PLANAR_TEST_JOBS=1 restores serial runs.
        jobs = os.environ.get("PLANAR_TEST_JOBS") or str(os.cpu_count() or 1)
        run(["ctest", "--test-dir", build, "--parallel", jobs, "--output-on-failure", "--output-junit", report])
        cases = ET.parse(report).findall(".//testcase")
        if len(cases) != count or any(case.find("skipped") is not None or
                                    case.get("status") in {"notrun", "disabled"} for case in cases):
            raise RuntimeError("Test count mismatch or skipped test")
    print(f"Executed {count} registered tests; zero skips", flush=True)

parser = argparse.ArgumentParser()
parser.add_argument("mode", choices=["test", "lint", "gate"])
parser.add_argument("--build-dir", required=True, type=Path)
args = parser.parse_args()
build = args.build_dir.resolve()
if args.mode in {"test", "gate"}:
    run(["cmake", "--build", build, "--target", "all", "planar_tests"])
    test(build)
if args.mode in {"lint", "gate"}:
    lint(build)
if args.mode == "gate":
    result = run(["cmake", "--build", build, "--target", "all", "planar_tests"], capture_output=True, text=True)
    print(result.stdout, end="", flush=True)
    if "ninja: no work to do." not in result.stdout:
        raise RuntimeError("No-change build performed work")
