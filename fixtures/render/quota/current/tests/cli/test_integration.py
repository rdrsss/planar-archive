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
