# Joins a ctest log with TIMEOUT properties from `ctest --show-only=json-v1`
# and prints (test, time, limit, ratio) sorted by ratio, largest first.
# Usage: nearest_limit.py CTEST_LOG TESTS_JSON [TOP_N]
# Tests without a TIMEOUT property use the ctest default of 1500 s.
import json
import re
import sys

DEFAULT_LIMIT = 1500.0
ROW = re.compile(r"Test\s+#(\d+):\s+(.*?)\s+\.{2,}\s*(?:\*\*\*)?(\S[^\n]*?)\s+([0-9.]+) sec")


def main(argv):
    if len(argv) < 3:
        print("usage: nearest_limit.py CTEST_LOG TESTS_JSON [TOP_N]", file=sys.stderr)
        return 2
    top = int(argv[3]) if len(argv) > 3 else 40
    limits = {}
    for test in json.load(open(argv[2]))["tests"]:
        limit = DEFAULT_LIMIT
        for prop in test.get("properties", []):
            if prop["name"] == "TIMEOUT":
                limit = float(prop["value"])
        limits[test["name"]] = limit
    rows = []
    passed = failed = 0
    for line in open(argv[1], errors="replace"):
        m = ROW.search(line)
        if not m:
            continue
        name, status, seconds = m.group(2), m.group(3).strip(), float(m.group(4))
        if status.startswith("Passed"):
            passed += 1
        else:
            failed += 1
        limit = limits.get(name)
        if limit is None:
            print(f"no TIMEOUT entry for {name!r}", file=sys.stderr)
            return 1
        rows.append((seconds / limit, name, seconds, limit, status))
    rows.sort(reverse=True)
    print(f"parsed {len(rows)} tests: passed {passed}, not passed {failed}")
    print(f"{'ratio':>7} {'time_s':>9} {'limit_s':>8}  test")
    for ratio, name, seconds, limit, status in rows[:top]:
        flag = "" if status.startswith("Passed") else f"  [{status}]"
        print(f"{ratio:7.3f} {seconds:9.2f} {limit:8.0f}  {name}{flag}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
