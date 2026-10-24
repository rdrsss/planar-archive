## authority

Render-only preview. No run is active. Checks have not been executed by this
render; no completion or approval is established.

Job: split; role: library_worker; kind: worker; work: library.
Work fingerprint: xxh3_128:56eb90066a90f7ebbc5c8e125bd65d02. Trusted registry: quota_tools/v1, driver_selected_trusted_configuration.

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

safe; scope:global; mandatory declaration data; workspace:text/shared.md
```
Never permit a negative balance. A failed take must not change the balance.
```
library_rule; scope:library; mandatory declaration data
```
Serialize balance reads and updates with one instance lock; reject booleans as amounts.
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

work:library: src/core/, tests/core/. Exclusive worker grant.
Frozen source boundaries: contracts/.
Excluded source boundaries: generated/.
All other source paths are read-only.

## dependencies

Explicit dependencies: none.
Pipeline predecessors: none (fanout_review).
Render establishes no completion or acceptance; quality obligations remain unverified.

## checks

work:library: contract.
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
Unit output library: xxh3_128:799d60775f9f630bf87d9f3fee5c8bd4.
Role output: xxh3_128:45973194a7668dc030e8b9dbd903eede.
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
Unchanged tests/core/__init__.py.
current:source:tests/core/__init__.py; current; mode 0644; 30 bytes; complete captured data
```
"""Library contract tests."""
```
Unchanged tests/core/test_contract.py.
current:source:tests/core/test_contract.py; current; mode 0644; 1227 bytes; complete captured data
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

rationale; optional immaterial declaration data
```
A tiny in-memory quota makes grouping and exact-target review visible without an application framework.
```

## workspace_notes

workspace_notes; optional immaterial declaration data
```
The driver supplies stable sibling source snapshots.
```
