# Lists flattened unittest ids of the four support modules, loaded from the support directory.
import sys
import unittest

MODULES = ["accept_test", "acceptance_cli_test", "quota_live_test", "fault_live_test"]


def flatten(suite):
    for item in suite:
        if isinstance(item, unittest.TestSuite):
            yield from flatten(item)
        else:
            yield item


def main():
    loader = unittest.TestLoader()
    ids = []
    for name in MODULES:
        suite = loader.loadTestsFromName(name)
        if suite.countTestCases() == 0:
            print(f"ZERO tests in {name}", file=sys.stderr)
            return 3
        for t in flatten(suite):
            if type(t).__name__ == "_FailedTest":
                print(f"IMPORT FAILED in {name}: {t.id()}", file=sys.stderr)
                return 4
            ids.append(t.id())
    for line in sorted(ids):
        print(line)
    return 0


if __name__ == "__main__":
    sys.exit(main())
