## authority

Render-only preview. No run is active. Checks have not been executed by this
render; no completion or approval is established.

Job: split; role: library_worker; kind: worker; work: library.
Work fingerprint: $W_SPLIT_LIBRARY_WORKER. Trusted registry: quota_tools/v1, driver_selected_trusted_configuration.

## goal

goal; declaration data
```
Provide an atomic in-memory quota and a small command-line client.
```
work:library; declaration data
```
Implement Quota.try_take and library tests.
```

## acceptance

api:atomic; declaration data
```
A rejected take preserves the balance, including under concurrent calls.
```

## requirements

safe; global; workspace:text/shared.md; mandatory declaration data
```
Never permit a negative balance. A failed take must not change the balance.
```
library_rule; scope:library; mandatory declaration data
```
Serialize balance reads and updates with one instance lock; reject booleans as amounts.
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

work:library: src/core/, tests/core/. Exclusive worker grant.
Frozen source boundary: contracts/ (absent in both snapshots; user-owned).
Excluded source boundary: generated/ (absent in both snapshots; omitted from observations/facets).
All other source paths are read-only. Workspace text/quota-api.md is the sole loaded contract; frozen source contracts/ is a distinct root, not another implicit contract.

## dependencies

No dependencies consumed by library. No pipeline predecessors. The produced quota_api contract remains mandatory above; no downstream command prescription or integration check applies to this worker.

## checks

work:library: contract.
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
Own unit outputs: library=$O_LIBRARY. Role output: $R_SPLIT_LIBRARY_WORKER.
current:source:src/core/quota.py; mode 0644; 691 bytes; content $F_002.
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
current:source:tests/core/__init__.py; mode 0644; 30 bytes; content $F_007.
Unchanged from base (0644); current content follows once.
current:source:tests/core/__init__.py; captured data
```
"""Library contract tests."""
```
current:source:tests/core/test_contract.py; mode 0644; 1227 bytes; content $F_008.
Unchanged from base (0644); current content follows once.
current:source:tests/core/test_contract.py; captured data
```
"""Exercise the actual library interface, including contention."""
import unittest
from concurrent.futures import ThreadPoolExecutor
from src.core.quota import Quota


class LibraryContract(unittest.TestCase):
    def test_take_and_exhaustion(self):
        quota = Quota(5)
        self.assertTrue(quota.try_take(3))
        self.assertFalse(quota.try_take(3))
        self.assertEqual(quota.remaining, 2)
        self.assertTrue(quota.try_take(0))
        self.assertEqual(quota.remaining, 2)

    def test_invalid_balance(self):
        for value in (-1, True, 1.5, "3"):
            with self.subTest(value=value), self.assertRaises(ValueError):
                Quota(value)

    def test_invalid_amount_is_atomic(self):
        quota = Quota(4)
        for value in (-1, False, 1.5, "3"):
            with self.subTest(value=value), self.assertRaises(ValueError):
                quota.try_take(value)
            self.assertEqual(quota.remaining, 4)

    def test_contended_takes(self):
        quota = Quota(17)
        with ThreadPoolExecutor(max_workers=8) as pool:
            results = list(pool.map(quota.try_take, [1] * 100))
        self.assertEqual(sum(results), 17)
        self.assertEqual(quota.remaining, 0)
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
