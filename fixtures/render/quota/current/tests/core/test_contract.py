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
