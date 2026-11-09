#!/usr/bin/env python3
"""Require execution of the R17 native test's retained contract scenarios."""
import argparse
import pathlib
import subprocess
import tempfile
import xml.etree.ElementTree as ET

CASE = "R17 ledger completion retains evidence and changes run to completed atomically"
FACET = "C03 nonempty facet payload retains directory edges"
COMMON = "R17 completed result and retained corruption checks"
REQUIRED = {
    (FACET,),
    (FACET, "C06 retirement removes former lineage facts but keeps historical publication"),
    (FACET, "N8 unrelated complete directory"),
    (FACET, "N8 semantic differs while saved response agrees"),
    ("C03 evidence-only rejects mismatched snapshot identity",),
    ("C03 evidence-only keeps weaker revision and fact preconditions",),
    ("C03 boundary vector cannot name another work",),
    ("C03 boundary vector output must match observation",),
    ("C03 observation requires exact own unit membership",),
    ("C03 observation refuses duplicate units",),
    ("C11 observation refuses an extra unused unit",),
    (COMMON,),
    (COMMON, "C02 malformed completed response"),
    (COMMON, "C02 invalid outcome reason"),
    (COMMON, "C03 invalid reason agrees with saved response"),
    (COMMON, "C02 missing required boundary evidence"),
}


def check(binary, report):
    command = [str(binary), CASE, "--reporter", "xml", "--out", str(report)]
    result = subprocess.run(command, capture_output=True, text=True, timeout=90)
    if result.returncode:
        raise AssertionError(f"native case failed ({result.returncode}): {result.stdout} {result.stderr}")
    root = ET.parse(report).getroot()
    cases = root.findall(".//TestCase")
    assert len(cases) == 1 and cases[0].get("name") == CASE, "expected exactly the named test case"
    for node in root.iter():
        assert node.tag not in ("Skip", "Skipped"), "unexpected skip element"
        assert node.get("skips", "0") in ("0", "false"), "unexpected skipped assertion/case"
        assert node.get("skipped", "0") in ("0", "false"), "unexpected skipped assertion/case"
        if node.tag in ("OverallResults", "OverallResultsCases"):
            assert int(node.get("failures", "0")) == 0, "unexpected assertion/case failure"
            assert int(node.get("expectedFailures", "0")) == 0, "unexpected expected failure"
        if node.tag == "OverallResult":
            assert node.get("success") == "true", "unsuccessful case"
    assertions = root.find("OverallResults")
    case_counts = root.find("OverallResultsCases")
    assert assertions is not None and int(assertions.get("successes", "0")) > 0, "zero assertions"
    assert case_counts is not None and int(case_counts.get("successes", "0")) == 1, "zero or extra cases"
    reached = set()

    def visit(node, parent=()):
        for section in node.findall("Section"):
            path = parent + (section.get("name"),)
            reached.add(path)
            if path in REQUIRED:
                counts = section.find("OverallResults")
                assert counts is not None and int(counts.get("successes", "0")) > 0, (
                    "required scenario has no assertions: " + repr(path))
            visit(section, path)

    visit(cases[0])
    missing = REQUIRED - reached
    assert not missing, "required scenarios were not executed: " + repr(sorted(missing))
    print(f"C11 section coverage: 1 case, {assertions.get('successes')} assertions, "
          f"{len(REQUIRED)} required paths, zero failures/skips")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("binary", type=pathlib.Path)
    parser.add_argument("--xml", type=pathlib.Path, help="retain the native XML report at this path")
    args = parser.parse_args()
    if args.xml:
        args.xml.parent.mkdir(parents=True, exist_ok=True)
        check(args.binary.resolve(), args.xml.resolve())
    else:
        with tempfile.TemporaryDirectory(prefix="planar-c11-sections-") as temporary:
            check(args.binary.resolve(), pathlib.Path(temporary) / "sections.xml")
