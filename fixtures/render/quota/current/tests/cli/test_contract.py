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
