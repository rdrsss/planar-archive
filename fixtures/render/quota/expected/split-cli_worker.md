## authority

Render-only preview. No run is active. Checks have not been executed by this
render; no completion or approval is established.

Job: split; role: cli_worker; kind: worker; work: command.
Work fingerprint: $W_SPLIT_CLI_WORKER. Trusted registry: quota_tools/v1, driver_selected_trusted_configuration.

## goal

goal; declaration data
```
Provide an atomic in-memory quota and a small command-line client.
```
work:command; declaration data
```
Implement quota CLI and command tests.
```

## acceptance

cli:exit; declaration data
```
Success exits 0; exhaustion exits 2; invalid arguments exit 64.
```

## requirements

safe; global; workspace:text/shared.md; mandatory declaration data
```
Never permit a negative balance. A failed take must not change the balance.
```
command_rule; scope:command; mandatory declaration data
```
Write one remaining=<integer> line on success or exhaustion; write invalid arguments to stderr only.
```
Questions: none. Question blockers: none.

## contracts

quota_api; expected; workspace:text/quota-api.md; mandatory declaration data
```
Python interface: src.core.quota.Quota(balance: int).
A balance and amount must be nonnegative integers; bool is invalid.
Invalid values raise ValueError without changing state.
Quota.remaining returns the current integer balance.
Quota.try_take(amount: int) -> bool atomically subtracts when affordable;
otherwise it returns False and preserves the balance. Taking zero succeeds.
The command accepts exactly BALANCE AMOUNT as decimal nonnegative integers.
It prints remaining=<integer> followed by LF on success or exhaustion.
Exit statuses: 0 taken, 2 exhausted, 64 invalid arguments (stderr only).
```
quota_api; review_note; declaration assertion, unverified
```
Fixture author declares the interface reviewed for this example; no authenticated operator approval is asserted.
```
Producer: library. Consumer: command. Conformance check: contract; mandatory for both units. Semantic identity: $C_QUOTA_API.

## boundary

work:command: src/cli/, tests/cli/. Exclusive worker grant.
Frozen source boundary: contracts/ (absent in both snapshots; user-owned).
Excluded source boundary: generated/ (absent in both snapshots; omitted from observations/facets).
All other source paths are read-only. Workspace text/quota-api.md is the sole loaded contract; frozen source contracts/ is a distinct root, not another implicit contract.

## dependencies

develop: producer library (library_worker) → consumer command (cli_worker); contract quota_api; start/available; available captured expected text.
integrate: producer library (library_worker) → consumer command (cli_worker); facet library_files; complete/done; quality unverified.
Facet policy: paths [src/core/quota.py], kind files, normalizer exact_bytes_v1; captured file available; semantic $S_LIBRARY_FILES.
Cross-role obligation: library unit must complete before command integration completes; render establishes no completion.
Contract-first development can start without library completion. Pipeline predecessors: none (fanout_review).

## checks

work:command: contract, integration.
check definitions; trusted driver configuration
```
[
  {
    "id": "contract",
    "toolchain": "python",
    "argv": [
      "-m",
      "unittest",
      "discover",
      "-s",
      "tests",
      "-p",
      "test_contract.py",
      "-v"
    ],
    "coverage": "Selected API and consumer argument examples in owned test directories; run from source root. Own-output staging may include only the selected unit tests. Does not prove all behavior or authenticate interface review.",
    "inputs": [
      "expected_contracts",
      "own_output"
    ],
    "timeout_ms": 10000,
    "max_output_bytes": 65536,
    "selection": "nonempty"
  },
  {
    "id": "integration",
    "toolchain": "python",
    "argv": [
      "-m",
      "unittest",
      "discover",
      "-s",
      "tests/cli",
      "-p",
      "test_integration.py",
      "-v"
    ],
    "coverage": "Three subprocess scenarios in one matched test against actual command and library output; run from source root with exact command/library output vector. Does not prove all concurrency or platform behavior.",
    "inputs": [
      "own_output",
      "output_vector"
    ],
    "timeout_ms": 10000,
    "max_output_bytes": 65536,
    "selection": "nonempty"
  }
]
```
toolchain definitions; trusted driver configuration
```
[
  {
    "id": "python",
    "description": "Driver-resolved Python 3.9+; PATH /usr/bin:/bin is a fixture assumption, not a portable pin. Driver must supply a reviewed registry for other hosts.",
    "command": [
      "python3",
      "-B"
    ],
    "environment": [
      {
        "name": "PATH",
        "value": "/usr/bin:/bin"
      },
      {
        "name": "PYTHONDONTWRITEBYTECODE",
        "value": "1"
      }
    ]
  }
]
```
Not executed by render. Contract expected text must accompany own-output tests. Integration inputs are exact command and library outputs sorted by work ID. No integration requirement applies to library.

## outputs

