## authority

Render-only preview. No run is active. Checks have not been executed by this
render; no completion or approval is established.

Job: split; role: cli_worker; kind: worker; work: command.
Work fingerprint: xxh3_128:915ccd272d476ba2c38828095cbb6442. Trusted registry: quota_tools/v1, driver_selected_trusted_configuration.

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

safe; scope:global; mandatory declaration data; workspace:text/shared.md
```
Never permit a negative balance. A failed take must not change the balance.
```
command_rule; scope:command; mandatory declaration data
```
Write one remaining=<integer> line on success or exhaustion; write invalid arguments to stderr only.
```
Questions: none. Question blockers: none.

## contracts

quota_api; expected; mandatory declaration data; workspace:text/quota-api.md
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
Producer: library. Consumers: command. Conformance check: contract. Semantic identity: xxh3_128:d420cba7d6f3772221b4d8d2d635c24c.

## boundary

work:command: src/cli/, tests/cli/. Exclusive worker grant.
Frozen source boundaries: contracts/.
Excluded source boundaries: generated/.
All other source paths are read-only.

## dependencies

Dependency develop; source: contract; reference: quota_api.
Producer: library (role library_worker); consumer: command (role cli_worker).
Phase: start; quality: available; cross-role obligation; status: available.
Dependency integrate; source: facet; reference: library_files.
Producer: library (role library_worker); consumer: command (role cli_worker).
Phase: complete; quality: done; cross-role obligation; status: unverified.
Facet library_files; producer: library; kind: files; normalizer: exact_bytes_v1; status: available.
Facet library_files paths; canonical JSON (string controls escaped)
```
["src/core/quota.py"]
```
Facet library_files captured file; mode 0644; 691 bytes
```
source:src/core/quota.py
```
Pipeline predecessors: none (fanout_review).
Render establishes no completion or acceptance; quality obligations remain unverified.

## checks

work:command: contract, integration.
Check contract; toolchain: python.
Timeout: 10000 ms; output cap: 65536 bytes; selection: nonempty.
Check contract argv; exact argument boundaries; canonical JSON (string controls escaped)
```
["-m","unittest","discover","-s","tests","-p","test_contract.py","-v"]
```
Check contract inputs; canonical JSON (string controls escaped)
```
["expected_contracts","own_output"]
```
Check contract coverage; trusted driver configuration
```
Selected API and consumer argument examples in owned test directories; run from source root. Own-output staging may include only the selected unit tests. Does not prove all behavior or authenticate interface review.
```
Check integration; toolchain: python.
Timeout: 10000 ms; output cap: 65536 bytes; selection: nonempty.
Check integration argv; exact argument boundaries; canonical JSON (string controls escaped)
```
["-m","unittest","discover","-s","tests/cli","-p","test_integration.py","-v"]
```
Check integration inputs; canonical JSON (string controls escaped)
```
["output_vector","own_output"]
```
Check integration coverage; trusted driver configuration
```
Three subprocess scenarios in one matched test against actual command and library output; run from source root with exact command/library output vector. Does not prove all concurrency or platform behavior.
```
Toolchain python; trusted driver configuration.
Toolchain python command argv; exact argument boundaries; canonical JSON (string controls escaped)
```
["python3","-B"]
```
Toolchain python environment; canonical JSON (string controls escaped)
```
[{"name":"PATH","value":"/usr/bin:/bin"},{"name":"PYTHONDONTWRITEBYTECODE","value":"1"}]
```
Toolchain python description
```
Driver-resolved Python 3.9+; PATH /usr/bin:/bin is a fixture assumption, not a portable pin. Driver must supply a reviewed registry for other hosts.
```
Not executed by render. Commands describe checks; coverage is a driver assertion, unverified. No check receipt exists.

## outputs

Current checkout and base captured.
Base label; driver declaration data
```
initial
```
Unit output command: xxh3_128:26251223d01bac7281172de5abd61dc4.
Unit output library: xxh3_128:799d60775f9f630bf87d9f3fee5c8bd4.
Role output: xxh3_128:8ca7563c2aa52e87b4802394a730a5f1.
Added src/cli/main.py; old absent.
current:source:src/cli/main.py; new; mode 0644; 788 bytes; complete captured data
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
Modified src/core/quota.py.
base:source:src/core/quota.py; old; mode 0644; 691 bytes; complete captured data
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
current:source:src/core/quota.py; new; mode 0644; 691 bytes; complete captured data
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
Added tests/cli/__init__.py; old absent.
current:source:tests/cli/__init__.py; new; mode 0644; 46 bytes; complete captured data
```
"""Command contract and integration tests."""
```
Added tests/cli/test_contract.py; old absent.
current:source:tests/cli/test_contract.py; new; mode 0644; 839 bytes; complete captured data
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
Added tests/cli/test_integration.py; old absent.
current:source:tests/cli/test_integration.py; new; mode 0644; 890 bytes; complete captured data
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

rationale; optional immaterial declaration data
```
A tiny in-memory quota makes grouping and exact-target review visible without an application framework.
```

## workspace_notes

workspace_notes; optional immaterial declaration data
```
The driver supplies stable sibling source snapshots.
```