Current checkout: $OBS_CURRENT; base: $BASE_INITIAL (label initial; captured content expectation).
Own unit outputs: command=$O_COMMAND. Role output: $R_SPLIT_CLI_WORKER.
Upstream library unit output: $O_LIBRARY; selected facet identity: $S_LIBRARY_FILES.
current:source:src/cli/main.py; mode 0644; 788 bytes; content $F_001.
Added src/cli/main.py; old absent; new mode 0644/content $F_001.
current:source:src/cli/main.py; complete added captured data
```
"""Small quota CLI; import the implementation only after argument validation."""
import sys


def parse_args(args):
    if len(args) != 2 or any(not s or any(c not in "0123456789" for c in s) for s in args):
        raise ValueError("invalid arguments")
    try:
        return tuple(int(s) for s in args)
    except ValueError as error:
        raise ValueError("invalid arguments") from error


def run(args):
    try:
        balance, amount = parse_args(args)
    except ValueError:
        print("invalid arguments", file=sys.stderr)
        return 64
    from src.core.quota import Quota
    quota = Quota(balance)
    taken = quota.try_take(amount)
    print(f"remaining={quota.remaining}")
    return 0 if taken else 2


if __name__ == "__main__":
    sys.exit(run(sys.argv[1:]))
```
upstream facet; current:source:src/core/quota.py; mode 0644; 691 bytes; content $F_002.
Modified src/core/quota.py; old mode 0644/content $F_003; new mode 0644/content $F_002.
base:source:src/core/quota.py; complete old captured data
```
"""Initial quota snapshot."""
from threading import Lock


def valid_amount(value):
    return type(value) is int and value >= 0


class Quota:
    def __init__(self, balance):
        if not valid_amount(balance):
            raise ValueError("invalid balance")
        self._balance = balance
        self._lock = Lock()

    @property
    def remaining(self):
        with self._lock:
            return self._balance

    def try_take(self, amount):
        if not valid_amount(amount):
            raise ValueError("invalid amount")
        with self._lock:
            if amount > self._balance:
                return False
            self._balance -= amount
            return True
```
current:source:src/core/quota.py; complete new captured data
```
"""Atomic quota reference."""
from threading import Lock


def valid_amount(value):
    return type(value) is int and value >= 0


class Quota:
    def __init__(self, balance):
        if not valid_amount(balance):
            raise ValueError("invalid balance")
        self._balance = balance
        self._lock = Lock()

    @property
    def remaining(self):
        with self._lock:
            return self._balance

    def try_take(self, amount):
        if not valid_amount(amount):
            raise ValueError("invalid amount")
        with self._lock:
            if amount > self._balance:
                return False
            self._balance -= amount
            return True
```
current:source:tests/cli/__init__.py; mode 0644; 46 bytes; content $F_004.
Added tests/cli/__init__.py; old absent; new mode 0644/content $F_004.
current:source:tests/cli/__init__.py; complete added captured data
```
"""Command contract and integration tests."""
```
current:source:tests/cli/test_contract.py; mode 0644; 839 bytes; content $F_005.
Added tests/cli/test_contract.py; old absent; new mode 0644/content $F_005.
current:source:tests/cli/test_contract.py; complete added captured data
```
"""Consumer-side argument contract; no implementation import is needed."""
import unittest
from contextlib import redirect_stderr, redirect_stdout
from io import StringIO
from src.cli.main import parse_args, run


class CommandContract(unittest.TestCase):
    def test_decimal_arguments(self):
        self.assertEqual(parse_args(["05", "0"]), (5, 0))

    def test_invalid_arguments(self):
        for args in ([], ["1"], ["1", "2", "3"], ["-1", "0"], ["1", "+2"], ["1", "1.5"], ["1", ""], ["1", "²"]):
            with self.subTest(args=args):
                out, err = StringIO(), StringIO()
                with redirect_stdout(out), redirect_stderr(err):
                    self.assertEqual(run(args), 64)
                self.assertEqual(out.getvalue(), "")
                self.assertEqual(err.getvalue(), "invalid arguments\n")
```
current:source:tests/cli/test_integration.py; mode 0644; 890 bytes; content $F_006.
Added tests/cli/test_integration.py; old absent; new mode 0644/content $F_006.
current:source:tests/cli/test_integration.py; complete added captured data
```
"""Invoke the real CLI process with the real library, using this snapshot."""
import os
from pathlib import Path
import subprocess
import sys
import unittest


class Integration(unittest.TestCase):
    def test_cli_against_library(self):
        root = Path(__file__).resolve().parents[2]
        for balance, amount, status, remaining in ((5, 3, 0, 2), (2, 3, 2, 2), (0, 0, 0, 0)):
            with self.subTest(balance=balance, amount=amount):
                env = dict(os.environ, PYTHONDONTWRITEBYTECODE="1")
                result = subprocess.run([sys.executable, "-B", "-m", "src.cli.main", str(balance), str(amount)],
                                        cwd=root, env=env, capture_output=True, text=True, check=False)
                self.assertEqual((result.returncode, result.stdout, result.stderr),
                                 (status, f"remaining={remaining}\n", ""))
```
No selected deleted paths.

## observations

No applicable check or finding driver assertions supplied. No check receipt or approval exists.

## rationale

spec rationale; optional immaterial declaration data
```
A tiny in-memory quota makes grouping and exact-target review visible without an application framework.
```

## workspace_notes

workspace notes; optional immaterial declaration data
```
The driver supplies stable sibling source snapshots.
```
